#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include <cstring>
#include <CL/cl.h>

// debug flag
#define PRINT_PROGRESS 0
#define TEST_SET 0x2

// constant
#define NUM_GROUPS 64*1024
#define GROUP_SIZE 256
#define TEST_ITERS 10

struct ProblemConfig
{
    ProblemConfig(unsigned int n, unsigned int c, unsigned int h, unsigned int w,
                  unsigned int k, unsigned int y, unsigned int x, unsigned int g)
        :m_n(n), m_c(c), m_h(h), m_w(w), m_k(k), m_y(y), m_x(x), m_g(g)
    {}

    unsigned int m_n{1};
    unsigned int m_c{32};
    unsigned int m_h{64};
    unsigned int m_w{64};
    unsigned int m_k{32};
    unsigned int m_y{1};
    unsigned int m_x{1};
    unsigned int m_g{1};
};

void checkCLError(cl_int err, const char* ptr)
{
    if (err != CL_SUCCESS)
    {
        std::cerr << ptr << " met cl error " << static_cast<int>(err) << std::endl;
        abort();
    }
}

// test by 3x3 filter with 1x1 padding.
int main()
{
    unsigned int   test_set = TEST_SET;
    cl_int         err = CL_SUCCESS;
    cl_platform_id platform;
    cl_device_id   device;

    ProblemConfig config {1, 32, 256, 256, 32, 3, 3, 1};

    // get cl platform
    err = clGetPlatformIDs(1, &platform, nullptr);
    checkCLError(err, "clGetPlatformIDs");

    // get cl device
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    checkCLError(err, "clGetDeviceIDs");

    // get cl context
    cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };
    cl_context context = clCreateContext(properties, 1, &device, nullptr, nullptr, &err);
    checkCLError(err, "clCreateContext");

    // get device name
    char devName[1024];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(devName), devName, NULL);
    std::cout << "gpu device: " << devName << std::endl;

    // create command queue with profiler
    cl_command_queue commandQueue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    checkCLError(err, "clCreateCommandQueue");

    // create progream from source
    std::ifstream clfile("kernel.cl");
    std::string source((std::istreambuf_iterator<char>(clfile)), std::istreambuf_iterator<char>());
    size_t length = source.length();
    const char* sourceptr = source.c_str();
    cl_program program = clCreateProgramWithSource(context, 1, &sourceptr, &length, &err);
    checkCLError(err, "clCreateProgramWithSource");

    // build progream
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        char buffer[10240];
        std::cerr << "clCreateProgramWithSource return" << err << std::endl;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        std::cerr << "CL Compilation failed: " << buffer << std::endl;
        abort();
    }

    // build kernels
    cl_kernel kernel = clCreateKernel(program, "Conv3x3_image" , &err);
    checkCLError(err, "clCreateKernel");

    cl_image_format clImageFormat;
    clImageFormat.image_channel_order = CL_R;
    clImageFormat.image_channel_data_type = CL_FLOAT;

    cl_mem inputbuf  = clCreateImage3D(context, CL_MEM_READ_ONLY,  &clImageFormat, config.m_w, config.m_h, config.m_c * config.m_n, 0, 0, nullptr, &err);
    checkCLError(err, "clCreateImage3D");
    cl_mem weightbuf = clCreateImage3D(context, CL_MEM_READ_ONLY,  &clImageFormat, config.m_x, config.m_y, config.m_c * config.m_k, 0, 0, nullptr, &err);
    checkCLError(err, "clCreateImage3D");
    cl_mem outputbuf = clCreateImage3D(context, CL_MEM_WRITE_ONLY, &clImageFormat, config.m_w, config.m_h, config.m_k * config.m_n, 0, 0, nullptr, &err);
    checkCLError(err, "clCreateImage3D");

    float color[1] = {1};
    size_t coor[3] = {0, 0, 0};
    size_t end[3]  = {config.m_w, config.m_h, config.m_c};
    checkCLError(clEnqueueFillImage(commandQueue, inputbuf, color, coor, end, 0, nullptr, nullptr), "clEnqueueFillImage");
    end[0] = config.m_x; end[1] = config.m_y; end[2] = config.m_c * config.m_k;
    checkCLError(clEnqueueFillImage(commandQueue, weightbuf, color, coor, end, 0, nullptr, nullptr), "clEnqueueFillImage");

    clFlush(commandQueue);
    clFinish(commandQueue);

#if 1
    std::vector<cl_event> events;

    // launch kernel and profile
    for(int i=0; i<TEST_ITERS; i++)
    {
        cl_event event;

        size_t global_work_size[3] = {config.m_w, config.m_h, config.m_k * config.m_n};
        size_t local_work_size[3] = {16, 16, 1};

        checkCLError(clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputbuf), "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel, 1, sizeof(cl_mem), &weightbuf), "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel, 2, sizeof(cl_mem), &outputbuf), "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel, 3, sizeof(cl_int), &config.m_n), "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel, 4, sizeof(cl_int), &config.m_c), "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel, 5, sizeof(cl_int), &config.m_h), "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel, 6, sizeof(cl_int), &config.m_w), "clSetKernelArg");
        checkCLError(clSetKernelArg(kernel, 7, sizeof(cl_int), &config.m_k), "clSetKernelArg");

        err = clEnqueueNDRangeKernel(commandQueue, kernel, 3, NULL, global_work_size, local_work_size, 0, NULL, &event);
        checkCLError(err, "clEnqueueNDRangeKernel");

        events.push_back(event);
    }

    // wait for GPU evnet
    clWaitForEvents(1, &events[events.size()-1]);

    // measure time
    cl_ulong st;
    err = clGetEventProfilingInfo(events[0], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &st, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    cl_ulong et;
    err = clGetEventProfilingInfo(events[events.size()-1], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &et, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    std::cout << "launchKernel : event " << 1e-6 * (et - st) / TEST_ITERS << " ms " << std::endl;
#endif

#if 1
//    float host[config.m_n * config.m_c * config.m_h * config.m_w];
    float* host = new float[config.m_w * config.m_h * config.m_c];
    end[0] = config.m_w; end[1] = config.m_h; end[2] = config.m_c;// config.m_k * config.m_k;
    clEnqueueReadImage(commandQueue, outputbuf, true, coor, end, 0, 0, host, 0, nullptr, nullptr);
    for(int j=0; j<16; j++)
    {
        for(int i=0; i<16; i++)
        {
            std::cout << host[config.m_w*config.m_h + j*config.m_w + i] << " ";
        }
        std::cout << std::endl;
    }
    delete [] host;
#endif

    // release memory object
    checkCLError(clReleaseMemObject(inputbuf), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(weightbuf), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(outputbuf), "clReleaseMemObject");

    // release environment
    checkCLError(clReleaseKernel(kernel), "clReleaseKernel");
    checkCLError(clReleaseProgram(program), "");
    checkCLError(clReleaseContext(context), "");

    return 0;
}

