#ifndef __SLOT_STATUS__
#define __SLOT_STATUS__

enum class SlotStatus {Empty = 0, Red, Yellow};

#include <cstdlib>
#include "player.hpp"

namespace SlotStatusHelpers
{

    /**
     * @brief      Finds the slot status that matches the player
     *
     * @param[in]  player  The player
     *
     * @return     The matching slot status
     */
    inline SlotStatus getSlotFromPlayer(Player player) {
        if(player == Player::Red)
        {
            return SlotStatus::Red;
        }
        else if(player == Player::Yellow)
        {
            return SlotStatus::Yellow;
        }
        else 
        {
            std::exit(911);
        }       
    }
};

#endif