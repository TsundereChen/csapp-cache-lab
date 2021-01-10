#include "cachelab.h"
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

bool verbose = false;
int setIndexBit;
int linePerSet;
int blockOffsetBit;
FILE* traceFile = NULL;

void parseArg(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    parseArg(argc, argv);
}

void parseArg(int argc, char* argv[]){
    int opt;
    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){
        switch(opt){
            case 'h':
                printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
                printf("Options:\n");
                printf("  -h         Print this help message.\n");
                printf("  -v         Optional verbose flag.\n");
                printf("  -s <num>   Number of set index bits.\n");
                printf("  -E <num>   Number of lines per set.\n");
                printf("  -b <num>   Number of block offset bits.\n");
                printf("  -t <file>  Trace file.\n");
                printf("\n");
                printf("Examples:\n");
                printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
                printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
                exit(1);
            case 'v':
                verbose = true;
                break;
            case 's':
                setIndexBit = atoi(optarg);
                break;
            case 'E':
                linePerSet = atoi(optarg);
                break;
            case 'b':
                blockOffsetBit = atoi(optarg);
                break;
            case 't':
                traceFile = fopen(optarg, "r");
                break;
            default:
                printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
                exit(1);
        }
    }
}
