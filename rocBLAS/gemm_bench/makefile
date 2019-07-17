all:

roc:
	hipcc -std=c++14 -I/opt/rocm/include  -L/opt/rocm/rocblas/lib  -lrocblas -L/usr/lib64/atlas/ -DUSE_ROCBLAS -ltatlas -o gemm gemm.cpp
cu:
	nvcc --std c++14 -m64 -lcublas -lcblas -DUSE_CUBLAS -o gemm gemm.cpp
clean:
	rm -f gemm
