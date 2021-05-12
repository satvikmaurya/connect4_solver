////////////////////////////////////////////////////////////////////////////
// Author       : Prajyot Gupta
// Department   : Grad Student @ Dept. of Electrical & Computer Engineering
// Contact      : pgupta54@wisc.edu
// Project      : ECE 759
// Description  : OpenMP Implementation of connect-4
//////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <omp.h>
#include "mpSolver.hpp"

#define DEBUG 1

/*MpSolver::MpSolver(uint_fast8_t width, uint_fast8_t height,
								   uint_fast8_t winningStreakSize):
								   GameTreeSearchSolver(), _nodesTraversed(0) {*/
MpSolver::MpSolver(uint_fast8_t width, uint_fast8_t height,
								   uint_fast8_t winningStreakSize):
								   _nodesTraversed(0), _totalNodesTraversed(0) {
	_boardMp = new BoardMp(width, height, winningStreakSize);
	int max_thr = omp_get_max_threads();
	printf("OpenMP initiated. Prepare for Doom. Max threads %d \n", max_thr);
}

int MpSolver::solve(Player player, int maxDepth, double time_limit)
{
    if (_boardMp->DetermineWinner() != Player::None) {
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
			int move = this->findBestMove(_boardMp->getBoard(), player, depth, time_limit);
			if(this->isTimeLeft(time_limit)) {
				bestMove = move;
				nodesTraversed = _nodesTraversed;
				_nodesTraversed = 0;
			}
		}
		_nodesTraversed = nodesTraversed;
	}
	else {
		bestMove = this->findBestMove(_boardMp->getBoard(), player, maxDepth, time_limit);
	}

	if(_boardMp->IsFull()) {
		std::cout << "Board is full" << std::endl;
		return -1;
	}

	if(bestMove > -1) {
		_boardMp->playMove(bestMove, this->getPlayerColor(player));
		//this->printBoard();
		retval = bestMove % _boardMp->getWidth();
	}

	Player winner = _boardMp->DetermineWinner();
	if(winner != Player::None) {
		//std::cout << "Player: " << ((winner == Player::Red)?"Red ":"Yellow ")
		//<< "is the winner" << std::endl;
		return -1;
	}
	_totalNodesTraversed += _nodesTraversed;
    return retval;
}

int MpSolver::findBestMove(SlotStatus* board, Player player, int maxDepth, double time_limit){
	// Will return the index of the best move in the board for the given player
	// Return if the board is full
	if(_boardMp->IsFull()) return -1;

	SlotStatus color = (player == Player::Red)?(SlotStatus::Red):(SlotStatus::Yellow);
	int move = -1;
	int bestScore = INT_MIN;
	bool empty_slot_avl = false;

	// Traverse through the board to find legal moves and see the maximum score
	// Since the board fills from the last row, it's better to traverse the 
	// board in a reverse order
	for(int i = _boardMp->getWidth() * _boardMp->getHeight() - 1; (i >= 0) && 
	                                                              	       this->isTimeLeft(time_limit) ; i--) {
		if(board[i] == SlotStatus::Empty) {
			if(!_boardMp->isLegalMove(i)) continue;
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
        	for(int i = _boardMp->getWidth() * _boardMp->getHeight() - 1; (i >= 0)/* && 
        										this->isTimeLeft(time_limit)*/ ; i--) {
        		if(board[i] == SlotStatus::Empty) {
        			if(!_boardMp->isLegalMove(i)) continue;
        			move = i;
        			break;
        		}
        	}
        }
	return move;
}

int MpSolver::minimax(SlotStatus* board, int depth, Player player, 
								bool maximizer) {
	// Board evaluations are static: The player won't change, it will always be
	// the maximizer wrt whom the score will be calculated.
	int score = _boardMp->EvaluateBoard(player);

	if(score == INT_MAX || score == INT_MIN) // someone has won already
		return score;

	if(_boardMp->IsFull()) return score;

	if(depth == 0) return score;

	SlotStatus color;
	if(maximizer) 
		color = (player == Player::Red)?(SlotStatus::Red):(SlotStatus::Yellow);
	else
		color = (player == Player::Red)?(SlotStatus::Yellow):(SlotStatus::Red);

	if(maximizer) {
		// Find maximum possible score
		int bestScore = INT_MIN;
		for(int i = _boardMp->getWidth() * _boardMp->getHeight() - 1; i >= 0 ; i--) {
			if(board[i] == SlotStatus::Empty) {
				if(_boardMp->isLegalMove(i)) {
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
		for(int i = _boardMp->getWidth() * _boardMp->getHeight() - 1; i >= 0 ; i--) {
			if(board[i] == SlotStatus::Empty) {
				if(_boardMp->isLegalMove(i)) {
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

void MpSolver::printBoard() {
	_boardMp->printBoard();
}

void MpSolver::printStats() {
	std::cout << "Total Nodes traversed: " << _nodesTraversed << std::endl;
}

int MpSolver::playMove(int column, Player player) {
	return _boardMp->playMove(column, player);
}

SlotStatus MpSolver::getPlayerColor(Player player) {
	if(player == Player::Red)
		return SlotStatus::Red;
	else if(player == Player::Yellow) 
		return SlotStatus::Yellow;
	else
		exit(911);
}

Player MpSolver::oppPlayer(Player player) {
	return _boardMp->oppPlayer(player);
}

void MpSolver::startTimer() {
	_start = std::chrono::high_resolution_clock::now();
}

bool MpSolver::isTimeLeft(double seconds) {
	if (seconds == -1) return true;
	_end = std::chrono::high_resolution_clock::now();
	_duration_millsec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(_end - _start);
	if(seconds * 1000 < _duration_millsec.count())
		return false;
	else
		return true;
}

uint64_t MpSolver::getTotalNodesTraversed() {
	return _totalNodesTraversed;
}

void MpSolver::resetSolver() {
	_nodesTraversed = 0;
	_totalNodesTraversed = 0;
	_boardMp->Reset();
}
