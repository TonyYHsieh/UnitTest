all: OpenCLDispatchLatency.o
	g++ -o OpenCLDispatchLatency OpenCLDispatchLatency.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL

OpenCLDispatchLatency.o: OpenCLDispatchLatency.cpp
	g++ -std=c++11 -I/opt/rocm/opencl/include/ -Wdeprecated-declarations -c OpenCLDispatchLatency.cpp -o OpenCLDispatchLatency.o

.PHONY:clean
clean:
	rm *.o OpenCLDispatchLatency
