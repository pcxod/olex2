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

#include "gxapp.h"
#include "log.h"
#include "xeval.h"

#include "network.h"
#include "unitcell.h"
#include "symmparser.h"

#include "efile.h"

#include "xlattice.h"
#include "egc.h"
#include "eutf8.h"
#include "ememstream.h"

#include "gpcollection.h"
#include "estopwatch.h"
#include "pers_util.h"
#include "planesort.h"
#include "cif.h"

#ifdef __WXWIDGETS__
  #include "wxglscene.h"
  #include "wx/string.h"
  #include "wx/fontutil.h"
  #include "wx/wfstream.h"
  #include "wxzipfs.h"
#elif __WIN32__
  #include "wglscene.h"
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
  TTypeList<TSAtom::FullRef> SAtomIds;
  TKillUndo(IUndoAction *action):TUndoData(action)  {  }
  virtual ~TKillUndo()  {  }
  void AddSAtom(const TSAtom& SA)  {  SAtomIds.AddCCopy(SA.GetFullRef());  }
};
class THideUndo: public TUndoData  {
public:
  AGDObjList Objects;
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
    // make sure that these are only cleared when file is loaded
    if( Sender && EsdlInstanceOf(*Sender, TXFile) )  {
      EmptyFile = SameFile = false;
      if( Data != NULL && EsdlInstanceOf(*Data, olxstr) )  {
        olxstr s1( TEFile::UnixPath(TEFile::ChangeFileExt(*(olxstr*)Data, EmptyString)) );
        olxstr s2( TEFile::UnixPath(TEFile::ChangeFileExt(FParent->XFile().GetFileName(), EmptyString)) );
        if( s1 != s2 )  {
          FParent->ClearIndividualCollections();
          FParent->GetRender().GetStyles().RemoveNamedStyles("Q");
          FParent->XFile().GetLattice().ClearPlaneDefinitions();
        }
        else  {
          const TAsymmUnit& au = FParent->XFile().GetAsymmUnit();
          size_t ac = 0;
          for( size_t i=0; i < au.AtomCount(); i++ )  {
            const TCAtom& ca = au.GetAtom(i);           
            if( ca.IsDeleted() || ca.GetType() == iQPeakZ )  continue;
            ac++;
          }
          AtomNames.SetCapacity(ac);
          CAtomMasks.SetSize(ac);
          ac = 0;
          for( size_t i=0; i < au.AtomCount(); i++ )  {
            const TCAtom& ca = au.GetAtom(i);
            if( ca.IsDeleted() || ca.GetType() == iQPeakZ )  continue;
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
        FParent->XFile().GetLattice().ClearPlaneDefinitions();
        FParent->ClearGroupDefinitions();
      }
      //FParent->XGrid().Clear();
    }
    else  {
      SameFile = true;
      EmptyFile = false;
    }
    B = FParent->GetRender().GetBasis();
    FParent->GetRender().Clear();
    FParent->HklFile().Clear();
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    const TAsymmUnit& au = FParent->XFile().GetAsymmUnit();
    bool sameAU = true;
    size_t ac = 0;
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      const TCAtom& ca = au.GetAtom(i);
      if( ca.IsDeleted() || ca.GetType() == iQPeakZ )  continue;
      if( ac >= AtomNames.Count() )  {
        sameAU = false;
        break;
      }
      if( !AtomNames[ac++].Equalsi(ca.GetLabel()) )  {
        sameAU = false;
        break;
      }
    }
    FParent->XFile().GetAsymmUnit().DetachAtomType(iQPeakZ, !FParent->AreQPeaksVisible());
    FParent->XFile().GetAsymmUnit().DetachAtomType(iHydrogenZ, !FParent->AreHydrogensVisible());
    if( sameAU )  {  // apply masks
      ac = 0;
      for( size_t i=0; i < au.AtomCount(); i++ )  {
        TCAtom& ca = au.GetAtom(i);
        if( ca.IsDeleted() || ca.GetType() == iQPeakZ )  continue;
        ca.SetMasked( CAtomMasks[ac++] );
      }
      FParent->XFile().GetLattice().SetGrowInfo(GrowInfo);
      GrowInfo = NULL;
    }
    else  {  // definition will get broken otherwise
      FParent->XFile().GetLattice().ClearPlaneDefinitions();
      FParent->ClearLabels();
      FParent->ClearGroupDefinitions();
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
    FParent->GetRender().SetZoom(FParent->GetRender().CalcZoom()*FParent->GetExtraZoom());
    FParent->Draw();
    return true;
  }
};
//..............................................................................
class xappXFileUniq : public AActionHandler  {
public:
  virtual bool Exit(const IEObject *Sender, const IEObject *Data)  {
    TGXApp& app = TGXApp::GetInstance();
    if( app.OverlayedXFileCount() != 0 )
      app.AlignOverlayedXFiles();
    return true;
  }
};
//..............................................................................
class xappXFileClose: public AActionHandler  {
public:
  virtual bool Exit(const IEObject *Sender, const IEObject *Data)  {
    TGXApp& app = TGXApp::GetInstance();
    app.ClearLabels();
    app.XGrid().Clear();
    app.CreateObjects(false, false);
    app.GetRender().SetZoom(app.GetRender().CalcZoom());
    return true;
  }
};
//..............................................................................
//----------------------------------------------------------------------------//
//TGXApp function bodies
//----------------------------------------------------------------------------//
enum  {
  ID_OnSelect = 1,
  ID_OnDisassemble,
  ID_OnUniq,
  ID_OnGrow,
  ID_OnClear,
  ID_OnFileLoad
};

TGXApp::TGXApp(const olxstr &FileName) : TXApp(FileName, this),
  OnGraphicsVisible(NewActionQueue("GRVISIBLE")),
  OnFragmentVisible(NewActionQueue("FRVISIBLE")),
  OnAllVisible(NewActionQueue("ALLVISIBLE")),
  OnObjectsDestroy(NewActionQueue("OBJECTSDESTROY")),
  OnObjectsCreate(NewActionQueue("OBJECTSCREATE"))
{
  FQPeaksVisible = FHydrogensVisible = FStructureVisible = FHBondsVisible = true;
  XGrowPointsVisible = FXGrowLinesVisible = FQPeakBondsVisible = false;
  MainFormVisible = false;
  FXPolyVisible = true;
  DeltaV = 3;
  const TGlMaterial glm("2049;0.698,0.698,0.698,1.000");
#ifdef __WXWIDGETS__
  TwxGlScene *GlScene = new TwxGlScene(GetBaseDir() + "etc/Fonts/");
  wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
  GlScene->CreateFont("Default", Font.GetNativeFontInfoDesc().c_str()).SetMaterial(glm);
#else
  TWGlScene *GlScene = new TWGlScene();
  GlScene->CreateFont("Default", "@20").SetMaterial(glm);
#endif
  FGrowMode = gmCovalent;
//  TWGlScene *GlScene = new TWGlScene;
//  TGlScene *GlScene = new TGlScene;
  FGlRender = new TGlRenderer(GlScene, 1,1);
  FDFrame = new TDFrame(*FGlRender, "DFrame");
  Fader = new TXFader(*FGlRender, "Fader");
  FDFrame->OnSelect.Add(this, ID_OnSelect);
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
  ObjectsToCreate.Add(FDBasis);
  ObjectsToCreate.Add(FDUnitCell);
  ObjectsToCreate.Add(FDFrame);
  ObjectsToCreate.Add(Fader);

  FHklFile = new THklFile();
  FHklVisible = false;

  FXGrid = new TXGrid("XGrid", this);

  ObjectsToCreate.Add(F3DFrame=new T3DFrameCtrl(*FGlRender, "3DFrame"));
  F3DFrame->SetVisible(false);
  XFile().GetLattice().OnDisassemble.Add(this, ID_OnDisassemble);

  xappXFileLoad *P = &TEGC::NewG<xappXFileLoad>(this);
  XFile().GetLattice().OnStructureGrow.Add(P);
  XFile().GetLattice().OnStructureGrow.Add(new xappXFileUniq);
  XFile().GetLattice().OnStructureUniq.Add(P);
  XFile().GetLattice().OnStructureUniq.Add(this, ID_OnUniq);
  XFile().GetLattice().OnStructureGrow.Add(this, ID_OnGrow);
  XFile().GetLattice().OnAtomsDeleted.Add(this, ID_OnClear);
  XFile().OnFileLoad.Add(this, ID_OnFileLoad);
  XFile().GetLattice().OnStructureUniq.Add(new xappXFileUniq);
  XFile().OnFileLoad.Add(P);
  XFile().OnFileClose.Add(new xappXFileClose);
}
//..............................................................................
TGXApp::~TGXApp()  {
  XFile().GetLattice().OnAtomsDeleted.Remove(this);
  Clear();
  delete FGlRender;
  delete FLabels;
  delete FHklFile;
  delete FXGrid;
  delete FGlMouse;
}
//..............................................................................
void TGXApp::ClearXObjects()  {
  OnObjectsDestroy.Enter(dynamic_cast<TBasicApp*>(this), NULL);
  FGlRender->SelectAll(false);
  FGlRender->ClearGroups();
  FGlRender->GetSelection().Clear();
  XAtoms.Clear();
  XBonds.Clear();
  XGrowLines.Clear();
  XGrowPoints.Clear();
  XReflections.Clear();
  XPlanes.Clear();
  OnObjectsDestroy.Exit(dynamic_cast<TBasicApp*>(this), NULL);
}
//..............................................................................
void TGXApp::Clear()  {
  ClearXObjects();
 
  for( size_t i=0; i < LooseObjects.Count(); i++ )  
    delete LooseObjects[i];   
  LooseObjects.Clear();

  for( size_t i=0; i < ObjectsToCreate.Count(); i++ )
    delete ObjectsToCreate[i];
  ObjectsToCreate.Clear();

  XLabels.Clear();
  GlBitmaps.Clear();
}
//..............................................................................
void TGXApp::CreateXRefs()  {
  if( !XReflections.IsEmpty() )  return;
  TRefList refs;
  vec3d mind = FGlRender->MinDim(),
        maxd = FGlRender->MaxDim();
  RefinementModel::HklStat stats = 
    XFile().GetRM().GetRefinementRefList<TUnitCell::SymSpace,RefMerger::StandardMerger>(
    XFile().GetUnitCell().GetSymSpace(), refs);
  for( size_t i=0; i < refs.Count(); i++ )  {
    TXReflection* xr = new TXReflection(*FGlRender, "XReflection", stats.MinI, stats.MaxI, refs[i],
      FXFile->GetAsymmUnit());
    xr->Create();
    XReflections.Add(*xr);
    vec3d::UpdateMinMax(xr->GetCenter(), mind, maxd);
  }
  FGlRender->UpdateMinMax(mind, maxd);
}
//..............................................................................
size_t TGXApp::GetNetworks(TNetPList& nets) {
  size_t c = XFile().GetLattice().FragmentCount();
  for( size_t i=0; i < c; i++ )
    nets.Add(XFile().GetLattice().GetFragment(i));

  for( size_t i=0; i < OverlayedXFiles.Count(); i++ )  {
    size_t fc = OverlayedXFiles[i].GetLattice().FragmentCount();
    c += fc;
    for( size_t j=0; j < fc; j++ )
      nets.Add(OverlayedXFiles[i].GetLattice().GetFragment(j));
  }
  return c;
}
//..............................................................................
void TGXApp::CreateObjects(bool SyncBonds, bool centerModel)  {
  OnObjectsCreate.Enter(dynamic_cast<TBasicApp*>(this), NULL);
  TStopWatch sw(__FUNC__);
  sw.start("Initialising");
  const vec3d
    glMax = FGlRender->MaxDim(),
    glMin = FGlRender->MinDim(),
    glCenter = FGlRender->GetBasis().GetCenter();
  TXAtom::ClearStaticObjects();
  TXBond::ClearStaticObjects();
  FGlRender->ClearPrimitives();
  FLabels->Clear();
  ClearXObjects();
  FGlRender->SetSceneComplete(false);

  for( size_t i=0; i < IndividualCollections.Count(); i++ )
    FGlRender->NewCollection(IndividualCollections[i]);

  size_t totalACount = XFile().GetLattice().AtomCount();
  for( size_t i=0; i < OverlayedXFiles.Count(); i++ )
    totalACount += OverlayedXFiles[i].GetLattice().AtomCount();
  size_t totalBCount = XFile().GetLattice().BondCount();
  for( size_t i=0; i < OverlayedXFiles.Count(); i++ )
    totalBCount += OverlayedXFiles[i].GetLattice().BondCount();

  GetRender().SetObjectsCapacity(totalACount + totalBCount + 512);

  sw.start("Atoms creation");
  TSAtomPList allAtoms;
  allAtoms.SetCapacity(totalACount);
  for( size_t i=0; i < XFile().GetLattice().AtomCount(); i++ )
    allAtoms.Add(XFile().GetLattice().GetAtom(i));
  for( size_t i=0; i < OverlayedXFiles.Count(); i++ )  {
    for( size_t j=0; j < OverlayedXFiles[i].GetLattice().AtomCount(); j++ )
      allAtoms.Add(OverlayedXFiles[i].GetLattice().GetAtom(j));
  }

  XAtoms.SetCapacity(allAtoms.Count());
  const size_t this_a_count = XFile().GetLattice().AtomCount();
  for( size_t i=0; i < allAtoms.Count(); i++ )  {
    allAtoms[i]->SetTag(i);
    TXAtom& XA = XAtoms.Add(new TXAtom(*FGlRender, EmptyString, *allAtoms[i]));
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
  allBonds.SetCapacity(totalBCount);
  for( size_t i=0; i < XFile().GetLattice().BondCount(); i++ )
    allBonds.Add(XFile().GetLattice().GetBond(i));
  for( size_t i=0; i < OverlayedXFiles.Count(); i++ )  {
    for( size_t j=0; j < OverlayedXFiles[i].GetLattice().BondCount(); j++ )
      allBonds.Add(OverlayedXFiles[i].GetLattice().GetBond(j));
  }
  XBonds.SetCapacity(allBonds.Count());
  for( size_t i=0; i < allBonds.Count(); i++ )  {
    TSBond* B = allBonds[i];
    TXBond& XB = XBonds.Add(new TXBond(*FGlRender, TXBond::GetLegend( *B, 2), *allBonds[i]));
    XB.SetDeleted(B->IsDeleted() || (B->A().IsDeleted() || allBonds[i]->B().IsDeleted()));
    BondCreationParams bcpar(XAtoms[B->A().GetTag()], XAtoms[B->B().GetTag()]);
    XB.Create(EmptyString, &bcpar);
    XB.SetXAppId(i);
    if( !FStructureVisible )  {  
      XB.SetVisible(FStructureVisible);  
      continue;  
    }
    XB.SetVisible( XAtoms[B->A().GetTag()].IsVisible() && XAtoms[B->B().GetTag()].IsVisible() );
    if( !XB.IsVisible() )  continue;
    if( (B->A().GetType() == iQPeakZ) || (B->B().GetType() == iQPeakZ) )  {  
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
    TXPlane& XP = XPlanes.Add(new TXPlane(*FGlRender, olxstr("TXPlane") << (P.GetDefId() == InvalidIndex ? i : P.GetDefId()), &P));
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
  DUnitCell().Init(cell);
  DBasis().SetAsymmUnit(XFile().GetAsymmUnit());

  for( size_t i=0; i < ObjectsToCreate.Count(); i++ )
    ObjectsToCreate[i]->Create();

  /*somehow if the XLAbels are created before the DBasis (like after picture drawing),
  the 'disappear' from opengl selection... - the plane is drawn in different color and
  selection is inpossible, unless properties are changed, odd... could not figure out
  what is going wrong... */

  XLabels.Pack(olx_alg::olx_not(AGDrawObject::FlagsAnalyser(sgdoVisible)));
  for( size_t i=0; i < XLabels.Count(); i++ )  {
    if( XLabels[i].IsVisible() )
      XLabels[i].Create();
    else
      XLabels.NullItem(i);
  }
  XLabels.Pack();

  for( size_t i=0; i < LooseObjects.Count(); i++ )  {
    if( LooseObjects[i]->IsDeleted() )  {
      delete LooseObjects[i];
      LooseObjects[i] = NULL;
    }
    else
      LooseObjects[i]->Create();
  }
  LooseObjects.Pack();

  FLabels->Init();
  FLabels->Create();

  if( FXGrowLinesVisible )  CreateXGrowLines();
  if( XGrowPointsVisible )  CreateXGrowPoints();
  FXGrid->Create();
  RestoreGroups();
  // create hkls
  if( FHklVisible )  SetHklVisible(true);

  if( SyncBonds )  XAtomDS2XBondDS("Sphere");

  if( centerModel )
    CenterModel();
  else  {
    FGlRender->ClearMinMax();
    FGlRender->UpdateMinMax(glMin, glMax);
    FGlRender->GetBasis().SetCenter(glCenter);
  }
  olx_gl::loadIdentity();
  GetRender().SetView(false, 1);
  GetRender().Initialise();
  FGlRender->SetSceneComplete(true);
  sw.stop();
  sw.print(GetLog(), &TLog::Info);
  OnObjectsCreate.Exit(dynamic_cast<TBasicApp*>(this), NULL);
}
//..............................................................................
void TGXApp::CenterModel()  {
  const size_t ac = FXFile->GetLattice().AtomCount();
  double weight = 0;
  vec3d center, maX(-100, -100, -100), miN(100, 100, 100);
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& A = FXFile->GetLattice().GetAtom(i);
    if( !A.IsDeleted() )  {
      center += A.crd();
      vec3d::UpdateMinMax(A.crd(), miN, maX);
      weight += 1;
    }
  }
  if( FDUnitCell->IsVisible() )  {
    for( size_t i=0; i < FDUnitCell->VertexCount(); i++ )  {
      center += FDUnitCell->GetVertex(i);
      weight += 1;
      vec3d::UpdateMinMax(FDUnitCell->GetVertex(i), miN, maX);
    }
  }
  for( size_t i=0; i < XReflections.Count(); i++ )  {
    if( !XReflections[i].IsVisible() || XReflections[i].IsDeleted() )  continue;
    center += XReflections[i].GetCenter();
    weight += 1;
    vec3d::UpdateMinMax(XReflections[i].GetCenter(), miN, maX);
  }
  if( weight == 0 )  return;
  center /= weight;
  center *= -1;
  FGlRender->GetBasis().SetCenter(center);
  vec3d max = FGlRender->MaxDim() + center;
  vec3d min = FGlRender->MinDim() + center;
  FGlRender->ClearMinMax();
  FGlRender->UpdateMinMax(min, max);
}
//..............................................................................
void TGXApp::CenterView(bool calcZoom)  {
  double weight = 0;
  vec3d center;
  vec3d maX(-100, -100, -100), miN(100, 100, 100);
  if( FXFile->GetLattice().AtomCount() == 0 )  return;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    TXAtom& XA = XAtoms[i];
    if( !XA.IsDeleted() && XA.IsVisible() )  {
      center += XA.Atom().crd();
      weight += 1;
      vec3d::UpdateMinMax(XA.Atom().crd(), miN, maX);
    }
  }
  if( FDUnitCell->IsVisible() )  {
    for( size_t i=0; i < FDUnitCell->VertexCount(); i++ )  {
      center += FDUnitCell->GetVertex(i);
      weight += 1;
      vec3d::UpdateMinMax(FDUnitCell->GetVertex(i), miN, maX);
    }
  }
  for( size_t i=0; i < XReflections.Count(); i++ )  {
    if( !XReflections[i].IsVisible() || XReflections[i].IsDeleted() )  continue;
    center += XReflections[i].GetCenter();
    weight += 1;
    vec3d::UpdateMinMax(XReflections[i].GetCenter(), miN, maX);
  }
  if( weight == 0 )  return;
  center /= weight;
  center *= -1;
  FGlRender->ClearMinMax();
  FGlRender->UpdateMinMax(miN+center, maX+center);
  if( calcZoom )
    FGlRender->GetBasis().SetZoom(FGlRender->CalcZoom()*ExtraZoom);
  FGlRender->GetBasis().SetCenter(center);
}
//..............................................................................
void TGXApp::CalcProbFactor(float Prob)  {
  if( Prob < 0 )  Prob = FProbFactor;
  TXAtom::TelpProb(ProbFactor(Prob));//, Prob50 = ProbFactor(50);
  FProbFactor = Prob;
//  AtomZoom(ProbFactor);
}
//..............................................................................
/* finds such a value of x at wich the value of integral 4*pi*exp(-x/2)*x^2 is Prob/100 of the max value, which is sqrt(8*pi^3),
max returned value is around 10 */
float TGXApp::ProbFactor(float Prob)  {
  static const double max_val = sqrt(8*M_PI*M_PI*M_PI)/(4*M_PI*100.0);  // max of 4pi*int(0,inf)(exp(-x/2)*x^2dx) [/(4*pi*100)]
  const double t_val = Prob * max_val, inc = 1e-4;
  double ProbFactor = 0, summ = 0;
  while( summ < t_val )  {
    const double v_sq = olx_sqr(ProbFactor + inc/2);
    summ += exp(-v_sq/2)*v_sq*inc;
    if( (ProbFactor += inc) >= 10 )  //  sanity check
      break;
  }
  return (float)ProbFactor;
}
//..............................................................................
void TGXApp::Init()  {
  try  {  CreateObjects(true, false);  }
  catch(...)  {
    GetRender().GetStyles().Clear();
    CreateObjects(true, false);
  }
}
//..............................................................................
void TGXApp::Quality(const short V)  {
  if( !XAtoms.IsEmpty() )  {
    XAtoms[0].Quality(V);
    XAtoms[0].CreateStaticObjects();
  }
  if( !XBonds.IsEmpty() )  {
    XBonds[0].Quality(V);
    XBonds[0].CreateStaticObjects();
  }
  Draw();
}
//..............................................................................
bool TGXApp::IsCellVisible()  const {
  return IsGraphicsVisible(FDUnitCell);
}
//..............................................................................
void TGXApp::SetCellVisible(bool v)  {
  SetGraphicsVisible(FDUnitCell, v);
}
//..............................................................................
bool TGXApp::IsBasisVisible() const {
  return IsGraphicsVisible(FDBasis);
}
//..............................................................................
void TGXApp::SetBasisVisible(bool v)  {
  SetGraphicsVisible(FDBasis, v);
}
//..............................................................................
TUndoData* TGXApp::SetGraphicsVisible(AGDrawObject *G, bool v)  {
  if( v != IsGraphicsVisible(G) )  {
    G->SetVisible(v);
    OnGraphicsVisible.Execute(dynamic_cast<TBasicApp*>(this), G);
    Draw();
  }
  return NULL;
}
//..............................................................................
TUndoData* TGXApp::SetGraphicsVisible(AGDObjList& G, bool v)  {
  for( size_t i=0; i < G.Count(); i++ )  {
    if( v == G[i]->IsVisible() )  continue;
    G[i]->SetVisible(v);
    OnGraphicsVisible.Execute(dynamic_cast<TBasicApp*>(this), G[i]);
  }
  Draw();
  return NULL;
}
//..............................................................................
void TGXApp::BangTable(TXAtom *XA, TTTable<TStrList>& Table) {
  const TSAtom &A = XA->Atom();
  if( A.BondCount() == 0 )  return;
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
double TGXApp::Tang(TSBond *B1, TSBond *B2, TSBond *Middle, olxstr *Sequence)  {
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
  const double angle = olx_dihedral_angle_signed(A1.crd(), A2.crd(), A3.crd(), A4.crd());
  if( Sequence != NULL )  {
    *Sequence = A1.GetLabel();
    *Sequence << '-' << A2.GetLabel() <<
                 '-' << A3.GetLabel() <<
                 '-' << A4.GetLabel();
  }
  return angle;
}
void TGXApp::TangList(TXBond *XMiddle, TStrList &L)  {
  TSBondPList BondsA, BondsB;
  size_t maxl=0;
  TSBond *B, *Middle = &XMiddle->Bond();
  TSAtom *A = &Middle->A();
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
      olxstr& T = L.Add();
      const double angle = Tang( BondsA[i], BondsB[j], Middle, &T);
      T << ':' << ' ';
      if( T.Length() > maxl ) maxl = T.Length();  // to format the string later
      T << olxstr::FormatFloat(3, angle);
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
        TSAtom &a1 = ((TXAtom&)Sel[0]).Atom(),
          &a2 = ((TXAtom&)Sel[1]).Atom();
        Tmp = "Distance (";
        Tmp << macSel_GetName2(a1, a2) << "): ";
        if( CheckFileType<TCif>() )  {
          ACifValue* cv = XFile().GetLastLoader<TCif>().GetDataManager().Match(a1, a2);
          if( cv != NULL )
            Tmp << cv->GetValue().ToString();
          else
            Tmp << olxstr::FormatFloat(3, a1.crd().DistanceTo(a2.crd()));
        }
        else
          Tmp << olxstr::FormatFloat(3, a1.crd().DistanceTo(a2.crd()));
      }
      else if( EsdlInstanceOf(Sel[0], TXBond) && EsdlInstanceOf(Sel[1], TXBond) )  {
        TXBond& A = (TXBond&)Sel[0], &B =(TXBond&)Sel[1];
        Tmp = "Angle (";
        Tmp << macSel_GetName4a(A.Bond().A(), A.Bond().B(), B.Bond().A(), B.Bond().B()) <<
          "): ";
        v = olx_angle(A.Bond().A().crd(), A.Bond().B().crd(), B.Bond().A().crd(), B.Bond().B().crd());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')' <<
          "\nAngle (" <<
          macSel_GetName4a(A.Bond().A(), A.Bond().B(), B.Bond().B(), B.Bond().A()) <<
          "): ";
        v = olx_angle(A.Bond().A().crd(), A.Bond().B().crd(), B.Bond().A().crd(), B.Bond().B().crd());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')';
        // check for adjacent bonds
        if( !(&A.Bond().A() == &B.Bond().A() || &A.Bond().A() == &B.Bond().B() ||
          &A.Bond().B() == &B.Bond().A() || &A.Bond().B() == &B.Bond().B()) )
        {
          Tmp << "\nTorsion angle (" <<
            macSel_GetName4(A.Bond().A(), A.Bond().B(), B.Bond().B(), B.Bond().A()) <<
            "): ";
          v = olx_dihedral_angle_signed(A.Bond().A().crd(), A.Bond().B().crd(), B.Bond().B().crd(), B.Bond().A().crd());
          Tmp << olxstr::FormatFloat(3, v) <<
            "\nTorsion angle (" <<
            macSel_GetName4(A.Bond().A(), A.Bond().B(), B.Bond().B(), B.Bond().A()) << 
            "): ";
          v = olx_dihedral_angle_signed(A.Bond().A().crd(), A.Bond().B().crd(), B.Bond().A().crd(), B.Bond().B().crd());
          Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')';
        }
      }
      else if( EsdlInstanceOf(Sel[0], TXLine) && EsdlInstanceOf(Sel[1], TXLine) )  {
        TXLine& A = (TXLine&)Sel[0], &B =(TXLine&)Sel[1];
        Tmp = "Angle: ";
        v = olx_angle(A.Edge(), A.Base(), B.Edge(), B.Base());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ")";
      }
      else if( EsdlInstanceOf(Sel[0], TXLine) && EsdlInstanceOf(Sel[1], TXBond) )  {
        TXLine& A = (TXLine&)Sel[0];
        TXBond& B =(TXBond&)Sel[1];
        Tmp = "Angle: ";
        v = olx_angle(A.Edge(), A.Base(), B.Bond().A().crd(), B.Bond().B().crd());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ")";
      }
      else if( EsdlInstanceOf(Sel[0], TXBond) && EsdlInstanceOf(Sel[1], TXLine) )  {
        TXLine& A = (TXLine&)Sel[1];
        TXBond& B =(TXBond&)Sel[0];
        Tmp = "Angle: ";
        v = olx_angle(A.Edge(), A.Base(), B.Bond().A().crd(), B.Bond().B().crd());
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
      else if( EsdlInstanceOf(Sel[0], TXLine) && EsdlInstanceOf(Sel[1], TXPlane) )  {
        TXLine& xl = (TXLine&)Sel[0];
        Tmp = "Angle (plane-line): ";
        v = ((TXPlane&)Sel[1]).Plane().Angle(xl.Edge()-xl.Base());
        Tmp << olxstr::FormatFloat(3, v);
      }
      else if( EsdlInstanceOf(Sel[1], TXLine) && EsdlInstanceOf(Sel[0], TXPlane) )  {
        TXLine& xl = (TXLine&)Sel[1];
        Tmp = "Angle (plane-line): ";
        v = ((TXPlane&)Sel[0]).Plane().Angle(xl.Edge()-xl.Base());
        Tmp << olxstr::FormatFloat(3, v);
      }
      if( EsdlInstanceOf(Sel[1], TXPlane) && EsdlInstanceOf(Sel[0], TXPlane) )  {
        TSPlane &a = ((TXPlane&)Sel[0]).Plane(),
          &b = ((TXPlane&)Sel[1]).Plane();
        
        vec3d n_c = (b.GetCenter()-a.GetCenter()).XProdVec(a.GetNormal()+b.GetNormal()).Normalise();
        vec3d p_a = a.GetNormal() -  n_c*n_c.DotProd(a.GetNormal());
        vec3d p_b = b.GetNormal() -  n_c*n_c.DotProd(b.GetNormal());
        const double ang = a.Angle(b);
        Tmp = "Angle (plane-plane): ";
        Tmp << olxstr::FormatFloat(3, ang) <<
          "\nTwist Angle (plane-plane, experimental): " <<
          olxstr::FormatFloat(3, olx_dihedral_angle(a.GetCenter()+a.GetNormal(), a.GetCenter(), b.GetCenter(), b.GetCenter()+b.GetNormal())) <<
          "\nFold Angle (plane-plane, experimental): " <<
          olxstr::FormatFloat(3, acos(p_a.CAngle(p_b))*180/M_PI) <<
          "\nDistance (plane centroid-plane centroid): " <<
          olxstr::FormatFloat(3, a.GetCenter().DistanceTo(b.GetCenter()))  <<
          "\nDistance (plane[" << macSel_GetPlaneName(a) << "]-centroid): " <<
          olxstr::FormatFloat(3, a.DistanceTo(b.GetCenter())) <<
          "\nShift (plane [" << macSel_GetPlaneName(a) << "]-plane): " <<
          olxstr::FormatFloat(3, 
            sqrt(olx_max(0, a.GetCenter().QDistanceTo(b.GetCenter())
            -
            olx_sqr(a.DistanceTo(b.GetCenter())))));
        if( olx_abs(ang) > 1e-6 )  {
          Tmp << "\nDistance (plane[" << macSel_GetPlaneName(b) << "]-centroid): " <<
          olxstr::FormatFloat(3, b.DistanceTo(a.GetCenter())) <<
          "\nShift (plane [" << macSel_GetPlaneName(b) << "]-plane): " <<
          olxstr::FormatFloat(3, 
            sqrt(olx_max(0, a.GetCenter().QDistanceTo(b.GetCenter())
            -
            olx_sqr(b.DistanceTo(a.GetCenter())))));
        }
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
        Tmp << macSel_GetName3(a1, a2, a3)<< "): ";
        if( CheckFileType<TCif>() )  {
          ACifValue* cv = XFile().GetLastLoader<TCif>().GetDataManager().Match(a1, a2, a3);
          if( cv != NULL )
            Tmp << cv->GetValue().ToString();
          else
            Tmp << olxstr::FormatFloat(3, olx_angle(a1.crd(), a2.crd(), a3.crd()));
        }
        else
          Tmp << olxstr::FormatFloat(3, olx_angle(a1.crd(), a2.crd(), a3.crd()));
      }
      else if( EsdlInstanceOf(Sel[0], TXPlane) &&
        EsdlInstanceOf(Sel[1], TXPlane) &&
        EsdlInstanceOf(Sel[2], TXPlane) )  {
          TSPlane &p1 = ((TXPlane&)Sel[0]).Plane(),
            &p2 = ((TXPlane&)Sel[1]).Plane(),
            &p3 = ((TXPlane&)Sel[2]).Plane();
          Tmp = "Angle between plane centroids: ";
          Tmp << olxstr::FormatFloat(3, 
            olx_angle(p1.GetCenter(), p2.GetCenter(), p3.GetCenter()));
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
          v = olx_dihedral_angle_signed(a1.crd(), a2.crd(), a3.crd(), a4.crd());
          Tmp << olxstr::FormatFloat(3, v);
          Tmp << 
            "\nAngle (" << macSel_GetName3(a1, a2, a3) << "): " <<
            olxstr::FormatFloat(3, olx_angle(a1.crd(), a2.crd(), a3.crd())) <<
            "\nAngle (" << macSel_GetName3(a2, a3, a4) << "): " << 
            olxstr::FormatFloat(3, olx_angle(a2.crd(), a3.crd(), a4.crd())) <<
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
              const double thv = olx_tetrahedron_volume( 
                central_atom->crd(),
                (atoms[i]->crd()-central_atom->crd()).Normalise() + central_atom->crd(),
                (atoms[j]->crd()-central_atom->crd()).Normalise() + central_atom->crd(),
                (atoms[k]->crd()-central_atom->crd()).Normalise() + central_atom->crd());
              if( thv < 0.1 )  continue;
              face_cnt++;
              TSAtomPList sorted_atoms;
              olxdict<index_t, vec3d, TPrimitiveComparator> transforms;
              atoms.ForEach(ACollectionItem::TagSetter<>(0));
              atoms[i]->SetTag(1);
              atoms[j]->SetTag(1);
              atoms[k]->SetTag(1);
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
      SA.Add(frags[i]->Node(j));
  }
  SAtoms2XAtoms(SA, XA);
  for( size_t i=0; i < XA.Count(); i++ )  {
    if( v )  {
      if( XA[i]->IsVisible() && !XA[i]->IsSelected() )
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
      SB.Add(frags[i]->Bond(j));
  }
  SBonds2XBonds(SB, XB);
  for( size_t i=0; i < XB.Count(); i++ )  {
    if( v )  {
      if( XB[i]->IsVisible() && !XB[i]->IsSelected() )
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
    SA.Add(N->Node(i));
  SAtoms2XAtoms(SA, XA);
  for( size_t i=0; i < XA.Count(); i++ )
    XA[i]->SetVisible(V);

  TSBondPList SB;
  TXBondPList XB;
  SB.SetCapacity(N->BondCount());
  for( size_t i=0; i < N->BondCount(); i++ )
    SB.Add(N->Bond(i));
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
  SetHBondsVisible(FHBondsVisible);
  _maskInvisible();
  //  OnFragmentsVisible->Exit(this, dynamic_cast<IEObject*>(Frags));
  Draw();
}
//..............................................................................
TGlGroup& TGXApp::GroupFragments(const TNetPList& Fragments, const olxstr groupName)  {
  GetRender().GetSelection().Clear();
  TSBondPList sbonds;
  TXBondPList xbonds;
  for( size_t i=0; i < Fragments.Count(); i++ )  {
    for( size_t j=0; j < Fragments[i]->BondCount(); j++ )
      sbonds.Add(Fragments[i]->Bond(j));
  }
  if( sbonds.IsEmpty() )  return *(TGlGroup*)NULL;
  SBonds2XBonds(sbonds, xbonds);
  for( size_t i=0; i < xbonds.Count(); i++ )
    GetRender().GetSelection().Add(*xbonds[i]);
  return *GetRender().GroupSelection(groupName);
}
//..............................................................................
size_t TGXApp::InvertFragmentsList(const TNetPList& SF, TNetPList& Result)  {
  TLattice& L = XFile().GetLattice();
  size_t fc=0;
  L.GetFragments().ForEach(ACollectionItem::TagSetter<>(1));
  SF.ForEach(ACollectionItem::TagSetter<>(0));
  for( size_t i=0; i < L.FragmentCount(); i++ )  {
    if( L.GetFragment(i).GetTag() != 0 )  {
      Result.Add(L.GetFragment(i));
      fc++;
    }
  }
  return fc;
}
//..............................................................................
void TGXApp::SyncAtomAndBondVisiblity(short atom_type, bool show_a, bool show_b)  {
  XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    TXAtom& a = XAtoms[i];
    if( a.Atom().GetType() != atom_type )
      continue;
    if( atom_type == iHydrogenZ )  {
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
    if( atom_type == iHydrogenZ )   {
      if( b.Bond().GetType() == sotHBond )
        b.SetVisible(show_b);
      else if( b.Bond().A().GetType() == atom_type || b.Bond().B().GetType() == atom_type )  {
        // there is always a special case...
        if( (b.Bond().A().GetType() == iQPeakZ || b.Bond().B().GetType() == iQPeakZ) &&
            !FQPeakBondsVisible )
        {
          b.SetVisible(false);
        }
        else
          b.SetVisible(show_a);
      }
    }
    else if( b.Bond().A().GetType() == atom_type || b.Bond().B().GetType() == atom_type )
      b.SetVisible(show_b);
  }
  if( FXGrowLinesVisible )  {
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      if( XGrowLines[i].SAtom()->GetType() == atom_type )
        XGrowLines[i].SetVisible( XAtoms[XGrowLines[i].SAtom()->GetTag()].IsVisible() );
    }
  }
}
//..............................................................................
void TGXApp::AllVisible(bool V)  {
  OnAllVisible.Enter(dynamic_cast<TBasicApp*>(this), NULL);
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
    UpdateConnectivity();
    CenterView(true);
  }
  OnAllVisible.Exit(dynamic_cast<TBasicApp*>(this), NULL);
  Draw();
}
//..............................................................................
void TGXApp::Select(const vec3d& From, const vec3d& To )  {
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    TXAtom& XA = XAtoms[i];
    if( XA.IsVisible() )  {
      vec3d Cnt = XA.Atom().crd() + GetRender().GetBasis().GetCenter();
      Cnt *= GetRender().GetBasis().GetMatrix();
      Cnt *= GetRender().GetBasis().GetZoom();
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
      vec3d Cnt = B.Bond().A().crd() + GetRender().GetBasis().GetCenter();
      Cnt *= GetRender().GetBasis().GetMatrix();
      Cnt *= GetRender().GetBasis().GetZoom();
      vec3d Cnt1 = B.Bond().B().crd() + GetRender().GetBasis().GetCenter();
      Cnt1 *= GetRender().GetBasis().GetMatrix();
      Cnt1 *= GetRender().GetBasis().GetZoom();
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
      vec3d Cnt = XR.GetCenter() + GetRender().GetBasis().GetCenter();
      Cnt *= GetRender().GetBasis().GetMatrix();
      Cnt *= GetRender().GetBasis().GetZoom();
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
  static bool ObjectsStored = false, LoadingFile = false;
  if( MsgId == ID_OnSelect )  {
    const TSelectionInfo* SData = dynamic_cast<const TSelectionInfo*>(Data);
    if(  !(SData->From == SData->To) )
      Select(SData->From, SData->To);
  }
  else if( (MsgId == ID_OnUniq || MsgId == ID_OnGrow) && MsgSubId == msiEnter ) {
  }
  else if( MsgId == ID_OnFileLoad )  {
    if( MsgSubId == msiEnter )  {
      SelectionCopy[0].Clear();
      StoreGroup(GetSelection(), SelectionCopy[0]);
      StoreLabels();
      LoadingFile = true;
    }
    else if( MsgSubId == msiExit )
      LoadingFile = false;
  }
  else if( MsgId == ID_OnDisassemble ) {
    if( MsgSubId == msiExit )
      CreateObjects(false, false);
    else if( MsgSubId == msiEnter )  {  // backup the selection
      if( ObjectsStored )
        ObjectsStored = false;
      else  if( !LoadingFile )  {
        SelectionCopy[0].Clear();
        StoreGroup(GetSelection(), SelectionCopy[0]);
        StoreLabels();
      }
    }
  }
  else if( MsgId == ID_OnClear ) {
    if( MsgSubId == msiEnter && !LoadingFile )  {  // backup the selection
      SelectionCopy[0].Clear();
      StoreGroup(GetSelection(), SelectionCopy[0]);
      StoreLabels();
      ObjectsStored = true;
    }
    ClearXObjects();
  }
  return false;
}
//..............................................................................
void TGXApp::GetSelectedCAtoms(TCAtomPList& List, bool Clear)  {
  TXAtomPList xAtoms;
  GetSelectedXAtoms(xAtoms, Clear);
  List.SetCapacity( xAtoms.Count() );
  for( size_t i=0; i < xAtoms.Count(); i++ )
    List.Add(xAtoms[i]->Atom().CAtom());
}
//..............................................................................
void TGXApp::RestoreSelection()  {
  if( !SelectionCopy[0].IsEmpty() || SelectionCopy[1].IsEmpty() )
    return;
  SelectionCopy[0] = SelectionCopy[1];
  GetRender().SelectAll(false);
  RestoreGroup(GetSelection(), SelectionCopy[1]);
}
//..............................................................................
void TGXApp::GetSelectedXAtoms(TXAtomPList& List, bool Clear)  {
  TPtrList<TGlGroup> S;
  S.Add(GetSelection());
  for( size_t i=0; i < S.Count(); i++ )  {
    TGlGroup& Sel = *S[i];
    for( size_t j=0; j < Sel.Count(); j++ )  {
      AGDrawObject& GO = Sel[j];
      if( GO.IsDeleted() )  continue;
      if( GO.IsGroup() )  // another group
        S.Add((TGlGroup&)GO);  
      else if( EsdlInstanceOf(GO, TXAtom) )
        List.Add((TXAtom&)GO);
    }
  }
  if( Clear )  
    SelectAll(false);
}
//..............................................................................
void TGXApp::CAtomsByType(const cm_Element& AI, TCAtomPList& res)  {
  ListFilter::Filter(XFile().GetLattice().GetAsymmUnit().GetAtoms(), res,
    olx_alg::olx_and(
      olx_alg::olx_not(TCAtom::FlagsAnalyser<>(catom_flag_Deleted)),
      TCAtom::TypeAnalyser<>(AI)));
}
//..............................................................................
void TGXApp::XAtomsByType(const cm_Element& AI, TXAtomPList& res, bool FindHidden) {
  //ListFilter::Filter(XAtoms, res,
  //  olx_alg::and(
  //    olx_alg::Bool(!FindHidden),
  //    olx_alg::and(
  //      AGDrawObject::FlagsAnalyser(sgdoVisible),
  //      TSAtom::TypeAnalyser<TXAtom::AtomAccessor<> >(AI))));
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( !FindHidden && !XAtoms[i].IsVisible() )  continue;
    if( XAtoms[i].Atom().GetType() == AI )  {
      res.Add(XAtoms[i]);
    }
  }
}
//..............................................................................
void TGXApp::CAtomsByMask(const olxstr &StrMask, int Mask, TCAtomPList& List)  {
  bool found;
  if( StrMask.Length() > 32 )
    throw TInvalidArgumentException(__OlxSourceInfo, "mask is too long");
  olxstr Name(olxstr::UpperCase(StrMask));
  TAsymmUnit& AU= XFile().GetLattice().GetAsymmUnit();
  for( size_t i=0; i < AU.AtomCount(); i++ )  {
    TCAtom& CA = AU.GetAtom(i);
    if( CA.IsDeleted() )  continue;
    if( CA.GetLabel().Length() != Name.Length() )  continue;
    olxstr Tmp = CA.GetLabel().ToUpperCase();
    found = true;
    for( size_t j=0; j < Name.Length(); j++ )  {
      if( !(Mask & (0x0001<<j)) )  {
        if( Name.CharAt(j) != Tmp.CharAt(j) )  {
          found = false;
          break;
        }
      }
    }
    if( found )
      List.Add(CA);
  }
}
//..............................................................................
void TGXApp::GrowAtom(TXAtom *XA, bool Shell, TCAtomPList* Template)  {
  FXFile->GetLattice().GrowAtom(XA->Atom(), Shell, Template);
}
//..............................................................................
void TGXApp::Grow(const TXAtomPList& atoms, const smatd_list& matrices)  {
  TSAtomPList satoms(atoms, TXAtom::AtomAccessor<>());
  FXFile->GetLattice().GrowAtoms(satoms, matrices);
}
//..............................................................................
bool TGXApp::AtomExpandable(TXAtom *XA)  {
  return FXFile->GetLattice().IsExpandable(XA->Atom());
}
//..............................................................................
void TGXApp::GetXAtoms(const olxstr& AtomName, TXAtomPList& res)  {
  const size_t xac = XAtoms.Count();
  const short SelMask = sgdoVisible|sgdoDeleted;
  if( AtomName.StartsFrom("#c") )  {  // TCAtom.Id
    const size_t id = AtomName.SubStringFrom(2).ToSizeT();
    for( size_t i=0; i < xac; i++ )  {
      if( XAtoms[i].Atom().CAtom().GetId() == id )  
        if( XAtoms[i].MaskFlags(SelMask) == sgdoVisible )
          res.Add(XAtoms[i]);
    }
  }
  else if( AtomName.StartsFrom("#s") )  {  // SAtom.LatId
    const size_t id = AtomName.SubStringFrom(2).ToSizeT();
    for( size_t i=0; i < xac; i++ )  {
      if( XAtoms[i].Atom().GetLattId() == id )  { // only one is possible
        if( XAtoms[i].MaskFlags(SelMask) == sgdoVisible )  
          res.Add(XAtoms[i]);
        break;
      }
    }
  }
  else if( AtomName.StartsFrom("#x") )  {  // XAtom.XAppId
    const size_t id = AtomName.SubStringFrom(2).ToSizeT();
    if( id >= xac )
      throw TInvalidArgumentException(__OlxSourceInfo, "xatom id");
    TXAtom& xa = XAtoms[id];
    if( xa.MaskFlags(SelMask) == sgdoVisible )  
      res.Add(xa);
  }
  else  {
    for( size_t i=0; i < xac; i++ )  {
      if( XAtoms[i].Atom().GetLabel().Equalsi(AtomName) )  {
        if( XAtoms[i].MaskFlags(SelMask) == sgdoVisible )  
          res.Add(XAtoms[i]);
      }
    }
  }
}
//..............................................................................
void TGXApp::GetXBonds(const olxstr& BondName, TXBondPList& res)  {
  const size_t xbc = XBonds.Count();
  const short SelMask = sgdoVisible|sgdoDeleted;
  if( BondName.StartsFrom("#t") )  {  // SBond.LatId
    size_t id = BondName.SubStringFrom(2).ToSizeT();
    for( size_t i=0; i < xbc; i++ )  {
      if( XBonds[i].Bond().GetLattId() == id )  { // only one is possible
        if( XBonds[i].MaskFlags(SelMask) == sgdoVisible )  
          res.Add(XBonds[i]);
        break;
      }
    }
  }
  else if( BondName.StartsFrom("#y") )  {  // SBond.XAppId
    size_t id = BondName.SubStringFrom(2).ToSizeT();
    if( id >= xbc )
      throw TInvalidArgumentException(__OlxSourceInfo, "xatom id");
    TXBond& xb = XBonds[id];
    if( xb.MaskFlags(SelMask) == sgdoVisible )  
      res.Add(xb);
  }
  else  {
    for( size_t i=0; i < xbc; i++ )  {
      if( XBonds[i].GetCollectionName().Equalsi(BondName) )  {
        if( XBonds[i].MaskFlags(SelMask) == sgdoVisible )  
          res.Add(XBonds[i]);
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
    if( !XA.IsVisible() )  continue;
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
                XA.Atom().GetType() == XAFrom->Atom().GetType() )  List.Add( &XA );
          }
        }
        if( XATo != NULL )  List.Add( XATo );
      }
    }
    if( Tmp.CharAt(0) == '$' )  {
      Tmp = Tmp.SubStringFrom(1);
      if( !Tmp.IsEmpty() )  {
        cm_Element* elm = XElementLib::FindBySymbol(Tmp);
        if( elm == NULL )
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("atom type=") << Tmp);
        XAtomsByType(*elm, List, FindHidden);
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
    SB.Add(XA.Atom().Bond(i));
  SBonds2XBonds(SB, XB);
  for(size_t i=0; i < XB.Count(); i++ )  {
    TXBond* xb = XB[i];
    /* check if any of the atoms still a Q-peak */
    if( xb->Bond().A().GetType() == iQPeakZ || xb->Bond().B().GetType() == iQPeakZ )  continue;
    /* check that the covalent bond really exists before showing it */
    xb->SetVisible(FXFile->GetLattice().GetNetwork().CBondExistsQ(xb->Bond().A(),
                    xb->Bond().B(), xb->Bond().QLength()));
  }
}
//..............................................................................
TUndoData* TGXApp::ChangeSuffix(const TXAtomPList& xatoms, const olxstr &To, bool CheckLabels)  {
  TNameUndo *undo = new TNameUndo( new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoName)));
  olxstr oldL, newL;
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    oldL = xatoms[i]->Atom().GetLabel();
    newL = xatoms[i]->Atom().GetType().symbol;
    for( size_t j=xatoms[i]->Atom().GetType().symbol.Length(); j < oldL.Length(); j++ )
      if( oldL[j] >= '0' && oldL[j] <= '9' )
        newL << oldL[j];
      else
        break;
    newL << To;
    if( newL == oldL )  continue;

    if( CheckLabels )
      newL = XFile().GetAsymmUnit().CheckLabel(&xatoms[i]->Atom().CAtom(), newL);
    xatoms[i]->Atom().CAtom().SetLabel(newL, false);
    undo->AddAtom(xatoms[i]->Atom().CAtom(), oldL);
  }
  return undo;
}
//..............................................................................
TUndoData* TGXApp::Name(TXAtom& XA, const olxstr& Name, bool CheckLabel)  {
  bool checkBonds = (XA.Atom().GetType() == iQPeakZ);
  cm_Element* elm = XElementLib::FindBySymbolEx(Name);
  if( elm == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid element");
  TNameUndo *undo = new TNameUndo(new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoName)));
  olxstr oldL = XA.Atom().GetLabel();
  bool recreate = ((elm == NULL) ? true : XA.Atom().GetType() != *elm);
  XA.Atom().CAtom().SetLabel(
    CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA.Atom().CAtom(), Name) : Name, false);
  if( oldL != XA.Atom().GetLabel() || *elm != XA.Atom().GetType() )
    undo->AddAtom(XA.Atom().CAtom(), oldL);
  XA.Atom().CAtom().SetType(*elm);
  // Dima's complaint - leave all in for manual naming
  //NameHydrogens(XA.Atom(), undo, CheckLabel);
  if( checkBonds )  CheckQBonds(XA);
  if( recreate )  {
    XA.GetPrimitives().RemoveObject(XA);
    XA.Create();
    TXAtomPList atoms;
    atoms.Add(XA);
    SynchroniseBonds(atoms);
  }
  return undo;
}
//..............................................................................
TUndoData* TGXApp::Name(const olxstr &From, const olxstr &To, bool CheckLabel, bool ClearSelection)  {
  TXAtom* XA = GetXAtom(From, false);
  if( XA != NULL )  {
    if( ClearSelection ) SelectAll(false);
    return Name(*XA, To, CheckLabel);
  }
  else  {
    TNameUndo *undo = new TNameUndo(new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoName)));
    TXAtomPList Atoms, ChangedAtoms;
    FindXAtoms(From, Atoms, ClearSelection);
    // leave only AU atoms
    for( size_t i=0; i < Atoms.Count(); i++ )
      Atoms[i]->SetTag(Atoms[i]->Atom().IsAUAtom() ? 1 : 0);
    Atoms.Pack(ACollectionItem::TagAnalyser<>(0));
    if( From.Equalsi("sel") && To.IsNumber() )  {
      int j = To.ToInt();
      for( size_t i=0; i < Atoms.Count(); i++ )  {
        XA = Atoms[i];
        bool checkBonds = (XA->Atom().GetType() == iQPeakZ);
        const olxstr Tmp = XA->Atom().GetLabel();
        olxstr NL = XA->Atom().GetType().symbol;
        NL << j++;
        const olxstr oldL = XA->Atom().GetLabel();
        XA->Atom().CAtom().SetLabel(
          CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->Atom().CAtom(), NL) : NL, false);
        undo->AddAtom(XA->Atom().CAtom(), oldL);
        NameHydrogens(XA->Atom(), undo, CheckLabel);
        if( checkBonds )
          CheckQBonds(*XA);
      }
    }
    else  if( From.CharAt(0) == '$' )  {
      const cm_Element* elm = XElementLib::FindBySymbolEx(
        To.CharAt(0) == '$' ? To.SubStringFrom(1) : To);
      if( elm != NULL )  {  // change type
        for( size_t i=0; i < Atoms.Count(); i++ )  {
          XA = Atoms[i];
          const bool checkBonds = (XA->Atom().GetType() == iQPeakZ);
          const olxstr Tmp = XA->Atom().GetLabel();
          olxstr NL = elm->symbol;
          NL << Tmp.SubStringFrom(From.Length()-1);
          bool recreate = XA->Atom().GetType() != *elm;
          const olxstr oldL = XA->Atom().GetLabel();
          XA->Atom().CAtom().SetLabel(
            CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->Atom().CAtom(), NL) : NL, false);
          undo->AddAtom(XA->Atom().CAtom(), oldL);
          XA->Atom().CAtom().SetType(*elm);
          NameHydrogens(XA->Atom(), undo, CheckLabel);
          if( recreate )  {
            ChangedAtoms.Add(XA);
            if( checkBonds )
              CheckQBonds(*XA);
          }
        }
      }
      else if( To.IsNumber() ) {  // change number
        int j = To.ToInt();
        for( size_t i=0; i < Atoms.Count(); i++ )  {
          XA = Atoms[i];
          bool checkBonds = (XA->Atom().GetType() == iQPeakZ);
          const olxstr Tmp = XA->Atom().GetLabel();
          olxstr NL = XA->Atom().GetType().symbol;
          NL << j++;
          const olxstr oldL = XA->Atom().GetLabel();
          XA->Atom().CAtom().SetLabel(
            CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->Atom().CAtom(), NL) : NL, false);
          undo->AddAtom(XA->Atom().CAtom(), oldL);
          NameHydrogens(XA->Atom(), undo, CheckLabel);
          if( checkBonds )
            CheckQBonds(*XA);
        }
      }
      else
        throw TFunctionFailedException(__OlxSourceInfo, "wrong syntax");
    }
    else  {  // C2? to C3? ; Q? to Ni? ...
      const cm_Element* elm = XElementLib::FindBySymbolEx(To);
      if( elm == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "wrong syntax");
      const bool to_element = XElementLib::IsElement(To);
      for( size_t i=0; i < Atoms.Count(); i++ )  {
        XA = (TXAtom*)Atoms[i];
        const bool checkBonds = (XA->Atom().GetType() == iQPeakZ);
        const olxstr Tmp = XA->Atom().GetLabel();
        olxstr NL = To;
        const bool recreate = XA->Atom().GetType() != *elm;
        if( to_element )  {
          if( XA->Atom().GetLabel().StartsFrom(XA->Atom().GetType().symbol) )
            NL << XA->Atom().GetLabel().SubStringFrom(XA->Atom().GetType().symbol.Length());
        }
        else  {
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
        }
        const olxstr oldL = XA->Atom().GetLabel();
        XA->Atom().CAtom().SetLabel(
          CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->Atom().CAtom(), NL) : NL, false);
        undo->AddAtom(XA->Atom().CAtom(), oldL);
        XA->Atom().CAtom().SetType(*elm);
        NameHydrogens(XA->Atom(), undo, CheckLabel);
        if( recreate )  {
          ChangedAtoms.Add(XA);
          if( checkBonds )
            CheckQBonds(*XA);
        }
      }
    }
    for( size_t i=0; i < ChangedAtoms.Count(); i++ )  {
      XA = ChangedAtoms[i];
      XA->GetPrimitives().RemoveObject(*XA);
      XA->Create();
    }
    SynchroniseBonds(ChangedAtoms);
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
      Table[i][1] = A.GetType().symbol;
      Table[i][2] = olxstr::FormatFloat(3, A.ccrd()[0]);
      Table[i][3] = olxstr::FormatFloat(3, A.ccrd()[1]);
      Table[i][4] = olxstr::FormatFloat(3, A.ccrd()[2]);
      Table[i][5] = olxstr::FormatFloat(3, A.GetUiso());
      if( A.GetType() == iQPeakZ )
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
      const TSAtom& A = AtomsList[i]->Atom();
      Table[i][0] = A.GetGuiLabel();
      Table[i][1] = A.GetType().symbol;
      Table[i][2] = olxstr::FormatFloat(3, A.ccrd()[0]);
      Table[i][3] = olxstr::FormatFloat(3, A.ccrd()[1]);
      Table[i][4] = olxstr::FormatFloat(3, A.ccrd()[2]);
      Table[i][5] = olxstr::FormatFloat(3, A.CAtom().GetUiso());
      if( A.GetType() == iQPeakZ )
        Table[i][6] = olxstr::FormatFloat(3, A.CAtom().GetQPeak());
      else
        Table[i][6] = '-';
    }
    Table.CreateTXTList(Info, "Atom information", true, true, ' ');
  }
}
//..............................................................................
TXGlLabel& TGXApp::CreateLabel(const TXAtom& a, uint16_t FontIndex)  {
  TXGlLabel& l = CreateLabel(a.Atom().crd(), a.Atom().GetLabel(), FontIndex);
  l.TranslateBasis(vec3d(1, -1, 0));  // in pixels
  return l;
}
//..............................................................................
TXGlLabel& TGXApp::CreateLabel(const vec3d& center, const olxstr& T, uint16_t FontIndex)  {
  TXGlLabel& L = XLabels.Add(new TXGlLabel(*FGlRender, PLabelsCollectionName));
  L.SetFontIndex(FontIndex);
  L.SetLabel(T);
  L.SetOffset(center);
  L.Create();
  return L;
}
//..............................................................................
TXGlLabel *TGXApp::AddLabel(const olxstr& Name, const vec3d& center, const olxstr& T)  {
  TXGlLabel* gl = new TXGlLabel(*FGlRender, Name);
  gl->SetLabel(T);
  gl->SetOffset(center);
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
    SAtoms.Assign(*atoms, TXAtom::AtomAccessor<>());
  else
    SAtoms.Assign(XAtoms, TXAtom::AtomAccessor<>());
  if( SAtoms.Count() < 3 )  return NULL;
  return XFile().GetLattice().TmpPlane(SAtoms, weightExtent);
}
//..............................................................................
TXPlane *TGXApp::AddPlane(TXAtomPList &Atoms, bool regular, int weightExtent)  {
  if( Atoms.Count() < 3 )  return NULL;
  TSAtomPList SAtoms(Atoms, TXAtom::AtomAccessor<>());
  TSPlanePList planes = XFile().GetLattice().NewPlane(SAtoms, weightExtent, regular);
  TXPlane* rv = NULL;
  for( size_t i=0; i < planes.Count(); i++ )  {
    TXPlane& XP = XPlanes.Add(new TXPlane(*FGlRender, olxstr("TXPlane") << planes[i]->GetDefId(), planes[i]));
    XP.Create();
    if( rv == NULL )
      rv = &XP;
  }
  return rv;
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
  TSAtom *A = XFile().GetLattice().NewCentroid(SAtoms);
  XFile().GetRM().Conn.Disconnect(A->CAtom());
  if( A != NULL )  {
    TXAtom& XA = XAtoms.Add(new TXAtom(*FGlRender, EmptyString, *A));
    XA.Create();
    XA.SetXAppId(XAtoms.Count() - 1);
    XA.Params()[0] = A->GetType().r_pers;
    return &XA;
  }
  return NULL;
}
//..............................................................................
void TGXApp::AdoptAtoms(const TAsymmUnit& au, TXAtomPList& atoms, TXBondPList& bonds) {
  TLattice latt;
  latt.GetAsymmUnit().SetRefMod(au.GetRefMod());
  latt.GetAsymmUnit().Assign(au);
  latt.GetAsymmUnit()._UpdateConnInfo();
  latt.Init();
  vec3d cnt1, cnt2;
  double R1, R2;
  CalcLatticeRandCenter(XFile().GetLattice(), R1, cnt1);
  CalcLatticeRandCenter(latt, R2, cnt2);
  const vec3d right_shift = FGlRender->GetBasis().GetMatrix()*vec3d(1, 0, 0);
  const size_t ac = XFile().GetLattice().AtomCount();
  const size_t bc = XFile().GetLattice().BondCount();
  for( size_t i=0; i < latt.AtomCount(); i++ )  {
    latt.GetAtom(i).crd() = latt.GetAtom(i).crd()-cnt2+cnt1+right_shift*(R1+R2);
  }
  XFile().GetLattice().AddLatticeContent(latt);
  if( FLabels->IsVisible() )
    FLabels->Clear();
  for( size_t i=ac; i < XFile().GetLattice().AtomCount(); i++ )  {
    TSAtom& A = XFile().GetLattice().GetAtom(i);
    TXAtom& XA = XAtoms.Add(new TXAtom(*FGlRender, EmptyString, A));
    XA.Create();
    XA.SetXAppId(XAtoms.Count() - 1);
    XA.Params()[0] = A.GetType().r_pers;
    atoms.Add(XA);
  }
  for( size_t i=bc; i < XFile().GetLattice().BondCount(); i++ )  {
    TSBond& B = XFile().GetLattice().GetBond(i);
    TXBond& XB = XBonds.Add(new TXBond(*FGlRender, TXBond::GetLegend(B, 2), B));
    XB.Create();
    XB.SetXAppId(XBonds.Count() - 1);
    bonds.Add(XB);
  }
  if( FLabels->IsVisible() )
    FLabels->Init();
}
//..............................................................................
TXAtom* TGXApp::AddAtom(TXAtom* templ)  {
  vec3d center;
  if( templ != NULL )
    center = templ->Atom().CAtom().ccrd();
  TSAtom *A = XFile().GetLattice().NewAtom(center);
  if( A != NULL )  {
    olxstr colName;
    if( templ != NULL )  {
      colName = templ->GetCollectionName();
      A->CAtom().SetType(templ->Atom().GetType());
      if( templ->Atom().GetType() == iQPeakZ )
        A->CAtom().SetQPeak(1.0);
    }
    else
      A->CAtom().SetType(XElementLib::GetByIndex(iCarbonIndex));
    TXAtom& XA = XAtoms.Add(new TXAtom(*FGlRender, colName, *A));
    XA.Create();
    XA.SetXAppId(XAtoms.Count() - 1);
    XA.Params()[0] = A->GetType().r_pers;
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
  UpdateConnectivity();
}
//..............................................................................
void TGXApp::undoName(TUndoData *data)  {
  TNameUndo *undo = dynamic_cast<TNameUndo*>(data);
  const TAsymmUnit& au = XFile().GetAsymmUnit();
  bool recreate = false;
  for( size_t i=0; i < undo->AtomCount(); i++ )  {
    if( undo->GetCAtomId(i) >= au.AtomCount() )  //could happen?
      continue;
    const TCAtom& ca = au.GetAtom(undo->GetCAtomId(i));
    if( ca.GetType() != undo->GetElement(i) )  {
      recreate = true;
      break;
    }
  }
  // could be optimised...
  TXApp::undoName(data);
  if( recreate )
    CreateObjects(false, false);
}
//..............................................................................
void TGXApp::undoHide(TUndoData *data)  {
  THideUndo *undo = dynamic_cast<THideUndo*>(data);
  for( size_t i=0; i < undo->Objects.Count(); i++ )
    undo->Objects[i]->SetVisible(true);
}
//..............................................................................
TUndoData* TGXApp::DeleteXObjects(AGDObjList& L)  {
  TXAtomPList atoms;
  atoms.SetCapacity(L.Count());
  bool planes_deleted = false;
  for( size_t i=0; i < L.Count(); i++ )  {
    if( EsdlInstanceOf(*L[i], TXAtom) )  
      atoms.Add( (TXAtom*)L[i] );
    else if( EsdlInstanceOf(*L[i], TXPlane) )  {
      ((TXPlane*)L[i])->SetDeleted(true);
      planes_deleted = true;
    }
    else if( EsdlInstanceOf(*L[i], TXBond) )  {
      TXBond* xb = (TXBond*)L[i];
      xb->SetVisible(false);
    }
    else
      L[i]->SetDeleted(true);
  }
  if( planes_deleted )
    XFile().GetLattice().UpdatePlaneDefinitions();
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
      if( SA.GetType().GetMr() > 3.5 && SH.GetType() == iHydrogenZ )  
        SAL.Add(SH)->SetDeleted(true);
    }
    XA->SetDeleted(true);
  }

  TXAtomPList XAL;
  SAtoms2XAtoms(SAL, XAL);
  for( size_t i=0; i < XAL.Count(); i++ )  {  
    XAL[i]->SetDeleted(true);
    undo->AddSAtom(XAL[i]->Atom());
  }

  GetSelection().Clear();
  UpdateConnectivity();
  //CenterView();
  return undo;
}
//..............................................................................
void TGXApp::SelectBondsWhere(const olxstr &Where, bool Invert)  {
  olxstr str = olxstr::LowerCase(Where);
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
        if( atoms[i]->IsConnectedTo(*ring->GetLast()) )  {
          ring->Add(atoms[i]);
          atoms[i]->SetTag(1);
          change = true;
        }
      }
    }
  }
  return true;
}

void TGXApp::FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings)  {
  ElementPList ring;
  if( Condition.Equalsi("sel") )  {
    TXAtomPList L;
    GetSelectedXAtoms(L, false);
    TSAtomPList SAtoms(L, TXAtom::AtomAccessor<>());
    SAtoms.ForEach(ACollectionItem::TagSetter<>(0));
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
      if( atoms[i]->Node(j).GetType() != iHydrogenZ )
        bc++;
    if( bc > maxbc )  {
      maxbc = bc;
      ind = i;
    }
  }
  if( ind != InvalidIndex && ind > 0 )
    atoms.ShiftL(ind);
}

void TGXApp::SelectRings(const olxstr& Condition, bool Invert)  {
  TTypeList< TSAtomPList > rings;
  try  {  FindRings(Condition, rings);  }
  catch( const TExceptionBase& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }

  if( rings.IsEmpty() )  return;

  TXAtomPList XA(rings.Count()*rings[0].Count());
  TSAtomPList allSAtoms;
  allSAtoms.SetCapacity(XA.Count());
  for( size_t i=0; i < rings.Count(); i++ )  {
    SortRing(rings[i]);
    for( size_t j=0; j < rings[i].Count(); j++ )
      allSAtoms.Add(rings[i][j]);
  }
  XFile().GetLattice().GetAtoms().ForEach(ACollectionItem::TagSetter<>(-1));
  allSAtoms.ForEach(ACollectionItem::IndexTagSetter<>());
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetTag() != -1 )
      XA[XAtoms[i].Atom().GetTag()] = &XAtoms[i];
  }
  XA.Pack();
  XA.ForEach(ACollectionItem::IndexTagSetter<>());
  for( size_t i=0; i < XA.Count(); i++ )
    if( XA[i]->GetTag() == i && XA[i]->IsVisible() )
      FGlRender->Select(*XA[i]);
}
//..............................................................................
void TGXApp::SelectAtoms(const olxstr &Names, bool Invert)  {
  TXAtomPList Sel;
  FindXAtoms(Names, Sel, true);
  for( size_t i=0; i < Sel.Count(); i++ )  {
    if( Invert )
      GetRender().Select(*Sel[i]);
    else
      if( !Sel[i]->IsSelected() )  
        GetRender().Select(*Sel[i]);
  }
}
//..............................................................................
void TGXApp::ExpandSelection(TCAtomGroup& atoms)  {
  TXAtomPList xatoms;
  GetSelectedXAtoms(xatoms, GetDoClearSelection());
  for( size_t i=0; i < xatoms.Count(); i++ )
    atoms.AddNew(&xatoms[i]->Atom().CAtom(), &xatoms[i]->Atom().GetMatrix(0));
}
//..............................................................................
void TGXApp::ExpandSelectionEx(TSAtomPList& atoms)  {
  TXAtomPList xatoms;
  GetSelectedXAtoms(xatoms, GetDoClearSelection());
  atoms.SetCapacity(atoms.Count()+xatoms.Count());
  for( size_t i=0; i < xatoms.Count(); i++ )
    atoms.Add(xatoms[i]->Atom());
}
//..............................................................................
void TGXApp::FindCAtoms(const olxstr &Atoms, TCAtomPList& List, bool ClearSelection)  {
  if( Atoms.IsEmpty() )  {
    GetSelectedCAtoms(List, ClearSelection);
    if( !List.IsEmpty() )  return;
    TAsymmUnit& AU = XFile().GetLattice().GetAsymmUnit();
    List.SetCapacity(List.Count() + AU.AtomCount());
    for( size_t i=0; i < AU.ResidueCount(); i++ )  {
      TResidue& resi = AU.GetResidue(i);
      for( size_t j=0; j < resi.Count(); j++ )  {
        if( !resi[j].IsDeleted() )
          List.Add(resi[j]);
      }
    }
    return;
  }
  TStrList Toks(Atoms, ' ');
  olxstr Tmp;
  for( size_t i = 0; i < Toks.Count(); i++ )  {
    Tmp = Toks[i];
    if( Tmp.Equalsi("sel") )  {
      GetSelectedCAtoms(List, ClearSelection);
      continue;
    }
    if( Tmp.CharAt(0) == '$' )  {
      Tmp = Tmp.SubStringFrom(1);
      if( Tmp.Length() != 0 )  {
        cm_Element* elm = XElementLib::FindBySymbol(Tmp);
        if( elm == NULL )
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("atom type=") << Tmp);
        CAtomsByType(*elm, List);
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
    if( A != NULL && !A->IsDeleted() )
      List.Add(A);
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
  // check just in case...
  for( size_t i=0; i < XLabels.Count(); i++ )
    if( XLabels[i].IsSelected() )
      XLabels[i].SetSelected(false);
  XLabels.Clear();
}
//..............................................................................
void TGXApp::SBonds2XBonds(TSBondPList& L, TXBondPList& Res)  {
  if( L.IsEmpty() )  return;
  TLattice& latt = L[0]->GetNetwork().GetLattice();
  latt.GetBonds().ForEach(ACollectionItem::TagSetter<>(0));
  L.ForEach(ACollectionItem::TagSetter<>(1));
  Res.SetCapacity(Res.Count() + L.Count());
  for( size_t i=0; i < XBonds.Count(); i++ )
    if( &XBonds[i].Bond().GetNetwork().GetLattice() == &latt && XBonds[i].Bond().GetTag() != 0 )
      Res.Add(XBonds[i]);
}
//..............................................................................
void TGXApp::SPlanes2XPlanes(TSPlanePList& L, TXPlanePList& Res)  {
  if( L.IsEmpty() )  return;
  TLattice& latt = L[0]->GetNetwork().GetLattice();
  latt.GetPlanes().ForEach(ACollectionItem::TagSetter<>(0));
  L.ForEach(ACollectionItem::TagSetter<>(1));
  Res.SetCapacity(Res.Count() + L.Count());
  for( size_t i=0; i < XPlanes.Count(); i++ )
    if( XPlanes[i].Plane().GetNetwork().GetLattice() == &latt && XPlanes[i].Plane().GetTag() != 0 )
      Res.Add(XPlanes[i]);
}
//..............................................................................
void TGXApp::SAtoms2XAtoms(TSAtomPList& L, TXAtomPList& Res)  {
  if( L.IsEmpty() )  return;
  TLattice& latt = L[0]->GetNetwork().GetLattice();
  latt.GetAtoms().ForEach(ACollectionItem::TagSetter<>(0));
  L.ForEach(ACollectionItem::TagSetter<>(1));
  Res.SetCapacity(Res.Count() + L.Count());
  for( size_t i=0; i < XAtoms.Count(); i++ )
    if( XAtoms[i].Atom().GetNetwork().GetLattice() == latt && XAtoms[i].Atom().GetTag() != 0 )
      Res.Add(XAtoms[i]);
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
    sbonds.ForEach(ACollectionItem::IndexTagSetter<>());
    sbonds.Pack(ACollectionItem::IndexTagAnalyser<>());
    if( !sbonds.IsEmpty() )  {
      SBonds2XBonds(sbonds, List);
      List.ForEach(ACollectionItem::IndexTagSetter<>());
      List.Pack(ACollectionItem::IndexTagAnalyser<>());
    }
    return;
  }
  TGPCollection *GPC = GetRender().FindCollection(Bonds);
  if( GPC == NULL )  return;
  for( size_t i=0; i < GPC->ObjectCount(); i++ )  {
    if( i == 0 )  {  // check if the right type !
      if( !EsdlInstanceOf(GPC->GetObject(0), TXBond) )  
        return;
    }
    List.Add((TXBond&)GPC->GetObject(i));
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
  else if( Rad.Equalsi("vdw") ) {
    DS = darVdW;
    if( Atoms == NULL )
      TXAtom::DefRad(darVdW);
  }
  if( DS == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "rad");

  if( Atoms != NULL )  {  // make sure all atoms of selected collections are updated
    Atoms->ForEach(ACollectionItem::IndexTagSetter<AGDrawObject::PrimitivesAccessor>());
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
void TGXApp::GetGPCollections(AGDObjList& GDObjects, TPtrList<TGPCollection>& Result)  {
  GDObjects.ForEach(ACollectionItem::IndexTagSetter<AGDrawObject::PrimitivesAccessor>());
  for( size_t i=0; i < GDObjects.Count(); i++ )  {
    if( GDObjects[i]->GetPrimitives().GetTag() == i )
      Result.Add(GDObjects[i]->GetPrimitives());
  }
}
//..............................................................................
void TGXApp::FillXAtomList(TXAtomPList& res, TXAtomPList* providedAtoms) {
  if( providedAtoms != NULL )
    res.AddList(*providedAtoms);
  else
    res.AddList(XAtoms);
}
//..............................................................................
void TGXApp::FillXBondList( TXBondPList& res, TXBondPList* providedBonds)  {
  if( providedBonds != NULL )
    res.AddList(*providedBonds);
  else
    res.AddList(XBonds);
}
//..............................................................................
void TGXApp::AtomZoom(float Zoom, TXAtomPList* Atoms)  {  // takes %
  AGDObjList objects;
  if( Atoms != NULL )  {
    objects.SetCapacity(Atoms->Count());
    for( size_t i=0; i < Atoms->Count(); i++ )
      objects.Add((AGDrawObject*)Atoms->GetItem(i));
  }
  else  {
    objects.SetCapacity(XAtoms.Count());
    for( size_t i=0; i < XAtoms.Count(); i++ )
      objects.Add((AGDrawObject*)&XAtoms[i]);
  }
  TPtrList<TGPCollection> Colls;
  GetGPCollections(objects, Colls);
  for( size_t i=0; i < Colls.Count(); i++ )  {
    if( Colls[i]->ObjectCount() != 0 )
      ((TXAtom&)Colls[i]->GetObject(0)).SetZoom(Zoom/100);
  }
}
//..............................................................................
void TGXApp::SetQPeakScale(float V)  {
  TXAtom::SetQPeakScale(V);
  if( XAtoms.IsEmpty() )  return;
  TPtrList<TGPCollection> Colls;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetType() == iQPeakZ )
      Colls.Add(XAtoms[i].GetPrimitives());
  }
  FGlRender->RemoveCollections(Colls);
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetType() == iQPeakZ )
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
    if( XAtoms[i].Atom().GetType() == iQPeakZ )
      Colls.Add(XAtoms[i].GetPrimitives());
  }
  FGlRender->RemoveCollections(Colls);
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetType() == iQPeakZ )
      XAtoms[i].Create();
  }

}
//..............................................................................
float TGXApp::GetQPeakSizeScale()  {
  return TXAtom::GetQPeakSizeScale();
}
//..............................................................................
void TGXApp::BondRad(float R, TXBondPList* Bonds)  {
  AGDObjList objects;
  if( Bonds != NULL )
    objects.Assign(*Bonds, CastAccessor<AGDrawObject*>());
  else
    objects.Assign(XBonds, CastAccessor<AGDrawObject>());

  TPtrList<TGPCollection> Colls;
  GetGPCollections(objects, Colls);
  for( size_t i=0; i < Colls.Count(); i++ )  {
    if( Colls[i]->ObjectCount() != 0 ) 
      ((TXBond&)Colls[i]->GetObject(0)).SetRadius(R);
  }
}
//..............................................................................
void TGXApp::UpdateAtomPrimitives(int Mask, TXAtomPList* Atoms) {
  TXAtomPList atoms;
  FillXAtomList(atoms, Atoms);
  atoms.ForEach(ACollectionItem::IndexTagSetter<AGDrawObject::PrimitivesAccessor>());
  for( size_t i=0; i < atoms.Count(); i++ )
    if( atoms[i]->GetPrimitives().GetTag() == i )
      atoms[i]->UpdatePrimitives(Mask);
  if( Atoms == NULL )  {
    TXAtom::DefMask(Mask);
    //for( size_t i=0; i < IndividualCollections.Count(); i++ )
    //  if( IndividualCollections[i].IndexOf('-') == InvalidIndex )
    //    IndividualCollections[i].SetLength(0);
    IndividualCollections.Pack();
  }
}
//..............................................................................
void TGXApp::UpdateBondPrimitives(int Mask, TXBondPList* Bonds, bool HBondsOnly)  {
  TXBondPList bonds;
  FillXBondList(bonds, Bonds);
  XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
  bonds.ForEach(ACollectionItem::IndexTagSetter<AGDrawObject::PrimitivesAccessor>());
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
    //for( size_t i=0; i < IndividualCollections.Count(); i++ )
    //  if( IndividualCollections[i].IndexOf('-') != InvalidIndex )
    //    IndividualCollections[i].SetLength(0);
    //IndividualCollections.Pack();
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
  XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
  XBonds.ForEach(ACollectionItem::IndexTagSetter<AGDrawObject::PrimitivesAccessor>());
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
        int dds = (int)GlP.Params.GetLast();
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
}
//..............................................................................
void TGXApp::GrowAtoms(const olxstr& AtomsStr, bool Shell, TCAtomPList* Template)  {
  TXAtomPList xatoms;
  FindXAtoms(AtomsStr, xatoms, true);
  TSAtomPList satoms(xatoms, TXAtom::AtomAccessor<>());
  FXFile->GetLattice().GrowAtoms(satoms, Shell, Template);
}
//..............................................................................
void TGXApp::RestoreGroup(TGlGroup& glg, const GroupData& gd)  {
  const AtomRegistry& ar = XFile().GetLattice().GetAtomRegistry();
  glg.SetVisible(gd.visible);
  if( gd.parent_id != -2 )
    (gd.parent_id == -1 ? FGlRender->GetSelection() : FGlRender->GetGroup(gd.parent_id)).Add(glg);
  TSAtomPList atoms(gd.atoms.Count());
  TSBondPList bonds(gd.bonds.Count());
  for( size_t j=0; j < gd.atoms.Count(); j++ )
    atoms[j] = ar.Find(gd.atoms[j]);
  for( size_t j=0; j < gd.bonds.Count(); j++ )
    bonds[j] = ar.Find(gd.bonds[j]);
  atoms.Pack();
  bonds.Pack();
  if( atoms.IsEmpty() && bonds.IsEmpty() )  return;
  TXAtomPList xatoms;
  TXBondPList xbonds;
  SAtoms2XAtoms(atoms, xatoms);
  SBonds2XBonds(bonds, xbonds);
  for( size_t j=0; j < xatoms.Count(); j++ )  {
    if( xatoms[j]->IsVisible() )
      glg.Add(*xatoms[j], false);
  }
  for( size_t j=0; j < xbonds.Count(); j++ )  {
    if( xbonds[j]->IsVisible() )
      glg.Add(*xbonds[j], false);
  }
}
//..............................................................................
void TGXApp::StoreGroup(const TGlGroup& glG, GroupData& gd)  {
  gd.collectionName = glG.GetCollectionName();  //planes
  gd.visible = glG.IsVisible();
  gd.parent_id = (glG.GetParentGroup() != NULL ? glG.GetParentGroup()->GetTag() : -2); 
  for( size_t j=0; j < glG.Count(); j++ )  {
    AGDrawObject& glO = glG[j];
    if( EsdlInstanceOf(glO, TXAtom) )
      gd.atoms.AddCCopy(((TXAtom&)glO).Atom().GetRef());
    if( EsdlInstanceOf(glO, TXBond) )
      gd.bonds.AddCCopy(((TXBond&)glO).Bond().GetRef());
  }
}
//..............................................................................
void TGXApp::StoreLabels()  {
  LabelInfo.Clear();
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].GetLabel().IsVisible() )  {
      LabelInfo.atoms.AddCCopy(XAtoms[i].Atom().GetRef());
      LabelInfo.labels.AddCCopy(XAtoms[i].GetLabel().GetLabel());
      LabelInfo.centers.AddCCopy(XAtoms[i].GetLabel().GetCenter());
    }
  }
  for( size_t i=0; i < XBonds.Count(); i++ )  {
    if( XBonds[i].GetLabel().IsVisible() )  {
      LabelInfo.bonds.AddCCopy(XBonds[i].Bond().GetRef());
      LabelInfo.labels.AddCCopy(XBonds[i].GetLabel().GetLabel());
      LabelInfo.centers.AddCCopy(XBonds[i].GetLabel().GetCenter());
    }
  }
}
//..............................................................................
void TGXApp::RestoreLabels()  {
  const AtomRegistry& ar = XFile().GetLattice().GetAtomRegistry();
  TSAtomPList atoms(LabelInfo.atoms.Count());
  TSBondPList bonds(LabelInfo.bonds.Count());
  TSizeList labels(atoms.Count()+bonds.Count());
  size_t li=0;
  for( size_t i=0; i < LabelInfo.atoms.Count(); i++ )  {
    atoms[i] = ar.Find(LabelInfo.atoms[i]);
    if( atoms[i] != NULL )
      labels[li++] = i;
  }
  for( size_t i=0; i < LabelInfo.bonds.Count(); i++ )  {
    bonds[i] = ar.Find(LabelInfo.bonds[i]);
    if( bonds[i] != NULL )
      labels[li++] = atoms.Count()+i;
  }
  bonds.Pack();
  atoms.Pack();
  TXAtomPList xatoms;
  TXBondPList xbonds;
  SAtoms2XAtoms(atoms, xatoms);
  SBonds2XBonds(bonds, xbonds);
  for( size_t j=0; j < xatoms.Count(); j++ )  {
    xatoms[j]->GetLabel().SetVisible(true);
    xatoms[j]->GetLabel().SetLabel(LabelInfo.labels[labels[j]]);
    xatoms[j]->GetLabel().SetOffset(xatoms[j]->Atom().crd());
    xatoms[j]->GetLabel().TranslateBasis(
      LabelInfo.centers[labels[j]]-xatoms[j]->GetLabel().GetCenter());
  }
  for( size_t j=0; j < xbonds.Count(); j++ )  {
    xbonds[j]->GetLabel().SetVisible(true);
    xbonds[j]->GetLabel().SetLabel(LabelInfo.labels[labels[xatoms.Count()+j]]);
    xbonds[j]->GetLabel().SetOffset(xbonds[j]->Bond().GetCenter());
    xbonds[j]->GetLabel().TranslateBasis(
      LabelInfo.centers[labels[xatoms.Count()+j]]-xbonds[j]->GetLabel().GetCenter());
  }
}
//..............................................................................
void TGXApp::RestoreGroups()  {
  if( !SelectionCopy[0].IsEmpty() )
    RestoreGroup(GetSelection(), SelectionCopy[0]);

  GroupDict.Clear();
  for( size_t i=0; i < GroupDefs.Count(); i++ )
    FGlRender->NewGroup(GroupDefs[i].collectionName).Create(GroupDefs[i].collectionName);
  for( size_t i=0; i < GroupDefs.Count(); i++ )  {
    TGlGroup& glg = FGlRender->GetGroup(i);
    RestoreGroup(glg, GroupDefs[i]);
    GroupDict(&glg, i);
  }
  RestoreLabels();
}
//..............................................................................
void TGXApp::StoreVisibility()  {
  FVisibility.SetSize((XAtoms.Count() + XBonds.Count() + XPlanes.Count()));
  for( size_t i=0; i < XAtoms.Count(); i++ )
    FVisibility.Set(i, XAtoms[i].IsVisible());
  size_t inc = XAtoms.Count();
  for( size_t i=0; i < XBonds.Count(); i++ )
    FVisibility.Set(inc+i, XBonds[i].IsVisible());
  inc += XBonds.Count();
  for( size_t i=0; i < XPlanes.Count(); i++ )
    FVisibility.Set(inc+i, XPlanes[i].IsVisible());
  StoreLabels();
}
void TGXApp::RestoreVisibility()  {
  for( size_t i=0; i < XAtoms.Count(); i++ )
    XAtoms[i].SetVisible(FVisibility.Get(i));
  size_t inc = XAtoms.Count();
  for( size_t i=0; i < XBonds.Count(); i++ )
    XBonds[i].SetVisible(FVisibility.Get(inc+i));
  inc += XBonds.Count();
  for( size_t i=0; i < XPlanes.Count(); i++ )
      XPlanes[i].SetVisible(FVisibility.Get(inc+i));
  RestoreLabels();
}
//..............................................................................
void TGXApp::BeginDrawBitmap(double resolution)  {
  FPictureResolution = resolution;
  FLabels->Clear();
  GetRender().GetScene().ScaleFonts(resolution);
  StoreVisibility();
  CreateObjects(false, false);
  FXGrid->GlContextChange();
  RestoreVisibility();
}
//..............................................................................
void TGXApp::FinishDrawBitmap()  {
  FLabels->Clear();
  GetRender().GetScene().RestoreFontScale();
  CreateObjects(false, false);
  FXGrid->GlContextChange();
  RestoreVisibility();
  FVisibility.Clear();
}
//..............................................................................
void TGXApp::UpdateLabels()  {
  for( size_t i=0; i < XLabels.Count(); i++ )
    XLabels[i].SetLabel(XLabels[i].GetLabel());
  for( size_t i=0; i < XAtoms.Count(); i++ )
    XAtoms[i].UpdateLabel();
  for( size_t i=0; i < XBonds.Count(); i++ )
    XBonds[i].UpdateLabel();
  for( size_t i=0; i < LooseObjects.Count(); i++ )
    LooseObjects[i]->UpdateLabel();
}
//..............................................................................
uint64_t TGXApp::Draw()  {
  if( !IsMainFormVisible() )  return 0;
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
void TGXApp::MoveToCenter()  {  FXFile->GetLattice().MoveToCenter();  }
//..............................................................................
void TGXApp::Compaq(bool All)  {
  if( All )
    FXFile->GetLattice().CompaqAll();
  else
    FXFile->GetLattice().Compaq();
}
//..............................................................................
void TGXApp::SetHBondsVisible(bool v)  {
  FHBondsVisible = v;
  if( !v )  {
    for( size_t i=0; i < XBonds.Count(); i++ )  {
      if( XBonds[i].Bond().GetType() == sotHBond )
        XBonds[i].SetVisible(false);
    }
  }
  else  {
    XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
    for( size_t i=0; i < XBonds.Count(); i++ )  {
      const TSBond& b = XBonds[i].Bond(); 
      if( b.GetType() == sotHBond )  {
        XBonds[i].SetVisible(XAtoms[b.A().GetTag()].IsVisible() && XAtoms[b.B().GetTag()].IsVisible());
      }
    }
    RestoreGroups();
  }
}
//..............................................................................
void TGXApp::SetHydrogensVisible(bool v)  {
  if( FHydrogensVisible != v )  {
    FHydrogensVisible = v;
    XFile().GetAsymmUnit().DetachAtomType(iHydrogenZ, !FHydrogensVisible);
    for( size_t i = 0; i < OverlayedXFiles.Count(); i++ )
      OverlayedXFiles[i].GetAsymmUnit().DetachAtomType(iHydrogenZ, !FHydrogensVisible);
    if( v && !XFile().GetLattice().IsGenerated() )
      XFile().GetLattice().CompaqH();
    else
      UpdateConnectivity();
    CenterView(true);
  }
}
//..............................................................................
void TGXApp::UpdateConnectivity()  {
  for( size_t i = 0; i < OverlayedXFiles.Count(); i++ )
    OverlayedXFiles[i].GetLattice().UpdateConnectivity();
  XFile().GetLattice().UpdateConnectivity();
}
//..............................................................................
void TGXApp::SetQPeaksVisible(bool v)  {
  if( FQPeaksVisible != v )  {
    FQPeaksVisible = v;
    XFile().GetAsymmUnit().DetachAtomType(iQPeakZ, !FQPeaksVisible);
    for( size_t i = 0; i < OverlayedXFiles.Count(); i++ )
      OverlayedXFiles[i].GetAsymmUnit().DetachAtomType(iQPeakZ, !FQPeaksVisible);
    if( v && !XFile().GetLattice().IsGenerated() )
      XFile().GetLattice().CompaqQ();
    else
      UpdateConnectivity();
    CenterView(true);
  }
}
//..............................................................................
void TGXApp::SyncQVisibility()  {
  if( !FQPeakBondsVisible )  return;
  XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
  const size_t bc = XBonds.Count();
  for( size_t i=0; i < bc; i++ )  {
    const TSBond& b = XBonds[i].Bond();
    if( b.A().GetType() == iQPeakZ || b.B().GetType() == iQPeakZ )
      XBonds[i].SetVisible(XAtoms[b.A().GetTag()].IsVisible() && XAtoms[b.B().GetTag()].IsVisible());
  }
  if( FXGrowLinesVisible )  {
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      if( XGrowLines[i].SAtom()->GetType() == iQPeakZ )
        XGrowLines[i].SetVisible(XAtoms[XGrowLines[i].SAtom()->GetTag()].IsVisible());
    }
  }
}
//..............................................................................
void TGXApp::SetQPeakBondsVisible(bool v)  {
  FQPeakBondsVisible = v;
  if( !v )  {
    for( size_t i=0; i < XBonds.Count(); i++ )  {
      if( (XBonds[i].Bond().A().GetType() == iQPeakZ) ||
        (XBonds[i].Bond().B().GetType() == iQPeakZ)  )
        XBonds[i].SetVisible(v);
    }
    if( FXGrowLinesVisible )  {
      for( size_t i=0; i < XGrowLines.Count(); i++ )  {
        if( XGrowLines[i].SAtom()->GetType() == iQPeakZ ||
          XGrowLines[i].CAtom()->GetType() == iQPeakZ )
          XGrowLines[i].SetVisible(v);
      }
    }
  }
  else  {
    XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
    for( size_t i=0; i < XBonds.Count(); i++ )  {
      const TSBond& b = XBonds[i].Bond();
      if( b.A().GetType() == iQPeakZ || b.B().GetType() == iQPeakZ )
        XBonds[i].SetVisible(XAtoms[b.A().GetTag()].IsVisible() && XAtoms[b.B().GetTag()].IsVisible());
    }
    RestoreGroups();
  }
}
//..............................................................................
void TGXApp::_maskInvisible()  {
  const TAsymmUnit& au = XFile().GetAsymmUnit();
  TEBitArray vis(au.AtomCount());
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    const TXAtom& xa = XAtoms[i];
    if( !XAtoms[i].IsVisible() )  continue;
    vis.SetTrue(xa.Atom().CAtom().GetId());
  }
  for( size_t i=0; i < vis.Count(); i++ )
    au.GetAtom(i).SetMasked(!vis[i]);
}
//..............................................................................
void TGXApp::_syncBondsVisibility()  {
  for( size_t i=0; i < XBonds.Count(); i++ )  {
    const TSBond& b = XBonds[i].Bond();
    if( !FQPeakBondsVisible && (b.A().GetType() == iQPeakZ || b.B().GetType() == iQPeakZ) )
      XBonds[i].SetVisible(false);
    else if( !FHBondsVisible && b.GetType() == sotHBond )
      XBonds[i].SetVisible(false);
    else
      XBonds[i].SetVisible(XAtoms[XBonds[i].Bond().A().GetTag()].IsVisible() && XAtoms[XBonds[i].Bond().B().GetTag()].IsVisible());
  }
  if( FXGrowLinesVisible )  {
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      if( XGrowLines[i].SAtom()->GetType() == iQPeakZ )
        XGrowLines[i].SetVisible(XAtoms[XGrowLines[i].SAtom()->GetTag()].IsVisible());
    }
  }
}
//..............................................................................
void TGXApp::SetStructureVisible(bool v)  {
  FStructureVisible = v;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( !v )
      XAtoms[i].SetVisible(v);
    else
      XAtoms[i].SetVisible(XAtoms[i].Atom().IsAvailable() && XAtoms[i].Atom().CAtom().IsAvailable());  
    XAtoms[i].Atom().SetTag(i);
  }
  for( size_t i=0; i < LooseObjects.Count(); i++ )  LooseObjects[i]->SetVisible(v);
  for( size_t i=0; i < XPlanes.Count(); i++ )       XPlanes[i].SetVisible(v);
  for( size_t i=0; i < XLabels.Count(); i++ )       XLabels[i].SetVisible(v);
  if( v )  {
    if( !FXGrid->IsEmpty() )
      FXGrid->SetVisible(true);
  } 
  else
    FXGrid->SetVisible(false);
  _syncBondsVisibility();
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
    AllVisible(true);
    return;
  }
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( parts.IndexOf(XAtoms[i].Atom().CAtom().GetPart()) != InvalidIndex )
      XAtoms[i].SetVisible(show);
    else
      XAtoms[i].SetVisible(!show);
    if( XAtoms[i].IsVisible() )
      XAtoms[i].SetVisible(XAtoms[i].Atom().IsAvailable());
  }
  _maskInvisible();
  UpdateConnectivity();
}
//..............................................................................
void TGXApp::SetHklVisible(bool v)  {
  if( v )  {
    // default if could not load the hkl ...
    FDUnitCell->SetReciprocal(false);
    FHklVisible = false;
    CreateXRefs();
  }
  for( size_t i=0; i < XReflections.Count(); i++ )  
    XReflections[i].SetVisible(v);
  FHklVisible = v;
  FDUnitCell->SetReciprocal(v);
}
//..............................................................................
void TGXApp::SetGridDepth(const vec3d& crd)  {  FXGrid->SetDepth(crd);  }
//..............................................................................
bool TGXApp::IsGridVisible() const {  return FXGrid->IsVisible();  }
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
void TGXApp::Individualise(const TXAtomPList& atoms, short _level, int32_t mask)  {
  if( atoms.IsEmpty() || _level > 2 )  return;
  TSBondPList sbonds;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    TXAtom& a = *atoms[i];
    const int cl = TXAtom::LegendLevel(a.GetPrimitives().GetName());
    if( cl >= 2 )  {
      if( mask >= 0 )
        a.UpdatePrimitives(mask);
    }
    else if( (_level != -1 && cl > _level) )
      continue;
    else  {
      const int legend_level = _level == -1 ? cl+1 : _level;
      const olxstr leg = a.GetLegend(a.Atom(), legend_level);
      TGPCollection* indCol = FGlRender->FindCollection(leg);
      if( &a.GetPrimitives() == indCol )  {
        if( mask >= 0 )
          a.UpdatePrimitives(mask);
      }
      else  {
        if( indCol == NULL )  {
          indCol = &FGlRender->NewCollection(leg);
          IndividualCollections.Add(leg);
        }
        TPtrList<AGDrawObject> objects;
        for( size_t oi=0; oi < a.GetPrimitives().ObjectCount(); oi++ )  {
          TXAtom* _xa = dynamic_cast<TXAtom*>(&a.GetPrimitives().GetObject(oi));
          if( _xa != NULL && TXAtom::GetLegend(_xa->Atom(), legend_level) == leg )
            objects.Add(_xa);
        }
        a.GetPrimitives().RemoveObjects(objects);
        for( size_t oi=0; oi < objects.Count(); oi++ )  {
          TXAtom* _xa = (TXAtom*)objects[oi];
          _xa->Create(leg);
          sbonds.AddList(_xa->Atom().GetBonds());
        }
        if( mask >= 0 )
          a.UpdatePrimitives(mask);
      }
    }
  }
  TXBondPList xbonds;
  SBonds2XBonds(sbonds, xbonds);
  XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
  for( size_t i=0; i < xbonds.Count(); i++ )  {
    int cl = TXAtom::LegendLevel(xbonds[i]->GetPrimitives().GetName())+1;
    if( cl > 2 )  continue;
    TSBond& bond = xbonds[i]->Bond();
    if( _level == -1 )  {
      const int al = olx_max(TXAtom::LegendLevel(XAtoms[bond.A().GetTag()].GetPrimitives().GetName()),
        TXAtom::LegendLevel(XAtoms[bond.B().GetTag()].GetPrimitives().GetName()));
      if( al < cl || al >= 2 )  continue;
      cl = al;
    }
    const olxstr leg = xbonds[i]->GetLegend(xbonds[i]->Bond(), cl);
    TGPCollection* indCol = FGlRender->FindCollection(leg);
    if( &xbonds[i]->GetPrimitives() == indCol )  
      continue;
    else  {
      if( indCol == NULL )  {
        indCol = &FGlRender->NewCollection(leg);
        IndividualCollections.Add(leg);
      }
      xbonds[i]->GetPrimitives().RemoveObject(*xbonds[i]);
      BondCreationParams cp(XAtoms[bond.A().GetTag()], XAtoms[bond.B().GetTag()]);
      xbonds[i]->Create(leg, &cp);
    }
  }
}
//..............................................................................
void TGXApp::Individualise(TXAtom& XA, short _level, int32_t mask)  {
  TXAtom const* atoms[] = {&XA};
  const int level = TXAtom::LegendLevel(XA.GetPrimitives().GetName());
  if( level >= 2 )  return;
  Individualise(TXAtomPList(1, atoms), _level == -1 ? level+1 : _level, mask);
}
//..............................................................................
void TGXApp::Collectivise(TXAtom& XA, short _level, int32_t mask)  {
  TXAtom const* atoms[] = {&XA};
  const int level = TXAtom::LegendLevel(XA.GetPrimitives().GetName());
  if( level == 0 )  return;
  Collectivise(TXAtomPList(1, atoms), _level == -1 ? level-1 : _level, mask);
}
//..............................................................................
void TGXApp::Individualise(TXBond& XB, short _level, int32_t mask)  {
  TXBond const* bonds[] = {&XB};
  const int level = TXAtom::LegendLevel(XB.GetPrimitives().GetName());
  if( level == 0 )  return;
  Individualise(TXBondPList(1, bonds), _level == -1 ? level+1 : _level, mask);
}
//..............................................................................
void TGXApp::Individualise(const TXBondPList& bonds, short _level, int32_t mask) {
  if( bonds.IsEmpty() || _level > 2 )  return;
  XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
  for( size_t i=0; i < bonds.Count(); i++ )  {
    TXBond& b = *bonds[i];
    const int cl = TXAtom::LegendLevel(b.GetPrimitives().GetName()); 
    if( cl >= 2 )  {
      if( mask >= 0 )  {
        BondCreationParams bcpar(XAtoms[b.Bond().A().GetTag()], XAtoms[b.Bond().B().GetTag()]); 
        b.UpdatePrimitives(mask, &bcpar);
      }
    }
    else if( _level != -1 && cl > _level )
      continue;
    else  {
      const int legend_level = _level == -1 ? cl+1 : _level;
      const olxstr leg = TXBond::GetLegend(b.Bond(), legend_level);
      TGPCollection* indCol = FGlRender->FindCollection(leg);
      if( &b.GetPrimitives() == indCol )  {
        if( mask >= 0 )  {
          BondCreationParams bcpar(XAtoms[b.Bond().A().GetTag()], XAtoms[b.Bond().B().GetTag()]); 
          b.UpdatePrimitives(mask, &bcpar);
        }
      }
      else  {
        if( indCol == NULL )  {
          indCol = &FGlRender->NewCollection(leg);
          IndividualCollections.Add(leg);
        }
        TPtrList<AGDrawObject> objects;
        for( size_t oi=0; oi < b.GetPrimitives().ObjectCount(); oi++ )  {
          TXBond* _xb = dynamic_cast<TXBond*>(&b.GetPrimitives().GetObject(oi));
          if( _xb != NULL && TXBond::GetLegend(_xb->Bond(), legend_level) == leg )
            objects.Add(_xb);
        }
        b.GetPrimitives().RemoveObjects(objects);
        BondCreationParams bcpar(XAtoms[b.Bond().A().GetTag()], XAtoms[b.Bond().B().GetTag()]); 
        for( size_t oi=0; oi < objects.Count(); oi++ )
          objects[oi]->Create(leg, &bcpar);
        if( mask >= 0 )  // this will affect the whole group
          b.UpdatePrimitives(mask, &bcpar);
      }
    }
  }
}
//..............................................................................
void TGXApp::Collectivise(TXBond& XB, short level, int32_t mask)  {
  TXBond const* bonds[] = {&XB};
  Collectivise(TXBondPList(1, bonds),
    level == -1 ? TXAtom::LegendLevel(XB.GetPrimitives().GetName())+1 : level, mask);
}
//..............................................................................
void TGXApp::Collectivise(const TXBondPList& bonds, short _level, int32_t mask)  {
  if( bonds.IsEmpty() )  return;
  XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
  for( size_t i=0; i < bonds.Count(); i++ )  {
    TXBond& b = *bonds[i];
    const int cl = TXAtom::LegendLevel(b.GetPrimitives().GetName());
    if( cl == 0 || (_level != -1 && cl < _level) )  continue;
    const olxstr leg = b.GetLegend(b.Bond(), _level == -1 ? cl-1 : _level);
    TGPCollection* indCol = FGlRender->FindCollection(leg);
    if( &b.GetPrimitives() == indCol )  
      continue;
    if( indCol == NULL )
      indCol = &FGlRender->NewCollection(leg);
    const size_t index = IndividualCollections.IndexOf(b.GetPrimitives().GetName());
    if( index != InvalidIndex )  
      IndividualCollections.Delete(index);
    BondCreationParams bcpar(XAtoms[b.Bond().A().GetTag()], XAtoms[b.Bond().B().GetTag()]); 
    TPtrList<AGDrawObject> objects = b.GetPrimitives().GetObjects();
    b.GetPrimitives().ClearObjects();
    for( size_t oi=0; oi < objects.Count(); oi++ )  {
      objects[oi]->Create(leg, &bcpar);
    }
    if( mask >= 0 )
      b.UpdatePrimitives(mask, &bcpar);
  }
}
//..............................................................................
void TGXApp::Collectivise(const TXAtomPList& atoms, short _level, int32_t mask)  {
  if( atoms.IsEmpty() )  return;
  //TSBondPList sbonds;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    TXAtom& a = *atoms[i];
    const int cl = TXAtom::LegendLevel(a.GetPrimitives().GetName());
    if( cl == 0 || (_level != -1 && cl < _level) )  continue;
    const olxstr leg = a.GetLegend(a.Atom(), _level == -1 ? cl-1 : _level);
    TGPCollection* indCol = FGlRender->FindCollection(leg);
    if( indCol != NULL && &a.GetPrimitives() == indCol )  
      continue;
    else  {
      if( indCol == NULL )  
        indCol = &FGlRender->NewCollection(leg);
      const size_t index = IndividualCollections.IndexOf(a.GetPrimitives().GetName());
      if( index != InvalidIndex )  
        IndividualCollections.Delete(index);
      TPtrList<AGDrawObject> objects = a.GetPrimitives().GetObjects();
      a.GetPrimitives().ClearObjects();
      for( size_t oi=0; oi < objects.Count(); oi++ )
        objects[oi]->Create(leg);
      if( mask >= 0 )
        a.UpdatePrimitives(mask);
      //for( size_t j=0; j < a.Atom().BondCount(); j++ )
      //  sbonds.Add(a.Atom().Bond(j));
    }
  }
  //TXBondPList xbonds;
  //SBonds2XBonds(sbonds, xbonds);
  //XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
  //for( size_t i=0; i < xbonds.Count(); i++ )  {
  //  if( TXAtom::LegendLevel(xbonds[i]->GetPrimitives().GetName()) <= level )
  //    continue;
  //  const olxstr leg = xbonds[i]->GetLegend(xbonds[i]->Bond(), level);
  //  TGPCollection* indCol = FGlRender->FindCollection(leg);
  //  if( indCol != NULL && &xbonds[i]->GetPrimitives() == indCol )  
  //    continue;
  //  else  {
  //    if( indCol == NULL )
  //      indCol = &FGlRender->NewCollection(leg);
  //    xbonds[i]->GetPrimitives().RemoveObject(*xbonds[i]);
  //    if( xbonds[i]->GetPrimitives().ObjectCount() == 0 )  {
  //      const size_t index = IndividualCollections.IndexOf(xbonds[i]->GetPrimitives().GetName());
  //      if( index != InvalidIndex )  
  //        IndividualCollections.Delete(index);
  //    }
  //    TSBond& bond = xbonds[i]->Bond();
  //    BondCreationParams cp(XAtoms[bond.A().GetTag()], XAtoms[bond.B().GetTag()]);
  //    xbonds[i]->Create(leg, &cp);
  //  }
  //}
}
//..............................................................................
size_t TGXApp::GetNextAvailableLabel(const olxstr& AtomType) {
  size_t nextLabel = 0, currentLabel;
  cm_Element* elm = XElementLib::FindBySymbol(AtomType);
  if( elm == NULL )  return nextLabel;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( XAtoms[i].Atom().GetType() == *elm)  {
      olxstr label = XAtoms[i].Atom().GetLabel().SubStringFrom(elm->symbol.Length());
      if( !label.Length() )  continue;
      olxstr nLabel;
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
void TGXApp::SynchroniseBonds(TXAtomPList& xatoms)  {
  TSBondPList sbonds;
  TXBondPList xbonds;
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    for( size_t j=0; j < xatoms[i]->Atom().BondCount(); j++ )
      sbonds.Add(xatoms[i]->Atom().Bond(j));
  }
  // prepare unique list of bonds
  sbonds.ForEach(ACollectionItem::IndexTagSetter<>());
  sbonds.Pack(ACollectionItem::IndexTagAnalyser<>());
  // have to call setatom function to set the correct order for atom of bond
  for( size_t i=0; i < sbonds.Count(); i++ )
    sbonds[i]->SetA(sbonds[i]->A());
  SBonds2XBonds(sbonds, xbonds);
  // safety sake...
  XFile().GetLattice().GetAtoms().ForEach(ACollectionItem::TagSetter<>(-1));
  XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
  for( size_t i=0; i < xbonds.Count(); i++ )  {
    if( xbonds[i]->Bond().A().GetTag() == -1 || xbonds[i]->Bond().B().GetTag() == -1 )
      continue;
    xbonds[i]->Update();
    xbonds[i]->GetPrimitives().RemoveObject(*xbonds[i]);
    TXAtom& XA  = XAtoms[xbonds[i]->Bond().A().GetTag()];
    TXAtom& XA1 = XAtoms[xbonds[i]->Bond().B().GetTag()];
    BondCreationParams bcp(XA, XA1);
    xbonds[i]->Create(
      TXBond::GetLegend(
        xbonds[i]->Bond(), olx_max(TXAtom::LegendLevel(XA.GetPrimitives().GetName()),
          TXAtom::LegendLevel(XA1.GetPrimitives().GetName()))),
        &bcp);
  }
}
//..............................................................................
void TGXApp::SetXGrowPointsVisible(bool v)  {
  if( !XGrowPoints.Count() && v )  {
    CreateXGrowPoints();
    XGrowPointsVisible = v;
    return;
  }
  for( size_t i=0; i < XGrowPoints.Count(); i++ )
    XGrowPoints[i].SetVisible(v);
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

  vec3d MFrom(-1.5, -1.5, -1.5), MTo(2, 2, 2);
  vec3d VTo = (MTo+1).Round<int>();
  vec3d VFrom = (MFrom-1).Round<int>();
  XFile().GetLattice().GenerateMatrices(matrices, VFrom, VTo, MFrom, MTo);

  VFrom = XFile().GetAsymmUnit().GetOCenter(false, false);
  for( size_t i=0; i < matrices.Count(); i++ )  {
    if( UsedTransforms.IndexOf(*matrices[i]) != InvalidIndex )  {
      delete matrices[i];
      continue;
    }
    VTo = VFrom * matrices[i]->r;
    VTo += matrices[i]->t;
    XFile().GetAsymmUnit().CellToCartesian(VTo);
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
    AGDObjList lines(XGrowLines.Count()*2);  // list of the AGDrawObject pointers to lines...
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      XGrowLines[i].GetPrimitives().SetTag(i);
      XGrowLines[i].GetLabel().GetPrimitives().SetTag(i);
      lines.Set(i*2, XGrowLines[i]);
      lines.Set(i*2+1, XGrowLines[i].GetLabel());
    }
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      if( XGrowLines[i].GetPrimitives().GetTag() == i )
        colls.Add(XGrowLines[i].GetPrimitives());
      if( XGrowLines[i].GetLabel().GetPrimitives().GetTag() == i )
        colls.Add(XGrowLines[i].GetLabel().GetPrimitives());
    }
    FGlRender->RemoveCollections(colls);  // remove collections with their primitives
    FGlRender->RemoveObjects(lines);  // remove the object references
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
    AtomsToProcess.AddList(xatoms, TXAtom::AtomAccessor<>());
  }
  else if( (FGrowMode & gmSameAtoms) == 0 ) {
    const size_t ac = FXFile->GetLattice().AtomCount();
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& A = FXFile->GetLattice().GetAtom(i);
      if( A.IsDeleted() || !A.CAtom().IsAvailable() )  continue;
      AtomsToProcess.Add(A);
    }
  }
  TPtrList<TCAtom> AttachedAtoms;
  TTypeList<TGXApp_Transform1> tr_list;
  const AtomRegistry& ar = XFile().GetLattice().GetAtomRegistry();
  typedef TArrayList<AnAssociation2<TCAtom*,smatd> > GInfo;
  TPtrList<GInfo> Info(au.AtomCount());
  for( size_t i=0; i < AtomsToProcess.Count(); i++ )  {
    TSAtom* A = AtomsToProcess[i];
    AttachedAtoms.Clear();
    if( (FGrowMode & gmCovalent) != 0 )  {
      for( size_t j=0; j < A->CAtom().AttachedAtomCount(); j++ )
        if( A->CAtom().GetAttachedAtom(j).IsAvailable() )
          AttachedAtoms.Add(A->CAtom().GetAttachedAtom(j));
    }
    if( (FGrowMode & gmSInteractions) != 0 )  {
      for( size_t j=0; j < A->CAtom().AttachedAtomICount(); j++ )
        if( A->CAtom().GetAttachedAtomI(j).IsAvailable() )
          AttachedAtoms.Add(A->CAtom().GetAttachedAtomI(j));
    }
    if( (FGrowMode & gmSameAtoms) != 0 )
      AttachedAtoms.Add(A->CAtom());

    if( AttachedAtoms.IsEmpty() )  continue;
    GInfo* gi = Info[A->CAtom().GetId()];
    if( gi == NULL )  {
      gi = new GInfo;
      if( FGrowMode == gmSameAtoms )  {
        uc.FindInRangeAM(A->CAtom().ccrd(), 2*A->GetType().r_bonding + 15,
          *gi, &AttachedAtoms);
      }
      else if( FGrowMode == gmSInteractions )  {
        uc.FindBindingAM(*A, FXFile->GetLattice().GetDeltaI(), *gi, &AttachedAtoms);
      }
      else  {
        uc.FindBindingAM(*A, FXFile->GetLattice().GetDelta(), *gi, &AttachedAtoms);
      }
      Info[A->CAtom().GetId()] = gi;
    }
    for( size_t j=0; j < gi->Count(); j++ )  {
      const AnAssociation2<TCAtom*,smatd>& gii = (*gi)[j];
      smatd transform = (A->GetMatrix(0).IsFirst() ? gii.GetB() : uc.MulMatrix(gii.GetB(), A->GetMatrix(0)));
      if( ar.Find(TSAtom::Ref(gii.GetA()->GetId(), transform.GetId())) != NULL )
        continue;
      vec3d tc = transform*gii.GetA()->ccrd();
      au.CellToCartesian(tc);
      const double qdist = tc.QDistanceTo(A->crd());
      bool uniq = true;
      for( size_t l=0; l < tr_list.Count(); l++ )  {
        if( tr_list[l].transform.GetId() == transform.GetId() )  {
          if( tr_list[l].dist > qdist )  {
            tr_list[l].dist = qdist;
            tr_list[l].to = gii.GetA();
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
        nt.to = gii.GetA();
        nt.from = A;
      }
    }
  }
  for( size_t i=0; i < tr_list.Count(); i++ )  {
    TGXApp_Transform1& nt = tr_list[i];
    TXGrowLine& gl = XGrowLines.Add(new TXGrowLine(*FGlRender, EmptyString, nt.from, nt.to, nt.transform));
    gl.Create("GrowBonds");
  }
  Info.DeleteItems(true);
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
    AtomsToProcess.AddList(xatoms, TXAtom::AtomAccessor<>());
    const size_t ac = FXFile->GetLattice().AtomCount();
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& A = FXFile->GetLattice().GetAtom(i);
      if( A.IsDeleted() || !A.CAtom().IsAvailable() )  continue;
      CrdMap.Add(A.crd());
    }
  }
  else  {
    const size_t ac = FXFile->GetLattice().AtomCount();
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& A = FXFile->GetLattice().GetAtom(i);
      if( A.IsDeleted() || !A.CAtom().IsAvailable() )  continue;
      AtomsToProcess.Add(A);
      CrdMap.Add(A.crd());
    }
  }
  typedef TTypeList<TGXApp_Transform> tr_list;
  olxdict<int, tr_list, TPrimitiveComparator> net_tr;
  TPtrList<TArrayList<AnAssociation2<TCAtom*,smatd> > > Info(au.AtomCount());
  for( size_t i=0; i < AtomsToProcess.Count(); i++ )  {
    TSAtom* A = AtomsToProcess[i];
    TArrayList<AnAssociation2<TCAtom*,smatd> >* envi = Info[A->CAtom().GetId()];
    if( envi == NULL )  {
      Info[A->CAtom().GetId()] = envi = new TArrayList<AnAssociation2<TCAtom*,smatd> >;
      uc.FindInRangeAM(A->ccrd(), DeltaV + A->GetType().r_bonding, *envi);
    }
    for( size_t j=0; j < envi->Count(); j++ )  {
      TCAtom *aa = envi->GetItem(j).GetA();
      if( !aa->IsAvailable() )  continue;
      const smatd transform = (A->GetMatrix(0).IsFirst() ? envi->GetItem(j).GetB() :
        uc.MulMatrix(envi->GetItem(j).GetB(), A->GetMatrix(0)));
      const vec3d& cc = aa->ccrd();
      vec3d tc = transform*cc;
      au.CellToCartesian(tc);
      const double qdist = tc.QDistanceTo(A->crd());
      if( qdist < 0.001 )  // skip atoms on special postions
        continue;
      bool uniq = true;
      if( CrdMap.Exists(tc) )  // check if point to one of already existing
        continue;
      tr_list& ntl = net_tr.Add(aa->GetFragmentId());
      //find the shortest one
      for( size_t l=0; l < ntl.Count(); l++ )  {
        if( ntl[l].transform.GetId() == transform.GetId() )  {
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
      }
    }
  }
  for( size_t i=0; i < net_tr.Count(); i++ )  {
    const tr_list& ntl = net_tr.GetValue(i);
    for( size_t j=0; j < ntl.Count(); j++ )  {
      TGXApp_Transform& nt = ntl[j];
      TXGrowLine& gl = XGrowLines.Add(new TXGrowLine(*FGlRender, EmptyString, nt.from, nt.to, nt.transform));
      gl.Create("GrowBonds");
    }
  }
  Info.DeleteItems(true);
}
//..............................................................................
void TGXApp::Grow(const TXGrowLine& growLine)  {
  UsedTransforms.AddCCopy(growLine.GetTransform());
  XFile().GetLattice().GrowAtom(growLine.CAtom()->GetFragmentId(), growLine.GetTransform());
}
//..............................................................................
void TGXApp::Grow(const TXGrowPoint& growPoint)  {
  UsedTransforms.AddCCopy(growPoint.GetTransform());
  XFile().GetLattice().Grow(growPoint.GetTransform());
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
  TXFile& f = OverlayedXFiles.Add((TXFile*)FXFile->Replicate());
  return f;
}
//..............................................................................
void TGXApp::SetActiveXFile(size_t i)  {
  if( i >= OverlayedXFiles.Count() )  return;
  FXFile = OverlayedXFiles.Replace(i, FXFile);
  FXFile->OnFileLoad.TakeOver(OverlayedXFiles[i].OnFileLoad);
  FXFile->OnFileSave.TakeOver(OverlayedXFiles[i].OnFileSave);
  FXFile->GetLattice().OnDisassemble.TakeOver(OverlayedXFiles[i].GetLattice().OnDisassemble);
  FXFile->GetLattice().OnStructureGrow.TakeOver(OverlayedXFiles[i].GetLattice().OnStructureGrow);
  FXFile->GetLattice().OnStructureUniq.TakeOver(OverlayedXFiles[i].GetLattice().OnStructureUniq);
  //CreateObjects(
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
    const double r = cnt.QDistanceTo(latt.GetAtom(i).crd());
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
  OverlayedXFiles.Delete(index);
  CreateObjects(true, false);
  CenterView();
  Draw();
}
//..............................................................................
void TGXApp::UpdateBonds()  {
  for( size_t i=0; i < XBonds.Count(); i++ )
    XBonds[i].Update();
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
    //if( XAtoms[i].Atom().GetType() == iQPeakZ )  continue;
    vec3d::UpdateMinMax(XAtoms[i].Atom().ccrd(), mn, mx);
    atoms.AddNew(XAtoms[i].Atom().crd(), olx_sqr(XAtoms[i].Atom().GetType().r_vdw)+inc);
  }
  mn -= 1./4;
  mx += 1./4;
  double res = 0.5;
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
  TPtrList<TXGlLabel> atom_labels(a_cnt);
  a_cnt = 0;
  for( size_t i=0; i < XAtoms.Count(); i++ )  {
    if( !XAtoms[i].Atom().IsDeleted() )  {
      atom_labels.Set(a_cnt, XAtoms[i].GetLabel());
      vis.Set(a_cnt++, XAtoms[i].IsVisible());
    }
  }
  visibility.AddField("atoms", vis.ToBase64String());
  size_t b_cnt = 0;
  for( size_t i=0; i < XBonds.Count(); i++ )
    if( !XBonds[i].Bond().IsDeleted() )
      b_cnt++;
  vis.SetSize(b_cnt);
  TPtrList<TXGlLabel> bond_labels(b_cnt);
  b_cnt = 0;
  for( size_t i=0; i < XBonds.Count(); i++ )  {
    if( !XBonds[i].Bond().IsDeleted() )  {
      bond_labels.Set(b_cnt, XBonds[i].GetLabel());
      vis.Set(b_cnt++, XBonds[i].IsVisible());
    }
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
    XLabels[i].ToDataItem(labels.AddItem("Label"));
  TDataItem& atom_labels_item = item.AddItem("AtomLabels");
  for( size_t i=0; i < atom_labels.Count(); i++ )
    atom_labels[i]->ToDataItem(atom_labels_item.AddItem("Label"));
  TDataItem& bond_labels_item = item.AddItem("BondLabels");
  for( size_t i=0; i < bond_labels.Count(); i++ )
    bond_labels[i]->ToDataItem(bond_labels_item.AddItem("Label"));

  FGlRender->GetSelection().SetTag(-1);
  FGlRender->GetGroups().ForEach(ACollectionItem::IndexTagSetter<>());
  
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
        atoms.AddField("atom_id", ((TXAtom&)glO).Atom().GetTag());
      if( EsdlInstanceOf(glO, TXBond) )
        bonds.AddField("bond_id", ((TXBond&)glO).Bond().GetTag());
      if( EsdlInstanceOf(glO, TXPlane) )
        planes.AddField("plane_id", ((TXPlane&)glO).Plane().GetTag());
    }
  }

  TDataItem& renderer = item.AddItem("Renderer");
  renderer.AddField("min", PersUtil::VecToStr(FGlRender->MinDim()));
  renderer.AddField("max", PersUtil::VecToStr(FGlRender->MaxDim()));
}
//..............................................................................
void TGXApp::FromDataItem(TDataItem& item, IInputStream& zis)  {
  FGlRender->Clear();
  ClearXObjects();
  ClearLabels();
  ClearGroupDefinitions();
  FXFile->FromDataItem(item.FindRequiredItem("XFile"));
  FGlRender->GetStyles().FromDataItem(item.FindRequiredItem("Style"), true);
  
  IndividualCollections.Clear();
  TDataItem& ind_col = item.FindRequiredItem("ICollections");
  for( size_t i=0; i < ind_col.FieldCount(); i++ )
    IndividualCollections.Add(ind_col.GetField(i));

  const TDataItem& labels = item.FindRequiredItem("Labels");
  for( size_t i=0; i < labels.ItemCount(); i++ )
    XLabels.Add(new TXGlLabel(*FGlRender, PLabelsCollectionName)).FromDataItem(labels.GetItem(i));

  FXGrid->FromDataItem(item.FindRequiredItem("Grid"), zis);
  FDBasis->FromDataItem(item.FindRequiredItem("DBasis"));

  TDataItem& visibility = item.FindRequiredItem("Visibility");
  FHydrogensVisible = visibility.GetRequiredField("h_atoms").ToBool();
  FHBondsVisible = visibility.GetRequiredField("h_bonds").ToBool();
  FQPeaksVisible = visibility.GetRequiredField("q_atoms").ToBool();
  FQPeakBondsVisible = visibility.GetRequiredField("q_bonds").ToBool();

  CreateObjects(true, true);

  FDBasis->SetVisible(visibility.GetRequiredField("basis").ToBool());
  FDUnitCell->SetVisible(visibility.GetRequiredField("cell").ToBool());

  const TDataItem* atom_labels = item.FindItem("AtomLabels");
  if( atom_labels != NULL )  {
    if( XAtoms.Count() != atom_labels->ItemCount() )
      throw TFunctionFailedException(__OlxSourceInfo, "integrity is broken");
    for( size_t i=0; i < atom_labels->ItemCount(); i++ )
      XAtoms[i].GetLabel().FromDataItem(atom_labels->GetItem(i));
  }
  const TDataItem* bond_labels = item.FindItem("BondLabels");
  if( bond_labels != NULL )  {
    if( XBonds.Count() != bond_labels->ItemCount() )
      throw TFunctionFailedException(__OlxSourceInfo, "integrity is broken");
    for( size_t i=0; i < bond_labels->ItemCount(); i++ )
      XBonds[i].GetLabel().FromDataItem(bond_labels->GetItem(i));
  }

  TEBitArray vis;
  vis.FromBase64String( visibility.GetRequiredField("atoms") );
  if( vis.Count() != XAtoms.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "integrity is broken");
  for( size_t i=0; i < vis.Count(); i++ )
    XAtoms[i].SetVisible(vis[i]);
  vis.FromBase64String( visibility.GetRequiredField("bonds") );
  if( vis.Count() != XBonds.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "integrity is broken");
  for( size_t i=0; i < XBonds.Count(); i++ )
    XBonds[i].SetVisible(vis[i]);
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
      glG.Add(XBonds[bonds.GetField(j).ToSizeT()]);
    TDataItem& planes = group.FindRequiredItem("Planes");
    for( size_t j=0; j < planes.FieldCount(); j++ )
      glG.Add(XPlanes[planes.GetField(j).ToSizeT()]);
    glG.Create(group.GetValue());
    StoreGroup(glG, GroupDefs.AddNew());
  }
  _UpdateGroupIds();

  TDataItem& renderer = item.FindRequiredItem("Renderer");
  vec3d min = PersUtil::FloatVecFromStr(renderer.GetRequiredField("min"));
  vec3d max = PersUtil::FloatVecFromStr(renderer.GetRequiredField("max"));
  FGlRender->SetSceneComplete(false);
  FGlRender->ClearMinMax();
  FGlRender->UpdateMinMax(min, max);
  FGlRender->GetBasis().FromDataItem(item.FindRequiredItem("Basis"));
  FGlRender->SetSceneComplete(true);
}
//..............................................................................
void TGXApp::SaveModel(const olxstr& fileName) const {
#ifdef __WXWIDGETS__
  try  {
    TDataFile df;
    wxFileOutputStream fos(fileName.u_str());
    if( !fos.IsOk() )
      throw TFunctionFailedException(__OlxSourceInfo, "to save model");
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
    olxcstr model(TUtf8::Encode(bf.ToString()));
#else
    olxcstr model(bf.ToString());
#endif
    zos.Write(model.raw_str(), model.RawLen());
    zos.CloseEntry();
    zos.Close();
    fos.Close();
  }
  catch(const TExceptionBase& e)  {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
#else
  throw TNotImplementedException(__OlxSourceInfo);
#endif
}
//..............................................................................
void TGXApp::LoadModel(const olxstr& fileName) {
#ifdef __WXWIDGETS__
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
#else
  throw TNotImplementedException(__OlxSourceInfo);
#endif
}
//..............................................................................
void TGXApp::GroupSelection(const olxstr& name)  {
  TGlGroup* glg = GetRender().GroupSelection(name);
  if( glg != NULL )  {
    StoreGroup(*glg, GroupDefs.AddNew());
    _UpdateGroupIds();
    GroupDict(glg, GroupDefs.Count()-1);
    Draw();
  }
}
//..............................................................................
void TGXApp::UnGroupSelection()  {
  TGlGroup& sel = GetSelection();
  if( sel.Count() < 2 )  return;
  for( size_t i=0; i < sel.Count(); i++ )  {
    if( EsdlInstanceOf(sel[i], TGlGroup) )  {
      TGlGroup& G = (TGlGroup&)sel[i];
      size_t i = GroupDict.IndexOf(&G);
      if( i != InvalidIndex )  {
        GroupDefs.Delete(GroupDict.GetValue(i));
        GroupDict.Delete(i);
      }
      GetRender().UnGroup(G);
    }
    _UpdateGroupIds();
  }
  Draw();
}
//..............................................................................
void TGXApp::UnGroup(TGlGroup& G)  {
  size_t i = GroupDict.IndexOf(&G);
  if( i != InvalidIndex )  {
    GroupDefs.Delete(GroupDict.GetValue(i));
    GroupDict.Delete(i);
  }
  GetRender().UnGroup(G);
  _UpdateGroupIds();
  Draw();
}
//..............................................................................
void TGXApp::_UpdateGroupIds()  {
  FGlRender->GetGroups().ForEach(ACollectionItem::IndexTagSetter<>());
  FGlRender->GetSelection().SetTag(-1);
  for( size_t i=0; i < GroupDefs.Count(); i++ )  {
    TGlGroup* p = FGlRender->GetGroup(i).GetParentGroup();
    GroupDefs[i].parent_id = (p == NULL ? -2 : p->GetTag()) ;
  }
}
//..............................................................................
void TGXApp::SelectAll(bool Select)  {
  if( !Select )  {
    if( !SelectionCopy[0].IsEmpty() )  {
      SelectionCopy[1] = SelectionCopy[0];
      SelectionCopy[0].Clear();
    }
    else
      StoreGroup(GetSelection(), SelectionCopy[1]);
  }
  GetRender().SelectAll(Select);
  _UpdateGroupIds();
  Draw();
}
//..............................................................................
BondCreationParams TGXApp::GetBondCreationParams(const TXBond& xb) const {
  xb.Bond().A().SetTag(-1);
  xb.Bond().B().SetTag(-1);
  XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
  if( xb.Bond().A().GetTag() == -1 || xb.Bond().B().GetTag() == -1 )
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  return BondCreationParams(XAtoms[xb.Bond().A().GetTag()], XAtoms[xb.Bond().B().GetTag()]);
}
//..............................................................................
void TGXApp::AtomTagsToIndexes() const {
  XAtoms.ForEach(ACollectionItem::IndexTagSetter<TXAtom::AtomAccessor<> >());
}
//..............................................................................
void TGXApp::BondTagsToIndexes() const {
  XBonds.ForEach(ACollectionItem::IndexTagSetter<TXBond::BondAccessor>());
}
//..............................................................................

