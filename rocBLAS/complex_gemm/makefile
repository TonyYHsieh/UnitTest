all:
	/opt/rocm/hip/bin/hipcc -std=c++14 -I/opt/rocm/include  -L/opt/rocm/rocblas/lib  -lrocblas -L/usr/lib64/atlas/ -ltatlas -D_ROCBLAS_INTERNAL_COMPLEX_ -o amd_complex_gemm amd_complex_gemm.cpp

clean:
	rm -f amd_complex_gemm
