/* (c) O. Dolomanov, 2004 */
#ifndef __olx_xl_symmlib_H
#define __olx_xl_symmlib_H
#include "log.h"
#include "typelist.h"
#include "asymmunit.h"
#include "symspace.h"

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
  TTypeList<vec3d> Vectors;
  TPtrList<TBravaisLattice> BravaisLattices;
  olxstr Name, Symbol;
  short Latt;
public:
  TCLattice(int Latt);
  virtual ~TCLattice()  {  }
  size_t VectorCount() const {  return Vectors.Count();  }
  vec3d&  GetVector(size_t i) const {  return Vectors[i];  }
  const olxstr& GetName() const {  return Name; }
  const olxstr& GetSymbol() const {  return Symbol; }

  short GetLatt() const {  return Latt;  }

  TCLattice& AddBravaiseLattice(TBravaisLattice* bl)  {
    BravaisLattices.Add(bl);
    return *this;
  }
  size_t BravaisLatticeCount() const {  return BravaisLattices.Count();  }
  TBravaisLattice& GetBravaisLattice(size_t i) const {  return *BravaisLattices[i];  }
  
  class TIncorrectLattExc: public TBasicException  {
    short Latt;
    public:
      TIncorrectLattExc(const olxstr& location, int latt) :
        TBasicException(location, olxstr("Incorrect Latt instruction: ") << latt) {
          Latt = latt;  
        }
      short GetLatt() const  {  return Latt;  }
      virtual IEObject* Replicate() const {  return new TIncorrectLattExc(*this);  }
  };
};

class TSpaceGroup : public IEObject {
  smatd_list Matrices;
  olxstr Name, FullName, Axis, HallSymbol;
  int Number;
  TCLattice *Latt;
  bool CentroSymmetric, Translations;
  TBravaisLattice* BravaisLattice;
  TSpaceGroup* LaueClass, *PointGroup;
  // initialised by the SymLib 
  vec3d InversionCenter;
public:
  TSpaceGroup(const olxstr& Name, const olxstr& FullName, const olxstr HallSymbol, 
              const olxstr& Axis, int Number, TCLattice& L, bool Centrosymmetric);
  virtual ~TSpaceGroup()  { ; }

  void SetBravaisLattice(TBravaisLattice& bl)  {  BravaisLattice = &bl;  }
  TBravaisLattice& GetBravaisLattice() const {  return *BravaisLattice;  }
  void SetLaueClass(TSpaceGroup& lc)  {  LaueClass = &lc;  }
  TSpaceGroup&     GetLaueClass() const {  return *LaueClass;  }
  void SetPointGroup(TSpaceGroup& lc)  {  PointGroup = &lc;  }
  TSpaceGroup&     GetPointGroup() const {  return *PointGroup;  }

  bool operator == (const TAsymmUnit& AU) const;
  bool operator == (const smatd_list& matrices) const;
  // compares m.R and summs (delta(m.t))^2 into st;
  bool Compare(const smatd_list& matrices, double& st) const;

  template <class SymSpace> bool EqualsSymSpace(const SymSpace& sp) const {
    if( MatrixCount() > sp.Count() )  return false;
    smatd_list allMatrices;
    GetMatrices(allMatrices, mattAll );
    if( allMatrices.Count() != sp.Count() )  return false;
    for( size_t i=0; i  < allMatrices.Count(); i++ )
      allMatrices[i].SetRawId(0);
    for( size_t i=0; i  < allMatrices.Count(); i++ )  {
      bool found = false;
      const smatd& m = sp[i];
      for( size_t j=0; j  < allMatrices.Count(); j++ )  {
        smatd& m1 = allMatrices[j];
        if( m1.GetId() != 0 )  continue;
        bool equal = true;
        for( size_t k=0; k < 3; k++ )  {
          for( size_t l=0; l < 3; l++ )  {
            if( m.r[k][l] != m1.r[k][l] )  {
              equal = false;
              break;
            }
          }
          if( !equal )  break;
        }
        if( !equal )  continue;
        vec3d translation;
        for( size_t k=0; k < 3; k++ )  {
          translation[k] = m.t[k] - m1.t[k];
          const int iv = (int)translation[k];
          translation[k] -= iv;
          if( olx_abs(translation[k]) < 0.01 || olx_abs(translation[k]) >= 0.99 )
            translation[k] = 0;
          if( translation[k] < 0 )
            translation[k] += 1;
        }
        if( translation.QLength() < 0.0001 )  {
          found = true;
          m1.SetRawId(1);
          break;
        }
      }
      if( !found )  return false;
    }
    return true;
  }
  bool EqualsWithoutTranslation (const TSpaceGroup& sg) const;
  bool IsSubElement( TSpaceGroup* symme )  const;
  // decomoses space group into symmetry elements using reference as the basis
  void SplitIntoElements(TPtrList<TSymmElement>& reference, TPtrList<TSymmElement>& res);
  static void SplitIntoElements(smatd_list& matrices, TPtrList<TSymmElement>& reference, TPtrList<TSymmElement>& res);
  
  void AddMatrix(const smatd& m);
  void AddMatrix(int xx, int xy, int xz, 
                 int yx, int yy, int yz, 
                 int zx, int zy, int zz,
                 double tx, double ty, double tz);
  size_t MatrixCount() const {  return Matrices.Count();  };
  smatd& GetMatrix(size_t i) const {  return Matrices[i];  }
  int GetNumber() const {  return Number;  }
  const olxstr& GetName() const {  return Name;  }
  olxstr GetBareName() const {  return Name.SubStringFrom(1);  }
  const olxstr& GetFullName() const {  return FullName;  }
  const olxstr& GetAxis() const {  return Axis;  }
  const olxstr& GetHallSymbol() const {  return HallSymbol;  }

  TCLattice& GetLattice() const {  return *Latt;  }
  bool IsCentrosymmetric() const {  return CentroSymmetric;  }
  // retruns true if any matrix of the SG has a nonzero translation
  bool HasTranslations() const {  return Translations;  }
  /* it is NOT (0,0,0) for Fdd2, I41, I4122, I4132, I41md, I41cd, I-42d! 
  http://xrayweb.chem.ou.edu/notes/symmetry.html
  */
  const vec3d& GetInversionCenter() const {  return InversionCenter;  }

  void GetMatrices(smatd_list& matrices, short Flags) const;
  // fills a list of uniq transformations (3,3) without translation and returns
  // the number of added matrices; the list is created from a call to GetMatrices(list, flag)
  size_t GetUniqMatrices(smatd_list& matrices, short Flags) const;

  // this function finds a symmetry element in a list of matrices
  static bool ContainsElement( const smatd_list& matrices, TSymmElement* symme);
  // this function is used to assign point groups to the space group
  bool ContainsElement(TSymmElement* symme);
  bool ContainsGroup(TSpaceGroup* symme);
  typedef TSymSpace<smatd_list> SymSpace;

  SymSpace GetSymSpace(const TAsymmUnit& au) const {
    smatd_list ml;
    GetMatrices(ml, mattAll);
    return SymSpace(ml,
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
  TSymmElement(const olxstr& name) : Name(name), SuperElement(NULL)  {  }
  virtual ~TSymmElement()  {  }

  TSymmElement& AddMatrix(const smatd& m)  {  Matrices.AddCCopy(m);  return *this;  }
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
  virtual ~TBravaisLattice()  {  ;  }

  const olxstr& GetName() const      {  return Name;  }

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

  // finds all space groups with this bravais lattice and returns the number of added to the list
  size_t FindSpaceGroups(TPtrList<TSpaceGroup>& SpaceGroups) const;

};
// this types is used to specify found bravais latteces: int < 0 - symmetry is lower
// and int > 0 - symmetry is higher (normally never returned :))
typedef AnAssociation2<TBravaisLattice*,int> TBravaisLatticeRef;

class TSymmLib: public IEObject  {
  TSStrPObjList<olxstr,TSpaceGroup*, true>  SpaceGroups;
  TStrPObjList<olxstr,TCLattice*>  Lattices;
  TStrPObjList<olxstr,TBravaisLattice*>  BravaisLattices;
  TPtrList<TSpaceGroup> PointGroups;
  TTypeList<TSymmElement> SymmetryElements;
  void InitRelations();
  static TSymmLib* Instance;
public:
  // 21.06.2008, the file name is not used
  TSymmLib(const olxstr& FN=EmptyString);
  virtual ~TSymmLib();

  TSpaceGroup* FindSG(const TAsymmUnit& AU) const;
  TSpaceGroup* FindSG(const smatd_list& expanded_matrices) const;

  // searches for expanded space groups like in the CIF
  template <class SymSpace> TSpaceGroup* FindSymSpace(const SymSpace& sp) const {
    for( size_t i=0; i < SGCount(); i++ )
      if( GetGroup(i).EqualsSymSpace(sp) )
        return &(GetGroup(i));
    return NULL;
  }
  size_t FindBravaisLattices(TAsymmUnit& AU, TTypeList<TBravaisLatticeRef>& res) const;
  // finds all space groups of specified point group
  size_t FindPointGroupGroups(const TSpaceGroup& PointGroup, TPtrList<TSpaceGroup>& res) const;
  // finds all space groups of specified Laue class
  size_t FindLaueClassGroups(const TSpaceGroup& LaueClass, TPtrList<TSpaceGroup>& res) const;

  size_t SGCount() const {  return SpaceGroups.Count();  }
  TSpaceGroup& GetGroup(size_t i) const {  return *SpaceGroups.GetObject(i);  }
  void GetGroupByNumber(int N, TPtrList<TSpaceGroup>& res) const;
  TSpaceGroup* FindGroup(const olxstr& Name) const {
    return SpaceGroups[Name];
  }

  size_t SymmElementCount() const {  return SymmetryElements.Count();  }
  TSymmElement&  GetSymmElement(size_t i) const {  return SymmetryElements[i];  }
  TSymmElement*  FindSymmElement(const olxstr& name)  const;

  size_t LatticeCount() const {  return Lattices.Count();  }
  TCLattice& GetLattice(size_t i) const {  return *Lattices.GetObject(i);  }
  TCLattice* FindLattice(const olxstr& Symbol) const {
    return Lattices.FindObjecti(Symbol);
  }

  size_t PointGroupCount() const {  return PointGroups.Count();  }
  TSpaceGroup& GetPointGroup(size_t i) const {  return *PointGroups[i];  }

  size_t BravaisLatticeCount() const {  return BravaisLattices.Count();  }
  TBravaisLattice&  GetBravaisLattice(size_t i) const {  return *BravaisLattices.GetObject(i);  }
  TBravaisLattice *  FindBravaisLattice(const olxstr& Name)  const {
    return BravaisLattices.FindObjecti(Name);
  }

  static bool IsInitialised()  {  return Instance != NULL;  }

  static TSymmLib& GetInstance()  {
    if( Instance == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "object is not initialised");
    return *Instance;
  }
};

EndXlibNamespace()

#endif
