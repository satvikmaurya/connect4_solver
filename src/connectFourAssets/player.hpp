#ifndef __PLAYER__
#define __PLAYER__

#include <cstdlib>

enum class Player {None=0, Red, Yellow};

namespace PlayerHelpers {


    /**
     * @brief      Finds the opponent for the given player
     *
     * @param[in]  player  The player
     *
     * @return     The opponent player
     */
    inline Player OppositePlayer(Player player) {
        if (player == Player::Red) {
            return Player::Yellow;
        }
        else if (player == Player::Yellow)
        {
            return Player::Red;
        } 
        else
        {
            std::exit(911);
        }
    }
}
#endif