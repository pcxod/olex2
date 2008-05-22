#ifndef dunitcellH
#define dunitcellH
#include "gxbase.h"
#include "vpoint.h"
#include "gdrawobject.h"

BeginGxlNamespace()

class TDUnitCell: public AGDrawObject  {
  bool FReciprical;
  TVPointD FCenter, FOldCenter;
  TGlPrimitive *FGlP;
  TMatrixD CellToCartesian, HklToCartesian;
public:
  TDUnitCell(const olxstr& collectionName, TGlRender *Render);
  virtual ~TDUnitCell() {  }
  void Init(const TVectorD& cell_params);
  void Create(const olxstr& cName = EmptyString);
  bool Orient(TGlPrimitive *P);
  bool GetDimensions(TVPointD &Max, TVPointD &Min){  return false;  }

  inline bool IsReciprical()  const {  return FReciprical;  }
  void Reciprical(bool v );
  inline TVPointD& Center()         {  return FCenter;  }
  inline const TMatrixD& GetCellToCartesian() const {  return CellToCartesian;  }
  inline const TMatrixD& GetHklToCartesian()  const {  return HklToCartesian;  }
  void ResetCentres();
};


EndGxlNamespace()
#endif
