////////////////////////////////////////////////////////////////////////////
// Author       : Prajyot Gupta
// Department   : Grad Student @ Dept. of Electrical & Computer Engineering
// Contact      : pgupta54@wisc.edu
// Project      : ECE 759
// Description  : OpenMP Implementation of connect-4
//////////////////////////////////////////////////////////////////////////////

#ifndef __MP_SOLVER__
#define __MP_SOLVER__

#include "boardMp.hpp"
//#include "gameTreeSearchSolver.hpp"
#include <climits>
#include <chrono>
#include <iostream>
#include <sstream>

//class MpSolver : GameTreeSearchSolver
class MpSolver
{
    public:
    	/**
    	 * @brief      Constructs a new instance.
    	 */
    	MpSolver(uint_fast8_t width = 7, uint_fast8_t height = 6,
    					 uint_fast8_t winningStreakLength = 4);

        /**
         * @brief     Find the best move for a given game board
         *
         * @param[in]  player      The player
         * @param[in]  maxDepth    The maximum depth
         * @param[in]  time_limit  The time limit
         *
         * @return     Returns the column for the best move, -1 if no move exists
         */
        int solve(Player player, int maxDepth = 6, double time_limit = -1);

        /**
         * @brief      Finds the best move.
         *
         * @param      board      The board
         * @param[in]  player     The player
         * @param[in]  maxDepth   The maximum depth for the search
         * @param[in]  time_limit The maximum time limit for the search
         *
         * @return     Returns the index (row major) of the best move on the board
         */
        int findBestMove(SlotStatus* board, Player player, int maxDepth, double time_limit);

        /**
         * @brief      Finds the opponent for the given player
         *
         * @param[in]  player  The player
         *
         * @return     Returns the opponent
         */
        Player oppPlayer(Player player);

        /**
         * @brief      Gets the player color.
         *
         * @param[in]  player  The player
         *
         * @return     The player color.
         */
        SlotStatus getPlayerColor(Player player);

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
        int minimax(SlotStatus* board, int depth, Player player, bool maximizer);

        /**
         * @brief      Prints the board.
         */
        void printBoard();

        /**
         * @brief      Prints statistics.
         */
        void printStats();

        /**
         * @brief      Reset the solver
         */
        void resetSolver();

        /**
         * @brief      Gets the total nodes traversed.
         *
         * @return     The nodes traversed.
         */
        uint64_t getTotalNodesTraversed();

        /**
         * @brief      Inserts a piece in the specified column
         *
         * @param[in]  column  The column
         * @param[in]  player  The player
         *
         * @return     Returns -1 if there is an error
         */
        int playMove(int column, Player player);

        /**
         * @brief      Starts the timer for a move.
         */
        void startTimer();

        /**
         * @brief      Determines whether time is left for a deeper search.
         *
         * @param[in]  seconds  The time limit
         *
         * @return     True if the specified seconds is time left, False otherwise.
         */
        bool isTimeLeft(double seconds);

    private:
    	BoardMp* _boardMp;
    	uint64_t _nodesTraversed;
	uint64_t _totalNodesTraversed;
		std::chrono::high_resolution_clock::time_point _start;
		std::chrono::high_resolution_clock::time_point _end;
		std::chrono::duration<double, std::milli> _duration_millsec;
};

#endif
