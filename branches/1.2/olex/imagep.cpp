/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "imagep.h"
#include "gldefs.h"
#include "emath.h"

void TProcessImage::FilterBW(unsigned char *Image, int width, int height, int bytePerColour,
          float Filter[3][3], unsigned char weight, int BGcolour, bool removeBG)
{
  float filterW = 0;
  for(int i=0; i < 3; i++ )
    for( int j=0; j < 3; j++ )
      filterW += Filter[i][j];
  if( !filterW )  filterW = 1;
  filterW = olx_abs(filterW);
  int index, index1, summ;
  int total = width*height*bytePerColour;
  int newImageMax = width*height;
  unsigned char MeanBG = (OLX_GetRValue(BGcolour) +OLX_GetGValue(BGcolour) +
    OLX_GetBValue(BGcolour))/3;
  unsigned char * newImg = new unsigned char [newImageMax];

  for( int i=0; i < width; i++ )
  {
    for( int j=0; j < height; j++ )
    {
      index = (i*height + j)*bytePerColour;
      Image[index] = (Image[index] + Image[index+1] + Image[index+2])/3;
      Image[index+1] = Image[index];
      Image[index+2] = Image[index];
    }
  }
  for( int i=0; i < height; i++ )
  {
    for( int j=0; j < width; j++ )
    {
      index = (i*width + j);
      summ = 0;
      for( int k=-1; k < 2; k++ )
      {
        for( int l=-1; l < 2; l++ )
        {
          index1 = ((i+k)*width + j+l)*bytePerColour;
          if( index1 >= total || index1 < 0 ) break;
          summ += (int)(Image[index1] * Filter[1+k][1+l]);
        }
        if( index1 >= total || index1 < 0 ) break;
      }
      if( index1 >= total || index1 < 0 ) summ = 255;
      summ /= (int)filterW;
      if( removeBG && (!summ) && Image[index*bytePerColour] == MeanBG )
      {  summ = 255;  }
      else
      {
       summ += weight;
       if( summ > 255 ) summ = 255;
       if( summ < 0 )   summ = 0;
      }
      newImg[index] = summ;
    }
  }
  for( int i=0, j=0; i < width*height; i++, j+=3 )
  {  Image[j] = Image[j+1] = Image[j+2] = newImg[i];  }
  delete []newImg;

}
void TProcessImage::FilterC(unsigned char *Image, int width, int height, int bytePerColour,
          float Filter[3][3], unsigned char weight, int BGcolour, bool removeBG)
{
  float filterW = 0;
  for(int i=0; i < 3; i++ )
    for( int j=0; j < 3; j++ )
      filterW += Filter[i][j];
  if( !filterW )  filterW = 1;
  filterW = olx_abs(filterW);
  int index, index1, summr, summg, summb;
  int total = width*height*bytePerColour;
  unsigned char * newImg = new unsigned char [total];

  for( int i=0; i < height; i++ )
  {
    for( int j=0; j < width; j++ )
    {
      index = (i*width + j)*bytePerColour;
      summr = summg = summb = 0;
      for( int k=-1; k < 2; k++ )
      {
        for( int l=-1; l < 2; l++ )
        {
          index1 = ((i+k)*width + j+l)*bytePerColour;
          if( index1 >= total || index1 < 0 ) break;
          summr += (int)(Image[index1] * Filter[1+k][1+l]);
          summg += (int)(Image[index1+1] * Filter[1+k][1+l]);
          summb += (int)(Image[index1+2] * Filter[1+k][1+l]);
        }
        if( index1 >= total || index1 < 0 ) break;
      }
      if( index1 >= total || index1 < 0 )
      { summr = summg = summb = 255;  }
      summr /= (int)filterW;
      summg /= (int)filterW;
      summb /= (int)filterW;

      summr += weight;
      summg += weight;
      summb += weight;
      if( summr > 255 ) summr = 255;
      if( summr < 0 )   summr = 0;
      if( summg > 255 ) summg = 255;
      if( summg < 0 )   summg = 0;
      if( summb > 255 ) summb = 255;
      if( summb < 0 )   summb = 0;
      newImg[index] = summr;
      newImg[index+1] = summg;
      newImg[index+2] = summb;
    }
  }
  for( int i=0; i < width*height*bytePerColour; i++ )
  {  Image[i] = newImg[i];  }
  delete []newImg;

}
void TProcessImage::EmbossC(unsigned char *Image, int width, int height, int bytePerColour,
        int BgColour)
{
  float filter[3][3]={ {-1,-1,-1}, {-1,9,-1}, {-1,-1,-1}};
  FilterC(Image, width, height, bytePerColour, filter, 128, BgColour, true);
}
void TProcessImage::EmbossBW(unsigned char *Image, int width, int height, int bytePerColour,
        int BgColour)
{
  float filter[3][3]={ {-1,-1,-1}, {-1,8,-1}, {-1,-1,-1}};
  FilterBW(Image, width, height, bytePerColour, filter, 128, BgColour, true);
}
