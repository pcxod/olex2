# Touch-gesture translation test: olex/touchnav.cpp against the real glib
# TGlRenderer/TGlMouse, fed synthesised wx gesture events. No window, no GL
# context. Desktop-only; the Android build lives in android/.
#   cmake -S tests/touch -B <build> -G Ninja -DwxWidgets_ROOT_DIR=C:/wxWidgets-3.2.6
#   cmake --build <build> -j 6 && ctest --test-dir <build>
cmake_minimum_required(VERSION 3.24)
project(olex2_touch_test CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(src "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
find_package(wxWidgets REQUIRED COMPONENTS core base)
include(${wxWidgets_USE_FILE})
find_package(OpenGL REQUIRED)

# sdl and glib as the MSVC projects build them: without __WXWIDGETS__
file(GLOB sdl_SRCS "${src}/sdl/*.cpp" "${src}/sdl/smart/*.cpp"
  "${src}/sdl/exparse/*.cpp" "${src}/sdl/math/*.cpp")
file(GLOB glib_SRCS "${src}/glib/*.cpp")
add_library(olx_gl STATIC ${sdl_SRCS} ${glib_SRCS})
target_include_directories(olx_gl PUBLIC "${src}/sdl" "${src}/sdl/math"
  "${src}/glib")
target_compile_definitions(olx_gl PUBLIC _UNICODE)
if (MSVC)
  target_compile_options(olx_gl PUBLIC /bigobj /W1)
endif ()
target_link_libraries(olx_gl PUBLIC OpenGL::GL OpenGL::GLU)

add_executable(touch_test touch_test.cpp "${src}/olex/touchnav.cpp")
target_include_directories(touch_test PRIVATE "${src}/olex")
target_link_libraries(touch_test PRIVATE olx_gl ${wxWidgets_LIBRARIES})

enable_testing()
add_test(NAME touch_gestures COMMAND touch_test)
