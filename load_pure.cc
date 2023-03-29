#include <immintrin.h>
#include <cstdlib>
#include <cstring>

inline void prefetchs_x(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/32/16; i ++) {
    auto *index = p + i * 32 * 16;
    asm volatile (
      "vmovups (%0), %%xmm0;"
      "vmovups 16(%0), %%xmm1;"
      "vmovups 2 * 16(%0), %%xmm2;"
      "vmovups 3 * 16(%0), %%xmm3;"
      "vmovups 4 * 16(%0), %%xmm4;"
      "vmovups 5 * 16(%0), %%xmm5;"
      "vmovups 6 * 16(%0), %%xmm6;"
      "vmovups 7 * 16(%0), %%xmm7;"
      "vmovups 8 * 16(%0), %%xmm8;"
      "vmovups 9 * 16(%0), %%xmm9;"
      "vmovups 10* 16(%0), %%xmm10;"
      "vmovups 11* 16(%0), %%xmm11;"
      "vmovups 12* 16(%0), %%xmm12;"
      "vmovups 13* 16(%0), %%xmm13;"
      "vmovups 14* 16(%0), %%xmm14;"
      "vmovups 15* 16(%0), %%xmm15;"
      "vmovups 16 *16(%0), %%xmm16;"
      "vmovups 17 *16(%0), %%xmm17;"
      "vmovups 18 * 16(%0), %%xmm18;"
      "vmovups 19 * 16(%0), %%xmm19;"
      "vmovups 20 * 16(%0), %%xmm20;"
      "vmovups 21 * 16(%0), %%xmm21;"
      "vmovups 22 * 16(%0), %%xmm22;"
      "vmovups 23 * 16(%0), %%xmm23;"
      "vmovups 24 * 16(%0), %%xmm24;"
      "vmovups 25 * 16(%0), %%xmm25;"
      "vmovups 26* 16(%0), %%xmm26;"
      "vmovups 27* 16(%0), %%xmm27;"
      "vmovups 28* 16(%0), %%xmm28;"
      "vmovups 29* 16(%0), %%xmm29;"
      "vmovups 30* 16(%0), %%xmm30;"
      "vmovups 31* 16(%0), %%xmm31;"
      : : "r" (index)/*, "r" (dst)*/
      : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10"
     , "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20"
     , "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27"
     , "xmm28", "xmm29", "xmm30", "xmm31");
  }
}

inline void prefetchs_y(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/32/32; i ++) {
    auto *index = p + i * 32 * 32;
    asm volatile (
      "vmovups (%0), %%ymm0;"
      "vmovups 32(%0), %%ymm1;"
      "vmovups 2 * 32(%0), %%ymm2;"
      "vmovups 3 * 32(%0), %%ymm3;"
      "vmovups 4 * 32(%0), %%ymm4;"
      "vmovups 5 * 32(%0), %%ymm5;"
      "vmovups 6 * 32(%0), %%ymm6;"
      "vmovups 7 * 32(%0), %%ymm7;"
      "vmovups 8 * 32(%0), %%ymm8;"
      "vmovups 9 * 32(%0), %%ymm9;"
      "vmovups 10* 32(%0), %%ymm10;"
      "vmovups 11* 32(%0), %%ymm11;"
      "vmovups 12* 32(%0), %%ymm12;"
      "vmovups 13* 32(%0), %%ymm13;"
      "vmovups 14* 32(%0), %%ymm14;"
      "vmovups 15* 32(%0), %%ymm15;"
      "vmovups 16 *32(%0), %%ymm16;"
      "vmovups 17 *32(%0), %%ymm17;"
      "vmovups 18 * 32(%0), %%ymm18;"
      "vmovups 19 * 32(%0), %%ymm19;"
      "vmovups 20 * 32(%0), %%ymm20;"
      "vmovups 21 * 32(%0), %%ymm21;"
      "vmovups 22 * 32(%0), %%ymm22;"
      "vmovups 23 * 32(%0), %%ymm23;"
      "vmovups 24 * 32(%0), %%ymm24;"
      "vmovups 25 * 32(%0), %%ymm25;"
      "vmovups 26* 32(%0), %%ymm26;"
      "vmovups 27* 32(%0), %%ymm27;"
      "vmovups 28* 32(%0), %%ymm28;"
      "vmovups 29* 32(%0), %%ymm29;"
      "vmovups 30* 32(%0), %%ymm30;"
      "vmovups 31* 32(%0), %%ymm31;"
      : : "r" (index)/*, "r" (dst)*/
      : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10"
     , "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "xmm16", "xmm17", "xmm18", "xmm19", "xmm20"
     , "xmm21", "xmm22", "xmm23", "xmm24", "xmm25", "xmm26", "xmm27"
     , "xmm28", "xmm29", "xmm30", "xmm31");
  }
}


inline void prefetchs(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/32/64; i ++) {
    auto *index = p + i * 32 * 64;
    asm volatile (
      "vmovups (%0), %%zmm0;"
      "vmovups 64(%0), %%zmm1;"
      "vmovups 2 * 64(%0), %%zmm2;"
      "vmovups 3 * 64(%0), %%zmm3;"
      "vmovups 4 * 64(%0), %%zmm4;"
      "vmovups 5 * 64(%0), %%zmm5;"
      "vmovups 6 * 64(%0), %%zmm6;"
      "vmovups 7 * 64(%0), %%zmm7;"
      "vmovups 8 * 64(%0), %%zmm8;"
      "vmovups 9 * 64(%0), %%zmm9;"
      "vmovups 10* 64(%0), %%zmm10;"
      "vmovups 11* 64(%0), %%zmm11;"
      "vmovups 12* 64(%0), %%zmm12;"
      "vmovups 13* 64(%0), %%zmm13;"
      "vmovups 14* 64(%0), %%zmm14;"
      "vmovups 15* 64(%0), %%zmm15;"
      "vmovups 16 *64(%0), %%zmm16;"
      "vmovups 17 *64(%0), %%zmm17;"
      "vmovups 18 * 64(%0), %%zmm18;"
      "vmovups 19 * 64(%0), %%zmm19;"
      "vmovups 20 * 64(%0), %%zmm20;"
      "vmovups 21 * 64(%0), %%zmm21;"
      "vmovups 22 * 64(%0), %%zmm22;"
      "vmovups 23 * 64(%0), %%zmm23;"
      "vmovups 24 * 64(%0), %%zmm24;"
      "vmovups 25 * 64(%0), %%zmm25;"
      "vmovups 26* 64(%0), %%zmm26;"
      "vmovups 27* 64(%0), %%zmm27;"
      "vmovups 28* 64(%0), %%zmm28;"
      "vmovups 29* 64(%0), %%zmm29;"
      "vmovups 30* 64(%0), %%zmm30;"
      "vmovups 31* 64(%0), %%zmm31;"
      : : "r" (index)/*, "r" (dst)*/
      : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10"
     , "zmm11", "zmm12", "zmm13", "zmm14", "zmm15", "zmm16", "zmm17", "zmm18", "zmm19", "zmm20"
     , "zmm21", "zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27"
     , "zmm28", "zmm29", "zmm30", "zmm31");
  }
}

int main() {
  constexpr size_t size = 68719476736ull;
  void *p, *d;
  posix_memalign(&p, 64, size);
  posix_memalign(&d, 64, size/32);
  memset(p, 1, size);

  constexpr int times = 1024*18;
  for (int i =0; i < times; i ++)
    prefetchs_y((char *)d, (const char*)p, size);
}
