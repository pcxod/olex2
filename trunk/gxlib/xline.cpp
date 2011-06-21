#include "xline.h"
#include "gpcollection.h"
#include "xatom.h"
#include "pers_util.h"

TXLine::TXLine(TGlRenderer& r, const olxstr& collectionName, const vec3d& base, const vec3d& edge): 
  TXBond(NULL, r, collectionName),
  FBase(base), FEdge(edge)
{
  Init();
}
//..............................................................................
void TXLine::Init(bool update_label) {
  vec3d C(FEdge - FBase);
  if( update_label )  {
    GetGlLabel().SetOffset((FBase+FEdge)/2);
    GetGlLabel().SetLabel(olxstr::FormatFloat(3, C.Length()));
    GetGlLabel().SetVisible(true);
  }
  if( !C.IsNull() )  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = (float)(acos(C[2])*180/M_PI);
    Params()[1] = -C[1];
    Params()[2] = C[0];
  }
}
//..............................................................................
bool TXLine::Orient(TGlPrimitive& GlP)  {
  olxstr Length = olxstr::FormatFloat(3, Params()[3]);
  if( GlP.GetType() == sgloText )  {
    vec3d V;
    V = (FEdge+FBase)/2;
    V += Parent.GetBasis().GetCenter();
    V = Parent.GetBasis().GetMatrix()*V;
    olx_gl::rasterPos(V[0]+0.15, V[1]+0.15, V[2]+5);
    GlP.SetString(&Length);
    GlP.Draw();
    GlP.SetString(NULL);
    return true;
  }
  else  {
    olx_gl::translate(FBase);
    olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
    olx_gl::scale(Params()[4], Params()[4], Params()[3]);
  }
  return false;
} 
//..............................................................................
void TXLine::ToDataItem(TDataItem &di) const {
  di.SetValue(GetCollectionName());
  di.AddField("r", GetRadius())
    .AddField("base", PersUtil::VecToStr(FBase))
    .AddField("edge", PersUtil::VecToStr(FEdge))
    ;
  GetGlLabel().ToDataItem(di.AddItem("Label"));
}
//..............................................................................
void TXLine::FromDataItem(const TDataItem &di)  {
  SetCollectionName(di.GetValue());
  FBase = PersUtil::FloatVecFromStr(di.GetRequiredField("base"));
  FEdge = PersUtil::FloatVecFromStr(di.GetRequiredField("edge"));
  SetRadius(di.GetRequiredField("r").ToDouble());
  GetGlLabel().FromDataItem(di.FindRequiredItem("Label"));
  Init(false);
}
//..............................................................................
