# OMP_NUM_THREADS=1 ./load_amx
OMP_NUM_THREADS=56 KMP_HW_SUBSET=1s,56c,1t KMP_AFFINITY=compact,granularity=fine ./store_amx
