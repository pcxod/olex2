//----------------------------------------------------------------------------//
// namespace TEXLib
// TGlFont - a text font 
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "glfont.h"
#include "exception.h"
#include "elist.h"
#include "emath.h"
#include "egc.h"
#include <memory.h>

UseGlNamespace()
//..............................................................................
TGlFont::TGlFont(const olxstr& name)  {
  FFontBase = glGenLists(256);
  if( FFontBase == -1 )
    throw TOutOfMemoryException(__OlxSourceInfo);
  CharSizes.SetCount(256);
  for( int i=0; i < 256; i++ )
    CharSizes[i] = new TFontCharSize();
  FCharOffset = 2;
  FFlags = 0;
  ClearData();
  Name = name;
}
//..............................................................................
TGlFont::~TGlFont()  {
  for( int i=0; i < 256; i++ )
    delete CharSizes[i];
  if( FFontBase != -1 )
    glDeleteLists(FFontBase, 256);
}
//..............................................................................
void TGlFont::ClearData()  {
  for( int i=0; i < 256; i++ )  {
    TFontCharSize* cs = CharSizes[i];
    cs->Top = -1;     cs->Left = -1;
    cs->Bottom = -1;  cs->Right = -1;
  }
  FMaxWidth = FMaxHeight = 0;
  FLeftmost = 1000; // an arbitrary value
  FTopmost  = 1000;
}
//..............................................................................
int TGlFont::TextWidth(const olxstr &Text)  {
  if( FixedWidth() )  {
    return Text.Length()*(FMaxWidth);
  }
  int w = 0, tl = Text.Length();
  for( int i=0; i < tl; i++ )  {
    TFontCharSize* cs = CharSize(Text[i]);
    if( i < (tl-1) )  w += (cs->Right + FCharOffset);
    else              w += cs->Right;
  }
  return w;
}
//..............................................................................
int TGlFont::MaxTextLength(int width)  {
  return (int)((double)width/(double)(FMaxWidth));
}
//..............................................................................
int TGlFont::TextHeight(const olxstr &Text)  {
  if( Text.IsEmpty() )  return FMaxHeight;

  int w = 0, w1 = 0, tl = Text.Length();
  for( int i=0; i < tl; i++ )  {
    TFontCharSize* cs = CharSize(Text[i]);
    short df = cs->Bottom - FTopmost; // height from the top
    if( df > w )  w = df;
    df = cs->Bottom - cs->Top;  // height itself
    if( df > w1 )  w1 = df;
  }
  return 2*w-w1;
}
//..............................................................................
bool TGlFont::CharFromRGBArray(size_t Char, unsigned char *RGBData, int width, int height) {
  TFontCharSize *cs = CharSizes[Char];
  int ind;
  int Leftmost = -1, Rightmost = -1, Bottommost=-1, Topmost = -1;
  unsigned char background = RGBData[3*width*height-1];
  for( int i=0; i < width; i++ )  {
    for( int j=0; j < height; j++ )  {
      ind = (j*width+i)*3;
      if( ( (RGBData[ind] | RGBData[ind+1] | RGBData[ind+2]) != background) )  { 
        Leftmost = i;  break;
      }
    }
    if( Leftmost >= 0 )  break;
  }
  for( int i=width-1; i >=0; i-- )  {
    for( int j=0; j < height; j++ )  {
      ind = (j*width+i)*3;
      if( ( (RGBData[ind] | RGBData[ind+1] | RGBData[ind+2]) != background) )  {
        Rightmost = i;  break;
      }
    }
    if( Rightmost >= 0 )  break;
  }
  for( int i=0; i < height; i++ )  {
    for( int j=0; j < width; j++ )  {
      ind = (i*width+j)*3;
      if( ( (RGBData[ind] | RGBData[ind+1] | RGBData[ind+2]) != background) )  {
        Topmost = i;  break;
      }
    }
    if( Topmost >= 0 )  break;
  }
  for( int i=height-1; i >=0; i-- )  {
    for( int j=0; j < width; j++ )  {
      ind = (i*width+j)*3;
      if( ( (RGBData[ind] | RGBData[ind+1] | RGBData[ind+2]) != background) )  {
        Bottommost = i;  break;
      }
    }
    if( Bottommost >= 0 )  break;
  }
  cs->Top = Topmost;
  cs->Left = Leftmost;
  cs->Right = Rightmost;
  cs->Bottom = Bottommost;
  cs->Background = background;
  if( Topmost >=0 && Leftmost >=0 && Rightmost >=0 && Bottommost >=0 )  {
    cs->Data  = RGBData;
    ind = Bottommost;
    if( ind > FMaxHeight )  FMaxHeight = ind;
    ind = Rightmost - Leftmost;
    if( ind > FMaxWidth )   FMaxWidth  = ind;
    if( Leftmost < FLeftmost )  FLeftmost = Leftmost;
    if( Topmost < FTopmost )  FTopmost = Topmost;
    return true;
  }
  else  {
    cs->Data  = NULL;
    return false;
  }
}
//..............................................................................
void TGlFont::CreateGlyphs(bool FW, short Width, short Height)  {
  if( Width < FMaxWidth || Width < 0 ||
      Height < FMaxHeight || Height < 0 ||
      FMaxWidth <=0 || FMaxHeight <=0 )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("font size w:") << Width << "; h:" << Height);

  SetBit(FW, FFlags, sglfFixedWidth);

  int ind;
  int NHeight = FMaxHeight;//(FMaxHeight/8+1)*8;
  int BWidth = (FMaxWidth/8+1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // byte alignment
  unsigned char *BmpData = new unsigned char [(NHeight+1)*BWidth];
  TFontCharSize* cs = CharSize('W');
  int maxCharW = cs->Right - cs->Left + FCharOffset;
  if( FMaxWidth > maxCharW ) // makes a lot of difference on lInux with its crappy fonts...
    FMaxWidth = maxCharW;
  for( int i=0; i < 256; i++ )  {
    cs = CharSize(i);
    memset(BmpData, 0, NHeight*BWidth); // initialise the bits array
    if( cs->Data )  {  // check if bitmap is not empty
      for( int j=cs->Left; j <= cs->Right; j++ )  {
        for( int k=cs->Top; k <= cs->Bottom; k++ )  {
          ind = (k*Width+j)*3;
          if( (cs->Data[ind] | cs->Data[ind+1] | cs->Data[ind+2]) != cs->Background ) // is black?
            BmpData[(NHeight-k)*BWidth + (j+FLeftmost)/8] |= (0x01 << (7-(j+FLeftmost)%8));
        }
      }
      glNewList(FFontBase +i, GL_COMPILE_AND_EXECUTE);
      if( FixedWidth() )
        glBitmap(BWidth*8, NHeight, 0.0, 0.0, (float)(FMaxWidth), 0.0, BmpData);
      else
        glBitmap(BWidth*8, NHeight, 0.0, 0.0, (float)(cs->Right + FCharOffset), 0.0, BmpData);

      glEndList();
      cs->Data = NULL;
    }
    else  {  // an empty character as a space char
      glNewList(FFontBase +i, GL_COMPILE_AND_EXECUTE);

      if( FixedWidth() )
        glBitmap(BWidth, FMaxHeight, 0.0, 0.0, (float)(FMaxWidth), 0.0, BmpData);
      else
        glBitmap(olx_min(BWidth*8, FCharOffset*5), FMaxHeight, 0.0, 0.0, 
           (float)(olx_min(BWidth*8, FCharOffset*5)+FCharOffset), 0.0, BmpData);

      glEndList();
      cs->Top = 0;                 cs->Left = 0;
      cs->Right = FCharOffset*5;   cs->Top = FMaxHeight;
    }
  }
  delete [] BmpData;
}
//..............................................................................
