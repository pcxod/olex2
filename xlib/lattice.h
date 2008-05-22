#ifndef latticekH
#define latticekH

#include "xbase.h"
#include "elist.h"
#include "vpoint.h"
#include "ematrix.h"
#include "atominfo.h"
#include "catom.h"
#include "satom.h"
#include "sbond.h"
#include "splane.h"
#include "network.h"

#include "macroerror.h"

BeginXlibNamespace()

class TLattice: public IEObject  {
private:
  TNetwork* Network;  // for internal use only
private:
  int GenerateMatrices(const TVPointD& VFrom, const TVPointD& VTo,
        const TVPointD& MFrom, const TVPointD& MTo);
  // generates matrices in volume {VFrom, VTo} and leaves only matrices, which
  //transform the center of gravity of the asymmertic unit within {MFrom, MTo} volume
  // useually VFrom = Round(MFrom), VTo = Round(VFrom)
  TMatrixDPList Matrices;    // list of all matrices
  TSAtomPList  Atoms;      // list of all atoms
  TSBondPList  Bonds;      // list of all nework nodes; some of them are equal to Atoms
  TNetPList    Fragments;
  TSPlanePList Planes;
protected:
  bool Generated;
  TAtomsInfo* AtomsInfo;  // a pointer only
  void Generate(TCAtomPList* Template, bool ClearCont, bool IncludeQ);  // generates atoms using current matrices list
  void GenerateAtoms( const TSAtomPList& atoms, TSAtomPList& result, const TMatrixDPList& matrices);
  void ClearFragments();
  void ClearAtoms();
  void ClearMatrices();
  void ClearBonds();
  void ClearPlanes();

  void AddSBond( TSBond *B );
  void AddSAtom( TSAtom *A );
  void AddSPlane( TSPlane *P );

  class TUnitCell*  UnitCell;
  class TAsymmUnit* AsymmUnit;
  float    Delta, DeltaI;

  void DoGrow(const TSAtomPList& Atoms, bool GrowShell, TCAtomPList* Template);
  void ListAsymmUnit(TSAtomPList& res, TCAtomPList* Template, bool IncludeQ);
  void InitBody();
  // fills the list with atoms of asymmertic unit
  // the atoms have to be deleted with a call to delete
  // if Template is specified, the CAtoms are taken from there instead of AsymmUnit
  void Disassemble();
  void RestoreCoordinates();
public:
  TLattice(TAtomsInfo *Info);
  virtual ~TLattice();

  // this is does not have any usefull data - just for functions call!!!
  inline TNetwork& GetNetwork()  const  {  return *Network; }

  void Clear(bool ClearUnitCell);
  void Uniq(bool removeSymmEquivalents = false);
  void Init();
  // generates atoms within specified volume
  void Generate(const TVPointD& MFrom, const TVPointD& MTo, TCAtomPList* Template,
    bool ClearCont, bool IncludeQ);
  // checks if the data alreade have been generated
  inline bool IsGenerated() const  {  return Generated;  }

  // generates matrices so that the center of asymmetric unit is inisde the specified volume
  int GenerateMatrices(TMatrixDPList& Result, const TVPointD& VFrom, const TVPointD& VTo,
        const TVPointD& MFrom, const TVPointD& MTo);

  void GrowFragments(bool GrowShells, TCAtomPList* Template);
  void GrowAtoms(const TSAtomPList& Atoms, bool GrowShells, TCAtomPList* Template);
  void GrowAtoms(const TSAtomPList& Atoms, const TMatrixDList& matrices);
  void GrowAtom(TSAtom& A, bool GrowShells, TCAtomPList* Template);
  /* grow a fragment using particular matrix */
  void GrowAtom(int FragId, const TMatrixD& transform);
  void Grow(const TMatrixD& transform);
  void GenerateWholeContent(TCAtomPList* Template); // generates content using current matrices
  bool IsExpandable(TSAtom& A) const;

  inline int FragmentCount()                const {  return Fragments.Count(); }
  inline TNetwork& GetFragment(int i)       const {  return *Fragments[i];  }

  inline int MatrixCount()                  const {  return Matrices.Count();  }
  const TMatrixD& GetMatrix(int i)          const {  return *Matrices[i];  }

  inline int AtomCount()                    const {  return Atoms.Count();  }
  inline TSAtom& GetAtom(int i)             const {  return *Atoms[i];  }
  TSAtom* FindSAtom(const olxstr &Label) const;

  inline int BondCount()                    const {  return Bonds.Count();  }
  inline TSBond& GetBond(int i)             const {  return *Bonds[i]; }

  inline int PlaneCount()                   const {  return Planes.Count(); }
  inline TSPlane& GetPlane(int i)           const {  return *Planes[i];  }
  TSPlane* NewPlane(const TSAtomPList& Atoms, int weightExtent=0);

  TSPlane* TmpPlane(const TSAtomPList& Atoms, int weightExtent=0); //the plane must be deleted by the caller !
  TSAtom* NewCentroid(const TSAtomPList& Atoms);
  TSAtom* NewAtom(const TVPointD& center);

  void SetAnis( const TCAtomPList& atoms, bool anis );

  inline TAtomsInfo& GetAtomsInfo()         const {  return *AtomsInfo; }
  inline TUnitCell& GetUnitCell()           const {  return *UnitCell; }
  inline TAsymmUnit& GetAsymmUnit()         const {  return *AsymmUnit; }
  void UpdateAsymmUnit();

  void MoveFragment(const TVPointD& to, TSAtom& fragAtom);
  void MoveFragment(TSAtom& to, TSAtom& fragAtom);
  void MoveToCenter();
  void MoveFragmentG(const TVPointD& to, TSAtom& fragAtom);
  void MoveFragmentG(TSAtom& to, TSAtom& fragAtom);
  void MoveToCenterG();
  void Compaq();
  void CompaqAll();
  void TransformFragments(const TSAtomPList& fragAtoms, const TMatrixD& transform);

  // beware - only pointers are compared
  inline bool operator == (const TLattice& l)  const  {  return this == &l;  }
  inline bool operator == (const TLattice* l)  const  {  return this == l;  }
  inline bool operator != (const TLattice& l)  const  {  return this != &l;  }
  inline bool operator != (const TLattice* l)  const  {  return this != l;  }

protected:
  // removes H2O and R3N from the list of potential hydrogen bond forming atoms
  void RemoveNonHBonding(class TAtomEnvi& envi);
//  void AnalyseHBonding(class TAtomEnvi& Envi);
  bool _AnalyseAtomHAdd(class AConstraintGenerator& cg, TSAtom& atom, TSAtomPList& ProcessingAtoms);
public:
  void AnalyseHAdd(class AConstraintGenerator& cg, const TSAtomPList& atoms);

  inline float GetDelta() const  {  return Delta; }
  inline void SetDelta(float v)  {  Delta = v; }
  inline float GetDeltaI() const {  return DeltaI; }
  inline void SetDeltaI(float v) {  DeltaI = v; }

  void LibGetFragmentCount(const TStrObjList& Params, TMacroError& E);
  void LibGetFragmentAtoms(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};

EndXlibNamespace()
#endif

