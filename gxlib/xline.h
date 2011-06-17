#ifndef __olx_gxl_xline_H
#define __olx_gxl_xline_H
#include "xbond.h"
#include "xatom.h"

BeginGxlNamespace()

class TXLine: public TXBond  {
  vec3d FBase, FEdge;
  void Init(bool update_label=true);
public:
  TXLine(TGlRenderer& Renderer, const olxstr& collectionName,
    const vec3d& base, const vec3d& edge);
  TXLine(TGlRenderer& Renderer) : TXBond(NULL, Renderer, EmptyString()) {}
  void Create(const olxstr& cName=EmptyString()) {
    TXBond::Create(cName);
  }
  virtual ~TXLine() {}

  vec3d& Base()  {  return FBase;  }
  vec3d& Edge()  {  return FEdge;  }

  bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }
  bool Orient(TGlPrimitive& P);
  void SetRadius(double V)  {  Params()[4] = V;  }
  double GetRadius() const {  return Params()[4]; }
  void SetLength(double V)  {  Params()[3] = V;  }
  double GetLength() const {  return Params()[3]; }

  void ToDataItem(TDataItem &di) const;
  void FromDataItem(const TDataItem &di);
};

EndGxlNamespace()
#endif
 
