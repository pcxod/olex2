//----------------------------------------------------------------------------//
// namespace TEXLib
// TXAtom  - a drawing object for atom
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

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

#include "glgroup.h"
#include "exyzgroup.h"

//..............................................................................
bool TXAtomStylesClear::Enter(const IEObject *Sender, const IEObject *Data)  {  
  TXAtom::FAtomParams = NULL; 
  return true; 
}
//..............................................................................
bool TXAtomStylesClear::Exit(const IEObject *Sender, const IEObject *Data)
{  TXAtom::ValidateAtomParams(); return true; }
//..............................................................................
//----------------------------------------------------------------------------//
// TSAtom function bodies
//----------------------------------------------------------------------------//
TStrPObjList<olxstr,TGlPrimitive*> TXAtom::FStaticObjects;
TTypeList<TGlPrimitiveParams> TXAtom::FPrimitiveParams;
float  TXAtom::FTelpProb = 0;
float  TXAtom::FQPeakScale = 0;
TGraphicsStyle* TXAtom::FAtomParams=NULL;
TXAtomStylesClear *TXAtom::FXAtomStylesClear=NULL;
//..............................................................................

TXAtom::TXAtom(const olxstr& collectionName, TSAtom& A, TGlRender *Render) :
  //AGDrawObject(collectionName)
  TGlMouseListener(collectionName, Render)
{
  FParent = Render;
  XAppId = -1;
  FAtom = &A;
  
  Move2D(false);
  Moveable(false);
  Zoomable(false);

  FDrawStyle = adsSphere;
  Params().Resize(2);
  Params()[0] = 1;
  Params()[1] = 1;
  if( FStaticObjects.IsEmpty() )  CreateStaticPrimitives();
  // the objects will be automatically deleted by the corresponding action collections
}
//..............................................................................
void TXAtom::Quality(const short V)  {
  olxstr Legend("Atoms");
  TGraphicsStyle *GS;
  GS = FParent->Styles()->Style(Legend);
  if( GS == NULL ) 
    GS = FParent->Styles()->NewStyle(Legend);

  olxstr &SphereQ   = GS->ParameterValue("SphereQ", EmptyString);
  olxstr &RimQ = GS->ParameterValue("RimQ", EmptyString);  // quality
  olxstr &DiskQ = GS->ParameterValue("DiskQ", EmptyString);  // quality

  olxstr &RimR = GS->ParameterValue("RimR", EmptyString);  // radius
  olxstr &RimW = GS->ParameterValue("RimW", EmptyString);  // width

  //olxstr &DiskIR = GS->ParameterValue("DiskIR", EmptyString);  // inner radius for disks
  olxstr &DiskOR = GS->ParameterValue("DiskOR", EmptyString);  // outer radius
  olxstr &DiskS = GS->ParameterValue("DiskS", EmptyString);  // separation

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
//  CreateStaticPrimitives(false);
  return;
}
//..............................................................................
void TXAtom::ListPrimitives(TStrList &List) const {
  List.Assign(FStaticObjects);
  return;
}
//..............................................................................
TStrList* TXAtom::FindPrimitiveParams(TGlPrimitive *P)  {
  for( int i=0; i < FPrimitiveParams.Count(); i++ )  {
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

  if( DefRadius == darPers )  
    Params()[0] = FAtom->GetAtomInfo().GetRad();   
  else if( DefRadius == darPack )  
    Params()[0] = FAtom->GetAtomInfo().GetRad2();
  else if( DefRadius == darBond )  
    Params()[0] = sqrt(caDefIso)/2;
  else if( DefRadius == darIsot )  {
    if( FAtom->GetAtomInfo() == iHydrogenIndex ) 
      Params()[0] = 2*caDefIso;
    else  {
      if( FAtom->CAtom().GetUiso() > 0 )
        Params()[0] = sqrt(FAtom->CAtom().GetUiso());
      else
        Params()[0] = 2*caDefIso; //sqrt(caDefIso);
    }
  }
  else if( DefRadius == darIsotH )  {
    if( FAtom->CAtom().GetUiso() > 0 )
      Params()[0] = sqrt(FAtom->CAtom().GetUiso());
    else
      Params()[0] = 2*caDefIso; //sqrt(caDefIso);
  }
}
//..............................................................................
void TXAtom::ValidateRadius(TGraphicsStyle *GS)  {
  Params()[1] = GS->ParameterValue("Z", DefZoom()).ToDouble();
  short DefRadius = DefRad();
  CalcRad(DefRadius);
}
void TXAtom::ValidateDS(TGraphicsStyle *GS)  {
  DrawStyle( GS->ParameterValue("DS", DefDS()).ToInt() );
}
//..............................................................................
void TXAtom::Create(const olxstr& cName, const ACreationParams* cpar)  {
  olxstr Legend, NewL;

  if( !cName.IsEmpty() )  {
    SetCollectionName(cName);
    Legend = cName;
  }
  else  
    Legend = GetLegend( *FAtom );

  TGlMaterial RGlM;
  TGraphicsStyle *GS = NULL;
  TGPCollection *GPC = NULL;
  int off;
  if( FStaticObjects.IsEmpty() )  
    CreateStaticPrimitives();

  // find collection
  if( FAtom->GetAtomInfo() == iQPeakIndex )  {
    Legend = GetLabelLegend(FAtom);
    GPC = FParent->FindCollection(Legend);
    if( GPC == NULL || GPC->ObjectCount() == 0 )  {  // if the collection is empty, need to fill it...
      GPC = FParent->NewCollection(Legend);
      GS = GPC->Style();
      GS->SetSaveable(false);
    }
    else  {
      ValidateRadius(GPC->Style());
      GPC->AddObject(this);
      return;
    }
  }
  else  {
    GPC = FParent->CollectionX(Legend, NewL);
    if( GPC == NULL )
      GPC = FParent->NewCollection(NewL);
    else  {
      if( GPC->PrimitiveCount() )  {
        GPC->AddObject(this);
        if( cpar == NULL )  {
          ValidateRadius( GPC->Style() );
          ValidateDS(GPC->Style());
        }
        else  {
          Params() = cpar->params;
        }
        return;
      }
    }
    GS = GPC->Style();
  }
  int PMask = 0;
  if( cpar == NULL )  {
    olxstr& SMask = GS->ParameterValue("PMask", EmptyString);
    if( SMask.IsEmpty() )  {
      if( FAtom->GetEllipsoid() != NULL )  {
        if( FAtom->GetEllipsoid()->IsNPD() )  {  SMask = DefNpdMask();  }
        else                                  {  SMask = DefElpMask();  }
      }
      else
        SMask = DefSphMask();
    }
    PMask = SMask.ToInt();
  }
  else
    PMask = ((AtomCreationParams*)cpar)->mask;
  GPC->AddObject(this);
  if( PMask == 0 )  
    return; // nothing to create then...
  // update primitives list
  if( cpar == NULL )  {
    ValidateDS(GS);
    ValidateRadius(GS);
  }
  else {
    Params() = cpar->params;
  }

  for( int i=0; i < FStaticObjects.Count(); i++ )  {
    off = 1;
    off = off << i;
    if( PMask & off )  {
      TGlPrimitive* SGlP = FStaticObjects.Object(i);
      TGlPrimitive* GlP = GPC->NewPrimitive(FStaticObjects.String(i));
      GlP->Type(sgloCommandList);
      /* copy the default drawing style tag*/
      GlP->Params().Resize(GlP->Params().Count()+1);
      GlP->Params().Last() = SGlP->Params().Last();

      GlP->StartList();
      GlP->CallList(SGlP);
      GlP->EndList();

      if( FAtom->GetAtomInfo() == iQPeakIndex )  {
        GetDefSphereMaterial(*FAtom, RGlM);
        GlP->SetProperties(&RGlM);
      }
      else  {
        TGlMaterial* GlM = const_cast<TGlMaterial*>(GS->Material(FStaticObjects.String(i)));
        if( GlM->Mark() )  {
          if( SGlP->Params().Last() == ddsDefSphere ) GetDefSphereMaterial(*FAtom, RGlM);
          if( SGlP->Params().Last() == ddsDefRim )    GetDefRimMaterial(*FAtom, RGlM);
          GS->PrimitiveMaterial(FStaticObjects.String(i), RGlM);
          GlP->SetProperties(&RGlM);
        }
        else
          GlP->SetProperties(GlM);
      }
    }
  }
  return; 
}
//..............................................................................
ACreationParams* TXAtom::GetCreationParams() const {
  TGPCollection* gp = Primitives();
  TGraphicsStyle* gs = gp->Style();
  AtomCreationParams& ap = *(new AtomCreationParams);
  ap.mask = Primitives()->Style()->ParameterValue("PMask", 0).ToInt();
  ap.params = FParams;
  const int soc = FStaticObjects.Count();
  int pc = 0;
  for( int i=0; i < soc; i++ )  {
    int off = 1;
    off = off << i;
    if( (ap.mask & off) != 0 )  {
      if( gp->PrimitiveCount() < pc )
        throw TFunctionFailedException(__OlxSourceInfo, "assert");
      ap.materials.Add( *((TGlMaterial*)gp->Primitive(pc)->GetProperties()) );
      pc++;
    }
  }
  return &ap;
}
//..............................................................................
TXAtom::~TXAtom()  {
  if( !FPrimitiveParams.IsEmpty() )
    FPrimitiveParams.Clear();
}
//..............................................................................
olxstr TXAtom::GetLabelLegend(TSAtom *A)  {
  olxstr L = A->GetAtomInfo().GetSymbol();
  L << '.' << A->CAtom().Label();
  return L;
}
//..............................................................................
olxstr TXAtom::GetLegend(const TSAtom& A, const short Level)  {
  olxstr L;
  L = A.GetAtomInfo().GetSymbol();  L << '_';
  if( A.GetEllipsoid() != NULL )
    L << (A.GetEllipsoid()->IsNPD() ? 'N' : 'E'); // not positevely defined
  else
    L << 'S';
  if( Level == 0 )  return L;
  L << '.' << A.CAtom().GetLabel();
  if( Level == 1 )  return L;
  L << '.';
  L << TSymmParser::MatrixToSymmCode(A.GetNetwork().GetLattice().GetUnitCell(), A.GetMatrix(0));
  return L;
}
//..............................................................................
short TXAtom::LegendLevel(const olxstr& legend)  {
  return legend.CharCount('.');
}
//..............................................................................
bool TXAtom::Orient(TGlPrimitive *GlP) {
//  static double qr = 2*caDefIso;
  vec3d c( Basis.GetCenter() );
  c += FAtom->crd();
  if( Roteable() )  {
    vec3d cr;
    int ac = 0;
    TGlGroup* gr = FParent->Selection();
    for( int i=0; i < gr->Count(); i++ )  {
      if( EsdlInstanceOf(*gr->Object(i), TXAtom) )  {
        cr += ((TXAtom*)gr->Object(i))->FAtom->crd();
        cr += ((TXAtom*)gr->Object(i))->Basis.GetCenter();
        ac ++;
      }
    }
    if( ac > 1 )  {
      cr /= ac;
      c -= cr;
      c *= Basis.GetMatrix();
      c += cr;
    }
  }
  //const TExyzGroup* eg = FAtom->CAtom().GetExyzGroup();
  //if( eg != NULL )  {
  //  //if( &(*eg)[0] != &FAtom->CAtom() )  return true;
  //  if( eg->Count() == 2 )  {
  //    const mat3d& m = FParent->GetBasis().GetMatrix();
  //    vec3d v(m[0][0], m[1][0], m[2][0]);
  //    if( &(*eg)[0] == &FAtom->CAtom() )  
  //      c -= v*0.25;
  //    else
  //      c += v*0.25;
  //  }
  //}
  
  FParent->GlTranslate(c);

//  if( FAtom->GetAtomInfo() == iHydrogenIndex )  {
//    float K = (float)(qr*FParams[1]*TelpProb());
//    Parent()->GlScale(K);
//    return false;
//  }
  if( (FDrawStyle & adsEllipsoid) != 0 )  {
    float TP = (float)(TelpProb()*FParams[1]);
    if( FAtom->GetEllipsoid() != NULL )  {
      if( FAtom->GetEllipsoid()->IsNPD() )  {
        FParent->GlScale(caDefIso*2*TP);
      }
      else  {
      FParent->GlOrient( FAtom->GetEllipsoid()->GetMatrix() );
      FParent->GlScale(
          (float)(FAtom->GetEllipsoid()->GetSX()*TP),
          (float)(FAtom->GetEllipsoid()->GetSY()*TP),
          (float)(FAtom->GetEllipsoid()->GetSZ()*TP)
          );
      }
    }
    return false;
  }
  if( (FDrawStyle & adsSphere) != 0 )  {
    float K = (float)(FParams[0]*FParams[1]*TelpProb());
    FParent->GlScale( K );
  }
  return false;
}
//..............................................................................
bool TXAtom::GetDimensions(vec3d &Max, vec3d &Min)  {
  double dZ = FAtom->GetAtomInfo().GetRad2();
  Max = FAtom->crd();
  Min = FAtom->crd();
  Max += dZ;
  Min -= dZ;
  return true;
}
//..............................................................................
void TXAtom::GetDefSphereMaterial(const TSAtom& Atom, TGlMaterial& M)  {
  int Cl, Mask;
  Mask = RGBA(0x5f, 0x5f, 0x5f, 0x00);
  Cl = (int)Atom.GetAtomInfo().GetDefColor();
///////////
  if( Atom.GetAtomInfo() == iQPeakIndex )  {
    double peak = Atom.CAtom().GetQPeak();
    if( peak > 0 )  {
      M.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|sglmTransparent);
      M.DiffuseF = 0x00007f;
      M.AmbientF = 0x007f7f;
      M.SpecularF = 0xffffff;
      M.ShininessF = 36;
      M.AmbientF[3] = (float)(atan(QPeakScale()*peak/Atom.CAtom().GetParent()->GetMaxQPeak())*2/M_PI);
      M.DiffuseF[3] = M.AmbientF[3];
    }
    else  {
      M.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|sglmTransparent);
      M.DiffuseF = 0x00007f;
      M.AmbientF = 0x7f007f;
      M.SpecularF = 0xffffff;
      M.ShininessF = 36;
      M.AmbientF[3] = (float)(atan(-QPeakScale()*peak/Atom.CAtom().GetParent()->GetMaxQPeak())*2/M_PI);
      M.DiffuseF[3] = M.AmbientF[3];
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
  M.Mark(false);
}
//..............................................................................
void TXAtom::GetDefRimMaterial(const TSAtom& Atom, TGlMaterial &M)  {
  int Cl, Mask;
  Mask = RGBA(0x5f, 0x5f, 0x5f, 0x00);

  M.SetFlags( sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmShininessF|sglmEmissionF);
//  |  sglmAmbientB|sglmDiffuseB|sglmSpecularB|sglmShininessB|sglmEmissionB);
  Cl = (int)Atom.GetAtomInfo().GetDefColor();

  M.AmbientF = Cl;
  M.DiffuseF =  Mask ^ Cl;
  M.SpecularF = 0xffffffff;
  M.EmissionF =  0x14141414;
  M.ShininessF = 12;
  M.Mark(false);
}
//..............................................................................
TGraphicsStyle* TXAtom::Style()  {
  return NULL;
}
//..............................................................................
void TXAtom::ApplyStyle(TGraphicsStyle *Style)  {
  for( int i=0; i < Style->PrimitiveStyleCount(); i++ )  {
    TGlPrimitive* GP = Primitives()->PrimitiveByName( Style->PrimitiveStyle(i)->PrimitiveName() );
    if( GP != NULL )
      GP->SetProperties(Style->PrimitiveStyle(i)->GetProperties());
  } 
}
//..............................................................................
void TXAtom::DrawStyle(short V)  {
  olxstr &DS = Primitives()->Style()->ParameterValue("DS", EmptyString);
  if( V == adsEllipsoid )  {
    if( FAtom->GetEllipsoid() != NULL )  {
      if( FAtom->GetEllipsoid()->IsNPD() )  {
        FDrawStyle = adsEllipsoidNPD;
        DS = adsEllipsoid;
      }
      else  {
        FDrawStyle = V;
        DS = V;
      }
    }
    return;
  }
  DS = V;
  FDrawStyle = V;
}
//..............................................................................
void TXAtom::UpdateStyle(TGraphicsStyle *Style) {
/*  TGraphicsStyle *GS=NULL;
  TGPCollection *GPC=NULL, *OGPC;
  TGDrawObject *GDO;
  olxstr Tmp;
  int i;
  GS = Render()->Styles()->FindStyle(Style);
  if( !GS )  return;  // uniq then
  GPC = Render()->Collection(GS->Label());

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
void TXAtom::UpdatePrimitiveParams(TGlPrimitive *GlP)  {
  int ind = FStaticObjects.IndexOfObject(GlP);

  if( ind == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "undefined primitive");

  olxstr Legend("Atoms");
  TGraphicsStyle *GS;
  GS = FParent->Styles()->Style(Legend);
  if( !GS ) GS = FParent->Styles()->NewStyle(Legend);
  if( FStaticObjects.String(ind) == "Sphere" )
    GS->SetParameter("SphereQ", GlP->Params()[1]);
  else if( FStaticObjects.String(ind) == "Small sphere" )
    GS->SetParameter("SphereQ", GlP->Params()[1]);
  else if( FStaticObjects.String(ind) == "Rims" )  {
    GS->SetParameter("RimR", GlP->Params()[0]);
    GS->SetParameter("RimW", GlP->Params()[1]);
    GS->SetParameter("RimQ", GlP->Params()[2]);
  }
  else if( FStaticObjects.String(ind) == "Disks" )  {
    GS->SetParameter("DiskIR", GlP->Params()[0]);
    GS->SetParameter("DiskOR", GlP->Params()[1]);
    GS->SetParameter("DiskQ", GlP->Params()[2]);
    GS->SetParameter("DiskS", GlP->Params()[3]);
  }
}
//..............................................................................
void TXAtom::CreateStaticPrimitives()  {
  TGlMaterial GlM;
  TGlPrimitiveParams *PParams;
  TGlPrimitive *GlP, *GlPRim, *GlPRC1, *GlPRD1, *GlPRD2;
  olxstr Legend("Atoms");
  TGraphicsStyle *GS;
  GS = FParent->Styles()->Style(Legend);
  if( !GS )
  {  GS = FParent->Styles()->NewStyle(Legend);  }
  double SphereQ   = GS->ParameterValue("SphereQ", 15).ToDouble();
  double RimR = GS->ParameterValue("RimR", 1.02).ToDouble();  // radius
  double RimW = GS->ParameterValue("RimW", 0.05).ToDouble();  // width
  double RimQ = GS->ParameterValue("RimQ", SphereQ).ToDouble();  // quality

  double DiskIR = GS->ParameterValue("DiskIR", 0).ToDouble();  // inner radius for disks
  double DiskOR = GS->ParameterValue("DiskOR", RimR).ToDouble();  // outer radius
  double DiskQ = GS->ParameterValue("DiskQ", RimQ).ToDouble();  // quality
  double DiskS = GS->ParameterValue("DiskS", RimW).ToDouble();  // separation

//..............................
  // create sphere
  GlP = FStaticObjects.FindObject("Sphere");
  if( GlP == NULL )  {
    GlP = FParent->NewPrimitive();  GlP->Type(sgloSphere);
    FStaticObjects.Add("Sphere", GlP);
  }
  else
  GlP->Params()[0] = 1;  GlP->Params()[1] = SphereQ; GlP->Params()[2] = SphereQ;
  GlP->Compile();
  GlP->Params().Resize(GlP->Params().Count()+1);
  GlP->Params().Last() = ddsDefSphere;
//..............................
  // create a small sphere
  GlP = (TGlPrimitive*)FStaticObjects.FindObject("Small sphere");
  if( !GlP )
  {
    GlP = FParent->NewPrimitive();  GlP->Type(sgloSphere);
    FStaticObjects.Add("Small sphere", GlP);
  }
  GlP->Params()[0] = 0.5;  GlP->Params()[1] = SphereQ; GlP->Params()[2] = SphereQ;
  GlP->Compile();
  GlP->Params().Resize(GlP->Params().Count()+1);
  GlP->Params().Last() = ddsDefSphere;
//..............................
  // create simple rims
  GlPRC1 = FParent->NewPrimitive();
  GlPRC1->Type(sgloCylinder);
  GlPRC1->Params()[0] = RimR;  GlPRC1->Params()[1] = RimR;  GlPRC1->Params()[2] = RimW;
    GlPRC1->Params()[3] = RimQ; GlPRC1->Params()[4] = 1;
  GlPRC1->Compile();

  GlPRim = (TGlPrimitive*)FStaticObjects.FindObject("Rims");
  if( !GlPRim )
  {  
    GlPRim = FParent->NewPrimitive();  GlPRim->Type(sgloCommandList); 
    FStaticObjects.Add("Rims", GlPRim);
  }
  GlPRim->StartList();
  GlPRim->CallList(GlPRC1);
  FParent->GlRotate(90, 1, 0, 0);
  GlPRim->CallList(GlPRC1);
  FParent->GlRotate(90, 0, 1, 0);
  GlPRim->CallList(GlPRC1);
  GlPRim->EndList();
  GlPRim->Params().Resize(3+1);  // radius, height, quality
  GlPRim->Params()[0] = RimR;
  GlPRim->Params()[1] = RimW;
  GlPRim->Params()[2] = RimQ;
  GlPRim->Params()[3] = ddsDefRim;
  PParams = new TGlPrimitiveParams;
  PParams->Params.Add("Radius");
  PParams->Params.Add("Width");
  PParams->Params.Add("Slices");
  FPrimitiveParams.Add(PParams);
//..............................
  // create disks
  GlPRD1 = FParent->NewPrimitive();
  GlPRD1->Type(sgloDisk);
  GlPRD1->Params()[0] = DiskIR;  GlPRD1->Params()[1] = DiskOR;
  GlPRD1->Params()[2] = DiskQ;   GlPRD1->Params()[3] = 1;
  GlPRD1->Compile();

  GlPRD2 = FParent->NewPrimitive();
  GlPRD2->Type(sgloDisk);
  GlPRD2->QuadricOrientation(GLU_INSIDE);
  GlPRD2->Params()[0] = DiskIR;  GlPRD1->Params()[1] = DiskOR;
  GlPRD2->Params()[2] = DiskQ;   GlPRD1->Params()[3] = 1;
  GlPRD2->Compile();

  GlPRim = (TGlPrimitive*)FStaticObjects.FindObject("Disks");
  if( !GlPRim )  {
    GlPRim = FParent->NewPrimitive();  GlPRim->Type(sgloCommandList); 
    FStaticObjects.Add("Disks", GlPRim);
  }
  GlPRim->StartList();
  GlPRim->CallList(GlPRD2);
  FParent->GlTranslate(0, 0, (float)DiskS);    GlPRim->CallList(GlPRD1);
  FParent->GlTranslate(0, 0, (float)(-DiskS) );    
  FParent->GlRotate(90, 1, 0, 0);
  GlPRim->CallList(GlPRD2);
  FParent->GlTranslate(0, 0, (float)DiskS);    GlPRim->CallList(GlPRD1);
  FParent->GlTranslate(0, 0, (float)(-DiskS) );    
  FParent->GlRotate(90, 0, 1, 0);
  GlPRim->CallList(GlPRD2);
  FParent->GlTranslate(0, 0, (float)DiskS);    GlPRim->CallList(GlPRD1);
  GlPRim->EndList();
  GlPRim->Params().Resize(4+1);  // inner radius, outer radius, Quality, offset
  GlPRim->Params()[0] = DiskIR;
  GlPRim->Params()[1] = DiskOR;
  GlPRim->Params()[2] = DiskQ;
  GlPRim->Params()[3] = DiskS;
  GlPRim->Params()[4] = ddsDefRim;
  PParams = new TGlPrimitiveParams;
  PParams->Params.Add("Inner radius");
  PParams->Params.Add("Outer radius");
  PParams->Params.Add("Slices");
  PParams->Params.Add("Separation");
  FPrimitiveParams.Add(PParams);
//..............................
}
//..............................................................................
void TXAtom::UpdatePrimitives(int32_t Mask, const ACreationParams* cpar)  {
  int SMask = Primitives()->Style()->ParameterValue("PMask", 0).ToInt();
  if( SMask == Mask )  return;
  Primitives()->Style()->SetParameter("PMask", Mask);
  Primitives()->ClearPrimitives();
  Primitives()->RemoveObject(this);
  Create(EmptyString, cpar);
}
//..............................................................................
//float TXAtom::Radius(){  return FParams()[0]; }
//..............................................................................
void TXAtom::ValidateAtomParams() {
  if( FAtomParams == NULL )  {
    FAtomParams =   TGlRender::GetStyles()->Style("AtomParams");
    if( !FAtomParams )  {
      FAtomParams = TGlRender::GetStyles()->NewStyle("AtomParams");
      FAtomParams->SetPersistent(true);
    }
  }
}
//..............................................................................
void TXAtom::Zoom(float V)  {
  Primitives()->Style()->SetParameter("Z", V);
  Params()[1] = V;
  // update radius for all members of the collection
  for( int i=0; i < Primitives()->ObjectCount(); i++ )
    Primitives()->Object(i)->Params()[1] = V;
}
//..............................................................................
void TXAtom::OnPrimitivesCleared()  {
  if( FStaticObjects.Count() != 0 )
    FStaticObjects.Clear();
}
//..............................................................................
void TXAtom::DefRad(short V)  {
  ValidateAtomParams();
  FAtomParams->SetParameter("DefR", V);
}
//..............................................................................
void TXAtom::DefDS(short V)  {
  ValidateAtomParams();
  FAtomParams->SetParameter("DefDS", V);
}
//..............................................................................
void TXAtom::DefSphMask(int V)  {
  ValidateAtomParams();
  FAtomParams->SetParameter("DefSphM", V);
}
//..............................................................................
void TXAtom::DefElpMask(int V)  {
  ValidateAtomParams();
  FAtomParams->SetParameter("DefElpM", V);
}
//..............................................................................
void TXAtom::DefNpdMask(int V)  {
  ValidateAtomParams();
  FAtomParams->SetParameter("DefNpdM", V);
}
//..............................................................................
void TXAtom::DefZoom(float V)  {
  ValidateAtomParams();
  FAtomParams->SetParameter("DefZ", V);
}
//..............................................................................
void TXAtom::TelpProb(float V)  {
  ValidateAtomParams();
  FAtomParams->SetParameter("TelpP", V);
  FTelpProb = V;
}
//..............................................................................
short TXAtom::DefRad()  {
  ValidateAtomParams();
  return FAtomParams->ParameterValue("DefR", darPers).ToInt();
}
//..............................................................................
short TXAtom::DefDS()  {
  ValidateAtomParams();
  return FAtomParams->ParameterValue("DefDS", adsSphere).ToInt();
}
//..............................................................................
int TXAtom::DefSphMask()  {
  ValidateAtomParams();
  return FAtomParams->ParameterValue("DefSphM", "1").ToInt();
}
//..............................................................................
int TXAtom::DefElpMask()  {
  ValidateAtomParams();
  return  FAtomParams->ParameterValue("DefElpM", "5").ToInt();
}
//..............................................................................
int TXAtom::DefNpdMask()  {
  ValidateAtomParams();
  return  FAtomParams->ParameterValue("DefNpdM", "6").ToInt();
}
//..............................................................................
float TXAtom::TelpProb()  {
  if( FTelpProb )  return FTelpProb;
  ValidateAtomParams();
  FTelpProb = (float)FAtomParams->ParameterValue("TelpP", "1").ToDouble();
  return FTelpProb;
}
//..............................................................................
float TXAtom::DefZoom()  {
  ValidateAtomParams();
  return (float)FAtomParams->ParameterValue("DefZ", "1").ToDouble();
}
//..............................................................................
float TXAtom::QPeakScale()  {
  if( FQPeakScale )  return FQPeakScale;
  ValidateAtomParams();
  return (float)FAtomParams->ParameterValue("QPeakScale", "3").ToDouble();
}
//..............................................................................
void TXAtom::QPeakScale(float V)  {
  ValidateAtomParams();
  if( V < 1 )  V = 3;
  FAtomParams->SetParameter("QPeakScale", V);
  FQPeakScale = V;
}
//..............................................................................
bool TXAtom::OnMouseDown(const IEObject *Sender, const TMouseData *Data)  {
  if( !Moveable() )  return true;
  return TGlMouseListener::OnMouseDown(Sender, Data);
}
//..............................................................................
bool TXAtom::OnMouseUp(const IEObject *Sender, const TMouseData *Data)  {
  if( !Moveable() )  return true;
  return TGlMouseListener::OnMouseUp(Sender, Data);
}
//..............................................................................
bool TXAtom::OnMouseMove(const IEObject *Sender, const TMouseData *Data)  {
  TGlMouseListener::OnMouseMove(Sender, Data);
  return true;
}
//..............................................................................
