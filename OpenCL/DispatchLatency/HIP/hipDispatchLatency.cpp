/*
Copyright (c) 2015-2016 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "hip/hip_runtime.h"
#include<iostream>
#include<time.h>
#include<math.h>
#include"ResultDatabase.h"

#define PRINT_PROGRESS 0

#define check(cmd) \
{\
  hipError_t status = cmd;\
  if(status != hipSuccess){ \
    printf("error: '%s'(%d) from %s at %s:%d\n", \
          hipGetErrorString(status), status, #cmd,\
          __FILE__, __LINE__); \
	abort(); \
  }\
}

#define LEN 1024*1024

#define NUM_GROUPS 1
#define GROUP_SIZE 64
#define TEST_ITERS 20          
#define DISPATCHES_PER_TEST 100

#define ARRAY_SIZE 10

const unsigned p_tests = 0xfffffff;


// HCC optimizes away fully NULL kernel calls, so run one that is nearly null:
__global__ void NearlyNull(hipLaunchParm lp, float* Ad, float* B, float* C, float* D, int t){
    if (Ad) {
        Ad[0] = 42;
    }

    if(B) {
	B[t] = 50;
    }
    if(C) {
	C[t] = 51;
    }
    if(D) {
	D[t] = 52;
    }
}

__global__ void NearlyNull2(hipLaunchParm lp, float* Ad, float* B, float* C, float* D, int t){
    if (Ad) {
	Ad[0] += 100;
    }
    if(B) {
	B[0] += 200;
    }
    if(C) {
	C[0] += 300;
    }
    if(D) {
	D[0] += 400;
    }
}
__global__ void NearlyNull3(hipLaunchParm lp, float* Ad, float* B, float* C, float* D, int t){
    if (Ad) {
	Ad[0] *= 100;
    }
    if(B) {
	B[0] *= 200;
    }
    if(C) {
	C[0] *= 300;
    }
    if(D) {
	D[0] *= 400;
    }
}
__global__ void NearlyNull4(hipLaunchParm lp, float* Ad, float* B, float* C, float* D, int t){
    if (Ad) {
	Ad[0] -= 100;
    }
    if(B) {
	B[0] -= 200;
    }
    if(C) {
	C[0] -= 300;
    }
    if(D) {
	D[0] -= 400;
    }
}
__global__ void NearlyNull5(hipLaunchParm lp, float* Ad, float* B, float* C, float* D, int t){
    if (Ad) {
	Ad[0] /= 100;
    }
    if(B) {
	B[0] /= 200;
    }
    if(C) {
	C[0] /= 300;
    }
    if(D) {
	D[0] /= 400;
    }
}
__global__ void NearlyNull6(hipLaunchParm lp, float* Ad, float* B, float* C, float* D, int t){
    if (Ad) {
	Ad[0] += 100*t;
    }
    if(B) {
	B[0] += 200*t;
    }
    if(C) {
	C[0] += 300*t;
    }
    if(D) {
	D[0] += 400*t;
    }
}
__global__ void NearlyNull7(hipLaunchParm lp, float* Ad, float* B, float* C, float* D, int t){
    if (Ad) {
	Ad[0] *= 100*t;
    }
    if(B) {
	B[0] *= 200*t;
    }
    if(C) {
	C[0] *= 300*t;
    }
    if(D) {
	D[0] *= 400*t;
    }
}
__global__ void NearlyNull8(hipLaunchParm lp, float* Ad, float* B, float* C, float* D, int t){
    if (Ad) {
	Ad[0] = sin(100);
    }
    if(B) {
	B[0] = sin(200);
    }
    if(C) {
	C[0] = sin(300);
    }
    if(D) {
	D[0] = sin(400);
    }
}
__global__ void NearlyNull9(hipLaunchParm lp, float* Ad, float* B, float* C, float* D, int t){
    if (Ad) {
	Ad[0] = tanh(100);
    }
    if(B) {
	B[0] = tanh(200);
    }
    if(C) {
	C[0] = tanh(300);
    }
    if(D) {
	D[0] = tanh(400);
    }
}
__global__ void NearlyNull10(hipLaunchParm lp, float* Ad, float* B, float* C, float* D, int t){
    if (Ad) {
	Ad[0] = exp(100);
    }
    if(B) {
	B[0] = exp(200);
    }
    if(C) {
	C[0] = exp(300);
    }
    if(D) {
	D[0] = exp(400);
    }
}

ResultDatabase resultDB;


void stopTest(hipEvent_t start, hipEvent_t stop, const char *msg, int iters)
{
	float mS = 0;
    check(hipEventRecord(stop));
    check(hipDeviceSynchronize());
    check(hipEventElapsedTime(&mS, start, stop));
    resultDB.AddResult(std::string(msg), "", "uS", mS*1000/iters); 
    if (PRINT_PROGRESS & 0x1 ) {
        std::cout<< msg <<"\t\t"<<mS*1000/iters<<" uS"<<std::endl;
    }
    if (PRINT_PROGRESS & 0x2 ) {
        resultDB.DumpSummary(std::cout);
    }
}


int main(){

	hipError_t err;
	float *Ad;
	float *B;
	float *C;
	float *D;
	int i;

    check(hipMalloc(&Ad, 4*LEN));
    check(hipMalloc(&B, 4*LEN));
    check(hipMalloc(&C, 4*LEN));
    check(hipMalloc(&D, 4*LEN));

	float *H[ARRAY_SIZE];
	float *I[ARRAY_SIZE];
	float *J[ARRAY_SIZE];
	float *K[ARRAY_SIZE];

for(i=0; i<ARRAY_SIZE; i++) {
    check(hipMalloc(&H[i], 4*100));
    check(hipMalloc(&I[i], 4*100));
    check(hipMalloc(&J[i], 4*100));
    check(hipMalloc(&K[i], 4*100));
}

	hipStream_t stream;
	check(hipStreamCreate(&stream));


	hipEvent_t start, sync, stop;
	check(hipEventCreate(&start));
	check(hipEventCreateWithFlags(&sync, hipEventBlockingSync));
	check(hipEventCreate(&stop));

    hipStream_t stream0 = 0;

    if (p_tests & 0x1) {
        hipEventRecord(start);
        hipLaunchKernel(NearlyNull, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, Ad);
        stopTest(start, stop, "FirstKernelLaunch", 1);

	hipEventRecord(start);
	hipLaunchKernel(NearlyNull, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[0], I[0], J[0], K[0], 1);
	hipLaunchKernel(NearlyNull2, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[1], I[1], J[1], K[1], 1);
	hipLaunchKernel(NearlyNull3, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[2], I[2], J[2], K[2], 1);
	hipLaunchKernel(NearlyNull4, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[3], I[3], J[3], K[3], 1);
	hipLaunchKernel(NearlyNull5, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[4], I[4], J[4], K[4], 1);
	hipLaunchKernel(NearlyNull6, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[5], I[5], J[5], K[5], 1);
	hipLaunchKernel(NearlyNull7, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[6], I[6], J[6], K[6], 1);
	hipLaunchKernel(NearlyNull8, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[7], I[7], J[7], K[7], 1);
	hipLaunchKernel(NearlyNull9, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[8], I[8], J[8], K[8], 1);
	hipLaunchKernel(NearlyNull10, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[9], I[9], J[9], K[9], 1);
	stopTest(start, stop, "MultiParamsFirstKernelLaunch", 1);
    }



    if (p_tests & 0x2) {
        hipEventRecord(start);
	for(int t=0; t<100; t++)
	{
        hipLaunchKernel(NearlyNull, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, Ad);
	}
        stopTest(start, stop, "SecondKernelLaunch", 1);

	hipEventRecord(start);
	for(int t=0; t<ARRAY_SIZE/10; t++) {
	hipLaunchKernel(NearlyNull, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[10*t+0], I[10*t+0], J[10*t+0], K[10*t+0], t%100);
	hipLaunchKernel(NearlyNull2, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[10*t+1], I[10*t+1], J[10*t+1], K[10*t+1], t%100);
	hipLaunchKernel(NearlyNull3, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[10*t+2], I[10*t+2], J[10*t+2], K[10*t+2], t%100);
	hipLaunchKernel(NearlyNull4, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[10*t+3], I[10*t+3], J[10*t+3], K[10*t+3], t%100);
	hipLaunchKernel(NearlyNull5, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[10*t+4], I[10*t+4], J[10*t+4], K[10*t+4], t%100);
	hipLaunchKernel(NearlyNull6, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[10*t+5], I[10*t+5], J[10*t+5], K[10*t+5], t%100);
	hipLaunchKernel(NearlyNull7, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[10*t+6], I[10*t+6], J[10*t+6], K[10*t+6], t%100);
	hipLaunchKernel(NearlyNull8, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[10*t+7], I[10*t+7], J[10*t+7], K[10*t+7], t%100);
	hipLaunchKernel(NearlyNull9, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[10*t+8], I[10*t+8], J[10*t+8], K[10*t+8], t%100);
	hipLaunchKernel(NearlyNull10, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, H[10*t+9], I[10*t+9], J[10*t+9], K[10*t+9], t%100);
	}
	stopTest(start, stop, "MultiParamsSecondKernelLaunch", 1);
    }

    if (p_tests & 0x4) {
        for (int t=0; t<TEST_ITERS; t++)  {
            hipEventRecord(start);
            for(int i=0;i<DISPATCHES_PER_TEST;i++){
                hipLaunchKernel(NearlyNull, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, Ad);
                hipEventRecord(sync);
                hipEventSynchronize(sync);
            }
            stopTest(start, stop, "NullStreamASyncDispatchWait", DISPATCHES_PER_TEST);
        }
    }


    if (p_tests & 0x10) {
        for (int t=0; t<TEST_ITERS; t++)  {
            hipEventRecord(start);
            for(int i=0;i<DISPATCHES_PER_TEST;i++){
                hipLaunchKernel(NearlyNull, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream, Ad);
                hipEventRecord(sync);
                hipEventSynchronize(sync);
            }
            stopTest(start, stop, "StreamASyncDispatchWait", DISPATCHES_PER_TEST);
        }
    }

#if 1

    if (p_tests & 0x40) {
        for (int t=0; t<TEST_ITERS; t++)  {
            hipEventRecord(start);
            for(int i=0;i<DISPATCHES_PER_TEST;i++){
                hipLaunchKernel(NearlyNull, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream0, Ad);
            }
            stopTest(start, stop, "NullStreamASyncDispatchNoWait", DISPATCHES_PER_TEST);
        }
    }

    if (p_tests & 0x80) {
        for (int t=0; t<TEST_ITERS; t++)  {
            hipEventRecord(start);
            for(int i=0;i<DISPATCHES_PER_TEST;i++){
                hipLaunchKernel(NearlyNull, dim3(NUM_GROUPS), dim3(GROUP_SIZE), 0, stream, Ad);
            }
            stopTest(start, stop, "StreamASyncDispatchNoWait", DISPATCHES_PER_TEST);
        }
    }
#endif
    resultDB.DumpSummary(std::cout);


	check(hipEventDestroy(start));
	check(hipEventDestroy(sync));
	check(hipEventDestroy(stop));
}
