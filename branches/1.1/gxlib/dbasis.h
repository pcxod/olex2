#ifndef __olx_glx_dbasis_H
#define __olx_glx_dbasis_H
#include "gllabel.h"
#include "asymmunit.h"

BeginGxlNamespace()

class TDBasis: public TGlMouseListener, public TXGlLabel::ICrdTransformer  {
  TAsymmUnit* AU;
protected:
  TXGlLabel* Labels[3];
  virtual vec3d ForRaster(const TXGlLabel&) const;
  virtual vec3d ForVector(const TXGlLabel&) const;
  virtual vec3d& AdjustZ(vec3d&) const;
public:
  TDBasis(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TDBasis();
  void SetAsymmUnit(TAsymmUnit& au);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }
  void ListPrimitives(TStrList &List) const;
  void UpdatePrimitives(int32_t Mask, const ACreationParams* cpar=NULL);

  void SetLabelsFont(uint16_t fnt_index);
  void UpdateLabels();
  size_t LabelCount() const {  return 3;  }
  TXGlLabel& GetLabel(size_t i) const {  return *Labels[i];  }
  void SetVisible(bool v);

  void ToDataItem(TDataItem& di) const;
  void FromDataItem(const TDataItem& di);
};

EndGxlNamespace()
#endif
 
