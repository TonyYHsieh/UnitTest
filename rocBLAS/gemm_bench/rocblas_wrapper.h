#ifndef PLATFORM_WRAPPER_H
#define PLATFORM_WRAPPER_H

#include <rocblas.h>
#include <hip/hip_runtime.h>

#define gpuDeviceSynchronize hipDeviceSynchronize
#define gpuFloatComplex rocblas_float_complex
#define gpuDoubleComplex rocblas_double_complex

#define gpublasHandle_t rocblas_handle
#define gpublasCreate rocblas_create_handle
#define gpublasDestroy rocblas_destroy_handle

#define gpuError_t hipError_t
#define gpuSuccess hipSuccess

#define gpublasOperation_t rocblas_operation
#define GPUBLAS_OP_N rocblas_operation_none
#define GPUBLAS_OP_T rocblas_operation_transpose
#define GPUBLAS_OP_C rocblas_operation_conjugate_transpose

#define gpuMemcpy2DAsync hipMemcpy2DAsync
#define gpuMemcpyHostToDevice hipMemcpyHostToDevice
#define gpuMemcpyDeviceToHost hipMemcpyDeviceToHost

#define gpuFree hipFree
#define gpuMalloc hipMalloc

#define gpublasSgemm rocblas_sgemm
#define gpublasDgemm rocblas_dgemm
#define gpublasCgemm rocblas_cgemm
#define gpublasZgemm rocblas_zgemm

#endif // PLATFORM_WRAPPER_H
