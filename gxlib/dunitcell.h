#ifndef dunitcellH
#define dunitcellH
#include "gxbase.h"
#include "threex3.h"
#include "gdrawobject.h"
#include "glprimitive.h"

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
  void ListPrimitives(TStrList &List) const;
  void UpdatePrimitives(int32_t Mask, const ACreationParams* cpar=NULL);

  size_t VertexCount() const {  return FGlP == NULL ? 0 : 8;  }
  const vec3f& GetVertex(size_t i) const {
    if( FGlP == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
    switch(i)  {
      case 0:  return FGlP->Vertices[0];  break;
      case 1:  return FGlP->Vertices[1];  break;
      case 2:  return FGlP->Vertices[3];  break;
      case 3:  return FGlP->Vertices[5];  break;
      case 4:  return FGlP->Vertices[7];  break;
      case 5:  return FGlP->Vertices[9];  break;
      case 6:  return FGlP->Vertices[13];  break;
      case 7:  return FGlP->Vertices[19];  break;
      default:
        throw TInvalidArgumentException(__OlxSourceInfo, "vertex index");
    }
  }
  size_t EdgeCount() const {  return FGlP == NULL ? 0 : 24;  }
  const vec3f& GetEdge(size_t i) const {  return FGlP->Vertices[i];  }
  inline bool IsReciprocal()  const {  return Reciprocal;  }
  void SetReciprocal(bool v);
  DefPropC(vec3d, Center)
  inline const mat3d& GetCellToCartesian() const {  return CellToCartesian;  }
  inline const mat3d& GetHklToCartesian()  const {  return HklToCartesian;  }
  void ResetCentres();
};


EndGxlNamespace()
#endif
