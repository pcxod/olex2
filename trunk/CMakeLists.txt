
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(olex2)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__WXWIDGETS__ -D_UNICODE")

FIND_PACKAGE(PythonLibs REQUIRED)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

SET(wxWidgets_USE_UNICODE ON)
FIND_PACKAGE(wxWidgets REQUIRED COMPONENTS base core gl html net aui)
INCLUDE(${wxWidgets_USE_FILE})


SET(src "${CMAKE_CURRENT_SOURCE_DIR}")
FILE(GLOB alglib_SRCS "${src}/alglib/*.cpp")
FILE(GLOB sdl_SRCS "${src}/sdl/*.cpp")
FILE(GLOB sdlsmart_SRCS "${src}/sdl/smart/*.cpp")
FILE(GLOB sdlexp_SRCS "${src}/sdl/exparse/*.cpp")
FILE(GLOB xlib_SRCS "${src}/xlib/*.cpp")
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
FILE(GLOB olex2_html_SRCS "${src}/olex/html/*.cpp")
FILE(GLOB olex2_ctrls_SRCS "${src}/olex/ctrls/*.cpp")

SET(all_SRCS
        ${alglib_SRCS}
        ${sdl_SRCS}
        ${sdlsmart_SRCS}
        ${sdlexp_SRCS}
        ${xlib_SRCS}
        ${xlibmacro_SRCS}
        ${glib_SRCS}
        ${gxlib_SRCS}
        ${repo_SRCS})
        ${olex2_html_SRCS})
        ${olex2_ctrls_SRCS})

INCLUDE_DIRECTORIES("${src}/alglib")
INCLUDE_DIRECTORIES("${src}/glib")
INCLUDE_DIRECTORIES("${src}/gxlib")
INCLUDE_DIRECTORIES("${src}/olex")
INCLUDE_DIRECTORIES("${src}/olex/html")
INCLUDE_DIRECTORIES("${src}/olex/ctrls")
INCLUDE_DIRECTORIES("${src}/repository")
INCLUDE_DIRECTORIES("${src}/sdl")
INCLUDE_DIRECTORIES("${src}/xlib")

ADD_EXECUTABLE(unirun ${unirun_SRCS} ${all_SRCS})
TARGET_LINK_LIBRARIES(unirun ${wxWidgets_LIBRARIES} ${PYTHON_LIBRARIES})

ADD_EXECUTABLE(olex2 ${olex2_SRCS} ${all_SRCS})
TARGET_LINK_LIBRARIES(olex2 ${wxWidgets_LIBRARIES} ${PYTHON_LIBRARIES})
