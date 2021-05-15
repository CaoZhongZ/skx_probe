#include <immintrin.h>
#include <cstdlib>
#include <cstring>

inline void prefetchs_s(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/2048; ++ i) {
	auto *index = p + i * 2048;
	auto *dst = d + i * 2048;
	auto count = 2048;
  asm volatile("rep stosb" :
               "=D" (index), "=c" (count) :
               "0" (index), "1" (count), "a" (0) :
               "memory", "cc");
  }
}

inline void prefetchs(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/32/64; i ++) {
    auto *index = p + i * 32 * 64;
    asm volatile (
      "vmovaps %%zmm0, (%0)      ;"
      "vmovaps %%zmm1, 64(%0)    ;"
      "vmovaps %%zmm2, 2 * 64(%0);"
      "vmovaps %%zmm3, 3 * 64(%0);"
      "vmovaps %%zmm4, 4 * 64(%0);"
      "vmovaps %%zmm5, 5 * 64(%0);"
      "vmovaps %%zmm6, 6 * 64(%0);"
      "vmovaps %%zmm7, 7 * 64(%0);"
      "vmovaps %%zmm8, 8 * 64(%0);"
      "vmovaps %%zmm9, 9 * 64(%0);"
      "vmovaps %%zmm10, 10* 64(%0);"
      "vmovaps %%zmm11, 11* 64(%0);"
      "vmovaps %%zmm12, 12* 64(%0);"
      "vmovaps %%zmm13, 13* 64(%0);"
      "vmovaps %%zmm14, 14* 64(%0);"
      "vmovaps %%zmm15, 15* 64(%0);"
      "vmovaps %%zmm16, 16 *64(%0);"
      "vmovaps %%zmm17, 17 *64(%0);"
      "vmovaps %%zmm18,18 * 64(%0);"
      "vmovaps %%zmm19,19 * 64(%0);"
      "vmovaps %%zmm20,20 * 64(%0);"
      "vmovaps %%zmm21,21 * 64(%0);"
      "vmovaps %%zmm22,22 * 64(%0);"
      "vmovaps %%zmm23,23 * 64(%0);"
      "vmovaps %%zmm24,24 * 64(%0);"
      "vmovaps %%zmm25,25 * 64(%0);"
      "vmovaps %%zmm26, 26* 64(%0);"
      "vmovaps %%zmm27, 27* 64(%0);"
      "vmovaps %%zmm28, 28* 64(%0);"
      "vmovaps %%zmm29, 29* 64(%0);"
      "vmovaps %%zmm30, 30* 64(%0);"
      "vmovaps %%zmm31, 31* 64(%0);"
      : : "r" (index)/*, "r" (dst)*/
      : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10"
     , "zmm11", "zmm12", "zmm13", "zmm14", "zmm15", "zmm16", "zmm17", "zmm18", "zmm19", "zmm20"
     , "zmm21", "zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27"
     , "zmm28", "zmm29", "zmm30", "zmm31");
  }
}

int main() {
  constexpr size_t size = 512 * 1024 * 1024ull;
  void *p, *d;
  auto ret = posix_memalign(&p, 64, size);
  ret = posix_memalign(&d, 64, size);
  memset(p, 1, size);

  constexpr int times = 2000 * 100;
  for (int i =0; i < times; i ++)
    prefetchs((char *)d, (const char*)p, size);
}
