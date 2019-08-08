#ifndef _TEST_UTILES_H_
#define _TEST_UTILES_H_

#include <sys/time.h>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <typeinfo>

#include <cblas.h>

#if USE_CUBLAS
#include "cublas_wrapper.h"
#elif USE_ROCBLAS
#include "rocblas_wrapper.h"
#endif

/*************************************************/
/**              timing function                **/
/*************************************************/
double get_time_us(void)
{
    gpuDeviceSynchronize();
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 * 1000) + tv.tv_usec;
};

/*************************************************/
/**              output function function       **/
/*************************************************/
template <typename T, typename std::enable_if<is_complex<T>>::type* = nullptr>
std::ostream& operator<<(std::ostream& out, const T& val)
{
    if (val.y >= 0)
        out << std::real(val) << "+" << std::imag(val) << "i";
    else
        out << std::real(val) << std::imag(val) << "i";
    return out;
}

/*************************************************/
/**              multiplication times function  **/
/*************************************************/
template <typename T, typename std::enable_if<!is_complex<T>>::type* = nullptr>
inline size_t multiplication_times(size_t m, size_t n, size_t k)
{
    return m * n * k * 2;
}

template <typename T, typename std::enable_if<is_complex<T>>::type* = nullptr>
inline size_t multiplication_times(size_t m, size_t n, size_t k)
{
    // a+bi and c+di has (ac - bd) + (ad + bc)i
    // 4 multiplications
    return m * n * k * 2 * 4;
}

/*************************************************/
/**              transpos help function         **/
/*************************************************/
gpublasOperation_t char_to_target_operation(char c)
{
    switch(c)
    {
    case 'n':
    case 'N':
        return GPUBLAS_OP_N;
    case 't':
    case 'T':
        return GPUBLAS_OP_T;
    case 'c':
    case 'C':
        return GPUBLAS_OP_C;
    default:
        return GPUBLAS_OP_N;
    }
}

/*************************************************/
/**              transpos help function         **/
/*************************************************/
CBLAS_TRANSPOSE char_to_cblas_operation(char c)
{
    switch(c)
    {
    case 'n':
    case 'N':
        return CblasNoTrans;
    case 't':
    case 'T':
        return CblasTrans;
    case 'c':
    case 'C':
        return CblasConjTrans;
    default:
        return CblasNoTrans;
    }
}

/*************************************************/
/**              difference function            **/
/*************************************************/
template <typename T, typename std::enable_if<!is_complex<T>>::type* = nullptr>
inline double distance(T a, T b)
{
    return (a < b) ? (b - a) : (a - b);
}

template <typename T, typename std::enable_if<is_complex<T>>::type* = nullptr>
inline double distance(T a, T b)
{
    return sqrt((std::real(a) - std::real(b)) * (std::real(a) - std::real(b))
                + (std::imag(a) - std::imag(b)) * (std::imag(a) - std::imag(b)));
}

/*************************************************/
/**              predicate function             **/
/*************************************************/
template <typename T, typename std::enable_if<!is_complex<T>>::type* = nullptr>
inline T string_to_datavalue(const std::string& value)
{
    if(value == "")
        return 0;

    std::string::size_type sz;
    T                      res = std::stod(value, &sz);

    if(sz != value.size())
        throw std::invalid_argument("Invalid Complex Value");

    return res;
}

template <typename T, typename std::enable_if<is_complex<T>>::type* = nullptr>
inline T string_to_datavalue(const std::string& value)
{
    std::string::size_type sz;
    decltype(std::real(T{})) re = 0.0;
    decltype(std::real(T{})) im = 0.0;

    if(value == "")
        return T{re, im};

    decltype(std::real(T{})) tmp = std::stod(value, &sz);

    if(value[sz] != 'i')
    {
        // real part
        re = tmp;

        // imagnary part
        if(sz != value.size())
        {
            if(value[sz] != '+' && value[sz] != '-')
                throw std::invalid_argument("Invalid Complex Value");

            std::string::size_type sz2;
            im = std::stod(value.substr(sz), &sz2);
            sz += sz2;

            if(value[sz] != 'i')
                throw std::invalid_argument("Invalid Complex Value");
            sz++;
        }
    }
    else
    {
        // only imagnary case
        im = tmp;
        sz++;
    }

    if(sz != value.size())
        throw std::invalid_argument("Invalid Complex Value");

    return T{re, im};
}

/*************************************************/
/**              copy function                  **/
/*************************************************/
int SetMatrix(size_t rows, size_t cols, size_t elemSize, const void *A, size_t lda, void *B, size_t ldb)
{
    gpuError_t res;
    res = gpuMemcpy2DAsync(B, ldb * elemSize, A, lda * elemSize, rows * elemSize, cols, gpuMemcpyHostToDevice);
    if(res != gpuSuccess)
        return -1;
    return 0;
}

int GetMatrix(size_t rows, size_t cols, size_t elemSize, const void *A, size_t lda, void *B, size_t ldb)
{
    gpuError_t res;
    res = gpuMemcpy2DAsync(B, ldb * elemSize, A, lda * elemSize, rows * elemSize, cols, gpuMemcpyDeviceToHost);
    if(res != gpuSuccess) return -1;
    return 0;
}

/*************************************************/
/**              rocblas handle wrapper         **/
/*************************************************/
class gpublas_handle
{
    gpublasHandle_t handle;

public:
    gpublas_handle()
    {
        gpublasCreate(&handle);
    }
    ~gpublas_handle()
    {
        gpublasDestroy(handle);
    }

    // Allow rocblas_local_handle to be used anywhere rocblas_handle is expected
    operator gpublasHandle_t&()
    {
        return handle;
    }
    operator const gpublasHandle_t&() const
    {
        return handle;
    }
};

/*************************************************/
/**              gemm function wrapper          **/
/*************************************************/
template <typename T>
void gpublas_gemm(gpublasHandle_t    handle,
                  gpublasOperation_t transA,
                  gpublasOperation_t transB,
                  int               m,
                  int               n,
                  int               k,
                  const T*          alpha,
                  const T*          A,
                  int               lda,
                  const T*          B,
                  int               ldb,
                  const T*          beta,
                  T*                C,
                  int               ldc)
{
    std::cout << "unsupport type " << typeid(T).name() << std::endl;
};

template <>
void gpublas_gemm(gpublasHandle_t    handle,
                  gpublasOperation_t transA,
                  gpublasOperation_t transB,
                  int                m,
                  int                n,
                  int                k,
                  const float*       alpha,
                  const float*       A,
                  int                lda,
                  const float*       B,
                  int                ldb,
                  const float*       beta,
                  float*             C,
                  int                ldc)
{
    gpublasSgemm(handle, transA, transB, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
}

template <>
void gpublas_gemm(gpublasHandle_t    handle,
                  gpublasOperation_t transA,
                  gpublasOperation_t transB,
                  int                m,
                  int                n,
                  int                k,
                  const double*      alpha,
                  const double*      A,
                  int                lda,
                  const double*      B,
                  int                ldb,
                  const double*      beta,
                  double*            C,
                  int                ldc)
{
    gpublasDgemm(handle, transA, transB, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
}

template <>
void gpublas_gemm(gpublasHandle_t        handle,
                  gpublasOperation_t     transA,
                  gpublasOperation_t     transB,
                  int                    m,
                  int                    n,
                  int                    k,
                  const gpuFloatComplex* alpha,
                  const gpuFloatComplex* A,
                  int                    lda,
                  const gpuFloatComplex* B,
                  int                    ldb,
                  const gpuFloatComplex* beta,
                  gpuFloatComplex*       C,
                  int                    ldc)
{
    gpublasCgemm(handle, transA, transB, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
}

template <>
void gpublas_gemm(gpublasHandle_t         handle,
                  gpublasOperation_t      transA,
                  gpublasOperation_t      transB,
                  int                     m,
                  int                     n,
                  int                     k,
                  const gpuDoubleComplex* alpha,
                  const gpuDoubleComplex* A,
                  int                     lda,
                  const gpuDoubleComplex* B,
                  int                     ldb,
                  const gpuDoubleComplex* beta,
                  gpuDoubleComplex*       C,
                  int                     ldc)
{
    gpublasZgemm(handle, transA, transB, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
}

/*************************************************/
/**              gemm function wrapper          **/
/*************************************************/
template <typename T>
void cblas_gemm(const CBLAS_ORDER     Order,
                const CBLAS_TRANSPOSE TransA,
                const CBLAS_TRANSPOSE TransB,
                const int             M,
                const int             N,
                const int             K,
                const T*              alpha,
                const T*              A,
                const int             lda,
                const T*              B,
                const int             ldb,
                const T*              beta,
                T *                   C,
                const int             ldc)
{
    std::cout << "unsupport type " << typeid(T).name() << std::endl;
}

template <>
void cblas_gemm(const CBLAS_ORDER     Order,
                const CBLAS_TRANSPOSE TransA,
                const CBLAS_TRANSPOSE TransB,
                const int             M,
                const int             N,
                const int             K,
                const float*          alpha,
                const float*          A,
                const int             lda,
                const float*          B,
                const int             ldb,
                const float*          beta,
                float*                C,
                const int             ldc)
{
    cblas_sgemm(Order, TransA, TransB, M, N, K, *alpha, A, lda, B, ldb, *beta, C, ldc);
}

template <>
void cblas_gemm(const CBLAS_ORDER     Order,
                const CBLAS_TRANSPOSE TransA,
                const CBLAS_TRANSPOSE TransB,
                const int             M,
                const int             N,
                const int             K,
                const double*         alpha,
                const double*         A,
                const int             lda,
                const double*         B,
                const int             ldb,
                const double*         beta,
                double*               C,
                const int             ldc)
{
    cblas_dgemm(Order, TransA, TransB, M, N, K, *alpha, A, lda, B, ldb, *beta, C, ldc);
}

template <>
void cblas_gemm(const CBLAS_ORDER     Order,
                const CBLAS_TRANSPOSE TransA,
                const CBLAS_TRANSPOSE TransB,
                const int             M,
                const int             N,
                const int             K,
                const gpuFloatComplex*      alpha,
                const gpuFloatComplex*      A,
                const int             lda,
                const gpuFloatComplex*      B,
                const int             ldb,
                const gpuFloatComplex*      beta,
                gpuFloatComplex*            C,
                const int             ldc)
{
    cblas_cgemm(Order, TransA, TransB, M, N, K, (void*)alpha, (void*)A, lda, (void*)B, ldb, (void*)beta, (void*)C, ldc);
}

template <>
void cblas_gemm(const CBLAS_ORDER      Order,
                const CBLAS_TRANSPOSE  TransA,
                const CBLAS_TRANSPOSE  TransB,
                const int              M,
                const int              N,
                const int              K,
                const gpuDoubleComplex* alpha,
                const gpuDoubleComplex* A,
                const int              lda,
                const gpuDoubleComplex* B,
                const int              ldb,
                const gpuDoubleComplex* beta,
                gpuDoubleComplex*       C,
                const int              ldc)
{
    cblas_zgemm(Order, TransA, TransB, M, N, K, (void*)alpha, (void*)A, lda, (void*)B, ldb, (void*)beta, (void*)C, ldc);
}


/*************************************************/
/**              dump matrix function           **/
/*************************************************/
template <typename T>
void dump_matrix(const T& data, size_t row, size_t col, size_t ld)
{
    for(int r=0; r<row; r++)
    {
        for(int c=0; c<col; c++)
        {
            std::cout << data[c * ld + r] << ", ";
        }
        std::cout << std::endl;
    }
} 

#endif
