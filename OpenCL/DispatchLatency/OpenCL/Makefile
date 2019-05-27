all: OpenCLDispatchLatency.o CLProfiler.o ResultDatabase.o
	g++ -o OpenCLDispatchLatency OpenCLDispatchLatency.o CLProfiler.o ResultDatabase.o -L/opt/rocm/opencl/lib/x86_64/ -lOpenCL

OpenCLDispatchLatency.o: OpenCLDispatchLatency.cpp
	g++ -std=c++11 -I/opt/rocm/opencl/include/ -Wdeprecated-declarations -c OpenCLDispatchLatency.cpp -o OpenCLDispatchLatency.o

CLProfiler.o: CLProfiler.cpp
	g++ -std=c++11 -I/opt/rocm/opencl/include/ -Wdeprecated-declarations -c CLProfiler.cpp -o CLProfiler.o

ResultDatabase.o: ResultDatabase.cpp
	g++ -std=c++11 -c ResultDatabase.cpp -o ResultDatabase.o

.PHONY:clean
clean:
	rm *.o OpenCLDispatchLatency
