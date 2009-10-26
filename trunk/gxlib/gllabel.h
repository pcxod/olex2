#ifndef __olx_glx_label_H
#define __olx_glx_label_H
#include "gxbase.h"

#include "glmouselistener.h"
#include "glfont.h"
#include "dataitem.h"

BeginGxlNamespace()

class TXGlLabel: public TGlMouseListener  {
  olxstr FLabel;
  uint16_t FontIndex;
  double OffsetX, OffsetY;
  vec3d Center;
public:
  TXGlLabel(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TXGlLabel();

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;  }
  inline const olxstr& GetLabel() const   {  return FLabel;  }
  void SetLabel(const olxstr& L);
  vec3d GetRasterPosition() const;
  DefPropC(vec3d, Center)

  TGlFont& GetFont() const;
  DefPropP(uint16_t, FontIndex)

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item);
};

EndGxlNamespace()
#endif
 
