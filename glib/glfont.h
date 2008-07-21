//---------------------------------------------------------------------------
#ifndef glfontH
#define glfontH
#include "glbase.h"
#include "glmaterial.h"
#include "elist.h"
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
  int FFontBase;
  TPtrList<TFontCharSize> CharSizes;
  GLuint* Textures;
  uint16_t FFlags, PointSize;
  uint16_t FMaxWidth, FMaxHeight,
        FLeftmost, FTopmost,
        FCharOffset, TextureHeight, TextureWidth;
  TGlMaterial FMaterial;
protected:
  olxstr FIdString, Name;
  bool AnalyseBitArray(const TEBitArray& ba, size_t Char, int width, int height);
public:
  TGlFont(const olxstr& name);
  virtual ~TGlFont();

  void ClearData(); // must be called to reset all data
  inline uint16_t MaxWidth() const {  return FMaxWidth;  }
  inline uint16_t MaxHeight() const {  return FMaxHeight;  }
  
  DefPropP(uint16_t, PointSize)

  int TextWidth(const olxstr &Text, int cnt=-1);
  int MaxTextLength(int width);
  int TextHeight(const olxstr &Text=EmptyString);
  bool CharFromRGBArray(size_t Char, unsigned char *RGBData, int width, int height);

  void CreateGlyphsFromRGBArray(bool FixedWidth, short Width, short Height);
  // much faster version
  void CreateGlyphs(const TEBitArray& ba, bool FixedWidth, short Width, short Height);

  void CreateTextures(short Width, short Height);
  inline bool HasTextures() const {  return Textures != NULL;  }
  inline TFontCharSize* CharSize(size_t Char)  { return CharSizes[(unsigned)Char];  }

  inline bool FixedWidth()  const {  return  (FFlags & sglfFixedWidth) == sglfFixedWidth; }
  inline short CharOffset() const {  return FCharOffset; }
  inline void CharOffset(short v) { FCharOffset = v; }
  inline int FontBase() const     {  return FFontBase; }
  void DrawGlText(const vec3d& from, const olxstr& text, bool FixedWidth);
  // draws text safely, e.g. checks if raster positions are valid
  void DrawTextSafe(const vec3d& from, double scale, const olxstr& text);
  void IdString(const olxstr &Str)              {  FIdString = Str; }
  inline const olxstr& IdString()         const {  return FIdString; }
  inline const olxstr& GetName()          const {  return Name; }
  inline TGlMaterial& Material()                {  return FMaterial;  }
  inline const TGlMaterial& GetMaterial() const {  return FMaterial;  }
  void SetMaterial(const TGlMaterial& m);
};


EndGlNamespace()

#endif
