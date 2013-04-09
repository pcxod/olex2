/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_dl_gldefs_H
#define __olx_dl_gldefs_H

#define FRotationDiv 2.0
#define FZoomDiv 15.0

// the selection buffer size
#define MAXSELECT 100
#define OLX_GetRValue(rgb)   (uint8_t) (rgb)
#define OLX_GetBValue(rgb)   (uint8_t) ((rgb) >> 16)
#define OLX_GetGValue(rgb)   (uint8_t) ((rgb) >> 8)
#define OLX_GetAValue(rgb)   (uint8_t) ((rgb) >> 24)
#define OLX_RGB(r, g ,b)  ((uint32_t) (((uint16_t) (r) | ((uint16_t) (g) << 8)) | (((uint32_t) (uint8_t) (b)) << 16)))
#define OLX_RGBA(r, g, b, a) ( ((uint32_t)(r)) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 16) | ((uint32_t)(a) << 24))
#endif
