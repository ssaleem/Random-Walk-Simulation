/*
 * CIS630-Project-II
 */
#include <fstream>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <algorithm>
#include <unistd.h>
#include "structs.h"


int main(int argc , char *argv[]) {
    
    /*   if(argc < 5) {
     usage(argv[0]);     mpiexec -np 2 rw fl_compact.tab fl_compact_part.2 5
     exit(1);
     } */
    
    //Extract Rounds
    int rounds;
    rounds = atoi(argv[3]);
    /************************MPI Initialization***************************************/
    //Initialize MPI-MPI specific variables
    int level, numtasks, taskid;
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    cout<<"MPI process "<<taskid<<" has started...\n";

    /************************Graph Reading***************************************/
    int degree, node1, node2, nodes;
    nodes = 0;
    double credit;
    double mytime,   /*variables used for gathering timing statistics*/
           maxtime, mytime1;
           
    vvi graphTableI, graphTableE;		//Internal/External graph in 2D int vectors
    vvf creditTable;		//Credits of a particular partition in 2D double vector
	vi partInfo, degreeInfo;
    
    char* f = NULL;
    char* g = NULL;
    char* end1 = NULL;
    char* end2 = NULL;
    size_t length1, length2;
    
    //memory mapping
    f = map_file(argv[1], length1);		//Map whole graph file
    g = map_file(argv[2], length2);   //Map Partition info file
    
    end1 = f + length1;
    end2 = g + length2;
    
    //Reserve memory
    int estLength = length1/100;
    partInfo.resize(estLength);
    degreeInfo.resize(estLength);
    
    //Start reading from file to graphTable
    MPI_Barrier(MPI_COMM_WORLD);
    mytime = MPI_Wtime();
    //fill partition info
    while (g && g!=end2){
        node1 = 0;
        while( (*g) != '\t')   //atoi() is slowing down performance so using my own function
            node1 = node1*10 + (*g++ - '0');
        g++;
        
        degree = 0;
        while( (*g) != '\t')
            degree = degree*10 + (*g++ - '0');
        g++;
        degreeInfo[node1] = degree;
        
        partInfo[node1] = *g++ - '0';
        g++;
        
        nodes++;
    }
    nodes++;  //to accomodate for no node 0
    graphTableE.resize(nodes);
    graphTableI.resize(nodes);
    
    while (f && f!=end1){
        node1 = 0;
        while( (*f) != '\t')
            node1 = node1*10 + (*f++ - '0');
        f++;
        
        node2 = 0;
        while( (*f) != '\n')
            node2 = node2*10 + (*f++ - '0');
        f++;
        
        if(partInfo[node1] == taskid){
            if(partInfo[node2] == taskid){
                graphTableI[node1].push_back(node2);
                graphTableI[node2].push_back(node1);
            }
            else
                graphTableE[node1].push_back(node2);
        }
        else if(partInfo[node2] == taskid)
            graphTableE[node2].push_back(node1);
    }
    
    mytime = MPI_Wtime() - mytime; 
    cout<<"time to read input files, partition "<<taskid<< "= "<<mytime<<"sec \n";
    
    /************************Prepare for rounds***************************************
     1-Find highest index
     2-Allocate space in creditTable for my partition
     *********************************************************************************/
     int neighbor, peerPart, highestIndex;
     double **v1;
	 double **v2;   //Temp credits/degree from last round
    for(int i = 1; i < nodes; i++)  //highest neighbor in my partition
        if(graphTableE[i].size() != 0 || graphTableI[i].size() != 0)
            highestIndex = i;
    creditTable.resize(highestIndex+1);
    for(int i = 1; i <= highestIndex; i++)
        creditTable[i].resize(rounds+1);
    
    v1 = new double*[numtasks];
    v2 =  new double*[numtasks]; 
    
    for( int p = 0; p < numtasks; p++) {
    	v1[p] = new double[nodes];
    	v2[p] = new double[nodes];
    }
    for(int j = 1; j <= highestIndex; j++)
        if(partInfo[j] == taskid)
            v2[taskid][j] = 1.0/degreeInfo[j];
    //calculate credit only for internal nodes
    int I, E;
    vi v;
    vi Esize, Isize;
    Esize.resize(highestIndex+1);
    for(int j = 1; j <= highestIndex; j++)
    	Esize[j] = graphTableE[j].size();
    Isize.resize(highestIndex+1);
    for(int j = 1; j <= highestIndex; j++)
    	Isize[j] = graphTableI[j].size();
    	
    for(int i = 1; i <= rounds; i++){
    	MPI_Barrier(MPI_COMM_WORLD);
    	mytime = MPI_Wtime();
    	for(int n = 0; n < numtasks; n++){
			if(i%2 == 0)
				MPI_Bcast(v1[n], nodes, MPI_DOUBLE, n, MPI_COMM_WORLD);  //&v1 is sending my credit/degree from last round
			else
				MPI_Bcast(v2[n], nodes, MPI_DOUBLE, n, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
            if(i%2 == 0) {
                for(int j = 1; j <= highestIndex; j++) {
                    if(partInfo[j] == taskid){
                        credit = 0;
                        v = graphTableI[j];
                        I = Isize[j];
                        E = Esize[j];
                        for(int k =0; k < I; k++)
                            credit += v1[taskid][v[k]];
                        for(int k =0; k < E; k++){
                        	neighbor = graphTableE[j][k];
                        	peerPart = partInfo[neighbor];
                            credit += v1[peerPart][neighbor];
                        }
                        creditTable[j][i] = credit;
                        v2[taskid][j] = credit/degreeInfo[j];
                    }
                    
                }
            }
            else {
                for(int j = 1; j <= highestIndex; j++) {
                    if(partInfo[j] == taskid){
                        credit = 0;
                        v = graphTableI[j];
                        I = graphTableI[j].size();
                        E = graphTableE[j].size();
                        for(int k =0; k < I; k++)
                            credit += v2[taskid][v[k]];
                        for(int k =0; k < E; k++){
                            neighbor = graphTableE[j][k];
                        	peerPart = partInfo[neighbor];
                            credit += v2[peerPart][neighbor];
                        }
                        creditTable[j][i] = credit;
                        v1[taskid][j] = credit/degreeInfo[j];
                    }
                }
				
            }
        mytime = MPI_Wtime() - mytime; 
        cout<<"time for round "<<i<<", partition "<<taskid<< "= "<<mytime<<"sec \n";
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Reduce(&mytime, &maxtime, 1, MPI_DOUBLE,MPI_MAX, 0, MPI_COMM_WORLD);
    	if (taskid == MASTER)
      		cout<<"Total time for round "<<i<<": "<<maxtime<<"sec\n";
    }
    //write credit to file
    char outFile[14];
    snprintf(outFile, sizeof(outFile), "%d.out", taskid);
    ofstream out(outFile);
    out.precision(6);
    out<<fixed;
     mytime = MPI_Wtime();
    for(int j = 1; j <= highestIndex; j++) {
        if(partInfo[j] == taskid){
            out<<j<<"\t"<<degreeInfo[j]<<"\t";
            for(int i = 1; i <= rounds; i++)
                out<<creditTable[j][i]<<"\t";
            out<<"\n";
        }
    }
    mytime = MPI_Wtime() - mytime; 
    cout<<"time to write the output file, partition "<<taskid<< "= "<<mytime<<"sec \n";
    out.close();
    MPI_Finalize();
    return 0;
}
