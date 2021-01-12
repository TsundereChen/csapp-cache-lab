/*
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */
#include "cachelab.h"
#include <stdio.h>

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
  if (N == 32) {
    // Case 1
    for (int i = 0; i < N; i += 8) {
      for (int j = 0; j < M; j += 8) {
        for (int k = i; k < i + 8; k++) {
          int t0 = A[k][j];
          int t1 = A[k][j + 1];
          int t2 = A[k][j + 2];
          int t3 = A[k][j + 3];
          int t4 = A[k][j + 4];
          int t5 = A[k][j + 5];
          int t6 = A[k][j + 6];
          int t7 = A[k][j + 7];

          B[j][k] = t0;
          B[j + 1][k] = t1;
          B[j + 2][k] = t2;
          B[j + 3][k] = t3;
          B[j + 4][k] = t4;
          B[j + 5][k] = t5;
          B[j + 6][k] = t6;
          B[j + 7][k] = t7;
        }
      }
    }
  } else if (N == 64) {
    // Case 2
    for (int i = 0; i < N; i += 8) {
      for (int j = 0; j < M; j += 8) {
        for (int k = j; k < j + 4; ++k) {
          int t0 = A[k][i];
          int t1 = A[k][i + 1];
          int t2 = A[k][i + 2];
          int t3 = A[k][i + 3];
          int t4 = A[k][i + 4];
          int t5 = A[k][i + 5];
          int t6 = A[k][i + 6];
          int t7 = A[k][i + 7];
          B[i][k] = t0;
          B[i + 1][k] = t1;
          B[i + 2][k] = t2;
          B[i + 3][k] = t3;
          B[i][k + 4] = t4;
          B[i + 1][k + 4] = t5;
          B[i + 2][k + 4] = t6;
          B[i + 3][k + 4] = t7;
        }
        for (int k = i; k < i + 4; ++k) {
          int t0 = B[k][j + 4];
          int t1 = B[k][j + 5];
          int t2 = B[k][j + 6];
          int t3 = B[k][j + 7];
          int t4 = A[j + 4][k];
          int t5 = A[j + 5][k];
          int t6 = A[j + 6][k];
          int t7 = A[j + 7][k];

          B[k + 4][j] = t0;
          B[k + 4][j + 1] = t1;
          B[k + 4][j + 2] = t2;
          B[k + 4][j + 3] = t3;
          B[k][j + 4] = t4;
          B[k][j + 5] = t5;
          B[k][j + 6] = t6;
          B[k][j + 7] = t7;
        }
        for (int k = i + 4; k < i + 8; k++) {
          int t0 = A[j + 4][k];
          int t1 = A[j + 5][k];
          int t2 = A[j + 6][k];
          int t3 = A[j + 7][k];
          B[k][j + 4] = t0;
          B[k][j + 5] = t1;
          B[k][j + 6] = t2;
          B[k][j + 7] = t3;
        }
      }
    }
  } else {
    // Case 3
    for (int i = 0; i < N; i += 16) {
      for (int j = 0; j < M; j += 16) {
        for (int k = i; k < i + 16 && k < N; k++) {
          for (int l = j; l < j + 16 && l < M; l++) {
            B[l][k] = A[k][l];
          }
        }
      }
    }
  }
  return;
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
  int i, j, tmp;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; j++) {
      tmp = A[i][j];
      B[j][i] = tmp;
    }
  }
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions() {
  /* Register your solution function */
  registerTransFunction(transpose_submit, transpose_submit_desc);

  /* Register any additional transpose functions */
  registerTransFunction(trans, trans_desc);
}

/*
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
  int i, j;

  for (i = 0; i < N; i++) {
    for (j = 0; j < M; ++j) {
      if (A[i][j] != B[j][i]) {
        return 0;
      }
    }
  }
  return 1;
}
