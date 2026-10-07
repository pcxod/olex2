# Olex2 (the wxWidgets GUI) as an Android APK. wx runs on its Qt port (wxQt);
# Qt is only the platform layer under wx, no Olex2 code uses Qt.
# Configure with a preset from CMakePresets.json, see README.md.
# OLX_PYTHON (S2/S3) embeds CPython 3.14 with cctbx and NoSpherA2 from the
# per-ABI payload in OLX_STAGE; OFF gives the S1 build without _PYTHON.
cmake_minimum_required(VERSION 3.24)
project(olex2_android C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT ANDROID)
  message(FATAL_ERROR "android/ is the Android build; desktop builds use the root CMakeLists.txt")
endif ()

# OFF gives the plain NDK Release build (-O3 -DNDEBUG), the baseline the
# tuned numbers in README.md are measured against
option(OLX_TUNE "per-ABI code generation and ThinLTO" ON)
if (OLX_TUNE)
  include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/abi_flags.cmake")
endif ()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/deps.cmake")  # wx (qt), gl4es, GLU

get_filename_component(src "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
# the same globs as the root CMakeLists.txt, minus unirun and the Windows-only files
file(GLOB olex2_SRCS
  "${src}/alglib/*.cpp"
  "${src}/sdl/*.cpp" "${src}/sdl/smart/*.cpp" "${src}/sdl/exparse/*.cpp"
  "${src}/sdl/math/*.cpp"
  "${src}/xlib/*.cpp" "${src}/xlib/absorpc/*.cpp" "${src}/xlib/henke/*.cpp"
  "${src}/xlib/macro/*.cpp"
  "${src}/glib/*.cpp" "${src}/gxlib/*.cpp"
  "${src}/repository/*.cpp"
  "${src}/olex/*.cpp" "${src}/olex/nui/*.cpp" "${src}/olex/html/*.cpp"
  "${src}/olex/ctrls/*.cpp")
list(REMOVE_ITEM olex2_SRCS
  "${src}/repository/fsindex.cpp"
  "${src}/repository/unzip.cpp"
  "${src}/repository/zip.cpp")

qt_add_executable(olex2 ${olex2_SRCS} src/android_main.cpp)
target_include_directories(olex2 PRIVATE
  "${src}/alglib" "${src}/glib" "${src}/gxlib" "${src}/olex"
  "${src}/olex/html" "${src}/olex/ctrls" "${src}/olex/nui" "${src}/repository"
  "${src}/sdl" "${src}/sdl/math" "${src}/xlib" "${src}/xlib/absorpc")
target_compile_definitions(olex2 PRIVATE __WXWIDGETS__ _UNICODE)
target_link_libraries(olex2 PRIVATE
  wx::base wx::core wx::gl wx::net wx::html wx::stc
  olx_glu gl4es_gl
  Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets
  android log)
# keep gl4es's gl* symbols inside libolex2: Qt's own GLES calls must not bind to them
target_link_options(olex2 PRIVATE "LINKER:--exclude-libs,ALL")
# stripped here: build.gradle keeps gradle's llvm-strip off every .so, because
# it breaks the patchelf'ed payload libs (segment offsets lose their alignment)
target_link_options(olex2 PRIVATE "$<$<NOT:$<CONFIG:Debug>>:-s>")
if (OLX_TUNE)
  # CMake's clang IPO is -flto=thin; the prebuilt static deps stay native code
  set_target_properties(olex2 PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
  target_link_options(olex2 PRIVATE "LINKER:--thinlto-jobs=6")
endif ()

option(OLX_PYTHON "embedded CPython + cctbx + NoSpherA2 (stage S2/S3)" ON)
set(OLX_STAGE "D:/Android/stage" CACHE PATH
  "per-ABI payload: <abi>/python, <abi>/cctbx, <abi>/bin/libNoSpherA2.so, <abi>/share/occ")
set(OLX_PYTHON_SRC "D:/Android/src" CACHE PATH
  "holds python-3.14.8-<triple>/prefix/include/python3.14")
set(payload "${OLX_STAGE}/${ANDROID_ABI}")
# PyToolLib/Analysis.py imports future at top level; desktop Olex2 has it from pip
set(OLX_FUTURE_WHEEL "D:/Android/dl/future-1.0.0-py3-none-any.whl" CACHE FILEPATH
  "https://pypi.org/project/future/1.0.0 sha256 929292d34f5872e70396626ef385ec22355a1fae8ad29e1a734c3e43f9fbc216")
if (OLX_PYTHON)
  set(_triple_arm64-v8a aarch64-linux-android)
  set(_triple_armeabi-v7a arm-linux-androideabi)
  set(_triple_x86 i686-linux-android)
  set(_triple_x86_64 x86_64-linux-android)
  # updates stay off on Android, so no _OPENSSL (see c6cb2350)
  target_compile_definitions(olex2 PRIVATE _PYTHON)
  target_include_directories(olex2 PRIVATE
    "${OLX_PYTHON_SRC}/python-3.14.8-${_triple_${ANDROID_ABI}}/prefix/include/python3.14")
  target_link_libraries(olex2 PRIVATE "${payload}/python/lib/libpython3.14.so")
endif ()

# androiddeployqt reads the manifest and assets/ from this directory; it is
# generated so rundir files never land in the source tree
set(pkg "${CMAKE_CURRENT_BINARY_DIR}/package")
configure_file(AndroidManifest.xml "${pkg}/AndroidManifest.xml" COPYONLY)
# Qt 6.11.1's template plus ignoreAssetsPattern (keeps "_*" asset dirs)
configure_file(build.gradle "${pkg}/build.gradle" COPYONLY)
set_target_properties(olex2 PROPERTIES
  QT_ANDROID_PACKAGE_SOURCE_DIR "${pkg}"
  QT_ANDROID_PACKAGE_NAME "org.olex2.android"
  QT_ANDROID_MIN_SDK_VERSION 28
  QT_ANDROID_TARGET_SDK_VERSION 36
  QT_ANDROID_VERSION_NAME "1.5-android-s3")

set(OLEX2_RUNDIR "D:/devel/rundir-py3" CACHE PATH
  "Installed Olex2 rundir the assets are copied from (read only)")
find_package(Python3 REQUIRED COMPONENTS Interpreter)
# a host CPython of the same minor version writes the .pyc files the device
# then loads without compiling (unchecked-hash)
if (OLX_PYTHON)
  find_program(OLX_PYC_PYTHON NAMES python3.14 python REQUIRED
    PATHS "$ENV{LOCALAPPDATA}/Python/pythoncore-3.14-64" NO_DEFAULT_PATH)
endif ()
add_custom_target(olex2_assets
  COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/assemble_assets.py"
    --rundir "${OLEX2_RUNDIR}"
    --overlay "${CMAKE_CURRENT_SOURCE_DIR}/rundir-overlay"
    --out "${pkg}/assets/olex2"
    --stale "${CMAKE_CURRENT_BINARY_DIR}/android-build/olex2.apk"
    $<$<BOOL:${OLX_PYTHON}>:--overlay> $<$<BOOL:${OLX_PYTHON}>:${OLX_STAGE}/overlay>
    $<$<BOOL:${OLX_PYTHON}>:--wheel> $<$<BOOL:${OLX_PYTHON}>:${OLX_FUTURE_WHEEL}>
    $<$<BOOL:${OLX_PYTHON}>:--pyc> $<$<BOOL:${OLX_PYTHON}>:${OLX_PYC_PYTHON}>
    $<$<BOOL:${OLX_PYTHON}>:--payload> $<$<BOOL:${OLX_PYTHON}>:${payload}>
    $<$<BOOL:${OLX_PYTHON}>:--jnilibs> $<$<BOOL:${OLX_PYTHON}>:${pkg}/libs/${ANDROID_ABI}>
  COMMENT "Assembling assets/olex2 from ${OLEX2_RUNDIR}"
  VERBATIM)
add_dependencies(olex2 olex2_assets)
