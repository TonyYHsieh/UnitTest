#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <streambuf>
#include <ctime>
#include <cstdio>
#include <cstdlib>

#include <CL/cl.h>

// constant
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

void fill_rand(float* buf, size_t size)
{
    for(size_t i=0; i<size; i++)
    {
        buf[i] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    }
}

void conv_basic_check(float* src_data, float* weights_data, float* dst_data, ProblemConfig& config,
                      int stride_w = 1, int stride_h = 1, int dilation_w = 1, int dilation_h = 1, int pad_w = 0, int pad_h = 0)
{
    int in_num = config.m_n;

    int in_channel = config.m_c;
    int in_h = config.m_h;
    int in_w = config.m_w;

    int out_channels = config.m_k;
    int out_h = config.m_h;
    int out_w = config.m_w;

    int kernel_h = config.m_y;
    int kernel_w = config.m_x;

    int group = config.m_g;
    int out_c_group = out_channels / config.m_g;
    int in_c_group = in_channel / config.m_g;

// #pragma omp parallel for num_threads(8) collapse(5) schedule(static)
    for (int n = 0; n < in_num; ++n) {
        for (int g = 0; g < group; ++g) {
            for (int oc = 0; oc < out_c_group; ++oc) {
                for (int oh = 0; oh < out_h; ++oh) {
                    for (int ow = 0; ow < out_w; ++ow) {
                        int out_idx = n * group * out_c_group * out_h * out_w + g * out_c_group * out_h * out_w + oc * out_h * out_w + oh * out_w + ow;
                        dst_data[out_idx] = 0;
                        for (int ic = 0; ic < in_c_group; ++ic) {
                            for (int kh = 0; kh < kernel_h; ++kh) {
                                for (int kw = 0; kw < kernel_w; ++kw) {
                                    int iw = ow * stride_w - pad_w + kw * (dilation_w);
                                    int ih = oh * stride_h - pad_h + kh * (dilation_h);
                                    if (iw < 0 || iw >= in_w) continue;
                                    if (ih < 0 || ih >= in_h) continue;

                                    int iidx = n * in_channel * in_h * in_w
                                               + g * in_c_group * in_h * in_w
                                               + ic * in_h * in_w
                                               + ih * in_w
                                               + iw;
                                    int widx = g * out_c_group * in_c_group * kernel_h * kernel_w
                                               + oc * in_c_group * kernel_h * kernel_w
                                               + ic * kernel_h * kernel_w
                                               + kh * kernel_w
                                               + kw;

                                    dst_data[out_idx] += src_data[iidx] * weights_data[widx];
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}


// test by 3x3 filter with 1x1 padding.
int main(int argc, char* argv[])
{
    cl_int         err = CL_SUCCESS;
    cl_platform_id platform;
    cl_device_id   device;

    if (argc != 9)
    {
        std::cout << "./GroupConv [N] [C] [H] [W] [K] [Y:3] [X:3] [G]" << std::endl;
    }

    ProblemConfig config(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), atoi(argv[8]));

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
    cl_kernel kernel = clCreateKernel(program, "Conv3x3" , &err);
    checkCLError(err, "clCreateKernel");

    size_t input_size  = config.m_n * config.m_c * config.m_h * config.m_w;
    size_t weight_size = config.m_k * config.m_c * config.m_y * config.m_x / config.m_g;
    size_t output_size = config.m_n * config.m_k * config.m_h * config.m_w;

    // create host input buffer
    float* inputbuf_h   = new float[input_size];
    float* weightbuf_h  = new float[weight_size];
    float* outputbuf_h  = new float[output_size];
    float* outputbuf_d  = new float[output_size];

    srand(time(NULL));
    fill_rand(inputbuf_h, input_size);
    fill_rand(weightbuf_h, weight_size);

    // create device input buffer
    cl_mem inputbuf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * input_size, NULL, &err);
    checkCLError(err, "clCreateBuffer");
    cl_mem weightbuf = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * weight_size, NULL, &err);
    checkCLError(err, "clCreateBuffer");
    // create device output buffer
    cl_mem outputbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * output_size, NULL, &err);
    checkCLError(err, "clCreateBuffer");

    // Fill Buffer
    err = clEnqueueWriteBuffer(commandQueue, inputbuf, CL_TRUE, 0, sizeof(float) * input_size, inputbuf_h, 0, nullptr, nullptr);
    checkCLError(err, "clEnqueueWriteBuffer");
    err = clEnqueueWriteBuffer(commandQueue, weightbuf, CL_TRUE, 0, sizeof(float) * weight_size, weightbuf_h, 0, nullptr, nullptr);
    checkCLError(err, "clEnqueueWriteBuffer");

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
        checkCLError(clSetKernelArg(kernel, 8, sizeof(cl_int), &config.m_g), "clSetKernelArg");

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

    clEnqueueReadBuffer(commandQueue, outputbuf, CL_TRUE, 0, sizeof(float) * output_size, outputbuf_d, 0, nullptr, nullptr);

    // do cpu calculation
    conv_basic_check(inputbuf_h, weightbuf_h, outputbuf_h, config, 1, 1, 1, 1, 1, 1); 

    for(size_t i=0; i<output_size; i++)
    {
        if (abs(outputbuf_d[i] - outputbuf_h[i]) > 0.0001)
        {
            std::cout << "ERROR: " << outputbuf_d[i] << " " << outputbuf_h[i] << std::endl;
            break;
        }
    }

#if 0
    for(int k=0; k<config.m_k; k++)
    {
        for(int j=0; j<config.m_h; j++)
        {
            for(int i=0; i<config.m_w; i++)
            {
                std::cout << outputbuf_d[(k * config.m_h + j) * config.m_w + i] << " ";
            }
            std::cout << std::endl;
        }
    }

    std::cout << std::endl << std::endl << std::endl << std::endl;

    for(int k=0; k<config.m_k; k++)
    {
        for(int j=0; j<config.m_h; j++)
        {
            for(int i=0; i<config.m_w; i++)
            {
                std::cout << outputbuf_h[(k * config.m_h + j) * config.m_w + i] << " ";
            }
            std::cout << std::endl;
        }
    }
#endif

    // release memory object
    checkCLError(clReleaseMemObject(inputbuf), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(weightbuf), "clReleaseMemObject");
    checkCLError(clReleaseMemObject(outputbuf), "clReleaseMemObject");

    // release environment
    checkCLError(clReleaseKernel(kernel), "clReleaseKernel");
    checkCLError(clReleaseProgram(program), "");
    checkCLError(clReleaseContext(context), "");

    delete [] inputbuf_h;
    delete [] weightbuf_h;
    delete [] outputbuf_h;
    delete [] outputbuf_d;


    return 0;
}

