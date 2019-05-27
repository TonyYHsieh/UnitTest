all:
	/opt/rocm/hip/bin/hipcc -I/opt/rocm/include  -L/opt/rocm/rocblas/lib  -lrocblas -L/usr/lib64/atlas/ -ltatlas -o amd_trsv amd_trsv.cpp

clean:
	rm -f amd_trsv
