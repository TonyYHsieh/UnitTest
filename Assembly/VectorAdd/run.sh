#/opt/rocm/opencl/bin/x86_64/clang  -x assembler -target amdgcn--amdhsa  -mcpu=gfx803 - -o kernel.bin < _temp_0_fiji.s
make clean
make
./VectorAdd
