//----------------------------------------------------------------------------//
// TGlFont - a text font 
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "glfont.h"
#include "exception.h"
#include "emath.h"
#include "egc.h"
#include <memory.h>

UseGlNamespace()
//..............................................................................
TGlFont::TGlFont(const olxstr& name) {
  FontBase = glGenLists(256);
  if( FontBase == (GLuint)~0 )
    throw TOutOfMemoryException(__OlxSourceInfo);
  CharSizes.SetCount(256);
  for( int i=0; i < 256; i++ )
    CharSizes[i] = new TFontCharSize();
  CharOffset = 2;
  Flags = 0;
  Textures = NULL;
  ClearData();
  Name = name;
  PointSize = 0;
  MaxWidth = MaxHeight = 0;
  Leftmost = 1000; 
  Topmost  = 1000;
}
//..............................................................................
TGlFont::~TGlFont()  {
  for( int i=0; i < 256; i++ )
    delete CharSizes[i];
  if( FontBase != (GLuint)~0 )
    glDeleteLists(FontBase, 256);
  if( Textures != NULL )  {
    glDeleteTextures(256, Textures);
    delete [] Textures;
  }
}
//..............................................................................
void TGlFont::ClearData()  {
  for( int i=0; i < 256; i++ )  {
    TFontCharSize* cs = CharSizes[i];
    cs->Top = -1;     cs->Left = -1;
    cs->Bottom = -1;  cs->Right = -1;
  }
  MaxWidth = MaxHeight = 0;
  Leftmost = 1000; // an arbitrary value
  Topmost  = 1000;
  //if( Textures != NULL )  {
  //  glDeleteTextures(256, Textures);
  //  delete [] Textures;
  //  Textures = NULL;
  //}
}
//..............................................................................
size_t TGlFont::TextWidth(const olxstr &Text, size_t cnt)  {
  if( IsFixedWidth() )
    return (cnt == (size_t)~0) ? Text.Length()*MaxWidth : cnt*MaxWidth;
  size_t w = 0, tl = (cnt == InvalidSize) ? Text.Length() : olx_min(cnt, Text.Length());
  for( size_t i=0; i < tl; i++ )  {
    TFontCharSize* cs = CharSize(Text[i]);
    if( i < (tl-1) )  w += (cs->Right + CharOffset);
    else              w += cs->Right;
  }
  return w;
}
//..............................................................................
size_t TGlFont::MaxTextLength(size_t width)  {
  return width/MaxWidth;
}
//..............................................................................
uint16_t TGlFont::TextHeight(const olxstr &Text)  {
  if( Text.IsEmpty() )  return MaxHeight;
  uint16_t w = 0, w1 = 0;
  for( size_t i=0; i < Text.Length(); i++ )  {
    TFontCharSize* cs = CharSize(Text.CharAt(i));
    short df = cs->Bottom - Topmost; // height from the top
    if( df > w )  w = df;
    df = cs->Bottom - cs->Top;  // height itself
    if( df > w1 )  w1 = df;
  }
  return 2*w-w1;
}
//..............................................................................
bool TGlFont::CharFromRGBArray(size_t Char, unsigned char *RGBData, uint16_t width, uint16_t height) {
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
  for( uint16_t i=width-1; i != (uint16_t)~0; i-- )  {
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
  for( uint16_t i=height-1; i != (uint16_t)~0; i-- )  {
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
void TGlFont::CreateGlyphsFromRGBArray(bool FW, uint16_t Width, uint16_t Height)  {
  if( Width < MaxWidth || Width < 0 ||
      Height < MaxHeight || Height < 0 ||
      MaxWidth <=0 || MaxHeight <=0 )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("font size w:") << Width << "; h:" << Height);

  SetBit(FW, Flags, sglfFixedWidth);
  uint16_t NHeight = MaxHeight;
  uint16_t BWidth = (MaxWidth/8+1);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // byte alignment
  unsigned char *BmpData = new unsigned char [(NHeight+1)*BWidth];
  TFontCharSize* cs = CharSize('W');
  uint16_t maxCharW = cs->Right - cs->Left + CharOffset;
  if( MaxWidth > maxCharW ) // makes a lot of difference on lInux with its crappy fonts...
    MaxWidth = maxCharW;
  for( int i=0; i < 256; i++ )  {
    cs = CharSize(i);
    memset(BmpData, 0, NHeight*BWidth); // initialise the bits array
    if( cs->Data != NULL )  {  // check if bitmap is not empty
      for( int16_t j=cs->Left; j <= cs->Right; j++ )  {
        for( int16_t k=cs->Top; k <= cs->Bottom; k++ )  {
          const size_t ind = (k*Width+j)*3;
          if( (cs->Data[ind] | cs->Data[ind+1] | cs->Data[ind+2]) != cs->Background ) // is black?
            BmpData[(NHeight-k)*BWidth + (j+Leftmost)/8] |= (0x01 << (7-(j+Leftmost)%8));
        }
      }
      glNewList(FontBase +i, GL_COMPILE_AND_EXECUTE);
      if( IsFixedWidth() )
        glBitmap(BWidth*8, NHeight, 0.0, 0.0, (float)(MaxWidth), 0.0, BmpData);
      else
        glBitmap(BWidth*8, NHeight, 0.0, 0.0, (float)(cs->Right + CharOffset), 0.0, BmpData);

      glEndList();
      cs->Data = NULL;
    }
    else  {  // an empty character as a space char
      glNewList(FontBase +i, GL_COMPILE_AND_EXECUTE);

      if( IsFixedWidth() )
        glBitmap(BWidth, MaxHeight, 0.0, 0.0, (float)(MaxWidth), 0.0, BmpData);
      else
        glBitmap(olx_min(BWidth*8, CharOffset*5), MaxHeight, 0.0, 0.0, 
           (float)(olx_min(BWidth*8, CharOffset*5)+CharOffset), 0.0, BmpData);

      glEndList();
      cs->Top = 0;                 
      cs->Left = 0;
      cs->Right = olx_min(BWidth*8, CharOffset*5);   
      cs->Bottom = MaxHeight;
    }
  }
  delete [] BmpData;
}
//..............................................................................
bool TGlFont::AnalyseBitArray(const TEBitArray& ba, size_t Char, uint16_t width, uint16_t height)  {
  TFontCharSize *cs = CharSizes[Char];
  int _Leftmost = -1, _Rightmost = -1, _Bottommost=-1, _Topmost = -1;
  const size_t off = width*height*Char;
  unsigned char background = 0;
  for( uint16_t i=0; i < width; i++ )  {
    for( uint16_t j=0; j < height; j++ )  {
      if( ba[off+j*width+i] )  {  _Leftmost = i;  break;  }
    }
    if( _Leftmost >= 0 )  break;
  }
  for( uint16_t i=width-1; i != (uint16_t)~0; i-- )  {
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
  for( uint16_t i=height-1; i != (uint16_t)~0; i-- )  {
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
  if( +Topmost >=0 && _Leftmost >=0 && _Rightmost >=0 && _Bottommost >=0 )  {
    int ind = _Bottommost;
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
void TGlFont::CreateGlyphs(const TEBitArray& ba, bool fixedWidth, uint16_t w, uint16_t h)  {
  for( int i=0; i < 256; i++ )
    AnalyseBitArray(ba, i, w, h);
  SetBit(fixedWidth, Flags, sglfFixedWidth);

  uint16_t BWidth = (MaxWidth/8+1)*8;
  uint16_t BHeight = MaxHeight+1;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // byte alignment
  unsigned char* bf = new unsigned char[BWidth*BHeight];
  for( int i=0; i < 256; i++ )  {
    TFontCharSize* cs = CharSize(i);
    const size_t off = i*w*h;
    memset(bf, 0, BWidth*BHeight);
    if( cs->Left > 0 || cs->Bottom > 0 )  {  // check if bitmap is not empty
      for( int j=cs->Left; j <= cs->Right; j++ )  {
        for( int k=cs->Top; k <= cs->Bottom; k++ )  {
          if( ba[off + k*w + j] ) 
            bf[((BHeight-k)*BWidth + (j+Leftmost))/8] |= (0x01 << (7-(j+Leftmost)%8));
        }
      }
      glNewList(FontBase +i, GL_COMPILE_AND_EXECUTE);
      if( fixedWidth )
        glBitmap(BWidth, BHeight, 0.0, 0.0, (float)(MaxWidth), 0.0, bf);
      else
        glBitmap(BWidth, BHeight, 0.0, 0.0, (float)(cs->Right + CharOffset), 0.0, bf);
      glEndList();
    }
    else  {  // an empty character as a space char
      glNewList(FontBase +i, GL_COMPILE_AND_EXECUTE);
      if( fixedWidth )
        glBitmap(BWidth, BHeight, 0.0, 0.0, (float)(MaxWidth), 0.0, bf);
      else
        glBitmap(olx_min(BWidth, CharOffset*3), BHeight, 0.0, 0.0, 
           (float)(olx_min(BWidth, CharOffset*3)+CharOffset), 0.0, bf);
      glEndList();
      cs->Top = 0;                 
      cs->Left = 0;
      cs->Right = olx_min(BWidth, CharOffset*3);   
      cs->Bottom = MaxHeight;
    }
  }
  delete [] bf;
}
//..............................................................................
//void TGlFont::CreateGlyphs(bool FW, short Width, short Height)  {
//  if( Width < FMaxWidth || Width < 0 ||
//      Height < FMaxHeight || Height < 0 ||
//      FMaxWidth <=0 || FMaxHeight <=0 )
//    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("font size w:") << Width << "; h:" << Height);
//
//  SetBit(FW, FFlags, sglfFixedWidth);
//  int NHeight = FMaxHeight;//(FMaxHeight/8+1)*8;
//  int BWidth  = FMaxWidth;
//  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // byte alignment
//  unsigned char *BmpData = new unsigned char [NHeight*(BWidth+4)*4];
//  TFontCharSize* cs = CharSize('W');
//  int maxCharW = cs->Right - cs->Left + FCharOffset;
//  if( FMaxWidth > maxCharW ) // makes a lot of difference on lInux with its crappy fonts...
//    FMaxWidth = maxCharW;
//  for( int i=0; i < 256; i++ )  {
//    cs = CharSize(i);
//    memset(BmpData, 0, NHeight*BWidth*4); // initialise the bits array
//    if( cs->Data )  {  // check if bitmap is not empty
//      for( int j=cs->Left; j <= cs->Right; j++ )  {
//        for( int k=cs->Top; k <= cs->Bottom; k++ )  {
//          int ind = (k*Width+j)*3;
//          if( (cs->Data[ind] | cs->Data[ind+1] | cs->Data[ind+2]) != cs->Background )  { // is black?
//            int ind1 = ((NHeight-k)*BWidth + (j+FLeftmost))*4;
//            BmpData[ind1]   = cs->Data[ind];
//            BmpData[ind1+1] = cs->Data[ind+1];
//            BmpData[ind1+2] = cs->Data[ind+2];
//            BmpData[ind1+3] = 255;
//          }
//        }
//      }
//      glNewList(FFontBase +i, GL_COMPILE);
////      glGetDoublev(GL_CURRENT_RASTER_POSITION, raster_pos);
//      if( FixedWidth() )  {
//        glDrawPixels(BWidth, NHeight, GL_RGBA, GL_UNSIGNED_BYTE, BmpData);
////        raster_pos[1] += FMaxWidth;
//      }
//      else  {
//        glDrawPixels(BWidth, NHeight, GL_RGBA, GL_UNSIGNED_BYTE, BmpData);
////        raster_pos[1] += (cs->Right + FCharOffset);
//      }
////      glRasterPos4dv(raster_pos);
//      glEndList();
//      cs->Data = NULL;
//    }
//    else  {  // an empty character as a space char
//      glNewList(FFontBase +i, GL_COMPILE);
////      glGetDoublev(GL_CURRENT_RASTER_POSITION, raster_pos);
//      if( FixedWidth() )  {
//        //glDrawPixels(BWidth, NHeight, GL_RGB, GL_UNSIGNED_BYTE, BmpData);
////        raster_pos[1] += FMaxWidth;
//      }
//      else  {
////        raster_pos[1] += (olx_min(BWidth, FCharOffset*5)+FCharOffset);
//        //glDrawPixels(BWidth, NHeight, GL_RGB, GL_UNSIGNED_BYTE, BmpData);
//      }
////      glRasterPos4dv(raster_pos);
//      glEndList();
//      cs->Top = 0;                 cs->Left = 0;
//      cs->Right = FCharOffset*5;   cs->Top = FMaxHeight;
//    }
//  }
//  delete [] BmpData;
//}
//..............................................................................
void TGlFont::CreateTextures(uint16_t Width, uint16_t Height)  {
  if( Width < MaxWidth || Width < 0 ||
      Height < MaxHeight || Height < 0 ||
      MaxWidth <=0 || MaxHeight <=0 )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("font size w:") << Width << "; h:" << Height);
  if( Textures == NULL )  {
    Textures = new GLuint[256];
    glGenTextures(256, &Textures[0]);
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
          if( cs->Data[ind] != cs->Background && cs->Data[ind+1] != cs->Background && cs->Data[ind+2] != cs->Background )  {
            BmpData[ind1+3] = 255;
            BmpData[ind1]   = (unsigned char)olx_round(255*Material.AmbientF[0]);
            BmpData[ind1+1] = (unsigned char)olx_round(255*Material.AmbientF[1]);
            BmpData[ind1+2] = (unsigned char)olx_round(255*Material.AmbientF[2]);
          }
        }
      }
      glBindTexture( GL_TEXTURE_2D, Textures[i]);
      glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );  
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
//      gluBuild2DMipmaps( GL_TEXTURE_2D, 3, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, cs->Data );      
      //gluBuild2DMipmaps( GL_TEXTURE_2D, 4, txt_dim, txt_dim, GL_RGBA, GL_UNSIGNED_BYTE, BmpData );      
      glTexImage2D(GL_TEXTURE_2D, 0, 4, txt_w, txt_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, BmpData);
    }
    else  {  // an empty character as a space char
      //glDeleteTextures(1, &Textures[i]);
      //Textures[i] = ~0;
      cs->Top = 0;                 cs->Left = 0;
      cs->Right = CharOffset*5;   cs->Top = MaxHeight;
    }
  }
  delete [] BmpData;
}
//..............................................................................
void TGlFont::DrawGlText(const vec3d& from, const olxstr& text, bool FixedW)  {
  if( Textures == NULL || text.IsEmpty() )  return;
  glEnable( GL_TEXTURE_2D );

  glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  double step = 0.2, tx = (double)MaxWidth*step/TextureWidth, st=0,
    aspect=step*(double)TextureHeight/TextureWidth;
  //glEnable(GL_COLOR_MATERIAL);
  //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
//  glColor4d(1, 1, 1, 0.5);
  glTexCoord2d( 0, 0 );
  glNormal3d(0, 0, 1);
  for( size_t i=0; i < text.Length(); i++ )  {
    const unsigned ch = (unsigned)text.CharAt(i);
    if( ch > 256 )  continue;
    TFontCharSize* cs = CharSizes[ch];
    glBindTexture(GL_TEXTURE_2D, Textures[ch] );
    if( Textures[ch] == (GLuint)~0 )  continue;  // empty char
    glBegin(GL_QUADS);
    glTexCoord2d( 1, 0 );  //0,1
    glVertex3d(from[0], from[1], from[2]);
    glTexCoord2d( 1, 1 );  // 0,0
    glVertex3d(from[0], from[1]-aspect, from[2]);
    glTexCoord2d( 0, 1 ); // 1,0
    glVertex3d(from[0]-step, from[1]-aspect, from[2]);
    glTexCoord2d( 0, 0 ); // 1, 1
    glVertex3d(from[0]-step, from[1], from[2]);
    glEnd();
    if( !FixedW )
      tx = step*(cs->Right-cs->Left+CharOffset)/TextureWidth;
    glTranslated(tx, 0, 0);
    st -= tx;
  }
  glTranslated(st, 0, 0);
  glDisable( GL_TEXTURE_2D );
  glDisable(GL_ALPHA_TEST);
  glDisable(GL_BLEND);
  glDisable(GL_COLOR_MATERIAL);
}
//..............................................................................
void TGlFont::SetMaterial(const TGlMaterial& m)  {
  if( Material == m )  return;
  bool ModifyTextures = !(Material.AmbientF == m.AmbientF);
  Material = m;
  if( Textures == NULL || !ModifyTextures )  return;
  unsigned char *BmpData = new unsigned char [TextureWidth*TextureHeight*4];
  for( int i=0; i < 256; i++ )  {
    glBindTexture(GL_TEXTURE_2D, Textures[i] );
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, BmpData);
    for( int j=0; j < TextureWidth*TextureHeight*4; j+=4 )  {
      if( BmpData[j+3] == 255 )  {
        BmpData[j]   = (unsigned char)olx_round(255*m.AmbientF.Data()[0]);
        BmpData[j+1] = (unsigned char)olx_round(255*m.AmbientF.Data()[1]);
        BmpData[j+2] = (unsigned char)olx_round(255*m.AmbientF.Data()[2]);
      }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, BmpData);
  }
}
//..............................................................................
