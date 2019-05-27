all: EltwiseSample.o
	g++ -o EltwiseSample EltwiseSample.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL

EltwiseSample.o: EltwiseSample.cpp
	g++ -std=c++11 -I/opt/rocm/opencl/include/ -Wdeprecated-declarations -c EltwiseSample.cpp -o EltwiseSample.o

.PHONY:clean
clean:
	rm *.o EltwiseSample 
