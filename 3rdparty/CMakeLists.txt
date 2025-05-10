CPMAddPackage(
    NAME wxWidgetsSource
    VERSION "3.1.5"
    URL "https://github.com/wxWidgets/wxWidgets/releases/download/v3.0.5/wxWidgets-3.0.5.tar.bz2"
    DOWNLOAD_ONLY YES
)

include(ExternalProject)
if(wxWidgetsSource_ADDED)
  message(STATUS "wxWidgets source downloaded to: ${wxWidgetsSource_SOURCE_DIR}")
  set(wxWidgets_INSTALL_DIR ${CMAKE_BINARY_DIR}/3rdparty/wxwidgets)
  set(wxWidgets_BUILD_DIR ${CMAKE_BINARY_DIR}/3rdparty/wxwidgets_build)

  set(WX_COMPONENTS_ARGS --enable-unicode --enable-gl --enable-html --enable-aui --enable-adv)

  ExternalProject_Add(
      wxWidgetsBuild
      SOURCE_DIR ${wxWidgetsSource_SOURCE_DIR}
      BINARY_DIR ${wxWidgetsSource_SOURCE_DIR}/build

      CONFIGURE_COMMAND ../configure CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
      --disable-shared
      --with-gtk2
      --prefix=${CMAKE_BINARY_DIR}/3rdparty/wxwidgets
      ${WX_COMPONENTS_ARGS}
      --disable-sys-libs
      --with-libjpeg=builtin
      --with-libtiff=builtin
      BUILD_COMMAND make -j
      INSTALL_COMMAND make install
      LOG_CONFIGURE ON
      LOG_BUILD ON
      LOG_INSTALL ON

      BUILD_BYPRODUCTS
      ${wxWidgets_INSTALL_DIR}/bin/wx-config
      # GTK Components (depend on base)
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_gl-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_core-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_aui-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_adv-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_html-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_propgrid-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_qa-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_ribbon-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_richtext-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_stc-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_xrc-3.0.a

      ${wxWidgets_INSTALL_DIR}/lib/libwxregexu-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxscintilla-3.0.a

      ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxtiff-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxpng-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-3.0.a

      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_net-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_xml-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu-3.0.a

  )
  set(wxWidgets_LIB_DIR ${wxWidgets_INSTALL_DIR}/lib PARENT_SCOPE)
  add_library(wx-static INTERFACE IMPORTED GLOBAL)
  add_dependencies(wx-static wxWidgetsBuild)

  set(WXWIDGETS_SETUP_H_INCLUDE_DIR ${wxWidgets_INSTALL_DIR}/lib/wx/include/gtk2-unicode-static-3.0)
  set(wxWidgets_INCLUDE_DIR_1
      ${wxWidgets_INSTALL_DIR}/include/wx-3.0
      PARENT_SCOPE
  )
  set(wxWidgets_INCLUDE_DIR_2
      ${WXWIDGETS_SETUP_H_INCLUDE_DIR}
      PARENT_SCOPE
  )
  file(MAKE_DIRECTORY ${wxWidgets_INSTALL_DIR}/include/wx-3.0)
  file(MAKE_DIRECTORY ${WXWIDGETS_SETUP_H_INCLUDE_DIR})
  target_include_directories(wx-static INTERFACE
      ${WXWIDGETS_SETUP_H_INCLUDE_DIR}
      ${wxWidgets_INSTALL_DIR}/include/wx-3.0
  )

  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
    pkg_check_modules(X11 REQUIRED x11)
  else()
    message(WARNING "pkg-config not found. Compiler flags for wxWidgets dependencies might be incomplete.")
  endif()
  target_compile_options(wx-static INTERFACE
      "-pthread"                     # Common for GTK/X11 apps
      ${GTK2_CFLAGS_OTHER}          # Other flags from pkg-config
      ${X11_CFLAGS_OTHER}
      "-D__WXGTK__"                # Define the toolkit
  )
  target_include_directories(wx-static INTERFACE
      ${GTK2_INCLUDE_DIRS}
      ${X11_INCLUDE_DIRS}
  )


  set(WX_LIBRARIES
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_gl-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_core-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_aui-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_adv-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_html-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_propgrid-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_qa-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_ribbon-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_richtext-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_stc-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_gtk2u_xrc-3.0.a

      ${wxWidgets_INSTALL_DIR}/lib/libwxregexu-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxscintilla-3.0.a

      ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxpng-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxtiff-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-3.0.a

      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_net-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu_xml-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwx_baseu-3.0.a

  )
  set(WX_LIBRARIES ${WX_LIBRARIES} PARENT_SCOPE)
  set(WX_SYSTEM_LIBS
      ${GTK2_LIBRARIES}
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
      ${wxWidgets_INSTALL_DIR}/lib/libwxjpeg-3.0.a
      ${wxWidgets_INSTALL_DIR}/lib/libwxpng-3.0.a ${wxWidgets_INSTALL_DIR}/lib/libwxzlib-3.0.a
      ${WX_SYSTEM_LIBS})


  message(STATUS "wxWidgets INTERFACE target created.")
  message(STATUS "  Include Dirs: ${wxWidgets_INSTALL_DIR}/include/wx-3.0;${WXWIDGETS_SETUP_H_INCLUDE_DIR}")
  message(STATUS "  Libraries: <See target_link_libraries call above>")
  message(WARNING "wxWidgets linking flags and system dependencies are hardcoded guesses based on common static GTK builds. Verify these for your platform and build options!")

else()
  message(FATAL_ERROR "Failed to download wxWidgets source using CPM.")
endif()
