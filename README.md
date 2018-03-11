# Many Random Walks on a Large Graph
This project simulates infinite number of random walks on a large graph in a distributed manner where _MPI(Message Passing Interface)_ is used for inter-process communication. The program keeps graph as well as credit for each node in memory to efficiently access and update.

## Installation
- Use the Makefile to compile `make`
- Tu run `make run ARGS=“no_of_partitions rw graphFile partitionFile no_of_rounds"`
    - `graphFile`: Edge representation of an undirected graph with each line containing IDs of two nodes that have an edge.
    - `partitionFile`: File containing partition information of nodes in the format `Node ID, Node Degree, Partition ID`.
