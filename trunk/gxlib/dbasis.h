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
  virtual ~TDBasis() {  }
  void AsymmUnit(TAsymmUnit *AU){  FAU = AU; }
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min)  {  return false;  }
};


EndGxlNamespace()
#endif
 
