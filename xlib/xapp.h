/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_xappH
#define __olx_xl_xappH
#include "bapp.h"
#include "xfiles.h"
#include "library.h"
#include "reflection.h"
#include "ecomplex.h"
#include "undo.h"
#include "arrays.h"
#include "atomref.h"
#include "etable.h"

BeginXlibNamespace()
// program state and some other special checks for functions
const uint32_t
  psFileLoaded       = 0x00010000,
  psCheckFileTypeIns = 0x00020000,
  psCheckFileTypeCif = 0x00040000,
  psCheckFileTypeP4P = 0x00080000,
  psCheckFileTypeCRS = 0x00100000;

class TNameUndo : public TUndoData {
public:
  struct NameRef {
    size_t catom_id;
    olxstr name;
    const cm_Element* elm;
    double peak_height;
    NameRef(size_t id, const cm_Element* _elm, double _peak_height,
      const olxstr& n)
      : catom_id(id), name(n), elm(_elm), peak_height(_peak_height)
    {}
  };

  TNameUndo(IUndoAction* action) : TUndoData(action)  {}

  TTypeList<NameRef> Data;

  void AddAtom(TCAtom& A, const olxstr& oldName)  {
    Data.Add(new NameRef(A.GetId(), &A.GetType(), A.GetQPeak(), oldName));
  }
  size_t AtomCount() const {  return Data.Count();  }
  size_t GetCAtomId(size_t i) const {  return  Data[i].catom_id;  }
  const olxstr& GetLabel(size_t i) const {  return  Data[i].name;  }
  double GetPeakHeight(size_t i) const {  return Data[i].peak_height;  }
  const cm_Element& GetElement(size_t i) const {  return *Data[i].elm;  }
};

typedef TSObject<TNetwork> SObject;
typedef TPtrList<SObject> SObjectPtrList;
class TXApp : public TBasicApp, public ALibraryContainer  {
  TUndoStack UndoStack;
protected:
  TTypeList<TXFile> Files;
  TLibrary Library;
  olxstr CifTemplatesDir;  // the folder with CIF templates/data
  ASelectionOwner* SelectionOwner;
  olx_object_ptr<double> min_hbond_angle,
    exyz_separation;
  olx_object_ptr<bool>
    preserve_fvars,
    safe_afix,
    rename_parts,
    stack_restraints,
    external_explicit_same,
    explicit_same;
  size_t max_label_length;
  bool interactions_i;
  SortedObjectList<int, TPrimitiveComparator> interactions_from,
    interactions_to;
  void InitInteractions();
  virtual bool CheckProgramState(unsigned int specialCheck);
  void ProcessRingAfix(TSAtomPList& ring, int afix, bool pivot_last);
  TXApp(const olxstr &basedir, bool dummy);
  void Init(ASObjectProvider* objectProvider=NULL,
    ASelectionOwner* selOwner=NULL);
  olxstr LastSGResult;
  virtual olxstr GetPlatformString_(bool full) const;
public:
  TXApp(const olxstr &basedir, ASObjectProvider* objectProvider=NULL,
    ASelectionOwner* selOwner=NULL);
  virtual ~TXApp();
  TXFile& XFile() const { return Files[0]; }
  const TTypeList<TXFile> &XFiles() const { return Files; }

  DefPropC(olxstr, CifTemplatesDir)

  virtual class TLibrary&  GetLibrary()  {  return Library;  }

  TUndoStack &GetUndo() { return UndoStack; }

  // fills the list with the matrices of the UnitCell
  void GetSymm(smatd_list& ml) const;

  template <class FT>
    bool CheckFileType() const {
      if (!XFile().HasLastLoader()) {
        return false;
      }
      return XFile().LastLoader()->Is<FT>();
    }

  static TXApp& GetInstance() {
    TBasicApp& bai = TBasicApp::GetInstance();
    TXApp* xai = dynamic_cast<TXApp*>(&bai);
    if (xai == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "unsuitable application instance");
    }
    return *xai;
  }
  /* calculates structure factors for current structure, F.Count must be
  greater or equal to the ref.Count. This function uses direct approach and is
  to be used for testing mainly
 */
  void CalcSF(const TRefList &refs, TArrayList<TEComplex<double> > &F) const;
  void CalcSFEx(const TRefList &refs, TArrayList<TEComplex<double> > &F,
    const TSAtomPList &atoms, const smatd_list &ml, int sg_order) const;
  /* calculates Fsq and fills the reflection list. spacial handling of twinned
  data is used - the resulting Fsq are 'twinned'. It uses SFUtil functions to
  calculate Fc. if scale is true, the refs i and S are scaled using the OSF
  If extinction correction is used, it will be applied to the Fsq
  */
  RefinementModel::HklStat CalcFsq(TRefList &refs, evecd &Fsq,
    bool scale) const;
  /* function undoes renaming atoms */
  void undoName(TUndoData *data);
  void NameHydrogens(TSAtom& a, TAsymmUnit::TLabelChecker &lc, TUndoData* ud);
  // fixes hydrogen atom labels
  TUndoData* FixHL();
  /* rearranges atoms to follow a ring and returns true if a ring sequence is 
  found; modifies tags of the given atoms and their immediate environment
  */
  static bool GetRingSequence(TSAtomPList& atoms);
  static void RingContentFromStr(const olxstr& textDescr,
    ElementPList& ringDesc);
  /* Condition could be a ring template like C2N2C2, or 'sel' to apply to the
  selected ring or 'selt' - to use the selected ring as a template
  */
  void FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings);
  //
  olx_object_ptr<TSAtomPList> FindSAtomsWhere(const olxstr& Where);
  virtual TSAtomPList::const_list_type FindSAtoms(
    const IStrList& names, bool ReturnAll = true, bool ClearSelection = true);
  ConstPtrList<SObject> GetSelected(bool unselect=true) const;
  ASelectionOwner *GetSelectionOwner() const { return SelectionOwner; }
  // finds Cp, Ph, Naph and Cp* rings and adds corresponding afixes
  void AutoAfixRings(int afix, TSAtom* sa = NULL, bool TryPyridine = false);
  void SetAtomUiso(TSAtom& sa, double val);
  /* initialises the vcov matrix from shelx or smtbx, might throw an exception..
  returns the source of the matrix like: shelxl or smtbx
 */
  olxstr InitVcoV(class VcoVContainer& vcov) const;
  olxstr GetLastSGResult() const { return LastSGResult; }
  // for the internal use
  void SetLastSGResult_(const olxstr &r) { LastSGResult = r; }
  /* loads specified radii from an element-radius a line file or from structured
  xld file
  */
  void UpdateRadii(const olxstr &fn, const olxstr &rtype=EmptyString(),
    bool log=false);
  /* reads a simple 'element radius' a line text file
  */
  static ElementRadii::const_dict_type ReadRadii(const olxstr& fn);
  /* which = 0 - r_bonding, 1 - r_pers, 2 - r_cov, 3 - r_sfil, 4 - r_vdw,
  5 - r_custom
  */
  static void PrintRadii(int which, const ElementRadii& radii,
    const ContentList& au_cont);
  static void PrintVdWRadii(const ElementRadii& radii,
    const ContentList& au_cont)
  {
    PrintRadii(4, radii, au_cont);
  }
  static void PrintCustomRadii(const ElementRadii& radii,
    const ContentList& au_cont)
  {
    PrintRadii(5, radii, au_cont);
  }
  template <class AtomType>  // could be either TCAtom or TSAtom
  static double GetVdWRadius(const AtomType& a, const ElementRadii* radii)  {
    const size_t ei = (radii == NULL ? InvalidIndex
      : radii->IndexOf(&a.GetType()));
    return (ei == InvalidIndex ? a.GetType().r_vdw : radii->GetValue(ei));
  }
  /* returns size of the box using radii in A and using r_sfil - in B. If the
 radii is NULL, the values will be identical. If the radii is not null, the
 number of radii must equal the number of atoms
 */
  static WBoxInfo CalcWBox(const TSAtomPList& atoms, const TDoubleList* radii,
    double (*weight_calculator)(const TSAtom&));
  struct CalcVolumeInfo {
    double total, overlapping;
    CalcVolumeInfo(double _total, double _overlapping)
      : total(_total),
      overlapping(_overlapping)
    {}
  };
  CalcVolumeInfo CalcVolume(const ElementRadii* radii);

  template <class atom_list> static void UnifyPAtomList(atom_list& alist) {
    ACollectionItem::Unify(alist);
  }
  // the returned value (in degrees) must be cached if used in loops etc
  static double GetMinHBondAngle();
  // possibly should go to TGXApp...
  static double GetEXYZSeparation();
  // preserve free vaiable if referenced once only
  static bool DoPreserveFVARs();
  // if true - AFIX are validated adter naming, deleting and HADD
  static bool DoUseSafeAfix();
  static size_t GetMaxLabelLength();
  // if true - parts cannot have identical labels
  static bool DoRenameParts();
  // used in the analysis of what short interactions to display
  static SortedObjectList<int, TPrimitiveComparator>& GetInteractionsFrom();
  static SortedObjectList<int, TPrimitiveComparator>& GetInteractionsTo();
  static bool DoStackRestraints();
  static bool DoUseExternalExplicitSAME();
  static bool DoUseExplicitSAME();
  void ResetOptions();

  static const_strlist BangList(const TSAtom &A);
  static void BangTable(const TSAtom& A, TTTable<TStrList>& Table);
  static double Tang(TSBond *B1, TSBond *B2, TSBond *Middle,
    olxstr *Sequence=NULL);
  static const_strlist TangList(TSBond *Middle);

  // finds an atom through global Id
  TSAtom& GetSAtom(size_t ind) const;
  // finds a bond through global Id
  TSBond& GetSBond(size_t ind) const;
  // global search through loaded Files
  TSAtom& GetSAtom(const TSAtom::Ref& r) const;
  // global search through loaded Files
  TSBond& GetSBond(const TSBond::Ref& r) const;

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
};

EndXlibNamespace()
#endif
