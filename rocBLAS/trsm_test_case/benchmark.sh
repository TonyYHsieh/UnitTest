arch=amd

EXE=./${arch}_trsm
#$(EXE) iter_num side|uplo|trans|diag m n

iter_num=4
format=LLNU
nb=384
upperBound=45000

# NN test
for((m=upperBound; m>0; m-=nb));
do
    ${EXE} ${iter_num} ${format} $nb $m
done


# NT test
format=RLTU
for((m=upperBound; m>0; m-=nb));
do
    ${EXE} ${iter_num} ${format} $nb $m
done
