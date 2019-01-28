/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_asymmunit_H
#define __olx_xl_asymmunit_H
#include "xbase.h"
#include "ematrix.h"
#include "threex3.h"
#include "macroerror.h"
#include "ellipsoid.h"
#include "dataitem.h"
#include "conninfo.h"
#include "eset.h"

#undef GetObject

BeginXlibNamespace()

class TResidue;

class TAsymmUnit: public IXVarReferencerContainer  {
  TCAtomPList CAtoms;
  // list of unique SG matrices (no centering or inversion)
  smatd_list  Matrices;
  TEllpPList Ellipsoids;
  mat3d
    // transformation from cell crd to cartesian
    Cell2Cartesian, Cell2CartesianT,
    // transformation from cartesian crd to cell
    Cartesian2Cell, Cartesian2CellT;
  // ADP transformation matrices
  mat3d UcifToUxyz,  UxyzToUcif,
        UcifToUxyzT, UxyzToUcifT;
  // transformation from Miller index to cartesian
  mat3d Hkl2Cartesian;
  TCAtomPList Centroids;
  double MaxQPeak,
         MinQPeak;
  double Z;
  short Latt;
  /* this flag specifies that _OnAtomTypeChange will do nothing, however
  whatever called Assign must call _UpdateConnInfo
  */
  bool Assigning;
  class TLattice* Lattice;  // parent lattice
  vec3d Axes, AxisEsds;     // axes and esds
  vec3d Angles, AngleEsds;  // angles and esds
  vec3d RAxes;              // reciprical axes
  vec3d RAngles;            // reciprical angles
protected:
  TActionQList Actions;
  TTypeListExt<TResidue, IOlxObject> Residues;
  TResidue& MainResidue;
  olx_pdict<olxch, olx_pdict<int, TResidue*> > ResidueRegistry;
  class RefinementModel* RefMod;
  static const olxstr IdName;
  void _UpdateQPeaks();
public:

  TAsymmUnit(TLattice* L);
  virtual ~TAsymmUnit();

  TLattice& GetLattice() const {  return *Lattice;  }
  bool HasLattice() const { return Lattice != 0; }
  vec3d& GetAxes()  {  return Axes;  }
  vec3d& GetAxisEsds()  {  return AxisEsds;  }
  vec3d& GetAngles()  {  return Angles;  }
  vec3d& GetAngleEsds()  {  return AngleEsds;  }
  const vec3d& GetAxes() const {  return Axes;  }
  const vec3d& GetAxisEsds() const {  return AxisEsds;  }
  const vec3d& GetAngles() const {  return Angles;  }
  const vec3d& GetAngleEsds() const {  return AngleEsds;  }
  const vec3d& GetRAxes() const {  return RAxes;  }
  const vec3d& GetRAngles() const {  return RAngles;  }
  double CalcCellVolume() const;
  /* estimates Z=Z'*sg.multiplicity according to 18.6A rule, partial occupancy
  implied double...
  */
  double EstimateZ(double atomCount) const;
  double GetZPrime() const;
  DefPropP(double, Z)
  DefPropP(short, Latt)

  const mat3d& GetCellToCartesian() const {  return Cell2Cartesian; }
  const mat3d& GetCartesianToCell() const {  return Cartesian2Cell; }
  const mat3d& GetHklToCartesian() const {  return Hkl2Cartesian; }
  template <typename FT> TVector3<FT>& CellToCartesian(TVector3<FT>& crt) const
  {
    crt[0] = (FT)(crt[0]*Cell2Cartesian[0][0] + crt[1]*Cell2Cartesian[1][0] +
      crt[2]*Cell2Cartesian[2][0]);
    crt[1] = (FT)(crt[1]*Cell2Cartesian[1][1] + crt[2]*Cell2Cartesian[2][1]);
    crt[2] = (FT)(crt[2]*Cell2Cartesian[2][2]);
    return crt;
    //crt *= Cell2Cartesian;
  }
  template <typename FT>
  TVector3<FT> Orthogonalise(const TVector3<FT>& crt) const {
    return TVector3<FT>(
      (FT)(crt[0]*Cell2Cartesian[0][0] + crt[1]*Cell2Cartesian[1][0] +
        crt[2]*Cell2Cartesian[2][0]),
      (FT)(crt[1]*Cell2Cartesian[1][1] + crt[2]*Cell2Cartesian[2][1]),
      (FT)(crt[2]*Cell2Cartesian[2][2]));
  }
  template <typename FT> TVector3<FT>& CartesianToCell(TVector3<FT>& cll) const
  {
    cll[0] = (FT)(cll[0]*Cartesian2Cell[0][0] + cll[1]*Cartesian2Cell[1][0] +
      cll[2]*Cartesian2Cell[2][0]);
    cll[1] = (FT)(cll[1]*Cartesian2Cell[1][1] + cll[2]*Cartesian2Cell[2][1]);
    cll[2] = (FT)(cll[2]*Cartesian2Cell[2][2]);
    return cll;
  }
  template <typename FT>
  TVector3<FT> Fractionalise(const TVector3<FT>& cll) const {
    return TVector3<FT>(
      (FT)(cll[0]*Cartesian2Cell[0][0] + cll[1]*Cartesian2Cell[1][0] +
      cll[2]*Cartesian2Cell[2][0]),
      (FT)(cll[1]*Cartesian2Cell[1][1] + cll[2]*Cartesian2Cell[2][1]),
      (FT)(cll[2]*Cartesian2Cell[2][2]));
  }

  // J App Cryst 2002, 35, 477-480
  template <class T> T& UcifToUcart(T& v) const {
    mat3d M(v[0], v[5], v[4], v[1], v[3], v[2]);
    M = UcifToUxyz*M*UcifToUxyzT;
    v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
    v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
    return v;
  }
  template <class T> T& UcartToUcif(T& v) const {
    mat3d M(v[0], v[5], v[4], v[1], v[3], v[2]);
    M = UxyzToUcif*M*UxyzToUcifT;
    v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
    v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
    return v;
  }
  template <class T> T& UstarToUcart(T& v) const {
    mat3d M(v[0], v[5], v[4], v[1], v[3], v[2]);
    M = Cell2Cartesian*M*Cell2CartesianT;
    v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
    v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
    return v;
  }
  template <class T> T& UcartToUstar(T& v) const {
    mat3d M(v[0], v[5], v[4], v[1], v[3], v[2]);
    M = Cartesian2Cell*M*Cartesian2CellT;
    v[0] = M[0][0];  v[1] = M[1][1];  v[2] = M[2][2];
    v[3] = M[1][2];  v[4] = M[0][2];  v[5] = M[0][1];
    return v;
  }
  /* copies the atoms from another AU, _UpdateConnInfo must be called after
  this
  */
  void Assign(const TAsymmUnit& C);
  void ChangeSpaceGroup(const class TSpaceGroup& sg);
  // executed from the above function, Data is the new space group
  TActionQueue& OnSGChange;
  /* initialises transofrmation matrices, called after axis and angles
  initialised
  */
  void InitMatrices();
  /* initialises data such as Q-peak heights, called after all atoms
  initialised
  */
  void InitData();

  void Clear();
  //creates a new residue
  TResidue& NewResidue(const olxstr& RClass, int number,
    int alias, olxch chainId);
  size_t ResidueCount() const {  return Residues.Count()+1;  }
  /* 0 - main residue */
  TResidue& GetResidue(size_t i) const {
    return (i==0) ? const_cast<TAsymmUnit*>(this)->MainResidue : Residues[i-1];
  }
  TResidue* NextResidue(const TResidue& r) const;
  TResidue* PrevResidue(const TResidue& r) const;
  TResidue* FindResidue(olxch chainId, int num) const;
  TResidue* FindResidue(const olxstr &number) const;
  // releases the given residues
  void Release(const TPtrList<TResidue> &rs);
  /* releases the given residues; after the residues restrored, they are sorted
  by the number
  */
  void Restore(const TPtrList<TResidue> &rs);
  void AssignResidues(const TAsymmUnit& au);
  // changes the atom order as in residues
  void ComplyToResidues();
  /* rearranges the atoms and updates the main residue atom order 
  */
  void RearrangeAtoms(const TSizeList &indices);
  // sorts residues by class and number
  void SortResidues();
  // if a number is provided, searches by Number otherwise - by ClassName
  ConstPtrList<TResidue> FindResidues(const olxstr& resi) const;
  // this is called internally by the TCAtom, to sync connectivity info
  void _OnAtomTypeChanged(TCAtom& caller);
  // called by the ref model
  void _UpdateConnInfo();
  void _UpdateAtomIds();
  //creates a new atom and puts it into the list
  TCAtom& NewAtom(TResidue* resi = NULL);
  //creates a new atom and puts it into the list
  TCAtom& NewCentroid(const vec3d &CCenter);
  /* returns an atom by label; if the label is not unique, returns the first
  found
  */
  TCAtom* FindCAtom(const olxstr &Label, TResidue* resi = NULL) const;
  /* direct label match search */
  TCAtom* FindCAtomDirect(const olxstr &Label) const;
  //returns an atom by Id
  TCAtom* FindCAtomById(size_t id) const  {
    return (id >= CAtoms.Count()) ? NULL : CAtoms[id];
  }
  // makes specified type detached or attached
  void DetachAtomType(short type, bool detach);
  /* removes all atoms marked as deleted */
  void PackAtoms();
  TCAtom& GetAtom(size_t i) const {  return *CAtoms[i];  }
  const TCAtomPList& GetAtoms() const {  return CAtoms;  }
  TCAtomPList& GetAtoms() {  return CAtoms;  }
  size_t AtomCount() const { return CAtoms.Count();  }
  void SetNonHAtomTags_();

  size_t MatrixCount() const {  return Matrices.Count();  }
  const smatd& GetMatrix(size_t i) const {  return Matrices[i];  }
  void ClearMatrices()  {  Matrices.Clear();  }
  void AddMatrix(const smatd& a);
  const smatd_list& GetMatices() const {  return Matrices;  }

  size_t EllpCount() const {  return Ellipsoids.Count();  }
  TEllipsoid& GetEllp(size_t i) const {  return *Ellipsoids[i];  }
  void NullEllp(size_t i);
  void ClearEllps();
  void PackEllps();
  // Q - six values representing quadratic form of a thermal ellipsoid
  TEllipsoid& NewEllp();  // initialisation performed manually !

  vec3d GetOCenter(bool IncludeQ, bool IncludeH) const;
  // returns properly sorted content list
  ContentList::const_list_type GetContentList(double mult=1.0) const;
  /* returns summarised formula of the asymmetric unit, use MutiplyZ to
  multiply the content by Z
  */
  olxstr SummFormula(const olxstr& sep, bool MultiplyZ=true) const;
  olxstr _SummFormula(const olxstr& sep, double mult) const;
  /* returns molecular weight of the asymmetric unit */
  double MolWeight() const;
  // returns sum of occupancy of any given element
  double CountElementOccupancy(const olxstr& Element) const;

  // sorts the content of the asymmetric unit or the list if provided
  void Sort(TCAtomPList* list = NULL);

  /* finds labels duplicate for the given atom list
  */
  TCAtomPList::const_list_type FindDiplicateLabels(const TCAtomPList &atoms,
    bool rename_parts);

  olxset<TCAtom *, TPointerComparator>::const_set_type
    GetAtomsNeedingPartInLabel() const;

  bool IsQPeakMinMaxInitialised() const {  return MaxQPeak != -1000;  }
  DefPropP(double, MinQPeak)
  DefPropP(double, MaxQPeak)
  /* atoms should have at least three atoms for fitting. If the atoms.atom is
  NULL, atoms.element must be provided, if atoms.bool is false, the atom is not
  used in the fitting. The missing atoms will be initialised on successful
  completion of the procedure. try_invert - try the fitting of the inverted set
  of coordinates
  */
  void FitAtoms(
    TTypeList<AnAssociation3<TCAtom*, const cm_Element*, bool> >& atoms,
    const vec3d_list& crds, bool try_invert);
  // returns next available positive/negative part
  int GetNextPart(bool negative=false) const;

  DefPropP(RefinementModel*, RefMod)
// IXVarReferencerContainer implementation
  static const olxstr& _GetIdName() {  return IdName;  }

  virtual olxstr GetIdName() const {  return IdName;  }
  // note - possibly unsafe, type is not checked
  virtual size_t GetIdOf(const IXVarReferencer& vr) const {
    if (!vr.Is<TCAtom>()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "referencer");
    }
    return ((TCAtom&)vr).GetId();
  }
  virtual size_t GetPersistentIdOf(const IXVarReferencer& vr) const {
    if (!vr.Is<TCAtom>()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "referencer");
    }
    return ((TCAtom&)vr).GetTag();
  }
  // note - possibly unsafe, range is unchecked
  virtual IXVarReferencer& GetReferencer(size_t id) const {
    if( id >= CAtoms.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "id");
    return *CAtoms[id];
  }
  virtual size_t ReferencerCount() const {  return CAtoms.Count();  }
//
  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms, bool export_conn);
#endif
  void FromDataItem(TDataItem& item);

  void LibGetAtomCount(const TStrObjList& Params, TMacroData& E);
  void LibNewAtom(const TStrObjList& Params, TMacroData& E);
  
  void LibGetAtomCrd(const TStrObjList& Params, TMacroData& E);
  void LibSetAtomCrd(const TStrObjList& Params, TMacroData& E);

  void LibGetAtomUiso(const TStrObjList& Params, TMacroData& E);
  void LibGetAtomU(const TStrObjList& Params, TMacroData& E);
  void LibSetAtomU(const TStrObjList& Params, TMacroData& E);

  void LibSetAtomOccu(const TStrObjList& Params, TMacroData& E);
  void LibGetAtomOccu(const TStrObjList& Params, TMacroData& E);

  void LibGetAtomPart(const TStrObjList& Params, TMacroData& E);
  void LibSetAtomPart(const TStrObjList& Params, TMacroData& E);

  void LibGetAtomName(const TStrObjList& Params, TMacroData& E);
  void LibGetAtomType(const TStrObjList& Params, TMacroData& E);
  void LibGetAtomAfix(const TStrObjList& Params, TMacroData& E);
  void LibGetPeak(const TStrObjList& Params, TMacroData& E);

  void LibGetCell(const TStrObjList& Params, TMacroData& E);
  void LibGetVolume(const TStrObjList& Params, TMacroData& E);
  void LibGetCellVolume(const TStrObjList& Params, TMacroData& E);

  void LibGetAtomLabel(const TStrObjList& Params, TMacroData& E);
  void LibSetAtomLabel(const TStrObjList& Params, TMacroData& E);
  // retruns label if +-num is used
  void LibIsAtomDeleted(const TStrObjList& Params, TMacroData& E);
  void LibIsPeak(const TStrObjList& Params, TMacroData& E);

  void LibNPDCount(const TStrObjList& Params, TMacroData& E);
  void LibGetSymm(const TStrObjList& Params, TMacroData& E);
  void LibGetZ(const TStrObjList& Params, TMacroData& E);
  void LibSetZ(const TStrObjList& Params, TMacroData& E);
  void LibGetZprime(const TStrObjList& Params, TMacroData& E);
  void LibSetZprime(const TStrObjList& Params, TMacroData& E);
  void LibFormula(const TStrObjList& Params, TMacroData& E);
  void LibWeight(const TStrObjList& Params, TMacroData& E);
  void LibOrthogonolise(const TStrObjList& Params, TMacroData& E);
  void LibFractionalise(const TStrObjList& Params, TMacroData& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString());

  struct VPtr : public olx_virtual_ptr<TAsymmUnit> {
    virtual IOlxObject *get_ptr() const;
  };

  struct TLabelChecker {
    const TAsymmUnit &parent;
    typedef olxstr_dict<size_t> label_dict_t;
    olxdict<uint32_t, label_dict_t, TPrimitiveComparator>
      r_labels;
    size_t max_label_length;
    TLabelChecker(const TAsymmUnit &au);
    olxstr CheckLabel(const TCAtom &ca, const olxstr &Label,
      const cm_Element *new_element,
      bool check_atom) const;
    void SetLabel(TCAtom &a, const olxstr& label, bool update_type=false);
  };
};


EndXlibNamespace()
#endif
