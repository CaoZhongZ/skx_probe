#include <immintrin.h>
#include <cstdlib>
#include <cstring>

inline void prefetchs_s(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/2048; ++ i) {
	auto *index = p + i * 2048;
	auto *dst = d + i * 2048;
	auto a = 2048;
  	asm volatile ("rep movsb"
		  : "+D" (dst), "+S" (index), "+c" (a)
                  :
                  : "memory");
  }
}

inline void prefetchs_y(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/32/32; i ++) {
    auto *index = p + i * 32 * 32;
    auto *dst = d + i * 32 * 32;
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
      "vmovups %%ymm0, (%1)      ;"
      "vmovups %%ymm1, 32(%1)    ;"
      "vmovups %%ymm2, 2 * 32(%1);"
      "vmovups %%ymm3, 3 * 32(%1);"
      "vmovups %%ymm4, 4 * 32(%1);"
      "vmovups %%ymm5, 5 * 32(%1);"
      "vmovups %%ymm6, 6 * 32(%1);"
      "vmovups %%ymm7, 7 * 32(%1);"
      "vmovups %%ymm8, 8 * 32(%1);"
      "vmovups %%ymm9, 9 * 32(%1);"
      "vmovups %%ymm10, 10* 32(%1);"
      "vmovups %%ymm11, 11* 32(%1);"
      "vmovups %%ymm12, 12* 32(%1);"
      "vmovups %%ymm13, 13* 32(%1);"
      "vmovups %%ymm14, 14* 32(%1);"
      "vmovups %%ymm15, 15* 32(%1);"
      "vmovups %%ymm16, 16 *32(%1);"
      "vmovups %%ymm17, 17 *32(%1);"
      "vmovups %%ymm18,18 * 32(%1);"
      "vmovups %%ymm19,19 * 32(%1);"
      "vmovups %%ymm20,20 * 32(%1);"
      "vmovups %%ymm21,21 * 32(%1);"
      "vmovups %%ymm22,22 * 32(%1);"
      "vmovups %%ymm23,23 * 32(%1);"
      "vmovups %%ymm24,24 * 32(%1);"
      "vmovups %%ymm25,25 * 32(%1);"
      "vmovups %%ymm26, 26* 32(%1);"
      "vmovups %%ymm27, 27* 32(%1);"
      "vmovups %%ymm28, 28* 32(%1);"
      "vmovups %%ymm29, 29* 32(%1);"
      "vmovups %%ymm30, 30* 32(%1);"
      "vmovups %%ymm31, 31* 32(%1);"
      : : "r" (index), "r" (dst)
      : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10"
     , "zmm11", "zmm12", "zmm13", "zmm14", "zmm15", "zmm16", "zmm17", "zmm18", "zmm19", "zmm20"
     , "zmm21", "zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27"
     , "zmm28", "zmm29", "zmm30", "zmm31");
  }
}


inline void prefetchs(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/32/64; i ++) {
    auto *index = p + i * 32 * 64;
    auto *dst = d + i * 32 * 64;
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
      "vmovups %%zmm0, (%1)      ;"
      "vmovups %%zmm1, 64(%1)    ;"
      "vmovups %%zmm2, 2 * 64(%1);"
      "vmovups %%zmm3, 3 * 64(%1);"
      "vmovups %%zmm4, 4 * 64(%1);"
      "vmovups %%zmm5, 5 * 64(%1);"
      "vmovups %%zmm6, 6 * 64(%1);"
      "vmovups %%zmm7, 7 * 64(%1);"
      "vmovups %%zmm8, 8 * 64(%1);"
      "vmovups %%zmm9, 9 * 64(%1);"
      "vmovups %%zmm10, 10* 64(%1);"
      "vmovups %%zmm11, 11* 64(%1);"
      "vmovups %%zmm12, 12* 64(%1);"
      "vmovups %%zmm13, 13* 64(%1);"
      "vmovups %%zmm14, 14* 64(%1);"
      "vmovups %%zmm15, 15* 64(%1);"
      "vmovups %%zmm16, 16 *64(%1);"
      "vmovups %%zmm17, 17 *64(%1);"
      "vmovups %%zmm18,18 * 64(%1);"
      "vmovups %%zmm19,19 * 64(%1);"
      "vmovups %%zmm20,20 * 64(%1);"
      "vmovups %%zmm21,21 * 64(%1);"
      "vmovups %%zmm22,22 * 64(%1);"
      "vmovups %%zmm23,23 * 64(%1);"
      "vmovups %%zmm24,24 * 64(%1);"
      "vmovups %%zmm25,25 * 64(%1);"
      "vmovups %%zmm26, 26* 64(%1);"
      "vmovups %%zmm27, 27* 64(%1);"
      "vmovups %%zmm28, 28* 64(%1);"
      "vmovups %%zmm29, 29* 64(%1);"
      "vmovups %%zmm30, 30* 64(%1);"
      "vmovups %%zmm31, 31* 64(%1);"
      : : "r" (index), "r" (dst)
      : "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10"
     , "zmm11", "zmm12", "zmm13", "zmm14", "zmm15", "zmm16", "zmm17", "zmm18", "zmm19", "zmm20"
     , "zmm21", "zmm22", "zmm23", "zmm24", "zmm25", "zmm26", "zmm27"
     , "zmm28", "zmm29", "zmm30", "zmm31");
  }
}

int main() {
  constexpr size_t size = 256 * 1024 * 1024ull;
  void *p, *d;
  auto ret = posix_memalign(&p, 64, size);
  ret = posix_memalign(&d, 64, size);
  memset(p, 1, size);

  constexpr int times = 2000 * 100;
  for (int i =0; i < times; i ++)
    prefetchs_s((char *)d, (const char*)p, size);
}
