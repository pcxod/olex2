/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_gxapp_H
#define __olx_gxl_gxapp_H
#include "gxbase.h"
#include "xapp.h"
#include "etable.h"
#include "symmlib.h"
#include "bitarray.h"
#include "xfader.h"
#include "glmouse.h"
#include "dframe.h"
#include "3dframe.h"
#include "glfont.h"
#include "styles.h"
#include "glbitmap.h"
#include "typelist.h"
#include "hkl.h"
#include "fracmask.h"
#include "glrender.h"
#include "xatom.h"
#include "xbond.h"
#include "xplane.h"
#include "gxfiles.h"
#include "label_utils.h"
#ifdef __WXWIDGETS__
#include "wx/zipstrm.h"
#endif
#if defined __APPLE__ && defined __MACH__
  #define OLX_RESOURCES_FOLDER "olex2.app/Contents/Resources/"
#else
  #define OLX_RESOURCES_FOLDER ""
#endif

BeginGxlNamespace()

const short
  simNone = 0x0000, // initialisation method
  simHDC  = 0x0001,
  simBMP  = 0x0002;

const short
  sdsNone   = 0, // drawing style
  sdsBS     = 1, // bals and sticks
  sdsSP     = 2, // space filling
  sdsES     = 3,  // ellipsoide and sticks
  sdsESR    = 4,  // ellipsoide and rims and sticks
  sdsWF     = 5,  // wire frame
  sdsSS     = 6,  // stippled cones + spheres
  sdsST     = 7;  // sticks

// grow mode
const short
  gmCovalent      = 0x0001,
  gmSInteractions = 0x0002,
  gmSameAtoms     = 0x0004,
  gmVanDerWaals   = 0x0008;
//---------------------------------------------------------------------------
class TDUnitCell;
class TDBasis;
class TGraphicsObjects;
class TXGlLabels;
class TXLine;
class TXAngle;
class TDRing;
class TXGrowLine;
class TXGrowPoint;
class TXGlLabel;
class TXReflection;
class TXGrid;
class TXLattice;
class TDUserObj;
class TDSphere;
class TAtomLegend;

class TGXApp : public TXApp, AEventsDispatcher, public ASelectionOwner {
  TTypeListExt<TXGrowPoint, AGDrawObject> XGrowPoints;
  TTypeListExt<TXGrowLine, AGDrawObject> XGrowLines;
  TTypeListExt<TDUserObj, AGDrawObject> UserObjects;
  TStrList AtomsToGrow;
  smatd_list UsedTransforms;
  TTypeListExt<TXReflection, AGDrawObject> XReflections;
  TPtrList<TGlBitmap> GlBitmaps;
  TTypeListExt<TXGlLabel, IOlxObject> XLabels;
  TTypeListExt<TDRing, AGDrawObject> Rings;
  TXGlLabels *FLabels;
  TTypeListExt<TXLine, AGDrawObject> Lines;
  TTypeListExt<TXAngle, AGDrawObject> Angles;
  // have to manage memory ourselves - base class is used
  AGDObjList LooseObjects;
  AGDObjList ObjectsToCreate;

  void ClearXObjects();

  void CreateXRefs();
  void CreateXGrowLines();
  void _CreateXGrowVLines();
  void CreateXGrowPoints();
  double DeltaV;
  // drawing data and functions
  double FPictureResolution;
  olxstr_set<> IndividualCollections;
  /* makes sure that only bonds (and grow mode lines) with both atoms visible
  are visible, also considers H, and Q bonds special handling
  */
  void _syncBondsVisibility();
  class TStateRegistry *States;
  TStrList TextureNames;
public:
  template <class obj_t, class act_t> struct TIterator {
    size_t offset, count;
    TTypeList<ObjectCaster<obj_t, act_t> > objects;
    TIterator()
      : offset(0), count(0)
    {}
    bool HasNext() const { return (offset < count); }
    act_t& Next() {
      size_t off = offset;
      for (size_t i = 0; i < objects.Count(); i++) {
        if (off >= objects[i].Count()) {
          off -= objects[i].Count();
        }
        else {
          offset++;
          return objects[i][off];
        }
      }
      throw TFunctionFailedException(__OlxSourceInfo, "end is reached");
    }
    void Reset() { offset = 0; }
    template <class Functor>
    const TIterator& ForEach(const Functor& f) const {
      for (size_t i = 0; i < objects.Count(); i++) {
        objects[i].ForEach(f);
      }
      return *this;
    }
  };
  struct AtomIterator : public TIterator<TSAtom, TXAtom> {
    AtomIterator(const TGXApp& app) {
      for (size_t i = 0; i < app.XFiles().Count(); i++) {
        objects.AddCopy(app.XFiles()[i].GetLattice().GetObjects()
          .atoms.GetAccessor<TXAtom>());
        count += objects.GetLast().Count();
      }
    }
  };
  struct BondIterator : public TIterator<TSBond, TXBond> {
    BondIterator(const TGXApp& app) {
      for (size_t i = 0; i < app.XFiles().Count(); i++) {
        objects.AddCopy(app.XFiles()[i].GetLattice().GetObjects()
          .bonds.GetAccessor<TXBond>());
        count += objects.GetLast().Count();
      }
    }
  };
  struct PlaneIterator : public TIterator<TSPlane, TXPlane> {
    PlaneIterator(const TGXApp& app) {
      for (size_t i = 0; i < app.XFiles().Count(); i++) {
        objects.AddCopy(app.XFiles()[i].GetLattice().GetObjects()
          .planes.GetAccessor<TXPlane>());
        count += objects.GetLast().Count();
      }
    }
  };
  AtomIterator GetAtoms() const { return AtomIterator(*this); }
  BondIterator GetBonds() const { return BondIterator(*this); }
  PlaneIterator GetPlanes() const { return PlaneIterator(*this); }
protected:
  TGlRenderer* GlRenderer;
  TXFader* Fader;
  THklFile* FHklFile;
  TGlMouse* FGlMouse;
  TDBasis* FDBasis;
  TDFrame* FDFrame;
  T3DFrameCtrl* F3DFrame;
  TXGrid* FXGrid;
  TDSphere *FDSphere;
  TAtomLegend *FAtomLegend;

  virtual bool HasGUI_() const { return true; }
  void FragmentVisible(TNetwork *N, bool V);
  bool Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
    const IOlxObject *Data, TActionQueue *);
  void GetGPCollections(AGDObjList& GDObjects, TPtrList<TGPCollection>& Result);
  struct GroupData {
    TTypeList<TSAtom::Ref> atoms;
    TTypeList<TSBond::Ref> bonds;
    olxstr collectionName;
    bool visible;
    index_t parent_id;
    bool IsEmpty() const { return atoms.IsEmpty() && bonds.IsEmpty(); }
    void Clear() {
      atoms.Clear();
      bonds.Clear();
    }
    GroupData& operator = (const GroupData& g) {
      atoms = g.atoms;
      bonds = g.bonds;
      collectionName = g.collectionName;
      visible = g.visible;
      parent_id = g.parent_id;
      return *this;
    }
  };
  struct {
    TTypeList<TSAtom::Ref> atoms;
    TTypeList<TSBond::Ref> bonds;
    TTypeList<olxstr> labels;
    TTypeList<vec3d> centers;
    void Clear() {
      atoms.Clear();
      bonds.Clear();
      labels.Clear();
      centers.Clear();
    }
  } LabelInfo;
  void StoreLabels();
  void RestoreLabels();
  TTypeList<GroupData> GroupDefs;
  GroupData SelectionCopy[2];
  olxdict<TGlGroup*, size_t, TPointerComparator> GroupDict;
  // stores numeric references
  void RestoreGroup(TGlGroup& glg, const GroupData& group);
  void RestoreGroups();
  void StoreGroup(const TGlGroup& glg, GroupData& group);
  void _UpdateGroupIds();
  static size_t CalcMaxAtomTag(const TLattice& latt) {
    size_t ac = 0;
    for (size_t i = 0; i < latt.GetObjects().atoms.Count(); i++) {
      if (!latt.GetObjects().atoms[i].IsDeleted()) {
        ac++;
      }
    }
    return ac;
  }
  static size_t CalcMaxBondTag(const TLattice& latt) {
    size_t bc = 0;
    for (size_t i = 0; i < latt.GetObjects().bonds.Count(); i++)
      if (!latt.GetObjects().bonds[i].IsDeleted())
        bc++;
    return bc;
  }
  size_t GetAtomTag(TSAtom& sa, TSizeList& latt_sz) const {
    size_t off = 0;
    for (size_t i = 0; i < Files.Count(); i++) {
      if (sa.GetNetwork().GetLattice() == Files[i].GetLattice()) {
        return off + sa.GetTag();
      }
      off += latt_sz[i];
    }
    return InvalidIndex;
  }
  size_t GetBondTag(TSBond& sb, TSizeList& latt_sz) const {
    size_t off = 0;
    for (size_t i = 0; i < Files.Count(); i++) {
      if (sb.GetNetwork().GetLattice() == Files[i].GetLattice()) {
        return off + sb.GetTag();
      }
      off += latt_sz[i];
    }
    return InvalidIndex;
  }
  TXAtom& GetXAtom(size_t ind) {
    return static_cast<TXAtom&>(GetSAtom(ind));
  }
  TXBond& GetXBond(size_t ind) {
    return static_cast<TXBond&>(GetSBond(ind));
  }
  sorted::ObjectPrimitive<index_t>::cons_list_type GetVisibleCAtomTags();
  virtual olxstr GetPlatformString_(bool full) const;
public:
  /* considers overlayed files
  */
  void UpdateConnectivity();
  size_t stateStructureVisible,
    stateHydrogensVisible,
    stateHydrogenBondsVisible,
    stateQPeaksVisible,
    stateQPeakBondsVisible,
    stateCellVisible,
    stateBasisVisible,
    stateGradientOn,
    stateLabelsVisible,
    stateXGridVisible,
    stateWBoxVisible;
  TStateRegistry & GetStatesRegistry() const { return *States; }
  // only works with wxWidgets and windows
  virtual bool ToClipboard(const olxstr &text) const;
  /* funny enough, if this is not overriden - all TStrLists are being converted
  to string, thus making the compilation impossible...
  */
  bool ToClipboard(const TStrList &l) const {
    return TBasicApp::ToClipboard(l);
  }
protected:
  double FProbFactor;
  // the default is 1, Calculated Zoom is multiplid by this number
  double ExtraZoom;
  /* intialises SAtom::Tag to XAtom::Id and checks if any atom with
  AtomInfo == atom_type has visible neighbours, if not - it will be hidden,
  otherwise its visibility will become 'show'; for bonds makes them visible
  only if both atoms are visible
  */
  void SyncAtomAndBondVisiblity(short atom_type, bool show_a, bool show_b);
  void _maskInvisible();
  bool MainFormVisible;
  struct DUnitCellPtr : public olx_virtual_ptr<TDUnitCell> {
    virtual IOlxObject *get_ptr() const;
  };
public:
  // FileName - argv[0];
  TGXApp(const olxstr& FileName, AGlScene* scene = 0);
  virtual ~TGXApp();
  void CreateObjects(bool CenterModel, bool init_visibility = true);
  void UpdateBonds();
  AGDrawObject* AddObjectToCreate(AGDrawObject* obj);
  bool RemoveObjectToCreate(const AGDrawObject* obj) {
    return ObjectsToCreate.Remove(obj);
  }
  void Clear();
  void ClearXGrowPoints();
  // changes the graphics quality
  int32_t Quality(int v);
  void Init();
  //..............................................................................
  void ClearIndividualCollections();
  void ClearGroupDefinitions();
  void ClearStructureRelated();
  //..............................................................................
  // GlRender interface
  void ClearColor(int Color) { GlRenderer->LightModel.SetClearColor(Color); }
  uint32_t ClearColor() const {
    return GlRenderer->LightModel.GetClearColor().GetRGB();
  }
  TGlRenderer& GetRenderer() const { return *GlRenderer; }
  TXFader& GetFader() { return *Fader; }
  void InitFadeMode();

  // implementation of BasicApp function - renders the scene
  virtual void Update();
  DefPropBIsSet(MainFormVisible)
    // renders the scene and returns used time in ms
    uint64_t Draw();
  /* prepares drawing on a bitmap, recreates graphics with different
  parameters, scales fonts
  */
  void BeginDrawBitmap(double res);
  // restores the on-screen rendering
  void FinishDrawBitmap();
  void Resize(int new_w, int new_h) { GlRenderer->Resize(new_w, new_h); }
  AGDrawObject* SelectObject(int x, int y, bool picking) const {
    return GlRenderer->SelectObject(x, y, picking);
  }
  TGlPrimitive *SelectPrimitive(int x, int y) const {
    return GlRenderer->SelectPrimitive(x, y);
  }
  DefPropP(double, ExtraZoom);
  //..............................................................................
  // TXApp interface
  TDBasis& DBasis() const { return *FDBasis; }
  THklFile& HklFile() { return *FHklFile; }
  TDFrame& DFrame() const { return *FDFrame; }
  TXGrid& XGrid() const { return *FXGrid; }
  TDSphere& DSphere() const { return *FDSphere; }
  TAtomLegend &AtomLegend() const { return *FAtomLegend; }
  T3DFrameCtrl& Get3DFrame() const { return *F3DFrame; }
  TGlMouse& GetMouseHandler() const { return *FGlMouse; }
  TGXFile &XFile() const { return (TGXFile &)Files[0]; }
  TGXFile &XFile(size_t i) const { return (TGXFile &)Files[i]; }

  // this function to be used to get all networks, including th overlayed files
  size_t GetNetworks(TNetPList& nets);
  // sets current active XFile...
  void SetActiveXFile(size_t i);
  TGXFile& NewXFile();
  // aligns overlayed structures on a 2D grid
  void AlignXFiles();
  // calculates maximum radius and center of given lattice
  void CalcLatticeRandCenter(const TLattice& latt, double& r, vec3d& cnt);
  void DeleteXFile(size_t index);
  void DeleteXFiles();

  void Select(const vec3d& From, const vec3d& To);
  void SelectAll(bool Select);
  void InvertSelection() { GetRenderer().InvertSelection();  Draw(); }
  TGlGroup* FindObjectGroup(AGDrawObject& G) {
    return GetRenderer().FindObjectGroup(G);
  }
  TGlGroup* FindGroup(const olxstr& colName) {
    return GetRenderer().FindGroupByName(colName);
  }
  TGlGroup& GetSelection() const { return GetRenderer().GetSelection(); }
  /* returns the newly created group or NULL if the grouping has failed */
  TGlGroup *GroupSelection(const olxstr& name);
  void UngroupSelection();
  void Ungroup(TGlGroup& G);
  void UngroupAll();
  // if list is true - the selection is considered as a list of bonds
  olxstr GetSelectionInfo(bool list = false) const;
  olxstr GetObjectInfoAt(int x, int y) const;
  // ASelection Owner interface
  virtual void ExpandSelection(TCAtomGroup& atoms);
  virtual void ExpandSelectionEx(TSAtomPList& atoms);
  virtual ConstPtrList<TSObject<TNetwork> > GetSelected();

  TGlBitmap* CreateGlBitmap(const olxstr& name, int left, int top,
    int width, int height, unsigned char* RGBa, unsigned int format);

  TGlBitmap* FindGlBitmap(const olxstr& name);
  void DeleteGlBitmap(const olxstr& name);
  size_t GlBitmapCount() const { return GlBitmaps.Count(); }
  TGlBitmap& GlBitmap(size_t i) { return *GlBitmaps[i]; }

  bool ShowGrid(bool v, const olxstr& FN = EmptyString());
  bool IsGridVisible() const;
  void SetGridDepth(const vec3d& crd);

protected:
  bool FQPeaksVisible,
    FHydrogensVisible,
    FHBondsVisible,
    FQPeakBondsVisible,
    FStructureVisible,
    FHklVisible,
    FXGrowLinesVisible,
    XGrowPointsVisible,
    FXPolyVisible,
    DisplayFrozen,
    ZoomAfterModelBuilt;
  short FGrowMode, PackMode;
public:
  TXGlLabels& GetLabels() const { return *FLabels; }
  void UpdateDuplicateLabels();
  bool AreLabelsVisible() const;
  void SetLabelsVisible(bool v);
  void SetLabelsMode(uint32_t lmode);
  short GetLabelsMode() const;
  void MarkLabel(const TXAtom& A, bool mark);
  void MarkLabel(size_t index, bool mark);
  bool IsLabelMarked(const TXAtom& A) const;
  bool IsLabelMarked(size_t index) const;
  void ClearLabelMarks();

  size_t GetNextAvailableLabel(const olxstr& AtomType);

  // moving atom from/to collection
  void Individualise(TXAtom& XA, short level = -1, int32_t mask = -1);
  void Collectivise(TXAtom& XA, short level = -1, int32_t mask = -1);
  void Individualise(const TXAtomPList& atoms, short level = -1, int32_t mask = -1);
  void Collectivise(const TXAtomPList& atoms, short level = -1, int32_t mask = -1);
  void Individualise(TXBond& XB, short level = -1, int32_t mask = -1);
  void Collectivise(TXBond& XB, short level = -1, int32_t mask = -1);
  void Individualise(const TXBondPList& bonds, short level = -1, int32_t mask = -1);
  void Collectivise(const TXBondPList& bonds, short level = -1, int32_t mask = -1);
  // should not be used externaly
  void ClearLabels();

  DefPropP(double, DeltaV)
    DefPropBIsSet(ZoomAfterModelBuilt)
    //
  //..............................................................................
  // XFile interface
    void RegisterXFileFormat(TBasicCFile *Parser, const olxstr& ext)
  {
    XFile().RegisterFileFormat(Parser, ext);
  }
  void LoadXFile(const olxstr& fn);
  void SaveXFile(const olxstr &fn, int flags = 0) {
    XFile().SaveToFile(fn, flags);
  }
  void Uniq() { XFile().GetLattice().Uniq(); }
  void GrowFragments(bool Shell, TCAtomPList* Template = NULL) {
    XFile().GetLattice().GrowFragments(Shell, Template);
  }
  void GrowAtoms(const IStrList& Atoms, bool Shell, TCAtomPList* Template = NULL);
  void GrowAtom(TXAtom *XA, bool Shell, TCAtomPList* Template = NULL);
  void Grow(const TXGrowPoint& growPoint);
  void ChangeAtomType(TXAtom *A, const olxstr& Element);
  void GrowWhole(TCAtomPList* Template = NULL) {
    XFile().GetLattice().GenerateWholeContent(Template);
  }
  void Grow(const TXAtomPList& atoms, const smatd_list& matrices);
  void GrowBonds();
  void MoveFragment(TXAtom* to, TXAtom* fragAtom, bool copy);
  void MoveFragment(const vec3d& to, TXAtom* fragAtom, bool copy);
  void MoveToCenter();
  void Compaq(bool AtomicLevel);
  void SetHydrogensVisible(bool v);
  bool AreHydrogensVisible() const { return FHydrogensVisible; }
  void SetHBondsVisible(bool v, bool update_groups = true);
  bool AreHBondsVisible() const { return FHBondsVisible; }
  void SetQPeaksVisible(bool v);
  bool AreQPeaksVisible() const { return FQPeaksVisible; }
  void SetQPeakBondsVisible(bool v, bool update_groups = true);
  bool AreQPeakBondsVisible() const { return FQPeakBondsVisible; }
  bool IsDisplayFrozen() const { return DisplayFrozen; }
  void SetDisplayFrozen(bool v) {
    DisplayFrozen = v;
    if (!v) {
      Draw();
    }
  }

  // hides all bonds for all hidden q-peaks
  void SetStructureVisible(bool v);
  void SetHklVisible(bool v);
  bool IsHklVisible() const { return FHklVisible; }
  bool IsStructureVisible() const { return FStructureVisible; }
  void ShowPart(const TIntList& parts, bool show, bool visible_only);
  void ShowResi(const TIntList& numbers, const TStrList &names,
    bool show, bool visible_only);

  void SetXGrowLinesVisible(bool v);
  bool GetXGrowLinesVisible() const { return FXGrowLinesVisible; }
  void LabelGrowBonds();
  const TTypeListExt<TXGrowLine, AGDrawObject>& GetGrowLines() const {
    return XGrowLines;
  }
  short GetGrowMode() const { return FGrowMode; }
  void SetGrowMode(short v, const IStrList& atoms);
  //
  void SetXGrowPointsVisible(bool v);
  bool GetXGrowPointsVisible() const { return XGrowPointsVisible; }
  short GetPackMode()  const { return PackMode; }
  void SetPackMode(short v, const olxstr& atoms);
  //
protected:
  ConstPtrList<TXAtom> XAtomsByMask(const olxstr& Name, int Mask,
    const AtomLabelInfo &label_info);
  ConstPtrList<TCAtom> CAtomsByMask(const olxstr& Name, int Mask,
    const AtomLabelInfo &label_info);
  ConstPtrList<TXAtom> XAtomsByType(const cm_Element& AI,
    const AtomLabelInfo &label_info, bool FindHidden = false);
  ConstPtrList<TCAtom> CAtomsByType(const cm_Element& AI,
    const AtomLabelInfo &label_info);
  ConstPtrList<TXAtom> GetSelectedXAtoms(bool Clear = true);
  ConstPtrList<TCAtom> GetSelectedCAtoms(bool Clear = true);
public:
  TXAtom* GetXAtom(const olxstr& AtomName, bool Clear);
  ConstPtrList<TXAtom> GetXAtoms(const olxstr& AtomName);
  ConstPtrList<TXBond> GetXBonds(const olxstr& BondName);
  // these two do a command line parsing "sel C1 $N C?? C4 to end"
  ConstPtrList<TCAtom> FindCAtoms(const IStrList& Atoms, bool ClearSelection = true);
  ConstPtrList<TXAtom> FindXAtoms(const IStrList& Atoms, bool getAll = true,
    bool ClearSelection = true, bool FindHidden = false);
  // this function will return atoms WITHOUT atoms of the overlayed files!
  virtual TSAtomPList::const_list_type FindSAtoms(
    const IStrList& names, bool ReturnAll = true, bool ClearSelection = true);
protected:
  /* the function simply checks if there are any invisible bonds connectd to the
   atom. Normally this happens when a Q-peak is renamed
  */
  void CheckQBonds(TXAtom& Atom);
  void BackupSelection();
  // helper functions
  void FillXAtomList(TXAtomPList& res, TXAtomPList* providedAtoms);
  void FillXBondList(TXBondPList& res, TXBondPList* providedBonds);
public:
  void RestoreSelection();
  void CopySelection() const;
  void PasteSelection();
  TUndoData* Name(const olxstr& From, const olxstr& To,
    bool ClearSelection, bool NameResi = false, bool DoNotSteal=false);
  TUndoData* Name(TXAtom& Atom, const olxstr& Name);
  TUndoData* ChangeSuffix(const TXAtomPList& xatoms, const olxstr& To);
  // makes sure that residues have the same labels as the reference atoms
  TUndoData* SynchroniseResidues(const TCAtomPList &reference);


  void InfoList(const IStrList& Atoms, TStrList& Info, bool Sort,
    int precision = 3, bool cart = false);

  void UpdateAtomPrimitives(int Mask, TXAtomPList* Atoms = NULL);
  void UpdateBondPrimitives(int Mask, TXBondPList* Bonds = NULL, bool HBondsOnly = false);

  void SetAtomDrawingStyle(short ADS, TXAtomPList* Atoms = NULL);

  ConstPtrList<TXBond> GetBonds(const TStrList& Bonds, bool inc_lines);

  void AtomRad(const olxstr& Rad, TXAtomPList* Atoms = NULL); // pers, sfil
  void AtomZoom(float Zoom, TXAtomPList* Atoms = NULL);  // takes %

  void BondRad(float R, TXBondPList* Bonds = NULL);
public:
  static double ProbFactor(double Prob);
  void CalcProbFactor(double Prob);

  TXPlane *AddPlane(const olxstr &name, const TXAtomPList& Atoms,
    size_t sides, double weightExtent = 0);
  TSPlane *TmpPlane(const TXAtomPList* Atoms = NULL, double weightExtent = 0);
  void DeletePlane(TXPlane &plane);
  void ClearPlanes();
  TXPlane *FindPlane(const olxstr& PlaneName);

  TXLine *AddLine(const olxstr& Name, const vec3d& base, const vec3d& edge);
  TXLine* AddLine(const olxstr& Name, const TSAtom& base, const TSAtom& edge);
  TXAngle* AddAngle(const olxstr& Name, const vec3d& center,
    const vec3d& from, const vec3d& to);
  TXAngle* AddAngle(const olxstr& Name, const TSAtom& center,
    const TSAtom& from, const TSAtom& to);
  void ClearLines() { Lines.Clear(); }
  void ClearAngles() { Angles.Clear(); }
  size_t LineCount() const { return Lines.Count(); }
  TXLine &GetLine(size_t i) const { return Lines[i]; }
  TXGlLabel *AddLabel(const olxstr& Name, const vec3d& center, const olxstr& T);
  AGDrawObject* FindLooseObject(const olxstr& Name);
  TDUserObj *FindUserObject(const olxstr& Name);

  TXLattice& AddLattice(const olxstr& Name, const mat3d& basis);
  // will return generated symmetry equivalents too
  ConstPtrList<TXAtom> AddCentroid(const TXAtomPList& Atoms);
  TXAtom &AddAtom(TXAtom* templ = NULL);
  // adopts atoms of the auinit and returns newly created atoms and bonds
  void AdoptAtoms(const TAsymmUnit& au, TXAtomPList& atoms, TXBondPList& bonds);
  void SelectAtoms(const IStrList& Names, glSelectionFlag flag);
  olx_object_ptr<TXAtomPList> FindXAtomsWhere(const olxstr& Where);
  olx_object_ptr< TXBondPList> FindXBondsWhere(const olxstr& Where);
  /* allows selcting rings: Condition describes the rings to select:
    C5N - content and 1-4, substitutions..
    SelectRing( "C6 1-4") selects all 1,4 substituted benzene rings
  */
  void SelectRings(const olxstr& Condition, glSelectionFlag flag);
  TTypeList<TSAtomPList>& FindRings(const olxstr& Condition,
    TTypeList<TSAtomPList>& rings);
  ConstTypeList<TSAtomPList> FindRings(const olxstr& Condition) {
    TTypeList<TSAtomPList> l;
    return FindRings(Condition, l);
  }

  // these two create structure scope labels
  TXGlLabel& CreateLabel(const vec3d& center, const olxstr& T, uint16_t FontIndex);
  TXGlLabel& CreateLabel(const TXAtom& A, uint16_t FontIndex);
  /* creates aromatic rings, if force is false - only creates if aromatic_rings
  option is set to true. The function also marks ring atoms (SetRingAtom(true)
  */
  void CreateRings(bool force = false, bool create = false);
  const TTypeListExt<TDRing, AGDrawObject> &GetRings() const {
    return Rings;
  }
  // recreated all labels (if any) in case if font size etc changed
  size_t LabelCount() const { return XLabels.Count(); }
  TXGlLabel& GetLabel(size_t i) { return XLabels[i]; }
  const TXGlLabel& GetLabel(size_t i) const { return XLabels[i]; }
  void UpdateLabels();
  static olxstr Label(const TXAtomPList &atoms, const olxstr &sep = ' ');
  //..............................................................................
  void SetQPeakScale(double V);
  double GetQPeakScale();
  void SetQPeakSizeScale(double V);
  double GetQPeakSizeScale();
  //..............................................................................
  // GlMouse interface
  bool MouseDown(int x, int y, short Shift, short Button) {
    return FGlMouse->MouseDown(x, y, Shift, Button);
  }
  bool MouseUp(int x, int y, short Shift, short Button) {
    return FGlMouse->MouseUp(x, y, Shift, Button);
  }
  bool MouseMove(int x, int y, short Shift) {
    return FGlMouse->MouseMove(x, y, Shift);
  }
  bool DblClick() { return FGlMouse->DblClick(); }
  void ResetMouseState(short x, short y, short shift = 0, short button = 0,
    bool keep_object = false)
  {
    FGlMouse->ResetMouseState(x, y, shift, button, keep_object);
  }
  AGDrawObject* GetMouseObject() const {
    return FGlMouse->GetMouseData().GetObject();
  }
  void EnableSelection(bool v) { FGlMouse->SetSelectionEnabled(v); }
  //..............................................................................
  // actions
  TActionQueue &OnGraphicsVisible,
    &OnFragmentVisible,
    &OnAllVisible,
    &OnObjectsDestroy,
    &OnObjectsCreate;
  bool IsCellVisible()  const;
  void SetCellVisible(bool v);
  bool IsBasisVisible() const;
  void SetBasisVisible(bool v);
  bool IsGraphicsVisible(AGDrawObject *G) const { return G->IsVisible(); }
  TUndoData* SetGraphicsVisible(AGDrawObject *G, bool v);
  TUndoData* SetGraphicsVisible(AGDObjList& G, bool v);

  void FragmentsVisible(const TNetPList& Networks, bool V);
  TNetPList InvertFragmentsList(const TNetPList& Fragments);
  void SelectFragmentsAtoms(const TNetPList& frags, bool v);
  void SelectFragmentsBonds(const TNetPList& frags, bool v);
  void SelectFragments(const TNetPList& frags, bool v);
  TGlGroup* GroupFragments(const TNetPList& Fragments, const olxstr groupName);
  void LoadTextures(const olxstr &folder);
  void ClearTextures(short flags);
  // inverts current list of TLattice using Selected Fragments, returns
  // the number of entries added to the result
  void AllVisible(bool V);
  void CenterModel();

  void CenterView(bool calcZoom = false);
  /* creates a mask of visible scene, Inc is the extra added to atomic Van-der
  -Waals radii
  */
  void BuildSceneMask(FractMask& mask, double Inc);
  static vec3d GetConstrainedDirection(const vec3d &t);
  //..............................................................................
  // X interface
  TUndoData* DeleteXAtoms(TXAtomPList& L);
  TUndoData* DeleteXObjects(const AGDObjList& L);
  /* function undoes renaming atoms */
  void undoName(TUndoData *data);
  /* function undoes hiding of objects */
  void undoHide(TUndoData *data);

  void SynchroniseBonds(const TXAtomPList& XAtoms);
  /* Saves only structure specific style as well as the scene properties.
  Three items will be added to the 'item' - Style, Scene and ICollections. The
  latter is important for restoring properties of individual atoms/bonds etc
  */
  void SaveStructureStyle(TDataItem& item) const;
  void LoadStructureStyle(const TDataItem &item);
  void ToDataItem(TDataItem& item, IOutputStream& zos) const;
  void FromDataItem(TDataItem& item, IInputStream& zis);

  void SaveModel(const olxstr& file_name) const;
  void LoadModel(const olxstr& file_name);

  const_strlist ToPov() const;
  const_strlist ToWrl() const;
  //..............................................................................
  static TGXApp& GetInstance() {
    TBasicApp& bai = TBasicApp::GetInstance();
    TGXApp* gxai = dynamic_cast<TGXApp*>(&bai);
    if (gxai == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "unsuitable application instance");
    }
    return *gxai;
  }
};
////////////////////////////////////////////////////////////////////////////////

EndGxlNamespace()
#endif
