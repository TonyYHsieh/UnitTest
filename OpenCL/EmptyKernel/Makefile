all: EmptyKernel.o
	g++ -o EmptyKernel EmptyKernel.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL

EmptyKernel.o: EmptyKernel.cpp
	g++ -std=c++11 -I//opt/rocm/opencl/include/ -Wdeprecated-declarations -c EmptyKernel.cpp -o EmptyKernel.o

.PHONY:clean
clean:
	rm *.o EmptyKernel 
