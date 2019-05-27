arch=amd
EXE=./amd_trsv
#$(EXE) iter_num side|uplo|trans|diag nb start end
iter_num=20
begin=512
step=20  #for 16GB card

# NN test
echo -e "----- LNN testing lda = m -----"
format=LNU
for((m=begin; m>0; m-=step));
do
    $EXE ${iter_num} ${format} $m $m 1
done

echo -e "\n\n----- LNN testing lda = 45056 -----"
format=LNU
for((m=begin; m>0; m-=step));
do
    $EXE ${iter_num} ${format} $m 45056 1
done

# NT test
#format=RLTU
#$EXE ${iter_num} ${format} $nb $nb $upperBound
