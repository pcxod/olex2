#ifndef asymmunitH
#define asymmunitH

#include "xbase.h"
#include "atominfo.h"
#include "elist.h"
#include "ematrix.h"
#include "threex3.h"

#include "evpoint.h"
#include "macroerror.h"

#include "scat_it.h"
#include "srestraint.h"

#include "ellipsoid.h"
#include "samegroup.h"

#include "dataitem.h"

#undef GetObject

BeginXlibNamespace()

class TAsymmUnit: public IEObject  {
  TCAtomPList CAtoms;      // list of TCAtoms
  smatd_list  Matrices;  // list of matrices (excluding ones after centering)
  TEllpPList Ellipsoids;
  TAtomsInfo *AtomsInfo;
  mat3d Cell2Cartesian,  // transformation from cell crd to cartesian
        Cartesian2Cell;  // transformation from cartesian crd to cell
  mat3d UcifToUxyz,  UxyzToUcif,
           UcifToUxyzT, UxyzToUcifT;
  mat3d Hkl2Cartesian;  // transformation from HKL crd to cartesian
  TCAtomPList Centroids;
  double            MaxQPeak,
                    MinQPeak;
  short             Z,
                    Latt;
  bool              ContainsEquivalents;
  class TLattice*   Lattice;    // parent lattice
  olxstr           FCellSymmetry;
  TEVPointD  FAxes;    // axes with errors
  TEVPointD  FAngles;    // angles + errors
  vec3d   RAxes;     // reciprical axes
  vec3d   RAngles;    // reciprical angles
  TCSTypeList<olxstr, TLibScatterer*> SfacData;  // label + params
  // this list holds the list of all atoms which are not deleted
public:  // residue class implementation
  class TResidue : public IEObject  {
    olxstr ClassName, Alias;
    int Number, Id;
    TCAtomPList Atoms;
    TAsymmUnit& Parent;
  public:
    TResidue(TAsymmUnit& parent, int id, const olxstr& cl=EmptyString, int number = 0, const olxstr& alias=EmptyString) : 
        Parent(parent), Id(id), ClassName(cl), Number(number), Alias(alias) {  }
      DefPropC(olxstr, ClassName)
      DefPropC(olxstr, Alias)
      DefPropP(int, Number)
      inline int GetId()          const {  return Id;  }
      inline TAsymmUnit& GetParent()    {  return Parent;  }
      TCAtomPList& AtomList()           {  return Atoms;  }
      virtual TIString ToString() const {
        if( Id == -1 )  return EmptyString;
        olxstr rv("RESI ");
        rv << ClassName;
        if( Number != 0 )  rv << ' ' << Number;
        return (rv << (Alias.IsEmpty() ? EmptyString : (olxstr(' ') << Alias)));
      }
      inline int Count()                 const {  return Atoms.Count();  }  
      inline TCAtom& GetAtom(int i)      const {  return *Atoms[i];  }
      inline TCAtom& operator [] (int i) const {  return *Atoms[i]; }
      inline void Clear()                      {  Atoms.Clear();  } 
      inline void Remove(TCAtom* ca)           {
        int i = Atoms.IndexOf(ca);
        if( i != -1 )  Atoms.Delete(i);
      }
      inline void AddAtom( TCAtom* ca )        {
        Atoms.Add( ca );
        ca->SetResiId( Id );
      }
      inline void SetCapacity(int c)  {  Atoms.SetCapacity(c);  }
      bool IsEmpty() const {
        for( int i=0; i < Atoms.Count(); i++ )
          if( !Atoms[i]->IsDeleted() )  return false;
        return true;
      }
      // this assumes that atoms and their Ids of the Parent already assigned 
      const TResidue& operator = (const TResidue& res)  {
        Atoms.Clear();
        Atoms.SetCapacity(res.Count() );
        for( int i=0; i < res.Count(); i++ )  {
          if( res[i].GetId() >= 0 && res[i].GetId() < Parent.AtomCount() )
            AddAtom( &Parent.GetAtom( res[i].GetId() ) );
        }
        this->ClassName = res.ClassName;
        this->Number = res.Number;
        this->Alias = res.Alias;
        return *this;
      }
  };
protected:
  TActionQList Actions;
  TPtrList<TResidue> Residues;
  TResidue MainResidue;
  // in INS file is EQUV command
  smatd_list UsedSymm;
  TSRestraintList rDfix,  // restrained distances (DFIX)
                  rAfix,  // restrained angles (DANG)
                  rDsim,  // similar distances (SADI)
                  rVfix,  // restrained atomic volume (CHIV)
                  rPfix,  // planar groups (FLAT)
                  rRBnd,  // rigid bond restraints (DELU)
                  rUsim,  // similar Uij (SIMU)
                  rUiso,  // Uij components approximate to isotropic behavior (ISOR)
                  rEADP;  // equivalent adp, constraint
                 
  TSameGroupList  rSAME;
  TAfixGroups AfixGroups;
  TExyzGroups ExyzGroups;
public:

  TAsymmUnit(TLattice *L, TAtomsInfo *AI);
  virtual ~TAsymmUnit();

  inline TLattice& GetLattice()       const {  return *Lattice;  }
  inline TEVPointD&  Axes()                 {  return FAxes;  }
  inline TEVPointD&  Angles()               {  return FAngles;  }
  inline const vec3d& GetRAxes()   const {  return RAxes;  }
  inline const vec3d& GetRAngles() const {  return RAngles;  }
  double CalcCellVolume() const;
  // estimates Z=Z'*sg.multiplicity according to 18.6A rule
  double EstimateZ(int atomCount) const;
  DefPropP(short, Z)
  DefPropP(short, Latt)
  inline olxstr& CellSymmetry()            {  return FCellSymmetry ;  }

  const mat3d& GetCellToCartesian() const {  return Cell2Cartesian; }
  const mat3d& GetCartesianToCell() const {  return Cartesian2Cell; }
  const mat3d& GetHklToCartesian()  const {  return Hkl2Cartesian; }
  template <class VC>
  inline void CellToCartesian(const VC& cell, VC& crt) const  {
    crt[0] = cell[0]*Cell2Cartesian[0][0] + cell[1]*Cell2Cartesian[1][0] + cell[2]*Cell2Cartesian[2][0];
    crt[1] = cell[1]*Cell2Cartesian[1][1] + cell[2]*Cell2Cartesian[2][1];
    crt[2] = cell[2]*Cell2Cartesian[2][2];
    //Cartesian = Cell * Cell2Cartesian;
  }
  template <class VC>
  inline void CellToCartesian(VC& crt) const {
    crt[0] = crt[0]*Cell2Cartesian[0][0] + crt[1]*Cell2Cartesian[1][0] + crt[2]*Cell2Cartesian[2][0];
    crt[1] = crt[1]*Cell2Cartesian[1][1] + crt[2]*Cell2Cartesian[2][1];
    crt[2] = crt[2]*Cell2Cartesian[2][2];
    //crt *= Cell2Cartesian;
  }
  template <class VC>
  inline void CartesianToCell(VC& cll)      const {
    cll[0] = cll[0]*Cartesian2Cell[0][0] + cll[1]*Cartesian2Cell[1][0] + cll[2]*Cartesian2Cell[2][0];
    cll[1] = cll[1]*Cartesian2Cell[1][1] + cll[2]*Cartesian2Cell[2][1];
    cll[2] = cll[2]*Cartesian2Cell[2][2];
    //cll *= Cartesian2Cell;
  }

  // J App Cryst 2002, 35, 477-480
  void UcifToUcart(evecd& v);
  void UcartToUcif(evecd& v);

  void Assign( const TAsymmUnit& C);
  void ChangeSpaceGroup(const class TSpaceGroup& sg);
  // executed from the above function, Data is the new space group
  TActionQueue* OnSGChange;
  // initialises transofrmation matrices, called after axis and angles initialised
  void InitMatrices();
  // initialises data such as Q-peak heights, called after all atoms initialised :)
  void InitData();
  
  void Clear();
  //creates a new residue
  TResidue& NewResidue(const olxstr& RClass, int number, const olxstr& alias = EmptyString);
  inline int ResidueCount()                 const {  return Residues.Count();  }
  inline TResidue& GetResidue(int i)        const { return (i==-1) ? const_cast<TAsymmUnit*>(this)->MainResidue : *Residues[i];  }
  inline TResidue* NextResidue(const TResidue& r) const {  return ((r.GetId()-1) == Residues.Count()) ? NULL : Residues[r.GetId()+1];  }
  inline TResidue* PrevResidue(const TResidue& r) const {  
    return (r.GetId() == -1) ? NULL : ((r.GetId() == 0) ? &const_cast<TAsymmUnit*>(this)->MainResidue : Residues[r.GetId()-1]);  
  }
  void ClearResidues(bool moveToMain);
  void AssignResidues(const TAsymmUnit& au);
  // if a number is provided, seraches by Number otherwise - by ClassName
  void FindResidues(const olxstr& resi, TPtrList<TResidue>& list);
  //creates a new atom and puts it into the list
  TCAtom& NewAtom(TResidue* resi = NULL);
  //creates a new atom and puts it into the list
  TCAtom& NewCentroid(const vec3d &CCenter);
  //returns an atom by label; if the label is not unique, returns the first found
  TCAtom* FindCAtom(const olxstr &Label, TResidue* resi = NULL) const;
  //returns an atom by LoaderId
  TCAtom* FindCAtomByLoaderId(int li) const;
  // searches for sfac data, first by atom label, then by
  //TVectorD* FindSfacData(const TCAtom& atom) const;
  void AddNewSfac(const olxstr& label,
                  double a1, double a2, double a3, double a4,
                  double b1, double b2, double b3, double b4,
                  double c);
  inline int SfacCount()  const  {  return SfacData.Count();  }
  inline const olxstr& GetSfacLabel(size_t index) const  {
    return SfacData.GetComparable(index);
  }
  inline TLibScatterer& GetSfacData(size_t index) const  {
    return *SfacData.GetObject(index);
  }
  inline TLibScatterer* FindSfacData(const olxstr& label) const  {
    return SfacData[label];
  }

  void DelAtom( size_t index );
  /* deletes the atom in the list and assigns corresponding of the list to NULL
   is used to optimize deletion procedure (for example in the Uniq procedure)
   use Pack function to remove NULL pointers after several iterations
  */
  void NullAtom( size_t index );

  /* removes NULL atoms (from the call to Null Atom) */
  void PackAtoms();
  inline TCAtom& GetAtom(size_t i)     const {  return *CAtoms[i];  }
  inline int AtomCount()               const { return CAtoms.Count();};
  inline int CentroidCount()           const {  return Centroids.Count(); }
  inline TCAtom& GetCentroid(size_t i) const {  return *Centroids[i]; }
  void InitAtomIds(); // initialises atom ids if any were added or removed

  inline int MatrixCount()                   const {  return Matrices.Count();  }
  inline const smatd& GetMatrix(size_t i) const {  return Matrices[i];  }
  void ClearMatrices()                             {  Matrices.Clear();  }
  void AddMatrix(const smatd& a);

  inline int EllpCount()               const {  return Ellipsoids.Count(); }
  inline TEllipsoid& GetEllp(size_t i) const {  return *Ellipsoids[i]; }
  void NullEllp(size_t i);
  void ClearEllps()  {
    for( int i=0; i < Ellipsoids.Count(); i++ )
      delete Ellipsoids[i];
    for( int i=0; i < CAtoms.Count(); i++ )
      CAtoms[i]->AssignEllps(NULL);
    Ellipsoids.Clear();
  }
  void PackEllps();
  // Q - six values representing quadratic form of a thermal ellipsoid
  TEllipsoid& NewEllp(const evecd& Q);
  TEllipsoid& NewEllp();  // initialisation performed manually !

  vec3d GetOCenter(bool IncludeQ, bool IncludeH) const;
  /* returns summarised formula of the asymmetric unit, use MutiplyZ to multiply the
     content by Z
  */
  olxstr SummFormula(const olxstr &Sep, bool MultiplyZ=true) const;
  /* creates a SFAC/UNIT strings; BasicAtoms is filled with atom labels constituting the
   asymmertic unit; see example in TIns::SaveToFile
  */
  void SummFormula(TStrPObjList<olxstr,TBasicAtomInfo*>& BasicAtoms, olxstr &Elements, olxstr &Numbers, bool MultiplyZ=true) const;
  /* returns molecular weight of the asymmetric unit */
  double MolWeight() const;
  size_t CountElements(const olxstr& Symbol) const;
  TAtomsInfo* GetAtomsInfo()  const {  return AtomsInfo; }

  // sorts the content of the asymmetric unit or the list if provided
  void Sort(TCAtomPList* list = NULL);

  olxstr CheckLabel(const TCAtom* ca, const olxstr &Label, char a='0', char b='a', char c='a') const;

  bool DoesContainEquivalents() const  {  return ContainsEquivalents; }
  void SetContainsEquivalents(bool v)  {  ContainsEquivalents = v; }

  inline double GetMaxQPeak()    const {  return MaxQPeak;  }
  inline double GetMinQPeak()    const {  return MinQPeak;  }

  const smatd& AddUsedSymm(const smatd& matr);
  void RemUsedSymm(const smatd& matr);
  inline int UsedSymmCount()     const {  return UsedSymm.Count();  }
  inline const smatd& GetUsedSymm(size_t ind)  {  return UsedSymm[ind];  }
  inline int UsedSymmIndex(const smatd& matr)  const {  return UsedSymm.IndexOf(matr);  }
  inline void ClearUsedSymm()          {  UsedSymm.Clear();  }


  /*this is to be called by TLattice when compaq or other procedurs, changing
    coordinates of atoms are called. This is to handle restraints in a correct
    way
  */
  void OnCAtomCrdChange( TCAtom* ca, const smatd& matr );
  // clears restraints and constraints
  void ClearRestraints();
  // DFIX equivalent
  inline TSRestraintList& RestrainedDistances() {  return rDfix;  }
  // if there is a restrained distance betweem the atoms - returns it or -1 otherwise
  double FindRestrainedDistance(const TCAtom& a1, const TCAtom& a2);
  // DANG equivalent
  inline TSRestraintList& RestrainedAngles()    {  return rAfix;  }
  // SADI equivalent
  inline TSRestraintList& SimilarDistances()    {  return rDsim;  }
  // CHIV equivalent
  inline TSRestraintList& RestrainedVolumes()   {  return rVfix;  }
  // FLAT equivalent
  inline TSRestraintList& RestrainedPlanarity() {  return rPfix;  }
  // DELU
  inline TSRestraintList& RigidBonds()          {  return rRBnd;  }
  // SIMU
  inline TSRestraintList& SimilarU()            {  return rUsim;  }
  // ISOR
  inline TSRestraintList& RestranedUaAsUi()     {  return rUiso;  }
  // EADP
  inline TSRestraintList& EquivalentU()         {  return rEADP;  }
  // SAME
  inline TSameGroupList& SimilarFragments()     {  return rSAME;  }
  // EXYZ
  inline TExyzGroups& SharedSites()             {  return ExyzGroups;  }
  inline TAfixGroups& GetAfixGroups()           {  return AfixGroups;  }

  // returns next available part istruction in atoms
  int GetMaxPart() const;
  /* returns next available LoaderId. Can be used to add enw atoms
   to the asymmetri unit and the ins file respectevely
  */
  int GetMaxLoaderId() const;

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);

  void LibNewAtom(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomCount(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomCrd(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomU(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomUiso(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomOccu(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomName(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomType(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomAfix(const TStrObjList& Params, TMacroError& E);
  void LibGetPeak(const TStrObjList& Params, TMacroError& E);
  void LibGetCell(const TStrObjList& Params, TMacroError& E);
  void LibGetVolume(const TStrObjList& Params, TMacroError& E);
  void LibGetCellVolume(const TStrObjList& Params, TMacroError& E);
  void LibSetAtomCrd(const TStrObjList& Params, TMacroError& E);
  void LibSetAtomLabel(const TStrObjList& Params, TMacroError& E);
  // retruns label if +-num is used
  void LibIsAtomDeleted(const TStrObjList& Params, TMacroError& E);
  void LibIsPeak(const TStrObjList& Params, TMacroError& E);
  void LibGetAtomLabel(const TStrObjList& Params, TMacroError& E);
  void LibSetAtomU(const TStrObjList& Params, TMacroError& E);
  void LibSetAtomOccu(const TStrObjList& Params, TMacroError& E);
  void LibGetSymm(const TStrObjList& Params, TMacroError& E);
  void LibGetZ(const TStrObjList& Params, TMacroError& E);
  void LibSetZ(const TStrObjList& Params, TMacroError& E);
  void LibGetZprime(const TStrObjList& Params, TMacroError& E);
  void LibSetZprime(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};

EndXlibNamespace()
#endif

