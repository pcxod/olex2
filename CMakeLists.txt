CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
PROJECT(olex2)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__WXWIDGETS__ -D_UNICODE")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPM.cmake")
add_subdirectory(3rdparty EXCLUDE_FROM_ALL)

# Example to get it working with Anaconda or Miniforge. Should work similarly with other environments:
# -DPYTHON_LIBRARY=<conda_home>/envs/<python3.8 environment>/lib/libpython3.8.so.1.0
# -DPYTHON_INCLUDE_DIR=<conda_home>/envs/<python3.8 environment>/include/python3.8
# -DPYTHON_ROOT_DIR=<conda_home>/envs/<python3.8 environment>
# -DPython3_FIND_STRATEGY=LOCATION

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
FIND_PACKAGE(PythonLibs 3.8 COMPONENTS Development REQUIRED)
message(STATUS "Python include: ${PYTHON_INCLUDE_DIRS}")
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
message(STATUS "Python version: ${PYTHONLIBS_VERSION_STRING}")
message(STATUS "Python libraries: ${PYTHON_LIBRARIES}")
# additional modules
find_package(PkgConfig REQUIRED)
pkg_check_modules(PANGOFT2 REQUIRED pangoft2)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(WAYLAND_CURSOR REQUIRED wayland-cursor) # Often needed for client apps
pkg_check_modules(WAYLAND_EGL REQUIRED wayland-egl)
set(OpenGL_GL_PREFERENCE "LEGACY")
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# Add user provided compile options
if (DEFINED $ENV{OLEX2_DIR})
  set(OLEX2_GUI $ENV{OLEX2_DIR})
endif ()

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(COPY_GUI_FILES "Copy the GUI files to the build location for testing" OFF)
  set(OLEX2_GUI_DIRECTORY ${OLEX2_GUI} CACHE PATH "Directory to copy from if COPY_GUI_FILES is set to ON")
endif ()

if (OLEX2_GUI_DIRECTORY STREQUAL "" AND COPY_GUI_FILES)
  message(FATAL_ERROR "COPY_GUI_FILES is only supported with OLEX2_GUI_DIRECTORY CMAKE variable or OLEX2_GUI env variable.")
endif ()
# END user provided compile options

SET(src "${CMAKE_CURRENT_SOURCE_DIR}")
FILE(GLOB alglib_SRCS "${src}/alglib/*.cpp")
FILE(GLOB sdl_SRCS "${src}/sdl/*.cpp")
FILE(GLOB sdlsmart_SRCS "${src}/sdl/smart/*.cpp")
FILE(GLOB sdlexp_SRCS "${src}/sdl/exparse/*.cpp")
FILE(GLOB esdl_math_SRCS "${src}/sdl/math/*.cpp")
FILE(GLOB xlib_SRCS "${src}/xlib/*.cpp")
FILE(GLOB xlibabsorb_SRCS "${src}/xlib/absorpc/*.cpp")
FILE(GLOB xlibhenke_SRCS "${src}/xlib/henke/*.cpp")
FILE(GLOB xlibmacro_SRCS "${src}/xlib/macro/*.cpp")
FILE(GLOB glib_SRCS "${src}/glib/*.cpp")
FILE(GLOB gxlib_SRCS "${src}/gxlib/*.cpp")

FILE(GLOB repo_SRCS "${src}/repository/*.cpp")

IF (NOT WIN32)
  LIST(REMOVE_ITEM repo_SRCS "${src}/repository/fsindex.cpp")
  LIST(REMOVE_ITEM repo_SRCS "${src}/repository/unzip.cpp")
  LIST(REMOVE_ITEM repo_SRCS "${src}/repository/zip.cpp")
endif (NOT WIN32)

FILE(GLOB unirun_SRCS "${src}/unirun/*.cpp")
FILE(GLOB olex2_SRCS "${src}/olex/*.cpp")
FILE(GLOB olex2_nui_SRCS "${src}/olex/nui/*.cpp")
FILE(GLOB olex2_html_SRCS "${src}/olex/html/*.cpp")
FILE(GLOB olex2_ctrls_SRCS "${src}/olex/ctrls/*.cpp")
SET(olex2_extras "${src}/olex/langdict.cpp" "${src}/olex/ctrls.cpp")
SET(all_SRCS
    ${alglib_SRCS}
    ${sdl_SRCS}
    ${sdlsmart_SRCS}
    ${sdlexp_SRCS}
    ${esdl_math_SRCS}
    ${olex2_nui_SRCS}
    ${xlib_SRCS}
    ${xlibabsorb_SRCS}
    ${xlibmacro_SRCS}
    ${xlibhenke_SRCS}
    ${glib_SRCS}
    ${gxlib_SRCS}
    ${repo_SRCS}
    ${olex2_html_SRCS}
    ${olex2_ctrls_SRCS}
    ${olex2_extras}
)


INCLUDE_DIRECTORIES("${src}/alglib")
INCLUDE_DIRECTORIES("${src}/glib")
INCLUDE_DIRECTORIES("${src}/gxlib")
INCLUDE_DIRECTORIES("${src}/olex")
INCLUDE_DIRECTORIES("${src}/olex/html")
INCLUDE_DIRECTORIES("${src}/olex/ctrls")
INCLUDE_DIRECTORIES("${src}/olex/nui")
INCLUDE_DIRECTORIES("${src}/repository")
INCLUDE_DIRECTORIES("${src}/sdl")
INCLUDE_DIRECTORIES("${src}/xlib")
INCLUDE_DIRECTORIES("${src}/xlib/absorpc")
INCLUDE_DIRECTORIES("${src}/sdl/math")
INCLUDE_DIRECTORIES("${wxWidgets_INCLUDE_DIR_2}")
INCLUDE_DIRECTORIES("${wxWidgets_INCLUDE_DIR_1}")
INCLUDE_DIRECTORIES(
    ${PANGOFT2_INCLUDE_DIRS}
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${WAYLAND_CURSOR_INCLUDE_DIRS}
    ${WAYLAND_EGL_INCLUDE_DIRS}
)
ADD_EXECUTABLE(unirun ${unirun_SRCS} ${all_SRCS})
ADD_EXECUTABLE(olex2 ${olex2_SRCS} ${all_SRCS} )

TARGET_LINK_LIBRARIES(unirun PRIVATE
    wx-static
    ${WX_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${Fontconfig_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${PANGOFT2_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    ${WAYLAND_CURSOR_LIBRARIES}
    EGL
    ${WAYLAND_EGL_LIBRARIES}
    GLU
    ${PYTHON_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)



TARGET_LINK_LIBRARIES(olex2 PRIVATE
    wx-static
    ${WX_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${Fontconfig_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${PANGOFT2_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    ${WAYLAND_CURSOR_LIBRARIES}
    EGL
    ${WAYLAND_EGL_LIBRARIES}
    GLU
    ${PYTHON_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)


target_compile_definitions(olex2 PRIVATE _PYTHON _OPENSSL)
target_compile_definitions(unirun PRIVATE _PYTHON _OPENSSL)
if (COPY_GUI_FILES)
  add_custom_command(TARGET olex2 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
      "${OLEX2_GUI_DIRECTORY}"
      "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Copying files to build directory"
  )
endif ()
