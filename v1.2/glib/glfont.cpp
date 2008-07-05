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
TGlFont::TGlFont(const olxstr& name) {
  FFontBase = glGenLists(256);
  if( FFontBase == -1 )
    throw TOutOfMemoryException(__OlxSourceInfo);
  CharSizes.SetCount(256);
  for( int i=0; i < 256; i++ )
    CharSizes[i] = new TFontCharSize();
  FCharOffset = 2;
  FFlags = 0;
  Textures = NULL;
  ClearData();
  Name = name;
  PointSize = 0;
}
//..............................................................................
TGlFont::~TGlFont()  {
  for( int i=0; i < 256; i++ )
    delete CharSizes[i];
  if( FFontBase != -1 )
    glDeleteLists(FFontBase, 256);
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
  FMaxWidth = FMaxHeight = 0;
  FLeftmost = 1000; // an arbitrary value
  FTopmost  = 1000;
  //if( Textures != NULL )  {
  //  glDeleteTextures(256, Textures);
  //  delete [] Textures;
  //  Textures = NULL;
  //}
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
void TGlFont::CreateTextures(short Width, short Height)  {
  return;
  if( Width < FMaxWidth || Width < 0 ||
      Height < FMaxHeight || Height < 0 ||
      FMaxWidth <=0 || FMaxHeight <=0 )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("font size w:") << Width << "; h:" << Height);
  if( Textures == NULL )  {
    Textures = new GLuint[256];
    glGenTextures(256, &Textures[0]);
  }
  // calculate the texture size
  short txt_w = 1, txt_h = 1;
  while( txt_w < FMaxWidth )  txt_w *= 2;
  while( txt_h < FMaxHeight )  txt_h *= 2;
  TextureWidth = txt_w;
  TextureHeight = txt_h;
  unsigned char *BmpData = new unsigned char [txt_w*txt_h*4];

  for( int i=0; i < 256; i++ )  {
    TFontCharSize* cs = CharSize(i);
    memset(BmpData, 0, txt_w*txt_h*4); // initialise the bits array
    if( cs->Data != NULL )  {
      for( int j=cs->Left; j <= cs->Right; j++ )  {
        for( int k=cs->Top; k <= cs->Bottom; k++ )  {
          const int ind = (k*Width+j)*3;
          const int ind1 = (k*txt_w+j-cs->Left)*4;
          if( cs->Data[ind] != cs->Background && cs->Data[ind+1] != cs->Background && cs->Data[ind+2] != cs->Background )  {
            BmpData[ind1+3] = 255;
            BmpData[ind1]   = (unsigned char)Round(255*FMaterial.AmbientF[0]);
            BmpData[ind1+1] = (unsigned char)Round(255*FMaterial.AmbientF[1]);
            BmpData[ind1+2] = (unsigned char)Round(255*FMaterial.AmbientF[2]);
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
      cs->Right = FCharOffset*5;   cs->Top = FMaxHeight;
    }
  }
  delete [] BmpData;
}
//..............................................................................
void TGlFont::DrawGlText(const TVPointD& from, const olxstr& text, bool FixedW)  {
  if( Textures == NULL || text.IsEmpty() )  return;
  glEnable( GL_TEXTURE_2D );

  glEnable(GL_ALPHA_TEST);
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  double step = 0.2, tx = (double)FMaxWidth*step/TextureWidth, st=0,
    aspect=step*(double)TextureHeight/TextureWidth;
  //glEnable(GL_COLOR_MATERIAL);
  //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
//  glColor4d(1, 1, 1, 0.5);
  glTexCoord2d( 0, 0 );
  glNormal3d(0, 0, 1);
  for( int i=0; i < text.Length(); i++ )  {
    const unsigned ch = (unsigned)text.CharAt(i);
    if( ch > 256 )  continue;
    TFontCharSize* cs = CharSizes[ch];
    glBindTexture(GL_TEXTURE_2D, Textures[ch] );
    if( Textures[ch] == ~0 )  continue;  // empty char
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
      tx = step*(cs->Right-cs->Left+FCharOffset)/TextureWidth;
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
  if( FMaterial == m )  return;
  bool ModifyTextures = !(FMaterial.AmbientF == m.AmbientF);
  FMaterial = m;
  if( Textures == NULL || !ModifyTextures )  return;
  unsigned char *BmpData = new unsigned char [TextureWidth*TextureHeight*4];
  for( int i=0; i < 256; i++ )  {
    glBindTexture(GL_TEXTURE_2D, Textures[i] );
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, BmpData);
    for( int j=0; j < TextureWidth*TextureHeight*4; j+=4 )  {
      if( BmpData[j+3] == 255 )  {
        BmpData[j]   = (unsigned char)Round(255*m.AmbientF.Data()[0]);
        BmpData[j+1] = (unsigned char)Round(255*m.AmbientF.Data()[1]);
        BmpData[j+2] = (unsigned char)Round(255*m.AmbientF.Data()[2]);
      }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, 4, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, BmpData);
  }
}
