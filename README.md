# Connect4 Game Tree Search Comparison

Final project for Spring 2021 ME759 for Prajyot Gupta, Satvik Maurya, and Robert Viramontes.

## To build

Project makes use of the CMake build system to manage the build of the project. This project uses out-of-source build methodology. To build, see below:

```
> mkdir build
> cd build 
> cmake ../src
> cmake --build .
```

## To run

The project supports multiple game modes for Connect-4. After building the application, information on all the relevant command line options can be printed via-

```
> ./app --help
```

Note- If no command line options are provided, the application defaults to an interactive game between the user and the base sequential solver.

The options are also listed here for convenience:



```
--no-time-limit    # No time limit per move.
--search-depth [depth]      # Set the max search depth per move
--time-limt [time]      # Set the time limit per move (in sec)
--width [width]     # Set the width of the board
--height [height]      # Set the height of the board
--winning-streak [streak]     # Set the length of the winning streak
--num-games [num]    # Set the number of games to be played
--human-first      # For the interactive games only - use if you want to play the first move
--num-threads      # Set number of threads OMP solver can use
--seq-vs-seq       # Plays a tournament b/w 2 seq solvers
--seq-vs-cuda      # Plays a tournament b/w the seq and cuda solvers
--seq-vs-omp       # Plays a tournament b/w the seq and omp solvers
--omp-vs-cuda      # Plays a tournament b/w the cuda and omp solvers
--cuda-vs-cuda     # Plays a tournament b/w 2 cuda solvers
--omp-vs-omp       # Plays a tournament b/w 2 omp solvers
--human-vs-seq     # Plays a game b/w a human and the seq solver
--human-vs-cuda    # Plays a game b/w a human and the cuda solver
--human-vs-omp     # Plays a game b/w a human and the omp solver
--time-seq         # Runs sequential solver timing
--time-cuda        # Does some CUDA timing
--time-omp         # Runs OpenMP solver timing
--help             # Prints this message
```
