all: KernelInterval.o
	g++ -o KernelInterval KernelInterval.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL

KernelInterval.o: KernelInterval.cpp
	g++ -std=c++11 -I/opt/rocm/opencl/include/ -Wdeprecated-declarations -c KernelInterval.cpp -o KernelInterval.o

.PHONY:clean
clean:
	rm *.o KernelInterval 
