//---------------------------------------------------------------------------

#ifndef symmlibH
#define symmlibH
#include "evtypes.h"
#include "log.h"
#include "typelist.h"

#include "asymmunit.h"
#undef GetObject

BeginXlibNamespace()
const short  mattTranslation = 0x0001,
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
  int Latt;
public:
  TCLattice(int Latt);
  virtual ~TCLattice()  {  }
  inline int VectorCount()  const          {  return Vectors.Count();  }
  inline vec3d&  GetVector(int i) const {  return Vectors[i];  }
  inline const olxstr& GetName() const   {  return Name; }
  inline const olxstr& GetSymbol() const {  return Symbol; }

  inline int GetLatt()  const              {  return Latt;  }

  inline TCLattice& AddBravaiseLattice(TBravaisLattice* bl)  {
    BravaisLattices.Add( bl );
    return *this;
  }
  inline int BravaisLatticeCount()   const                {  return BravaisLattices.Count();  }
  inline TBravaisLattice& GetBravaisLattice(int i) const  {  return *BravaisLattices[i];  }
  
  class TIncorrectLattExc: public TBasicException  {
    int Latt;
    public:
      TIncorrectLattExc(const olxstr& location, int latt) :
        TBasicException(location, olxstr("Incorrect Latt instruction: ") << latt) {
          Latt = latt;  
        }
      inline int GetLatt() const  {  return Latt;  }
      virtual IEObject* Replicate()  const  {  return new TIncorrectLattExc(*this);  }
  };
};

class TSpaceGroup  {
  smatd_list Matrices;
  olxstr Name, FullName, Axis, HallSymbol;
  int Number;
  TCLattice *Latt;
  bool CentroSymmetric, Translations;
  TBravaisLattice* BravaisLattice;
  TSpaceGroup* LaueClass, *PointGroup;
public:
  TSpaceGroup(const olxstr& Name, const olxstr& FullName, const olxstr HallSymbol, 
              const olxstr& Axis, int Number, TCLattice& L, bool Centrosymmetric);
  virtual ~TSpaceGroup()  { ; }

  void SetBravaisLattice(TBravaisLattice& bl)  {  BravaisLattice = &bl;  }
  TBravaisLattice& GetBravaisLattice()  const  {  return *BravaisLattice;  }
  void SetLaueClass(TSpaceGroup& lc)           {  LaueClass = &lc;  }
  TSpaceGroup&     GetLaueClass()       const  {  return *LaueClass;  }
  void SetPointGroup(TSpaceGroup& lc)          {  PointGroup = &lc;  }
  TSpaceGroup&     GetPointGroup()       const {  return *PointGroup;  }

  bool operator == (const TAsymmUnit& AU) const;
  bool operator == (const smatd_list& matrices) const;
  // compares m.R and summs (delta(m.t))^2 into st;
  bool Compare(const smatd_list& matrices, double& st) const;

  bool EqualsExpandedSG(const TAsymmUnit& AU) const;

  bool EqualsWithoutTranslation (const TSpaceGroup& sg) const;
  bool IsSubElement( TSpaceGroup* symme )  const;

  void AddMatrix(const smatd& m);
  void AddMatrix(double xx, double xy, double xz, 
                 double yx, double yy, double yz, 
                 double zx, double zy, double zz,
                 double tx, double ty, double tz);
  inline int MatrixCount()             const {  return Matrices.Count();  };
  inline smatd& GetMatrix(int i)       const {  return Matrices[i];  }
  inline int GetNumber()               const {  return Number;  }
  inline const olxstr& GetName()       const {  return Name;  }
  inline olxstr GetBareName()          const {  return Name.SubStringFrom(1);  }
  inline const olxstr& GetFullName()   const {  return FullName;  }
  inline const olxstr& GetAxis()       const {  return Axis;  }
  inline const olxstr& GetHallSymbol() const {  return HallSymbol;  }

  inline TCLattice& GetLattice()         const {  return *Latt;  }
  inline bool IsCentrosymmetric()        const {  return CentroSymmetric;  }
  // retruns true if any matrix of the SG has a nonzero translation
  inline bool HasTranslations()          const {  return Translations;  }

  void GetMatrices(smatd_list& matrices, short Flags) const;
  // fills a list of uniq transformations (3,3) without translation and returns
  // the number of added matrices; the list is created from a call to GetMatrices(list, flag)
  int GetUniqMatrices(smatd_list& matrices, short Flags) const;

  // this function finds a symmetry element in a list of matrices
  static bool ContainsElement( const smatd_list& matrices, TSymmElement* symme);
  // this function is used to assign point groups to the space group
  bool ContainsElement(TSymmElement* symme);
  bool ContainsGroup(TSpaceGroup* symme);
};

class TSymmElement  {
  smatd_list Matrices;
  olxstr Name;
public:
  TSymmElement(const olxstr& name, TSpaceGroup* sg);
  TSymmElement(const olxstr& name)  {  Name = name;  }
  virtual ~TSymmElement()  {  }

  inline TSymmElement& AddMatrix(const smatd& m)  {  Matrices.AddCCopy(m);  return *this;  }
  inline int  MatrixCount()  const  {  return Matrices.Count();  }
  smatd&  GetMatrix(int i) const  {  return Matrices[i];  }
  const olxstr& GetName()  const   {  return Name;  }
};

class TBravaisLattice  {
  TPtrList<TSpaceGroup> Symmetries;
  TPtrList<TCLattice> Lattices;
  olxstr Name;
public:
  TBravaisLattice(const olxstr& Name)  {  this->Name = Name;  }
  virtual ~TBravaisLattice()  {  ;  }

  const olxstr& GetName()  const       {  return Name;  }

  TBravaisLattice& AddLattice(TCLattice* latt)  {
    Lattices.Add(latt);
    return *this;
  }
  inline TCLattice& GetLattice(int i)  const  {  return *Lattices[i];  }
  inline int LatticeCount()  const            {  return Lattices.Count();  }

  TBravaisLattice& AddSymmetry(TSpaceGroup* sg)  {
    Symmetries.Add( sg );
    return *this;
  }
  inline TSpaceGroup& GetSymmetry(int i)  const  {  return *Symmetries[i];  }
  inline int SymmetryCount()  const              {  return Symmetries.Count();  }

  // finds all space groups with this bravais lattice and returns the number of added to the list
  int FindSpaceGroups(TPtrList<TSpaceGroup>& SpaceGroups) const;

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
  TSpaceGroup* FindExpandedSG(const TAsymmUnit& AU) const;
  int FindBravaisLattices(TAsymmUnit& AU, TTypeList<TBravaisLatticeRef>& res) const;
  // finds all space groups of specified point group
  int FindPointGroupGroups(const TSpaceGroup& PointGroup, TPtrList<TSpaceGroup>& res) const;
  // finds all space groups of specified Laue class
  int FindLaueClassGroups(const TSpaceGroup& LaueClass, TPtrList<TSpaceGroup>& res) const;

  inline int SGCount() const                {  return SpaceGroups.Count();  }
  inline TSpaceGroup& GetGroup(int i) const {  return *SpaceGroups.GetObject(i);  }
  void GetGroupByNumber(int N, TPtrList<TSpaceGroup>& res) const;
  inline TSpaceGroup* FindGroup(const olxstr& Name)  const  {
    return SpaceGroups[Name];
  }

  inline int SymmElementCount()  const  {  return SymmetryElements.Count();  }
  inline TSymmElement&  GetSymmElement(int i)  const  {  return SymmetryElements[i];  }
  TSymmElement*  FindSymmElement(const olxstr& name)  const;

  inline int LatticeCount() const           {  return Lattices.Count();  }
  inline TCLattice& GetLattice(int i) const {  return *Lattices.Object(i);  }
  inline TCLattice* FindLattice(const olxstr& Symbol) const {
    return Lattices.FindObjectCI(Symbol);
  }

  inline int PointGroupCount() const             {  return PointGroups.Count();  }
  inline TSpaceGroup& GetPointGroup(int i) const {  return *PointGroups[i];  }

  inline int BravaisLatticeCount() const    {  return BravaisLattices.Count();  }
  inline TBravaisLattice&  GetBravaisLattice(int i)  {  return *BravaisLattices.Object(i);  }
  inline TBravaisLattice *  FindBravaisLattice(const olxstr& Name)  const {
    return BravaisLattices.FindObjectCI(Name);
  }

  static TSymmLib*  GetInstance()  {  return Instance;  }
};

EndXlibNamespace()

#endif
