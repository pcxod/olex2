/******************************************************************************
 * Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
 *                                                                             *
 * This file is part of the OlexSys Development Framework.                     *
 *                                                                             *
 * This source file is distributed under the terms of the licence located in   *
 * the root folder.                                                            *
 ******************************************************************************/

#ifndef __olx_gl_glbase_H
#define __olx_gl_glbase_H

#define BeginGlNamespace()  namespace glObj {
#define EndGlNamespace()  };\
  using namespace glObj;
#define UseGlNamespace()  using namespace glObj;
#define GlobalGlFunction( fun )     glObj::fun

#include "ebase.h"
#include "gldefs.h"
#include "ebasis.h"

#if defined __APPLE__ && defined __MACH__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

BeginGlNamespace()

struct olx_gl {

  template <typename MC> static void orient(const MC& m) {
    static float Bf[4][4];
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        Bf[i][j] = (float) m[i][j];
    Bf[0][3] = Bf[1][3] = Bf[2][3] = Bf[3][0] = Bf[3][1] = Bf[3][2] = 0;
    Bf[3][3] = 1.0;
    glMultMatrixf(&Bf[0][0]);
  }

  static void orient(const TEBasis& b) {
    glMultMatrixf(b.GetMData());
  }

  static void orient(const float *m) {
    glMultMatrixf(m);
  }

  static void orient(const double *m) {
    glMultMatrixd(m);
  }

  static void loadMatrix(const TEBasis& b) {
    glLoadMatrixf(b.GetMData());
  }

  static void loadMatrix(const float *m) {
    glLoadMatrixf(m);
  }

  static void loadMatrix(const double *m) {
    glLoadMatrixd(m);
  }

  static void loadIdentity() {
    glLoadIdentity();
  }

  static void translate(int _x, int _y, int _z) {
    glTranslated(_x, _y, _z);
  }

  static void translate(float _x, float _y, float _z) {
    glTranslatef(_x, _y, _z);
  }

  static void translate(double _x, double _y, double _z) {
    glTranslated(_x, _y, _z);
  }

  static void translate(const vec3f& trans) {
    glTranslatef(trans[0], trans[1], trans[2]);
  }

  static void translate(const vec3d& trans) {
    glTranslated(trans[0], trans[1], trans[2]);
  }

  static void translate(const float* trans) {
    glTranslatef(trans[0], trans[1], trans[2]);
  }

  static void translate(const double* trans) {
    glTranslated(trans[0], trans[1], trans[2]);
  }

  static void rotate(int angle, int x, int y, int z) {
    glRotated(angle, x, y, z);
  }

  static void rotate(float angle, float x, float y, float z) {
    glRotatef(angle, x, y, z);
  }

  static void rotate(double angle, double x, double y, double z) {
    glRotated(angle, x, y, z);
  }

  static void rotate(int angle, const vec3i& v) {
    rotate(angle, v[0], v[1], v[2]);
  }

  static void rotate(float angle, const vec3f& v) {
    rotate(angle, v[0], v[1], v[2]);
  }

  static void rotate(double angle, const vec3d& v) {
    rotate(angle, v[0], v[1], v[2]);
  }

  static void scale(int _x, int _y, int _z) {
    glScaled(_x, _y, _z);
  }

  static void scale(float _x, float _y, float _z) {
    glScalef(_x, _y, _z);
  }

  static void scale(double _x, double _y, double _z) {
    glScaled(_x, _y, _z);
  }

  static void scale(int S) {
    scale(S, S, S);
  }

  static void scale(float S) {
    scale(S, S, S);
  }

  static void scale(double S) {
    scale(S, S, S);
  }

  static void vertex(const vec3f& v) {
    glVertex3fv(v.GetData());
  }

  static void vertex(const vec3d& v) {
    glVertex3dv(v.GetData());
  }

  static void vertex(const float* v) {
    glVertex3fv(v);
  }

  static void vertex(const double* v) {
    glVertex3dv(v);
  }

  static void vertex(int x, int y) {
    glVertex2d(x, y);
  }

  static void vertex(float x, float y) {
    glVertex2f(x, y);
  }

  static void vertex(double x, double y) {
    glVertex2d(x, y);
  }

  static void vertex(int x, int y, int z) {
    glVertex3d(x, y, z);
  }

  static void vertex(float x, float y, float z) {
    glVertex3f(x, y, z);
  }

  static void vertex(double x, double y, double z) {
    glVertex3d(x, y, z);
  }

  static void rasterPos(const vec3f& v) {
    rasterPos(v.GetData());
  }

  static void rasterPos(const vec3d& v) {
    rasterPos(v.GetData());
  }

  static void rasterPos(const float* v) {
    glRasterPos3fv(v);
  }

  static void rasterPos(const double* v) {
    glRasterPos3dv(v);
  }

  static void rasterPos(int x, int y) {
    glRasterPos2d(x, y);
  }

  static void rasterPos(float x, float y) {
    glRasterPos2f(x, y);
  }

  static void rasterPos(double x, double y) {
    glRasterPos2d(x, y);
  }

  static void rasterPos(int x, int y, int z) {
    glRasterPos3d(x, y, z);
  }

  static void rasterPos(float x, float y, float z) {
    glRasterPos3f(x, y, z);
  }

  static void rasterPos(double x, double y, double z) {
    glRasterPos3d(x, y, z);
  }

  static void normal(const vec3f& v) {
    normal(v.GetData());
  }

  static void normal(const vec3d& v) {
    normal(v.GetData());
  }

  static void normal(const float* v) {
    glNormal3fv(v);
  }

  static void normal(const double* v) {
    glNormal3dv(v);
  }

  static void normal(int x, int y, int z) {
    glNormal3i(x, y, z);
  }

  static void normal(float x, float y, float z) {
    glNormal3f(x, y, z);
  }

  static void normal(double x, double y, double z) {
    glNormal3d(x, y, z);
  }

  static void color(const float* v) {
    glColor4fv(v);
  }

  static void color(const double* v) {
    glColor4dv(v);
  }

  static void color(int x, int y, int z) {
    glColor3i(x, y, z);
  }

  static void color(float x, float y, float z) {
    glColor3f(x, y, z);
  }

  static void color(double x, double y, double z) {
    glColor3d(x, y, z);
  }

  static void color(int x, int y, int z, int a) {
    glColor4i(x, y, z, a);
  }

  static void color(float x, float y, float z, float a) {
    glColor4f(x, y, z, a);
  }

  static void color(double x, double y, double z, double a) {
    glColor4d(x, y, z, a);
  }

  template <typename cl_t>
    static void color(cl_t v) {
    glColor4f(
      (float) OLX_GetRValue(v) / 255,
      (float) OLX_GetGValue(v) / 255,
      (float) OLX_GetBValue(v) / 255,
      (float) OLX_GetAValue(v) / 255);
  }

  static void material(GLenum face, GLenum pname, GLint param) {
    glMateriali(face, pname, param);
  }

  static void material(GLenum face, GLenum pname, GLfloat param) {
    glMaterialf(face, pname, param);
  }

  static void material(GLenum face, GLenum pname, const GLint* param) {
    glMaterialiv(face, pname, param);
  }

  static void material(GLenum face, GLenum pname, const GLfloat* param) {
    glMaterialfv(face, pname, param);
  }

  static void colorMaterial(GLenum face, GLenum mode) {
    glColorMaterial(face, mode);
  }

  static void colorMask(bool r, bool g, bool b, bool a) {
    glColorMask(r, g, b, a);
  }

  static void bindTexture(GLenum target, GLuint tex) {
    glBindTexture(target, tex);
  }

  static void texImage(GLenum target, GLint level, GLint internalFormat,
    GLsizei width, GLint border, GLint format, GLenum type, const GLvoid *pixels)
  {
    glTexImage1D(target, level, internalFormat, width, border, format,
      type, pixels);
  }

  static void texImage(GLenum target, GLint level, GLint internalFormat,
    GLsizei width, GLsizei height, GLint border, GLint format, GLenum type,
    const GLvoid *pixels)
  {
    glTexImage2D(target, level, internalFormat, width, height, border, format,
      type, pixels);
  }

  static void getTexImage(GLenum target, GLint level, GLenum format, GLenum type,
    GLvoid* data)
  {
    glGetTexImage(target, level, format, type, data);
  }

  static void deleteTextures(GLsizei n, const GLuint* textures) {
    glDeleteTextures(n, textures);
  }

  static void genTextures(GLsizei n, GLuint* textures) {
    glGenTextures(n, textures);
  }

  static void bitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
    GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) {
    glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
  }

  static void pixelStore(GLenum pname, GLint param) {
    glPixelStorei(pname, param);
  }

  static void pixelStore(GLenum pname, GLfloat param) {
    glPixelStoref(pname, param);
  }

  static void texCoord(int s) {
    glTexCoord1i(s);
  }

  static void texCoord(float s) {
    glTexCoord1f(s);
  }

  static void texCoord(double s) {
    glTexCoord1d(s);
  }

  static void texCoord(int s, int t) {
    glTexCoord2i(s, t);
  }

  static void texCoord(float s, float t) {
    glTexCoord2f(s, t);
  }

  static void texCoord(double s, double t) {
    glTexCoord2d(s, t);
  }

  static void texCoord(int s, int t, int r) {
    glTexCoord3i(s, t, r);
  }

  static void texCoord(float s, float t, float r) {
    glTexCoord3f(s, t, r);
  }

  static void texCoord(double s, double t, double r) {
    glTexCoord3d(s, t, r);
  }

  static void texCoord(int s, int t, int r, int q) {
    glTexCoord4i(s, t, r, q);
  }

  static void texCoord(float s, float t, float r, float q) {
    glTexCoord4f(s, t, r, q);
  }

  static void texCoord(double s, double t, double r, float q) {
    glTexCoord4d(s, t, r, q);
  }

  static void texGen(GLenum coord, GLenum pname, GLint param) {
    glTexGeni(coord, pname, param);
  }

  static void texGen(GLenum coord, GLenum pname, const float* param) {
    glTexGenfv(coord, pname, param);
  }

  static void texGen(GLenum coord, GLenum pname, const double* param) {
    glTexGendv(coord, pname, param);
  }

  static void texParam(GLenum target, GLenum pname, GLint param) {
    glTexParameteri(target, pname, param);
  }

  static void texParam(GLenum target, GLenum pname, GLfloat param) {
    glTexParameterf(target, pname, param);
  }

  static void texParam(GLenum target, GLenum pname, const GLint* param) {
    glTexParameteriv(target, pname, param);
  }

  static void texParam(GLenum target, GLenum pname, const GLfloat* param) {
    glTexParameterfv(target, pname, param);
  }

  static void getTexParam(GLenum target, GLenum pname, GLint* dest) {
    glGetTexParameteriv(target, pname, dest);
  }

  static void getTexParam(GLenum target, GLenum pname, GLfloat* dest) {
    glGetTexParameterfv(target, pname, dest);
  }

  static void getTexLevelParam(GLenum target, GLint level, GLenum pname,
    GLfloat* dest) {
    glGetTexLevelParameterfv(target, level, pname, dest);
  }

  static void getTexLevelParam(GLenum target, GLint level, GLenum pname,
    GLint* dest) {
    glGetTexLevelParameteriv(target, level, pname, dest);
  }

  static void texEnv(GLenum target, GLenum pname, GLint param) {
    glTexEnvi(target, pname, param);
  }

  static void texEnv(GLenum target, GLenum pname, GLfloat param) {
    glTexEnvf(target, pname, param);
  }

  static void texEnv(GLenum target, GLenum pname, const GLint* param) {
    glTexEnviv(target, pname, param);
  }

  static void texEnv(GLenum target, GLenum pname, const GLfloat* param) {
    glTexEnvfv(target, pname, param);
  }

  static void getTexEnv(GLenum target, GLenum pname, GLint* dest) {
    glGetTexEnviv(target, pname, dest);
  }

  static void getTexEnv(GLenum target, GLenum pname, GLfloat* dest) {
    glGetTexEnvfv(target, pname, dest);
  }

  static void getTexGen(GLenum coord, GLenum pname, GLint* dest) {
    glGetTexGeniv(coord, pname, dest);
  }

  static void getTexGen(GLenum coord, GLenum pname, GLfloat* dest) {
    glGetTexGenfv(coord, pname, dest);
  }

  static void getTexGen(GLenum coord, GLenum pname, GLdouble* dest) {
    glGetTexGendv(coord, pname, dest);
  }

  static void enable(GLenum v) {
    glEnable(v);
  }

  static bool isEnabled(GLenum param) {
    return glIsEnabled(param) == GL_TRUE;
  }

  static void disable(GLenum v) {
    glDisable(v);
  }

  static void begin(GLenum v) {
    glBegin(v);
  }

  static void end() {
    glEnd();
  }

  static void get(GLenum param, GLboolean* dest) {
    glGetBooleanv(param, dest);
  }

  static void get(GLenum param, GLint* dest) {
    glGetIntegerv(param, dest);
  }

  static void get(GLenum param, GLfloat* dest) {
    glGetFloatv(param, dest);
  }

  static void get(GLenum param, GLdouble* dest) {
    glGetDoublev(param, dest);
  }

  static GLint getInt(GLenum param) {
    GLint v[4]; // safety sake
    glGetIntegerv(param, v);
    return v[0];
  }

  static void blendFunc(GLenum sfactor, GLenum dfactor) {
    glBlendFunc(sfactor, dfactor);
  }

  static void alphaFunc(GLenum func, GLfloat ref) {
    glAlphaFunc(func, ref);
  }

  static void callList(GLuint list) {
    glCallList(list);
  }

  static void newList(GLuint list, GLenum mode) {
    glNewList(list, mode);
  }

  static void endList() {
    glEndList();
  }

  static GLuint genLists(GLsizei range) {
    return glGenLists(range);
  }

  static void deleteLists(GLuint list, GLsizei range) {
    glDeleteLists(list, range);
  }

  static void shadeModel(GLenum mode) {
    glShadeModel(mode);
  }

  static void cullFace(GLenum back_or_front) {
    glCullFace(back_or_front);
  }

  static void lineWidth(GLfloat width) {
    glLineWidth(width);
  }

  static void lineWidth(GLdouble width) {
    glLineWidth(static_cast<GLfloat> (width));
  }

  static void pointSize(GLfloat size) {
    glPointSize(size);
  }

  static void clearColor(const GLfloat* v) {
    glClearColor(v[0], v[1], v[2], v[3]);
  }

  static void clearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    glClearColor(r, g, b, a);
  }

  static void clearDepth(GLfloat v) {
    glClearDepth(v);
  }

  static void depthFunc(GLenum func) {
    glDepthFunc(func);
  }

  static void clear(GLbitfield mask) {
    glClear(mask);
  }

  static void clearAccum(const GLfloat* v) {
    glClearAccum(v[0], v[1], v[2], v[3]);
  }

  static void clearAccum(GLfloat r, GLfloat g, GLfloat b, GLfloat a) {
    glClearAccum(r, g, b, a);
  }

  static void accum(GLenum op, GLfloat value) {
    glAccum(op, value);
  }

  static void stencilOp(GLenum fail, GLenum zfail, GLenum zpass) {
    glStencilOp(fail, zfail, zpass);
  }

  static void stencilFunc(GLenum func, GLint ref, GLuint mask) {
    glStencilFunc(func, ref, mask);
  }

  static void hint(GLenum target, GLenum mode) {
    glHint(target, mode);
  }

  static void lightModel(GLenum pname, GLint param) {
    glLightModeli(pname, param);
  }

  static void lightModel(GLenum pname, GLfloat param) {
    glLightModelf(pname, param);
  }

  static void lightModel(GLenum pname, const GLint* param) {
    glLightModeliv(pname, param);
  }

  static void lightModel(GLenum pname, const GLfloat* param) {
    glLightModelfv(pname, param);
  }

  static void light(GLenum light, GLenum pname, GLint param) {
    glLighti(light, pname, param);
  }

  static void light(GLenum light, GLenum pname, GLfloat param) {
    glLightf(light, pname, param);
  }

  static void light(GLenum light, GLenum pname, const GLint* param) {
    glLightiv(light, pname, param);
  }

  static void light(GLenum light, GLenum pname, const GLfloat* param) {
    glLightfv(light, pname, param);
  }

  static void fog(GLenum pname, GLint param) {
    glFogi(pname, param);
  }

  static void fog(GLenum pname, GLfloat param) {
    glFogf(pname, param);
  }

  static void fog(GLenum pname, const GLint* param) {
    glFogiv(pname, param);
  }

  static void fog(GLenum pname, const GLfloat* param) {
    glFogfv(pname, param);
  }

  static void pushAttrib(GLbitfield mask) {
    glPushAttrib(mask);
  }

  static void pushMatrix() {
    glPushMatrix();
  }

  static void popAttrib() {
    glPopAttrib();
  }

  static void popMatrix() {
    glPopMatrix();
  }

  static void viewport(GLint left, GLint top, GLsizei width, GLsizei height) {
    glViewport(left, top, width, height);
  }

  static void matrixMode(GLenum mode) {
    glMatrixMode(mode);
  }

  static void frustum(GLdouble left, GLdouble right,
    GLdouble bottom, GLdouble top, GLdouble _near, GLdouble _far) {
    glFrustum(left, right, bottom, top, _near, _far);
  }

  static void ortho(GLdouble left, GLdouble right,
    GLdouble bottom, GLdouble top, GLdouble _near, GLdouble _far) {
    glOrtho(left, right, bottom, top, _near, _far);
  }

  static void drawBuffer(GLenum mode) {
    glDrawBuffer(mode);
  }

  static void readBuffer(GLenum mode) {
    glReadBuffer(mode);
  }

  static void readPixels(GLint left, GLint top, GLsizei width, GLsizei height,
    GLenum format, GLenum type, GLvoid* pixels) {
    glReadPixels(left, top, width, height, format, type, pixels);
  }

  static void drawPixels(GLsizei width, GLsizei height, GLenum format,
    GLenum type, const GLvoid* data) {
    glDrawPixels(width, height, format, type, data);
  }

  static GLint renderMode(GLenum mode) {
    return glRenderMode(mode);
  }

  static void polygonMode(GLenum face, GLenum mode) {
    glPolygonMode(face, mode);
  }

  static void polygonStipple(const GLubyte *mask) {
    glPolygonStipple(mask);
  }

  static void lineStipple(GLint factor, GLushort pattern) {
    glLineStipple(factor, pattern);
  }

  static void loadName(GLuint name) {
    glLoadName(name);
  }

  static void pushName(GLuint name) {
    glPushName(name);
  }

  static void rect(int left, int top, int right, int bottom) {
    glRecti(left, top, right, bottom);
  }

  static void clipPlane(GLenum plane, const GLdouble* equation) {
    glClipPlane(plane, equation);
  }

  static void selectBuffer(GLsizei size, GLuint* buffer) {
    glSelectBuffer(size, buffer);
  }

  static void initNames() {
    glInitNames();
  }

  static void flush() {
    glFlush();
  }

  static const GLubyte * getString(GLenum name) {
    return glGetString(name);
  }
  // changes a flag and restores it on destroying
  class FlagChanger {
    GLenum flag;
    bool original_state, current_state;
    void restore() {
      if (original_state != current_state) {
        if (original_state) {
          olx_gl::enable(flag);
        }
        else {
          olx_gl::disable(flag);
        }
      }
    }
  public:
    FlagChanger(GLenum flag)
      : flag(flag)
    {
      original_state = current_state = isEnabled(flag);
    }
    void enable() {
      if (!current_state) {
        olx_gl::enable(flag);
        current_state = true;
      }
    }
    void disable() {
      if (current_state) {
        olx_gl::disable(flag);
        current_state = false;
      }
    }
    ~FlagChanger() { restore(); }
  };

  struct FlagManipulatorBase {
    FlagChanger fc;
    FlagManipulatorBase(GLenum f)
      : fc(f)
    {}
    void disable() { fc.disable(); }
    void enable() { fc.enable(); }
  };

  struct FlagEnabler : FlagManipulatorBase {
    FlagEnabler(GLenum f)
      : FlagManipulatorBase(f)
    {
      FlagManipulatorBase::fc.enable();
    }
  };

  struct FlagDisabler : FlagManipulatorBase {
    FlagDisabler(GLenum f)
      : FlagManipulatorBase(f)
    {
      FlagManipulatorBase::fc.disable();
    }
  };

  struct FlagManager {
    olx_pdict<GLenum, FlagChanger *>  state;
    ~FlagManager() {
      clear();
    }
    void clear() {
      for (size_t i = 0; i < state.Count(); i++) {
        delete state.GetValue(i);
      }
      state.Clear();
    }
    void enable(GLenum f) {
      size_t idx = state.IndexOf(f);
      if (idx == InvalidIndex) {
        state.Add(f, new FlagChanger(f))->enable();
      }
      else {
        state.GetValue(idx)->enable();
      }
    }
    void disable(GLenum f) {
      size_t idx = state.IndexOf(f);
      if (idx == InvalidIndex) {
        state.Add(f, new FlagChanger(f))->disable();
      }
      else {
        state.GetValue(idx)->disable();
      }
    }
  };

};

EndGlNamespace()
#endif
