#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <hip/hip_runtime.h>
#include <rocblas.h>
#include <assert.h>

// #define PAGE_LOCK
#define OUT_CSV
#ifdef ACCURACY_CHECK
#include <cblas.h>

CBLAS_UPLO getUplo_cpu(char uplo){
    if(uplo == 'U' || uplo == 'u')
        return CblasUpper;
    else
        return CblasLower;
}
CBLAS_TRANSPOSE getTranspose_cpu(char trans){
    if(trans == 'N' || trans == 'n')
        return CblasNoTrans;
    else
        return CblasTrans;
}
CBLAS_DIAG getDiag_cpu(char diag){
    if(diag == 'U' || diag == 'u')
        return CblasUnit;
    else
        return CblasNonUnit;
}
#endif

rocblas_fill getUplo_gpu(char uplo){
    if(uplo == 'U' || uplo == 'u')
        return rocblas_fill_upper;
    else
        return rocblas_fill_lower;
}
rocblas_operation getTranspose_gpu(char trans){
    if(trans == 'N' || trans == 'n')
        return rocblas_operation_none;
    else
        return rocblas_operation_transpose;

}
rocblas_diagonal getDiag_gpu(char diag){
    if(diag == 'U' || diag == 'u')
        return rocblas_diagonal_unit;
    else
        return rocblas_diagonal_non_unit;
}

int hipSetMatrix(size_t rows, size_t cols, size_t elemSize, const void *A, size_t lda, void *B, size_t ldb)
{
    hipError_t res;
    res = hipMemcpy2DAsync(B, ldb * elemSize, A, lda * elemSize, rows * elemSize, cols, hipMemcpyHostToDevice);
    if(res != hipSuccess)
        return -1;
    return 0;
}

int hipGetMatrix(size_t rows, size_t cols, size_t elemSize, const void *A, size_t lda, void *B, size_t ldb)
{
    hipError_t res;
    res = hipMemcpy2DAsync(B, ldb * elemSize, A, lda * elemSize, rows * elemSize, cols, hipMemcpyDeviceToHost);
    if(res != hipSuccess) return -1;
    return 0;
}

double abs_sub(double a, double b){
    return (a < b) ? (b - a) : (a - b);
}

template <typename T>
inline double abs(T a)
{
    return (a > 0) ? a : -a;
}

int main(int argc, char* argv[])
{
    // column major
    int iter_num = atoi(argv[1]);
    // Linear dimension of matrices
    char uplo  = argv[2][0];
    char trans = argv[2][1];
    char diag  = argv[2][2];

    size_t m    = atoi(argv[3]);
    size_t lda  = atoi(argv[4]);
    size_t n    = 1;
    int    incx = atoi(argv[5]);
    size_t ldb  = m * abs(incx);

    size_t rows_a = m;
    size_t cols_a = m;

    size_t rows_b = m;
    size_t cols_b = n;

    double EPSILON = 0.0001;

    // Allocate host storage for A,B square matrices
    double *A, *B, *C, *BB;
#ifdef PAGE_LOCK
    hipMallocHost((void **)&A,  lda * cols_a * sizeof(double));
    hipMallocHost((void **)&B,  ldb * cols_b * sizeof(double));
    hipMallocHost((void **)&C,  ldb * cols_b * sizeof(double));
    hipMallocHost((void **)&BB, ldb * cols_b * sizeof(double));
#else
    A  = (double*)malloc(lda * cols_a * sizeof(double));
    B  = (double*)malloc(ldb * cols_b * sizeof(double));
    C  = (double*)malloc(ldb * cols_b * sizeof(double));
    BB = (double*)malloc(ldb * cols_b * sizeof(double));
#endif
    // Allocate device storage for A,B
    double *d_A, *d_B;
    hipMalloc((void**)&d_A, lda * cols_a * sizeof(double));
    hipMalloc((void**)&d_B, ldb * cols_b * sizeof(double));

    // Matrices are arranged column major
//    memset(A, 0, lda * cols_a * sizeof(double));
    for(size_t j=0; j<cols_a; j++) {
        if(uplo == 'U' || uplo == 'u')
            for(size_t i=0; i<=j; i++) {
                size_t index = j * lda + i;
                if(diag == 'U' || diag == 'u')
                    A[index] = 1;
                else
                    A[index] = sin(index);
            }
        else
            for(size_t i=j; i<rows_a; i++) {
                size_t index = j * lda + i;
                if(diag == 'U' || diag == 'u')
                    A[index] = 1;
                else
                    A[index] = sin(index);
            }
    }

//    memset(B, 0, ldb * cols_b * sizeof(double));
    for(size_t j=0; j<cols_b; j++) {
        for(size_t i=0; i<rows_b; i++) {
            size_t index = j * ldb + i;
            B[index] = cos(index);
        }
    }

    // Create cublas instance
    rocblas_handle handle;
    rocblas_create_handle(&handle);
    hipEvent_t time0, time1, time2, time3;
    hipEventCreate(&time0);
    hipEventCreate(&time1);
    hipEventCreate(&time2);
    hipEventCreate(&time3);

    {
        float eventTimer = 0.0f;
        double time_stage1 = 0.0, time_stage2 = 0.0, time_stage3 = 0.0;

        for(int index = -2; index<iter_num; index++){
            // Set input matrices on device
            hipEventRecord(time0, NULL);
            hipSetMatrix(rows_a, cols_a, sizeof(double), A, lda, d_A, lda);
            hipSetMatrix(rows_b, cols_b, sizeof(double), B, ldb, d_B, ldb);
            hipEventRecord(time1, NULL);
            hipEventSynchronize(time1);
            hipEventElapsedTime(&eventTimer, time0, time1);
            if (index > 0)
                time_stage1 += eventTimer / 1000.0;

            rocblas_dtrsv(handle,
                    getUplo_gpu(uplo),
                    getTranspose_gpu(trans),
                    getDiag_gpu(diag),
                    m,
                    d_A,
                    lda,
                    d_B,
                    incx);

            hipEventRecord(time2, NULL);
            hipEventSynchronize(time2);
            hipEventElapsedTime(&eventTimer, time1, time2);
            if (index > 0)
                time_stage2 += eventTimer / 1000.0;
        
            // Retrieve result matrix from device
            hipGetMatrix(rows_b, cols_b, sizeof(double), d_B, ldb, BB, ldb);

            hipEventRecord(time3, NULL);
            hipEventSynchronize(time3);
            hipEventElapsedTime(&eventTimer, time2, time3);
            if (index > 0)
                time_stage3 += eventTimer / 1000.0;
        }

        double trsm_perf = 1.0 * 1e-12 * m * m * n / (time_stage2 / iter_num);
        double copy_trsm_perf = 1.0 * 1e-12 * m * m * n / ((time_stage1 + time_stage2 + time_stage3) / iter_num);
#ifdef OUT_CSV
        printf("%c,%c,%c,%ld,%ld,%ld,%d,%.8lf\n",
               uplo, trans, diag,
               m, lda, n, incx,
               trsm_perf);
#else
        printf("DTRSM Performance: uplo %c trans %c diag %c m %ld - lda %ld - n %ld - incx %d - copy %.5lfs - trsm %.5lfs - copy %.5lfs"
               " - with_copy %.3lf TFLOPS - without_copy %.3lf TFLOPS\n",
               uplo, trans, diag,
               m, lda, n, incx,
               time_stage1/iter_num, time_stage2/iter_num, time_stage3/iter_num,
               copy_trsm_perf, trsm_perf);
#endif
	
	fflush(stdout);

#ifdef ACCURACY_CHECK
        memcpy(C, B, ldb * cols_b * sizeof(double));
        cblas_dtrsv(CblasColMajor,
                getUplo_cpu(uplo),
                getTranspose_cpu(trans),
                getDiag_cpu(diag),
                rows,
                A,
                lda,
                C,
                incx);

        // Accuracy check
        for(size_t i=0; i<m; i++){
            for(size_t j=0; j<n; j++){
                size_t B_index = j * ldb + i;
                if(abs_sub(C[B_index], BB[B_index]) > EPSILON)
                    printf("diffn: %f, %f\n",C[B_index], BB[B_index]);
            }
        }
#endif
    }
    // Clean up resources
#ifdef PAGE_LOCK
    hipFreeHost(A);
    hipFreeHost(B);
    hipFreeHost(C);
    hipFreeHost(BB);
#else
    free(A);
    free(B);
    free(C);
    free(BB);
#endif
    hipFree(d_A);
    hipFree(d_B);
    rocblas_destroy_handle(handle);
    hipEventDestroy(time0);
    hipEventDestroy(time1);
    hipEventDestroy(time2);
    hipEventDestroy(time3);

    return 0;
}

