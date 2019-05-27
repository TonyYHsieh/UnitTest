all: HelloWorld.o
	g++ -o HelloWorld HelloWorld.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL

HelloWorld.o: HelloWorld.cpp
	g++ -std=c++11 -I//opt/rocm/opencl/include/ -Wdeprecated-declarations -c HelloWorld.cpp -o HelloWorld.o

.PHONY:clean
clean:
	rm *.o HelloWorld
