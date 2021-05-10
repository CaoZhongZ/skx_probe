#include <immintrin.h>
#include <cstdlib>
#include <cstring>

inline void prefetchs(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/32/64; i ++) {
    auto *index = p + i * 32 * 64;
    asm volatile (
      "prefetcht1 (%0);"
      "prefetcht1 64(%0);"
      "prefetcht1 2 * 64(%0);"
      "prefetcht1 3 * 64(%0);"
      "prefetcht1 4 * 64(%0);"
      "prefetcht1 5 * 64(%0);"
      "prefetcht1 6 * 64(%0);"
      "prefetcht1 7 * 64(%0);"
      "prefetcht1 8 * 64(%0);"
      "prefetcht1 9 * 64(%0);"
      "prefetcht1 10* 64(%0);"
      "prefetcht1 11* 64(%0);"
      "prefetcht1 12* 64(%0);"
      "prefetcht1 13* 64(%0);"
      "prefetcht1 14* 64(%0);"
      "prefetcht1 15* 64(%0);"
      "prefetcht1 16 *64(%0);"
      "prefetcht1 17 *64(%0);"
      "prefetcht1 18 * 64(%0);"
      "prefetcht1 19 * 64(%0);"
      "prefetcht1 20 * 64(%0);"
      "prefetcht1 21 * 64(%0);"
      "prefetcht1 22 * 64(%0);"
      "prefetcht1 23 * 64(%0);"
      "prefetcht1 24 * 64(%0);"
      "prefetcht1 25 * 64(%0);"
      "prefetcht1 26* 64(%0);"
      "prefetcht1 27* 64(%0);"
      "prefetcht1 28* 64(%0);"
      "prefetcht1 29* 64(%0);"
      "prefetcht1 30* 64(%0);"
      "prefetcht1 31* 64(%0);"
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
