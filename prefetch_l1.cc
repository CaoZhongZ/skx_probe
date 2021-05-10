#include <immintrin.h>
#include <cstdlib>
#include <cstring>

inline void prefetchs(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/32/64; i ++) {
    auto *index = p + i * 32 * 64;
    asm volatile (
      "prefetcht0 (%0);"
      "prefetcht0 64(%0);"
      "prefetcht0 2 * 64(%0);"
      "prefetcht0 3 * 64(%0);"
      "prefetcht0 4 * 64(%0);"
      "prefetcht0 5 * 64(%0);"
      "prefetcht0 6 * 64(%0);"
      "prefetcht0 7 * 64(%0);"
      "prefetcht0 8 * 64(%0);"
      "prefetcht0 9 * 64(%0);"
      "prefetcht0 10* 64(%0);"
      "prefetcht0 11* 64(%0);"
      "prefetcht0 12* 64(%0);"
      "prefetcht0 13* 64(%0);"
      "prefetcht0 14* 64(%0);"
      "prefetcht0 15* 64(%0);"
      "prefetcht0 16 *64(%0);"
      "prefetcht0 17 *64(%0);"
      "prefetcht0 18 * 64(%0);"
      "prefetcht0 19 * 64(%0);"
      "prefetcht0 20 * 64(%0);"
      "prefetcht0 21 * 64(%0);"
      "prefetcht0 22 * 64(%0);"
      "prefetcht0 23 * 64(%0);"
      "prefetcht0 24 * 64(%0);"
      "prefetcht0 25 * 64(%0);"
      "prefetcht0 26* 64(%0);"
      "prefetcht0 27* 64(%0);"
      "prefetcht0 28* 64(%0);"
      "prefetcht0 29* 64(%0);"
      "prefetcht0 30* 64(%0);"
      "prefetcht0 31* 64(%0);"
      : : "r" (index)/*, "r" (dst)*/);
  }
}

int main() {
  constexpr size_t size = 0x40000000ull;
  void *p, *d;
  posix_memalign(&p, 64, size);
  posix_memalign(&d, 64, size/32);
  memset(p, 1, size);

//  constexpr int times = 1024;
//  for (int i =0; i < times; i ++)
    prefetchs((char *)d, (const char*)p, size);
}
