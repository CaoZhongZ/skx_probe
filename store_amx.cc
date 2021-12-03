#include <immintrin.h>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <asm/prctl.h>        /* Definition of ARCH_* constants */
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>
#include <iostream>

#define XFEATURE_XTILECFG 17
#define XFEATURE_XTILEDATA 18
#define XFEATURE_MASK_XTILECFG (1 << XFEATURE_XTILECFG)
#define XFEATURE_MASK_XTILEDATA (1 << XFEATURE_XTILEDATA)
#define XFEATURE_MASK_XTILE (XFEATURE_MASK_XTILECFG | XFEATURE_MASK_XTILEDATA)


bool init() {
    unsigned long bitmask = 0;
    long status = syscall(SYS_arch_prctl, ARCH_GET_XCOMP_PERM, &bitmask);
    if (0 != status) return false;
    if (bitmask & XFEATURE_MASK_XTILEDATA) return true;

    status = syscall(SYS_arch_prctl, ARCH_REQ_XCOMP_PERM, XFEATURE_XTILEDATA);
    if (0 != status)
        return false; // XFEATURE_XTILEDATA setup is failed, TMUL usage is not allowed
    status = syscall(SYS_arch_prctl, ARCH_GET_XCOMP_PERM, &bitmask);

    // XFEATURE_XTILEDATA setup is failed, can't use TMUL
    if (0 != status || !(bitmask & XFEATURE_MASK_XTILEDATA)) return false;

    // XFEATURE_XTILEDATA set successfully, TMUL usage is allowed
    return true;
}

struct tile_config
{
  uint8_t palette;
  uint8_t start_row;
  uint8_t reserved[14];
  short tile_colsb[16];
  uint8_t tile_rows[16];

  tile_config(): palette(1), start_row(0) 
  {
    memset(reserved, 0, sizeof(reserved));
    for (int i = 0; i < 8; i++)
    {
      tile_colsb[i] = 64;
      tile_colsb[i+8] = 0;
      tile_rows[i] = 16;
      tile_rows[i+8] = 0;
    }
  }
};

inline void load_amx(char *d, const char *p, size_t sz) {
# pragma omp parallel
{
  bool status = init();
  tile_config cfg;
  _tile_loadconfig(&cfg);
  if (!status)
  {
    std::cout << "status : error" << std::endl;
  }
  _tile_loadd(0, p, 64);
  _tile_loadd(1, p+64, 64);
  _tile_loadd(2, p+64*2, 64);
  _tile_loadd(3, p+64*3, 64);
  _tile_loadd(4, p+64*4, 64);
  _tile_loadd(5, p+64*5, 64);
  _tile_loadd(6, p+64*6, 64);
  _tile_loadd(7, p+64*7, 64);

# pragma omp for
  for (size_t i = 0; i < sz/1024/8; i ++) {
        auto *index = p + i * 1024 * 8;
    int64_t stride = 64;
    // _tile_stored(0, index, stride);
    // _tile_stored(1, index+64, 64);
    // _tile_stored(2, index+64*2, 64);
    // _tile_stored(3, index+64*3, 64);
    // _tile_stored(4, index+64*4, 64);
    // _tile_stored(5, index+64*5, 64);
    // _tile_stored(6, index+64*6, 64);
    // _tile_stored(7, index+64*7, 64);
    asm volatile (
      "tilestored %%tmm0, (%0, %1);"
      "tilestored %%tmm1, 1024(%0, %1);"
      "tilestored %%tmm2, 2 * 1024(%0, %1);"
      "tilestored %%tmm3, 3 * 1024(%0, %1);"
      "tilestored %%tmm4, 4 * 1024(%0, %1);"
      "tilestored %%tmm5, 5 * 1024(%0, %1);"
      "tilestored %%tmm6, 6 * 1024(%0, %1);"
      "tilestored %%tmm7, 7 * 1024(%0, %1);"
      : : "r" (index),/*, "r" (dst)*/
          "r" (stride)
      );
  }
}
}


int main() {
  constexpr size_t size = 0x40000000ull;
  void *p, *d;
  auto status = init();
  if (!status) 
  {
    std::cout << "main status error" << std::endl;
    return -1;
  }
  /*
  The function posix_memalign() allocates size bytes and places the
  address of the allocated memory in *memptr.  The address of the
  allocated memory will be a multiple of alignment, which must be a
  power of two and a multiple of sizeof(void *).  This address can
  later be successfully passed to free(3).  If size is 0, then the
  value placed in *memptr is either NULL or a unique pointer value.
  */
  posix_memalign(&p, 64, size);
  posix_memalign(&d, 64, size/32);
  size_t stride = size / 64;
# pragma omp parallel for
  for (int i = 0; i < 64; i++)
  {
    memset((char*)p+stride*i, 1, size/64);
  }
  
  constexpr int times = 4096;
  tile_config cfg;
  void* wcfg = malloc(64);
  _tile_loadconfig(&cfg);
  _tile_storeconfig(wcfg);

  for (int i =0; i < times; i ++)
    load_amx((char *)d, (const char*)p, size);

  std::cout << "Job Done!" << std::endl;
  return 0;
}
