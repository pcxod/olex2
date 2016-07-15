/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glfont.h"
#include "exception.h"
#include "emath.h"
#include "egc.h"
#include "exparse/exptree.h"
#include "glscene.h"
#include "glrender.h"
UseGlNamespace()

using namespace exparse::parser_util;

TGlFont::TGlFont(AGlScene& parent, size_t _Id, const olxstr& name, size_t _SmallId)
  : Parent(parent), Id(_Id), SmallId(_SmallId), Name(name)
{
  FontBase = ~0;
  CharSizes.SetCount(256);
  for (int i = 0; i < 256; i++) {
    CharSizes[i] = new TFontCharSize();
  }
  CharOffset = 2;
  Flags = 0;
  Textures = NULL;
  ClearData();
  PointSize = 0;
  MaxWidth = MaxHeight = 0;
  Leftmost = 1000;
  Topmost  = 1000;
  VectorScale = 700;
}
//..............................................................................
TGlFont::~TGlFont() {
  CharSizes.DeleteItems();
  if (olx_is_valid_index(FontBase)) {
    olx_gl::deleteLists(FontBase, 256);
  }
  if (Textures != NULL) {
    olx_gl::deleteTextures(256, Textures);
    delete[] Textures;
  }
}
//..............................................................................
void TGlFont::ClearData() {
  for (int i = 0; i < 256; i++) {
    TFontCharSize* cs = CharSizes[i];
    cs->Top = -1;     cs->Left = -1;
    cs->Bottom = -1;  cs->Right = -1;
  }
  MaxWidth = MaxHeight = 0;
  Leftmost = 1000; // an arbitrary value
  Topmost = 1000;
  Flags = 0;
  if (olx_is_valid_index(FontBase)) {
    olx_gl::deleteLists(FontBase, 256);
    FontBase = (GLuint)~0;
  }
  //if( Textures != NULL )  {
  //  olx_gl::deleteTextures(256, Textures);
  //  delete [] Textures;
  //  Textures = NULL;
  //}
}
//..............................................................................
size_t TGlFont::TextWidth(const olxstr& Text, short& state) const {
  size_t w = 0;
  const size_t tl = Text.Length();
  for (size_t i = 0; i < tl; i++) {
    if (Text.CharAt(i) == '\\' && !is_escaped(Text, i) && (i + 1) < tl) {
      if (Text.CharAt(i + 1) == '+' || Text.CharAt(i + 1) == '-') {
        state = 1;
        i++;
        continue;
      }
      else if (Text.CharAt(i + 1) == '0') {
        i++;
        state = 0;
        continue;
      }
    }
    const TGlFont& glf = (state != 0 && olx_is_valid_index(SmallId))
      ? Parent.GetSmallFont(SmallId) : *this;
    TFontCharSize* cs = glf.CharSize(Text[i]);
    if (cs == NULL)  cs = glf.CharSize('?');
    if (glf.IsFixedWidth())
      w += glf.MaxWidth;
    else {
      if (i < (tl - 1))
        w += (cs->Right + glf.CharOffset);
      else
        w += cs->Right;
    }
  }
  return AdjustVectorFontSize(w);
}
//..............................................................................
double TGlFont::GetCharHalfWidth(short state) const {
  double rv;
  if (state == 0 || !olx_is_valid_index(SmallId)) {
    rv = (double)MaxWidth / 2;
  }
  else {
    rv = (double)(Parent.GetSmallFont(SmallId).MaxWidth) / 2;
  }
  return AdjustVectorFontSize(rv);
}
//..............................................................................
size_t TGlFont::TextWidth(const olxstr &Text) const {
  short state = 0;
  return TextWidth(Text, state);
}
//..............................................................................
size_t TGlFont::MaxTextLength(size_t width) const {
  return AdjustVectorFontSize(width/MaxWidth);
}
//..............................................................................
size_t TGlFont::LengthForWidth(const olxstr& str, size_t width) const {
  short state = 0;
  return LengthForWidth(str, width, state);
}
//..............................................................................
size_t TGlFont::LengthForWidth(const olxstr& str,
  size_t width, short& state) const
{
  if (MaxTextLength(width) > str.Length()) {
    return str.Length();
  }
  size_t w = 0;
  for (size_t i = 0; i < str.Length(); i++) {
    if (str.CharAt(i) == '\\' && !is_escaped(str, i) && (i + 1) < str.Length()) {
      if (str.CharAt(i + 1) == '+' || str.CharAt(i + 1) == '-') {
        state = 1;
        i++;
        continue;
      }
      else if (str.CharAt(i + 1) == '0') {
        i++;
        state = 0;
        continue;
      }
    }
    const TGlFont& glf = (state != 0 && olx_is_valid_index(SmallId))
      ? Parent.GetSmallFont(SmallId) : *this;
    TFontCharSize* cs = glf.CharSize(str.CharAt(i));
    if (cs == NULL)  cs = glf.CharSize('?');
    if (str.CharAt(i) == '\t' && !is_escaped(str, i)) {
      const int count = 8 - i % 8 - 1;
      if (count != 0) {
        if (IsFixedWidth()) {
          w += count*cs->Right;
        }
        else {
          w += count*(cs->Right + glf.CharOffset);
        }
      }
    }
    else {
      if (glf.IsFixedWidth()) {
        w += glf.MaxWidth;
      }
      else {
        if ((i + 1) < str.Length()) {
          w += (cs->Right + glf.CharOffset);
        }
        else {
          w += cs->Right;
        }
      }
    }
    if (AdjustVectorFontSize(w) >= width) {
      return i;
    }
  }
  return str.Length();
}
//..............................................................................
uint16_t TGlFont::TextHeight(const olxstr &Text) const {
  if (Text.IsEmpty()) {
    if (olx_is_valid_index(SmallId)) {
      return (uint16_t)AdjustVectorFontSize(olx_round((double)MaxHeight*0.75 +
        Parent.GetSmallFont(SmallId).MaxHeight));
    }
    return AdjustVectorFontSize(MaxHeight);
  }
  uint16_t w = 0, y_shift = 0;
  const uint16_t st = MaxHeight / 4;
  bool small_font = false;
  for (size_t i = 0; i < Text.Length(); i++) {
    if (Text.CharAt(i) == '\\' && !is_escaped(Text, i) && (i + 1) < Text.Length()) {
      if (Text.CharAt(i + 1) == '+') {
        y_shift = 2 * st;
        small_font = true;
        i++;
        continue;
      }
      else if (Text.CharAt(i + 1) == '-') {
        y_shift = -st;
        small_font = true;
        i++;
        continue;
      }
      else if (Text.CharAt(i + 1) == '0') {
        y_shift = 0;
        small_font = false;
        i++;
        continue;
      }
    }
    const TGlFont& glf = (small_font && olx_is_valid_index(SmallId))
      ? Parent.GetSmallFont(SmallId) : *this;
    TFontCharSize* cs = glf.CharSize(Text.CharAt(i));
    if (cs == NULL)  cs = glf.CharSize('?');
    const short df = cs->Bottom - cs->Top + y_shift;
    if (df > w) {
      w = df;
    }
  }
  return AdjustVectorFontSize(w);
}
//..............................................................................
TTextRect TGlFont::GetTextRect(const olxstr& str) const {
  TTextRect tr;
  tr.top = str.IsEmpty() ? MaxHeight : 100;
  const double st = (double)MaxHeight / 4;
  if (IsVectorFont()) {
    double scale = 1, y_shift = 0;
    for (size_t i = 0; i < str.Length(); i++) {
      TFontCharSize* cs = CharSize(str.CharAt(i));
      if (cs == NULL) {
        cs = CharSize('?');
      }
      if (str.CharAt(i) == '\\' && !is_escaped(str, i) && (i + 1) < str.Length()) {
        if (str.CharAt(i + 1) == '+' || str.CharAt(i + 1) == '-') {
          scale = 0.75;
          if (str.CharAt(i + 1) == '+')
            y_shift = 2 * st;
          else
            y_shift = -st;
          i++;
          continue;
        }
        else if (str.CharAt(i + 1) == '0') {
          scale = 1;
          y_shift = 0;
          i++;
          continue;
        }
      }
      //const double dt = cs->Top+y_shift*scale;
      const double dt = (cs->Top + y_shift)*scale;
      if (dt < tr.top) {
        tr.top = dt;
      }
      const double dy = (cs->Bottom - cs->Top + y_shift)*scale;
      if (dy > tr.height) {
        tr.height = dy;
      }
      tr.width += (cs->Right + CharOffset)*scale;  // left is unused in drawing
    }
    const double scalex = (double)PointSize / (15 * VectorScale);
    tr.height -= tr.top;
    tr.left *= scalex;
    tr.top *= scalex;
    tr.width *= scalex;
    tr.height *= scalex;
  }
  else {
    double y_shift = 0;
    bool small_fnt = false;
    for (size_t i = 0; i < str.Length(); i++) {
      if (str.CharAt(i) == '\\' && !is_escaped(str, i) && (i + 1) < str.Length()) {
        if (str.CharAt(i + 1) == '+' || str.CharAt(i + 1) == '-') {
          if (str.CharAt(i + 1) == '+')
            y_shift = 2 * st;
          else
            y_shift = -st;
          small_fnt = true;
          i++;
          continue;
        }
        else if (str.CharAt(i + 1) == '0') {
          small_fnt = false;
          y_shift = 0;
          i++;
          continue;
        }
      }
      const TGlFont& glf = (small_fnt && olx_is_valid_index(SmallId))
        ? Parent.GetSmallFont(SmallId) : *this;
      TFontCharSize* cs = glf.CharSize(str.CharAt(i));
      if (cs == NULL) {
        cs = glf.CharSize('?');
      }
      const double dt = (glf.MaxHeight - cs->Bottom) + y_shift;
      if (dt < tr.top) {
        tr.top = dt;
      }
      const double dy = (glf.MaxHeight - cs->Top) + y_shift;
      if (dy > tr.height) {
        tr.height = dy;
      }
      if (glf.IsFixedWidth()) {
        tr.width += glf.MaxWidth;
      }
      else {
        tr.width += (cs->Right + glf.CharOffset);
      }
    }
    tr.height -= tr.top;
  }
  return tr;
}
//..............................................................................
bool TGlFont::CharFromRGBArray(size_t Char, unsigned char *RGBData,
  uint16_t width, uint16_t height)
{
  TFontCharSize *cs = CharSizes[Char];
  int _Leftmost = -1, _Rightmost = -1, _Bottommost=-1, _Topmost = -1;
  unsigned char background = RGBData[3*width*height-1];
  for( uint16_t i=0; i < width; i++ )  {
    for( uint16_t j=0; j < height; j++ )  {
      const size_t ind = (j*width+i)*3;
      if( ( (RGBData[ind] | RGBData[ind+1] | RGBData[ind+2]) != background) )  {
        _Leftmost = i;  break;
      }
    }
    if( _Leftmost >= 0 )  break;
  }
  for( uint16_t i=width-1; olx_is_valid_index(i); i-- )  {
    for( uint16_t j=0; j < height; j++ )  {
      const size_t ind = (j*width+i)*3;
      if( ( (RGBData[ind] | RGBData[ind+1] | RGBData[ind+2]) != background) )  {
        _Rightmost = i;  break;
      }
    }
    if( _Rightmost >= 0 )  break;
  }
  for( uint16_t i=0; i < height; i++ )  {
    for( uint16_t j=0; j < width; j++ )  {
      const size_t ind = (i*width+j)*3;
      if( ( (RGBData[ind] | RGBData[ind+1] | RGBData[ind+2]) != background) )  {
        _Topmost = i;  break;
      }
    }
    if( _Topmost >= 0 )  break;
  }
  for( uint16_t i=height-1; olx_is_valid_index(i); i-- )  {
    for( uint16_t j=0; j < width; j++ )  {
      const size_t ind = (i*width+j)*3;
      if( ( (RGBData[ind] | RGBData[ind+1] | RGBData[ind+2]) != background) )  {
        _Bottommost = i;  break;
      }
    }
    if( _Bottommost >= 0 )  break;
  }
  cs->Top = _Topmost;
  cs->Left = _Leftmost;
  cs->Right = _Rightmost;
  cs->Bottom = _Bottommost;
  cs->Background = background;
  if( _Topmost >=0 && _Leftmost >=0 && _Rightmost >=0 && _Bottommost >=0 )  {
    cs->Data  = RGBData;
    int ind = _Bottommost;
    if( _Bottommost > MaxHeight )  MaxHeight = ind;
    ind = _Rightmost - _Leftmost;
    if( ind > MaxWidth )  MaxWidth  = ind;
    if( _Leftmost < Leftmost )  Leftmost = _Leftmost;
    if( _Topmost < Topmost )  Topmost = _Topmost;
    return true;
  }
  else  {
    cs->Data  = NULL;
    return false;
  }
}
//..............................................................................
void TGlFont::CreateGlyphsFromRGBArray(bool FW,
  uint16_t Width, uint16_t Height)
{
  if (Width < MaxWidth ||
    Height < MaxHeight ||
    MaxWidth == 0 || MaxHeight == 0)
  {
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("font size w:") <<
      Width << "; h:" << Height);
  }
  CharOffset = 2+MaxWidth/10;
  if (!olx_is_valid_index(FontBase)) {
    FontBase = olx_gl::genLists(256);
  }
  olx_set_bit(FW, Flags, fntFixedWidth);
  uint16_t NHeight = MaxHeight;
  uint16_t BWidth = (MaxWidth/8+1);
  olx_gl::pixelStore(GL_UNPACK_ALIGNMENT, 1);  // byte alignment
  unsigned char *BmpData = new unsigned char [(NHeight+1)*BWidth];
  uint16_t rightmost=0, leftmost=Width;
  for (size_t i = 0; i < 256; i++) {
    TFontCharSize* cs = this->CharSizes[i];
    if (cs->Data == 0) continue;
    if (cs->Left < leftmost) {
      leftmost = cs->Left;
    }
    if (cs->Right > rightmost) {
      rightmost = cs->Right;
    }
  }
  uint16_t maxCharW = rightmost - leftmost + CharOffset;
  if( MaxWidth > maxCharW ) // makes a lot of difference on lInux with its crappy fonts...
    MaxWidth = maxCharW;
  for( int i=0; i < 256; i++ )  {
    TFontCharSize* cs = CharSize(i);
    memset(BmpData, 0, NHeight*BWidth); // initialise the bits array
    if( cs->Data != NULL )  {  // check if bitmap is not empty
      for( int16_t j=cs->Left; j <= cs->Right; j++ )  {
        for( int16_t k=cs->Top; k <= cs->Bottom; k++ )  {
          const size_t ind = (k*Width+j)*3;
          if( (cs->Data[ind] | cs->Data[ind+1] | cs->Data[ind+2]) != cs->Background ) // is black?
            BmpData[(NHeight-k)*BWidth + (j-Leftmost)/8] |= (0x01 << (7-(j-Leftmost)%8));
        }
      }
      olx_gl::newList(FontBase +i, GL_COMPILE);
      if( IsFixedWidth() )
        olx_gl::bitmap(BWidth*8, NHeight, 0.0, 0.0, (float)(MaxWidth), 0.0, BmpData);
      else
        olx_gl::bitmap(BWidth*8, NHeight, 0.0, 0.0, (float)(cs->Right + CharOffset), 0.0, BmpData);
      olx_gl::endList();
      cs->Data = NULL;
    }
    else  {  // an empty character as a space char
      olx_gl::newList(FontBase +i, GL_COMPILE);
      if( IsFixedWidth() )  {
        olx_gl::bitmap(BWidth, MaxHeight, 0.0, 0.0, (float)(MaxWidth), 0.0, BmpData);
        cs->Right = MaxWidth;
      }
      else  {
        olx_gl::bitmap(olx_min(BWidth*8, CharOffset*5), MaxHeight, 0.0, 0.0,
           (float)(olx_min(BWidth*8, CharOffset*5)+CharOffset), 0.0, BmpData);
        cs->Right = olx_min(BWidth*8, CharOffset*5);
      }
      olx_gl::endList();
      cs->Top = MaxHeight/2;
      cs->Left = 0;
      cs->Bottom = MaxHeight/2;//MaxHeight;
    }
  }
  delete [] BmpData;
  Flags |= fntCreated;
}
//..............................................................................
bool TGlFont::AnalyseBitArray(const TEBitArray& ba, size_t Char,
  uint16_t width, uint16_t height)
{
  TFontCharSize *cs = CharSizes[Char];
  int16_t _Leftmost = -1, _Rightmost = -1, _Bottommost=-1, _Topmost = -1;
  const size_t off = width*height*Char;
  unsigned char background = 0;
  for( uint16_t i=0; i < width; i++ )  {
    for( uint16_t j=0; j < height; j++ )  {
      if( ba[off+j*width+i] )  {  _Leftmost = i;  break;  }
    }
    if( _Leftmost >= 0 )  break;
  }
  for( uint16_t i=width-1; olx_is_valid_index(i); i-- )  {
    for( uint16_t j=0; j < height; j++ )  {
      if( ba[off+j*width+i] )  {  _Rightmost = i;  break;  }
    }
    if( _Rightmost >= 0 )  break;
  }
  for( uint16_t i=0; i < height; i++ )  {
    for( uint16_t j=0; j < width; j++ )  {
      if( ba[off+i*width+j] )  {  _Topmost = i;  break;  }
    }
    if( _Topmost >= 0 )  break;
  }
  for( uint16_t i=height-1; olx_is_valid_index(i); i-- )  {
    for( uint16_t j=0; j < width; j++ )  {
      if( ba[off+i*width+j] )  {  _Bottommost = i;  break;  }
    }
    if( _Bottommost >= 0 )  break;
  }
  cs->Top = _Topmost;
  cs->Left = _Leftmost;
  cs->Right = _Rightmost;
  cs->Bottom = _Bottommost;
  cs->Background = background;
  cs->Data  = NULL;
  if( _Topmost >=0 && _Leftmost >=0 && _Rightmost >=0 && _Bottommost >=0 )  {
    int16_t ind = _Bottommost;
    if( ind > MaxHeight )  MaxHeight = ind;
    ind = _Rightmost - _Leftmost;
    if( ind > MaxWidth )  MaxWidth  = ind;
    if( _Leftmost < Leftmost )  Leftmost = _Leftmost;
    if( _Topmost < Topmost )  Topmost = _Topmost;
    return true;
  }
  return false;
}
//..............................................................................
void TGlFont::CreateGlyphs(const TEBitArray& ba, bool fixedWidth,
  uint16_t w, uint16_t h)
{
  for( int i=0; i < 256; i++ )
    AnalyseBitArray(ba, i, w, h);
  if( !olx_is_valid_index(FontBase) )
    FontBase = olx_gl::genLists(256);
  olx_set_bit(fixedWidth, Flags, fntFixedWidth);
  uint16_t BWidth = (MaxWidth/8+1)*8;
  uint16_t BHeight = MaxHeight+1;
  olx_gl::pixelStore(GL_UNPACK_ALIGNMENT, 1);  // byte alignment
  unsigned char* bf = new unsigned char[BWidth*BHeight];
/* //quad a pixel extras section
  const double scale = 1./VectorScale;
  Flags |= fntVectorFont;
*/
  for( int i=0; i < 256; i++ )  {
    TFontCharSize* cs = CharSize(i);
    const size_t off = i*w*h;
    memset(bf, 0, BWidth*BHeight);
    /* the commented section creates the 'quad a pixel', scalable font, which
    which does not look very good... */
    //if( cs->Left > 0 || cs->Bottom > 0 )  {  // check if bitmap is not empty
    //  olx_gl::newList(FontBase +i, GL_COMPILE);
    //  olx_gl::begin(GL_QUADS);
    //  for( int j=cs->Left; j <= cs->Right; j++ )  {
    //    for( int k=cs->Top; k <= cs->Bottom; k++ )  {
    //      if( ba[off + k*w + j] )  {
    //        const int y = MaxHeight-k;
    //        olx_gl::vertex(j*scale, y*scale);
    //        olx_gl::vertex((j+1)*scale, y*scale);
    //        olx_gl::vertex((j+1)*scale, (y+1)*scale);
    //        olx_gl::vertex(j*scale, (y+1)*scale);
    //      }
    //    }
    //  }
    //  olx_gl::end();
    //  olx_gl::endList();
    //}
    //else  {  // an empty character as a space char
    //  cs->Top = 0;
    //  cs->Left = 0;
    //  cs->Right = olx_min(BWidth, CharOffset*3);
    //  cs->Bottom = MaxHeight;
    //}
    if( cs->Left > 0 || cs->Bottom > 0 )  {  // check if bitmap is not empty
      for( int j=cs->Left; j <= cs->Right; j++ )  {
        for( int k=cs->Top; k <= cs->Bottom; k++ )  {
          if( ba[off + k*w + j] )
            bf[((BHeight-k)*BWidth + (j-Leftmost))/8] |= (0x01 << (7-(j-Leftmost)%8));
        }
      }
      olx_gl::newList(FontBase +i, GL_COMPILE);
      if( fixedWidth )
        olx_gl::bitmap(BWidth, BHeight, 0.0, 0.0, (float)(MaxWidth), 0.0, bf);
      else {
        olx_gl::bitmap(BWidth, BHeight, 0.0, 0.0,
          (float)(cs->Right + CharOffset), 0.0, bf);
      }
      olx_gl::endList();
    }
    else  {  // an empty character as a space char
      olx_gl::newList(FontBase +i, GL_COMPILE);
      if( fixedWidth )  {
        olx_gl::bitmap(BWidth, BHeight, 0.0, 0.0, (float)(MaxWidth), 0.0, bf);
        cs->Right = MaxWidth;
      }
      else  {
        olx_gl::bitmap(olx_min(BWidth, CharOffset*3), BHeight, 0.0, 0.0,
           (float)(olx_min(BWidth, CharOffset*3)+CharOffset), 0.0, bf);
        cs->Right = olx_min(BWidth, CharOffset*3);
      }
      olx_gl::endList();
      cs->Top = MaxHeight/2;
      cs->Left = 0;
      cs->Bottom = MaxHeight/2; //MaxHeight;
    }
  }
  delete [] bf;
  Flags |= fntCreated;
}
//..............................................................................
void TGlFont::CreateTextures(uint16_t Width, uint16_t Height)  {
  if( Width < MaxWidth ||
      Height < MaxHeight ||
      MaxWidth == 0 || MaxHeight == 0 )
  {
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("font size w:") <<
     Width << "; h:" << Height);
  }
  if( Textures == NULL )  {
    Textures = new GLuint[256];
    olx_gl::genTextures(256, &Textures[0]);
  }
  // calculate the texture size
  uint16_t txt_w = 1, txt_h = 1;
  while( txt_w < MaxWidth )  txt_w *= 2;
  while( txt_h < MaxHeight )  txt_h *= 2;
  TextureWidth = txt_w;
  TextureHeight = txt_h;
  unsigned char *BmpData = new unsigned char [txt_w*txt_h*4];

  for( int i=0; i < 256; i++ )  {
    TFontCharSize* cs = CharSize(i);
    memset(BmpData, 0, txt_w*txt_h*4); // initialise the bits array
    if( cs->Data != NULL )  {
      for( int16_t j=cs->Left; j <= cs->Right; j++ )  {
        for( int16_t k=cs->Top; k <= cs->Bottom; k++ )  {
          const size_t ind = (k*Width+j)*3;
          const size_t ind1 = (k*txt_w+j-cs->Left)*4;
          if( cs->Data[ind] != cs->Background &&
            cs->Data[ind+1] != cs->Background &&
            cs->Data[ind+2] != cs->Background )
          {
            BmpData[ind1+3] = 255;
            BmpData[ind1]   = (unsigned char)olx_round(255*Material.AmbientF[0]);
            BmpData[ind1+1] = (unsigned char)olx_round(255*Material.AmbientF[1]);
            BmpData[ind1+2] = (unsigned char)olx_round(255*Material.AmbientF[2]);
          }
        }
      }
      olx_gl::bindTexture(GL_TEXTURE_2D, Textures[i]);
      olx_gl::texEnv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      olx_gl::texParam(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      olx_gl::texParam(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      olx_gl::texParam(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      olx_gl::texParam(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      olx_gl::pixelStore(GL_UNPACK_ALIGNMENT, 4);
//      gluBuild2DMipmaps( GL_TEXTURE_2D, 3, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, cs->Data );
      //gluBuild2DMipmaps( GL_TEXTURE_2D, 4, txt_dim, txt_dim, GL_RGBA, GL_UNSIGNED_BYTE, BmpData );
      olx_gl::texImage(GL_TEXTURE_2D, 0, 4, txt_w, txt_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, BmpData);
    }
    else  {  // an empty character as a space char
      //glDeleteTextures(1, &Textures[i]);
      //Textures[i] = ~0;
      cs->Top = MaxHeight/2;
      cs->Left = 0;
      cs->Right = CharOffset*5;
      cs->Bottom = MaxHeight/2;//MaxHeight;
    }
  }
  delete [] BmpData;
  Flags |= fntCreated;
}
//..............................................................................
// http://local.wasp.uwa.edu.au/~pbourke/dataformats/hershey/
// will need to move this somewhere else...
int gl_font_simplex[95][112] = {
    0,16, /* Ascii 32 */
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,10, /* Ascii 33 */
    5,21, 5, 7,-1,-1, 5, 2, 4, 1, 5, 0, 6, 1, 5, 2,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,16, /* Ascii 34 */
    4,21, 4,14,-1,-1,12,21,12,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   11,21, /* Ascii 35 */
   11,25, 4,-7,-1,-1,17,25,10,-7,-1,-1, 4,12,18,12,-1,-1, 3, 6,17, 6,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   26,20, /* Ascii 36 */
    8,25, 8,-4,-1,-1,12,25,12,-4,-1,-1,17,18,15,20,12,21, 8,21, 5,20, 3,
   18, 3,16, 4,14, 5,13, 7,12,13,10,15, 9,16, 8,17, 6,17, 3,15, 1,12, 0,
    8, 0, 5, 1, 3, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   31,24, /* Ascii 37 */
   21,21, 3, 0,-1,-1, 8,21,10,19,10,17, 9,15, 7,14, 5,14, 3,16, 3,18, 4,
   20, 6,21, 8,21,10,20,13,19,16,19,19,20,21,21,-1,-1,17, 7,15, 6,14, 4,
   14, 2,16, 0,18, 0,20, 1,21, 3,21, 5,19, 7,17, 7,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   34,26, /* Ascii 38 */
   23,12,23,13,22,14,21,14,20,13,19,11,17, 6,15, 3,13, 1,11, 0, 7, 0, 5,
    1, 4, 2, 3, 4, 3, 6, 4, 8, 5, 9,12,13,13,14,14,16,14,18,13,20,11,21,
    9,20, 8,18, 8,16, 9,13,11,10,16, 3,18, 1,20, 0,22, 0,23, 1,23, 2,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    7,10, /* Ascii 39 */
    5,19, 4,20, 5,21, 6,20, 6,18, 5,16, 4,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   10,14, /* Ascii 40 */
   11,25, 9,23, 7,20, 5,16, 4,11, 4, 7, 5, 2, 7,-2, 9,-5,11,-7,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   10,14, /* Ascii 41 */
    3,25, 5,23, 7,20, 9,16,10,11,10, 7, 9, 2, 7,-2, 5,-5, 3,-7,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,16, /* Ascii 42 */
    8,21, 8, 9,-1,-1, 3,18,13,12,-1,-1,13,18, 3,12,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,26, /* Ascii 43 */
   13,18,13, 0,-1,-1, 4, 9,22, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,10, /* Ascii 44 */
    6, 1, 5, 0, 4, 1, 5, 2, 6, 1, 6,-1, 5,-3, 4,-4,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    2,26, /* Ascii 45 */
    4, 9,22, 9,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,10, /* Ascii 46 */
    5, 2, 4, 1, 5, 0, 6, 1, 5, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    2,22, /* Ascii 47 */
   20,25, 2,-7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   17,20, /* Ascii 48 */
    9,21, 6,20, 4,17, 3,12, 3, 9, 4, 4, 6, 1, 9, 0,11, 0,14, 1,16, 4,17,
    9,17,12,16,17,14,20,11,21, 9,21,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    4,20, /* Ascii 49 */
    6,17, 8,18,11,21,11, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   14,20, /* Ascii 50 */
    4,16, 4,17, 5,19, 6,20, 8,21,12,21,14,20,15,19,16,17,16,15,15,13,13,
   10, 3, 0,17, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   15,20, /* Ascii 51 */
    5,21,16,21,10,13,13,13,15,12,16,11,17, 8,17, 6,16, 3,14, 1,11, 0, 8,
    0, 5, 1, 4, 2, 3, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    6,20, /* Ascii 52 */
   13,21, 3, 7,18, 7,-1,-1,13,21,13, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   17,20, /* Ascii 53 */
   15,21, 5,21, 4,12, 5,13, 8,14,11,14,14,13,16,11,17, 8,17, 6,16, 3,14,
    1,11, 0, 8, 0, 5, 1, 4, 2, 3, 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   23,20, /* Ascii 54 */
   16,18,15,20,12,21,10,21, 7,20, 5,17, 4,12, 4, 7, 5, 3, 7, 1,10, 0,11,
    0,14, 1,16, 3,17, 6,17, 7,16,10,14,12,11,13,10,13, 7,12, 5,10, 4, 7,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,20, /* Ascii 55 */
   17,21, 7, 0,-1,-1, 3,21,17,21,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   29,20, /* Ascii 56 */
    8,21, 5,20, 4,18, 4,16, 5,14, 7,13,11,12,14,11,16, 9,17, 7,17, 4,16,
    2,15, 1,12, 0, 8, 0, 5, 1, 4, 2, 3, 4, 3, 7, 4, 9, 6,11, 9,12,13,13,
   15,14,16,16,16,18,15,20,12,21, 8,21,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   23,20, /* Ascii 57 */
   16,14,15,11,13, 9,10, 8, 9, 8, 6, 9, 4,11, 3,14, 3,15, 4,18, 6,20, 9,
   21,10,21,13,20,15,18,16,14,16, 9,15, 4,13, 1,10, 0, 8, 0, 5, 1, 4, 3,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   11,10, /* Ascii 58 */
    5,14, 4,13, 5,12, 6,13, 5,14,-1,-1, 5, 2, 4, 1, 5, 0, 6, 1, 5, 2,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   14,10, /* Ascii 59 */
    5,14, 4,13, 5,12, 6,13, 5,14,-1,-1, 6, 1, 5, 0, 4, 1, 5, 2, 6, 1, 6,
   -1, 5,-3, 4,-4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    3,24, /* Ascii 60 */
   20,18, 4, 9,20, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,26, /* Ascii 61 */
    4,12,22,12,-1,-1, 4, 6,22, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    3,24, /* Ascii 62 */
    4,18,20, 9, 4, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   20,18, /* Ascii 63 */
    3,16, 3,17, 4,19, 5,20, 7,21,11,21,13,20,14,19,15,17,15,15,14,13,13,
   12, 9,10, 9, 7,-1,-1, 9, 2, 8, 1, 9, 0,10, 1, 9, 2,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   55,27, /* Ascii 64 */
   18,13,17,15,15,16,12,16,10,15, 9,14, 8,11, 8, 8, 9, 6,11, 5,14, 5,16,
    6,17, 8,-1,-1,12,16,10,14, 9,11, 9, 8,10, 6,11, 5,-1,-1,18,16,17, 8,
   17, 6,19, 5,21, 5,23, 7,24,10,24,12,23,15,22,17,20,19,18,20,15,21,12,
   21, 9,20, 7,19, 5,17, 4,15, 3,12, 3, 9, 4, 6, 5, 4, 7, 2, 9, 1,12, 0,
   15, 0,18, 1,20, 2,21, 3,-1,-1,19,16,18, 8,18, 6,19, 5,
    8,18, /* Ascii 65 */
    9,21, 1, 0,-1,-1, 9,21,17, 0,-1,-1, 4, 7,14, 7,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   23,21, /* Ascii 66 */
    4,21, 4, 0,-1,-1, 4,21,13,21,16,20,17,19,18,17,18,15,17,13,16,12,13,
   11,-1,-1, 4,11,13,11,16,10,17, 9,18, 7,18, 4,17, 2,16, 1,13, 0, 4, 0,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   18,21, /* Ascii 67 */
   18,16,17,18,15,20,13,21, 9,21, 7,20, 5,18, 4,16, 3,13, 3, 8, 4, 5, 5,
    3, 7, 1, 9, 0,13, 0,15, 1,17, 3,18, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   15,21, /* Ascii 68 */
    4,21, 4, 0,-1,-1, 4,21,11,21,14,20,16,18,17,16,18,13,18, 8,17, 5,16,
    3,14, 1,11, 0, 4, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   11,19, /* Ascii 69 */
    4,21, 4, 0,-1,-1, 4,21,17,21,-1,-1, 4,11,12,11,-1,-1, 4, 0,17, 0,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,18, /* Ascii 70 */
    4,21, 4, 0,-1,-1, 4,21,17,21,-1,-1, 4,11,12,11,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   22,21, /* Ascii 71 */
   18,16,17,18,15,20,13,21, 9,21, 7,20, 5,18, 4,16, 3,13, 3, 8, 4, 5, 5,
    3, 7, 1, 9, 0,13, 0,15, 1,17, 3,18, 5,18, 8,-1,-1,13, 8,18, 8,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,22, /* Ascii 72 */
    4,21, 4, 0,-1,-1,18,21,18, 0,-1,-1, 4,11,18,11,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    2, 8, /* Ascii 73 */
    4,21, 4, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   10,16, /* Ascii 74 */
   12,21,12, 5,11, 2,10, 1, 8, 0, 6, 0, 4, 1, 3, 2, 2, 5, 2, 7,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,21, /* Ascii 75 */
    4,21, 4, 0,-1,-1,18,21, 4, 7,-1,-1, 9,12,18, 0,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,17, /* Ascii 76 */
    4,21, 4, 0,-1,-1, 4, 0,16, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   11,24, /* Ascii 77 */
    4,21, 4, 0,-1,-1, 4,21,12, 0,-1,-1,20,21,12, 0,-1,-1,20,21,20, 0,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,22, /* Ascii 78 */
    4,21, 4, 0,-1,-1, 4,21,18, 0,-1,-1,18,21,18, 0,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   21,22, /* Ascii 79 */
    9,21, 7,20, 5,18, 4,16, 3,13, 3, 8, 4, 5, 5, 3, 7, 1, 9, 0,13, 0,15,
    1,17, 3,18, 5,19, 8,19,13,18,16,17,18,15,20,13,21, 9,21,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   13,21, /* Ascii 80 */
    4,21, 4, 0,-1,-1, 4,21,13,21,16,20,17,19,18,17,18,14,17,12,16,11,13,
   10, 4,10,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   24,22, /* Ascii 81 */
    9,21, 7,20, 5,18, 4,16, 3,13, 3, 8, 4, 5, 5, 3, 7, 1, 9, 0,13, 0,15,
    1,17, 3,18, 5,19, 8,19,13,18,16,17,18,15,20,13,21, 9,21,-1,-1,12, 4,
   18,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   16,21, /* Ascii 82 */
    4,21, 4, 0,-1,-1, 4,21,13,21,16,20,17,19,18,17,18,15,17,13,16,12,13,
   11, 4,11,-1,-1,11,11,18, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   20,20, /* Ascii 83 */
   17,18,15,20,12,21, 8,21, 5,20, 3,18, 3,16, 4,14, 5,13, 7,12,13,10,15,
    9,16, 8,17, 6,17, 3,15, 1,12, 0, 8, 0, 5, 1, 3, 3,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,16, /* Ascii 84 */
    8,21, 8, 0,-1,-1, 1,21,15,21,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   10,22, /* Ascii 85 */
    4,21, 4, 6, 5, 3, 7, 1,10, 0,12, 0,15, 1,17, 3,18, 6,18,21,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,18, /* Ascii 86 */
    1,21, 9, 0,-1,-1,17,21, 9, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   11,24, /* Ascii 87 */
    2,21, 7, 0,-1,-1,12,21, 7, 0,-1,-1,12,21,17, 0,-1,-1,22,21,17, 0,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,20, /* Ascii 88 */
    3,21,17, 0,-1,-1,17,21, 3, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    6,18, /* Ascii 89 */
    1,21, 9,11, 9, 0,-1,-1,17,21, 9,11,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,20, /* Ascii 90 */
   17,21, 3, 0,-1,-1, 3,21,17,21,-1,-1, 3, 0,17, 0,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   11,14, /* Ascii 91 */
    4,25, 4,-7,-1,-1, 5,25, 5,-7,-1,-1, 4,25,11,25,-1,-1, 4,-7,11,-7,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    2,14, /* Ascii 92 */
    0,21,14,-3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   11,14, /* Ascii 93 */
    9,25, 9,-7,-1,-1,10,25,10,-7,-1,-1, 3,25,10,25,-1,-1, 3,-7,10,-7,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   10,16, /* Ascii 94 */
    6,15, 8,18,10,15,-1,-1, 3,12, 8,17,13,12,-1,-1, 8,17, 8, 0,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    2,16, /* Ascii 95 */
    0,-2,16,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    7,10, /* Ascii 96 */
    6,21, 5,20, 4,18, 4,16, 5,15, 6,16, 5,17,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   17,19, /* Ascii 97 */
   15,14,15, 0,-1,-1,15,11,13,13,11,14, 8,14, 6,13, 4,11, 3, 8, 3, 6, 4,
    3, 6, 1, 8, 0,11, 0,13, 1,15, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   17,19, /* Ascii 98 */
    4,21, 4, 0,-1,-1, 4,11, 6,13, 8,14,11,14,13,13,15,11,16, 8,16, 6,15,
    3,13, 1,11, 0, 8, 0, 6, 1, 4, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   14,18, /* Ascii 99 */
   15,11,13,13,11,14, 8,14, 6,13, 4,11, 3, 8, 3, 6, 4, 3, 6, 1, 8, 0,11,
    0,13, 1,15, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   17,19, /* Ascii 100 */
   15,21,15, 0,-1,-1,15,11,13,13,11,14, 8,14, 6,13, 4,11, 3, 8, 3, 6, 4,
    3, 6, 1, 8, 0,11, 0,13, 1,15, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   17,18, /* Ascii 101 */
    3, 8,15, 8,15,10,14,12,13,13,11,14, 8,14, 6,13, 4,11, 3, 8, 3, 6, 4,
    3, 6, 1, 8, 0,11, 0,13, 1,15, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,12, /* Ascii 102 */
   10,21, 8,21, 6,20, 5,17, 5, 0,-1,-1, 2,14, 9,14,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   22,19, /* Ascii 103 */
   15,14,15,-2,14,-5,13,-6,11,-7, 8,-7, 6,-6,-1,-1,15,11,13,13,11,14, 8,
   14, 6,13, 4,11, 3, 8, 3, 6, 4, 3, 6, 1, 8, 0,11, 0,13, 1,15, 3,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   10,19, /* Ascii 104 */
    4,21, 4, 0,-1,-1, 4,10, 7,13, 9,14,12,14,14,13,15,10,15, 0,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8, 8, /* Ascii 105 */
    3,21, 4,20, 5,21, 4,22, 3,21,-1,-1, 4,14, 4, 0,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   11,10, /* Ascii 106 */
    5,21, 6,20, 7,21, 6,22, 5,21,-1,-1, 6,14, 6,-3, 5,-6, 3,-7, 1,-7,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,17, /* Ascii 107 */
    4,21, 4, 0,-1,-1,14,14, 4, 4,-1,-1, 8, 8,15, 0,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    2, 8, /* Ascii 108 */
    4,21, 4, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   18,30, /* Ascii 109 */
    4,14, 4, 0,-1,-1, 4,10, 7,13, 9,14,12,14,14,13,15,10,15, 0,-1,-1,15,
   10,18,13,20,14,23,14,25,13,26,10,26, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   10,19, /* Ascii 110 */
    4,14, 4, 0,-1,-1, 4,10, 7,13, 9,14,12,14,14,13,15,10,15, 0,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   17,19, /* Ascii 111 */
    8,14, 6,13, 4,11, 3, 8, 3, 6, 4, 3, 6, 1, 8, 0,11, 0,13, 1,15, 3,16,
    6,16, 8,15,11,13,13,11,14, 8,14,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   17,19, /* Ascii 112 */
    4,14, 4,-7,-1,-1, 4,11, 6,13, 8,14,11,14,13,13,15,11,16, 8,16, 6,15,
    3,13, 1,11, 0, 8, 0, 6, 1, 4, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   17,19, /* Ascii 113 */
   15,14,15,-7,-1,-1,15,11,13,13,11,14, 8,14, 6,13, 4,11, 3, 8, 3, 6, 4,
    3, 6, 1, 8, 0,11, 0,13, 1,15, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,13, /* Ascii 114 */
    4,14, 4, 0,-1,-1, 4, 8, 5,11, 7,13, 9,14,12,14,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   17,17, /* Ascii 115 */
   14,11,13,13,10,14, 7,14, 4,13, 3,11, 4, 9, 6, 8,11, 7,13, 6,14, 4,14,
    3,13, 1,10, 0, 7, 0, 4, 1, 3, 3,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,12, /* Ascii 116 */
    5,21, 5, 4, 6, 1, 8, 0,10, 0,-1,-1, 2,14, 9,14,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   10,19, /* Ascii 117 */
    4,14, 4, 4, 5, 1, 7, 0,10, 0,12, 1,15, 4,-1,-1,15,14,15, 0,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,16, /* Ascii 118 */
    2,14, 8, 0,-1,-1,14,14, 8, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   11,22, /* Ascii 119 */
    3,14, 7, 0,-1,-1,11,14, 7, 0,-1,-1,11,14,15, 0,-1,-1,19,14,15, 0,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    5,17, /* Ascii 120 */
    3,14,14, 0,-1,-1,14,14, 3, 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    9,16, /* Ascii 121 */
    2,14, 8, 0,-1,-1,14,14, 8, 0, 6,-4, 4,-6, 2,-7, 1,-7,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    8,17, /* Ascii 122 */
   14,14, 3, 0,-1,-1, 3,14,14,14,-1,-1, 3, 0,14, 0,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   39,14, /* Ascii 123 */
    9,25, 7,24, 6,23, 5,21, 5,19, 6,17, 7,16, 8,14, 8,12, 6,10,-1,-1, 7,
   24, 6,22, 6,20, 7,18, 8,17, 9,15, 9,13, 8,11, 4, 9, 8, 7, 9, 5, 9, 3,
    8, 1, 7, 0, 6,-2, 6,-4, 7,-6,-1,-1, 6, 8, 8, 6, 8, 4, 7, 2, 6, 1, 5,
   -1, 5,-3, 6,-5, 7,-6, 9,-7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    2, 8, /* Ascii 124 */
    4,25, 4,-7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   39,14, /* Ascii 125 */
    5,25, 7,24, 8,23, 9,21, 9,19, 8,17, 7,16, 6,14, 6,12, 8,10,-1,-1, 7,
   24, 8,22, 8,20, 7,18, 6,17, 5,15, 5,13, 6,11,10, 9, 6, 7, 5, 5, 5, 3,
    6, 1, 7, 0, 8,-2, 8,-4, 7,-6,-1,-1, 8, 8, 6, 6, 6, 4, 7, 2, 8, 1, 9,
   -1, 9,-3, 8,-5, 7,-6, 5,-7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   23,24, /* Ascii 126 */
    3, 6, 3, 8, 4,11, 6,12, 8,12,10,11,14, 8,16, 7,18, 7,20, 8,21,10,-1,
   -1, 3, 8, 4,10, 6,11, 8,11,10,10,14, 7,16, 6,18, 6,20, 7,21,10,21,12,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

const olxcstr& TGlFont::DefinePSChar(olxch ch, const double& drawScale,
                               TGlFont::PSRenderContext& context) const
{
  if( ch < 32 || ch > 126 )  return CEmptyString();
  const uint32_t ch_id = PSRenderContext::make_id(ch, PointSize);
  const size_t _ind = context.def_dict.IndexOf(ch_id);
  if( _ind != InvalidIndex )
    return context.definitions[context.def_dict.GetValue(_ind)].id;
  size_t ch_ind = ch;
  context.def_dict.Add(ch_id, context.definitions.Count());
  TCStrList& def = context.definitions.AddNew(olxcstr("char_") << ch_id).definition;
  def.Add("  /char_") << ch_id << "  {";
  ch_ind -= 32;
  bool path_started = false;
  double scalex = (double)PointSize/15;
  for( size_t j=2; j < 112; j+=2 )  {
    if( gl_font_simplex[ch_ind][j] == -1 && gl_font_simplex[ch_ind][j] == -1 )  {
      path_started = false;
      continue;
    }
    else  {
      def.Add("    ") <<
        (double)gl_font_simplex[ch_ind][j]*drawScale*scalex/VectorScale <<
        ' ' << (double)gl_font_simplex[ch_ind][j+1]*drawScale*scalex/VectorScale;
      if( !path_started )  {
        def.GetLastString()  << " moveto";
        path_started = true;
      }
      else
        def.GetLastString()  << " lineto";
    }
  }
  def.Add("    stroke");
  def.Add("  } bind def");
  return context.definitions.GetLast().id;
}
TStrList TGlFont::ExportHersheyToPS(const olxstr& uniq_chars)  {
  TStrList rv;
  if( uniq_chars.IsEmpty() )  return rv;
  rv.Add("8 dict begin");
  rv.Add("/FontType 3 def");
  rv.Add("/FontMatrix [ .001  0  0  .001  0  0] def");
  rv.Add("/FontBBox [-30  -210  780  750] def");
  rv.Add("/Encoding 256 array def");
  rv.Add("0  1  255  {Encoding exch /.notdef put } for");
  for( size_t i=0; i < uniq_chars.Length(); i++ )
    rv.Add("  Encoding ") << (size_t)uniq_chars.CharAt(i) << " /char_" <<
    (size_t)uniq_chars.CharAt(i) << " put";
  rv.Add("/CharProcs 3 dict def");
  rv.Add("  CharProcs begin");
  rv.Add("  /.notdef { } def");
  for( size_t i=0; i < uniq_chars.Length(); i++ )  {
    size_t ch_ind = (size_t)uniq_chars.CharAt(i);
    if( ch_ind < 32 )  continue;
    rv.Add("  /char_") << ch_ind << "  {";
    ch_ind -= 32;
    bool path_started = false;
    for( size_t j=2; j < 112; j+=2 )  {
      if( gl_font_simplex[ch_ind][j] == -1 && gl_font_simplex[ch_ind][j] == -1 )  {
        path_started = false;
        continue;
      }
      else  {
        rv.Add("    ") << gl_font_simplex[ch_ind][j]*30 << ' ' <<
          gl_font_simplex[ch_ind][j+1]*30;
        if( !path_started )  {
          rv.GetLastString()  << " moveto";
          path_started = true;
        }
        else
          rv.GetLastString()  << " lineto";
      }
    }
    rv.Add("  stroke");
    rv.Add("  } bind def");
  }
  rv.Add("end");
  rv.Add("/BuildGlyph");
  rv.Add("{ 1000  0  -30  -210  780  750");
  rv.Add("  setcachedevice");
  rv.Add("  exch /CharProcs get exch");
  rv.Add("2 copy known not");
  rv.Add("{pop /.notdef } if");
  rv.Add("get exec");
  rv.Add("} bind def");
  rv.Add("/BuildChar");
  rv.Add("{ 1 index /Encoding get exch get");
  rv.Add("  1 index /BuildGlyph get exec");
  rv.Add("} bind def");
  rv.Add("currentdict");
  rv.Add("end");
  rv.Add("/HersheyFont exch definefont pop");
  rv.Add("");
  return rv;
}
void TGlFont::CreateHershey(const olx_pdict<size_t, olxstr>& definition,
  double scale)
{
  ClearData();
  Flags |= fntVectorFont;
  if( !olx_is_valid_index(FontBase) )
    FontBase = olx_gl::genLists(256);
  CharOffset = 0;
  VectorScale = scale*10;
  PointSize = 15;
  int top=100, left = 100, right=0, bottom=0;
  for( size_t i=0; i < 95; i++ )  {
    TFontCharSize* cs = CharSize(i+32);
    for( int j=2; j < 112; j+=2 )  {
      if( gl_font_simplex[i][j] == -1 && gl_font_simplex[i][j+1] == -1 )
        continue;
      if( gl_font_simplex[i][j] < cs->Left )
        cs->Left = gl_font_simplex[i][j];
      if( gl_font_simplex[i][j] > cs->Right )
        cs->Right = gl_font_simplex[i][j];
      if( gl_font_simplex[i][j+1] < cs->Top )
        cs->Top = gl_font_simplex[i][j+1];
      if( gl_font_simplex[i][j+1] > cs->Bottom )
        cs->Bottom = gl_font_simplex[i][j+1];
    }
    if( cs->Left < Leftmost )  Leftmost = cs->Left;
    if( cs->Left < left )  left = cs->Left;
    if( cs->Top < top )  top = cs->Top;
    if( cs->Right > right )  right = cs->Right;
    if( cs->Bottom > bottom )  bottom = cs->Bottom;
    const short w = cs->Right - cs->Left;
    if( w > MaxWidth )  MaxWidth = w;
    if( cs->Top > Topmost )  Topmost = cs->Top;
    const short h = cs->Bottom - cs->Top;
    if( h > MaxHeight )  MaxHeight = h;
    //cs->Right = gl_font_simplex[i][1];
  }
  for( size_t i=0; i < 95; i++ )  {
    TFontCharSize* cs = CharSize(i+32);
    olx_gl::newList((GLuint)(FontBase + i + 32), GL_COMPILE);
    bool loop_started = false;
    for( int j=2; j < 112; j+=2 )  {
      if( gl_font_simplex[i][j] == -1 && gl_font_simplex[i][j] == -1 )  {
        if( loop_started )  {
          olx_gl::end();
          loop_started = false;
        }
        continue;
      }
      else  {
        if( !loop_started )  {
          olx_gl::begin(GL_LINE_STRIP);
          loop_started = true;
        }
        olx_gl::vertex((double)gl_font_simplex[i][j]/VectorScale,
          (double)gl_font_simplex[i][j+1]/VectorScale);
      }
    }
    if( i == 0 )  {
      cs->Left = 0;
      cs->Right = MaxWidth/2;
      cs->Bottom = MaxHeight;
      cs->Top = 0;
    }
    if( loop_started )
      olx_gl::end();
    olx_gl::endList();
  }
  Flags |= fntCreated;
}
//..............................................................................
TCStrList TGlFont::RenderPSLabel(const vec3d& pos, const olxstr& label,
  double drawScale, PSRenderContext& context) const
{
  TCStrList out;
  out.Add("gsave");
  out.Add() << pos[0] << ' ' << pos[1] << " translate";
  short cstate = 0;
  const double st = 0.25*MaxHeight*drawScale*PointSize/(15*VectorScale);
  for( size_t i=0; i < label.Length(); i++ )  {
    if( label.CharAt(i) == '\\' && ! is_escaped(label, i) && (i+1) < label.Length() )  {
      if( label.CharAt(i+1) == '+' )  {
        if( cstate == 0 )  {
          out.Add("0 ") <<  2*st << " translate";
          out.Add("0.75 0.75 scale");
        }
        else if( cstate == -1 )
          out.Add("0 ") <<  3*st << " translate";
        cstate = 1;
        i++;
        continue;
      }
      else if( label.CharAt(i+1) == '-' )  {
        if( cstate == 0 )  {
          out.Add("0 -") <<  st << " translate";
          out.Add("0.75 0.75 scale");
        }
        else if( cstate == 1 )
          out.Add("0 -") <<  3*st << " translate";
        cstate = -1;
        i++;
        continue;
      }
      else if( label.CharAt(i+1) == '0' )  {
        if( cstate != 0 )  {
          out.Add("4/3 4/3 scale");
          if( cstate == 1 )
            out.Add("0 -") <<  2*st << " translate";
          else
            out.Add("0 +") <<  st << " translate";
          i++;
          cstate = 0;
          continue;
        }
      }
    }
    out.Add(DefinePSChar(label.CharAt(i), drawScale, context));
    if( i+1 < label.Length() )  {
      TFontCharSize* cs = CharSizes[label.CharAt(i)];
      out.Add() << (double)(cs->Right)*drawScale*PointSize/(15*VectorScale) << " 0 translate";
    }
  }
  out.Add("grestore");
  return out;
}
//..............................................................................
void TGlFont::_DrawText(const vec3d& from, const olxstr& text, double scale) const {
  if( IsVectorFont() )  {
    olx_gl::pushMatrix();
    olx_gl::translate(from);
    const double _scale = PointSize*scale/15;
    olx_gl::scale(_scale, _scale, 1.0);
    short cstate=0;
    for( size_t i = 0; i < text.Length(); i++ )
      DrawVectorChar(i, text, cstate);
    olx_gl::popMatrix();
    return;
  }
  else  {
    short cstate=0;
    for( size_t i = 0; i < text.Length(); i++ )
      DrawRasterChar(i, text, cstate);
    return;
  }
//
//  if( Textures == NULL || text.IsEmpty() )  return;
//  olx_gl::enable(GL_TEXTURE_2D);
//  olx_gl::enable(GL_ALPHA_TEST);
//  olx_gl::enable(GL_BLEND);
//  olx_gl::disable(GL_CULL_FACE);
//  olx_gl::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//
//  double step = 0.2, tx = (double)MaxWidth*step/TextureWidth, st=0,
//    aspect=step*(double)TextureHeight/TextureWidth;
//  //glEnable(GL_COLOR_MATERIAL);
//  //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
////  glColor4d(1, 1, 1, 0.5);
//  olx_gl::texCoord(0, 0);
//  olx_gl::normal(0, 0, 1);
//  for( size_t i=0; i < text.Length(); i++ )  {
//    const unsigned ch = (unsigned)text.CharAt(i);
//    if( ch > 256 )  continue;
//    TFontCharSize* cs = CharSizes[ch];
//    olx_gl::bindTexture(GL_TEXTURE_2D, Textures[ch] );
//    if( Textures[ch] == (GLuint)~0 )  continue;  // empty char
//    olx_gl::begin(GL_QUADS);
//    olx_gl::texCoord(1, 0);  //0,1
//    olx_gl::vertex(from);
//    olx_gl::texCoord(1, 1);  // 0,0
//    olx_gl::vertex(from[0], from[1]-aspect, from[2]);
//    olx_gl::texCoord(0, 1); // 1,0
//    olx_gl::vertex(from[0]-step, from[1]-aspect, from[2]);
//    olx_gl::texCoord(0, 0); // 1, 1
//    olx_gl::vertex(from[0]-step, from[1], from[2]);
//    olx_gl::end();
//    if( !FixedW )
//      tx = step*(cs->Right-cs->Left+CharOffset)/TextureWidth;
//    olx_gl::translate(tx, 0.0, 0.0);
//    st -= tx;
//  }
//  olx_gl::translate(st, 0.0, 0.0);
//  olx_gl::disable(GL_TEXTURE_2D);
//  olx_gl::disable(GL_ALPHA_TEST);
//  olx_gl::disable(GL_BLEND);
//  olx_gl::disable(GL_COLOR_MATERIAL);
}
//..............................................................................
void TGlFont::DrawVectorText(const vec3d& pos, const olxstr& text,
  double scale, short& state) const
{
  olx_gl::pushMatrix();
  olx_gl::translate(pos);
  const double _scale = PointSize*scale/15;
  olx_gl::scale(_scale, _scale, 1.0);
  for( size_t i = 0; i < text.Length(); i++ )
    DrawVectorChar(i, text, state);
  olx_gl::popMatrix();
}
//..............................................................................
void TGlFont::DrawRasterText(const olxstr& text, short& state) const {
  for( size_t i = 0; i < text.Length(); i++ )
    DrawRasterChar(i, text, state);
}
//..............................................................................
void TGlFont::DrawRasterChar(size_t &i, const olxstr& str, short& cstate) const {
#ifdef _DEBUG
  if( !IsCreated() )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid font");
#endif
  const float st = (float)MaxHeight/4;
  if( str.CharAt(i) == '\\' && !is_escaped(str, i) && (i+1) < str.Length() )  {
    if( str.CharAt(i+1) == '+' )  {
      if( cstate == 0 )
        olx_gl::bitmap(0, 0, 0, 0, 0, 2*st, NULL);
      else if( cstate == -1 )
        olx_gl::bitmap(0, 0, 0, 0, 0, 3*st, NULL);
      cstate = 1;
      i += 2;
    }
    else if( str.CharAt(i+1) == '-' )  {
      if( cstate == 0 )
        olx_gl::bitmap(0, 0, 0, 0, 0, -st, NULL);
      else if( cstate == 1 )
        olx_gl::bitmap(0, 0, 0, 0, 0, -3*st, NULL);
      cstate = -1;
      i += 2;
    }
    else if( str.CharAt(i+1) == '0' )  {
      if( cstate != 0 )  {
        if( cstate == 1 )
          olx_gl::bitmap(0, 0, 0, 0, 0, -2*st, NULL);
        else if( cstate == -1 )
          olx_gl::bitmap(0, 0, 0, 0, 0, st, NULL);
        i += 2;
        cstate = 0;
      }
    }
  }
  if( i >= str.Length() )  return;
  if( str.CharAt(i) == '\t' && !is_escaped(str, i) )  {
    const TGlFont& glf = (cstate != 0 && olx_is_valid_index(SmallId))
      ? Parent.GetSmallFont(SmallId) : *this;
    TFontCharSize* cs = glf.CharSize(' ');
    const int count = 8-i%8-1;
    if( count != 0 )  {
      if( IsFixedWidth() )
        olx_gl::bitmap(0, 0, 0, 0, (float)(count*cs->Right), 0, NULL);
      else
        olx_gl::bitmap(0, 0, 0, 0, (float)(count*(cs->Right+glf.CharOffset)), 0, NULL);
    }
  }
  else  {
    const GLuint ind = i > 255 ? (GLuint)'?' : (GLuint)str.CharAt(i);
    if( cstate != 0 && olx_is_valid_index(SmallId) )
      olx_gl::callList(Parent.GetSmallFont(SmallId).FontBase+ind);
    else
      olx_gl::callList(FontBase+ind);
  }
}
//..............................................................................
void TGlFont::DrawVectorChar(size_t &i, const olxstr& str, short& cstate) const {
#ifdef _DEBUG
  if( !IsCreated() )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid font");
#endif
  const double st = 0.25*MaxHeight/VectorScale;
  if( str.CharAt(i) == '\\' && !is_escaped(str, i) && (i+1) < str.Length() )  {
    if( str.CharAt(i+1) == '+' )  {
      if( cstate == 0 )  {
        olx_gl::scale(0.75f, 0.75f, 1.0f);
        olx_gl::translate(0.0, 2*st, 0.0);
      }
      else if( cstate == -1 )
        olx_gl::translate(0.0, 3*st, 0.0);
      cstate = 1;
      i += 2;
    }
    else if( str.CharAt(i+1) == '-' )  {
      if( cstate == 0 )  {
        olx_gl::scale(0.75f, 0.75f, 1.0f);
        olx_gl::translate(0.0, -st, 0.0);
      }
      else if( cstate == 1 )
        olx_gl::translate(0.0, -3*st, 0.0);
      cstate = -1;
      i += 2;
    }
    else if( str.CharAt(i+1) == '0' )  {
      if( cstate != 0 )  {
        if( cstate == 1 )
          olx_gl::translate(0.0, -2*st, 0.0);
        else if( cstate == -1 )
          olx_gl::translate(0.0, st, 0.0);
        olx_gl::scale(4./3, 4./3, 1.0);
        i += 2;
        cstate = 0;
      }
    }
  }
  if( i >= str.Length() )  return;
  if( str.CharAt(i) == '\t' && !is_escaped(str, i) )  {
    TFontCharSize* cs = CharSize(' ');
    const int count = 8-i%8-1;
    olx_gl::translate((double)(cs->Right+CharOffset)*count/VectorScale, 0.0, 0.0);
  }
  else  {
    const GLuint ind = i > 255 ? (GLuint)'?' : (GLuint)str.CharAt(i);
    TFontCharSize* cs = CharSize(ind);
    if( cs->Top != cs->Bottom )
      olx_gl::callList(FontBase+ind);
    olx_gl::translate((double)(cs->Right+CharOffset)/VectorScale, 0.0, 0.0);
  }
}
//..............................................................................
void TGlFont::SetMaterial(const TGlMaterial& m)  {
  if( Material == m )  return;
  bool ModifyTextures = !(Material.AmbientF == m.AmbientF);
  Material = m;
  if( Textures == NULL || !ModifyTextures )  return;
  unsigned char *BmpData = new unsigned char [TextureWidth*TextureHeight*4];
  for( int i=0; i < 256; i++ )  {
    olx_gl::bindTexture(GL_TEXTURE_2D, Textures[i] );
    olx_gl::getTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, BmpData);
    for( int j=0; j < TextureWidth*TextureHeight*4; j+=4 )  {
      if( BmpData[j+3] == 255 )  {
        BmpData[j]   = (unsigned char)olx_round(255*m.AmbientF.Data()[0]);
        BmpData[j+1] = (unsigned char)olx_round(255*m.AmbientF.Data()[1]);
        BmpData[j+2] = (unsigned char)olx_round(255*m.AmbientF.Data()[2]);
      }
    }
    olx_gl::texImage(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0,
      GL_RGBA, GL_UNSIGNED_BYTE, BmpData);
  }
}
//..............................................................................
