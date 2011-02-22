//----------------------------------------------------------------------------//
// TXBond
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "xbond.h"
#include "gpcollection.h"
#include "xatom.h"
#include "lattice.h"
#include "symmparser.h"
#include "unitcell.h"

//..............................................................................
bool TXBond::TStylesClear::Enter(const IEObject *Sender, const IEObject *Data)  {
  TXBond::FBondParams = NULL;
  TXBond::ClearStaticObjects();
  return true;
}
//..............................................................................
bool TXBond::TStylesClear::Exit(const IEObject *Sender, const IEObject *Data)  {
  TXBond::ValidateBondParams();
  TXBond::ClearStaticObjects();
  return true;
}
//..............................................................................
//..............................................................................
TXBond::TContextClear::TContextClear(TGlRenderer& Render)  {
  Render.OnClear.Add(this);
}
//..............................................................................
bool TXBond::TContextClear::Enter(const IEObject *Sender, const IEObject *Data)  {
  TXBond::ClearStaticObjects();
  return true;
}
//..............................................................................
bool TXBond::TContextClear::Exit(const IEObject *Sender, const IEObject *Data)  {
  return true;
}
//..............................................................................
//----------------------------------------------------------------------------//
// TXBond function bodies
//----------------------------------------------------------------------------//
TStrPObjList<olxstr,TGlPrimitive*> TXBond::FStaticObjects;
TArrayList<TGlPrimitiveParams>  TXBond::FPrimitiveParams;
TGraphicsStyle* TXBond::FBondParams=NULL;
TXBond::TStylesClear *TXBond::OnStylesClear=NULL;
//..............................................................................
TXBond::TXBond(TNetwork* net, TGlRenderer& R, const olxstr& collectionName) :
  TSBond(net),
  AGDrawObject(R, collectionName),
  FDrawStyle(0x0001)
{
  SetGroupable(true);
  Params().Resize(5);
  Params()[4] = 0.8;
  Label = new TXGlLabel(GetParent(), PLabelsCollectionName);
  Label->SetVisible(false);
  if( OnStylesClear == NULL )  {
    OnStylesClear = new TStylesClear(R);
    new TContextClear(R);
  }
}
//..............................................................................
TXBond::~TXBond()  {
  if( GetParentGroup() != NULL )  {
    GetParentGroup()->Remove(*this);
#ifdef _DEBUG
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
#endif
  }
  delete Label;
}
//..............................................................................
void TXBond::Update()  {
  if( !IsValid() )  return;
  vec3d C(B().crd() - A().crd());
  if( C.IsNull() )  
    Params().Null();
  else  {
    Params()[3] = C.Length();
    C.Normalise();
    Params()[0] = acos(C[2])*180/M_PI;
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
void TXBond::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  if( FStaticObjects.IsEmpty() )  
    CreateStaticObjects(Parent);
  if( IsValid() && Label->GetOffset().IsNull() )  // init label offset
    Label->SetOffset((A().crd()+B().crd())/2);
  Label->SetFontIndex(Parent.GetScene().FindFontIndexForType<TXBond>());
  Label->Create();
  // find collection
  olxstr NewL;
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
    (GetType() == sotHBond) ? 2048 : DefMask(), IsMaskSaveable()).ToInt();

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
      GlP.Params.GetLast() = SGlP->Params.GetLast();

      GlP.StartList();
      GlP.CallList(SGlP);
      GlP.EndList();
      TGlMaterial* style_mat = GS.FindMaterial(FStaticObjects[i]);
      if( IsValid() )  {
        if( style_mat != NULL )
          GlP.SetProperties(*style_mat);
        else  {
          TGlMaterial RGlM;
          if( SGlP->Params.GetLast() == ddsDefAtomA || SGlP->Params.GetLast() == ddsDef )  {
            const size_t mi = A().Style().IndexOfMaterial("Sphere");
            if( mi != InvalidIndex )
              RGlM = A().Style().GetPrimitiveStyle(mi).GetProperties();
            else
              TXAtom::GetDefSphereMaterial(A(), RGlM);
          }
          else if( SGlP->Params.GetLast() == ddsDefAtomB )  {
            const size_t mi = B().Style().IndexOfMaterial("Sphere");
            if( mi != InvalidIndex )
              RGlM = B().Style().GetPrimitiveStyle(mi).GetProperties();
            else
              TXAtom::GetDefSphereMaterial(B(), RGlM);
          }
          GlP.SetProperties(GS.GetMaterial(FStaticObjects[i], RGlM));
        }
      }
      else  {  // no atoms
        GlP.SetProperties(GS.GetMaterial(FStaticObjects[i],
          TGlMaterial("85;2155839359;2155313015;1.000,1.000,1.000,0.502;36")));
      }
    }
  }
}
//..............................................................................
bool TXBond::Orient(TGlPrimitive& GlP)  {
  olx_gl::translate(A().crd());
  olx_gl::rotate(Params()[0], Params()[1], Params()[2], 0.0);
  olx_gl::scale(Params()[4], Params()[4], Params()[3]);
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
void TXBond::CreateStaticObjects(TGlRenderer& Parent)  {
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
  GlP->Params.GetLast() = ddsDefAtomA;
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
  olx_gl::translate(0, 0, 1);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
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
  GlP->Params.GetLast() = ddsDefAtomA;
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
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();

  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
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
  GlP->Params.GetLast() = ddsDefAtomA;
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
  olx_gl::translate(0.0f, 0.0f, 0.5f);
  GlP->CallList(GlPRC1);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
//..............................
  // create bottom line
  if( (GlP = FStaticObjects.FindObject("Bottom line")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Bottom line", GlP);
  }
  GlP->StartList();
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0, 0, 0);
      olx_gl::vertex(0.0f, 0.0f, 0.5f);
    olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create top line
  if( (GlP = FStaticObjects.FindObject("Top line")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Top line", GlP);
  }
  GlP->StartList();
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0.0f, 0.0f, 0.5f);
      olx_gl::vertex(0, 0, 1);
    olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;
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
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0f, 0.0f, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDef;
  //..............................
  if( (GlP = FStaticObjects.FindObject("Bottom stipple cone")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Bottom stipple cone", GlP);
  }
  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, CL/2);
  for( int i=0; i < ConeStipples/2; i++ )  {
    if( i != 0 )
      GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0f, 0.0f, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
  //..............................
  if( (GlP = FStaticObjects.FindObject("Top stipple cone")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Top stipple cone", GlP);
  }
  GlP->StartList();
  olx_gl::translate(0.0f, 0.0f, (float)(0.5 + CL/2));
  for( int i=0; i < ConeStipples/2; i++ )  {
    GlP->CallList(GlPRD2);
    GlP->CallList(GlPRC1);
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRD1);
    olx_gl::translate(0.0f, 0.0f, CL);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomB;

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
    olx_gl::translate(0.0f, 0.0f, CL);
    GlP->CallList(GlPRC1);
  }
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDef;
//..............................
  // create line
  if( (GlP = FStaticObjects.FindObject("Line")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Line", GlP);
  }
  GlP->StartList();
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0, 0, 0);
      olx_gl::vertex(0, 0, 1);
    olx_gl::end();
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
//..............................
  // create stippled line
  if( (GlP = FStaticObjects.FindObject("Stippled line")) == NULL )  {
    GlP = &Parent.NewPrimitive(sgloCommandList);
    FStaticObjects.Add("Stippled line", GlP);
  }
  GlP->StartList();
    olx_gl::enable(GL_LINE_STIPPLE);
    olx_gl::lineStipple(1, 0xf0f0);
    olx_gl::begin(GL_LINES);
      olx_gl::vertex(0, 0, 0);
      olx_gl::vertex(0, 0, 1);
    olx_gl::end();
  olx_gl::disable(GL_LINE_STIPPLE);
  GlP->EndList();
  GlP->Params.Resize(GlP->Params.Count()+1);  //
  GlP->Params.GetLast() = ddsDefAtomA;
}
//..............................................................................
olxstr TXBond::GetLegend(const TSBond& Bnd, const short level)  {
  olxstr L(EmptyString(), 32);
  const TSAtom *A = &Bnd.A(),
               *B = &Bnd.B();
  if( A->GetType() != B->GetType() )  {
    if( A->GetType() < B->GetType() )
      olx_swap(A, B);
  }
  else  {
    if( A->GetLabel().Compare(B->GetLabel()) < 0 )
      olx_swap(A, B);
  }
  L << A->GetType().symbol << '-' << B->GetType().symbol;
  if( Bnd.GetType() == sotHBond )  
    L << "@H";
  if( level == 0 )  return L;
  L << '.' << A->GetLabel() << '-' << B->GetLabel();
  if( level == 1 )  return L;
  TUnitCell::SymSpace sp = A->GetNetwork().GetLattice().GetUnitCell().GetSymSpace();
  L << '.' << TSymmParser::MatrixToSymmCode(sp, A->GetMatrix(0)) <<
    '-' <<
    TSymmParser::MatrixToSymmCode(sp, B->GetMatrix(0));
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
    (GetType() == sotHBond) ? 2048 : DefMask(), IsMaskSaveable()).ToUInt();
}
//..............................................................................
void TXBond::OnPrimitivesCleared()  {
  if( !FStaticObjects.IsEmpty() )
    FStaticObjects.Clear();
}
//..............................................................................
void TXBond::ValidateBondParams()  {
  if( FBondParams == NULL )  {
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
bool TXBond::OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseDown(Sender, Data) : false;
}
//..............................................................................
bool TXBond::OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseMove(Sender, Data) : false;
}
//..............................................................................
bool TXBond::OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {
  return Label->IsVisible() ? Label->OnMouseMove(Sender, Data) : false;
}
//..............................................................................
