////////////////////////////////////////////////////////////////////////////
// Author       : Prajyot Gupta
// Department   : Grad Student @ Dept. of Electrical & Computer Engineering
// Contact      : pgupta54@wisc.edu
// Project      : ECE 759
// Description  : OpenMP Implementation of connect-4
//////////////////////////////////////////////////////////////////////////////

#include "boardMp.hpp"
#include <iostream>
#include <omp.h>

BoardMp::BoardMp(uint_fast8_t width = 7, uint_fast8_t height = 6, uint_fast8_t winningStreakSize = 4) : Board(width, height, winningStreakSize) {
    
    // Initialize the board with all empty
    this->board = new SlotStatus[width*height];
#pragma omp parallel for schedule(static) 
    for (uint_fast8_t i = 0; i < width*height; i++) this->board[i] = SlotStatus::Empty;

    // Initalize the next row for all columns to 0
    this->next_row = new uint_fast8_t[width];
#pragma omp parallel for schedule(static) 
    for (uint_fast8_t i = 0; i < width; i++) this->next_row[i] = 0;
}

bool BoardMp::determineIfPlayerIsWinner(Player player)
{
    if (player == Player::None)
    {
        std::cerr << "Can't check if player none is a winner!\n";
        return false;
    }

    auto slotStatusToCheck = player == Player::Red ? SlotStatus::Red : SlotStatus::Yellow;

    bool return_val = false;
#pragma omp parallel for shared(slotStatusToCheck) collapse(2) 
    for (uint_fast8_t r = 0; r < this->height; r++)
    {
        for (uint_fast8_t c = 0; c < this->width; c++)
        {
            auto slotToCheck = this->board[this->width * r + c];
            // If this slot matches the player color, see if it's the start of a winning streak
            if (slotToCheck == slotStatusToCheck)
            {
                // Not possible to have horizontal streak without enough horiz. room
                if (c <= (this->width - this->winningStreakSize))
                {
                    // check if there is a winning horizontal streak
                    for (uint_fast8_t i = 1; i < this->winningStreakSize; i++)
                    {
                        auto nextInStreak = this->board[this->width * r + c + i];

                        if (i == this->winningStreakSize - 1 &&  nextInStreak == slotToCheck)
                        {
                            // If checking the last element and it matches, we've found 4 in a row
			    return_val = true;
                        }
                        if (nextInStreak != slotToCheck)
                        {
                            // Found one element in horizontal streak that doesn't match, stop checking horizontal
                            break;
                        }
                    }
                }

                // Not possible to have vertical streak without enough vertical room
                if (r <= (this->height - this->winningStreakSize))
                {
                    // check if there is a purely vertical streak
                    for (uint_fast8_t i = 1; i < this->winningStreakSize; i++)
                    {
                        auto nextInStreak = this->board[(this->width * (r + i)) + c];
                        if (i == this->winningStreakSize - 1 &&  nextInStreak == slotToCheck)
                        {
                            // If checking the last element and it matches, we've found 4 in a row
			    return_val = true;
                        }
                        if (nextInStreak != slotToCheck)
                        {
                            // Found one element in horizontal streak that doesn't match, stop checking horizontal
                            break;
                        }
                    }
                    // Check for right diagonal only if there's room on the right side
                    if (c <= (this->width - this->winningStreakSize))
                    {
                        for (uint_fast8_t i = 1; i < this->winningStreakSize; i++)
                        {
                            auto nextInStreak = this->board[this->width * (r + i) + (c + i)];
                            if (i == this->winningStreakSize - 1 &&  nextInStreak == slotToCheck)
                            {
                                // If checking the last element and it matches, we've found 4 in a row
				return_val = true;
                            }
                            if (nextInStreak != slotToCheck)
                            {
                                // Found one element in horizontal streak that doesn't match, stop checking horizontal
                                break;
                            }
                        }
                    }

                    // check for left-up diagonal
                    if (c >= (this->winningStreakSize - 1))
                    {
                        for (uint_fast8_t i = 1; i < this->winningStreakSize; i++)
                        {
                            auto nextInStreak = this->board[this->width * (r + i) + (c - i)];
                            if (i == this->winningStreakSize - 1 &&  nextInStreak == slotToCheck)
                            {
                                // If checking the last element and it matches, we've found 4 in a row
                                //prajyotg :: return true;
				return_val = true;
                            }
                            if (nextInStreak != slotToCheck)
                            {
                                // Found one element in horizontal streak that doesn't match, stop checking horizontal
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    return return_val;
}

void BoardMp::Reset()
{
    const uint_fast8_t totalSlots = this->width * this->height;
#pragma omp parallel for schedule (static)	
    for (uint_fast8_t i = 0; i < totalSlots; i++)
    {
        this->board[i] = SlotStatus::Empty;
    }

#pragma omp parallel for schedule (static)	
    for (uint_fast8_t i = 0; i < this->width; i++)
    {
        this->next_row[i] = 0;
    }

    this->winner = Player::None;
    this->isGameOver = false;
}

/**
 * SATVIK: The evaluation function can be enhanced. A static evaluation function
 * is easier to implement but a threat based function will be more intelligent.
 * However, the potential to parallelize this computation will remain the same 
 * for either case.
 *
 */
int BoardMp::EvaluateBoard(Player player) {
    // Heuristics for finding the score of the current board
    
    /**
     * SATVIK: This if-else could be avoided if we use simpler data structures 
     * that can be complemented
     */
    SlotStatus color_for, color_against;
    color_for = this->getPlayerColor(player);
    if(player == Player::Red) { 
        color_against = SlotStatus::Yellow;
    }
    else if (player == Player::Yellow) {
        color_against = SlotStatus::Red;
    }

    // Determine if a leaf node has been reached- in this case, the score will
    // be either INT_MAX or INT_MIN, depending on the winner.
    Player winner = this->DetermineWinner();
    if(winner == player) {
        return INT_MAX;
    }
    else if(winner == this->oppPlayer(player)) {
        return INT_MIN;
    }
    // Simple heuristic- find the streaks in the horizontal, vertical, and 
    // diagonal directions.
    // Early game stages may have very few or no streaks at all. A static map
    // will serve as a better heuristic where the player who has more pieces in
    // the center of the board is in a stronger position.
    
    /**
     * Streaks of a longer length should have a higher impact on the score-
     * An polynomial distribution can be used.
     */

    uint32_t score_for = 0;
    uint32_t score_against = 0;
    uint32_t val   = 0;
    uint32_t score = 0;
//#pragma omp parallel for reduction (+:score) 
    for(int streak = this->winningStreakSize; streak >= 2; streak--) {
        	score_for 	= this->checkStreak(color_for, streak);
        	score_against 	= this->checkStreak(color_against, streak);
        	score += (score_for - score_against) * streak * streak * streak;
    }

//#pragma omp parallel sections private(val) reduction(+:score)
//    {
//    #pragma omp section
//	{
//    	    val = this->checkStreakMp(color_for, 4); 
//	    score += val*4*4*4;
//        }
//
//    #pragma omp section
//	 {
//    	    val = this->checkStreakMp(color_for, 3); 
//	    score += val*3*3*3;
//	 }
//    
//    #pragma omp section
//	 {
//    	    val = this->checkStreakMp(color_for, 2); 
//	    score += val*3*3*3;
//	 }
//
//    #pragma omp section
//	 {
//	    val = this->checkStreakMp(color_against, 4);
//	    score -= val*4*4*4;
//	 }
//
//    #pragma omp section
//	 {
//	    val = this->checkStreakMp(color_against, 3);
//	    score -= val*3*3*3;
//	 }
//
//    #pragma omp section
//	 {
//	    val = this->checkStreakMp(color_against, 2);
//	    score -= val*2*2*2;
//	 }
//    }
    return score;
}   

uint32_t BoardMp::checkStreakMp(SlotStatus color, int streak) {
    // Compute the number of streaks for the given streak length for all directions
    uint32_t count = 0;
#pragma omp parallel sections reduction(+:count)
    {
    #pragma omp section
	{
    	    count += this->checkHorzStreak(color, streak);
        }
    #pragma omp section
	 {
    	    count += this->checkVertStreak(color, streak);
	 }
    
    #pragma omp section
	 {
    	    count += this->checkDiagStreak(color, streak);
	 }
    }
    return count;
}
