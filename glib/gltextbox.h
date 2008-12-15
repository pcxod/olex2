//---------------------------------------------------------------------------

#ifndef gltextboxH
#define gltextboxH
#include "glbase.h"
#include "estrlist.h"
#include "glmouselistener.h"

BeginGlNamespace()

class TGlTextBox: public TGlMouseListener  {
  float LineSpacing;
  int Width, Height, Top, Left;
  TStrPObjList<olxstr,TGlMaterial*> FBuffer;   // the content
  double Z;
  short FontIndex;
  bool FScrollDirectionUp;
  int MaxStringLength;
protected:
public:
  TGlTextBox(const olxstr& collectionName, TGlRender *Render);
  void Create(const olxstr& cName = EmptyString, const CreationParams* cpar = NULL);
  virtual ~TGlTextBox();

  void Clear();

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min) {  return false;  }

  DefPropP(double, Z)
  DefPropP(int, Width)
  DefPropP(int, Height)
  void SetLeft(int w);
  inline int GetLeft()  const  {  return Left;  }
  void SetTop(int w);
  inline int GetTop()    const {  return Top;  }

  DefPropP(int, FontIndex)
  DefPropP(int, MaxStringLength)
  DefPropP(float, LineSpacing)

  void PostText(const olxstr &S, class TGlMaterial *M=NULL);
  void PostText(const TStrList &SL, TGlMaterial *M=NULL);
  inline void NewLine() {  FBuffer.Add(EmptyString); }

  class TGlFont *Font()  const;

  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
};


EndGlNamespace()

#endif
