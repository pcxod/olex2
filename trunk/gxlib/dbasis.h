#ifndef dbasisH
#define dbasisH
#include "gxbase.h"
#include "glmouselistener.h"
#include "asymmunit.h"

BeginGxlNamespace()

class TDBasis: public TGlMouseListener  {
  TAsymmUnit *FAU;
protected:
public:
  TDBasis(const olxstr& collectionName, TGlRender *Render);
  virtual ~TDBasis(){  return; };
  void AsymmUnit(TAsymmUnit *AU){  FAU = AU; }
  void Create(const olxstr& cName = EmptyString);
  bool Orient(TGlPrimitive *P);
  bool GetDimensions(TVPointD &Max, TVPointD &Min)  {  return false;  }
};


EndGxlNamespace()
#endif
 
