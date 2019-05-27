all: ByPass.o
	g++ -o ByPass ByPass.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL

ByPass.o: ByPass.cpp
	g++ -std=c++11 -I//opt/rocm/opencl/include/ -Wdeprecated-declarations -c ByPass.cpp -o ByPass.o

.PHONY:clean
clean:
	rm *.o *.bin ByPass
