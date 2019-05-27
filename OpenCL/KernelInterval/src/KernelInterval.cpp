#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>

#include <CL/cl.h>

// constant
#define LEN 512*1024
#define GROUP_SIZE 64
#define TEST_ITERS 10

void checkCLError(cl_int err, const char* ptr)
{
    if (err != CL_SUCCESS)
    {
        std::cerr << ptr << " met cl error " << static_cast<int>(err) << std::endl;
        abort();
    }
}

cl_event LaunchKernel(
        cl_command_queue& commandQueue,
        cl_kernel& kernel,
        cl_mem* arg1,
        cl_mem* arg2)
{
    cl_int err;
    cl_event event;
    size_t global_work_size[3] = {LEN, 1, 1};
    size_t local_work_size[3] = {GROUP_SIZE, 1, 1};

    checkCLError(clSetKernelArg(kernel, 0, sizeof(cl_mem), arg1 ), "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 1, sizeof(cl_mem), arg2 ), "clSetKernelArg");
    err = clEnqueueNDRangeKernel(commandQueue, kernel, 3, NULL, global_work_size, local_work_size, 0, NULL, &event);
    checkCLError(err, "clEnqueueNDRangeKernel");

    return event;
}

int main()
{
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
    cl_command_queue commandQueue = clCreateCommandQueue(context, device, CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
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
    cl_kernel kernel = clCreateKernel(program, "copy" , &err);
    checkCLError(err, "clCreateKernel");

    // create buffers
    cl_mem inBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(float)*LEN, NULL, &err);
    checkCLError(err, "clCreateBuffer");
    cl_mem outBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(float)*LEN, NULL, &err);
    checkCLError(err, "clCreateBuffer");

    // fill buffer with value 1
    float one = 1.0f;
    clEnqueueFillBuffer(commandQueue, inBuffer, &one, sizeof(one), 0, sizeof(float)*LEN, 0, NULL, NULL);
    // wait for process complete
    clFlush(commandQueue);
    clFinish(commandQueue);

    // launch kernel and store event for profiling
    std::vector<cl_event> events;
    for(int i=0; i<TEST_ITERS; i++)
    {
        events.push_back(LaunchKernel(commandQueue, kernel, &inBuffer, &outBuffer));
    }

    // wait for process complete
    clFlush(commandQueue);
    clFinish(commandQueue);

    // check output data
    float* hostBuffer = new float[LEN];
    err = clEnqueueReadBuffer(commandQueue, outBuffer, CL_TRUE, 0, sizeof(float)*LEN, hostBuffer, 0, NULL, NULL);
    checkCLError(err, "clEnqueueReadBuffer");
    for(int i=LEN/2; i<LEN/2+10; ++i)
    {
        std::cout << hostBuffer[i] << " ";
    }
    std::cout << std::endl;
    delete [] hostBuffer;

    // output kernel duration
    for(size_t i=0; i<events.size(); i++)
    {
        cl_ulong st;
        err = clGetEventProfilingInfo(events[i], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &st, NULL);
        checkCLError(err, "clGetEventProfilingInfo");

        cl_ulong et;
        err = clGetEventProfilingInfo(events[i], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &et, NULL);
        checkCLError(err, "clGetEventProfilingInfo");

        std::cout << "kernel " << i << " time : " << 1e-6 * (et - st) << " ms" << std::endl;
    }

    // release event
    for(auto event : events)
    {
        checkCLError(clReleaseEvent(event), "clReleaseEvent");
    }

    // release memory object
    checkCLError(clReleaseMemObject(inBuffer), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(outBuffer), "clReleaseMemObject");

    // release environment
    checkCLError(clReleaseKernel(kernel), "clReleaseKernel");
    checkCLError(clReleaseProgram(program), "");
    checkCLError(clReleaseContext(context), "");

    return 0;
}

