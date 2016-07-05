/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_font_H
#define __olx_gl_font_H
#include "glbase.h"
#include "glmaterial.h"
#include "datastream.h"
#include "bitarray.h"
#include "threex3.h"
#include "edict.h"
BeginGlNamespace()

struct TFontCharSize  {
  int16_t Top, Left, Bottom, Right;
  unsigned char *Data;
  TFontCharSize()  {
    Top = Left = Right = Bottom = -1;
    Data = NULL;
  }
  unsigned char Background;
};

struct TTextRect  {
  double top, left, width, height;
  TTextRect() : top(0), left(0), width(0), height(0) {}
};

class TGlFont: public IOlxObject  {
public:
  static const short
    fntFixedWidth = 0x0001,
    fntIntalic    = 0x0002,
    fntBold       = 0x0004,
    fntBmp        = 0x0008,  // create bitmap font
    fntTexture    = 0x0010,  // create texture font
    fntCreated    = 0x0020,
    fntVectorFont = 0x0040;
  struct PSRenderContext  {
    struct PSChar  {
      olxcstr id;
      TCStrList definition;
      PSChar(const olxcstr& _id) : id(_id) {}
    };
    TTypeList<PSChar> definitions;
    olx_pdict<uint32_t, size_t> def_dict;
    // only very limited number of olxch range is supported in vector fonts
    static uint32_t make_id(olxch ch, uint16_t font_size)  {
      return ((uint32_t(ch) << 16) | font_size);
    }
  };
protected:
  class AGlScene& Parent;
  GLuint FontBase;
  TPtrList<TFontCharSize> CharSizes;
  GLuint* Textures;
  uint16_t Flags, PointSize;
  uint16_t MaxWidth, MaxHeight,
        CharOffset, TextureHeight, TextureWidth;
  int16_t Leftmost, Topmost;
  size_t Id, SmallId;
  TGlMaterial Material;
  double VectorScale;
  olxstr IdString, Name;
  bool AnalyseBitArray(const TEBitArray& ba, size_t Char,
    uint16_t width, uint16_t height);
  const olxcstr& DefinePSChar(olxch ch, const double& drawScale,
    PSRenderContext& context) const;
  void _DrawText(const vec3d& from, const olxstr& text, double scale) const;
public:
  TGlFont(AGlScene& parent, size_t _Id, const olxstr& name, size_t _SmallId=~0);
  virtual ~TGlFont();

  void ClearData(); // must be called to reset all data

  inline uint16_t GetMaxWidth() const {  return MaxWidth;  }
  inline uint16_t GetMaxHeight() const {  return MaxHeight;  }
  inline int16_t GetLeftmost() const {  return Leftmost;  }
  inline int16_t GetTopmost() const {  return Topmost;  }
  inline double GetVectorScale() const {  return VectorScale;  }
  size_t GetId() const {  return Id;  }
  DefPropP(uint16_t, PointSize)
  size_t TextWidth(const olxstr& Text) const;
  size_t TextWidth(const olxstr& Text, short& state) const;
  size_t MaxTextLength(size_t width) const;
  double GetCharHalfWidth(short state) const;
  size_t LengthForWidth(const olxstr& str, size_t width) const;
  size_t LengthForWidth(const olxstr& str, size_t width, short& state) const;
  uint16_t TextHeight(const olxstr& Text=EmptyString()) const;
  bool IsCreated() const {  return (Flags&fntCreated) != 0;  }
  TTextRect GetTextRect(const olxstr& str) const;
  bool CharFromRGBArray(size_t Char, unsigned char *RGBData,
    uint16_t width, uint16_t height);

  void CreateGlyphsFromRGBArray(bool FixedWidth,
    uint16_t Width, uint16_t Height);
  // much faster version
  void CreateGlyphs(const TEBitArray& ba, bool FixedWidth,
    uint16_t Width, uint16_t Height);
  void CreateHershey(const olx_pdict<size_t, olxstr>& definition,
    double scale);
  static TStrList ExportHersheyToPS(const olxstr& uniq_chars);
  TCStrList RenderPSLabel(const vec3d& pos, const olxstr& label,
    double drawScale, PSRenderContext& context) const;
  void CreateTextures(uint16_t Width, uint16_t Height);
  inline bool HasTextures() const {  return Textures != NULL;  }
  inline TFontCharSize* CharSize(size_t Char) const {
    return Char < 256 ? CharSizes[(unsigned)Char] : NULL;
  }

  inline bool IsFixedWidth() const {
    return (Flags & fntFixedWidth) == fntFixedWidth;
  }
  inline bool IsVectorFont() const {
    return (Flags & fntVectorFont) == fntVectorFont;
  }
  DefPropP(uint16_t, CharOffset)
  inline void Reset_ATI(bool v) const {
    if( v )  {
      olx_gl::rasterPos(0, 0, 0);
      olx_gl::callList(FontBase + ' ');
    }
  }
  void DrawVectorText(const vec3d& from, const olxstr& text,
    double scale=1.0) const
  {
    short state = 0;
    DrawVectorText(from, text, scale, state);
  }
  void DrawVectorText(const vec3d& from, const olxstr& text, double scale,
    short& state) const;
  void DrawRasterText(const olxstr& text) const {
    short state = 0;
    DrawRasterText(text, state);
  }
  void DrawRasterText(const olxstr& text, short& state) const;
  /*  renders a single char, if the \+, \- or \0 is used, the index is scrolled
  accordingly
  */
  void DrawRasterChar(size_t& i, const olxstr& str, short& state) const;
  void DrawVectorChar(size_t& i, const olxstr& str, short& state) const;
  DefPropC(olxstr, IdString)
  inline const olxstr& GetName() const {  return Name;  }
  inline TGlMaterial& GetMaterial()  {  return Material;  }
  inline const TGlMaterial& GetMaterial() const {  return Material;  }
  void SetMaterial(const TGlMaterial& m);
  friend class AGlScene;
};

EndGlNamespace()
#endif
