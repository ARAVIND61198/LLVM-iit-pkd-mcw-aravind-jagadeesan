#define N 128

void matmul(const double *A, const double *B, double *C) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            double s = 0.0;
            for (int k = 0; k < N; k++)
                s += A[i * N + k] * B[k * N + j];
            C[i * N + j] = s;
        }
}

void interchange_demo(double *restrict X, const double *restrict Y) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            X[j * N + i] = X[j * N + i] + Y[j * N + i];
}
