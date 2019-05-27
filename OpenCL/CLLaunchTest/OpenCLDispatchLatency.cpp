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
#define ARRAY_SIZE 100000
#define LEN 1024*1024
#define NUM_GROUPS 1
#define GROUP_SIZE 64
#define TEST_ITERS 20
#define DISPATCHES_PER_TEST 100

void checkCLError(cl_int err, const char* ptr)
{
    if (err != CL_SUCCESS)
    {
        std::cerr << ptr << " met cl error " << static_cast<int>(err) << std::endl;
        abort();
    }
}

void LaunchKernel(cl_command_queue& commandQueue, cl_kernel& kernel,
        cl_mem* arg1, cl_mem* arg2, cl_mem* arg3, cl_mem* arg4, int arg5)
{
    cl_int err;
    size_t global_work_size[3] = {NUM_GROUPS * GROUP_SIZE, 1, 1};
    size_t local_work_size[3] = {GROUP_SIZE, 1, 1};

    checkCLError(clSetKernelArg(kernel, 0, sizeof(cl_mem), arg1 ), "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 1, sizeof(cl_mem), arg2 ), "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 2, sizeof(cl_mem), arg3 ), "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 3, sizeof(cl_mem), arg4 ), "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 4, sizeof(int)   , &arg5), "clSetKernelArg");
    err = clEnqueueNDRangeKernel(commandQueue, kernel, 3, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    checkCLError(err, "clEnqueueNDRangeKernel");
}

void secondKernelLaunch(
        cl_command_queue& commandQueue,
        std::vector<cl_kernel>& kernels,
        cl_mem& output_a)
{
    cl_int   err;
    cl_event start = nullptr;
    cl_event end = nullptr;

    err = clEnqueueMarkerWithWaitList(commandQueue, 0, NULL, &start);
    checkCLError(err, "clEnqueueMarkerWithWaitList");

    for(int t=0; t<ARRAY_SIZE; t++)
    {
        LaunchKernel(commandQueue, kernels[0], &output_a, NULL, NULL, NULL, 0);
    }

    err = clEnqueueMarkerWithWaitList(commandQueue, 0, NULL, &end);
    checkCLError(err, "clEnqueueMarkerWithWaitList");

    checkCLError(clWaitForEvents(1, &end), "clWaitForEvents");

    cl_ulong st;
    err = clGetEventProfilingInfo(start, CL_PROFILING_COMMAND_SUBMIT, sizeof(cl_ulong), &st, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    cl_ulong et;
    err = clGetEventProfilingInfo(end, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &et, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    std::cout << "secondKernelLaunch time : " << 1e-6 * (et - st) << std::endl;
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
    std::vector<cl_kernel> kernels;
    kernels.push_back(clCreateKernel(program, "NearlyNull1" , &err));
    checkCLError(err, "clCreateKernel");

    // create buffer    
    cl_mem output_a = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(float)*LEN, NULL, &err);
    checkCLError(err, "clCreateBuffer");
    cl_mem output_b = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(float)*LEN, NULL, &err);
    checkCLError(err, "clCreateBuffer");
    cl_mem output_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(float)*LEN, NULL, &err);
    checkCLError(err, "clCreateBuffer");
    cl_mem output_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(float)*LEN, NULL, &err);
    checkCLError(err, "clCreateBuffer");

    secondKernelLaunch(commandQueue, kernels, output_a);

    // release memory object
    checkCLError(clReleaseMemObject(output_a), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(output_b), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(output_c), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(output_d), "clReleaseMemObject");

    // release environment
    checkCLError(clReleaseKernel(kernels[0]), "clReleaseKernel");
    checkCLError(clReleaseProgram(program), "");
    checkCLError(clReleaseContext(context), "");

    return 0;
}

