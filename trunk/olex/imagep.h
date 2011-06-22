/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_imagepH
#define __olx_imagepH

class TProcessImage  {
public:
   // takes a 3x3 filter
   static void FilterBW(unsigned char *Image, int width, int height, int bytePerColour,
          float Filter[3][3], unsigned char weight, int BGcolour, bool removeBG);
   static void FilterC(unsigned char *Image, int width, int height, int bytePerColour,
          float Filter[3][3], unsigned char weight, int BGcolour, bool removeBG);
   // applies {-1,-1,-1; -1;9-1; -1;-1;-1} filter preserving colour
   static void EmbossC(unsigned char *Image, int width, int height, int bytePerColour,
          int BgColour);
   // applies {-1,-1,-1; -1;8-1; -1;-1;-1} filter with average colour
   static void EmbossBW(unsigned char *Image, int width, int height, int bytePerColour,
          int BgColour);
};
#endif
