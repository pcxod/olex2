# Per-ABI code generation, kept inside what the Android ABI guarantees so one
# APK per ABI runs on every device of that ABI. Anything beyond the baseline
# (AVX2, dotprod, SVE, VFPv4) needs runtime dispatch in the code itself.
# One deliberate exception: armeabi-v7a targets armv7ve (see below).
# Included by android/CMakeLists.txt and, through CMAKE_PROJECT_INCLUDE, by
# every dependency build_deps.py configures. The NDK's clang already enables
# most of this for Android targets; the flags make the baseline explicit.
if (ANDROID_ABI STREQUAL "x86_64")
  # SSE4.2 + POPCNT are guaranteed; clang < 12 (NDK r21) has no x86-64-v2
  if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12)
    set(_olx_abi_flags -msse4.2 -mpopcnt)
  else ()
    set(_olx_abi_flags -march=x86-64-v2)
  endif ()
elseif (ANDROID_ABI STREQUAL "x86")
  set(_olx_abi_flags -mssse3)           # the x86 ABI guarantee
elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
  # armv7ve: hardware sdiv/udiv instead of __aeabi_idiv calls, exact, so
  # results do not change. Drops Cortex-A5/A8/A9 and Scorpion (SIGILL there),
  # accepted 9 Oct 2026. Thumb-2 + NEON; not neon-vfpv4, FMA would change
  # rounding.
  set(_olx_abi_flags -march=armv7ve -mthumb -mfpu=neon)
else ()
  set(_olx_abi_flags)                   # arm64-v8a: armv8-a baseline
endif ()
add_compile_options("$<$<COMPILE_LANGUAGE:C,CXX>:${_olx_abi_flags}>")
