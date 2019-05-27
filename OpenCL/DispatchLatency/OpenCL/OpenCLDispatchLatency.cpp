#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>

#include <CL/cl.h>

#include "CLProfiler.h"
#include "ResultDatabase.h"

// debug flag
#define PRINT_PROGRESS 0
#define TEST_SET 0x03

// constant
#define ARRAY_SIZE 3000
#define LEN 1024*1024
#define NUM_GROUPS 1
#define GROUP_SIZE 64
#define TEST_ITERS 20
#define DISPATCHES_PER_TEST 100


void addRecordToDB(ResultDatabase& db, float ms, const char* msg, int iters)
{
    // send to resultDB
    db.AddResult(std::string(msg), "", "uS", ms*1000/iters);
    if (PRINT_PROGRESS & 0x1 ) {
        std::cout<< msg << "\t\t" << ms*1000/iters << " uS" << std::endl;
    }
    if (PRINT_PROGRESS & 0x2 ) {
        db.DumpSummary(std::cout);
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

void firstKernelLaunch(
        cl_command_queue& commandQueue,
        std::vector<cl_kernel>& kernels,
        cl_mem& output_a,
        ResultDatabase& resultDB)
{
    CLProfiler profiler(commandQueue);

    profiler.start();
    LaunchKernel(commandQueue, kernels[0], &output_a, NULL, NULL, NULL, 0);
    profiler.end();

    addRecordToDB(resultDB, profiler.getTime(), "FirstKernelLaunch", 1);
}

void multiParamsFirstKernelLaunch(
        cl_command_queue& commandQueue,
        std::vector<cl_kernel>& kernels,
        std::vector<cl_mem>& output_h,
        std::vector<cl_mem>& output_i,
        std::vector<cl_mem>& output_j,
        std::vector<cl_mem>& output_k,
        ResultDatabase& resultDB)
{
    CLProfiler profiler(commandQueue);

    profiler.start();
    LaunchKernel(commandQueue, kernels[0], &output_h[0], &output_i[0], &output_j[0], &output_k[0], 1);
    LaunchKernel(commandQueue, kernels[1], &output_h[1], &output_i[1], &output_j[1], &output_k[1], 1);
    LaunchKernel(commandQueue, kernels[2], &output_h[2], &output_i[2], &output_j[2], &output_k[2], 1);
    LaunchKernel(commandQueue, kernels[3], &output_h[3], &output_i[3], &output_j[3], &output_k[3], 1);
    LaunchKernel(commandQueue, kernels[4], &output_h[4], &output_i[4], &output_j[4], &output_k[4], 1);
    LaunchKernel(commandQueue, kernels[5], &output_h[5], &output_i[5], &output_j[5], &output_k[5], 1);
    LaunchKernel(commandQueue, kernels[6], &output_h[6], &output_i[6], &output_j[6], &output_k[6], 1);
    LaunchKernel(commandQueue, kernels[7], &output_h[7], &output_i[7], &output_j[7], &output_k[7], 1);
    LaunchKernel(commandQueue, kernels[8], &output_h[8], &output_i[8], &output_j[8], &output_k[8], 1);
    LaunchKernel(commandQueue, kernels[9], &output_h[9], &output_i[9], &output_j[9], &output_k[9], 1);
    profiler.end();
    addRecordToDB(resultDB, profiler.getTime(), "MultiParamsFirstKernelLaunch", 1);
}

void secondKernelLaunch(
        cl_command_queue& commandQueue,
        std::vector<cl_kernel>& kernels,
        cl_mem& output_a,
        ResultDatabase& resultDB)
{
    CLProfiler profiler(commandQueue);

    profiler.start();
    for(int t=0; t<100; t++)
    {
        LaunchKernel(commandQueue, kernels[0], &output_a, NULL, NULL, NULL, 0);
    }
    profiler.end();
    addRecordToDB(resultDB, profiler.getTime(), "SecondKernelLaunch", 1);
}

void multiParamsSecondKernelLaunch(
        cl_command_queue& commandQueue,
        std::vector<cl_kernel>& kernels,
        std::vector<cl_mem>& output_h,
        std::vector<cl_mem>& output_i,
        std::vector<cl_mem>& output_j,
        std::vector<cl_mem>& output_k,
        ResultDatabase& resultDB)
{
    CLProfiler profiler(commandQueue);

    profiler.start();
    for(int t=0; t<ARRAY_SIZE/10; t++)
    {
        LaunchKernel(commandQueue, kernels[0], &output_h[10*t+0], &output_i[10*t+0], &output_j[10*t+0], &output_k[10*t+0], t%100);
        LaunchKernel(commandQueue, kernels[1], &output_h[10*t+1], &output_i[10*t+1], &output_j[10*t+1], &output_k[10*t+1], t%100);
        LaunchKernel(commandQueue, kernels[2], &output_h[10*t+2], &output_i[10*t+2], &output_j[10*t+2], &output_k[10*t+2], t%100);
        LaunchKernel(commandQueue, kernels[3], &output_h[10*t+3], &output_i[10*t+3], &output_j[10*t+3], &output_k[10*t+3], t%100);
        LaunchKernel(commandQueue, kernels[4], &output_h[10*t+4], &output_i[10*t+4], &output_j[10*t+4], &output_k[10*t+4], t%100);
        LaunchKernel(commandQueue, kernels[5], &output_h[10*t+5], &output_i[10*t+5], &output_j[10*t+5], &output_k[10*t+5], t%100);
        LaunchKernel(commandQueue, kernels[6], &output_h[10*t+6], &output_i[10*t+6], &output_j[10*t+6], &output_k[10*t+6], t%100);
        LaunchKernel(commandQueue, kernels[7], &output_h[10*t+7], &output_i[10*t+7], &output_j[10*t+7], &output_k[10*t+7], t%100);
        LaunchKernel(commandQueue, kernels[8], &output_h[10*t+8], &output_i[10*t+8], &output_j[10*t+8], &output_k[10*t+8], t%100);
        LaunchKernel(commandQueue, kernels[9], &output_h[10*t+9], &output_i[10*t+9], &output_j[10*t+9], &output_k[10*t+9], t%100);
    }
    profiler.end();
    addRecordToDB(resultDB, profiler.getTime(), "MultiParamsSecondKernelLaunch", 1);
}

void streamASyncDispatchWait(
        cl_command_queue& commandQueue,
        std::vector<cl_kernel>& kernels,
        cl_mem& output_a,
        ResultDatabase& resultDB)
{
    for(int t=0; t<TEST_ITERS; t++)
    {
        CLProfiler profiler(commandQueue);
        profiler.start();
        for(int t=0; t<DISPATCHES_PER_TEST; t++)
        {
            cl_int err;
            cl_event event;

            LaunchKernel(commandQueue, kernels[0], &output_a, NULL, NULL, NULL, 0);            
            err = clEnqueueMarkerWithWaitList(commandQueue, 0, NULL, &event);
            checkCLError(err, "clEnqueueMarkerWithWaitList");
            err = clWaitForEvents(1, &event);
            checkCLError(err, "clWaitForEvents");
            err = clReleaseEvent(event);
            checkCLError(err, "clReleaseEvent");
        }
        profiler.end();
        addRecordToDB(resultDB, profiler.getTime(), "StreamASyncDispatchWait", DISPATCHES_PER_TEST);
    }
}

void streamASyncDispatchNoWait(
        cl_command_queue& commandQueue,
        std::vector<cl_kernel>& kernels,
        cl_mem& output_a,
        ResultDatabase& resultDB)
{
    for(int t=0; t<TEST_ITERS; t++)
    {
        CLProfiler profiler(commandQueue);
        profiler.start();
        for(int t=0; t<DISPATCHES_PER_TEST; t++)
        {
            LaunchKernel(commandQueue, kernels[0], &output_a, NULL, NULL, NULL, 0);
        }
        profiler.end();
        addRecordToDB(resultDB, profiler.getTime(), "StreamASyncDispatchNoWait", DISPATCHES_PER_TEST);
    }
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
    kernels.push_back(clCreateKernel(program, "NearlyNull2" , &err));
    kernels.push_back(clCreateKernel(program, "NearlyNull3" , &err));
    kernels.push_back(clCreateKernel(program, "NearlyNull4" , &err));
    kernels.push_back(clCreateKernel(program, "NearlyNull5" , &err));
    kernels.push_back(clCreateKernel(program, "NearlyNull6" , &err));
    kernels.push_back(clCreateKernel(program, "NearlyNull7" , &err));
    kernels.push_back(clCreateKernel(program, "NearlyNull8" , &err));
    kernels.push_back(clCreateKernel(program, "NearlyNull9" , &err));
    kernels.push_back(clCreateKernel(program, "NearlyNull10", &err));
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

    std::vector<cl_mem> output_h;
    std::vector<cl_mem> output_i;
    std::vector<cl_mem> output_j;
    std::vector<cl_mem> output_k;

    for(int i=0; i<ARRAY_SIZE; i++)
    {
        cl_mem buffer;
        buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(float)*100, NULL, &err);
        checkCLError(err, "clCreateBuffer");
        output_h.push_back(buffer);
        buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(float)*100, NULL, &err);
        checkCLError(err, "clCreateBuffer");
        output_i.push_back(buffer);
        buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(float)*100, NULL, &err);
        checkCLError(err, "clCreateBuffer");
        output_j.push_back(buffer);
        buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,  sizeof(float)*100, NULL, &err);
        checkCLError(err, "clCreateBuffer");
        output_k.push_back(buffer);
    }

    // result recorder
    ResultDatabase resultDB;

    // Test set 0x1
    if (test_set & 0x1)
    {
        firstKernelLaunch(commandQueue, kernels, output_a, resultDB);

        // multiParamsFirstKernelLaunch(commandQueue, kernels, output_h, output_i, output_j, output_k, resultDB);
    }

    if (test_set & 0x2)
    {
        secondKernelLaunch(commandQueue, kernels, output_a, resultDB);

        // multiParamsSecondKernelLaunch(commandQueue, kernels, output_h, output_i, output_j, output_k, resultDB);
    }

    if (test_set & 0x4)
    {
        //TODO: no null stream use case in OpenCL
    }

    if (test_set & 0x10)
    {
        streamASyncDispatchWait(commandQueue, kernels, output_a, resultDB);
    }

    if (test_set & 0x40)
    {
        //TODO: no null stream use case in OpenCL
    }

    if (test_set & 0x80)
    {
        streamASyncDispatchNoWait(commandQueue, kernels, output_a, resultDB);
    }

    resultDB.DumpSummary(std::cout);

    // release memory object
    checkCLError(clReleaseMemObject(output_a), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(output_b), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(output_c), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(output_d), "clReleaseMemObject");

    for(int i=0; i<output_h.size(); i++)
    {
        checkCLError(clReleaseMemObject(output_h[i]), "clReleaseMemObject");
        checkCLError(clReleaseMemObject(output_i[i]), "clReleaseMemObject");
        checkCLError(clReleaseMemObject(output_j[i]), "clReleaseMemObject");
        checkCLError(clReleaseMemObject(output_k[i]), "clReleaseMemObject");
    }

    // release environment
    checkCLError(clReleaseKernel(kernels[0]), "clReleaseKernel");
    checkCLError(clReleaseProgram(program), "");
    checkCLError(clReleaseContext(context), "");

    return 0;
}

