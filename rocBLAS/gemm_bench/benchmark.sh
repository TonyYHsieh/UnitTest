arch=amd
EXE=./gemm
#$(EXE) iter_num side|uplo|trans|diag nb start end
iter_num=20
begin=3072
end=44032
step=640  #for 16GB card

# NN test
echo -e "----- NN testing lda = m -----"
for((m=begin; m<=$end; m+=step));
do
    $EXE d $m 1792 256 n t $m 1792 $m 1 0 $iter_num
done

