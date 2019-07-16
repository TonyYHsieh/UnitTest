#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <random>
#include <rocblas.h>
#include <hip/hip_runtime.h>
#include <assert.h>
#include <cblas.h>

#include "utils.h"

/*************************************************/
/**              random function                **/
/*************************************************/
std::random_device rd;
std::mt19937 rocblas_rng(rd());

/*! \brief  generate a random number in range [1,2,3,4,5,6,7,8,9,10] */
template <typename T, typename std::enable_if<!is_complex<T>>::type* = nullptr>
inline T random_generator()
{
    return std::uniform_int_distribution<int>(1, 10)(rocblas_rng);
}

template <typename T, typename std::enable_if<is_complex<T>>::type* = nullptr>
inline T random_generator()
{
    decltype(T::x) real = std::uniform_int_distribution<int>(1, 10)(rocblas_rng);
    decltype(T::y) imag = std::uniform_int_distribution<int>(1, 10)(rocblas_rng);
    return T{real, imag};
}

template <typename T>
void fill_matrix(std::vector<T>& data, size_t row, size_t col, size_t ld)
{
    for(size_t c=0; c<col; c++)
    {
        for(size_t r=0; r<row; r++)
        {
            data[c * ld + r] = random_generator<T>();
        }
    }
}

inline void free_device_memory(void* ptr)
{
    hipFree(ptr);
}

/*************************************************/
/**              test function                  **/
/*************************************************/
template<typename T>
void testing_gemm(size_t m, size_t n, size_t k,
                  rocblas_operation trans_a, rocblas_operation trans_b,
                  size_t lda, size_t ldb, size_t ldc,
                  const char* alpha_str, const char* beta_str,
                  int iter_num, double tolerance)
{
    /**** get dimension information *****/
    size_t row_a = ((trans_a == rocblas_operation_none) ? m : k);
    size_t col_a = ((trans_a == rocblas_operation_none) ? k : m);

    size_t row_b = ((trans_a == rocblas_operation_none) ? k : n);
    size_t col_b = ((trans_a == rocblas_operation_none) ? n : k);

    size_t row_c = m;
    size_t col_c = n;

    /**** transfrom alpha/beta *****/
    T alpha = string_to_rocblas_datavalue<T>(alpha_str);
    T beta  = string_to_rocblas_datavalue<T>(beta_str);

    /***** init host memory *****/
    std::vector<T> host_a(lda * col_a, T{});
    std::vector<T> host_b(ldb * col_b, T{});
    std::vector<T> host_c(ldc * col_c, T{});
    std::vector<T> host_dev_c(ldc * col_c, T{});

    // initial with random data
    fill_matrix(host_a, row_a, col_a, lda);
    fill_matrix(host_b, row_b, col_b, ldb);

    /***** init device memory *****/
    T* dev_tmp = nullptr;
    hipMalloc((void**)&dev_tmp, lda * col_a * sizeof(T));
    std::unique_ptr<T, decltype(&free_device_memory)> dev_a(dev_tmp, &free_device_memory);
    hipMalloc((void**)&dev_tmp, ldb * col_b * sizeof(T));
    std::unique_ptr<T, decltype(&free_device_memory)> dev_b(dev_tmp, &free_device_memory);
    hipMalloc((void**)&dev_tmp, ldc * col_c * sizeof(T));
    std::unique_ptr<T, decltype(&free_device_memory)> dev_c(dev_tmp, &free_device_memory);

    /**** rocblas *****/
    // copy host data to device
    double gpu_time = get_time_us();
    hipSetMatrix(row_a, col_a, sizeof(T), host_a.data(), lda, dev_a.get(), lda);
    hipSetMatrix(row_b, col_b, sizeof(T), host_b.data(), ldb, dev_b.get(), ldb);
    double mem_write_time = (get_time_us() - gpu_time) * 1e-6;

    rocblas_local_handle handle;

    // warm up GPU
    for(int i=0; i<2; i++)
    {
        rocblas_gemm<T>(handle, trans_a, trans_b, m, n, k, &alpha, dev_a.get(), lda, dev_b.get(), ldb, &beta, dev_c.get(), ldc);
    }

    // execute iteration
    gpu_time = get_time_us();
    for(int i=0; i<iter_num; i++)
    {
        rocblas_gemm<T>(handle, trans_a, trans_b, m, n, k, &alpha, dev_a.get(), lda, dev_b.get(), ldb, &beta, dev_c.get(), ldc);
    }
    double exe_time = (get_time_us() - gpu_time) * 1e-6;

    // read result back to host
    gpu_time = get_time_us();
    hipGetMatrix(row_c, col_c, sizeof(T), dev_c.get(), ldc, host_dev_c.data(), ldc);
    double mem_read_time = (get_time_us() - gpu_time) * 1e-6;

    double gemm_perf = multiplication_times<T>(m, n, k) * 1e-12 / (exe_time / iter_num);

    std::cout << m << " " << n << " " << k << " " << lda << " " << ldb << " " << ldc << " " << gemm_perf << std::endl;

    /***** cblas *****/
    cblas_gemm<T>(CblasColMajor,
                  static_cast<CBLAS_TRANSPOSE>(trans_a),
                  static_cast<CBLAS_TRANSPOSE>(trans_b),
                  m,
                  n,
                  k,
                  &alpha,
                  host_a.data(),
                  lda,
                  host_b.data(),
                  ldb,
                  &beta,
                  host_c.data(),
                  ldc);

    // Accuracy check
    for(size_t i=0; i<m; i++)
    {
        for(size_t j=0; j<n; j++)
        {
            size_t C_index = j * ldc + i;
            if(distance(host_dev_c[C_index], host_c[C_index]) > tolerance)
            {
                std::cout << "too big difference in (" << i << "," << j << ") host_dev_c "
                          << host_dev_c[C_index] << " host_c " << host_c[C_index] << std::endl;
            }
        }
    }

#ifdef DEBUG // for debug
    std::cout << std::endl << std::endl << "----- a -----" << std::endl;
    dump_matrix(host_a, row_a, col_a, lda);

    std::cout << std::endl << std::endl << "----- b -----" << std::endl;
    dump_matrix(host_b, row_b, col_b, ldb);

    std::cout << std::endl << std::endl << "----- hc -----" << std::endl;
    dump_matrix(host_c, row_c, col_c, ldc);

    std::cout << std::endl << std::endl << "----- dc -----" << std::endl;
    dump_matrix(host_dev_c, row_c, col_c, ldc);
#endif
}

int main(int argc, char* argv[])
{
    // ./gemm prec m n k t t lda ldb ldc alpha beta it
    if (argc != 13)
    {
        std::cout << "amd_complex_gemm [prec] [m] [n] [k] [tA] [tB] [lda] [ldb] [ldc] [alpha] [beta] [it]" << std::endl;
        return 0;
    }

    // column major
    char prec = argv[1][0];

    size_t m = atoi(argv[2]);
    size_t n = atoi(argv[3]);
    size_t k = atoi(argv[4]);

    rocblas_operation trans_a = char_to_rocblas_operation(argv[5][0]);
    rocblas_operation trans_b = char_to_rocblas_operation(argv[6][0]);

    size_t lda = atoi(argv[7]);
    size_t ldb = atoi(argv[8]);
    size_t ldc = atoi(argv[9]);

    const char* alpha = argv[10];
    const char* beta  = argv[11];

    int iter_num   = atoi(argv[12]);
    // Linear dimension of matrices

    constexpr double EPSILON = 0.0001;

    switch (prec)
    {
    case 's':
    case 'S':
        testing_gemm<float>(m, n, k, trans_a, trans_b, lda, ldb, ldc, alpha, beta, iter_num, EPSILON);
        break;
    case 'd':
    case 'D':
        testing_gemm<double>(m, n, k, trans_a, trans_b, lda, ldb, ldc, alpha, beta, iter_num, EPSILON);
        break;
    case 'c':
    case 'C':
        testing_gemm<rocblas_float_complex>(m, n, k, trans_a, trans_b, lda, ldb, ldc, alpha, beta, iter_num, EPSILON);
        break;
    case 'z':
    case 'Z':
        testing_gemm<rocblas_double_complex>(m, n, k, trans_a, trans_b, lda, ldb, ldc, alpha, beta, iter_num, EPSILON);
        break;
    }

    return 0;
}

