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
  TDUserObj(TGlRenderer& Render, short type, ematd* data, const olxstr& collectionName);
  virtual ~TDUserObj()  {  
    if( Data != NULL )  
      delete Data;
  }
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;  }
};


EndGxlNamespace()
#endif
