#ifndef _TEST_UTILES_H_
#define _TEST_UTILES_H_

#include <hip/hip_runtime.h>
#include <rocblas.h>
#include <sys/time.h>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <cblas.h>

/*************************************************/
/**              timing function                **/
/*************************************************/
double get_time_us(void)
{
    hipDeviceSynchronize();
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 * 1000) + tv.tv_usec;
};

/*************************************************/
/**              predicate function             **/
/*************************************************/
template <typename T>
static constexpr bool is_complex = false;
template <>
static constexpr bool is_complex<rocblas_float_complex> = true;
template <>
static constexpr bool is_complex<rocblas_double_complex> = true;

/*************************************************/
/**              multiplication times function  **/
/*************************************************/
template <typename T, typename std::enable_if<!is_complex<T>>::type* = nullptr>
size_t multiplication_times(size_t m, size_t n, size_t k)
{
    return m * n * k;
}

template <typename T, typename std::enable_if<is_complex<T>>::type* = nullptr>
size_t multiplication_times(size_t m, size_t n, size_t k)
{
    // a+bi and c+di has (ac - bd) + (ad + bc)i
    // 4 multiplications
    return m * n * k * 4;
}

/*************************************************/
/**              transpos help function         **/
/*************************************************/
rocblas_operation char_to_rocblas_operation(char c)
{
    switch(c)
    {
    case 'n':
    case 'N':
        return rocblas_operation_none;
    case 't':
    case 'T':
        return rocblas_operation_transpose;
    case 'c':
    case 'C':
        return rocblas_operation_conjugate_transpose;
    default:
        return rocblas_operation_none;
    }
}

/*************************************************/
/**              difference function            **/
/*************************************************/
template <typename T, typename std::enable_if<!is_complex<T>>::type* = nullptr>
double distance(T a, T b)
{
    return (a < b) ? (b - a) : (a - b);
}

template <typename T, typename std::enable_if<is_complex<T>>::type* = nullptr>
double distance(T a, T b)
{
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

/*************************************************/
/**              predicate function             **/
/*************************************************/
template <typename T, typename std::enable_if<!is_complex<T>>::type* = nullptr>
inline T string_to_rocblas_datavalue(const std::string& value)
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
inline T string_to_rocblas_datavalue(const std::string& value)
{
    std::string::size_type sz;
    T                      res{};

    if(value == "")
        return res;

    decltype(res.x) tmp = std::stod(value, &sz);

    if(value[sz] != 'i')
    {
        // real part
        res.x = tmp;

        // imagnary part
        if(sz != value.size())
        {
            if(value[sz] != '+' && value[sz] != '-')
                throw std::invalid_argument("Invalid Complex Value");

            std::string::size_type sz2;
            res.y = std::stod(value.substr(sz), &sz2);
            sz += sz2;

            if(value[sz] != 'i')
                throw std::invalid_argument("Invalid Complex Value");
            sz++;
        }
    }
    else
    {
        // only imagnary case
        res.y = tmp;
        sz++;
    }

    if(sz != value.size())
        throw std::invalid_argument("Invalid Complex Value");

    return res;
}

/*************************************************/
/**              copy function                  **/
/*************************************************/
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

/*************************************************/
/**              rocblas handle wrapper         **/
/*************************************************/
class rocblas_local_handle
{
    rocblas_handle handle;

public:
    rocblas_local_handle()
    {
        rocblas_create_handle(&handle);
    }
    ~rocblas_local_handle()
    {
        rocblas_destroy_handle(handle);
    }

    // Allow rocblas_local_handle to be used anywhere rocblas_handle is expected
    operator rocblas_handle&()
    {
        return handle;
    }
    operator const rocblas_handle&() const
    {
        return handle;
    }
};

/*************************************************/
/**              gemm function wrapper          **/
/*************************************************/
template <typename T>
rocblas_status (*rocblas_gemm)(rocblas_handle    handle,
                               rocblas_operation transA,
                               rocblas_operation transB,
                               rocblas_int       m,
                               rocblas_int       n,
                               rocblas_int       k,
                               const T*          alpha,
                               const T*          A,
                               rocblas_int       lda,
                               const T*          B,
                               rocblas_int       ldb,
                               const T*          beta,
                               T*                C,
                               rocblas_int       ldc);
template <>
static constexpr auto rocblas_gemm<float> = rocblas_sgemm;
template <>
static constexpr auto rocblas_gemm<double> = rocblas_dgemm;
template <>
static constexpr auto rocblas_gemm<rocblas_float_complex> = rocblas_cgemm;
template <>
static constexpr auto rocblas_gemm<rocblas_double_complex> = rocblas_zgemm;

/*************************************************/
/**              gemm function wrapper          **/
/*************************************************/
template <typename T>
void cblas_gemm(const CBLAS_ORDER Order,
                const CBLAS_TRANSPOSE TransA,
                const CBLAS_TRANSPOSE TransB,
                const int M,
                const int N,
                const int K,
                const T *alpha,
                const T *A,
                const int lda,
                const T *B,
                const int ldb,
                const T *beta,
                T *C,
                const int ldc)
{
    std::cout << "unsupport type " << typeid(T).name() << std::endl;
}

template <>
void cblas_gemm(const CBLAS_ORDER Order,
                const CBLAS_TRANSPOSE TransA,
                const CBLAS_TRANSPOSE TransB,
                const int M,
                const int N,
                const int K,
                const float *alpha,
                const float *A,
                const int lda,
                const float *B,
                const int ldb,
                const float *beta,
                float *C,
                const int ldc)
{
    cblas_sgemm(Order, TransA, TransB, M, N, K, *alpha, A, lda, B, ldb, *beta, C, ldc);
}

template <>
void cblas_gemm(const CBLAS_ORDER Order,
                const CBLAS_TRANSPOSE TransA,
                const CBLAS_TRANSPOSE TransB,
                const int M,
                const int N,
                const int K,
                const double *alpha,
                const double *A,
                const int lda,
                const double *B,
                const int ldb,
                const double *beta,
                double *C,
                const int ldc)
{
    cblas_dgemm(Order, TransA, TransB, M, N, K, *alpha, A, lda, B, ldb, *beta, C, ldc);
}

template <>
void cblas_gemm(const CBLAS_ORDER Order,
                const CBLAS_TRANSPOSE TransA,
                const CBLAS_TRANSPOSE TransB,
                const int M,
                const int N,
                const int K,
                const rocblas_float_complex *alpha,
                const rocblas_float_complex *A,
                const int lda,
                const rocblas_float_complex *B,
                const int ldb,
                const rocblas_float_complex *beta,
                rocblas_float_complex *C,
                const int ldc)
{
    cblas_cgemm(Order, TransA, TransB, M, N, K, (void*)alpha, (void*)A, lda, (void*)B, ldb, (void*)beta, (void*)C, ldc);
}

template <>
void cblas_gemm(const CBLAS_ORDER Order,
                const CBLAS_TRANSPOSE TransA,
                const CBLAS_TRANSPOSE TransB,
                const int M,
                const int N,
                const int K,
                const rocblas_double_complex *alpha,
                const rocblas_double_complex *A,
                const int lda,
                const rocblas_double_complex *B,
                const int ldb,
                const rocblas_double_complex *beta,
                rocblas_double_complex *C,
                const int ldc)
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
