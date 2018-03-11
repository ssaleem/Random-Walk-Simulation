# Many Random Walks on a Large Graph
This project simulates infinite number of random walks on a large graph in a distributed manner where _MPI(Message Passing Interface)_ is used for inter-process communication. The program keeps graph as well as credit for each node in memory to efficiently access and update.

## Installation
- Use the Makefile to compile `make`
- Tu run `make run ARGS=“no_of_partitions rw graphFile partitionFile no_of_rounds`
    - `graphFile`: Edge representation of an undirected graph with each line containing IDs of two nodes that have an edge.
    - `partitionFile`: File containing partition information of nodes in the format `Node ID, Node Degree, Partition ID`.


make   //For compilation
make run ARGS=“partitions rw graphFile partitionFile rounds”  //For running the program(rw is executable's name)
For example for running the program with 4 and 2 partitions respectively
$make run ARGS="4 rw fl_compact.tab fl_compact_part.4 5"
$make run ARGS="2 rw fl_compact.tab fl_compact_part.2 5"

Does your program run on ix-trusty: Yes

Does your program generate the correct results with 2 and 4 partitions on ix-trusty: Yes

Does your program have a limit for the number of nodes in the input graph? No

How long does it take for your program to read the input file on ix-trusty?
3-6 seconds
How long does it take for your program (on average) to complete each round of processing on ix-trusty?
0.93 seconds for 2 partitions
0.65 for 4 partitions