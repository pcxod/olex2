# glprobe: wxGLCanvas + gl4es + gestures on wxQt/Android, see main.cpp.
# Configure like the app (Qt Android toolchain, ANDROID_ABI) after
# python android/build_deps.py; GLPROBE_SELF_INIT=ON for a stock (unpatched) wx.
cmake_minimum_required(VERSION 3.24)
project(glprobe CXX)
set(CMAKE_CXX_STANDARD 17)
option(GLPROBE_SELF_INIT "the app initialises gl4es itself (stock wxWidgets)" OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets)
include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/deps.cmake")

qt_add_executable(glprobe main.cpp)
if (GLPROBE_SELF_INIT)
  target_compile_definitions(glprobe PRIVATE GLPROBE_SELF_INIT)
endif ()
target_link_libraries(glprobe PRIVATE wx::gl wx::core wx::base olx_glu gl4es_gl
  Qt6::OpenGLWidgets log)
target_link_options(glprobe PRIVATE "LINKER:--exclude-libs,ALL")
set_target_properties(glprobe PROPERTIES
  QT_ANDROID_PACKAGE_NAME "org.olex2.glprobe"
  QT_ANDROID_MIN_SDK_VERSION 28
  QT_ANDROID_TARGET_SDK_VERSION 36)
