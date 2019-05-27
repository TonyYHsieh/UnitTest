#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>

#include <CL/cl.h>

// debug flag
#define PRINT_PROGRESS 0
#define TEST_SET 0x2

// constant
#define NUM_GROUPS 64*1024
#define GROUP_SIZE 256
#define TEST_ITERS 20

void checkCLError(cl_int err, const char* ptr)
{
    if (err != CL_SUCCESS)
    {
        std::cerr << ptr << " met cl error " << static_cast<int>(err) << std::endl;
        abort();
    }
}

void launchKernelOneTime(
        cl_command_queue& commandQueue,
        cl_kernel& kernel,
        cl_mem& output_a)
{
    cl_int err;
    cl_event event;
    cl_event start = nullptr, end = nullptr;

    size_t global_work_size[3] = {NUM_GROUPS * GROUP_SIZE, 1, 1};
    size_t local_work_size[3] = {GROUP_SIZE, 1, 1};

    checkCLError(clSetKernelArg(kernel, 0, sizeof(cl_mem), &output_a), "clSetKernelArg");
    err = clEnqueueNDRangeKernel(commandQueue, kernel, 3, NULL, global_work_size, local_work_size, 0, NULL, &event);
    checkCLError(err, "clEnqueueNDRangeKernel");

    clWaitForEvents(1, &event);

    cl_ulong st;
    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &st, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    cl_ulong et;
    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &et, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    std::cout << "launchKernelOneTime : event " << 1e-6 * (et - st) << " ms " << std::endl;
}

int main()
{
    unsigned int   test_set = TEST_SET;
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
    cl_kernel kernel = clCreateKernel(program, "Fill10" , &err);
    checkCLError(err, "clCreateKernel");

    // create buffer    
    cl_mem output_a = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(float)*NUM_GROUPS*GROUP_SIZE, NULL, &err);
    checkCLError(err, "clCreateBuffer");

    // launch kernel and profile
    // for(int i=0; i<TEST_ITERS; i++)
    {
        launchKernelOneTime(commandQueue, kernel, output_a);
    }

    // release memory object
    checkCLError(clReleaseMemObject(output_a), "clReleaseMemObject");

    // release environment
    checkCLError(clReleaseKernel(kernel), "clReleaseKernel");
    checkCLError(clReleaseProgram(program), "");
    checkCLError(clReleaseContext(context), "");

    return 0;
}

