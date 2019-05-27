#ifndef CLPROFILER_H
#define CLPROFILER_H

#include <CL/cl.h>

void checkCLError(cl_int err, const char* ptr);

class CLProfiler
{
public:
    CLProfiler(cl_command_queue& command_queue);
    ~CLProfiler();
    void start();
    float end();
    float getTime();

private:
    cl_command_queue& mQueue;
    cl_event mStart{nullptr};
    cl_event mEnd{nullptr};
    float mTime{0.0f};
};

#endif
