# Olex2 (the wxWidgets GUI) as an Android APK. wx runs on its Qt port (wxQt);
# Qt is only the platform layer under wx, no Olex2 code uses Qt.
# Configure with a preset from CMakePresets.json, see README.md.
# OLX_PYTHON (S2/S3) embeds CPython 3.14 with cctbx and NoSpherA2 from the
# per-ABI payload in OLX_STAGE; OFF gives the S1 build without _PYTHON.
# OLX_QT=5 is the legacy line: Qt 5.15.2, NDK r21e, minSdk 21, CPython 3.13
# (the legacy-* presets; android/build_legacy.ps1 packages the APK).
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

set(OLX_QT 6 CACHE STRING "Qt major version: 6 (API 28+) or 5 (API 21+)")
set(pkg "${CMAKE_CURRENT_BINARY_DIR}/package")
if (OLX_QT EQUAL 5)
  # read by Qt5AndroidSupport.cmake (Qt5Core) into android_deployment_settings.json
  set(QT_ANDROID_APPLICATION_BINARY olex2)
  set(ANDROID_PACKAGE_SOURCE_DIR "${pkg}")
  set(ANDROID_VERSION_NAME "1.5-android-legacy")
  find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL AndroidExtras)
  set(_olx_qt_libs Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL Qt5::AndroidExtras)
else ()
  find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
  set(_olx_qt_libs Qt6::Core Qt6::Gui Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets)
endif ()
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

if (OLX_QT EQUAL 5)
  # Qt 5 loads lib<target>_<abi>.so and calls its main()
  add_library(olex2 SHARED ${olex2_SRCS} src/android_main.cpp)
else ()
  qt_add_executable(olex2 ${olex2_SRCS} src/android_main.cpp)
endif ()
target_include_directories(olex2 PRIVATE
  "${src}/alglib" "${src}/glib" "${src}/gxlib" "${src}/olex"
  "${src}/olex/html" "${src}/olex/ctrls" "${src}/olex/nui" "${src}/repository"
  "${src}/sdl" "${src}/sdl/math" "${src}/xlib" "${src}/xlib/absorpc")
target_compile_definitions(olex2 PRIVATE __WXWIDGETS__ _UNICODE)
# armv7: Android <= 8.1's libm exports libgcc's unwinder and precedes Qt5Core
# in our DT_NEEDED, so an imported _Unwind_Resume or __aeabi_unwind_cpp_pr*
# bound there and every C++ exception aborted (SM-T585). The NDK's libunwind
# goes first, before any .so can satisfy those symbols
set(_olx_unwind "")
if (ANDROID_ABI STREQUAL "armeabi-v7a")
  file(GLOB _olx_unwind
    "${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libs/armeabi-v7a/libunwind.a"
    "${ANDROID_NDK}/toolchains/llvm/prebuilt/*/lib/clang/*/lib/linux/arm/libunwind.a")
  list(GET _olx_unwind 0 _olx_unwind)
endif ()
target_link_libraries(olex2 PRIVATE ${_olx_unwind}
  wx::base wx::core wx::gl wx::net wx::html wx::stc
  olx_glu gl4es_gl
  ${_olx_qt_libs} android log)
# keep gl4es's gl* symbols inside libolex2: Qt's own GLES calls must not bind to them
target_link_options(olex2 PRIVATE "LINKER:--exclude-libs,ALL")
# stripped here: build.gradle keeps gradle's llvm-strip off every .so, because
# it breaks the patchelf'ed payload libs (segment offsets lose their alignment)
target_link_options(olex2 PRIVATE "$<$<NOT:$<CONFIG:Debug>>:-s>")
if (OLX_TUNE)
  # CMake's clang IPO is -flto=thin; the prebuilt static deps stay native code
  set_target_properties(olex2 PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
  # CMake appends -fuse-ld=gold to IPO links for NDK < 22 (ndk issue 242,
  # gold LTO on arm); r21 with ANDROID_LD=lld does ThinLTO in lld
  if (ANDROID_LD STREQUAL "lld")
    set(CMAKE_CXX_LINK_OPTIONS_IPO "")
  endif ()
  target_link_options(olex2 PRIVATE "LINKER:--thinlto-jobs=6")
endif ()

option(OLX_PYTHON "embedded CPython + cctbx + NoSpherA2 (stage S2/S3)" ON)
set(OLX_STAGE "D:/Android/stage" CACHE PATH
  "per-ABI payload: <abi>/python, <abi>/cctbx, <abi>/bin/libNoSpherA2.so, <abi>/share/occ")
set(OLX_PYTHON_SRC "D:/Android/src" CACHE PATH
  "holds python-3.14.8-<triple>/prefix/include/python3.14")
set(OLX_PYTHON_VERSION 3.14 CACHE STRING "CPython of the payload, 3.13 for OLX_QT=5")
set(OLX_PYTHON_INCLUDE "" CACHE PATH
  "CPython headers; empty: <payload>/python/include, else the OLX_PYTHON_SRC layout")
set(OLX_STAGE_OVERLAY "${OLX_STAGE}/overlay" CACHE PATH
  "Olex2 Python sources patched for Android, overlaid on the rundir")
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
  if (NOT OLX_PYTHON_INCLUDE)
    set(OLX_PYTHON_INCLUDE "${payload}/python/include/python${OLX_PYTHON_VERSION}")
    if (NOT EXISTS "${OLX_PYTHON_INCLUDE}/Python.h")
      set(OLX_PYTHON_INCLUDE
        "${OLX_PYTHON_SRC}/python-3.14.8-${_triple_${ANDROID_ABI}}/prefix/include/python3.14")
    endif ()
  endif ()
  target_include_directories(olex2 PRIVATE "${OLX_PYTHON_INCLUDE}")
  target_link_libraries(olex2 PRIVATE
    "${payload}/python/lib/libpython${OLX_PYTHON_VERSION}.so")
endif ()

# androiddeployqt reads the manifest and assets/ from ${pkg}; it is
# generated so rundir files never land in the source tree
if (OLX_QT EQUAL 5)
  configure_file(qt5/AndroidManifest.xml "${pkg}/AndroidManifest.xml" COPYONLY)
  configure_file(qt5/build.gradle "${pkg}/build.gradle" COPYONLY)
  # Qt 5.15.2's own wrapper is Gradle 5.6.4: Qt 6.11.1's (9.3.1) instead
  set(OLX_GRADLE_WRAPPER "C:/Qt/6.11.1/android_armv7/src/3rdparty/gradle" CACHE PATH
    "gradlew, gradlew.bat and gradle/wrapper of a Gradle that runs on JDK 17")
  file(COPY "${OLX_GRADLE_WRAPPER}/gradlew" "${OLX_GRADLE_WRAPPER}/gradlew.bat"
    "${OLX_GRADLE_WRAPPER}/gradle" DESTINATION "${pkg}")
else ()
  configure_file(AndroidManifest.xml "${pkg}/AndroidManifest.xml" COPYONLY)
  # Qt 6.11.1's template plus ignoreAssetsPattern (keeps "_*" asset dirs)
  configure_file(build.gradle "${pkg}/build.gradle" COPYONLY)
endif ()
# launcher icon (@mipmap/ic_launcher), the Olex2 1.5 logo
file(COPY res DESTINATION "${pkg}")
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
  find_program(OLX_PYC_PYTHON NAMES python${OLX_PYTHON_VERSION} python REQUIRED
    PATHS "$ENV{LOCALAPPDATA}/Python/pythoncore-${OLX_PYTHON_VERSION}-64" NO_DEFAULT_PATH)
  # a list, not $<$<BOOL:..>:..> per argument: with OLX_PYTHON off those
  # expand to empty "" arguments on Windows, which argparse refuses
  set(_olx_py_args --overlay "${OLX_STAGE_OVERLAY}" --wheel "${OLX_FUTURE_WHEEL}"
    --pyc "${OLX_PYC_PYTHON}" --payload "${payload}" --jnilibs "${pkg}/libs/${ANDROID_ABI}")
  # pre-M bionic binds each library to its own weak typeinfo copy, which
  # breaks Boost.Python's casts across cctbx libraries (assemble_assets.py)
  if (ANDROID_PLATFORM_LEVEL LESS 23)
    list(APPEND _olx_py_args --share-typeinfo)
  endif ()
endif ()
add_custom_target(olex2_assets
  COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/assemble_assets.py"
    --rundir "${OLEX2_RUNDIR}"
    --overlay "${CMAKE_CURRENT_SOURCE_DIR}/rundir-overlay"
    --out "${pkg}/assets/olex2"
    --stale "${CMAKE_CURRENT_BINARY_DIR}/android-build/olex2.apk"
    ${_olx_py_args}
  COMMENT "Assembling assets/olex2 from ${OLEX2_RUNDIR}"
  VERBATIM)
add_dependencies(olex2 olex2_assets)
