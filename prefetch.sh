#!/bin/bash

. /opt/intel/bin/compilervars.sh intel64

OMP_NUM_THREADS=1 KMP_HW_SUBSET=1s,18c,1t ./prefetch_x
