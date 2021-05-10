#include <immintrin.h>
#include <cstdlib>

inline void prefetchs(char *d, const char *p, size_t sz) {
    constexpr int instr = 8;
# pragma omp parallel for
  for (size_t i = 0; i < sz/instr/64; i ++) {
    auto *index = p + i * instr * 64;
    auto *dst = p + i * instr * 64;
    asm volatile (
      "add %%r8, (%0);"
      "mov %%r9, 64(%0) ;"
      "mov %%r10, 2 * 64(%0);"
      "mov %%r11, 3 * 64(%0);"
      "mov %%r12, 4 * 64(%0);"
      "mov %%r13, 5 * 64(%0);"
      "mov %%r14, 6 * 64(%0);"
      "mov %%r15, 7 * 64(%0);"
      // "vmovups %%zmm31, (%1);"
      : : "r" (index), "r" (dst)
      : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
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
