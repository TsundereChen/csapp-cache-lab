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
int checkCache(uint64_t address);

int main(int argc, char *argv[]) {
  parseArg(argc, argv);
  simulator();
  printSummary(hits, misses, evictions);
  return 0;
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
      int ret = 0;
      sscanf(buffer, " %c %lx,%d", &operation, &addr, &size);
      switch (operation) {
      case 'S':
        ret = checkCache(addr);
        break;
      case 'L':
        ret = checkCache(addr);
        break;
      case 'M':
        ret = checkCache(addr);
        hits++;
        break;
      }
      if (verbose) {
        switch (ret) {
        case 0:
          printf("%c %lx,%d HIT.\n", operation, addr, size);
          break;
        case 1:
          printf("%c %lx,%d MISS.\n", operation, addr, size);
          break;
        case 2:
          printf("%c %lx,%d EVICTION.\n", operation, addr, size);
          break;
        }
      }
    }
  }
  for (int i = 0; i < setIndexBit; i++)
    free(cacheTable[i]);
  free(cacheTable);
  fclose(traceFile);
  return 0;
};

int checkCache(uint64_t address) {
  uint64_t tag = address >> (setIndexBit + blockOffsetBit);
  unsigned int setIndex = address >> blockOffsetBit & ((1 << setIndexBit) - 1);

  int evict = 0;
  int empty = -1;
  cacheData *cacheSet = cacheTable[setIndex];
  for (int i = 0; i < linePerSet; i++) {
    if (cacheSet[i].valid) {
      if (cacheSet[i].tag == tag) {
        hits++;
        cacheSet[i].lru = 1;
        return 0;
      }
      cacheSet[i].lru++;
      if (cacheSet[evict].lru <= cacheSet[i].lru) {
        evict = i;
      }
    } else
      empty = i;
  }
  misses++;
  if (empty != -1) {
    cacheSet[empty].valid = 1;
    cacheSet[empty].tag = tag;
    cacheSet[empty].lru = 1;
    return 1;
  } else {
    cacheSet[evict].tag = tag;
    cacheSet[evict].lru = 1;
    evictions++;
    return 2;
  }
};
