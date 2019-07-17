#ifndef _CUBLAS_WRAPPER_H_
#define _CUBLAS_WRAPPER_H_

#include <cublas_v2.h>
#include <cuda_runtime.h>

#define gpuDeviceSynchronize cudaDeviceSynchronize
#define gpuFloatComplex cuComplex
#define gpuDoubleComplex cuDoubleComplex

#define gpublasHandle_t cublasHandle_t
#define gpublasCreate cublasCreate
#define gpublasDestroy cublasDestroy

#define gpuError_t cudaError_t
#define gpuSuccess cudaSuccess

#define gpublasOperation_t cublasOperation_t
#define GPUBLAS_OP_N CUBLAS_OP_N
#define GPUBLAS_OP_T CUBLAS_OP_T
#define GPUBLAS_OP_C CUBLAS_OP_C

#define gpuMemcpy2DAsync cudaMemcpy2DAsync
#define gpuMemcpyHostToDevice cudaMemcpyHostToDevice
#define gpuMemcpyDeviceToHost cudaMemcpyDeviceToHost

#define gpuFree cudaFree
#define gpuMalloc cudaMalloc

#define gpublasSgemm cublasSgemm
#define gpublasDgemm cublasDgemm
#define gpublasCgemm cublasCgemm
#define gpublasZgemm cublasZgemm

#endif // _CUBLAS_WRAPPER_H_
