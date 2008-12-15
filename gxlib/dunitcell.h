#ifndef dunitcellH
#define dunitcellH
#include "gxbase.h"
#include "threex3.h"
#include "gdrawobject.h"

BeginGxlNamespace()

class TDUnitCell: public AGDrawObject  {
  bool FReciprical;
  vec3d FCenter, FOldCenter;
  TGlPrimitive *FGlP;
  mat3d CellToCartesian, HklToCartesian;
public:
  TDUnitCell(const olxstr& collectionName, TGlRender *Render);
  virtual ~TDUnitCell() {  }
  void Init(const double cell_params[6]);
  void Create(const olxstr& cName = EmptyString, const CreationParams* cpar = NULL);
  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min);

  inline bool IsReciprical()  const {  return FReciprical;  }
  void Reciprical(bool v );
  inline vec3d& Center()         {  return FCenter;  }
  inline const mat3d& GetCellToCartesian() const {  return CellToCartesian;  }
  inline const mat3d& GetHklToCartesian()  const {  return HklToCartesian;  }
  void ResetCentres();
};


EndGxlNamespace()
#endif
