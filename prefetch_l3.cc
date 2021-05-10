#include <immintrin.h>
#include <cstdlib>
#include <cstring>

inline void prefetchs(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/32/64; i ++) {
    auto *index = p + i * 32 * 64;
    asm volatile (
      "prefetcht2 (%0);"
      "prefetcht2 64(%0);"
      "prefetcht2 2 * 64(%0);"
      "prefetcht2 3 * 64(%0);"
      "prefetcht2 4 * 64(%0);"
      "prefetcht2 5 * 64(%0);"
      "prefetcht2 6 * 64(%0);"
      "prefetcht2 7 * 64(%0);"
      "prefetcht2 8 * 64(%0);"
      "prefetcht2 9 * 64(%0);"
      "prefetcht2 10* 64(%0);"
      "prefetcht2 11* 64(%0);"
      "prefetcht2 12* 64(%0);"
      "prefetcht2 13* 64(%0);"
      "prefetcht2 14* 64(%0);"
      "prefetcht2 15* 64(%0);"
      "prefetcht2 16 *64(%0);"
      "prefetcht2 17 *64(%0);"
      "prefetcht2 18 * 64(%0);"
      "prefetcht2 19 * 64(%0);"
      "prefetcht2 20 * 64(%0);"
      "prefetcht2 21 * 64(%0);"
      "prefetcht2 22 * 64(%0);"
      "prefetcht2 23 * 64(%0);"
      "prefetcht2 24 * 64(%0);"
      "prefetcht2 25 * 64(%0);"
      "prefetcht2 26* 64(%0);"
      "prefetcht2 27* 64(%0);"
      "prefetcht2 28* 64(%0);"
      "prefetcht2 29* 64(%0);"
      "prefetcht2 30* 64(%0);"
      "prefetcht2 31* 64(%0);"
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
