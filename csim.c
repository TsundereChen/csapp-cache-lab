#include "cachelab.h"
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct cacheData {
  int valid;
  int lru;
  uint64_t tag;
} cacheData;

typedef cacheData* cacheSet;
typedef cacheSet* cacheTable;
cacheTable cache;

bool verbose = false;
int setIndexBit;
int linePerSet;
int blockOffsetBit;
FILE *traceFile = NULL;

int hits = 0;
int misses = 0;
int evictions = 0;

void parseArg(int argc, char *argv[]);
int simulator(void);
void checkCache(int tag, int cacheSet, int offset, int counter);
unsigned int parseTag(int setIndexBit, int blockOffsetBit, int address);
unsigned int parseCacheSet(int setIndexBit, int blockOffsetBit, int address);
unsigned int parseOffset(int setIndexBit, int blockOffsetBit, int address);

int main(int argc, char *argv[]) {
  parseArg(argc, argv);
  simulator();
  printSummary(hits, misses, evictions);
  return 0;
}

void parseArg(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
    exit(1);
  };
  int opt;
  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
    switch (opt) {
    case 'h':
      printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
      printf("Options:\n");
      printf("  -h         Print this help message.\n");
      printf("  -v         Optional verbose flag.\n");
      printf("  -s <num>   Number of set index bits.\n");
      printf("  -E <num>   Number of lines per set.\n");
      printf("  -b <num>   Number of block offset bits.\n");
      printf("  -t <file>  Trace file.\n");
      printf("\n");
      printf("Examples:\n");
      printf("  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n");
      printf("  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
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
      printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
      exit(1);
    }
  }
}

int simulator(void) {
    int cacheSize = 1 << setIndexBit;
    cache = malloc(cacheSize * sizeof(cacheSet));
    // Quit function if malloc failed
    if(cache == NULL)
        return -1;
    for(int i = 0; i < cacheSize; i++){
        cache[i] = malloc(linePerSet * sizeof(cacheData));
        // Quit function if malloc failed
        if(cache[i] == NULL)
            return -1;
        for(int j = 0; j < linePerSet; j++){
            // Initialize
            cache[i][j].valid = 0;
            cache[i][j].lru = 0;
            cache[i][j].tag = 0;
        }
    }
    char buffer[20];
    char operation;
    uint64_t address;
    int size;

    int counter = 0;

    while(fgets(buffer, sizeof(buffer), traceFile) != NULL){
        counter++;
        if(buffer[0] == 'I')
            continue;
        else{
            sscanf(buffer, " %c %lx,%d", &operation, &address, &size);
            unsigned int tag = parseTag(setIndexBit, blockOffsetBit, address);
            unsigned int cacheset = parseCacheSet(setIndexBit, blockOffsetBit, address);
            unsigned int offset = parseOffset(setIndexBit, blockOffsetBit, address);
            switch(operation){
                case 'L':
                    checkCache(tag, cacheset, offset, counter);
                    break;
                case 'M':
                    checkCache(tag, cacheset, offset, counter);
                    checkCache(tag, cacheset, offset, counter);
                    break;
                case 'S':
                    checkCache(tag, cacheset, offset, counter);
                    break;
            }
        }
    }
    for(int i = 0; i < cacheSize; i++)
        free(cache[i]);
    free(cache);
    return 0;
};

void checkCache(int tag, int cacheSet, int offset, int counter) {
    for(int i = 0; i < linePerSet; i++){
        if(cache[cacheSet][i].valid == 1 && cache[cacheSet][i].tag == tag){
            cache[cacheSet][i].lru = counter;
            hits++;
            if(verbose)
                printf("HIT\n");
            return;
        }
    }
    misses++;
    if(verbose)
        printf("MISS\n");
    int n_lru = 0;
    for(int i = 0; i < linePerSet; i++){
        if(cache[cacheSet][i].valid == 0){
            cache[cacheSet][i].valid = 1;
            cache[cacheSet][i].tag = tag;
            cache[cacheSet][i].lru = counter;
            return;
        }
        else
            if(cache[cacheSet][n_lru].lru > cache[cacheSet][i].lru)
                n_lru = i;
    }

    evictions++;
    if(verbose)
        printf("EVICTION!\n");
    cache[cacheSet][n_lru].valid = 1;
    cache[cacheSet][n_lru].tag = tag;
    cache[cacheSet][n_lru].lru = counter;
    return;
};
unsigned int parseTag(int setIndexBit, int blockOffsetBit, int address){
    unsigned maskOffset = (1 << blockOffsetBit) - 1;
    unsigned maskSet = (1 << (setIndexBit + blockOffsetBit)) - (1 << blockOffsetBit);
    unsigned maskTag = ~0 - (maskOffset + maskSet);
    unsigned tag = (address & maskTag) >> (setIndexBit + blockOffsetBit);
    return tag;
};
unsigned int parseCacheSet(int setIndexBit, int blockOffsetBit, int address){
    unsigned maskSet = (1 << (setIndexBit + blockOffsetBit)) - (1 << blockOffsetBit);
    unsigned cacheSet = (address & maskSet) >> (blockOffsetBit);
    return cacheSet;

};
unsigned int parseOffset(int setIndexBit, int blockOffsetBit, int address){
    unsigned maskOffset = (1 << blockOffsetBit) - 1;
    unsigned offset = address & maskOffset;
    return offset;
};
