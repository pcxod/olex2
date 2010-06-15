#ifndef __olx_glx_label_H
#define __olx_glx_label_H
#include "gxbase.h"
#include "glmousehandler.h"
#include "glfont.h"
#include "dataitem.h"

BeginGxlNamespace()

class TXGlLabel: public AGlMouseHandlerImp  {
public:
  class ICrdTransformer  {
  public:
    virtual vec3d ForRaster(const TXGlLabel&) const = 0;
    virtual vec3d ForVector(const TXGlLabel&) const = 0;
    // returns the argument after the adjustment
    virtual vec3d& AdjustZ(vec3d& v) const = 0;
  };
private:
  olxstr FLabel;
  uint16_t FontIndex;
  TTextRect text_rect;
  vec3d Offset;
  ICrdTransformer* Transformer;
protected:
  vec3d _Center;
  virtual bool DoTranslate(const vec3d& t) {  _Center += t;  return true;  }
  virtual bool DoRotate(const vec3d&, double) {  return false;  }
  virtual bool DoZoom(double, bool)  {  return false;  }
  const vec3d& GetCenter() const {  return _Center;  }
public:
  TXGlLabel(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TXGlLabel();

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;  }
  inline const olxstr& GetLabel() const   {  return FLabel;  }
  void SetLabel(const olxstr& L);
  vec3d GetRasterPosition() const;
  vec3d GetVectorPosition() const;
  // the object must be mannaged by whoever created it!
  DefPropP(ICrdTransformer*, Transformer)
  DefPropC(vec3d, Offset)
  TGlFont& GetFont() const;
  DefPropP(uint16_t, FontIndex)
  void TranslateBasis(const vec3d& v)  {  DoTranslate(v);  }

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item);
};

EndGxlNamespace()
#endif
 
