#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include <cstdlib>
#include <random>
#include <sys/time.h>

#include <cblas.h>
#include <CL/cl.h>

// #define DEBUG

struct ProblemConfig
{
    char  tA;
    char  tB;
    int   m;
    int   n;
    int   k;
    int   lda;
    int   ldb;
    int   ldc;
    int   ldd;
    int   iter;
    float alpha;
    float beta;
};

double get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return (tv.tv_sec * 1000 * 1000) + tv.tv_usec;
};

/*************************************************/
/**              random function                **/
/*************************************************/
std::random_device rd;
std::mt19937 rocblas_rng(rd());

template <typename T>
inline T random_generator()
{
//    return 1.0f;
    return std::uniform_int_distribution<int>(1, 10)(rocblas_rng);
}

template <typename T>
void init_buffer(std::vector<T>& buf)
{
    for(int i=0; i<buf.size(); i++)
    {
        buf[i] = random_generator<T>();
    }
}

/*************************************************/
/**          CBlas operation wrapper            **/
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
/**          Dump matrix content                **/
/*************************************************/
void dump_matrix(const char* title, std::vector<float> data, int m, int n, int ld)
{
    std::cout << std::endl << "----- " << title << " -----" << std::endl;
    for(int i=0; i<m; i++)
    {
        for(int j=0; j<n; j++)
        {
            std::cout << std::setw(4) << data[i + j * ld] << ",";
        }
        std::cout << std::endl;
    }
}

/*************************************************/
/**             CL status check                 **/
/*************************************************/
void checkCLError(cl_int err, const char* ptr)
{
    if (err != CL_SUCCESS)
    {
        std::cerr << ptr << " met cl error " << static_cast<int>(err) << std::endl;
        abort();
    }
}

int main(int argc, char** argv)
{
    // ./sgemm prec m n k tA tB lda ldb ldc ldd alpha beta it
    if (argc != 14)
    {
        std::cout << "./sgemm [prec] [m] [n] [k] [tA] [tB] [lda] [ldb] [ldc] [ldd] [alpha] [beta] [it]" << std::endl;
        return 0;
    }

    ProblemConfig problem;

    problem.m     = std::stoi(std::string{argv[2]});
    problem.n     = std::stoi(std::string{argv[3]});
    problem.k     = std::stoi(std::string{argv[4]});
    problem.tA    = argv[5][0];
    problem.tB    = argv[6][0];
    problem.lda   = std::stoi(std::string{argv[7]});
    problem.ldb   = std::stoi(std::string{argv[8]});
    problem.ldc   = std::stoi(std::string{argv[9]});
    problem.ldd   = std::stoi(std::string{argv[10]});
    problem.alpha = std::stof(std::string{argv[11]});
    problem.beta  = std::stof(std::string{argv[12]});
    problem.iter  = std::stoi(std::string{argv[13]});

    if (problem.tA != 't' || problem.tB != 'n')
    {
        std::cout << "only supprot tA(t) and tB(n) currently" << std::endl;
        return 0;
    }

#ifdef DEBUG
    std::cout << "problem.m     " << problem.m     << std::endl;
    std::cout << "problem.n     " << problem.n     << std::endl;
    std::cout << "problem.k     " << problem.k     << std::endl;
    std::cout << "problem.tA    " << problem.tA    << std::endl;
    std::cout << "problem.tB    " << problem.tB    << std::endl;
    std::cout << "problem.lda   " << problem.lda   << std::endl;
    std::cout << "problem.ldb   " << problem.ldb   << std::endl;
    std::cout << "problem.ldc   " << problem.ldc   << std::endl;
    std::cout << "problem.ldd   " << problem.ldd   << std::endl;
    std::cout << "problem.alpha " << problem.alpha << std::endl;
    std::cout << "problem.beta  " << problem.beta  << std::endl;
    std::cout << "problem.iter  " << problem.iter  << std::endl;
#endif

    std::vector<float> hA(problem.lda * problem.m);
    std::vector<float> hB(problem.ldb * problem.n);
    std::vector<float> hC(problem.ldc * problem.n);
    std::vector<float> hD_c(problem.ldd * problem.n);
    std::vector<float> hD_g(problem.ldd * problem.n);

    init_buffer(hA);
    init_buffer(hB);
    init_buffer(hC);
    hD_g = hD_c = hC;

    cblas_sgemm(CblasColMajor,
                char_to_cblas_operation(problem.tA),
                char_to_cblas_operation(problem.tB),
                problem.m,
                problem.n,
                problem.k,
                problem.alpha,
                hA.data(),
                problem.lda,
                hB.data(),
                problem.ldb,
                problem.beta,
                hD_c.data(),
                problem.ldd);

    /********************************************/
    /*           Create Environment             */
    /********************************************/
    cl_int         err = CL_SUCCESS;
    cl_platform_id platform;
    cl_device_id   device;

    // get cl platform
    err = clGetPlatformIDs(1, &platform, nullptr);
    checkCLError(err, "clGetPlatformIDs");

    // get cl device
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    checkCLError(err, "clGetDeviceIDs");

    // get cl context
    cl_context_properties context_props[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    cl_context context = clCreateContext(context_props, 1, &device, nullptr, nullptr, &err);
    checkCLError(err, "clCreateContext");

    // get device name
    char devName[1024];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(devName), devName, nullptr);
    // std::cout << "gpu device: " << devName << std::endl;

    // create command queue with profiler
    const cl_queue_properties queue_props[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
    cl_command_queue command_queue = clCreateCommandQueueWithProperties(context, device, queue_props, &err);
    checkCLError(err, "clCreateCommandQueueWithProperties");

    /********************************************/
    /*              Create Kernel               */
    /********************************************/
    // create progream from source
    std::ifstream clfile("sgemm.cl");
    std::string source((std::istreambuf_iterator<char>(clfile)), std::istreambuf_iterator<char>());
    size_t length = source.length();
    const char* sourceptr = source.c_str();
    cl_program program = clCreateProgramWithSource(context, 1, &sourceptr, &length, &err);
    checkCLError(err, "clCreateProgramWithSource");

    // build progream
    err = clBuildProgram(program, 1, &device, "-cl-std=CL2.0", nullptr, nullptr);
    if (err != CL_SUCCESS)
    {
        char buffer[10240];
        std::cerr << "clCreateProgramWithSource return" << err << std::endl;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, nullptr);
        std::cerr << "CL Compilation failed: " << buffer << std::endl;
        abort();
    }

    // build kernels
    cl_kernel kernel = clCreateKernel(program, "sgemm" , &err);
    checkCLError(err, "clCreateKernel");

    /********************************************/
    /*              Create Buffer               */
    /********************************************/
    // input buffer
    cl_mem dA  = clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(float) * problem.lda * problem.m, nullptr, &err);
    checkCLError(err, "clCreateBuffer");
    cl_mem dB  = clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(float) * problem.ldb * problem.n, nullptr, &err);
    checkCLError(err, "clCreateBuffer");
    cl_mem dC  = clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(float) * problem.ldc * problem.n, nullptr, &err);
    checkCLError(err, "clCreateBuffer");
    // output buffer
    cl_mem dD  = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * problem.ldd * problem.n, nullptr, &err);
    checkCLError(err, "clCreateBuffer");

    checkCLError(clEnqueueWriteBuffer(command_queue, dA, CL_TRUE, 0, sizeof(float) * problem.lda * problem.m, hA.data()  , 0, nullptr, nullptr), "clEnqueueWriteBuffer");
    checkCLError(clEnqueueWriteBuffer(command_queue, dB, CL_TRUE, 0, sizeof(float) * problem.ldb * problem.n, hB.data()  , 0, nullptr, nullptr), "clEnqueueWriteBuffer");
    checkCLError(clEnqueueWriteBuffer(command_queue, dC, CL_TRUE, 0, sizeof(float) * problem.ldc * problem.n, hC.data()  , 0, nullptr, nullptr), "clEnqueueWriteBuffer");
    checkCLError(clEnqueueWriteBuffer(command_queue, dD, CL_TRUE, 0, sizeof(float) * problem.ldd * problem.n, hD_g.data(), 0, nullptr, nullptr), "clEnqueueWriteBuffer");

    /********************************************/
    /*              Launch Kernel               */
    /********************************************/
    size_t local_0  = 8;
    size_t local_1  = 8;
    size_t macro_tile_0 = 32;
    size_t macro_tile_1 = 32;
    size_t global_0 = (problem.m + macro_tile_0 - 1) / macro_tile_0 * local_0;
    size_t global_1 = (problem.n + macro_tile_1 - 1) / macro_tile_1 * local_1;
    size_t local_work_size[3]  = {local_0,  local_1,  1};
    size_t global_work_size[3] = {global_0, global_1, 1};

    clFlush(command_queue);
    clFinish(command_queue);
    double st = get_time_us();

    for(int i=0; i<problem.iter; i++)
    {
        checkCLError(clSetKernelArg(kernel,  0, sizeof(cl_mem),   &dA),            "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel,  1, sizeof(cl_mem),   &dB),            "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel,  2, sizeof(cl_mem),   &dC),            "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel,  3, sizeof(cl_mem),   &dD),            "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel,  4, sizeof(cl_int),   &problem.m),     "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel,  5, sizeof(cl_int),   &problem.n),     "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel,  6, sizeof(cl_int),   &problem.k),     "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel,  7, sizeof(cl_int),   &problem.lda),   "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel,  8, sizeof(cl_int),   &problem.ldb),   "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel,  9, sizeof(cl_int),   &problem.ldc),   "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel, 10, sizeof(cl_int),   &problem.ldd),   "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel, 11, sizeof(cl_float), &problem.alpha), "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel, 12, sizeof(cl_float), &problem.beta),  "clSetKernelArg");

        err = clEnqueueNDRangeKernel(command_queue, kernel, 3, nullptr, global_work_size, local_work_size, 0, nullptr, nullptr);
        checkCLError(err, "clEnqueueNDRangeKernel");
    }

    clFlush(command_queue);
    clFinish(command_queue);
    double duration = (get_time_us() - st) * 1e-6;

    // get result back to host side
    clEnqueueReadBuffer(command_queue, dD, CL_TRUE, 0, sizeof(float) * problem.ldd * problem.n, hD_g.data(), 0, nullptr, nullptr);

    /********************************************/
    /*             Clean Up Resource            */
    /********************************************/
    // release memory object
    checkCLError(clReleaseMemObject(dA),  "clReleaseMemObject");
    checkCLError(clReleaseMemObject(dB),  "clReleaseMemObject");
    checkCLError(clReleaseMemObject(dC),  "clReleaseMemObject");
    checkCLError(clReleaseMemObject(dD),  "clReleaseMemObject");

    // release environment
    checkCLError(clReleaseKernel(kernel),              "clReleaseKernel");
    checkCLError(clReleaseProgram(program),            "clReleaseProgram");
    checkCLError(clReleaseCommandQueue(command_queue), "clReleaseCommandQueue");
    checkCLError(clReleaseContext(context),            "clReleaseContext");
    checkCLError(clReleaseDevice(device),              "clReleaseDevice");

    /********************************************/
    /*             Compare Result               */
    /********************************************/
#ifdef DEBUG
    dump_matrix("hA",   hA,   problem.k, problem.m, problem.lda);
    dump_matrix("hB",   hB,   problem.k, problem.n, problem.ldb);
    dump_matrix("hC",   hC,   problem.m, problem.n, problem.ldc);
    dump_matrix("hD_c", hD_c, problem.m, problem.n, problem.ldd);
    dump_matrix("hD_g", hD_g, problem.m, problem.n, problem.ldd);
#endif

    for(int j=0; j<problem.n; j++)
    {
        for(int i=0; i<problem.m; i++)
        {
            if(abs(hD_c[j * problem.ldd + i] - hD_g[j * problem.ldd + i]) > 0.001f)
            {
                std::cout << "[FAILED ]"
                          << "[" << std::setw(5) << duration << " ms]"
                          << " " << problem.m
                          << " " << problem.n
                          << " " << problem.k
                          << " " << problem.tA
                          << " " << problem.tB
                          << " " << problem.lda
                          << " " << problem.ldb
                          << " " << problem.ldc
                          << " " << problem.ldd
                          << " " << problem.alpha
                          << " " << problem.beta
                          << " " << problem.iter
                          << std::endl;
                return -1;
            }
        }
    }

    std::cout << "[SUCCESS] "
              << "[" << std::setw(5) << duration << " ms]"
              << " " << problem.m
              << " " << problem.n
              << " " << problem.k
              << " " << problem.tA
              << " " << problem.tB
              << " " << problem.lda
              << " " << problem.ldb
              << " " << problem.ldc
              << " " << problem.ldd
              << " " << problem.alpha
              << " " << problem.beta
              << " " << problem.iter
              << std::endl;

    return 0;
}

