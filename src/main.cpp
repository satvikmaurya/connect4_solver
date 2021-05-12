#if CUDA_FOUND
#include "cudaSolver.cuh"
#endif
#include "mpSolver.hpp"
#include "sequentialSolver.hpp"
#include "tournament.hpp"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <omp.h>

// The std::chrono namespace provides timer functions in C++
#include <chrono>

// Provide some namespace shortcuts
using std::chrono::high_resolution_clock;
using std::chrono::duration;

using namespace std;

void parser(int argc, char** argv) {
    // Parse all inputs
    int i = 1;
    if(argc < 2) {
        // Default mode- Sequential solver vs human
        cout << "NO command line options provided, the default human vs "
                "sequential solver mode is being run.\n";
        tournament_human_vs_seq(Player::Red, 2.0, 6, 7, 6, 4, 1, false);
        return;
    }

    // Initialize default values
    int width = 7;
    int height = 6;
    int winningStreak = 4;
    double time_limit = 2.0;
    int maxDepth = 6;
    Player p1 = Player::Red;
    int num_games = 10;
    bool human_first = false;
    int num_threads = 11; // empirically determined 11 is a good number!

    bool seq_vs_seq = false;
    bool seq_vs_cuda = false;
    bool seq_vs_omp = false;
    bool omp_vs_cuda = false;
    bool cuda_vs_cuda = false;
    bool omp_vs_omp = false;
    bool human_vs_cuda = false;
    bool human_vs_omp = false;
    bool human_vs_seq = false;
    bool time_seq = false;
    bool time_cuda = false;
    bool time_omp = false;

    string help_message = "Available options are: \n\n"
                    "--no-time-limit    # No time limit per move.\n"
                    "--search-depth [depth]      # Set the max search depth per move\n"
                    "--time-limt [time]      # Set the time limit per move (in sec)\n"
                    "--width [width]     # Set the width of the board\n"
                    "--height [height]      # Set the height of the board\n"
                    "--winning-streak [streak]     # Set the length of the winning streak\n"
                    "--num-games [num]    # Set the number of games to be played\n"
                    "--human-first      # For the interactive games only - use if you"
                        " want to play the first move\n"
                    "--num-threads      # Set number of threads OMP solver can use\n"
                    "--seq-vs-seq       # Plays a tournament b/w 2 seq solvers\n"
                    "--seq-vs-cuda      # Plays a tournament b/w the seq and cuda solvers\n"
                    "--seq-vs-omp       # Plays a tournament b/w the seq and omp solvers\n"
                    "--omp-vs-cuda      # Plays a tournament b/w the cuda and omp solvers\n"
                    "--cuda-vs-cuda     # Plays a tournament b/w 2 cuda solvers\n"
                    "--omp-vs-omp       # Plays a tournament b/w 2 omp solvers\n"
                    "--human-vs-seq     # Plays a game b/w a human and the seq solver\n"
                    "--human-vs-cuda    # Plays a game b/w a human and the cuda solver\n"
                    "--human-vs-omp     # Plays a game b/w a human and the omp solver\n"
                    "--time-seq         # Runs sequential solver timing\n"
                    "--time-cuda        # Does some CUDA timing\n"
                    "--time-omp        # Runs OpenMP solver timing\n"
                    "--help             # Prints this message";

    // Start parsing all given options
    while(i <= argc - 1) {
        if(!strcmp(argv[i], "--no-time-limit")) {
            time_limit = -1;
            i++;
        }
        else if(!strcmp(argv[i], "--search-depth")) {
            maxDepth = atoi(argv[i + 1]);
            i += 2;
        }
        else if(!strcmp(argv[i], "--time-limit")) {
            time_limit = (double)atoi(argv[i + 1]);
            i += 2;
        }
        else if(!strcmp(argv[i], "--width")) {
            width = atoi(argv[i + 1]);
            i += 2;
        }
        else if(!strcmp(argv[i], "--height")) {
            height = atoi(argv[i + 1]);
            i += 2;
        }
        else if(!strcmp(argv[i], "--winning-streak")) {
            winningStreak = atoi(argv[i + 1]);
            i += 2;
        }
        else if(!strcmp(argv[i], "--num-games")) {
            num_games = atoi(argv[i + 1]);
            i += 2;
        }
        else if(!strcmp(argv[i], "--human-first")) {
            human_first = true;
            i++;
        }
        else if(!strcmp(argv[i], "--num-threads")) {
            num_threads = atoi(argv[i + 1]);;
            i += 2;
        }
        else if(!strcmp(argv[i], "--seq-vs-seq")) {
            seq_vs_seq = true;
            i++;
        }
        else if(!strcmp(argv[i], "--seq-vs-cuda")) {
            seq_vs_cuda = true;
            i++;
        }
        else if(!strcmp(argv[i], "--seq-vs-omp")) {
            seq_vs_omp = true;
            i++;
        }
        else if(!strcmp(argv[i], "--omp-vs-cuda")) {
            omp_vs_cuda = true;
            i++;
        }
        else if(!strcmp(argv[i], "--cuda-vs-cuda")) {
            cuda_vs_cuda = true;
            i++;
        }
        else if(!strcmp(argv[i], "--omp-vs-omp")) {
            omp_vs_omp = true;
            i++;
        }
        else if(!strcmp(argv[i], "--human-vs-seq")) {
            human_vs_seq = true;
            i++;
        }
        else if(!strcmp(argv[i], "--human-vs-cuda")) {
            human_vs_cuda = true;
            i++;
        }
        else if(!strcmp(argv[i], "--human-vs-omp")) {
            human_vs_omp = true;
            i++;
        }
        else if(!strcmp(argv[i], "--time-seq")) {
            time_seq = true;
            i++;
        }
        else if(!strcmp(argv[i], "--time-cuda")) {
            time_cuda = true;
            i++;
        }
        else if(!strcmp(argv[i], "--time-omp")) {
            time_omp = true;
            i++;
        }
        else if(!strcmp(argv[i], "--help")) {
            cout << help_message << endl;
            return;
        }
        else {
            cout << "[ERROR] Given command line option not recognized" << endl;
            cout << endl;
            cout << help_message << endl;
            return;
        }
    }

    if (seq_vs_omp || omp_vs_cuda || omp_vs_omp || human_vs_omp || time_omp) {
        std::cout << "setting num threads " << num_threads << std::endl;
        omp_set_num_threads(num_threads);
    }

    if (time_seq) {
        test_seq_timing(width, height, winningStreak);
	return;
    }
    else if (time_cuda) {
        test_cuda_timing(maxDepth, width, height, winningStreak);
        return;
    } else if (time_omp) {
        std::cout << "time omp" << std::endl;
        test_omp_timing(width, height, winningStreak);
    }

    if(seq_vs_seq) {
        tournament_seq_vs_seq(p1, time_limit, maxDepth, width, height, 
                                winningStreak, num_games);
        return;
    }
    else if(seq_vs_cuda) {
        tournament_seq_vs_cuda(p1, time_limit, maxDepth, width, height, 
                                winningStreak, num_games);
        return;
    }
    else if(seq_vs_omp) {
        tournament_seq_vs_omp(p1, time_limit, maxDepth, width, height, 
                                winningStreak, num_games);
        return;
    }
    else if(omp_vs_cuda) {
        tournament_cuda_vs_omp(p1, time_limit, maxDepth, width, height, 
                                winningStreak, num_games);
        return;
    }
    else if(cuda_vs_cuda) {
        tournament_cuda_vs_cuda(p1, time_limit, maxDepth, width, height, 
                                winningStreak, num_games);
        return;
    }
    else if(omp_vs_omp) {
        tournament_omp_vs_omp(p1, time_limit, maxDepth, width, height, 
                                winningStreak, num_games);
        return;
    }
    else if(human_vs_seq) {
        cout << "You are playing the sequential solver SLO-MO! Prepare to be owned (slowly)!\n";
        tournament_human_vs_seq(p1, time_limit, maxDepth, width, height, 
                                winningStreak, 1, human_first);
        return;
    }
    else if(human_vs_omp) {
        cout << "You are playing the OMP solver! Prepare for your Doom!\n";
        tournament_human_vs_omp(p1, time_limit, maxDepth, width, height, 
                                winningStreak, 1, human_first);
        return;
    }
    else if(human_vs_cuda) {
        cout << "You are playing the CUDA solver I-CUDA-B-DA-BEST! Prepare to be owned!\n";
        tournament_human_vs_cuda(p1, time_limit, maxDepth, width, height, 
                                winningStreak, 1, human_first);
        return;
    }
}

int main(int argc, char **argv)
{
    // Command line options parser
    parser(argc, argv);
    return 0;
}   
