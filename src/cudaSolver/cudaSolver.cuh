/**
 * @defgroup   CUDA_SOLVER
 *
 * @brief      This file implements tournament APIs which can allow humans/solvers
 * to play against each other.
 *
 * @author     Robert Viramontes
 * @date       2021
 */
#ifndef __CUDA_SOLVER__
#define __CUDA_SOLVER__

#include "gameTreeSearchSolver.hpp"
#include "connectFourAssets/slotStatus.hpp"
#include <vector>
#include <unordered_map>

class CudaSolver : public GameTreeSearchSolver
{
    public:

        struct boardAndPath {
            SlotStatus* board;
            std::vector<int> path;
        };

        CudaSolver(int width, int height, int winningStreakSize);
        ~CudaSolver();

        int solve(Player player, int maxDepth, double timeLimit) override;

        /**
         * @deprecated Use minimax2 for full CUDA implementation
         * @brief      Finds the best move.
         *
         * @param      board   The board
         * @param[in]  player  The player
         * @param[in]  player  The maximum depth for the search
         *
         * @return     Returns the index (row major) of the best move on the board
         */
        int findBestMove(SlotStatus* board, Player player, int maxDepth) override;

        /**
         * @brief      Finds the best move.
         *
         * @param      board   The board
         * @param[in]  player  The player
         * @param[in]  player  The maximum depth for the search
         *
         * @return     Returns the index (row major) of the best move on the board
         */
        int findBestMove2(SlotStatus* board, Player player, int maxDepth);

        /**
         * @deprecated Use minimax2 for full CUDA implementation
         * @brief      Minimax search of the game tree
         *
         * @param      board      The board
         * @param[in]  depth      The depth
         * @param[in]  player     The player
         * @param[in]  maximizer  The maximizer
         *
         * @return     Returns the best possible score for the current player
         */
        int minimax(SlotStatus* board, int depth, Player player, bool maximizer) override;
        
        /**
         * @brief      Minimax search of the precomputed game tree
         *
         * @param      board      The board
         * @param[in]  scores     Array of precomputed scores for all possible boards
         * @param[in]  depth      The depth
         * @param[in]  player     The player
         * @param[in]  maximizer  The maximizer
         * @param      pathSoFar  Vector of the current graph traversal path
         * @param[in]  pathtoIndex  Unordered mapping between path hash and board index in scores
         *
         * @return     Returns the best possible score for the current player
         */
        int minimax2(SlotStatus* board, const double *scores, const int depth, const Player player, const bool maximizer, std::vector<int> pathSoFar, const std::unordered_map<uint32_t, int> *pathToIndex);
        
        /**
         * @brief      Prints statistics.
         */
        void printStats() override;

        /** 
         * @deprecated Use EvaluateBoards for full CUDA implementation 
         */
        int EvaluateBoard(SlotStatus* board, Player player);

        /**
         * @brief      Minimax search of the precomputed game tree
         *
         * @param[in]  concat_boards  Array of all possible boards concat together
         * @param[in]  numBoards      Number of boards in concat_boards
         * @param[in]  player         The player
         * @param      scores         Array to be filled with score for each board
         *
         * @return     0 for success, otherwise error code
         */
        int EvaluateBoards(const SlotStatus* concat_boards, const int numBoards, const Player player, double scores[]);

    private:
    	uint64_t _nodesTraversed;
        uint32_t _numStreams;
        void* _streams;

        /** 
        * @deprecated  Use findBoards2 for full CUDA implementation.
        */
        void findBoards(SlotStatus* board, const int width, const int height, const SlotStatus pieceToPlace, SlotStatus* &concatBoard, int &numBoards);

        /**
         * @brief      Finds all possible boards for a depth starting from board
         *
         * @param[in]  boards            Starting board
         * @param[in]  width             Width of the board
         * @param[in]  height            Height of the board
         * @param[in]  player            The player to start with
         * @param      completed_boards  Array to fill in the complete possible boards
         * @param      pathSoFar         Vector of graph traversal path so far
         */
        void findBoards2(const SlotStatus* board, const int width, const int height, const Player player, const int depth, std::vector<CudaSolver::boardAndPath> *completed_boards, std::vector<int> pathSoFar);

        /**
         * @brief      Creates a unique index for each path in the graph traversal
         *
         * @param[in]  path  The path to convert to a unique index for hash map
         *
         * @return     The index computed
         */
        uint32_t createPathMapping(const std::vector<int> *path);

        bool isLegalMove(const SlotStatus *board, int width, int height, int index);
};

#endif // __CUDA_SOLVER__