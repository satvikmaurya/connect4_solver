/**
 * @defgroup   __BOARD_SEQ__
 *
 * @brief      This file implements board class for the sequential solver.
 *
 * @author     Satvik
 * @date       2021
 */
#ifndef __BOARD_SEQ__
#define __BOARD_SEQ__

#include "connectFourAssets/slotStatus.hpp"
#include "connectFourAssets/player.hpp"
#include "connectFourAssets/board.hpp"

class BoardSequential : public Board {
	// No additional methods needed for the seq solver
	public:
		BoardSequential(uint_fast8_t width, uint_fast8_t height, 
					uint_fast8_t winningStreakSize);
};

#endif
