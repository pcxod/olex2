set(wxVERSION_FAMILY "3.2")
set(wxVERSION "${wxVERSION_FAMILY}.5")
CPMAddPackage(
    NAME wxWidgetsSource
    VERSION "${wxVERSION}"
    URL "https://github.com/wxWidgets/wxWidgets/releases/download/v${wxVERSION}/wxWidgets-${wxVERSION}.tar.bz2"
    DOWNLOAD_ONLY YES
)

include(ExternalProject)
if(wxWidgetsSource_ADDED)
  message(STATUS "wxWidgets source downloaded to: ${wxWidgetsSource_SOURCE_DIR}")
  set(wxWidgets_INSTALL_DIR ${CMAKE_BINARY_DIR}/3rdparty/wxwidgets)
  set(wxWidgets_BUILD_DIR ${CMAKE_BINARY_DIR}/3rdparty/wxwidgets_build)

  ExternalProject_Add(
      wxWidgetsBuild
      SOURCE_DIR ${wxWidgetsSource_SOURCE_DIR}
      BINARY_DIR ${wxWidgetsSource_SOURCE_DIR}/buildgtk

      CONFIGURE_COMMAND ../configure CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
      --disable-shared
      --prefix=${CMAKE_BINARY_DIR}/3rdparty/wxwidgets
      --enable-aui
      --enable-unicode
      --enable-html
      --disable-sys-libs
      --with-gtk=3
      --with-libjpeg=builtin
      --with-libtiff=builtin
      BUILD_COMMAND make -j${EXTERNAL_JOBS}
      INSTALL_COMMAND make install
      LOG_CONFIGURE ON
      LOG_BUILD ON
      LOG_INSTALL ON

      BUILD_BYPRODUCTS
      ${wxWidgets_INSTALL_DIR}/bin/wx-config
      # GTK Components (depend on base)
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_gl-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_core-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_aui-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_adv-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_html-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_propgrid-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_qa-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_ribbon-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_richtext-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_stc-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_xrc-${wxVERSION_FAMILY}.a

      ${wxWidgets_INSTALL_DIR}/lib/libwxregexu-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxscintilla-${wxVERSION_FAMILY}.a

      ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxtiff-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxpng-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-${wxVERSION_FAMILY}.a

      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_net-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_xml-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu-${wxVERSION_FAMILY}.a

  )
  set(wxWidgets_LIB_DIR ${wxWidgets_INSTALL_DIR}/lib PARENT_SCOPE)
  add_library(wx-static INTERFACE IMPORTED GLOBAL)
  add_dependencies(wx-static wxWidgetsBuild)

  set(WXWIDGETS_SETUP_H_INCLUDE_DIR ${wxWidgets_INSTALL_DIR}/lib/wx/include/gtk3-unicode-static-${wxVERSION_FAMILY})
  set(wxWidgets_INCLUDE_DIR_1
      ${wxWidgets_INSTALL_DIR}/include/wx-${wxVERSION_FAMILY}
      PARENT_SCOPE
  )
  set(wxWidgets_INCLUDE_DIR_2
      ${WXWIDGETS_SETUP_H_INCLUDE_DIR}
      PARENT_SCOPE
  )
  file(MAKE_DIRECTORY ${wxWidgets_INSTALL_DIR}/include/wx-${wxVERSION_FAMILY})
  file(MAKE_DIRECTORY ${WXWIDGETS_SETUP_H_INCLUDE_DIR})
  target_include_directories(wx-static INTERFACE
      ${WXWIDGETS_SETUP_H_INCLUDE_DIR}
      ${wxWidgets_INSTALL_DIR}/include/wx-${wxVERSION_FAMILY}
  )

  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(X11 REQUIRED x11)
  else()
    message(WARNING "pkg-config not found. Compiler flags for wxWidgets dependencies might be incomplete.")
  endif()
  target_compile_options(wx-static INTERFACE
      "-pthread"                     # Common for GTK/X11 apps
      ${GTK3_CFLAGS_OTHER}          # Other flags from pkg-config
      ${X11_CFLAGS_OTHER}
      "-D__WXGTK__"                # Define the toolkit
  )
  target_include_directories(wx-static INTERFACE
      ${GTK3_INCLUDE_DIRS}
      ${X11_INCLUDE_DIRS}
  )


  set(WX_LIBRARIES
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_gl-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_core-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_aui-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_adv-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_html-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_propgrid-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_qa-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_ribbon-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_richtext-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_stc-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk3u_xrc-${wxVERSION_FAMILY}.a

      ${wxWidgets_INSTALL_DIR}/lib/libwxregexu-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxscintilla-${wxVERSION_FAMILY}.a

      ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxpng-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxtiff-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-${wxVERSION_FAMILY}.a

      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_net-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_xml-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu-${wxVERSION_FAMILY}.a

  )
  set(WX_LIBRARIES ${WX_LIBRARIES} PARENT_SCOPE)
  set(WX_SYSTEM_LIBS
      ${GTK3_LIBRARIES}
      X11
      SM
      dl
      m
      pthread
      lzma
  )
  set(WX_SYSTEM_LIBS ${WX_SYSTEM_LIBS} PARENT_SCOPE)
  target_link_libraries(wx-static INTERFACE
      ${WX_LIBRARIES}
      ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-${wxVERSION_FAMILY}.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxpng-${wxVERSION_FAMILY}.a ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-${wxVERSION_FAMILY}.a
      ${WX_SYSTEM_LIBS})


  message(STATUS "wxWidgets INTERFACE target created.")
  message(STATUS "  Include Dirs: ${wxWidgets_INSTALL_DIR}/include/wx-${wxVERSION_FAMILY};${WXWIDGETS_SETUP_H_INCLUDE_DIR}")
  message(STATUS "  Libraries: <See target_link_libraries call above>")
  message(WARNING "wxWidgets linking flags and system dependencies are hardcoded guesses based on common static GTK builds. Verify these for your platform and build options!")

else()
  message(FATAL_ERROR "Failed to download wxWidgets source using CPM.")
endif()

if("${EXTERNAL_PYTHON_BINDINGS}")
CPMAddPackage(
    NAME nanobind
    GITHUB_REPOSITORY "wjakob/nanobind"
    GIT_TAG "v2.7.0"
)
endif()
