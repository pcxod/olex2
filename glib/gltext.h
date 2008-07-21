//---------------------------------------------------------------------------

#ifndef gltextH
#define gltextH
#include "glbase.h"
#include "glmouselistener.h"

BeginGlNamespace()

const short  gltStaticPos     = 0x0001,  // TGlText flags
             gltStaticWidth   = 0x0002,
             gltStaticFace   = 0x0004;

class TGlText: public TGlMouseListener  {
  olxstr Text;
  short TextFlags;
  float Width, // width of the text specified by user
        CalcWidth, // calculated width (using GlyphMetrics info)
        CalcHeight;  // calculated height
  short FontIndex;
protected:
  void CalcWH();  // calculates width of the text
public:
  TGlText(const olxstr& collectionName, TGlRender *Render, const olxstr &Text);
  void Create(const olxstr& cName = EmptyString);
  virtual ~TGlText()  {  ;  }

  void SetText(const olxstr &T);
  inline const olxstr& GetText() const {  return Text; }
  inline float GetHeight(){  return CalcHeight; }
  inline float GetWidth(){  return Width; }
  void SetWidth(float W);  // sets width of the text
  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;};

  inline bool IsStaticPos() const {  return (TextFlags & gltStaticPos) == gltStaticPos; };
  virtual void SetStaticPos(bool On);
  // specifies if text is rotated with the model

  inline bool IsStaticWidth() const {  return (TextFlags & gltStaticWidth) == gltStaticWidth; };
  virtual void SetStaticWidth(bool On);
  // specifies if text is zoomed with the model

  inline bool IsStaticFace() const {  return (TextFlags & gltStaticFace)  == gltStaticFace; };
  virtual void SetStaticFace(bool On);
  // specifies if text appearence does not depend on model basis

  class TGlFont *Font()  const;
  DefPropP(short, FontIndex)
};


EndGlNamespace()
#endif
