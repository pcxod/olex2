/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "gltexture.h"

//.............................................................................
TGlTexture::TGlTexture(GLuint id, uint32_t type, GLint level,
  const olxstr& name)
  : Id(id), Level(level), SetParams(0), Name(name)
{
  if( type == tpt1D )
    olx_set_true(SetParams, tp1D);
  else if( type == tpt2D )
    olx_set_true(SetParams, tp2D);
  else
    throw TInvalidArgumentException(__OlxSourceInfo, "type");
}
//.............................................................................
void TGlTexture::SetCrdGen(GLenum crdName, GLenum modeName,
  const TGlOption& opt) const
{
  olx_gl::texGen(crdName, GL_TEXTURE_GEN_MODE, modeName);
  olx_gl::texGen(crdName, modeName, opt.Data());
  if( crdName == GL_S )  olx_gl::enable(GL_TEXTURE_GEN_S);
  if( crdName == GL_T )  olx_gl::enable(GL_TEXTURE_GEN_T);
  if( crdName == GL_R )  olx_gl::enable(GL_TEXTURE_GEN_R);
  if( crdName == GL_Q )  olx_gl::enable(GL_TEXTURE_GEN_Q);
}
//.............................................................................
void TGlTexture::GetCrdGen(GLenum crdName, GLenum& modeName, TGlOption& opt)
  const
{
  static GLfloat bf[4];
  olx_gl::getTexGen(crdName, GL_TEXTURE_GEN_MODE, (GLint*)&modeName);
  olx_gl::getTexGen(crdName, modeName, bf);
  opt = bf;
}
//.............................................................................
TGlTexture& TGlTexture::operator = (const TGlTexture& tex) {
  SetParams   = tex.SetParams;
  MinFilter   = tex.MinFilter;
  MagFilter   = tex.MagFilter;
  SCrd        = tex.SCrd;
  TCrd        = tex.TCrd;
  BorderColor = tex.BorderColor;

  EnvMode      = tex.EnvMode;
  EnvColor    = tex.EnvColor;

  SGenParams  = tex.SGenParams;
  TGenParams  = tex.TGenParams;
  RGenParams  = tex.RGenParams;
  QGenParams  = tex.QGenParams;
  SGenMode    = tex.SGenMode;
  TGenMode    = tex.TGenMode;
  RGenMode    = tex.RGenMode;
  QGenMode    = tex.QGenMode;

  Id          = tex.Id;
  return *this;
}
//.............................................................................
unsigned short TGlTexture::ReadStatus() {
  unsigned short status = 0;
  if (olx_gl::isEnabled(GL_TEXTURE_1D) ) {
    olx_set_true(status, tp1D);
    olx_set_true(status, tpEnabled);
  }
  else if (olx_gl::isEnabled(GL_TEXTURE_2D))  {
    olx_set_true(status, tp2D);
    olx_set_true(status, tpEnabled);
  }
  if (olx_gl::isEnabled(GL_TEXTURE_GEN_S))
    olx_set_true(status, tpSGen);
  if (olx_gl::isEnabled(GL_TEXTURE_GEN_T))
    olx_set_true(status, tpTGen);
  if (olx_gl::isEnabled(GL_TEXTURE_GEN_Q))
    olx_set_true(status, tpQGen);
  if (olx_gl::isEnabled(GL_TEXTURE_GEN_R))
    olx_set_true(status, tpRGen);
  return status;
}
//.............................................................................
void TGlTexture::RestoreStatus(unsigned short status) {
  GLint TextureType = GL_TEXTURE_1D;
  if ((status & tp2D) != 0)
    TextureType = GL_TEXTURE_2D;

  if ((status & tpEnabled) != 0)
    olx_gl::enable(TextureType);
  else
    olx_gl::disable(TextureType);

  if ((status & tpSGen ) != 0)
    olx_gl::enable(GL_TEXTURE_GEN_S);
  if ((status & tpTGen ) != 0)
    olx_gl::enable(GL_TEXTURE_GEN_T);
  if ((status & tpRGen ) != 0)
    olx_gl::enable(GL_TEXTURE_GEN_R);
  if ((status & tpQGen ) != 0)
    olx_gl::enable(GL_TEXTURE_GEN_Q);
}
//.............................................................................
void TGlTexture::ReadCurrent(TGlTexture& tex) {
  GLint TextureType = GL_TEXTURE_1D;
  if ((SetParams & tp2D) != 0)
    TextureType = GL_TEXTURE_2D;

  tex.SetParams = SetParams;
  tex.SetEnabled(olx_gl::isEnabled(TextureType));

  if ((SetParams & tp2D) != 0)
    olx_gl::get(GL_TEXTURE_BINDING_2D, (GLint*)&(tex.Id));
  if ((SetParams & tp1D) != 0)
    olx_gl::get(GL_TEXTURE_BINDING_1D, (GLint*)&(tex.Id));

  if ((SetParams & tpmagSet) != 0)
    olx_gl::getTexParam(TextureType, GL_TEXTURE_MAG_FILTER, &(tex.MagFilter));
  if ((SetParams & tpminSet) != 0)
    olx_gl::getTexParam(TextureType, GL_TEXTURE_MIN_FILTER, &(tex.MinFilter));
  if ((SetParams & tptcrdSet) != 0)
    olx_gl::getTexParam(TextureType, GL_TEXTURE_WRAP_T, &(tex.TCrd));
  if ((SetParams & tpscrdSet) != 0)
    olx_gl::getTexParam(TextureType, GL_TEXTURE_WRAP_S, &(tex.SCrd));
  if ((SetParams & tpbrdSet) != 0) {
    GLfloat bf[4];
    olx_gl::getTexParam(TextureType, GL_TEXTURE_BORDER_COLOR, &bf[0]);
    tex.BorderColor = bf;
  }
  if ((SetParams & tpSGen) != 0)
    GetCrdGen(GL_S, tex.SGenMode, tex.SGenParams);
  if ((SetParams & tpTGen) != 0)
    GetCrdGen(GL_T, tex.TGenMode, tex.TGenParams);
  if ((SetParams & tpRGen) != 0)
    GetCrdGen(GL_R, tex.RGenMode, tex.RGenParams);
  if ((SetParams & tpSGen) != 0)
    GetCrdGen(GL_Q, tex.QGenMode, tex.QGenParams);
  if ((SetParams & tpEnvColor) != 0) {
    GLfloat bf[4];
    olx_gl::getTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, bf);
    tex.EnvColor = bf;
  }
  if ((SetParams & tpEnvMode) != 0)
    olx_gl::getTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &(tex.EnvMode));
}
//.............................................................................
void TGlTexture::SetCurrent() const {
  GLint TextureType = GL_TEXTURE_1D;
  if ((SetParams & tp2D) != 0)
    TextureType = GL_TEXTURE_2D;

  if ((SetParams & tpEnabled) != 0) {
    olx_gl::enable(TextureType);
    olx_gl::bindTexture(TextureType, Id);
  }
  else
    olx_gl::disable(TextureType);

  if ((SetParams & tpmagSet) != 0)
    olx_gl::texParam(TextureType, GL_TEXTURE_MAG_FILTER, MagFilter);
  if ((SetParams & tpminSet) != 0)
    olx_gl::texParam(TextureType, GL_TEXTURE_MIN_FILTER, MinFilter);
  if ((SetParams & tptcrdSet) != 0)
    olx_gl::texParam(TextureType, GL_TEXTURE_WRAP_T, TCrd);
  if ((SetParams & tpscrdSet) != 0)
    olx_gl::texParam(TextureType, GL_TEXTURE_WRAP_S, SCrd);
  if ((SetParams & tpbrdSet) != 0) {
    olx_gl::texParam(TextureType, GL_TEXTURE_BORDER_COLOR,
      BorderColor.Data());
  }

  if ((SetParams & tpSGen) != 0)
    SetCrdGen(GL_S, SGenMode, SGenParams);
  if ((SetParams & tpTGen) != 0)
    SetCrdGen(GL_T, TGenMode, TGenParams);
  if ((SetParams & tpRGen) != 0)
    SetCrdGen(GL_R, RGenMode, RGenParams);
  if ((SetParams & tpSGen) != 0)
    SetCrdGen(GL_Q, QGenMode, QGenParams);

  if ((SetParams & tpEnvMode) != 0)
    olx_gl::texEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, EnvMode);
  if ((SetParams & tpEnvColor) != 0)
    olx_gl::texEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, EnvColor.Data());
}
//.............................................................................
TGlTexture::Data TGlTexture::ReadData(const TGlTexture& tex) {
  Data rv;
  olx_gl::bindTexture(tex.GetType(), tex.GetId());
  olx_gl::getTexLevelParam(tex.GetType(), tex.GetLevel(),
    GL_TEXTURE_WIDTH, &rv.width);
  olx_gl::getTexLevelParam(tex.GetType(), tex.GetLevel(),
    GL_TEXTURE_HEIGHT, &rv.height);
  olx_gl::getTexLevelParam(tex.GetType(), tex.GetLevel(),
    GL_TEXTURE_BORDER, &rv.border);
  if (rv.width == 0 || rv.height == 0)
    return rv;
  short extraBytes = (4-(rv.width*3)%4)%4;  //4 bytes aligment
  rv.data = new unsigned char[(rv.width*4+extraBytes)*rv.height];
  if( rv.data == NULL )
    throw TOutOfMemoryException(__OlxSourceInfo);
  olx_gl::pixelStore(GL_PACK_ALIGNMENT, 4);
  olx_gl::getTexImage(tex.GetType(), tex.GetLevel(), GL_RGBA,
    GL_UNSIGNED_BYTE, rv.data);
  return rv;
}
//.............................................................................
void TGlTexture::WriteData(TGlTexture& tex, const TGlTexture::Data &data) {
  if (data.data.is_null()) return;
  olx_gl::bindTexture(tex.GetType(), tex.GetId());
  olx_gl::pixelStore(GL_UNPACK_ALIGNMENT, 4);
  if (data.height != 0) {
    olx_gl::texImage(tex.GetType(), tex.GetLevel(), 4,
      data.width, data.height, data.border,
      GL_RGBA, GL_UNSIGNED_BYTE, data.data());
  }
  else {
    olx_gl::texImage(tex.GetType(), tex.GetLevel(), 4,
      data.width, data.border, GL_RGBA, GL_UNSIGNED_BYTE, data.data());
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
TTextureManager::~TTextureManager()  {
  for( size_t i=0; i < Textures.Count(); i++ )  {
    TGlTexture* tex = Textures.GetObject(i);
    GLuint texId = tex->GetId();
    olx_gl::deleteTextures(1, (GLuint*)&texId);
    delete tex;
  }
  TextureData.Clear();
}
//.............................................................................
GLuint TTextureManager::Add2DTexture(const olxstr& name, GLint level,
  GLsizei width, GLsizei height, GLint border, GLenum format,
  const GLvoid *pixels)
{
  GLuint texId = 0;
  olx_gl::genTextures(1, &texId);
  olx_gl::bindTexture(GL_TEXTURE_2D, texId);

  olx_gl::pixelStore(GL_UNPACK_ALIGNMENT, 4);
  if (format == GL_RGB) {
    olx_gl::texImage(GL_TEXTURE_2D, level, 3, width, height, 0, format,
      GL_UNSIGNED_BYTE, pixels);
  }
  else if (format == GL_RGBA) {
    olx_gl::texImage(GL_TEXTURE_2D, level, 4, width, height, 0, format,
      GL_UNSIGNED_BYTE, pixels);
  }
  else {  
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("format=") << (int)format);
  }
  TGlTexture* tex = new TGlTexture(texId, tpt2D, level, name);
  Textures.Add(texId, tex);
  return texId;
}
//.............................................................................
void TTextureManager::Replace2DTexture(TGlTexture& tex, GLint level,
  GLsizei width, GLsizei height, GLint border, GLenum format,
  const GLvoid *pixels)
{
  GLuint texId = tex.GetId();
  olx_gl::deleteTextures(1, &texId);
  olx_gl::bindTexture(GL_TEXTURE_2D, texId);
  olx_gl::pixelStore(GL_UNPACK_ALIGNMENT, 4);
  if (format == GL_RGB) {
    olx_gl::texImage(GL_TEXTURE_2D, level, 3, width, height, 0, format,
      GL_UNSIGNED_BYTE, pixels);
  }
  else if (format == GL_RGBA) {
    olx_gl::texImage(GL_TEXTURE_2D, level, 4, width, height, 0, format,
      GL_UNSIGNED_BYTE, pixels);
  }
  else {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("format=") << (int)format);
  }
}
//.............................................................................
GLuint TTextureManager::Add1DTexture(const olxstr& name, GLint level,
  GLsizei width, GLint border, GLenum format, const GLvoid *pixels)
{
  olx_gl::pixelStore(GL_UNPACK_ALIGNMENT, 4);
  if (format == GL_RGB) {
    olx_gl::texImage(GL_TEXTURE_2D, level, format, width, 0, format,
      GL_UNSIGNED_BYTE, pixels);
  }
  else if (format == GL_RGBA) {
    olx_gl::texImage(GL_TEXTURE_2D, level, format, width, 0, format,
      GL_UNSIGNED_BYTE, pixels);
  }
  else {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("format=") << (int)format);
  }
  GLuint texId = 0;
  olx_gl::genTextures(1, &texId);
  TGlTexture* tex = new TGlTexture(texId, tpt1D, level, name);
  Textures.Add(texId, tex);
  return texId;
}
//.............................................................................
//.............................................................................
void TTextureManager::BeforeContextChange() {
  TextureData.Clear();
  for (size_t i=0; i < Textures.Count(); i++) {
    TGlTexture &glt = *Textures.GetObject(i);
    if (glt.GetId() != ~0) {
      TextureData.AddCopy(glt.ReadData());
      GLuint texId = glt.GetId();
      olx_gl::deleteTextures(1, &texId);
      glt.SetId(~0);
    }
    else
      TextureData.AddNew();
  }
}
//.............................................................................
void TTextureManager::AfterContextChange() {
  if (Textures.Count() != TextureData.Count())
    throw TFunctionFailedException(__OlxSourceInfo, "mismatching array sizes");
  GLuint texId = 0;
  olx_gl::genTextures((GLuint)Textures.Count(), &texId);
  for (size_t i=0; i < Textures.Count(); i++) {
    TGlTexture &glt = *Textures.GetObject(i);
    if (glt.GetId() == ~0) {
      glt.SetId((GLuint)(texId+i));
      glt.WriteData(TextureData[i]);
    }
  }
  TextureData.Clear();
}
//.............................................................................

