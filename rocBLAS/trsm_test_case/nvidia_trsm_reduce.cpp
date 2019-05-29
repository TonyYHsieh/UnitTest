/*
 * -- Heterogeneous High Performance Computing Linpack Benchmark (HHPL)
 *    Xianzhi Yu
 *    Institute of Computing Technology, Chinese Academy of Sciences
 *    High Performance Computing Center
 *    (C) Copyright 2018 All Rights Reserved
 *
 * -- Copyright notice and Licensing terms:
 *
 * Redistribution  and  use in  source and binary forms, with or without
 * modification, are  permitted provided  that the following  conditions
 * are met:
 *
 * 1. Redistributions  of  source  code  must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce  the above copyright
 * notice, this list of conditions,  and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. All  advertising  materials  mentioning  features  or  use of this
 * software must display the following acknowledgement:
 * This  product  includes  software  developed  at  the   Institute  of
 * Computing Technology, Chinese Academy of Sciences,  High  Performance
 * Computing Center.
 *
 * 4. The name of the  University,  the name of the  Laboratory,  or the
 * names  of  its  contributors  may  not  be used to endorse or promote
 * products  derived   from   this  software  without  specific  written
 * permission.
 *
 * -- Disclaimer:
 *
 * THIS  SOFTWARE  IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,  INCLUDING,  BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE UNIVERSITY
 * OR  CONTRIBUTORS  BE  LIABLE FOR ANY  DIRECT,  INDIRECT,  INCIDENTAL,
 * SPECIAL,  EXEMPLARY,  OR  CONSEQUENTIAL DAMAGES  (INCLUDING,  BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA OR PROFITS; OR BUSINESS INTERRUPTION)  HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT,  STRICT LIABILITY,  OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ---------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cuda_runtime.h>
#include <cublas_v2.h>
#include <assert.h>

#define PAGE_LOCK
#define OUT_CSV
#define ACCURACY_CHECK

#ifdef ACCURACY_CHECK
#include <mkl_cblas.h>

CBLAS_SIDE getSide_cpu(char side){
    if(side == 'L' || side == 'l')
        return CblasLeft;
    else
        return CblasRight;
}
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
cublasSideMode_t getSide_gpu(char side){
    if(side == 'L' || side == 'l')
        return CUBLAS_SIDE_LEFT;
    else
        return CUBLAS_SIDE_RIGHT;
}
cublasFillMode_t getUplo_gpu(char uplo){
    if(uplo == 'U' || uplo == 'u')
        return CUBLAS_FILL_MODE_UPPER;
    else
        return CUBLAS_FILL_MODE_LOWER;
}
cublasOperation_t getTranspose_gpu(char trans){
    if(trans == 'N' || trans == 'n')
        return CUBLAS_OP_N;
    else
        return CUBLAS_OP_T;
}
cublasDiagType_t getDiag_gpu(char diag){
    if(diag == 'U' || diag == 'u')
        return CUBLAS_DIAG_UNIT;
    else
        return CUBLAS_DIAG_NON_UNIT;
}

int main(int argc, char* argv[])
{
    // column major
    int iter_num = atoi(argv[1]);
    // Linear dimension of matrices
    char side  = argv[2][0];
    char uplo  = argv[2][1];
    char trans = argv[2][2];
    char diag  = argv[2][3];
    size_t nb    = atoi(argv[3]);
    size_t start = atoi(argv[4]);
    size_t end   = atoi(argv[5]);
    size_t lda, ldb, ldc;
    size_t rows_a, rows_b;
    size_t cols_a, cols_b;
    lda = rows_a = cols_a = nb;
    size_t m = nb, current;
    if(trans == 'N'){
        ldb = nb;
        rows_b = nb;
        cols_b = end;
    }
    else{
        ldb = end;
        rows_b = end;
        cols_b = nb;
    }

    double alpha = 1.0;
    double EPSILON = 0.0001;

    // Allocate host storage for A,B square matrices
    double *A, *B, *BB;
#ifdef PAGE_LOCK
    cudaMallocHost((void **)&A,  lda * cols_a * sizeof(double));
    cudaMallocHost((void **)&B,  ldb * cols_b * sizeof(double));
    cudaMallocHost((void **)&BB, ldb * cols_b * sizeof(double));
#else
    A  = (double*)malloc(lda * cols_a * sizeof(double));
    B  = (double*)malloc(ldb * cols_b * sizeof(double));
    BB = (double*)malloc(ldb * cols_b * sizeof(double));
#endif
    // Allocate device storage for A,B
    double *d_A, *d_B;
    cudaMalloc((void**)&d_A, lda * cols_a * sizeof(double));
    cudaMalloc((void**)&d_B, ldb * cols_b * sizeof(double));

    // Matrices are arranged column major
    size_t i, j, index;
    for(j=0; j<cols_a; j++) {
        if((uplo == 'L' || uplo == 'l') && (trans == 'N' || trans == 'n') || \
           (uplo == 'U' || uplo == 'u') && (trans == 'T' || trans == 't'))
            for(i=0; i<=j; i++) {
                index = j * lda + i;
                if(diag == 'U' || diag == 'u')
                    A[index] = 1;
                else
                    A[index] = sin(index);
            }
        else
            for(i=j; i<rows_a; i++) {
                index = j * lda + i;
                if(diag == 'U' || diag == 'u')
                    A[index] = 1;
                else
                    A[index] = sin(index);
            }
    }
    for(j=0; j<cols_b; j++) {
        for(i=0; i<rows_b; i++) {
            index = j * ldb + i;
            B[index] = cos(index);
        }
    }

    // Create cublas instance
    cublasHandle_t handle;
    cublasCreate(&handle);
    cudaEvent_t time0, time1, time2, time3;
    cudaEventCreate(&time0);
    cudaEventCreate(&time1);
    cudaEventCreate(&time2);
    cudaEventCreate(&time3);

    for(current=start; current<=end; current+=nb){
        float eventTimer = 0.0f;
        double time_stage1 = 0.0, time_stage2 = 0.0, time_stage3 = 0.0;
        int rows, cols;
        if(side == 'L' || side == 'l')
        {
            rows = m;
            cols = current;
        }
        else{
            rows = current;
            cols = m;
        }
        for(index = 0; index<iter_num; index++){
            // Set input matrices on device
            cudaEventRecord(time0, NULL);
            cublasSetMatrix(rows_a, cols_a, sizeof(double), A, lda, d_A, lda);
            if(trans == 'N' || trans == 'n')
                cublasSetMatrix(rows_b, current, sizeof(double), B, ldb, d_B, ldb);
            else
                cublasSetMatrix(current, cols_b, sizeof(double), B, ldb, d_B, ldb);
            cudaEventRecord(time1, NULL);
            cudaEventSynchronize(time1);
            cudaEventElapsedTime(&eventTimer, time0, time1);
            time_stage1 += eventTimer / 1000.0;

            cublasDtrsm(handle,
                    getSide_gpu(side),
                    getUplo_gpu(uplo),
                    getTranspose_gpu(trans),
                    getDiag_gpu(diag),
                    rows, cols,
                    &alpha,
                    d_A, lda,
                    d_B, ldb);

            cudaEventRecord(time2, NULL);
            cudaEventSynchronize(time2);
            cudaEventElapsedTime(&eventTimer, time1, time2);
            time_stage2 += eventTimer / 1000.0;
        
            // Retrieve result matrix from device
            if(trans == 'N' || trans == 'n')
                cublasGetMatrix(rows_b, current, sizeof(double), d_B, ldb, BB, ldb);
            else
                cublasGetMatrix(current, cols_b, sizeof(double), d_B, ldb, BB, ldb);
            cudaEventRecord(time3, NULL);
            cudaEventSynchronize(time3);
            cudaEventElapsedTime(&eventTimer, time2, time3);
            time_stage3 += eventTimer / 1000.0;
        }

        double trsm_perf = 1.0 * 1e-12 * m * m * current / (time_stage2 / iter_num);
        double copy_trsm_perf = 1.0 * 1e-12 * m * m * current / ((time_stage1 + time_stage2 + time_stage3) / iter_num);
#ifdef OUT_CSV
        printf("%c,%c,%c,%c,%ld,%ld,%.3lf\n",
               side, uplo, trans, diag,
               m, current,
               trsm_perf);
#else
        printf("DTRSM Performance: side %c uplo %c trans %c diag %c m %ld - n %ld - copy %.5lfs - trsm %.5lfs - copy %.5lfs"
               " - with_copy %.3lf TFLOPS - without_copy %.3lf TFLOPS\n",
               side, uplo, trans, diag,
               m, current,
               time_stage1/iter_num, time_stage2/iter_num, time_stage3/iter_num,
               copy_trsm_perf, trsm_perf);
#endif
#ifdef ACCURACY_CHECK
        cblas_dtrsm(CblasColMajor,
                getSide_cpu(side), getUplo_cpu(uplo),
                getTranspose_cpu(trans), getDiag_cpu(diag),
                rows, cols,
                alpha,
                A, lda,
                B, ldb);

        // Accuracy check
        size_t B_index;
        for(i=0; i<m; i++){
            for(j=0; j<current; j++){
                if(trans == 'N' || trans == 'n')
                    B_index = j * ldb + index;
                else
                    B_index = index * ldb + j;
                assert(abs(B[B_index] - BB[B_index]) < EPSILON);
            }
        }
#endif
    }
    // Clean up resources
#ifdef PAGE_LOCK
    cudaFreeHost(A);
    cudaFreeHost(B);
    cudaFreeHost(BB);
#else
    free(A);
    free(B);
    free(BB);
#endif
    cudaFree(d_A);
    cudaFree(d_B);
    cublasDestroy(handle);
    cudaEventDestroy(time0);
    cudaEventDestroy(time1);
    cudaEventDestroy(time2);
    cudaEventDestroy(time3);

    return 0;
}

