#include <immintrin.h>
#include <cstdlib>
#include <cstring>

inline void prefetchs(char *d, const char *p, size_t sz) {
  constexpr int instr = 8;
  asm volatile ("xor %%r8, %%r8":::"r8");

// # pragma omp parallel for
  for (size_t i = 0; i < sz/instr/64; i ++) {
    auto *index = p + i * instr * 64;
    auto *dst = p + i * instr * 64;
    asm volatile (
      "add (%0), %%r8;"
      "add 64(%0), %%r8;"
      "add 2 * 64(%0), %%r8;"
      "add 3 * 64(%0), %%r8;"
      "add 4 * 64(%0), %%r8;"
      "add 5 * 64(%0), %%r8;"
      "add 6 * 64(%0), %%r8;"
      "add 7 * 64(%0), %%r8;"
      // "add %%r8, (%0)"
      // "vmovups %%zmm31, (%1);"
      : : "r" (index), "r" (dst)
      : "r8");
  }

  asm volatile ("mov %%r8, (%0)"::"r"(d):"r8");
}

int main() {
  constexpr size_t size = 0x40000000ull;
  void *p, *d;
  posix_memalign(&p, 64, size);
  posix_memalign(&d, 64, size/32);

  std::memset(p, 1, size);

  constexpr int times = 1024;
  for (int i =0; i < times; i ++)
    prefetchs((char *)d, (const char*)p, size);
}
