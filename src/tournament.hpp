/**
 * @defgroup   TOURNAMENT
 *
 * @brief      This file implements tournament APIs which can allow humans/solvers
 * to play against each other.
 *
 * @author     Satvik
 * @date       2021
 */
#ifndef __TOURNAMENT__
#define __TOURNAMENT__

#include "connectFourAssets/player.hpp" 
#include "connectFourAssets/slotStatus.hpp"
#include "sequentialSolver/sequentialSolver.hpp"
#include "gameTreeSearchSolver.hpp"
#include "mpSolver/mpSolver.hpp"
#include <iostream>

using namespace std;

#define DURATION std::chrono::duration_cast<std::chrono::duration<double, std::milli>> 
#define NOW std::chrono::high_resolution_clock::now
typedef std::chrono::high_resolution_clock::time_point TimePoint;

void tournament_seq_vs_seq(Player p1, double time_limit, int maxDepth,
						   int width, int height, int winningStreakSize,
						   int num_games) {
	SequentialSolver* seq1 = new SequentialSolver(width, height, winningStreakSize);
	SequentialSolver* seq2 = new SequentialSolver(width, height, winningStreakSize);
	Player p2 = seq1->oppPlayer(p1);	
	TimePoint start, end;
	uint64_t totalNodes1, totalNodes2;
	totalNodes2 = 0;
	totalNodes1 = 0;
	start = NOW();
	for(int i = 0; i < num_games; i++) {
		while(1) {
			int move = seq1->solve(p1, maxDepth, time_limit);
			if(move == -1) break;
			seq2->playMove(move+1, p1); // add one because this uses 1-indexed col
			move = seq2->solve(p2, maxDepth, time_limit);
			if(move == -1) break;
			seq1->playMove(move+1, p2);
		}
		totalNodes1 += seq1->getTotalNodesTraversed();
		totalNodes2 += seq2->getTotalNodesTraversed();
		seq1->resetSolver();
		seq2->resetSolver();
	}
	end = NOW();
	double time = DURATION(end - start).count();
	time = time / num_games;

	// Print stats of tournament
	cout << "[SLO-POKE1 VS SLO-POKE2] AvgTime = " << time << endl;
	cout << "[SLO-POKE1 VS SLO-POKE2] SLO-POKE1.AvgNodesTraversed = " << 
										totalNodes1 / num_games << endl;
	cout << "[SLO-POKE1 VS SLO-POKE2] SLO-POKE2.AvgNodesTraversed = " << 
										totalNodes2 / num_games << endl;
}

void tournament_seq_vs_cuda(Player p1, double time_limit, int maxDepth,
						   int width, int height, int winningStreakSize,
                           int num_games) {
    SequentialSolver* seq = new SequentialSolver(width, height, winningStreakSize);
	CudaSolver* cu = new CudaSolver(width, height, winningStreakSize);
	Player p2 = seq->oppPlayer(p1);	
	TimePoint start, end;
	uint64_t totalNodes1, totalNodes2;
	totalNodes2 = 0;
	totalNodes1 = 0;
	start = NOW();
	for(int i = 0; i < num_games; i++) {
		while(1) {
			int move = seq->solve(p1, maxDepth, time_limit);
			if(move == -1) break;
			cu->playMove(move+1, p1); // add one because this uses 1-indexed col
			move = cu->solve(p2, maxDepth, time_limit);
			if(move == -1) break;
			seq->playMove(move+1, p2);
		}
		totalNodes1 += seq->getTotalNodesTraversed();
		totalNodes2 += cu->getTotalNodesTraversed();
		seq->resetSolver();
        cu->resetSolver();
	}
	end = NOW();
	double time = DURATION(end - start).count();
	time = time / num_games;

	// Print stats of tournament
	cout << "[SLO-POKE VS I-CUDA-B-DA-BEST] AvgTime = " << time << endl;
	cout << "[SLO-POKE VS I-CUDA-B-DA-BEST] SLO-POKE.AvgNodesTraversed = " << 
										totalNodes1 / num_games << endl;
	cout << "[SLO-POKE VS I-CUDA-B-DA-BEST] I-CUDA-B-DA-BEST.AvgNodesTraversed = " << 
										totalNodes2 / num_games << endl;
}

void tournament_seq_vs_omp(Player p1, double time_limit, int maxDepth,
						   int width, int height, int winningStreakSize,
						   int num_games) {
	SequentialSolver* seq = new SequentialSolver(width, height, winningStreakSize);                        					   
	MpSolver* 	  mp  = new MpSolver(width, height, winningStreakSize);                        					   
	Player p2 = seq->oppPlayer(p1);	                                                                					   
	TimePoint start, end;                                                                                   					   
	uint64_t totalNodes1, totalNodes2;                                                                      					   
	totalNodes2 = 0;                                                                                        					   
	totalNodes1 = 0;                                                                                        					   
	start = NOW();                                                                                          					   
	for(int i = 0; i < num_games; i++) {                                                                    					   
		while(1) {                                                                                      					   
			int move = seq->solve(p1, maxDepth, time_limit);                                       					   
			if(move == -1) break;                                                                   					   
			mp->playMove(move+1, p1); // add one because this uses 1-indexed col                                                             					   
			move = mp->solve(p2, maxDepth, time_limit);                                           					   
			if(move == -1) break;                                                                   					   
			seq->playMove(move+1, p2);                                                               					   
		}                                                                                              					   
		totalNodes1 += seq->getTotalNodesTraversed();                                                  					   
		totalNodes2 += mp->getTotalNodesTraversed();                                                  					   
		seq->resetSolver();                                                                            					   
		mp->resetSolver();                                                                            					   
	}                                                                                                       					   
	end = NOW();                                                                                            					   
	double time = DURATION(end - start).count();                                                            					   
	time = time / num_games;                                                                                					   
	                                                                                                         					   
	// Print stats of tournament                                                                            					   
	cout << "[SLO-POKE1 VS SLO-POKE2] AvgTime = " << time << endl;                                          					   
	cout << "[SLO-POKE1 VS SLO-POKE2] SLO-POKE1.AvgNodesTraversed = " <<                                    					   
										totalNodes1 / num_games << endl;					   
	cout << "[SLO-POKE1 VS SLO-POKE2] SLO-POKE2.AvgNodesTraversed = " <<                                    					   
										totalNodes2 / num_games << endl;					   
						   
 }
						   
void test_cuda_timing(int maxDepth, int width, int height, int winningStreakSize) {

    SequentialSolver* seq = new SequentialSolver(width, height, winningStreakSize);
	CudaSolver* cu = new CudaSolver(width, height, winningStreakSize);
    auto p1 = Player::Red;
	Player p2 = seq->oppPlayer(p1);	
	TimePoint start, end;

    // Get a few rounds in first
    int move = seq->solve(p1, 2, -1);
    cu->playMove(move, p1);
    start = NOW();
    move = cu->solve(p2, maxDepth, -1);
    end = NOW();
    double time_first = DURATION(end - start).count();
    seq->playMove(move, p2);
    move = seq->solve(p1, 2, -1);
	cu->playMove(move, p1);
	
    start = NOW();
    cu->solve(p2, maxDepth, -1);
    end = NOW();
	double time_second = DURATION(end - start).count();
	// Print stats of tournament
	cout << "CUDA solver first turn: " << time_first << endl;
    cout << "CUDA solver first turn: " << time_second << endl;
}

int test_seq_timing(int width, int height, int winningStreakSize)
{
    // Command line options parser
    // parser(argc, argv);
    SequentialSolver* sol = new SequentialSolver(width, height, winningStreakSize);
    TimePoint start, end;
    for(int i = 2; i <= 12; i+=2) {
        start = NOW();
        sol->solve(Player::Red, i, -1);
        end = NOW();
        cout << "depth " << i << " took " << DURATION(end - start).count() << endl;
        sol->resetSolver();
    }
    return 0;
}

int test_omp_timing(int width, int height, int winningStreakSize) {
    MpSolver* sol = new MpSolver(width, height, winningStreakSize);
    TimePoint start, end;
    for(int i = 2; i <= 6; i+=2) {
      start = NOW();
      sol->solve(Player::Red, i, -1);
      end = NOW();
      cout << "depth " << i << " took " << DURATION(end - start).count() << endl;
      sol->resetSolver();
    }
}

void tournament_cuda_vs_omp(Player p1, double time_limit, int maxDepth,
						   int width, int height, int winningStreakSize,
						   int num_games) {
	CudaSolver* cu = new CudaSolver(width, height, winningStreakSize);                        					   
	MpSolver* 	  mp  = new MpSolver(width, height, winningStreakSize);         
	Player p2 = PlayerHelpers::OppositePlayer(p1);	                                      						   
	TimePoint start, end;                                                						   
	uint64_t totalNodes1, totalNodes2;                                     					   						   
	totalNodes2 = 0;                                                  					   						   
	totalNodes1 = 0;                                               
	start = NOW();                                           					   						   
	for(int i = 0; i < num_games; i++) {                                        
		while(1) {                                                    						   
			int move = cu->solve(p1, maxDepth, time_limit);      						   
			if(move == -1) break;                             	   						   
			mp->playMove(move+1, p1); // add one because this uses 1-indexed col                         						   
			move = mp->solve(p2, maxDepth, time_limit);     	     						   
			if(move == -1) break;                          				   						   
			cu->playMove(move+1, p2);                      				    						   
		}                                                     					   						   
		totalNodes1 += cu->getTotalNodesTraversed();        					    						   
		totalNodes2 += mp->getTotalNodesTraversed();        					     						   
		cu->resetSolver();                                  					    						   
		mp->resetSolver();                                  					     						   
	}                                                             					   						   
	end = NOW();                                                  					   						   
	double time = DURATION(end - start).count();                  					   						   
	time = time / num_games;                                      					   						   
	                                                               					   						   
	// Print stats of tournament          
	cout << "[SLO-POKE1 VS SLO-POKE2] AvgTime = " << time << endl;           
	cout << "[SLO-POKE1 VS SLO-POKE2] SLO-POKE1.AvgNodesTraversed = " <<                         
										totalNodes1 / num_games << endl; 
	cout << "[SLO-POKE1 VS SLO-POKE2] SLO-POKE2.AvgNodesTraversed = " <<                                 
										totalNodes2 / num_games << endl;
						   
}

void tournament_human_vs_seq(Player p1, double time_limit, int maxDepth,
						   int width, int height, int winningStreakSize,
						   int num_games, bool human_first) {
	SequentialSolver* seq1 = new SequentialSolver(width, height, winningStreakSize);
	Player p2 = seq1->oppPlayer(p1);	
	uint64_t totalNodes1;
	totalNodes1 = 0;
	int move = -1;
	if(human_first) {
		while(move == -1) {
            cout << "Enter the column where you want to play your move: ";
         	cin >> move;
         	cout << endl;
            move = seq1->playMove(move, p2);
		}
	}
	seq1->printBoard();
	for(int i = 0; i < num_games; i++) {
		while(1) {
			move = seq1->solve(p1, maxDepth, time_limit);
			if(move == -1) break;
			cout << "AI plays in column: " << move + 1 << endl;
			seq1->printBoard();
			move = -1;
			while(move == -1) {
	            cout << "Enter the column where you want to play your move: ";
             	cin >> move;
             	cout << endl;
	            move = seq1->playMove(move, p2);
			}
			if(move == -1) break;
			seq1->printBoard();
			cout << endl << endl;
		}
		totalNodes1 += seq1->getTotalNodesTraversed();
		seq1->resetSolver();
	}
	// Print stats of tournament
	cout << "[SLO-POKE VS PUNY-MORTAL] AvgNodesTraversed = " << 
										totalNodes1 / num_games << endl;
}

void tournament_human_vs_cuda(Player p1, double time_limit, int maxDepth,
						   int width, int height, int winningStreakSize,
						   int num_games, bool human_first) {
	CudaSolver* cu = new CudaSolver(width, height, winningStreakSize);
	Player p2 = PlayerHelpers::OppositePlayer(p1);	
	uint64_t totalNodes1;
	totalNodes1 = 0;
	int move = -1;
	if(human_first) {
		while(move == -1) {
            cout << "Enter the column where you want to play your move: ";
         	cin >> move;
         	cout << endl;
            move = cu->playMove(move, p2);
		}
	}
	cu->printBoard();
	for(int i = 0; i < num_games; i++) {
		while(1) {
			move = cu->solve(p1, maxDepth, time_limit);
			if(move == -1) break;
			cout << "AI plays in column: " << move + 1 << endl;
			cu->printBoard();
			move = -1;
			while(move == -1) {
	            cout << "Enter the column where you want to play your move: ";
             	cin >> move;
             	cout << endl;
	            move = cu->playMove(move, p2);
			}
			if(move == -1) break;
			cu->printBoard();
			cout << endl << endl;
		}
		totalNodes1 += cu->getTotalNodesTraversed();
		cu->resetSolver();
	}
	// Print stats of tournament
	cout << "[I-CUDA-B-DA-BEST VS PUNY-MORTAL] AvgNodesTraversed = " << 
										totalNodes1 / num_games << endl;

}

void tournament_human_vs_omp(Player p1, double time_limit, int maxDepth,
						   int width, int height, int winningStreakSize,
						   int num_games, bool human_first) {
	MpSolver* mp1 = new MpSolver(width, height, winningStreakSize);    
	Player p2 = mp1->oppPlayer(p1);	                          
	uint64_t totalNodes1;                                            
	totalNodes1 = 0;                                                
	int move = -1;                                                 
	if(human_first) {                                             
		while(move == -1) {                                  
	    cout << "Enter the column where you want to play your move: ";
	 	cin >> move;                                             
	 	cout << endl;                                           
	    move = mp1->playMove(move, p2);                           
		}                                                     
	}                                                            
	mp1->printBoard();                                         
	for(int i = 0; i < num_games; i++) {                       
		while(1) {                                        
			move = mp1->solve(p1, maxDepth, time_limit);
			if(move == -1) break;                       
			cout << "AI plays in column: " << move + 1 << endl; 
			mp1->printBoard();                                
			move = -1;                                        
			while(move == -1) {                              
	            cout << "Enter the column where you want to play your move: "; 
	     	cin >> move;                                                      
	     	cout << endl;                                                    
	            move = mp1->playMove(move, p2);                          
			}                                                    
			if(move == -1) break;                               
			mp1->printBoard();                                
			cout << endl << endl;                             
		}                                                        
		totalNodes1 += mp1->getTotalNodesTraversed();          
		mp1->resetSolver();                                   
	}                                                             
	// Print stats of tournament                                 
	cout << "[SLO-POKE VS PUNY-MORTAL] AvgNodesTraversed = " << 
										totalNodes1 / num_games << endl;
}

void tournament_omp_vs_omp(Player p1, double time_limit, int maxDepth,
						   int width, int height, int winningStreakSize,
						   int num_games) {
	MpSolver* mp1  = new MpSolver(width, height, winningStreakSize);              
	MpSolver* mp2  = new MpSolver(width, height, winningStreakSize);                                         					   
	Player p2 = mp1->oppPlayer(p1);	                                      		 
	TimePoint start, end;                                                	  					   
	uint64_t totalNodes1, totalNodes2;                                     					   					   
	totalNodes2 = 0;                                                  					   					   
	totalNodes1 = 0;                                                                                                       					   
	start = NOW();                                           					       					   
	for(int i = 0; i < num_games; i++) {                                                                                   					   
		while(1) {                                                                    					   
			int move = mp1->solve(p1, maxDepth, time_limit);      			                               					   
			if(move == -1) break;                             	   	                          					   
			mp2->playMove(move+1, p1); // add one because this uses 1-indexed col                              	                                 					   
			move = mp2->solve(p2, maxDepth, time_limit);     			                              					   
			if(move == -1) break;                          				         					   
			mp1->playMove(move+1, p2);                      					         					   
		}                                                    						   					   
		totalNodes1 += mp1->getTotalNodesTraversed();        						   					   
		totalNodes2 += mp2->getTotalNodesTraversed();         						   					   
		mp1->resetSolver();                                  						   					   
		mp2->resetSolver();                                   						   					   
	}                                                       		   						   					   
	end = NOW();                                          				   					   
	double time = DURATION(end - start).count();        						   					   
	time = time / num_games;                            						   					   
	                                                		   						   					   
	// Print stats of tournament                                                                                           					   
	cout << "[SLO-POKE1 VS SLO-POKE2] AvgTime = " << time << endl;                                                         					   
	cout << "[SLO-POKE1 VS SLO-POKE2] SLO-POKE1.AvgNodesTraversed = " <<                                                   					   
										totalNodes1 / num_games << endl;      					   
	cout << "[SLO-POKE1 VS SLO-POKE2] SLO-POKE2.AvgNodesTraversed = " <<                                          					   
										totalNodes2 / num_games << endl; 
}

void tournament_cuda_vs_cuda(Player p1, double time_limit, int maxDepth,
						   int width, int height, int winningStreakSize,
						   int num_games) {
    CudaSolver* cu1 = new CudaSolver(width, height, winningStreakSize);
	CudaSolver* cu2 = new CudaSolver(width, height, winningStreakSize);
	Player p2 = PlayerHelpers::OppositePlayer(p1);	
	TimePoint start, end;
	uint64_t totalNodes1, totalNodes2;
	totalNodes2 = 0;
	totalNodes1 = 0;
	start = NOW();
	for(int i = 0; i < num_games; i++) {
		while(1) {
			int move = cu1->solve(p1, maxDepth, time_limit);
			if(move == -1) break;
			cu2->playMove(move+1, p1); // add one because this uses 1-indexed col
			move = cu2->solve(p2, maxDepth, time_limit);
			if(move == -1) break;
			cu1->playMove(move+1, p2);
		}
        std::cout << "round " << i << " nodes traversed\n\tsolver1 " << cu1->getTotalNodesTraversed() << "\n\tsolver2 " << cu2->getTotalNodesTraversed() << std::endl;
		totalNodes1 += cu1->getTotalNodesTraversed();
		totalNodes2 += cu2->getTotalNodesTraversed();
		cu1->resetSolver();
        cu2->resetSolver();
	}
	end = NOW();
	double time = DURATION(end - start).count();
	time = time / num_games;

	// Print stats of tournament
	cout << "[I-CUDA-B-DA-BEST1 VS I-CUDA-B-DA-BEST2] AvgTime = " << time << endl;
	cout << "[I-CUDA-B-DA-BEST1 VS I-CUDA-B-DA-BEST2] I-CUDA-B-DA-BEST1.AvgNodesTraversed = " << 
										totalNodes1 / num_games << endl;
	cout << "[I-CUDA-B-DA-BEST1 VS I-CUDA-B-DA-BEST2] I-CUDA-B-DA-BEST2.AvgNodesTraversed = " << 
										totalNodes2 / num_games << endl;
}

#endif
