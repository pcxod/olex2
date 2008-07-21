//----------------------------------------------------------------------------//
// namespace TEXLib
// TGXApp - a wraper for basic crystallographic graphic application
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "dunitcell.h"
#include "dbasis.h"
#include "xreflection.h"
#include "gllabel.h"
#include "gllabels.h"
#include "xplane.h"
#include "xline.h"
#include "xgrowline.h"
#include "xgrowpoint.h"
#include "xgrid.h"

#include "globj.h"
#include "gxapp.h"
#include "log.h"
//#include "wglscene.h"
#include "xeval.h"

#include "network.h"
#include "unitcell.h"
#include "symmparser.h"

#include "hkl.h"

#include "efile.h"
#include "ecast.h"

#include "xlattice.h"

#include "etime.h"

#include "gpcollection.h"

#ifdef __WXWIDGETS__
  #include "wxglscene.h"
  #include "wx/string.h"
  #include "wx/fontutil.h"
#endif
#define ConeStipple  6.0
#define LineStipple  0xf0f0

int CompareStr(const olxstr &Str, const olxstr &Str1, bool IC) {
  int minl = olx_min(Str1.Length(), Str.Length());
  int diff1;
  for( int i = 0; i < minl; i++ )  {
    int diff = Str[i] - Str1[i];
    if( olxstr::o_isdigit(Str[i]) )  {
      if( olxstr::o_isdigit(Str1[i]) )  {
        olxstr S, S1;
        int j = i, k = i;
        while( (j < Str.Length()) && olxstr::o_isdigit(Str[j]) )  {
          S << Str[j];  j++;
        }
        while( (k < Str1.Length()) && olxstr::o_isdigit(Str1[k]) )  {
          S1 << Str[k];  k++;
        }
        diff1 = S.ToInt() - S1.ToInt();
        if( diff1 != 0 )  return diff1;
        // if the number of digits different - diff1 != 0, so now k=j
        i = k-1;
      }
    }
    else  {
      diff = IC ? (olxstr::o_toupper(Str[i]) - olxstr::o_toupper(Str1[i])) :
                 (Str[i] - Str1[i]);
      if( diff != 0 )  return 0;
    }
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// UNDO DATA CLASSES
class TKillUndo: public TUndoData  {
  TSBondPList* SBonds;
  TSAtomPList* SAtoms;
  TSPlanePList* SPlanes;
public:
  TKillUndo(IUndoAction *action):TUndoData(action)  {
    SBonds = NULL;
    SAtoms = NULL;
    SPlanes = NULL;
  }

  virtual ~TKillUndo()  {
    if( SBonds )  delete SBonds;
    if( SAtoms )  delete SAtoms;
    if( SPlanes )  delete SPlanes;
  }

  int SBondCount()  {
    if( !SBonds )  return 0;
    return SBonds->Count();
  }
  int SAtomCount()  {
    if( !SAtoms )  return 0;
    return SAtoms->Count();
  }
  int SPlaneCount()  {
    if( !SPlanes ) return 0;
    return SPlanes->Count();
  }
  TSBond& SBond(int i)    {  return *SBonds->Item(i);    }
  TSAtom& SAtom(int i)    {  return *SAtoms->Item(i);    }
  TSPlane& SPlane(int i)  {  return *SPlanes->Item(i);  }

  TSAtomPList& GetSAtoms()  {  return *SAtoms;  }
  TSBondPList& GetSBonds()  {  return *SBonds;  }
  TSPlanePList& GetSPlanes() {  return *SPlanes;  }

  void AddSAtom(TSAtom& SA)  {
    if( !SAtoms )  SAtoms = new TSAtomPList();
    SAtoms->Add(&SA);
  }
  void AddSBond(TSBond& SB)  {
    if( !SBonds )  SBonds = new TSBondPList();
    SBonds->Add(&SB);
  }
  void AddSPlane(TSPlane& SP)  {
    if( !SPlanes )  SPlanes = new TSPlanePList();
    SPlanes->Add(&SP);
  }

  void AddSAtoms(const TSAtomPList& atoms)  {
    if( !SAtoms )  SAtoms = new TSAtomPList();
    SAtoms->AddList(atoms);
  }
  void AddSBonds(const TSBondPList& bonds)  {
    if( !SBonds )  SBonds = new TSBondPList();
    SBonds->AddList(bonds);
  }
  void AddSPlanes(const TSPlanePList& planes)  {
    if( !SPlanes )  SPlanes = new TSPlanePList();
    SPlanes->AddList(planes);
  }
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//..............................................................................
TXBondStylesClear::~TXBondStylesClear()  {  ;  }

class xappXFileLoad: public AActionHandler  {
  TGXApp *FParent;
  TEBasis B;
public:
  xappXFileLoad(TGXApp *Parent) {  FParent = Parent;  }
  ~xappXFileLoad()  {  return;  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {  return false;  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    FParent->ClearLabels();
    // make sure that these are only cleared when file is loaded
    if( Sender && EsdlInstanceOf(*Sender, TXFile) )  {
      FParent->ClearIndividualCollections();
      FParent->DUnitCell().ResetCentres();
      //FParent->XGrid().Clear();
    }
    B = FParent->GetRender().GetBasis();
    FParent->GetRender().Clear();
    FParent->HklFile().Clear();
    FParent->ClearSelectionCopy();
    return true;
  }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    // lets make Horst HAPPY!
    //FParent->GetRender().CleanUpStyles();
    //  FParent->CenterModel();
    FParent->GetRender().SetBasis(B);
    FParent->CreateObjects(true);
    FParent->CenterView();
    FParent->GetRender().SetZoom( FParent->GetRender().CalcZoom()*FParent->GetExtraZoom() );
    //FParent->CenterModel();
    //FParent->GetRender().Compile();
    FParent->Draw();
    return true;
  }
};
//..............................................................................
//----------------------------------------------------------------------------//
//TGXApp function bodies
//----------------------------------------------------------------------------//
enum  {
  ID_OnSelect = 1
};

TGXApp::TGXApp(const olxstr &FileName):TXApp(FileName)  {
  FStructureVisible = FQPeaksVisible = FHydrogensVisible =  FHBondsVisible = true;
  XGrowPointsVisible = FXGrowLinesVisible = FQPeakBondsVisible = false;

  TwxGlScene *GlScene = new TwxGlScene( BaseDir() + "etc/Fonts/" );
  FGrowMode = gmCovalent;
//  TWGlScene *GlScene = new TWGlScene;
//  TGlScene *GlScene = new TGlScene;
  FGlRender = new TGlRender(GlScene, 1,1);
  FDFrame = new TDFrame("DFrame", FGlRender);
  Fader = new TXFader("Fader", FGlRender);
  FDFrame->OnSelect->Add(this, ID_OnSelect);
  FGlMouse = new TGlMouse(FGlRender, FDFrame);
  FDUnitCell = new TDUnitCell("DUnitCell", FGlRender);
  FDUnitCell->Visible(false);
  FDBasis = new TDBasis("DBasis", FGlRender);
  FDBasis->Visible(false);
  FProbFactor = 50;
  ExtraZoom = 1.25;

  FLabels = new TXGlLabels("Labels", FGlRender);

  ObjectsToCreate.Add( FDBasis );
  ObjectsToCreate.Add( FDUnitCell );
  ObjectsToCreate.Add( FDFrame );
  ObjectsToCreate.Add( Fader );

  FHklFile = new THklFile();
  FHklVisible = false;

  FXGrid = new TXGrid("XGrid", this);

  xappXFileLoad *P = new xappXFileLoad(this);
  XFile().GetLattice().OnStructureGrow->Add(P);
  XFile().GetLattice().OnStructureUniq->Add(P);
  XFile().OnFileLoad->Add(P);

  OnGraphicsVisible = &NewActionQueue("GRVISIBLE");
  OnFragmentVisible = &NewActionQueue("FRVISIBLE");
  OnAllVisible = &NewActionQueue("ALLVISIBLE");
  OnObjectsDestroy = &NewActionQueue("OBJECTSDESTROY");
}
//..............................................................................
TGXApp::~TGXApp()  {
  Clear();
  delete FGlRender;
  delete FLabels;
  delete FHklFile;
  delete FXGrid;
  delete FGlMouse;
}
//..............................................................................
void TGXApp::ClearXObjects()  {
  OnObjectsDestroy->Execute( dynamic_cast<TBasicApp*>(this), NULL);
  XAtoms.Clear();
  XBonds.Clear();
  XGrowLines.Clear();
  XGrowPoints.Clear();
  XReflections.Clear();
  XPlanes.Clear();
}
//..............................................................................
void TGXApp::Clear()  {
  FGlRender->Selection()->Clear();
  FGlRender->ClearGroups();
  ClearXObjects();
  
  for( int i=0; i < LooseObjects.Count(); i++ )  
    delete LooseObjects[i];   
  LooseObjects.Clear();

  for( int i=0; i < ObjectsToCreate.Count(); i++ )
    delete ObjectsToCreate[i];
  ObjectsToCreate.Clear();

  XLabels.Clear();
  GlBitmaps.Clear();
  ClearGroups();

}
//..............................................................................
void TGXApp::CreateXRefs()  {
  if( XReflections.Count() != 0 )  return;

  TRefList refs;
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(FXFile->GetAsymmUnit());
  if( sg == NULL ) 
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate sapce group");
  THklFile::MergeStats ms = FHklFile->Merge( *sg, false, refs);

  vec3d Center;
  for( int i=0; i < refs.Count(); i++ )  {
    TXReflection* xr = new TXReflection("XReflection", *FHklFile, refs[i],
      &FXFile->GetAsymmUnit(), FGlRender);
    xr->Create();
    XReflections.Add( *xr );
    Center += xr->Center();
  }
  if( refs.Count() )  Center /= refs.Count();
  Center += FGlRender->GetBasis().GetCenter();
  for( int i=0; i < XReflections.Count(); i++ )
    XReflections[i].Center() -= Center;
}
//..............................................................................
int TGXApp::GetNetworks(TNetPList& nets) {
  int c = XFile().GetLattice().FragmentCount();
  for( int i=0; i < c; i++ )
    nets.Add( &XFile().GetLattice().GetFragment(i) );

  for( int i=0; i < OverlayedXFiles.Count(); i++ )  {
    int fc = OverlayedXFiles[i].GetLattice().FragmentCount();
    c += fc;
    for( int j=0; j < fc; j++ )
      nets.Add( &OverlayedXFiles[i].GetLattice().GetFragment(j) );
  }
  return c;
}
//..............................................................................
void TGXApp::CreateObjects(bool SyncBonds, bool centerModel)  {
  int64_t st = TETime::msNow();
  GetLog().Info("Start xobject creation");

  vec3d glMax, glMin, glCenter;
  glMax = FGlRender->MaxDim();
  glMin = FGlRender->MinDim();
  glCenter = FGlRender->GetBasis().GetCenter();
  TXAtom::FStaticObjects.Clear();
  TXBond::FStaticObjects.Clear();
  FGlRender->ClearGroups();
  FGlRender->ClearPrimitives();
  FLabels->Clear();
  ClearXObjects();
  FGlRender->SetSceneComplete(false);

  for( int i=0; i < FIndividualCollections.Count(); i++ )
    FGlRender->NewCollection( FIndividualCollections.String(i) );

  TSAtomPList allAtoms;
  int totalACount = XFile().GetLattice().AtomCount();
  for( int i=0; i < OverlayedXFiles.Count(); i++ )
    totalACount += OverlayedXFiles[i].GetLattice().AtomCount();
  allAtoms.SetCapacity( totalACount );

  TSBondPList allBonds;
  int totalBCount = XFile().GetLattice().BondCount();
  for( int i=0; i < OverlayedXFiles.Count(); i++ )
    totalBCount += OverlayedXFiles[i].GetLattice().BondCount();
  allBonds.SetCapacity( totalBCount );

  for( int i=0; i < XFile().GetLattice().AtomCount(); i++ )
    allAtoms.Add( &XFile().GetLattice().GetAtom(i) );
  for( int i=0; i < OverlayedXFiles.Count(); i++ )  {
    for( int j=0; j < OverlayedXFiles[i].GetLattice().AtomCount(); j++ )
      allAtoms.Add( &OverlayedXFiles[i].GetLattice().GetAtom(j) );
  }

  for( int i=0; i < XFile().GetLattice().BondCount(); i++ )
    allBonds.Add( &XFile().GetLattice().GetBond(i) );
  for( int i=0; i < OverlayedXFiles.Count(); i++ )  {
    for( int j=0; j < OverlayedXFiles[i].GetLattice().BondCount(); j++ )
      allBonds.Add( &OverlayedXFiles[i].GetLattice().GetBond(j) );
  }
  XAtoms.SetCapacity( allAtoms.Count() );
  GetRender().SetObjectsCapacity( allAtoms.Count() + allBonds.Count() + 512);
  for( int i=0; i < allAtoms.Count(); i++ )  {
    allAtoms[i]->SetTag(i);
    TXAtom& XA = XAtoms.Add( *(new TXAtom(EmptyString, *allAtoms[i], FGlRender)) );
    if( allAtoms[i]->IsDeleted() )  XA.Deleted(true);
    XA.Create();
    if( !FStructureVisible )  {  XA.Visible(FStructureVisible);  continue;  }
    if( allAtoms[i]->GetAtomInfo() == iHydrogenIndex )    {  XA.Visible(FHydrogensVisible);  }
    if( allAtoms[i]->GetAtomInfo() == iQPeakIndex )       {  XA.Visible(FQPeaksVisible);  }
  }
  TBasicApp::GetLog().Info( olxstr("Atoms created in ") << (TETime::msNow()-st) << "ms" );
  XBonds.SetCapacity( allBonds.Count() );
  for( int i=0; i < allBonds.Count(); i++ )  {
    TSBond* B = allBonds[i];
    TXAtom& XA = XAtoms[ B->A().GetTag() ];
    TXAtom& XA1 = XAtoms[ B->B().GetTag() ];
    TXBond& XB = XBonds.Add( *(new TXBond(TXBond::GetLegend( *B, TXAtom::LegendLevel(XA.Primitives()->Name()),
                TXAtom::LegendLevel(XA1.Primitives()->Name())), *allBonds[i], FGlRender)) );
    if( B->IsDeleted() || (B->A().IsDeleted() || allBonds[i]->B().IsDeleted()) )
      XB.Deleted(true);
    XB.Create();
    if( (B->A().GetAtomInfo() == iQPeakIndex) ||
        (B->B().GetAtomInfo() == iQPeakIndex)  ) {  XB.Visible(FQPeakBondsVisible);  continue;  }
    if( !FStructureVisible )  {  XB.Visible(FStructureVisible);  continue;  }
    if( (B->A().GetAtomInfo() == iHydrogenIndex) ||
        (B->B().GetAtomInfo() == iHydrogenIndex)  )
    {
      if( B->GetType() == sotHBond )  XB.Visible(FHBondsVisible);
      else                            XB.Visible( FHydrogensVisible );
    }
  }
  TBasicApp::GetLog().Info( olxstr("Bonds created in ") << (TETime::msNow()-st) << "ms" );
  for( int i=0; i < FXFile->GetLattice().PlaneCount(); i++ )  {
    TSPlane& P = FXFile->GetLattice().GetPlane(i);
    TXPlane& XP = XPlanes.AddNew(EsdlClassName(TXPlane) + i, &P, FGlRender);
    if( P.IsDeleted() )  XP.Deleted(true);
    XP.Create();
  }
  double cell[6];
  cell[0] = XFile().GetAsymmUnit().Axes()[0].GetV();
  cell[1] = XFile().GetAsymmUnit().Axes()[1].GetV();
  cell[2] = XFile().GetAsymmUnit().Axes()[2].GetV();
  cell[3] = XFile().GetAsymmUnit().Angles()[0].GetV();
  cell[4] = XFile().GetAsymmUnit().Angles()[1].GetV();
  cell[5] = XFile().GetAsymmUnit().Angles()[2].GetV();
  DUnitCell().Init( cell );
  DBasis().AsymmUnit( &XFile().GetAsymmUnit() );
  for( int i=0; i < ObjectsToCreate.Count(); i++ )
    ObjectsToCreate[i]->Create();

  for( int i=0; i < LooseObjects.Count(); i++ )  {
    if( LooseObjects[i]->Deleted() )  {
      delete LooseObjects[i];
      LooseObjects.Delete(i);
      i--;
      continue;
    }
    LooseObjects[i]->Create();
  }

  InitLabels();
  FLabels->Create();

  if( FXGrowLinesVisible )  CreateXGrowLines();
  if( XGrowPointsVisible )  CreateXGrowPoints();

  FXGrid->Create();

  // create hkls
  if( FHklVisible )  HklVisible(true);

  if( SyncBonds )  XAtomDS2XBondDS("Sphere");

  if( centerModel )
    CenterModel();
  else  {
   FGlRender->ClearMinMax();
   FGlRender->UpdateMaxMin(glMin, glMax);
   FGlRender->Basis()->SetCenter( glCenter );
  }

  GetRender().Selection()->Create();
  GetRender().LoadIdentity();
  GetRender().SetView();
  GetRender().Initialise();
  FGlRender->SetSceneComplete(true);
  TBasicApp::GetLog().Info( olxstr("Completed in ") << (TETime::msNow()-st) << "ms" );
}
//..............................................................................
void TGXApp::CenterModel()  {
  int ac;
  double aan = 0;
  vec3d Center;
  ac = FXFile->GetLattice().AtomCount();
  if( !ac )  return;

  for( int i=0; i < ac; i++ )  {
    TSAtom& A = FXFile->GetLattice().GetAtom(i);
    if( !A.IsDeleted() )  {
      Center += A.crd()*A.CAtom().GetOccp();
      aan += A.CAtom().GetOccp();
    }
  }
  if( aan == 0 )  return;
  Center /= aan;

  Center *= -1;
  FGlRender->Basis()->SetCenter( Center );
  vec3d max = FGlRender->MaxDim();
  vec3d min = FGlRender->MinDim();
  max -= Center;
  min -= Center;
  FGlRender->ClearMinMax();
  FGlRender->UpdateMaxMin(max, min);
}
//..............................................................................
void TGXApp::CenterView()  {
  int aan = 0;
  vec3d Center,
           maX(-100, -100, -100),
           miN(100, 100, 100);
  if( FXFile->GetLattice().AtomCount() == 0 )  return;
  for( int i=0; i < XAtoms.Count(); i++ )  {
    TXAtom& XA = XAtoms[i];
    if( !XA.Deleted() && XA.Visible() )  {
      for( int j=0; j < 3; j++ )  {
        if( XA.Atom().crd()[j] > maX[j] )
          maX[j] = XA.Atom().crd()[j];
        if( XA.Atom().crd()[j] < miN[j] )
          miN[j] = XA.Atom().crd()[j];
      }
      Center += XA.Atom().crd();
      aan ++;
    }
  }
  if( aan == 0 )  return;

  Center /= (float)aan;
  Center *= -1;
  maX += Center;
  miN += Center;
  FGlRender->ClearMinMax();
  FGlRender->UpdateMaxMin(maX, miN);
//  FGlRender->Basis()->SetZoom( FGlRender->CalcZoom() );
  FGlRender->Basis()->SetCenter( Center );
}
//..............................................................................
void TGXApp::CalcProbFactor(float Prob)  {
  if( Prob < 0 )  Prob = FProbFactor;
  TXAtom::TelpProb(ProbFactor(Prob));//, Prob50 = ProbFactor(50);
  FProbFactor = Prob;
//  AtomZoom(ProbFactor);
}
//..............................................................................
float TGXApp::ProbFactor(float Prob)  {
  Prob /= 100;
  double ProbFactor = 0, Inc = 0.0001, Summ = 0, TVal = Prob * 15.75, Var;
  while( Summ < TVal && ProbFactor <= 10 )
  {
    Var = ProbFactor + Inc/2;
    Summ += 4*M_PI*exp(-Var*Var/2)*Var*Var*Inc;
    ProbFactor += Inc;
  }
  return ProbFactor;
}
//..............................................................................
void TGXApp::Init()  {
  CreateObjects(true);
}
//..............................................................................
void TGXApp::Quality(const short V)  {
  if( XAtoms.Count() )  {
    XAtoms[0].Quality(V);
    XAtoms[0].CreateStaticPrimitives();
  }
  if( XBonds.Count() )  {
    XBonds[0].Quality(V);
    XBonds[0].CreateStaticPrimitives();
  }
  Draw();
}
//..............................................................................
bool TGXApp::IsCellVisible()  const {
  return IsGraphicsVisible(FDUnitCell);
}
//..............................................................................
void TGXApp::SetCellVisible( bool v)  {
  SetGraphicsVisible(FDUnitCell, v);
}
//..............................................................................
bool TGXApp::IsBasisVisible() const {
  return IsGraphicsVisible(FDBasis);
}
//..............................................................................
void TGXApp::SetBasisVisible( bool v)  {
  SetGraphicsVisible(FDBasis, v);
}
//..............................................................................
TUndoData* TGXApp::SetGraphicsVisible( AGDrawObject *G, bool v )  {
  if( v != IsGraphicsVisible(G) )  {
    if( EsdlInstanceOf(*G, TXAtom)  )  {
       TXAtomPList L;  L.Add( (TXAtom*)G );
       return DeleteXAtoms(L);
    }
    if( EsdlInstanceOf(*G, TXBond) )  {
      TKillUndo *undo = new TKillUndo( new TUndoActionImpl<TGXApp>(this, &GxlObject(TGXApp::undoDelete)));
      undo->AddSBond( ((TXBond*)G)->Bond() );
      ((TXBond*)G)->Deleted(true);
      Draw();
      return undo;
    }
    if( EsdlInstanceOf(*G, TXPlane) )  {
      TKillUndo *undo = new TKillUndo( new TUndoActionImpl<TGXApp>(this, &GxlObject(TGXApp::undoDelete)));
      undo->AddSPlane( ((TXPlane*)G)->Plane() );
      ((TXPlane*)G)->Deleted(true);
      Draw();
      return undo;
    }
    G->Visible(v);
    OnGraphicsVisible->Execute(dynamic_cast<TBasicApp*>(this), G);
    Draw();
  }
  return NULL;
}
//..............................................................................
void TGXApp::BangTable(TXAtom *XA, TTTable<TStrList>& Table)
{
  TSAtom* A = &XA->Atom();
  float angle;
  TSBond *B, *B1;
  vec3d V, V1;
  Table.Resize(A->BondCount(), A->BondCount());

  Table.ColName(0) = A->GetLabel();
  for( int i=0; i < A->BondCount()-1; i++ )
    Table.ColName(i+1) = A->Bond(i).Another(*A).GetLabel();

  for( int i=0; i < A->BondCount(); i++ )  {
    B = &A->Bond(i);
    Table[i].String(0) = olxstr::FormatFloat(3, B->Length());
    Table.RowName(i) = B->Another(*A).GetLabel();
    for( int j=0; j < A->BondCount()-1; j++ )  {
      B1 = &A->Bond(j);

      if( i == j )  { Table[i].String(j+1) = '-'; continue; }
      if( i <= j )  { Table[i].String(j+1) = '-'; continue; }

      V = B->Another(*A).crd() - A->crd();
      V1 = B1->Another(*A).crd() - A->crd();
      if( V.QLength()*V1.QLength() != 0 )  {
        angle = V.CAngle(V1);
        angle = acos(angle)*180/M_PI;
        Table.Row(i)->String(j+1) = olxstr::FormatFloat(3, angle);
      }
      else
      { Table.Row(i)->String(j+1) = '-'; }
    }
  }
}
//..............................................................................
void TGXApp::BangList(TXAtom *XA, TStrList &L)  {
  float angle;
  TSAtom* A = &XA->Atom();
  olxstr T;
  vec3d V, V1;
  for( int i=0; i < A->BondCount(); i++ )  {
    TSBond& B = A->Bond(i);
    T = A->GetLabel();  T << '-'  << B.Another(*A).GetLabel();
    T << ": " << olxstr::FormatFloat(3, B.Length());
    L.Add(T);
  }
  for( int i=0; i < A->BondCount(); i++ )  {
    TSBond& B = A->Bond(i);
    for( int j=i+1; j < A->BondCount(); j++ )  {
      TSBond& B1 = A->Bond(j);

      T = B.Another(*A).GetLabel();  T << '-' << A->GetLabel() << '-';
      T << B1.Another(*A).GetLabel() << ": ";
      V = B.Another(*A).crd() - A->crd();
      V1 = B1.Another(*A).crd() - A->crd();
      if( V.Length() && V1.Length() )  {
        angle = V.CAngle(V1);
        angle = acos(angle)*180/M_PI;
        T << olxstr::FormatFloat(3, angle);
      }
      L.Add(T);
    }
  }
}
float TGXApp::Tang( TSBond *B1, TSBond *B2, TSBond *Middle, olxstr *Sequence )
{
  // right parameters should be passed, e.g. the bonds should be connecetd like
  // B1-Middle-B2 or B2-Middle->B1, otherwise the result is almost meaningless!
  TSAtom *A1, *A2, *A3, *A4;
  TSBond *bt;
  if( Middle->A() == B1->A() || Middle->A() == B1->B() )
    ;
  else
  {
    bt = B1;
    B1 = B2;
    B2 = bt; // swap bonds
  }
  // using next scheme : A1-A2-A3-A4
  A2 = &Middle->A();
  A3 = &Middle->B();
  A1 = &B1->Another(*A2);
  A4 = &B2->Another(*A3);
  vec3d A, B, C, D, E, F;
  A = A1->crd() - A2->crd();
  B = A3->crd() - A2->crd();
  C = A2->crd() - A3->crd();
  D = A4->crd() - A3->crd();

  E = A.XProdVec(B);
  F = C.XProdVec(D);

  if( !E.Length() || ! F.Length() )  return -1;

  float ca = E.CAngle(F), angle;
  angle = acos(ca);
  if( Sequence )
  {
    *Sequence = A1->GetLabel();
    *Sequence << '-' << A2->GetLabel() <<
                 '-' << A3->GetLabel() <<
                 '-' << A4->GetLabel();
  }
  return angle/M_PI*180;
}
void TGXApp::TangList(TXBond *XMiddle, TStrList &L)  {
  TSBondPList BondsA, BondsB;
  int maxl=0;
  TSAtom *A;
  TSBond *B, *Middle = &XMiddle->Bond();
  olxstr T;
  float angle;
  A = &Middle->A();
  for( int i=0; i < A->BondCount(); i++ )  {
    B = &A->Bond(i);
    if( B != Middle ) BondsA.Add(B);
  }
  A = &Middle->B();
  for( int i=0; i < A->BondCount(); i++ )  {
    B = &A->Bond(i);
    if( B != Middle ) BondsB.Add(B);
  }
  for( int i=0; i < BondsA.Count(); i++ )  {
    for( int j=0; j < BondsB.Count(); j++ )  {
      angle = Tang( BondsA[i], BondsB[j], Middle, &T);
      if( angle )  {
        T << ':' << ' ';
        if( T.Length() > maxl ) maxl = T.Length();  // to format thestring later
        T << olxstr::FormatFloat(3, angle);
        L.Add(T);
      }
    }
  }
  for( int i=0; i < L.Count(); i++ )  {
    int j = L.String(i).IndexOf(':');
    L.String(i).Insert(' ', j, maxl-j);  
  }
}
//..............................................................................
olxstr TGXApp::GetSelectionInfo()  {
  olxstr rv;
  double v;
  TGlGroup* Sel = FGlRender->Selection();
  if( Sel->Count() == 2 )  {
    if( EsdlInstanceOf(*Sel->Object(0), TXAtom) &&
      EsdlInstanceOf(*Sel->Object(1), TXAtom) )  {
        rv = "Distance: ";
        v = ((TXAtom*)Sel->Object(0))->Atom().crd().DistanceTo(
          ((TXAtom*)Sel->Object(1))->Atom().crd());
        rv << olxstr::FormatFloat(3, v);
    }
    else if( EsdlInstanceOf(*Sel->Object(0), TXBond) &&
      EsdlInstanceOf(*Sel->Object(1), TXBond) )  {
        rv = "Angle (bond-bond): ";
        vec3d V, V1;
        TXBond* A = (TXBond*)Sel->Object(0), *B =(TXBond*)Sel->Object(1);
        V = A->Bond().A().crd() - A->Bond().B().crd();
        V1 = B->Bond().A().crd() - B->Bond().B().crd();
        v = V.CAngle(V1);  v = acos(v)*180/M_PI;
        rv << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')';
        double distances[4];
        int minInd;
        distances[0] = A->Bond().A().crd().DistanceTo( B->Bond().A().crd() );
        distances[1] = A->Bond().A().crd().DistanceTo( B->Bond().B().crd() );
        distances[2] = A->Bond().B().crd().DistanceTo( B->Bond().A().crd() );
        distances[3] = A->Bond().B().crd().DistanceTo( B->Bond().B().crd() );

        evecd::ArrayMin(&distances[0], minInd, 4);
        // check if the adjastent bonds
        if( fabs(distances[minInd]) < 0.01 )  return rv;
        vec3d V2, V3, V4, V5;
        switch( minInd )  {
          case 0:
            V = A->Bond().B().crd() - A->Bond().A().crd();
            V1 = B->Bond().A().crd() - A->Bond().A().crd();
            V2 = B->Bond().B().crd() - B->Bond().A().crd();
            V3 = A->Bond().A().crd() - B->Bond().A().crd();
            break;
          case 1:
            V = A->Bond().B().crd() - A->Bond().A().crd();
            V1 = B->Bond().B().crd() - A->Bond().A().crd();
            V2 = B->Bond().A().crd() - B->Bond().B().crd();
            V3 = A->Bond().A().crd() - B->Bond().B().crd();
            break;
          case 2:
            V = A->Bond().A().crd() - A->Bond().B().crd();
            V1 = B->Bond().A().crd() - A->Bond().B().crd();
            V2 = B->Bond().B().crd() - B->Bond().A().crd();
            V3 = A->Bond().B().crd() - B->Bond().A().crd();
            break;
          case 3:
            V = A->Bond().A().crd() - A->Bond().B().crd();
            V1 = B->Bond().B().crd() - A->Bond().B().crd();
            V2 = B->Bond().A().crd() - B->Bond().B().crd();
            V3 = A->Bond().B().crd() - B->Bond().B().crd();
            break;
        }
        V4 = V.XProdVec(V1);
        V5 = V2.XProdVec(V3);
        if( V4.Length() != 0 && V5.Length() != 0 )  {
          rv << "\nTorsion angle (bond-bond, away from closest atoms): ";
          v = V4.CAngle(V5);  v = acos(v)*180/M_PI;
          rv << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')';
        }
    }
    else if( EsdlInstanceOf(*Sel->Object(0), TXPlane) &&
      EsdlInstanceOf(*Sel->Object(1), TXAtom) )  {
        rv = "Distance (plane-atom): ";
        v = ((TXPlane*)Sel->Object(0))->Plane().DistanceTo(((TXAtom*)Sel->Object(1))->Atom());
        rv << olxstr::FormatFloat(3, v);
        rv << "\nDistance (plane centroid-atom): ";
        v = ((TXPlane*)Sel->Object(0))->Plane().Center().DistanceTo(((TXAtom*)Sel->Object(1))->Atom().crd());
        rv << olxstr::FormatFloat(3, v);
    }
    else if( EsdlInstanceOf(*Sel->Object(0), TXAtom) &&
      EsdlInstanceOf(*Sel->Object(1), TXPlane) )  {
        rv = "Distance (plane-atom): ";
        v = ((TXPlane*)Sel->Object(1))->Plane().DistanceTo(((TXAtom*)Sel->Object(0))->Atom());
        rv << olxstr::FormatFloat(3, v);
        rv << "\nDistance (plane centroid-atom): ";
        v = ((TXPlane*)Sel->Object(1))->Plane().Center().DistanceTo(((TXAtom*)Sel->Object(0))->Atom().crd());
        rv << olxstr::FormatFloat(3, v);
    }
    else if( EsdlInstanceOf(*Sel->Object(0), TXBond) &&
      EsdlInstanceOf(*Sel->Object(1), TXPlane) )  {
        rv = "Angle (plane-bond): ";
        v = ((TXPlane*)Sel->Object(1))->Plane().Angle(((TXBond*)Sel->Object(0))->Bond());
        rv << olxstr::FormatFloat(3, v);
    }
    else if( EsdlInstanceOf(*Sel->Object(1), TXBond) &&
      EsdlInstanceOf(*Sel->Object(0), TXPlane) )  {
        rv = "Angle (plane-bond): ";
        v = ((TXPlane*)Sel->Object(0))->Plane().Angle(((TXBond*)Sel->Object(1))->Bond());
        rv << olxstr::FormatFloat(3, v);
    }
    else if( EsdlInstanceOf(*Sel->Object(1), TXPlane) &&
      EsdlInstanceOf(*Sel->Object(0), TXPlane) )  {
        rv = "Angle (plane-plane): ";
        v = ((TXPlane*)Sel->Object(0))->Plane().Angle(((TXPlane*)Sel->Object(1))->Plane());
        rv << olxstr::FormatFloat(3, v);
        rv << "\nDistance (plane centroid-plane centroid): ";
        v = ((TXPlane*)Sel->Object(0))->Plane().Center().DistanceTo(((TXPlane*)Sel->Object(1))->Plane().Center());
        rv << olxstr::FormatFloat(3, v);
    }
  }
  else if( Sel->Count() == 3 )  {
    if( EsdlInstanceOf(*Sel->Object(0), TXAtom) &&
      EsdlInstanceOf(*Sel->Object(1), TXAtom) &&
      EsdlInstanceOf(*Sel->Object(2), TXAtom) )  {
        rv = "Angle: ";
        vec3d V, V1;
        V = ((TXAtom*)Sel->Object(0))->Atom().crd() - ((TXAtom*)Sel->Object(1))->Atom().crd();
        V1 = ((TXAtom*)Sel->Object(2))->Atom().crd() - ((TXAtom*)Sel->Object(1))->Atom().crd();
        v = V.CAngle(V1);  v = acos(v)*180/M_PI;
        rv << olxstr::FormatFloat(3, v);
    }
  }
  else if( Sel->Count() == 4 )  {
    if( EsdlInstanceOf(*Sel->Object(0), TXAtom) &&
      EsdlInstanceOf(*Sel->Object(1), TXAtom) &&
      EsdlInstanceOf(*Sel->Object(2), TXAtom) &&
      EsdlInstanceOf(*Sel->Object(3), TXAtom) )  {
        vec3d V1, V2, V3, V4;
        V1 = ((TXAtom*)Sel->Object(0))->Atom().crd() - ((TXAtom*)Sel->Object(1))->Atom().crd();
        V2 = ((TXAtom*)Sel->Object(2))->Atom().crd() - ((TXAtom*)Sel->Object(1))->Atom().crd();
        V3 = ((TXAtom*)Sel->Object(3))->Atom().crd() - ((TXAtom*)Sel->Object(2))->Atom().crd();
        V4 = ((TXAtom*)Sel->Object(1))->Atom().crd() - ((TXAtom*)Sel->Object(2))->Atom().crd();

        V1 = V1.XProdVec(V2);
        V3 = V3.XProdVec(V4);
        if( V1.Length() != 0  && V3.Length() != 0 )  {
          rv = "Torsion angle: ";
          v = V1.CAngle(V3);  v = acos(v)*180/M_PI;
          rv << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')';
        }
    }
  }
  return rv;
}
//..............................................................................
void TGXApp::ChangeAtomType( TXAtom *A, const olxstr &Element)  {
/*  TBasicAtomInfo *BAI = FAtomsInfo->GetAtomInfo(Element);
  if( A->AtomInfo() == BAI )  return;

  TGPCollection *GPC;
  olxstr Legend = CreateAtomTypeLegend(A);

  GPC = FGlRender->Collection(Legend);
  GPC->RemoveObject(A);

  A->AtomInfo(BAI);
  Legend = CreateAtomTypeLegend(A);

  GPC = FGlRender->Collection(Legend);
  if( GPC ) GPC->AddObject(A);
  else
  {
    GPC = FGlRender->NewCollection(Legend);
    CreateAtom(GPC, A);
    A->Compile();
  }
  DrawStyle(-1, dynamic_cast<TBasicApp*>(this) );
  Draw();*/
}
//..............................................................................
void TGXApp::InvertFragments(const TXAtomPList& NetworkAtoms)  {
  smatd m;
  m.r.I();
  m.r *= -1;
  TransformFragments(NetworkAtoms, m);
}
//..............................................................................
void TGXApp::MoveFragments(const TXAtomPList& NetworkAtoms, const vec3d& v)  {
  smatd m;
  m.r.I();
  m.t = v;
  TransformFragments(NetworkAtoms, m);
}
//..............................................................................
void TGXApp::TransformFragments(const TXAtomPList& NetworkAtoms, const smatd& m)  {
  TSAtomPList SAtoms;
  TListCaster::POP(NetworkAtoms, SAtoms);
  XFile().GetLattice().TransformFragments(SAtoms, m);
}
//..............................................................................
void TGXApp::FragmentVisible(TNetwork *N, bool V)  {
  TSAtomPList SA;
  TXAtomPList XA;
//  OnFragmentVisible->Enter(dynamic_cast<TBasicApp*>(this), dynamic_cast<IEObject*>(N));
  SA.SetCapacity( N->NodeCount() );
  for( int i=0; i < N->NodeCount(); i++ )
    SA.Add( &N->Node(i) );
  SAtoms2XAtoms(SA, XA);
  for( int i=0; i < XA.Count(); i++ )
    XA[i]->Visible(V);

  TSBondPList SB;
  TXBondPList XB;
  SB.SetCapacity( N->BondCount() );
  for( int i=0; i < N->BondCount(); i++ )
    SB.Add( &N->Bond(i) );
  SBonds2XBonds(SB, XB);
  for( int i=0; i < XB.Count(); i++ )
    XB[i]->Visible(V);
//  OnFragmentVisible->Exit(dynamic_cast<TBasicApp*>(this), dynamic_cast<IEObject*>(N));
}
//..............................................................................
void TGXApp::FragmentsVisible(const TNetPList& Frags, bool V)  {
//  OnFragmentsVisible->Enter(this, dynamic_cast<IEObject*>(Frags));
  for( int i=0; i < Frags.Count(); i++ )  {
    FragmentVisible(Frags[i], V);
  }
//  OnFragmentsVisible->Exit(this, dynamic_cast<IEObject*>(Frags));
  Draw();
}
//..............................................................................
TGlGroup& TGXApp::GroupFragments(const TNetPList& Fragments, const olxstr groupName)  {
  GetRender().Selection()->Clear();
  TSAtomPList satoms;
  TXAtomPList xatoms;
  for( int i=0; i < Fragments.Count(); i++ )  {
    for( int j=0; j < Fragments[i]->NodeCount(); j++ )
      satoms.Add( &Fragments[i]->Node(j) );
  }
  if( satoms.IsEmpty() )  return *(TGlGroup*)NULL;
  SAtoms2XAtoms(satoms, xatoms);
  for( int i=0; i < xatoms.Count(); i++ )
    GetRender().Selection()->Add( xatoms[i] );
  return *GetRender().GroupSelection(groupName);
}
//..............................................................................
int TGXApp::InvertFragmentsList(const TNetPList& SF, TNetPList& Result)  {
  TLattice *L = &XFile().GetLattice();
  int fc=0;;
  for( int i=0; i < L->FragmentCount(); i++ )    L->GetFragment(i).SetTag(1);
  for( int i=0; i < SF.Count(); i++ )           SF[i]->SetTag(0);
  for( int i=0; i < L->FragmentCount(); i++ )  {
    if( L->GetFragment(i).GetTag() )  {
      Result.Add( &L->GetFragment(i) );
      fc++;
    }
  }
  return fc;
}
//..............................................................................
void TGXApp::AllVisible(bool V)  {
  OnFragmentVisible->Enter(dynamic_cast<TBasicApp*>(this), NULL);
  for( int i=0; i < XAtoms.Count(); i++ )
    XAtoms[i].Visible(V);

  for( int i=0; i < XBonds.Count(); i++ )
    XBonds[i].Visible(V);

  OnFragmentVisible->Exit(dynamic_cast<TBasicApp*>(this), NULL);
  Draw();
}
//..............................................................................
void TGXApp::Select(const vec3d& From, const vec3d& To )  {
  vec3d Cnt, Cnt1, AC;
  for( int i=0; i < XAtoms.Count(); i++ )  {
    TXAtom& XA = XAtoms[i];
    if( XA.Visible() )  {
      AC = XA.Atom().crd();
      AC += GetRender().GetBasis().GetCenter();
      Cnt = AC * GetRender().GetBasis().GetMatrix();
      if( Cnt[0] < To[0] && Cnt[1] < To[1] &&
          Cnt[0] > From[0] && Cnt[1] > From[1] )  {
        if( !XA.Selected() )  {
          if( XA.Primitives()->PrimitiveCount() )  GetRender().Select(&XA);
        }
      }
    }
  }
  for(int i=0; i < XBonds.Count(); i++ )  {
    TXBond& B = XBonds[i];
    if( B.Visible() )  {
      AC = B.Bond().A().crd();  AC += GetRender().GetBasis().GetCenter();
      Cnt  = AC * GetRender().GetBasis().GetMatrix();
      AC = B.Bond().B().crd();  AC += GetRender().GetBasis().GetCenter();
      Cnt1 = AC * GetRender().GetBasis().GetMatrix();
      if( Cnt[0] < To[0] && Cnt[1] < To[1] && Cnt[0] > From[0] && Cnt[1] > From[1] &&
          Cnt1[0] < To[0] && Cnt1[1] < To[1] && Cnt1[0] > From[0] && Cnt1[1] > From[1] )  {
        if( !B.Selected() )  {
          if( B.Primitives()->PrimitiveCount() )  GetRender().Select(&B);
        }
      }
    }
  }
  for(int i=0; i < XReflections.Count(); i++ )  {
    TXReflection& XR = XReflections[i];
    if( XR.Visible() )  {
      Cnt = GetRender().GetBasis().GetMatrix() * XR.Center();
      if( Cnt[0] < To[0] && Cnt[1] < To[1] &&
          Cnt[0] > From[0] && Cnt[1] > From[1] )  {
        if( !XR.Selected() )  GetRender().Select(&XR);
      }
    }
  }
  Draw();
}
//..............................................................................
bool TGXApp::Dispatch(int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data)  {
  switch( MsgId )  {
    case ID_OnSelect:
      const TSelectionInfo *SData = dynamic_cast<const TSelectionInfo*>(Data);
      if(  !(SData->From == SData->To) )
        Select(SData->From, SData->To);
      break;
  }
  return false;
}
//..............................................................................
void TGXApp::GetSelectedCAtoms(TCAtomPList& List, bool Clear)  {
  TXAtomPList xAtoms;
  GetSelectedXAtoms(xAtoms, Clear);
  List.SetCapacity( xAtoms.Count() );
  for( int i=0; i < xAtoms.Count(); i++ )
    List.Add( &xAtoms[i]->Atom().CAtom() );
}
//..............................................................................
void TGXApp::ClearSelectionCopy()  {  SelectionCopy.Clear();  }
//..............................................................................
void TGXApp::BackupSelection()  {
  TGlGroup *Sel = Selection();
  if( !Sel->Count() )  return;
  SelectionCopy.Clear();
  for( int i=0; i < Sel->Count(); i++ )
    SelectionCopy.Add( Sel->Object(i) );
}
//..............................................................................
void TGXApp::RestoreSelection()  {
  GetRender().SelectAll(false);
  for( int i=0; i < SelectionCopy.Count(); i++ )
    GetRender().Select( (AGDrawObject*)SelectionCopy[i] );
  Draw();
}
//..............................................................................
void TGXApp::GetSelectedXAtoms(TXAtomPList& List, bool Clear)  {
  TGlGroup *Sel = Selection();
  TTypeList<AGDrawObject*> S;
  S.AddACopy( (AGDrawObject*)Sel );
  for( int i=0; i < S.Count(); i++ )  {
    Sel = (TGlGroup*)S.Item(i);
    for( int j=0; j < Sel->Count(); j++ )  {
      AGDrawObject* GO = Sel->Object(j);
      if( GO->Deleted() )  continue;
      if( GO->Group() )  {  // another group
        S.AddACopy(GO);  continue;
      }
      if( EsdlInstanceOf(*GO, TXAtom) )  {
        List.Add( (TXAtom*)GO );
      }
    }
  }
  if( Clear )  SelectAll(false);
}
//..............................................................................
void TGXApp::CAtomsByType(const TBasicAtomInfo &AI, TCAtomPList& List)  {
  TAsymmUnit& AU= XFile().GetLattice().GetAsymmUnit();
  for( int i=0; i < AU.AtomCount(); i++ )  {
    if( AU.GetAtom(i).IsDeleted())  continue;
    if( AU.GetAtom(i).GetAtomInfo() == AI )  {
      List.Add( &AU.GetAtom(i) );
    }
  }
}
//..............................................................................
void TGXApp::XAtomsByType(const TBasicAtomInfo &AI, TXAtomPList& List) {
  for( int i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Deleted() || !XAtoms[i].Visible() )  continue;
    if( XAtoms[i].Atom().GetAtomInfo() == AI )  {
      List.Add( &XAtoms[i] );
    }
  }
}
//..............................................................................
void TGXApp::CAtomsByMask(const olxstr &StrMask, int Mask, TCAtomPList& List)  {
  bool found;
  if( StrMask.Length() > 32 )
    throw TInvalidArgumentException(__OlxSourceInfo, "mask is too long");
  olxstr Tmp, Name( olxstr::UpperCase(StrMask));
  TAsymmUnit& AU= XFile().GetLattice().GetAsymmUnit();
  for( int i=0; i < AU.AtomCount(); i++ )  {
    TCAtom& CA = AU.GetAtom(i);
    if( CA.IsDeleted() )  continue;
    if( CA.Label().Length() != Name.Length() )  continue;
    Tmp = CA.Label().UpperCase();
    found = true;
    for( int j=0; j < Name.Length(); j++ )  {
      if( !(Mask & (0x0001<<j)) )  {
        if( Name[j] != Tmp[j] )  {
          found = false;  break;
        }
      }
    }
    if( found )  {
      List.Add( &CA );
    }
  }
}
//..............................................................................
void TGXApp::GrowAtom(TXAtom *XA, bool Shell, TCAtomPList* Template)  {
  FXFile->GetLattice().GrowAtom(XA->Atom(), Shell, Template);
}
//..............................................................................
void TGXApp::Grow(const TXAtomPList& atoms, const smatd_list& matrices)  {
  TSAtomPList satoms;
  TListCaster::POP(atoms, satoms);
  FXFile->GetLattice().GrowAtoms( satoms, matrices);
}
//..............................................................................
bool TGXApp::AtomExpandable(TXAtom *XA)  {
  return FXFile->GetLattice().IsExpandable( XA->Atom() );
}
//..............................................................................
void TGXApp::GetXAtoms(const olxstr &AtomName, TXAtomPList& res)  {
  for( int i=0; i < XAtoms.Count(); i++ )  {
    if( !XAtoms[i].Atom().GetLabel().Comparei(AtomName) )  {
      if( !XAtoms[i].Deleted() && XAtoms[i].Visible() )  {
        res.Add( &XAtoms[i] );
      }
    }
  }
}
//..............................................................................
TXAtom* TGXApp::GetXAtom(const olxstr& AtomName, bool clearSelection)  {
  if( !AtomName.Comparei("sel" ) )  {
    TXAtomPList L;
    GetSelectedXAtoms(L, clearSelection);
    if( L.Count() != 1 )  return NULL;
    return L[0];
  }
  for( int i=0; i < XAtoms.Count(); i++ )
    if( !XAtoms[i].Atom().GetLabel().Comparei(AtomName) )
      return  &XAtoms[i];
  return NULL;
}
//..............................................................................
void TGXApp::XAtomsByMask(const olxstr &StrMask, int Mask, TXAtomPList& List)  {
  bool found;
  if( StrMask.Length() > 32 )
    throw TInvalidArgumentException(__OlxSourceInfo, "mask is too long");
  olxstr Tmp, Name( olxstr::UpperCase(StrMask) );
  for( int i=0; i < XAtoms.Count(); i++ )  {
    TXAtom& XA = XAtoms[i];
    if( XA.Deleted() || !XA.Visible() )  continue;
    if( XA.Atom().GetLabel().Length() != Name.Length() )  continue;
    Tmp = olxstr::UpperCase(XA.Atom().GetLabel());
    found = true;
    for( int j=0; j < Name.Length(); j++ )  {
      if( !(Mask & (0x0001<<j)) )  {
        if( Name[j] != Tmp[j] )  {
          found = false;  break;
        }
      }
    }
    if( found )  {
      List.Add( &XA );
    }
  }
}
//..............................................................................
void TGXApp::FindXAtoms(const olxstr &Atoms, TXAtomPList& List, bool ClearSelection)  {
  int ind, mask;
  TXAtom *XAFrom, *XATo;
  if( Atoms.IsEmpty() )  {  // return all atoms
    List.SetCapacity( List.Count() + XAtoms.Count() );
    for( int i=0; i < XAtoms.Count(); i++ )  {
      if( XAtoms[i].Deleted() || !XAtoms[i].Visible() ) continue;
      List.Add( &XAtoms[i] );
    }
    return;
  }
  TStrList Toks;
  olxstr Tmp;
  Toks.Strtok(Atoms, ' ');
  TBasicAtomInfo *BAI;
  for( int i = 0; i < Toks.Count(); i++ )  {
    Tmp = Toks[i];
    if( !Tmp.Comparei("sel") )  {
      GetSelectedXAtoms(List, ClearSelection);
      continue;
    }
    if( !Tmp.Comparei("to") || !Tmp.Comparei(">") )  {
      if( (i+1) < Toks.Count() && List.Count() )  {
        i++;
        XATo = NULL;
        if( !Toks.String(i).Comparei("end") )  ;
        else  {
          XATo = GetXAtom( Toks[i], ClearSelection );
          if( XATo == NULL )
            throw TInvalidArgumentException(__OlxSourceInfo, "\'to\' atoms is undefined");
        }
        XAFrom = List[List.Count()-1];
        for( int j=0; j < XAtoms.Count(); j++ )  {
          TXAtom& XA = XAtoms[j];
          if( XA.Deleted() || !XA.Visible() ) continue;
          if( XATo != NULL )  {
            if( CompareStr(XA.Atom().GetLabel(), XAFrom->Atom().GetLabel(), true) > 0 &&
                CompareStr(XA.Atom().GetLabel(), XATo->Atom().GetLabel(), true) < 0 )  List.Add( &XA );
          }
          else  {
            if( CompareStr(XA.Atom().GetLabel(), XAFrom->Atom().GetLabel(), true) > 0 &&
                XA.Atom().GetAtomInfo() == XAFrom->Atom().GetAtomInfo() )  List.Add( &XA );
          }
        }
        if( XATo != NULL )  List.Add( XATo );
      }
    }
    if( Tmp.CharAt(0) == '$' )  {
      Tmp = Tmp.SubStringFrom(1);
      if( Tmp.Length() )  {
        BAI = AtomsInfo()->FindAtomInfoBySymbol(Tmp);
        if( BAI == NULL )
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("atom type=") << Tmp);
        XAtomsByType(*BAI, List);
      }
      continue;
    }
    ind = Tmp.FirstIndexOf('?');
    if( ind >= 0 )  {
      mask = 0x0001 << ind;
      for( int j=ind+1; j < Tmp.Length(); j++ )  {
        ind = Tmp.FirstIndexOf('?', j);
        if( ind >=0 )  {
          mask |= 0x0001 << ind;
          j = ind;
         }
      }
      XAtomsByMask(Tmp, mask, List);
      continue;
    }
    GetXAtoms(Tmp, List);
  }
}
//..............................................................................
void TGXApp::CheckQBonds(TXAtom& XA)  {
  TSBondPList SB;
  TXBondPList XB;
  for(int i=0; i < XA.Atom().BondCount(); i++ )
    SB.Add( &XA.Atom().Bond(i) );
  SBonds2XBonds(SB, XB);
  for(int i=0; i < XB.Count(); i++ )  {
    TXBond* xb = XB[i];
    /* check if any of the atoms still a Q-peak */
    if( xb->Bond().A().GetAtomInfo() == iQPeakIndex ||
        xb->Bond().B().GetAtomInfo() == iQPeakIndex )  continue;
    /* check that the covalent bond really exists before showing it */
    xb->Visible( FXFile->GetLattice().GetNetwork().CBondExists(xb->Bond().A().CAtom(),
                    xb->Bond().B().CAtom(), xb->Bond().Length()) );
  }
}
//..............................................................................
TUndoData* TGXApp::ChangeSuffix(const TXAtomPList& xatoms, const olxstr &To, bool CheckLabels)  {
  TNameUndo *undo = new TNameUndo( new TUndoActionImpl<TGXApp>(this, &GxlObject(TGXApp::undoName)));
  olxstr oldL, newL;
  for( int i=0; i < xatoms.Count(); i++ )  {
    oldL = xatoms[i]->Atom().GetLabel();
    newL = xatoms[i]->Atom().GetAtomInfo().GetSymbol();
    for(int j=xatoms[i]->Atom().GetAtomInfo().GetSymbol().Length(); j < oldL.Length(); j++ )
      if( oldL[j] >= '0' && oldL[j] <= '9' )
        newL << oldL[j];
      else
        break;
    newL << To;
    if( newL == oldL )  continue;

    if( CheckLabels )
      newL = XFile().GetAsymmUnit().CheckLabel(&xatoms[i]->Atom().CAtom(), newL);
    xatoms[i]->Atom().CAtom().Label() = newL;
    undo->AddAtom( xatoms[i]->Atom(), oldL );
  }
  return undo;
}
//..............................................................................
TUndoData* TGXApp::Name(TXAtom& XA, const olxstr &Name, bool CheckLabel)  {
  bool checkBonds = (XA.Atom().GetAtomInfo() == iQPeakIndex);
  TBasicAtomInfo *bai = FAtomsInfo->FindAtomInfoEx(Name);

  TNameUndo *undo = new TNameUndo( new TUndoActionImpl<TGXApp>(this, &GxlObject(TGXApp::undoName)));
  olxstr oldL = XA.Atom().GetLabel();

  bool recreate = ((bai == NULL) ? true : XA.Atom().GetAtomInfo() != *bai);

  XA.Atom().SetLabel( CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA.Atom().CAtom(), Name) : Name);

  if( oldL != XA.Atom().GetLabel() )
    undo->AddAtom( XA.Atom(), oldL );

  NameHydrogens(XA.Atom(), undo, CheckLabel);
  if( checkBonds )  CheckQBonds(XA);
  if( recreate )  {
    XA.Primitives()->RemoveObject(&XA);
    XA.Create();
    TXAtomPList atoms;
    atoms.Add( &XA );
    SynchroniseBonds( atoms );
  }
  return undo;
}
//..............................................................................
TUndoData* TGXApp::Name(const olxstr &From, const olxstr &To, bool CheckLabel, bool ClearSelection)  {
  TXAtom* XA = GetXAtom(From, false);
  bool checkBonds;
  if( XA != NULL )  {
    if( ClearSelection ) SelectAll(false);
    return Name( *XA, To, CheckLabel);
  }
  else  {
    TNameUndo *undo = new TNameUndo( new TUndoActionImpl<TGXApp>(this, &GxlObject(TGXApp::undoName)));
    olxstr oldL;

    TXAtomPList Atoms, ChangedAtoms;
    olxstr Tmp, NL;
    FindXAtoms(From, Atoms, ClearSelection);
    TBasicAtomInfo *bai;
    bool recreate;
    if( !From.Comparei("sel") && To.IsNumber() )  {
      int j = To.ToInt();
      for( int i=0; i < Atoms.Count(); i++ )  {
        XA = Atoms[i];
        checkBonds = (XA->Atom().GetAtomInfo() == iQPeakIndex);
        Tmp = XA->Atom().GetLabel();
        NL = XA->Atom().GetAtomInfo().GetSymbol();
        NL << j;  j++;

        bai = FAtomsInfo->FindAtomInfoEx(NL);
        recreate = XA->Atom().GetAtomInfo() != *bai;
        oldL = XA->Atom().GetLabel();

        XA->Atom().SetLabel( CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->Atom().CAtom(), NL) : NL );

        undo->AddAtom(XA->Atom(), oldL);

        NameHydrogens(XA->Atom(), undo, CheckLabel);

        if( recreate )  {
          ChangedAtoms.Add( XA );
          if( checkBonds )  CheckQBonds(*XA);
        }
      }
    }
    else  if( From[0] == '$' && To[0] == '$' )  {  // change type
      for( int i=0; i < Atoms.Count(); i++ )  {
        XA = Atoms[i];
        checkBonds = (XA->Atom().GetAtomInfo() == iQPeakIndex);
        Tmp = XA->Atom().GetLabel();
        NL  = EmptyString;
        for( int j=1; j < To.Length(); j++ )  NL << To[j];
        for( int j=From.Length()-1; j < Tmp.Length(); j++ )  NL << Tmp[j];

        bai = FAtomsInfo->FindAtomInfoEx(NL);
        recreate = XA->Atom().GetAtomInfo() != *bai;

        oldL = XA->Atom().GetLabel();

        XA->Atom().SetLabel( CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->Atom().CAtom(), NL) : NL );

        undo->AddAtom( XA->Atom(), oldL);

        NameHydrogens(XA->Atom(), undo, CheckLabel);

        if( recreate )  {
          ChangedAtoms.Add( XA );
          if( checkBonds )  CheckQBonds(*XA);
        }
      }
    }
    else  {  // C2? to C3? ; Q? to Ni? ...
      int qmi;
      for( int i=0; i < Atoms.Count(); i++ )  {
        XA = (TXAtom*)Atoms[i];
        checkBonds = (XA->Atom().GetAtomInfo() == iQPeakIndex);
        Tmp = XA->Atom().GetLabel();
        NL = To;
        qmi = 0;
        for( int j=0; j < NL.Length(); j++ )  {
          if( NL.CharAt(j) == '?' )  {
            qmi = From.FirstIndexOf('?', qmi);
            if( qmi >= 0 )  {
              NL[j] = Tmp.CharAt(qmi);
              qmi++;
            }
            else
              NL[j] = '_';
          }
        }

        bai = FAtomsInfo->FindAtomInfoEx(NL);
        if( bai == NULL )
          throw TFunctionFailedException(__OlxSourceInfo, "wrong syntax");

        recreate = XA->Atom().GetAtomInfo() != *bai;

        oldL = XA->Atom().GetLabel();

        XA->Atom().SetLabel( CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->Atom().CAtom(), NL) : NL );

        undo->AddAtom(XA->Atom(), oldL);

        NameHydrogens(XA->Atom(), undo, CheckLabel);

        if( recreate )  {
          ChangedAtoms.Add( XA );
          if( checkBonds )  CheckQBonds(*XA);
        }
      }
    }
    for( int i=0; i < ChangedAtoms.Count(); i++ )  {
      XA = ChangedAtoms[i];
      XA->Primitives()->RemoveObject(XA);
      XA->Create();
    }
    SynchroniseBonds( ChangedAtoms );
    return undo;
  }
}
//..............................................................................
int XAtomLabelSort(const TXAtom* I1, const TXAtom* I2)  {
  int v = TCAtomPComparator::Compare( &I1->Atom().CAtom(), &I2->Atom().CAtom() );
  return (v == 0) ? TCAtom::CompareAtomLabels( I1->Atom().GetLabel(), I2->Atom().GetLabel() ) : v;
}
//..............................................................................
void TGXApp::InfoList(const olxstr &Atoms, TStrList &Info)  {
  olxstr Tmp;
  TXAtomPList AtomsList;
  FindXAtoms(Atoms, AtomsList, false);
  AtomsList.QuickSorter.SortSF(AtomsList, XAtomLabelSort);
  TTTable<TStrList> Table(AtomsList.Count(), 7);
  Table.ColName(0) = "Atom";
  Table.ColName(1) = "Symb";
  Table.ColName(2) = "X";
  Table.ColName(3) = "Y";
  Table.ColName(4) = "Z";
  Table.ColName(5) = "Ueq";
  Table.ColName(6) = "Peak";
  for(int i = 0; i < AtomsList.Count(); i++ )  {
    TSAtom& A = AtomsList[i]->Atom();
    Table[i][0] = A.GetLabel();
    Table[i][1] = A.GetAtomInfo().GetSymbol();
    Table[i][2] = olxstr::FormatFloat(3, A.ccrd()[0]);
    Table[i][3] = olxstr::FormatFloat(3, A.ccrd()[1]);
    Table[i][4] = olxstr::FormatFloat(3, A.ccrd()[2]);
    Table[i][5] = olxstr::FormatFloat(3, A.CAtom().GetUiso());
    if( A.CAtom().GetQPeak() != -1 )
      Table[i][6] = olxstr::FormatFloat(3, A.CAtom().GetQPeak());
    else
      Table[i][0] = '-';
  }
  Table.CreateTXTList(Info, "Atom information", true, true, ' ');
}
//..............................................................................
TXGlLabel *TGXApp::AddLabel(const olxstr& Name, const vec3d& center, const olxstr& T)  {
  TXGlLabel* gl = new TXGlLabel(Name, FGlRender);
  gl->FontIndex( FLabels->FontIndex() );
  gl->SetLabel( T );
  gl->Basis.SetCenter( center );
  gl->Create();
  LooseObjects.Add(gl);
  return gl;
}
//..............................................................................
TXLine& TGXApp::AddLine(const olxstr& Name, const vec3d& base, const vec3d& edge)  {
  TXLine *XL = new TXLine(Name, base, edge, FGlRender);
  XL->Create();
  LooseObjects.Add( XL );
  return *XL;
}
//..............................................................................
AGDrawObject* TGXApp::FindLooseObject(const olxstr &Name)  {
  for( int i=0; i < LooseObjects.Count(); i++ )
    if( LooseObjects[i]->Primitives()->Name().Comparei(Name) == 0 )
      return LooseObjects[i];
  return NULL;
}
//..............................................................................
TSPlane *TGXApp::TmpPlane(TXAtomPList* atoms, int weightExtent)  {
  TSAtomPList SAtoms;
  if( atoms != NULL )
    TListCaster::POP(*atoms, SAtoms);
  else
    TListCaster::TOP(XAtoms, SAtoms );

  if( SAtoms.Count() < 3 )  return NULL;
  return  XFile().GetLattice().NewPlane(SAtoms, weightExtent);
}
//..............................................................................
TXPlane * TGXApp::AddPlane(TXAtomPList &Atoms, bool Rectangular, int weightExtent)  {
  if( Atoms.Count() < 3 )  return NULL;
  TSAtomPList SAtoms;
  for( int i=0; i < Atoms.Count(); i++ )
    SAtoms.Add( &Atoms[i]->Atom() );

  TSPlane *S = XFile().GetLattice().NewPlane(SAtoms, weightExtent);
  if( S )  {
    TXPlane& XP = XPlanes.AddNew(EsdlClassName(TXPlane)+XPlanes.Count(), S, FGlRender);
    XP.Rectangular(Rectangular);
    XP.Create();
    return &XP;
  }
  return NULL;
}
//..............................................................................
TXPlane *TGXApp::XPlane(const olxstr &PlaneName)  {
  for(int i=0; i < XPlanes.Count(); i++ )
    if( !XPlanes[i].Primitives()->Name().Comparei(PlaneName) )
      return &XPlanes[i];
  return NULL;
}
//..............................................................................
void TGXApp::DeletePlane(TXPlane* plane)  {
  plane->Plane().SetDeleted(true);
}
//..............................................................................
void TGXApp::ClearPlanes()  {
  for( int i=0; i < XPlanes.Count(); i++ )
    XPlanes[i].Deleted(true);
}
//..............................................................................
TXAtom * TGXApp::AddCentroid(TXAtomPList& Atoms)  {
  if( Atoms.Count() < 2 )  return NULL;
  TSAtomPList SAtoms;
  for( int i=0; i < Atoms.Count(); i++ )
    SAtoms.Add( &Atoms[i]->Atom() );
  TSAtom *A = XFile().GetLattice().NewCentroid( SAtoms );
  if( A != NULL )  {
    TXAtom& XA = XAtoms.AddNew( *(new TXAtom(EmptyString, *A, FGlRender)) );
    XA.Create();
    XA.Params()[0] = (float)A->GetAtomInfo().GetRad();
    return &XA;
  }
  return NULL;
}
//..............................................................................
TXAtom* TGXApp::AddAtom(TXAtom* templ)  {
  vec3d center;
  if( templ != NULL )
    center = templ->Atom().CAtom().ccrd();
  TSAtom *A = XFile().GetLattice().NewAtom( center );
  if( A != NULL )  {
    olxstr colName;
    if( templ != NULL )  {
      colName = templ->GetCollectionName();
      A->CAtom().AtomInfo( &templ->Atom().GetAtomInfo() );
    }
    else  {
      A->CAtom().AtomInfo( &AtomsInfo()->GetAtomInfo(6) );
    }
    TXAtom& XA = XAtoms.AddNew( *(new TXAtom(colName, *A, FGlRender)) );
    XA.Create();
    XA.Params()[0] = (float)A->GetAtomInfo().GetRad();
    return &XA;
  }
  return NULL;
}
//..............................................................................
void TGXApp::undoDelete(TUndoData *data)  {
  TKillUndo *undo = static_cast<TKillUndo*>(data);
  for( int i=0; i < undo->SAtomCount(); i++ )  undo->SAtom(i).SetDeleted(false);
  for( int i=0; i < undo->SBondCount(); i++ )  undo->SBond(i).SetDeleted(false);
  for( int i=0; i < undo->SPlaneCount(); i++ )  undo->SPlane(i).SetDeleted(false);
  if( undo->SBondCount() )  {
    TXBondPList res;
    SBonds2XBonds(undo->GetSBonds(), res);
    for( int i=0; i < res.Count(); i++ )  res[i]->Deleted(false);
  }
  if( undo->SPlaneCount() )  {
    TXPlanePList res;
    SPlanes2XPlanes(undo->GetSPlanes(), res);
    for( int i=0; i < res.Count(); i++ )  res[i]->Deleted(false);
  }
  if( undo->SAtomCount() )  {
    TXAtomPList res;
    SAtoms2XAtoms(undo->GetSAtoms(), res);
    for( int i=0; i < res.Count(); i++ )  res[i]->Deleted(false);
    XFile().GetLattice().UpdateAsymmUnit();
    CenterView();
  }
}
//..............................................................................
void TGXApp::undoName(TUndoData *data)  {
  TNameUndo *undo = static_cast<TNameUndo*>(data);
  TSAtomPList sal;
  for( int i=0; i < undo->AtomCount(); i++ )  {
    TBasicAtomInfo* bai = FAtomsInfo->FindAtomInfoEx( undo->GetLabel(i) );
    if( undo->GetAtom(i).GetAtomInfo() != *bai )
      sal.Add( &undo->GetAtom(i) );
    undo->GetAtom(i).SetLabel( undo->GetLabel(i) );
  }
  if( sal.Count() != 0 )  {
    TXAtomPList xal;
    SAtoms2XAtoms( sal, xal );
    for( int i=0; i < xal.Count(); i ++ )  {
      xal[i]->Primitives()->RemoveObject( xal[i] );
      xal[i]->Create();
    }
    SynchroniseBonds( xal );
  }
}
//..............................................................................
TUndoData* TGXApp::DeleteXObjects(TPtrList<AGDrawObject>& L)  {
  AGDrawObject *GO;
  TXAtomPList atoms;
  TXBondPList bonds;
  TXPlanePList planes;
  for( int i=0; i < L.Count(); i++ )  {
    GO = (AGDrawObject*)L[i];
    if( EsdlInstanceOf(*GO, TXAtom) )  {  atoms.Add( (TXAtom*)GO );  continue;  }
    if( EsdlInstanceOf(*GO, TXBond) )  {  bonds.Add( (TXBond*)GO );  continue;  }
    if( EsdlInstanceOf(*GO, TXPlane) ) {  planes.Add( (TXPlane*)GO); continue;  }
  }
  TKillUndo *undo = (TKillUndo*)DeleteXAtoms(atoms);
  for( int i=0; i < bonds.Count(); i++ )  {
    if( !bonds[i]->Deleted() )  {
      bonds[i]->Deleted(true);
      undo->AddSBond( bonds[i]->Bond() );
    }
  }
  for( int i=0; i < planes.Count(); i++ )  {
    if( !planes[i]->Deleted() )  {
      planes[i]->Deleted(true);
      undo->AddSPlane( planes[i]->Plane() );
    }
  }
  return undo;
}
//..............................................................................
TUndoData* TGXApp::DeleteXAtoms(TXAtomPList& L)  {
  TKillUndo *undo = new TKillUndo( new TUndoActionImpl<TGXApp>(this, &GxlObject(TGXApp::undoDelete)));
  TSBondPList SBL;
  TSAtomPList SAL;
  for( int i=0; i < L.Count(); i++ )  {
    TXAtom* XA = (TXAtom*)L[i];
    TSAtom* SA = &XA->Atom();
    undo->AddSAtom( XA->Atom() );
    for( int j=0; j < SA->BondCount(); j++ )  {
      TSBond* SB = &SA->Bond(j);
      SBL.Add( SB );
      SB->SetDeleted(true);
    }
    for( int j=0; j < SA->NodeCount();j++ ) {
      TSAtom* SH = &SA->Node(j);
      if( (SA->GetAtomInfo() != iQPeakIndex ) &&  (SH->GetAtomInfo() == iHydrogenIndex) )  {
        SH->SetDeleted(true);
        SAL.Add( SH );
      }
    }
    XA->Deleted(true);
  }
  undo->AddSAtoms(SAL);
  undo->AddSBonds(SBL);

  TXBondPList XBL;
  SBonds2XBonds(SBL, XBL);
  for( int i=0; i < XBL.Count(); i++ )  XBL[i]->Deleted(true);

  TXAtomPList XAL;
  SAtoms2XAtoms(SAL, XAL);
  for( int i=0; i < XAL.Count(); i++ )  XAL[i]->Deleted(true);

  Selection()->RemoveDeleted();
  XFile().GetLattice().UpdateAsymmUnit();
  CenterView();
  return undo;
}
//..............................................................................
void TGXApp::SelectBondsWhere(const olxstr &Where, bool Invert)  {
  olxstr str( olxstr::LowerCase(Where) );
  if( str.FirstIndexOf("xatom") != -1 || str.FirstIndexOf("satom") != -1)  {
    Log->Error("SelectBonds: xatom/satom are not allowed here");
    return;
  }
  if( str.FirstIndexOf("sel") != -1 )  {
    if( FGlRender->Selection()->Count() != 1 )  {
      Log->Error("SelectBonds: please select one bond only");
      return;
    }
    if( !EsdlInstanceOf( *FGlRender->Selection()->Object(0), TXBond) )  {
      Log->Error("SelectBonds: please select a bond");
      return;
    }
  }
  TXFactoryRegister rf;
  TTXBond_EvaluatorFactory *xbond = (TTXBond_EvaluatorFactory*)rf.BindingFactory("xbond");
  TTGlGroupEvaluatorFactory *sel = (TTGlGroupEvaluatorFactory*)rf.BindingFactory("sel");
  sel->SetTGlGroup( FGlRender->Selection() );
  TSyntaxParser SyntaxParser(&rf, Where);
  if( !SyntaxParser.Errors().Count() )  {
    for( int i=0; i < XBonds.Count(); i++ )  {
      if( XBonds[i].Selected() )  continue;
      xbond->SetTXBond_( &XBonds[i] );
      if( SyntaxParser.Evaluate() )  GetRender().Select( &XBonds[i] );
    }
  }
  else
    Log->Error( SyntaxParser.Errors().Text('\n') );
}
//..............................................................................
void TGXApp::SelectAtomsWhere(const olxstr &Where, bool Invert)  {
  olxstr str( olxstr::LowerCase(Where) );
  if( str.FirstIndexOf("xbond") != -1 || str.FirstIndexOf("satom") != -1)  {
    Log->Error("SelectAtoms: xbond/satom are not allowed here");
    return;
  }
  if( str.FirstIndexOf("sel") != -1 )  {
    if( FGlRender->Selection()->Count() != 1 )  {
      Log->Error("SelectAtoms: please select one atom only");
      return;
    }
    if( !EsdlInstanceOf( *FGlRender->Selection()->Object(0), TXAtom) )  {
      Log->Error("SelectAtoms: please select an atom");
      return;
    }
  }
  TXFactoryRegister rf;
  TTXAtom_EvaluatorFactory *xatom = (TTXAtom_EvaluatorFactory*)rf.BindingFactory("xatom");
  TTGlGroupEvaluatorFactory *sel = (TTGlGroupEvaluatorFactory*)rf.BindingFactory("sel");
  sel->SetTGlGroup( FGlRender->Selection() );
  TSyntaxParser SyntaxParser(&rf, Where);
  if( !SyntaxParser.Errors().Count() )  {
    for( int i=0; i < XAtoms.Count(); i++ )  {
      if( XAtoms[i].Selected() )  continue;
      xatom->SetTXAtom( &XAtoms[i] );
      if( SyntaxParser.Evaluate() )  GetRender().Select( &XAtoms[i] );
    }
  }
  else
    Log->Error( SyntaxParser.Errors().Text('\n') );
}
//..............................................................................
bool GetRing(TSAtomPList& atoms, TTypeList<TSAtomPList>& rings)  {
  TSAtomPList *ring = NULL;
  int starta = 0;
  for(int i=0; i < atoms.Count(); i++ )  {
    if( atoms[i]->GetTag() == 0 )  {  // unused atom
      ring = &rings.AddNew();
      ring->Add( atoms[i] );
      atoms[i]->SetTag(1);
      starta = i+1;
      break;
    }
  }
  if( ring == NULL )  return false;
  bool change = true;
  while( change )  {
    change = false;
    for(int i=starta; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetTag() == 0 )  {  // unused atom
        if( atoms[i]->IsConnectedTo( *ring->Item(ring->Count()-1) ) )  {
          ring->Add( atoms[i] );
          atoms[i]->SetTag(1);
          change = true;
        }
      }
    }
  }
  return true;
}

void TGXApp::FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings)  {
  TPtrList<TBasicAtomInfo> ring;
  if( !Condition.Comparei("sel") )  {
    TXAtomPList L;
    TSAtomPList SAtoms;
    GetSelectedXAtoms(L, false);
    TListCaster::POP(L, SAtoms);
    for(int i=0; i < SAtoms.Count(); i++ )
      SAtoms[i]->SetTag(0);
    while( GetRing(SAtoms, rings) )
      ;
    return;
  }
  TXApp::FindRings(Condition, rings);
}
//..............................................................................
void SortRing( TSAtomPList& atoms )  {
  int maxbc = 0, ind = -1;

  for(int i=0; i < atoms.Count(); i++ )  {
    int bc = 0;
    for( int j=0; j < atoms[i]->NodeCount(); j++ )
      if( atoms[i]->Node(j).GetAtomInfo() != iHydrogenIndex )
        bc++;
    if( bc > maxbc )  {
      maxbc = bc;
      ind = i;
    }
  }
  if( ind > 0 )
    atoms.ShiftL( ind );
//  for(int i=0; i < ind; i++ )
//    atoms.ShiftL( 1 );
}

void TGXApp::SelectRings(const olxstr& Condition, bool Invert)  {
  TTypeList< TSAtomPList > rings;
  try  {  FindRings(Condition, rings);  }
  catch( const TExceptionBase& exc )  {  throw TFunctionFailedException(__OlxSourceInfo, exc);  }

  if( rings.IsEmpty() )  return;

  TXAtomPList XA( rings.Count()*rings[0].Count() );
  TSAtomPList allSAtoms;
  allSAtoms.SetCapacity( XA.Count() );
  for(int i=0; i < rings.Count(); i++ )  {
    SortRing( rings[i] );
    for( int j=0; j < rings[i].Count(); j++ )
      allSAtoms.Add( rings[i][j] );
  }

  for( int i=0; i < XFile().GetLattice().AtomCount(); i++ )
    XFile().GetLattice().GetAtom(i).SetTag(-1);

  for( int i=0; i < allSAtoms.Count(); i++ )
    allSAtoms[i]->SetTag(i);

  for( int i=0; i < XAtoms.Count(); i++ )
    if( XAtoms[i].Atom().GetTag() != -1 )  {
      XA[ XAtoms[i].Atom().GetTag() ] =&XAtoms[i];
    }

  XA.Pack();

  for( int i=0; i < XA.Count(); i++ )
    XA[i]->SetTag(i);
  for( int i=0; i < XA.Count(); i++ )
    if( XA[i]->GetTag() == i && XA[i]->Visible() )
      FGlRender->Select( XA[i] );
}
//..............................................................................
void TGXApp::SelectAtoms(const olxstr &Names, bool Invert)  {
  TXAtomPList Sel;
  FindXAtoms(Names, Sel, true);
  for(int i=0; i < Sel.Count(); i++ )  {
    if( Invert )
      GetRender().Select( Sel[i] );
    else
      if( !Sel[i]->Selected() )  GetRender().Select( Sel[i] );
  }
}
//..............................................................................
void TGXApp::FindCAtoms(const olxstr &Atoms, TCAtomPList& List, bool ClearSelection)
{
  if( Atoms.Length() == 0 )  {
    TAsymmUnit& AU = XFile().GetLattice().GetAsymmUnit();
    for(int i=0; i < AU.AtomCount(); i++ )  {
      if( !AU.GetAtom(i).IsDeleted() )
        List.Add( &AU.GetAtom(i) );
    }
    return;
  }
  TStrList Toks(Atoms, ' ');
  olxstr Tmp;
  TBasicAtomInfo *BAI;
  TCAtom *A;
  for( int i = 0; i < Toks.Count(); i++ )  {
    Tmp = Toks.String(i);
    if( !Tmp.Comparei("sel") )  {
      GetSelectedCAtoms(List, ClearSelection);
      continue;
    }
    if( Tmp.CharAt(0) == '$' )  {
      Tmp = Tmp.SubStringFrom(1);
      if( Tmp.Length() != 0 )  {
        BAI = AtomsInfo()->FindAtomInfoBySymbol(Tmp);
        if( BAI == NULL )
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("atom type=") << Tmp);
        CAtomsByType(*BAI, List);
      }
      continue;
    }
    int ind = Tmp.FirstIndexOf('?');
    if( ind >= 0 )  {
      int mask = 0x0001 << ind;
      for( int j=ind+1; j < Tmp.Length(); j++ )  {
        ind = Tmp.FirstIndexOf('?', j);
        if( ind >=0 )  {
          mask |= 0x0001 << ind;
          j = ind;
        }
      }
      CAtomsByMask(Tmp, mask, List);
      continue;
    }
    A = XFile().GetAsymmUnit().FindCAtom(Tmp);
    if( A != NULL )
      if( !A->IsDeleted() )  List.Add(A);
  }
}
//..............................................................................
bool TGXApp::LabelsVisible()    const {  return FLabels->Visible(); }
//..............................................................................
void TGXApp::LabelsVisible(bool v)    {  FLabels->Visible(v); }
//..............................................................................
void TGXApp::LabelsMode(short lmode)  {  FLabels->Mode(lmode); }
//..............................................................................
short TGXApp::LabelsMode()      const {  return FLabels->Mode(); }
//..............................................................................
void TGXApp::LabelsFont(short Findex){  FLabels->FontIndex(Findex);  }
//..............................................................................
TGlMaterial & TGXApp::LabelsMarkMaterial()  {  return FLabels->MarkMaterial();  }
//..............................................................................
void TGXApp::MarkLabel(TXAtom *A, bool v)  {  FLabels->MarkLabel(A, v);  }
//..............................................................................
void TGXApp::ClearLabelMarks()  {  FLabels->ClearLabelMarks();  }
//..............................................................................
void TGXApp::ClearLabels()  {
  FLabels->Clear();
  XLabels.Clear();
}
//..............................................................................
void TGXApp::InitLabels(TXAtomPList* Atoms)  {
  if( Atoms != NULL )  {
    for( int i=0; i < Atoms->Count(); i++ )
      FLabels->AddAtom( Atoms->Item(i) );
  }
  else  {
    FLabels->Clear();
    for( int i=0; i < XAtoms.Count(); i++ )
      FLabels->AddAtom( &XAtoms[i] );
  }
}
//..............................................................................
void TGXApp::SBonds2XBonds(TSBondPList& L, TXBondPList& Res)  {
  if( L.IsEmpty() )  return;
  TLattice* latt = &L[0]->GetNetwork().GetLattice();

  for( int i=0; i < latt->BondCount(); i++ )
    latt->GetBond(i).SetTag(0);

  for( int i=0; i < L.Count(); i++ )
    L[i]->SetTag(1);

  Res.SetCapacity( Res.Count() + L.Count() );
  for( int i=0; i < XBonds.Count(); i++ )
    if( &XBonds[i].Bond().GetNetwork().GetLattice() == latt && XBonds[i].Bond().GetTag() != 0 )
      Res.Add( &XBonds[i] );
}
//..............................................................................
void TGXApp::SPlanes2XPlanes(TSPlanePList& L, TXPlanePList& Res)  {
  if( L.IsEmpty() )  return;
  TLattice* latt = &L[0]->GetNetwork().GetLattice();

  for( int i=0; i < latt->PlaneCount(); i++ )
    latt->GetPlane(i).SetTag(0);

  for( int i=0; i < L.Count(); i++ )
    L[i]->SetTag(1);

  Res.SetCapacity( Res.Count() + L.Count() );
  for( int i=0; i < XPlanes.Count(); i++ )
    if( &XPlanes[i].Plane().GetNetwork().GetLattice() == latt && XPlanes[i].Plane().GetTag() != 0 )
      Res.Add( &XPlanes[i] );
}
//..............................................................................
void TGXApp::SAtoms2XAtoms(TSAtomPList& L, TXAtomPList& Res)  {
  if( L.IsEmpty() )  return;
  TLattice* latt = &L[0]->GetNetwork().GetLattice();

  for( int i=0; i < latt->AtomCount(); i++ )
    latt->GetAtom(i).SetTag(0);

  for( int i=0; i < L.Count(); i++ )
    L[i]->SetTag(1);

  Res.SetCapacity( Res.Count() + L.Count() );
  for( int i=0; i < XAtoms.Count(); i++ )
    if( &XAtoms[i].Atom().GetNetwork().GetLattice() == latt && XAtoms[i].Atom().GetTag() != 0 )
      Res.Add( &XAtoms[i] );
}
//..............................................................................
void TGXApp::GetBonds(const olxstr &Bonds, TXBondPList& List)  {
  TGPCollection *GPC = GetRender().FindCollection(Bonds);
  if( GPC == NULL )  return;
  for( int i=0; i < GPC->ObjectCount(); i++ )  {
    if( i == 0 )  {  // check if the right type !
      if( !EsdlInstanceOf( *GPC->Object(0), TXBond) )  return;
    }
    List.Add( (TXBond*)GPC->Object(i) );
  }
}
//..............................................................................
void TGXApp::AtomRad(const olxstr& Rad, TXAtomPList* Atoms)  { // pers, sfil
  short DS = -1;
  if( !Rad.Comparei("sfil") ){  DS = darPack;  TXAtom::DefRad(darPack); }
  if( !Rad.Comparei("pers") ){  DS = darPers;  TXAtom::DefRad(darPers); }
  if( !Rad.Comparei("isot") ){  DS = darIsot;  TXAtom::DefRad(darIsot); }
  if( !Rad.Comparei("bond") ){  DS = darBond;  TXAtom::DefRad(darIsot); }
  if( DS == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "rad");

  if( Atoms != NULL )  {
    for( int i=0; i < Atoms->Count(); i++ )
    Atoms->Item(i)->CalcRad(DS);
  }
  else {
    for( int i=0; i < XAtoms.Count(); i++ )
      XAtoms[i].CalcRad(DS);
  }
  TXAtom::DefZoom(1);
  TXAtom::TelpProb(1);
}
//..............................................................................
void TGXApp::GetGPCollections(TPtrList<AGDrawObject>& GDObjects, TPtrList<TGPCollection>& Result)  {
  for( int i=0; i < GDObjects.Count(); i++ )
    GDObjects[i]->Primitives()->SetTag(i);

  for( int i=0; i < GDObjects.Count(); i++ )  {
    if( GDObjects[i]->Primitives()->GetTag() == i )
      Result.Add( GDObjects[i]->Primitives() );
  }
}
//..............................................................................
void TGXApp::FillXAtomList( TXAtomPList& res, TXAtomPList* providedAtoms) {
  if( providedAtoms != NULL )  {
    res.AddList( *providedAtoms );
  }
  else  {
    res.SetCapacity( XAtoms.Count() );
    for( int i=0; i < XAtoms.Count(); i++ )
      res.Add( &XAtoms[i] );
  }
}
//..............................................................................
void TGXApp::FillXBondList( TXBondPList& res, TXBondPList* providedBonds)  {
  if( providedBonds != NULL )  {
    res.AddList( *providedBonds );
  }
  else  {
    res.SetCapacity( XBonds.Count() );
    for( int i=0; i < XBonds.Count(); i++ )
      res.Add( &XBonds[i] );
  }
}
//..............................................................................
void TGXApp::AtomZoom(float Zoom, TXAtomPList* Atoms)  {  // takes %
  TPtrList<AGDrawObject> objects;
  if( Atoms != NULL )  {
    objects.SetCapacity( Atoms->Count() );
    for( int i=0; i < Atoms->Count(); i++ )
      objects.Add( (AGDrawObject*)Atoms->Item(i) );
  }
  else  {
    objects.SetCapacity( XAtoms.Count() );
    for( int i=0; i < XAtoms.Count(); i++ )
      objects.Add( (AGDrawObject*)&XAtoms[i] );
  }
  TPtrList<TGPCollection> Colls;
  GetGPCollections(objects, Colls);
  for( int i=0; i < Colls.Count(); i++ )  {
    if( Colls[i]->ObjectCount() )  {
      TXAtom* XA = dynamic_cast<TXAtom*>(Colls[i]->Object(0));
      XA->Zoom(Zoom/100);
    }
  }
}
//..............................................................................
void TGXApp::QPeakScale(float V)  {
  if( !XAtoms.Count() )  return;
  TXAtom::QPeakScale(V);
  TPtrList<TGPCollection> Colls;
  for( int i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetAtomInfo() == iQPeakIndex )
      Colls.Add( XAtoms[i].Primitives() );
  }
  FGlRender->RemoveCollections(Colls);
  for( int i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetAtomInfo() == iQPeakIndex )
      XAtoms[i].Create();
  }

}
//..............................................................................
float TGXApp::QPeakScale()  {
  return TXAtom::QPeakScale();
}
//..............................................................................
void TGXApp::BondRad(float R, TXBondPList* Bonds)  {
  TPtrList<AGDrawObject> objects;
  if( Bonds != NULL )
    TListCaster::TT(*Bonds, objects);
  else
    TListCaster::TTP(XBonds, objects);

  TPtrList<TGPCollection> Colls;
  GetGPCollections(objects, Colls);
  for( int i=0; i < Colls.Count(); i++ )  {
    if( Colls[i]->ObjectCount() != 0 )  {
      TXBond* XB = dynamic_cast<TXBond*>(Colls[i]->Object(0));
      XB->Radius(R);
    }
  }
}
//..............................................................................
void TGXApp::UpdateAtomPrimitives(int Mask, TXAtomPList* Atoms)
{
  TXAtomPList atoms;
  FillXAtomList( atoms, Atoms );
  for( int i=0; i < atoms.Count(); i++ )
    atoms[i]->Primitives()->SetTag(i);
  for( int i=0; i < atoms.Count(); i++ )
    if( atoms[i]->Primitives()->GetTag() == i )
      atoms[i]->UpdatePrimitives(Mask);
}
//..............................................................................
void TGXApp::UpdateBondPrimitives(int Mask, TXBondPList* Bonds)  {
  TXBondPList bonds;
  FillXBondList(bonds, Bonds);
  for( int i=0; i < bonds.Count(); i++ )
    bonds[i]->Primitives()->SetTag(i);

  for( int i=0; i < bonds.Count(); i++ )  {
    if( (Bonds == NULL) && (bonds[i]->Bond().GetType() == sotHBond) )  continue;
    if( bonds[i]->Primitives()->GetTag() == i )
      bonds[i]->UpdatePrimitives(Mask);
  }
}
//..............................................................................
void TGXApp::SetAtomDrawingStyle(short ADS, TXAtomPList* Atoms)  {
  TXAtomPList atoms;
  FillXAtomList(atoms, Atoms);
  for( int i=0; i < atoms.Count(); i++ )
    atoms[i]->DrawStyle(ADS);

  CalcProbFactor(FProbFactor);
  TXAtom::DefDS(ADS);
}
//..............................................................................
void TGXApp::XAtomDS2XBondDS(const olxstr &Source)  {
  int dds;
  TGlPrimitive *AGlP, *BGlP;
  for( int i=0; i < XAtoms.Count(); i++ )  XAtoms[i].Atom().SetTag(i);
  for( int i=0; i < XBonds.Count(); i++ )  XBonds[i].Primitives()->SetTag(i);

  for( int i=0; i < XBonds.Count(); i++ )  {
    if( XBonds[i].Primitives()->GetTag() != i )  continue;
    TXBond* XB = &XBonds[i];
    TXAtom* XA = &XAtoms[ XB->Bond().A().GetTag() ];

    AGlP = XA->Primitives()->PrimitiveByName(Source);
    if( !AGlP )  continue;
    TGlMaterial* GlMA = (TGlMaterial*)AGlP->GetProperties();

    XA = &XAtoms[ XB->Bond().B().GetTag() ];
    BGlP = XA->Primitives()->PrimitiveByName(Source);
    if( !BGlP )  continue;
    TGlMaterial* GlMB = (TGlMaterial*)BGlP->GetProperties();

    for( int j=0; j < XB->Primitives()->PrimitiveCount(); j++ )  {
      TGlPrimitive* GlP = XBonds[i].Primitives()->Primitive(j);
      if( GlP->Params().Count() >= 1 )  {
        dds = GlP->Params().Last();
        if( dds == ddsDefAtomA )  {  // from atom A
          GlP->SetProperties(GlMA);
          XB->Primitives()->Style()->PrimitiveMaterial(GlP->Name(), GlMA);
          continue;
        }
        if( dds == ddsDef )  {  // from haviest atom
          GlP->SetProperties(GlMA);
          XB->Primitives()->Style()->PrimitiveMaterial(GlP->Name(), GlMA);
          continue;
        }
        if( dds == ddsDefAtomB )  {
          GlP->SetProperties(GlMB);
          XB->Primitives()->Style()->PrimitiveMaterial(GlP->Name(), GlMB);
          continue;
        }
      }
    }
  }
  return;
}
//..............................................................................
void TGXApp::GrowAtoms(const olxstr& AtomsStr, bool Shell, TCAtomPList* Template)  {
  TXAtomPList xatoms;
  FindXAtoms(AtomsStr, xatoms, true);
  TSAtomPList satoms;
  TListCaster::POP(xatoms, satoms);
  FXFile->GetLattice().GrowAtoms(satoms, Shell, Template);
}
//..............................................................................
double TGXApp::CalcVolume(const TSStrPObjList<olxstr,double, true>* volumes, olxstr &report)
{
  if( !FXFile )  {  report = "File is not loaded";  return 0;  }
  int ac = FXFile->GetLattice().AtomCount();
  int bc = FXFile->GetLattice().BondCount();
  if( ac == 0 )  {  report = "Could not find any atoms";  return 0;  }
  TSAtom *SA, *OA;
  TSBond *SB;
  for( int i=0; i < bc; i++ )
    FXFile->GetLattice().GetBond(i).SetTag(0);
  double R1, R2, h1, h2, d, Vi=0, Vt=0;
  int ind;
  for( int i=0; i < ac; i++ )  {
    SA = &FXFile->GetLattice().GetAtom(i);
    if( SA->IsDeleted() )  continue;
    if( SA->GetAtomInfo() == iQPeakIndex )  continue;
    R1 = 0;
    if( volumes )  {
      ind = volumes->IndexOfComparable( SA->GetAtomInfo().GetSymbol() );
      if( ind != -1 ) R1 = volumes->GetObject(ind);
    }
    if( R1 == 0 )  R1 = SA->GetAtomInfo().GetRad2();
//    if( !SA->BondCount() )
//    {
      Vt += M_PI*(R1*R1*R1)*4.0/3;
//      continue;
//    }
    for( int j=0; j < SA->BondCount(); j++ )  {
      SB = &SA->Bond(j);
      if( SB->GetTag() != 0 )  continue;
      OA = &SB->Another(*SA);
      SB->SetTag(1);
      if( OA->IsDeleted() )  continue;
      if( OA->GetAtomInfo() == iQPeakIndex )  continue;
      d = SB->Length();
      R1 = R2 = 0;
      if( volumes != NULL )  {
        ind = volumes->IndexOfComparable(SA->GetAtomInfo().GetSymbol());
        if( ind != -1 ) R1 = volumes->GetObject(ind);
        ind = volumes->IndexOfComparable(OA->GetAtomInfo().GetSymbol());
        if( ind != -1 ) R2 = volumes->GetObject(ind);
      }
      if( R1 == 0 )  R1 = SA->GetAtomInfo().GetRad2();
      if( R2 == 0 )  R2 = OA->GetAtomInfo().GetRad2();
      h2 = (R1*R1 - (R2-d)*(R2-d))/(2*d);
      h1 = (R1+R2-d-h2);
      Vi += M_PI*( h1*h1*(R1-h1/3) + h2*h2*(R2-h2/3));
      //Vt += M_PI*(R1*R1*R1 + R2*R2*R2)*4.0/3;
    }
  }
  report = "Total volume (A): ";  report << Vt <<  '\n';
  report << "Overlapping volume (A): " << Vi << '\n';
  report << "Overlapping area (%): " <<  Vi*100.0/Vt << '\n';
  report << "Molecular volume (A): " <<  (Vt-Vi) << '\n';
  return Vt-Vi;
}
//..............................................................................
void TGXApp::ClearGroups()  {
  for( int i=0; i < FOldGroups.Count(); i++ )  {
    TEList* lG = (TEList*)FOldGroups.Item(i);
    for( int j=0; j < lG->Count()-2; j++ )
      delete (IEObject*)lG->Item(j);
    delete (bool*)lG->Item( lG->Count()-2 );
    delete (TGlMaterial*)lG->Item( lG->Count()-1 );
  }
  FOldGroups.Clear();
}
//..............................................................................
void TGXApp::StoreGroups()  {
  ClearGroups();
  for( int i=0; i < FGlRender->GroupCount(); i++ )  {
    TGlGroup* glG = FGlRender->Group(i);
    TEList* lG = new TEList();
    FOldGroups.Add(lG);
    lG->Add(new TSAtomPList());  // atoms
    lG->Add(new TSBondPList());  // bonds
    lG->Add(new TSPlanePList());  //planes
    lG->Add(new olxstr( glG->GetCollectionName()) );  //planes
    bool *p = new bool;
    *p = glG->Visible();
    lG->Add(p);

    TGlMaterial* gM = new TGlMaterial();
    *gM = *glG->GlM();
    lG->Add( gM );
    for( int j=0; j < glG->Count(); j++ )  {
      AGDrawObject* glO = glG->Object(j);
      if( EsdlInstanceOf( *glO, TXAtom) )
        ((TSAtomPList*)lG->Item(oglAtoms))->Add( &((TXAtom*)glO)->Atom() );
      if( EsdlInstanceOf( *glO, TXBond) )
        ((TSBondPList*)lG->Item(oglBonds))->Add( &((TXBond*)glO)->Bond() );
      if( EsdlInstanceOf( *glO, TXPlane) )
        ((TSPlanePList*)lG->Item(oglPlanes))->Add( &((TXPlane*)glO)->Plane() );
    }
  }
}
//..............................................................................
void TGXApp::RestoreGroups()  {
  TXAtomPList xatoms;
  TXBondPList xbonds;
  TXPlanePList xplanes;
  olxstr className;
  for( int i=0; i < FOldGroups.Count(); i++ )  {
    TEList* lG = (TEList*)FOldGroups.Item(i);

    xatoms.Clear();
    SAtoms2XAtoms(*(TSAtomPList*)lG->Item(oglAtoms), xatoms);

    xbonds.Clear();
    SBonds2XBonds(*(TSBondPList*)lG->Item(oglBonds), xbonds);

    xplanes.Clear();
    SPlanes2XPlanes(*(TSPlanePList*)lG->Item(oglPlanes), xplanes);

    FGlRender->Selection()->Clear();
    for(int j=0; j < xatoms.Count(); j++ )
      FGlRender->Selection()->Add( xatoms[j] );
    for(int j=0; j < xbonds.Count(); j++ )
      FGlRender->Selection()->Add( xbonds[j] );
    for(int j=0; j < xplanes.Count(); j++ )
      FGlRender->Selection()->Add( xplanes[j] );

    TGlGroup* glG = FGlRender->GroupSelection( *(olxstr*)lG->Item(lG->Count()-3) );
    if( glG == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "could not recreate groups");
    glG->Selected(false);
    FGlRender->Selection()->Clear();
    glG->GlM( *(TGlMaterial*)lG->Item(lG->Count()-1) );
    glG->Visible( *(bool*)lG->Item(lG->Count()-2) );
  }
}
//..............................................................................
void TGXApp::StoreVisibility()  {
  FVisibility.SetSize( XAtoms.Count() + XBonds.Count() + XPlanes.Count() );
  // atoms
  for( int i=0; i < XAtoms.Count(); i++ )
    if( XAtoms[i].Visible() )
      FVisibility.SetTrue(i);
  // bonds
  for(int i=0; i < XBonds.Count(); i++ )
    if( XBonds[i].Visible() )
      FVisibility.SetTrue(XAtoms.Count() + i);
  // planes
  for(int i=0; i < XPlanes.Count(); i++ )
    if( XPlanes[i].Visible() )
      FVisibility.SetTrue(XAtoms.Count() + XBonds.Count() + i);
}
void TGXApp::RestoreVisibility()
{
  //atoms
  for( int i=0; i < XAtoms.Count(); i++ )
    XAtoms[i].Visible( FVisibility.Get(i) );
  //bonds
  for(int i=0; i < XBonds.Count(); i++ )
    XBonds[i].Visible(FVisibility.Get(XAtoms.Count() + i));
  // planes
  for(int i=0; i < XPlanes.Count(); i++ )
      XPlanes[i].Visible( FVisibility.Get( XAtoms.Count() + XBonds.Count() + i) );
}
//..............................................................................
void TGXApp::BeginDrawBitmap(double resolution)  {
  FPictureResolution = resolution;
  FLabels->Clear();

  GetRender().Scene()->ScaleFonts(resolution);
  // store groups && visibility
  StoreGroups();
  StoreVisibility();
  /* end */

  CreateObjects( false, false );
  //CenterView();
  for( int i=0; i < XLabels.Count(); i++ )  XLabels[i].Create();
  // restore the visiblity && groups
  RestoreGroups();
  RestoreVisibility();
}
//..............................................................................
void TGXApp::FinishDrawBitmap()  {
  FLabels->Clear();
  GetRender().Scene()->RestoreFontScale();
  CreateObjects( false, false );
  for( int i=0; i < XLabels.Count(); i++ )  XLabels[i].Create();
  // restore visibility && clean up the memory
  RestoreVisibility();
  FVisibility.Clear();
  // recreate groups && clean up the memory
  RestoreGroups();
  ClearGroups();
  //CenterView();
}
//..............................................................................
void TGXApp::UpdateLabels()  {
  for( int i=0; i < XLabels.Count(); i++ )
    XLabels[i].SetLabel( XLabels[i].GetLabel() ); 
}
//..............................................................................
TXGlLabel* TGXApp::CreateLabel(TXAtom *A, int FontIndex)  {
  TXGlLabel& L = XLabels.AddNew( "PLabels", FGlRender );
  L.FontIndex( FontIndex );
  L.SetLabel(A->Atom().GetLabel());
  L.Basis.SetCenter( A->Atom().crd() );
  L.Basis.TranslateX(0.15);
  L.Basis.TranslateY(0.15);
  L.Basis.TranslateZ(0.15);
  L.Create();
  return &L;
}
//..............................................................................
long TGXApp::Draw()  {
//  TDrawThread *th = new TDrawThread(Render());
//  th->Create();
//  th->Run();

// this line is for a visual test when drawing happens
//  GetRender().InvertSelection();
  static int64_t mspf = 0, lastcall = 0;
  int64_t st = TETime::msNow();
//  if( lastcall != 0 && ((st-lastcall) < mspf) )  {
    //lastcall = st;
//    return 0;
//  }
  GetRender().Draw();
  lastcall = TETime::msNow();
  mspf = lastcall - st;
  return mspf;
}
//..............................................................................
void TGXApp::MoveFragment(TXAtom* to, TXAtom* fragAtom, bool copy)  {
  if( copy )
    FXFile->GetLattice().MoveFragmentG(to->Atom(), fragAtom->Atom());
  else
    FXFile->GetLattice().MoveFragment(to->Atom(), fragAtom->Atom());
}
//..............................................................................
void TGXApp::MoveFragment(const vec3d& to, TXAtom* fragAtom, bool copy)  {
  if( copy )
    FXFile->GetLattice().MoveFragmentG(to, fragAtom->Atom());
  else
    FXFile->GetLattice().MoveFragment(to, fragAtom->Atom());
}
//..............................................................................
void TGXApp::MoveToCenter()  {
  FXFile->GetLattice().MoveToCenter();
}
//..............................................................................
void TGXApp::Compaq(bool All)  {
  // compact on the atomic (ALl) or fragment level
  if( All )  FXFile->GetLattice().CompaqAll();
  else       FXFile->GetLattice().Compaq();
}
//..............................................................................
void TGXApp::HBondsVisible(bool v)  {
  FHBondsVisible = v;
  for( int i=0; i < XBonds.Count(); i++ )  {
    if( XBonds[i].Bond().GetType() == sotHBond )
      XBonds[i].Visible(FHBondsVisible);
  }
}
//..............................................................................
void TGXApp::HydrogensVisible(bool v)  {
  FHydrogensVisible = v;
  // proporgate visibility to the H-bonds
  //if( !v )  HBondsVisible( v );

  for( int i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetAtomInfo() == iHydrogenIndex )
      XAtoms[i].Visible(FHydrogensVisible);
  }
  for( int i=0; i < XBonds.Count(); i++ )  {
    if( XBonds[i].Bond().GetType() == sotHBond )  continue;
    if( ((XBonds[i].Bond().A().GetAtomInfo() == iHydrogenIndex) ||
        (XBonds[i].Bond().B().GetAtomInfo() == iHydrogenIndex)) &&
        ((XBonds[i].Bond().A().GetAtomInfo() != iQPeakIndex) &&
        (XBonds[i].Bond().B().GetAtomInfo() != iQPeakIndex)) )
    {
      XBonds[i].Visible(FHydrogensVisible);
    }
  }
  for( int i=0; i < XGrowLines.Count(); i++ )  {
    if( XGrowLines[i].SAtom()->GetAtomInfo() == iHydrogenIndex ||
          XGrowLines[i].CAtom()->GetAtomInfo()== iHydrogenIndex )
      XGrowLines[i].Visible(v);
  }
}
//..............................................................................
void TGXApp::QPeaksVisible(bool v)  {
  FQPeaksVisible = v;
  for( int i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetAtomInfo() == iQPeakIndex )
      XAtoms[i].Visible(FQPeaksVisible);
  }
}
//..............................................................................
void TGXApp::QPeakBondsVisible(bool v)  {
  FQPeakBondsVisible = v;
  for( int i=0; i < XBonds.Count(); i++ )  {
    if( (XBonds[i].Bond().A().GetAtomInfo() == iQPeakIndex) ||
        (XBonds[i].Bond().B().GetAtomInfo() == iQPeakIndex)  )
      XBonds[i].Visible(v);
  }
  for( int i=0; i < XGrowLines.Count(); i++ )  {
    if( XGrowLines[i].SAtom()->GetAtomInfo() == iQPeakIndex ||
          XGrowLines[i].CAtom()->GetAtomInfo() == iQPeakIndex )
      XGrowLines[i].Visible(v);
  }
}
//..............................................................................
void TGXApp::StructureVisible(bool v)  {
  FStructureVisible = v;
  for( int i=0; i < XAtoms.Count(); i++ )        XAtoms[i].Visible(v);
  for( int i=0; i < XBonds.Count(); i++ )        XBonds[i].Visible(v);
  for( int i=0; i < LooseObjects.Count(); i++ )  LooseObjects[i]->Visible(v);
  for( int i=0; i < XPlanes.Count(); i++ )       XPlanes[i].Visible(v);
  for( int i=0; i < XLabels.Count(); i++ )       XLabels[i].Visible(v);

  if( v )  {
    QPeaksVisible(FQPeaksVisible);
    QPeakBondsVisible(FQPeakBondsVisible);
    HydrogensVisible(FHydrogensVisible);
  }
}
//..............................................................................
void TGXApp::LoadXFile(const olxstr &fn)  {
  FXFile->LoadFromFile(fn);
  if( !FHydrogensVisible )  {  Log->Warning("Note: hydrogens are invisible");  }
  if( !FQPeaksVisible )     {  Log->Warning("Note: Q-peaks are invisible");  }
  if( !FStructureVisible )  {  Log->Warning("Note: structure is invisible");  }
}
//..............................................................................
void TGXApp::SwapExyz(TXAtom *XA, const olxstr& Elm)
{

}
//..............................................................................
void TGXApp::AddExyz(TXAtom *XA, const olxstr& Elm)
{

}
//..............................................................................
void TGXApp::ShowPart(const TIntList& parts, bool show)  {
  if( !parts.Count() )  {
    for( int i=0; i < XAtoms.Count(); i++ )  {
      if( XAtoms[i].Deleted() )  continue;
      else if( XAtoms[i].Atom().GetAtomInfo() == iHydrogenIndex )
        XAtoms[i].Visible(FHydrogensVisible);
      else if( XAtoms[i].Atom().GetAtomInfo() == iQPeakIndex )
        XAtoms[i].Visible(FQPeaksVisible);
      else
        XAtoms[i].Visible(true);
    }
    for( int i=0; i < XBonds.Count(); i++ )  {
      if( XBonds[i].Deleted() )  continue;

      if( (XBonds[i].Bond().A().GetAtomInfo() == iQPeakIndex) ||
          (XBonds[i].Bond().B().GetAtomInfo() == iQPeakIndex)  )
        XBonds[i].Visible(FQPeakBondsVisible);
      else
        if( (XBonds[i].Bond().A().GetAtomInfo() == iHydrogenIndex) ||
            (XBonds[i].Bond().B().GetAtomInfo() == iHydrogenIndex)  )
          XBonds[i].Visible(FHydrogensVisible);
        else
          XBonds[i].Visible(true);
    }
    return;
  }
  for( int i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Deleted() ) continue;
    if( parts.IndexOf( XAtoms[i].Atom().CAtom().GetPart() ) != - 1 )  {
      if( XAtoms[i].Atom().GetAtomInfo() == iHydrogenIndex )
        XAtoms[i].Visible(FHydrogensVisible);
      else
        if( XAtoms[i].Atom().GetAtomInfo() == iQPeakIndex )
          XAtoms[i].Visible(FQPeaksVisible);
        else
         XAtoms[i].Visible(show);
    }
    else
      XAtoms[i].Visible(!show);
  }
  for( int i=0; i < XBonds.Count(); i++ )  {
    if( XBonds[i].Deleted() )  continue;
    if( parts.IndexOf(XBonds[i].Bond().A().CAtom().GetPart()) != -1 &&
        parts.IndexOf(XBonds[i].Bond().B().CAtom().GetPart()) != -1 )  {

      if( (XBonds[i].Bond().A().GetAtomInfo() == iQPeakIndex) ||
          (XBonds[i].Bond().B().GetAtomInfo() == iQPeakIndex)  )
        XBonds[i].Visible(FQPeakBondsVisible);
      else
        if( (XBonds[i].Bond().A().GetAtomInfo() == iHydrogenIndex) ||
            (XBonds[i].Bond().B().GetAtomInfo() == iHydrogenIndex)  )
          XBonds[i].Visible(FHydrogensVisible);
        else
          XBonds[i].Visible(show);
    }
    else
      XBonds[i].Visible(!show);
  }
}
//..............................................................................
void TGXApp::HklVisible(bool v)  {
  if( v )  {
    // default if could not load the hkl ...
    FDUnitCell->Reciprical(false);
    FHklVisible = false;
    if( !FHklFile->RefCount() )  {
      if( !FXFile->GetLastLoader() )
      {  Log->Error("Cannot display HKL - file is not loaded");  return;  }
      if( !TEFile::FileExists(FXFile->GetLastLoader()->GetHKLSource()) )
      {  Log->Error("Cannot display HKL - could locate HKL file");  return;  }
      if( !FHklFile->LoadFromFile(FXFile->GetLastLoader()->GetHKLSource()) )
      {  Log->Error("Cannot display HKL - could load HKL file");  return;  }
    }
    CreateXRefs();
  }
  for( int i=0; i < XReflections.Count(); i++ )  XReflections[i].Visible(v);
  FHklVisible = v;
  FDUnitCell->Reciprical(v);
}
//..............................................................................
void TGXApp::SetGridDepth(const vec3d& crd)  {
  FXGrid->SetDepth( crd );
}
//..............................................................................
bool TGXApp::GridVisible()  const {  
  return FXGrid->Visible();  
}
//..............................................................................
bool TGXApp::ShowGrid(bool v, const olxstr& FN)  {
  if( v )  {
    if(  FXGrid->IsEmpty() && FN.IsEmpty() )  {
      Log->Error("Cannot display empty grid");
      return false;
    }
    FXGrid->Visible(true);
  }
  else
    FXGrid->Visible(false);
  return v;
}
//..............................................................................
void TGXApp::Individualise(TXAtom* XA)  {
  if( XA->Primitives()->ObjectCount() == 1 )  return;
  short level = XA->LegendLevel( XA->Primitives()->Name() );
  if( level >= 2 )  return;
  else  level++;

  olxstr leg = XA->GetLegend( XA->Atom(), level );
  TGPCollection* indCol = FGlRender->FindCollection( leg );
  if( indCol != NULL && XA->Primitives() == indCol )  return;
  else  {
    if( indCol == NULL )  {
      indCol = FGlRender->NewCollection( leg );
      FIndividualCollections.Add( leg );
    }
    XA->Create( leg );
    XA->Primitives()->RemoveObject(XA);
    TSAtomPList satoms;
    TSBondPList sbonds;
    TXAtomPList xatoms;
    TXBondPList xbonds;
    short level1;
    for( int i=0; i < XA->Atom().BondCount(); i++ )  {
      TSBond* SB = &XA->Atom().Bond(i);
      sbonds.Add( SB );
      satoms.Add( &SB->Another(XA->Atom()) );
    }
    SAtoms2XAtoms(satoms, xatoms);
    SBonds2XBonds(sbonds, xbonds);
    for( int i=0; i < xbonds.Count(); i++ )  {
      level1 = TXAtom::LegendLevel( xatoms[i]->Primitives()->Name() );
      if( xatoms[i]->Atom() == xbonds[i]->Bond().A() )
        leg = xbonds[i]->GetLegend( xbonds[i]->Bond(), level1, level);
      else
        leg = xbonds[i]->GetLegend( xbonds[i]->Bond(), level, level1);
      indCol = FGlRender->FindCollection( leg );
      if( indCol != NULL && xbonds[i]->Primitives() == indCol )  continue;
      else  {
        if( indCol == NULL )  {
          indCol = FGlRender->NewCollection( leg );
          FIndividualCollections.Add( leg );
        }
        xbonds[i]->Primitives()->RemoveObject( xbonds[i] );
        xbonds[i]->Create(leg);
      }
    }
  }
}
//..............................................................................
void TGXApp::Collectivise(TXAtom* XA)  {
  short level = XA->LegendLevel( XA->Primitives()->Name() );
  if( !level )  return;
  else  level--;

  olxstr leg = XA->GetLegend( XA->Atom(), level );
  TGPCollection* indCol = FGlRender->FindCollection( leg );
  if( indCol != NULL && XA->Primitives() == indCol )  return;
  else
  {
    if( indCol == NULL )  indCol = FGlRender->NewCollection( leg );

    XA->Primitives()->RemoveObject(XA);
    if( !XA->Primitives()->ObjectCount() )
    {
      int index = FIndividualCollections.IndexOfComparable( XA->Primitives()->Name() );
      if( index >= 0 )  FIndividualCollections.Remove(index);
    }
    XA->Create(leg);
    TSAtomPList satoms;
    TSBondPList sbonds;
    TXAtomPList xatoms;
    TXBondPList xbonds;
    short level1;
    for( int i=0; i < XA->Atom().BondCount(); i++ )  {
      TSBond* SB = &XA->Atom().Bond(i);
      sbonds.Add( SB );
      satoms.Add( &SB->Another(XA->Atom()) );
    }
    SAtoms2XAtoms(satoms, xatoms);
    SBonds2XBonds(sbonds, xbonds);
    for( int i=0; i < xbonds.Count(); i++ )  {
      level1 = TXAtom::LegendLevel( xatoms[i]->Primitives()->Name() );
      if( xatoms[i]->Atom() == xbonds[i]->Bond().A() )
        leg = xbonds[i]->GetLegend( xbonds[i]->Bond(), level1, level);
      else
        leg = xbonds[i]->GetLegend( xbonds[i]->Bond(), level, level1);
      indCol = FGlRender->FindCollection( leg );
      if( indCol != NULL && xbonds[i]->Primitives() == indCol )  continue;
      else  {
        if( indCol == NULL )  indCol = FGlRender->NewCollection( leg );
        xbonds[i]->Primitives()->RemoveObject(xbonds[i]);
        if( xbonds[i]->Primitives()->ObjectCount() == 0 )  {
          int index = FIndividualCollections.IndexOfComparable( xbonds[i]->Primitives()->Name() );
          if( index >= 0 )  FIndividualCollections.Remove(index);
        }
        xbonds[i]->Create(leg);
      }
    }
  }
}
//..............................................................................
int TGXApp::GetNextAvailableLabel(const olxstr& AtomType)
{
  int nextLabel = 0, currentLabel;

  TBasicAtomInfo *bai = FAtomsInfo->FindAtomInfoBySymbol(AtomType);
  if( !bai )  return nextLabel;
  olxstr label, nLabel;
  for( int i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetAtomInfo() == *bai )  {
      label = XAtoms[i].Atom().GetLabel().SubStringFrom( bai->GetSymbol().Length() );
      if( !label.Length() )  continue;
      nLabel = EmptyString;
      int j=0;
      while( (j < label.Length()) && label[j] <= '9' && label[j] >= '0' )  {
        nLabel << label[j];
        j++;
      }
      if( !nLabel.Length() )  continue;
      currentLabel = nLabel.ToInt();
      if( currentLabel > nextLabel )  nextLabel = currentLabel;
    }
  }
  return nextLabel+1;
}
//..............................................................................
void TGXApp::SynchroniseBonds( TXAtomPList& xatoms )  {
  TSBondPList sbonds;
  TXBondPList xbonds;
  for( int i=0; i < xatoms.Count(); i++ )  {
    for( int j=0; j < xatoms[i]->Atom().BondCount(); j++ )
      sbonds.Add( &xatoms[i]->Atom().Bond(j) );
  }

  // prepare unique list of bonds
  for( int i=0; i < sbonds.Count(); i++ )  sbonds[i]->SetTag(i);
  for( int i=0; i < sbonds.Count(); i++ )
    if( sbonds[i]->GetTag() != i )
      sbonds[i] = NULL;
  sbonds.Pack();
  // have to call setatom function to set thecorrect order for atom of bond
  for( int i=0; i < sbonds.Count(); i++ )
    sbonds[i]->SetA( sbonds[i]->A() );

  SBonds2XBonds(sbonds, xbonds);

  for(int i=0; i < XAtoms.Count(); i++ )  XAtoms[i].Atom().SetTag(i);

  for( int i=0; i < xbonds.Count(); i++ )  {
//    if( XB->Primitives()->ObjectCount() == 1 )  continue;
    // change the orientation if necessary
    xbonds[i]->BondUpdated();
    xbonds[i]->Primitives()->RemoveObject( xbonds[i] );
    TXAtom& XA  = XAtoms[ xbonds[i]->Bond().A().GetTag() ];
    TXAtom& XA1 = XAtoms[ xbonds[i]->Bond().B().GetTag() ];
    xbonds[i]->Create( TXBond::GetLegend( xbonds[i]->Bond(), TXAtom::LegendLevel(XA.Primitives()->Name()),
                TXAtom::LegendLevel(XA1.Primitives()->Name())) );
  }
  XAtomDS2XBondDS("Sphere");
}
//..............................................................................
void TGXApp::SetXGrowPointsVisible(bool v)  {
  if( !XGrowPoints.Count() && v )  {
    CreateXGrowPoints();
    XGrowPointsVisible = v;
    return;
  }
  for( int i=0; i < XGrowPoints.Count(); i++ )
    XGrowPoints[i].Visible( v );
  XGrowPointsVisible = v;
}
//..............................................................................
void TGXApp::SetPackMode(short v, const olxstr& atoms)  {
  PackMode = v;
  UsedTransforms.Clear();
}
//..............................................................................
void TGXApp::CreateXGrowPoints()  {
  if( XGrowPoints.Count() != 0 )  return;
  // remove the identity matrix 
  smatd_plist matrices;
  smatd I;
  I.r.I();
  UsedTransforms.AddCCopy(I);

  vec3d VFrom, VTo;
  vec3d MFrom(-1.5, -1.5, -1.5), MTo(2, 2, 2);

  VTo[0] = Round(MTo[0]+1);     VTo[1] = Round(MTo[1]+1);     VTo[2] = Round(MTo[2]+1);
  VFrom[0] = Round(MFrom[0]-1); VFrom[1] = Round(MFrom[1]-1); VFrom[2] = Round(MFrom[2]-1);

  XFile().GetLattice().GenerateMatrices(matrices, VFrom, VTo, MFrom, MTo);

  VFrom = XFile().GetAsymmUnit().GetOCenter(false, false);
  for( int i=0; i < matrices.Count(); i++ )  {
    if( UsedTransforms.IndexOf( *matrices[i] ) != -1 )  {
      delete matrices[i];
      continue;
    }
    VTo = VFrom * matrices[i]->r;
    VTo += matrices[i]->t;
    XFile().GetAsymmUnit().CellToCartesian( VTo );
    TXGrowPoint& gp = XGrowPoints.AddNew(EmptyString, VTo, *matrices[i], FGlRender );
    gp.Create("GrowPoint");
    delete matrices[i];
  }
}
//..............................................................................
void TGXApp::SetXGrowLinesVisible(bool v)  {
  if( !XGrowLines.Count() && v )  {
    CreateXGrowLines();
    FXGrowLinesVisible = v;
    return;
  }
  for( int i=0; i < XGrowLines.Count(); i++ )
    XGrowLines[i].Visible( v );
  FXGrowLinesVisible = v;
}
//..............................................................................
void TGXApp::SetGrowMode(short v, const olxstr& atoms)  {
  if( atoms.Length() )  {
    TXAtomPList xatoms;
    FindXAtoms(atoms, xatoms);
    // have to preprocess instructions like 'sel'
    olxstr ats;
    for( int i=0; i < xatoms.Count(); i++ )
      ats << xatoms[i]->Atom().GetLabel() << ' ';
    AtomsToGrow = ats;
  }
  else  {
    AtomsToGrow = EmptyString;
  }
  FGrowMode = v;
  UsedTransforms.Clear();
}
//..............................................................................
void TGXApp::CreateXGrowLines()  {
  if( XGrowLines.Count() != 0 )  return;
  int ac = FXFile->GetLattice().AtomCount();
  TSAtomPList AtomsToProcess;
  if( AtomsToGrow.Length() != 0 )  {
    TXAtomPList xatoms;
    FindXAtoms(AtomsToGrow, xatoms);
    TListCaster::POP( xatoms, AtomsToProcess );
  }
  else  {
    for( int i=0; i < ac; i++ )  {
      TSAtom* A = &FXFile->GetLattice().GetAtom(i);
      if( A->IsDeleted() )  continue;
      AtomsToProcess.Add( A );
    }
  }

  vec3d cnt;
  TPtrList<TCAtom> AttachedAtoms;
  vec3d cc;
  TTypeList<vec3d> TransformedCrds;
  for( int i=0; i < AtomsToProcess.Count(); i++ )  {
    TSAtom* A = AtomsToProcess[i];
    if( A->IsDeleted() )  continue;
    AttachedAtoms.Clear();
    if( (FGrowMode & gmCovalent) != 0 )  {
      for( int j=0; j < A->CAtom().AttachedAtomCount(); j++ )
        if( !A->CAtom().GetAttachedAtom(j).IsDeleted() )
          AttachedAtoms.Add( &A->CAtom().GetAttachedAtom(j) );
    }
    if( (FGrowMode & gmSInteractions) != 0 )  {
      for( int j=0; j < A->CAtom().AttachedAtomICount(); j++ )
        if( !A->CAtom().GetAttachedAtomI(j).IsDeleted() )
          AttachedAtoms.Add( &A->CAtom().GetAttachedAtomI(j) );
    }
    if( (FGrowMode & gmSameAtoms) != 0 )
      AttachedAtoms.Add( &A->CAtom() );

    if( AttachedAtoms.IsEmpty() )  continue;
    for( int j=0; j < AttachedAtoms.Count(); j++ )  {
      TCAtom *aa = AttachedAtoms[j];
      cc = aa->ccrd();
      smatd_list *transforms;
      if( FGrowMode & gmSameAtoms )  {
//        transforms = FXFile->GetLattice().GetUnitCell()->Getclosest(A->ccrd(), cc, false );
        transforms = FXFile->GetLattice().GetUnitCell().GetInRangeEx(A->ccrd(), cc,
                             A->GetAtomInfo().GetRad1() + aa->GetAtomInfo().GetRad1() + 15,
                             false, UsedTransforms );
      }
      else if( FGrowMode & gmSInteractions )  {
        transforms = FXFile->GetLattice().GetUnitCell().GetInRange(A->ccrd(), cc,
                             A->GetAtomInfo().GetRad1() + aa->GetAtomInfo().GetRad1() + FXFile->GetLattice().GetDeltaI(),
                             false );
      }
      else  {
        transforms = FXFile->GetLattice().GetUnitCell().GetInRange(A->ccrd(), cc,
                             A->GetAtomInfo().GetRad1() + aa->GetAtomInfo().GetRad1() + FXFile->GetLattice().GetDelta(),
                             false );
      }
      if( transforms->IsEmpty() )  {  delete transforms;  continue;  }
      // remove identity transforms
      TransformedCrds.Clear();
      for( int k=0; k < transforms->Count(); k++ )  {
        smatd& transform = transforms->Item(k);
        cc *= transform.r;
        cc += transform.t;
        XFile().GetAsymmUnit().CellToCartesian(cc);
        TransformedCrds.AddCCopy( cc );
        if( cc.QDistanceTo( A->crd() ) < 0.01 )  {
          transforms->NullItem(k);
        }
        cc = aa->ccrd();
      }
      transforms->Pack();
      for( int k=0; k < transforms->Count(); k++ )  {
        for( int l = 0; l < A->NodeCount(); l++ )  {
          TSAtom *sa = &A->Node(l);
          if( sa->CAtom().GetLoaderId() == aa->GetLoaderId() )  {
            if( TransformedCrds[k].QDistanceTo( sa->crd() ) < 0.01 )  {
              transforms->Delete(k);
            }
          }
        }
      }
      for( int k=0; k < transforms->Count(); k++ )  {
        smatd& transform = transforms->Item(k);
        TXGrowLine& gl = XGrowLines.AddNew(EmptyString, A, aa, transform, FGlRender );

        if( (A->GetAtomInfo() == iQPeakIndex || aa->GetAtomInfo() == iQPeakIndex ) && !QPeakBondsVisible() )
          gl.Visible(false);
        if( (A->GetAtomInfo() == iHydrogenIndex || aa->GetAtomInfo() == iHydrogenIndex) && !HBondsVisible() )
          gl.Visible(false);

        double dist = TransformedCrds[k].DistanceTo( A->crd() );
        if( dist < (A->GetAtomInfo().GetRad1() + aa->GetAtomInfo().GetRad1() + FXFile->GetLattice().GetDelta()) )
          gl.Create("COV");
        else
          gl.Create("SI");
      }
      delete transforms;
    }
  }
}
//..............................................................................
void TGXApp::Grow(const TXGrowLine& growLine)  {
  UsedTransforms.AddCCopy( growLine.GetTransform() );
  XFile().GetLattice().GrowAtom( growLine.CAtom()->GetFragmentId(), growLine.GetTransform() );
}
//..............................................................................
void TGXApp::Grow(const TXGrowPoint& growPoint)  {
  UsedTransforms.AddCCopy( growPoint.GetTransform() );
  XFile().GetLattice().Grow( growPoint.GetTransform() );
}
//..............................................................................
TGlBitmap* TGXApp::FindGlBitmap(const olxstr& name)  {
  for(int i=0; i < GlBitmaps.Count(); i++ )
    if( GlBitmaps[i]->GetCollectionName() == name )  return GlBitmaps[i];
  return NULL;
}
//..............................................................................
TGlBitmap* TGXApp::CreateGlBitmap(const olxstr& name,
  int left, int top, int width, int height,
  unsigned char* RGBa, unsigned int format)  {

  TGlBitmap* glB = FindGlBitmap(name);
  if( glB == NULL )  {
    glB = new TGlBitmap(name, FGlRender, left, top, width, height, RGBa, format );
    GlBitmaps.Add(glB);
    glB->Create();
    ObjectsToCreate.Add( (AGDrawObject*)glB );
    glB->SetZ(-10.0 + (double)GlBitmaps.Count()/100 );
  }
  else  {
    glB->ReplaceData( width, height, RGBa, format );
    if( !glB->Visible() ) glB->Visible(true);
  }
  return glB;
}
//..............................................................................
void TGXApp::DeleteGlBitmap(const olxstr& name)  {
  TGlBitmap* glb = NULL;
  for(int i=0; i < GlBitmaps.Count(); i++ )  {
    if( GlBitmaps[i]->GetCollectionName() == name )  {
      glb = GlBitmaps[i];
      GlBitmaps.Delete(i);
      break;
    }
  }
  if( glb != NULL )  {
    //GlBitmaps.Delete(index);
    int ind = ObjectsToCreate.IndexOf( (AGDrawObject*)glb );
    if( ind != -1 )
      ObjectsToCreate.Delete(ind);
    glb->Primitives()->RemoveObject( glb );
    FGlRender->RemoveObject( glb );
    delete glb;
  }
}
//..............................................................................
TXFile& TGXApp::NewOverlayedXFile() {
  TXFile& f = OverlayedXFiles.Add( *(TXFile*)FXFile->Replicate() );
  return f;
}
//..............................................................................
void TGXApp::DeleteOverlayedXFile(int index) {

  ClearLabels();
  ClearSelectionCopy();
  OverlayedXFiles.Delete(index);
  CreateObjects(true);
  CenterView();
  Draw();
}
//..............................................................................
void TGXApp::UpdateBonds()  {
  for( int i=0; i < XBonds.Count(); i++ )  {
    XBonds[i].BondUpdated();
  }
}
//..............................................................................
TXLattice& TGXApp::AddLattice(const olxstr& Name, const mat3d& basis)  {
  TXLattice *XL = new TXLattice(Name, FGlRender);
  XL->SetLatticeBasis(basis);
  XL->Create();
  LooseObjects.Add( XL );
  return *XL;
}
//..............................................................................
void TGXApp::InitFadeMode()  {
}
//..............................................................................
//..............................................................................
//..............................................................................
