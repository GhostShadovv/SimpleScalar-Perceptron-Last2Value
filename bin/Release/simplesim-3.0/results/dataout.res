sim-perceptron: SimpleScalar/PISA Tool Set version 3.0 of November, 2000.
Copyright (c) 1994-2000 by Todd M. Austin.  All Rights Reserved.
This version of SimpleScalar is licensed for academic non-commercial use only.

sim: command line: ./sim-perceptron -redir:sim results/dataout.res -max:inst 50000 -threshold 10 -percep 10 -pattern 32 -vht 512 benchmarks/applu.ss 

sim: simulation started @ Sun Apr 17 13:12:36 2022, options follow:

sim-bpred: This simulator implements a branch predictor analyzer.

# -config                     # load configuration from a file
# -dumpconfig                 # dump configuration to a file
# -h                    false # print help message    
# -v                    false # verbose operation     
# -d                    false # enable debug message  
# -i                    false # start in Dlite debugger
-seed                       1 # random number generator seed (0 for timer seed)
# -q                    false # initialize and terminate immediately
# -chkpt               <null> # restore EIO trace execution from <fname>
# -redir:sim     results/dataout.res # redirect simulator output to file (non-interactive only)
# -redir:prog          <null> # redirect simulated program output to file
-nice                       0 # simulator scheduling priority
-max:inst               50000 # maximum number of inst's to execute
-memaddr                    0 # address type {0-instruction address| 1-memory address}
-pred                       1 # what to do {0-load value locality| 1-prediction}
-assoc                      0 # table type {0-dirrect mapped| 1-associative}
-history                    1 # maximum number of values memorized for a certain address
-lvpt                       4 # dimension of LVPT     
-percep                    10 # number of perceptrons 
-contextual                 1 # type of predictor     
-bpred                  bimod # branch predictor type {nottaken|taken|bimod|2lev|comb}
-vht                      512 # dimensiune VHT        
-pattern                   32 # size of pattern       
-threshold                 10 # threshold value       

sim: ** starting functional simulation w/ predictors **
am pornit initializarea pt VHT 
am intrat in vhtINit
am facut initializarea pt VHT, pornesc pt tabela de perceptroni 
warning: syscall: sigvec ignored
warning: syscall: sigvec ignored
warning: syscall: sigvec ignored
warning: syscall: sigvec ignored
warning: syscall: sigvec ignored
warning: syscall: sigvec ignored
Acuratetea predictiei perceptron= 33.808690 
Acuratetea predictiei load value= 0.000000 
Incarcare VHT Table 64/512 = 12.500000 

sim: ** simulation statistics **
sim_num_insn                  50000 # total number of instructions executed
valuePrediction                   0 # total number of correctly predicted values
classifiedPred                    0 # number of loads classified as predictable
classifiedUnpred                  5 # number of loads classified as unpredictable
predictable                       0 # correctly classified predictable loads
unpredictable                     5 # correctly classified unpredictable loads
sim_num_refs                  20264 # total number of loads and stores executed
sim_num_loads                 11991 # total number of loads executed
sim_elapsed_time                  1 # total simulation time in seconds
sim_inst_rate            50000.0000 # simulation speed (in insts/sec)
sim_num_branches              11440 # total number of branches executed
sim_IPB                      4.3706 # instruction per branch
missVht                        5118 # number of misses in vht
nrWrong                        2819 # total number of wrong predictions
nrCorrect                      4054 # total number of correct predictions
nrCorrectThresh                3084 # total number of correct predictions but the output was greater then the threshold

