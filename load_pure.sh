# OMP_NUM_THREADS=1 gdb ./load_pure
OMP_NUM_THREADS=40 KMP_HW_SUBSET=1s,40c,1t KMP_AFFINITY=compact,granularity=fine numactl -p 3 ./load_pure
