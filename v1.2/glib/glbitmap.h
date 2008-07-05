#ifndef glbitmapH
#define glbitmapH
#include "glbase.h"
#include "glmouselistener.h"
//---------------------------------------------------------------------------
BeginGlNamespace()

class TGlBitmap : public TGlMouseListener  {
  int Width, Height, Top, Left; // to clip the content
  int TextureId;
  double Z;
public:

  TGlBitmap(const olxstr& collectionName, TGlRender *Render,
    int left, int top, int width, int height,
      unsigned char* RGB, GLenum format);

  void ReplaceData(int width, int height, unsigned char* RGB, GLenum format);

  void Create(const olxstr& cName = EmptyString);
  virtual ~TGlBitmap();

  void SetZ( double z );
  inline double GetZ( ) const   {  return Z;  }

  void SetWidth(int w);
  int GetWidth() const;
  void SetHeight(int w);
  int GetHeight() const;
  void SetLeft(int w);
  int GetLeft()  const;
  void SetTop(int w);
  int GetTop()  const;

  virtual bool Orient(TGlPrimitive *P);
  virtual bool GetDimensions(TVPointD &Max, TVPointD &Min);
};

EndGlNamespace()
#endif
