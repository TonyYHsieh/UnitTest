arch=amd
EXE=amd_trsm_reduce
#$(EXE) iter_num side|uplo|trans|diag nb start end
iter_num=4
nb=384
upperBound=44928  #for 16GB card
# NN test
format=LLNU
./amd_trsm_reduce ${iter_num} ${format} $nb $nb $upperBound 
# NT test
format=RLTU
./amd_trsm_reduce ${iter_num} ${format} $nb $nb $upperBound
