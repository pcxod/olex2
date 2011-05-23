//----------------------------------------------------------------------------//
// TXAtom  - a drawing object for atom
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "xatom.h"
#include "glprimitive.h"
#include "gpcollection.h"
#include "styles.h"
#include "ellipsoid.h"
#include "symmparser.h"
#include "network.h"

#include "asymmunit.h"
#include "unitcell.h"
#include "lattice.h"
#include "planesort.h"

#include "glgroup.h"
#include "exyzgroup.h"
#include "glutil.h"

//..............................................................................
bool TXAtom::TStylesClear::Enter(const IEObject *Sender, const IEObject *Data)  {  
  TXAtom::FAtomParams = NULL; 
  TXAtom::ClearStaticObjects();
  return true; 
}
//..............................................................................
bool TXAtom::TStylesClear::Exit(const IEObject *Sender, const IEObject *Data)  {
  TXAtom::ValidateAtomParams();
  TXAtom::ClearStaticObjects();
  return true;
}
//..............................................................................
//..............................................................................
TXAtom::TContextClear::TContextClear(TGlRenderer& Render)  {
  Render.OnClear.Add(this);
}
//..............................................................................
bool TXAtom::TContextClear::Enter(const IEObject *Sender, const IEObject *Data)  {  
  TXAtom::ClearStaticObjects();
  return true; 
}
//..............................................................................
bool TXAtom::TContextClear::Exit(const IEObject *Sender, const IEObject *Data)  {
  return true;
}
//..............................................................................
//----------------------------------------------------------------------------//
// TSAtom function bodies
//----------------------------------------------------------------------------//
TStrPObjList<olxstr,TGlPrimitive*> TXAtom::FStaticObjects;
TTypeList<TGlPrimitiveParams> TXAtom::FPrimitiveParams;
float TXAtom::FTelpProb = 0;
float TXAtom::FQPeakScale = 0;
float TXAtom::FQPeakSizeScale = 0;
short TXAtom::FDefRad = 0;
short TXAtom::FDefDS = 0;
int TXAtom::OrtepSpheres = -1;
TGraphicsStyle* TXAtom::FAtomParams=NULL;
TXAtom::TStylesClear *TXAtom::OnStylesClear=NULL;
uint8_t TXAtom::PolyhedronIndex = ~0;
uint8_t TXAtom::SphereIndex = ~0;
uint8_t TXAtom::SmallSphereIndex = ~0;
uint8_t TXAtom::RimsIndex = ~0;
uint8_t TXAtom::DisksIndex = ~0;
uint8_t TXAtom::CrossIndex = ~0;
olxstr TXAtom::PolyTypeName("PolyType");
//..............................................................................
TXAtom::TXAtom(TNetwork* net, TGlRenderer& Render, const olxstr& collectionName) :
  TSAtom(net),
  AGlMouseHandlerImp(Render, collectionName),
  Polyhedron(NULL)
{
  SetGroupable(true);
  if( GetEllipsoid() != NULL )
    FDrawStyle = adsEllipsoid;
  else
    FDrawStyle = adsSphere;
  FRadius = darIsot;
  Params().Resize(2);
  Params()[0] = 1;
  Params()[1] = 1;
  // the objects will be automatically deleted by the corresponding action collections
  Label = new TXGlLabel(Render, PLabelsCollectionName);
  Label->SetOffset(crd());
  Label->SetVisible(false);
  if( OnStylesClear == NULL )  {
    OnStylesClear = new TStylesClear(Render);
    new TContextClear(Render);
  }
}
//..............................................................................
TXAtom::~TXAtom()  {
  if( GetParentGroup() != NULL )  {
    GetParentGroup()->Remove(*this);
#ifdef _DEBUG
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
#endif
  }
  if( !FPrimitiveParams.IsEmpty() )
    FPrimitiveParams.Clear();
  if( OrtepSpheres != -1 )  {
    olx_gl::deleteLists(OrtepSpheres, 9);
    OrtepSpheres = -1;
  }
  if( Polyhedron != NULL )  {
    delete Polyhedron;
    Polyhedron = NULL;
  }
  delete Label;
}
//..............................................................................
void TXAtom::Quality(const short V)  {
  olxstr Legend("Atoms");
  TGraphicsStyle& GS = TGlRenderer::_GetStyles().NewStyle(Legend, true);

  olxstr &SphereQ   = GS.GetParam("SphereQ", EmptyString(), true);
  olxstr &RimQ = GS.GetParam("RimQ", EmptyString(), true);  // quality
  olxstr &DiskQ = GS.GetParam("DiskQ", EmptyString(), true);  // quality

  olxstr &RimR = GS.GetParam("RimR", EmptyString(), true);  // radius
  olxstr &RimW = GS.GetParam("RimW", EmptyString(), true);  // width

  //olxstr &DiskIR = GS.ParameterValue("DiskIR", EmptyString());  // inner radius for disks
  olxstr &DiskOR = GS.GetParam("DiskOR", EmptyString(), true);  // outer radius
  olxstr &DiskS = GS.GetParam("DiskS", EmptyString(), true);  // separation

  RimR = 1.02;
  DiskQ  = RimQ;
  DiskS  = RimW;

  switch( V )  {
    case qaHigh:
      SphereQ = 30;
      RimQ=30;
      DiskQ = 30;
      break;
    case qaMedium:
      SphereQ = 15;
      RimQ=15;
      DiskQ = 15;
      break;
    case qaLow:
      SphereQ = 5;
      RimQ=5;
      DiskQ = 5;
      break;
    case qaPict:
      SphereQ = 120;
      RimQ = 120;
      DiskQ = 120;
      RimR = 1.0005;
      break;
  }
  DiskOR = RimR;
  return;
}
//..............................................................................
void TXAtom::ListPrimitives(TStrList &List) const {
  for( size_t i=0; i < FStaticObjects.Count(); i++ )
    List.Add(FStaticObjects[i]);
  return;
}
//..............................................................................
TStrList* TXAtom::FindPrimitiveParams(TGlPrimitive *P)  {
  for( size_t i=0; i < FPrimitiveParams.Count(); i++ )  {
    if( FPrimitiveParams[i].GlP == P )  
      return &(FPrimitiveParams[i].Params);
  }
  return NULL;
}
//..............................................................................
void TXAtom::ListParams(TStrList &List, TGlPrimitive *P)  {
  TStrList *L = FindPrimitiveParams(P);
  if( L != NULL )  
    List.Assign(*L);
}
//..............................................................................
void TXAtom::ListParams(TStrList &List)  {
  List.Add("Radius");
  List.Add("Color");
}
//..............................................................................
void TXAtom::CalcRad(short DefRadius)  {
  /*  remember the value in the style */
  //DefRad(DefRadius);
  FRadius = DefRadius;
  GetPrimitives().GetStyle().SetParam("DR", DefRadius);

  if( DefRadius == darPers )  
    FParams[0] = GetType().r_pers;   
  else if( DefRadius == darPack )  
    FParams[0] = GetType().r_sfil;
  else if( DefRadius == darVdW )  
    FParams[0] = GetType().r_vdw;
  else if( DefRadius == darBond )  
    FParams[0] = sqrt(caDefIso)/2;
  else if( DefRadius == darIsot )  {
    if( GetType() == iHydrogenZ ) 
      FParams[0] = 2*caDefIso;
    else  {
      if( GetType() == iQPeakZ )
        FParams[0] = sqrt(CAtom().GetUiso())*GetQPeakSizeScale();
      else if( CAtom().GetUiso() > 0 )
        FParams[0] = sqrt(CAtom().GetUiso());
      else
        FParams[0] = 2*caDefIso; //sqrt(caDefIso);
    }
  }
  else if( DefRadius == darIsotH )  {
    if( CAtom().GetUiso() > 0 )
      FParams[0] = sqrt(CAtom().GetUiso());
    else
      FParams[0] = 2*caDefIso; //sqrt(caDefIso);
  }
}
//..............................................................................
void TXAtom::ValidateRadius(TGraphicsStyle& GS)  {
  Params()[1] = GS.GetParam("Z", DefZoom()).ToDouble();
  short dr = GS.GetParam("DR", DefRad()).ToInt();
  CalcRad(dr);
}
void TXAtom::ValidateDS(TGraphicsStyle& GS)  {
  DrawStyle(GS.GetParam("DS", DefDS()).ToInt());
}
//..............................................................................
void TXAtom::Create(const olxstr& cName)  {
  olxstr Legend;
  if( !cName.IsEmpty() )  {
    SetCollectionName(cName);
    Legend = cName;
  }
  else  
    Legend = GetLegend(*this);

  TGPCollection *GPC = NULL;
  if( FStaticObjects.IsEmpty() )  
    CreateStaticObjects(Parent);
  Label->SetFontIndex(Parent.GetScene().FindFontIndexForType<TXAtom>());
  //Label->SetLabel(Atom().GetLabel());
  Label->Create();
  // find collection
  if( GetType() == iQPeakZ )  {
    Legend = GetLabelLegend(*this);
    GPC = Parent.FindCollection(Legend);
    if( GPC == NULL || GPC->PrimitiveCount() == 0 )  {  // if the collection is empty, need to fill it...
      if( GPC == NULL )
        GPC = &Parent.NewCollection(Legend);
      GPC->GetStyle().SetSaveable(false);
    }
    else  {
      GPC->AddObject(*this);
      ValidateRadius(GPC->GetStyle());
      return;
    }
  }
  else  {
    olxstr NewL;
    GPC = Parent.FindCollectionX(Legend, NewL);
    if( GPC == NULL )
      GPC = &Parent.NewCollection(NewL);
    else  {
      if( GPC->PrimitiveCount() != 0 )  {
        GPC->AddObject(*this);
        if( (GPC->GetStyle().GetParam(GetPrimitiveMaskName(), "0").ToInt() & (1 << PolyhedronIndex)) != 0 )
          CreatePolyhedron(true);
        ValidateRadius(GPC->GetStyle());
        ValidateDS(GPC->GetStyle());
        return;
      }
    }
  }

  TGraphicsStyle& GS = GPC->GetStyle();
  GS.SetSaveable(GPC->GetName().CharCount('.') == 0);

  olxstr& SMask = GS.GetParam(GetPrimitiveMaskName(), EmptyString());
  if( SMask.IsEmpty() )
    SMask = DefMask();
  int PMask = SMask.ToInt();

  GPC->AddObject(*this);
  if( PMask == 0 )  
    return; // nothing to create then...
  // update primitives list
  ValidateDS(GS);
  ValidateRadius(GS);

  TGlMaterial RGlM;
  for( size_t i=0; i < FStaticObjects.Count(); i++ )  {
    const int off = 1 << i;
    if( PMask & off )  {
      TGlPrimitive* SGlP = FStaticObjects.GetObject(i);
      TGlPrimitive& GlP = GPC->NewPrimitive(FStaticObjects.GetString(i), sgloCommandList);
      if( i == SphereIndex )
        GlP.SetOwnerId(xatom_SphereId);
      else if( i == SmallSphereIndex )
        GlP.SetOwnerId(xatom_SmallSphereId);
      else if( i == RimsIndex )
        GlP.SetOwnerId(xatom_RimsId);
      else if( i == DisksIndex )
        GlP.SetOwnerId( xatom_DisksId );
      else if( i == CrossIndex )
        GlP.SetOwnerId(xatom_CrossId);
      else if( i == PolyhedronIndex )  {
        GlP.SetOwnerId(xatom_PolyId);
        CreatePolyhedron(true);
      }
      else
        GlP.SetOwnerId(0);
      /* copy the default drawing style tag*/
      GlP.Params.Resize(GlP.Params.Count()+1);
      GlP.Params.GetLast() = SGlP->Params.GetLast();

      GlP.StartList();
      GlP.CallList(SGlP);
      GlP.EndList();

      if( GetType() == iQPeakZ )  {
        GetDefSphereMaterial(*this, RGlM);
        GlP.SetProperties(RGlM);
      }
      else  {
        const size_t mi = GS.IndexOfMaterial(FStaticObjects.GetString(i));
        if( mi != InvalidIndex )
          GlP.SetProperties(GS.GetPrimitiveStyle(mi).GetProperties());
        else if( SGlP->Params.GetLast() == ddsDefSphere )   {
          const size_t lmi = GS.IndexOfMaterial("Sphere");
          if( lmi != InvalidIndex )
            RGlM = GS.GetPrimitiveStyle(lmi).GetProperties();
          else
            GetDefSphereMaterial(*this, RGlM);
        }
        else if( SGlP->Params.GetLast() == ddsDefRim )  {
          const size_t lmi = GS.IndexOfMaterial("Rims");
          if( lmi != InvalidIndex )
            RGlM = GS.GetPrimitiveStyle(lmi).GetProperties();
          else
            GetDefRimMaterial(*this, RGlM);
        }
        GlP.SetProperties(GS.GetMaterial(FStaticObjects[i], RGlM));
      }
    }
  }
}
//..............................................................................
olxstr TXAtom::GetLabelLegend(const TSAtom& A)  {
  olxstr L = A.GetType().symbol;
  L << '.' << A.CAtom().GetLabel();
  return L;
}
//..............................................................................
olxstr TXAtom::GetLegend(const TSAtom& A, const short Level)  {
  if( A.GetType() == iQPeakZ )
    return GetLabelLegend(A);
  olxstr L = A.GetType().symbol;  
  if( Level == 0 )  return L;
  L << '.' << A.CAtom().GetLabel();
  if( Level == 1 )  return L;
  L << '.';
  L << TSymmParser::MatrixToSymmCode(
    A.GetNetwork().GetLattice().GetUnitCell().GetSymSpace(), A.GetMatrix(0));
  return L;
}
//..............................................................................
uint16_t TXAtom::LegendLevel(const olxstr& legend)  {
  return (uint16_t)legend.CharCount('.');
}
//..............................................................................
bool TXAtom::Orient(TGlPrimitive& GlP) {
  // override for standalone atoms
  if( FDrawStyle == adsStandalone && !IsStandalone() )
    return true;
 
  if( GlP.GetOwnerId() == xatom_PolyId )  {
    if( Polyhedron == NULL )  return true;
    olx_gl::begin(GL_TRIANGLES);
    const TXAtom::Poly& pl = *Polyhedron;
    for( size_t j=0; j < pl.faces.Count(); j++ )  {
      olx_gl::normal(pl.norms[j]);
      for( int k=0; k < 3; k++ )
        olx_gl::vertex(pl.vecs[pl.faces[j][k]]);
    }
    olx_gl::end();
    return true;
  }
  // override for iso atoms
  if( GetEllipsoid() == NULL && 
    (GlP.GetOwnerId() == xatom_DisksId || GlP.GetOwnerId() == xatom_RimsId) )  
  {
    return true;
  }

  vec3d c = GetCenter();
  c += crd();
  //const TExyzGroup* eg = FAtom.CAtom().GetExyzGroup();
  //if( eg != NULL )  {
  //  //if( &(*eg)[0] != &FAtom.CAtom() )  return true;
  //  if( eg->Count() == 2 )  {
  //    const mat3d& m = Parent.GetBasis().GetMatrix();
  //    vec3d v(m[0][0], m[1][0], m[2][0]);
  //    if( &(*eg)[0] == &FAtom.CAtom() )  
  //      c -= v*0.25;
  //    else
  //      c += v*0.25;
  //  }
  //}
  
  olx_gl::translate(c);

  double scale = FParams[1];
  if( (FRadius & (darIsot|darIsotH)) != 0 )
    scale *= TelpProb();
  
  if( FDrawStyle == adsEllipsoid || FDrawStyle == adsOrtep )  {
    if( GetEllipsoid() != NULL )  {
      // override for NPD atoms
      if( GetEllipsoid()->IsNPD() )  {
        olx_gl::scale(caDefIso*2*scale);
        if( GlP.GetOwnerId() == xatom_SphereId )  {
          FStaticObjects.GetObject(SmallSphereIndex)->Draw();
          return true;
        }
        if( GlP.GetOwnerId() == xatom_SmallSphereId )
          return true;
      }
      else  {
        olx_gl::orient(GetEllipsoid()->GetMatrix());
        olx_gl::scale(
          GetEllipsoid()->GetSX()*scale,
          GetEllipsoid()->GetSY()*scale,
          GetEllipsoid()->GetSZ()*scale
          );
        if( FDrawStyle == adsOrtep && GlP.GetOwnerId() == xatom_SphereId )  {
          short mask = 0;
          const mat3d mat = GetEllipsoid()->GetMatrix()*Parent.GetBasis().GetMatrix();
          for( int i=0; i < 3; i++ )  {
            if( mat[i][2] < 0 )
              mask |= (1<<i);
          }
          olx_gl::callList(OrtepSpheres+mask);
          return true;
        }
      }
    }
    else 
      olx_gl::scale(FParams[0]*scale);
    return false;
  }
  // override for standalone atoms
  if( FDrawStyle == adsStandalone )  {
    olx_gl::scale(FParams[0]*scale);
    return false;
  }
  
  if( FDrawStyle == adsSphere )
    olx_gl::scale(FParams[0]*scale);
  return false;
}
//..............................................................................
bool TXAtom::GetDimensions(vec3d& Max, vec3d& Min)  {
  const double dZ = GetType().r_sfil;
  Max = crd();
  Min = crd();
  Max += dZ;
  Min -= dZ;
  return true;
}
//..............................................................................
void TXAtom::GetDefSphereMaterial(const TSAtom& Atom, TGlMaterial& M)  {
  uint32_t Cl, Mask;
  Mask = RGBA(0x5f, 0x5f, 0x5f, 0x00);
  Cl = (int)Atom.GetType().def_color;
///////////
  if( Atom.GetType() == iQPeakZ )  {
    const double peak = Atom.CAtom().GetQPeak();
    // this is to tackle the shelxs86 output...
    if( olx_abs(Atom.CAtom().GetParent()->GetMaxQPeak() - Atom.CAtom().GetParent()->GetMinQPeak()) < 0.001 )  {
      M.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|sglmTransparent);
      M.DiffuseF = 0x00007f;
      M.AmbientF = 0x007f7f;
      M.SpecularF = 0xffffff;
      M.ShininessF = 36;
      M.AmbientF[3] = 0.5;
      M.DiffuseF[3] = M.AmbientF[3];
    }
    else  {
      if( peak > 0 )  {
        M.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|sglmTransparent);
        M.DiffuseF = 0x00007f;
        M.AmbientF = 0x007f7f;
        M.SpecularF = 0xffffff;
        M.ShininessF = 36;
        M.AmbientF[3] = (float)(atan(GetQPeakScale()*peak/Atom.CAtom().GetParent()->GetMaxQPeak())*2/M_PI);
        M.DiffuseF[3] = M.AmbientF[3];
      }
      else  {
        M.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|sglmTransparent);
        M.DiffuseF = 0x00007f;
        M.AmbientF = 0x7f007f;
        M.SpecularF = 0xffffff;
        M.ShininessF = 36;
        if( Atom.CAtom().GetParent()->GetMaxQPeak() < 0 )
          M.AmbientF[3] = (float)(atan(GetQPeakScale()*peak/Atom.CAtom().GetParent()->GetMinQPeak())*2/M_PI);
        else
          M.AmbientF[3] = (float)(atan(-GetQPeakScale()*peak/Atom.CAtom().GetParent()->GetMaxQPeak())*2/M_PI);
        M.DiffuseF[3] = M.AmbientF[3];
      }
    }
    return;
  }
//////////
  M.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF |
              sglmAmbientB|sglmDiffuseB|sglmSpecularB|sglmShininessB);
  M.AmbientF = Cl;
  M.DiffuseF =  Mask ^ Cl;
  M.SpecularF = 0xffffffff;
  M.ShininessF = 12;
  M.ShininessB = M.ShininessF;
  M.AmbientB = M.AmbientF;
  M.DiffuseB = M.DiffuseF;
  M.SpecularB = M.SpecularF;
}
//..............................................................................
void TXAtom::GetDefRimMaterial(const TSAtom& Atom, TGlMaterial &M)  {
  uint32_t Cl, Mask;
  Mask = RGBA(0x5f, 0x5f, 0x5f, 0x00);

  M.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|sglmEmissionF);
//  |  sglmAmbientB|sglmDiffuseB|sglmSpecularB|sglmShininessB|sglmEmissionB);
  Cl = Atom.GetType().def_color;
  M.AmbientF = Cl;
  M.DiffuseF =  Mask ^ Cl;
  M.SpecularF = 0xffffffff;
  M.EmissionF =  0x14141414;
  M.ShininessF = 12;
}
//..............................................................................
TGraphicsStyle& TXAtom::Style()  {
  return GetPrimitives().GetStyle();
}
//..............................................................................
void TXAtom::ApplyStyle(TGraphicsStyle& Style)  {
  for( size_t i=0; i < Style.PrimitiveStyleCount(); i++ )  {
    TGlPrimitive* GP = GetPrimitives().FindPrimitiveByName( 
      Style.GetPrimitiveStyle(i).GetName() );
    if( GP != NULL )
      GP->SetProperties(Style.GetPrimitiveStyle(i).GetProperties());
  } 
}
//..............................................................................
void TXAtom::DrawStyle(short V)  {
  olxstr &DS = GetPrimitives().GetStyle().GetParam("DS", EmptyString());
  DS = V;
  FDrawStyle = V;
}
//..............................................................................
void TXAtom::UpdateStyle(TGraphicsStyle& Style) {
/*  TGraphicsStyle *GS=NULL;
  TGPCollection *GPC=NULL, *OGPC;
  TGDrawObject *GDO;
  olxstr Tmp;
  int i;
  GS = Render()->Styles()->FindStyle(Style);
  if( !GS )  return;  // uniq then
  GPC = Render()->Collection(GS.Label());

  OGPC = Atom->Primitives();
  for( i=0; i < GPC->ObjectCount(); i++ ) // copy the group
  { OGPC->Add(GPC->Object(i));  }
  for( i=0; i < GPC->ObjectCount(); i++ )
  { GPC->DeleteObject(i);  }  // delete the old group
  Render()->RemoveCollection(GPC);*/
}
//..............................................................................
void TXAtom::ListDrawingStyles(TStrList &List)  {
  List.Add("Sphere");
  List.Add("Ellipsoid");
  List.Add("NPD Ellipsoid");
}
void UpdateDrawingStyle(int Index) {  
  return; 
};
//..............................................................................
void TXAtom::UpdatePrimitiveParams(TGlPrimitive* GlP)  {
  size_t ind = FStaticObjects.IndexOfObject(GlP);
  if( ind == InvalidIndex )
    throw TInvalidArgumentException(__OlxSourceInfo, "undefined primitive");

  olxstr Legend("Atoms");
  TGraphicsStyle& GS= Parent.GetStyles().NewStyle(Legend, true);
  if( FStaticObjects[ind] == "Sphere" )
    GS.SetParam("SphereQ", GlP->Params[1], true);
  else if( FStaticObjects[ind] == "Small sphere" )
    GS.SetParam("SphereQ", GlP->Params[1], true);
  else if( FStaticObjects.GetString(ind) == "Rims" )  {
    GS.SetParam("RimR", GlP->Params[0], true);
    GS.SetParam("RimW", GlP->Params[1], true);
    GS.SetParam("RimQ", GlP->Params[2], true);
  }
  else if( FStaticObjects[ind] == "Disks" )  {
    GS.SetParam("DiskIR", GlP->Params[0], true);
    GS.SetParam("DiskOR", GlP->Params[1], true);
    GS.SetParam("DiskQ", GlP->Params[2], true);
    GS.SetParam("DiskS", GlP->Params[3], true);
  }
}
//..............................................................................
void TXAtom::CreateStaticObjects(TGlRenderer& Parent)  {
  TGlMaterial GlM;
  TGlPrimitiveParams *PParams;
  TGlPrimitive *GlP, *GlPRC1, *GlPRD1, *GlPRD2;
  olxstr Legend("Atoms");
  TGraphicsStyle& GS= Parent.GetStyles().NewStyle(Legend, true);
  double SphereQ   = GS.GetParam("SphereQ", "15", true).ToDouble();
  double RimR = GS.GetParam("RimR", "1.02", true).ToDouble();  // radius
  double RimW = GS.GetParam("RimW", "0.05", true).ToDouble();  // width
  double RimQ = GS.GetParam("RimQ", SphereQ, true).ToDouble();  // quality

  double DiskIR = GS.GetParam("DiskIR", "0", true).ToDouble();  // inner radius for disks
  double DiskOR = GS.GetParam("DiskOR", RimR, true).ToDouble();  // outer radius
  double DiskQ = GS.GetParam("DiskQ", RimQ, true).ToDouble();  // quality
  double DiskS = GS.GetParam("DiskS", RimW, true).ToDouble();  // separation

//..............................
  // create sphere
  if( (GlP = FStaticObjects.FindObject("Sphere")) == NULL )
    FStaticObjects.Add("Sphere", GlP = &Parent.NewPrimitive(sgloSphere));
  GlP->Params[0] = 1;  GlP->Params[1] = SphereQ; GlP->Params[2] = SphereQ;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count()+1);
  GlP->Params.GetLast() = ddsDefSphere;
//..............................
  // create a small sphere
  if( (GlP = FStaticObjects.FindObject("Small sphere")) == NULL )
    FStaticObjects.Add("Small sphere", GlP = &Parent.NewPrimitive(sgloSphere));
  GlP->Params[0] = 0.5;  GlP->Params[1] = SphereQ; GlP->Params[2] = SphereQ;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count()+1);
  GlP->Params.GetLast() = ddsDefSphere;
//..............................
  // create simple rims
  GlPRC1 = &Parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = RimR;  GlPRC1->Params[1] = RimR;  GlPRC1->Params[2] = RimW;
    GlPRC1->Params[3] = RimQ; GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  if( (GlP = FStaticObjects.FindObject("Rims")) == NULL )
    FStaticObjects.Add("Rims", GlP = &Parent.NewPrimitive(sgloCommandList));
  GlP->StartList();
  GlP->CallList(GlPRC1);
  olx_gl::rotate(90, 1, 0, 0);
  GlP->CallList(GlPRC1);
  olx_gl::rotate(90, 0, 1, 0);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(3+1);  // radius, height, quality
  GlP->Params[0] = RimR;
  GlP->Params[1] = RimW;
  GlP->Params[2] = RimQ;
  GlP->Params[3] = ddsDefRim;
  PParams = new TGlPrimitiveParams;
  PParams->Params.Add("Radius");
  PParams->Params.Add("Width");
  PParams->Params.Add("Slices");
  FPrimitiveParams.Add(PParams);
//..............................
  // create disks
  GlPRD1 = &Parent.NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = DiskIR;  GlPRD1->Params[1] = DiskOR;
  GlPRD1->Params[2] = DiskQ;   GlPRD1->Params[3] = 1;
  GlPRD1->Compile();

  GlPRD2 = &Parent.NewPrimitive(sgloDisk);
  GlPRD2->SetQuadricOrientation(GLU_INSIDE);
  GlPRD2->Params[0] = DiskIR;  GlPRD1->Params[1] = DiskOR;
  GlPRD2->Params[2] = DiskQ;   GlPRD1->Params[3] = 1;
  GlPRD2->Compile();

  if( (GlP = FStaticObjects.FindObject("Disks")) == NULL )
    FStaticObjects.Add("Disks", GlP = &Parent.NewPrimitive(sgloCommandList));
  GlP->StartList();
  GlP->CallList(GlPRD2);
  olx_gl::translate(0.0, 0.0, DiskS);    GlP->CallList(GlPRD1);
  olx_gl::translate(0.0, 0.0, -DiskS);    
  olx_gl::rotate(90, 1, 0, 0);
  GlP->CallList(GlPRD2);
  olx_gl::translate(0.0, 0.0, DiskS);    GlP->CallList(GlPRD1);
  olx_gl::translate(0.0, 0.0, -DiskS);    
  olx_gl::rotate(90, 0, 1, 0);
  GlP->CallList(GlPRD2);
  olx_gl::translate(0.0, 0.0, DiskS);    GlP->CallList(GlPRD1);
  GlP->EndList();
  GlP->Params.Resize(4+1);  // inner radius, outer radius, Quality, offset
  GlP->Params[0] = DiskIR;
  GlP->Params[1] = DiskOR;
  GlP->Params[2] = DiskQ;
  GlP->Params[3] = DiskS;
  GlP->Params[4] = ddsDefRim;
  PParams = new TGlPrimitiveParams;
  PParams->Params.Add("Inner radius");
  PParams->Params.Add("Outer radius");
  PParams->Params.Add("Slices");
  PParams->Params.Add("Separation");
  FPrimitiveParams.Add(PParams);
//..............................
  // create cross
  if( (GlP = FStaticObjects.FindObject("Cross")) == NULL )
    FStaticObjects.Add("Cross", GlP = &Parent.NewPrimitive(sgloLines));
  GlP->Vertices.SetCount(6);
  GlP->Vertices[0][0] = -1; 
  GlP->Vertices[1][0] =  1; 
  GlP->Vertices[2][1] = -1; 
  GlP->Vertices[3][1] =  1; 
  GlP->Vertices[4][2] = -1; 
  GlP->Vertices[5][2] =  1; 
  GlP->Params[0] = 1.0;
  GlP->Params.Resize(GlP->Params.Count()+1);
  GlP->Params.GetLast() = ddsDefSphere;
//..............................
  // polyhedron - dummy
  if( (FStaticObjects.FindObject("Polyhedron")) == NULL )
    FStaticObjects.Add("Polyhedron", GlP = &Parent.NewPrimitive(sgloMacro));
  GlP->Params.Resize(GlP->Params.Count()+1);
  GlP->Params.GetLast() = ddsDefSphere;
// init indexes after all are added
  PolyhedronIndex = (uint8_t)FStaticObjects.IndexOf("Polyhedron");
  SphereIndex = (uint8_t)FStaticObjects.IndexOf("Sphere");
  SmallSphereIndex = (uint8_t)FStaticObjects.IndexOf("Small sphere");
  RimsIndex = (uint8_t)FStaticObjects.IndexOf("Rims");
  DisksIndex = (uint8_t)FStaticObjects.IndexOf("Disks");
  CrossIndex = (uint8_t)FStaticObjects.IndexOf("Cross");
//..............................
  TTypeList<vec3f> vecs;
  TTypeList<IndexTriangle> triags;
  TArrayList<vec3f> norms;
  typedef GlSphereEx<float, OctahedronFP<vec3f> > gls;
  gls::Generate(1, olx_round(log(SphereQ)+0.5), vecs, triags, norms);
  if( OrtepSpheres == -1 )
    OrtepSpheres = olx_gl::genLists(9);
  
  olx_gl::newList(OrtepSpheres, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(0,0,0), vec3f(1,1,1));
  olx_gl::endList();  
  olx_gl::newList(OrtepSpheres+1, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(-1,0,0), vec3f(0,1,1));
  olx_gl::endList();  
  olx_gl::newList(OrtepSpheres+2, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(0,-1,0), vec3f(1,0,1));
  olx_gl::endList();  
  olx_gl::newList(OrtepSpheres+3, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(-1,-1,0), vec3f(0,0,1));
  olx_gl::endList();  
  olx_gl::newList(OrtepSpheres+4, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(0,0,-1), vec3f(1,1,0));
  olx_gl::endList();  
  olx_gl::newList(OrtepSpheres+5, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(-1,0,-1), vec3f(0,1,0));
  olx_gl::endList();  
  olx_gl::newList(OrtepSpheres+6, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(0,-1,-1), vec3f(1,0,0));
  olx_gl::endList();  
  olx_gl::newList(OrtepSpheres+7, GL_COMPILE);
  gls::RenderEx(vecs, triags, norms, vec3f(-1,-1,-1), vec3f(0,0,0));
  olx_gl::endList();  
  olx_gl::newList(OrtepSpheres+8, GL_COMPILE);
  gls::Render(vecs, triags, norms);
  olx_gl::endList();  
}
//..............................................................................
void TXAtom::UpdatePrimitives(int32_t Mask)  {
  AGDrawObject::UpdatePrimitives(Mask);
  bool create_polyhedron = (Mask & (1 << PolyhedronIndex)) != 0;
  for( size_t i=0; i < GetPrimitives().ObjectCount(); i++ ) {
    if( EsdlInstanceOf(GetPrimitives().GetObject(i), TXAtom) )
      ((TXAtom&)GetPrimitives().GetObject(i)).CreatePolyhedron(create_polyhedron);
  }
}
//..............................................................................
//float TXAtom::Radius(){  return FParams()[0]; }
//..............................................................................
void TXAtom::ValidateAtomParams() {
  if( FAtomParams == NULL )  {
    FAtomParams = &TGlRenderer::_GetStyles().NewStyle("AtomParams", true);
    FAtomParams->SetPersistent(true);
  }
}
//..............................................................................
void TXAtom::SetZoom(double V)  {
  GetPrimitives().GetStyle().SetParam("Z", V);
  Params()[1] = V;
  // update radius for all members of the collection
  for( size_t i=0; i < GetPrimitives().ObjectCount(); i++ )
    GetPrimitives().GetObject(i).Params()[1] = V;
}
//..............................................................................
uint32_t TXAtom::GetPrimitiveMask() const {
  return GetPrimitives().GetStyle().GetParam(GetPrimitiveMaskName(), "0").ToUInt();
}
//..............................................................................
void TXAtom::OnPrimitivesCleared()  {
  if( FStaticObjects.Count() != 0 )
    FStaticObjects.Clear();
}
//..............................................................................
void TXAtom::DefRad(short V)  {
  if( V != 0 )  {
    ValidateAtomParams();
    FAtomParams->SetParam("DefR", V, true);
  }
  FDefRad = V;
}
//..............................................................................
void TXAtom::DefDS(short V)  {
  if( V != 0 )  {
    ValidateAtomParams();
    FAtomParams->SetParam("DefDS", V, true);
  }
  FDefDS = V;
}
//..............................................................................
void TXAtom::DefMask(int V)  {
  ValidateAtomParams();
  FAtomParams->SetParam("DefMask", V, true);
}
//..............................................................................
void TXAtom::DefZoom(float V)  {
  ValidateAtomParams();
  FAtomParams->SetParam("DefZ", V, true);
}
//..............................................................................
void TXAtom::TelpProb(float V)  {
  if( V != 0 )  {
    ValidateAtomParams();
    FAtomParams->SetParam("TelpP", V, true);
  }
  FTelpProb = V;
}
//..............................................................................
short TXAtom::DefRad()  {
  if( FDefRad != 0 )  
    return FDefRad;
  ValidateAtomParams();
  FDefRad = FAtomParams->GetParam("DefR", darPers, true).ToInt();
  return FDefRad;
}
//..............................................................................
short TXAtom::DefDS()  {
  if( FDefDS != 0 )  
    return FDefDS;
  ValidateAtomParams();
  FDefDS = FAtomParams->GetParam("DefDS", adsSphere, true).ToInt();
  return FDefDS;
}
//..............................................................................
int TXAtom::DefMask()  {
  ValidateAtomParams();
  return FAtomParams->GetParam("DefMask", "5", true).ToInt();
}
//..............................................................................
float TXAtom::TelpProb()  {
  if( FTelpProb != 0 )  
    return FTelpProb;
  ValidateAtomParams();
  FTelpProb = (float)FAtomParams->GetParam("TelpP", "1", true).ToDouble();
  return FTelpProb;
}
//..............................................................................
float TXAtom::DefZoom()  {
  ValidateAtomParams();
  return (float)FAtomParams->GetParam("DefZ", "1", true).ToDouble();
}
//..............................................................................
float TXAtom::GetQPeakScale()  {
  if( FQPeakScale != 0 )  return FQPeakScale;
  ValidateAtomParams();
  return (float)FAtomParams->GetParam("QPeakScale", "3", true).ToDouble();
}
//..............................................................................
void TXAtom::SetQPeakScale(float V)  {
  ValidateAtomParams();
  if( V < 1 )  V = 3;
  FAtomParams->SetParam("QPeakScale", V, true);
  FQPeakScale = V;
}
//..............................................................................
float TXAtom::GetQPeakSizeScale()  {
  if( FQPeakSizeScale != 0 )  return FQPeakSizeScale;
  ValidateAtomParams();
  return (float)FAtomParams->GetParam("QPeakSizeScale", "1", true).ToDouble();
}
//..............................................................................
void TXAtom::SetQPeakSizeScale(float V)  {
  ValidateAtomParams();
  if( V < 0 )  V = 1;
  FAtomParams->SetParam("QPeakSizeScale", V, true);
  FQPeakSizeScale = V;
}
//..............................................................................
bool TXAtom::OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {
  if( !IsMoveable() )  return Label->IsVisible() ? Label->OnMouseDown(Sender, Data) : false;
  return AGlMouseHandlerImp::OnMouseDown(Sender, Data);
}
//..............................................................................
bool TXAtom::OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {
  if( !IsMoveable() )  return Label->IsVisible() ? Label->OnMouseUp(Sender, Data) : false;
  return AGlMouseHandlerImp::OnMouseUp(Sender, Data);
}
//..............................................................................
bool TXAtom::OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {
  if( !IsMoveable() )  return Label->IsVisible() ? Label->OnMouseMove(Sender, Data) : false;
  return AGlMouseHandlerImp::OnMouseMove(Sender, Data);
}
//..............................................................................
void TXAtom::CreateNormals(TXAtom::Poly& pl, const vec3f& cnt)  {
  const size_t off = pl.norms.Count();
  pl.norms.SetCapacity(pl.faces.Count());
  for( size_t i=off; i < pl.faces.Count(); i++ )  {
    const vec3f& v1 = pl.vecs[pl.faces[i][0]];
    const vec3f& v2 = pl.vecs[pl.faces[i][1]];
    const vec3f& v3 = pl.vecs[pl.faces[i][2]];
    vec3f n = (v2-v1).XProdVec(v3-v1);
    if( n.QLength() < 1e-15 )  {  // oups...
      pl.faces.Clear();
      pl.vecs.Clear();
      pl.norms.Clear();
      return;
    }
    const float d = n.DotProd((v1+v2+v3)/3)/n.Length();
    n.Normalise();
    if( (n.DotProd(cnt) - d) > 0 )  { // normal looks inside?
      olx_swap(pl.faces[i][0], pl.faces[i][1]);
      n *= -1;
    }
    pl.norms.AddCCopy(n);
  }
}
vec3f TXAtom::TriangulateType2(Poly& pl, const TSAtomPList& atoms)  {
  TSPlane plane(NULL);
  TTypeList< AnAssociation2<TSAtom*, double> > pa;
  vec3f cnt;
  double wght = 0;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    pa.AddNew( atoms[i], atoms[i]->CAtom().GetOccu());
    cnt += atoms[i]->crd()*atoms[i]->CAtom().GetOccu();
    wght += atoms[i]->CAtom().GetOccu();
  }
  cnt += crd();
  wght += CAtom().GetOccu();
  cnt /= (float)wght;
  plane.Init(pa);
  plane.GetCenter();
  // this might fail if one of the atoms is at the center 
  PlaneSort::Sorter sp;
  try  {  sp.DoSort(plane);  }
  catch( ... )  {
    return cnt;
  }
  const size_t start = pl.vecs.Count();
  pl.vecs.SetCount(pl.vecs.Count()+atoms.Count()+2);
  pl.vecs[start] = crd();
  pl.vecs[start+1] = plane.GetCenter();
  for( size_t i=0; i < sp.sortedPlane.Count(); i++ )
    pl.vecs[start+i+2] = *sp.sortedPlane.GetObject(i);
  // create the base
  for( size_t i=0; i < sp.sortedPlane.Count()-1; i++ )
    pl.faces.AddNew(start+1, start+i+2, start+i+3);
  pl.faces.AddNew(start+1, start+2, start+sp.sortedPlane.Count()+1);
  // and the rest to close the polyhedron
  for( size_t i=0; i < sp.sortedPlane.Count()-1; i++ )
    pl.faces.AddNew(start, start+i+2, start+i+3);
  pl.faces.AddNew(start, start+2, start+sp.sortedPlane.Count()+1);
  return cnt;
}
//..............................................................................
void TXAtom::CreatePoly(const TSAtomPList& bound, short type, const vec3d* _normal, const vec3d* _pc)  {
  try  {
    static const vec3d NullVec;
    TXAtom::Poly& pl = *((Polyhedron == NULL) ? (Polyhedron=new TXAtom::Poly) : Polyhedron);
    if( type == polyRegular )  {
      pl.vecs.SetCount(bound.Count());
      for( size_t i=0; i < bound.Count(); i++ )  {
        pl.vecs[i] = bound[i]->crd();
        for( size_t j=i+1; j < bound.Count(); j++ )  {
          for( size_t k=j+1; k < bound.Count(); k++ )  {
            if( olx_tetrahedron_volume(
              NullVec, 
              (bound[i]->crd()-crd()).Normalise(), 
              (bound[j]->crd()-crd()).Normalise(), 
              (bound[k]->crd()-crd()).Normalise() ) > 0.03 )  // reagular octahedron vol would be ~0.47A^3
              pl.faces.AddNew(i, j, k);
          }
        }
      }
      CreateNormals(pl, crd());
    }
    else if( type == polyPyramid )  {
      vec3f cnt = TriangulateType2(pl, bound);
      CreateNormals(pl, cnt);
    }
    else if( type == polyBipyramid )  {
      vec3d normal, pc;
      if( _normal == NULL || _pc == NULL )  {
        vec3d rms;
        mat3d normals;
        TSPlane::CalcPlanes(bound, normals, rms, pc);
        normal = normals[2];
      }
      else  {
        normal = *_normal;
        pc = *_pc;
      }
      TSAtomPList sidea, sideb;
      pl.vecs.Clear();
      for( size_t i=0; i < bound.Count(); i++ )  {
        const double ca = normal.CAngle(bound[i]->crd() - pc);
        if( ca >= 0 )
          sidea.Add(bound[i]);
        else
          sideb.Add(bound[i]);
      }
      if( sidea.Count() > 2 )  {
        vec3f cnt = TriangulateType2(pl, sidea);
        CreateNormals(pl, cnt);
      }
      if( sideb.Count() > 2 )  {
        vec3f cnt = TriangulateType2(pl, sideb);
        CreateNormals(pl, cnt);
      }
    }
  }
  catch(...)  {
    if( Polyhedron != NULL )
      delete Polyhedron;
    Polyhedron = NULL;
  }
}
//..............................................................................
void TXAtom::CreatePolyhedron(bool v)  {
  if( Polyhedron != NULL )  {
    delete Polyhedron;
    Polyhedron = NULL;
  }
  if( !v )  return;
  if( NodeCount() < 4 )  return;
  if( IsDeleted() || GetType() == iQPeakZ )
    return;
  TPtrList<TSAtom> bound;
  for( size_t i=0; i < NodeCount(); i++ )  {
    if( Node(i).IsDeleted() || Node(i).GetType().GetMr() < 3.5  )
      continue;
    //if( FAtom.Node(i).NodeCount() > FAtom.NodeCount() )
    //  continue;
    bound.Add(Node(i));
  }
  if( bound.Count() < 4 )  return;
  int type = GetPrimitives().GetStyle().GetParam(PolyTypeName, "0", true).ToInt();
  if( type != polyAuto && type != polyNone )  {
    CreatePoly(bound, type);
    return;
  }

  TXAtom::Poly& pl = *(Polyhedron = new TXAtom::Poly);
  vec3f sv;
  double sd = 0;
  for( size_t i=0; i < bound.Count(); i++ )  {
    vec3d nd = bound[i]->crd() - crd();
    sd += nd.Length();
    sv += nd.Normalise();
  }
  sv /= bound.Count();
  sd /= bound.Count();
  double ratio = sv.Length()/sd;
  if( ratio < 0.2 )  {  // atom is inside
    // test for Cp-kind polyhedron, should be drawn as two of the other kind (atom outside)
    vec3d pc, rms;
    mat3d normals;
    TSPlane::CalcPlanes(bound, normals, rms, pc);
    const double pd = normals[0].DotProd(pc)/normals[0].Length();
    const double pd_x = normals[2].DotProd(pc)/normals[2].Length();
    normals.Normalise();
    int deviating = 0, deviating_x = 0;
    for( size_t i=0; i < bound.Count(); i++ )  {
      if( olx_abs(bound[i]->crd().DotProd(normals[0]) - pd) > rms[0] )
        deviating++;
      if( olx_abs(bound[i]->crd().DotProd(normals[2]) - pd_x) > rms[2] )
        deviating_x++;
    }
    if( deviating < 3 || deviating_x < 3 )  // a proper polyhedra
      CreatePoly(bound, polyRegular);
    else  // two polyhedra of atom outside..
      CreatePoly(bound, polyBipyramid, &normals[2], &pc);
  }
  else  // atom outside
    CreatePoly(bound, polyPyramid);
}
//..............................................................................
void TXAtom::SetPolyhedronType(short type)  {
  olxstr& str_type = GetPrimitives().GetStyle().GetParam(PolyTypeName, "0", true);
  int int_type = str_type.ToInt();
  int int_mask = GetPrimitives().GetStyle().GetParam(GetPrimitiveMaskName(), "0").ToInt();
  if( type == polyNone )  {
    if( (int_mask & (1 << PolyhedronIndex)) != 0 )
      UpdatePrimitives(int_mask & ~(1 << PolyhedronIndex) );
  }
  else  {
    if( int_type != type || (int_mask & (1 << PolyhedronIndex)) == 0 )  {
      str_type = type;
      if( (int_mask & (1 << PolyhedronIndex)) == 0 )
        UpdatePrimitives(int_mask | (1 << PolyhedronIndex) );
      else  {
        GetPrimitives().ClearPrimitives();
        GetPrimitives().RemoveObject(*this);
        Create();
        for( size_t i=0; i < GetPrimitives().ObjectCount(); i++ ) {
          if( EsdlInstanceOf(GetPrimitives().GetObject(i), TXAtom) )
            ((TXAtom&)GetPrimitives().GetObject(i)).CreatePolyhedron(true);
        }
      }
    }
  }
}
//..............................................................................
int TXAtom::GetPolyhedronType()  {
  int int_mask = GetPrimitives().GetStyle().GetParam(GetPrimitiveMaskName(), "0").ToInt();
  return (int_mask & (1 << PolyhedronIndex)) == 0 ? polyNone :
    GetPrimitives().GetStyle().GetParam(PolyTypeName, "0", true).ToInt();
}
//..............................................................................
