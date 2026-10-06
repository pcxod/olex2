# ptitSeb's GLU (Mesa GLU 9 with an Android.mk) ships no CMake. Olex2 needs the
# quadrics, gluPickMatrix and error strings only: libutil, no tessellator/NURBS.
# Built by android/build_deps.py against gl4es's GL headers.
cmake_minimum_required(VERSION 3.16)
project(olx_glu C)
set(GLU_SOURCE_DIR "" CACHE PATH "unpacked GLU tree")
set(GL_INCLUDE_DIR "" CACHE PATH "directory holding GL/gl.h (gl4es/include)")

add_library(GLU STATIC
  ${GLU_SOURCE_DIR}/src/libutil/error.c
  ${GLU_SOURCE_DIR}/src/libutil/glue.c
  ${GLU_SOURCE_DIR}/src/libutil/mipmap.c
  ${GLU_SOURCE_DIR}/src/libutil/project.c
  ${GLU_SOURCE_DIR}/src/libutil/quad.c
  ${GLU_SOURCE_DIR}/src/libutil/registry.c)
target_include_directories(GLU PRIVATE
  ${GLU_SOURCE_DIR}/include ${GLU_SOURCE_DIR}/src/include ${GL_INCLUDE_DIR})
target_compile_definitions(GLU PRIVATE LIBRARYBUILD)
target_compile_options(GLU PRIVATE -fsigned-char -fvisibility=hidden)

install(TARGETS GLU ARCHIVE DESTINATION lib)
install(FILES ${GLU_SOURCE_DIR}/include/GL/glu.h ${GLU_SOURCE_DIR}/include/GL/glu_mangle.h
  DESTINATION include/GL)
