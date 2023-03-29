CXX=g++
# CXXFLAGS=-mavx512f -std=c++14 -O2 -g -fopenmp -fpermissive
CXXFLAGS=-march=native -std=c++14 -O2 -g -fopenmp -fpermissive
objs = prefetch_x load_x load_amx store_amx int8_amx load_noavx prefetch_pure prefetch_l1 prefetch_l2 prefetch_l3 copy_simd store_pure triadd load_512

all : $(objs)

clean :
	rm -f $(objs)
