/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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
#include "povdraw.h"
#include "dring.h"
#include "dusero.h"
#include "gxmacro.h"
#include "glbackground.h"
#include "olxstate.h"
#include "vcov.h"
#include "ins.h"
#include "xmacro.h"
#include "md5.h"
#include "index_range.h"
#ifdef __WXWIDGETS__
  #include "wxglscene.h"
  #include "wx/string.h"
  #include "wx/fontutil.h"
  #include "wx/wfstream.h"
  #include "wxzipfs.h"
  #include "wx/clipbrd.h"
#elif __WIN32__
  #include "wglscene.h"
  #include <WinGDI.h>
#endif
#define ConeStipple  6.0
#define LineStipple  0xf0f0

// on Linux it is defined as something..
#undef QLength

int CompareStr(const olxstr &Str, const olxstr &Str1, bool IC) {
  size_t minl = olx_min(Str1.Length(), Str.Length());
  for (size_t i = 0; i < minl; i++) {
    if (olxstr::o_isdigit(Str[i])) {
      if (olxstr::o_isdigit(Str1[i])) {
        size_t j = i, k = i;
        while ((++j < Str.Length()) && olxstr::o_isdigit(Str[j]))
          ;
        while ((++k < Str1.Length()) && olxstr::o_isdigit(Str1[k]))
          ;
        int diff = Str.SubString(i, j-i).ToInt() - Str1.SubString(i, k-i).ToInt();
        if (diff != 0) return diff;
        i = k-1;
      }
    }
    else {
      int diff = IC ? (olxstr::o_toupper(Str[i]) - olxstr::o_toupper(Str1[i]))
        : (Str[i] - Str1[i]);
      if (diff != 0)  return diff;
    }
  }
  return 0;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
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

class xappXFileLoad: public AActionHandler  {
  TGXApp *FParent;
  TEBasis B;
  TStrList AtomNames;
  TEBitArray CAtomMasks;
  TLattice::GrowInfo* GrowInfo;
  bool SameFile, EmptyFile;
  int state;
public:
  xappXFileLoad(TGXApp *Parent) {
    FParent = Parent;  
    AActionHandler::SetToDelete(false);
    GrowInfo = NULL;
    SameFile = false;
    state = 0;
  }
  ~xappXFileLoad()  {
    if( GrowInfo != NULL )
      delete GrowInfo;
    return;  
  }
  bool Enter(const IEObject *Sender, const IEObject *Data, TActionQueue *)  {
    state = 1;
    FParent->GetUndo().Clear();
    if( GrowInfo != NULL )  {
      delete GrowInfo;
      GrowInfo = NULL;
    }
    EmptyFile = SameFile = false;
    if( Data != NULL && EsdlInstanceOf(*Data, olxstr) )  {
      olxstr s1 = TEFile::UnixPath(TEFile::ChangeFileExt(
        *(olxstr*)Data, EmptyString()));
      olxstr s2 = TEFile::UnixPath(TEFile::ChangeFileExt(
        FParent->XFile().GetFileName(), EmptyString()));
      if( s1 != s2 )  {
        FParent->ClearStructureRelated();
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
      FParent->ClearStructureRelated();
    }
    //FParent->XGrid().Clear();
    B = FParent->GetRender().GetBasis();
    FParent->GetRender().Clear();
    FParent->HklFile().Clear();
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data, TActionQueue *) {
    state = 2;
    const TAsymmUnit& au = FParent->XFile().GetAsymmUnit();
    bool sameAU = true, hasNonQ = false;
    size_t ac = 0;
    for (size_t i=0; i < au.AtomCount(); i++) {
      const TCAtom& ca = au.GetAtom(i);
      if (ca.IsDeleted() || ca.GetType() == iQPeakZ) continue;
      hasNonQ = true;
      if (ac >= AtomNames.Count()) {
        sameAU = false;
        break;
      }
      if (!AtomNames[ac++].Equalsi(ca.GetLabel())) {
        sameAU = false;
        break;
      }
    }
    FParent->XFile().GetAsymmUnit().DetachAtomType(iQPeakZ,
      !FParent->AreQPeaksVisible());
    FParent->XFile().GetAsymmUnit().DetachAtomType(iHydrogenZ,
      !FParent->AreHydrogensVisible());
    if (sameAU) {  // apply masks
      ac = 0;
      for (size_t i=0; i < au.AtomCount(); i++) {
        TCAtom& ca = au.GetAtom(i);
        if (ca.IsDeleted() || ca.GetType() == iQPeakZ) continue;
        ca.SetMasked(CAtomMasks[ac++]);
      }
      FParent->XFile().GetLattice().SetGrowInfo(GrowInfo);
      GrowInfo = NULL;
    }
    else {  // definition will get broken otherwise
      FParent->ClearStructureRelated();
    }
    if (!hasNonQ && !FParent->AreQPeaksVisible())
      FParent->SetQPeaksVisible(true);
    if (GrowInfo != NULL) {
      delete GrowInfo;
      GrowInfo = NULL;
    }
    CAtomMasks.Clear();
    AtomNames.Clear();
    return false;
  }
  bool Exit(const IEObject *Sender, const IEObject *Data, TActionQueue *)  {
    if( state == 1 )  // somehing went not as expected? try to recover then...
      FParent->CreateObjects(true);
    state = 3;
    FParent->GetRender().SetBasis(B);
    FParent->CenterView(!SameFile);
    FParent->Draw();
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
  ID_OnFileLoad,
  ID_OnFileClose
};

TGXApp::TGXApp(const olxstr &FileName, AGlScene *scene)
  : TXApp(FileName, true),
  OnGraphicsVisible(NewActionQueue("GRVISIBLE")),
  OnFragmentVisible(NewActionQueue("FRVISIBLE")),
  OnAllVisible(NewActionQueue("ALLVISIBLE")),
  OnObjectsDestroy(NewActionQueue("OBJECTSDESTROY")),
  OnObjectsCreate(NewActionQueue("OBJECTSCREATE"))
{
  FQPeaksVisible = FHydrogensVisible = FStructureVisible = FHBondsVisible
    = true;
  ShowChemicalOccu = true;
  XGrowPointsVisible = FXGrowLinesVisible = FQPeakBondsVisible = false;
  DisplayFrozen = MainFormVisible = false;
  ZoomAfterModelBuilt = FXPolyVisible = true;
  stateStructureVisible = stateHydrogensVisible = stateHydrogenBondsVisible =
    stateQPeaksVisible = stateQPeakBondsVisible = stateCellVisible =
    stateBasisVisible = stateGradientOn = stateLabelsVisible =
    stateXGridVisible = stateWBoxVisible = InvalidIndex;
  DeltaV = 3;
  const TGlMaterial glm("2049;0.698,0.698,0.698,1.000");
  AGlScene *GlScene = scene;
  if (GlScene == NULL) {
#ifdef __WXWIDGETS__
    GlScene = new TwxGlScene(GetBaseDir() + "etc/Fonts/");
    wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
    GlScene->CreateFont("Default", Font.GetNativeFontInfoDesc()).SetMaterial(glm);
#else
    GlScene = new TWGlScene();
    GlScene->CreateFont("Default", "@20").SetMaterial(glm);
#endif
  }
  FGrowMode = gmCovalent;
//  TWGlScene *GlScene = new TWGlScene;
//  TGlScene *GlScene = new TGlScene;
  FGlRender = new TGlRenderer(GlScene, 1,1);
  TXApp::Init(new XObjectProvider(*FGlRender), this);
  FDFrame = new TDFrame(*FGlRender, "DFrame");
  Fader = new TXFader(*FGlRender, "Fader");
  FDFrame->OnSelect.Add(this, ID_OnSelect);
  FGlMouse = new TGlMouse(FGlRender, FDFrame);
  Library.AttachLibrary(FGlMouse->ExportLib());
  FDUnitCell = new TDUnitCell(*FGlRender, "DUnitCell");
  FDUnitCell->SetVisible(false);
  FDBasis = new TDBasis(*FGlRender, "DBasis");
  FDBasis->SetVisible(false);
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
  XFile().GetLattice().OnStructureUniq.Add(this, ID_OnUniq);
  XFile().GetLattice().OnStructureGrow.Add(this, ID_OnGrow);
  XFile().GetLattice().OnAtomsDeleted.Add(this, ID_OnClear);
  XFile().OnFileLoad.Add(this, ID_OnFileLoad);
  XFile().OnFileLoad.Add(&TEGC::NewG<xappXFileLoad>(this));
  XFile().OnFileClose.Add(this, ID_OnFileClose);
  try {
    float ma = TBasicApp::GetInstance().GetOptions()
      .FindValue("q_peak_min_alpha", "0").ToFloat();
    if (ma < 0) ma = 0;
    else if (ma > 0.75f) ma = 0.75f;
    TXAtom::SetMinQAlpha(ma);
  }
  catch(...) {}
  GXLibMacros *macros = new GXLibMacros(*this);
  TEGC::AddP(macros);
  macros->Export(Library);
  States = new TStateRegistry();
  stateStructureVisible = States->Register("strvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter(*this, &TGXApp::IsStructureVisible),
      new TStateRegistry::TMacroSetter("ShowStr")
    )
  );

  stateHydrogensVisible = States->Register("hvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter(*this, &TGXApp::AreHydrogensVisible),
      new TStateRegistry::TMacroSetter("ShowH a")
    )
  );
  stateHydrogenBondsVisible = States->Register("hbvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter(*this, &TGXApp::AreHBondsVisible),
      new TStateRegistry::TMacroSetter("ShowH b")
    )
  );
  stateQPeaksVisible = States->Register("qvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter(*this, &TGXApp::AreQPeaksVisible),
      new TStateRegistry::TMacroSetter("ShowQ a")
    )
  );
  stateQPeakBondsVisible = States->Register("qbvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter(*this, &TGXApp::AreQPeakBondsVisible),
      new TStateRegistry::TMacroSetter("ShowQ b")
    )
  );
  stateCellVisible = States->Register("cellvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter<TDUnitCell>(this->DUnitCell(),
        &TDUnitCell::IsVisible),
      new TStateRegistry::TMacroSetter("cell")
    )
  );
  stateBasisVisible = States->Register("basisvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter<TDBasis>(this->DBasis(),
        &TDBasis::IsVisible),
      new TStateRegistry::TMacroSetter("basis")
    )
  );
  stateGradientOn = States->Register("gradBG",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter<TGlBackground>(*this->GetRender().Background(),
        &TGlBackground::IsVisible),
      new TStateRegistry::TMacroSetter("grad")
    )
  );
  stateLabelsVisible = States->Register("labelsvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter(*this, &TGXApp::AreLabelsVisible),
      new TStateRegistry::TMacroSetter("labels")
    )
  );
  stateXGridVisible = States->Register("gridvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter<TXGrid>(this->XGrid(),
        &TXGrid::IsVisible),
      new TStateRegistry::TNoneSetter()
    )
  );
  stateWBoxVisible = States->Register("wboxvis",
    new TStateRegistry::Slot(
      TStateRegistry::NewGetter<T3DFrameCtrl>(this->Get3DFrame(),
        &T3DFrameCtrl::IsVisible),
      TStateRegistry::NewSetter<T3DFrameCtrl>(this->Get3DFrame(),
        &T3DFrameCtrl::SetVisible)
    )
  );
  TextureNames << "LockedAtoms";
  TextureNames << "ConstrainedAtoms";
}
//..............................................................................
TGXApp::~TGXApp()  {
  delete States;
  XFile().GetLattice().OnAtomsDeleted.Remove(this);
  Clear();
  delete FLabels;
  delete FHklFile;
  delete FXGrid;
  delete FGlRender;
  delete FGlMouse;
}
//..............................................................................
void TGXApp::ClearXObjects()  {
  OnObjectsDestroy.Enter(dynamic_cast<TBasicApp*>(this), NULL);
  FGlRender->SelectAll(false);
  FGlRender->ClearGroups();
  FGlRender->GetSelection().Clear();
  XGrowLines.Clear();
  XGrowPoints.Clear();
  XReflections.Clear();
  OnObjectsDestroy.Exit(dynamic_cast<TBasicApp*>(this), NULL);
}
//..............................................................................
void TGXApp::Clear()  {
  ClearXObjects();
  LooseObjects.DeleteItems().Clear();
  ObjectsToCreate.DeleteItems().Clear();
  UserObjects.Clear();
  XLabels.Clear();
  GlBitmaps.Clear();
}
//..............................................................................
void TGXApp::CreateXRefs()  {
  if( !XReflections.IsEmpty() )  return;
  TRefList refs;
  evecd Fsq;
  vec3d mind = FGlRender->MinDim(),
        maxd = FGlRender->MaxDim();
  RefinementModel::HklStat stats = CalcFsq(refs, Fsq, true);
  for( size_t i=0; i < refs.Count(); i++ )  {
    TXReflection* xr = new TXReflection(*FGlRender, "XReflection",
      stats.MinI, stats.MaxI, refs[i],
      FXFile->GetAsymmUnit());
    if (olx_abs(refs[i].GetI()-Fsq[i])/(refs[i].GetS() + 1e-2) > 8)
      xr->Params()[1] = -1;
    else
      xr->Params()[1] = 1;
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
void TGXApp::CreateObjects(bool centerModel, bool init_visibility)  {
  GetRender().GetScene().MakeCurrent();
  OnObjectsCreate.Enter(dynamic_cast<TBasicApp*>(this), NULL);
  TStopWatch sw(__FUNC__);
  sw.start("Initialising");
  const vec3d glCenter = FGlRender->GetBasis().GetCenter();
  ClearXObjects();
  FGlRender->ClearObjects();
  FGlRender->SetSceneComplete(false);

  for( size_t i=0; i < IndividualCollections.Count(); i++ )  {
    if( FGlRender->FindCollection(IndividualCollections[i]) == NULL )
      FGlRender->NewCollection(IndividualCollections[i]);
  }

  sw.start("Atoms creation");
  AtomIterator ai = GetAtoms();
  BondIterator bi = GetBonds();
  GetRender().SetObjectsCapacity(ai.count + bi.count + 512);
  while( ai.HasNext() )  {
    TXAtom& xa = ai.Next();
    xa.Create();
    if( !xa.IsDeleted() && init_visibility )  {
      xa.SetVisible(!FStructureVisible ? false
        : (xa.IsAvailable() && xa.CAtom().IsAvailable()));
    }
  }
  
  sw.start("Bonds creation");
  while( bi.HasNext() )  {
    TXBond& xb = bi.Next();
    xb.Update();
    xb.Create(TXBond::GetLegend(xb, 3));
    if( !xb.IsDeleted() && init_visibility )
      xb.SetVisible(xb.A().IsVisible() && xb.B().IsVisible());
    if( xb.IsVisible() && init_visibility )  {
      if( xb.A().GetType() == iQPeakZ || xb.B().GetType() == iQPeakZ )
        xb.SetVisible(FQPeakBondsVisible);
      else if( xb.GetType() == sotHBond )
        xb.SetVisible(FHBondsVisible);
    }
  }
  sw.start("Other objects creation");

  ObjectCaster<TSPlane,TXPlane> latt_planes =
    XFile().GetLattice().GetObjects().planes.GetAccessor<TXPlane>();
  for( size_t i=0; i < latt_planes.Count(); i++ )  {
    TXPlane& xp = latt_planes[i];
    xp.Create(olxstr("TXPlane") << xp.GetDefId());
  }
  double cell[] = {
    XFile().GetAsymmUnit().GetAxes()[0],
    XFile().GetAsymmUnit().GetAxes()[1],
    XFile().GetAsymmUnit().GetAxes()[2],
    XFile().GetAsymmUnit().GetAngles()[0],
    XFile().GetAsymmUnit().GetAngles()[1],
    XFile().GetAsymmUnit().GetAngles()[2]
  };
  DUnitCell().Init(cell);
  DBasis().SetAsymmUnit(XFile().GetAsymmUnit());

  for( size_t i=0; i < ObjectsToCreate.Count(); i++ )
    ObjectsToCreate[i]->Create();

  /*somehow if the XLAbels are created before the DBasis (like after picture drawing),
  the 'disappear' from opengl selection... - the plane is drawn in different color and
  selection is inpossible, unless properties are changed, odd... could not figure out
  what is going wrong... */

  XLabels.Pack(AGDrawObject::FlagsAnalyser(sgdoHidden));
  for( size_t i=0; i < XLabels.Count(); i++ )  {
    if( XLabels[i].IsVisible() )
      XLabels[i].Create();
    else
      XLabels.NullItem(i);
  }
  XLabels.Pack();

  for( size_t i=0; i < Lines.Count(); i++ )  {
    if( Lines[i].IsVisible() )
      Lines[i].Create();
    else
      Lines.NullItem(i);
  }
  Lines.Pack();

  for( size_t i=0; i < LooseObjects.Count(); i++ )  {
    if( !LooseObjects[i]->IsVisible() )  {
      delete LooseObjects[i];
      LooseObjects[i] = NULL;
    }
    else
      LooseObjects[i]->Create();
  }
  LooseObjects.Pack();
  for( size_t i=0; i < Rings.Count(); i++ )
    Rings[i].Create();

  FLabels->Init(false);
  FLabels->Create();

  if( FXGrowLinesVisible )  CreateXGrowLines();
  if( XGrowPointsVisible )  CreateXGrowPoints();
  FXGrid->Create();
  // create hkls
  if( FHklVisible )  SetHklVisible(true);
  for (size_t i=0; i < UserObjects.Count(); i++)
    UserObjects[i].Create();
  if( centerModel )
    CenterModel();
  else
    FGlRender->GetBasis().SetCenter(glCenter);
  olx_gl::loadIdentity();
  GetRender().SetView(false, 1);
  GetRender().Initialise();
  RestoreGroups();  // selection is created above
  FGlRender->SetSceneComplete(true);
  sw.stop();
  OnObjectsCreate.Exit(dynamic_cast<TBasicApp*>(this), NULL);
}
//..............................................................................
void TGXApp::CenterModel()  {
  double weight = 0;
  vec3d center, maX(-100, -100, -100), miN(100, 100, 100);
  for( size_t i=0; i <= OverlayedXFiles.Count(); i++ )  {
    const TLattice& latt = (i == OverlayedXFiles.Count() ? XFile() : OverlayedXFiles[i]).GetLattice();
    const size_t ac = latt.GetObjects().atoms.Count();
    for( size_t j=0; j < ac; j++ )  {
      const TSAtom& a = latt.GetObjects().atoms[j];
      if( a.IsAvailable() )  {
        center += a.crd();
        vec3d::UpdateMinMax(a.crd(), miN, maX);
        weight += 1;
      }
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
    if( !XReflections[i].IsVisible() )  continue;
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
  AtomIterator ai = GetAtoms();
  while( ai.HasNext() )  {
    const TXAtom& a = ai.Next();
    if( a.IsVisible() )  {
      center += a.crd();
      vec3d::UpdateMinMax(a.crd(), miN, maX);
      weight += 1;
    }
  }
  if( weight == 0 )  return;
  if( FDUnitCell->IsVisible() )  {
    for( size_t i=0; i < FDUnitCell->VertexCount(); i++ )  {
      center += FDUnitCell->GetVertex(i);
      weight += 1;
      vec3d::UpdateMinMax(FDUnitCell->GetVertex(i), miN, maX);
    }
  }
  for( size_t i=0; i < XReflections.Count(); i++ )  {
    if( !XReflections[i].IsVisible() )  continue;
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
void TGXApp::CalcProbFactor(double Prob)  {
  if( Prob < 0 )  Prob = FProbFactor;
  TXAtom::TelpProb((float)ProbFactor(Prob));//, Prob50 = ProbFactor(50);
  FProbFactor = Prob;
//  AtomZoom(ProbFactor);
}
//..............................................................................
/* finds such a value of x at wich the value of integral 4*pi*exp(-x^2/2)*x^2 is
Prob/100 of the max value, which is sqrt(8*pi^3), max returned value is around
10 
*/
double TGXApp::ProbFactor(double Prob) {
  // max of 4pi*int(0,inf)(exp(-x^2/2)*x^2dx) [/(4*pi*100)]
  static const double max_val = sqrt(8*M_PI*M_PI*M_PI)/(4*M_PI*100.0);
  const double t_val = Prob * max_val, inc = 1e-4;
  double ProbFactor = 0, summ = 0;
  while (summ < t_val) {
    const double v_sq = olx_sqr(ProbFactor + inc/2);
    summ += exp(-v_sq/2)*v_sq*inc;
    if ((ProbFactor += inc) >= 10)  //  sanity check
      break;
  }
  return ProbFactor;
}
//..............................................................................
void TGXApp::Init()  {
  try  {  CreateObjects(false);  }
  catch(const TExceptionBase &e)  {
    GetRender().GetStyles().Clear();
    try {
      GetRender()._OnStylesLoaded();
      CreateObjects(false);
    }
    catch(const TExceptionBase &e1) {
      throw TFunctionFailedException(__OlxSourceInfo, e1);
    }
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
  }
  try {
    ShowChemicalOccu = GetOptions().FindValue(
      "tooltip_occu_chem", TrueString()).ToBool();
  }
  catch(...) {
    TBasicApp::NewLogEntry(logInfo) <<
      (olxstr("Invalid boolean value for ").quote() << "tooltip_occu_chem");
  }
}
//..............................................................................
int32_t TGXApp::Quality(const short V)  {
  GetRender().GetScene().MakeCurrent();
  uint16_t aq = (uint16_t)(V >> 16);
  if (aq == 0) aq = V;
  int32_t rv = TXAtom::Quality(aq) << 16;
  TXAtom::CreateStaticObjects(*FGlRender);
  rv |= TXBond::Quality((uint16_t)(V&0x0000FFFF));
  TXBond::CreateStaticObjects(*FGlRender);
  AtomIterator ai = GetAtoms();
  while( ai.HasNext() )
    ai.Next().GetPrimitives().ClearPrimitives();
  BondIterator bi = GetBonds();
  while( bi.HasNext() )
    bi.Next().GetPrimitives().ClearPrimitives();
  CreateObjects(false, false);
  Draw();
  return rv;
}
//..............................................................................
bool TGXApp::IsCellVisible() const {
  return IsGraphicsVisible(FDUnitCell);
}
//..............................................................................
void TGXApp::SetCellVisible(bool v)  {
  GetUndo().Push(SetGraphicsVisible(FDUnitCell, v));
}
//..............................................................................
bool TGXApp::IsBasisVisible() const {
  return IsGraphicsVisible(FDBasis);
}
//..............................................................................
void TGXApp::SetBasisVisible(bool v)  {
  GetUndo().Push(SetGraphicsVisible(FDBasis, v));
}
//..............................................................................
TUndoData* TGXApp::SetGraphicsVisible(AGDrawObject *G, bool v)  {
  THideUndo* undo = new THideUndo(UndoAction::New(this, &TGXApp::undoHide));
  if (v != IsGraphicsVisible(G)) {
    G->SetVisible(v);
    undo->AddObject(G);
    OnGraphicsVisible.Execute(dynamic_cast<TBasicApp*>(this), G);
    Draw();
  }
  return undo;
}
//..............................................................................
TUndoData* TGXApp::SetGraphicsVisible(AGDObjList& G, bool v)  {
  THideUndo* undo = new THideUndo(UndoAction::New(this, &TGXApp::undoHide));
  for( size_t i=0; i < G.Count(); i++ )  {
    if( v == G[i]->IsVisible() )  continue;
    G[i]->SetVisible(v);
    undo->AddObject(G[i]);
  }
  OnGraphicsVisible.Execute(dynamic_cast<TBasicApp*>(this), &G);
  Draw();
  return undo;
}
//..............................................................................
olxstr macSel_GetName2(const TSAtom& a1, const TSAtom& a2)  {
  return olxstr(a1.GetGuiLabel()) << '-' << a2.GetGuiLabel();
}
olxstr macSel_GetName3(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3)  {
  return olxstr(a1.GetGuiLabel()) << '-' << a2.GetGuiLabel() << '-' <<
    a3.GetGuiLabel();
}
olxstr macSel_GetName4(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3,
  const TSAtom& a4)
{
  return olxstr(a1.GetGuiLabel()) << '-' << a2.GetGuiLabel() << '-' <<
    a3.GetGuiLabel() << '-' << a4.GetGuiLabel();
}
olxstr macSel_GetName4a(const TSAtom& a1, const TSAtom& a2, const TSAtom& a3,
  const TSAtom& a4)
{
  return olxstr(a1.GetGuiLabel()) << '-' << a2.GetGuiLabel() << ' ' <<
    a3.GetGuiLabel() << '-' << a4.GetGuiLabel();
}
olxstr macSel_GetPlaneName(const TSPlane& p)  {
  if( p.Count() == 0 )  return EmptyString();
  olxstr rv(p.GetAtom(0).GetGuiLabel());
  for( size_t i=1; i < p.Count(); i++ )
    rv << ' ' << p.GetAtom(i).GetGuiLabel();
  return rv;
}
olxstr macSel_FormatValue(double d, const ACifValue *cv) {
  if (cv == NULL)
    return olxstr::FormatFloat(3, d);
  if (olx_abs(cv->GetValue().GetV()-d) > 1e-2) {
    return  olxstr("CIF: ") << cv->GetValue().ToString() <<
      " Actual: " << olxstr::FormatFloat(3, d);
  }
  return cv->GetValue().ToString();
}

olxstr macSel_FormatDistance(const TSAtom &a, const TSAtom &b, double d,
  const ACifValue *cv)
{
  olxstr l = "Distance (";
  return (l << macSel_GetName2(a, b) << "): " << macSel_FormatValue(d, cv));
}

olxstr TGXApp::GetSelectionInfo(bool list) const {
  olxstr Tmp;
  try {
    double v;
    TGlGroup& Sel = FGlRender->GetSelection();
    if (list) {
      TStrList rv;
      for (size_t i=0; i < Sel.Count(); i++) {
        if (!EsdlInstanceOf(Sel[i], TXBond))  continue;
        TXBond &b = ((TXBond&)Sel[i]);
        ACifValue* cv=NULL;
        if( CheckFileType<TCif>() )  {
          cv = XFile().GetLastLoader<TCif>().GetDataManager().Match(
            b.A(), b.B());
        }
        rv << macSel_FormatDistance(b.A(), b.B(), b.Length(), cv);
      }
      return rv.Text(NewLineSequence());
    }
    if( Sel.Count() == 1 )  {
      if( EsdlInstanceOf(Sel[0], TXBond) )  {
        TXBond &b = ((TXBond&)Sel[0]);
        ACifValue* cv=NULL;
        if( CheckFileType<TCif>() )  {
          cv = XFile().GetLastLoader<TCif>().GetDataManager().Match(
            b.A(), b.B());
        }
        Tmp = macSel_FormatDistance(b.A(), b.B(), b.Length(), cv);
      }
    }
    else if( Sel.Count() == 2 )  {
      if( EsdlInstanceOf(Sel[0], TXAtom) &&
        EsdlInstanceOf(Sel[1], TXAtom) )
      {
        TSAtom &a1 = ((TXAtom&)Sel[0]),
          &a2 = ((TXAtom&)Sel[1]);
        ACifValue* cv=NULL;
        if( CheckFileType<TCif>() )
          cv = XFile().GetLastLoader<TCif>().GetDataManager().Match(a1, a2);
        Tmp = macSel_FormatDistance(a1, a2, a1.crd().DistanceTo(a2.crd()), cv);
      }
      else if( EsdlInstanceOf(Sel[0], TXBond) && EsdlInstanceOf(Sel[1], TXBond) )  {
        TXBond& A = (TXBond&)Sel[0], &B =(TXBond&)Sel[1];
        Tmp = "Angle (";
        Tmp << macSel_GetName4a(A.A(), A.B(), B.A(), B.B()) <<
          "): ";
        v = olx_angle(A.A().crd(), A.B().crd(), B.A().crd(), B.B().crd());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')' <<
          "\nAngle (" <<
          macSel_GetName4a(A.A(), A.B(), B.B(), B.A()) <<
          "): ";
        v = olx_angle(A.A().crd(), A.B().crd(), B.A().crd(), B.B().crd());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ')';
        // check for adjacent bonds
        if( !(&A.A() == &B.A() || &A.A() == &B.B() ||
          &A.B() == &B.A() || &A.B() == &B.B()) )
        {
          Tmp << "\nTorsion angle (" <<
            macSel_GetName4(A.A(), A.B(), B.B(), B.A()) <<
            "): ";
          v = olx_dihedral_angle_signed(A.A().crd(), A.B().crd(), B.B().crd(), B.A().crd());
          Tmp << olxstr::FormatFloat(3, v) <<
            "\nTorsion angle (" <<
            macSel_GetName4(A.A(), A.B(), B.B(), B.A()) <<
            "): ";
          v = olx_dihedral_angle_signed(A.A().crd(), A.B().crd(), B.A().crd(), B.B().crd());
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
        v = olx_angle(A.Edge(), A.Base(), B.A().crd(), B.B().crd());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ")";
      }
      else if( EsdlInstanceOf(Sel[0], TXBond) && EsdlInstanceOf(Sel[1], TXLine) )  {
        TXLine& A = (TXLine&)Sel[1];
        TXBond& B =(TXBond&)Sel[0];
        Tmp = "Angle: ";
        v = olx_angle(A.Edge(), A.Base(), B.A().crd(), B.B().crd());
        Tmp << olxstr::FormatFloat(3, v) << " (" << olxstr::FormatFloat(3, 180-v) << ")";
      }
      else if( (EsdlInstanceOf(Sel[0], TXPlane) && EsdlInstanceOf(Sel[1], TXAtom)) ||
               (EsdlInstanceOf(Sel[1], TXPlane) && EsdlInstanceOf(Sel[0], TXAtom)))
      {
        const TXPlane &p = (TXPlane&)(EsdlInstanceOf(Sel[0], TXPlane) ? Sel[0] : Sel[1]);
        const TXAtom &a = (TXAtom&)(EsdlInstanceOf(Sel[0], TXPlane) ? Sel[1] : Sel[0]);
        Tmp = "Distance (plane-atom): ";
        v = p.DistanceTo(a);
        Tmp << olxstr::FormatFloat(3, v) << "\nDistance (plane centroid-atom): " <<
          olxstr::FormatFloat(3, p.GetCenter().DistanceTo(a.crd()));
        vec3d x = (a.crd()-p.GetCenter());
        x -= p.GetNormal()*p.GetNormal().DotProd(x);
        Tmp << "\nAtom projection to the plane fractional coordinates: " <<
          XFile().GetAsymmUnit().Fractionalise(x+p.GetCenter()).ToString();
        Tmp << "\nCentroid-projection point distance: " <<
          olxstr::FormatFloat(3, x.Length());
        Tmp << "\nAtom-Centroid-projection point angle: " <<
          olxstr::FormatFloat(2,
            acos((a.crd()-p.GetCenter()).CAngle(x))*180/M_PI);
      }
      else if( EsdlInstanceOf(Sel[0], TXBond) && EsdlInstanceOf(Sel[1], TXPlane) )  {
        Tmp = "Angle (plane-bond): ";
        v = ((TXPlane&)Sel[1]).Angle(((TXBond&)Sel[0]));
        Tmp << olxstr::FormatFloat(3, v);
      }
      else if( EsdlInstanceOf(Sel[1], TXBond) && EsdlInstanceOf(Sel[0], TXPlane) )  {
        Tmp = "Angle (plane-bond): ";
        v = ((TXPlane&)Sel[0]).Angle(((TXBond&)Sel[1]));
        Tmp << olxstr::FormatFloat(3, v);
      }
      else if( EsdlInstanceOf(Sel[0], TXLine) && EsdlInstanceOf(Sel[1], TXPlane) )  {
        TXLine& xl = (TXLine&)Sel[0];
        Tmp = "Angle (plane-line): ";
        v = ((TXPlane&)Sel[1]).Angle(xl.Edge()-xl.Base());
        Tmp << olxstr::FormatFloat(3, v);
      }
      else if( EsdlInstanceOf(Sel[1], TXLine) && EsdlInstanceOf(Sel[0], TXPlane) )  {
        TXLine& xl = (TXLine&)Sel[1];
        Tmp = "Angle (plane-line): ";
        v = ((TXPlane&)Sel[0]).Angle(xl.Edge()-xl.Base());
        Tmp << olxstr::FormatFloat(3, v);
      }
      if( EsdlInstanceOf(Sel[1], TXPlane) && EsdlInstanceOf(Sel[0], TXPlane) )  {
        TXPlane &a = ((TXPlane&)Sel[0]),
          &b = ((TXPlane&)Sel[1]);
        
        const double ang = a.Angle(b);
        Tmp = "Angle (plane-plane): ";
        Tmp << olxstr::FormatFloat(3, ang) <<
          "\nTwist Angle (plane-plane, experimental): ";
        try {
          Tmp << olxstr::FormatFloat(3, olx_dihedral_angle(a.GetCenter()+a.GetNormal(),
            a.GetCenter(), b.GetCenter(), b.GetCenter()+b.GetNormal()));
        }
        catch (const TDivException &) {
          Tmp << "n/a";
        }
        Tmp << "\nFold Angle (plane-plane, experimental): ";
        try {
          vec3d n_c = (b.GetCenter()-a.GetCenter()).XProdVec(
            a.GetNormal()+b.GetNormal()).Normalise();
          vec3d p_a = a.GetNormal() -  n_c*n_c.DotProd(a.GetNormal());
          vec3d p_b = b.GetNormal() -  n_c*n_c.DotProd(b.GetNormal());
          Tmp << olxstr::FormatFloat(3, acos(p_a.CAngle(p_b))*180/M_PI);
        }
        catch (const TDivException &) {
          Tmp << "n/a";
        }
        Tmp <<
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
        if (a.Count() == b.Count() && a.Count() == 3) {
          TSAtomPList atoms(6), sorted_atoms;
          for (size_t i=0; i < 3; i++) {
            (atoms[i] = &a.GetAtom(i))->SetTag(0);
            (atoms[i+3] = &b.GetAtom(i))->SetTag(1);
          }
          olxdict<index_t, vec3d, TPrimitiveComparator> transforms;
          transforms.Add(1, -b.GetCenter());
          transforms.Add(0, -a.GetCenter());
          PlaneSort::Sorter::DoSort(atoms, transforms, vec3d(),
            b.GetNormal(), sorted_atoms);
          vec3d_alist pts;
          pts << vec3d_alist::FromList(sorted_atoms,
            FunctionAccessor::Make(&TSAtom::crd));
          VcoVContainer::TriangleTwistBP tt(pts);
          double v = tt.calc();
          if (v > 60) {
            v = 120-v;
          }
          Tmp << "\nMean triange twist angle: " << olxstr::FormatFloat(3, v);
        }
      }
    }
    else if( Sel.Count() == 3 )  {
      if( EsdlInstanceOf(Sel[0], TXAtom) &&
        EsdlInstanceOf(Sel[1], TXAtom) &&
        EsdlInstanceOf(Sel[2], TXAtom) )  {
          TSAtom &a1 = ((TXAtom&)Sel[0]),
            &a2 = ((TXAtom&)Sel[1]),
            &a3 = ((TXAtom&)Sel[2]);
        Tmp = "Angle (";
        Tmp << macSel_GetName3(a1, a2, a3)<< "): ";
        ACifValue* cv=NULL;
        if( CheckFileType<TCif>() )  {
          cv = XFile().GetLastLoader<TCif>().GetDataManager().Match(a1, a2, a3);
        }
        Tmp << macSel_FormatValue(olx_angle(a1.crd(), a2.crd(), a3.crd()), cv);
      }
      else if( EsdlInstanceOf(Sel[0], TXPlane) &&
        EsdlInstanceOf(Sel[1], TXPlane) &&
        EsdlInstanceOf(Sel[2], TXPlane) )  {
          TXPlane &p1 = ((TXPlane&)Sel[0]),
            &p2 = ((TXPlane&)Sel[1]),
            &p3 = ((TXPlane&)Sel[2]);
          Tmp = "Angle between plane centroids: ";
          Tmp << olxstr::FormatFloat(3, 
            olx_angle(p1.GetCenter(), p2.GetCenter(), p3.GetCenter()));
      }
      else if( EsdlInstanceOf(Sel[0], TXPlane) &&
        EsdlInstanceOf(Sel[1], TXAtom) &&
        EsdlInstanceOf(Sel[2], TXPlane) )  {
          TXPlane &p1 = ((TXPlane&)Sel[0]),
            &p2 = ((TXPlane&)Sel[2]);
          Tmp = "Angle between plane centroid - atom - plane centroid: ";
          Tmp << olxstr::FormatFloat(3, 
            olx_angle(p1.GetCenter(), ((TXAtom&)Sel[1]).crd(), p2.GetCenter()));
      }
    }
    else if( Sel.Count() == 4 )  {
      if( EsdlInstanceOf(Sel[0], TXAtom) &&
        EsdlInstanceOf(Sel[1], TXAtom) &&
        EsdlInstanceOf(Sel[2], TXAtom) &&
        EsdlInstanceOf(Sel[3], TXAtom) )  {
          TSAtom &a1 = ((TXAtom&)Sel[0]),
            &a2 = ((TXAtom&)Sel[1]),
            &a3 = ((TXAtom&)Sel[2]),
            &a4 = ((TXAtom&)Sel[3]);
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
    else if( Sel.Count() == 7 &&
      olx_list_and_st(Sel, &olx_is<TXAtom, TGlGroup::list_item_type>))
    {
      TSAtomPList atoms(Sel, DynamicCastAccessor<TSAtom>());
      TSAtom* central_atom = atoms[0];
      atoms.Delete(0);
      size_t face_cnt=0;
      double total_val_bp=0, total_val=0;
      for( size_t i=0; i < 6; i++ )  {
        for( size_t j=i+1; j < 6; j++ )  {
          for( size_t k=j+1; k < 6; k++ )  {
            const double thv = olx_tetrahedron_volume_n(central_atom->crd(),
              atoms[i]->crd(), atoms[j]->crd(), atoms[k]->crd());
            if( thv < 0.1 )  continue;
            face_cnt++;
            TSAtomPList sorted_atoms;
            olxdict<index_t, vec3d, TPrimitiveComparator> transforms;
            atoms.ForEach(ACollectionItem::TagSetter(0));
            atoms[i]->SetTag(1);
            atoms[j]->SetTag(1);
            atoms[k]->SetTag(1);
            const vec3d face_center =
              (atoms[i]->crd()+atoms[j]->crd()+atoms[k]->crd())/3;
            const vec3d normal = vec3d::Normal(
              atoms[i]->crd(), atoms[j]->crd(), atoms[k]->crd());

            vec3d face1_center;
            for( size_t l=0; l < atoms.Count(); l++ )
              if( atoms[l]->GetTag() == 0 )
                face1_center += atoms[l]->crd();
            face1_center /= 3;
            transforms.Add(1, central_atom->crd()-face_center);
            transforms.Add(0, central_atom->crd()-face1_center);
            PlaneSort::Sorter::DoSort(atoms, transforms, central_atom->crd(),
              normal, sorted_atoms);
            if (sorted_atoms[0]->GetTag() != 1)
              sorted_atoms.ShiftR(1);
            vec3d_alist pts;
            pts << central_atom->crd() << vec3d_alist::FromList(sorted_atoms,
              FunctionAccessor::Make(&TSAtom::crd));
            total_val += VcoVContainer::OctahedralDistortion(pts).calc();
            total_val_bp += VcoVContainer::OctahedralDistortionBP(pts).calc();
          }
        }
      }
      if( face_cnt == 8 ) {
        if (olx_abs(total_val-total_val_bp) < 1e-3) {
          Tmp << "Combined distortion: " <<
            olxstr::FormatFloat(2, total_val*3) << ", mean: " <<
            olxstr::FormatFloat(2, total_val/8) << NewLineSequence();
        }
        else {
          Tmp << "Combined distortion (cross-projections): " <<
            olxstr::FormatFloat(2, total_val*3) << ", mean: " <<
            olxstr::FormatFloat(2, total_val/8) << NewLineSequence();
          Tmp << "Combined distortion (best plane): " <<
            olxstr::FormatFloat(2, total_val_bp*3) << ", mean: " <<
            olxstr::FormatFloat(2, total_val_bp/8) << " degrees";
        }
      }
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
        Tmp << "Octahedral distortion (for the selection): " <<
          olxstr::FormatFloat(2, (sum*180/3)/M_PI);
      }
    }
  }
  catch( const TExceptionBase& )  {
    Tmp = "n/a";
  }
  return Tmp;
}
//..............................................................................
olxstr TGXApp::GetObjectInfoAt(int x, int y) const {
  AGDrawObject *G = SelectObject(x, y);
  if (G == NULL) return EmptyString();
  olxstr rv;
  if (G->IsSelected())
    rv = GetSelectionInfo();
  else if (EsdlInstanceOf(*G, TXAtom)) {
    const TXAtom &xa = *(TXAtom*)G;
    const TCAtom& ca = xa.CAtom();
    rv = xa.GetGuiLabelEx();
    if (xa.GetType() == iQPeakZ)
      rv << ':' << xa.CAtom().GetQPeak();
    double occu = (ShowChemicalOccu ? ca.GetChemOccu() : ca.GetOccu());
    rv << (ShowChemicalOccu ? "\nChem occu(" : "\nXtal occu(");
    if (ca.GetVarRef(catom_var_name_Sof) != NULL) {
      if (ca.GetVarRef(catom_var_name_Sof)->relation_type == relation_None) {
        rv << "fixed): ";
        if (olx_abs(occu-olx_round(occu)) < 1e-3)
          occu = olx_round(occu);
      }
      else {
        rv << "linked to ";
        if (olx_abs(ca.GetVarRef(catom_var_name_Sof)->coefficient-1.0) > 1e-6) {
          rv << olxstr(ca.GetVarRef(catom_var_name_Sof)->coefficient)
            .TrimFloat() << 'x';
        }
        if (ca.GetVarRef(catom_var_name_Sof)->relation_type == relation_AsVar)
          rv << "[FVAR#";
        else
          rv << "[1-FVAR #";
        rv << (ca.GetVarRef(catom_var_name_Sof)->Parent.GetId()+1) << "]): ";
      }
    }
    else
      rv << "free): ";
    rv << TEValueD(occu, ca.GetOccuEsd()*ca.GetDegeneracy()).ToString();
    if (ca.GetEllipsoid() == NULL) {
      rv << "\nUiso (";
      if (ca.GetVarRef(catom_var_name_Uiso) != NULL &&
        ca.GetVarRef(catom_var_name_Uiso)->relation_type == relation_None &&
        ca.GetUisoOwner() == NULL)
      {
        rv << "fixed): " << olxstr::FormatFloat(3, ca.GetUiso());
      }
      else if( ca.GetUisoOwner() != NULL )
        rv << "riding): " << olxstr::FormatFloat(3, ca.GetUiso());
      else
        rv << "free): " << TEValueD(ca.GetUiso(), ca.GetUisoEsd()).ToString();
    }
    else
      rv << "\nUeq " << olxstr::FormatFloat(3, ca.GetEllipsoid()->GetUeq());
#ifdef _DEBUG
    rv << "\nBonds: " << xa.BondCount() << ", nodes: " << xa.NodeCount();
    if (xa.GetEllipsoid() == NULL) {
      rv << "\nV: " << olxstr::FormatFloat(3,
        pow(xa.CAtom().GetUiso(), 3./2)*4*M_PI/3, true);
    }
    else {
      rv << "\nV: " << olxstr::FormatFloat(3,
        xa.GetEllipsoid()->GetSX()*xa.GetEllipsoid()->GetSY()*
        xa.GetEllipsoid()->GetSZ()*4*M_PI/3, true);
    }
#endif
  }
  else if (EsdlInstanceOf(*G, TXBond)) {
    TXBond& xb = *(TXBond*)G;
    rv = xb.A().GetLabel();
    rv << '-' << xb.B().GetLabel() << ": ";
    if (CheckFileType<TCif>()) {
      ACifValue* cv = XFile().GetLastLoader<TCif>()
        .GetDataManager().Match(xb.A(), xb.B());
      if (cv != NULL)
        rv << cv->GetValue().ToString();
      else
        rv << olxstr::FormatFloat(3, xb.Length());
    }
    else
      rv << olxstr::FormatFloat(3, xb.Length());
#ifdef _DEBUG
    vec3d n = (xb.A().crd()-xb.B().crd()).Normalise();
    rv << "\nn: " << olx_round(n[0], 1000) << ',' << olx_round(n[1], 1000) <<
      ',' << olx_round(n[2], 1000);
#endif
  } 
  else if (EsdlInstanceOf(*G, TXReflection)) {
    rv = ((TXReflection*)G)->GetHKL()[0];
    rv << ' '
      << ((TXReflection*)G)->GetHKL()[1] << ' '
      << ((TXReflection*)G)->GetHKL()[2] << ": "
      << ((TXReflection*)G)->GetI();
  }
  else if (EsdlInstanceOf(*G, TXLine)) {
    rv = olxstr::FormatFloat(3, ((TXLine*)G)->Length());
  }
  else if (EsdlInstanceOf(*G, TXGrowLine)) {
    rv = ((TXGrowLine*)G)->XAtom().GetLabel();
    rv << '-' << ((TXGrowLine*)G)->CAtom().GetLabel() << ": "
      << olxstr::FormatFloat(3, ((TXGrowLine*)G)->Length()) << '('
      << TSymmParser::MatrixToSymmEx(((TXGrowLine*)G)->GetTransform()) << ')';
  }
  else if (EsdlInstanceOf(*G, TXGrowPoint)) {
    rv = TSymmParser::MatrixToSymmEx(((TXGrowPoint*)G)->GetTransform());
  }
  else if (EsdlInstanceOf(*G, TXPlane)) {
    rv << "HKL direction: " <<
      ((TXPlane*)G)->GetCrystallographicDirection().ToString();
  }
  else if (EsdlInstanceOf(*G, TDUserObj)) {
    TDUserObj &o = *(TDUserObj*)G;
    if (o.GetType() == sgloSphere && !o.Params().IsEmpty()) {
      double r=o.Params()[0]*o.Basis.GetZoom();
      rv << "Sphere volume/radius, A: " <<
        olxstr::FormatFloat(3, olx_sphere_volume(r)) << '/' <<
        olxstr::FormatFloat(3, r);
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
void TGXApp::SelectFragmentsAtoms(const TNetPList& frags, bool v)  {
  TXAtomPList XA;
  for( size_t i=0; i < frags.Count(); i++ )  {
    for( size_t j=0; j < frags[i]->NodeCount(); j++ )
      XA.Add(static_cast<TXAtom&>(frags[i]->Node(j)));
  }
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
  TXBondPList XB;
  for( size_t i=0; i < frags.Count(); i++ )  {
    for( size_t j=0; j < frags[i]->BondCount(); j++ )
      XB.Add(static_cast<TXBond&>(frags[i]->Bond(j)));
  }
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
  TXAtomPList XA;
  SortedPtrList<TGlGroup, TPointerComparator> groups;
//  OnFragmentVisible->Enter(dynamic_cast<TBasicApp*>(this), dynamic_cast<IEObject*>(N));
  XA.SetCapacity(N->NodeCount());
  for( size_t i=0; i < N->NodeCount(); i++ )
    XA.Add(static_cast<TXAtom&>(N->Node(i)));
  for( size_t i=0; i < XA.Count(); i++ )  {
    if( XA[i]->GetParentGroup() != NULL )
      groups.AddUnique(XA[i]->GetParentGroup());
    else
      XA[i]->SetVisible(V);
  }

  TXBondPList XB;
  XB.SetCapacity(N->BondCount());
  for( size_t i=0; i < N->BondCount(); i++ )
    XB.Add(static_cast<TXBond&>(N->Bond(i)));
  for( size_t i=0; i < XB.Count(); i++ )  {
    if( XB[i]->GetParentGroup() != NULL )
      groups.AddUnique(XB[i]->GetParentGroup());
    else
      XB[i]->SetVisible(V);
  }
  for( size_t i=0; i < groups.Count(); i++ )
    groups[i]->SetVisible(V);
//  OnFragmentVisible->Exit(dynamic_cast<TBasicApp*>(this), dynamic_cast<IEObject*>(N));
}
//..............................................................................
void TGXApp::FragmentsVisible(const TNetPList& Frags, bool V)  {
//  OnFragmentsVisible->Enter(this, dynamic_cast<IEObject*>(Frags));
  for( size_t i=0; i < Frags.Count(); i++ )
    FragmentVisible(Frags[i], V);
  // synchronise the intermolecular bonds
  SetHBondsVisible(FHBondsVisible, false);
  _maskInvisible();
  //  OnFragmentsVisible->Exit(this, dynamic_cast<IEObject*>(Frags));
  Draw();
}
//..............................................................................
TGlGroup& TGXApp::GroupFragments(const TNetPList& Fragments, const olxstr groupName)  {
  GetRender().GetSelection().Clear();
  TXBondPList xbonds;
  for( size_t i=0; i < Fragments.Count(); i++ )  {
    for( size_t j=0; j < Fragments[i]->BondCount(); j++ )
      xbonds.Add(static_cast<TXBond&>(Fragments[i]->Bond(j)));
  }
  if( xbonds.IsEmpty() )  return *(TGlGroup*)NULL;
  for( size_t i=0; i < xbonds.Count(); i++ )
    GetRender().GetSelection().Add(*xbonds[i]);
  return *GetRender().GroupSelection(groupName);
}
//..............................................................................
TNetPList TGXApp::InvertFragmentsList(const TNetPList& SF)  {
  TNetPList res;
  TLattice& L = XFile().GetLattice();
  L.GetFragments().ForEach(ACollectionItem::TagSetter(1));
  SF.ForEach(ACollectionItem::TagSetter(0));
  for( size_t i=0; i < L.FragmentCount(); i++ )  {
    if( L.GetFragment(i).GetTag() != 0 )
      res.Add(L.GetFragment(i));
  }
  return res;
}
//..............................................................................
void TGXApp::SyncAtomAndBondVisiblity(short atom_type, bool show_a, bool show_b)  {
  AtomIterator ai(*this);
  while( ai.HasNext() )  {
    TXAtom& a = ai.Next();
    if( a.GetType() != atom_type )
      continue;
    if( atom_type == iHydrogenZ )  {
      bool vis = false;
      size_t nc = 0;
      for( size_t j=0; j < a.NodeCount(); j++ )  {
        if( a.Node(j).IsDeleted() )
          continue;
        nc++;
        if( a.Node(j).IsVisible() )  {
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
  BondIterator bi(*this);
  while( bi.HasNext() )  {
    TXBond& b = bi.Next();
    if( !b.A().IsVisible() || !b.B().IsVisible() )  {
      b.SetVisible(false);
      continue;
    }
    if( atom_type == iHydrogenZ )   {
      if( b.GetType() == sotHBond )
        b.SetVisible(show_b);
      else if( b.A().GetType() == atom_type || b.B().GetType() == atom_type )  {
        // there is always a special case...
        if( (b.A().GetType() == iQPeakZ || b.B().GetType() == iQPeakZ) &&
            !FQPeakBondsVisible )
        {
          b.SetVisible(false);
        }
        else
          b.SetVisible(show_a);
      }
    }
    else if( b.A().GetType() == atom_type || b.B().GetType() == atom_type )
      b.SetVisible(show_b);
  }
  //if( FXGrowLinesVisible )  {
  //  for( size_t i=0; i < XGrowLines.Count(); i++ )  {
  //    if( XGrowLines[i].SAtom()->GetType() == atom_type )
  //      XGrowLines[i].SetVisible(XAtoms[XGrowLines[i].SAtom()->GetTag()].IsVisible());
  //  }
  //}
}
//..............................................................................
void TGXApp::AllVisible(bool V)  {
  OnAllVisible.Enter(dynamic_cast<TBasicApp*>(this), NULL);
  if (!V) {
    AtomIterator ai(*this);
    while (ai.HasNext()) ai.Next().SetVisible(false);
    BondIterator bi(*this);
    while (bi.HasNext()) bi.Next().SetVisible(false);
  }
  else {
    AtomIterator ai(*this);
    while (ai.HasNext()) ai.Next().SetMasked(false);
    TAsymmUnit& au = XFile().GetAsymmUnit();
    for (size_t i=0; i < au.AtomCount(); i++)
      au.GetAtom(i).SetMasked(false);
    UpdateConnectivity();
    CenterView(true);
    GetLabels().ClearLabelMarks(lmiDefault);
    UpdateDuplicateLabels();
  }
  OnAllVisible.Exit(dynamic_cast<TBasicApp*>(this), NULL);
  Draw();
}
//..............................................................................
void TGXApp::Select(const vec3d& From, const vec3d& To )  {
  AtomIterator ai(*this);
  BondIterator bi(*this);
  while( ai.HasNext() )  {
    TXAtom& XA = ai.Next();
    if( XA.IsVisible() )  {
      vec3d Cnt = XA.crd() + GetRender().GetBasis().GetCenter();
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
  while( bi.HasNext() )  {
    TXBond& B = bi.Next();
    if( B.IsVisible() )  {
      vec3d Cnt = B.A().crd() + GetRender().GetBasis().GetCenter();
      Cnt *= GetRender().GetBasis().GetMatrix();
      Cnt *= GetRender().GetBasis().GetZoom();
      vec3d Cnt1 = B.B().crd() + GetRender().GetBasis().GetCenter();
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
  //for( size_t i=0; i < XReflections.Count(); i++ )  {
  //  TXReflection& XR = XReflections[i];
  //  if( XR.IsVisible() )  {
  //    vec3d Cnt = XR.GetCenter() + GetRender().GetBasis().GetCenter();
  //    Cnt *= GetRender().GetBasis().GetMatrix();
  //    Cnt *= GetRender().GetBasis().GetZoom();
  //    if( Cnt[0] < To[0] && Cnt[1] < To[1] &&
  //        Cnt[0] > From[0] && Cnt[1] > From[1] )  {
  //      if( !XR.IsSelected() )  
  //        GetRender().Select(XR);
  //    }
  //  }
  //}
  Draw();
}
//..............................................................................
bool TGXApp::Dispatch(int MsgId, short MsgSubId, const IEObject *Sender,
  const IEObject *Data, TActionQueue *)
{
  static bool ObjectsStored = false, LoadingFile = false, Disassembling = false;
  if( MsgId == ID_OnSelect )  {
    if( FGlMouse->IsSelectionEnabled() )  {
      const TSelectionInfo* SData = dynamic_cast<const TSelectionInfo*>(Data);
      if(  !(SData->From == SData->To) )
        Select(SData->From, SData->To);
    }
  }
  else if( MsgId == ID_OnUniq || MsgId == ID_OnGrow )  {
    if( MsgSubId == msiEnter )  {
      FGlRender->ClearObjects();
      ClearXObjects();
    }
    else if( MsgSubId == msiExit )  {
      if( OverlayedXFileCount() != 0 )  {
        AlignOverlayedXFiles();
        UpdateBonds();
      }
      if( ZoomAfterModelBuilt )
        CenterView(true);
    }
  }
  else if( MsgId == ID_OnFileLoad )  {
    if( MsgSubId == msiEnter )  {
      SelectionCopy[0].Clear();
      StoreGroup(GetSelection(), SelectionCopy[0]);
      StoreLabels();
      ClearLines();
      LoadingFile = true;
    }
    else if( MsgSubId == msiExit )
      LoadingFile = false;
  }
  else if( MsgId == ID_OnFileClose )  {
    if( MsgSubId == msiExit )  {
      ClearLabels();
      ClearLines();
      XGrid().Clear();
      CreateObjects(false);
      GetRender().SetZoom(GetRender().CalcZoom());
    }
  }
  else if( MsgId == ID_OnDisassemble ) {
    if( MsgSubId == msiExit )  {
      CreateRings();
      CreateObjects(false);
      Disassembling = false;
      UpdateDuplicateLabels();
    }
    else if( MsgSubId == msiEnter )  {  // backup the selection
      if( ObjectsStored )
        ObjectsStored = false;
      else  if( !LoadingFile )  {
        SelectionCopy[0].Clear();
        StoreGroup(GetSelection(), SelectionCopy[0]);
        StoreLabels();
      }
      GetRender().ClearGroups();
      GetRender().ClearSelection();
      Rings.Clear();
      Disassembling = true;
    }
  }
  else if( MsgId == ID_OnClear ) {
    if( MsgSubId == msiEnter )  {  // backup the selection
      if( !LoadingFile && !Disassembling )  {
        SelectionCopy[0].Clear();
        StoreGroup(GetSelection(), SelectionCopy[0]);
        StoreLabels();
        ObjectsStored = true;
      }
      FGlRender->ClearObjects();
      ClearXObjects();
    }
    else if( MsgSubId == msiExit )  {
      //GetRender().SetBasis(basis);
    }
  }
  return false;
}
//..............................................................................
ConstPtrList<TCAtom> TGXApp::GetSelectedCAtoms(bool Clear)  {
  TCAtomPList rv(GetSelectedXAtoms(Clear),
    FunctionAccessor::MakeConst(&TXAtom::CAtom));
  return rv;  // GCC needs this code!
}
//..............................................................................
void TGXApp::CopySelection() const {
  TDataItem root(NULL, "root"),
    &di = root.AddItem("objects"),
    &atoms = di.AddItem("atoms"),
    &bonds = di.AddItem("bonds");
  olxstr fid = XFile().GetFileName() + XFile().GetLastLoaderSG().GetName();
  di.SetValue(MD5::Digest(fid));
  TGlGroup &sel = GetRender().GetSelection();
  for (size_t i=0; i < sel.Count(); i++) {
    TXAtom *a = dynamic_cast<TXAtom *>(&sel[i]);
    if (a != NULL) {
      a->GetRef().ToDataItem(atoms.AddItem(atoms.ItemCount()+1));
    }
    else {
      TXBond *b = dynamic_cast<TXBond *>(&sel[i]);
      if (b != NULL) {
        b->GetRef().ToDataItem(bonds.AddItem(bonds.ItemCount()+1));
      }
    }
  }
  TEStrBuffer bf;
  di.SaveToStrBuffer(bf);
  ToClipboard(bf.ToString());
}
//..............................................................................
void TGXApp::PasteSelection() {
  olxstr content;
#if defined(__WXWIDGETS__)
  if( wxTheClipboard->Open() )  {
    if (wxTheClipboard->IsSupported(wxDF_TEXT) )  {
      wxTextDataObject data;
      wxTheClipboard->GetData(data);
      content = data.GetText();
    }
    wxTheClipboard->Close();
  }
#elif __WIN32__
  if (OpenClipboard(NULL)) {
    HGLOBAL data=NULL;
    bool unicode=false;
    if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
      data = GetClipboardData(CF_UNICODETEXT);
      unicode = true;
    }
    else if (IsClipboardFormatAvailable(CF_TEXT)) {
      data = GetClipboardData(CF_TEXT);
    }
    if (data == NULL) {
      RestoreSelection();
    }
    else {
      uint8_t *str = (uint8_t *)GlobalLock(data);
      if (str) {
        if (unicode)
          content = olxwstr((const wchar_t *)str);
        else
          content = olxcstr((const char *)str);
        GlobalUnlock(data);
      }
    }
    CloseClipboard();
  }
#endif
  if (!content.IsEmpty()) {
    try {
      TDataItem root(NULL, "root");
      root.LoadFromString(0, content, NULL);
      TDataItem &di = root.GetItemByName("objects");
      olxstr fid = XFile().GetFileName() + XFile().GetLastLoaderSG().GetName();
      if (di.GetValue() == MD5::Digest(fid)) {
        TDataItem &atoms = di.GetItemByName("atoms"),
          &bonds = di.GetItemByName("bonds");
        SelectAll(false);
        for (size_t i=0; i < atoms.ItemCount(); i++) {
          TSAtom *sa = XFile().GetLattice().GetAtomRegistry().Find(
            TSAtom::Ref(atoms.GetItemByIndex(i)));
          if (sa != NULL) {
            GetRender().Select(*dynamic_cast<TXAtom *>(sa), true);
          }
        }
        for (size_t i=0; i < bonds.ItemCount(); i++) {
          TSBond *sb = XFile().GetLattice().GetAtomRegistry().Find(
            TSBond::Ref(bonds.GetItemByIndex(i)));
          if (sb != NULL) {
            GetRender().Select(*dynamic_cast<TXBond *>(sb), true);
          }
        }
      }
      else {
        RestoreSelection();
      }
    }
    catch(const TExceptionBase &e) {
      RestoreSelection();
    }
  }
  Draw();
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
ConstPtrList<TXAtom> TGXApp::GetSelectedXAtoms(bool Clear)  {
  TPtrList<TGlGroup> S;
  S.Add(GetSelection());
  TXAtomPList rv;
  TXBondPList bonds;
  for( size_t i=0; i < S.Count(); i++ )  {
    TGlGroup& Sel = *S[i];
    for( size_t j=0; j < Sel.Count(); j++ )  {
      AGDrawObject& GO = Sel[j];
      if( GO.IsGroup() )  // another group
        S.Add((TGlGroup&)GO);
      else if( EsdlInstanceOf(GO, TXAtom) )
        rv.Add((TXAtom&)GO);
      else if (EsdlInstanceOf(GO, TXBond))
        bonds << (TXBond&)GO;
    }
  }
  if (rv.IsEmpty()) {
    for (size_t i=0; i < bonds.Count(); i++)
      rv << bonds[i]->A() << bonds[i]->B();
  }
  if( Clear )
    SelectAll(false);
  return rv;
}
//..............................................................................
ConstPtrList<TCAtom> TGXApp::CAtomsByType(const cm_Element& AI)  {
  return &olx_list_filter::Filter(
    XFile().GetLattice().GetAsymmUnit().GetAtoms(),
    *(new TCAtomPList),
    olx_alg::olx_and(
      olx_alg::olx_not(TCAtom::FlagsAnalyser(catom_flag_Deleted)),
      TCAtom::TypeAnalyser(AI)));
}
//..............................................................................
ConstPtrList<TXAtom> TGXApp::XAtomsByType(const cm_Element& AI, bool FindHidden) {
  //ListFilter::Filter(XAtoms, res,
  //  olx_alg::and(
  //    olx_alg::Bool(!FindHidden),
  //    olx_alg::and(
  //      AGDrawObject::FlagsAnalyser(sgdoVisible),
  //      TSAtom::TypeAnalyser<>(AI))));
  TXAtomPList l;
  AtomIterator ai(*this);
  l.SetCapacity(ai.count);
  while( ai.HasNext() )  {
    TXAtom& xa = ai.Next();
    if( !FindHidden && !xa.IsVisible() )  continue;
    if( xa.GetType() == AI )
      l.Add(xa);
  }
  return l;
}
//..............................................................................
ConstPtrList<TCAtom> TGXApp::CAtomsByMask(const olxstr &StrMask, int Mask)  {
  if( StrMask.Length() > 32 )
    throw TInvalidArgumentException(__OlxSourceInfo, "mask is too long");
  olxstr Name = StrMask.ToUpperCase();
  TAsymmUnit& AU = XFile().GetLattice().GetAsymmUnit();
  TCAtomPList l;
  for( size_t i=0; i < AU.AtomCount(); i++ )  {
    TCAtom& CA = AU.GetAtom(i);
    if( CA.IsDeleted() )  continue;
    if( CA.GetLabel().Length() != Name.Length() )  continue;
    olxstr Tmp = CA.GetLabel().ToUpperCase();
    bool found = true;
    for( size_t j=0; j < Name.Length(); j++ )  {
      if( !(Mask & (0x0001<<j)) )  {
        if( Name.CharAt(j) != Tmp.CharAt(j) )  {
          found = false;
          break;
        }
      }
    }
    if( found )
      l.Add(CA);
  }
  return l;
}
//..............................................................................
void TGXApp::GrowAtom(TXAtom *XA, bool Shell, TCAtomPList* Template)  {
  FXFile->GetLattice().GrowAtom(*XA, Shell, Template);
}
//..............................................................................
void TGXApp::Grow(const TXAtomPList& atoms, const smatd_list& matrices)  {
  FXFile->GetLattice().GrowAtoms(
    TCAtomPList(atoms, FunctionAccessor::MakeConst(&TXAtom::CAtom)),
    matrices);
}
//..............................................................................
ConstPtrList<TXAtom> TGXApp::GetXAtoms(const olxstr& AtomName) {
  TXAtomPList res;
  if (AtomName.StartsFrom("#c")) {  // TCAtom.Id
    const size_t id = AtomName.SubStringFrom(2).ToSizeT();
    AtomIterator ai(*this);
    while (ai.HasNext()) {
      TXAtom& xa = ai.Next();
      if (xa.CAtom().GetId() == id && xa.IsVisible())
        res.Add(xa);
    }
  }
  else if (AtomName.StartsFrom("#s"))  {  // SAtom.LatId
    const size_t id = AtomName.SubStringFrom(2).ToSizeT();
    AtomIterator ai(*this);
    while (ai.HasNext()) {
      TXAtom& xa = ai.Next();
      if (xa.GetOwnerId() == id && xa.IsVisible()) {
        res.Add(xa);
        break;
      }
    }
  }
  else {
    size_t idx = AtomName.IndexOf('_');
    olxstr label;
    static const int32_t mxi32 = (int32_t)((uint32_t)(~0) >> 1);
    int32_t resi_n = mxi32, part_n = mxi32;
    if (idx != InvalidIndex) {
      olxstr sfx = AtomName.SubStringFrom(idx + 1);
      if (sfx.IsNumber()) {
        label = AtomName.SubStringTo(idx);
        resi_n = AtomName.SubStringFrom(idx + 1).ToInt();
      }
      else if (sfx.Length() == 1 && olxstr::o_isalpha(sfx.CharAt(0))) {
        label = AtomName.SubStringTo(idx);
        part_n = (sfx.CharAt(0) - 'a') + 1;
      }
    }
    else {
      label = AtomName;
    }
    TAsymmUnit &au = XFile().GetAsymmUnit();
    AtomIterator ai(*this);
    while (ai.HasNext()) {
      TXAtom& xa = ai.Next();
      if (!xa.IsVisible()) continue;
      bool le = xa.GetLabel().Equalsi(label);
      if (!le) continue;
      if (resi_n != mxi32) {
        if (resi_n == au.GetResidue(xa.CAtom().GetResiId()).GetNumber())
          res.Add(xa);
      }
      else if (part_n != mxi32) {
        if (part_n == xa.CAtom().GetPart())
          res.Add(xa);
      }
    }
  }
  return res;
}
//..............................................................................
ConstPtrList<TXBond> TGXApp::GetXBonds(const olxstr& BondName)  {
  TXBondPList res;
  if( BondName.StartsFrom("#t") )  {  // SBond.LatId
    size_t id = BondName.SubStringFrom(2).ToSizeT();
    if (id < XFile().GetLattice().GetObjects().bonds.Count()) {
      res.Add(
        dynamic_cast<TXBond &>(XFile().GetLattice().GetObjects().bonds[id]));
    }
  }
  else  {
    BondIterator bi(*this);
    while( bi.HasNext())  {
      TXBond& xb = bi.Next();
      if( xb.GetCollectionName().Equalsi(BondName) && xb.IsVisible() )
        res.Add(xb);
    }
  }
  return res;
}
//..............................................................................
TXAtom* TGXApp::GetXAtom(const olxstr& AtomName, bool clearSelection) {
  if (AtomName.Equalsi("sel")) {
    TXAtomPList L = GetSelectedXAtoms(clearSelection);
    if (L.Count() != 1) return NULL;
    return L[0];
  }
  else if (AtomName.StartsFrom("#s")) {  // SAtom.LatId
    const size_t id = AtomName.SubStringFrom(2).ToSizeT();
    if (id < XFile().GetLattice().GetObjects().atoms.Count()) {
      return &dynamic_cast<TXAtom&>(XFile().GetLattice().GetObjects().atoms[id]);
    }
  }
  AtomIterator ai(*this);
  while (ai.HasNext()) {
    TXAtom& xa = ai.Next();
    if (xa.GetLabel().Equalsi(AtomName))
      return &xa;
  }
  return NULL;
}
//..............................................................................
ConstPtrList<TXAtom> TGXApp::XAtomsByMask(const olxstr &StrMask, int Mask)  {
  if( StrMask.Length() > 32 )
    throw TInvalidArgumentException(__OlxSourceInfo, "mask is too long");
  olxstr Tmp, Name(StrMask.ToUpperCase());
  TXAtomPList rv;
  AtomIterator ai(*this);
  while( ai.HasNext() )  {
    TXAtom& XA = ai.Next();
    if( !XA.IsVisible() )  continue;
    if( XA.GetLabel().Length() != Name.Length() )  continue;
    Tmp = XA.GetLabel().ToUpperCase();
    bool found = true;
    for( size_t j=0; j < Name.Length(); j++ )  {
      if( (Mask & (0x0001<<j)) == 0 )  {
        if( Name.CharAt(j) != Tmp.CharAt(j) )  {
          found = false;
          break;
        }
      }
    }
    if( found )
      rv.Add(XA);
  }
  return rv;
}
//..............................................................................
bool TGXApp::FindSAtoms(const olxstr& condition, TSAtomPList& res,
  bool ReturnAll, bool ClearSelection)
{
  TPtrList<TXAtom> al = FindXAtoms(condition, ReturnAll, ClearSelection);
  if (!OverlayedXFiles.IsEmpty()) {
    TLattice &latt = XFile().GetLattice();
    for (size_t i=0; i < al.Count(); i++) {
      if (((TSAtom*)al[i])->GetParent() != latt)
        al[i] = NULL;
    }
    al.Pack();
  }
  res.AddList(al, StaticCastAccessor<TSAtom>());
  return !al.IsEmpty();
}
//..............................................................................
ConstPtrList<TXAtom> TGXApp::FindXAtoms(const olxstr &Atoms, bool getAll,
  bool ClearSelection, bool FindHidden)
{
  TXAtomPList rv;
  if( Atoms.IsEmpty() )  {  // return selection/all atoms
    TGlGroup& sel = GetRender().GetSelection();
    for( size_t i=0; i < sel.Count(); i++ )  {
      if( EsdlInstanceOf(sel[i], TXAtom) )
        rv.Add((TXAtom&)sel[i]);
    }
    if( !rv.IsEmpty() )  {
      if (ClearSelection)
        SelectAll(false);
      return rv;
    }
    if (getAll) {
      AtomIterator ai(*this);
      rv.SetCapacity(ai.count);
      while( ai.HasNext() )  {
        TXAtom& xa = ai.Next();
        if( xa.IsDeleted() )  continue; 
        if( !FindHidden && !xa.IsVisible() ) continue;
        rv.Add(xa);
      }
    }
  }
  else  {
    TStrList Toks(Atoms, ' ');
    //TXAtom *XAFrom, *XATo;
    for( size_t i = 0; i < Toks.Count(); i++ )  {
      olxstr Tmp = Toks[i];
      if( Tmp.Equalsi("sel") )  {
        rv += GetSelectedXAtoms(ClearSelection);
        continue;
      }
      if( Tmp.Equalsi("to") || Tmp.Equalsi(">") )  {
        if( (i+1) < Toks.Count() && !rv.IsEmpty() )  {
          i++;
          TXAtom* XATo = NULL;
          if( Toks[i].Equalsi("end") );
          else  {
            XATo = GetXAtom(Toks[i], ClearSelection);
            if( XATo == NULL )
              throw TInvalidArgumentException(__OlxSourceInfo, "\'to\' atoms is undefined");
          }
          TXAtom* XAFrom = rv.GetLast();
          AtomIterator ai(*this);
          while( ai.HasNext() )  {
            TXAtom& XA = ai.Next();
            if( XA.IsDeleted() ) continue;
            if( !FindHidden && !XA.IsVisible() )  continue;
            if( XATo != NULL )  {
              if( CompareStr(XA.GetLabel(), XAFrom->GetLabel(), true) > 0 &&
                CompareStr(XA.GetLabel(), XATo->GetLabel(), true) < 0 )
              {
                  rv.Add(XA);
              }
            }
            else  {
              if( CompareStr(XA.GetLabel(), XAFrom->GetLabel(), true) > 0 &&
                XA.GetType() == XAFrom->GetType() )
              {
                rv.Add(XA);
              }
            }
          }
          if( XATo != NULL )
            rv.Add(XATo);
        }
      }
      if( Tmp.CharAt(0) == '$' )  {
        SortedElementPList elms = TAtomReference::DecodeTypes(
          Tmp.SubStringFrom(1), XFile().GetAsymmUnit());
        for( size_t ei=0; ei < elms.Count(); ei++ )
          rv += XAtomsByType(*elms[ei], FindHidden);
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
        rv += XAtomsByMask(Tmp, mask);
        continue;
      }
      rv += GetXAtoms(Tmp);
    }
  }
  return rv;
}
//..............................................................................
ConstPtrList<TXAtom> TGXApp::FindXAtoms(const TStrObjList &Cmds, bool GetAll,
  bool unselect)
{
  TXAtomPList atoms;
  if( Cmds.IsEmpty() )  {
    atoms.AddList(
      FindXAtoms(EmptyString(), GetAll, EsdlInstanceOf(GetSelection(), TGlGroup)
      ? unselect : false));
  }
  else
    atoms = FindXAtoms(Cmds.Text(' '), unselect);
  return atoms;
}
//..............................................................................
void TGXApp::CheckQBonds(TXAtom& XA)  {
  TXBondPList XB;
  for(size_t i=0; i < XA.BondCount(); i++ )
    XB.Add(XA.Bond(i));
  for(size_t i=0; i < XB.Count(); i++ )  {
    TXBond* xb = XB[i];
    /* check if any of the atoms still a Q-peak */
    if( xb->A().GetType() == iQPeakZ || xb->B().GetType() == iQPeakZ )  continue;
    /* check that the covalent bond really exists before showing it */
    xb->SetVisible(FXFile->GetLattice().GetNetwork().CBondExistsQ(xb->A(),
                    xb->B(), xb->QLength()));
  }
}
//..............................................................................
TUndoData* TGXApp::ChangeSuffix(const TXAtomPList& xatoms, const olxstr &To,
  bool CheckLabels)
{
  TNameUndo *undo = new TNameUndo(
    new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoName)));
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    const olxstr oldL = xatoms[i]->GetLabel();
    olxstr newL = xatoms[i]->GetType().symbol;
    for( size_t j=newL.Length(); j < oldL.Length(); j++ )  {
      if( olxstr::o_isdigit(oldL.CharAt(j)) )
        newL << oldL[j];
      else
        break;
    }
    newL << To;
    if( newL == oldL )  continue;

    if( CheckLabels )
      newL = XFile().GetAsymmUnit().CheckLabel(&xatoms[i]->CAtom(), newL);
    xatoms[i]->CAtom().SetLabel(newL, false);
    undo->AddAtom(xatoms[i]->CAtom(), oldL);
  }
  return undo;
}
//..............................................................................
TUndoData* TGXApp::Name(TXAtom& XA, const olxstr& _Name, bool CheckLabel)  {
  olxstr Name = _Name;
  bool checkBonds = (XA.GetType() == iQPeakZ);
  cm_Element* elm = XElementLib::FindBySymbolEx(Name);
  if( elm == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid element");
  TNameUndo *undo = new TNameUndo(
    new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoName)));
  olxstr oldL = XA.GetLabel();
  bool recreate = ((elm == NULL) ? true : XA.GetType() != *elm);
  if( Name.Length() == elm->symbol.Length() && oldL.StartsFromi(XA.GetType().symbol) )
    Name = elm->symbol + oldL.SubStringFrom(XA.GetType().symbol.Length());
  XA.CAtom().SetLabel(
    CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA.CAtom(), Name) : Name, false);
  if( oldL != XA.GetLabel() || *elm != XA.GetType() )
    undo->AddAtom(XA.CAtom(), oldL);
  XA.CAtom().SetType(*elm);
  // Dima's complaint - leave all in for manual naming
  //NameHydrogens(XA.Atom(), undo, CheckLabel);
  if( checkBonds )  CheckQBonds(XA);
  if( recreate )  {
    AGDObjList objects = XA.GetPrimitives().GetObjects();
    XA.GetPrimitives().ClearObjects();
    for (size_t i=0; i < objects.Count(); i++) {
      objects[i]->Create();
    }
    SynchroniseBonds(TXAtomPList(objects, DynamicCastAccessor<TXAtom>()));
  }
  UpdateDuplicateLabels();
  return undo;
}
//..............................................................................
TUndoData* TGXApp::Name(const olxstr &From, const olxstr &To, bool CheckLabel,
  bool ClearSelection, bool NameResi)
{
  TXAtomPList Atoms;
  TNameUndo *undo;
  TXAtom* XA = GetXAtom(From, false);
  if (XA != NULL) {
    Atoms << XA;
    if (ClearSelection) SelectAll(false);
    undo = dynamic_cast<TNameUndo *>(Name(*XA, To, CheckLabel));
  }
  else {
    undo = new TNameUndo(
      new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoName)));
    Atoms = FindXAtoms(From, ClearSelection);
    TXAtomPList ChangedAtoms;
    // leave only AU atoms
    Atoms.ForEach(ACollectionItem::IndexTagSetter());
    Atoms.Pack(ACollectionItem::IndexTagAnalyser());
    if (From.Equalsi("sel") && To.IsNumber()) {
      int j = To.ToInt();
      for (size_t i=0; i < Atoms.Count(); i++) {
        XA = Atoms[i];
        bool checkBonds = (XA->GetType() == iQPeakZ);
        const olxstr Tmp = XA->GetLabel();
        olxstr NL = XA->GetType().symbol;
        NL << j++;
        const olxstr oldL = XA->GetLabel();
        XA->CAtom().SetLabel(
          CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->CAtom(), NL) : NL,
          false);
        undo->AddAtom(XA->CAtom(), oldL);
        NameHydrogens(*XA, undo, CheckLabel);
        if (checkBonds)
          CheckQBonds(*XA);
      }
    }
    else if (From.CharAt(0) == '$') {
      const cm_Element* elm = XElementLib::FindBySymbolEx(
        To.CharAt(0) == '$' ? To.SubStringFrom(1) : To);
      if (elm != NULL) {  // change type
        for (size_t i=0; i < Atoms.Count(); i++) {
          XA = Atoms[i];
          const bool checkBonds = (XA->GetType() == iQPeakZ);
          const olxstr Tmp = XA->GetLabel();
          olxstr NL = elm->symbol;
          NL << Tmp.SubStringFrom(From.Length()-1);
          bool recreate = XA->GetType() != *elm;
          const olxstr oldL = XA->GetLabel();
          XA->CAtom().SetLabel(
            CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->CAtom(), NL) : NL, false);
          undo->AddAtom(XA->CAtom(), oldL);
          XA->CAtom().SetType(*elm);
          NameHydrogens(*XA, undo, CheckLabel);
          if (recreate) {
            ChangedAtoms.Add(XA);
            if (checkBonds)
              CheckQBonds(*XA);
          }
        }
      }
      else if (To.IsNumber()) {  // change number
        int j = To.ToInt();
        for (size_t i=0; i < Atoms.Count(); i++) {
          XA = Atoms[i];
          bool checkBonds = (XA->GetType() == iQPeakZ);
          const olxstr Tmp = XA->GetLabel();
          olxstr NL = XA->GetType().symbol;
          NL << j++;
          const olxstr oldL = XA->GetLabel();
          XA->CAtom().SetLabel(
            CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->CAtom(), NL) : NL,
            false);
          undo->AddAtom(XA->CAtom(), oldL);
          NameHydrogens(*XA, undo, CheckLabel);
          if( checkBonds )
            CheckQBonds(*XA);
        }
      }
      else
        throw TFunctionFailedException(__OlxSourceInfo, "wrong syntax");
    }
    else {  // C2? to C3? ; Q? to Ni? ...
      const cm_Element* elm = XElementLib::FindBySymbolEx(To);
      if (elm == NULL)
        throw TFunctionFailedException(__OlxSourceInfo, "wrong syntax");
      const bool to_element = XElementLib::IsElement(To);
      for (size_t i=0; i < Atoms.Count(); i++) {
        XA = (TXAtom*)Atoms[i];
        const bool checkBonds = (XA->GetType() == iQPeakZ);
        const olxstr Tmp = XA->GetLabel();
        olxstr NL = To;
        const bool recreate = XA->GetType() != *elm;
        if (to_element) {
          if (XA->GetLabel().StartsFrom(XA->GetType().symbol))
            NL << XA->GetLabel().SubStringFrom(XA->GetType().symbol.Length());
        }
        else {
          for (size_t j=0; j < NL.Length(); j++) {
            if (NL.CharAt(j) == '?') {
              size_t qmi = 0;
              qmi = From.FirstIndexOf('?', qmi);
              if (qmi != InvalidIndex) {
                NL[j] = Tmp.CharAt(qmi);
                qmi++;
              }
              else
                NL[j] = '_';
            }
          }
        }
        const olxstr oldL = XA->GetLabel();
        XA->CAtom().SetLabel(
          CheckLabel ? XFile().GetAsymmUnit().CheckLabel(&XA->CAtom(), NL) : NL,
          false);
        undo->AddAtom(XA->CAtom(), oldL);
        XA->CAtom().SetType(*elm);
        NameHydrogens(*XA, undo, CheckLabel);
        if (recreate) {
          ChangedAtoms.Add(XA);
          if (checkBonds)
            CheckQBonds(*XA);
        }
      }
    }
    AGDObjList objects;
    for (size_t i=0; i < ChangedAtoms.Count(); i++) {
      XA = ChangedAtoms[i];
      objects.AddList(XA->GetPrimitives().GetObjects());
      XA->GetPrimitives().ClearObjects();
    }
    for (size_t i=0; i < objects.Count(); i++) {
      objects[i]->Create();
    }
    SynchroniseBonds(TXAtomPList(objects, DynamicCastAccessor<TXAtom>()));
    UpdateDuplicateLabels();
  }
  if (NameResi) {
    undo->AddAction(SynchroniseResidues(Atoms));
  }
  return undo;
}
//..............................................................................
TUndoData* TGXApp::SynchroniseResidues(const TXAtomPList &reference) {
  TNameUndo *undo = new TNameUndo(
    new TUndoActionImplMF<TGXApp>(this, &GxlObject(TGXApp::undoName)));
  olxdict<olxstr, TTypeList<TCAtomPList>, olxstrComparator<true> > groups;
  TAsymmUnit &au = XFile().GetAsymmUnit();
  for (size_t i=1; i < au.ResidueCount(); i++) {
    TResidue &r = au.GetResidue(i);
    TCAtomPList &l = groups.Add(r.GetClassName()).AddNew();
    l.SetCapacity(r.Count());
    for (size_t j=0; j < r.Count(); j++) {
      if (r[j].IsDeleted()) continue;
      l.Add(r[j])->SetTag((index_t)l.Count());
    }

  }
  for (size_t i=0; i < reference.Count(); i++) {
    TCAtom &a = reference[i]->CAtom();
    if (a.GetResiId() == 0) continue;
    TTypeList<TCAtomPList> &g = groups[au.GetResidue(a.GetResiId()).GetClassName()];
    size_t idx = InvalidIndex;
    for (size_t j=0; j < g.Count(); j++) {
      if ((size_t)a.GetTag() < g[j].Count() &&
          g[j][a.GetTag()]->GetId() == a.GetId())
      {
        idx = j;
        break;
      }
    }
    if (idx == InvalidIndex) { // how could it?
      continue;
    }
    for (size_t j=0; j < g.Count(); j++) {
      if (j == idx || g[j].Count() != g[idx].Count())
        continue;
      if (g[j][a.GetTag()]->GetLabel() != a.GetLabel()) {
        undo->AddAtom(*g[j][a.GetTag()], g[j][a.GetTag()]->GetLabel());
        g[j][a.GetTag()]->SetLabel(a.GetLabel());
      }
    }
  }
  return undo;
}
//..............................................................................
int XAtomLabelSort(const TXAtom &I1, const TXAtom &I2)  {
  int v = TCAtomComparator::Compare(I1.CAtom(), I2.CAtom());
  return (v == 0) ? TCAtom::CompareAtomLabels(I1.GetLabel(), I2.GetLabel()) : v;
}
//..............................................................................
void TGXApp::InfoList(const olxstr &Atoms, TStrList &Info, bool sort,
  int precision, bool cart)
{
  TTypeList<AnAssociation2<vec3d, TCAtom*> > atoms;
  bool have_q = false;
  if( XFile().GetLattice().IsGenerated() )  {
    TXAtomPList AtomsList = FindXAtoms(Atoms, false);
    for(size_t i = 0; i < AtomsList.Count(); i++ )  {
      atoms.Add(Association::New(AtomsList[i]->ccrd(), &AtomsList[i]->CAtom()));
      if( AtomsList[i]->GetType() == iQPeakZ )
        have_q = true;
    }
  }
  else {
    TCAtomPList catoms = FindCAtoms(Atoms, false);
    for( size_t i=0; i < catoms.Count(); i++ )  {
      atoms.Add(Association::New(catoms[i]->ccrd(), catoms[i]));
      if( catoms[i]->GetType() == iQPeakZ )
        have_q = true;
    }
  }
  if (olx_abs(precision) > 10)
    precision = -10;
  const TAsymmUnit &au = XFile().GetAsymmUnit();
  TTTable<TStrList> Table(atoms.Count(), have_q ? 12 : 11);
  Table.ColName(0) = "Atom";
  Table.ColName(1) = "Type";
  Table.ColName(2) = "X";
  Table.ColName(3) = "Y";
  Table.ColName(4) = "Z";
  Table.ColName(5) = "Ueq";
  Table.ColName(6) = "Um";
  Table.ColName(7) = "Uvol";
  Table.ColName(8) = "ChemOccu";
  Table.ColName(9) = "R-bond";
  Table.ColName(10) = "R-VdW";
  if( have_q )
    Table.ColName(11) = "Peak";
  for(size_t i = 0; i < atoms.Count(); i++ )  {
    const TCAtom& A = *atoms[i].GetB();
    Table[i][0] = A.GetLabel();
    Table[i][1] = A.GetType().symbol;
    vec3d c = cart ? au.Orthogonalise(atoms[i].GetA()) : atoms[i].GetA();
    for (int ci=0; ci < 3; ci++)
      Table[i][2+ci] = olxstr::FormatFloat(precision, c[ci]);
    Table[i][5] = olxstr::FormatFloat(3, A.GetUiso());
    if( A.GetEllipsoid() != NULL )  {
      Table[i][6] << olxstr::FormatFloat(3,
          pow(A.GetEllipsoid()->GetSX()*A.GetEllipsoid()->GetSY()*
          A.GetEllipsoid()->GetSZ(), 2./3));
      Table[i][7] << olxstr::FormatFloat(3,
          A.GetEllipsoid()->GetSX()*A.GetEllipsoid()->GetSY()*
          A.GetEllipsoid()->GetSZ()*4*M_PI/3);
    }
    else  {
      Table[i][6] << '.';
      Table[i][7] << olxstr::FormatFloat(3,
          pow(A.GetUiso(), 3./2)*4*M_PI/3);
    }
    Table[i][8] = olxstr::FormatFloat(3, A.GetChemOccu());
    Table[i][9] = A.GetConnInfo().r;
    Table[i][10] = A.GetType().r_vdw;
    if( have_q )  {
      if( A.GetType() == iQPeakZ )
        Table[i][11] = olxstr::FormatFloat(3, A.GetQPeak());
      else
        Table[i][11] = '-';
    }
  }
  Table.CreateTXTList(Info, "Atom information", true, true, ' ');
}
//..............................................................................
TXGlLabel& TGXApp::CreateLabel(const TXAtom& a, uint16_t FontIndex)  {
  TXGlLabel& l = CreateLabel(a.crd(), a.GetLabel(), FontIndex);
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
  TXLine *XL = new TXLine(*FGlRender,
    Name.IsEmpty() ? olxstr("TXLine") << LooseObjects.Count() : Name,
    base, edge);
  XL->Create();
  return Lines.Add(XL);
}
//..............................................................................
AGDrawObject* TGXApp::FindLooseObject(const olxstr &Name)  {
  for( size_t i=0; i < LooseObjects.Count(); i++ )
    if( LooseObjects[i]->GetPrimitives().GetName().Equalsi(Name) )
      return LooseObjects[i];
  return NULL;
}
//..............................................................................
TDUserObj* TGXApp::FindUserObject(const olxstr &Name)  {
  for( size_t i=0; i < UserObjects.Count(); i++ )
    if( UserObjects[i].GetPrimitives().GetName().Equalsi(Name) )
      return &UserObjects[i];
  return NULL;
}
//..............................................................................
TSPlane *TGXApp::TmpPlane(const TXAtomPList* atoms, double weightExtent)  {
  TSAtomPList SAtoms;
  if( atoms != NULL )
    SAtoms.Assign(*atoms);
  else  {
    AtomIterator ai(*this);
    SAtoms.SetCapacity(ai.count);
    while( ai.HasNext() )
      SAtoms.Add(ai.Next());
  }
  if( SAtoms.Count() < 3 )  return NULL;
  return XFile().GetLattice().TmpPlane(SAtoms, weightExtent);
}
//..............................................................................
TXPlane *TGXApp::AddPlane(const olxstr &name_, const TXAtomPList &Atoms,
  bool regular, double weightExtent)
{
  if( Atoms.Count() < 3 )  return NULL;
  TSPlanePList planes = XFile().GetLattice().NewPlane(
    TSAtomPList(Atoms), weightExtent, regular);
  size_t pi=0;
  olxstr name = name_;
  for( size_t i=0; i < planes.Count(); i++ ) {
    TXPlane * p =static_cast<TXPlane*>(planes[i]);
    if (!p->IsVisible())
      p->SetVisible(true);
    if (name.IsEmpty())
      name = olxstr("TXPlane") << planes[i]->GetDefId();
    p->Create(name);
    if (&planes[i]->GetAtom(0) == Atoms[0]) {
      pi = i;
    }
  }
  return planes.IsEmpty() ? NULL : static_cast<TXPlane*>(planes[pi]);
}
//..............................................................................
TXPlane *TGXApp::FindPlane(const olxstr &PlaneName)  {
  PlaneIterator pi(*this);
  while( pi.HasNext() )  {
    TXPlane& p = pi.Next();
    if( p.GetPrimitives().GetName().Equalsi(PlaneName) )
      return &p;
  }
  return NULL;
}
//..............................................................................
void TGXApp::DeletePlane(TXPlane* plane)  {
  plane->SetDeleted(true);
}
//..............................................................................
void TGXApp::ClearPlanes()  {
  PlaneIterator pi(*this);
  while( pi.HasNext() )
    pi.Next().SetDeleted(true);
}
//..............................................................................
ConstPtrList<TXAtom> TGXApp::AddCentroid(const TXAtomPList& Atoms)  {
  if( Atoms.Count() < 2 )  return new TXAtomPList;
  TXAtomPList centroids(
    XFile().GetLattice().NewCentroid(TSAtomPList(Atoms)),
    StaticCastAccessor<TXAtom>());
  for( size_t i=0; i < centroids.Count(); i++ )  {
    XFile().GetRM().Conn.Disconnect(centroids[i]->CAtom());
    centroids[i]->Create();
    centroids[i]->Params()[0] = centroids[i]->GetType().r_pers;
  }
  if (FLabels->IsVisible())
    FLabels->Init();
  return centroids;
}
//..............................................................................
void TGXApp::AdoptAtoms(const TAsymmUnit& au, TXAtomPList& atoms, TXBondPList& bonds) {
  TLattice latt(*(new SObjectProvider));
  latt.GetAsymmUnit().SetRefMod(au.GetRefMod());
  latt.GetAsymmUnit().Assign(au);
  latt.GetAsymmUnit()._UpdateConnInfo();
  latt.Init();
  vec3d cnt1, cnt2;
  double R1, R2;
  CalcLatticeRandCenter(XFile().GetLattice(), R1, cnt1);
  CalcLatticeRandCenter(latt, R2, cnt2);
  const vec3d right_shift = FGlRender->GetBasis().GetMatrix()*vec3d(1, 0, 0);
  const size_t ac = XFile().GetLattice().GetObjects().atoms.Count();
  const size_t bc = XFile().GetLattice().GetObjects().bonds.Count();
  const vec3d shift = cnt1-cnt2+right_shift*(R1+R2);
  for( size_t i=0; i < latt.GetObjects().atoms.Count(); i++ )  {
    TSAtom& sa = latt.GetObjects().atoms[i];
    sa.crd() += shift;
  }
  XFile().GetLattice().AddLatticeContent(latt);
  if( FLabels->IsVisible() )
    FLabels->Clear();
  ObjectCaster<TSAtom, TXAtom> cas = XFile().GetLattice().GetObjects().atoms.GetAccessor<TXAtom>();
  for( size_t i=ac; i < cas.Count(); i++ )  {
    TXAtom& XA = cas[i];
    XA.Create();
    XA.Params()[0] = XA.GetType().r_pers;
    atoms.Add(XA);
  }
  ObjectCaster<TSBond, TXBond> cbs = XFile().GetLattice().GetObjects().bonds.GetAccessor<TXBond>();
  for( size_t i=bc; i < cbs.Count(); i++ )  {
    TXBond& XB = cbs[i];
    XB.Update();
    XB.Create();
    bonds.Add(XB);
  }
  if( FLabels->IsVisible() )
    FLabels->Init();
}
//..............................................................................
TXAtom& TGXApp::AddAtom(TXAtom* templ)  {
  vec3d center;
  if( templ != NULL )
    center = templ->CAtom().ccrd();
  TXAtom &A = static_cast<TXAtom&>(XFile().GetLattice().NewAtom(center));
  olxstr colName;
  if( templ != NULL )  {
    colName = templ->GetCollectionName();
    A.CAtom().SetType(templ->GetType());
    if( templ->GetType() == iQPeakZ )
      A.CAtom().SetQPeak(1.0);
  }
  else
    A.CAtom().SetType(XElementLib::GetByIndex(iCarbonIndex));
  A.Create();
  A.Params()[0] = A.GetType().r_pers;
  return A;
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
    CreateObjects(false);
}
//..............................................................................
void TGXApp::undoHide(TUndoData *data)  {
  THideUndo *undo = dynamic_cast<THideUndo*>(data);
  for( size_t i=0; i < undo->Objects.Count(); i++ )
    undo->Objects[i]->SetVisible(!undo->Objects[i]->IsVisible());
}
//..............................................................................
TUndoData* TGXApp::DeleteXObjects(const AGDObjList& L)  {
  TXAtomPList atoms;
  atoms.SetCapacity(L.Count());
  bool planes_deleted = false;
  for( size_t i=0; i < L.Count(); i++ )  {
    if( EsdlInstanceOf(*L[i], TXAtom) )  
      atoms.Add((TXAtom*)L[i]);
    else if( EsdlInstanceOf(*L[i], TXPlane) )  {
      ((TXPlane*)L[i])->Delete(true);
      if( L[i]->GetPrimitives().ObjectCount() == 1 )
        L[i]->GetPrimitives().ClearPrimitives();
      planes_deleted = true;
    }
    else if( EsdlInstanceOf(*L[i], TXBond) )  {
      TXBond* xb = (TXBond*)L[i];
      xb->Delete();
    }
    else
      L[i]->SetVisible(false);
  }
  if( planes_deleted )
    XFile().GetLattice().UpdatePlaneDefinitions();
  return DeleteXAtoms(atoms);
}
//..............................................................................
TUndoData* TGXApp::DeleteXAtoms(TXAtomPList& L)  {
  if (L.IsEmpty()) return NULL;
  TSAtomPList deleted;
  bool safe_afix = TXApp::DoUseSafeAfix();
  for( size_t i=0; i < L.Count(); i++ )  {
    TXAtom* XA = L[i];
    if (XA->IsDeleted())  continue;
    deleted.Add(XA);
    if (XA->GetType().z > 1) {
      for (size_t j=0; j < XA->NodeCount();j++) {
        TXAtom& SH = XA->Node(j);
        if (SH.IsDeleted() || SH.GetType() == iQPeakZ)
          continue;
        if (SH.GetType() == iHydrogenZ) {
          SH.SetDeleted(true);
          deleted.Add(SH);
        }
        else if (safe_afix) { // go one level deeper
          for (size_t k=0; k < SH.NodeCount(); k++) {
            TXAtom& SH1 = SH.Node(k);
            if (SH1.IsDeleted() || SH1.GetType() != iHydrogenZ) continue;
            if (SH1.CAtom().GetParentAfixGroup() != NULL &&
              SH1.CAtom().GetParentAfixGroup()->GetM() != 0 &&
              TNetwork::IsBondAllowed(XA->CAtom(), SH1.CAtom()))
            {
              SH1.SetDeleted(true);
              deleted.Add(SH1);
            }
          }
        }
      }
    }
    XA->SetDeleted(true);
  }
  //CenterView();
  TUndoData *undo = new TDeleteUndo(NULL);
  olxdict<const TLattice *, TDeleteUndo *, TPointerComparator> lud;
  for (size_t i=0; i < deleted.Count(); i++) {
    TDeleteUndo *du = lud.Find(&deleted[i]->GetParent(), NULL);
    if (du == NULL) {
      lud(&deleted[i]->GetParent(),
        (du=new TDeleteUndo(
          UndoAction::New(&deleted[i]->GetParent(), &TLattice::undoDelete))));
      undo->AddAction(du);
    }
    du->AddSAtom(*deleted[i]);
  }
  GetSelection().Clear();
  UpdateConnectivity();
  return undo;
}
//..............................................................................
void TGXApp::SelectBondsWhere(const olxstr &Where, bool Invert)  {
  olxstr str = Where.ToLowerCase();
  if( str.FirstIndexOf("xatom") != InvalidIndex || str.FirstIndexOf("satom") != InvalidIndex)  {
    NewLogEntry(logError) << "SelectBonds: xatom/satom are not allowed here";
    return;
  }
  if( str.FirstIndexOf("sel") != InvalidIndex )  {
    if( FGlRender->GetSelection().Count() != 1 ||
        !EsdlInstanceOf(FGlRender->GetSelection()[0], TXBond) )
    {
      NewLogEntry(logError) << "SelectBonds: please select one bond only";
      return;
    }
  }
  TXFactoryRegister rf;
  TTXBond_EvaluatorFactory *xbond = (TTXBond_EvaluatorFactory*)rf.BindingFactory("xbond");
  TTGlGroupEvaluatorFactory *sel = (TTGlGroupEvaluatorFactory*)rf.BindingFactory("sel");
  sel->SetTGlGroup(&FGlRender->GetSelection());
  TSyntaxParser SyntaxParser(&rf, Where);
  if( !SyntaxParser.Errors().Count() )  {
    BondIterator bi(*this);
    while( bi.HasNext() )  {
      TXBond& xb = bi.Next();
      if( xb.IsSelected() )  continue;
      xbond->provider->SetTXBond(&xb);
      if( SyntaxParser.Evaluate() )  
        GetRender().Select(xb);
    }
  }
  else
    NewLogEntry(logError) << SyntaxParser.Errors().Text(NewLineSequence());
}
//..............................................................................
void TGXApp::SelectAtomsWhere(const olxstr &Where, bool Invert)  {
  olxstr str = Where.ToLowerCase();
  if( str.FirstIndexOf("xbond") != InvalidIndex || str.FirstIndexOf("satom") != InvalidIndex )  {
    NewLogEntry(logError) << "SelectAtoms: xbond/satom are not allowed here";
    return;
  }
  if( str.FirstIndexOf("sel") != InvalidIndex )  {
    if( FGlRender->GetSelection().Count() != 1 )  {
      NewLogEntry(logError) << "SelectAtoms: please select one atom only";
      return;
    }
    if( !EsdlInstanceOf(FGlRender->GetSelection()[0], TXAtom) )  {
      NewLogEntry(logError) << "SelectAtoms: please select an atom";
      return;
    }
  }
  TXFactoryRegister rf;
  TTXAtom_EvaluatorFactory *xatom = (TTXAtom_EvaluatorFactory*)rf.BindingFactory("xatom");
  TTGlGroupEvaluatorFactory *sel = (TTGlGroupEvaluatorFactory*)rf.BindingFactory("sel");
  sel->SetTGlGroup(&FGlRender->GetSelection());
  TSyntaxParser SyntaxParser(&rf, Where);
  if( !SyntaxParser.Errors().Count() )  {
    AtomIterator ai(*this);
    while( ai.HasNext() )  {
      TXAtom& xa = ai.Next();
      if( xa.IsSelected() )  continue;
      xatom->provider->SetTXAtom(&xa);
      if( SyntaxParser.Evaluate() )  
        GetRender().Select(xa);
    }
  }
  else
    NewLogEntry(logError) << SyntaxParser.Errors().Text(NewLineSequence());
}
//..............................................................................
bool GetRing(TSAtomPList& atoms, TTypeList<TSAtomPList>& rings)  {
  TSAtomPList *ring = NULL;
  size_t starta = 0;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( atoms[i]->GetTag() == 0 )  {  // unused atom
      ring = &rings.AddNew();
      ring->Add(atoms[i]);
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

TTypeList<TSAtomPList>&TGXApp::FindRings(const olxstr& Condition,
  TTypeList<TSAtomPList>& rings)
{
  ElementPList ring;
  if( Condition.Equalsi("sel") )  {
    TSAtomPList SAtoms(GetSelectedXAtoms(false));
    SAtoms.ForEach(ACollectionItem::TagSetter(0));
    while( GetRing(SAtoms, rings) )
      ;
    return rings;
  }
  TXApp::FindRings(Condition, rings);
  return rings;
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
  if (Condition.StartsFrom('*')) {
    TXAtomPList atoms = FindXAtoms(Condition.SubStringFrom(1), false, false);
    TTypeList<TSAtomPList> rings;
    for (size_t i=0; i < atoms.Count(); i++)
      atoms[i]->GetNetwork().FindAtomRings(*atoms[i], rings);
    if( !rings.IsEmpty() )  {
      for( size_t i=0; i < rings.Count(); i++ )  {
        for( size_t j=0; j < rings[i].Count(); j++ )  {
          TXAtom* xa = dynamic_cast<TXAtom*>(rings[i][j]);
          GetRender().Select(*xa, true);
        }
      }
    }
  }
  else {
    TTypeList< TSAtomPList > rings;
    try  {  FindRings(Condition, rings);  }
    catch( const TExceptionBase& exc )  {
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    if( rings.IsEmpty() )  return;
    TXAtomPList all;
    all.SetCapacity(rings.Count()*rings[0].Count());
    for( size_t i=0; i < rings.Count(); i++ )  {
      SortRing(rings[i]);
      for( size_t j=0; j < rings[i].Count(); j++ )
        all.Add(static_cast<TXAtom*>(rings[i][j]));
    }
    all.ForEach(ACollectionItem::IndexTagSetter());
    for( size_t i=0; i < all.Count(); i++ )
      if( (size_t)all[i]->GetTag() == i && all[i]->IsVisible() )
        FGlRender->Select(*all[i]);
  }
}
//..............................................................................
void TGXApp::SelectAtoms(const olxstr &Names, bool Invert)  {
  TXAtomPList Sel = FindXAtoms(Names, true);
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
  TXAtomPList xatoms = GetSelectedXAtoms(GetDoClearSelection());
  atoms.SetCapacity(atoms.Count() + xatoms.Count());
  for( size_t i=0; i < xatoms.Count(); i++ )
    atoms.AddNew(&xatoms[i]->CAtom(), &xatoms[i]->GetMatrix());
}
//..............................................................................
void TGXApp::ExpandSelectionEx(TSAtomPList& atoms)  {
  atoms.AddList(GetSelectedXAtoms(GetDoClearSelection()));
}
//..............................................................................
ConstPtrList<TSObject<TNetwork> > TGXApp::GetSelected() {
  //TPtrList<TSObject<TNetwork> > res;
  ConstPtrList<TSObject<TNetwork> > rv =
    GetSelection().Extract<TSObject<TNetwork> >();
  if (GetDoClearSelection())
    SelectAll(false);
  return rv;
}
//..............................................................................
ConstPtrList<TCAtom> TGXApp::FindCAtoms(const olxstr &Atoms, bool ClearSelection)  {
  if( Atoms.IsEmpty() )  {
    TCAtomPList list = GetSelectedCAtoms(ClearSelection);
    if( !list.IsEmpty() )  return list;
    TAsymmUnit& AU = XFile().GetLattice().GetAsymmUnit();
    list.SetCapacity(list.Count() + AU.AtomCount());
    for( size_t i=0; i < AU.ResidueCount(); i++ )  {
      TResidue& resi = AU.GetResidue(i);
      for( size_t j=0; j < resi.Count(); j++ )  {
        if( !resi[j].IsDeleted() )
          list.Add(resi[j]);
      }
    }
    return list;
  }
  TStrList Toks(Atoms, ' ');
  olxstr Tmp;
  TCAtomPList list;
  for( size_t i = 0; i < Toks.Count(); i++ )  {
    Tmp = Toks[i];
    if( Tmp.Equalsi("sel") )  {
      list.AddList(GetSelectedCAtoms(ClearSelection));
      continue;
    }
    if( Tmp.CharAt(0) == '$' )  {
      SortedElementPList elms = TAtomReference::DecodeTypes(
        Tmp.SubStringFrom(1), XFile().GetAsymmUnit());
      for( size_t ei=0; ei < elms.Count(); ei++ )
        list += CAtomsByType(*elms[ei]);
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
      list += CAtomsByMask(Tmp, mask);
      continue;
    }
    TCAtom* A = XFile().GetAsymmUnit().FindCAtom(Tmp);
    if( A != NULL && !A->IsDeleted() )
      list.Add(A);
  }
  return list;
}
//..............................................................................
void TGXApp::UpdateDuplicateLabels() {
  if (!FLabels->IsVisible() || (FLabels->GetMode()&lmLabels) == 0) return;
  olxdict<olxstr, size_t, olxstrComparator<true> > ld;
  AtomIterator ai = GetAtoms();
  while (ai.HasNext()) {
    TXAtom &a = ai.Next();
    if (!a.IsAvailable() || !a.GetMatrix().IsFirst()) continue;
    olxstr gl = a.GetGuiLabel();
    size_t idx = ld.IndexOf(gl);
    if (idx != InvalidIndex) {
      if (ld.GetValue(idx) != InvalidIndex) {
        if (FLabels->GetMaterialIndex(ld.GetValue(idx)) != lmiMark) {
          FLabels->SetMaterialIndex(ld.GetValue(idx), lmiDuplicateLabel);
          ld.GetValue(idx) = InvalidIndex;
        }
      }
      if (FLabels->GetMaterialIndex(a.GetOwnerId()) != lmiMark)
        FLabels->SetMaterialIndex(a.GetOwnerId(), lmiDuplicateLabel);
    }
    else {
      ld(gl, a.GetOwnerId());
      if (FLabels->GetMaterialIndex(a.GetOwnerId()) == lmiDuplicateLabel)
        FLabels->SetMaterialIndex(a.GetOwnerId(), (LabelMaterialIndex)~0);
    }
  }
}
//..............................................................................
bool TGXApp::AreLabelsVisible()  const {  return FLabels->IsVisible(); }
//..............................................................................
void TGXApp::SetLabelsVisible(bool v)  {
  bool update = v != FLabels->IsVisible();
  FLabels->SetVisible(v);
  if (update)
    UpdateDuplicateLabels();
}
//..............................................................................
void TGXApp::SetLabelsMode(uint32_t lmode)  {  FLabels->SetMode(lmode); }
//..............................................................................
short TGXApp::GetLabelsMode()      const {  return FLabels->GetMode(); }
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
ConstPtrList<TXBond> TGXApp::GetBonds(const TStrList& Bonds, bool inc_lines)  {
  TXBondPList List;
  if( Bonds.IsEmpty() )  {
    TGlGroup& sel = GetRender().GetSelection();
    for( size_t i=0; i < sel.Count(); i++ )  {
      if( EsdlInstanceOf(sel[i], TXBond) )
        List.Add((TXBond&)sel[i]);
      else if( inc_lines && EsdlInstanceOf(sel[i], TXLine) )
        List.Add((TXBond&)sel[i]);
      else if( EsdlInstanceOf(sel[i], TXAtom) ) {
        TXAtom& xa = ((TXAtom&)sel[i]);
        for( size_t j=0; j < xa.BondCount(); j++ )
          List.Add(xa.Bond(j));
      }
    }
    return ACollectionItem::Unique(List);
  }
  for (size_t i=0; i < Bonds.Count(); i++) {
    TGPCollection *GPC = GetRender().FindCollection(Bonds[i]);
    if( GPC == NULL )  continue;
    for( size_t i=0; i < GPC->ObjectCount(); i++ )  {
      // check if the right type !
      if( i == 0 &&  !EsdlInstanceOf(GPC->GetObject(0), TXBond) )  
        break;
      List.Add((TXBond&)GPC->GetObject(i));
    }
  }
  return List;
}
//..............................................................................
void TGXApp::AtomRad(const olxstr& Rad, TXAtomPList* Atoms)  { // pers, sfil
  short DS = -1;
  double r = 0;
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
  else if (Rad.IsNumber()) {
    DS = darCustom;
    r = Rad.ToDouble();
  }
  else 
    throw TInvalidArgumentException(__OlxSourceInfo, "rad");

  if( Atoms != NULL )  {  // make sure all atoms of selected collections are updated
    Atoms->ForEach(ACollectionItem::IndexTagSetter(
      FunctionAccessor::MakeConst(&TXAtom::GetPrimitives)));
    for( size_t i=0; i < Atoms->Count(); i++ )  {
      TGPCollection &gpc = (*Atoms)[i]->GetPrimitives();
      if ((size_t)gpc.GetTag() == i)  {
        for (size_t j=0; j < gpc.ObjectCount(); j++) {
          TXAtom *at = dynamic_cast<TXAtom*>(&gpc.GetObject(j));
          if (at == NULL) continue;
          at->CalcRad(DS);
          if (DS == darCustom)
            at->SetR(r);
        }
        gpc.SetTag(-1);
      }
    }
  }
  else {
    AtomIterator ai(*this);
    while( ai.HasNext() ) {
      TXAtom &a = ai.Next();
      a.CalcRad(DS);
      if (DS == darCustom)
        a.SetR(r);
    }
  }
  if( Atoms == NULL )  { // 
    TXAtom::DefZoom(1);
    TXAtom::TelpProb(1);
    TXAtom::DefRad(DS);
  }
}
//..............................................................................
void TGXApp::GetGPCollections(AGDObjList& GDObjects, TPtrList<TGPCollection>& Result)  {
  GDObjects.ForEach(ACollectionItem::IndexTagSetter(
      FunctionAccessor::MakeConst(&TXAtom::GetPrimitives)));
  for( size_t i=0; i < GDObjects.Count(); i++ )  {
    if( (size_t)GDObjects[i]->GetPrimitives().GetTag() == i )
      Result.Add(GDObjects[i]->GetPrimitives());
  }
}
//..............................................................................
void TGXApp::FillXAtomList(TXAtomPList& res, TXAtomPList* providedAtoms) {
  if( providedAtoms != NULL )
    res.AddList(*providedAtoms);
  else  {
    AtomIterator ai(*this);
    res.SetCapacity(res.Count()+ai.count);
    while( ai.HasNext() )
      res.Add(ai.Next());
  }
}
//..............................................................................
void TGXApp::FillXBondList(TXBondPList& res, TXBondPList* providedBonds)  {
  if( providedBonds != NULL )
    res.AddList(*providedBonds);
  else  {
    BondIterator bi(*this);
    res.SetCapacity(res.Count()+bi.count);
    while( bi.HasNext() )
      res.Add(bi.Next());
  }
}
//..............................................................................
void TGXApp::AtomZoom(float Zoom, TXAtomPList* Atoms)  {  // takes %
  double z = Zoom/100;
  AGDObjList objects;
  if( Atoms != NULL )  {
    objects.SetCapacity(Atoms->Count());
    for( size_t i=0; i < Atoms->Count(); i++ )
      Atoms->GetItem(i)->SetZoom(z);
  }
  else  {
    AtomIterator ai(*this);
    objects.SetCapacity(ai.count);
    while( ai.HasNext() )
      ai.Next().SetZoom(z);
  }
}
//..............................................................................
void TGXApp::SetQPeakScale(float V)  {
  TXAtom::SetQPeakScale(V);
  TPtrList<TGPCollection> Colls;
  AtomIterator ai(*this);
  while( ai.HasNext() )  {
    TXAtom& xa = ai.Next();
    if( xa.GetType() == iQPeakZ )
      Colls.Add(xa.GetPrimitives());
  }
  FGlRender->RemoveCollections(Colls);
  ai.Reset();
  while( ai.HasNext() )  {
    TXAtom& xa = ai.Next();
    if( xa.GetType() == iQPeakZ )
      xa.Create();
  }
}
//..............................................................................
float TGXApp::GetQPeakScale()  {
  return TXAtom::GetQPeakScale();
}
//..............................................................................
void TGXApp::SetQPeakSizeScale(float V)  {
  TXAtom::SetQPeakSizeScale(V);
  TPtrList<TGPCollection> Colls;
  AtomIterator ai(*this);
  while( ai.HasNext() )  {
    TXAtom& xa = ai.Next();
    if( xa.GetType() == iQPeakZ )
      Colls.Add(xa.GetPrimitives());
  }
  FGlRender->RemoveCollections(Colls);
  ai.Reset();
  while( ai.HasNext() )  {
    TXAtom& xa = ai.Next();
    if( xa.GetType() == iQPeakZ )
      xa.Create();
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
    objects.Assign(*Bonds, StaticCastAccessor<AGDrawObject>());
  else  {
    BondIterator bi(*this);
    objects.SetCapacity(bi.count);
    while( bi.HasNext() )
      objects.Add(bi.Next());
  }

  TPtrList<TGPCollection> Colls;
  GetGPCollections(objects, Colls);
  for( size_t i=0; i < Colls.Count(); i++ )  {
    if( Colls[i]->ObjectCount() != 0 ) 
      ((TXBond&)Colls[i]->GetObject(0)).SetRadius(R);
  }
}
//..............................................................................
void TGXApp::UpdateAtomPrimitives(int Mask, TXAtomPList* Atoms) {
  GetRender().GetScene().MakeCurrent();
  TXAtomPList atoms;
  FillXAtomList(atoms, Atoms);
  atoms.ForEach(ACollectionItem::IndexTagSetter(
    FunctionAccessor::MakeConst(&TXAtom::GetPrimitives)));
  for( size_t i=0; i < atoms.Count(); i++ )
    if( (size_t)atoms[i]->GetPrimitives().GetTag() == i )
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
  GetRender().GetScene().MakeCurrent();
  TXBondPList bonds;
  FillXBondList(bonds, Bonds);
  bonds.ForEach(ACollectionItem::IndexTagSetter(
    FunctionAccessor::MakeConst(&TXBond::GetPrimitives)));
  if( HBondsOnly )  {
    for( size_t i=0; i < bonds.Count(); i++ )  {
      if( bonds[i]->GetType() != sotHBond )  continue;
      if( (size_t)bonds[i]->GetPrimitives().GetTag() == i )
        bonds[i]->UpdatePrimitives(Mask);
    }
  }
  else  {
    for( size_t i=0; i < bonds.Count(); i++ )  {
      if( bonds[i]->GetType() == sotHBond )  continue;
      if( (size_t)bonds[i]->GetPrimitives().GetTag() == i )
        bonds[i]->UpdatePrimitives(Mask);
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
  GetRender().GetScene().MakeCurrent();
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
void TGXApp::GrowAtoms(const olxstr& AtomsStr, bool Shell, TCAtomPList* Template)  {
  TSAtomPList satoms(FindXAtoms(AtomsStr, true));
  FXFile->GetLattice().GrowAtoms(satoms, Shell, Template);
}
//..............................................................................
void TGXApp::RestoreGroup(TGlGroup& glg, const GroupData& gd)  {
  TXAtomPList xatoms(gd.atoms.Count());
  TXBondPList xbonds(gd.bonds.Count());
  for( size_t i=0; i < gd.atoms.Count(); i++ )  {
    TSAtom* sa = gd.atoms[i].latt.GetAtomRegistry().Find(gd.atoms[i].ref);
    if( sa != NULL )
      xatoms[i] = static_cast<TXAtom*>(sa);
  }
  for( size_t i=0; i < gd.bonds.Count(); i++ )  {
    TSBond* sb = gd.bonds[i].latt.GetAtomRegistry().Find(gd.bonds[i].ref);
    if( sb != NULL )
      xbonds[i] = static_cast<TXBond*>(sb);
  }
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    if( xatoms[i] != NULL && xatoms[i]->IsVisible() )
      glg.Add(*xatoms[i], false);
  }
  for( size_t i=0; i < xbonds.Count(); i++ )  {
    if( xbonds[i] != NULL && xbonds[i]->IsVisible() )
      glg.Add(*xbonds[i], false);
  }
  glg.SetVisible(gd.visible);
}
//..............................................................................
void TGXApp::StoreGroup(const TGlGroup& glG, GroupData& gd)  {
  gd.collectionName = glG.GetCollectionName();  //planes
  gd.visible = glG.IsVisible();
  gd.parent_id = (glG.GetParentGroup() != NULL ? glG.GetParentGroup()->GetTag() : -2); 
  for( size_t j=0; j < glG.Count(); j++ )  {
    AGDrawObject& glO = glG[j];
    if( EsdlInstanceOf(glO, TXAtom) )  {
      const TSAtom& sa = ((TXAtom&)glO);
      gd.atoms.Add(new TGXApp::AtomRef(sa.GetNetwork().GetLattice(), sa.GetRef()));
    }
    if( EsdlInstanceOf(glO, TXBond) )  {
      const TSBond& sb = ((TXBond&)glO);
      gd.bonds.Add(new TGXApp::BondRef(sb.GetNetwork().GetLattice(), sb.GetRef()));
    }
  }
}
//..............................................................................
void TGXApp::StoreLabels()  {
  LabelInfo.Clear();
  AtomIterator ai(*this);
  while( ai.HasNext() )  {
    TXAtom& xa = ai.Next();
    if( xa.GetGlLabel().IsVisible() )  {
      LabelInfo.atoms.Add(new TGXApp::AtomRef(xa.GetNetwork().GetLattice(), xa.GetRef()));
      LabelInfo.labels.AddCopy(xa.GetGlLabel().GetLabel());
      LabelInfo.centers.AddCopy(xa.GetGlLabel().GetCenter());
    }
  }
  BondIterator bi(*this);
  while( bi.HasNext() )  {
    TXBond& xb = bi.Next();
    if( xb.GetGlLabel().IsVisible() )  {
      LabelInfo.bonds.Add(new TGXApp::BondRef(xb.GetNetwork().GetLattice(), xb.GetRef()));
      LabelInfo.labels.AddCopy(xb.GetGlLabel().GetLabel());
      LabelInfo.centers.AddCopy(xb.GetGlLabel().GetCenter());
    }
  }
}
//..............................................................................
void TGXApp::RestoreLabels()  {
  TXAtomPList xatoms(LabelInfo.atoms.Count());
  TXBondPList xbonds(LabelInfo.bonds.Count());
  for( size_t i=0; i < LabelInfo.atoms.Count(); i++ )  {
    TSAtom* sa = LabelInfo.atoms[i].latt.GetAtomRegistry().Find(LabelInfo.atoms[i].ref);
    if( sa != NULL )
      xatoms[i] = static_cast<TXAtom*>(sa);
  }
  for( size_t i=0; i < LabelInfo.bonds.Count(); i++ )  {
    TSBond* sb = LabelInfo.bonds[i].latt.GetAtomRegistry().Find(LabelInfo.bonds[i].ref);
    if( sb != NULL )
      xbonds[i] = static_cast<TXBond*>(sb);
  }
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    if( xatoms[i] == NULL )  continue;
    xatoms[i]->GetGlLabel().SetVisible(true);
    xatoms[i]->GetGlLabel().SetLabel(LabelInfo.labels[i]);
    xatoms[i]->GetGlLabel().SetOffset(xatoms[i]->crd());
    xatoms[i]->GetGlLabel().TranslateBasis(LabelInfo.centers[i]-xatoms[i]->GetGlLabel().GetCenter());
  }
  for( size_t i=0; i < xbonds.Count(); i++ )  {
    if( xbonds[i] == NULL )  continue;
    xbonds[i]->GetGlLabel().SetVisible(true);
    xbonds[i]->GetGlLabel().SetLabel(LabelInfo.labels[xatoms.Count()+i]);
    xbonds[i]->GetGlLabel().SetOffset(xbonds[i]->GetCenter());
    xbonds[i]->GetGlLabel().TranslateBasis(
      LabelInfo.centers[xatoms.Count()+i]-xbonds[i]->GetGlLabel().GetCenter());
  }
  LabelInfo.Clear();
}
//..............................................................................
void TGXApp::RestoreGroups()  {
  if( !SelectionCopy[0].IsEmpty() )
    RestoreGroup(GetSelection(), SelectionCopy[0]);
  GroupDict.Clear();
  FGlRender->ClearGroups();
  for( size_t i=0; i < GroupDefs.Count(); i++ )
    FGlRender->NewGroup(GroupDefs[i].collectionName).Create(GroupDefs[i].collectionName);
  for( size_t i=0; i < GroupDefs.Count(); i++ )  {
    TGlGroup& glg = FGlRender->GetGroup(i);
    RestoreGroup(glg, GroupDefs[i]);
    if( GroupDefs[i].parent_id == -1 )
      FGlRender->GetSelection().Add(glg);
    else if( GroupDefs[i].parent_id >= 0 )
      FGlRender->GetGroup(GroupDefs[i].parent_id).Add(glg);
    GroupDict(&glg, i);
  }
  RestoreLabels();
}
//..............................................................................
void TGXApp::BeginDrawBitmap(double resolution)  {
  FPictureResolution = resolution;
  FLabels->Clear();
  GetRender().GetScene().ScaleFonts(resolution);
  double LW = 0;
  olx_gl::get(GL_LINE_WIDTH, &LW);
  olx_gl::lineWidth(LW*resolution);
  CreateObjects(false, false);
  UpdateLabels();
}
//..............................................................................
void TGXApp::FinishDrawBitmap()  {
  FLabels->Clear();
  GetRender().GetScene().RestoreFontScale();
  CreateObjects(false, false);
  UpdateLabels();
}
//..............................................................................
void TGXApp::UpdateLabels()  {
  TGlRenderer& r = GetRender();
  for( size_t i=0; i < r.ObjectCount(); i++ )
    r.GetObject(i).UpdateLabel();
}
//..............................................................................
uint64_t TGXApp::Draw()  {
  if( !IsMainFormVisible() || DisplayFrozen )  return 0;
  uint64_t st = TETime::msNow();
  GetRender().Draw();
  return TETime::msNow() - st;
}
//..............................................................................
void TGXApp::MoveFragment(TXAtom* to, TXAtom* fragAtom, bool copy)  {
  if( copy )
    FXFile->GetLattice().MoveFragmentG(*to, *fragAtom);
  else
    FXFile->GetLattice().MoveFragment(*to, *fragAtom);
}
//..............................................................................
void TGXApp::MoveFragment(const vec3d& to, TXAtom* fragAtom, bool copy)  {
  if( copy )
    FXFile->GetLattice().MoveFragmentG(to, *fragAtom);
  else
    FXFile->GetLattice().MoveFragment(to, *fragAtom);
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
void TGXApp::SetHBondsVisible(bool v, bool update_groups)  {
  FHBondsVisible = v;
  if( !v )  {
    BondIterator bi(*this);
    while( bi.HasNext() )  {
      TXBond& xb = bi.Next();
      if( xb.GetType() == sotHBond )
        xb.SetVisible(false);
    }
  }
  else  {
    BondIterator bi(*this);
    while( bi.HasNext() )  {
      TXBond& xb = bi.Next();
      if( xb.GetType() == sotHBond )
        xb.SetVisible(xb.A().IsVisible() && xb.B().IsVisible());
    }
    if( update_groups )
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
    /* TODO: this is disabled because it causes problems with molecules
    disordered nearby an element of symmetry - need better treatment...
    */
    //if( v && !XFile().GetLattice().IsGenerated() )
    //  XFile().GetLattice().CompaqH();
    //else
    UpdateConnectivity();
    CenterView(true);
  }
}
//..............................................................................
void TGXApp::UpdateConnectivity()  {
  XFile().GetLattice().OnDisassemble.Enter(NULL);
  XFile().GetLattice().OnDisassemble.SetEnabled(false);
  for (size_t i = 0; i < OverlayedXFiles.Count(); i++)
    OverlayedXFiles[i].GetLattice().UpdateConnectivity();
  XFile().GetLattice().UpdateConnectivity();
  XFile().GetLattice().OnDisassemble.SetEnabled(true);
  XFile().GetLattice().OnDisassemble.Exit(NULL);
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
void TGXApp::SetQPeakBondsVisible(bool v, bool update_groups)  {
  FQPeakBondsVisible = v;
  if( !v )  {
    BondIterator bi(*this);
    while( bi.HasNext() )  {
      TXBond& xb = bi.Next();
      if( xb.A().GetType() == iQPeakZ || xb.B().GetType() == iQPeakZ )
        xb.SetVisible(v);
    }
    if( FXGrowLinesVisible )  {
      for( size_t i=0; i < XGrowLines.Count(); i++ )  {
        if( XGrowLines[i].XAtom().GetType() == iQPeakZ ||
          XGrowLines[i].CAtom().GetType() == iQPeakZ )
          XGrowLines[i].SetVisible(v);
      }
    }
  }
  else  {
    BondIterator bi(*this);
    while( bi.HasNext() )  {
      TXBond& xb = bi.Next();
      if( xb.A().GetType() == iQPeakZ || xb.B().GetType() == iQPeakZ )
        xb.SetVisible(xb.A().IsVisible() && xb.B().IsVisible());
    }
    if( update_groups )
      RestoreGroups();
  }
}
//..............................................................................
void TGXApp::_maskInvisible()  {
  const TAsymmUnit& au = XFile().GetAsymmUnit();
  TEBitArray vis(au.AtomCount());
  AtomIterator ai(*this);
  while( ai.HasNext() )  {
    TXAtom& xa = ai.Next();
    if( !xa.IsVisible() )  {
      xa.SetMasked(true);
      continue;
    }
    else
      vis.SetTrue(xa.CAtom().GetId());
  }
  for( size_t i=0; i < vis.Count(); i++ )
    au.GetAtom(i).SetMasked(!vis[i]);
}
//..............................................................................
void TGXApp::_syncBondsVisibility()  {
  BondIterator bi(*this);
  while( bi.HasNext() )  {
    TXBond& xb = bi.Next();
    if( !FQPeakBondsVisible && (xb.A().GetType() == iQPeakZ || xb.B().GetType() == iQPeakZ) )
      xb.SetVisible(false);
    else if( !FHBondsVisible && xb.GetType() == sotHBond )
      xb.SetVisible(false);
    else
      xb.SetVisible(xb.A().IsVisible() && xb.B().IsVisible());
  }
  if( FXGrowLinesVisible )  {
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      if( XGrowLines[i].XAtom().GetType() == iQPeakZ )
        XGrowLines[i].SetVisible(XGrowLines[i].XAtom().IsVisible());
    }
  }
}
//..............................................................................
void TGXApp::SetStructureVisible(bool v)  {
  FStructureVisible = v;
  AtomIterator ai(*this);
  while( ai.HasNext() )  {
    TXAtom& xa = ai.Next();
    if( !v )
      xa.SetVisible(v);
    else
      xa.SetVisible(xa.IsAvailable());
  }
  for( size_t i=0; i < LooseObjects.Count(); i++ )
    LooseObjects[i]->SetVisible(v);
  for (size_t i=0; i < Lines.Count(); i++) {
    if (!Lines[i].IsDeleted())
      Lines[i].SetVisible(v);
  }
  PlaneIterator pi(*this);
  while( pi.HasNext() )
    pi.Next().SetVisible(v);
  for( size_t i=0; i < XLabels.Count(); i++ )
    XLabels[i].SetVisible(v);
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
  volatile TStopWatch sw(__FUNC__);
  FXFile->LoadFromFile(fn);
  if (CheckFileType<TIns>() && DoUseSafeAfix()) {
    GetUndo().Push(FXFile->GetLattice().ValidateHGroups(true, true));
  }
  if( !FStructureVisible )
    NewLogEntry() << "Note: structure is invisible";
  else {
    if( !FQPeaksVisible ) 
      NewLogEntry() << "Note: Q-peaks are invisible";
    if( !FHydrogensVisible )
      NewLogEntry() << "Note: H-atoms are invisible";
  }
  Draw();  // fixes native loader is not draw after load
}
//..............................................................................
void ShowPart_TagSetter(TCAtom &a, index_t v) {
  if (a.IsProcessed()) return;
  a.SetProcessed(true);
  for (size_t i=0; i < a.AttachedSiteCount(); i++) {
    TCAtom &aa = a.GetAttachedAtom(i);
    aa.SetTag(v);
    ShowPart_TagSetter(aa, v);
  }
}
void TGXApp::ShowPart(const TIntList& parts, bool show, bool visible_only)  {
  if (visible_only) {
    SortedObjectList<index_t, TPrimitiveComparator> tags;
    AtomIterator ai(*this);
    TAsymmUnit &au = XFile().GetAsymmUnit();
    for (size_t i=0; i < au.AtomCount(); i++) {
      TCAtom &a = au.GetAtom(i);
      a.SetProcessed(false);
      a.SetTag(-1);
    }
    for (size_t i=0; i < au.AtomCount(); i++) {
      TCAtom &a = au.GetAtom(i);
      if (!a.IsProcessed())
        ShowPart_TagSetter(a, (index_t)i);

    }
    while (ai.HasNext()) {
      TXAtom& xa = ai.Next();
      if (xa.IsVisible()) {
        tags.AddUnique(xa.CAtom().GetTag());
      }
    }
    ai.Reset();
    while (ai.HasNext()) {
      TXAtom& xa = ai.Next();
      if (!xa.IsVisible() && tags.Contains(xa.CAtom().GetTag())) {
        xa.SetMasked(false);
        xa.SetVisible(true);
      }
    }
    if (parts.IsEmpty()) {
      _maskInvisible();
      UpdateConnectivity();
    }
  }
  if (parts.IsEmpty()) {
    if (!visible_only)
      AllVisible(true);
    return;
  }
  AtomIterator ai(*this);
  while (ai.HasNext()) {
    TXAtom& xa = ai.Next();
    if (visible_only && !xa.IsVisible())
      continue;
    if (parts.Contains(xa.CAtom().GetPart())) {
      xa.SetVisible(show);
      xa.SetMasked(!show);
    }
    else {
      xa.SetVisible(!show);
      xa.SetMasked(show);
    }
    if (xa.IsVisible())
      xa.SetVisible(xa.IsAvailable());
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
      NewLogEntry(logError) << "Cannot display empty grid";
      return false;
    }
  }
  GetUndo().Push(SetGraphicsVisible(FXGrid, v));
  return v;
}
//..............................................................................
void TGXApp::Individualise(const TXAtomPList& atoms, short _level, int32_t mask)  {
  if( atoms.IsEmpty() || _level > 2 )  return;
  TXBondPList xbonds;
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
      const olxstr leg = a.GetLegend(a, legend_level);
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
          if( _xa != NULL && TXAtom::GetLegend(*_xa, legend_level) == leg )
            objects.Add(_xa);
        }
        a.GetPrimitives().RemoveObjects(objects);
        for( size_t oi=0; oi < objects.Count(); oi++ )  {
          TXAtom* _xa = (TXAtom*)objects[oi];
          _xa->Create(leg);
          xbonds.AddList(_xa->GetBonds(), StaticCastAccessor<TXBond>());
        }
        if( mask >= 0 )
          a.UpdatePrimitives(mask);
      }
    }
  }
  for( size_t i=0; i < xbonds.Count(); i++ )  {
    int cl = TXAtom::LegendLevel(xbonds[i]->GetPrimitives().GetName())+1;
    if( cl > 2 )  continue;
    TXBond& bond = *xbonds[i];
    if( _level == -1 )  {
      const int al = olx_max(TXAtom::LegendLevel(bond.A().GetPrimitives().GetName()),
        TXAtom::LegendLevel(bond.B().GetPrimitives().GetName()));
      if( al < cl || al >= 3 )  continue;
      cl = al;
    }
    else
      cl = _level;
    const olxstr leg = xbonds[i]->GetLegend(*xbonds[i], cl);
    TGPCollection* indCol = FGlRender->FindCollection(leg);
    if( &xbonds[i]->GetPrimitives() == indCol )  
      continue;
    else  {
      if( indCol == NULL )  {
        indCol = &FGlRender->NewCollection(leg);
        IndividualCollections.Add(leg);
      }
      xbonds[i]->GetPrimitives().RemoveObject(*xbonds[i]);
      xbonds[i]->Create(leg);
    }
  }
}
//..............................................................................
void TGXApp::Individualise(TXAtom& XA, short _level, int32_t mask)  {
  const int level = TXAtom::LegendLevel(XA.GetPrimitives().GetName());
  if( level >= 2 )  return;
  Individualise(TXAtomPList() << XA, _level == -1 ? level+1 : _level, mask);
}
//..............................................................................
void TGXApp::Collectivise(TXAtom& XA, short _level, int32_t mask)  {
  const int level = TXAtom::LegendLevel(XA.GetPrimitives().GetName());
  if( level == 0 )  return;
  Collectivise(TXAtomPList() << XA, _level == -1 ? level-1 : _level, mask);
}
//..............................................................................
void TGXApp::Individualise(TXBond& XB, short _level, int32_t mask)  {
  const int level = TXAtom::LegendLevel(XB.GetPrimitives().GetName());
  if( level >= 3 )  return;
  Individualise(TXBondPList() << XB, _level == -1 ? level+1 : _level, mask);
}
//..............................................................................
void TGXApp::Individualise(const TXBondPList& bonds, short _level, int32_t mask) {
  if( bonds.IsEmpty() || _level > 3 )  return;
  for( size_t i=0; i < bonds.Count(); i++ )  {
    TXBond& b = *bonds[i];
    const int cl = TXAtom::LegendLevel(b.GetPrimitives().GetName()); 
    if( cl >= 3 )  {
      if( mask >= 0 )
        b.UpdatePrimitives(mask);
    }
    else if( _level != -1 && cl > _level )
      continue;
    else  {
      const int legend_level = _level == -1 ? cl+1 : _level;
      const olxstr leg = TXBond::GetLegend(b, legend_level);
      TGPCollection* indCol = FGlRender->FindCollection(leg);
      if( &b.GetPrimitives() == indCol )  {
        if( mask >= 0 )
          b.UpdatePrimitives(mask);
      }
      else  {
        if( indCol == NULL )  {
          indCol = &FGlRender->NewCollection(leg);
          IndividualCollections.Add(leg);
        }
        TPtrList<AGDrawObject> objects;
        for( size_t oi=0; oi < b.GetPrimitives().ObjectCount(); oi++ )  {
          TXBond* _xb = dynamic_cast<TXBond*>(&b.GetPrimitives().GetObject(oi));
          if( _xb != NULL && TXBond::GetLegend(*_xb, legend_level) == leg )
            objects.Add(_xb);
        }
        b.GetPrimitives().RemoveObjects(objects);
        for( size_t oi=0; oi < objects.Count(); oi++ )
          objects[oi]->Create(leg);
        if( mask >= 0 )  // this will affect the whole group
          b.UpdatePrimitives(mask);
      }
    }
  }
}
//..............................................................................
void TGXApp::Collectivise(TXBond& XB, short level, int32_t mask)  {
  Collectivise(TXBondPList() << XB,
    level == -1 ? TXAtom::LegendLevel(XB.GetPrimitives().GetName())+1 : level, mask);
}
//..............................................................................
void TGXApp::Collectivise(const TXBondPList& bonds, short _level, int32_t mask)  {
  if( bonds.IsEmpty() )  return;
  for( size_t i=0; i < bonds.Count(); i++ )  {
    TXBond& b = *bonds[i];
    const int cl = TXAtom::LegendLevel(b.GetPrimitives().GetName());
    if( cl == 0 || (_level != -1 && cl < _level) )  continue;
    const olxstr leg = b.GetLegend(b, _level == -1 ? cl-1 : _level);
    TGPCollection* indCol = FGlRender->FindCollection(leg);
    if( &b.GetPrimitives() == indCol )
      continue;
    if( indCol == NULL )
      indCol = &FGlRender->NewCollection(leg);
    const size_t index = IndividualCollections.IndexOf(b.GetPrimitives().GetName());
    if( index != InvalidIndex )
      IndividualCollections.Delete(index);
    TPtrList<AGDrawObject> objects = b.GetPrimitives().GetObjects();
    b.GetPrimitives().ClearObjects();
    for( size_t oi=0; oi < objects.Count(); oi++ )  {
      objects[oi]->Create(leg);
    }
    if( mask >= 0 )
      b.UpdatePrimitives(mask);
  }
}
//..............................................................................
void TGXApp::Collectivise(const TXAtomPList& atoms, short _level, int32_t mask)  {
  if( atoms.IsEmpty() )  return;
  //TSBondPList sbonds;
  TPtrList<TGPCollection> colls;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    TXAtom& a = *atoms[i];
    const int cl = TXAtom::LegendLevel(a.GetPrimitives().GetName());
    if( cl == 0 || (_level != -1 && cl < _level) )  continue;
    const olxstr leg = a.GetLegend(a, _level == -1 ? cl-1 : _level);
    TGPCollection* indCol = FGlRender->FindCollection(leg);
    if( indCol != NULL && &a.GetPrimitives() == indCol )  
      continue;
    else  {
      if( indCol == NULL )
        indCol = &FGlRender->NewCollection(leg);
      const size_t index = IndividualCollections.IndexOf(a.GetPrimitives().GetName());
      if( index != InvalidIndex )
        IndividualCollections.Delete(index);
      TGPCollection &gpc = a.GetPrimitives();
      colls << gpc;
      TPtrList<AGDrawObject> objects = gpc.GetObjects();
      gpc.ClearObjects();
      for( size_t oi=0; oi < objects.Count(); oi++ )
        objects[oi]->Create(leg);
      if( mask >= 0 )
        a.UpdatePrimitives(mask);
      //for( size_t j=0; j < a.BondCount(); j++ )
      //  sbonds.Add(a.Bond(j));

    }
    GetRender().RemoveCollections(colls);
  }
}
//..............................................................................
size_t TGXApp::GetNextAvailableLabel(const olxstr& AtomType) {
  size_t nextLabel = 0;
  cm_Element* elm = XElementLib::FindBySymbol(AtomType);
  if (elm == NULL) return nextLabel+1;
  TAsymmUnit &au = XFile().GetAsymmUnit();
  for (size_t i=0; i < au.AtomCount(); i++) {
    TCAtom &ca = au.GetAtom(i);
    if (ca.GetType() == *elm) {
      olxstr label = ca.GetLabel().SubStringFrom(elm->symbol.Length());
      if (label.IsEmpty()) continue;
      size_t j=0;
      while ((j < label.Length()) && olxstr::o_isdigit(label.CharAt(j))) {
        j++;
      }
      if (j == 0) continue;
      size_t cl = label.SubStringTo(j).ToUInt();
      if (cl > nextLabel) nextLabel = cl;
    }
  }
  return nextLabel+1;
}
//..............................................................................
void TGXApp::SynchroniseBonds(const TXAtomPList& xatoms)  {
  TXBondPList xbonds;
  for( size_t i=0; i < xatoms.Count(); i++ )
    xbonds.AddList(xatoms[i]->GetBonds(), StaticCastAccessor<TXBond>());
  // prepare unique list of bonds
  xbonds.ForEach(ACollectionItem::IndexTagSetter());
  xbonds.Pack(ACollectionItem::IndexTagAnalyser());
  // have to call setatom function to set the correct order for atom of bond
  for( size_t i=0; i < xbonds.Count(); i++ )
    xbonds[i]->SetA(xbonds[i]->A());
  // safety sake...
  for( size_t i=0; i < xbonds.Count(); i++ )  {
    TXBond& xb = *xbonds[i];
    xb.Update();
    xb.GetPrimitives().RemoveObject(xb);
    xb.Create(
      TXBond::GetLegend(xb,
        olx_max(TXAtom::LegendLevel(xb.A().GetPrimitives().GetName()),
        TXAtom::LegendLevel(xb.B().GetPrimitives().GetName()))));
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
  smatd I;
  I.r.I();
  UsedTransforms.AddCopy(I);

  vec3d MFrom(-1.5, -1.5, -1.5), MTo(2, 2, 2);
  smatd_plist matrices = XFile().GetLattice().GenerateMatrices(MFrom, MTo);

  vec3d VFrom = XFile().GetAsymmUnit().GetOCenter(false, false);
  for( size_t i=0; i < matrices.Count(); i++ )  {
    if( UsedTransforms.IndexOf(*matrices[i]) != InvalidIndex )  {
      delete matrices[i];
      continue;
    }
    vec3d VTo = VFrom * matrices[i]->r;
    VTo += matrices[i]->t;
    XFile().GetAsymmUnit().CellToCartesian(VTo);
    TXGrowPoint& gp = XGrowPoints.Add(
      new TXGrowPoint(*FGlRender, EmptyString(), VTo, *matrices[i]));
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
  TXAtomPList xatoms = FindXAtoms(atoms);
  // have to preprocess instructions like 'sel'
  olxstr ats;
  for( size_t i=0; i < xatoms.Count(); i++ )
    ats << xatoms[i]->GetLabel() << ' ';
  AtomsToGrow = ats;
  FGrowMode = v;
  UsedTransforms.Clear();
}
//..............................................................................
//..............................................................................
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
//..............................................................................
struct TGXApp_Transform {
  TCAtom* to;
  TXAtom* from;
  double dist;
  smatd transform;
  TGXApp_Transform() : to(NULL), from(NULL), dist(0) { }
};
//..............................................................................
struct TGXApp_Transform1 : public TGXApp_Transform {
  vec3d dest;
};
//..............................................................................
void TGXApp::CreateXGrowLines()  {
  if( !XGrowLines.IsEmpty() )  {  // clear the existing ones...
    TPtrList<TGPCollection> colls; // list of unique collections
    AGDObjList lines(XGrowLines.Count()*2);  // list of the AGDrawObject pointers to lines...
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      XGrowLines[i].GetPrimitives().SetTag(i);
      XGrowLines[i].GetGlLabel().GetPrimitives().SetTag(i);
      lines.Set(i*2, XGrowLines[i]);
      lines.Set(i*2+1, XGrowLines[i].GetGlLabel());
    }
    for( size_t i=0; i < XGrowLines.Count(); i++ )  {
      if( (size_t)XGrowLines[i].GetPrimitives().GetTag() == i )
        colls.Add(XGrowLines[i].GetPrimitives());
      if( (size_t)XGrowLines[i].GetGlLabel().GetPrimitives().GetTag() == i )
        colls.Add(XGrowLines[i].GetGlLabel().GetPrimitives());
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
  TGXApp_CrdMap CrdMap;
  TXAtomPList AtomsToProcess;
  if( !AtomsToGrow.IsEmpty() )
    AtomsToProcess.AddList(FindXAtoms(AtomsToGrow));
  else if( (FGrowMode & gmSameAtoms) == 0 ) {
    const size_t ac = FXFile->GetLattice().GetObjects().atoms.Count();
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& A = XFile().GetLattice().GetObjects().atoms[i];
      if( A.IsDeleted() || !A.CAtom().IsAvailable() )  continue;
      AtomsToProcess.Add(static_cast<TXAtom&>(A));
    }
  }
  for( size_t i=0; i < AtomsToProcess.Count(); i++ )  {
    if( AtomsToProcess[i]->GetNetwork().GetLattice() != FXFile->GetLattice() )  {
      AtomsToProcess[i] = NULL;
      continue;
    }
    CrdMap.Add(AtomsToProcess[i]->crd());
  }
  AtomsToProcess.Pack();
  TTypeList<TGXApp_Transform1> tr_list;
  typedef TArrayList<AnAssociation2<TCAtom*,smatd> > GInfo;
  TPtrList<GInfo> Info(au.AtomCount());
  for( size_t i=0; i < AtomsToProcess.Count(); i++ )  {
    TXAtom* A = AtomsToProcess[i];
    if( FGrowMode == gmCovalent && A->IsGrown() )
      continue;
    TPtrList<TCAtom> AttachedAtoms;
    if( (FGrowMode & gmSInteractions) != 0 )  {
      for( size_t j=0; j < A->CAtom().AttachedSiteICount(); j++ )
        if( A->CAtom().GetAttachedAtomI(j).IsAvailable() )
          AttachedAtoms.Add(A->CAtom().GetAttachedAtomI(j));
    }
    if( (FGrowMode & gmSameAtoms) != 0 )
      AttachedAtoms.Add(A->CAtom());

    if( AttachedAtoms.IsEmpty() && (FGrowMode & gmCovalent) == 0 )  continue;
    GInfo* gi = Info[A->CAtom().GetId()];
    if( gi == NULL )  {
      gi = new GInfo;
      if( FGrowMode == gmSameAtoms )  {
        uc.FindInRangeAM(A->CAtom().ccrd(), 2*A->GetType().r_bonding + 15,
          *gi, &AttachedAtoms);
      }
      else if( FGrowMode == gmSInteractions )  {
        uc.FindBindingAM(A->CAtom(), FXFile->GetLattice().GetDeltaI(), *gi,
          &AttachedAtoms);
      }
      else  {
        for( size_t j=0; j < A->CAtom().AttachedSiteCount(); j++ )  {
          if( !A->CAtom().GetAttachedAtom(j).IsAvailable() )  continue;
          TCAtom::Site& site = A->CAtom().GetAttachedSite(j);
          gi->Add(AnAssociation2<TCAtom*,smatd>(site.atom, site.matrix));
        }
      }
      Info[A->CAtom().GetId()] = gi;
    }
    for( size_t j=0; j < gi->Count(); j++ )  {
      const AnAssociation2<TCAtom*,smatd>& gii = (*gi)[j];
      smatd transform = (A->GetMatrix().IsFirst() ? gii.GetB()
        : uc.MulMatrix(gii.GetB(), A->GetMatrix()));
      vec3d tc = transform*gii.GetA()->ccrd();
      au.CellToCartesian(tc);
      const double qdist = tc.QDistanceTo(A->crd());
      if( qdist < 1e-2 || CrdMap.Exists(tc) )
        continue;
      bool uniq = true;
      for( size_t l=0; l < tr_list.Count(); l++ )  {
        if( tr_list[l].transform.GetId() == transform.GetId() &&
            tr_list[l].to == gii.GetA() )
        {
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
    TXGrowLine& gl = XGrowLines.Add(
      new TXGrowLine(*FGlRender, EmptyString(), *nt.from, *nt.to, nt.transform));
    gl.Create("GrowBonds");
  }
  Info.DeleteItems(true);
}
//..............................................................................
void TGXApp::_CreateXGrowVLines()  {
  if( !XGrowLines.IsEmpty() )  return;
  const TAsymmUnit& au = FXFile->GetAsymmUnit();
  const TUnitCell& uc = FXFile->GetUnitCell();
  TGXApp_CrdMap CrdMap;
  TXAtomPList AtomsToProcess;
  if( !AtomsToGrow.IsEmpty() )  {
    AtomsToProcess.AddList(FindXAtoms(AtomsToGrow));
    const size_t ac = FXFile->GetLattice().GetObjects().atoms.Count();
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& A = FXFile->GetLattice().GetObjects().atoms[i];
      if( A.IsDeleted() || !A.CAtom().IsAvailable() )  continue;
      CrdMap.Add(A.crd());
    }
  }
  else  {
    const size_t ac = FXFile->GetLattice().GetObjects().atoms.Count();
    for( size_t i=0; i < ac; i++ )  {
      TSAtom& A = XFile().GetLattice().GetObjects().atoms[i];
      if( A.IsDeleted() || !A.CAtom().IsAvailable() )  continue;
      AtomsToProcess.Add(static_cast<TXAtom&>(A));
      CrdMap.Add(A.crd());
    }
  }
  typedef TTypeList<TGXApp_Transform> tr_list;
  olxdict<int, tr_list, TPrimitiveComparator> net_tr;
  TPtrList<TArrayList<AnAssociation2<TCAtom*,smatd> > > Info(au.AtomCount());
  for( size_t i=0; i < AtomsToProcess.Count(); i++ )  {
    TXAtom* A = AtomsToProcess[i];
    TArrayList<AnAssociation2<TCAtom*,smatd> >* envi = Info[A->CAtom().GetId()];
    if( envi == NULL )  {
      Info[A->CAtom().GetId()] = envi = new TArrayList<AnAssociation2<TCAtom*,smatd> >;
      uc.FindInRangeAM(A->CAtom().ccrd(), DeltaV + A->GetType().r_bonding, *envi);
    }
    for( size_t j=0; j < envi->Count(); j++ )  {
      TCAtom *aa = envi->GetItem(j).GetA();
      if( !aa->IsAvailable() )  continue;
      const smatd transform = (A->GetMatrix().IsFirst() ? envi->GetItem(j).GetB() :
        uc.MulMatrix(envi->GetItem(j).GetB(), A->GetMatrix()));
      if( !aa->IsAvailable() )  continue;
      const vec3d& cc = aa->ccrd();
      vec3d tc = transform*cc;
      au.CellToCartesian(tc);
      const double qdist = tc.QDistanceTo(A->crd());
      if( qdist < 0.5 || CrdMap.Exists(tc) )  // check if point to one of already existing
        continue;
      tr_list& ntl = net_tr.Add(aa->GetFragmentId());
      //find the shortest one
      bool uniq = true;
      //find the shortest one
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
      }
    }
  }
  for( size_t i=0; i < net_tr.Count(); i++ )  {
    const tr_list& ntl = net_tr.GetValue(i);
    for( size_t j=0; j < ntl.Count(); j++ )  {
      TGXApp_Transform& nt = ntl[j];
      TXGrowLine& gl = XGrowLines.Add(
        new TXGrowLine(*FGlRender, EmptyString(), *nt.from, *nt.to, nt.transform));
      gl.Create("GrowBonds");
    }
  }
  Info.DeleteItems(true);
}
//..............................................................................
void TGXApp::Grow(const TXGrowPoint& growPoint)  {
  UsedTransforms.AddCopy(growPoint.GetTransform());
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
  unsigned char* RGBa, unsigned int format)
{
  TGlBitmap* glB = FindGlBitmap(name);
  if( glB == NULL )  {
    glB = new TGlBitmap(*FGlRender, name, left, top, width, height, RGBa, format );
    GlBitmaps.Add(glB);
    glB->Create();
    ObjectsToCreate.Add( (AGDrawObject*)glB );
    glB->SetZ(-FGlRender->GetMaxRasterZ() + (double)GlBitmaps.Count()/100);
  }
  else  {
    glB->ReplaceData(width, height, RGBa, format);
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
    size_t ind = ObjectsToCreate.IndexOf((AGDrawObject*)glb);
    if( ind != InvalidIndex )
      ObjectsToCreate.Delete(ind);
    glb->GetPrimitives().RemoveObject(*glb);
    FGlRender->RemoveObject(*glb);
    delete glb;
  }
}
//..............................................................................
TXFile& TGXApp::NewOverlayedXFile() {
  return OverlayedXFiles.Add((TXFile*)FXFile->Replicate());
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
  FXFile->GetLattice().OnAtomsDeleted.TakeOver(OverlayedXFiles[i].GetLattice().OnAtomsDeleted);
  AlignOverlayedXFiles();
  CreateObjects(true);
  DUnitCell().SetReciprocal(false);
}
//..............................................................................
void TGXApp::CalcLatticeRandCenter(const TLattice& latt, double& maxR, vec3d& cnt)  {
  maxR = 0;
  cnt.Null();
  const size_t ac = latt.GetObjects().atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom& sa = latt.GetObjects().atoms[i];
    cnt += (sa.crd() = latt.GetAsymmUnit().Orthogonalise(sa.ccrd()));
  }

  if( ac != 0 )
    cnt /= ac;
  for( size_t i=0; i < ac; i++ )  {
    const double r = cnt.QDistanceTo(latt.GetObjects().atoms[i].crd());
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
  size_t dim = olx_round(sqrt((double)OverlayedXFiles.Count()+1));
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
        row.Add(new grid_type(maxR, cnt, &XFile().GetLattice()));
      }
      else if( ind-1 >= OverlayedXFiles.Count() )
        break;
      else  {
        CalcLatticeRandCenter(OverlayedXFiles[ind-1].GetLattice(), maxR, cnt);
        row.Add(new grid_type(maxR, cnt, &OverlayedXFiles[ind-1].GetLattice()));
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
      if( i == 0 && j == 0 )  continue;
      if( j+1 > grid[i].Count() )  break;
      vec3d shift_vec = (grid[0][0].GetB()-grid[i][j].GetB());
      shift_vec += up_shift*row_height[i];
      shift_vec += right_shift*col_width[j];
      TLattice& latt = *grid[i][j].GetC();
      for( size_t k=0; k < latt.GetObjects().atoms.Count(); k++ )
        latt.GetObjects().atoms[k].crd() += shift_vec;
    }
  }
}
//..............................................................................
void TGXApp::DeleteOverlayedXFile(size_t index) {
  const TLattice& latt = OverlayedXFiles[index].GetLattice();
  for( size_t i = 0; i < GroupDefs.Count(); i++ )  {
    for( size_t j=0; j < GroupDefs[i].atoms.Count(); j++ )  {
      if( GroupDefs[i].atoms[j].latt == latt )
        GroupDefs[i].atoms.NullItem(j);
    }
    GroupDefs[i].atoms.Pack();
    for( size_t j=0; j < GroupDefs[i].bonds.Count(); j++ )  {
      if( GroupDefs[i].bonds[j].latt == latt )
        GroupDefs[i].bonds.NullItem(j);
    }
    GroupDefs[i].bonds.Pack();
  }
  for( size_t i=0; i < LabelInfo.atoms.Count(); i++ )  {
    if( LabelInfo.atoms[i].latt == latt )
      LabelInfo.atoms.NullItem(i);
  }
  LabelInfo.atoms.Pack();
  for( size_t i=0; i < LabelInfo.bonds.Count(); i++ )  {
    if( LabelInfo.bonds[i].latt == latt )
      LabelInfo.bonds.NullItem(i);
  }
  LabelInfo.bonds.Pack();
  OverlayedXFiles.Delete(index);
  CreateObjects(false);
  CenterView();
  Draw();
}
//..............................................................................
void TGXApp::DeleteOverlayedXFiles() {
  const TLattice& latt = XFile().GetLattice();
  for( size_t i = 0; i < GroupDefs.Count(); i++ )  {
    for( size_t j=0; j < GroupDefs[i].atoms.Count(); j++ )  {
      if( GroupDefs[i].atoms[j].latt != latt )
        GroupDefs[i].atoms.NullItem(j);
    }
    GroupDefs[i].atoms.Pack();
    for( size_t j=0; j < GroupDefs[i].bonds.Count(); j++ )  {
      if( GroupDefs[i].bonds[j].latt != latt )
        GroupDefs[i].bonds.NullItem(j);
    }
    GroupDefs[i].bonds.Pack();
  }
  for( size_t i=0; i < LabelInfo.atoms.Count(); i++ )  {
    if( LabelInfo.atoms[i].latt != latt )
      LabelInfo.atoms.NullItem(i);
  }
  LabelInfo.atoms.Pack();
  for( size_t i=0; i < LabelInfo.bonds.Count(); i++ )  {
    if( LabelInfo.bonds[i].latt != latt )
      LabelInfo.bonds.NullItem(i);
  }
  LabelInfo.bonds.Pack();
}
//..............................................................................
void TGXApp::UpdateBonds()  {
  BondIterator bi(*this);
  while( bi.HasNext() )
    bi.Next().Update();
}
//..............................................................................
TXLattice& TGXApp::AddLattice(const olxstr& Name, const mat3d& basis)  {
  TXLattice *XL = new TXLattice(*FGlRender, Name);
  XL->SetLatticeBasis(basis);
  XL->Create();
  LooseObjects.Add(XL);
  return *XL;
}
//..............................................................................
void TGXApp::InitFadeMode()  {
}
//..............................................................................
struct SceneMaskTask : public TaskBase {
  TAsymmUnit &au;
  TTypeList<AnAssociation2<vec3d,double> >& atoms;
  TArray3D<vec3d> &cmap;
  TArray3D<bool>& mdata;
  size_t da, db, dc;
public:
    SceneMaskTask(TAsymmUnit &au,
      TTypeList<AnAssociation2<vec3d,double> >& atoms,
      TArray3D<vec3d> &cmap,
      TArray3D<bool>& mdata)
      : au(au), cmap(cmap), atoms(atoms), mdata(mdata)
    {
      da = mdata.Length1();
      db = mdata.Length2();
      dc = mdata.Length3();
    }
    void Run(size_t ind) const {
      for (size_t j = 0; j < da; j++) {
        for (size_t k = 0; k < db; k++) {
          for (size_t l = 0; l < dc; l++) {
            if (cmap.Data[j][k][l].QDistanceTo(atoms[ind].GetA()) <=
                atoms[ind].GetB())
            {
              mdata.Data[j][k][l] = true;
            }
          }
        }
      }
    }
    SceneMaskTask* Replicate() const {
      return new SceneMaskTask(au, atoms, cmap, mdata);
    }

};

void TGXApp::BuildSceneMask(FractMask& mask, double inc)  {
  TStopWatch st(__FUNC__);
  st.start("Initialising");
  TAsymmUnit& au = XFile().GetAsymmUnit();
  vec3d mn(100, 100, 100),
        mx(-100, -100, -100),
        norms(au.GetAxes());
  TTypeList<AnAssociation2<vec3d,double> > atoms;
  AtomIterator ai(*this);
  atoms.SetCapacity(ai.count);
  while (ai.HasNext()) {
    TXAtom& xa = ai.Next();
    if (xa.IsDeleted() || !xa.IsVisible()) continue;
    vec3d::UpdateMinMax(xa.ccrd(), mn, mx);
    atoms.AddNew(xa.crd(), olx_sqr(xa.GetType().r_vdw)+inc);
  }
  mn -= 1./4;
  mx += 1./4;
  double res = 0.5;
  mask.Init(mn, mx, norms, res);
  norms /= res;
  TArray3D<bool>* mdata = mask.GetMask();
  mdata->FastInitWith(0);
  TArray3D<vec3d> cmap(0, mdata->Length1(),
    0, mdata->Length2(),
    0, mdata->Length3());
  for (size_t j = 0; j < mdata->Length1(); j++) {
    const double dx = (double)j/norms[0] + mn[0];
    for (size_t k = 0; k < mdata->Length2(); k++) {
      const double dy = (double)k/norms[1] + mn[1];
      for (size_t l = 0; l < mdata->Length3(); l++) {
        cmap.Data[j][k][l] = au.Orthogonalise(
          vec3d(dx, dy, (double)l/norms[2]+mn[2]));
      }
    }
  }
  st.start("Building scene mask");
  SceneMaskTask task(au, atoms, cmap, *mdata);
  TListIteratorManager<SceneMaskTask> im(task, atoms.Count(), tLinearTask, 10);
}
//..............................................................................
void TGXApp::SaveStructureStyle(TDataItem& item) const {
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
      styles.Add(gs);
  }
  styles.Add(TXAtom::GetParamStyle());
  styles.Add(TXBond::GetParamStyle());
  FGlRender->GetStyles().ToDataItem(item.AddItem("Style"), styles);
  FGlRender->GetScene().ToDataItem(item.AddItem("Scene"));
  TDataItem& ind_col = item.AddItem("ICollections");
  for( size_t i=0; i < IndividualCollections.Count(); i++ )
    ind_col.AddField( olxstr("col_") << i, IndividualCollections[i]);
}
//..............................................................................
void TGXApp::ToDataItem(TDataItem& item, IOutputStream& zos) const {
  TSizeList LattAtomSz(LattCount()),
    LattBondSz(LattCount());
  XFile().ToDataItem(item.AddItem("XFile"));
  LattAtomSz[0] = CalcMaxAtomTag(XFile().GetLattice());
  LattBondSz[0] = CalcMaxBondTag(XFile().GetLattice());
  if( !OverlayedXFiles.IsEmpty() )  {
    TDataItem& overlays = item.AddItem("Overlays");
    for( size_t i=0; i < OverlayedXFiles.Count(); i++ )  {
      OverlayedXFiles[i].ToDataItem(overlays.AddItem(i));
      LattAtomSz[i+1] = CalcMaxAtomTag(OverlayedXFiles[i].GetLattice());
      LattBondSz[i+1] = CalcMaxBondTag(OverlayedXFiles[i].GetLattice());
    }
  }
  SaveStructureStyle(item);
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
  AtomIterator ai(*this);
  while (ai.HasNext()) {
    if (!ai.Next().IsDeleted())
      a_cnt++;
  }
  TEBitArray vis(a_cnt);
  TPtrList<TXGlLabel> atom_labels(a_cnt);
  a_cnt = 0;
  ai.Reset();
  while (ai.HasNext()) {
    TXAtom& xa = ai.Next();
    if (!xa.IsDeleted()) {
      atom_labels.Set(a_cnt, xa.GetGlLabel());
      vis.Set(a_cnt++, xa.IsVisible());
    }
  }
  visibility.AddField("atoms", vis.ToBase64String());
  size_t b_cnt = 0;
  BondIterator bi(*this);
  while (bi.HasNext()) {
    if (!bi.Next().IsDeleted())
      b_cnt++;
  }
  vis.SetSize(b_cnt);
  TPtrList<TXGlLabel> bond_labels(b_cnt);
  b_cnt = 0;
  bi.Reset();
  while (bi.HasNext()) {
    TXBond& xb = bi.Next();
    if (!xb.IsDeleted()) {
      bond_labels.Set(b_cnt, xb.GetGlLabel());
      vis.Set(b_cnt++, xb.IsVisible());
    }
  }
  visibility.AddField("bonds", vis.ToBase64String());
  size_t p_cnt = 0;
  PlaneIterator pi(*this);
  while (pi.HasNext())
    if (!pi.Next().IsDeleted())
      p_cnt++;
  vis.SetSize(p_cnt);
  p_cnt = 0;
  pi.Reset();
  while (pi.HasNext()) {
    TXPlane& xp = pi.Next();
    if (!xp.IsDeleted())
      vis.Set(p_cnt++, xp.IsVisible());
  }
  visibility.AddField("planes", vis.ToBase64String());
  
  F3DFrame->ToDataItem(item.AddItem("3DFrame"));
  FXGrid->ToDataItem(item.AddItem("Grid"), zos);
  FDBasis->ToDataItem(item.AddItem("DBasis"));

  TDataItem& labels = item.AddItem("Labels");
  for (size_t i=0; i < XLabels.Count(); i++)
    XLabels[i].ToDataItem(labels.AddItem("Label"));
  TDataItem& atom_labels_item = item.AddItem("AtomLabels");
  for (size_t i=0; i < atom_labels.Count(); i++)
    atom_labels[i]->ToDataItem(atom_labels_item.AddItem("Label"));
  TDataItem& bond_labels_item = item.AddItem("BondLabels");
  for (size_t i=0; i < bond_labels.Count(); i++)
    bond_labels[i]->ToDataItem(bond_labels_item.AddItem("Label"));

  FGlRender->GetSelection().SetTag(-1);
  FGlRender->GetGroups().ForEach(ACollectionItem::IndexTagSetter());
  
  TDataItem& groups = item.AddItem("Groups");
  for (size_t i=0; i < FGlRender->GroupCount(); i++) {
    TGlGroup& glG = FGlRender->GetGroup(i);
    TDataItem& group = groups.AddItem(i, glG.GetCollectionName());
    group.AddField("visible", glG.IsVisible());
    group.AddField("parent_id", glG.GetParentGroup() == NULL ? -2
      : glG.GetParentGroup()->GetTag());
    IndexRange::Builder ra, rb;
    for (size_t j=0; j < glG.Count(); j++) {
      AGDrawObject& glO = glG.GetObject(j);
      if (EsdlInstanceOf(glO, TXAtom)) {
        ra << GetAtomTag(((TXAtom&)glO), LattAtomSz);
      }
      else if (EsdlInstanceOf(glO, TXBond)) {
        rb << GetBondTag(((TXBond&)glO), LattBondSz);
      }
    }
    group.AddField("atom_range", ra.GetString());
    group.AddField("bond_range", rb.GetString());
  }

  TDataItem &lines = item.AddItem("Lines");
  for (size_t i=0; i < Lines.Count(); i++) {
    if (Lines[i].IsVisible())
      Lines[i].ToDataItem(lines.AddItem("object"));
  }

  TDataItem &user_objects = item.AddItem("UserObjects");
  for (size_t i=0; i < UserObjects.Count(); i++) {
    if (UserObjects[i].IsVisible())
      UserObjects[i].ToDataItem(user_objects.AddItem("object"));
  }
  TDataItem &rings = item.AddItem("Rings");
  for (size_t i = 0; i < Rings.Count(); i++) {
    if (Rings[i].IsVisible())
      Rings[i].ToDataItem(rings.AddItem("object"));
  }

  TDataItem& renderer = item.AddItem("Renderer");
  renderer.AddField("min", PersUtil::VecToStr(FGlRender->MinDim()));
  renderer.AddField("max", PersUtil::VecToStr(FGlRender->MaxDim()));
}
//..............................................................................
void TGXApp::LoadStructureStyle(const TDataItem &item) {
  FGlRender->GetStyles().FromDataItem(item.GetItemByName("Style"), true);
  TDataItem *scene_item = item.FindItem("Scene");
  if( scene_item != NULL )
    FGlRender->GetScene().FromDataItem(*scene_item);
  IndividualCollections.Clear();
  TDataItem& ind_col = item.GetItemByName("ICollections");
  for (size_t i=0; i < ind_col.FieldCount(); i++)
    IndividualCollections.Add(ind_col.GetFieldByIndex(i));
}
//..............................................................................
void TGXApp::FromDataItem(TDataItem& item, IInputStream& zis)  {
  FGlRender->Clear();
  ClearXObjects();
  ClearLabels();
  ClearLines();
  LabelInfo.Clear();
  ClearGroupDefinitions();
  OverlayedXFiles.Clear();
  UserObjects.Clear();
  Rings.Clear();
  TXAtom::TelpProb(0);  //force re-reading
  TXAtom::DefRad(0);
  TXAtom::DefDS(0);

  FXFile->FromDataItem(item.GetItemByName("XFile"));
  TDataItem* overlays = item.FindItem("Overlays");
  if (overlays != NULL) {
    for (size_t i=0; i < overlays->ItemCount(); i++)
      NewOverlayedXFile().FromDataItem(overlays->GetItemByIndex(i));
  }
  LoadStructureStyle(item);
  const TDataItem& labels = item.GetItemByName("Labels");
  for (size_t i=0; i < labels.ItemCount(); i++) {
    XLabels.Add(
      new TXGlLabel(
      *FGlRender, PLabelsCollectionName)).FromDataItem(
        labels.GetItemByIndex(i));
  }
  bool vis;
  TDataItem *frame_i = item.FindItem("3DFrame");
  if (frame_i != NULL) {
    vis = F3DFrame->IsVisible();
    F3DFrame->FromDataItem(*frame_i);
    if (vis != F3DFrame->IsVisible())
      OnGraphicsVisible.Execute(dynamic_cast<TBasicApp*>(this), F3DFrame);
  }
  vis = FXGrid->IsVisible();
  FXGrid->FromDataItem(item.GetItemByName("Grid"), zis);
  if (vis != FXGrid->IsVisible())
    OnGraphicsVisible.Execute(dynamic_cast<TBasicApp*>(this), FXGrid);

  FDBasis->FromDataItem(item.GetItemByName("DBasis"));
  {
    TDataItem *lines = item.FindItem("Lines");
    if (lines != NULL) {
      for (size_t i = 0; i < lines->ItemCount(); i++) {
        Lines.Add(new TXLine(*FGlRender))
          .FromDataItem(lines->GetItemByIndex(i));
      }
    }
  }
  {
    TDataItem *user_objects = item.FindItem("UserObjects");
    if (user_objects != NULL) {
      for (size_t i = 0; i < user_objects->ItemCount(); i++) {
        UserObjects.Add(
          new TDUserObj(*FGlRender, user_objects->GetItemByIndex(i)));
      }
    }
  }
  {
    const TDataItem *rings = item.FindItem("Rings");
    if (rings != NULL) {
      for (size_t i = 0; i < rings->ItemCount(); i++) {
        Rings.Add(new TDRing(GetRender(), EmptyString()))
          .FromDataItem(rings->GetItemByIndex(i));
      }
    }
  }

  TDataItem& visibility = item.GetItemByName("Visibility");
  FHydrogensVisible = visibility.GetFieldByName("h_atoms").ToBool();
  FHBondsVisible = visibility.GetFieldByName("h_bonds").ToBool();
  FQPeaksVisible = visibility.GetFieldByName("q_atoms").ToBool();
  FQPeakBondsVisible = visibility.GetFieldByName("q_bonds").ToBool();
  CreateObjects(true);
  vis = FDBasis->IsVisible();
  FDBasis->SetVisible(visibility.GetFieldByName("basis").ToBool());
  if (vis != FDBasis->IsVisible())
    OnGraphicsVisible.Execute(dynamic_cast<TBasicApp*>(this), FDBasis);

  vis = FDUnitCell->IsVisible();
  FDUnitCell->SetVisible(visibility.GetFieldByName("cell").ToBool());
  if (vis != FDUnitCell->IsVisible())
    OnGraphicsVisible.Execute(dynamic_cast<TBasicApp*>(this), FDUnitCell);

  const TDataItem* atom_labels = item.FindItem("AtomLabels");
  if (atom_labels != NULL) {
    AtomIterator ai(*this);
    if (ai.count != atom_labels->ItemCount())
      throw TFunctionFailedException(__OlxSourceInfo, "integrity is broken");
    size_t i=0;
    while (ai.HasNext())
      ai.Next().GetGlLabel().FromDataItem(atom_labels->GetItemByIndex(i++));
  }
  const TDataItem* bond_labels = item.FindItem("BondLabels");
  if (bond_labels != NULL) {
    BondIterator bi(*this);
    if (bi.count != bond_labels->ItemCount())
      throw TFunctionFailedException(__OlxSourceInfo, "integrity is broken");
    size_t i = 0;
    while (bi.HasNext())
      bi.Next().GetGlLabel().FromDataItem(bond_labels->GetItemByIndex(i++));
  }
  //// restore 

  //BondIterator bonds = GetBonds();
  //while( bonds.HasNext() )
  //  bonds.Next().Update();
  const TDataItem& groups = item.GetItemByName("Groups");
  // pre-create all groups
  GroupDict.Clear();
  for (size_t i=0; i < groups.ItemCount(); i++)
    FGlRender->NewGroup(groups.GetItemByIndex(i).GetValue());
  // load groups
  for (size_t i=0; i < groups.ItemCount(); i++) {
    const TDataItem& group = groups.GetItemByIndex(i);
    TGlGroup& glG = FGlRender->GetGroup(i);
    glG.SetVisible(group.GetFieldByName("visible").ToBool());
    const int p_id = group.GetFieldByName("parent_id").ToInt();
    if (p_id == -1)
      FGlRender->GetSelection().Add(glG);
    else if (p_id >= 0)
      FGlRender->GetGroup(p_id).Add(glG);
    // compatibility
    TDataItem* atoms = group.FindItem("Atoms");
    if (atoms != NULL) {
      glG.IncCapacity(atoms->FieldCount());
      for (size_t j = 0; j < atoms->FieldCount(); j++)
        glG.Add(GetXAtom(atoms->GetFieldByIndex(j).ToSizeT()));
      TDataItem& bonds = group.GetItemByName("Bonds");
      glG.IncCapacity(bonds.FieldCount());
      for (size_t j = 0; j < bonds.FieldCount(); j++)
        glG.Add(GetXBond(bonds.GetFieldByIndex(j).ToSizeT()));
    }
    else {
      IndexRange::RangeItr ai(group.GetFieldByName("atom_range"));
      glG.IncCapacity(ai.CalcSize());
      while (ai.HasNext()) {
        glG.Add(GetXAtom(ai.Next()));
      }
      ai = IndexRange::RangeItr(group.GetFieldByName("bond_range"));
      glG.IncCapacity(ai.CalcSize());
      while (ai.HasNext()) {
        glG.Add(GetXBond(ai.Next()));
      }
    }
    glG.Create(group.GetValue());
    StoreGroup(glG, GroupDefs.AddNew());
    GroupDict(&glG, GroupDefs.Count()-1);
  }
  _UpdateGroupIds();
  TDataItem& renderer = item.GetItemByName("Renderer");
  vec3d min, max;
  PersUtil::VecFromStr(renderer.GetFieldByName("min"), min);
  PersUtil::VecFromStr(renderer.GetFieldByName("max"), max);
  FGlRender->SetSceneComplete(false);
  FGlRender->ClearMinMax();
  FGlRender->UpdateMinMax(min, max);
  FGlRender->GetBasis().FromDataItem(item.GetItemByName("Basis"));
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
    zos.PutNextEntry(wxT("grid"));
    TwxOutputStreamWrapper os(zos);
    ToDataItem(mi, os);
    zos.CloseEntry();
    zos.PutNextEntry(wxT("model"));
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
  wxZipInputStream zin(fis);
  fis.Read(sig, 3);
  if( olxstr::o_memcmp(sig, "oxm", 3) != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file signature");

  wxZipEntry* model = NULL, *grid = NULL, *zen;
  olxstr entryModel("model"), entryGrid("grid");

  while( (zen = zin.GetNextEntry()) != NULL )  {
    if( entryModel == zen->GetName() )
      model = zen;
    else if( entryGrid == zen->GetName() )
      grid = zen;
    else
      delete zen;
  }
  if( model == NULL || grid == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid model file description");
  zin.OpenEntry(*model);
  uint32_t contentLen = zin.GetLength();
  unsigned char * bf = new unsigned char[contentLen + 1];
  zin.Read(bf, contentLen);
  TEMemoryInputStream ms(bf, contentLen);
  TDataFile df;
  df.LoadFromTextStream(ms);
  delete [] bf;
  zin.OpenEntry(*grid);
  TwxInputStreamWrapper in(zin);
  try  {
    FromDataItem(df.Root().GetItemByName("olex_model"), in);
  }
  catch(const TExceptionBase& exc)  {
    NewLogEntry(logException) << "Failed to load model: " << exc.GetException()->GetError();
    FXFile->SetLastLoader(NULL);
    FXFile->LastLoaderChanged();
    CreateObjects(false);
  }
  delete model;
  delete grid;
#else
  throw TNotImplementedException(__OlxSourceInfo);
#endif
}
//..............................................................................
TGlGroup *TGXApp::GroupSelection(const olxstr& name)  {
  TGlGroup* glg = GetRender().GroupSelection(name);
  if( glg != NULL )  {
    StoreGroup(*glg, GroupDefs.AddNew());
    _UpdateGroupIds();
    Draw();
  }
  return glg;
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
  if( i != InvalidIndex )
    GroupDefs.Delete(GroupDict.GetValue(i));
  GetRender().UnGroup(G);
  _UpdateGroupIds();
  Draw();
}
//..............................................................................
void TGXApp::_UpdateGroupIds()  {
  FGlRender->GetGroups().ForEach(ACollectionItem::IndexTagSetter());
  FGlRender->GetSelection().SetTag(-1);
  GroupDict.Clear();
  for( size_t i=0; i < GroupDefs.Count(); i++ )  {
    TGlGroup* p = FGlRender->GetGroup(i).GetParentGroup();
    GroupDefs[i].parent_id = (p == NULL ? -2 : p->GetTag());
    GroupDict(&FGlRender->GetGroup(i), i);
  }
}
//..............................................................................
void TGXApp::SelectAll(bool Select)  {
  if( !Select )  {
    if( !SelectionCopy[0].IsEmpty() )  {
      SelectionCopy[1] = SelectionCopy[0];
      SelectionCopy[0].Clear();
    }
    else  {
      SelectionCopy[1].Clear();
      StoreGroup(GetSelection(), SelectionCopy[1]);
    }
  }
  GetRender().SelectAll(Select);
  _UpdateGroupIds();
  Draw();
}
//..............................................................................
const_strlist TGXApp::ToPov() const {
  olxdict<TGlMaterial, olxstr, TComparableComparator> materials;
  TStrList out = GetRender().GetScene().ToPov();
  out << TXAtom::PovDeclare();
  out << TXBond::PovDeclare();
  out << TXPlane::PovDeclare();
  out.Add("union {");
  TGXApp::AtomIterator ai = GetAtoms();
  while( ai.HasNext() )  {
    TXAtom &a = ai.Next();
    if( a.IsVisible() )
      out << a.ToPov(materials);
  }
  TGXApp::BondIterator bi = GetBonds();
  while( bi.HasNext() )  {
    TXBond &b = bi.Next();
    if( b.IsVisible() )
      out << b.ToPov(materials);
  }
  TGXApp::PlaneIterator pi = GetPlanes();
  while( pi.HasNext() )  {
    TXPlane &p = pi.Next();
    if( p.IsVisible() )
      out << p.ToPov(materials);
  }
  for( size_t i=0; i < Lines.Count(); i++ )  {
    if( Lines[i].IsVisible() )
      out << Lines[i].ToPov(materials);
  }
  for( size_t i=0; i < XGrowLines.Count(); i++ )  {
    if( XGrowLines[i].IsVisible() )
      out << XGrowLines[i].ToPov(materials);
  }
  if( XGrid().IsVisible() && !XGrid().IsEmpty() )
    out << XGrid().ToPov(materials);
  for (size_t i=0; i < UserObjects.Count(); i++)
    out << UserObjects[i].ToPov(materials);

  if (DUnitCell().IsVisible())
    out << DUnitCell().ToPov(materials);
  out.Add("}");
  TStrList mat_out;
  for( size_t i=0; i < materials.Count(); i++ )  {
    mat_out.Add("#declare ") << materials.GetValue(i) << '=';
    mat_out.Add(materials.GetKey(i).ToPOV());
  }
  return mat_out << out;
}
//..............................................................................
const_strlist TGXApp::ToWrl() const {
  olxdict<TGlMaterial, olxstr, TComparableComparator> materials;
  TStrList out;// = GetRender().GetScene().ToPov();
  out << TXAtom::WrlDeclare();
  out << TXBond::WrlDeclare();
  TGXApp::AtomIterator ai = GetAtoms();
  while (ai.HasNext()) {
    TXAtom &a = ai.Next();
    if (a.IsVisible())
      out << a.ToWrl(materials);
  }
  TGXApp::BondIterator bi = GetBonds();
  while (bi.HasNext()) {
    TXBond &b = bi.Next();
    if (b.IsVisible())
      out << b.ToWrl(materials);
  }
  for (size_t i=0; i < Lines.Count(); i++) {
    if (Lines[i].IsVisible())
      out << Lines[i].ToWrl(materials);
  }
  for (size_t i=0; i < XGrowLines.Count(); i++) {
    if (XGrowLines[i].IsVisible())
      out << XGrowLines[i].ToWrl(materials);
  }
  if (DUnitCell().IsVisible())
    out << DUnitCell().ToWrl(materials);
  out.Insert(0, "#Created by Olex2 ") << XLibMacros::GetCompilationInfo();
  out.Insert(0, "#VRML V2.0 utf8");
  return out;
  //TGXApp::PlaneIterator pi = GetPlanes();
  //while( pi.HasNext() )  {
  //  TXPlane &p = pi.Next();
  //  if( p.IsVisible() )
  //    out << p.ToPov(materials);
  //}
  //if( XGrid().IsVisible() && !XGrid().IsEmpty() )
  //  out << XGrid().ToPov(materials);
  //for (size_t i=0; i < UserObjects.Count(); i++)
  //  out << UserObjects[i].ToPov(materials);

  //out.Add("}");
}
//..............................................................................
void TGXApp::CreateRings(bool force, bool create) {
  if (!force &&
    !TBasicApp::Options.FindValue("aromatic_rings", FalseString()).ToBool())
  {
    return;
  }
  TGraphicsStyle *cgs = GetRender().GetStyles().FindStyle('C');
  TGlMaterial *glm = NULL;
  if (cgs != NULL)
    glm = cgs->FindMaterial("Sphere");
  TStrList str_rings(TBasicApp::GetOptions().FindValue(
    "aromatic_rings_def", "C5,C6,NC5,SC4,N2C3,NCNC2"), ',');
  TTypeList<TSAtomPList> rings;
  for (size_t i=0; i < str_rings.Count(); i++) {
    FindRings(str_rings[i], rings);
  }
  for (size_t i=0; i < rings.Count(); i++) {
    vec3d normal, center;
    if (TSPlane::CalcPlane(rings[i], normal, center) > 0.05 ||
      !TNetwork::IsRingRegular(rings[i]))
    {
      continue;
    }
    TDRing &r = *(new TDRing(GetRender(), olxstr("DRing_") << i));
    Rings.Add(r);
    r.Basis.OrientNormal(normal);
    r.Basis.SetCenter(center);
    double min_d = 100;
    for (size_t j=0; j < rings[i].Count(); j++) {
      const double qd = rings[i][j]->crd().QDistanceTo(center);
      rings[i][j]->CAtom().SetRingAtom(true);
      if (qd < min_d)
        min_d = qd;
    }
    min_d = sqrt(min_d)*cos(M_PI/rings[i].Count())/r.GetRadius();
    r.Basis.SetZoom(min_d*0.85);
    if (glm != NULL)
      r.material = *glm;
    if (create)
      r.Create();
  }
}
//..............................................................................
void TGXApp::GrowBonds() {
  olxdict<uint32_t, smatd_list, TPrimitiveComparator> transforms;
  for (size_t i=0; i < XGrowLines.Count(); i++) {
    if (XGrowLines[i].IsVisible()) {
      transforms.Add(XGrowLines[i].CAtom().GetFragmentId()) << 
        XGrowLines[i].GetTransform();
    }
  }
  XFile().GetLattice().GrowFragments(transforms);
}
//..............................................................................
AGDrawObject* TGXApp::AddObjectToCreate(AGDrawObject* obj)  {
  TDUserObj *o = dynamic_cast<TDUserObj*>(obj);
  if (o != NULL)
    return &UserObjects.Add(o);
  return ObjectsToCreate.Add(obj);
}
//..............................................................................
void TGXApp::ClearStructureRelated() {
  ClearIndividualCollections();
  GetRender().GetStyles().RemoveNamedStyles("Q");
  XFile().GetLattice().ClearPlaneDefinitions();
  ClearGroupDefinitions();
  SelectionCopy[0].Clear();
  SelectionCopy[1].Clear();
  GetRender().SelectAll(false);
  UserObjects.Clear();
  Rings.Clear();
  GetLabels().ClearLabelMarks(lmiDefault);
}
//..............................................................................
olxstr TGXApp::Label(const TXAtomPList &atoms, const olxstr &sp) {
  olxstr_buf rv;
  if (!atoms.IsEmpty()) {
    size_t cnt = atoms.Count()-1;
    for( size_t i=0; i < cnt; i++ )
      rv << atoms[i]->GetGuiLabel() << sp;
    rv << atoms.GetLast()->GetGuiLabel();
  }
  return rv;
}
//..............................................................................
bool TGXApp::ToClipboard(const olxstr &text) const {
#ifdef __WXWIDGETS__
  if( wxTheClipboard->Open() )  {
    if (wxTheClipboard->IsSupported(wxDF_TEXT) )
      wxTheClipboard->SetData(new wxTextDataObject(text.u_str()));
    wxTheClipboard->Close();
    return true;
  }
#elif defined(__WIN32__)
  if (OpenClipboard(NULL)) {
    EmptyClipboard();
    HGLOBAL cd = GlobalAlloc(GMEM_MOVEABLE, text.RawLen()+sizeof(olxch));
    if (!cd) {
      CloseClipboard();
      return false;
    }
    LPTSTR cdt = (LPTSTR)GlobalLock(cd);
    memcpy(cdt, text.raw_str(), text.RawLen());
    cdt[text.Length()] = '\0';
    GlobalUnlock(cd);
    SetClipboardData(
#ifdef _UNICODE
      CF_UNICODETEXT,
#else
      CF_TEXT,
#endif
      cd);
    CloseClipboard();
    return true;
  }
  else {
    return false;
  }

#endif

  return false;
}
//..............................................................................
void TGXApp::ClearTextures(short) {
  TTextureManager &tm = GetRender().GetTextureManager();
  bool update = false;
  for (size_t i=0; i < TextureNames.Count(); i++) {
    TGlTexture *glt = tm.FindByName(TextureNames[i]);
    if (glt != NULL) {
      glt->SetEnabled(false);
      update = true;
    }
  }
  if (update) {
    TXAtom::CreateStaticObjects(GetRender());
    TXBond::CreateStaticObjects(GetRender());
  }
}
//..............................................................................
void TGXApp::LoadTextures(const olxstr &folder) {
  if (!TEFile::IsDir(folder)) {
    throw TInvalidArgumentException(__OlxSourceInfo, "folder name");
  }
  GetRender().GetScene().MakeCurrent();
#if !defined(__WXWIDGETS__) && !defined(__WIN32__)
  throw TNotImplementedException(__OlxSourceInfo);
#endif
  TTextureManager &tm = GetRender().GetTextureManager();
  bool update=false;
  olxstr dn = folder;
  TEFile::AddPathDelimeterI(dn);
  for (size_t i=0; i < TextureNames.Count(); i++) {
#if defined(__WXWIDGETS__)
    olxstr fn = (olxstr(dn) << TextureNames[i]  << ".png");
#else
    olxstr fn = (olxstr(dn) << TextureNames[i]  << ".bmp");
#endif
    if (!TEFile::Exists(fn)) {
      TBasicApp::NewLogEntry() << "Could not locate '" << fn <<
        "' skiping this texture";
      continue;
    }
    int sz = -1;
#if defined(__WXWIDGETS__)
    wxImage img;
    img.LoadFile(fn.u_str());
    if( img.Ok() && olx_is_pow2(img.GetWidth()) &&
      (img.GetWidth() == img.GetHeight()))
    {
      sz = img.GetWidth();
    }
#else
    HBITMAP bmp = (HBITMAP)LoadImage(NULL, fn.u_str(), IMAGE_BITMAP, 0, 0,
      LR_CREATEDIBSECTION|LR_LOADFROMFILE);
    if (bmp != NULL) {
      BITMAP bm;
      GetObjectW(bmp , sizeof(bm) , &bm);
      if (olx_is_pow2(bm.bmHeight) && (bm.bmHeight == bm.bmWidth)) {
        sz = bm.bmHeight;
      }
      else {
        ::DeleteObject(bmp);
      }
    }
#endif
    if (sz == -1) {
      TBasicApp::NewLogEntry() << "Invalid image file '" << fn <<
        "' skiping this texture";
      continue;
    }
#if defined(__WXWIDGETS__)
    unsigned char * tex_data = new unsigned char[sz*sz*3];
    for (int ix=0; ix < sz; ix++) {
      int off = ix*sz;
      for (int jx=0; jx < sz; jx++) {
        int off1 = (off+jx)*3;
        int off2 = (jx*sz+ix)*3;
        tex_data[off1+0] = img.GetRed(ix,jx);
        tex_data[off1+1] = img.GetGreen(ix,jx);
        tex_data[off1+2] = img.GetBlue(ix,jx);
      }
    }
#else
    HDC hdc = CreateCompatibleDC(NULL);
    ::SelectObject(hdc, bmp);
    BITMAPINFO bmpInfo;
    memset(&bmpInfo, 0, sizeof(bmpInfo));
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = sz;
    bmpInfo.bmiHeader.biHeight = sz;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 24;
    bmpInfo.bmiHeader.biCompression = BI_RGB;
    unsigned char * tex_data = new unsigned char[sz*sz*3];
    GetDIBits(hdc, bmp, 0, sz, tex_data, &bmpInfo, DIB_RGB_COLORS);
    ::SelectObject(hdc, NULL);
    ::DeleteObject(hdc);
    ::DeleteObject(bmp);
    for (int ix=0; ix < sz; ix++) {
      size_t off = ix*sz;
      for (int jx=ix+1; jx < sz; jx++) {
        int off1 = (off+jx)*3;
        int off2 = (jx*sz+ix)*3;
        olx_swap(tex_data[off1+0], tex_data[off2+2]);
        olx_swap(tex_data[off1+1], tex_data[off2+1]);
        olx_swap(tex_data[off1+2], tex_data[off2+0]);
      }
    }
#endif
    TGlTexture *tex = tm.FindByName(TextureNames[i]);
    if (tex == NULL) {
      GLuint tex_id = tm.Add2DTexture(
        TextureNames[i],
        0, sz, sz, 0,
        GL_RGB, tex_data);
      tex = tm.FindTexture(tex_id);
    }
    else {
      tm.Replace2DTexture(*tex,
        0, sz, sz, 0,
        GL_RGB, tex_data);
    }
    delete [] tex_data;
    tex->SetSCrdWrapping(tpCrdRepeat);
    tex->SetTCrdWrapping(tpCrdRepeat);
    tex->SetMinFilter(tpFilterNearest);
    tex->SetMagFilter(tpFilterLinear);
    tex->SetEnabled(true);
    update = true;
    TBasicApp::NewLogEntry() << TextureNames[i] << " loaded...";
  }
  if (update) {
    TXAtom::CreateStaticObjects(GetRender());
    TXBond::CreateStaticObjects(GetRender());
  }
}
//..............................................................................
