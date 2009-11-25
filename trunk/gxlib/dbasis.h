#ifndef __olx_dbasis_H
#define __olx_dbasis_H
#include "gxbase.h"
#include "glmouselistener.h"
#include "asymmunit.h"

BeginGxlNamespace()

class TDBasis: public TGlMouseListener  {
  TAsymmUnit* AU;
protected:
public:
  TDBasis(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TDBasis() {  }
  void SetAsymmUnit(TAsymmUnit* au){  AU = au; }
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }
  void ToDataItem(TDataItem& di) const;
  void FromDataItem(const TDataItem& di);
};


EndGxlNamespace()
#endif
 
