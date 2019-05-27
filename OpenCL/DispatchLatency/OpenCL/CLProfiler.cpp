#include <iostream>
#include "CLProfiler.h"

// utils function
void checkCLError(cl_int err, const char* ptr)
{
    if (err != CL_SUCCESS)
    {
        std::cerr << ptr << " met cl error " << static_cast<int>(err) << std::endl;
        abort();
    }
}

void destroy_cl_event(cl_event& event)
{
    if (event == nullptr)
        return;

    cl_int err;
    cl_command_type type;
    err = clGetEventInfo(event, CL_EVENT_COMMAND_TYPE, sizeof(cl_command_type), &type, NULL);
    checkCLError(err, "clGetEventInfo");
    if (type == CL_COMMAND_USER)
    {
        cl_int refs;
        clGetEventInfo(event, CL_EVENT_REFERENCE_COUNT, sizeof(cl_int), &refs, NULL);
        checkCLError(err, "clGetEventInfo");
        if (refs == 1)
        {
           checkCLError(clSetUserEventStatus(event, CL_COMPLETE), "clSetUserEventStatus");
        }
        checkCLError(clReleaseEvent(event), "clReleaseEvent");
    }
}

void record_event(cl_command_queue& command_queue, cl_event& event)
{
    cl_int err = clEnqueueMarkerWithWaitList(command_queue, 0, NULL, &event);
    checkCLError(err, "clEnqueueMarkerWithWaitList");
}

void sync_event(cl_event& event)
{
    if (event == NULL)
    {
        std::cout << __FUNCTION__ << ": event is nullpter" << std::endl;
        return;
    }

    checkCLError(clWaitForEvents(1, &event), "clWaitForEvents");
}

// CLProfiler implementation
CLProfiler::CLProfiler(cl_command_queue& command_queue)
    : mQueue {command_queue}
{
}

CLProfiler::~CLProfiler()
{
    destroy_cl_event(mStart);
    destroy_cl_event(mEnd);
}

void CLProfiler::start()
{
    destroy_cl_event(mStart);
    record_event(mQueue, mStart);
}

float CLProfiler::end()
{
    destroy_cl_event(mEnd);
    record_event(mQueue, mEnd);
    sync_event(mEnd);

    cl_int err;

    cl_ulong start;
    err = clGetEventProfilingInfo(mStart, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    cl_ulong end;
    err = clGetEventProfilingInfo(mEnd, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
    checkCLError(err, "clGetEventProfilingInfo");

    mTime = 1e-6 * (end - start);

    return mTime;
}

float CLProfiler::getTime()
{
    return mTime;
}
