#include <immintrin.h>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <asm/prctl.h>        /* Definition of ARCH_* constants */
#include <sys/syscall.h>      /* Definition of SYS_* constants */
#include <unistd.h>
#include <iostream>
#include "profile.h"
#include "amx_tdpbuud.hpp"
#include "amx_tdpbsud.hpp"
#include "amx_tdpbusd.hpp"
#include "amx_tdpbssd.hpp"

#define XFEATURE_XTILECFG 17
#define XFEATURE_XTILEDATA 18
#define XFEATURE_MASK_XTILECFG (1 << XFEATURE_XTILECFG)
#define XFEATURE_MASK_XTILEDATA (1 << XFEATURE_XTILEDATA)
#define XFEATURE_MASK_XTILE (XFEATURE_MASK_XTILECFG | XFEATURE_MASK_XTILEDATA)

#define TILE_M 16
#define TILE_K 16
#define TILE_N 16
#define MAX_ELEMENTS		((TILE_M * TILE_K) + (TILE_K * TILE_N) + (TILE_M * TILE_N))
#define BYTES_PER_ELEMENT	4

#define TMM0	0
#define TMM1	1
#define TMM2	2
#define TMM3	3
#define TMM4	4
#define TMM5	5
#define TMM6	6
#define TMM7	7


enum class TMMCOM {T012, T345, T245, T067, T467};

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

static struct tile_config
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
} __attribute__((packed)) tilecfg;


struct tile_buffer {
	union {
		struct {
			uint32_t a[TILE_M * TILE_K];
			uint32_t b[TILE_K * TILE_N];
			uint32_t c[TILE_M * TILE_N];
		};
		uint32_t bytes[0];
	};
};

static void set_rand_tiledata(struct tile_buffer *tbuf)
{
	int data;
	int i;

	/*
	 * Ensure that 'data' is never 0.  This ensures that
	 * the registers are never in their initial configuration
	 * and thus never tracked as being in the init state.
	 */

	for (i = 0; i < MAX_ELEMENTS; i++) {
		data = (rand() % 0xff);
		tbuf->bytes[i] = data;
	}
}

void load_tilematrix(struct tile_buffer *tbuf)
{
  set_rand_tiledata(tbuf);
  _tile_release();
  _tile_loadconfig(&tilecfg);

  _tile_loadd(TMM1, tbuf->a, TILE_K * BYTES_PER_ELEMENT);
  _tile_loadd(TMM2, tbuf->b, TILE_N * BYTES_PER_ELEMENT);
  _tile_loadd(TMM0, tbuf->c, TILE_N * BYTES_PER_ELEMENT);

  _tile_loadd(TMM4, tbuf->a, TILE_K * BYTES_PER_ELEMENT);
  _tile_loadd(TMM5, tbuf->b, TILE_N * BYTES_PER_ELEMENT);
  _tile_loadd(TMM3, tbuf->c, TILE_N * BYTES_PER_ELEMENT);

  _tile_loadd(TMM6, tbuf->a, TILE_K * BYTES_PER_ELEMENT);
  _tile_loadd(TMM7, tbuf->b, TILE_N * BYTES_PER_ELEMENT);
}

void do_normalmm(struct tile_buffer *tbuf)
{
  for (int i = 0; i < TILE_M; i++)
  {
    for (int j = 0; j < TILE_N; j++)
    {
      for (int k = 0; k < TILE_K; k++)
      {
        tbuf->c[i*TILE_N+j] += tbuf->a[i*TILE_K+k] * tbuf->b[k*TILE_N+j];
      }
    }
  }
}


void print_tile(struct tile_buffer *tbuf)
{
  std::cout << "Matrix A : " << std::endl;
  for (int i = 0; i < TILE_M; i++)
  {
    for (int j = 0; j < TILE_K; j++)
    {
      std::cout << tbuf->a[i * TILE_K + j] << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "Matrix B : " << std::endl;
  for (int i = 0; i < TILE_K; i++)
  {
    for (int j = 0; j < TILE_N; j++)
    {
      std::cout << tbuf->b[i * TILE_N + j] << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "Matrix C : " << std::endl;
  for (int i = 0; i < TILE_M; i++)
  {
    for (int j = 0; j < TILE_N; j++)
    {
      std::cout << tbuf->c[i * TILE_N + j] << " ";
    }
    std::cout << std::endl;
  }
}

inline void test_tdpbuud(struct tile_buffer *tbuf, int times)
{
  auto start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 1, 2>();
  }
  auto duration =
    std::chrono::duration_cast<
    std::chrono::nanoseconds>(Time::now() - start).count();
  std::cout << "---------------test tdpbuud----------------" << std::endl;
  std::cout<<"012 exe_duration "<< (float)duration / (float)times <<" ns"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<5, 0, 4>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<5, 0, 4>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<5, 0, 4>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<5, 0, 4>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<5, 0, 4>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<5, 0, 4>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+504 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbuud<4, 0, 2>();
    __tile_dpbuud<5, 0, 3>();
    __tile_dpbuud<6, 1, 2>();
    __tile_dpbuud<7, 1, 3>();
    __tile_dpbuud<4, 0, 2>();
    __tile_dpbuud<5, 0, 3>();
    __tile_dpbuud<6, 1, 2>();
    __tile_dpbuud<7, 1, 3>();
    __tile_dpbuud<4, 0, 2>();
    __tile_dpbuud<5, 0, 3>();
    __tile_dpbuud<6, 1, 2>();
    __tile_dpbuud<7, 1, 3>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"402+503+612+713 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;


  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;
  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<3, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+345 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345+245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 6, 7>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 6, 7>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 6, 7>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 6, 7>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 6, 7>();
    __tile_dpbuud<0, 1, 2>();
    __tile_dpbuud<0, 6, 7>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+067 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<4, 6, 7>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<4, 6, 7>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<4, 6, 7>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<4, 6, 7>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<4, 6, 7>();
    __tile_dpbuud<3, 4, 5>();
    __tile_dpbuud<4, 6, 7>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345+467 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "---------------test tdpbuud----------------" << std::endl;
}

inline void test_tdpbsud(struct tile_buffer *tbuf, int times)
{
  auto start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 1, 2>();
  }
  auto duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout << "---------------test tdpbsud----------------" << std::endl;
  std::cout<<"012 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<5, 0, 4>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<5, 0, 4>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<5, 0, 4>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<5, 0, 4>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<5, 0, 4>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<5, 0, 4>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+504 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbsud<4, 0, 2>();
    __tile_dpbsud<5, 0, 3>();
    __tile_dpbsud<6, 1, 2>();
    __tile_dpbsud<7, 1, 3>();
    __tile_dpbsud<4, 0, 2>();
    __tile_dpbsud<5, 0, 3>();
    __tile_dpbsud<6, 1, 2>();
    __tile_dpbsud<7, 1, 3>();
    __tile_dpbsud<4, 0, 2>();
    __tile_dpbsud<5, 0, 3>();
    __tile_dpbsud<6, 1, 2>();
    __tile_dpbsud<7, 1, 3>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"402+503+612+713 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;


  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;
  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<3, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+345 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345+245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 6, 7>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 6, 7>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 6, 7>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 6, 7>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 6, 7>();
    __tile_dpbsud<0, 1, 2>();
    __tile_dpbsud<0, 6, 7>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+067 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<4, 6, 7>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<4, 6, 7>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<4, 6, 7>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<4, 6, 7>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<4, 6, 7>();
    __tile_dpbsud<3, 4, 5>();
    __tile_dpbsud<4, 6, 7>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345+467 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "---------------test tdpbsud----------------" << std::endl;
}

inline void test_tdpbssd(struct tile_buffer *tbuf, int times)
{
  auto start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 1, 2>();
  }
  auto duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout << "---------------test tdpbssd----------------" << std::endl;
  std::cout<<"012 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<5, 0, 4>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<5, 0, 4>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<5, 0, 4>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<5, 0, 4>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<5, 0, 4>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<5, 0, 4>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+504 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbssd<4, 0, 2>();
    __tile_dpbssd<5, 0, 3>();
    __tile_dpbssd<6, 1, 2>();
    __tile_dpbssd<7, 1, 3>();
    __tile_dpbssd<4, 0, 2>();
    __tile_dpbssd<5, 0, 3>();
    __tile_dpbssd<6, 1, 2>();
    __tile_dpbssd<7, 1, 3>();
    __tile_dpbssd<4, 0, 2>();
    __tile_dpbssd<5, 0, 3>();
    __tile_dpbssd<6, 1, 2>();
    __tile_dpbssd<7, 1, 3>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"402+503+612+713 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;


  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;
  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<3, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+345 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345+245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 6, 7>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 6, 7>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 6, 7>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 6, 7>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 6, 7>();
    __tile_dpbssd<0, 1, 2>();
    __tile_dpbssd<0, 6, 7>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+067 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<4, 6, 7>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<4, 6, 7>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<4, 6, 7>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<4, 6, 7>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<4, 6, 7>();
    __tile_dpbssd<3, 4, 5>();
    __tile_dpbssd<4, 6, 7>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345+467 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "---------------test tdpbssd----------------" << std::endl;
}

inline void test_tdpbusd(struct tile_buffer *tbuf, int times)
{
  auto start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 1, 2>();
  }
  auto duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout << "---------------test tdpbusd----------------" << std::endl;
  std::cout<<"012 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<5, 0, 4>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<5, 0, 4>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<5, 0, 4>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<5, 0, 4>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<5, 0, 4>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<5, 0, 4>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+504 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbusd<4, 0, 2>();
    __tile_dpbusd<5, 0, 3>();
    __tile_dpbusd<6, 1, 2>();
    __tile_dpbusd<7, 1, 3>();
    __tile_dpbusd<4, 0, 2>();
    __tile_dpbusd<5, 0, 3>();
    __tile_dpbusd<6, 1, 2>();
    __tile_dpbusd<7, 1, 3>();
    __tile_dpbusd<4, 0, 2>();
    __tile_dpbusd<5, 0, 3>();
    __tile_dpbusd<6, 1, 2>();
    __tile_dpbusd<7, 1, 3>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"402+503+612+713 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;


  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;
  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<3, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+345 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<2, 4, 5>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345+245 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 6, 7>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 6, 7>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 6, 7>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 6, 7>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 6, 7>();
    __tile_dpbusd<0, 1, 2>();
    __tile_dpbusd<0, 6, 7>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"012+067 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "-------------------------------" << std::endl;

  start = Time::now();

  for (int i =0; i < times/12; i++)
  {
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<4, 6, 7>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<4, 6, 7>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<4, 6, 7>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<4, 6, 7>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<4, 6, 7>();
    __tile_dpbusd<3, 4, 5>();
    __tile_dpbusd<4, 6, 7>();
  }
  duration =
    std::chrono::duration_cast<
    std::chrono::microseconds>(Time::now() - start).count();
  std::cout<<"345+467 exe_duration "<<duration<<"us"<<std::endl;
  std::cout << "---------------test tdpbusd----------------" << std::endl;
}




int main(int argc, char* argv[]) {
  auto status = init();
  if (!status) 
  {
    std::cout << "main status error" << std::endl;
    return -1;
  } 
  int times = 1024;
  if (argc != 2)
  {
    std::cout << "usage int8_amx [times]! Now the times is default which is 1024!" << std::endl;
  }
  else
  {
    times = atoi(argv[1]);
  }
  struct tile_buffer *tbuf = nullptr;
  posix_memalign((void**)&tbuf, 64, MAX_ELEMENTS * BYTES_PER_ELEMENT);
  load_tilematrix(tbuf);

  do_normalmm(tbuf);
  print_tile(tbuf);

  __tile_dpbuud<0, 1, 2>();
  _tile_stored(0, tbuf->c, TILE_N * BYTES_PER_ELEMENT);
  print_tile(tbuf);

  test_tdpbuud(tbuf, times);
  // test_tdpbsud(tbuf, times);
  // test_tdpbusd(tbuf, times);
  // test_tdpbssd(tbuf, times);

  std::cout << "Job Done!" << std::endl;
  return 0;
}
