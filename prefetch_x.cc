#include <immintrin.h>
#include <cstdlib>
#include <cstring>

template <int level>
inline void prefetchs(const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz; i += 64 * 32) {
#   pragma unroll (32)
    for (int j = 0; j < 32; j ++ )
      _mm_prefetch(p + i + j * 64, level);
  }
}

int main() {
  constexpr size_t size = 0x40000000ull;
  void *p;
  posix_memalign(&p, 64, size);
  memset(p, 1, size);

  constexpr int times = 1;
  for (int i =0; i < times; i ++)
    prefetchs<0>((const char*)p, size);
}
