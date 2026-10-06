# Third-party sources for the Android build, fetched at configure time.
# Every download is pinned by SHA256: an empty hash stops the configure unless
# FETCHCONTENT_SOURCE_DIR_<NAME> points at an already unpacked tree, e.g.
#   -DFETCHCONTENT_SOURCE_DIR_WXWIDGETS=D:/src/wxWidgets-3.3.1
# Patches in android/patches/<name>/*.patch are applied to downloaded trees in
# name order (git apply); a local SOURCE_DIR tree is used as is.
include(FetchContent)

set(OLX_WX_VERSION "3.3.1" CACHE STRING "wxWidgets release (3.3+, wxQt on Qt6)")
set(OLX_WX_SHA256 "" CACHE STRING "SHA256 of wxWidgets-${OLX_WX_VERSION}.tar.bz2")
set(OLX_GL4ES_REF "v1.1.6" CACHE STRING "gl4es tag or commit")
set(OLX_GL4ES_SHA256 "" CACHE STRING "SHA256 of the gl4es archive")
set(OLX_GLU_VERSION "9.0.3" CACHE STRING "Mesa GLU release")
set(OLX_GLU_SHA256 "" CACHE STRING "SHA256 of glu-${OLX_GLU_VERSION}.tar.xz")

find_package(Git QUIET)

function(olx_declare name url sha)
  string(TOUPPER "${name}" uname)
  if (FETCHCONTENT_SOURCE_DIR_${uname})
    FetchContent_Declare(${name} URL "${url}")
    return()
  endif ()
  if ("${sha}" STREQUAL "")
    message(FATAL_ERROR "${name}: no SHA256 set. Download ${url}, check it, "
      "then pass its SHA256 (see android/README.md) or "
      "-DFETCHCONTENT_SOURCE_DIR_${uname}=<unpacked tree>")
  endif ()
  file(GLOB patches "${CMAKE_CURRENT_LIST_DIR}/../patches/${name}/*.patch")
  list(SORT patches)
  set(patch_cmd)
  if (patches)
    set(patch_cmd PATCH_COMMAND "${GIT_EXECUTABLE}" apply --whitespace=nowarn ${patches})
  endif ()
  FetchContent_Declare(${name} URL "${url}" URL_HASH SHA256=${sha}
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE ${patch_cmd})
endfunction()

olx_declare(wxwidgets
  "https://github.com/wxWidgets/wxWidgets/releases/download/v${OLX_WX_VERSION}/wxWidgets-${OLX_WX_VERSION}.tar.bz2"
  "${OLX_WX_SHA256}")
olx_declare(gl4es
  "https://github.com/ptitSeb/gl4es/archive/${OLX_GL4ES_REF}.tar.gz"
  "${OLX_GL4ES_SHA256}")
olx_declare(glu
  "https://archive.mesa3d.org/glu/glu-${OLX_GLU_VERSION}.tar.xz"
  "${OLX_GLU_SHA256}")

# --- wxWidgets: the Qt port, static, bundled image/regex/expat libraries
set(wxBUILD_TOOLKIT "qt" CACHE STRING "" FORCE)
set(wxBUILD_SHARED OFF CACHE BOOL "" FORCE)
set(wxBUILD_MONOLITHIC OFF CACHE BOOL "" FORCE)
set(wxBUILD_SAMPLES OFF CACHE STRING "" FORCE)
set(wxBUILD_TESTS OFF CACHE STRING "" FORCE)
set(wxUSE_OPENGL ON CACHE BOOL "" FORCE)
foreach (lib REGEX ZLIB EXPAT LIBJPEG LIBPNG LIBTIFF)
  set(wxUSE_${lib} "builtin" CACHE STRING "" FORCE)
endforeach ()
foreach (opt WEBVIEW MEDIACTRL LIBSDL SECRETSTORE LIBWEBP LIBLZMA)
  set(wxUSE_${opt} OFF CACHE BOOL "" FORCE)
endforeach ()

# --- gl4es: desktop GL 1.x/2.x on GLES2, static, no X11/EGL of its own
set(NOX11 ON CACHE BOOL "" FORCE)
set(NOEGL ON CACHE BOOL "" FORCE)
set(STATICLIB ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(wxwidgets gl4es)

# gl4es's library target is GL; wrap it so the include dir travels with it
add_library(gl4es_gl INTERFACE)
target_link_libraries(gl4es_gl INTERFACE GL)
target_include_directories(gl4es_gl INTERFACE "${gl4es_SOURCE_DIR}/include")

# --- GLU: only libutil (quadrics, project/unproject, pick matrix, mipmaps,
# error strings); Olex2 uses no tessellator or NURBS. Mesa GLU ships no CMake.
FetchContent_GetProperties(glu)
if (NOT glu_POPULATED)
  FetchContent_Populate(glu)
endif ()
add_library(olx_glu STATIC
  "${glu_SOURCE_DIR}/src/libutil/error.c"
  "${glu_SOURCE_DIR}/src/libutil/glue.c"
  "${glu_SOURCE_DIR}/src/libutil/mipmap.c"
  "${glu_SOURCE_DIR}/src/libutil/project.c"
  "${glu_SOURCE_DIR}/src/libutil/quad.c"
  "${glu_SOURCE_DIR}/src/libutil/registry.c")
target_include_directories(olx_glu
  PUBLIC "${glu_SOURCE_DIR}/include"
  PRIVATE "${glu_SOURCE_DIR}/src/include")
target_link_libraries(olx_glu PUBLIC gl4es_gl)
