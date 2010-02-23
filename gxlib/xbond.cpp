//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXBond
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xbond.h"
#include "gpcollection.h"
#include "xatom.h"
#include "lattice.h"
#include "symmparser.h"

//..............................................................................
bool TXBondStylesClear::Enter(const IEObject *Sender, const IEObject *Data)
{  TXBond::FBondParams = NULL; return true; }
//..............................................................................
bool TXBondStylesClear::Exit(const IEObject *Sender, const IEObject *Data)
{  TXBond::ValidateBondParams(); return true; }
//..............................................................................
//----------------------------------------------------------------------------//
// TXBond function bodies
//----------------------------------------------------------------------------//
TStrPObjList<olxstr,TGlPrimitive*> TXBond::FStaticObjects;
TArrayList<TGlPrimitiveParams>  TXBond::FPrimitiveParams;
TGraphicsStyle* TXBond::FBondParams=NULL;
TXBondStylesClear *TXBond::FXBondStylesClear=NULL;
//..............................................................................
TXBond::TXBond(TGlRenderer& R, const olxstr& collectionName, TSBond& B) :
  AGDrawObject(R, collectionName)
{
  SetGroupable(true);
  FDrawStyle = 0x0001;
  Params().Resize(5);
  FBond = &B;
  if( FBond != NULL )
    BondUpdated();
  Params()[4] = 0.8;
  if( FStaticObjects.IsEmpty() )  
    CreateStaticPrimitives();
  // the objects will be automatically deleted by the corresponding action collections
}
//..............................................................................
void TXBond::BondUpdated()  {
  vec3d C(FBond->B().crd() - FBond->A().crd());
  if( C.IsNull() )  
    Params().Null();
  else  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = (float)(acos(C[2])*180/M_PI);
    if( olx_abs(Params()[0]-180) < 1e-3 )  { // degenerate case with Pi rotation
      Params()[1] = 0;
      Params()[2] = 1;
    }
    else {
      Params()[1] = -C[1];
      Params()[2] = C[0];
    }
  }
}
//..............................................................................
void TXBond::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  olxstr NewL;

  TGlMaterial RGlM;

  if( FStaticObjects.IsEmpty() )  
    CreateStaticPrimitives();
  // find collection
  TGPCollection* GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = &Parent.NewCollection(NewL);
  else if( GPC->PrimitiveCount() != 0 )  {
    GPC->AddObject(*this);
    Params()[4] = GPC->GetStyle().GetParam("R", "1").ToDouble();
    return;
  }
  TGraphicsStyle& GS = GPC->GetStyle();
//  GS.SetSaveable( GPC->Name().CharCount('.') == 0 );
  GS.SetSaveable(IsStyleSaveable());

  const int PrimitiveMask = GS.GetParam(GetPrimitiveMaskName(),
    (FBond && (FBond->GetType() == sotHBond)) ? 2048 : DefMask(), IsMaskSaveable()).ToInt();

  GPC->AddObject(*this);
  if( PrimitiveMask == 0 )  
    return;  // nothing to create then...

  Params()[4]= GS.GetParam("R", Params()[4]).ToDouble();

  for( size_t i=0; i < FStaticObjects.Count(); i++ )  {
    if( (PrimitiveMask & (1<<i)) != 0 )    {
      TGlPrimitive* SGlP = FStaticObjects.GetObject(i);
      TGlPrimitive& GlP = GPC->NewPrimitive(FStaticObjects[i], sgloCommandList);
      /* copy the default drawing style tag*/
      GlP.Params.Resize(GlP.Params.Count()+1);
      GlP.Params.Last() = SGlP->Params.Last();

      GlP.StartList();
      GlP.CallList(SGlP);
      GlP.EndList();
      if( FBond == NULL )  { // no bond?
        RGlM.FromString("85;2155839359;2155313015;1.000,1.000,1.000,0.502;36");
        GlP.SetProperties(GS.GetMaterial(FStaticObjects[i], RGlM));
      }
      else  {
        if( SGlP->Params.Last() == ddsDefAtomA || SGlP->Params.Last() == ddsDef )  {
          if( cpar == NULL )
            TXAtom::GetDefSphereMaterial(FBond->A(), RGlM);
          else  {
            size_t mi = ((BondCreationParams*)cpar)->a1.Style().IndexOfMaterial("Sphere");
            if( mi != InvalidIndex )
              RGlM = ((BondCreationParams*)cpar)->a1.Style().GetPrimitiveStyle(mi).GetProperties();
            else
              TXAtom::GetDefSphereMaterial(FBond->A(), RGlM);
          }
        }
        else if( SGlP->Params.Last() == ddsDefAtomB )  {
          if( cpar == NULL )
            TXAtom::GetDefSphereMaterial(FBond->B(), RGlM);
          else  {
            size_t mi = ((BondCreationParams*)cpar)->a2.Style().IndexOfMaterial("Sphere");
            if( mi != InvalidIndex )
              RGlM = ((BondCreationParams*)cpar)->a2.Style().GetPrimitiveStyle(mi).GetProperties();
            else
              TXAtom::GetDefSphereMaterial(FBond->B(), RGlM);
          }
        }
        GlP.SetProperties(RGlM);
      }
    }
  }
}
//..............................................................................
ACreationParams* TXBond::GetACreationParams() const {
  return NULL;
}
//..............................................................................
TXBond::~TXBond()  {  }
//..............................................................................
bool TXBond::Orient(TGlPrimitive& GlP)  {
  Parent.GlTranslate(FBond->A().crd());
  Parent.GlRotate((float)Params()[0], (float)Params()[1], (float)Params()[2], 0.0);
  Parent.GlScale((float)Params()[4], (float)Params()[4], (float)Params()[3]);
  return false;
}
//..............................................................................
TGraphicsStyle* TXBond::Style()  {
  return NULL;
}
//..............................................................................
void TXBond::ListParams(TStrList &List, TGlPrimitive *Primitive)  {
}
//..............................................................................
void TXBond::ListParams(TStrList &List)  {
}
//..............................................................................
void TXBond::UpdatePrimitiveParams(TGlPrimitive *Primitive)  {
}
//..............................................................................
void TXBond::ListPrimitives(TStrList &List) const {
  List.Assign(FStaticObjects);
}
//..............................................................................
void TXBond::Quality(const short Val)  {
  ValidateBondParams();
  olxstr& ConeQ = FBondParams->GetParam("ConeQ", "15", true);
  switch( Val )  {
    case qaPict:
    case qaHigh:   ConeQ = 30;  break;
    case qaMedium: ConeQ = 15;  break;
    case qaLow:    ConeQ = 5;  break;
  }
  return;
}
//..............................................................................
void TXBond::ListDrawingStyles(TStrList &L){  return; }
//..............................................................................
void TXBond::CreateStaticPrimitives()  {
  TGlMaterial GlM;
  TGlPrimitive *GlP, *GlPRC1, *GlPRD1, *GlPRD2;
  ValidateBondParams();
  double ConeQ = FBondParams->GetParam("ConeQ", "15", true).ToDouble();
  double ConeStipples = FBondParams->GetParam("ConeStipples", "6", true).ToDouble();
//..............................
  // create single color cylinder
  if( (GlP = FStaticObjects.FindObject("Single cone")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCylinder);
    FStaticObjects.Add("Single cone", GlP);
  }
  GlP->Params[0] = 0.1;  GlP->Params[1] = 0.1;  GlP->Params[2] = 1;
  GlP->Params[3] = ConeQ;   GlP->Params[4] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
//..............................
  // create top disk
  if( (GlP = FStaticObjects.FindObject("Top disk")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Top disk", GlP);
  }
  GlPRC1 = &Parent.NewPrimitive(sgloDisk); 
  GlPRC1->Params[0] = 0;  GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = ConeQ;
  GlPRC1->Params[3] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  Parent.GlTranslate(0, 0, 1);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomB;
//..............................
  // create bottom disk
  if( (GlP = FStaticObjects.FindObject("Bottom disk")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloDisk);
    FStaticObjects.Add("Bottom disk", GlP);
  }
  GlP->SetQuadricOrientation(GLU_INSIDE);
  GlP->Params[0] = 0;  GlP->Params[1] = 0.1;  GlP->Params[2] = ConeQ;
  GlP->Params[3] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
//..............................
  // create middle disk
  if( (GlP = FStaticObjects.FindObject("Middle disk")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Middle disk", GlP);
  }
  GlPRC1 = &Parent.NewPrimitive(sgloDisk);
  GlPRC1->Params[0] = 0;  GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = ConeQ;
  GlPRC1->Params[3] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  Parent.GlTranslate(0, 0, 0.5);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
//..............................
  // create bottom cylinder
  if( (GlP = FStaticObjects.FindObject("Bottom cone")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCylinder);
    FStaticObjects.Add("Bottom cone", GlP);
  }
  GlP->Params[0] = 0.1;  GlP->Params[1] = 0.1;  GlP->Params[2] = 0.5;
  GlP->Params[3] = ConeQ;   GlP->Params[4] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
//..............................
  // create top cylinder
  if( (GlP = FStaticObjects.FindObject("Top cone")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Top cone", GlP);
  }
  GlPRC1 = &Parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = 0.5;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  Parent.GlTranslate(0, 0, 0.5);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomB;
//..............................
  // create bottom line
  if( (GlP = FStaticObjects.FindObject("Bottom line")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Bottom line", GlP);
  }
  GlP->StartList();
    glBegin(GL_LINES);
      glVertex3f(0, 0, 0);
      glVertex3f(0, 0, 0.5);
    glEnd();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
//..............................
  // create top line
  if( (GlP = FStaticObjects.FindObject("Top line")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Top line", GlP);
  }
  GlP->StartList();
    glBegin(GL_LINES);
      glVertex3f(0, 0, 0.5);
      glVertex3f(0, 0, 1.0);
    glEnd();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomB;
//..............................
  // create stipple cone
  float CL = (float)(1.0/(2*ConeStipples));
  if( (GlP = FStaticObjects.FindObject("Stipple cone")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Stipple cone", GlP);
  }
  GlPRC1 = &Parent.NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = CL;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlPRD1 = &Parent.NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = 0;  GlPRD1->Params[1] = 0.1;  GlPRD1->Params[2] = ConeQ;
  GlPRD1->Params[3] = 1;
  GlPRD1->Compile();

  GlPRD2 = &Parent.NewPrimitive(sgloDisk);
  GlPRD2->SetQuadricOrientation(GLU_INSIDE);
  GlPRD2->Params[0] = 0;  GlPRD2->Params[1] = 0.1;  GlPRD2->Params[2] = ConeQ;
  GlPRD2->Params[3] = 1;
  GlPRD2->Compile();

  GlP->StartList();
  for( int i=0; i < ConeStipples; i++ )  {
    if( i != 0 )
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    Parent.GlTranslate(0, 0, CL);
    GlP->CallList(GlPRD1);
    Parent.GlTranslate(0, 0, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDef;
  //..............................
  if( (GlP = FStaticObjects.FindObject("Bottom stipple cone")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Bottom stipple cone", GlP);
  }
  GlP->StartList();
  Parent.GlTranslate(0, 0, CL/2);
  for( int i=0; i < ConeStipples/2; i++ )  {
    if( i != 0 )
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    Parent.GlTranslate(0, 0, CL);
    GlP->CallList(GlPRD1);
    Parent.GlTranslate(0, 0, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
  //..............................
  if( (GlP = FStaticObjects.FindObject("Top stipple cone")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Top stipple cone", GlP);
  }
  GlP->StartList();
  Parent.GlTranslate(0, 0, (float)(0.5 + CL/2));
  for( int i=0; i < ConeStipples/2; i++ )  {
    GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    Parent.GlTranslate(0, 0, CL);
    GlP->CallList(GlPRD1);
    Parent.GlTranslate(0, 0, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomB;

//..............................
  // create stipped ball bond
  CL = (float)(1.0/(12.0));
  if( (GlP = FStaticObjects.FindObject("Sphere")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Balls bond", GlP);
  }
  GlPRC1 = &Parent.NewPrimitive(sgloSphere);
  GlPRC1->Params[0] = 0.02;    GlPRC1->Params[1] = 5;  GlPRC1->Params[2] = 5;
  GlPRC1->Compile();

  GlP->StartList();
  for( int i=0; i < 12; i++ )  {
    Parent.GlTranslate(0, 0, CL);
    GlP->CallList(GlPRC1);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDef;
//..............................
  // create line
  if( (GlP = FStaticObjects.FindObject("Line")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Line", GlP);
  }
  GlP->StartList();
    glBegin(GL_LINES);
      glVertex3f(0, 0, 0);
      glVertex3f(0, 0, 1.0);
    glEnd();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
//..............................
  // create stippled line
  if( (GlP = FStaticObjects.FindObject("Stippled line")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Stippled line", GlP);
  }
  GlP->StartList();
    glEnable(GL_LINE_STIPPLE );
    glLineStipple(1, 0xf0f0 );
    glBegin(GL_LINES);
      glVertex3f(0, 0, 0);
      glVertex3f(0, 0, 1.0);
    glEnd();
  glDisable(GL_LINE_STIPPLE );
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
}
//..............................................................................
olxstr TXBond::GetLegend(const TSBond& Bnd, const short level)  {
  olxstr L(EmptyString, 32);
  const TSAtom *A = &Bnd.A(),
               *B = &Bnd.B();
  if( A->GetType().z != B->GetType().z )  {
    if( A->GetType().z < B->GetType().z )  {
      A = &Bnd.B();  
      B = &Bnd.A();
    }
  }
  else  {
    if( A->GetLabel().Compare(B->GetLabel()) < 0 )  {
      A = &Bnd.B();
      B = &Bnd.A();
    }
  }
  L << A->GetType().symbol << '-' << B->GetType().symbol;
  if( Bnd.GetType() == sotHBond )  
    L << "@H";
  if( level == 0 )  return L;
  L << '.' << A->GetLabel() << '-' << B->GetLabel();
  if( level == 1 )  return L;
  L << '.' << TSymmParser::MatrixToSymmCode(A->GetNetwork().GetLattice().GetUnitCell(), A->GetMatrix(0)) <<
    '-' <<
    TSymmParser::MatrixToSymmCode(B->GetNetwork().GetLattice().GetUnitCell(), B->GetMatrix(0));
  return L;
}
//..............................................................................
void TXBond::SetRadius(float V)  {
  Params()[4] = V;
  GetPrimitives().GetStyle().SetParam("R", V, IsRadiusSaveable());
  // update radius for all members of the collection
  for( size_t i=0; i < GetPrimitives().ObjectCount(); i++ )
    GetPrimitives().GetObject(i).Params()[4] = V;
}
//..............................................................................
uint32_t TXBond::GetPrimitiveMask() const {
  return GetPrimitives().GetStyle().GetParam(GetPrimitiveMaskName(),
    (FBond && (FBond->GetType() == sotHBond)) ? 2048 : DefMask(), IsMaskSaveable()).ToUInt();
}
//..............................................................................
void TXBond::OnPrimitivesCleared()  {
  if( !FStaticObjects.IsEmpty() )
    FStaticObjects.Clear();
}
//..............................................................................
void TXBond::ValidateBondParams()  {
  if( !FBondParams )  {
    FBondParams = &TGlRenderer::_GetStyles().NewStyle("BondParams", true);
    FBondParams->SetPersistent(true);
  }
}
//..............................................................................
void TXBond::DefMask(int V)  {
  ValidateBondParams();
  FBondParams->SetParam("DefM", V, true);
}
//..............................................................................
int TXBond::DefMask()  {
  ValidateBondParams();
  return FBondParams->GetParam("DefM", "7", true).ToInt();
}
//..............................................................................
bool TXBond::OnMouseDown(const IEObject *Sender, const TMouseData *Data)  {
  return true;
}
//..............................................................................
bool TXBond::OnMouseUp(const IEObject *Sender, const TMouseData *Data)  {
  return false;
}
//..............................................................................
bool TXBond::OnMouseMove(const IEObject *Sender, const TMouseData *Data)  {
  return true;
}
//..............................................................................
