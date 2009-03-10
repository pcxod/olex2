//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifndef gldefsH
#define gldefsH

#define FRotationDiv 2.0
#define FZoomDiv 15.0

// the selection buffer size
#define MAXSELECT 100
//---------------------------------------------------------------------------
#ifndef GetRValue
  #define GetRValue(rgb)   ((uint8_t) (rgb))
#endif
//---------------------------------------------------------------------------
#ifndef GetBValue
  #define GetBValue(rgb)   ((uint8_t) ((rgb) >> 16))
#endif
//---------------------------------------------------------------------------
#ifndef GetGValue
  #define GetGValue(rgb)   ((uint8_t) (((uint16_t) (rgb)) >> 8))
#endif
//---------------------------------------------------------------------------
#ifndef GetAValue
  #define GetAValue(rgb)   GetGValue( rgb >> 16)
#endif
//---------------------------------------------------------------------------
#ifndef RGB
  #define RGB(r, g ,b)  ((uint32_t) (((uint16_t) (r) | ((uint16_t) (g) << 8)) | (((uint32_t) (uint8_t) (b)) << 16)))
#endif
//---------------------------------------------------------------------------
#ifndef RGBA
  #define RGBA(r, g, b, a) ( ((uint32_t)(r)) | ((uint32_t)(g) << 8) | ((uint32_t)(b) << 16) | ((uint32_t)(a) << 24))
#endif

#endif
