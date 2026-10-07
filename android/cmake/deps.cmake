# The GUI base libraries for one ABI, prebuilt by android/build_deps.py from
# the pinned upstream archives (URLs and SHA256 in android/README.md) with
# android/patches/<name>/*.patch applied:
#   <OLX_ANDROID_PREFIX>/<ANDROID_ABI>/wx     wxWidgets 3.3.3, wxQt, static, GL on
#   <OLX_ANDROID_PREFIX>/<ANDROID_ABI>/gl4es  gl4es 1.1.6 libGL.a, GLU libGLU.a
# All static: they end up inside the app's own .so, so androiddeployqt has
# nothing extra to package.
set(OLX_ANDROID_PREFIX "D:/Android/prefix" CACHE PATH
  "build_deps.py install prefix (one sub-directory per ABI)")
set(_olx_p "${OLX_ANDROID_PREFIX}/${ANDROID_ABI}")
foreach (f gl4es/lib/libGL.a gl4es/lib/libGLU.a wx/include)
  if (NOT EXISTS "${_olx_p}/${f}")
    message(FATAL_ERROR "${_olx_p}/${f} missing: run "
      "python android/build_deps.py --abi ${ANDROID_ABI}")
  endif ()
endforeach ()

add_library(gl4es_gl STATIC IMPORTED)
set_target_properties(gl4es_gl PROPERTIES
  IMPORTED_LOCATION "${_olx_p}/gl4es/lib/libGL.a"
  INTERFACE_INCLUDE_DIRECTORIES "${_olx_p}/gl4es/include"
  INTERFACE_LINK_LIBRARIES "dl;log;m")
# wxGLCanvas reaches these through weak references (patches/wxwidgets), and a
# weak reference does not pull an archive member: force them in
target_link_options(gl4es_gl INTERFACE "LINKER:-u,initialize_gl4es"
  "LINKER:-u,gl4es_setMainFBO" "LINKER:-u,gl4es_pre_swap" "LINKER:-u,gl4es_post_swap"
  "LINKER:-u,gl4es_resyncState")

add_library(olx_glu STATIC IMPORTED)
set_target_properties(olx_glu PROPERTIES
  IMPORTED_LOCATION "${_olx_p}/gl4es/lib/libGLU.a"
  INTERFACE_LINK_LIBRARIES gl4es_gl)

# wx's static config looks for OpenGL again; the NDK has no desktop GL
set(OPENGL_INCLUDE_DIR "${_olx_p}/gl4es/include" CACHE PATH "" FORCE)
set(OPENGL_gl_LIBRARY "${_olx_p}/gl4es/lib/libGL.a" CACHE FILEPATH "" FORCE)
set(OPENGL_glu_LIBRARY "${_olx_p}/gl4es/lib/libGLU.a" CACHE FILEPATH "" FORCE)
# the Qt modules static wxQt links (Test for wxUIActionSimulator)
if (OLX_QT EQUAL 5)
  find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL PrintSupport Test)
else ()
  find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets
    PrintSupport Test)
endif ()
find_package(Threads REQUIRED)
# the Qt toolchain re-roots package searches into the NDK sysroot
find_package(wxWidgets 3.3.3 REQUIRED CONFIG
  PATHS "${_olx_p}/wx" NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
# wx keeps wxBUILD_DEBUG_LEVEL to itself. A prefix built with 0 (build_deps.py,
# no asserts) has no assert handler, and code that includes wx headers must
# use the same level or it fails to link
file(STRINGS "${_olx_p}/wx/lib/libwx_baseu-3.3-Android.a" _olx_wx_assert
  LIMIT_COUNT 1 REGEX "wxTheAssertHandler")
if (NOT _olx_wx_assert)
  add_compile_definitions(wxDEBUG_LEVEL=0)
endif ()
