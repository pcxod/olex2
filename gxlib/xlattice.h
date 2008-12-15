#ifndef xlatticeH
#define xlatticeH
#include "gxbase.h"
#include "glmouselistener.h"
//#include "arrays.h"

#include "asymmunit.h"

BeginGxlNamespace()

class TXLattice: public TGlMouseListener  {
  bool Fixed;
  short Size;
  class TGlPrimitive* Lines;
  mat3d LatticeBasis;
public:
  TXLattice(const olxstr& collectionName, TGlRender *Render);
  virtual ~TXLattice();
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d& Max, vec3d& Min);

  DefPropC(mat3d, LatticeBasis)

  inline bool IsFixed()  const {  return Fixed;  }
  void SetFixed(bool v );

  inline short GetSize()  const {  return Size;  }
  void SetSize(short v);

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);
};

EndGxlNamespace()
#endif
