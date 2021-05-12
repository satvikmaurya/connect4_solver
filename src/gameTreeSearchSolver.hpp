#ifndef __GAME_TREE_SOLVER_HPP__
#define __GAME_TREE_SOLVER_HPP__

#include "connectFourAssets/slotStatus.hpp"
#include "connectFourAssets/player.hpp"
#include "connectFourAssets/board.hpp"

#include <chrono>

class GameTreeSearchSolver
{
    public:
        virtual int solve(Player player, int maxDepth, double timeLimit) = 0;

        /**
         * @brief      Finds the best move.
         *
         * @param      board   The board
         * @param[in]  player  The player
         * @param[in]  player  The maximum depth for the search
         *
         * @return     Returns the index (row major) of the best move on the board
         */
        virtual int findBestMove(SlotStatus* board, Player player, int maxDepth) = 0;

        /**
         * @brief      Minimax search of the game tree
         *
         * @param      board      The board
         * @param[in]  depth      The depth
         * @param[in]  player     The player
         * @param[in]  maximizer  The maximizer
         *
         * @return     Returns the best possible score for the current player
         */
        virtual int minimax(SlotStatus* board, int depth, Player player, bool maximizer) = 0;

        /**
         * @brief      Prints the board.
         */
        virtual void printBoard() { _board->printBoard(); }

        /**
         * @brief      Prints statistics.
         */
        virtual void printStats() = 0;

        /**
         * @brief      Inserts a piece in the specified column
         *
         * @param[in]  column  The column
         * @param[in]  player  The player
         *
         * @return     Returns -1 if there is an error
         */
        int playMove(int column, Player player) { return _board->playMove(column, player); }

        /**
         * @brief      Reset the solver
         */
        void resetSolver() {
            _nodesTraversed = 0;
            _totalNodesTraversed = 0;
            _board->Reset();
        };

        /**
         * @brief      Gets the total nodes traversed.
         *
         * @return     The nodes traversed.
         */
        uint64_t getTotalNodesTraversed() { return _totalNodesTraversed; }

    protected:
    	Board *_board;
    	uint64_t _nodesTraversed = 0;
        uint64_t _totalNodesTraversed = 0;
        
        std::chrono::high_resolution_clock::time_point _start;
		std::chrono::high_resolution_clock::time_point _end;
		std::chrono::duration<double, std::milli> _duration_millsec;
        
        /**
         * @brief      Starts the timer for a move.
         */
        void startTimer() { _start = std::chrono::high_resolution_clock::now(); }

        /**
         * @brief      Determines whether time is left for a deeper search.
         *
         * @param[in]  seconds  The time limit
         *
         * @return     True if the specified seconds is time left, False otherwise.
         */
        bool isTimeLeft(double seconds) {
            if (seconds == -1) return true;
            _end = std::chrono::high_resolution_clock::now();
            _duration_millsec = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(_end - _start);
            if(seconds * 1000 < _duration_millsec.count())
                return false;
            else
                return true;
        }

};

#endif