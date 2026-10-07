#!/bin/bash
# static glibc ptb for Android: armeabi-v7a, arm64-v8a (Arm GNU toolchain 14.2), x86_64 (system gfortran), x86 (bootlin i686 + system gfortran -m32)
# usage: build.sh <abi>...   output: ~/ptb-android/out/<abi>/ptb
set -eo pipefail
here=$(cd "$(dirname "$0")" && pwd)
W=~/ptb-android
SRC=/mnt/d/Android/src
J=${J:-6}
mkdir -p $W/tc $W/out
cd $W
for t in arm-none-linux-gnueabihf aarch64-none-linux-gnu; do
  [ -d tc/arm-gnu-toolchain-14.2.rel1-x86_64-$t ] || tar -C tc -xf $SRC/arm-gnu-toolchain-14.2.rel1-x86_64-$t.tar.xz
done
[ -d tc/x86-i686--glibc--stable-2026.08-1 ] || tar -C tc -xf $SRC/x86-i686--glibc--stable-2026.08-1.tar.xz
[ -d OpenBLAS-0.3.28 ] || tar -xzf $SRC/OpenBLAS-0.3.28.tar.gz
# the release tag as a plain tree (never build from the working copy, see the pTB release-build note)
if [ ! -d ptb-src ]; then
  mkdir ptb-src
  git -c safe.directory='*' -C /mnt/d/git/ptb archive v3.11-NoSpherA2 source | tar -x -C ptb-src
fi

for abi in "$@"; do
  case $abi in
    armeabi-v7a) P=$W/tc/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-
                 OB="TARGET=ARMV7" ;;
    arm64-v8a)   P=$W/tc/arm-gnu-toolchain-14.2.rel1-x86_64-aarch64-none-linux-gnu/bin/aarch64-none-linux-gnu-
                 OB="TARGET=ARMV8 DYNAMIC_ARCH=1" ;;
    x86_64)      P=x86_64-linux-gnu-
                 OB="TARGET=NEHALEM DYNAMIC_ARCH=1" ;;
    x86)         P=$W/tc/x86-i686--glibc--stable-2026.08-1/bin/i686-linux-
                 OB="TARGET=PRESCOTT DYNAMIC_ARCH=1 BINARY=32" ;;
    *) echo "unknown abi $abi"; exit 1 ;;
  esac
  CC=${P}gcc; FC=${P}gfortran; LD=$FC; RT=""
  [ $abi = x86_64 ] && { CC=gcc; FC=gfortran; LD=$FC; }
  # bootlin's i686 toolchain has no Fortran: the system gfortran compiles
  # -m32, bootlin's gcc links against its glibc with Ubuntu's 32-bit
  # libgfortran (lib32gfortran-13-dev, lib32gcc-13-dev unpacked in lib32/)
  if [ $abi = x86 ]; then
    printf '#!/bin/sh\nexec gfortran -m32 "$@"\n' > ${P}gfortran; chmod +x ${P}gfortran
    L32=$W/lib32/x/usr/lib/gcc/x86_64-linux-gnu/13/32
    LD=$CC; RT=" $L32/libgfortran.a $L32/libquadmath.a"
  fi
  pre=$W/openblas-$abi
  if [ ! -f $pre/lib/libopenblas.a ]; then
    rm -rf ob-$abi; cp -r OpenBLAS-0.3.28 ob-$abi
    make -C ob-$abi -j$J -s CC=$CC FC=$FC HOSTCC=gcc $OB NO_SHARED=1 USE_OPENMP=1 NUM_THREADS=64 libs netlib > ob-$abi.log 2>&1
    make -C ob-$abi -s PREFIX=$pre NO_SHARED=1 $OB install >> ob-$abi.log 2>&1
  fi
  # libgfortran/libgomp reach pthread through weak references (gthr-posix.h),
  # which never pull a member out of libc.a: unforced, close() calls address 0
  KEEP=""
  for s in mutex_destroy mutex_init mutex_trylock mutex_timedlock cond_destroy cond_init \
           cond_broadcast cond_signal cond_wait cond_timedwait key_create key_delete \
           getspecific setspecific once create join detach equal self cancel \
           mutexattr_init mutexattr_settype mutexattr_destroy; do KEEP="$KEEP -Wl,-u,pthread_$s"; done
  b=ptb-$abi; rm -rf $b; cp -r ptb-src/source $b
  # ptb's own sscal (uncalled) shadows BLAS sscal in a static link, and
  # LAPACK's ssyev then jumps into it
  sed -i '1s/subroutine sscal(/subroutine ptb_sscal_unused(/' $b/sscal.f90
  # runs before TLS exists: no stack canary (x86 gcc adds one by default)
  $CC -O2 -fno-stack-protector -c $here/sigsys.c -o $b/sigsys.o
  mkdir -p out/$abi
  # no -j: the Makefile's module dependencies are incomplete
  make -C $b -s COMPILER=gfortran FC=$FC CC=$CC PROG=$W/out/$abi/ptb \
    FFLAGS="-O2 -ffree-line-length-none -fopenmp -fno-backtrace" CCFLAGS="-O2 -std=gnu17 -DLINUX" \
    LINKER="$LD -static -O2 -fopenmp$KEEP -Wl,-e,ptb_start" \
    LIBS="sigsys.o $pre/lib/libopenblas.a$RT -lpthread -lm" > $b.log 2>&1
  ${P}strip $W/out/$abi/ptb 2>/dev/null || strip $W/out/$abi/ptb
  file $W/out/$abi/ptb; ls -la $W/out/$abi/ptb
  echo "=== $abi built"
done
