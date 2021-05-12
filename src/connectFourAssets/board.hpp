/*
 * Robert Viramontes
 * March 28, 2021
 * 
 * Implements a basic Connect 4 board object for maintaining the state of the game. 
 * Written for completion of the ME759 project.
 */

#ifndef __BOARD__
#define __BOARD__

#include "slotStatus.hpp"
#include "player.hpp"

#include <cstdint>
#include <sstream>
#include <climits>

#define DEBUG 1

class Board {

    protected:
        SlotStatus *board;
    private: 
        uint_fast8_t *next_row;
        uint_fast8_t width;
        uint_fast8_t height;
        uint_fast8_t winningStreakSize;
        
        bool isGameOver = false;
        Player winner = Player::None;
        bool determineIfPlayerIsWinner(Player player);

    public:
        Board(uint_fast8_t width = 7, uint_fast8_t height = 6, uint_fast8_t winningStreakSize = 4);
        ~Board();

        // Allows a player to drop their piece into the specified column.
        // Note that the column is 0-indexed.
        // Returns 0 for success. 
        int AddPiece(Player player, uint_fast8_t column);

        // Checks if the board is full
        bool IsFull();

        // Determine if there is a winner
        Player DetermineWinner();

        // Resets the board to all empty
        void Reset();

        // Getter for isGameOver
        bool IsGameOver() { return isGameOver; }

        // Getter for winner
        Player Winner() {return winner; }

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
        virtual int EvaluateBoard(Player player);

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
        uint32_t checkStreak(SlotStatus color, int streak);

        uint32_t checkHorzStreak(SlotStatus color, int streak); 
        uint32_t checkVertStreak(SlotStatus color, int streak); 
        uint32_t checkDiagStreak(SlotStatus color, int streak); 

        /**
         * @brief      Determines whether the specified index is legal move.
         *
         * @param[in]  index  The index
         *
         * @return     True if the specified index is legal move, False otherwise.
         */
        bool isLegalMove(int index);

        /**
         * @brief      Gets the board handle.
         *
         * @return     The board handle.
         */
        SlotStatus* getBoard();

        SlotStatus getPlayerColor(Player player);

        Player oppPlayer(Player player);

        uint_fast8_t getWidth();

        uint_fast8_t getHeight();

        uint_fast8_t getWinningStreakSize() { return winningStreakSize; }

    public:
        // Helper functions
        int playMove(int column, Player player);

        void playMove(int index, SlotStatus color);

        int conv2DTo1D(int row, int column);

        std::pair<int, int> conv1DTo2D(int index);

        void printBoard();

};

#endif
