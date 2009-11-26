//----------------------------------------------------------------------------//
// TGXApp
// (c) Oleg V. Dolomanov, 2004-2009
//----------------------------------------------------------------------------//
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
#include "xeval.h"

#include "network.h"
#include "unitcell.h"
#include "symmparser.h"

#include "efile.h"
#include "ecast.h"

#include "xlattice.h"
#include "egc.h"
#include "eutf8.h"
#include "ememstream.h"

#include "gpcollection.h"
#include "estopwatch.h"
#include "pers_util.h"
#include "planesort.h"

#ifdef __WXWIDGETS__
  #include "wxglscene.h"
  #include "wx/string.h"
  #include "wx/fontutil.h"
  #include "wx/wfstream.h"
  #include "wxzipfs.h"
#endif
#define ConeStipple  6.0
#define LineStipple  0xf0f0

// on Linux it is defined as something..
#undef QLength

int CompareStr(const olxstr &Str, const olxstr &Str1, bool IC) {
  size_t minl = olx_min(Str1.Length(), Str.Length());
  for( size_t i = 0; i < minl; i++ )  {
    int diff = Str[i] - Str1[i];
    if( olxstr::o_isdigit(Str[i]) )  {
      if( olxstr::o_isdigit(Str1[i]) )  {
        olxstr S, S1;
        size_t j = i, k = i;
        while( (j < Str.Length()) && olxstr::o_isdigit(Str[j]) )  {
          S << Str[j];  j++;
        }
        while( (k < Str1.Length()) && olxstr::o_isdigit(Str1[k]) )  {
          S1 << Str1[k];  k++;
        }
        int diff1 = S.ToInt() - S1.ToInt();
        if( diff1 != 0 )  return diff1;
        // if the number of digits different - diff1 != 0, so now k=j
        i = k-1;
      }
    }
    else  {
      diff = IC ? (olxstr::o_toupper(Str[i]) - olxstr::o_toupper(Str1[i])) :
                 (Str[i] - Str1[i]);
      if( diff != 0 )  return diff;
    }
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// UNDO DATA CLASSES
class TKillUndo: public TUndoData  {
public:
  TTypeList<TSAtom::Ref> SAtomIds;
  TKillUndo(IUndoAction *action):TUndoData(action)  {  }
  virtual ~TKillUndo()  {  }
  void AddSAtom(const TSAtom& SA)  {  SAtomIds.AddCCopy( SA.GetRef() );  }
};
class THideUndo: public TUndoData  {
public:
  TPtrList<AGDrawObject> Objects;
  THideUndo(IUndoAction *action):TUndoData(action)  {  }
  virtual ~THideUndo()  {  }
  template <class T>  void AddObject(T& go)  {  Objects.Add(go);  }
};
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//..............................................................................
TXBondStylesClear::~TXBondStylesClear()  {  ;  }

class xappXFileLoad: public AActionHandler  {
  TGXApp *FParent;
  TEBasis B;
  TStrList AtomNames;
  TEBitArray CAtomMasks;
  TLattice::GrowInfo* GrowInfo;
  bool SameFile, EmptyFile;
public:
  xappXFileLoad(TGXApp *Parent) {  
    FParent = Parent;  
    AActionHandler::SetToDelete(false);
    GrowInfo = NULL;
    SameFile = false;
  }
  ~xappXFileLoad()  {  
    if( GrowInfo != NULL )
      delete GrowInfo;
    return;  
  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( GrowInfo != NULL )  {
      delete GrowInfo;
      GrowInfo = NULL;
    }
    EmptyFile = SameFile = false;
    FParent->ClearLabels();
    // make sure that these are only cleared when file is loaded
    if( Sender && EsdlInstanceOf(*Sender, TXFile) )  {
      if( Data != NULL && EsdlInstanceOf(*Data, olxstr) )  {
        olxstr s1( TEFile::UnixPath(TEFile::ChangeFileExt(*(olxstr*)Data, EmptyString)) );
        olxstr s2( TEFile::UnixPath(TEFile::ChangeFileExt(FParent->XFile().GetFileName(), EmptyString)) );
        if( s1 != s2 )  {
          FParent->ClearIndividualCollections();
          FParent->GetRender().GetStyles().RemoveNamedStyles("Q");
        }
        else  {
          const TAsymmUnit& au = FParent->XFile().GetAsymmUnit();
          int ac = 0;
          for( size_t i=0; i < au.AtomCount(); i++ )  {
            const TCAtom& ca = au.GetAtom(i);           
            if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  continue;
            ac++;
          }
          AtomNames.SetCapacity(ac);
          CAtomMasks.SetSize(ac);
          ac = 0;
          for( size_t i=0; i < au.AtomCount(); i++ )  {
            const TCAtom& ca = au.GetAtom(i);
            if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  continue;
            AtomNames.Add(ca.GetLabel());
            CAtomMasks.Set(ac++, ca.IsMasked());
          }
          GrowInfo = FParent->XFile().GetLattice().GetGrowInfo();
          SameFile = true;
          EmptyFile = (ac == 0);
        }
      }
      else  {
        FParent->ClearIndividualCollections();
        FParent->GetRender().GetStyles().RemoveNamedStyles("Q");
      }
      FParent->DUnitCell().ResetCentres();
      //FParent->XGrid().Clear();
    }
    B = FParent->GetRender().GetBasis();
    FParent->GetRender().Clear();
    FParent->HklFile().Clear();
    FParent->ClearSelectionCopy();
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    const TAsymmUnit& au = FParent->XFile().GetAsymmUnit();
    bool sameAU = true;
    size_t ac = 0;
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      const TCAtom& ca = au.GetAtom(i);
      if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  continue;
      if( ac >= AtomNames.Count() )  {
        sameAU = false;
        break;
      }
      if( !AtomNames[ac++].Equalsi(ca.GetLabel()) )  {
        sameAU = false;
        break;
      }
    }
    FParent->XFile().GetAsymmUnit().DetachAtomType(iQPeakIndex, !FParent->QPeaksVisible());
    FParent->XFile().GetAsymmUnit().DetachAtomType(iHydrogenIndex, !FParent->HydrogensVisible());
    if( sameAU )  {  // apply masks
      ac = 0;
      for( size_t i=0; i < au.AtomCount(); i++ )  {
        TCAtom& ca = au.GetAtom(i);
        if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  continue;
        ca.SetMasked( CAtomMasks[ac++] );
      }
      FParent->XFile().GetLattice().SetGrowInfo( GrowInfo );
      GrowInfo = NULL;
    }
    if( GrowInfo != NULL )  {
      delete GrowInfo;
      GrowInfo = NULL;
    }
    CAtomMasks.Clear();
    AtomNames.Clear();
    return false;
  }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    FParent->GetRender().SetBasis(B);
    FParent->CenterView();
    if( !SameFile || EmptyFile )
      FParent->GetRender().SetZoom( FParent->GetRender().CalcZoom()*FParent->GetExtraZoom() );
    FParent->Draw();
    return true;
  }
};
//..............................................................................
class xappXFileUniq : public AActionHandler  {
public:
  virtual bool Enter(const IEObject *Sender, const IEObject *Data)  {
    TGXApp& app = TGXApp::GetInstance();
    if( app.OverlayedXFileCount() != 0 )
      app.AlignOverlayedXFiles();
    return true;
  }
};
//..............................................................................
//----------------------------------------------------------------------------//
//TGXApp function bodies
//----------------------------------------------------------------------------//
enum  {
  ID_OnSelect = 1,
  ID_OnDisassemble
};

TGXApp::TGXApp(const olxstr &FileName) : TXApp(FileName, this)  {
  FQPeaksVisible = FHydrogensVisible = FStructureVisible = FHBondsVisible = true;
  XGrowPointsVisible = FXGrowLinesVisible = FQPeakBondsVisible = false;
  FXPolyVisible = true;
  DeltaV = 3;
  TwxGlScene *GlScene = new TwxGlScene( GetBaseDir() + "etc/Fonts/" );
  FGrowMode = gmCovalent;
//  TWGlScene *GlScene = new TWGlScene;
//  TGlScene *GlScene = new TGlScene;
  FGlRender = new TGlRenderer(GlScene, 1,1);
  FDFrame = new TDFrame(*FGlRender, "DFrame");
  Fader = new TXFader(*FGlRender, "Fader");
  FDFrame->OnSelect->Add(this, ID_OnSelect);
  FGlMouse = new TGlMouse(FGlRender, FDFrame);
  FDUnitCell = new TDUnitCell(*FGlRender, "DUnitCell");
  FDUnitCell->SetVisible(false);
  FDBasis = new TDBasis(*FGlRender, "DBasis");
  FDBasis->SetVisible(false);
  TXAtom::Init(FGlRender);
  TXBond::Init(FGlRender);
  FProbFactor = 50;
  ExtraZoom = 1.25;

  FLabels = new TXGlLabels(*FGlRender, "Labels");
  ObjectsToCreate.Add( FDBasis );
  ObjectsToCreate.Add( FDUnitCell );
  ObjectsToCreate.Add( FDFrame );
  ObjectsToCreate.Add( Fader );

  FHklFile = new THklFile();
  FHklVisible = false;

  FXGrid = new TXGrid("XGrid", this);

  XFile().GetLattice().OnDisassemble->Add(this, ID_OnDisassemble);

  xappXFileLoad *P = &TEGC::NewG<xappXFileLoad>(this);
  XFile().GetLattice().OnStructureGrow->Add(P);
  XFile().GetLattice().OnStructureUniq->Add(P);
  XFile().GetLattice().OnStructureUniq->Add( new xappXFileUniq);
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
  FGlRender->GetSelection().Clear();
  FGlRender->ClearGroups();
  ClearXObjects();
  
  for( size_t i=0; i < LooseObjects.Count(); i++ )  
    delete LooseObjects[i];   
  LooseObjects.Clear();

  for( size_t i=0; i < ObjectsToCreate.Count(); i++ )
    delete ObjectsToCreate[i];
  ObjectsToCreate.Clear();

  XLabels.Clear();
  GlBitmaps.Clear();
  ClearGroups();
}
//..............................................................................
void TGXApp::CreateXRefs()  {
  if( !XReflections.IsEmpty() )  return;

  TRefList refs;
  RefinementModel::HklStat stats = XFile().GetRM().GetRefinementRefList<RefMerger::StandardMerger>(XFile().GetLastLoaderSG(), refs);
  vec3d Center;
  for( size_t i=0; i < refs.Count(); i++ )  {
    TXReflection* xr = new TXReflection(*FGlRender, "XReflection", stats.MinI, stats.MaxI, refs[i],
      &FXFile->GetAsymmUnit());
    xr->Create();
    XReflections.Add( *xr );
    Center += xr->Center();
  }
  if( !refs.IsEmpty() )  Center /= refs.Count();
  Center += FGlRender->GetBasis().GetCenter();
  for( size_t i=0; i < XReflections.Count(); i++ )
    XReflections[i].Center() -= Center;
}
//..............................................................................
size_t TGXApp::GetNetworks(TNetPList& nets) {
  size_t c = XFile().GetLattice().FragmentCount();
  for( size_t i=0; i < c; i++ )
    nets.Add( &XFile().GetLattice().GetFragment(i) );

  for( size_t i=0; i < OverlayedXFiles.Count(); i++ )  {
    size_t fc = OverlayedXFiles[i].GetLattice().FragmentCount();
    c += fc;
    for( size_t j=0; j < fc; j++ )
      nets.Add( &OverlayedXFiles[i].GetLattice().GetFragment(j) );
  }
  return c;
}
//..............................................................................
void TGXApp::CreateObjects(bool SyncBonds, bool centerModel)  {
  TStopWatch sw(__FUNC__);
  sw.start("Initialising");

  vec3d glMax = FGlRender->MaxDim();
  vec3d glMin = FGlRender->MinDim();
  vec3d glCenter = FGlRender->GetBasis().GetCenter();
  TXAtom::FStaticObjects.Clear();
  TXBond::FStaticObjects.Clear();
  FGlRender->ClearPrimitives();
  FLabels->Clear();
  ClearXObjects();
  FGlRender->SetSceneComplete(false);

  for( size_t i=0; i < IndividualCollections.Count(); i++ )
    FGlRender->NewCollection( IndividualCollections[i] );

  size_t totalACount = XFile().GetLattice().AtomCount();
  for( size_t i=0; i < OverlayedXFiles.Count(); i++ )
    totalACount += OverlayedXFiles[i].GetLattice().AtomCount();
  size_t totalBCount = XFile().GetLattice().BondCount();
  for( size_t i=0; i < OverlayedXFiles.Count(); i++ )
    totalBCount += OverlayedXFiles[i].GetLattice().BondCount();

  GetRender().SetObjectsCapacity( totalACount + totalBCount + 512);

  sw.start("Atoms creation");
  TSAtomPList allAtoms;
  allAtoms.SetCapacity( totalACount );
  for( size_t i=0; i < XFile().GetLattice().AtomCount(); i++ )
    allAtoms.Add( &XFile().GetLattice().GetAtom(i) );
  for( size_t i=0; i < OverlayedXFiles.Count(); i++ )  {
    for( size_t j=0; j < OverlayedXFiles[i].GetLattice().AtomCount(); j++ )
      allAtoms.Add( &OverlayedXFiles[i].GetLattice().GetAtom(j) );
  }

  XAtoms.SetCapacity( allAtoms.Count() );
  const size_t this_a_count = XFile().GetLattice().AtomCount();
  for( size_t i=0; i < allAtoms.Count(); i++ )  {
    allAtoms[i]->SetTag(i);
    TXAtom& XA = XAtoms.Add( *(new TXAtom(*FGlRender, EmptyString, *allAtoms[i])) );
    XA.SetDeleted(allAtoms[i]->IsDeleted());
    XA.Create(EmptyString);
    XA.SetXAppId(i);
    if( !FStructureVisible )
      XA.SetVisible(false);  
    else 
      XA.SetVisible(allAtoms[i]->CAtom().IsAvailable());  
  }
  
  sw.start("Bonds creation");
  TSBondPList allBonds;
  allBonds.SetCapacity( totalBCount );
  for( size_t i=0; i < XFile().GetLattice().BondCount(); i++ )
    allBonds.Add( &XFile().GetLattice().GetBond(i) );
  for( size_t i=0; i < OverlayedXFiles.Count(); i++ )  {
    for( size_t j=0; j < OverlayedXFiles[i].GetLattice().BondCount(); j++ )
      allBonds.Add( &OverlayedXFiles[i].GetLattice().GetBond(j) );
  }
  XBonds.SetCapacity(allBonds.Count());
  for( size_t i=0; i < allBonds.Count(); i++ )  {
    TSBond* B = allBonds[i];
    TXBond& XB = XBonds.Add( *(new TXBond(*FGlRender, TXBond::GetLegend( *B, 2), *allBonds[i])) );
    XB.SetDeleted(B->IsDeleted() || (B->A().IsDeleted() || allBonds[i]->B().IsDeleted()));
    BondCreationParams bcpar(XAtoms[B->A().GetTag()], XAtoms[B->B().GetTag()]);
    XB.Create(EmptyString, &bcpar);
    if( !FStructureVisible )  {  
      XB.SetVisible(FStructureVisible);  
      continue;  
    }
    XB.SetVisible( XAtoms[B->A().GetTag()].IsVisible() && XAtoms[B->B().GetTag()].IsVisible() );
    if( !XB.IsVisible() )  continue;
    if( (B->A().GetAtomInfo() == iQPeakIndex) ||
        (B->B().GetAtomInfo() == iQPeakIndex)  ) 
    {  
      XB.SetVisible(FQPeakBondsVisible);  
      continue;  
    }
    if( (B->GetType() == sotHBond) )  {
      XB.SetVisible(FHBondsVisible);
    }
  }
  sw.start("Other objects creation");

  for( size_t i=0; i < FXFile->GetLattice().PlaneCount(); i++ )  {
    TSPlane& P = FXFile->GetLattice().GetPlane(i);
    TXPlane& XP = XPlanes.Add( new TXPlane(*FGlRender, olxstr("TXPlane") << i, &P) );
    XP.SetDeleted(P.IsDeleted());
    XP.Create();
  }
  double cell[] = {  
    XFile().GetAsymmUnit().Axes()[0].GetV(),
    XFile().GetAsymmUnit().Axes()[1].GetV(),
    XFile().GetAsymmUnit().Axes()[2].GetV(),
    XFile().GetAsymmUnit().Angles()[0].GetV(),
    XFile().GetAsymmUnit().Angles()[1].GetV(),
    XFile().GetAsymmUnit().Angles()[2].GetV()
  };
  DUnitCell().Init( cell );
  DBasis().SetAsymmUnit(&XFile().GetAsymmUnit());

  for( size_t i=0; i < ObjectsToCreate.Count(); i++ )
    ObjectsToCreate[i]->Create();

  /*somehow if the XLAbels are created before the DBasis (like after picture drawing),
  the 'disappear' from opengl selection... - the plane is drawn in different color and
  selection is inpossible, unless properties are changed, odd... could not figure out
  what is going wrong... */

  for( size_t i=0; i < XLabels.Count(); i++ )
    XLabels[i].Create();

  for( size_t i=0; i < LooseObjects.Count(); i++ )  {
    if( LooseObjects[i]->IsDeleted() )  {
      delete LooseObjects[i];
      LooseObjects.Delete(i);
      i--;
      continue;
    }
    LooseObjects[i]->Create();
  }

  FLabels->Init();
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
   FGlRender->GetBasis().SetCenter( glCenter );
  }

  GetRender().GetSelection().Create();
  GetRender().LoadIdentity();
  GetRender().SetView();
  GetRender().Initialise();
  FGlRender->SetSceneComplete(true);
  sw.stop();
  if( IsProfiling() )
    sw.print(GetLog(), &TLog::Info);
}
//..............................................................................
void TGXApp::CenterModel()  {
  const size_t ac = FXFile->GetLattice().AtomCount();
  if( ac == 0 )  return;
  vec3d maX(-100, -100, -100), miN(100, 100, 100);
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& A = FXFile->GetLattice().GetAtom(i);
    if( !A.IsDeleted() )
      vec3d::UpdateMinMax(A.crd(), miN, maX);
  }
  if( FDUnitCell->IsVisible() )  {
    vec3d cell_min, cell_max;
    FDUnitCell->GetDimensions(cell_max, cell_min);
    vec3d::UpdateMinMax(cell_min, miN, maX);
    vec3d::UpdateMinMax(cell_max, miN, maX);
  }
  vec3d Center((miN+maX)/2);
  Center *= -1;
  FGlRender->GetBasis().SetCenter(Center);
  vec3d max = FGlRender->MaxDim() + Center;
  vec3d min = FGlRender->MinDim() + Center;
  FGlRender->ClearMinMax();
  FGlRender->UpdateMaxMin(max, min);
}
//..............................................................................
void TGXApp::CenterView(bool calcZoom)  {
  vec3d maX(-100, -100, -100), miN(100, 100, 100);
  if( FXFile->GetLattice().AtomCount() == 0 )  return;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    TXAtom& XA = XAtoms[i];
    if( !XA.IsDeleted() && XA.IsVisible() )
      vec3d::UpdateMinMax(XA.Atom().crd(), miN, maX);
  }
  if( FDUnitCell->IsVisible() )  {
    vec3d cell_min, cell_max;
    FDUnitCell->GetDimensions(cell_max, cell_min);
    vec3d::UpdateMinMax(cell_min, miN, maX);
    vec3d::UpdateMinMax(cell_max, miN, maX);
  }
  vec3d Center((miN+maX)/2);
  Center *= -1;
  maX += Center;
  miN += Center;
  FGlRender->ClearMinMax();
  FGlRender->UpdateMaxMin(maX, miN);
  if( calcZoom )
    FGlRender->GetBasis().SetZoom(FGlRender->CalcZoom()*ExtraZoom);
  FGlRender->GetBasis().SetCenter(Center);
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
  try  {  CreateObjects(true);  }
  catch(...)  {
    GetRender().GetStyles().Clear();
    CreateObjects(true);
  }
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
    G->SetVisible(v);
    OnGraphicsVisible->Execute(dynamic_cast<TBasicApp*>(this), G);
    Draw();
  }
  return NULL;
}
//..............................................................................
TUndoData* TGXApp::SetGraphicsVisible( TPtrList<AGDrawObject>& G, bool v )  {
  for( size_t i=0; i < G.Count(); i++ )  {
    if( v == G[i]->IsVisible() )  continue;
    G[i]->SetVisible(v);
    OnGraphicsVisible->Execute(dynamic_cast<TBasicApp*>(this), G[i]);
  }
  Draw();
  return NULL;
}
//..............................................................................
void TGXApp::BangTable(TXAtom *XA, TTTable<TStrList>& Table) {
  const TSAtom &A = XA->Atom();
  Table.Resize(A.BondCount(), A.BondCount());
  Table.ColName(0) = A.GetLabel();
  for( size_t i=0; i < A.BondCount()-1; i++ )
    Table.ColName(i+1) = A.Bond(i).Another(A).GetLabel();

  for( size_t i=0; i < A.BondCount(); i++ )  {
    const TSBond &B = A.Bond(i);
    Table[i][0] = olxstr::FormatFloat(3, B.Length());
    Table.RowName(i) = B.Another(A).GetLabel();
    for( size_t j=0; j < A.BondCount()-1; j++ )  {
      const TSBond& B1 = A.Bond(j);
      if( i == j )  { Table[i][j+1] = '-'; continue; }
      if( i <= j )  { Table[i][j+1] = '-'; continue; }
      const vec3d V = B.Another(A).crd() - A.crd();
      const vec3d V1 = B1.Another(A).crd() - A.crd();
      if( V.QLength()*V1.QLength() != 0 )  {
        double angle = V.CAngle(V1);
        angle = acos(angle)*180/M_PI;
        Table[i][j+1] = olxstr::FormatFloat(3, angle);
      }
      else
        Table[i][j+1] = '-';
    }
  }
}
//..............................................................................
void TGXApp::BangList(TXAtom *XA, TStrList &L)  {
  const TSAtom &A = XA->Atom();
  for( size_t i=0; i < A.BondCount(); i++ )  {
    const TSBond &B = A.Bond(i);
    olxstr& T = L.Add(A.GetLabel());
    T << '-'  << B.Another(A).GetLabel();
    T << ": " << olxstr::FormatFloat(3, B.Length());
  }
  for( size_t i=0; i < A.BondCount(); i++ )  {
    const TSBond &B = A.Bond(i);
    for( size_t j=i+1; j < A.BondCount(); j++ )  {
      const TSBond &B1 = A.Bond(j);
      olxstr& T = L.Add(B.Another(A).GetLabel());
      T << '-' << A.GetLabel() << '-';
      T << B1.Another(A).GetLabel() << ": ";
      const vec3d V = B.Another(A).crd() - A.crd();
      const vec3d V1 = B1.Another(A).crd() - A.crd();
      if( V.QLength()*V1.QLength() != 0 )  {
        double angle = V.CAngle(V1);
        angle = acos(angle)*180/M_PI;
        T << olxstr::FormatFloat(3, angle);
      }
    }
  }
}
double TGXApp::Tang( TSBond *B1, TSBond *B2, TSBond *Middle, olxstr *Sequence )  {
  // right parameters should be passed, e.g. the bonds should be connecetd like
  // B1-Middle-B2 or B2-Middle->B1, otherwise the result is almost meaningless!
  if( Middle->A() == B1->A() || Middle->A() == B1->B() )
    ;
  else  {
    olx_swap(B1, B2);
  }
  // using next scheme : A1-A2-A3-A4
  TSAtom &A2 = Middle->A();
  TSAtom &A3 = Middle->B();
  TSAtom &A1 = B1->Another(A2);
  TSAtom &A4 = B2->Another(A3);
  const vec3d A = A1.crd() - A2.crd();
  const vec3d B = A3.crd() - A2.crd();
  const vec3d C = A2.crd() - A3.crd();
  const vec3d D = A4.crd() - A3.crd();

  const vec3d E = A.XProdVec(B);
  const vec3d F = C.XProdVec(D);

  if( E.QLength()*F.QLength() == 0 )  return -1;

  double ca = E.CAngle(F);
  double angle = acos(ca);
  if( Sequence != NULL )  {
    *Sequence = A1.GetLabel();
    *Sequence << '-' << A2.GetLabel() <<
                 '-' << A3.GetLabel() <<
                 '-' << A4.GetLabel();
  }
  return angle/M_PI*180;
}
void TGXApp::TangList(TXBond *XMiddle, TStrList &L)  {
  TSBondPList BondsA, BondsB;
  size_t maxl=0;
  TSAtom *A;
  TSBond *B, *Middle = &XMiddle->Bond();
  olxstr T;
  float angle;
  A = &Middle->A();
  for( size_t i=0; i < A->BondCount(); i++ )  {
    B = &A->Bond(i);
    if( B != Middle ) BondsA.Add(B);
  }
  A = &Middle->B();
  for( size_t i=0; i < A->BondCount(); i++ )  {
    B = &A->Bond(i);
    if( B != Middle ) BondsB.Add(B);
  }
  for( size_t i=0; i < BondsA.Count(); i++ )  {
    for( size_t j=0; j < BondsB.Count(); j++ )  {
      angle = Tang( BondsA[i], BondsB[j], Middle, &T);
      if( angle )  {
        T << ':' << ' ';
        if( T.Length() > maxl ) maxl = T.Length();  // to format thestring later
        T << olxstr::FormatFloat(3, angle);
        L.Add(T);
      }
    }
  }
  for( size_t i=0; i < L.Count(); i++ )  {
    size_t j = L[i].IndexOf(':');
    L[i].Insert(' ', j, maxl-j);  
  }
}
//..............................................................................
olxstr macSel_GetName2(const TSAtom& a1, const TSAtom& a2)  {
  return olxstr(a1.GetGuiLabel()) << '-' << a2.GetGuiLabel();
}
olxstr macSel_GetName3(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3)  {
  return olxstr(a1.GetGuiLabel()) << '-' << a2.GetGuiLabel() << '-' << a3.GetGuiLabel();
}
olxstr macSel_GetName4(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4)  {
  return olxstr(a1.GetGuiLabel()) << '-' << a2.GetGuiLabel() << '-' << a3.GetGuiLabel() << '-' << a4.GetGuiLabel();
}
olxstr macSel_GetName4a(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3, const TSAtom& a4)  {
  return olxstr(a1.GetGuiLabel()) << '-' << a2.GetGuiLabel() << ' ' << a3.GetGuiLabel() << '-' << a4.GetGuiLabel();
}
olxstr macSel_GetPlaneName(const TSPlane& p)  {
  if( p.Count() == 0 )  return EmptyString;
  olxstr rv(p.GetAtom(0).GetGuiLabel());
  for( size_t i=1; i < p.Count(); i++ )
    rv << ' ' << p.GetAtom(i).GetGuiLabel();
  return rv;
}
olxstr TGXApp::GetSelectionInfo()  {
  olxstr Tmp;
  try {
    double v;
    TGlGroup& Sel = FGlRender->GetSelection();
    if( Sel.Count() == 2 )  {
      if( EsdlInstanceOf(Sel[0], TXAtom) &&
        EsdlInstanceOf(Sel[1], TXAtom) )
      {
        Tmp = "Distance (";
        Tmp << macSel_GetName2(((TXAtom&)Sel[0]).Atom(), ((TXAtom&)Sel[1]).Atom());
        v = ((TXAtom&)Sel[0]).Atom().crd().DistanceTo( ((TXAtom&)Sel[1]).Atom().crd() );
        Tmp << "): " << olxstr::FormatFloat(3, v);
      }
      else if( EsdlInstanceOf(Sel[0], TXBond) && EsdlInstanceOf(Sel[1], TXBond) )  {
        TXBond& A = (TXBond&)Sel[0], &B =(TXBond&)Sel[1];
        Tmp = "Angle (";
        Tmp << macSel_GetName4a(A.Bond().A(), A.Bond().B(), B.Bond().A(), B.Bond().B()) <<
          "): ";
        v = Angle(A.Bond().A().crd(), A.Bond().B().crd(), B.Bond().A().crd(), B.Bond().B().crd());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')' <<
          "\nAngle (" <<
          macSel_GetName4a(A.Bond().A(), A.Bond().B(), B.Bond().B(), B.Bond().A()) <<
          "): ";
        v = Angle(A.Bond().A().crd(), A.Bond().B().crd(), B.Bond().A().crd(), B.Bond().B().crd());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')';
        // check for ajusten bonds
        if( !(&A.Bond().A() == &B.Bond().A() || &A.Bond().A() == &B.Bond().B() ||
          &A.Bond().B() == &B.Bond().A() || &A.Bond().B() == &B.Bond().B()) )
        {
          Tmp << "\nTorsion angle (" <<
            macSel_GetName4(A.Bond().A(), A.Bond().B(), B.Bond().B(), B.Bond().A()) <<
            "): ";
          v = TorsionAngle(A.Bond().A().crd(), A.Bond().B().crd(), B.Bond().B().crd(), B.Bond().A().crd());
          Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')' <<
            "\nTorsion angle (" <<
            macSel_GetName4(A.Bond().A(), A.Bond().B(), B.Bond().B(), B.Bond().A()) << 
            "): ";
          v = TorsionAngle(A.Bond().A().crd(), A.Bond().B().crd(), B.Bond().A().crd(), B.Bond().B().crd());
          Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')';
        }
      }
      else if( EsdlInstanceOf(Sel[0], TXLine) && EsdlInstanceOf(Sel[1], TXLine) )  {
        TXLine& A = (TXLine&)Sel[0], &B =(TXLine&)Sel[1];
        Tmp = "Angle: ";
        v = Angle(A.Edge(), A.Base(), B.Edge(), B.Base());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ")";
      }
      else if( EsdlInstanceOf(Sel[0], TXPlane) && EsdlInstanceOf(Sel[1], TXAtom) )  {
        Tmp = "Distance (plane-atom): ";
        v = ((TXPlane&)Sel[0]).Plane().DistanceTo(((TXAtom&)Sel[1]).Atom());
        Tmp << olxstr::FormatFloat(3, v) << 
          "\nDistance (plane centroid-atom): ";
        v = ((TXPlane&)Sel[0]).Plane().GetCenter().DistanceTo(((TXAtom&)Sel[1]).Atom().crd());
        Tmp << olxstr::FormatFloat(3, v);
      }
      else if( EsdlInstanceOf(Sel[0], TXAtom) && EsdlInstanceOf(Sel[1], TXPlane) )  {
        Tmp = "Distance (plane-atom): ";
        v = ((TXPlane&)Sel[1]).Plane().DistanceTo(((TXAtom&)Sel[0]).Atom());
        Tmp << olxstr::FormatFloat(3, v) <<
          "\nDistance (plane centroid-atom): ";
        v = ((TXPlane&)Sel[1]).Plane().GetCenter().DistanceTo(((TXAtom&)Sel[0]).Atom().crd());
        Tmp << olxstr::FormatFloat(3, v);
      }
      else if( EsdlInstanceOf(Sel[0], TXBond) && EsdlInstanceOf(Sel[1], TXPlane) )  {
        Tmp = "Angle (plane-bond): ";
        v = ((TXPlane&)Sel[1]).Plane().Angle(((TXBond&)Sel[0]).Bond());
        Tmp << olxstr::FormatFloat(3, v);
      }
      else if( EsdlInstanceOf(Sel[1], TXBond) && EsdlInstanceOf(Sel[0], TXPlane) )  {
        Tmp = "Angle (plane-bond): ";
        v = ((TXPlane&)Sel[0]).Plane().Angle(((TXBond&)Sel[1]).Bond());
        Tmp << olxstr::FormatFloat(3, v);
      }
      if( EsdlInstanceOf(Sel[1], TXPlane) && EsdlInstanceOf(Sel[0], TXPlane) )  {
        TSPlane &a = ((TXPlane&)Sel[0]).Plane(),
          &b = ((TXPlane&)Sel[1]).Plane();
        Tmp = "Angle (plane-plane): ";
        Tmp << olxstr::FormatFloat(3, a.Angle(b)) <<
          "\nDistance (plane centroid-plane centroid): " <<
          olxstr::FormatFloat(3, a.GetCenter().DistanceTo(b.GetCenter())) <<
          "\nDistance (plane[" << macSel_GetPlaneName(a) << "]-centroid): " <<
          olxstr::FormatFloat(3, a.DistanceTo(b.GetCenter())) <<
          "\nDistance (plane[" << macSel_GetPlaneName(b) << "]-centroid): " <<
          olxstr::FormatFloat(3, b.DistanceTo(a.GetCenter()));
      }
    }
    else if( Sel.Count() == 3 )  {
      if( EsdlInstanceOf(Sel[0], TXAtom) &&
        EsdlInstanceOf(Sel[1], TXAtom) &&
        EsdlInstanceOf(Sel[2], TXAtom) )  {
          TSAtom &a1 = ((TXAtom&)Sel[0]).Atom(),
            &a2 = ((TXAtom&)Sel[1]).Atom(),
            &a3 = ((TXAtom&)Sel[2]).Atom();
          Tmp = "Angle (";
          Tmp << macSel_GetName3(a1, a2, a3)<< "): " << 
            olxstr::FormatFloat(3, Angle(a1.crd(), a2.crd(), a3.crd()));
      }
      else if( EsdlInstanceOf(Sel[0], TXPlane) &&
        EsdlInstanceOf(Sel[1], TXPlane) &&
        EsdlInstanceOf(Sel[2], TXPlane) )  {
          TSPlane &p1 = ((TXPlane&)Sel[0]).Plane(),
            &p2 = ((TXPlane&)Sel[1]).Plane(),
            &p3 = ((TXPlane&)Sel[2]).Plane();
          Tmp = "Angle between plane centroids: ";
          Tmp << olxstr::FormatFloat(3, 
            Angle(p1.GetCenter(), p2.GetCenter(), p3.GetCenter()));
      }
    }
    else if( Sel.Count() == 4 )  {
      if( EsdlInstanceOf(Sel[0], TXAtom) &&
        EsdlInstanceOf(Sel[1], TXAtom) &&
        EsdlInstanceOf(Sel[2], TXAtom) &&
        EsdlInstanceOf(Sel[3], TXAtom) )  {
          TSAtom &a1 = ((TXAtom&)Sel[0]).Atom(),
            &a2 = ((TXAtom&)Sel[1]).Atom(),
            &a3 = ((TXAtom&)Sel[2]).Atom(),
            &a4 = ((TXAtom&)Sel[3]).Atom();
          Tmp = "Torsion angle (";
          Tmp << macSel_GetName4(a1, a2, a3, a4) << "): ";
          v = TorsionAngle(a1.crd(), a2.crd(), a3.crd(), a4.crd());
          if( v >= 0 )
            Tmp << olxstr::FormatFloat(3, v) << " (" 
            << olxstr::FormatFloat(3, 180-v) << ')';
          else 
            Tmp << "n/a (n/a)";

          Tmp << 
            "\nAngle (" << macSel_GetName3(a1, a2, a3) << "): " <<
            olxstr::FormatFloat(3, Angle(a1.crd(), a2.crd(), a3.crd())) <<
            "\nAngle (" << macSel_GetName3(a2, a3, a4) << "): " << 
            olxstr::FormatFloat(3, Angle(a2.crd(), a3.crd(), a4.crd())) <<
            "\nDistance (" << macSel_GetName2(a1, a2) << "): " << 
            olxstr::FormatFloat(3, a1.crd().DistanceTo(a2.crd())) <<
            "\nDistance (" << macSel_GetName2(a2, a3) << "): " << 
            olxstr::FormatFloat(3, a2.crd().DistanceTo(a3.crd())) <<
            "\nDistance (" << macSel_GetName2(a3, a4) << "): " << 
            olxstr::FormatFloat(3, a3.crd().DistanceTo(a4.crd()));
      }
    }
    else if( Sel.Count() == 7 )  {
      TSAtomPList atoms;
      for( size_t i=0; i < Sel.Count(); i++ )  {
        if( EsdlInstanceOf(Sel[i], TXAtom) )
          atoms.Add( ((TXAtom&)Sel[i]).Atom() );
      }
      if( atoms.Count() == 7 )  {
        TSAtom* central_atom = atoms[0];
        atoms.Delete(0);
        size_t face_cnt = 0;
        double total_val_bp = 0, test_val=0;
        for( size_t i=0; i < 6; i++ )  {
          for( size_t j=i+1; j < 6; j++ )  {
            for( size_t k=j+1; k < 6; k++ )  {
              const double thv = TetrahedronVolume( 
                central_atom->crd(),
                (atoms[i]->crd()-central_atom->crd()).Normalise() + central_atom->crd(),
                (atoms[j]->crd()-central_atom->crd()).Normalise() + central_atom->crd(),
                (atoms[k]->crd()-central_atom->crd()).Normalise() + central_atom->crd());
              if( thv < 0.1 )  continue;
              face_cnt++;
              TSAtomPList sorted_atoms;
              olxdict<index_t, vec3d, TPrimitiveComparator> transforms;
              for( size_t l=0; l < atoms.Count(); l++ )
                atoms[l]->SetTag(0);
              atoms[i]->SetTag(1);  atoms[j]->SetTag(1);  atoms[k]->SetTag(1);
              const vec3d face_center = (atoms[i]->crd()+atoms[j]->crd()+atoms[k]->crd())/3;
              const vec3d normal = (face_center - central_atom->crd()).Normalise();

              vec3d face1_center, new_normal, new_center;
              for( size_t l=0; l < atoms.Count(); l++ )
                if( atoms[l]->GetTag() == 0 )
                  face1_center += atoms[l]->crd();
              face1_center /= 3;
              
              transforms.Add(1, central_atom->crd()-face_center);
              transforms.Add(0, central_atom->crd()-face1_center);
              PlaneSort::Sorter::DoSort(atoms, transforms, central_atom->crd(), normal, sorted_atoms);
              
              TTypeList<AnAssociation2<vec3d, double> > p1;
              for( size_t l=0; l < sorted_atoms.Count(); l++ )  {
                if( sorted_atoms[l]->GetTag() == 0 )
                  p1.AddNew(sorted_atoms[l]->crd() - face1_center, 1.0);
                else
                  p1.AddNew(sorted_atoms[l]->crd() - face_center, 1.0);
              }
              TSPlane::CalcPlane(p1, new_normal, new_center, plane_best);
              new_normal.Normalise();
              for( size_t l=0; l < 3; l++ )  {
                vec3d v1 = p1[l*2].GetA() - new_normal*p1[l*2].GetA().DotProd(new_normal);
                vec3d v2 = p1[l*2+1].GetA() - new_normal*p1[l*2+1].GetA().DotProd(new_normal);
                total_val_bp += olx_abs(M_PI/3-acos(v1.CAngle(v2)));
              }
            }
          }
        }
        if( face_cnt == 8 )
          Tmp << "Combined distortion: " << olxstr::FormatFloat(2, total_val_bp*180/M_PI);
        else  {  // calculate just for the selection
          // centroids
          vec3d c1 = (atoms[0]->crd() + atoms[2]->crd() + atoms[4]->crd())/3;
          vec3d c2 = (atoms[1]->crd() + atoms[3]->crd() + atoms[5]->crd())/3;
          TTypeList<AnAssociation2<vec3d, double> > p1;
          for( size_t i=0; i < atoms.Count(); i++ )
            p1.AddNew(atoms[i]->crd() - ((i%2)==0 ? c1 : c2), 1.0);
          vec3d normal, center;
          TSPlane::CalcPlane(p1, normal, center, plane_best);
          normal.Normalise();
          double sum = 0;
          for( size_t i=0; i < 3; i++ )  {
            vec3d v1 = p1[i*2].GetA() - normal*p1[i*2].GetA().DotProd(normal);
            vec3d v2 = p1[i*2+1].GetA() - normal*p1[i*2+1].GetA().DotProd(normal);
            sum += olx_abs(M_PI/3 - acos(v1.CAngle(v2)));
          }
          Tmp << "Octahedral distortion (for the selection): " << olxstr::FormatFloat(2, (sum*180/3)/M_PI);
        }
///
      }
    }
  }
  catch( const TExceptionBase& )  {
    Tmp = "n/a";
  }
  return Tmp;
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
void TGXApp::SelectFragmentsAtoms(const TNetPList& frags, bool v)  {
  TSAtomPList SA;
  TXAtomPList XA;
  for( size_t i=0; i < frags.Count(); i++ )  {
    for( size_t j=0; j < frags[i]->NodeCount(); j++ )
      SA.Add( frags[i]->Node(j) );
  }
  SAtoms2XAtoms(SA, XA);
  for( size_t i=0; i < XA.Count(); i++ )  {
    if( v )  {
      if( !XA[i]->IsSelected() )
        GetRender().Select(*XA[i]);
    }
    else  {
      if( XA[i]->IsSelected() )
        GetRender().DeSelect(*XA[i]);
    }
  }
}
//..............................................................................
void TGXApp::SelectFragmentsBonds(const TNetPList& frags, bool v)  {
  TSBondPList SB;
  TXBondPList XB;
  for( size_t i=0; i < frags.Count(); i++ )  {
    for( size_t j=0; j < frags[i]->BondCount(); j++ )
      SB.Add( frags[i]->Bond(j) );
  }
  SBonds2XBonds(SB, XB);
  for( size_t i=0; i < XB.Count(); i++ )  {
    if( v )  {
      if( !XB[i]->IsSelected() )
        GetRender().Select(*XB[i]);
    }
    else  {
      if( XB[i]->IsSelected() )
        GetRender().DeSelect(*XB[i]);
    }
  }
}
//..............................................................................
void TGXApp::SelectFragments(const TNetPList& frags, bool v)  {
  SelectFragmentsAtoms(frags, v);
  SelectFragmentsBonds(frags, v);
}
//..............................................................................
void TGXApp::FragmentVisible(TNetwork *N, bool V)  {
  TSAtomPList SA;
  TXAtomPList XA;
//  OnFragmentVisible->Enter(dynamic_cast<TBasicApp*>(this), dynamic_cast<IEObject*>(N));
  SA.SetCapacity( N->NodeCount() );
  for( size_t i=0; i < N->NodeCount(); i++ )
    SA.Add( N->Node(i) );
  SAtoms2XAtoms(SA, XA);
  for( size_t i=0; i < XA.Count(); i++ )
    XA[i]->SetVisible(V);

  TSBondPList SB;
  TXBondPList XB;
  SB.SetCapacity( N->BondCount() );
  for( size_t i=0; i < N->BondCount(); i++ )
    SB.Add( N->Bond(i) );
  SBonds2XBonds(SB, XB);
  for( size_t i=0; i < XB.Count(); i++ )
    XB[i]->SetVisible(V);
//  OnFragmentVisible->Exit(dynamic_cast<TBasicApp*>(this), dynamic_cast<IEObject*>(N));
}
//..............................................................................
void TGXApp::FragmentsVisible(const TNetPList& Frags, bool V)  {
//  OnFragmentsVisible->Enter(this, dynamic_cast<IEObject*>(Frags));
  for( size_t i=0; i < Frags.Count(); i++ )
    FragmentVisible(Frags[i], V);
  // synchronise the intermolecular bonds
  HBondsVisible( FHBondsVisible );
  TAsymmUnit& au = XFile().GetAsymmUnit();
  TEBitArray vis( (uint32_t)au.AtomCount() );
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    const TXAtom& xa = XAtoms[i];
    if( !XAtoms[i].IsVisible() )  continue;
    vis.SetTrue( xa.Atom().CAtom().GetId() );
  }
  for( size_t i=0; i < vis.Count(); i++ )
    au.GetAtom(i).SetMasked( !vis[i] );
//  OnFragmentsVisible->Exit(this, dynamic_cast<IEObject*>(Frags));
  Draw();
}
//..............................................................................
TGlGroup& TGXApp::GroupFragments(const TNetPList& Fragments, const olxstr groupName)  {
  GetRender().GetSelection().Clear();
  TSAtomPList satoms;
  TXAtomPList xatoms;
  for( size_t i=0; i < Fragments.Count(); i++ )  {
    for( size_t j=0; j < Fragments[i]->NodeCount(); j++ )
      satoms.Add( Fragments[i]->Node(j) );
  }
  if( satoms.IsEmpty() )  return *(TGlGroup*)NULL;
  SAtoms2XAtoms(satoms, xatoms);
  for( size_t i=0; i < xatoms.Count(); i++ )
    GetRender().GetSelection().Add( *xatoms[i] );
  return *GetRender().GroupSelection(groupName);
}
//..............................................................................
size_t TGXApp::InvertFragmentsList(const TNetPList& SF, TNetPList& Result)  {
  TLattice *L = &XFile().GetLattice();
  size_t fc=0;
  for( size_t i=0; i < L->FragmentCount(); i++ )    
    L->GetFragment(i).SetTag(1);
  for( size_t i=0; i < SF.Count(); i++ )           
    SF[i]->SetTag(0);
  for( size_t i=0; i < L->FragmentCount(); i++ )  {
    if( L->GetFragment(i).GetTag() )  {
      Result.Add( L->GetFragment(i) );
      fc++;
    }
  }
  return fc;
}
//..............................................................................
void TGXApp::SyncAtomAndBondVisiblity(short atom_type, bool show_a, bool show_b)  {
  for( size_t i=0; i < XAtoms.Count(); i++ )
    XAtoms[i].Atom().SetTag(i);
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    TXAtom& a = XAtoms[i];
    if( a.Atom().GetAtomInfo() != atom_type )
      continue;
    if( atom_type == iHydrogenIndex)  {
      bool vis = false;
      size_t nc = 0;
      for( size_t j=0; j < a.Atom().NodeCount(); j++ )  {
        if( a.Atom().Node(j).IsDeleted() )
          continue;
        nc++;
        if( XAtoms[a.Atom().Node(j).GetTag()].IsVisible() )  {
          vis = true;
          break;
        }
      }
      if( nc > 0 )
        a.SetVisible(vis ? show_a : false);
      else
        a.SetVisible(show_a);
    }
    else
      a.SetVisible(show_a);
  }
  for( size_t i=0; i < XBonds.Count(); i++ )  {
    TXBond& b = XBonds[i];
    if( !XAtoms[b.Bond().A().GetTag()].IsVisible() ||
        !XAtoms[b.Bond().B().GetTag()].IsVisible() )  
    {
      b.SetVisible(false);
      continue;
    }
    if( atom_type == iHydrogenIndex )   {
      if( b.Bond().GetType() == sotHBond )
        b.SetVisible(show_b);
      else if( b.Bond().A().GetAtomInfo() == atom_type || b.Bond().B().GetAtomInfo() == atom_type )  {
        // there is always a special case...
        if( (b.Bond().A().GetAtomInfo() == iQPeakIndex || b.Bond().B().GetAtomInfo() == iQPeakIndex) &&
            !FQPeakBondsVisible )
        {
          b.SetVisible(false);
        }
        else
          b.SetVisible(show_a);
      }
    }
    else if( b.Bond().A().GetAtomInfo() == atom_type || b.Bond().B().GetAtomInfo() == atom_type )
      b.SetVisible(show_b);
  }
  if( FXGrowLinesVisible )  {
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      if( XGrowLines[i].SAtom()->GetAtomInfo() == atom_type )
        XGrowLines[i].SetVisible( XAtoms[XGrowLines[i].SAtom()->GetTag()].IsVisible() );
    }
  }
}
//..............................................................................
void TGXApp::AllVisible(bool V)  {
  OnAllVisible->Enter(dynamic_cast<TBasicApp*>(this), NULL);
  if( !V )  {
    for( size_t i=0; i < XAtoms.Count(); i++ )
      XAtoms[i].SetVisible(false);
    for( size_t i=0; i < XBonds.Count(); i++ )
      XBonds[i].SetVisible(false);
  }
  else  {
    TAsymmUnit& au = XFile().GetAsymmUnit();
    for( size_t i=0; i < au.AtomCount(); i++ )
      au.GetAtom(i).SetMasked(false);
    XFile().GetLattice().UpdateConnectivity();
    CenterView(true);
  }
  OnAllVisible->Exit(dynamic_cast<TBasicApp*>(this), NULL);
  Draw();
}
//..............................................................................
void TGXApp::Select(const vec3d& From, const vec3d& To )  {
  vec3d Cnt, Cnt1, AC;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    TXAtom& XA = XAtoms[i];
    if( XA.IsVisible() )  {
      AC = XA.Atom().crd();
      AC += GetRender().GetBasis().GetCenter();
      Cnt = AC * GetRender().GetBasis().GetMatrix();
      if( Cnt[0] < To[0] && Cnt[1] < To[1] &&
          Cnt[0] > From[0] && Cnt[1] > From[1] )  {
        if( !XA.IsSelected() )  {
          if( XA.GetPrimitives().PrimitiveCount() )  
            GetRender().Select(XA);
        }
      }
    }
  }
  for(size_t i=0; i < XBonds.Count(); i++ )  {
    TXBond& B = XBonds[i];
    if( B.IsVisible() )  {
      AC = B.Bond().A().crd();  AC += GetRender().GetBasis().GetCenter();
      Cnt  = AC * GetRender().GetBasis().GetMatrix();
      AC = B.Bond().B().crd();  AC += GetRender().GetBasis().GetCenter();
      Cnt1 = AC * GetRender().GetBasis().GetMatrix();
      if( Cnt[0] < To[0] && Cnt[1] < To[1] && Cnt[0] > From[0] && Cnt[1] > From[1] &&
          Cnt1[0] < To[0] && Cnt1[1] < To[1] && Cnt1[0] > From[0] && Cnt1[1] > From[1] )  {
        if( !B.IsSelected() )  {
          if( B.GetPrimitives().PrimitiveCount() )  
            GetRender().Select(B);
        }
      }
    }
  }
  for( size_t i=0; i < XReflections.Count(); i++ )  {
    TXReflection& XR = XReflections[i];
    if( XR.IsVisible() )  {
      Cnt = GetRender().GetBasis().GetMatrix() * XR.Center();
      if( Cnt[0] < To[0] && Cnt[1] < To[1] &&
          Cnt[0] > From[0] && Cnt[1] > From[1] )  {
        if( !XR.IsSelected() )  
          GetRender().Select(XR);
      }
    }
  }
  Draw();
}
//..............................................................................
bool TGXApp::Dispatch(int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data)  {
  if( MsgId == ID_OnSelect )  {
    const TSelectionInfo* SData = dynamic_cast<const TSelectionInfo*>(Data);
    if(  !(SData->From == SData->To) )
      Select(SData->From, SData->To);
  }
  else if( MsgId == ID_OnDisassemble && MsgSubId == msiEnter ) {
    //StoreGroups();
  }
  else if( MsgId == ID_OnDisassemble && MsgSubId == msiExit ) {
    CreateObjects(false, false);
    //RestoreGroups();
  }
  return false;
}
//..............................................................................
void TGXApp::GetSelectedCAtoms(TCAtomPList& List, bool Clear)  {
  TXAtomPList xAtoms;
  GetSelectedXAtoms(xAtoms, Clear);
  List.SetCapacity( xAtoms.Count() );
  for( size_t i=0; i < xAtoms.Count(); i++ )
    List.Add( &xAtoms[i]->Atom().CAtom() );
}
//..............................................................................
void TGXApp::ClearSelectionCopy()  {  SelectionCopy.Clear();  }
//..............................................................................
void TGXApp::BackupSelection()  {
  TGlGroup& Sel = GetSelection();
  if( !Sel.Count() )  return;
  SelectionCopy.Clear();
  for( size_t i=0; i < Sel.Count(); i++ )
    SelectionCopy.Add(Sel.GetObject(i));
}
//..............................................................................
void TGXApp::RestoreSelection()  {
  GetRender().SelectAll(false);
  for( size_t i=0; i < SelectionCopy.Count(); i++ )
    GetRender().Select( *SelectionCopy[i] );
  Draw();
}
//..............................................................................
void TGXApp::GetSelectedXAtoms(TXAtomPList& List, bool Clear)  {
  TPtrList<TGlGroup> S;
  S.Add( GetSelection() );
  for( size_t i=0; i < S.Count(); i++ )  {
    TGlGroup& Sel = *S[i];
    for( size_t j=0; j < Sel.Count(); j++ )  {
      AGDrawObject& GO = Sel[j];
      if( GO.IsDeleted() )  continue;
      if( GO.IsGroup() )  // another group
        S.Add((TGlGroup&)GO);  
      else if( EsdlInstanceOf(GO, TXAtom) )
        List.Add( (TXAtom&)GO );
    }
  }
  if( Clear )  
    SelectAll(false);
}
//..............................................................................
void TGXApp::CAtomsByType(const TBasicAtomInfo &AI, TCAtomPList& List)  {
  TAsymmUnit& AU= XFile().GetLattice().GetAsymmUnit();
  for( size_t i=0; i < AU.AtomCount(); i++ )  {
    if( AU.GetAtom(i).IsDeleted())  continue;
    if( AU.GetAtom(i).GetAtomInfo() == AI )  {
      List.Add( &AU.GetAtom(i) );
    }
  }
}
//..............................................................................
void TGXApp::XAtomsByType(const TBasicAtomInfo &AI, TXAtomPList& List, bool FindHidden) {
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].IsDeleted() )  continue;
    if( !FindHidden && !XAtoms[i].IsVisible() )  continue;
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
  for( size_t i=0; i < AU.AtomCount(); i++ )  {
    TCAtom& CA = AU.GetAtom(i);
    if( CA.IsDeleted() )  continue;
    if( CA.Label().Length() != Name.Length() )  continue;
    Tmp = CA.Label().UpperCase();
    found = true;
    for( size_t j=0; j < Name.Length(); j++ )  {
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
TEBitArray& TGXApp::GetVisibilityMask(TEBitArray& ba) const {
  for( size_t i=0; i < XAtoms.Count(); i++ )
    XAtoms[i].Atom().SetTag(i);
  ba.SetSize( (uint32_t)XAtoms.Count() );
  const TLattice& latt = XFile().GetLattice();
  for( size_t i=0; i < latt.AtomCount(); i++ )
    ba.Set(i, XAtoms[latt.GetAtom(i).GetTag()].IsVisible() );
  return ba;
}
void TGXApp::GrowAtom(TXAtom *XA, bool Shell, TCAtomPList* Template)  {
  FXFile->GetLattice().GrowAtom(XA->Atom(), Shell, Template);
}
//..............................................................................
void TGXApp::Grow(const TXAtomPList& atoms, const smatd_list& matrices)  {
  TSAtomPList satoms;
  TListCaster::POP(atoms, satoms);
  FXFile->GetLattice().GrowAtoms(satoms, matrices);
}
//..............................................................................
bool TGXApp::AtomExpandable(TXAtom *XA)  {
  return FXFile->GetLattice().IsExpandable( XA->Atom() );
}
//..............................................................................
void TGXApp::GetXAtoms(const olxstr& AtomName, TXAtomPList& res)  {
  const size_t xac = XAtoms.Count();
  const short SelMask = sgdoVisible|sgdoDeleted;
  if( AtomName.StartsFrom("#c") )  {  // TCAtom.Id
    size_t id = AtomName.SubStringFrom(2).ToSizeT();
    for( size_t i=0; i < xac; i++ )  {
      if( XAtoms[i].Atom().CAtom().GetId() == id )  
        if( XAtoms[i].MaskFlags(SelMask) == sgdoVisible )
          res.Add( &XAtoms[i] );
    }
  }
  else if( AtomName.StartsFrom("#s") )  {  // SAtom.LatId
    size_t id = AtomName.SubStringFrom(2).ToSizeT();
    for( size_t i=0; i < xac; i++ )  {
      if(XAtoms[i].Atom().GetLattId() == id )  { // only one is possible
        if( XAtoms[i].MaskFlags(SelMask) == sgdoVisible )  
          res.Add( &XAtoms[i] );
        break;
      }
    }
  }
  else if( AtomName.StartsFrom("#x") )  {  // XAtom.XAppId
    size_t id = AtomName.SubStringFrom(2).ToSizeT();
    if( id >= xac )
      throw TInvalidArgumentException(__OlxSourceInfo, "xatom id");
    TXAtom& xa = XAtoms[id];
    if( xa.MaskFlags(SelMask) == sgdoVisible )  
      res.Add( &xa );
  }
  else  {
    for( size_t i=0; i < xac; i++ )  {
      if( XAtoms[i].Atom().GetLabel().Equalsi(AtomName) )  {
        if( XAtoms[i].MaskFlags(SelMask) == sgdoVisible )  
          res.Add( &XAtoms[i] );
      }
    }
  }
}
//..............................................................................
TXAtom* TGXApp::GetXAtom(const olxstr& AtomName, bool clearSelection)  {
  if( AtomName.Equalsi("sel" ) )  {
    TXAtomPList L;
    GetSelectedXAtoms(L, clearSelection);
    if( L.Count() != 1 )  return NULL;
    return L[0];
  }
  for( size_t i=0; i < XAtoms.Count(); i++ )
    if( XAtoms[i].Atom().GetLabel().Equalsi(AtomName) )
      return  &XAtoms[i];
  return NULL;
}
//..............................................................................
void TGXApp::XAtomsByMask(const olxstr &StrMask, int Mask, TXAtomPList& List)  {
  bool found;
  if( StrMask.Length() > 32 )
    throw TInvalidArgumentException(__OlxSourceInfo, "mask is too long");
  olxstr Tmp, Name( olxstr::UpperCase(StrMask) );
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    TXAtom& XA = XAtoms[i];
    if( XA.IsDeleted() || !XA.IsVisible() )  continue;
    if( XA.Atom().GetLabel().Length() != Name.Length() )  continue;
    Tmp = olxstr::UpperCase(XA.Atom().GetLabel());
    found = true;
    for( size_t j=0; j < Name.Length(); j++ )  {
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
void TGXApp::FindXAtoms(const olxstr &Atoms, TXAtomPList& List, bool ClearSelection, bool FindHidden)  {
  TXAtom *XAFrom, *XATo;
  if( Atoms.IsEmpty() )  {  // return all atoms
    List.SetCapacity( List.Count() + XAtoms.Count() );
    for( size_t i=0; i < XAtoms.Count(); i++ )  {
      if( XAtoms[i].IsDeleted() )  continue; 
      if( !FindHidden && !XAtoms[i].IsVisible() ) continue;
      List.Add( &XAtoms[i] );
    }
    return;
  }
  TStrList Toks;
  Toks.Strtok(Atoms, ' ');
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  for( size_t i = 0; i < Toks.Count(); i++ )  {
    olxstr Tmp = Toks[i];
    if( Tmp.Equalsi("sel") )  {
      GetSelectedXAtoms(List, ClearSelection);
      continue;
    }
    if( Tmp.Equalsi("to") || Tmp.Equalsi(">") )  {
      if( (i+1) < Toks.Count() && !List.IsEmpty() )  {
        i++;
        XATo = NULL;
        if( Toks[i].Equalsi("end") )  ;
        else  {
          XATo = GetXAtom( Toks[i], ClearSelection );
          if( XATo == NULL )
            throw TInvalidArgumentException(__OlxSourceInfo, "\'to\' atoms is undefined");
        }
        XAFrom = List[List.Count()-1];
        for( size_t j=0; j < XAtoms.Count(); j++ )  {
          TXAtom& XA = XAtoms[j];
          if( XA.IsDeleted() ) continue;
          if( !FindHidden && !XA.IsVisible() )  continue;
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
      if( !Tmp.IsEmpty() )  {
        TBasicAtomInfo* BAI = AtomsInfo.FindAtomInfoBySymbol(Tmp);
        if( BAI == NULL )
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("atom type=") << Tmp);
        XAtomsByType(*BAI, List, FindHidden);
      }
      continue;
    }
    size_t ind = Tmp.FirstIndexOf('?');
    if( ind != InvalidIndex )  {
      uint16_t mask = 0x0001 << ind;
      for( size_t j=ind+1; j < Tmp.Length(); j++ )  {
        ind = Tmp.FirstIndexOf('?', j);
        if( ind != InvalidIndex )  {
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
  for(size_t i=0; i < XA.Atom().BondCount(); i++ )
    SB.Add( &XA.Atom().Bond(i) );
  SBonds2XBonds(SB, XB);
  for(size_t i=0; i < XB.Count(); i++ )  {
    TXBond* xb = XB[i];
    /* check if any of the atoms still a Q-peak */
    if( xb->Bond().A().GetAtomInfo() == iQPeakIndex ||
        xb->Bond().B().GetAtomInfo() == iQPeakIndex )  continue;
    /* check that the covalent bond really exists before showing it */
    xb->SetVisible( FXFile->GetLattice().GetNetwork().CBondExistsQ(xb->Bond().A(),
                    xb->Bond().B(), xb->Bond().QLength()) );
  }
}
//..............................................................................
TUndoData* TGXApp::ChangeSuffix(const TXAtomPList& xatoms, const olxstr &To, bool CheckLabels)  {
  TNameUndo *undo = new TNameUndo( new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoName)));
  olxstr oldL, newL;
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    oldL = xatoms[i]->Atom().GetLabel();
    newL = xatoms[i]->Atom().GetAtomInfo().GetSymbol();
    for( size_t j=xatoms[i]->Atom().GetAtomInfo().GetSymbol().Length(); j < oldL.Length(); j++ )
      if( oldL[j] >= '0' && oldL[j] <= '9' )
        newL << oldL[j];
      else
        break;
    newL << To;
    if( newL == oldL )  continue;

    if( CheckLabels )
      newL = XFile().GetAsymmUnit().CheckLabel(&xatoms[i]->Atom().CAtom(), newL);
    xatoms[i]->Atom().CAtom().Label() = newL;
    undo->AddAtom( xatoms[i]->Atom().CAtom(), oldL );
  }
  return undo;
}
//..............................................................................
TUndoData* TGXApp::Name(TXAtom& XA, const olxstr &Name, bool CheckLabel)  {
  bool checkBonds = (XA.Atom().GetAtomInfo() == iQPeakIndex);
  TBasicAtomInfo *bai = FAtomsInfo->FindAtomInfoEx(Name);

  TNameUndo *undo = new TNameUndo( new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoName)));
  olxstr oldL = XA.Atom().GetLabel();

  bool recreate = ((bai == NULL) ? true : XA.Atom().GetAtomInfo() != *bai);

  XA.Atom().SetLabel( CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA.Atom().CAtom(), Name) : Name);

  if( oldL != XA.Atom().GetLabel() )
    undo->AddAtom( XA.Atom().CAtom(), oldL );
  // Dima's complaint - leave all in for manual naming
  //NameHydrogens(XA.Atom(), undo, CheckLabel);
  if( checkBonds )  CheckQBonds(XA);
  if( recreate )  {
    XA.GetPrimitives().RemoveObject(XA);
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
  if( XA != NULL )  {
    if( ClearSelection ) SelectAll(false);
    return Name( *XA, To, CheckLabel);
  }
  else  {
    TNameUndo *undo = new TNameUndo( new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoName)));
    olxstr oldL;

    TXAtomPList Atoms, ChangedAtoms;
    olxstr Tmp, NL;
    FindXAtoms(From, Atoms, ClearSelection);
    if( From.Equalsi("sel") && To.IsNumber() )  {
      int j = To.ToInt();
      for( size_t i=0; i < Atoms.Count(); i++ )  {
        XA = Atoms[i];
        bool checkBonds = (XA->Atom().GetAtomInfo() == iQPeakIndex);
        Tmp = XA->Atom().GetLabel();
        NL = XA->Atom().GetAtomInfo().GetSymbol();
        NL << j;  j++;
        TBasicAtomInfo* bai = FAtomsInfo->FindAtomInfoEx(NL);
        bool recreate = XA->Atom().GetAtomInfo() != *bai;
        oldL = XA->Atom().GetLabel();
        XA->Atom().SetLabel( CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->Atom().CAtom(), NL) : NL );
        undo->AddAtom(XA->Atom().CAtom(), oldL);
        NameHydrogens(XA->Atom(), undo, CheckLabel);
        if( recreate )  {
          ChangedAtoms.Add( XA );
          if( checkBonds )  CheckQBonds(*XA);
        }
      }
    }
    else  if( From[0] == '$' && To[0] == '$' )  {  // change type
      for( size_t i=0; i < Atoms.Count(); i++ )  {
        XA = Atoms[i];
        bool checkBonds = (XA->Atom().GetAtomInfo() == iQPeakIndex);
        Tmp = XA->Atom().GetLabel();
        NL  = EmptyString;
        NL << To.SubStringFrom(1);
        NL << Tmp.SubStringFrom( From.Length()-1 );
        TBasicAtomInfo* bai = FAtomsInfo->FindAtomInfoEx(NL);
        bool recreate = XA->Atom().GetAtomInfo() != *bai;
        oldL = XA->Atom().GetLabel();
        XA->Atom().SetLabel( CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->Atom().CAtom(), NL) : NL );
        undo->AddAtom( XA->Atom().CAtom(), oldL);
        NameHydrogens(XA->Atom(), undo, CheckLabel);
        if( recreate )  {
          ChangedAtoms.Add( XA );
          if( checkBonds )  CheckQBonds(*XA);
        }
      }
    }
    else  {  // C2? to C3? ; Q? to Ni? ...
      for( size_t i=0; i < Atoms.Count(); i++ )  {
        XA = (TXAtom*)Atoms[i];
        bool checkBonds = (XA->Atom().GetAtomInfo() == iQPeakIndex);
        Tmp = XA->Atom().GetLabel();
        NL = To;
        for( size_t j=0; j < NL.Length(); j++ )  {
          if( NL.CharAt(j) == '?' )  {
            size_t qmi = 0;
            qmi = From.FirstIndexOf('?', qmi);
            if( qmi != InvalidIndex )  {
              NL[j] = Tmp.CharAt(qmi);
              qmi++;
            }
            else
              NL[j] = '_';
          }
        }
        TBasicAtomInfo* bai = FAtomsInfo->FindAtomInfoEx(NL);
        if( bai == NULL )
          throw TFunctionFailedException(__OlxSourceInfo, "wrong syntax");
        bool recreate = XA->Atom().GetAtomInfo() != *bai;
        oldL = XA->Atom().GetLabel();
        XA->Atom().SetLabel( CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->Atom().CAtom(), NL) : NL );
        undo->AddAtom(XA->Atom().CAtom(), oldL);
        NameHydrogens(XA->Atom(), undo, CheckLabel);
        if( recreate )  {
          ChangedAtoms.Add( XA );
          if( checkBonds )  CheckQBonds(*XA);
        }
      }
    }
    for( size_t i=0; i < ChangedAtoms.Count(); i++ )  {
      XA = ChangedAtoms[i];
      XA->GetPrimitives().RemoveObject(*XA);
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
void TGXApp::InfoList(const olxstr &Atoms, TStrList &Info, bool sort)  {
  if( !XFile().GetLattice().IsGenerated() )  {
    TCAtomPList AtomsList;
    FindCAtoms(Atoms, AtomsList, false);
    TTTable<TStrList> Table(AtomsList.Count(), 7);
    Table.ColName(0) = "Atom";
    Table.ColName(1) = "Type";
    Table.ColName(2) = "X";
    Table.ColName(3) = "Y";
    Table.ColName(4) = "Z";
    Table.ColName(5) = "Ueq";
    Table.ColName(6) = "Peak";
    for(size_t i = 0; i < AtomsList.Count(); i++ )  {
      const TCAtom& A = *AtomsList[i];
      Table[i][0] = A.GetLabel();
      Table[i][1] = A.GetAtomInfo().GetSymbol();
      Table[i][2] = olxstr::FormatFloat(3, A.ccrd()[0]);
      Table[i][3] = olxstr::FormatFloat(3, A.ccrd()[1]);
      Table[i][4] = olxstr::FormatFloat(3, A.ccrd()[2]);
      Table[i][5] = olxstr::FormatFloat(3, A.GetUiso());
      if( A.GetAtomInfo() == iQPeakIndex  )
        Table[i][6] = olxstr::FormatFloat(3, A.GetQPeak());
      else
        Table[i][6] = '-';
    }
    Table.CreateTXTList(Info, "Atom information", true, true, ' ');
  }
  else  {
    TXAtomPList AtomsList;
    FindXAtoms(Atoms, AtomsList, false);
    TTTable<TStrList> Table(AtomsList.Count(), 7);
    Table.ColName(0) = "Atom";
    Table.ColName(1) = "Type";
    Table.ColName(2) = "X";
    Table.ColName(3) = "Y";
    Table.ColName(4) = "Z";
    Table.ColName(5) = "Ueq";
    Table.ColName(6) = "Peak";
    for(size_t i = 0; i < AtomsList.Count(); i++ )  {
      const TCAtom& A = AtomsList[i]->Atom().CAtom();
      Table[i][0] = A.GetLabel();
      Table[i][1] = A.GetAtomInfo().GetSymbol();
      Table[i][2] = olxstr::FormatFloat(3, A.ccrd()[0]);
      Table[i][3] = olxstr::FormatFloat(3, A.ccrd()[1]);
      Table[i][4] = olxstr::FormatFloat(3, A.ccrd()[2]);
      Table[i][5] = olxstr::FormatFloat(3, A.GetUiso());
      if( A.GetAtomInfo() == iQPeakIndex  )
        Table[i][6] = olxstr::FormatFloat(3, A.GetQPeak());
      else
        Table[i][6] = '-';
    }
    Table.CreateTXTList(Info, "Atom information", true, true, ' ');
  }
}
//..............................................................................
TXGlLabel *TGXApp::AddLabel(const olxstr& Name, const vec3d& center, const olxstr& T)  {
  TXGlLabel* gl = new TXGlLabel(*FGlRender, Name);
  gl->SetFontIndex(FLabels->GetFontIndex());
  gl->SetLabel(T);
  gl->SetCenter(center);
  gl->Create();
  LooseObjects.Add(gl);
  return gl;
}
//..............................................................................
TXLine& TGXApp::AddLine(const olxstr& Name, const vec3d& base, const vec3d& edge)  {
  TXLine *XL = new TXLine(*FGlRender, Name, base, edge);
  XL->Create();
  LooseObjects.Add(XL);
  return *XL;
}
//..............................................................................
AGDrawObject* TGXApp::FindLooseObject(const olxstr &Name)  {
  for( size_t i=0; i < LooseObjects.Count(); i++ )
    if( LooseObjects[i]->GetPrimitives().GetName().Equalsi(Name) )
      return LooseObjects[i];
  return NULL;
}
//..............................................................................
TSPlane *TGXApp::TmpPlane(TXAtomPList* atoms, int weightExtent)  {
  TSAtomPList SAtoms;
  if( atoms != NULL )
    TListCaster::POP(*atoms, SAtoms);
  else
    TListCaster::TOP(XAtoms, SAtoms);

  if( SAtoms.Count() < 3 )  return NULL;
  return XFile().GetLattice().NewPlane(SAtoms, weightExtent);
}
//..............................................................................
TXPlane * TGXApp::AddPlane(TXAtomPList &Atoms, bool Rectangular, int weightExtent)  {
  if( Atoms.Count() < 3 )  return NULL;
  TSAtomPList SAtoms;
  for( size_t i=0; i < Atoms.Count(); i++ )
    SAtoms.Add( &Atoms[i]->Atom() );

  TSPlane *S = XFile().GetLattice().NewPlane(SAtoms, weightExtent);
  if( S )  {
    TXPlane& XP = XPlanes.Add( new TXPlane(*FGlRender, olxstr("TXPlane") << XPlanes.Count(), S) );
    S->SetRegular(Rectangular);
    XP.Create();
    return &XP;
  }
  return NULL;
}
//..............................................................................
TXPlane *TGXApp::XPlane(const olxstr &PlaneName)  {
  for( size_t i=0; i < XPlanes.Count(); i++ )
    if( XPlanes[i].GetPrimitives().GetName().Equalsi(PlaneName) )
      return &XPlanes[i];
  return NULL;
}
//..............................................................................
void TGXApp::DeletePlane(TXPlane* plane)  {
  plane->Plane().SetDeleted(true);
}
//..............................................................................
void TGXApp::ClearPlanes()  {
  for( size_t i=0; i < XPlanes.Count(); i++ )
    XPlanes[i].SetDeleted(true);
}
//..............................................................................
TXAtom * TGXApp::AddCentroid(TXAtomPList& Atoms)  {
  if( Atoms.Count() < 2 )  return NULL;
  TSAtomPList SAtoms;
  for( size_t i=0; i < Atoms.Count(); i++ )
    SAtoms.Add(Atoms[i]->Atom());
  TSAtom *A = XFile().GetLattice().NewCentroid( SAtoms );
  if( A != NULL )  {
    TXAtom& XA = XAtoms.AddNew( *(new TXAtom(*FGlRender, EmptyString, *A)) );
    XA.Create();
    XA.SetXAppId( XAtoms.Count() - 1);
    XA.Params()[0] = (float)A->GetAtomInfo().GetRad();
    return &XA;
  }
  return NULL;
}
//..............................................................................
void TGXApp::AdoptAtoms(const TAsymmUnit& au, TXAtomPList& xatoms) {
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    const TCAtom& ca = au.GetAtom(i);
    vec3d center = ca.ccrd();
    XFile().GetAsymmUnit().CartesianToCell(center);
    TSAtom *A = XFile().GetLattice().NewAtom( center );
    if( A != NULL )  {
      A->CAtom().SetAtomInfo( ca.GetAtomInfo() );
      A->CAtom().Label() = ca.GetLabel();
      TXAtom& XA = XAtoms.Add( new TXAtom(*FGlRender, EmptyString, *A) );
      XA.Create();
      XA.SetXAppId( XAtoms.Count() - 1 );
      XA.Params()[0] = (float)A->GetAtomInfo().GetRad();
      xatoms.Add(&XA);
    }
  }
}
//..............................................................................
TXAtom* TGXApp::AddAtom(TXAtom* templ)  {
  vec3d center;
  if( templ != NULL )
    center = templ->Atom().CAtom().ccrd();
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  TSAtom *A = XFile().GetLattice().NewAtom( center );
  if( A != NULL )  {
    olxstr colName;
    if( templ != NULL )  {
      colName = templ->GetCollectionName();
      A->CAtom().SetAtomInfo( templ->Atom().GetAtomInfo() );
      if( templ->Atom().GetAtomInfo() == iQPeakIndex )
        A->CAtom().SetQPeak(1.0);
    }
    else  {
      A->CAtom().SetAtomInfo( AtomsInfo.GetAtomInfo(6) );
    }
    TXAtom& XA = XAtoms.Add( new TXAtom(*FGlRender, colName, *A) );
    XA.Create();
    XA.SetXAppId( XAtoms.Count() - 1 );
    XA.Params()[0] = (float)A->GetAtomInfo().GetRad();
    return &XA;
  }
  return NULL;
}
//..............................................................................
void TGXApp::undoDelete(TUndoData *data)  {
  TKillUndo *undo = dynamic_cast<TKillUndo*>(data);
  TLattice& latt = XFile().GetLattice();
  for( size_t i=0; i < undo->SAtomIds.Count(); i++ )
    latt.RestoreAtom(undo->SAtomIds[i]);
  XFile().GetLattice().UpdateConnectivity();
}
//..............................................................................
void TGXApp::undoName(TUndoData *data)  {
  TNameUndo *undo = dynamic_cast<TNameUndo*>(data);
  TCAtomPList cal;
  TAsymmUnit& au = XFile().GetAsymmUnit();
  for( size_t i=0; i < undo->AtomCount(); i++ )  {
    if( undo->GetCAtomId(i) >= au.AtomCount() )  //could happen?
      continue;
    TCAtom& ca = au.GetAtom( undo->GetCAtomId(i));
    TBasicAtomInfo* bai = FAtomsInfo->FindAtomInfoEx( undo->GetLabel(i) );
    ca.Label() = undo->GetLabel(i);
    if( ca.GetAtomInfo() != *bai )
      cal.Add(ca)->SetAtomInfo(*bai);
  }
  // could be optimised...
  if( !cal.IsEmpty() )
    CreateObjects(false, false);
}
//..............................................................................
void TGXApp::undoHide(TUndoData *data)  {
  THideUndo *undo = dynamic_cast<THideUndo*>(data);
  for( size_t i=0; i < undo->Objects.Count(); i++ )  {
    undo->Objects[i]->SetVisible(true);
  }
}
//..............................................................................
TUndoData* TGXApp::DeleteXObjects(TPtrList<AGDrawObject>& L)  {
  TXAtomPList atoms;
  atoms.SetCapacity(L.Count());
  for( size_t i=0; i < L.Count(); i++ )  {
    if( EsdlInstanceOf(*L[i], TXAtom) )  
      atoms.Add( (TXAtom*)L[i] );
    else if( EsdlInstanceOf(*L[i], TXPlane) )  {
      TXPlane* xp = (TXPlane*)L[i];
      xp->SetDeleted(true);
    }
    else if( EsdlInstanceOf(*L[i], TXBond) )  {
      TXBond* xb = (TXBond*)L[i];
      xb->SetVisible(false);
    }
    else
      L[i]->SetDeleted(true);
  }
  return DeleteXAtoms(atoms);
}
//..............................................................................
TUndoData* TGXApp::DeleteXAtoms(TXAtomPList& L)  {
  TKillUndo *undo = new TKillUndo(new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoDelete)));
  if( L.IsEmpty() )
    return undo;
  TSAtomPList SAL;
  for( size_t i=0; i < L.Count(); i++ )  {
    TXAtom* XA = L[i];
    TSAtom& SA = XA->Atom();
    if( SA.IsDeleted() )  continue;
    undo->AddSAtom( XA->Atom() );
    for( size_t j=0; j < SA.NodeCount();j++ ) {
      TSAtom& SH = SA.Node(j);
      if( SH.IsDeleted() )  continue;
      if( SA.GetAtomInfo().GetMr() > 3.5 && SH.GetAtomInfo() == iHydrogenIndex )  
        SAL.Add(SH)->SetDeleted(true);
    }
    XA->SetDeleted(true);
  }

  TXAtomPList XAL;
  SAtoms2XAtoms(SAL, XAL);
  for( size_t i=0; i < XAL.Count(); i++ )  {  
    XAL[i]->SetDeleted(true);
    undo->AddSAtom( XAL[i]->Atom() );
  }

  //GetSelection().RemoveDeleted();
  GetSelection().Clear();
  XFile().GetLattice().UpdateConnectivity();
  //CenterView();
  return undo;
}
//..............................................................................
void TGXApp::SelectBondsWhere(const olxstr &Where, bool Invert)  {
  olxstr str( olxstr::LowerCase(Where) );
  if( str.FirstIndexOf("xatom") != InvalidIndex || str.FirstIndexOf("satom") != InvalidIndex)  {
    Log->Error("SelectBonds: xatom/satom are not allowed here");
    return;
  }
  if( str.FirstIndexOf("sel") != InvalidIndex )  {
    if( FGlRender->GetSelection().Count() != 1 )  {
      Log->Error("SelectBonds: please select one bond only");
      return;
    }
    if( !EsdlInstanceOf(FGlRender->GetSelection()[0], TXBond) )  {
      Log->Error("SelectBonds: please select a bond");
      return;
    }
  }
  TXFactoryRegister rf;
  TTXBond_EvaluatorFactory *xbond = (TTXBond_EvaluatorFactory*)rf.BindingFactory("xbond");
  TTGlGroupEvaluatorFactory *sel = (TTGlGroupEvaluatorFactory*)rf.BindingFactory("sel");
  sel->SetTGlGroup( &FGlRender->GetSelection() );
  TSyntaxParser SyntaxParser(&rf, Where);
  if( !SyntaxParser.Errors().Count() )  {
    for( size_t i=0; i < XBonds.Count(); i++ )  {
      if( XBonds[i].IsSelected() )  continue;
      xbond->SetTXBond_( &XBonds[i] );
      if( SyntaxParser.Evaluate() )  
        GetRender().Select( XBonds[i] );
    }
  }
  else
    Log->Error( SyntaxParser.Errors().Text('\n') );
}
//..............................................................................
void TGXApp::SelectAtomsWhere(const olxstr &Where, bool Invert)  {
  olxstr str( olxstr::LowerCase(Where) );
  if( str.FirstIndexOf("xbond") != InvalidIndex || str.FirstIndexOf("satom") != InvalidIndex )  {
    Log->Error("SelectAtoms: xbond/satom are not allowed here");
    return;
  }
  if( str.FirstIndexOf("sel") != InvalidIndex )  {
    if( FGlRender->GetSelection().Count() != 1 )  {
      Log->Error("SelectAtoms: please select one atom only");
      return;
    }
    if( !EsdlInstanceOf(FGlRender->GetSelection()[0], TXAtom) )  {
      Log->Error("SelectAtoms: please select an atom");
      return;
    }
  }
  TXFactoryRegister rf;
  TTXAtom_EvaluatorFactory *xatom = (TTXAtom_EvaluatorFactory*)rf.BindingFactory("xatom");
  TTGlGroupEvaluatorFactory *sel = (TTGlGroupEvaluatorFactory*)rf.BindingFactory("sel");
  sel->SetTGlGroup( &FGlRender->GetSelection() );
  TSyntaxParser SyntaxParser(&rf, Where);
  if( !SyntaxParser.Errors().Count() )  {
    for( size_t i=0; i < XAtoms.Count(); i++ )  {
      if( XAtoms[i].IsSelected() )  continue;
      xatom->SetTXAtom( &XAtoms[i] );
      if( SyntaxParser.Evaluate() )  
        GetRender().Select( XAtoms[i] );
    }
  }
  else
    Log->Error( SyntaxParser.Errors().Text('\n') );
}
//..............................................................................
bool GetRing(TSAtomPList& atoms, TTypeList<TSAtomPList>& rings)  {
  TSAtomPList *ring = NULL;
  size_t starta = 0;
  for( size_t i=0; i < atoms.Count(); i++ )  {
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
    for( size_t i=starta; i < atoms.Count(); i++ )  {
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
  if( Condition.Equalsi("sel") )  {
    TXAtomPList L;
    TSAtomPList SAtoms;
    GetSelectedXAtoms(L, false);
    TListCaster::POP(L, SAtoms);
    for( size_t i=0; i < SAtoms.Count(); i++ )
      SAtoms[i]->SetTag(0);
    while( GetRing(SAtoms, rings) )
      ;
    return;
  }
  TXApp::FindRings(Condition, rings);
}
//..............................................................................
void SortRing(TSAtomPList& atoms)  {
  size_t maxbc = 0, ind = InvalidIndex;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    size_t bc = 0;
    for( size_t j=0; j < atoms[i]->NodeCount(); j++ )
      if( atoms[i]->Node(j).GetAtomInfo() != iHydrogenIndex )
        bc++;
    if( bc > maxbc )  {
      maxbc = bc;
      ind = i;
    }
  }
  if( ind != InvalidIndex && ind > 0 )
    atoms.ShiftL( ind );
}

void TGXApp::SelectRings(const olxstr& Condition, bool Invert)  {
  TTypeList< TSAtomPList > rings;
  try  {  FindRings(Condition, rings);  }
  catch( const TExceptionBase& exc )  {  throw TFunctionFailedException(__OlxSourceInfo, exc);  }

  if( rings.IsEmpty() )  return;

  TXAtomPList XA( rings.Count()*rings[0].Count() );
  TSAtomPList allSAtoms;
  allSAtoms.SetCapacity( XA.Count() );
  for( size_t i=0; i < rings.Count(); i++ )  {
    SortRing(rings[i]);
    for( size_t j=0; j < rings[i].Count(); j++ )
      allSAtoms.Add( rings[i][j] );
  }
  for( size_t i=0; i < XFile().GetLattice().AtomCount(); i++ )
    XFile().GetLattice().GetAtom(i).SetTag(-1);

  for( size_t i=0; i < allSAtoms.Count(); i++ )
    allSAtoms[i]->SetTag(i);

  for( size_t i=0; i < XAtoms.Count(); i++ )
    if( XAtoms[i].Atom().GetTag() != -1 )  {
      XA[ XAtoms[i].Atom().GetTag() ] =&XAtoms[i];
    }

  XA.Pack();

  for( size_t i=0; i < XA.Count(); i++ )
    XA[i]->SetTag(i);
  for( size_t i=0; i < XA.Count(); i++ )
    if( XA[i]->GetTag() == i && XA[i]->IsVisible() )
      FGlRender->Select( *XA[i] );
}
//..............................................................................
void TGXApp::SelectAtoms(const olxstr &Names, bool Invert)  {
  TXAtomPList Sel;
  FindXAtoms(Names, Sel, true);
  for( size_t i=0; i < Sel.Count(); i++ )  {
    if( Invert )
      GetRender().Select( *Sel[i] );
    else
      if( !Sel[i]->IsSelected() )  
        GetRender().Select( *Sel[i] );
  }
}
//..............................................................................
void TGXApp::ExpandSelection(TCAtomGroup& atoms)  {
  TXAtomPList xatoms;
  GetSelectedXAtoms(xatoms, GetDoClearSelection());
  for( size_t i=0; i < xatoms.Count(); i++ )
    atoms.AddNew( &xatoms[i]->Atom().CAtom(), &xatoms[i]->Atom().GetMatrix(0) );
}
//..............................................................................
void TGXApp::ExpandSelectionEx(TSAtomPList& atoms)  {
  TXAtomPList xatoms;
  GetSelectedXAtoms(xatoms, GetDoClearSelection());
  atoms.SetCapacity(atoms.Count()+xatoms.Count());
  for( size_t i=0; i < xatoms.Count(); i++ )
    atoms.Add( xatoms[i]->Atom() );
}
//..............................................................................
void TGXApp::FindCAtoms(const olxstr &Atoms, TCAtomPList& List, bool ClearSelection)  {
  if( Atoms.IsEmpty() )  {
    GetSelectedCAtoms(List, ClearSelection);
    if( !List.IsEmpty() )  return;
    TAsymmUnit& AU = XFile().GetLattice().GetAsymmUnit();
    List.SetCapacity( List.Count() + AU.AtomCount() );
    for( size_t i=0; i < AU.ResidueCount(); i++ )  {
      TResidue& resi = AU.GetResidue(i);
      for( size_t j=0; j < resi.Count(); j++ )  {
        if( !resi[j].IsDeleted() )
          List.Add( &resi[j] );
      }
    }
    return;
  }
  TStrList Toks(Atoms, ' ');
  olxstr Tmp;
  TBasicAtomInfo *BAI;
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  for( size_t i = 0; i < Toks.Count(); i++ )  {
    Tmp = Toks[i];
    if( Tmp.Equalsi("sel") )  {
      GetSelectedCAtoms(List, ClearSelection);
      continue;
    }
    if( Tmp.CharAt(0) == '$' )  {
      Tmp = Tmp.SubStringFrom(1);
      if( Tmp.Length() != 0 )  {
        BAI = AtomsInfo.FindAtomInfoBySymbol(Tmp);
        if( BAI == NULL )
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("atom type=") << Tmp);
        CAtomsByType(*BAI, List);
      }
      continue;
    }
    size_t ind = Tmp.FirstIndexOf('?');
    if( ind != InvalidIndex )  {
      int mask = 0x0001 << ind;
      for( size_t j=ind+1; j < Tmp.Length(); j++ )  {
        ind = Tmp.FirstIndexOf('?', j);
        if( ind != InvalidIndex )  {
          mask |= 0x0001 << ind;
          j = ind;
        }
      }
      CAtomsByMask(Tmp, mask, List);
      continue;
    }
    TCAtom* A = XFile().GetAsymmUnit().FindCAtom(Tmp);
    if( A != NULL )
      if( !A->IsDeleted() )  List.Add(A);
  }
}
//..............................................................................
bool TGXApp::AreLabelsVisible()    const {  return FLabels->IsVisible(); }
//..............................................................................
void TGXApp::SetLabelsVisible(bool v)    {  FLabels->SetVisible(v); }
//..............................................................................
void TGXApp::SetLabelsMode(short lmode)  {  FLabels->SetMode(lmode); }
//..............................................................................
short TGXApp::GetLabelsMode()      const {  return FLabels->GetMode(); }
//..............................................................................
void TGXApp::SetLabelsFont(short Findex){  FLabels->SetFontIndex(Findex);  }
//..............................................................................
TGlMaterial& TGXApp::LabelsMarkMaterial()  {  return FLabels->MarkMaterial();  }
//..............................................................................
void TGXApp::MarkLabel(const TXAtom& A, bool v)  {  FLabels->MarkLabel(A, v);  }
//..............................................................................
void TGXApp::MarkLabel(size_t i, bool v)  {  FLabels->MarkLabel(i, v);  }
//..............................................................................
bool TGXApp::IsLabelMarked(const TXAtom& A) const {  return FLabels->IsLabelMarked(A);  }
//..............................................................................
bool TGXApp::IsLabelMarked(size_t i) const {  return FLabels->IsLabelMarked(i);  }
//..............................................................................
void TGXApp::ClearLabelMarks()  {  FLabels->ClearLabelMarks();  }
//..............................................................................
void TGXApp::ClearLabels()  {
  FLabels->Clear();
  XLabels.Clear();
}
//..............................................................................
void TGXApp::SBonds2XBonds(TSBondPList& L, TXBondPList& Res)  {
  if( L.IsEmpty() )  return;
  TLattice& latt = L[0]->GetNetwork().GetLattice();

  for( size_t i=0; i < latt.BondCount(); i++ )
    latt.GetBond(i).SetTag(0);

  for( size_t i=0; i < L.Count(); i++ )
    L[i]->SetTag(1);

  Res.SetCapacity( Res.Count() + L.Count() );
  for( size_t i=0; i < XBonds.Count(); i++ )
    if( &XBonds[i].Bond().GetNetwork().GetLattice() == &latt && XBonds[i].Bond().GetTag() != 0 )
      Res.Add( &XBonds[i] );
}
//..............................................................................
void TGXApp::SPlanes2XPlanes(TSPlanePList& L, TXPlanePList& Res)  {
  if( L.IsEmpty() )  return;
  TLattice* latt = &L[0]->GetNetwork().GetLattice();

  for( size_t i=0; i < latt->PlaneCount(); i++ )
    latt->GetPlane(i).SetTag(0);

  for( size_t i=0; i < L.Count(); i++ )
    L[i]->SetTag(1);

  Res.SetCapacity( Res.Count() + L.Count() );
  for( size_t i=0; i < XPlanes.Count(); i++ )
    if( &XPlanes[i].Plane().GetNetwork().GetLattice() == latt && XPlanes[i].Plane().GetTag() != 0 )
      Res.Add( &XPlanes[i] );
}
//..............................................................................
void TGXApp::SAtoms2XAtoms(TSAtomPList& L, TXAtomPList& Res)  {
  if( L.IsEmpty() )  return;
  TLattice* latt = &L[0]->GetNetwork().GetLattice();

  for( size_t i=0; i < latt->AtomCount(); i++ )
    latt->GetAtom(i).SetTag(0);

  for( size_t i=0; i < L.Count(); i++ )
    L[i]->SetTag(1);

  Res.SetCapacity( Res.Count() + L.Count() );
  for( size_t i=0; i < XAtoms.Count(); i++ )
    if( &XAtoms[i].Atom().GetNetwork().GetLattice() == latt && XAtoms[i].Atom().GetTag() != 0 )
      Res.Add( &XAtoms[i] );
}
//..............................................................................
void TGXApp::GetBonds(const olxstr& Bonds, TXBondPList& List)  {
  if( Bonds.IsEmpty() || Bonds.Equalsi("sel") )  {
    TGlGroup& sel = GetRender().GetSelection();
    TSBondPList sbonds;
    for( size_t i=0; i < sel.Count(); i++ )  {
      if( EsdlInstanceOf(sel[i], TXBond) )
        List.Add( (TXBond&)sel[i]);
      else if( EsdlInstanceOf(sel[i], TXAtom) ) {
        TSAtom& sa = ((TXAtom&)sel[i]).Atom();
        for( size_t j=0; j < sa.BondCount(); j++ )
          sbonds.Add(sa.Bond(j));
      }
    }
    for( size_t i=0; i < sbonds.Count(); i++ )
      sbonds[i]->SetTag(i);
    for( size_t i=0; i < sbonds.Count(); i++ )
      if( sbonds[i]->GetTag() != i )
        sbonds[i] = NULL;
    sbonds.Pack();
    if( !sbonds.IsEmpty() )  {
      SBonds2XBonds(sbonds, List);
      for( size_t i=0; i < List.Count(); i++ )
        List[i]->SetTag(i);
      for( size_t i=0; i < List.Count(); i++ )
        if( List[i]->GetTag() != i )
          List[i] = NULL;
      List.Pack();
    }
    return;
  }
  TGPCollection *GPC = GetRender().FindCollection(Bonds);
  if( GPC == NULL )  return;
  for( size_t i=0; i < GPC->ObjectCount(); i++ )  {
    if( i == 0 )  {  // check if the right type !
      if( !EsdlInstanceOf( GPC->GetObject(0), TXBond) )  
        return;
    }
    List.Add( (TXBond*)&GPC->GetObject(i) );
  }
}
//..............................................................................
void TGXApp::AtomRad(const olxstr& Rad, TXAtomPList* Atoms)  { // pers, sfil
  short DS = -1;
  if( Rad.Equalsi("sfil") ) {
    DS = darPack;  
    if( Atoms == NULL )
      TXAtom::DefRad(darPack); 
  }
  else if( Rad.Equalsi("pers") ) {  
    DS = darPers;  
    if( Atoms == NULL )
      TXAtom::DefRad(darPers); 
  }
  else if( Rad.Equalsi("isot") ) {  
    DS = darIsot;  
    if( Atoms == NULL )
      TXAtom::DefRad(darIsot); 
  }
  else if( Rad.Equalsi("isoth") ) {  
    DS = darIsotH;  
    if( Atoms == NULL )
      TXAtom::DefRad(darIsotH); 
  }
  else if( Rad.Equalsi("bond") ) {  
    DS = darBond;  
    if( Atoms == NULL )
      TXAtom::DefRad(darIsot); 
  }
  if( DS == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "rad");

  if( Atoms != NULL )  {  // make sure all atoms of selected collections are updated
    for( size_t i=0; i < Atoms->Count(); i++ )
      (*Atoms)[i]->GetPrimitives().SetTag(i);
    for( size_t i=0; i < Atoms->Count(); i++ )  {
      if( (*Atoms)[i]->GetPrimitives().GetTag() == i )  {
        TGPCollection& gpc = (*Atoms)[i]->GetPrimitives();
        for( size_t j=0; j < gpc.ObjectCount(); j++ )
          ((TXAtom&)gpc.GetObject(j)).CalcRad(DS);
      }
    }
  }
  else {
    for( size_t i=0; i < XAtoms.Count(); i++ )
      XAtoms[i].CalcRad(DS);
  }
  if( Atoms == NULL )  { // 
    TXAtom::DefZoom(1);
    TXAtom::TelpProb(1);
  }
}
//..............................................................................
void TGXApp::GetGPCollections(TPtrList<AGDrawObject>& GDObjects, TPtrList<TGPCollection>& Result)  {
  for( size_t i=0; i < GDObjects.Count(); i++ )
    GDObjects[i]->GetPrimitives().SetTag(i);

  for( size_t i=0; i < GDObjects.Count(); i++ )  {
    if( GDObjects[i]->GetPrimitives().GetTag() == i )
      Result.Add( &GDObjects[i]->GetPrimitives() );
  }
}
//..............................................................................
void TGXApp::FillXAtomList( TXAtomPList& res, TXAtomPList* providedAtoms) {
  if( providedAtoms != NULL )  {
    res.AddList( *providedAtoms );
  }
  else  {
    res.SetCapacity( XAtoms.Count() );
    for( size_t i=0; i < XAtoms.Count(); i++ )
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
    for( size_t i=0; i < XBonds.Count(); i++ )
      res.Add( &XBonds[i] );
  }
}
//..............................................................................
void TGXApp::AtomZoom(float Zoom, TXAtomPList* Atoms)  {  // takes %
  TPtrList<AGDrawObject> objects;
  if( Atoms != NULL )  {
    objects.SetCapacity( Atoms->Count() );
    for( size_t i=0; i < Atoms->Count(); i++ )
      objects.Add( (AGDrawObject*)Atoms->Item(i) );
  }
  else  {
    objects.SetCapacity( XAtoms.Count() );
    for( size_t i=0; i < XAtoms.Count(); i++ )
      objects.Add( (AGDrawObject*)&XAtoms[i] );
  }
  TPtrList<TGPCollection> Colls;
  GetGPCollections(objects, Colls);
  for( size_t i=0; i < Colls.Count(); i++ )  {
    if( Colls[i]->ObjectCount() )  {
      ((TXAtom&)Colls[i]->GetObject(0)).SetZoom(Zoom/100);
    }
  }
}
//..............................................................................
void TGXApp::SetQPeakScale(float V)  {
  TXAtom::SetQPeakScale(V);
  if( XAtoms.IsEmpty() )  return;
  TPtrList<TGPCollection> Colls;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetAtomInfo() == iQPeakIndex )
      Colls.Add(XAtoms[i].GetPrimitives());
  }
  FGlRender->RemoveCollections(Colls);
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetAtomInfo() == iQPeakIndex )
      XAtoms[i].Create();
  }

}
//..............................................................................
float TGXApp::GetQPeakScale()  {
  return TXAtom::GetQPeakScale();
}
//..............................................................................
void TGXApp::SetQPeakSizeScale(float V)  {
  TXAtom::SetQPeakSizeScale(V);
  if( XAtoms.IsEmpty() )  return;
  TPtrList<TGPCollection> Colls;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetAtomInfo() == iQPeakIndex )
      Colls.Add(XAtoms[i].GetPrimitives());
  }
  FGlRender->RemoveCollections(Colls);
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetAtomInfo() == iQPeakIndex )
      XAtoms[i].Create();
  }

}
//..............................................................................
float TGXApp::GetQPeakSizeScale()  {
  return TXAtom::GetQPeakSizeScale();
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
  for( size_t i=0; i < Colls.Count(); i++ )  {
    if( Colls[i]->ObjectCount() != 0 )  {
      ((TXBond&)Colls[i]->GetObject(0)).SetRadius(R);
    }
  }
}
//..............................................................................
void TGXApp::UpdateAtomPrimitives(int Mask, TXAtomPList* Atoms) {
  TXAtomPList atoms;
  FillXAtomList( atoms, Atoms );
  for( size_t i=0; i < atoms.Count(); i++ )
    atoms[i]->GetPrimitives().SetTag(i);
  for( size_t i=0; i < atoms.Count(); i++ )
    if( atoms[i]->GetPrimitives().GetTag() == i )
      atoms[i]->UpdatePrimitives(Mask);
  if( Atoms == NULL )  {
    TXAtom::DefMask(Mask);
    for( size_t i=0; i < IndividualCollections.Count(); i++ )
      if( IndividualCollections[i].IndexOf('-') == InvalidIndex )
        IndividualCollections[i] = EmptyString;
    IndividualCollections.Pack();
  }
}
//..............................................................................
void TGXApp::UpdateBondPrimitives(int Mask, TXBondPList* Bonds, bool HBondsOnly)  {
  TXBondPList bonds;
  FillXBondList(bonds, Bonds);
  for( size_t i=0; i < XAtoms.Count(); i++ )  XAtoms[i].Atom().SetTag(i);
  for( size_t i=0; i < bonds.Count(); i++ )  bonds[i]->GetPrimitives().SetTag(i);

  if( HBondsOnly )  {
    for( size_t i=0; i < bonds.Count(); i++ )  {
      if( bonds[i]->Bond().GetType() != sotHBond )  continue;
      if( bonds[i]->GetPrimitives().GetTag() == i )  {
        BondCreationParams bcpar(XAtoms[bonds[i]->Bond().A().GetTag()], 
          XAtoms[bonds[i]->Bond().B().GetTag()]);
        bonds[i]->UpdatePrimitives(Mask, &bcpar);
      }
    }
  }
  else  {
    for( size_t i=0; i < bonds.Count(); i++ )  {
      if( bonds[i]->Bond().GetType() == sotHBond )  continue;
      if( bonds[i]->GetPrimitives().GetTag() == i )  {
        BondCreationParams bcpar(XAtoms[bonds[i]->Bond().A().GetTag()], 
          XAtoms[bonds[i]->Bond().B().GetTag()]);
        bonds[i]->UpdatePrimitives(Mask, &bcpar);
      }
    }
  }
  if( Bonds == NULL )  {
    for( size_t i=0; i < IndividualCollections.Count(); i++ )
      if( IndividualCollections[i].IndexOf('-') != InvalidIndex )
        IndividualCollections[i] = EmptyString;
    IndividualCollections.Pack();
    if( !HBondsOnly )
      TXBond::DefMask(Mask);
  }
}
//..............................................................................
void TGXApp::SetAtomDrawingStyle(short ADS, TXAtomPList* Atoms)  {
  TXAtomPList atoms;
  FillXAtomList(atoms, Atoms);
  for( size_t i=0; i < atoms.Count(); i++ )
    atoms[i]->DrawStyle(ADS);
  if( Atoms == NULL )  {
    CalcProbFactor(FProbFactor);
    TXAtom::DefDS(ADS);
  }
}
//..............................................................................
void TGXApp::XAtomDS2XBondDS(const olxstr &Source)  {
  int dds;
  for( size_t i=0; i < XAtoms.Count(); i++ )  XAtoms[i].Atom().SetTag(i);
  for( size_t i=0; i < XBonds.Count(); i++ )  XBonds[i].GetPrimitives().SetTag(i);

  for( size_t i=0; i < XBonds.Count(); i++ )  {
    if( XBonds[i].GetPrimitives().GetTag() != i )  continue;
    TXBond& XB = XBonds[i];
    const short bll = TXAtom::LegendLevel(XB.GetPrimitives().GetName());
    TGlMaterial *GlMA = NULL, *GlMB = NULL;
    TXAtom* XA = &XAtoms[ XB.Bond().A().GetTag() ];
    if( TXAtom::LegendLevel(XA->GetPrimitives().GetName()) >= bll )  {
      TGlPrimitive *AGlP = XA->GetPrimitives().FindPrimitiveByName(Source);
      if( AGlP == NULL )  continue;
      GlMA = &AGlP->GetProperties();
    }
    XA = &XAtoms[ XB.Bond().B().GetTag() ];
    if( TXAtom::LegendLevel(XA->GetPrimitives().GetName()) >= bll )  {
      TGlPrimitive *BGlP = XA->GetPrimitives().FindPrimitiveByName(Source);
      if( BGlP == NULL )  continue;
      GlMB = &BGlP->GetProperties();
    }
    if( GlMA == NULL && GlMB == NULL )  continue;
    for( size_t j=0; j < XB.GetPrimitives().PrimitiveCount(); j++ )  {
      TGlPrimitive& GlP = XBonds[i].GetPrimitives().GetPrimitive(j);
      if( GlP.Params.Count() >= 1 )  {
        dds = (int)GlP.Params.Last();
        if( dds == ddsDefAtomA && GlMA != NULL )  {  // from atom A
          GlP.SetProperties(*GlMA);
          XB.GetPrimitives().GetStyle().SetMaterial(GlP.GetName(), *GlMA);
          continue;
        }
        if( dds == ddsDef  && GlMA != NULL )  {  // from haviest atom
          GlP.SetProperties(*GlMA);
          XB.GetPrimitives().GetStyle().SetMaterial(GlP.GetName(), *GlMA);
          continue;
        }
        if( dds == ddsDefAtomB  && GlMB != NULL )  {
          GlP.SetProperties(*GlMB);
          XB.GetPrimitives().GetStyle().SetMaterial(GlP.GetName(), *GlMB);
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
double TGXApp::CalcVolume(const TSStrPObjList<olxstr,double, true>* volumes, olxstr &report)  {
  if( !FXFile )  {  report = "File is not loaded";  return 0;  }
  size_t ac = FXFile->GetLattice().AtomCount();
  size_t bc = FXFile->GetLattice().BondCount();
  if( ac == 0 )  {  report = "Could not find any atoms";  return 0;  }
  for( size_t i=0; i < bc; i++ )
    FXFile->GetLattice().GetBond(i).SetTag(0);
  double R1, R2, h1, h2, d, Vi=0, Vt=0;
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& SA = FXFile->GetLattice().GetAtom(i);
    if( SA.IsDeleted() )  continue;
    if( SA.GetAtomInfo() == iQPeakIndex )  continue;
    R1 = 0;
    if( volumes )  {
      size_t ind = volumes->IndexOfComparable(SA.GetAtomInfo().GetSymbol());
      if( ind != InvalidIndex )
        R1 = volumes->GetObject(ind);
    }
    if( R1 == 0 )  R1 = SA.GetAtomInfo().GetRad2();
    Vt += M_PI*(R1*R1*R1)*4.0/3;
    for( size_t j=0; j < SA.BondCount(); j++ )  {
      TSBond& SB = SA.Bond(j);
      if( SB.GetTag() != 0 )  continue;
      TSAtom& OA = SB.Another(SA);
      SB.SetTag(1);
      if( OA.IsDeleted() )  continue;
      if( OA.GetAtomInfo() == iQPeakIndex )  continue;
      d = SB.Length();
      R1 = R2 = 0;
      if( volumes != NULL )  {
        size_t ind = volumes->IndexOfComparable(SA.GetAtomInfo().GetSymbol());
        if( ind != InvalidIndex )
          R1 = volumes->GetObject(ind);
        ind = volumes->IndexOfComparable(OA.GetAtomInfo().GetSymbol());
        if( ind != InvalidIndex )
          R2 = volumes->GetObject(ind);
      }
      if( R1 == 0 )  R1 = SA.GetAtomInfo().GetRad2();
      if( R2 == 0 )  R2 = OA.GetAtomInfo().GetRad2();
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
void TGXApp::StoreGroups()  {
  ClearGroups();
  for( size_t i=0; i <= FGlRender->GroupCount(); i++ )  {
    TGlGroup& glG = (i < FGlRender->GroupCount() ? FGlRender->GetGroup(i) : FGlRender->GetSelection());
    GroupData& gd = FOldGroups.AddNew();
    gd.collectionName = glG.GetCollectionName();  //planes
    gd.visible = glG.IsVisible();
    gd.material = glG.GetGlM();
    for( size_t j=0; j < glG.Count(); j++ )  {
      AGDrawObject& glO = glG[j];
      if( EsdlInstanceOf(glO, TXAtom) )
        gd.atoms.Add( ((TXAtom&)glO).Atom() );
      if( EsdlInstanceOf(glO, TXBond) )
        gd.bonds.Add( ((TXBond&)glO).Bond() );
      if( EsdlInstanceOf(glO, TXPlane) )
        gd.planes.Add( ((TXPlane&)glO).Plane() );
    }
  }
}
//..............................................................................
void TGXApp::RestoreGroups()  {
  TXAtomPList xatoms;
  TXBondPList xbonds;
  TXPlanePList xplanes;
  olxstr className;
  for( size_t i=0; i < FOldGroups.Count()-1; i++ )  {
    GroupData& gd = FOldGroups[i];
    xatoms.Clear();   SAtoms2XAtoms(gd.atoms, xatoms);
    xbonds.Clear();   SBonds2XBonds(gd.bonds, xbonds);
    xplanes.Clear();  SPlanes2XPlanes(gd.planes, xplanes);
    FGlRender->GetSelection().Clear();
    for( size_t j=0; j < xatoms.Count(); j++ )
      FGlRender->GetSelection().Add(*xatoms[j]);
    for( size_t j=0; j < xbonds.Count(); j++ )
      FGlRender->GetSelection().Add(*xbonds[j]);
    for( size_t j=0; j < xplanes.Count(); j++ )
      FGlRender->GetSelection().Add(*xplanes[j]);
    TGlGroup* glG = FGlRender->GroupSelection(gd.collectionName);
    if( glG == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "could not recreate groups");
    glG->SetSelected(false);
    FGlRender->GetSelection().Clear();
    glG->SetGlM(gd.material);
    glG->SetVisible(gd.visible);
  }
  GroupData& gd = FOldGroups.Last(); // the selection
  xatoms.Clear();   SAtoms2XAtoms(gd.atoms, xatoms);
  xbonds.Clear();   SBonds2XBonds(gd.bonds, xbonds);
  xplanes.Clear();  SPlanes2XPlanes(gd.planes, xplanes);
  FGlRender->GetSelection().Clear();
  for( size_t j=0; j < xatoms.Count(); j++ )
    FGlRender->Select(*xatoms[j]);
  for( size_t j=0; j < xbonds.Count(); j++ )
    FGlRender->Select(*xbonds[j]);
  for( size_t j=0; j < xplanes.Count(); j++ )
    FGlRender->Select(*xplanes[j]);

}
//..............................................................................
void TGXApp::StoreVisibility()  {
  FVisibility.SetSize((uint32_t)(XAtoms.Count() + XBonds.Count() + XPlanes.Count()));
  // atoms
  for( size_t i=0; i < XAtoms.Count(); i++ )
    if( XAtoms[i].IsVisible() )
      FVisibility.SetTrue(i);
  // bonds
  for( size_t i=0; i < XBonds.Count(); i++ )
    if( XBonds[i].IsVisible() )
      FVisibility.SetTrue(XAtoms.Count() + i);
  // planes
  for( size_t i=0; i < XPlanes.Count(); i++ )
    if( XPlanes[i].IsVisible() )
      FVisibility.SetTrue(XAtoms.Count() + XBonds.Count() + i);
}
void TGXApp::RestoreVisibility()  {
  //atoms
  for( size_t i=0; i < XAtoms.Count(); i++ )
    XAtoms[i].SetVisible( FVisibility.Get(i) );
  //bonds
  for( size_t i=0; i < XBonds.Count(); i++ )
    XBonds[i].SetVisible(FVisibility.Get(XAtoms.Count() + i));
  // planes
  for( size_t i=0; i < XPlanes.Count(); i++ )
      XPlanes[i].SetVisible( FVisibility.Get(XAtoms.Count() + XBonds.Count() + i) );
}
//..............................................................................
void TGXApp::BeginDrawBitmap(double resolution)  {
  FPictureResolution = resolution;
  FLabels->Clear();
  GetRender().GetScene().ScaleFonts(resolution);
  StoreGroups();
  StoreVisibility();
  CreateObjects(false, false);
  FXGrid->GlContextChange();
  RestoreGroups();
  RestoreVisibility();
}
//..............................................................................
void TGXApp::FinishDrawBitmap()  {
  FLabels->Clear();
  GetRender().GetScene().RestoreFontScale();
  CreateObjects(false, false);
  FXGrid->GlContextChange();
  RestoreGroups();
  ClearGroups();
  RestoreVisibility();
  FVisibility.Clear();
}
//..............................................................................
void TGXApp::UpdateLabels()  {
  for( size_t i=0; i < XLabels.Count(); i++ )
    XLabels[i].SetLabel(XLabels[i].GetLabel()); 
}
//..............................................................................
TXGlLabel* TGXApp::CreateLabel(TXAtom *A, uint16_t FontIndex)  {
  TXGlLabel& L = XLabels.Add(new TXGlLabel(*FGlRender, "PLabels"));
  L.SetFontIndex(FontIndex);
  L.SetLabel(A->Atom().GetLabel());
  L.SetCenter(A->Atom().crd());
  L.Basis.Translate(vec3d(1, -1, 0));  // in pixels
  L.Create();
  return &L;
}
//..............................................................................
uint64_t TGXApp::Draw()  {
  uint64_t st = TETime::msNow();
  GetRender().Draw();
  return TETime::msNow() - st;
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
  if( !v )  {
    for( size_t i=0; i < XBonds.Count(); i++ )  {
      if( XBonds[i].Bond().GetType() == sotHBond )
        XBonds[i].SetVisible(false);
    }
  }
  else  {
    const size_t ac = XAtoms.Count();
    for( size_t i=0; i < ac; i++ )  
      XAtoms[i].Atom().SetTag(i);
    for( size_t i=0; i < XBonds.Count(); i++ )  {
      const TSBond& b = XBonds[i].Bond(); 
      if( b.GetType() == sotHBond )  {
        XBonds[i].SetVisible( XAtoms[b.A().GetTag()].IsVisible() && XAtoms[b.B().GetTag()].IsVisible() );
      }
    }
  }
}
//..............................................................................
void TGXApp::HydrogensVisible(bool v)  {
  if( FHydrogensVisible != v )  {
    FHydrogensVisible = v;
    GetRender().ClearSelection();
    XFile().GetAsymmUnit().DetachAtomType(iHydrogenIndex, !FHydrogensVisible);
    for( size_t i = 0; i < OverlayedXFiles.Count(); i++ )  {
      OverlayedXFiles[i].GetAsymmUnit().DetachAtomType(iHydrogenIndex, !FHydrogensVisible);
      OverlayedXFiles[i].GetLattice().UpdateConnectivity();
    }
    XFile().GetLattice().UpdateConnectivity();
    CenterView(true);
  }
}
//..............................................................................
void TGXApp::QPeaksVisible(bool v)  {
  if( FQPeaksVisible != v )  {
    FQPeaksVisible = v;
    GetRender().ClearSelection();
    XFile().GetAsymmUnit().DetachAtomType(iQPeakIndex, !FQPeaksVisible);
    for( size_t i = 0; i < OverlayedXFiles.Count(); i++ )  {
      OverlayedXFiles[i].GetAsymmUnit().DetachAtomType(iQPeakIndex, !FQPeaksVisible);
      OverlayedXFiles[i].GetLattice().UpdateConnectivity();
    }
    XFile().GetLattice().UpdateConnectivity();
    CenterView(true);
  }
}
//..............................................................................
void TGXApp::SyncQVisibility()  {
  if( !FQPeakBondsVisible )  return;
 
  const size_t ac = XAtoms.Count();
  for( size_t i=0; i < ac; i++ )  
    XAtoms[i].Atom().SetTag(i);

  const size_t bc = XBonds.Count();
  for( size_t i=0; i < bc; i++ )  {
    const TSBond& b = XBonds[i].Bond();
    if( b.A().GetAtomInfo().GetIndex() == iQPeakIndex || b.B().GetAtomInfo().GetIndex() == iQPeakIndex )
      XBonds[i].SetVisible(XAtoms[b.A().GetTag()].IsVisible() && XAtoms[b.B().GetTag()].IsVisible());
  }
  if( FXGrowLinesVisible )  {
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      if( XGrowLines[i].SAtom()->GetAtomInfo() == iQPeakIndex )
        XGrowLines[i].SetVisible( XAtoms[XGrowLines[i].SAtom()->GetTag()].IsVisible() );
    }
  }
}
//..............................................................................
void TGXApp::QPeakBondsVisible(bool v)  {
  FQPeakBondsVisible = v;
  if( !v )  {
    for( size_t i=0; i < XBonds.Count(); i++ )  {
      if( (XBonds[i].Bond().A().GetAtomInfo() == iQPeakIndex) ||
        (XBonds[i].Bond().B().GetAtomInfo() == iQPeakIndex)  )
        XBonds[i].SetVisible(v);
    }
    if( FXGrowLinesVisible )  {
      for( size_t i=0; i < XGrowLines.Count(); i++ )  {
        if( XGrowLines[i].SAtom()->GetAtomInfo() == iQPeakIndex ||
          XGrowLines[i].CAtom()->GetAtomInfo() == iQPeakIndex )
          XGrowLines[i].SetVisible(v);
      }
    }
  }
  else  {
    const size_t ac = XAtoms.Count();
    for( size_t i=0; i < ac; i++ )  
      XAtoms[i].Atom().SetTag(i);

    for( size_t i=0; i < XBonds.Count(); i++ )  {
      const TSBond& b = XBonds[i].Bond();
      if( b.A().GetAtomInfo() == iQPeakIndex || b.B().GetAtomInfo() == iQPeakIndex )
        XBonds[i].SetVisible(XAtoms[b.A().GetTag()].IsVisible() && XAtoms[b.B().GetTag()].IsVisible());
    }
  }
}
//..............................................................................
void TGXApp::StructureVisible(bool v)  {
  FStructureVisible = v;
  for( size_t i=0; i < XAtoms.Count(); i++ )        XAtoms[i].SetVisible(v);
  for( size_t i=0; i < XBonds.Count(); i++ )        XBonds[i].SetVisible(v);
  for( size_t i=0; i < LooseObjects.Count(); i++ )  LooseObjects[i]->SetVisible(v);
  for( size_t i=0; i < XPlanes.Count(); i++ )       XPlanes[i].SetVisible(v);
  for( size_t i=0; i < XLabels.Count(); i++ )       XLabels[i].SetVisible(v);
  if( v )  {
    if( !FXGrid->IsEmpty() )
      FXGrid->SetVisible(true);
  } 
  else
    FXGrid->SetVisible(false);
}
//..............................................................................
void TGXApp::LoadXFile(const olxstr& fn)  {
  FXFile->LoadFromFile(fn);
  if( !FStructureVisible )  
    GetLog() << "Note: structure is invisible\n";
  else {
    if( !FQPeaksVisible ) 
      GetLog() << "Note: Q-peaks are invisible\n";
    if( !FHydrogensVisible )
      GetLog() << "Note: H-atoms are invisible\n";
  }
  Draw();  // fixes native loader is not draw after load
}
//..............................................................................
void TGXApp::ShowPart(const TIntList& parts, bool show)  {
  if( parts.IsEmpty() )  {
    for( size_t i=0; i < XAtoms.Count(); i++ )  {
      if( XAtoms[i].IsDeleted() )  continue;
      XAtoms[i].SetVisible(true);
    }
    for( size_t i=0; i < XBonds.Count(); i++ )  {
      if( XBonds[i].IsDeleted() )  continue;
      XBonds[i].SetVisible(true);
    }
    return;
  }
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].IsDeleted() ) continue;
    if( parts.IndexOf( XAtoms[i].Atom().CAtom().GetPart() ) != InvalidIndex )
      XAtoms[i].SetVisible(show);
    else
      XAtoms[i].SetVisible(!show);
  }
  for( size_t i=0; i < XBonds.Count(); i++ )  {
    if( XBonds[i].IsDeleted() )  continue;
    if( parts.IndexOf(XBonds[i].Bond().A().CAtom().GetPart()) != InvalidIndex &&
        parts.IndexOf(XBonds[i].Bond().B().CAtom().GetPart()) != InvalidIndex )
    {
      XBonds[i].SetVisible(show);
    }
    else
      XBonds[i].SetVisible(!show);
  }
}
//..............................................................................
void TGXApp::HklVisible(bool v)  {
  if( v )  {
    // default if could not load the hkl ...
    FDUnitCell->SetReciprocal(false);
    FHklVisible = false;
    if( !FHklFile->RefCount() )  {
      if( !FXFile->HasLastLoader() )
      {  Log->Error("Cannot display HKL - file is not loaded");  return;  }
      if( !TEFile::Exists(FXFile->LastLoader()->GetRM().GetHKLSource()) )
      {  Log->Error("Cannot display HKL - could locate HKL file");  return;  }
      if( !FHklFile->LoadFromFile(FXFile->LastLoader()->GetRM().GetHKLSource()) )
      {  Log->Error("Cannot display HKL - could load HKL file");  return;  }
    }
    CreateXRefs();
  }
  for( size_t i=0; i < XReflections.Count(); i++ )  
    XReflections[i].SetVisible(v);
  FHklVisible = v;
  FDUnitCell->SetReciprocal(v);
}
//..............................................................................
void TGXApp::SetGridDepth(const vec3d& crd)  {
  FXGrid->SetDepth( crd );
}
//..............................................................................
bool TGXApp::GridVisible()  const {  
  return FXGrid->IsVisible();  
}
//..............................................................................
bool TGXApp::ShowGrid(bool v, const olxstr& FN)  {
  if( v )  {
    if(  FXGrid->IsEmpty() && FN.IsEmpty() )  {
      Log->Error("Cannot display empty grid");
      return false;
    }
    FXGrid->SetVisible(true);
  }
  else
    FXGrid->SetVisible(false);
  return v;
}
//..............................................................................
void TGXApp::Individualise(TXAtom& XA)  {
  if( XA.GetPrimitives().ObjectCount() == 1 )  return;
  short level = XA.LegendLevel( XA.GetPrimitives().GetName() ), 
    required_level = FXFile->GetLattice().IsGenerated() ? 2 : 1;
  
  if( level >= required_level )  return;
  else  
    level = required_level;

  olxstr leg = XA.GetLegend( XA.Atom(), level );
  TGPCollection* indCol = FGlRender->FindCollection( leg );
  if( indCol != NULL && &XA.GetPrimitives() == indCol )  
    return;
  else  {
    if( indCol == NULL )  {
      indCol = &FGlRender->NewCollection( leg );
      IndividualCollections.Add(leg);
    }
    XA.GetPrimitives().RemoveObject(XA);
    XA.Create( leg );
    TSAtomPList satoms;
    TSBondPList sbonds;
    TXAtomPList xatoms;
    TXBondPList xbonds;
    short level1;
    for( size_t i=0; i < XA.Atom().BondCount(); i++ )  {
      TSBond& SB = XA.Atom().Bond(i);
      sbonds.Add( &SB );
      satoms.Add( &SB.Another(XA.Atom()) );
    }
    SAtoms2XAtoms(satoms, xatoms);
    SBonds2XBonds(sbonds, xbonds);
    for( size_t i=0; i < xbonds.Count(); i++ )  {
      level1 = TXAtom::LegendLevel( xatoms[i]->GetPrimitives().GetName() );
      leg = xbonds[i]->GetLegend( xbonds[i]->Bond(), level);
      indCol = FGlRender->FindCollection( leg );
      if( indCol != NULL && &xbonds[i]->GetPrimitives() == indCol )  
        continue;
      else  {
        if( indCol == NULL )  {
          indCol = &FGlRender->NewCollection( leg );
          IndividualCollections.Add(leg);
        }
        xbonds[i]->GetPrimitives().RemoveObject( *xbonds[i] );
        xbonds[i]->Create(leg);
      }
    }
  }
}
//..............................................................................
void TGXApp::Individualise(TXBond& XB)  {
  if( XB.GetPrimitives().ObjectCount() == 1 )  return;
  short required_level = FXFile->GetLattice().IsGenerated() ? 2 : 1;
  for( size_t i=0; i < XAtoms.Count(); i++)
    XAtoms[i].Atom().SetTag(i);
  olxstr leg = XB.GetLegend(XB.Bond(), required_level);
  TGPCollection* indCol = FGlRender->FindCollection( leg );
  if( indCol != NULL && &XB.GetPrimitives() == indCol )  
    return;
  if( indCol == NULL )  {
    indCol = &FGlRender->NewCollection( leg );
    IndividualCollections.Add(leg);
  }
  XB.GetPrimitives().RemoveObject(XB);
  BondCreationParams bcpar(XAtoms[XB.Bond().A().GetTag()], XAtoms[XB.Bond().B().GetTag()]); 
  XB.Create( leg, &bcpar );
}
//..............................................................................
void TGXApp::Collectivise(TXAtom& XA)  {
  short level = XA.LegendLevel( XA.GetPrimitives().GetName() );
  if( !level )  return;
  else  level--;

  olxstr leg = XA.GetLegend( XA.Atom(), level );
  TGPCollection* indCol = FGlRender->FindCollection( leg );
  if( indCol != NULL && &XA.GetPrimitives() == indCol )  
    return;
  else  {
    if( indCol == NULL )  
      indCol = &FGlRender->NewCollection( leg );

    XA.GetPrimitives().RemoveObject(XA);
    if( XA.GetPrimitives().ObjectCount() == 0 )  {
      size_t index = IndividualCollections.IndexOf( XA.GetPrimitives().GetName() );
      if( index >= InvalidIndex )  
        IndividualCollections.Delete(index);
    }
    XA.Create(leg);
    TSAtomPList satoms;
    TSBondPList sbonds;
    TXAtomPList xatoms;
    TXBondPList xbonds;
    for( size_t i=0; i < XA.Atom().BondCount(); i++ )  {
      TSBond& SB = XA.Atom().Bond(i);
      sbonds.Add( &SB );
      satoms.Add( &SB.Another(XA.Atom()) );
    }
    SAtoms2XAtoms(satoms, xatoms);
    SBonds2XBonds(sbonds, xbonds);
    for( size_t i=0; i < xbonds.Count(); i++ )  {
      leg = xbonds[i]->GetLegend( xbonds[i]->Bond(), level);
      indCol = FGlRender->FindCollection( leg );
      if( indCol != NULL && &xbonds[i]->GetPrimitives() == indCol )  
        continue;
      else  {
        if( indCol == NULL )  
          indCol = &FGlRender->NewCollection( leg );
        xbonds[i]->GetPrimitives().RemoveObject(*xbonds[i]);
        if( xbonds[i]->GetPrimitives().ObjectCount() == 0 )  {
          size_t index = IndividualCollections.IndexOf( xbonds[i]->GetPrimitives().GetName() );
          if( index != InvalidIndex )  
            IndividualCollections.Delete(index);
        }
        xbonds[i]->Create(leg);
      }
    }
  }
}
//..............................................................................
size_t TGXApp::GetNextAvailableLabel(const olxstr& AtomType) {
  size_t nextLabel = 0, currentLabel;
  TBasicAtomInfo *bai = FAtomsInfo->FindAtomInfoBySymbol(AtomType);
  if( bai == NULL )  return nextLabel;
  olxstr label, nLabel;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetAtomInfo() == *bai )  {
      label = XAtoms[i].Atom().GetLabel().SubStringFrom( bai->GetSymbol().Length() );
      if( !label.Length() )  continue;
      nLabel = EmptyString;
      size_t j=0;
      while( (j < label.Length()) && label[j] <= '9' && label[j] >= '0' )  {
        nLabel << label[j];
        j++;
      }
      if( !nLabel.Length() )  continue;
      currentLabel = nLabel.ToUInt();
      if( currentLabel > nextLabel )  nextLabel = currentLabel;
    }
  }
  return nextLabel+1;
}
//..............................................................................
void TGXApp::SynchroniseBonds( TXAtomPList& xatoms )  {
  TSBondPList sbonds;
  TXBondPList xbonds;
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    for( size_t j=0; j < xatoms[i]->Atom().BondCount(); j++ )
      sbonds.Add( &xatoms[i]->Atom().Bond(j) );
  }

  // prepare unique list of bonds
  for( size_t i=0; i < sbonds.Count(); i++ )  sbonds[i]->SetTag(i);
  for( size_t i=0; i < sbonds.Count(); i++ )
    if( sbonds[i]->GetTag() != i )
      sbonds[i] = NULL;
  sbonds.Pack();
  // have to call setatom function to set thecorrect order for atom of bond
  for( size_t i=0; i < sbonds.Count(); i++ )
    sbonds[i]->SetA( sbonds[i]->A() );

  SBonds2XBonds(sbonds, xbonds);

  for(size_t i=0; i < XAtoms.Count(); i++ )  XAtoms[i].Atom().SetTag(i);

  for( size_t i=0; i < xbonds.Count(); i++ )  {
//    if( XB->GetPrimitives().ObjectCount() == 1 )  continue;
    // change the orientation if necessary
    xbonds[i]->BondUpdated();
    xbonds[i]->GetPrimitives().RemoveObject( *xbonds[i] );
    TXAtom& XA  = XAtoms[ xbonds[i]->Bond().A().GetTag() ];
    TXAtom& XA1 = XAtoms[ xbonds[i]->Bond().B().GetTag() ];
    xbonds[i]->Create( TXBond::GetLegend( xbonds[i]->Bond(), 
      olx_max(TXAtom::LegendLevel(XA.GetPrimitives().GetName()),
        TXAtom::LegendLevel(XA1.GetPrimitives().GetName()))) );
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
  for( size_t i=0; i < XGrowPoints.Count(); i++ )
    XGrowPoints[i].SetVisible( v );
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

  VTo[0] = olx_round(MTo[0]+1);     VTo[1] = olx_round(MTo[1]+1);     VTo[2] = olx_round(MTo[2]+1);
  VFrom[0] = olx_round(MFrom[0]-1); VFrom[1] = olx_round(MFrom[1]-1); VFrom[2] = olx_round(MFrom[2]-1);

  XFile().GetLattice().GenerateMatrices(matrices, VFrom, VTo, MFrom, MTo);

  VFrom = XFile().GetAsymmUnit().GetOCenter(false, false);
  for( size_t i=0; i < matrices.Count(); i++ )  {
    if( UsedTransforms.IndexOf( *matrices[i] ) != InvalidIndex )  {
      delete matrices[i];
      continue;
    }
    VTo = VFrom * matrices[i]->r;
    VTo += matrices[i]->t;
    XFile().GetAsymmUnit().CellToCartesian( VTo );
    TXGrowPoint& gp = XGrowPoints.Add( new TXGrowPoint(*FGlRender, EmptyString, VTo, *matrices[i]) );
    gp.Create("GrowPoint");
    delete matrices[i];
  }
}
//..............................................................................
void TGXApp::SetXGrowLinesVisible(bool v)  {
  if( v )  {
    CreateXGrowLines();
    FXGrowLinesVisible = v;
    return;
  }
  for( size_t i=0; i < XGrowLines.Count(); i++ )
    XGrowLines[i].SetVisible( v );
  FXGrowLinesVisible = v;
}
//..............................................................................
void TGXApp::SetGrowMode(short v, const olxstr& atoms)  {
  TXAtomPList xatoms;
  if( atoms.IsEmpty() )
    FindXAtoms("sel", xatoms);
  else
    FindXAtoms(atoms, xatoms);
  // have to preprocess instructions like 'sel'
  olxstr ats;
  for( size_t i=0; i < xatoms.Count(); i++ )
    ats << xatoms[i]->Atom().GetLabel() << ' ';
  AtomsToGrow = ats;
  FGrowMode = v;
  UsedTransforms.Clear();
}
//..............................................................................
struct TGXApp_Transform {
  TCAtom* to;
  TSAtom* from;
  double dist;
  smatd transform;
  TGXApp_Transform() : to(NULL), from(NULL), dist(0) { }
};
struct TGXApp_Transform1 : public TGXApp_Transform {
  vec3d dest;
};
void TGXApp::CreateXGrowLines()  {
  if( !XGrowLines.IsEmpty() )  {  // clear the existing ones...
    TPtrList<TGPCollection> colls; // list of unique collections
    TPtrList<AGDrawObject> lines;  // list of the AGDrawObject pointers to lines...
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      XGrowLines[i].GetPrimitives().SetTag(i);
      lines.Add(&XGrowLines[i]);
    }
    for( size_t i=0; i < XGrowLines.Count(); i++ )
      if( XGrowLines[i].GetPrimitives().GetTag() == i )
        colls.Add( &XGrowLines[i].GetPrimitives() );
    FGlRender->RemoveCollections( colls );  // remove collections with their primitives
    FGlRender->RemoveObjects( lines );  // remove the object references
    XGrowLines.Clear(); // and delete the objects
  }
  if( FGrowMode & gmVanDerWaals )  {
    _CreateXGrowVLines();
    return;
  }
  const TAsymmUnit& au = FXFile->GetAsymmUnit();
  const TUnitCell& uc = FXFile->GetUnitCell();
  TSAtomPList AtomsToProcess;
  if( !AtomsToGrow.IsEmpty() )  {
    TXAtomPList xatoms;
    FindXAtoms(AtomsToGrow, xatoms);
    TListCaster::POP(xatoms, AtomsToProcess);
  }
  else if( (FGrowMode & gmSameAtoms) == 0 ) {
    const size_t ac = FXFile->GetLattice().AtomCount();
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& A = FXFile->GetLattice().GetAtom(i);
      if( A.IsDeleted() )  continue;
      AtomsToProcess.Add(A);
    }
  }
  TPtrList<TCAtom> AttachedAtoms;
  TTypeList<TGXApp_Transform1> tr_list;
  for( size_t i=0; i < AtomsToProcess.Count(); i++ )  {
    TSAtom* A = AtomsToProcess[i];
    if( A->IsDeleted() )  continue;
    AttachedAtoms.Clear();
    if( (FGrowMode & gmCovalent) != 0 )  {
      for( size_t j=0; j < A->CAtom().AttachedAtomCount(); j++ )
        if( !A->CAtom().GetAttachedAtom(j).IsDeleted() )
          AttachedAtoms.Add( &A->CAtom().GetAttachedAtom(j) );
    }
    if( (FGrowMode & gmSInteractions) != 0 )  {
      for( size_t j=0; j < A->CAtom().AttachedAtomICount(); j++ )
        if( !A->CAtom().GetAttachedAtomI(j).IsDeleted() )
          AttachedAtoms.Add( &A->CAtom().GetAttachedAtomI(j) );
    }
    if( (FGrowMode & gmSameAtoms) != 0 )
      AttachedAtoms.Add( &A->CAtom() );

    if( AttachedAtoms.IsEmpty() )  continue;

    for( size_t j=0; j < AttachedAtoms.Count(); j++ )  {
      TCAtom *aa = AttachedAtoms[j];
      const vec3d& cc = aa->ccrd();
      smatd_list *transforms;
      if( FGrowMode & gmSameAtoms )  {
//        transforms = FXFile->GetLattice().GetUnitCell()->Getclosest(A->ccrd(), cc, false );
        transforms = uc.GetInRangeEx(A->ccrd(), cc,
                       A->GetAtomInfo().GetRad1() + aa->GetAtomInfo().GetRad1() + 15,
                       false, UsedTransforms );
      }
      else if( FGrowMode & gmSInteractions )  {
        transforms = uc.GetInRange(A->ccrd(), cc,
                       A->GetAtomInfo().GetRad1() + aa->GetAtomInfo().GetRad1() + FXFile->GetLattice().GetDeltaI(),
                       false );
      }
      else  {
        transforms = uc.GetInRange(A->ccrd(), cc,
                       A->GetAtomInfo().GetRad1() + aa->GetAtomInfo().GetRad1() + FXFile->GetLattice().GetDelta(),
                       false );
      }
      if( transforms->IsEmpty() )  {  delete transforms;  continue;  }
      for( size_t k=0; k < transforms->Count(); k++ )  {
        smatd& transform = transforms->Item(k);
        vec3d tc = transform*cc;
        au.CellToCartesian(tc);
        const double qdist = tc.QDistanceTo( A->crd() );
        bool uniq = true;
        for( size_t l=0; l < A->NodeCount(); l++ )  {  // check if point to one of already connected
          if( A->Node(l).crd().QDistanceTo(tc) < 0.001 )  {
            uniq = false;
            break;
          }
        }
        if( !uniq )  
          continue;
        for( size_t l=0; l < tr_list.Count(); l++ )  {
          if( tr_list[l].dest.QDistanceTo(tc) < 0.001 )  {
            if( tr_list[l].dist > qdist )  {
              tr_list[l].transform = transform;
              tr_list[l].dist = qdist;
              tr_list[l].to = aa;
              tr_list[l].from = A;
            }
            uniq = false;
            break;
          }
        }
        if( uniq )  {
          TGXApp_Transform1& nt = tr_list.AddNew();
          nt.transform = transform;
          nt.dist = qdist;
          nt.to = aa;
          nt.from = A;
        }
      }
      delete transforms;
    }
  }
  for( size_t i=0; i < tr_list.Count(); i++ )  {
    TGXApp_Transform1& nt = tr_list[i];
    TXGrowLine& gl = XGrowLines.Add( new TXGrowLine(*FGlRender, EmptyString, nt.from, nt.to, nt.transform) );

    if( !QPeakBondsVisible() &&
      (nt.from->GetAtomInfo() == iQPeakIndex || nt.to->GetAtomInfo() == iQPeakIndex ) )
      gl.SetVisible(false);
    if( !HBondsVisible() && 
      (nt.from->GetAtomInfo() == iHydrogenIndex || nt.to->GetAtomInfo() == iHydrogenIndex) )
      gl.SetVisible(false);
      gl.Create("GrowBonds");
  }
}
//..............................................................................
struct TGXApp_CrdMap  {
  typedef SortedObjectList<int, TPrimitiveComparator> ZDict;
  typedef olxdict<int, ZDict, TPrimitiveComparator> YDict;
  olxdict<int, YDict, TPrimitiveComparator> data;
  const int resolution;
  TGXApp_CrdMap() : resolution(5) {}
  void Add(const vec3d& pt)  {
    YDict& yd = data.Add( olx_round(pt[0]*resolution) );
    ZDict& zd = yd.Add( olx_round(pt[1]*resolution) );
    zd.AddUnique( olx_round(pt[2]*resolution) );
  }
  bool Exists(const vec3d& pt) const  {
    const size_t y_ind = data.IndexOf( olx_round(pt[0]*resolution) );
    if( y_ind == InvalidIndex )  return false;
    const YDict& yd = data.GetValue(y_ind);
    const size_t z_ind = yd.IndexOf( olx_round(pt[1]*resolution) );
    if( z_ind == InvalidIndex )  return false;
    const ZDict& zd = yd.GetValue( z_ind );
    return zd.IndexOf( olx_round(pt[2]*resolution) ) == InvalidIndex ? false : true;
  }
};
void TGXApp::_CreateXGrowVLines()  {
  if( !XGrowLines.IsEmpty() )  return;
  const TAsymmUnit& au = FXFile->GetAsymmUnit();
  const TUnitCell& uc = FXFile->GetUnitCell();
  TGXApp_CrdMap CrdMap;
  TSAtomPList AtomsToProcess;
  if( !AtomsToGrow.IsEmpty() )  {
    TXAtomPList xatoms;
    FindXAtoms(AtomsToGrow, xatoms);
    TListCaster::POP( xatoms, AtomsToProcess );
    const size_t ac = FXFile->GetLattice().AtomCount();
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& A = FXFile->GetLattice().GetAtom(i);
      if( A.IsDeleted() )  continue;
      CrdMap.Add( A.crd() );
    }
  }
  else  {
    const size_t ac = FXFile->GetLattice().AtomCount();
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& A = FXFile->GetLattice().GetAtom(i);
      if( A.IsDeleted() )  continue;
      AtomsToProcess.Add( &A );
      CrdMap.Add( A.crd() );
    }
  }
  typedef TTypeList<TGXApp_Transform> tr_list;
  olxdict<int, tr_list, TPrimitiveComparator> net_tr;
  for( size_t i=0; i < AtomsToProcess.Count(); i++ )  {
    TSAtom* A = AtomsToProcess[i];
    if( A->IsDeleted() )  continue;
    TArrayList< AnAssociation2<TCAtom const*,smatd> > envi;
    uc.FindInRangeAM(A->ccrd(), DeltaV+ A->GetAtomInfo().GetRad1(), envi);
    for( size_t j=0; j < envi.Count(); j++ )  {
      TCAtom *aa = const_cast<TCAtom*>(envi[j].GetA());
      const vec3d& cc = aa->ccrd();
      const smatd& transform = envi[j].GetB();
      vec3d tc = transform*cc;
      au.CellToCartesian(tc);
      const double qdist = tc.QDistanceTo( A->crd() );
      if( qdist < 0.001 )  // skip atoms on special postions
        continue;
      bool uniq = true;
      if( CrdMap.Exists(tc) )  // check if point to one of already connected
        continue;
      tr_list& ntl = net_tr.Add(aa->GetFragmentId());
      for( size_t l=0; l < ntl.Count(); l++ )  {
        if( ntl[l].transform == transform )  {
          if( ntl[l].dist > qdist )  {
            ntl[l].transform = transform;
            ntl[l].dist = qdist;
            ntl[l].to = aa;
            ntl[l].from = A;
          }
          uniq = false;
          break;
        }
      }
      if( uniq )  {
        TGXApp_Transform& nt = ntl.AddNew();
        nt.transform = transform;
        nt.dist = qdist;
        nt.to = aa;
        nt.from = A;
        CrdMap.Add(tc);
      }
    }
  }
  for( size_t i=0; i < net_tr.Count(); i++ )  {
    const tr_list& ntl = net_tr.GetValue(i);
    for( size_t j=0; j < ntl.Count(); j++ )  {
      TGXApp_Transform& nt = ntl[j];
      TXGrowLine& gl = XGrowLines.Add( new TXGrowLine(*FGlRender, EmptyString, nt.from, nt.to, nt.transform) );

      if( !QPeakBondsVisible() &&
        (nt.from->GetAtomInfo() == iQPeakIndex || nt.to->GetAtomInfo() == iQPeakIndex ) )
        gl.SetVisible(false);
      if( !HBondsVisible() && 
        (nt.from->GetAtomInfo() == iHydrogenIndex || nt.to->GetAtomInfo() == iHydrogenIndex) )
        gl.SetVisible(false);
        gl.Create("GrowBonds");
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
  for( size_t i=0; i < GlBitmaps.Count(); i++ )
    if( GlBitmaps[i]->GetCollectionName() == name )  return GlBitmaps[i];
  return NULL;
}
//..............................................................................
TGlBitmap* TGXApp::CreateGlBitmap(const olxstr& name,
  int left, int top, int width, int height,
  unsigned char* RGBa, unsigned int format)  {

  TGlBitmap* glB = FindGlBitmap(name);
  if( glB == NULL )  {
    glB = new TGlBitmap(*FGlRender, name, left, top, width, height, RGBa, format );
    GlBitmaps.Add(glB);
    glB->Create();
    ObjectsToCreate.Add( (AGDrawObject*)glB );
    glB->SetZ(-10.0 + (double)GlBitmaps.Count()/100 );
  }
  else  {
    glB->ReplaceData( width, height, RGBa, format );
    glB->SetVisible(!glB->IsVisible());
  }
  return glB;
}
//..............................................................................
void TGXApp::DeleteGlBitmap(const olxstr& name)  {
  TGlBitmap* glb = NULL;
  for( size_t i=0; i < GlBitmaps.Count(); i++ )  {
    if( GlBitmaps[i]->GetCollectionName() == name )  {
      glb = GlBitmaps[i];
      GlBitmaps.Delete(i);
      break;
    }
  }
  if( glb != NULL )  {
    //GlBitmaps.Delete(index);
    size_t ind = ObjectsToCreate.IndexOf( (AGDrawObject*)glb );
    if( ind != InvalidIndex )
      ObjectsToCreate.Delete(ind);
    glb->GetPrimitives().RemoveObject( *glb );
    FGlRender->RemoveObject( *glb );
    delete glb;
  }
}
//..............................................................................
TXFile& TGXApp::NewOverlayedXFile() {
  TXFile& f = OverlayedXFiles.Add( (TXFile*)FXFile->Replicate() );
  return f;
}
//..............................................................................
void TGXApp::CalcLatticeRandCenter(const TLattice& latt, double& maxR, vec3d& cnt)  {
  maxR = 0;
  cnt.Null();
  for( size_t i=0; i < latt.AtomCount(); i++ )
    cnt += latt.GetAsymmUnit().CellToCartesian(latt.GetAtom(i).ccrd(), latt.GetAtom(i).crd());

  if( latt.AtomCount() != 0 )
    cnt /= latt.AtomCount();
  for( size_t i=0; i < latt.AtomCount(); i++ )  {
    const double r = cnt.QDistanceTo( latt.GetAtom(i).crd() );
    if( r > maxR )
      maxR = r;
  }
  maxR = sqrt(maxR);
}
//..............................................................................
void TGXApp::AlignOverlayedXFiles() {
  typedef AnAssociation3<double,vec3d,TLattice*> grid_type;
  typedef TTypeList<grid_type> row_type;
  TTypeList<row_type> grid;
  size_t dim = olx_round( sqrt((double)OverlayedXFiles.Count()+1) );
  if( (OverlayedXFiles.Count()+1) - dim*dim > 0 )
    dim++;
  vec3d cnt;
  double maxR;
  for( size_t i=0; i < dim; i++ )  {
    row_type& row = grid.AddNew();
    for( size_t j=0; j < dim; j++ )  {
      const size_t ind = i*dim+j;
      if( ind == 0 )  {
        CalcLatticeRandCenter(XFile().GetLattice(), maxR, cnt);
        row.Add( new grid_type(maxR, cnt, &XFile().GetLattice()) );
      }
      else if( ind-1 >= OverlayedXFiles.Count() )
        break;
      else  {
        CalcLatticeRandCenter(OverlayedXFiles[ind-1].GetLattice(), maxR, cnt);
        row.Add( new grid_type(maxR, cnt, &OverlayedXFiles[ind-1].GetLattice()) );
      }
    }
  }
  TDoubleList row_height(dim), col_width(dim);
  for( size_t i=0; i < dim; i++ )
    row_height[i] = col_width[0] = 0;
  // calc widths and heights
  for( size_t i=0; i < dim; i++ )  {
    for( size_t j=0; j < dim; j++ )  {
      if( j+1 > grid[i].Count() )  break;
      if( grid[i][j].GetA() > row_height[i] )
        row_height[i] = grid[i][j].GetA();
      if( grid[i][j].GetA() > col_width[j] )
        col_width[j] = grid[i][j].GetA();
    }
  }
  // propagate widths and heights
  // wee need a sequence ( r1, r1+r2, (r1+r2)+(r2+r3), (r1+r2)+(r2+r3)+(r3+r4)...)
  for( size_t i=dim-1; i >= 1; i-- )  {
    row_height[i] += row_height[i-1];
    col_width[i] += col_width[i-1];
  }
  for( size_t i=1; i < dim-1; i++ )  {
    row_height[i+1] += row_height[i];
    col_width[i+1] += col_width[i];
  }
  col_width[0] = row_height[0] = 0;

  const vec3d right_shift = FGlRender->GetBasis().GetMatrix()*vec3d(1, 0, 0);
  const vec3d up_shift = FGlRender->GetBasis().GetMatrix()*vec3d(0, 1, 0);
  for( size_t i=0; i < dim; i++ )  {
    for( size_t j=0; j < dim; j++ )  {
      if( (i|j) == 0 )  continue;
      if( j+1 > grid[i].Count() )  break;
      vec3d shift_vec = (grid[0][0].GetB()-grid[i][j].GetB());
      shift_vec += up_shift*row_height[i];
      shift_vec += right_shift*col_width[j];
      TLattice& latt = *grid[i][j].GetC();
      for( size_t k=0; k < latt.AtomCount(); k++ )
        latt.GetAtom(k).crd() += shift_vec;
    }
  }
}
//..............................................................................
void TGXApp::DeleteOverlayedXFile(size_t index) {
  ClearLabels();
  ClearSelectionCopy();
  OverlayedXFiles.Delete(index);
  CreateObjects(true);
  CenterView();
  Draw();
}
//..............................................................................
void TGXApp::UpdateBonds()  {
  for( size_t i=0; i < XBonds.Count(); i++ )
    XBonds[i].BondUpdated();
}
//..............................................................................
TXLattice& TGXApp::AddLattice(const olxstr& Name, const mat3d& basis)  {
  TXLattice *XL = new TXLattice(*FGlRender, Name);
  XL->SetLatticeBasis(basis);
  XL->Create();
  LooseObjects.Add( XL );
  return *XL;
}
//..............................................................................
void TGXApp::InitFadeMode()  {
}
//..............................................................................
void TGXApp::BuildSceneMask(FractMask& mask, double inc)  {
  TAsymmUnit& au = XFile().GetAsymmUnit();
  vec3d mn(100, 100, 100), 
        mx(-100, -100, -100),
        norms(
          au.Axes()[0].GetV(), 
          au.Axes()[1].GetV(),
          au.Axes()[2].GetV()
        );
  TTypeList<AnAssociation2<vec3d,double> > atoms;
  atoms.SetCapacity( XAtoms.Count() );
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].IsDeleted() || !XAtoms[i].IsVisible() )  continue;
    if( XAtoms[i].Atom().GetAtomInfo() == iQPeakIndex )  continue;
    vec3d::UpdateMinMax(XAtoms[i].Atom().ccrd(), mn, mx);
    atoms.AddNew( XAtoms[i].Atom().crd(), olx_sqr(XAtoms[i].Atom().GetAtomInfo().GetRad2())+inc );
  }
  mn -= 1./4;
  mx += 1./4;
  float res = 0.5;
  mask.Init(mn, mx, norms, res);
  norms /= res;
  TArray3D<bool>* mdata = mask.GetMask();
  mdata->FastInitWith(0);
  const size_t 
    da = mdata->Length1(),
    db = mdata->Length2(),
    dc = mdata->Length3();
  const size_t ac = atoms.Count();
  for( size_t j = 0; j < da; j++ )  {
    const double dx = (double)j/norms[0];
    for( size_t k = 0; k < db; k++ )  {
      const double dy = (double)k/norms[1];
      for( size_t l = 0; l < dc; l++ )  {
        vec3d p(dx, dy, (double)l/norms[2]);
        p += mn;
        au.CellToCartesian(p);
        for( size_t i=0; i < ac; i++ )  {
          if( p.QDistanceTo( atoms[i].GetA() ) <= atoms[i].GetB() )  {  
            mdata->Data[j][k][l] = true;
            break;
          }
        }
      }
    }
  }
}
//..............................................................................
void TGXApp::ToDataItem(TDataItem& item, IOutputStream& zos) const  {
  FXFile->ToDataItem(item.AddItem("XFile"));
  TPtrList<TGraphicsStyle> styles;
  for( size_t i=0; i < FGlRender->ObjectCount(); i++ )  {
    TGraphicsStyle* gs = &FGlRender->GetObject(i).GetPrimitives().GetStyle();
    while( gs->GetParentStyle() != NULL )  {
      if( gs->GetParentStyle()->GetParentStyle() == NULL ) // avoid the root
        break;
      gs = gs->GetParentStyle();
    }
    if( gs->GetName() == "Q" )
      continue;
    if( styles.IndexOf(gs) == InvalidIndex )
      styles.Add( gs );
  }
  styles.Add(TXAtom::GetParamStyle());
  styles.Add(TXBond::GetParamStyle());
  FGlRender->GetStyles().ToDataItem(item.AddItem("Style"), styles);
  TDataItem& ind_col = item.AddItem("ICollections");
  for( size_t i=0; i < IndividualCollections.Count(); i++ )
    ind_col.AddField( olxstr("col_") << i, IndividualCollections[i]);
  FGlRender->GetBasis().ToDataItem(item.AddItem("Basis"));

  TDataItem& visibility = item.AddItem("Visibility");
  visibility.AddField("h_atoms", FHydrogensVisible);
  visibility.AddField("h_bonds", FHBondsVisible);
  visibility.AddField("q_atoms", FQPeaksVisible);
  visibility.AddField("q_bonds", FQPeakBondsVisible);
  visibility.AddField("basis", FDBasis->IsVisible());
  visibility.AddField("cell", FDUnitCell->IsVisible());
  // store objects visibility
  size_t a_cnt = 0;
  for( size_t i=0; i < XAtoms.Count(); i++ )
    if( !XAtoms[i].Atom().IsDeleted() )
      a_cnt++;
  TEBitArray vis(a_cnt);
  a_cnt = 0;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( !XAtoms[i].Atom().IsDeleted() )
      vis.Set(a_cnt++, XAtoms[i].IsVisible());
  }
  visibility.AddField("atoms", vis.ToBase64String());
  size_t b_cnt = 0;
  for( size_t i=0; i < XBonds.Count(); i++ )
    if( !XBonds[i].Bond().IsDeleted() )
      b_cnt++;
  vis.SetSize(b_cnt);
  b_cnt = 0;
  for( size_t i=0; i < XBonds.Count(); i++ )  {
    if( !XBonds[i].Bond().IsDeleted() )
      vis.Set(b_cnt++, XBonds[i].IsVisible() );
  }
  visibility.AddField("bonds", vis.ToBase64String());
  size_t p_cnt = 0;
  for( size_t i=0; i < XPlanes.Count(); i++ )
    if( !XPlanes[i].Plane().IsDeleted() )
      p_cnt++;
  vis.SetSize(p_cnt);
  p_cnt = 0;
  for( size_t i=0; i < XPlanes.Count(); i++ )  {
    if( !XPlanes[i].Plane().IsDeleted() )
      vis.Set(p_cnt++, XPlanes[i].IsVisible());
  }
  visibility.AddField("planes", vis.ToBase64String());
  
  FXGrid->ToDataItem(item.AddItem("Grid"), zos);
  FDBasis->ToDataItem(item.AddItem("DBasis"));

  TDataItem& labels = item.AddItem("Labels");
  for( size_t i=0; i < XLabels.Count(); i++ )
    XLabels[i].ToDataItem( labels.AddItem("Label") );

  FGlRender->GetSelection().SetTag(-1);
  for( size_t i=0; i < FGlRender->GroupCount(); i++ )
    FGlRender->GetGroup(i).SetTag(i);
  
  TDataItem& groups = item.AddItem("Groups");
  for( size_t i=0; i < FGlRender->GroupCount(); i++ )  {
    TGlGroup& glG = FGlRender->GetGroup(i);
    TDataItem& group = groups.AddItem(i, glG.GetCollectionName());
    group.AddField("visible", glG.IsVisible());
    group.AddField("parent_id", glG.GetParentGroup() == NULL ? -2 : glG.GetParentGroup()->GetTag());
    TDataItem& atoms = group.AddItem("Atoms");
    TDataItem& bonds = group.AddItem("Bonds");
    TDataItem& planes = group.AddItem("Planes");
    for( size_t j=0; j < glG.Count(); j++ )  {
      AGDrawObject& glO = glG.GetObject(j);
      if( EsdlInstanceOf(glO, TXAtom) )
        atoms.AddField("atom_id", ((TXAtom&)glO).Atom().GetTag() );
      if( EsdlInstanceOf(glO, TXBond) )
        bonds.AddField("bond_id", ((TXBond&)glO).Bond().GetTag() );
      if( EsdlInstanceOf(glO, TXPlane) )
        planes.AddField("plane_id", ((TXPlane&)glO).Plane().GetTag() );
    }
  }

  TDataItem& renderer = item.AddItem("Renderer");
  renderer.AddField("min", PersUtil::VecToStr( FGlRender->MinDim() ) );
  renderer.AddField("max", PersUtil::VecToStr( FGlRender->MaxDim() ) );
}
//..............................................................................
void TGXApp::FromDataItem(TDataItem& item, IInputStream& zis)  {
  FGlRender->Clear();
  ClearXObjects();
  FXFile->FromDataItem(item.FindRequiredItem("XFile"));
  FGlRender->GetStyles().FromDataItem( item.FindRequiredItem("Style") );
  
  IndividualCollections.Clear();
  TDataItem& ind_col = item.FindRequiredItem("ICollections");
  for( size_t i=0; i < ind_col.FieldCount(); i++ )
    IndividualCollections.Add(ind_col.GetField(i));

  const TDataItem& labels = item.FindRequiredItem("Labels");
  for( size_t i=0; i < labels.ItemCount(); i++ )
    XLabels.Add(new TXGlLabel(*FGlRender,"PLabels") ).FromDataItem(labels.GetItem(i));

  FXGrid->FromDataItem(item.FindRequiredItem("Grid"), zis);
  FDBasis->FromDataItem(item.FindRequiredItem("DBasis"));

  CreateObjects(true, true);

  TDataItem& visibility = item.FindRequiredItem("Visibility");
  bool v = visibility.GetRequiredField("h_atoms").ToBool();
  if( v != FHydrogensVisible )  HydrogensVisible(v);
  v = visibility.GetRequiredField("h_bonds").ToBool();
  if( v != FHBondsVisible )  HBondsVisible(v);
  v = visibility.GetRequiredField("q_atoms").ToBool();
  if( v != FQPeaksVisible )  QPeaksVisible(v);
  v = visibility.GetRequiredField("q_bonds").ToBool();
  if( v != FQPeakBondsVisible )  QPeakBondsVisible(v);
  FDBasis->SetVisible(visibility.GetRequiredField("basis").ToBool());
  FDUnitCell->SetVisible( visibility.GetRequiredField("cell").ToBool() );

  TEBitArray vis;
  vis.FromBase64String( visibility.GetRequiredField("atoms") );
  if( vis.Count() != XAtoms.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "integrity is broken");
  for( size_t i=0; i < vis.Count(); i++ )
    XAtoms[i].SetVisible( vis[i] );
  vis.FromBase64String( visibility.GetRequiredField("bonds") );
  if( vis.Count() != XBonds.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "integrity is broken");
  for( size_t i=0; i < vis.Count(); i++ )
    XBonds[i].SetVisible( vis[i] );
  vis.FromBase64String( visibility.GetRequiredField("planes") );
  if( vis.Count() != XPlanes.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "integrity is broken");
  for( size_t i=0; i < vis.Count(); i++ )
    XPlanes[i].SetVisible( vis[i] );

  const TDataItem& groups = item.FindRequiredItem("Groups");
  // pre-create all groups
  for( size_t i=0; i < groups.ItemCount(); i++ )
    FGlRender->NewGroup(groups.GetItem(i).GetValue());
  // load groups
  for( size_t i=0; i < groups.ItemCount(); i++ )  {
    const TDataItem& group = groups.GetItem(i);
    TGlGroup& glG = FGlRender->GetGroup(i);
    glG.SetVisible( group.GetRequiredField("visible").ToBool() );
    const int p_id = group.GetRequiredField("parent_id").ToInt();
    if( p_id == -1 )
      FGlRender->GetSelection().Add(glG);
    else if( p_id >= 0 )
      FGlRender->GetGroup(p_id).Add(glG);
    TDataItem& atoms = group.FindRequiredItem("Atoms");
    for( size_t j=0; j < atoms.FieldCount(); j++ )
      glG.Add( XAtoms[atoms.GetField(j).ToSizeT()] );
    TDataItem& bonds = group.FindRequiredItem("Bonds");
    for( size_t j=0; j < bonds.FieldCount(); j++ )
      glG.Add( XBonds[bonds.GetField(j).ToSizeT()] );
    TDataItem& planes = group.FindRequiredItem("Planes");
    for( size_t j=0; j < planes.FieldCount(); j++ )
      glG.Add( XPlanes[planes.GetField(j).ToSizeT()] );
    glG.Create();
  }

  TDataItem& renderer = item.FindRequiredItem("Renderer");
  vec3d min = PersUtil::FloatVecFromStr(renderer.GetRequiredField("min"));
  vec3d max = PersUtil::FloatVecFromStr(renderer.GetRequiredField("max"));
  FGlRender->SetSceneComplete(false);
  FGlRender->ClearMinMax();
  FGlRender->UpdateMaxMin(max, min);
  FGlRender->GetBasis().FromDataItem( item.FindRequiredItem("Basis") );
  FGlRender->SetSceneComplete(true);
}
//..............................................................................
void TGXApp::SaveModel(const olxstr& fileName) const {
  TDataFile df;
  wxFileOutputStream fos( fileName.u_str() );
  fos.Write("oxm", 3);
  wxZipOutputStream zos(fos, 9);
  TDataItem& mi = df.Root().AddItem("olex_model");
  zos.PutNextEntry( wxT("grid") );
  TwxOutputStreamWrapper os(zos);
  ToDataItem(mi, os);
  zos.CloseEntry();
  zos.PutNextEntry( wxT("model") );
  TEStrBuffer bf(1024*32);
  df.Root().SaveToStrBuffer(bf);
#ifdef _UNICODE
  olxcstr model( TUtf8::Encode(bf.ToString()) );
#else
  olxcstr model( bf.ToString() );
#endif
  zos.Write(model.raw_str(), model.RawLen());
  zos.CloseEntry();
  zos.Close();
  fos.Close();
}
//..............................................................................
void TGXApp::LoadModel(const olxstr& fileName) {
  TEFile::CheckFileExists(__OlxSourceInfo, fileName);
  wxFileInputStream fis(fileName.u_str());
  char sig[3];
  wxZipInputStream* zin = new wxZipInputStream(fis);
  fis.Read(sig, 3);
  if( olxstr::o_memcmp(sig, "oxm", 3) != 0 )  {
    delete zin;
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file signature");
  }

  wxZipEntry* model = NULL, *grid = NULL, *zen;
  olxstr entryModel("model"), entryGrid("grid");

  while( (zen = zin->GetNextEntry()) != NULL )  {
    if( entryModel == zen->GetName().c_str() )
      model = zen;
    else if( entryGrid == zen->GetName().c_str() )
      grid = zen;
    else
      delete zen;
  }
  if( model == NULL || grid == NULL )  {
    delete zin;
    throw TFunctionFailedException(__OlxSourceInfo, "invalid model file description");
  }
  zin->OpenEntry(*model);
  uint32_t contentLen = zin->GetLength();
  unsigned char * bf = new unsigned char[contentLen + 1];
  zin->Read(bf, contentLen);
  zin->CloseEntry();
  TEMemoryInputStream ms(bf, contentLen);
  TDataFile df;
  df.LoadFromTextStream(ms);
  delete [] bf;
  zin->OpenEntry(*grid);
  TwxInputStreamWrapper in(*zin);
  try  {  FromDataItem( df.Root().FindRequiredItem("olex_model"), in );  }
  catch( const TExceptionBase& exc )  {
    GetLog().Exception( olxstr("Failed to load model: ") << exc.GetException()->GetError() );
    FXFile->SetLastLoader(NULL);
    FXFile->LastLoaderChanged();
    CreateObjects(false, false);
  }
  delete model;
  delete grid;
  delete zin;
}
//..............................................................................
