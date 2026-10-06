# Per-ABI code generation, kept inside what the Android ABI guarantees so one
# APK per ABI runs on every device of that ABI. Anything beyond the baseline
# (AVX2, dotprod, SVE, VFPv4) needs runtime dispatch in the code itself.
# Included by android/CMakeLists.txt and, through CMAKE_PROJECT_INCLUDE, by
# every dependency build_deps.py configures. The NDK's clang already enables
# most of this for Android targets; the flags make the baseline explicit.
if (ANDROID_ABI STREQUAL "x86_64")
  set(_olx_abi_flags -march=x86-64-v2)  # SSE4.2 + POPCNT are guaranteed
elseif (ANDROID_ABI STREQUAL "x86")
  set(_olx_abi_flags -mssse3)           # the x86 ABI guarantee
elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
  # Thumb-2 + NEON; not neon-vfpv4, Cortex-A9 devices lack VFPv4
  set(_olx_abi_flags -mthumb -mfpu=neon)
else ()
  set(_olx_abi_flags)                   # arm64-v8a: armv8-a baseline
endif ()
add_compile_options("$<$<COMPILE_LANGUAGE:C,CXX>:${_olx_abi_flags}>")
