//---------------------------------------------------------------------------

#ifndef gltextboxH
#define gltextboxH
#include "glbase.h"
#include "estrlist.h"
#include "glmouselistener.h"

BeginGlNamespace()

class TGlTextBox: public TGlMouseListener  {
  float LineSpacing;
  uint16_t Width, Height, MaxStringLength;
  int Top, Left;
  TStrPObjList<olxstr,TGlMaterial*> FBuffer;   // the content
  double Z;
  uint16_t FontIndex;
  bool ScrollDirectionUp;
protected:
public:
  TGlTextBox(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TGlTextBox();

  void Clear();

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min) {  return false;  }

  DefPropP(double, Z)
  DefPropP(uint16_t, Width)
  DefPropP(uint16_t, Height)
  void SetLeft(int left);
  int GetLeft() const {  return Left;  }
  void SetTop(int top);
  int GetTop() const {  return Top;  }

  DefPropP(uint16_t, FontIndex)
  DefPropP(uint16_t, MaxStringLength)
  DefPropP(float, LineSpacing)

  void PostText(const olxstr &S, class TGlMaterial *M=NULL);
  void PostText(const TStrList &SL, TGlMaterial *M=NULL);
  inline void NewLine() {  FBuffer.Add(EmptyString); }
  class TGlFont& GetFont()  const;
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
};


EndGlNamespace()

#endif
