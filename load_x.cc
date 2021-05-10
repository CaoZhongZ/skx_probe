#include <immintrin.h>
#include <cstdlib>
#include <cstring>

inline void prefetchs(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz; i += 32 * 64) {
#   pragma unroll (32)
    for (int j = 0; j < 32; j ++) {
      auto volatile x = _mm512_load_ps(p + i + j * 64);
    }
//
//#   pragma unroll
//    for (int j = 0; j < 32; j ++)
//      _mm512_store_ps((void *)(p + i + j * 64), x[j]);
  }
}

int main() {
  constexpr size_t size = 0x40000000ull;
  void *p, *d;
  posix_memalign(&p, 64, size);
  posix_memalign(&d, 64, size);
  memset(p, 1, size);

  constexpr int times = 1024;
  for (int i =0; i < times; i ++)
    prefetchs((char *)d, (const char*)p, size);
}
