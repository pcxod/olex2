/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_symmlib_H
#define __olx_xl_symmlib_H
#include "log.h"
#include "typelist.h"
#include "asymmunit.h"
#include "hall.h"
#undef GetObject
BeginXlibNamespace()

const short
  mattTranslation = 0x0001,
  mattCentering   = 0x0002,
  mattInversion   = 0x0004,
  mattIdentity    = 0x0008,
  mattAll =  0x0001|0x0002|0x0004|0x0008;

class TBravaisLattice;
class TSymmElement;

class TCLattice  {
  vec3d_list Vectors;
  TPtrList<TBravaisLattice> BravaisLattices;
  olxstr Name, Symbol;
  short Latt;
public:
  TCLattice(int Latt);
  virtual ~TCLattice()  {  }
  const vec3d_list &GetVectors() const { return Vectors; }
  /** returns the multiplicity imposed by latt instruction, considers
  inversion for positive number (will multiply returned value by 2 for
  positive numbers)
  */
  static size_t GetLattMultiplier(short latt)  {
    size_t count = 0;
    switch( olx_abs(latt) )  {
      case 1: count = 1;  break; // P
      case 2:  // Body Centered (I)
      case 5:  // A Centered (A)
      case 6:  // B Centered (B)
      case 7: count = 2;  break;  // C Centered (C);
      case 8:  // S Centered
      case 9:  // T Centered
      case 10:  // D Centered
      case 11:  // H Centered
      case 3: count = 3;  break;  // R Centered
      case 4: count = 4;  break;  // Face Centered (F)
      default:
        throw TIncorrectLattExc(__OlxSourceInfo, latt);
    }
    if( latt > 0 )  count *= 2;
    return count;
  }
  /* returns numerical representation of lattice symbol */
  static short LattForSymbol(olxch latt)  {
    switch( olxstr::o_toupper(latt) )  {
      case 'P': return 1;
      case 'I': return 2;
      case 'A': return 5;
      case 'B': return 6;
      case 'C': return 7;
      case 'S': return 8;
      case 'T': return 9;
      case 'D': return 10;
      case 'H': return 11;
      case 'R': return 3;
      case 'F': return 4;
      default:
        throw TIncorrectLattExc(__OlxSourceInfo, latt);
    }
  }
  /* returns symbolic representation of lattice centering */
  static char SymbolForLatt(short latt)  {
    const char rv[12] = "PIRFABCSTDH"; // +\0
    if (latt > 0 && latt < 12)
      return rv[latt-1];
    throw TIncorrectLattExc(__OlxSourceInfo, latt);
  }
  const olxstr& GetName() const {  return Name; }
  const olxstr& GetSymbol() const {  return Symbol; }

  short GetLatt() const {  return Latt;  }

  TCLattice& AddBravaiseLattice(TBravaisLattice* bl)  {
    BravaisLattices.Add(bl);
    return *this;
  }
  size_t BravaisLatticeCount() const {  return BravaisLattices.Count();  }
  TBravaisLattice& GetBravaisLattice(size_t i) const {
    return *BravaisLattices[i];
  }
  // returns a transformation to this lattice from primitive lattice
  static mat3d FromPrimitive(int latt);
  mat3d FromPrimitive() const {  return FromPrimitive(Latt);  }
  class TIncorrectLattExc: public TBasicException  {
    short Latt;
    public:
      TIncorrectLattExc(const olxstr& location, int latt) :
        TBasicException(location,
          olxstr("Incorrect Latt instruction: ") << latt)
        {
          Latt = latt;
        }
      short GetLatt() const  {  return Latt;  }
      virtual IOlxObject* Replicate() const {
        return new TIncorrectLattExc(*this);
      }
  };
};

class TSpaceGroup : public IOlxObject {
  smatd_list Matrices;
  olxstr Name, FullName, Axis, HallSymbol;
  int Number;
  TCLattice& Latt;
  bool CentroSymmetric, Translations;
  TBravaisLattice* BravaisLattice;
  TSpaceGroup* LaueClass, *PointGroup;
  // initialised by the SymLib
  vec3d InversionCenter;
  static bool _checkTDS(const vec3d& t1, const vec3d& t2);
  static bool _checkTD(const vec3d& t1, const vec3d& t2);
public:
  TSpaceGroup(const smatd_list &matrices,
    const olxstr& Name, const olxstr& FullName,
    const olxstr& Axis, int Number, TCLattice& L,
    bool Centrosymmetric, const olxstr hall_symbol=EmptyString());
  virtual ~TSpaceGroup()  {}

  void SetBravaisLattice(TBravaisLattice& bl)  {  BravaisLattice = &bl;  }
  TBravaisLattice& GetBravaisLattice() const {  return *BravaisLattice;  }
  bool HasBravaisLattice() const { return BravaisLattice != 0; }
  void SetLaueClass(TSpaceGroup& lc)  {  LaueClass = &lc;  }
  TSpaceGroup& GetLaueClass() const {  return *LaueClass;  }
  bool HasLaueClass() const { return LaueClass != 0; }
  void SetPointGroup(TSpaceGroup& lc)  {  PointGroup = &lc;  }
  TSpaceGroup& GetPointGroup() const {  return *PointGroup;  }
  bool HasPointGroup() const { return PointGroup != 0; }

  SymmSpace::Info GetInfo() const;
  /* compares m.R and translations for positive rotation matrix
  elements. If the origin shift is identified - it is returned
  */
  olx_object_ptr<vec3d> Compare(const smatd_list& matrices) const;
  bool EqualsWithoutTranslation(const TSpaceGroup& sg) const;
  bool IsSubElement(TSpaceGroup* symme )  const;
  // decomoses space group into symmetry elements using reference as the basis
  void SplitIntoElements(TPtrList<TSymmElement>& reference,
    TPtrList<TSymmElement>& res);
  static void SplitIntoElements(smatd_list& matrices,
    TPtrList<TSymmElement>& reference, TPtrList<TSymmElement>& res);

  size_t MatrixCount() const {  return Matrices.Count();  };
  smatd& GetMatrix(size_t i) const {  return Matrices[i];  }
  int GetNumber() const {  return Number;  }
  const olxstr& GetName() const {  return Name;  }
  olxstr GetBareName() const {  return Name.SubStringFrom(1);  }
  const olxstr& GetFullName() const {  return FullName;  }
  const olxstr& GetAxis() const {  return Axis;  }
  const olxstr& GetHallSymbol() const {  return HallSymbol;  }

  const TCLattice& GetLattice() const {  return Latt;  }
  bool IsCentrosymmetric() const {  return CentroSymmetric;  }
  // retruns true if any matrix of the SG has a nonzero translation
  bool HasTranslations() const {  return Translations;  }
  /* it is NOT (0,0,0) for Fdd2, I41, I4122, I4132, I41md, I41cd, I-42d!
  http://xrayweb.chem.ou.edu/notes/symmetry.html
  */
  const vec3d& GetInversionCenter() const {  return InversionCenter;  }

  void GetMatrices(smatd_list& matrices, short Flags) const;
  smatd_list::const_list_type GetMatrices(short Flags) const {
    smatd_list matrices;
    GetMatrices(matrices, Flags);
    return matrices;
  }
  /* fills a list of uniq transformations (3,3) without translation and returns
   the number of added matrices; the list is created from a call to
   GetMatrices(list, flag)
   */
  size_t GetUniqMatrices(smatd_list& matrices, short Flags) const;

  // this function finds a symmetry element in a list of matrices
  static bool ContainsElement(const smatd_list& matrices, TSymmElement* symme);
  // this function is used to assign point groups to the space group
  bool ContainsElement(TSymmElement* symme);
  bool ContainsGroup(TSpaceGroup* symme);

  typedef TSymmSpace<smatd_list> SymmSpace_;

  SymmSpace_ GetSymSpace(const TAsymmUnit& au) const {
    smatd_list ml;
    GetMatrices(ml, mattAll);
    return SymmSpace_(ml,
      au.GetCartesianToCell(), au.GetCellToCartesian(), au.GetHklToCartesian(),
      IsCentrosymmetric());
  }
  friend class TSymmLib;
};

class TSymmElement : public ACollectionItem {
  smatd_list Matrices;
  olxstr Name;
  TSymmElement* SuperElement;
public:
  TSymmElement(const olxstr& name, TSpaceGroup* sg);
  TSymmElement(const olxstr& name) : Name(name), SuperElement(NULL)  {}
  virtual ~TSymmElement()  {}

  TSymmElement& AddMatrix(const smatd& m)  {
    Matrices.AddCopy(m);
    return *this;
  }
  size_t  MatrixCount() const {  return Matrices.Count();  }
  smatd&  GetMatrix(size_t i) const {  return Matrices[i];  }
  const olxstr& GetName() const {  return Name;  }
  TSymmElement* GetSuperElement() const {  return SuperElement;  }
  friend class TSymmLib;
};

class TBravaisLattice  {
  TPtrList<TSpaceGroup> Symmetries;
  TPtrList<TCLattice> Lattices;
  olxstr Name;
public:
  TBravaisLattice(const olxstr& Name)  {  this->Name = Name;  }
  virtual ~TBravaisLattice()  {}

  const olxstr& GetName() const {  return Name;  }

  TBravaisLattice& AddLattice(TCLattice* latt)  {
    Lattices.Add(latt);
    return *this;
  }
  TCLattice& GetLattice(size_t i) const {  return *Lattices[i];  }
  size_t LatticeCount() const {  return Lattices.Count();  }

  TBravaisLattice& AddSymmetry(TSpaceGroup* sg)  {
    Symmetries.Add( sg );
    return *this;
  }
  TSpaceGroup& GetSymmetry(size_t i) const {  return *Symmetries[i];  }
  size_t SymmetryCount() const {  return Symmetries.Count();  }

  /* finds all space groups with this bravais lattice and returns the number of
  added to the list
  */
  size_t FindSpaceGroups(TPtrList<TSpaceGroup>& SpaceGroups) const;

};
/* these types are used to specify found bravais latteces: int < 0 - symmetry
is lower and int > 0 - symmetry is higher (normally never returned :))
*/
typedef olx_pair_t<TBravaisLattice*,int> TBravaisLatticeRef;

class TSymmLib: public IOlxObject  {
  sorted::StringAssociation<TSpaceGroup*, true> SpaceGroups;
  TStringToList<olxstr, TCLattice*> Lattices;
  TStringToList<olxstr, TBravaisLattice*> BravaisLattices;
  TPtrList<TSpaceGroup> PointGroups;
  TTypeList<TPtrList<TSpaceGroup> > _PointGroups;
  TTypeList<TSymmElement> SymmetryElements;
  olxstr_dict<TSpaceGroup*, false> hall_symbols;
  void InitRelations();
  static TSymmLib* Instance;
  int extra_added;
  TSpaceGroup &CreateNewFromCompact(int latt,
    const smatd_list& compact_matrices, const olxstr &hall_sb=EmptyString());
  TSpaceGroup &InitSpaceGroup(TSpaceGroup &sg);
  TSpaceGroup &CreateNew(const SymmSpace::Info &si_info, const olxstr &hall_sb);
public:
  // 21.06.2008, the file name is not used
  TSymmLib(const olxstr& FN=EmptyString());
  virtual ~TSymmLib();
  // creates a dummy space group if not found
  TSpaceGroup& FindSG(const TAsymmUnit& AU);

  TSpaceGroup &CreateNew(const olxstr &hall);
  TSpaceGroup &CreateNew(const SymmSpace::Info &si_info);
  // searches for expanded space groups like in the CIF
  template <class SymmSpaceT>
  TSpaceGroup& FindSymSpace(const SymmSpaceT &sp) {
    smatd_list all_ml(sp);
    SymmSpace::Info si = SymmSpace::GetInfo(all_ml);
    olxstr hs = HallSymbol::Evaluate(si);
    TSpaceGroup *sg = hall_symbols.Find(hs, NULL);
    return sg == NULL ? CreateNew(si, hs) : *sg;
  }
  size_t FindBravaisLattices(TAsymmUnit& AU,
    TTypeList<TBravaisLatticeRef>& res) const;
  // finds all space groups of specified point group
  size_t FindPointGroupGroups(const TSpaceGroup& PointGroup,
    TPtrList<TSpaceGroup>& res) const;
  // finds all space groups of specified Laue class
  size_t FindLaueClassGroups(const TSpaceGroup& LaueClass,
    TPtrList<TSpaceGroup>& res) const;

  size_t SGCount() const {  return SpaceGroups.Count();  }
  TSpaceGroup& GetGroup(size_t i) const {  return *SpaceGroups.GetValue(i);  }
  void GetGroupByNumber(int N, TPtrList<TSpaceGroup>& res) const;
  TSpaceGroup* FindGroupByName(const olxstr& Name) const {
    return SpaceGroups.Find(Name);
  }
  TSpaceGroup* FindGroupByHallSymbol(const olxstr &hs,
  TSpaceGroup *def=NULL) const
 {
    return hall_symbols.Find(hs, def);
  }

  size_t SymmElementCount() const {  return SymmetryElements.Count();  }
  TSymmElement& GetSymmElement(size_t i) const {
    return SymmetryElements[i];
  }
  TSymmElement* FindSymmElement(const olxstr& name) const;

  size_t LatticeCount() const {  return Lattices.Count();  }
  TCLattice& GetLatticeByIndex(size_t i) const {
    return *Lattices.GetObject(i);
  }
  TCLattice& GetLatticeByNumber(short latt) const {
    size_t l = olx_abs(latt)-1;
    if( l >= Lattices.Count() )
      throw TCLattice::TIncorrectLattExc(__OlxSourceInfo, latt);
    return *Lattices.GetObject(l);
  }
  TCLattice* FindLattice(const olxstr& Symbol) const {
    return Lattices.FindPointeri(Symbol, NULL);
  }

  size_t PointGroupCount() const {  return PointGroups.Count();  }
  TSpaceGroup& GetPointGroup(size_t i) const {  return *PointGroups[i];  }

  size_t BravaisLatticeCount() const {  return BravaisLattices.Count();  }
  TBravaisLattice&  GetBravaisLattice(size_t i) const {
    return *BravaisLattices.GetObject(i);
  }
  TBravaisLattice *FindBravaisLattice(const olxstr& Name) const {
    return BravaisLattices.FindPointeri(Name, NULL);
  }

  template <typename MatList> void ExpandLatt(smatd_list& out,
    const MatList& ml, short _latt) const
  {
    const TCLattice& latt = GetLatticeByNumber(_latt);
    out.SetCapacity(latt.GetVectors().Count()*ml.Count()* (_latt > 0 ? 2 : 1));
    out.AddNew().r.I();
    for (size_t i=0;  i < ml.Count(); i++) {
      const smatd& m = ml[i];
      if (!m.r.IsI())  // skip I matrix - always the first one
        out.AddNew(m);
    }
    const size_t mc = out.Count();
    for (size_t i= 0; i < latt.GetVectors().Count(); i++) {
      for (size_t j = 0; j < mc; j++) {
        out.AddCopy(out[j]).t += latt.GetVectors()[i];
      }
    }
    if (_latt > 0) {
      for (size_t j = 0; j < mc; j++) out.AddCopy(out[j].Negate());
      for (size_t i = 0; i < latt.GetVectors().Count(); i++) {
        for (size_t j = 0; j < mc; j++) {
          out.AddCopy(out[j].Negate()).t += latt.GetVectors()[i];
        }
      }
    }
  }

  static bool IsInitialised()  {  return Instance != NULL;  }

  static TSymmLib& GetInstance()  {
    if( Instance == NULL ) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "object is not initialised");
    }
    return *Instance;
  }
};

EndXlibNamespace()

#endif
