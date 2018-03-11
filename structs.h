#include <iostream>
#include <vector>
#include <pthread.h>
#include <mpi.h>
using namespace std;

typedef vector<int> vi;
typedef vector< vi > vvi;
typedef vector<double> vf;
typedef vector< vf > vvf;


#define  MASTER	0

//Function definitions
void usage(char* argv0) {
    cout<<"Usage: "<< argv0 << " infile outfile rounds \n";
} 

void handle_error(const char* msg) {
    perror(msg);
    exit(1);
}

char* map_file(const char* file, size_t& length) {
    int fd = open(file, O_RDONLY);
    if (fd == -1)
        handle_error("open");
    
    struct stat sb;
    if (fstat(fd, &sb) == -1)
        handle_error("fstat");
    length = sb.st_size;
    
    char* addr = static_cast<char*>(mmap(NULL, length, PROT_READ, MAP_PRIVATE, fd, 0u));
    if (addr == MAP_FAILED)
        handle_error("mmap");
    return addr;
}
void unmap_file(char* &addr, size_t length) {
    if (munmap(addr, length) < 0)
        handle_error("unmap");
}

