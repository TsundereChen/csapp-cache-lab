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

cacheData **cacheTable;

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
void checkCache(uint64_t address);

int main(int argc, char *argv[]) {
  parseArg(argc, argv);
  simulator();
}

void parseArg(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
    switch (opt) {
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

int simulator(void) {
  // Create space for cache
  int cacheSize = pow(2, setIndexBit);
  cacheTable = malloc(sizeof(cacheData) * cacheSize);
  // Return if malloc failed
  if (cacheTable == NULL)
    return -1;
  for (int i = 0; i < setIndexBit; i++) {
    // Create space for cache
    cacheTable[i] = calloc(linePerSet, sizeof(cacheData));
    // Return if malloc failed
    if (cacheTable[i] == NULL)
      return -1;
  }

  // Prepare variables for per-line operation
  char buffer[20];
  char operation;
  uint64_t addr;
  int size;

  while (fgets(buffer, sizeof(buffer), traceFile) != NULL) {
    int returnValue;
    if (buffer[0] == 'I') {
      // Ignoring instruction load operation
      continue;
    } else {
      sscanf(buffer, " %c %lx,%d", &operation, &addr, &size);
      switch (operation) {
      case 'S':
        checkCache(addr);
        break;
      case 'L':
        checkCache(addr);
        break;
      case 'M':
        checkCache(addr);
        hits++;
        break;
      }
    }
  }
  for (int i = 0; i < setIndexBit; i++)
    free(cacheTable[i]);
  free(cacheTable);
  fclose(traceFile);
  return 0;
};
