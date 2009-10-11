#ifndef dunitcellH
#define dunitcellH
#include "gxbase.h"
#include "threex3.h"
#include "gdrawobject.h"

BeginGxlNamespace()

class TDUnitCell: public AGDrawObject  {
  bool Reciprocal;
  vec3d Center, OldCenter;
  TGlPrimitive *FGlP;
  mat3d CellToCartesian, HklToCartesian;
public:
  TDUnitCell(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TDUnitCell() {  }
  void Init(const double cell_params[6]);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min);

  inline bool IsReciprocal()  const {  return Reciprocal;  }
  void SetReciprocal(bool v);
  DefPropC(vec3d, Center)
  inline const mat3d& GetCellToCartesian() const {  return CellToCartesian;  }
  inline const mat3d& GetHklToCartesian()  const {  return HklToCartesian;  }
  void ResetCentres();
};


EndGxlNamespace()
#endif
