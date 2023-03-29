#include <immintrin.h>
#include <cstdlib>
#include <cstring>


inline void prefetchs_y(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/16/64; i ++) {
    auto *index = p + i * 16 * 64;
    asm volatile (
      "vmovdqu8 %%zmm0, (%0);"
      "vmovdqu8 %%zmm1, 64(%0);"
      "vmovdqu8 %%zmm2, 2 * 64(%0);"
      "vmovdqu8 %%zmm3, 3 * 64(%0);"
      "vmovdqu8 %%zmm4, 4 * 64(%0);"
      "vmovdqu8 %%zmm5, 5 * 64(%0);"
      "vmovdqu8 %%zmm6, 6 * 64(%0);"
      "vmovdqu8 %%zmm7, 7 * 64(%0);"
      "vmovdqu8 %%zmm8, 8 * 64(%0);"
      "vmovdqu8 %%zmm9, 9 * 64(%0);"
      "vmovdqu8 %%zmm10, 10 * 64(%0);"
      "vmovdqu8 %%zmm11, 11 * 64(%0);"
      "vmovdqu8 %%zmm12, 12 * 64(%0);"
      "vmovdqu8 %%zmm13, 13 * 64(%0);"
      "vmovdqu8 %%zmm14, 14 * 64(%0);"
      "vmovdqu8 %%zmm15, 15 * 64(%0);"
      : : "r" (index)/*, "r" (dst)*/);
  }
}


int main() {
  constexpr size_t size = 0x40000000ull;
  void *p, *d;
  posix_memalign(&p, 64, size);
  posix_memalign(&d, 64, size/32);
  memset(p, 1, size);

  constexpr int times = 1024;
  for (int i =0; i < times; i ++)
    prefetchs_y((char *)d, (const char*)p, size);
}
