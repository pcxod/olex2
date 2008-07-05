#ifndef duserobjH
#define duserobjH
#include "gxbase.h"
#include "glmouselistener.h"

BeginGxlNamespace()

class TDUserObj: public TGlMouseListener  {
  short Type;
  TMatrixD* Data;
public:
  TDUserObj(short type, TMatrixD* data, const olxstr& collectionName, TGlRender *Render);
  virtual ~TDUserObj()  {  
    if( Data != NULL )  delete Data;
  }
  void Create(const olxstr& cName = EmptyString);
  bool Orient(TGlPrimitive *P);
  bool GetDimensions(TVPointD &Max, TVPointD &Min){  return false;  }
};


EndGxlNamespace()
#endif
