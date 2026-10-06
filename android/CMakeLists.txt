# Olex2 (the wxWidgets GUI) as an Android APK. wx runs on its Qt port (wxQt);
# Qt is only the platform layer under wx, no Olex2 code uses Qt.
# Configure with a preset from CMakePresets.json, see README.md.
# Stage S1: no embedded Python, no cctbx (_PYTHON is not defined).
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
if (OLX_TUNE)
  # CMake's clang IPO is -flto=thin; the prebuilt static deps stay native code
  set_target_properties(olex2 PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
  target_link_options(olex2 PRIVATE "LINKER:--thinlto-jobs=6")
endif ()

# androiddeployqt reads the manifest and assets/ from this directory; it is
# generated so rundir files never land in the source tree
set(pkg "${CMAKE_CURRENT_BINARY_DIR}/package")
configure_file(AndroidManifest.xml "${pkg}/AndroidManifest.xml" COPYONLY)
set_target_properties(olex2 PROPERTIES
  QT_ANDROID_PACKAGE_SOURCE_DIR "${pkg}"
  QT_ANDROID_PACKAGE_NAME "org.olex2.android"
  QT_ANDROID_MIN_SDK_VERSION 28
  QT_ANDROID_TARGET_SDK_VERSION 36
  QT_ANDROID_VERSION_NAME "1.5-android-s1")

set(OLEX2_RUNDIR "D:/devel/rundir-py3" CACHE PATH
  "Installed Olex2 rundir the assets are copied from (read only)")
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_target(olex2_assets
  COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/assemble_assets.py"
    --rundir "${OLEX2_RUNDIR}"
    --overlay "${CMAKE_CURRENT_SOURCE_DIR}/rundir-overlay"
    --out "${pkg}/assets/olex2"
    --stale "${CMAKE_CURRENT_BINARY_DIR}/android-build/olex2.apk"
  COMMENT "Assembling assets/olex2 from ${OLEX2_RUNDIR}"
  VERBATIM)
add_dependencies(olex2 olex2_assets)
