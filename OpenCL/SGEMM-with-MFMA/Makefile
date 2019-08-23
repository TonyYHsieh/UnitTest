all: sgemm.o
	g++ -o sgemm sgemm.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL -lblas

sgemm.o: sgemm.cpp
	g++ -std=c++14 -I/opt/rocm/opencl/include/ -c sgemm.cpp -o sgemm.o

.PHONY:clean
clean:
	rm *.o sgemm
