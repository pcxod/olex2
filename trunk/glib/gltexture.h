#ifndef __olx_gl_texture_H
#define __olx_gl_texture_H
#include "gloption.h"
#include "emath.h"
#include "estlist.h"

//---------------------------------------------------------------------------
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
  tpeColor                 = GL_TEXTURE_ENV_COLOR,

  tpt1D                    = GL_TEXTURE_1D,
  tpt2D                    = GL_TEXTURE_2D;


const uint16_t
  tpminSet  = 0x0001,
  tpmagSet  = 0x0002,
  tpscrdSet = 0x0004,
  tptcrdSet = 0x0008,
  tpbrdSet  = 0x0010,
  tpclrSet  = 0x0020,
  tp1D      = 0x0040,
  tp2D      = 0x0080,
  tpSGen    = 0x0100,
  tpTGen    = 0x0200,
  tpRGen    = 0x0400,
  tpQGen    = 0x0800,
  tpEnvMode = 0x1000,
  tpEnvColor= 0x2000,
  tpEnabled = 0x4000;

class TGlTexture : public IEObject {
  GLint MinFilter, MagFilter, SCrd, TCrd, EnvMode;
  unsigned short SetParams;
  TGlOption BorderColor, SGenParams, TGenParams, RGenParams, QGenParams, EnvColor;
  GLenum SGenMode, TGenMode, RGenMode, QGenMode;
  unsigned int Id;
  olxstr Name;
protected:
//..............................................................................
  inline void SetCrdGen(GLenum crdName, GLenum modeName, const TGlOption& opt)  {
    olx_gl::texGen(crdName, GL_TEXTURE_GEN_MODE, modeName);
    olx_gl::texGen(crdName, modeName, opt.Data());
    if( crdName == GL_S )  olx_gl::enable(GL_TEXTURE_GEN_S);
    if( crdName == GL_T )  olx_gl::enable(GL_TEXTURE_GEN_T);
    if( crdName == GL_R )  olx_gl::enable(GL_TEXTURE_GEN_R);
    if( crdName == GL_Q )  olx_gl::enable(GL_TEXTURE_GEN_Q);
  }
//..............................................................................
  inline void GetCrdGen(GLenum crdName, GLenum& modeName, TGlOption& opt)  {
    static GLfloat bf[4];
    olx_gl::getTexGen(crdName, GL_TEXTURE_GEN_MODE, (GLint*)&modeName);
    olx_gl::getTexGen(crdName, modeName, bf);
    opt = bf;
  }
public:
//..............................................................................
  TGlTexture()  {  SetParams = 0;  }
//..............................................................................
  TGlTexture(unsigned int id, unsigned short dimension, const olxstr& name)  {
    Id = id;
    SetParams = 0;
    Name = name;
    if( dimension == tpt1D )
      SetBitTrue(SetParams, tp1D);
    else if( dimension == tpt2D )
      SetBitTrue(SetParams, tp2D);
    else
      throw TInvalidArgumentException(__OlxSourceInfo, "dimention");
  }
//..............................................................................
  virtual ~TGlTexture()  {  }
//..............................................................................
  inline const olxstr& GetName()  const  {  return Name;  }
//..............................................................................
  inline unsigned int GetId()  const  {  return Id;  }
//..............................................................................
  inline void Clear()  {
    if( (SetParams & tp1D) != 0 )  {
      SetParams = 0;
      SetBitTrue(SetParams, tp1D);
    }
    else if( (SetParams & tp2D) != 0 )  {
      SetParams = 0;
      SetBitTrue(SetParams, tp2D);
    }

  }
//..............................................................................
  void SetEnabled(bool val)  {
    SetBit(val, SetParams, tpEnabled);
  }
//..............................................................................
  void SetMinFilter(GLint val)  {
    SetBitTrue(SetParams, tpminSet);
    MinFilter = val;
  }
//..............................................................................
  void SetMagFilter(GLint val)  {
    SetBitTrue(SetParams, tpmagSet);
    MagFilter = val;
  }
//..............................................................................
  void SetSCrdWrapping(GLint val)  {
    SetBitTrue(SetParams, tpscrdSet);
    SCrd = val;
  }
//..............................................................................
  void SetTCrdWrapping(GLint val)  {
    SetBitTrue(SetParams, tptcrdSet);
    TCrd = val;
  }
//..............................................................................
  void SetBorderColor(const TGlOption& cl)  {
    SetBitTrue(SetParams, tpbrdSet);
    BorderColor = cl;
  }
//..............................................................................
  inline void SetSCrdGen(GLenum modeName, const TGlOption& values)  {
    SetBitTrue(SetParams, tpSGen);
    SGenParams = values;
  }
//..............................................................................
  inline void SetTCrdGen(GLenum modeName, const TGlOption& values)  {
    SetBitTrue(SetParams, tpTGen);
    TGenParams = values;
  }
//..............................................................................
  inline void SetRCrdGen(GLenum modeName, const TGlOption& values)  {
    SetBitTrue(SetParams, tpRGen);
    RGenParams = values;
  }
//..............................................................................
  inline void SetQCrdGen(GLenum modeName, const TGlOption& values)  {
    SetBitTrue(SetParams, tpQGen);
    QGenParams = values;
  }
//..............................................................................
  inline void SetEnvMode(GLenum modeName)  {
    SetBitTrue(SetParams, tpEnvMode);
    EnvMode = modeName;
  }
//..............................................................................
  inline void SetEnvColor(const TGlOption& clr)  {
    SetBitTrue(SetParams, tpEnvColor);
    EnvColor = clr;
  }
//..............................................................................
  TGlTexture& operator = (const TGlTexture& tex)  {
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
//..............................................................................
  static void ReadStatus( unsigned short& status)  {
    status = 0;
    if( olx_gl::isEnabled(GL_TEXTURE_1D) )  {
      SetBitTrue(status, tp1D);
      SetBitTrue(status, tpEnabled);
    }
    else if( olx_gl::isEnabled(GL_TEXTURE_2D) )  {
      SetBitTrue(status, tp2D);
      SetBitTrue(status, tpEnabled);
    }
    if( olx_gl::isEnabled(GL_TEXTURE_GEN_S) )
      SetBitTrue(status, tpSGen);
    if( olx_gl::isEnabled(GL_TEXTURE_GEN_T) )
      SetBitTrue(status, tpTGen);
    if( olx_gl::isEnabled(GL_TEXTURE_GEN_Q) )
      SetBitTrue(status, tpQGen);
    if( olx_gl::isEnabled(GL_TEXTURE_GEN_R) )
      SetBitTrue(status, tpRGen);
  }
//..............................................................................
  static void RestoreStatus( const unsigned short status)  {
    GLint TextureType = GL_TEXTURE_1D;
    if( (status & tp2D) != 0 )  TextureType = GL_TEXTURE_2D;

    if( (status & tpEnabled) != 0 )
      olx_gl::enable(TextureType);
    else
      olx_gl::disable(TextureType);

    if( (status & tpSGen ) != 0 )
      olx_gl::enable(GL_TEXTURE_GEN_S);
    if( (status & tpTGen ) != 0 )
      olx_gl::enable(GL_TEXTURE_GEN_T);
    if( (status & tpRGen ) != 0 )
      olx_gl::enable(GL_TEXTURE_GEN_R);
    if( (status & tpQGen ) != 0 )
      olx_gl::enable(GL_TEXTURE_GEN_Q);
  }
//..............................................................................
/* the function reads the parameters which are going to be changed when this texture
   is applied */
  void ReadCurrent(TGlTexture& tex)  {
    GLint TextureType = GL_TEXTURE_1D;
    if( (SetParams & tp2D) != 0 )  TextureType = GL_TEXTURE_2D;

    tex.SetParams = SetParams;
    tex.SetEnabled(olx_gl::isEnabled(TextureType));

    if( (SetParams & tp2D) != 0 )  olx_gl::get(GL_TEXTURE_BINDING_2D, (GLint*)&(tex.Id));
    if( (SetParams & tp1D) != 0 )  olx_gl::get(GL_TEXTURE_BINDING_1D, (GLint*)&(tex.Id));

    if( (SetParams & tpmagSet) != 0 )
      olx_gl::getTexParam(TextureType, GL_TEXTURE_MAG_FILTER, &(tex.MagFilter));
    if( (SetParams & tpminSet) != 0 )
      olx_gl::getTexParam(TextureType, GL_TEXTURE_MIN_FILTER, &(tex.MinFilter));
    if( (SetParams & tptcrdSet) != 0 )
      olx_gl::getTexParam(TextureType, GL_TEXTURE_WRAP_T, &(tex.TCrd));
    if( (SetParams & tpscrdSet) != 0 )
      olx_gl::getTexParam(TextureType, GL_TEXTURE_WRAP_S, &(tex.SCrd));
    if( (SetParams & tpbrdSet) != 0 )  {
      GLfloat bf[4];
      olx_gl::getTexParam(TextureType, GL_TEXTURE_BORDER_COLOR, &bf[0]);
      tex.BorderColor = bf;
    }
    if( (SetParams & tpSGen) != 0 )
      GetCrdGen(GL_S, tex.SGenMode, tex.SGenParams);
    if( (SetParams & tpTGen) != 0 )
      GetCrdGen(GL_T, tex.TGenMode, tex.TGenParams);
    if( (SetParams & tpRGen) != 0 )
      GetCrdGen(GL_R, tex.RGenMode, tex.RGenParams);
    if( (SetParams & tpSGen) != 0 )
      GetCrdGen(GL_Q, tex.QGenMode, tex.QGenParams);
    if( (SetParams & tpEnvColor) != 0 )  {
      GLfloat bf[4];
      olx_gl::getTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, bf);
      tex.EnvColor = bf;
    }
    if( (SetParams & tpEnvMode) != 0 )
      olx_gl::getTexEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, &(tex.EnvMode));
  }
//..............................................................................
  void SetCurrent()  {
    GLint TextureType = GL_TEXTURE_1D;
    if( (SetParams & tp2D) != 0 )  TextureType = GL_TEXTURE_2D;

    if( (SetParams & tpEnabled) != 0 )  {
      olx_gl::enable(TextureType);
      olx_gl::bindTexture(TextureType, Id);
    }
    else
      olx_gl::disable(TextureType);

    if( (SetParams & tpmagSet) != 0 )
      olx_gl::texParam(TextureType, GL_TEXTURE_MAG_FILTER, MagFilter);
    if( (SetParams & tpminSet) != 0 )
      olx_gl::texParam(TextureType, GL_TEXTURE_MIN_FILTER, MinFilter);
    if( (SetParams & tptcrdSet) != 0 )
      olx_gl::texParam(TextureType, GL_TEXTURE_WRAP_T, TCrd);
    if( (SetParams & tpscrdSet) != 0 )
      olx_gl::texParam(TextureType, GL_TEXTURE_WRAP_S, SCrd);
    if( (SetParams & tpbrdSet) != 0 )
      olx_gl::texParam(TextureType, GL_TEXTURE_BORDER_COLOR, BorderColor.Data());

    if( (SetParams & tpSGen) != 0 )
      SetCrdGen(GL_S, SGenMode, SGenParams);
    if( (SetParams & tpTGen) != 0 )
      SetCrdGen(GL_T, TGenMode, TGenParams);
    if( (SetParams & tpRGen) != 0 )
      SetCrdGen(GL_R, RGenMode, RGenParams);
    if( (SetParams & tpSGen) != 0 )
      SetCrdGen(GL_Q, QGenMode, QGenParams);

    if( (SetParams & tpEnvMode) != 0 )
      olx_gl::texEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, EnvMode);
    if( (SetParams & tpEnvColor) != 0 ) 
      olx_gl::texEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, EnvColor.Data());
  }
};

/* the textures might be reused by different object (just having different parameters
applied to the same texture). So the AddTexture function returns an integer which allows
to access the texture object, but must not be used to bind textures - just use texture::SetCurrent()
*/
class TTextureManager : public IEObject  {
  TPSTypeList<int, TGlTexture*> Textures;

public:
   TTextureManager()  {  }
   // when cloning is implemented - this has to change!
   virtual ~TTextureManager()  {
     for( size_t i=0; i < Textures.Count(); i++ )  {
       TGlTexture* tex = Textures.GetObject(i);
       unsigned int texId = tex->GetId();
       olx_gl::deleteTextures(1, (GLuint*)&texId);
       delete tex;
     }
   }
  /* creates a 2D, texture takes arrays of GL_RGB or GL_RGBA only, see glTexImage2D
   for details. returns -1 is texture createion failed */
  int Add2DTexture(const olxstr& name, GLint level, GLsizei width, GLsizei height, GLint border,
                         GLenum format, const GLvoid *pixels)
  {
    unsigned int texId = 0;
    olx_gl::genTextures(1, (GLuint*)&texId);
    olx_gl::bindTexture(GL_TEXTURE_2D, texId);

    olx_gl::pixelStore(GL_UNPACK_ALIGNMENT, 4);
    if( format == GL_RGB )
      olx_gl::texImage(GL_TEXTURE_2D, level, 3, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    else if ( format == GL_RGBA )
      olx_gl::texImage(GL_TEXTURE_2D, level, 4, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    else  
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("format=") << (int)format);
    TGlTexture* tex = new TGlTexture(texId, tpt2D, name);
    Textures.Add(texId, tex);
    return texId;
  }
  /* replaces data for a particular texture */
  void Replace2DTexture(TGlTexture& tex, GLint level, GLsizei width, GLsizei height, GLint border,
                         GLenum format, const GLvoid *pixels)
  {
    unsigned int texId = tex.GetId();
    olx_gl::deleteTextures(1, (GLuint*)&texId);
    olx_gl::bindTexture( GL_TEXTURE_2D, texId);
    olx_gl::pixelStore(GL_UNPACK_ALIGNMENT, 4);
    if( format == GL_RGB )
      olx_gl::texImage(GL_TEXTURE_2D, level, 3, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    else if ( format == GL_RGBA )
      olx_gl::texImage(GL_TEXTURE_2D, level, 4, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
    else  
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("format=") << (int)format);
  }
  /* creates a 1D, texture takes arrays of GL_RGB or GL_RGBA only, see glTexImage1D
   for details. returns -1 is texture createion failed  */
  int Add1DTexture(const olxstr& name, GLint level, GLsizei width, GLint border,
                         GLenum format, const GLvoid *pixels)
  {
    olx_gl::pixelStore(GL_UNPACK_ALIGNMENT, 4);
    if( format == GL_RGB )
      olx_gl::texImage(GL_TEXTURE_2D, 0, format, width, 0, format, GL_UNSIGNED_BYTE, pixels);
    else if ( format == GL_RGBA )
      olx_gl::texImage(GL_TEXTURE_2D, 0, format, width, 0, format, GL_UNSIGNED_BYTE, pixels);
    else  
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("format=") << (int)format);
    unsigned int texId = 0;
    olx_gl::genTextures(1, (GLuint*)&texId);
    TGlTexture* tex = new TGlTexture(texId, tpt1D, name);
    Textures.Add(texId, tex);
    return texId;
  }
  
  //int CloneTexture( int )
  //inline int GetTextureIndex( const TGlTexture& tex) const {  return Textures.}
  inline TGlTexture* FindTexture(int textureIndex)  {
    size_t index = Textures.IndexOfComparable(textureIndex);
    return Textures.GetObject(index);
  }
};

EndGlNamespace()
#endif
