all: VectorAdd.o
	g++ -o VectorAdd VectorAdd.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL

VectorAdd.o: VectorAdd.cpp
	g++ -std=c++11 -I//opt/rocm/opencl/include/ -Wdeprecated-declarations -c VectorAdd.cpp -o VectorAdd.o

.PHONY:clean
clean:
	rm *.o *.bin VectorAdd
