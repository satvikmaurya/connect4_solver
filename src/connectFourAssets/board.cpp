/*
 * Robert Viramontes
 * March 28, 2021
 * 
 * Implements a basic Connect 4 board object for maintaining the state of the game. 
 * Written for completion of the ME759 project.
 */

#include "board.hpp"
#include <iostream>
#include <omp.h>

Board::Board(uint_fast8_t width, uint_fast8_t height, uint_fast8_t winningStreakSize) 
    : width(width), height(height), winningStreakSize(winningStreakSize)
{
    // Initialize the board with all empty
    this->board = new SlotStatus[width*height];
    for (uint_fast8_t i = 0; i < width*height; i++) this->board[i] = SlotStatus::Empty;

    // Iniitalize the next row for all columns to 0
    this->next_row = new uint_fast8_t[width];
    for (uint_fast8_t i = 0; i < width; i++) this->next_row[i] = 0;
}

Board::~Board()
{
    delete[] this->board;
    delete[] this->next_row;
}

int Board::AddPiece(Player player, uint_fast8_t column)
{
    if (isGameOver)
    {
        std::cerr << "Game is over, please reset the board before playing any more!";
        return 1;
    }

    if (column >= this->width)
    {
        std::cerr << "Tried to insert in column " << column << "which doesn't exist in " << this->width << "wide board.\n";
        return 1;
    }

    if (this->next_row[column] >= this->height)
    {
        std::cerr << "Column " << column << "is full.\n";
        return 1;
    }

    // Set the status of the position of the board. 
    this->board[this->width * this->next_row[column] + column] = player == Player::Red ? SlotStatus::Red : SlotStatus::Yellow;
    this->next_row[column] += 1; 

    // Now, check if the game is over and if we have a winner
    bool isFull = this->IsFull();
    // Only have to check if player that just added a piece is the winner
    auto isWinner = this->determineIfPlayerIsWinner(player);
    std::cerr << "is full: " << (isFull ? "true" : "false") << " is winner: " << (isWinner ? "true" : "false") << '\n';
    if (isWinner)
    {
        // Update the winner
        this->winner = player;
    }

    this->isGameOver = isFull || isWinner;

    return 0;
}

bool Board::determineIfPlayerIsWinner(Player player)
{
    if (player == Player::None)
    {
        std::cerr << "Can't check if player none is a winner!\n";
        return false;
    }

    auto slotStatusToCheck = player == Player::Red ? SlotStatus::Red : SlotStatus::Yellow;

    bool return_val = false;
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
                            //return true;
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
                            //return true;
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

// Determine if either player is a winner
Player Board::DetermineWinner()
{
    auto redIsWinner = this->determineIfPlayerIsWinner(Player::Red);
    auto yellowIsWinner = this->determineIfPlayerIsWinner(Player::Yellow);
    if (redIsWinner & yellowIsWinner)
    {
        std::cerr << "Something went wrong, cannot have two winners.";
    } else if (redIsWinner)
    {
        return Player::Red;
    } else if (yellowIsWinner)
    {
        return Player::Yellow;
    }

    return Player::None;
}

bool Board::IsFull()
{
    const uint_fast8_t totalSlots = this->width * this->height;
    for (uint_fast8_t i = 0; i < totalSlots; i++)
    {
        if (this->board[i] == SlotStatus::Empty)
        {
            // Found an empty slot, board not full
            return false;
        }
    }

    // Didn't find an empty slot, board must be full
    return true;
}

void Board::Reset()
{
    const uint_fast8_t totalSlots = this->width * this->height;
    for (uint_fast8_t i = 0; i < totalSlots; i++)
    {
        this->board[i] = SlotStatus::Empty;
    }

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
int Board::EvaluateBoard(Player player) {
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
    uint32_t score = 0;
    for(int streak = this->winningStreakSize; streak >= 2; streak--) {
        	score_for = this->checkStreak(color_for, streak);
        	score_against = this->checkStreak(color_against, streak);
        	score += (score_for - score_against) * streak * streak * streak;
    }

    return score;
}

uint32_t Board::checkStreak(SlotStatus color, int streak) {
    // Compute the number of streaks for the given streak length for all directions
    uint32_t count = 0;
    count += this->checkHorzStreak(color, streak);
    count += this->checkVertStreak(color, streak);
    count += this->checkDiagStreak(color, streak);
    return count;
}

uint32_t Board::checkHorzStreak(SlotStatus color, int streak) {
    // Keep track of the current streak and when the board doesn't have the right
    // color, check if the accumulated streak is as long as the input streak
    // The traversal will be row-wise
    uint32_t current_streak = 0;
    uint32_t count = 0;
    for(int i = 0; i < this->height; i++) {
        for(int j = 0; j < this->width; j++) {
            if(this->board[this->width * i + j] == color) {
                current_streak += 1;
            } 
            else {
                if(current_streak == streak) count++;
                current_streak = 0;
            }
        }
        if(current_streak == streak) count++;
        current_streak = 0;
    }
    return count;
}

uint32_t Board::checkVertStreak(SlotStatus color, int streak) {
    // Keep track of the current streak and when the board doesn't have the right
    // color, check if the accumulated streak is as long as the input streak
    // The traversal will be column wise
    uint32_t current_streak = 0;
    uint32_t count = 0;
    for(int j = 0; j < this->width; j++) {
        for(int i = 0; i < this->height; i++) {
            if(this->board[this->width * i + j] == color) {
                current_streak += 1;
            } 
            else {
                if(current_streak == streak) count++;
                current_streak = 0;
            }
        }
        if(current_streak == streak) count++;
        current_streak = 0;
    }
    return count;
}

uint32_t Board::checkDiagStreak(SlotStatus color, int streak) {
    // Rather than search all the diagonals, only the diagonals starting at a 
    // board slot which contains the right color can be searched.
    // There are two directions that have to be searched - left and right
    uint32_t current_streak = 0;
    uint32_t count = 0;
    for(int i = 0; i < this->height; i++) {
        for(int j = 0; j < this->width; j++) {
            if(this->board[this->width * i + j] == color) {
                // Search both diagonals for this position
                // Right diagonal
                for(int k = 1; k < this->width; k++) {
                    if((i + k < this->height) && (j + k < this->width)) {
                        if(this->board[this->width * (i + k) + (j + k)] == color) {
                            current_streak++;
                        }
                        else {
                            if(current_streak == streak) count++;
                            current_streak = 0;
                        }
                    }
                }
                if(current_streak == streak) count++;
                current_streak = 0;
                // Left diagonal
                for(int k = 1; k < this->width; k++) {
                    if((i - k >= 0) && (j - k >= 0)) {
                        if(this->board[this->width * (i - k) + (j - k)] == color) {
                            current_streak++;
                        }
                        else {
                            if(current_streak == streak) count++;
                            current_streak = 0;
                        }
                    }
                }
                if(current_streak == streak) count++;
                current_streak = 0;
            }
        }
    }
    return count;
}

bool Board::isLegalMove(int index) {
    if(this->board[index] != SlotStatus::Empty) return false;
    int row = index / this->width;
    if(row == this->height - 1) return true;
    if(this->board[index + this->width] == SlotStatus::Empty) return false;
    //if(row == 0) return false;
    return true;
}

SlotStatus* Board::getBoard() {
    return this->board;
}

SlotStatus Board::getPlayerColor(Player player) {
    if(player == Player::Red)
        return SlotStatus::Red;
    else if(player == Player::Yellow) 
        return SlotStatus::Yellow;
    else
        return SlotStatus::Empty;
}

uint_fast8_t Board::getWidth() {
    return this->width;
}

uint_fast8_t Board::getHeight() {
    return this->height;
}

Player Board::oppPlayer(Player player) {
    return (player == Player::Red)?(Player::Yellow):(Player::Red);
}

int Board::playMove(int column, Player player) {
    int row = -1;
    for(int i = this->height - 1; i >= 0; i--) {
        if(this->board[i * this->width + column - 1] == SlotStatus::Empty) {
            row = i + 1;
            break;
        }
    }
    if(row == -1) {
        std::cerr << "ERROR: This column is full, please try another" << std::endl;
        return -1;
    }
    int index = this->conv2DTo1D(row, column);
    if(index == -1) return index;
    SlotStatus color = this->getPlayerColor(player);
    this->playMove(index, color);
    return 0;
}

void Board::playMove(int index, SlotStatus color) {
    this->board[index] = color;
}

int Board::conv2DTo1D(int row, int column) {
    if(row < 1 || column < 1) {
        std::cerr << "ERROR: Please enter a row/column value "
                     "greater than or equal to 1" << std::endl;
        fflush(stdout);
        return -1;
    }
    if(row > this->height || column > this->width) {
        std::cerr << "ERROR: Please enter a row/column within the bounds of the "
                     "board" << std::endl;
        fflush(stdout);
        return -1;
    }
    return (row - 1) * this->width + (column - 1);
}

void Board::printBoard() {
    // Will print the board out to stdout
    std::stringstream str;
    str << "Current Board: \n\n";
    // Print out all the column numbers first
    str << "  ";
    for(int i = 1; i <= this->width; i++) {
        str << i << " ";
    }
    // Print out the board row-wise, with each row starting with the row no.
    str << "\n";
    for(int i = 0; i < this->height; i++) {
        str << i + 1 << " ";
        for(int j = 0; j < this->width; j++) {
            if(this->board[i * this->width + j] == SlotStatus::Red) 
                str << "R ";
            else if(this->board[i * this->width + j] == SlotStatus::Yellow)
                str << "Y ";
            else
                str << "_ ";
        }
        str << "\n";
    }
    std::cout << str.str() << std::endl;
    fflush(stdout);
}

std::pair<int, int> Board::conv1DTo2D(int index) {
    // Converts a 1D index to a 2D one for the board
    std::pair<int, int> rowcol;
    rowcol.first = (index / this->width) + 1;
    rowcol.second = (index % this->width) + 1;
    return rowcol;
}
