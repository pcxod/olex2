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
      : catom_id(id), name(n), elm(_elm), peak_height(_peak_height)   {}
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
  TXFile *FXFile;
  TLibrary Library;
  olxstr CifTemplatesDir;  // the folder with CIF templates/data
  ASelectionOwner* SelectionOwner;
  bool preserve_fvars, preserve_fvars_i;
  double min_hbond_angle;
  bool min_hbond_angle_i;
  bool safe_afix, safe_afix_i;
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
  inline TXFile& XFile() const {  return *FXFile; }

  DefPropC(olxstr, CifTemplatesDir)

  virtual class TLibrary&  GetLibrary()  {  return Library;  }

  TUndoStack &GetUndo() { return UndoStack; }

  // fills the list with the matrices of the UnitCell
  void GetSymm(smatd_list& ml) const;

  template <class FT>
    bool CheckFileType() const {
      if( !FXFile->HasLastLoader() )  return false;
      return EsdlInstanceOf(*FXFile->LastLoader(), FT);
    }

  static TXApp& GetInstance()  {
    TBasicApp& bai = TBasicApp::GetInstance();
    TXApp* xai = dynamic_cast<TXApp*>(&bai);
    if( xai == NULL ) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "unsuitable application instance");
    }
    return *xai;
  }
  /* calculates structure factors for current structure, F.Count must be
  greater or equal to the ref.Count. This function uses direct approach and is
  to be used for testing mainly
 */
  void CalcSF(const TRefList& refs, TArrayList<TEComplex<double> >& F) const;
  /* calculates Fsq and fills the reflection list. spacial handling of twinned
  data is used - the resulting Fsq are 'twinned'. It uses SFUtil functions to
  calculate Fc. if scale is true, the refs i and S are scaled using the OSF
  If extinction correction is used, it will be applied to the Fsq
  */
  RefinementModel::HklStat CalcFsq(TRefList &refs, evecd &Fsq,
    bool scale) const;
  /* function undoes renaming atoms */
  void undoName(TUndoData *data);
  void NameHydrogens(TSAtom& a, TUndoData* ud);
  // fixes hydrogen atom labels
  TUndoData* FixHL();
  static void RingContentFromStr(const olxstr& textDescr,
    ElementPList& ringDesc);
  void FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings);
  //
  virtual bool FindSAtoms(const olxstr& condition, TSAtomPList& res,
    bool ReturnAll=true, bool ClearSelection=true);
  ConstPtrList<TSAtom> FindSAtoms(
    const olxstr& condition, bool ReturnAll=true, bool ClearSelection=true)
  {
    TSAtomPList atoms;
    FindSAtoms(condition, atoms, ReturnAll, ClearSelection);
    return atoms;
  }
  ConstPtrList<TSAtom> FindSAtoms(
    const TStrObjList& names, bool ReturnAll=true, bool ClearSelection=true)
  {
    return FindSAtoms(names.Text(' '), ReturnAll, ClearSelection);
  }
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
  struct CalcVolumeInfo  {
    double total, overlapping;
    CalcVolumeInfo(double _total, double _overlapping) : total(_total),
      overlapping(_overlapping)  {}
  };
  CalcVolumeInfo CalcVolume(const ElementRadii* radii);

  template <class atom_list> static void UnifyPAtomList(atom_list& alist) {
    const size_t ac = alist.Count();
    for( size_t i=0; i < ac; i++ )
      alist[i]->SetTag(i);
    for( size_t i=0; i < ac; i++ )
      if( alist[i]->GetTag() != i || alist[i]->IsDeleted() )
        alist[i] = NULL;
    alist.Pack();
  }
  // the returned value (in degrees) must be cached if used in loops etc
  static double GetMinHBondAngle();
  // preserve free vaiable if referenced once only
  static bool DoPreserveFVARs();
  // if true - AFIX are validated adter naming, deleting and HADD
  static bool DoUseSafeAfix();
  // used in the analysis of what short interactions to display
  static SortedObjectList<int, TPrimitiveComparator>& GetInteractionsFrom();
  static SortedObjectList<int, TPrimitiveComparator>& GetInteractionsTo();

  static const_strlist BangList(const TSAtom &A);
  static void BangTable(const TSAtom& A, TTTable<TStrList>& Table);
  static double Tang(TSBond *B1, TSBond *B2, TSBond *Middle,
    olxstr *Sequence=NULL);
  static const_strlist TangList(TSBond *Middle);

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
};

EndXlibNamespace()
#endif
