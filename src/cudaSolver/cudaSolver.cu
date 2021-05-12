/**
 * @defgroup   CUDA_SOLVER
 *
 * @brief      This file implements tournament APIs which can allow humans/solvers
 * to play against each other.
 *
 * @author     Robert Viramontes (except where otherwise noted)
 * @date       2021
 */

#include "cudaSolver.cuh"

#include <iostream>
#include "cublas_v2.h"

// There are 4 streak directions: horizontal, vertical, '/' and '\'
#define NUM_STREAK_DIR 4

// Current implementation seems to be limited to a depth of 6 because of memory requirements
#define MAX_DEPTH 6

// Error check macro
// Based on https://github.com/NVIDIA-developer-blog/code-samples/blob/master/posts/tensor-cores/simpleTensorCoreGEMM.cu
#define cudaErrCheck(stat) { cudaErrCheck_((stat), __FILE__, __LINE__); }
inline void cudaErrCheck_(cudaError_t stat, const char *file, int line) {
   if (stat != cudaSuccess) {
      fprintf(stderr, "CUDA Error: %s %s %d\n", cudaGetErrorString(stat), file, line);
      abort();
   }
}
__global__ void findSlotsOnBoard(const SlotStatus* concat_board, const int concat_board_len, const SlotStatus color, uint8_t updateBoard[])
{
    size_t idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < concat_board_len) {
        updateBoard[idx] = concat_board[idx] == color ? 1 : 0;
    }
}

__global__ void findStreaksForLength(const uint8_t *board, int width, int height, double *scores, int stencilLength, const int startBoard, const int entriesPerBoard) {
    auto numSlots = width*height;
    if (threadIdx.x >= numSlots*NUM_STREAK_DIR)
        return;
    
    int matched = 0;

    bool stencilCanGoRight = threadIdx.x % width <= width - stencilLength;
    bool stencilCanGoLeft = threadIdx.x % width >= stencilLength - 1;
    bool stencilCanGoUp = (threadIdx.x/width ) % height <= height - stencilLength;

    int boardStartIndex = (blockIdx.x + startBoard) * numSlots;

    if (threadIdx.x < numSlots) {
        // Check horizontal scores
        if (stencilCanGoRight) {
            for (int i = 0; i < stencilLength; i++) {
                matched += board[threadIdx.x + i + boardStartIndex];
            }
        }           
    } else if (threadIdx.x < 2*numSlots) {
        // Check vertical scores
        if (stencilCanGoUp) {
            for (int i = 0; i < stencilLength; i++) {
                matched += board[(threadIdx.x-numSlots) + (i*width) + boardStartIndex];
            }
        }
    } else if (threadIdx.x < 3*numSlots) {
        // Check \ diagonal
        if (stencilCanGoLeft && stencilCanGoUp) {
            for (int i = 0; i < stencilLength; i++) {
                matched += board[(threadIdx.x-2*numSlots) + i*width - i + boardStartIndex];
            }
        }
    } else if (threadIdx.x < 4*numSlots) {
        // Check / diagonal
        if (stencilCanGoRight && stencilCanGoUp) {
            for (int i = 0; i < stencilLength; i++) {
                matched += board[(threadIdx.x-3*numSlots) + i*width + i + boardStartIndex];
            }
        }
    }

    scores[threadIdx.x + (stencilLength-2)*numSlots*NUM_STREAK_DIR + (blockIdx.x + startBoard) * entriesPerBoard] = matched == stencilLength ? 1 : 0;
}

CudaSolver::CudaSolver(int width, int height, int winningStreakSize)
{
    _board = new Board(width, height, winningStreakSize);
    // Set a reasonable number of streams so we don't spend a ton of time spinning up streams.
    _numStreams = 32;
    _streams = new cudaStream_t[_numStreams];
    cudaStream_t* streams = (cudaStream_t*)_streams;
    
    for (int i = 0; i < _numStreams; i++) {
        cudaStreamCreate(&streams[i]);
    }
}

CudaSolver::~CudaSolver() 
{
    cudaStream_t* streams = (cudaStream_t*)_streams;
    for (int i = 0; i < _numStreams; i++) {
        cudaStreamDestroy(streams[i]);
    } 

    delete[] streams;
}

int CudaSolver::solve(Player player, int maxDepth, double time_limit) 
{
    if (_board->DetermineWinner() != Player::None) {
        return -1;
    }

	_nodesTraversed = 0;
	int retval = -1;
	int bestMove = -1;
	auto nodesTraversed = _nodesTraversed;

	if(time_limit > 0) {
		// Implement Iterative Deepening to adhere to a time limit per move
		this->startTimer();
		for (int depth = 2; this->isTimeLeft(time_limit) && depth <= MAX_DEPTH; depth += 2) {
			// Find the best move
			int move = this->findBestMove2(_board->getBoard(), player, depth);
			if(this->isTimeLeft(time_limit)) {
				bestMove = move;
				nodesTraversed = _nodesTraversed;
				_nodesTraversed = 0;
			}
		}
		_nodesTraversed = nodesTraversed;
	}
	else {
		bestMove = this->findBestMove2(_board->getBoard(), player, maxDepth);
	}

	if(_board->IsFull()) {
		std::cout << "Board is full" << std::endl;
		return -1;
	}

	if(bestMove > -1) {
		_board->playMove(bestMove, SlotStatusHelpers::getSlotFromPlayer(player));
		retval = bestMove % _board->getWidth();
	}

	Player winner = _board->DetermineWinner();
	if(winner != Player::None) {
		std::cout << "Player: " << ((winner == Player::Red)?"Red ":"Yellow ")
		<< "is the winner" << std::endl;
		return -1;
	}
	_totalNodesTraversed += _nodesTraversed;
    return retval;
}

int CudaSolver::EvaluateBoard(SlotStatus* board, Player player)
{
    int numSlots =  _board->getWidth() * _board->getHeight();
    // subtract one from winning streak size since there are no streaks of size 1
    int numStreakLengths = _board->getWinningStreakSize() - 1; 
    int entriesPerBoard = numSlots*NUM_STREAK_DIR*numStreakLengths;
    uint8_t *player_board;
    uint8_t *opp_board;
    cudaMallocManaged(&player_board, numSlots*sizeof(uint8_t));
    cudaMallocManaged(&opp_board, numSlots*sizeof(uint8_t));

    double* d_player_streak_counts;
    double* d_opp_streak_counts;
    cudaMalloc(&d_player_streak_counts, entriesPerBoard*sizeof(double));
    cudaMalloc(&d_opp_streak_counts, entriesPerBoard*sizeof(double));

    double *h_x = new double[entriesPerBoard];
    double *h_y = new double[1];
    for (int strk_len = 0; strk_len < numStreakLengths; strk_len++) {
        for (int i = 0; i < numSlots*NUM_STREAK_DIR; i++) {
            h_x[strk_len*numSlots*NUM_STREAK_DIR + i] = pow(strk_len+2, 3);
        }
    }
    h_y[0] = 0;

    double *d_x;
    double *d_y;
    cudaMalloc(&d_x, entriesPerBoard*sizeof(double));
    cudaMalloc(&d_y, sizeof(double));

    cudaMemcpy(d_x, h_x, entriesPerBoard*sizeof(double), cudaMemcpyHostToDevice);
    cudaMemset(d_y, 0, sizeof(double));

    SlotStatus slotToMatch = SlotStatusHelpers::getSlotFromPlayer(player);
    SlotStatus oppSlot = SlotStatusHelpers::getSlotFromPlayer(PlayerHelpers::OppositePlayer(player));
    for (int i = 0; i < numSlots; i++) {
        player_board[i] = board[i] == slotToMatch ? 1 : 0;
        opp_board[i] = board[i] == oppSlot ? 1 : 0;
    }

    cudaStream_t player_stream, opp_stream;
    cudaStreamCreate(&player_stream);
    cudaStreamCreate(&opp_stream);

    for (int n = 0; n < numStreakLengths; n++)
    {
        auto index = n*numSlots*NUM_STREAK_DIR;
        // launch one worker kernel per stream
        findStreaksForLength<<<1,256,0, player_stream>>>(
            player_board, 
            _board->getWidth(), 
            _board->getHeight(), 
            &d_player_streak_counts[index], 
            n+2, 0, entriesPerBoard);
        
        findStreaksForLength<<<1,256,0, opp_stream>>>(
            opp_board, 
            _board->getWidth(), 
            _board->getHeight(), 
            &d_opp_streak_counts[index], 
            n+2, 0, entriesPerBoard);
    }

    // Make sure all kernels are done executing
    cudaDeviceSynchronize();

    cublasStatus_t stat;
    cublasHandle_t handle;

    double alpha = 1;
    double beta = 0;

    stat = cublasCreate(&handle);
    if (stat != CUBLAS_STATUS_SUCCESS) {
        printf ("CUBLAS initialization failed\n");
        return EXIT_FAILURE;
    }

    stat = cublasDgemv(
        handle, CUBLAS_OP_T,
        entriesPerBoard, 1, 
        &alpha,
        d_player_streak_counts, entriesPerBoard,
        d_x, 1,
        &beta, 
        d_y, 1);
    if (stat != CUBLAS_STATUS_SUCCESS) {
        printf("CUBLAS gemv failed\n");
        return 0;
    }

    alpha = -1;
    beta = 1;

    // auto h_temp = new double[numBoards];
    // cudaMemcpy(h_temp, d_y, numBoards*sizeof(double), cudaMemcpyDeviceToHost);
    // for (int i = 0; i < numBoards; i++) {
    //     // see if anything happened
    //     std:: cout << h_temp[i] << " ";
    // }
    // std::cout << std::endl;

    stat = cublasDgemv(
        handle, CUBLAS_OP_T,
        entriesPerBoard, 1, 
        &alpha,
        d_opp_streak_counts, entriesPerBoard,
        d_x, 1,
        &beta, 
        d_y, 1);
    if (stat != CUBLAS_STATUS_SUCCESS) {
        printf("CUBLAS gemv failed\n");
        return 0;
    }

    cudaMemcpy(h_y, d_y, sizeof(double), cudaMemcpyDeviceToHost);
    auto score = h_y[0];

    // for (int i = 0; i < num_streams; i++) {
    //     // for (int j = 0; j < 3; j++)
    //     // {
    //     //     std::cout << streak_counts[i][j] << " streaks of " << j+2 << " ";
    //     // }
    //     // std::cout << std::endl;
    //     cudaStreamDestroy(streams[i]);
    // }

    cublasDestroy(handle);
    cudaStreamDestroy(player_stream);
    cudaStreamDestroy(opp_stream);
    cudaFree(player_board);
    cudaFree(opp_board);
    cudaFree(d_player_streak_counts);
    cudaFree(d_opp_streak_counts);
    cudaFree(d_x);
    cudaFree(d_y);

    delete[] h_x;
    delete[] h_y;

    return score;
}

int CudaSolver::EvaluateBoards(const SlotStatus* concat_boards, const int numBoards, const Player player, double scores[])
{
    cudaStream_t* streams = (cudaStream_t*)_streams;
    const int boardWidth = _board->getWidth();
    const int boardHeight = _board->getHeight();
    const int numSlots = boardWidth * boardHeight;
    // subtract one from winning streak size since there are no streaks of size 1
    const int numStreakLengths = _board->getWinningStreakSize() - 1; 
    const int concat_boards_len = numSlots*numBoards;
    const int entriesPerBoard = numStreakLengths*numSlots*NUM_STREAK_DIR;

    uint8_t *d_player_board;
    uint8_t *d_opp_board;
    cudaErrCheck( cudaMalloc(&d_player_board, concat_boards_len*sizeof(uint8_t)) );
    cudaErrCheck( cudaMalloc(&d_opp_board, concat_boards_len*sizeof(uint8_t)) );

    SlotStatus* d_concat_boards;
    cudaErrCheck( cudaMalloc(&d_concat_boards, concat_boards_len*sizeof(SlotStatus)) );
    cudaErrCheck( cudaMemcpy(d_concat_boards, concat_boards, concat_boards_len*sizeof(SlotStatus), cudaMemcpyHostToDevice) );

    SlotStatus slotToMatch = SlotStatusHelpers::getSlotFromPlayer(player);
    SlotStatus oppSlot = SlotStatusHelpers::getSlotFromPlayer(PlayerHelpers::OppositePlayer(player));
    int numThreads = 256;
    int numBlocks = (concat_boards_len/256) + 1;
    findSlotsOnBoard<<<numBlocks, numThreads, 0, streams[0]>>>(d_concat_boards, concat_boards_len, slotToMatch, d_player_board);
    findSlotsOnBoard<<<numBlocks, numThreads, 0, streams[1]>>>(d_concat_boards, concat_boards_len, oppSlot, d_opp_board);

    double* d_player_streak_counts;
    double* d_opp_streak_counts;
    cudaErrCheck( cudaMalloc(&d_player_streak_counts, numBoards*entriesPerBoard*sizeof(double)) );
    cudaErrCheck( cudaMalloc(&d_opp_streak_counts, numBoards*entriesPerBoard*sizeof(double)) );

    double *h_x;
    cudaErrCheck( cudaMallocHost((void **)&h_x, entriesPerBoard*sizeof(double)) );
    for (int strk_len = 0; strk_len < numStreakLengths; strk_len++) {
        for (int i = 0; i < numSlots*NUM_STREAK_DIR; i++) {
            h_x[strk_len*numSlots*NUM_STREAK_DIR + i] = pow(strk_len+2, 3);
        }
    }

    double *d_x;
    double *d_y;
    cudaErrCheck( cudaMalloc(&d_x, entriesPerBoard*sizeof(double)) );
    cudaErrCheck( cudaMalloc(&d_y, numBoards*sizeof(double)) );

    // Throw this on the last-used stream so it can queue up and we can start launching kernels without waiting for this, since it's used later
    cudaMemcpyAsync(d_x, h_x, entriesPerBoard*sizeof(double), cudaMemcpyHostToDevice, streams[_numStreams-1]);
    cudaErrCheck( cudaMemsetAsync(d_y, 0, numBoards*sizeof(double), streams[_numStreams-1]) );

    const int boardsPerLaunch = 32;
    int stream = -1;
    for (int b = 0; b < numBoards; b += 32)
    {
        stream++;
        if (stream % _numStreams == 0) { stream = 0; }
        for (int n = 0; n < numStreakLengths; n++)
        {
            int blocksToLaunch = (b + boardsPerLaunch < numBoards) ? boardsPerLaunch : (numBoards % boardsPerLaunch) ;

            // launch one worker kernel per stream
            findStreaksForLength<<<blocksToLaunch,256, 0, streams[stream]>>>(
                d_player_board, 
                boardWidth, 
                boardHeight, 
                d_player_streak_counts,
                n+2,
                b,
                entriesPerBoard);

            findStreaksForLength<<<blocksToLaunch,256, 0, streams[stream]>>>(
                d_opp_board, 
                boardWidth, 
                boardHeight, 
                d_opp_streak_counts,
                n+2,
                b,
                entriesPerBoard);
        }
    }

    cudaFreeHost(h_x);
    cudaFree(d_concat_boards);
    cudaFree(d_player_board);
    cudaFree(d_opp_board);

    // At this point, we want to make sure that all the work we've queued up is done before we do the matrix multiplies
    // I think this is redundant since cudaFree is synchronizing, but keep it just for clarity
    cudaDeviceSynchronize();

    cublasStatus_t stat;
    cublasHandle_t handle;

    double alpha = 1;
    double beta = 0;

    stat = cublasCreate(&handle);
    if (stat != CUBLAS_STATUS_SUCCESS) {
        printf ("CUBLAS initialization failed\n");
        return EXIT_FAILURE;
    }

    stat = cublasDgemv(
        handle, CUBLAS_OP_T,
        entriesPerBoard, numBoards, 
        &alpha,
        d_player_streak_counts, entriesPerBoard,
        d_x, 1,
        &beta, 
        d_y, 1);
    if (stat != CUBLAS_STATUS_SUCCESS) {
        printf("CUBLAS gemv failed\n");
        return 2;
    }

    // Setting alpha = -1 and beta = 1 allows us to use this call to 
    // do player - opponent (since it's beta*player + alpha*opponent)
    alpha = -1;
    beta = 1;

    stat = cublasDgemv(
        handle, CUBLAS_OP_T,
        entriesPerBoard, numBoards, 
        &alpha,
        d_opp_streak_counts, entriesPerBoard,
        d_x, 1,
        &beta, 
        d_y, 1);
    if (stat != CUBLAS_STATUS_SUCCESS) {
        printf("CUBLAS gemv failed\n");
        return 3;
    }

    cublasDestroy(handle);

    cudaFree(d_player_streak_counts);
    cudaFree(d_opp_streak_counts);

    cudaMemcpy(scores, d_y, numBoards*sizeof(double), cudaMemcpyDeviceToHost);

    cudaFree(d_x);
    cudaFree(d_y);

    return 0;
}

bool CudaSolver::isLegalMove(const SlotStatus *board, int width, int height, int index) 
{
    if (board[index] != SlotStatus::Empty) return false;
    int row = index / width;
    if(row == height - 1) return true;
    if(board[index + width] == SlotStatus::Empty) return false;
    return true;
}

void CudaSolver::findBoards(SlotStatus* board, const int width, const int height, const SlotStatus pieceToPlace, SlotStatus* &concatBoard, int &numBoards)
{
    std::vector<SlotStatus *> boards;
    auto numSlots = width*height;
    for (int i = numSlots - 1; i >= 0; i--) {
        if (isLegalMove(board, width, height, i)) {
            auto copied_board = new SlotStatus[width*height];

            // Copy the board over
            for (int j = 0; j < numSlots; j++) copied_board[j] = board[j];

            copied_board[i] = pieceToPlace;
            boards.push_back(copied_board);
        }
    }

    concatBoard = new SlotStatus[boards.size()*numSlots];
    numBoards = boards.size();

    for (int i = 0; i < boards.size(); i++) {
        for(int j = 0; j < numSlots; j++) concatBoard[i*numSlots + j] = boards.at(i)[j];
        delete[] boards.at(i);
    }
}

void CudaSolver::findBoards2(const SlotStatus* board, const int width, const int height, const Player player, const int depth, std::vector<CudaSolver::boardAndPath> *completed_boards, std::vector<int> pathSoFar)
{
    // Iterate through all possible moves to find legal moves.
    for (int i = width * height - 1; i >= 0; i--) {
        if (isLegalMove(board, width, height, i)) {
            auto copied_board = new SlotStatus[width*height];
            std::vector<int> nextPath;
            std::copy(pathSoFar.begin(), pathSoFar.end(), back_inserter(nextPath));
            nextPath.push_back(i);

            // Copy the board over
            for (int j = 0; j < width*height; j++) copied_board[j] = board[j];

            copied_board[i] = SlotStatusHelpers::getSlotFromPlayer(player);

            if (depth == 1) {
                // If we're at the bottom of the stack, add the boards
                // with its path to the completed_boards
                CudaSolver::boardAndPath t;
                t.board = copied_board;
                t.path = nextPath;
                completed_boards->push_back(t);
            } else {
                findBoards2(copied_board, width, height, PlayerHelpers::OppositePlayer(player), depth-1, completed_boards, nextPath);
                delete[] copied_board;
            }
        }
    }
}

uint32_t CudaSolver::createPathMapping(const std::vector<int> *path)
{
    uint32_t pathMapping = 0;
    uint32_t multiplier = 1;
    for (int p = path->size() -1; p >= 0; p--) {
        pathMapping += path->at(p) * multiplier;
        multiplier *= 100;
    }

    return pathMapping;
}

int CudaSolver::findBestMove(SlotStatus* board, Player player, int maxDepth)
{
    if (_board->IsFull()) return -1;

    auto color = SlotStatusHelpers::getSlotFromPlayer(player);
    int move = -1;
    int bestScore = INT_MIN;

    // Traverse through the board to find legal moves and see the maximum score
	// Since the board fills from the last row, it's better to traverse the 
	// board in a reverse order

    std::vector<boardAndPath> endNodes;

	for(int i = _board->getWidth() * _board->getHeight() - 1; i >= 0 ; i--) {
		if(board[i] == SlotStatus::Empty && _board->isLegalMove(i)) {
			board[i] = color;
			int score = this->minimax(board, maxDepth, player, false);
        
			board[i] = SlotStatus::Empty;
			if(score > bestScore) {
				move = i % _board->getWidth() + 1;
				bestScore = score;
			}
		}
	}

	return move;
}

int CudaSolver::findBestMove2(SlotStatus* board, Player player, int maxDepth)
{
    if (_board->IsFull()) return -1;

    auto color = SlotStatusHelpers::getSlotFromPlayer(player);
    int move = -1;
    int bestScore = INT_MIN;

    // Traverse through the board to find legal moves and see the maximum score
	// Since the board fills from the last row, it's better to traverse the 
	// board in a reverse order

    std::vector<boardAndPath> endNodes;
    std::vector<int> pathSoFar;
    findBoards2(board, _board->getWidth(), _board->getHeight(), player, maxDepth, &endNodes, pathSoFar);
    int numEndNodes = endNodes.size();
    _nodesTraversed = endNodes.size();
    int numSlots =  _board->getWidth() * _board->getHeight();
    auto concatBoards = new SlotStatus[numEndNodes * numSlots];

    std::unordered_map<uint32_t, int> pathToIndex;
    for (int n = 0; n < numEndNodes; n++) {
        for(int j = 0; j < numSlots; j++) concatBoards[n*numSlots + j] = endNodes.at(n).board[j];
        // for (auto p : endNodes.at(n).path){
        //     std::cout << p << ", ";
        // }
        // std::cout <<std::endl;
        pathToIndex[createPathMapping(&endNodes.at(n).path)] = n;
        delete[] endNodes.at(n).board;
    }

    auto scores = new double[numEndNodes];

    cudaEvent_t start;
    cudaEvent_t stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start);

    EvaluateBoards(concatBoards, numEndNodes, player, scores);

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);

    float ms;
    cudaEventElapsedTime(&ms, start, stop);
    // Uncoment to print GPU-intensive timing part
    // std::cout << "time in GPU intense part: " << ms << std::endl;

    // std::cout << "scores ";
    // for (int as = 0; as < numEndNodes; as++) {
    //     std::cout << scores[as] << ", ";
    // }
    // std::cout <<std::endl;

	for(int i = _board->getWidth() * _board->getHeight() - 1; i >= 0 ; i--) {
		if(board[i] == SlotStatus::Empty && _board->isLegalMove(i)) {
			board[i] = color;
            std::vector<int> miniMaxPath = {i};
            auto score = minimax2(board, scores, maxDepth-1, player, false, miniMaxPath, &pathToIndex);        
    		board[i] = SlotStatus::Empty;

            // std::cout << "move " << i << " scored " << score << std::endl;

			if(score > bestScore) {
				move = i;
				bestScore = score;
			}
		}
	}

    delete[] scores;

	return move;
}

int CudaSolver::minimax(SlotStatus* board, int depth, Player player, bool maximizer)
{
    // Board evaluations are static: The player won't change, it will always be
	// the maximizer wrt whom the score will be calculated.

    if (depth == 0)	return EvaluateBoard(board, player);

	// if(score == INT_MAX || score == INT_MIN) // someone has won already
	// 	return score;

	//if(depth == 0) return score;

	auto color = SlotStatusHelpers::getSlotFromPlayer(
        (maximizer ? player : PlayerHelpers::OppositePlayer(player)));
    
    // Last stop, find all boards at this level and process in parallel
    if (depth == 1) {
        SlotStatus* concatBoard;
        int numBoards = 1;
        findBoards(board, _board->getWidth(), _board->getHeight(), color,  concatBoard, numBoards);
        auto scores  = new double[numBoards];
        int max = INT_MIN;
        int min = INT_MAX;
        EvaluateBoards(concatBoard, numBoards, maximizer ? player : PlayerHelpers::OppositePlayer(player), scores);

        for (int i = 0; i < numBoards; i++) {
            if (scores[i] > max) {
                max = scores[i];
            }
            if (scores[i] < min) {
                min = scores[i];
        }
    }

        delete[] concatBoard;
        delete[] scores;

        return maximizer ? max : min;
    }
    int bestScore = maximizer ? INT_MIN : INT_MAX;
    for(int i = _board->getWidth() * _board->getHeight() - 1; i >= 0 ; i--) {
        if(isLegalMove(board, _board->getWidth(), _board->getHeight(), i)) {
            board[i] = color;
            auto score = this->minimax(board, depth - 1, player, !maximizer);
            bestScore = maximizer ? std::max(score, bestScore) : std::min(score, bestScore);
            board[i] = SlotStatus::Empty;
            ++_nodesTraversed;
        }
    }
    
	return bestScore;
}

int CudaSolver::minimax2(SlotStatus* board, const double *scores, const int depth, const Player player, const bool maximizer, std::vector<int> pathSoFar, const std::unordered_map<uint32_t, int> *pathToIndex)
{
    // Board evaluations are static: The player won't change, it will always be
	// the maximizer wrt whom the score will be calculated.
    int score;
    auto pathMapping = createPathMapping(&pathSoFar);

    // if (depth == 0) {
    //    for (auto p : pathSoFar){
    //                 std::cout << p << ", ";
    //             }
    //             std::cout << std::endl;
    // }

    // Can only do this if the path mapping exists
    if (pathToIndex->find(pathMapping) != pathToIndex->end()) {
        int scoresIndex = pathToIndex->at(pathMapping);
        score = scores[scoresIndex];

        if(score == INT_MAX || score == INT_MIN) // someone has won already
            return score;

        if(depth == 0) return score;
    }

    if (depth == 0) {
        std::cerr << "Ahh didn't find a matching path!!!!!!" << std::endl;
        exit(911);
    }

	auto color = SlotStatusHelpers::getSlotFromPlayer(
        (maximizer ? player : PlayerHelpers::OppositePlayer(player)));
    
    int bestScore = maximizer ? INT_MIN : INT_MAX;
    for(int i = _board->getWidth() * _board->getHeight() - 1; i >= 0 ; i--) {
        if(isLegalMove(board, _board->getWidth(), _board->getHeight(), i)) { 
            std::vector<int> nextPath;
            std::copy(pathSoFar.begin(), pathSoFar.end(), back_inserter(nextPath));
            nextPath.push_back(i);
            
            board[i] = color;
            auto score = this->minimax2(board, scores, depth - 1, player, !maximizer, nextPath, pathToIndex);
            board[i] = SlotStatus::Empty;
            bestScore = maximizer ? std::max(score, bestScore) : std::min(score, bestScore);
        }
    }
    
	return bestScore;
}

void CudaSolver::printStats()
{
    printf("Stats!!");
}
