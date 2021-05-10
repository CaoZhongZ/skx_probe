#!/bin/bash

. /opt/intel/bin/compilervars.sh intel64

OMP_NUM_THREADS=18 KMP_HW_SUBSET=1s,18c,1t ./load_x
