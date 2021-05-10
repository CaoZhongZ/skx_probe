#include <immintrin.h>
#include <cstdlib>
#include <cstring>

inline void prefetchs(char *d, const char *p, size_t sz) {
// # pragma omp parallel for
  for (size_t i = 0; i < sz/32/64; i ++) {
    // accessing same 32 address see L1 and load buffer counts
    auto *index = p /* + i * 32 * 64*/;
    asm volatile (
      "vmovups (%0), %%zmm0;"
      "vmovups (%0), %%zmm1;"
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
  constexpr size_t size = 0x40000000ull;
  void *p, *d;
  posix_memalign(&p, 64, size);
  posix_memalign(&d, 64, size/32);
  memset(p, 1, size);

  constexpr int times = 1024;
  for (int i =0; i < times; i ++)
    prefetchs((char *)d, (const char*)p, size);
}
