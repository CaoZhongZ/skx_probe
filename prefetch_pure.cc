#include <immintrin.h>
#include <cstdlib>
#include <cstring>

inline void prefetchs(char *d, const char *p, size_t sz) {
# pragma omp parallel for
  for (size_t i = 0; i < sz/32/64; i ++) {
    auto *index = p + i * 32 * 64;
    asm volatile (
      "prefetchnta (%0);"
      "prefetchnta 64(%0);"
      "prefetchnta 2 * 64(%0);"
      "prefetchnta 3 * 64(%0);"
      "prefetchnta 4 * 64(%0);"
      "prefetchnta 5 * 64(%0);"
      "prefetchnta 6 * 64(%0);"
      "prefetchnta 7 * 64(%0);"
      "prefetchnta 8 * 64(%0);"
      "prefetchnta 9 * 64(%0);"
      "prefetchnta 10* 64(%0);"
      "prefetchnta 11* 64(%0);"
      "prefetchnta 12* 64(%0);"
      "prefetchnta 13* 64(%0);"
      "prefetchnta 14* 64(%0);"
      "prefetchnta 15* 64(%0);"
      "prefetchnta 16 *64(%0);"
      "prefetchnta 17 *64(%0);"
      "prefetchnta 18 * 64(%0);"
      "prefetchnta 19 * 64(%0);"
      "prefetchnta 20 * 64(%0);"
      "prefetchnta 21 * 64(%0);"
      "prefetchnta 22 * 64(%0);"
      "prefetchnta 23 * 64(%0);"
      "prefetchnta 24 * 64(%0);"
      "prefetchnta 25 * 64(%0);"
      "prefetchnta 26* 64(%0);"
      "prefetchnta 27* 64(%0);"
      "prefetchnta 28* 64(%0);"
      "prefetchnta 29* 64(%0);"
      "prefetchnta 30* 64(%0);"
      "prefetchnta 31* 64(%0);"
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
