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

struct ProgramParam
{
    ProgramParam(size_t n=1, size_t c=256, size_t w=56, size_t h=56, float f1=1.0f, float f2=1.0f)
        : N{n}, C{c}, W{w}, H{h}, coff1{f1}, coff2{f2}
        {}
    size_t N{1};
    size_t C{256};
    size_t W{56};
    size_t H{56};
    cl_float coff1{1.0f};
    cl_float coff2{1.0f};
};

void checkCLError(cl_int err, const char* ptr)
{
    if (err != CL_SUCCESS)
    {
        std::cout << ptr << " met cl error " << static_cast<int>(err) << std::endl;
        abort();
    }
}

void LaunchKernel(
        cl_command_queue commandQueue, cl_kernel kernel,
        cl_mem input_1,
        cl_mem input_2,
        ProgramParam& param,
        cl_mem output)
{
    cl_int err;
    cl_int count = 0;
    cl_event event;
    size_t global_work_size[3] = {param.N*param.C*param.W*param.H, 1, 1};
    size_t local_work_size[3] = {256, 1, 1};

    checkCLError(clSetKernelArg(kernel, 0, sizeof(cl_mem), &output), "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 1, sizeof(cl_mem), &input_1), "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 2, sizeof(cl_mem), &input_2), "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 3, sizeof(cl_float), &param.coff1) , "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 4, sizeof(cl_float), &param.coff2) , "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 5, sizeof(cl_int), &count), "clSetKernelArg");

    err = clEnqueueNDRangeKernel(commandQueue, kernel, 3, NULL, global_work_size, local_work_size, 0, NULL, &event);
    checkCLError(err, "clEnqueueNDRangeKernel");

    checkCLError(clWaitForEvents(1, &event), "clWaitForEvents");

    cl_ulong st;
    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &st, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    cl_ulong et;
    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &et, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    checkCLError(clReleaseEvent(event), "clReleaseEvent");

    std::cout << "launchKernelOneTime time : " << 1e-6 * (et - st) << " ms" << std::endl;
}

int main()
{
    // problem definition
    ProgramParam param(32, 256, 56, 56, 2.0f, 2.0f);

    // cl status
    cl_int err = CL_SUCCESS;

    // get cl platform
    cl_platform_id platform;
    err = clGetPlatformIDs(1, &platform, nullptr);
    checkCLError(err, "clGetPlatformIDs");

    // get cl device
    cl_device_id   device;
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
    std::ifstream clfile("Eltwise.cl");
    std::string source((std::istreambuf_iterator<char>(clfile)), std::istreambuf_iterator<char>());
    size_t length = source.length();
    const char* sourceptr = source.c_str();
    cl_program program = clCreateProgramWithSource(context, 1, &sourceptr, &length, &err);
    checkCLError(err, "clCreateProgramWithSource");

    // build progream
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        cl_char buffer[10240];
        std::cerr << "clCreateProgramWithSource return" << err << std::endl;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        std::cerr << "CL Compilation failed: " << buffer << std::endl;
        abort();
    }

    // build kernel
    cl_kernel kernel = clCreateKernel(program, "EltSum" , &err);
    checkCLError(err, "clCreateKernel");

    // create buffer
    size_t size = param.N * param.C * param.W * param.H;
    cl_mem input_1 = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(cl_float)*size, NULL, &err);
    checkCLError(err, "clCreateBuffer");
    cl_mem input_2 = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(cl_float)*size, NULL, &err);
    checkCLError(err, "clCreateBuffer");
    cl_mem output  = clCreateBuffer(context, CL_MEM_READ_WRITE,  sizeof(cl_float)*size, NULL, &err);
    checkCLError(err, "clCreateBuffer");

    cl_float one = 1;
    err = clEnqueueFillBuffer(commandQueue, input_1, (void*) &one, sizeof(cl_float), 0, sizeof(cl_float)*size, 0, NULL, NULL);
    checkCLError(err, "clEnqueueFillBuffer");
    err = clEnqueueFillBuffer(commandQueue, input_2, (void*) &one, sizeof(cl_float), 0, sizeof(cl_float)*size, 0, NULL, NULL);
    checkCLError(err, "clEnqueueFillBuffer");

    clFlush(commandQueue);
    clFinish(commandQueue);

    // launch kernel and profile
    for(size_t i=0; i<10; ++i)
    {
        LaunchKernel(commandQueue, kernel, input_1, input_2, param, output);
    }

#if 1
    // check output
    cl_float* hostBuf = new cl_float[size];
    err = clEnqueueReadBuffer(commandQueue, output, CL_TRUE, 0, sizeof(cl_float)*size, (void*)hostBuf, 0, NULL, NULL); 
    checkCLError(err, "clEnqueueReadBuffer");

    size_t NCOffset = ((param.N / 2 * param.C) + param.C / 2) * param.W * param.H;
    for (size_t j=0; j<param.H; j++)
    {
        for(size_t i=0; i<param.W; i++)
        {
            size_t idx = NCOffset + j * param.W + i;
            std::cout << hostBuf[idx] << " ";
        }
        std::cout << std::endl;
    }
    delete [] hostBuf;
#endif

    // release memory object
    checkCLError(clReleaseMemObject(input_1), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(input_2), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(output) , "clReleaseMemObject");

    // release environment
    checkCLError(clReleaseKernel(kernel), "clReleaseKernel");
    checkCLError(clReleaseProgram(program), "");
    checkCLError(clReleaseContext(context), "");

    return 0;
}

