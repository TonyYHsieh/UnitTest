#include <iostream>
#include <iomanip>
#include <fstream>
#include <streambuf>
#include <cstring>
#include <random>
#include <sys/time.h>

#include <cblas.h>
#include <CL/cl.h>

#define MATRIX_SIZE 1024

#define DEBUG

/*************************************************/
/**              random function                **/
/*************************************************/
std::random_device rd;
std::mt19937 rocblas_rng(rd());

template <typename T>
inline T random_generator()
{
    return std::uniform_int_distribution<int>(1, 10)(rocblas_rng);
}

template <typename T>
void init_buffer_with_random(std::vector<T>& buf)
{
    for(int i=0; i<buf.size(); i++)
    {
        buf[i] = random_generator<T>();
    }
}

template <typename T>
void init_buffer_with_value(std::vector<T>& buf, T value)
{
    for(int i=0; i<buf.size(); i++)
    {
        buf[i] = value;
    }
}

void compare_result(float* cpu, float* gpu, size_t size)
{
    for(size_t i=0; i<size; i++)
    {
        if (abs(cpu[i] - gpu[i]) > 0.001f)
        {
            std::cout << "[FAILED ]" << std::endl;
            return;
        }
    }

    std::cout << "[SUCCESS] " << std::endl;
}

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

void krun(const void *a, int lda, const void *b, int ldb, void *d, int ldd)
{
    // Saved state to minimize calls to OpenCL runtime API
    // Assuming here that lda, ldb, and ldd are not going to change
    static struct
    {
        cl_context ctx;
        cl_device_id did;
        cl_command_queue cmd_queue;
        cl_program program;
        cl_kernel kernel;
        cl_mem a_buf;
        cl_mem b_buf;
        cl_mem d_buf;
    } saved;

    cl_platform_id plat_ids[8];
    cl_uint nplat, ndev;
    cl_context_properties cprops[9];
    cl_device_id dev_ids[16];
    cl_int err;
    char dstr[128];

    // Get the platform
    err = clGetPlatformIDs(8U, plat_ids, &nplat);
    if (err != CL_SUCCESS) {
        std::cout << "get platform ids failed, err=" << err << std::endl;
        exit(1);
    }

    if (!nplat) {
        fputs("no platforms available\n", stderr);
        exit(1);
    }

    err = clGetPlatformInfo(plat_ids[0], CL_PLATFORM_VENDOR, sizeof(dstr), dstr, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "failed to get platform info, err=" << err << std::endl;
        exit(1);
    }

    std::cout << "Selecting platform 0 (Vendor=" << dstr <<") of " << nplat << " available" << std::endl;

    if (strncmp(dstr, "Advanced Micro", 14)) {
        std::cout << "The vendor of the selected platform is not AMD" << std::endl;
        exit(1);
    }

    // create the OpenCL context on specified device
    cprops[0] = CL_CONTEXT_PLATFORM;
    cprops[1] = (cl_context_properties)plat_ids[0];
    cprops[2] = 0;
    saved.ctx = clCreateContextFromType(cprops, CL_DEVICE_TYPE_GPU, NULL, NULL, &err);
    if (saved.ctx == (cl_context)0) {
        std::cout << "create context failed, err=" << err << std::endl;
        exit(1);
    }

    // get a device
    err = clGetContextInfo(saved.ctx, CL_CONTEXT_NUM_DEVICES, sizeof(ndev), (void *)&ndev, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "get context info num devices failed, err=" << err << std::endl;
        exit(1);
    } 

    if (ndev > 16) {
        std::cout << "device array too small, platform has " << ndev << " devices" << std::endl;
        exit(1);
    }

    err = clGetContextInfo(saved.ctx, CL_CONTEXT_DEVICES, ndev*sizeof(cl_device_id), dev_ids, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "get context info devices failed, err=" << err << std::endl;
        exit(1);
    } 

    // Make sure it is MI-100
    err = clGetDeviceInfo(dev_ids[0], CL_DEVICE_NAME, sizeof(dstr), dstr, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "get device name failed, err=" << err << std::endl;
        exit(1);
    }

    std::cout << "Selecting device 0 (Name=" << dstr << ") of " << ndev <<" available" << std::endl;

#if 0
    if (strcmp(dstr, "gfx908")) {
        std::cout << "Device " << dstr << " does not support any of these tests" << std::endl;
        exit(1);
    }
#endif

    saved.did = dev_ids[0];

    // create a command-queue
    saved.cmd_queue = clCreateCommandQueueWithProperties(saved.ctx, saved.did, NULL, &err);
    if (saved.cmd_queue == (cl_command_queue)0) {
        std::cout << "create command queue failed, err=" << err << std::endl;
        exit(1);
    }

    std::ifstream clfile("ktest.cl");
    std::string source((std::istreambuf_iterator<char>(clfile)), std::istreambuf_iterator<char>());
    size_t length = source.length();
    const char* sourceptr = source.c_str();
    saved.program = clCreateProgramWithSource(saved.ctx, 1, &sourceptr, NULL, &err);
    if (saved.program == (cl_program)0) {
        std::cout << "create program with source failed, err=" << err << std::endl;
        exit(1);
    }

    // build the program
    std::string build_option{"-cl-std=CL2.0 -DMATRIX_SIZE="};
    build_option = build_option + std::to_string(MATRIX_SIZE);
    err = clBuildProgram(saved.program, 1, &saved.did, build_option.c_str(), NULL, NULL);
    if (err != CL_SUCCESS) {
        char build_log[16384];
        size_t log_sz;
        std::cout << "build program failed, err=" << err << std::endl;
        err = clGetProgramBuildInfo(saved.program, saved.did, CL_PROGRAM_BUILD_LOG,
                                sizeof(build_log), build_log, &log_sz);
        if (err != CL_SUCCESS)
            std::cout << "failed to get build log, err=" << err << std::endl;
        else
            std::cout << "----- Build Log -----"<< std::endl << build_log << std::endl << "----- ----- --- -----" << std::endl;
        exit(1);
    }

    // create the kernel
    saved.kernel = clCreateKernel(saved.program, "mixgemm1K",  &err);
    if (saved.kernel == (cl_kernel)0) {
        std::cout << "create kernel failed, err=" << err << std::endl;
        exit(1);
    }

    // allocate the buffer memory objects
    saved.a_buf = clCreateBuffer(saved.ctx, CL_MEM_READ_ONLY, lda*MATRIX_SIZE*sizeof(float), NULL, &err);
    if (err != CL_SUCCESS) {
        std::cout << "create a buffer failed, err=" << err << std::endl;
        exit(1);
    }

    saved.b_buf = clCreateBuffer(saved.ctx, CL_MEM_READ_ONLY, ldb*MATRIX_SIZE*sizeof(float), NULL, &err);
    if (err != CL_SUCCESS) {
        std::cout << "create b buffer failed, err=" << err << std::endl;
        exit(1);
    }

    saved.d_buf = clCreateBuffer(saved.ctx, CL_MEM_READ_WRITE, ldd*MATRIX_SIZE*sizeof(float), NULL, &err);
    if (err != CL_SUCCESS) {
        std::cout << "create d buffer failed, err=" << err << std::endl;
        exit(1);
    }

    err  = clSetKernelArg(saved.kernel, 0, sizeof(cl_mem), (void *) &saved.a_buf);
    if (err != CL_SUCCESS) {
        std::cout << "setting a kernel arg failed, err=" << err << std::endl;
        exit(1);
    }

    err  = clSetKernelArg(saved.kernel, 1, sizeof(cl_int), (void *)&lda);
    if (err != CL_SUCCESS) {
        std::cout << "setting lda kernel arg failed, err=" << err << std::endl;
        exit(1);
    }

    err  = clSetKernelArg(saved.kernel, 2, sizeof(cl_mem), (void *) &saved.b_buf);
    if (err != CL_SUCCESS) {
        std::cout << "setting b kernel arg failed, err=" << err << std::endl;
        exit(1);
    }

    err  = clSetKernelArg(saved.kernel, 3, sizeof(cl_int), (void *)&ldb);
    if (err != CL_SUCCESS) {
        std::cout << "setting ldb kernel arg failed, err=" << err << std::endl;
        exit(1);
    }

    err  = clSetKernelArg(saved.kernel, 4, sizeof(cl_mem), (void *) &saved.d_buf);
    if (err != CL_SUCCESS) {
        std::cout << "setting d kernel arg failed, err=" << err << std::endl;
        exit(1);
    }

    err  = clSetKernelArg(saved.kernel, 5, sizeof(cl_int), (void *)&ldd);
    if (err != CL_SUCCESS) {
        std::cout << "setting ldd kernel arg failed, err=" << err << std::endl;
        exit(1);
    }

    err = clEnqueueWriteBuffer(saved.cmd_queue, saved.a_buf, CL_FALSE, 0, lda*MATRIX_SIZE*sizeof(float), a, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "enqueue write a buffer failed, err=" << err << std::endl;
        exit(1);
    }

    err = clEnqueueWriteBuffer(saved.cmd_queue, saved.b_buf, CL_FALSE, 0, ldb*MATRIX_SIZE*sizeof(float), b, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "enqueue write b buffer failed, err=" << err << std::endl;
        exit(1);
    }

    // set work-item dimensions
    cl_event ke;
    size_t global_work_size[1] = { 64*32 };
    size_t local_work_size[1] = { 64*4 };

    // execute kernel
    err = clEnqueueNDRangeKernel(saved.cmd_queue, saved.kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, &ke);
    if (err != CL_SUCCESS) {
        std::cout << "enqueue ND range kernel failed, err=" << err << std::endl;
        exit(1);
    }

    // read results
    err = clEnqueueReadBuffer(saved.cmd_queue, saved.d_buf, CL_FALSE, 0, ldd*MATRIX_SIZE*sizeof(float), d, 0, NULL, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "enqueue read d buffer failed, err=" << err << std::endl;
        exit(1);
    }

    err = clFinish(saved.cmd_queue);
    if (err != CL_SUCCESS) {
        std::cout << "finish failed, err=" << err << std::endl;
        exit(1);
    }

    cl_int ks;

    err = clGetEventInfo(ke, CL_EVENT_COMMAND_EXECUTION_STATUS, sizeof(ks), &ks, NULL);
    if (err != CL_SUCCESS) {
        std::cout << "get event info execution status failed, err=" << err << std::endl;
        exit(1);
    }

    if (ks != CL_COMPLETE) {
        std::cout << "kernel execution status not complete but " << ks << std::endl;
        exit(1);
    }

    err = clReleaseEvent(ke);
    if (err != CL_SUCCESS) {
        std::cout << "release event failed, err=" << err << std::endl;
        exit(1);
    }
// #endif
}

int main(int argc, char** argv)
{
    std::vector<float> hA(MATRIX_SIZE * MATRIX_SIZE);
    std::vector<float> hB(MATRIX_SIZE * MATRIX_SIZE);
    std::vector<float> hD_c(MATRIX_SIZE * MATRIX_SIZE);
    std::vector<float> hD_g(MATRIX_SIZE * MATRIX_SIZE);

//    init_buffer_with_value(hA, 1.0f);
    init_buffer_with_random(hA);
//    init_buffer_with_value(hB, 1.0f);
    init_buffer_with_random(hB);
//    init_buffer_with_value(hD_c, 0.0f);
    init_buffer_with_random(hD_c);
    hD_g = hD_c;

    cblas_sgemm(CblasColMajor,
                CblasTrans,
                CblasNoTrans,
                MATRIX_SIZE,
                MATRIX_SIZE,
                MATRIX_SIZE,
                1.0f,
                hA.data(),
                MATRIX_SIZE,
                hB.data(),
                MATRIX_SIZE,
                0.0f,
                hD_c.data(),
                MATRIX_SIZE);


    krun(hA.data(), MATRIX_SIZE, hB.data(), MATRIX_SIZE, hD_g.data(), MATRIX_SIZE);

#ifdef DEBUG
    dump_matrix("hA",   hA,   MATRIX_SIZE, MATRIX_SIZE, MATRIX_SIZE);
    dump_matrix("hB",   hB,   MATRIX_SIZE, MATRIX_SIZE, MATRIX_SIZE);
    dump_matrix("hD_c", hD_c, MATRIX_SIZE, MATRIX_SIZE, MATRIX_SIZE);
    dump_matrix("hD_g", hD_g, MATRIX_SIZE, MATRIX_SIZE, MATRIX_SIZE);
#endif

    compare_result(hD_c.data(), hD_g.data(), MATRIX_SIZE*MATRIX_SIZE);

    return 0;
}
