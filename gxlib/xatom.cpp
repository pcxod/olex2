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
#include "splane.h"
#include "planesort.h"

#include "glgroup.h"
#include "exyzgroup.h"
#include "glutil.h"

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
float TXAtom::FTelpProb = 0;
float TXAtom::FQPeakScale = 0;
short TXAtom::FDefRad = 0;
short TXAtom::FDefDS = 0;
int TXAtom::OrtepSpheres = -1;
TGraphicsStyle* TXAtom::FAtomParams=NULL;
TXAtomStylesClear *TXAtom::FXAtomStylesClear=NULL;
short TXAtom::PolyhedronIndex = -1;
short TXAtom::SphereIndex = -1;
//..............................................................................

TXAtom::TXAtom(const olxstr& collectionName, TSAtom& A, TGlRenderer *Render) :
  //AGDrawObject(collectionName)
  TGlMouseListener(collectionName, Render)
{
  FParent = Render;
  XAppId = -1;
  FAtom = &A;
  Polyhedron = NULL;
  Move2D(false);
  Moveable(false);
  Zoomable(false);

  if( A.GetEllipsoid() != NULL )
    FDrawStyle = A.GetEllipsoid()->IsNPD() ? adsEllipsoidNPD : adsEllipsoid;
  else
    FDrawStyle = adsSphere;
  FRadius = darIsot;
  Params().Resize(2);
  Params()[0] = 1;
  Params()[1] = 1;
  if( FStaticObjects.IsEmpty() )  
    CreateStaticPrimitives();
  // the objects will be automatically deleted by the corresponding action collections
}
//..............................................................................
TXAtom::~TXAtom()  {
  if( !FPrimitiveParams.IsEmpty() )
    FPrimitiveParams.Clear();
  if( OrtepSpheres != -1 )  {
    glDeleteLists(OrtepSpheres, 9);
    OrtepSpheres = -1;
  }
  if( Polyhedron != NULL )  {
    delete Polyhedron;
    Polyhedron = NULL;
  }
}
//..............................................................................
void TXAtom::Quality(const short V)  {
  olxstr Legend("Atoms");
  TGraphicsStyle *GS;
  GS = FParent->Styles()->NewStyle(Legend, true);

  olxstr &SphereQ   = GS->GetParam("SphereQ", EmptyString, true);
  olxstr &RimQ = GS->GetParam("RimQ", EmptyString, true);  // quality
  olxstr &DiskQ = GS->GetParam("DiskQ", EmptyString, true);  // quality

  olxstr &RimR = GS->GetParam("RimR", EmptyString, true);  // radius
  olxstr &RimW = GS->GetParam("RimW", EmptyString, true);  // width

  //olxstr &DiskIR = GS->ParameterValue("DiskIR", EmptyString);  // inner radius for disks
  olxstr &DiskOR = GS->GetParam("DiskOR", EmptyString, true);  // outer radius
  olxstr &DiskS = GS->GetParam("DiskS", EmptyString, true);  // separation

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
  FRadius = DefRadius;
  Primitives()->Style()->SetParam("DR", DefRadius);

  if( DefRadius == darPers )  
    FParams[0] = FAtom->GetAtomInfo().GetRad();   
  else if( DefRadius == darPack )  
    FParams[0] = FAtom->GetAtomInfo().GetRad2();
  else if( DefRadius == darBond )  
    FParams[0] = sqrt(caDefIso)/2;
  else if( DefRadius == darIsot )  {
    if( FAtom->GetAtomInfo() == iHydrogenIndex ) 
      FParams[0] = 2*caDefIso;
    else  {
      if( FAtom->CAtom().GetUiso() > 0 )
        FParams[0] = sqrt(FAtom->CAtom().GetUiso());
      else
        FParams[0] = 2*caDefIso; //sqrt(caDefIso);
    }
  }
  else if( DefRadius == darIsotH )  {
    if( FAtom->CAtom().GetUiso() > 0 )
      FParams[0] = sqrt(FAtom->CAtom().GetUiso());
    else
      FParams[0] = 2*caDefIso; //sqrt(caDefIso);
  }
}
//..............................................................................
void TXAtom::ValidateRadius(TGraphicsStyle *GS)  {
  Params()[1] = GS->GetParam("Z", DefZoom()).ToDouble();
  short dr = GS->GetParam("DR", DefRad()).ToInt();
  CalcRad(dr);
}
void TXAtom::ValidateDS(TGraphicsStyle *GS)  {
  DrawStyle( GS->GetParam("DS", DefDS()).ToInt() );
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
  if( FStaticObjects.IsEmpty() )  
    CreateStaticPrimitives();

  // find collection
  if( FAtom->GetAtomInfo() == iQPeakIndex )  {
    Legend = GetLabelLegend(FAtom);
    GPC = FParent->FindCollection(Legend);
    if( GPC == NULL || GPC->ObjectCount() == 0 )  {  // if the collection is empty, need to fill it...
      if( GPC == NULL )
        GPC = FParent->NewCollection(Legend);
      GS = GPC->Style();
      GS->SetSaveable(false);
    }
    else  {
      GPC->AddObject(this);
      ValidateRadius(GPC->Style());
      return;
    }
  }
  else  {
    GPC = FParent->CollectionX(Legend, NewL);
    if( GPC == NULL )
      GPC = FParent->NewCollection(NewL);
    else  {
      if( GPC->PrimitiveCount() != 0 )  {
        GPC->AddObject(this);
        if( (GPC->Style()->GetParam("PMask", "0").ToInt() & (1 << PolyhedronIndex)) != 0 )
          CreatePolyhedron(true);
        if( cpar == NULL )  {
          ValidateRadius( GPC->Style() );
          ValidateDS(GPC->Style());
        }
        else
          Params() = cpar->params;
        return;
      }
    }
    GS = GPC->Style();
  }

  GS->SetSaveable( GPC->Name().CharCount('.') == 0 );

  olxstr& SMask = GS->GetParam("PMask", EmptyString);
  if( SMask.IsEmpty() )  {
    if( FAtom->GetEllipsoid() != NULL && DefRad() == darIsot )  {
      if( FAtom->GetEllipsoid()->IsNPD() )  {  SMask = DefNpdMask();  }
      else                                  {  SMask = DefElpMask();  }
    }
    else
      SMask = DefSphMask();
  }
  int PMask = SMask.ToInt();

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
    const int off = 1 << i;
    if( PMask & off )  {
      TGlPrimitive* SGlP = FStaticObjects.Object(i);
      TGlPrimitive* GlP = GPC->NewPrimitive(FStaticObjects.String(i), sgloCommandList);
      if( i == SphereIndex )
        GlP->SetOwnerId( xatom_SphereId );
      else if( i == PolyhedronIndex )  {
        GlP->SetOwnerId( xatom_PolyId );
        CreatePolyhedron(true);
      }
      else
        GlP->SetOwnerId( 0 );
      /* copy the default drawing style tag*/
      GlP->Params.Resize(GlP->Params.Count()+1);
      GlP->Params.Last() = SGlP->Params.Last();

      GlP->StartList();
      GlP->CallList(SGlP);
      GlP->EndList();

      if( FAtom->GetAtomInfo() == iQPeakIndex )  {
        GetDefSphereMaterial(*FAtom, RGlM);
        GlP->SetProperties(&RGlM);
      }
      else  {
        const TGlMaterial* GlM = GS->Material(FStaticObjects.String(i));
        if( GlM->Mark() )  {
          if( SGlP->Params.Last() == ddsDefSphere ) GetDefSphereMaterial(*FAtom, RGlM);
          if( SGlP->Params.Last() == ddsDefRim )    GetDefRimMaterial(*FAtom, RGlM);
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
  AtomCreationParams& ap = *(new AtomCreationParams);
  ap.params = FParams;
  return &ap;
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
  if( GlP->GetOwnerId() == xatom_PolyId )  {
    if( Polyhedron == NULL )  return true;
    glBegin(GL_TRIANGLES);
    const TXAtom::Poly& pl = *Polyhedron;
    for( int j=0; j < pl.faces.Count(); j++ )  {
      glNormal3f( pl.norms[j][0], pl.norms[j][1], pl.norms[j][2] );
      for( int k=0; k < 3; k++ )  {
        const vec3f& vec = pl.vecs[pl.faces[j][k]];
        glVertex3f( vec[0], vec[1], vec[2] );
      }
    }
    glEnd();
    return true;
  }
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

  double scale = FParams[1];
  if( (FRadius & (darIsot|darIsotH)) != 0 )
    scale *= TelpProb();

  if( FDrawStyle == adsEllipsoid || FDrawStyle == adsOrtep )  {
    if( FAtom->GetEllipsoid() != NULL )  {
      if( FAtom->GetEllipsoid()->IsNPD() )  {
        FParent->GlScale((float)(caDefIso*2*scale));
      }
      else  {
        FParent->GlOrient( FAtom->GetEllipsoid()->GetMatrix() );
        FParent->GlScale(
          (float)(FAtom->GetEllipsoid()->GetSX()*scale),
          (float)(FAtom->GetEllipsoid()->GetSY()*scale),
          (float)(FAtom->GetEllipsoid()->GetSZ()*scale)
          );
        if( FDrawStyle == adsOrtep && GlP->GetOwnerId() == xatom_SphereId )  {
          short mask = 0;
          const mat3d mat = FAtom->GetEllipsoid()->GetMatrix()*FParent->GetBasis().GetMatrix();
          for( int i=0; i < 3; i++ )  {
            if( mat[i][2] < 0 )
              mask |= (1<<i);
          }
          glCallList(OrtepSpheres+mask);
          return true;
        }
      }
    }
    return false;
  }
  if( FDrawStyle == adsSphere )
    FParent->GlScale( (float)(FParams[0]*scale) );
  return false;
}
//..............................................................................
bool TXAtom::DrawStencil()  {
  double scale = FParams[1];
  if( (FRadius & (darIsot|darIsotH)) != 0 )
    scale *= TelpProb();

  if( FDrawStyle == adsEllipsoid || FDrawStyle == adsOrtep )  {
    if( FAtom->GetEllipsoid() != NULL )  {
      glPushMatrix();  
      FParent->GlTranslate(Basis.GetCenter()+ FAtom->crd());
      FParent->GlOrient( FAtom->GetEllipsoid()->GetMatrix() );
      FParent->GlScale(
        (float)(FAtom->GetEllipsoid()->GetSX()*scale),
        (float)(FAtom->GetEllipsoid()->GetSY()*scale),
        (float)(FAtom->GetEllipsoid()->GetSZ()*scale)
        );
      glCallList(OrtepSpheres+8);
      glPopMatrix();
      return true;
    }
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
      if( Atom.CAtom().GetParent()->GetMaxQPeak() < 0 )
        M.AmbientF[3] = (float)(atan(QPeakScale()*peak/Atom.CAtom().GetParent()->GetMinQPeak())*2/M_PI);
      else
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
    TGlPrimitive* GP = Primitives()->FindPrimitiveByName( Style->PrimitiveStyle(i)->PrimitiveName() );
    if( GP != NULL )
      GP->SetProperties(Style->PrimitiveStyle(i)->GetProperties());
  } 
}
//..............................................................................
void TXAtom::DrawStyle(short V)  {
  olxstr &DS = Primitives()->Style()->GetParam("DS", EmptyString);
  if( V == adsEllipsoid || V == adsOrtep )  {
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
  GS = FParent->Styles()->NewStyle(Legend, true);
  if( FStaticObjects.String(ind) == "Sphere" )
    GS->SetParam("SphereQ", GlP->Params[1], true);
  else if( FStaticObjects.String(ind) == "Small sphere" )
    GS->SetParam("SphereQ", GlP->Params[1], true);
  else if( FStaticObjects.String(ind) == "Rims" )  {
    GS->SetParam("RimR", GlP->Params[0], true);
    GS->SetParam("RimW", GlP->Params[1], true);
    GS->SetParam("RimQ", GlP->Params[2], true);
  }
  else if( FStaticObjects.String(ind) == "Disks" )  {
    GS->SetParam("DiskIR", GlP->Params[0], true);
    GS->SetParam("DiskOR", GlP->Params[1], true);
    GS->SetParam("DiskQ", GlP->Params[2], true);
    GS->SetParam("DiskS", GlP->Params[3], true);
  }
}
//..............................................................................
void TXAtom::CreateStaticPrimitives()  {
  if( !FStaticObjects.IsEmpty() )
    return;
  TGlMaterial GlM;
  TGlPrimitiveParams *PParams;
  TGlPrimitive *GlP, *GlPRim, *GlPRC1, *GlPRD1, *GlPRD2;
  olxstr Legend("Atoms");
  TGraphicsStyle *GS;
  GS = FParent->Styles()->NewStyle(Legend, true);
  double SphereQ   = GS->GetParam("SphereQ", "15", true).ToDouble();
  double RimR = GS->GetParam("RimR", "1.02", true).ToDouble();  // radius
  double RimW = GS->GetParam("RimW", "0.05", true).ToDouble();  // width
  double RimQ = GS->GetParam("RimQ", SphereQ, true).ToDouble();  // quality

  double DiskIR = GS->GetParam("DiskIR", "0", true).ToDouble();  // inner radius for disks
  double DiskOR = GS->GetParam("DiskOR", RimR, true).ToDouble();  // outer radius
  double DiskQ = GS->GetParam("DiskQ", RimQ, true).ToDouble();  // quality
  double DiskS = GS->GetParam("DiskS", RimW, true).ToDouble();  // separation

//..............................
  // create sphere
  GlP = FParent->NewPrimitive(sgloSphere);
  FStaticObjects.Add("Sphere", GlP);
  GlP->Params[0] = 1;  GlP->Params[1] = SphereQ; GlP->Params[2] = SphereQ;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count()+1);
  GlP->Params.Last() = ddsDefSphere;
  SphereIndex = FStaticObjects.IndexOf("Sphere");
//..............................
  // create a small sphere
  GlP = FParent->NewPrimitive(sgloSphere);
  FStaticObjects.Add("Small sphere", GlP);
  GlP->Params[0] = 0.5;  GlP->Params[1] = SphereQ; GlP->Params[2] = SphereQ;
  GlP->Compile();
  GlP->Params.Resize(GlP->Params.Count()+1);
  GlP->Params.Last() = ddsDefSphere;
//..............................
  // create simple rims
  GlPRC1 = FParent->NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = RimR;  GlPRC1->Params[1] = RimR;  GlPRC1->Params[2] = RimW;
    GlPRC1->Params[3] = RimQ; GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlPRim = FParent->NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Rims", GlPRim);
  GlPRim->StartList();
  GlPRim->CallList(GlPRC1);
  FParent->GlRotate(90, 1, 0, 0);
  GlPRim->CallList(GlPRC1);
  FParent->GlRotate(90, 0, 1, 0);
  GlPRim->CallList(GlPRC1);
  GlPRim->EndList();
  GlPRim->Params.Resize(3+1);  // radius, height, quality
  GlPRim->Params[0] = RimR;
  GlPRim->Params[1] = RimW;
  GlPRim->Params[2] = RimQ;
  GlPRim->Params[3] = ddsDefRim;
  PParams = new TGlPrimitiveParams;
  PParams->Params.Add("Radius");
  PParams->Params.Add("Width");
  PParams->Params.Add("Slices");
  FPrimitiveParams.Add(PParams);
//..............................
  // create disks
  GlPRD1 = FParent->NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = DiskIR;  GlPRD1->Params[1] = DiskOR;
  GlPRD1->Params[2] = DiskQ;   GlPRD1->Params[3] = 1;
  GlPRD1->Compile();

  GlPRD2 = FParent->NewPrimitive(sgloDisk);
  GlPRD2->SetQuadricOrientation(GLU_INSIDE);
  GlPRD2->Params[0] = DiskIR;  GlPRD1->Params[1] = DiskOR;
  GlPRD2->Params[2] = DiskQ;   GlPRD1->Params[3] = 1;
  GlPRD2->Compile();

  GlPRim = FParent->NewPrimitive(sgloCommandList);
  FStaticObjects.Add("Disks", GlPRim);
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
  GlPRim->Params.Resize(4+1);  // inner radius, outer radius, Quality, offset
  GlPRim->Params[0] = DiskIR;
  GlPRim->Params[1] = DiskOR;
  GlPRim->Params[2] = DiskQ;
  GlPRim->Params[3] = DiskS;
  GlPRim->Params[4] = ddsDefRim;
  PParams = new TGlPrimitiveParams;
  PParams->Params.Add("Inner radius");
  PParams->Params.Add("Outer radius");
  PParams->Params.Add("Slices");
  PParams->Params.Add("Separation");
  FPrimitiveParams.Add(PParams);
//..............................
  // create cross
  GlP = FParent->NewPrimitive(sgloLines);
  FStaticObjects.Add("Cross", GlP);
  GlP->Data.Resize(3, 6);
  GlP->Data[0][0] = -1; 
  GlP->Data[0][1] =  1; 
  GlP->Data[1][2] = -1; 
  GlP->Data[1][3] =  1; 
  GlP->Data[2][4] = -1; 
  GlP->Data[2][5] =  1; 
  GlP->Params[0] = 1.0;
  GlP->Params.Resize(GlP->Params.Count()+1);
  GlP->Params.Last() = ddsDefSphere;
//..............................
  // polyhedron - dummy
  GlP = FParent->NewPrimitive(sgloMacro);
  FStaticObjects.Add("Polyhedron", GlP);
  GlP->Params.Resize(GlP->Params.Count()+1);
  GlP->Params.Last() = ddsDefSphere;
  PolyhedronIndex = FStaticObjects.IndexOf("Polyhedron");
//..............................
  GlSphereEx gls;
  TTypeList<vec3f> vecs;
  TTypeList<GlTriangle> triags;
  TArrayList<vec3f> norms;
  gls.Generate(1, Round(log(SphereQ)+0.5), vecs, triags, norms);
  if( OrtepSpheres == -1 )
    OrtepSpheres = glGenLists(9);
  
  glNewList(OrtepSpheres, GL_COMPILE);
  gls.RenderEx(vecs, triags, norms, vec3f(0,0,0), vec3f(1,1,1));
  glEndList();  
  glNewList(OrtepSpheres+1, GL_COMPILE);
  gls.RenderEx(vecs, triags, norms, vec3f(-1,0,0), vec3f(0,1,1));
  glEndList();  
  glNewList(OrtepSpheres+2, GL_COMPILE);
  gls.RenderEx(vecs, triags, norms, vec3f(0,-1,0), vec3f(1,0,1));
  glEndList();  
  glNewList(OrtepSpheres+3, GL_COMPILE);
  gls.RenderEx(vecs, triags, norms, vec3f(-1,-1,0), vec3f(0,0,1));
  glEndList();  
  glNewList(OrtepSpheres+4, GL_COMPILE);
  gls.RenderEx(vecs, triags, norms, vec3f(0,0,-1), vec3f(1,1,0));
  glEndList();  
  glNewList(OrtepSpheres+5, GL_COMPILE);
  gls.RenderEx(vecs, triags, norms, vec3f(-1,0,-1), vec3f(0,1,0));
  glEndList();  
  glNewList(OrtepSpheres+6, GL_COMPILE);
  gls.RenderEx(vecs, triags, norms, vec3f(0,-1,-1), vec3f(1,0,0));
  glEndList();  
  glNewList(OrtepSpheres+7, GL_COMPILE);
  gls.RenderEx(vecs, triags, norms, vec3f(-1,-1,-1), vec3f(0,0,0));
  glEndList();  
  glNewList(OrtepSpheres+8, GL_COMPILE);
  gls.Render(vecs, triags, norms);
  glEndList();  
}
//..............................................................................
void TXAtom::UpdatePrimitives(int32_t Mask, const ACreationParams* cpar)  {
  olxstr& mstr = Primitives()->Style()->GetParam("PMask", "0");
  if( mstr.ToInt() == Mask )  return;
  mstr = Mask;
  Primitives()->ClearPrimitives();
  Primitives()->RemoveObject(this);
  Create(EmptyString, cpar);
  bool create_polyhedron = (Mask & (1 << PolyhedronIndex)) != 0;
  for( int i=0; i < Primitives()->ObjectCount(); i++ ) {
    if( EsdlInstanceOf(*Primitives()->Object(i), TXAtom) )
      ((TXAtom*)Primitives()->Object(i))->CreatePolyhedron(create_polyhedron);
  }
}
//..............................................................................
//float TXAtom::Radius(){  return FParams()[0]; }
//..............................................................................
void TXAtom::ValidateAtomParams() {
  if( FAtomParams == NULL )  {
    FAtomParams = TGlRenderer::GetStyles()->NewStyle("AtomParams", true);
    FAtomParams->SetPersistent(true);
  }
}
//..............................................................................
void TXAtom::Zoom(float V)  {
  Primitives()->Style()->SetParam("Z", V);
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
  FAtomParams->SetParam("DefR", V, true);
  FDefRad = V;
}
//..............................................................................
void TXAtom::DefDS(short V)  {
  ValidateAtomParams();
  FAtomParams->SetParam("DefDS", V, true);
  FDefDS = V;
}
//..............................................................................
void TXAtom::DefSphMask(int V)  {
  ValidateAtomParams();
  FAtomParams->SetParam("DefSphM", V, true);
}
//..............................................................................
void TXAtom::DefElpMask(int V)  {
  ValidateAtomParams();
  FAtomParams->SetParam("DefElpM", V, true);
}
//..............................................................................
void TXAtom::DefNpdMask(int V)  {
  ValidateAtomParams();
  FAtomParams->SetParam("DefNpdM", V, true);
}
//..............................................................................
void TXAtom::DefZoom(float V)  {
  ValidateAtomParams();
  FAtomParams->SetParam("DefZ", V, true);
}
//..............................................................................
void TXAtom::TelpProb(float V)  {
  ValidateAtomParams();
  FAtomParams->SetParam("TelpP", V, true);
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
int TXAtom::DefSphMask()  {
  ValidateAtomParams();
  return FAtomParams->GetParam("DefSphM", "1", true).ToInt();
}
//..............................................................................
int TXAtom::DefElpMask()  {
  ValidateAtomParams();
  return FAtomParams->GetParam("DefElpM", "5", true).ToInt();
}
//..............................................................................
int TXAtom::DefNpdMask()  {
  ValidateAtomParams();
  return FAtomParams->GetParam("DefNpdM", "6", true).ToInt();
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
float TXAtom::QPeakScale()  {
  if( FQPeakScale )  return FQPeakScale;
  ValidateAtomParams();
  return (float)FAtomParams->GetParam("QPeakScale", "3", true).ToDouble();
}
//..............................................................................
void TXAtom::QPeakScale(float V)  {
  ValidateAtomParams();
  if( V < 1 )  
    V = 3;
  FAtomParams->SetParam("QPeakScale", V, true);
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
void TXAtom::TriangulateType2(Poly& pl, TSAtomPList& atoms)  {
  TSPlane plane(NULL);
  TTypeList< AnAssociation2<TSAtom*, double> > pa;
  for( int i=0; i < atoms.Count(); i++ )
    pa.AddNew( atoms[i], 1 );
  plane.Init(pa);
  PlaneSort::Sorter sp(plane);
  const int start = pl.vecs.Count();
  pl.vecs.SetCount(pl.vecs.Count()+atoms.Count()+2);
  pl.vecs[start] = FAtom->crd();
  pl.vecs[start+1] = plane.GetCenter();
  for( int i=0; i < sp.sortedPlane.Count(); i++ )
    pl.vecs[start+i+2] = *sp.sortedPlane.GetObject(i);
  // create the base
  for( int i=0; i < sp.sortedPlane.Count()-1; i++ )
    pl.faces.AddNew(start+1, start+i+2, start+i+3);
  pl.faces.AddNew(start+1, start+2, start+sp.sortedPlane.Count()+1);
  // and the rest to close the polyhedron
  for( int i=0; i < sp.sortedPlane.Count()-1; i++ )
    pl.faces.AddNew(start, start+i+2, start+i+3);
  pl.faces.AddNew(start, start+2, start+sp.sortedPlane.Count()+1);
}
//..............................................................................
void TXAtom::CreatePolyhedron(bool v)  {
  static const double cos75 = cos(75.0*M_PI/180);
  if( Polyhedron != NULL )  {
    delete Polyhedron;
    Polyhedron = NULL;
    if( !v )
      return;
  }

  if( FAtom->NodeCount() < 4 )  return;
  if( FAtom->IsDeleted() || FAtom->GetAtomInfo() == iQPeakIndex )
    return;
  TPtrList<TSAtom> bound;
  for( int i=0; i < FAtom->NodeCount(); i++ )  {
    if( FAtom->Node(i).IsDeleted() || 
      FAtom->Node(i).GetAtomInfo() == iHydrogenIndex ||
      FAtom->Node(i).GetAtomInfo() == iDeuteriumIndex || 
      FAtom->Node(i).GetAtomInfo() == iQPeakIndex )
      continue;
    //if( FAtom->Node(i).NodeCount() > FAtom->NodeCount() )
    //  continue;
    bound.Add( &FAtom->Node(i) );
  }
  if( bound.Count() < 4 )  return;
  TXAtom::Poly& pl = *(Polyhedron = new TXAtom::Poly);
  pl.vecs.SetCount( bound.Count() );
  vec3f sv;
  double sd = 0;
  for( int i=0; i < bound.Count(); i++ )  {
    pl.vecs[i] = bound[i]->crd();
    vec3d nd = bound[i]->crd() - FAtom->crd();
    sd += nd.Length();
    sv += nd.Normalise();
  }
  sv /= bound.Count();
  sd /= bound.Count();
  double ratio = sv.Length()/sd;
  if( ratio < 0.2 )  {  // atom is inside
    if( bound.Count() == 4 )  {  // easy case
      pl.faces.AddNew( 0, 1, 2 );
      pl.faces.AddNew( 0, 2, 3 );
      pl.faces.AddNew( 0, 3, 1 );
      pl.faces.AddNew( 1, 2, 3 );
    }
    else  {
      // test for Cp-kind polyhedron, should be drawn as two of the other kind (atom outside)
      vec3d pn, pc;
      const double rms = TSPlane::CalcPlane(bound, pn, pc, plane_best);
      const double pd = pc.DotProd(pn)/pn.Length();
      pn.Normalise();
      int deviating = 0;
      for( int i=0; i < bound.Count(); i++ )  {
        if( olx_abs(bound[i]->crd().DotProd(pn) - pd) > rms )
          deviating++;
      }
      if( deviating < 3 )  {  // a proepr polyhedra
        for( int i=0; i < bound.Count(); i++ )  {
          for( int j=i+1; j < bound.Count(); j++ )  {
            for( int k=j+1; k < bound.Count(); k++ )  {
              if( TetrahedronVolume(FAtom->crd(), bound[i]->crd(), bound[j]->crd(), bound[k]->crd() ) > 0.1 )
                pl.faces.AddNew(i, j, k);
            }
          }
        }
      }
      else  {  // two polyhedra of atom outside..
        TSAtomPList sidea, sideb;
        TSPlane::CalcPlane(bound, pn, pc, plane_worst);
        pl.vecs.Clear();
        for( int i=0; i < bound.Count(); i++ )  {
          const double ca = pn.CAngle(bound[i]->crd() - pc);
          if( ca >= 0 )
            sidea.Add(bound[i]);
          else
            sideb.Add(bound[i]);
        }
        if( sidea.Count() > 2 )
          TriangulateType2(pl, sidea);
        if( sideb.Count() > 2 )
          TriangulateType2(pl, sideb);
      }
    }
  }
  else  {  // atom outside
    pl.vecs.Clear();
    TriangulateType2(pl, bound);
  }
  pl.norms.SetCount( pl.faces.Count() );
  for( int i=0; i < pl.faces.Count(); i++ )  {
    const vec3f& v1 = pl.vecs[pl.faces[i][0]];
    const vec3f& v2 = pl.vecs[pl.faces[i][1]];
    const vec3f& v3 = pl.vecs[pl.faces[i][2]];
    vec3f n = (v2-v1).XProdVec(v3-v1).Normalise();
    if( n.DotProd(v1) < 0 )  {
      int vi = pl.faces[i][0];
      pl.faces[i][0] = pl.faces[i][1];
      pl.faces[i][1] = vi;
      n *= -1;
    }
    pl.norms[i] = n;
  }
}
//..............................................................................
