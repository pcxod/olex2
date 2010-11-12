#ifndef imagepH
#define imagepH

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
 
