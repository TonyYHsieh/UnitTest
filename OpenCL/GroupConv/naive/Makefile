all: GroupConv.o
	g++ -o GroupConv GroupConv.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL

GroupConv.o: GroupConv.cpp
	g++ -std=c++11 -I//opt/rocm/opencl/include/ -Wdeprecated-declarations -c GroupConv.cpp -o GroupConv.o

.PHONY:clean
clean:
	rm *.o GroupConv
