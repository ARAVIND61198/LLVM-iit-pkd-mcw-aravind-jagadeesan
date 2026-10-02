#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

static int N   = 128;
static int BLK = 32;

static double now_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1.0e6;
}

static void mm_naive(const double *A, const double *B, double *C) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            double s = 0.0;
            for (int k = 0; k < N; k++)
                s += A[i * N + k] * B[k * N + j];
            C[i * N + j] = s;
        }
}

static void mm_interchange(const double *A, const double *B, double *C) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) C[i * N + j] = 0.0;
        for (int k = 0; k < N; k++) {
            double a = A[i * N + k];
            for (int j = 0; j < N; j++)
                C[i * N + j] += a * B[k * N + j];
        }
    }
}

static void mm_tiled(const double *A, const double *B, double *C) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) C[i * N + j] = 0.0;

    for (int ii = 0; ii < N; ii += BLK)
        for (int kk = 0; kk < N; kk += BLK)
            for (int jj = 0; jj < N; jj += BLK) {
                int iMax = (ii + BLK < N) ? ii + BLK : N;
                int kMax = (kk + BLK < N) ? kk + BLK : N;
                int jMax = (jj + BLK < N) ? jj + BLK : N;
                for (int i = ii; i < iMax; i++)
                    for (int k = kk; k < kMax; k++) {
                        double a = A[i * N + k];
                        for (int j = jj; j < jMax; j++)
                            C[i * N + j] += a * B[k * N + j];
                    }
            }
}

static void mm_tiled_unroll(const double *A, const double *B, double *C) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) C[i * N + j] = 0.0;

    for (int ii = 0; ii < N; ii += BLK)
        for (int kk = 0; kk < N; kk += BLK)
            for (int jj = 0; jj < N; jj += BLK) {
                int iMax = (ii + BLK < N) ? ii + BLK : N;
                int kMax = (kk + BLK < N) ? kk + BLK : N;
                int jMax = (jj + BLK < N) ? jj + BLK : N;
                for (int i = ii; i < iMax; i++)
                    for (int k = kk; k < kMax; k++) {
                        double a = A[i * N + k];
                        const double *Brow = &B[k * N];
                        double *Crow = &C[i * N];
                        int j = jj;
                        int jEnd4 = jMax - ((jMax - jj) & 3);
                        for (; j < jEnd4; j += 4) {
                            Crow[j]     += a * Brow[j];
                            Crow[j + 1] += a * Brow[j + 1];
                            Crow[j + 2] += a * Brow[j + 2];
                            Crow[j + 3] += a * Brow[j + 3];
                        }
                        for (; j < jMax; j++)
                            Crow[j] += a * Brow[j];
                    }
            }
}

static double checksum(const double *C) {
    double s = 0.0;
    for (int i = 0; i < N * N; i++) s += C[i];
    return s;
}

typedef void (*kernel_fn)(const double *, const double *, double *);

static double run_kernel(const char *name, kernel_fn fn,
                         const double *A, const double *B, double *C,
                         double baseline_ms, double ref_sum, int have_ref) {
    memset(C, 0, (size_t)N * N * sizeof(double));
    double t0 = now_ms();
    fn(A, B, C);
    double t1 = now_ms();
    double ms = t1 - t0;

    double gflops = (2.0 * N * N * N) / (ms / 1000.0) / 1.0e9;
    double sum = checksum(C);
    const char *check;
    if (!have_ref)                         check = "ref";
    else if (fabs(sum - ref_sum) < 1e-6 * fabs(ref_sum) + 1e-9) check = "OK";
    else                                   check = "FAIL";

    double speedup = (baseline_ms > 0.0) ? baseline_ms / ms : 1.0;
    printf("  %-16s %10.3f %9.2f %8.2fx    %s\n",
           name, ms, gflops, speedup, check);
    return ms;
}

int main(int argc, char **argv) {
    if (argc > 1) N   = atoi(argv[1]);
    if (argc > 2) BLK = atoi(argv[2]);
    if (N <= 0)   N = 128;
    if (BLK <= 0) BLK = 32;

    size_t bytes = (size_t)N * N * sizeof(double);
    double *A = (double *)malloc(bytes);
    double *B = (double *)malloc(bytes);
    double *C = (double *)malloc(bytes);
    if (!A || !B || !C) { fprintf(stderr, "alloc failed\n"); return 1; }

    for (int i = 0; i < N * N; i++) {
        A[i] = (double)((i * 7 + 3) % 13) * 0.5;
        B[i] = (double)((i * 5 + 1) % 11) * 0.25;
    }

    printf("Matrix multiply  N=%d  BLK=%d  (%.2f MFLOP/run)\n",
           N, BLK, 2.0 * N * N * N / 1.0e6);
    printf("  %-16s %10s %9s %9s    %s\n",
           "kernel", "time(ms)", "GFLOP/s", "speedup", "check");
    printf("  ------------------------------------------------------------\n");

    double base = run_kernel("naive", mm_naive, A, B, C, 0.0, 0.0, 0);
    double ref_sum = checksum(C);

    run_kernel("interchange",  mm_interchange,  A, B, C, base, ref_sum, 1);
    run_kernel("tiled",        mm_tiled,        A, B, C, base, ref_sum, 1);
    run_kernel("tiled+unroll", mm_tiled_unroll, A, B, C, base, ref_sum, 1);

    free(A); free(B); free(C);
    return 0;
}
