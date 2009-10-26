#ifndef __olx_gl_font_H
#define __olx_gl_font_H
#include "glbase.h"
#include "glmaterial.h"
#include "datastream.h"
#include "bitarray.h"
#include "threex3.h"

BeginGlNamespace()
// font attributes
const short  sglfFixedWidth   = 0x0001;

struct TFontCharSize  {
  int16_t Top, Left, Bottom, Right;
  unsigned char *Data;
  TFontCharSize()  {
    Top = Left = Right = Bottom = -1;
    Data = NULL;
  }
  unsigned char Background;
};

class TGlFont: public IEObject  {
public:
  static const short fntFixedWidth = 0x0001,
                     fntIntalic    = 0x0002,
                     fntBold       = 0x0004,
                     fntBmp        = 0x0008,  // create bitmap font
                     fntTexture    = 0x0010;  // create texture font
  GLuint FontBase;
  TPtrList<TFontCharSize> CharSizes;
  GLuint* Textures;
  uint16_t Flags, PointSize;
  uint16_t MaxWidth, MaxHeight,
        Leftmost, Topmost,
        CharOffset, TextureHeight, TextureWidth;
  TGlMaterial Material;
protected:
  olxstr IdString, Name;
  bool AnalyseBitArray(const TEBitArray& ba, size_t Char, uint16_t width, uint16_t height);
public:
  TGlFont(const olxstr& name);
  virtual ~TGlFont();

  void ClearData(); // must be called to reset all data

  inline uint16_t GetMaxWidth() const {  return MaxWidth;  }
  inline uint16_t GetMaxHeight() const {  return MaxHeight;  }
  inline uint16_t GetLeftmost() const {  return Leftmost;  }
  inline uint16_t GetTopmost() const {  return Topmost;  }
  
  DefPropP(uint16_t, PointSize)

  size_t TextWidth(const olxstr &Text, size_t cnt=InvalidSize);
  size_t MaxTextLength(size_t width);
  uint16_t TextHeight(const olxstr &Text=EmptyString);
  bool CharFromRGBArray(size_t Char, unsigned char *RGBData, uint16_t width, uint16_t height);

  void CreateGlyphsFromRGBArray(bool FixedWidth, uint16_t Width, uint16_t Height);
  // much faster version
  void CreateGlyphs(const TEBitArray& ba, bool FixedWidth, uint16_t Width, uint16_t Height);

  void CreateTextures(uint16_t Width, uint16_t Height);
  inline bool HasTextures() const {  return Textures != NULL;  }
  inline TFontCharSize* CharSize(size_t Char)  { return CharSizes[(unsigned)Char];  }

  inline bool IsFixedWidth() const {  return  (Flags & sglfFixedWidth) == sglfFixedWidth; }
  DefPropP(uint16_t, CharOffset)
  inline GLuint GetFontBase() const {  return FontBase; }
  void DrawGlText(const vec3d& from, const olxstr& text, bool FixedWidth);
  DefPropC(olxstr, IdString)
  inline const olxstr& GetName() const {  return Name; }
  inline TGlMaterial& GetMaterial()  {  return Material;  }
  inline const TGlMaterial& GetMaterial() const {  return Material;  }
  void SetMaterial(const TGlMaterial& m);
};


EndGlNamespace()

#endif
