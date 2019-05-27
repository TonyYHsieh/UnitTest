#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>

#include <CL/cl.h>

// constant
#define GLOBAL_WORK 200
#define GROUP_SIZE 64

#define SOURCE "gen_kernel.s"

void checkCLError(cl_int err, const char* ptr)
{
    if (err != CL_SUCCESS)
    {
        std::cerr << ptr << " met cl error " << static_cast<int>(err) << std::endl;
        abort();
    }
}

cl_program compileAssambly(std::string src, std::string param, std::string dst)
{
    std::string command = "/opt/rocm/opencl/bin/x86_64/clang  -x assembler -target amdgcn--amdhsa -mcpu=gfx803";
    command = command + " -o " + dst;
    command = command + " " + src;
    system(command.c_str());
}

cl_program CreateProgramWithBinary(cl_context& context, cl_device_id& device, std::string src)
{
    cl_int err = CL_SUCCESS;

    std::string source;
    std::ifstream file(src, std::ios::binary | std::ios::ate);
    bool src_read_failed = false;
    do {
        const auto size = file.tellg();
        if (size == -1) {
            src_read_failed = true;
            break;
        }
        source.resize(size, '\0');
        file.seekg(std::ios::beg);
        if (file.fail()) {
            src_read_failed = true;
            break;
        }
        if (file.rdbuf()->sgetn(&source[0], size) != size) {
            src_read_failed = true;
            break;
        }
    } while (false);
    file.close();

    size_t size = source.size();
    const char* source_data = source.data();
    cl_program program = clCreateProgramWithBinary(context, 1, &device, &size, (const unsigned char**)&source_data, nullptr, &err);
    checkCLError(err, "clCreateProgramWithBinary");

    return program;
}

cl_program CreateProgramWithAssembly(cl_context& context, cl_device_id& device)
{
    cl_int err = CL_SUCCESS;

    compileAssambly(SOURCE, "", "kernel.bin");

    return CreateProgramWithBinary(context, device, "kernel.bin");
}

void launchKernel(
        cl_command_queue& commandQueue,
        cl_kernel& kernel,
        cl_mem& inputa,
        cl_mem& inputb,
        cl_int size,
        cl_mem& output)
{
    cl_int err;
    cl_event event;

    int groups = (GLOBAL_WORK + GROUP_SIZE - 1) / GROUP_SIZE;

    size_t global_work_size[3] = {groups * GROUP_SIZE, 1, 1};
    size_t local_work_size[3] = {GROUP_SIZE, 1, 1};

    checkCLError(clSetKernelArg(kernel, 0, sizeof(cl_mem), &inputa), "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 1, sizeof(cl_mem), &inputb), "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 2, sizeof(size)  , &size)  , "clSetKernelArg");
    checkCLError(clSetKernelArg(kernel, 3, sizeof(cl_mem), &output), "clSetKernelArg");

    err = clEnqueueNDRangeKernel(commandQueue, kernel, 3, NULL, global_work_size, local_work_size, 0, NULL, &event);
    checkCLError(err, "clEnqueueNDRangeKernel");

    clWaitForEvents(1, &event);

    cl_ulong st;
    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &st, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    cl_ulong et;
    err = clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &et, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    std::cout << "launchKernel : event " << 1e-6 * (et - st) << " ms " << std::endl;
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
    cl_command_queue commandQueue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    checkCLError(err, "clCreateCommandQueue");

#if 0
    // create progream from source
    std::ifstream clfile("kernel.cl");
    std::string source((std::istreambuf_iterator<char>(clfile)), std::istreambuf_iterator<char>());
    size_t length = source.length();
    const char* sourceptr = source.c_str();
    cl_program program = clCreateProgramWithSource(context, 1, &sourceptr, &length, &err);
    checkCLError(err, "clCreateProgramWithSource");
#else
    cl_program program = CreateProgramWithAssembly(context, device);
#endif

    // build progream
    err = clBuildProgram(program, 1, &device, "", NULL, NULL);
    if (err != CL_SUCCESS)
    {
        char buffer[10240];
        std::cerr << "clBuildProgram return" << err << std::endl;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, NULL);
        std::cerr << "CL Compilation failed: " << buffer << std::endl;
        abort();
    }

#if 1
    // build kernels
    cl_kernel kernel = clCreateKernel(program, "VectorAdd" , &err);
    checkCLError(err, "clCreateKernel");

    // create buffer
    cl_mem inputa = clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(int) * GLOBAL_WORK + 10, NULL, &err);
    cl_mem inputb = clCreateBuffer(context, CL_MEM_READ_ONLY,  sizeof(int) * GLOBAL_WORK + 10, NULL, &err);
    cl_mem output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * GLOBAL_WORK + 10, NULL, &err);
    checkCLError(err, "clCreateBuffer");

    // fillbuffer
    int value = 4;
    checkCLError(clEnqueueFillBuffer(commandQueue, inputa, &value, sizeof(value), 0, sizeof(int) * GLOBAL_WORK + 10, 0, nullptr, nullptr), "clEnqueueFillBuffer");
    value = 5;
    checkCLError(clEnqueueFillBuffer(commandQueue, inputb, &value, sizeof(value), 0, sizeof(int) * GLOBAL_WORK + 10, 0, nullptr, nullptr), "clEnqueueFillBuffer");

    clFlush(commandQueue);
    clFinish(commandQueue);

    // launch kernel and profile
    {
        launchKernel(commandQueue, kernel, inputa, inputb, GLOBAL_WORK, output);
    }

    int* data_p = (int*) clEnqueueMapBuffer(commandQueue, output, CL_TRUE, CL_MAP_READ, 0, sizeof(int) * GLOBAL_WORK + 10, 0, nullptr, nullptr, &err);
    checkCLError(err, "clEnqueueMapBuffer");
    for(int i=0; i< GLOBAL_WORK + 10; i++)
    {
        if (i%20 == 0)
        {
            std::cout << std::endl;
        }
        std::cout << data_p[i] << " ";
    }
    std::cout << std::endl;

    err = clEnqueueUnmapMemObject(commandQueue, output, (void*)data_p, 0, nullptr, nullptr);
    checkCLError(err, "clEnqueueUnmapMemObject");

    // release memory object
    checkCLError(clReleaseMemObject(inputa), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(inputb), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(output), "clReleaseMemObject");

    // release environment
    checkCLError(clReleaseKernel(kernel), "clReleaseKernel");
    checkCLError(clReleaseProgram(program), "");
    checkCLError(clReleaseContext(context), "");
#endif
    return 0;
}

