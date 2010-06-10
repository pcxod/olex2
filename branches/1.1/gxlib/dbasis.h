#ifndef __olx_glx_dbasis_H
#define __olx_glx_dbasis_H
#include "gllabel.h"
#include "asymmunit.h"

BeginGxlNamespace()

class TDBasis: public TGlMouseListener  {
  TAsymmUnit* AU;
protected:
  TXGlLabel* Labels[3];
public:
  TDBasis(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TDBasis();
  void SetAsymmUnit(TAsymmUnit& au);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }
  void ListPrimitives(TStrList &List) const;
  void UpdatePrimitives(int32_t Mask, const ACreationParams* cpar=NULL);

  void ToDataItem(TDataItem& di) const;
  void FromDataItem(const TDataItem& di);
};

EndGxlNamespace()
#endif
 
