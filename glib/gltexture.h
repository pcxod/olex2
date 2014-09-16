/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_texture_H
#define __olx_gl_texture_H
#include "gloption.h"
#include "emath.h"
#include "estlist.h"
BeginGlNamespace()

const GLint
  tpFilterNearestMMNearest = GL_NEAREST_MIPMAP_NEAREST,
  tpFilterNearestMMLinear  = GL_NEAREST_MIPMAP_LINEAR,
  tpFilterLinearMMLinear   = GL_LINEAR_MIPMAP_LINEAR,
  tpFilterLinearMMNearest  = GL_LINEAR_MIPMAP_NEAREST,

  tpFilterNearest          = GL_NEAREST,
  tpFilterLinear           = GL_LINEAR,

  tpCrdRepeat              = GL_REPEAT,
  tpCrdClamp               = GL_CLAMP,
  // texture generation attributes
  tpgObjectLinear          = GL_OBJECT_LINEAR,
  tpgEyeLinear             = GL_EYE_LINEAR,
  tpgSphereMap             = GL_SPHERE_MAP,

  tpeModulate              = GL_MODULATE,
  tpeDecal                 = GL_DECAL,
  tpeBlend                 = GL_BLEND,
  tpeMode                  = GL_TEXTURE_ENV_MODE,
  tpeColor                 = GL_TEXTURE_ENV_COLOR;

const uint32_t
  tpminSet  = 0x00000001,
  tpmagSet  = 0x00000002,
  tpscrdSet = 0x00000004,
  tptcrdSet = 0x00000008,
  tpbrdSet  = 0x00000010,
  tpclrSet  = 0x00000020,
  tp1D      = 0x00000040,
  tp2D      = 0x00000080,
  tpEnvMode = 0x00000100,
  tpEnvColor= 0x00000200,
  tpEnabled = 0x00000400,
  tpt1D     = 0x00000800,
  tpt2D     = 0x00001000,
  tpSGen    = 0x00002000,
  tpTGen    = 0x00004000,
  tpRGen    = 0x00008000,
  tpQGen    = 0x00010000
  ;

class TGlTexture : public IEObject {
  GLint MinFilter, MagFilter, SCrd, TCrd, EnvMode, Level;
  unsigned short SetParams;
  TGlOption BorderColor, EnvColor,
    CrdObjPlaneParams[4],
    CrdEyePlaneParams[4];
  GLenum CrdGenMode[4]; // S,T,R,Q;
  GLuint Id;
  olxstr Name;
protected:
  void SetCrdGen(GLenum crdName, GLenum modeName,
    const TGlOption& op, const TGlOption& ep) const;
  void GetCrdGen(GLenum crdName, GLenum& genMode,
    TGlOption& op, TGlOption& ep) const;
public:
  TGlTexture() : SetParams(0) {}
  TGlTexture(GLuint id, uint32_t type, GLint level, const olxstr& name);
  virtual ~TGlTexture() {}
  const olxstr& GetName() const { return Name; }
  GLuint GetId() const { return Id; }
  // this is mostly for internal use
  void SetId(GLuint id) { Id = id; }
  GLint GetLevel() const { return Level; }
  GLuint GetType() const {
    if ((SetParams&tp1D) != 0)  return GL_TEXTURE_1D;
    if ((SetParams&tp2D) != 0)  return GL_TEXTURE_2D;
    throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
  }
  void Clear() { SetParams &= (tp1D|tp2D); }
//.............................................................................
  void SetEnabled(bool val) { olx_set_bit(val, SetParams, tpEnabled); }
  bool IsEnabled() const { return (SetParams & tpEnabled) != 0; }
//.............................................................................
  void SetMinFilter(GLint val) {
    olx_set_true(SetParams, tpminSet);
    MinFilter = val;
  }
//.............................................................................
  void SetMagFilter(GLint val) {
    olx_set_true(SetParams, tpmagSet);
    MagFilter = val;
  }
//.............................................................................
  void SetSCrdWrapping(GLint val) {
    olx_set_true(SetParams, tpscrdSet);
    SCrd = val;
  }
//.............................................................................
  void SetTCrdWrapping(GLint val) {
    olx_set_true(SetParams, tptcrdSet);
    TCrd = val;
  }
//.............................................................................
  void SetBorderColor(const TGlOption& cl) {
    olx_set_true(SetParams, tpbrdSet);
    BorderColor = cl;
  }
//.............................................................................
  void SetCrdGenMode(GLenum crd, GLenum modeName);
  void SetObjectPlane(GLenum crd, const TGlOption &val);
  void SetEyePlane(GLenum crd, const TGlOption &val);
//.............................................................................
//.............................................................................
  void SetEnvMode(GLenum modeName) {
    olx_set_true(SetParams, tpEnvMode);
    EnvMode = modeName;
  }
//.............................................................................
  void SetEnvColor(const TGlOption& clr) {
    olx_set_true(SetParams, tpEnvColor);
    EnvColor = clr;
  }
//.............................................................................
  TGlTexture& operator = (const TGlTexture& tex);
  static unsigned short ReadStatus();
  static void RestoreStatus(unsigned short status);
/* the function reads the parameters which are going to be changed when this
texture is applied
*/
  void ReadCurrent(TGlTexture& tex);
  void SetCurrent() const;
  struct Data {
    olx_array_ptr<unsigned char> data;
    GLint width, height, border;
    Data() : width(0), height(0), border(0) {}
  };
  static Data ReadData(const TGlTexture& tex);
  Data ReadData() const { return ReadData(*this); }
  // reverse to read - must be done on the same textures!
  static void WriteData(TGlTexture& tex, const Data &data);
  void WriteData(const Data &data) { WriteData(*this, data); }
};

/* the textures might be reused by different object (just having different
parameters applied to the same texture). So the AddTexture function returns an
integer which allows to access the texture object, but must not be used to bind
textures - just use texture::SetCurrent()
*/
class TTextureManager : public IEObject  {
  TPSTypeList<GLuint, TGlTexture*> Textures;
  TTypeList<TGlTexture::Data> TextureData;
public:
   TTextureManager()  {}
   // when cloning is implemented - this has to change!
   virtual ~TTextureManager();
  /* creates a 2D, texture takes arrays of GL_RGB or GL_RGBA only, see
  glTexImage2D for details. returns -1 is texture createion failed
  */
  GLuint Add2DTexture(const olxstr& name, GLint level, GLsizei width,
    GLsizei height, GLint border, GLenum format, const GLvoid *pixels);
  /* replaces data for a particular texture */
  void Replace2DTexture(TGlTexture& tex, GLint level, GLsizei width,
    GLsizei height, GLint border, GLenum format, const GLvoid *pixels);
  /* creates a 1D, texture takes arrays of GL_RGB or GL_RGBA only, see
  glTexImage1D for details. returns ~0 is texture creation failed
  */
  GLuint Add1DTexture(const olxstr& name, GLint level, GLsizei width,
    GLint border, GLenum format, const GLvoid *pixels);

  //int CloneTexture( int )
  //int GetTextureIndex( const TGlTexture& tex) const {  return Textures.}
  TGlTexture* FindTexture(GLuint textureIndex)  {
    const size_t index = Textures.IndexOf(textureIndex);
    return Textures.GetObject(index);
  }
  TGlTexture* FindByName(const olxstr &name)  {
    for (size_t i=0; i < Textures.Count(); i++) {
      if (Textures.GetObject(i)->GetName() == name)
        return Textures.GetObject(i);
    }
    return NULL;
  }
  void BeforeContextChange();
  void AfterContextChange();
};

EndGlNamespace()
#endif
