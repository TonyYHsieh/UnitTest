all: krun.o
	g++ -o krun krun.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL -lblas

krun.o: krun.cpp
	g++ -std=c++14 -I/opt/rocm/opencl/include/ -c krun.cpp -o krun.o

.PHONY:clean
clean:
	rm *.o krun
