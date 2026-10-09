# CMAKE_PROJECT_INCLUDE of the legacy wx build (build_deps.py --legacy):
# the per-ABI flags, plus Threads::Threads, which Qt 5.15's Android CMake
# config links without finding it.
include("${CMAKE_CURRENT_LIST_DIR}/abi_flags.cmake")
if (ANDROID AND NOT TARGET Threads::Threads)
  find_package(Threads REQUIRED)
endif ()
