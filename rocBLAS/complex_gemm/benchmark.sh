arch=amd
EXE=./amd_complex_gemm
#$(EXE) iter_num side|uplo|trans|diag nb start end
iter_num=20
begin=65536
step=2048  #for 16GB card

# NN test
echo -e "----- NN testing lda = m -----"
format=LNU
for((m=begin; m>0; m-=step));
do
    $EXE c 512 512 $m n n 512 $m 512 1 0 $iter_num
done

