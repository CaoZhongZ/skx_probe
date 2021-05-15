CXX=icpc
# CXXFLAGS=-mavx512f -std=c++14 -O2 -g -fopenmp -fpermissive
CXXFLAGS=-xCORE-AVX512 -std=c++14 -O2 -g -fopenmp -fpermissive -qopt-zmm-usage=high
objs = prefetch_x load_x load_pure load_noavx prefetch_pure prefetch_l1 prefetch_l2 prefetch_l3 copy_simd store_pure triadd

all : $(objs)

clean :
	rm -f $(objs)
