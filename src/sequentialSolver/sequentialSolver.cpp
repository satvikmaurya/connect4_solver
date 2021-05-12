#include "sequentialSolver.hpp"

#define DEBUG 1

/*SequentialSolver::SequentialSolver(uint_fast8_t width, uint_fast8_t height,
								   uint_fast8_t winningStreakSize):
								   GameTreeSearchSolver(), _nodesTraversed(0) {*/
SequentialSolver::SequentialSolver(uint_fast8_t width, uint_fast8_t height,
								   uint_fast8_t winningStreakSize):
								   _nodesTraversed(0), _totalNodesTraversed(0) {
	_boardSeq = new BoardSequential(width, height, winningStreakSize);
}

int SequentialSolver::solve(Player player, int maxDepth, double time_limit)
{
    if (_boardSeq->DetermineWinner() != Player::None) {
        return -1;
    }

	_nodesTraversed = 0;
	int retval = -1;
	int bestMove = -1;
	auto nodesTraversed = _nodesTraversed;

	if(time_limit > 0) {
		// Implement Iterative Deepening to adhere to a time limit per move
		this->startTimer();
		for (int depth = 2; this->isTimeLeft(time_limit); depth += 2) {
			// Find the best move
			int move = this->findBestMove(_boardSeq->getBoard(), player, depth, time_limit);
			if(this->isTimeLeft(time_limit)) {
				bestMove = move;
				nodesTraversed = _nodesTraversed;
				_nodesTraversed = 0;
			}
		}
		_nodesTraversed = nodesTraversed;
	}
	else {
		bestMove = this->findBestMove(_boardSeq->getBoard(), player, maxDepth, time_limit);
	}

	if(_boardSeq->IsFull()) {
		std::cout << "Board is full" << std::endl;
		return -1;
	}

	if(bestMove > -1) {
		_boardSeq->playMove(bestMove, this->getPlayerColor(player));
		//this->printBoard();
		retval = bestMove % _boardSeq->getWidth();
	}

	Player winner = _boardSeq->DetermineWinner();
	if(winner != Player::None) {
		//std::cout << "Player: " << ((winner == Player::Red)?"Red ":"Yellow ")
		//<< "is the winner" << std::endl;
		return -1;
	}
	_totalNodesTraversed += _nodesTraversed;
    return retval;
}

int SequentialSolver::findBestMove(SlotStatus* board, Player player, int maxDepth, double time_limit) {
	// Will return the index of the best move in the board for the given player
	// Return if the board is full
	if(_boardSeq->IsFull()) return -1;

	SlotStatus color = (player == Player::Red)?(SlotStatus::Red):(SlotStatus::Yellow);
	int move = -1;
	int bestScore = INT_MIN;
	bool empty_slot_avl = false;

	// Traverse through the board to find legal moves and see the maximum score
	// Since the board fills from the last row, it's better to traverse the 
	// board in a reverse order
	for(int i = _boardSeq->getWidth() * _boardSeq->getHeight() - 1; (i >= 0) && 
										this->isTimeLeft(time_limit) ; i--) {
		if(board[i] == SlotStatus::Empty) {
			if(!_boardSeq->isLegalMove(i)) continue;
			empty_slot_avl = true;
			board[i] = color;
			int score = this->minimax(board, maxDepth, player, false);
			board[i] = SlotStatus::Empty;
			++_nodesTraversed;
			if(score > bestScore) {
				move = i;
				bestScore = score;
			}
		}
	}
	// This means that there is no move which can avoid defeat, so at this point
	// it doesn't really matter where the move is played
	if(move == -1 && empty_slot_avl) {
		for(int i = _boardSeq->getWidth() * _boardSeq->getHeight() - 1; (i >= 0)/* && 
											this->isTimeLeft(time_limit)*/ ; i--) {
			if(board[i] == SlotStatus::Empty) {
				if(!_boardSeq->isLegalMove(i)) continue;
				move = i;
				break;
			}
		}
	}
	return move;
}

int SequentialSolver::minimax(SlotStatus* board, int depth, Player player, 
								bool maximizer) {
	// Board evaluations are static: The player won't change, it will always be
	// the maximizer wrt whom the score will be calculated.
	int score = _boardSeq->EvaluateBoard(player);

	if(score == INT_MAX || score == INT_MIN) // someone has won already
		return score;

	if(_boardSeq->IsFull()) return score;

	if(depth == 0) return score;

	SlotStatus color;
	if(maximizer) 
		color = (player == Player::Red)?(SlotStatus::Red):(SlotStatus::Yellow);
	else
		color = (player == Player::Red)?(SlotStatus::Yellow):(SlotStatus::Red);

	if(maximizer) {
		// Find maximum possible score
		int bestScore = INT_MIN;
		for(int i = _boardSeq->getWidth() * _boardSeq->getHeight() - 1; i >= 0 ;
																		 i--) {
			if(board[i] == SlotStatus::Empty) {
				if(_boardSeq->isLegalMove(i)) {
					board[i] = color;
					score = this->minimax(board, depth - 1, player, !maximizer);
					bestScore = std::max(score, bestScore);
					board[i] = SlotStatus::Empty;
					++_nodesTraversed;
				}
			}
		}
		return bestScore;
	}
	else {
		// Find the best score for the minimizer
		int bestScore = INT_MAX;
		for(int i = _boardSeq->getWidth() * _boardSeq->getHeight() - 1; i >= 0 ; 
																		i--) {
			if(board[i] == SlotStatus::Empty) {
				if(_boardSeq->isLegalMove(i)) {
					board[i] = color;
					score = this->minimax(board, depth - 1, player, !maximizer);
					bestScore = std::min(bestScore, score);
					board[i] = SlotStatus::Empty;
					++_nodesTraversed;
				}
			}
		}
		return bestScore;
	}
}

void SequentialSolver::printBoard() {
	_boardSeq->printBoard();
}

void SequentialSolver::printStats() {
	std::cout << "Total Nodes traversed: " << _nodesTraversed << std::endl;
}

int SequentialSolver::playMove(int column, Player player) {
	return _boardSeq->playMove(column, player);
}

SlotStatus SequentialSolver::getPlayerColor(Player player) {
	if(player == Player::Red)
		return SlotStatus::Red;
	else if(player == Player::Yellow) 
		return SlotStatus::Yellow;
	else
		exit(911);
}

Player SequentialSolver::oppPlayer(Player player) {
	return _boardSeq->oppPlayer(player);
}

void SequentialSolver::startTimer() {
	_start = std::chrono::high_resolution_clock::now();
}

bool SequentialSolver::isTimeLeft(double seconds) {
	if (seconds == -1) return true;
	_end = std::chrono::high_resolution_clock::now();
	_duration_millsec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(_end - _start);
	if(seconds * 1000 < _duration_millsec.count())
		return false;
	else
		return true;
}

uint64_t SequentialSolver::getTotalNodesTraversed() {
	return _totalNodesTraversed;
}

void SequentialSolver::resetSolver() {
	_nodesTraversed = 0;
	_totalNodesTraversed = 0;
	_boardSeq->Reset();
}
