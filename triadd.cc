#include <iostream>
#include <cstdlib>
#include <cstring>
#include "profile.h"

int main() {
  constexpr size_t size = 256 * 1024 * 1024ull;
  float *a, *b, *c;

  auto ret = posix_memalign(&a, 64, size);
  ret = posix_memalign(&b, 64, size);
  ret = posix_memalign(&c, 64, size);

  memset(a, 0, size);
  memset(b, 0, size);
  memset(c, 0, size);

  auto times = 2000;

  for (int j = 0; j < times; ++j) {
#   pragma omp parallel for
    for (size_t i = 0; i < size / sizeof(float); i++) {
      a[i] = b[i] + c[i] * 0.2;
    }
  }
}
