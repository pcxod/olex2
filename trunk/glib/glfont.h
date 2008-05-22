//---------------------------------------------------------------------------

#ifndef glfontH
#define glfontH
#include "glbase.h"
#include "glmaterial.h"
#include "elist.h"

BeginGlNamespace()
// font attributes
const short  sglfFixedWidth   = 0x0001;

struct TFontCharSize  {
  short Top, Left, Bottom, Right;
  unsigned char *Data;
  TFontCharSize()  {
    Top = Left = Right = Bottom = -1;
    Data = NULL;
  }
  unsigned char Background;
};

class TGlFont: public IEObject  {
  int FFontBase;
  TPtrList<TFontCharSize> CharSizes;
  short FFlags;
  short FMaxWidth, FMaxHeight,
        FLeftmost, FTopmost,
        FCharOffset;
  TGlMaterial FMaterial;
protected:
  olxstr FIdString, Name;
public:
  TGlFont(const olxstr& name);
  virtual ~TGlFont();

  void ClearData(); // must be called to reset all data

  int TextWidth(const olxstr &Text);
  int MaxTextLength(int width);
  int TextHeight(const olxstr &Text=EmptyString);
  bool CharFromRGBArray(size_t Char, unsigned char *RGBData, int width, int height);

  void CreateGlyphs(bool FixedWidth, short Width, short Height);

  inline TFontCharSize* CharSize(size_t Char)  { return CharSizes[(unsigned)Char];  }

  inline bool FixedWidth()  const {  return  (FFlags & sglfFixedWidth) == sglfFixedWidth; }
  inline short CharOffset() const {  return FCharOffset; }
  inline void CharOffset(short v) { FCharOffset = v; }
  inline int FontBase() const     {  return FFontBase; }

  void IdString(const olxstr &Str)             {  FIdString = Str; }
  inline const olxstr& IdString()        const {  return FIdString; }

  inline const olxstr& GetName()         const {  return Name; }

  inline TGlMaterial& Material()               {  return FMaterial;  }
};


EndGlNamespace()

#endif
