# Make wxWidgets version configurable with default value
set(wxVERSION "3.2.5" CACHE STRING "wxWidgets version to use")
# Extract the family version (major.minor) from the full version
string(REGEX MATCH "^([0-9]+\\.[0-9]+)" wxVERSION_FAMILY "${wxVERSION}")
if(NOT wxVERSION_FAMILY)
    message(FATAL_ERROR "Invalid wxVERSION format: ${wxVERSION}. Expected format: X.Y.Z")
endif()

CPMAddPackage(
  NAME Ccache.cmake
  GITHUB_REPOSITORY TheLartians/Ccache.cmake
  VERSION 1.2.5
)

set(wxWidgets_USE_STATIC "ON")
set(wxWidgets_CONFIG_OPTIONS "--linkdeps --libs")

CPMAddPackage(
    NAME wxWidgets
    VERSION "${wxVERSION}"
    URL "https://github.com/wxWidgets/wxWidgets/releases/download/v${wxVERSION}/wxWidgets-${wxVERSION}.tar.bz2"
    OPTIONS
        "wxBUILD_SHARED OFF"
        "wxUSE_REGEX builtin"
#        "wxUSE_ZLIB builtin"
        "wxUSE_EXPAT builtin"
        "wxUSE_LIBJPEG builtin"
        "wxUSE_LIBPNG builtin"
        "wxUSE_LIBTIFF builtin"
)
set(wxWidgets_ROOT_DIR ${wxWidgets_BINARY_DIR} PARENT_SCOPE)
set(wxWidgets_SOURCE_DIR ${wxWidgets_SOURCE_DIR} PARENT_SCOPE)
add_library(wx-static INTERFACE IMPORTED GLOBAL)
add_dependencies(wx-static wxWidgets)
message(WARNING ${wxWidgets_USE_FILE})
