#ifndef duserobjH
#define duserobjH
#include "gxbase.h"
#include "glmouselistener.h"
#include "ematrix.h"

BeginGxlNamespace()

class TDUserObj: public TGlMouseListener  {
  short Type;
  ematd* Data;
public:
  TDUserObj(short type, ematd* data, const olxstr& collectionName, TGlRender *Render);
  virtual ~TDUserObj()  {  
    if( Data != NULL )  delete Data;
  }
  void Create(const olxstr& cName = EmptyString, const CreationParams* cpar = NULL);
  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;  }
};


EndGxlNamespace()
#endif
