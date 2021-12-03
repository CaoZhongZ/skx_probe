# OMP_NUM_THREADS=1 gdb ./load_pure
OMP_NUM_THREADS=28 KMP_HW_SUBSET=1s,28c,1t KMP_AFFINITY=compact,granularity=fine ./load_pure
