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
TXBond::TXBond(const olxstr& collectionName, TSBond& B, TGlRenderer *R) :
  AGDrawObject(collectionName)
{
  Groupable(true);
  FParent = R;
  FDrawStyle = 0x0001;
  Params().Resize(5);
  FBond = &B;
  if( FBond != NULL )
    BondUpdated();
  Params()[4] = 0.8;
  if( !FStaticObjects.Count() )  CreateStaticPrimitives();
  // the objects will be automatically deleted by the corresponding action collections
}
//..............................................................................
void TXBond::BondUpdated()  {
  vec3d C( FBond->B().crd() - FBond->A().crd() );
  if( C.IsNull() )  
    Params().Null();
  else  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = (float)(acos(C[2])*180/M_PI);
    if( olx_abs(Params()[0]-180) < 0.001 )  { // degenerate case with Pi rotation
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
  TGPCollection* GPC = FParent->CollectionX( GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = FParent->NewCollection(NewL);
  else if( GPC->PrimitiveCount() != 0 )  {
    GPC->AddObject(this);
    Params()[4] = GPC->Style()->GetParam("R", "1").ToDouble();
    return;
  }
  TGraphicsStyle* GS = GPC->Style();
  GS->SetSaveable( GPC->Name().CharCount('.') == 0 );

  int PrimitiveMask = GS->GetParam("PMask",
    (FBond && (FBond->GetType() == sotHBond)) ? 2048 : DefMask() ).ToInt();

  GPC->AddObject(this);
  if( PrimitiveMask == 0 )  
    return;  // nothing to create then...

  Params()[4]= GS->GetParam("R", Params()[4]).ToDouble();

  for( int i=0; i < FStaticObjects.Count(); i++ )  {
    int off = 1;
    off = off << i;
    if( PrimitiveMask & off )    {
      TGlPrimitive* SGlP = FStaticObjects.GetObject(i);
      TGlPrimitive* GlP = GPC->NewPrimitive(FStaticObjects[i], sgloCommandList);
      /* copy the default drawing style tag*/
      GlP->Params.Resize(GlP->Params.Count()+1);
      GlP->Params.Last() = SGlP->Params.Last();

      GlP->StartList();
      GlP->CallList(SGlP);
      GlP->EndList();
      TGlMaterial* GlM = const_cast<TGlMaterial*>(GS->Material(FStaticObjects[i]));
      if( GlM->Mark() && FBond )  {
        if( SGlP->Params.Last() == ddsDefAtomA )  {
          TXAtom::GetDefSphereMaterial(FBond->A(), RGlM);
          GS->PrimitiveMaterial(FStaticObjects[i], RGlM);
          GlM = &RGlM;
        }
        if( SGlP->Params.Last() == ddsDefAtomB )  {
          TXAtom::GetDefSphereMaterial(FBond->B(), RGlM);
          GS->PrimitiveMaterial(FStaticObjects[i], RGlM);
          GlM = &RGlM;
        }
      }
      else if( GlM->Mark() )  {
        GlM->SetIdentityDraw( false );
      }
      GlP->SetProperties(GlM);
    }
  }
  return;
}
//..............................................................................
ACreationParams* TXBond::GetACreationParams() const {
  return NULL;
}
//..............................................................................
TXBond::~TXBond()  {  }
//..............................................................................
bool TXBond::Orient(TGlPrimitive *GlP)  {
  FParent->GlTranslate( FBond->A().crd() );
  FParent->GlRotate((float)Params()[0], (float)Params()[1], (float)Params()[2], 0.0);
  FParent->GlScale((float)Params()[4], (float)Params()[4], (float)Params()[3]);
  return false;
}
//..............................................................................
TGraphicsStyle* TXBond::Style()  {
  return NULL;
}
//..............................................................................
void TXBond::ListParams(TStrList &List, TGlPrimitive *Primitive)
{
}
//..............................................................................
void TXBond::ListParams(TStrList &List)
{
}
//..............................................................................
void TXBond::UpdatePrimitiveParams(TGlPrimitive *Primitive)
{
}
//..............................................................................
void TXBond::ListPrimitives(TStrList &List) const {
  List.Assign(FStaticObjects);
}
//..............................................................................
void TXBond::Quality(const short Val)  {
  olxstr Legend("Bonds");
  TGraphicsStyle *GS;
  GS = FParent->Styles()->NewStyle(Legend, true);

  olxstr& ConeQ = GS->GetParam("ConeQ", "0", true);
//  double &ConeStipples = GS->ParameterValue("ConeStipples");

  switch( Val )  {
    case qaPict:
    case qaHigh:   ConeQ = 30;  break;
    case qaMedium: ConeQ = 15;  break;
    case qaLow:    ConeQ = 5;  break;
  }
//  CreateStaticPrimitives(false);
  return;
}
//..............................................................................
void TXBond::ListDrawingStyles(TStrList &L){  return; }
//..............................................................................
void TXBond::CreateStaticPrimitives()  {
  TGlMaterial GlM;
  TGlPrimitive *GlP, *GlPRC1, *GlPRD1, *GlPRD2;
  olxstr Legend("Bonds");
  TGraphicsStyle* GS = FParent->Styles()->NewStyle(Legend, true);
  double ConeQ = GS->GetParam("ConeQ", "5", true).ToDouble();
  double ConeStipples = GS->GetParam("ConeStipples", "6", true).ToDouble();
//..............................
  // create single color cylinder
  GlP = (TGlPrimitive*)FStaticObjects.FindObject("Single cone");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCylinder);
    FStaticObjects.Add("Single cone", GlP);
  }
  GlP->Params[0] = 0.1;  GlP->Params[1] = 0.1;  GlP->Params[2] = 1;
  GlP->Params[3] = ConeQ;   GlP->Params[4] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDef;
//..............................
  // create top disk
  GlP = (TGlPrimitive*)FStaticObjects.FindObject("Top disk");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Top disk", GlP);
  }
  GlPRC1 = FParent->NewPrimitive(sgloDisk); 
  GlPRC1->Params[0] = 0;  GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = ConeQ;
  GlPRC1->Params[3] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  FParent->GlTranslate(0, 0, 1);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomB;
//..............................
  // create bottom disk
  GlP = (TGlPrimitive*)FStaticObjects.FindObject("Bottom disk");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloDisk);
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
  GlP = (TGlPrimitive*)FStaticObjects.FindObject("Middle disk");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Middle disk", GlP);
  }
  GlPRC1 = FParent->NewPrimitive(sgloDisk);
  GlPRC1->Params[0] = 0;  GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = ConeQ;
  GlPRC1->Params[3] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  FParent->GlTranslate(0, 0, 0.5);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
//..............................
  // create bottom cylinder
  GlP = (TGlPrimitive*)FStaticObjects.FindObject("Bottom cone");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCylinder);
    FStaticObjects.Add("Bottom cone", GlP);
  }
  GlP->Params[0] = 0.1;  GlP->Params[1] = 0.1;  GlP->Params[2] = 0.5;
  GlP->Params[3] = ConeQ;   GlP->Params[4] = 1;
  GlP->Compile();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
//..............................
  // create top cylinder
  GlP = (TGlPrimitive*)FStaticObjects.FindObject("Top cone");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Top cone", GlP);
  }
  GlPRC1 = FParent->NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = 0.5;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlP->StartList();
  FParent->GlTranslate(0, 0, 0.5);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomB;
//..............................
  // create bottom line
  GlP = (TGlPrimitive*)FStaticObjects.FindObject("Bottom line");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
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
  GlP = (TGlPrimitive*)FStaticObjects.FindObject("Top line");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
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
  GlP = (TGlPrimitive*)FStaticObjects.FindObject("Stipple cone");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Stipple cone", GlP);
  }
  GlPRC1 = FParent->NewPrimitive(sgloCylinder);
  GlPRC1->Params[0] = 0.1;    GlPRC1->Params[1] = 0.1;  GlPRC1->Params[2] = CL;
  GlPRC1->Params[3] = ConeQ;  GlPRC1->Params[4] = 1;
  GlPRC1->Compile();

  GlPRD1 = FParent->NewPrimitive(sgloDisk);
  GlPRD1->Params[0] = 0;  GlPRD1->Params[1] = 0.1;  GlPRD1->Params[2] = ConeQ;
  GlPRD1->Params[3] = 1;
  GlPRD1->Compile();

  GlPRD2 = FParent->NewPrimitive(sgloDisk);
  GlPRD2->SetQuadricOrientation(GLU_INSIDE);
  GlPRD2->Params[0] = 0;  GlPRD2->Params[1] = 0.1;  GlPRD2->Params[2] = ConeQ;
  GlPRD2->Params[3] = 1;
  GlPRD2->Compile();

  GlP->StartList();
  for( int i=0; i < ConeStipples; i++ )  {
    if( i != 0 )
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    FParent->GlTranslate(0, 0, CL);
    GlP->CallList(GlPRD1);
    FParent->GlTranslate(0, 0, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDef;
  //..............................
  GlP = FStaticObjects.FindObject("Bottom stipple cone");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Bottom stipple cone", GlP);
  }
  GlP->StartList();
  FParent->GlTranslate(0, 0, CL/2);
  for( int i=0; i < ConeStipples/2; i++ )  {
    if( i != 0 )
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    FParent->GlTranslate(0, 0, CL);
    GlP->CallList(GlPRD1);
    FParent->GlTranslate(0, 0, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomA;
  //..............................
  GlP = FStaticObjects.FindObject("Top stipple cone");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Top stipple cone", GlP);
  }
  GlP->StartList();
  FParent->GlTranslate(0, 0, (float)(0.5 + CL/2));
  for( int i=0; i < ConeStipples/2; i++ )  {
    GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    FParent->GlTranslate(0, 0, CL);
    GlP->CallList(GlPRD1);
    FParent->GlTranslate(0, 0, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDefAtomB;

//..............................
  // create stipped ball bond
  CL = (float)(1.0/(12.0));
  GlP = FStaticObjects.FindObject("Sphere");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Balls bond", GlP);
  }
  GlPRC1 = FParent->NewPrimitive(sgloSphere);
  GlPRC1->Params[0] = 0.02;    GlPRC1->Params[1] = 5;  GlPRC1->Params[2] = 5;
  GlPRC1->Compile();

  GlP->StartList();
  for( int i=0; i < 12; i++ )  {
    FParent->GlTranslate(0, 0, CL);
    GlP->CallList(GlPRC1);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.Last() = ddsDef;
//..............................
  // create line
  GlP = FStaticObjects.FindObject("Line");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
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
  GlP = FStaticObjects.FindObject("Stippled line");
  if( !GlP )  {
    GlP = FParent->NewPrimitive(sgloCommandList);
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
void TXBond::UpdatePrimitives(int32_t Mask, const ACreationParams* cpar)  {
  olxstr& pm = Primitives()->Style()->GetParam("PMask", "0"); 
  if( pm.ToInt() == Mask )  return;
  Primitives()->Style()->SetParam("PMask", Mask);
  Primitives()->ClearPrimitives();
  Primitives()->RemoveObject(this);
  Create(EmptyString, cpar);
}
//..............................................................................
olxstr TXBond::GetLegend(const TSBond& Bnd, const short AtomALevel, const short AtomBLevel)  {
  olxstr L;
  const TSAtom *A = &Bnd.A(),
               *B = &Bnd.B();
  short ALevel = AtomALevel, BLevel = AtomBLevel;
  if( A->GetAtomInfo().GetMr() != B->GetAtomInfo().GetMr() )  {
    if( A->GetAtomInfo().GetMr() < B->GetAtomInfo().GetMr() )  {
      A = &Bnd.B();      B = &Bnd.A();
      ALevel = AtomBLevel;  BLevel = AtomALevel;
    }
  }
  else  {
    if( A->GetLabel().Compare( B->GetLabel() ) < 0 )  {
      A = &Bnd.B();      B = &Bnd.A();
      ALevel = AtomBLevel;  BLevel = AtomALevel;
    }
  }
  olxstr LA, LB;
  TStrList T(TXAtom::GetLegend(*A, ALevel), '.'), 
           T1(TXAtom::GetLegend(*B, BLevel), '.');
  int maxI = olx_max(T.Count(), T1.Count());
  for( int i=0; i < maxI; i++ )  {
    LA = (i >= T.Count()) ? T.Last().String : T[i];
    LB = (i >= T1.Count()) ? T1.Last().String : T1[i];
    if( LA.Compare(LB) < 0 )
      L << LA << '-' << LB;
    else
      L << LB << '-' << LA;

    if( (i+1) < maxI ) 
      L << '.';
  }
  /*
  L = A->GetAtomInfo()->Symbol;
  L += '-';
  L += B->GetAtomInfo()->Symbol;*/

  if( Bnd.GetType() == sotHBond )  L << "@H";
  return L;
}
//..............................................................................
void TXBond::Radius(float V)  {
  Params()[4] = Primitives()->Style()->GetParam("R", V).ToDouble();
  // update radius for all members of the collection
  for( int i=0; i < Primitives()->ObjectCount(); i++ )
    Primitives()->Object(i)->Params()[4] = V;
}
//..............................................................................
void TXBond::OnPrimitivesCleared()  {
  if( FStaticObjects.Count() )
    FStaticObjects.Clear();
}
//..............................................................................
void TXBond::ValidateBondParams()  {
  if( !FBondParams )  {
    FBondParams = TGlRenderer::GetStyles()->NewStyle("BondParams", true);
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
