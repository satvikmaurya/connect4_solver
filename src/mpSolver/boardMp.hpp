////////////////////////////////////////////////////////////////////////////
// Author       : Prajyot Gupta
// Department   : Grad Student @ Dept. of Electrical & Computer Engineering
// Contact      : pgupta54@wisc.edu
// Project      : ECE 759
// Description  : OpenMP Implementation of connect-4
//////////////////////////////////////////////////////////////////////////////

#ifndef __BOARD_MP__
#define __BOARD_MP__

#include "../connectFourAssets/slotStatus.hpp"
#include "../connectFourAssets/player.hpp"
#include "../connectFourAssets/board.hpp"

class BoardMp : public Board {
	
	private: 
        uint_fast8_t *next_row;
        uint_fast8_t width;
        uint_fast8_t height;
        uint_fast8_t winningStreakSize;
        
        bool isGameOver = false;
        Player winner = Player::None;
        bool determineIfPlayerIsWinner(Player player);

	public:
	BoardMp(uint_fast8_t width, uint_fast8_t height, uint_fast8_t winningStreakSize);

        // Checks if the board is full
        //bool IsFull();

	void Reset();

        /**
         * @brief      Evaluation function
         * This is a virtual function so that the CUDA and OMP implementations
         * can override it directly.
         *
         * @param[in]  board   The board
         * @param[in]  player  Current player
         *
         * @return     The score of the given board
         */
        int EvaluateBoard(Player player);

		/**
         * @brief      Checks the number of consecutive pieces in the horizontal
         * vertial and diagonal directions 
         *
         * @param[in]  board  The board
         * @param[in]  color  The color of the player
         * @param[in]  streak The streak length which is to be counted
         *
         * @return     number of streaks for the given streak.
         */
	 //prajyotg :: new
        uint32_t checkStreakMp(SlotStatus color, int streak);
};

#endif
