#ifndef latticekH
#define latticekH

#include "xbase.h"
#include "elist.h"
#include "symmat.h"
#include "atominfo.h"
#include "catom.h"
#include "satom.h"
#include "sbond.h"
#include "splane.h"
#include "network.h"

#include "macroerror.h"
#include "library.h"
#include "bitarray.h"

BeginXlibNamespace()

class TLattice: public IEObject  {
private:
  TNetwork* Network;  // for internal use only
private:
  /* generates matrices in volume {VFrom, VTo} and leaves only matrices, which
  transform the center of gravity of the asymmertic unit within {MFrom, MTo} volume
  useually VFrom = Round(MFrom), VTo = Round(VFrom)
  */
  int GenerateMatrices(const vec3d& VFrom, const vec3d& VTo,
        const vec3d& MFrom, const vec3d& MTo);
  // generates matrices from -4 to 4 which generate aunit within the rad sphere
  int GenerateMatrices(smatd_plist& Result, const vec3d& center, double rad);
  smatd_plist Matrices;    // list of all matrices
  TSAtomPList  Atoms;      // list of all atoms
  TSBondPList  Bonds;      // list of all nework nodes; some of them are equal to Atoms
  TNetPList    Fragments;
  TSPlanePList Planes;
protected:
  TActionQList Actions;
  bool Generated;
  TAtomsInfo& AtomsInfo;  // reference to TAtomsInfo::Instance
  void Generate(TCAtomPList* Template, bool ClearCont, bool IncludeQ);  // generates atoms using current matrices list
  void GenerateAtoms( const TSAtomPList& atoms, TSAtomPList& result, const smatd_plist& matrices);
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
  double Delta, DeltaI;
  // the mask to decide which atoms to be used in the connectivity
  TEBitArray AtomMask;
  // if Template is specified, the CAtoms are taken from there instead of AsymmUnit
  void DoGrow(const TSAtomPList& Atoms, bool GrowShell, TCAtomPList* Template);
  // fills the list with atoms of asymmertic unit
  // the atoms have to be deleted with a call to delete
  void ListAsymmUnit(TSAtomPList& res, TCAtomPList* Template, bool IncludeQ);
  void InitBody();
  void Disassemble();
  void RestoreCoordinates();
public:
  TLattice();
  virtual ~TLattice();

  TActionQueue* OnStructureGrow, *OnStructureUniq, *OnDisassemble;

  // this is does not have any usefull data - just for functions call!!!
  inline TNetwork& GetNetwork()  const  {  return *Network; }

  void Clear(bool ClearUnitCell);
  void Uniq(bool removeSymmEquivalents = false);
  // to be called after the mask is set
  void UpdateConnectivity(TEBitArray& amask) {  
    AtomMask = amask;
    Disassemble();  
  }
  void Init();
  void SetAtomMask(TEBitArray& ba)  {  AtomMask = ba;  }
  // generates atoms within specified volume
  void Generate(const vec3d& MFrom, const vec3d& MTo, TCAtomPList* Template,
    bool ClearCont, bool IncludeQ);
  // generates atoms within sphere volume at center
  void Generate(const vec3d& center, double rad, TCAtomPList* Template,
    bool ClearCont, bool IncludeQ);
  // checks if the data alreade have been generated
  inline bool IsGenerated() const  {  return Generated;  }

  // generates matrices so that the center of asymmetric unit is inisde the specified volume
  int GenerateMatrices(smatd_plist& Result, const vec3d& VFrom, const vec3d& VTo,
        const vec3d& MFrom, const vec3d& MTo);

  void GrowFragments(bool GrowShells, TCAtomPList* Template);
  void GrowAtoms(const TSAtomPList& Atoms, bool GrowShells, TCAtomPList* Template);
  void GrowAtoms(const TSAtomPList& Atoms, const smatd_list& matrices);
  void GrowAtom(TSAtom& A, bool GrowShells, TCAtomPList* Template);
  /* grow a fragment using particular matrix */
  void GrowAtom(int FragId, const smatd& transform);
  void Grow(const smatd& transform);
  void GenerateWholeContent(TCAtomPList* Template); // generates content using current matrices
  bool IsExpandable(TSAtom& A) const;

  inline int FragmentCount()                const {  return Fragments.Count(); }
  inline TNetwork& GetFragment(int i)       const {  return *Fragments[i];  }

  inline int MatrixCount()                  const {  return Matrices.Count();  }
  const smatd& GetMatrix(int i)             const {  return *Matrices[i];  }
  smatd& GetMatrix(int i)                         {  return *Matrices[i];  }

  inline int AtomCount()                    const {  return Atoms.Count();  }
  inline TSAtom& GetAtom(int i)             const {  return *Atoms[i];  }
  TSAtom* FindSAtom(const olxstr &Label) const;
  TSAtom* FindSAtom(const TCAtom& ca) const;

  inline int BondCount()                    const {  return Bonds.Count();  }
  inline TSBond& GetBond(int i)             const {  return *Bonds[i]; }

  inline int PlaneCount()                   const {  return Planes.Count(); }
  inline TSPlane& GetPlane(int i)           const {  return *Planes[i];  }
  TSPlane* NewPlane(const TSAtomPList& Atoms, int weightExtent=0);

  TSPlane* TmpPlane(const TSAtomPList& Atoms, int weightExtent=0); //the plane must be deleted by the caller !
  TSAtom* NewCentroid(const TSAtomPList& Atoms);
  TSAtom* NewAtom(const vec3d& center);

  void SetAnis( const TCAtomPList& atoms, bool anis );

  inline TUnitCell& GetUnitCell()           const {  return *UnitCell; }
  inline TAsymmUnit& GetAsymmUnit()         const {  return *AsymmUnit; }
  void UpdateAsymmUnit();

  void MoveFragment(const vec3d& to, TSAtom& fragAtom);
  void MoveFragment(TSAtom& to, TSAtom& fragAtom);
  void MoveToCenter();
  void MoveFragmentG(const vec3d& to, TSAtom& fragAtom);
  void MoveFragmentG(TSAtom& to, TSAtom& fragAtom);
  void MoveToCenterG();
  void Compaq();
  void CompaqAll();
  void TransformFragments(const TSAtomPList& fragAtoms, const smatd& transform);

  // beware - only pointers are compared
  inline bool operator == (const TLattice& l)  const  {  return this == &l;  }
  inline bool operator == (const TLattice* l)  const  {  return this == l;  }
  inline bool operator != (const TLattice& l)  const  {  return this != &l;  }
  inline bool operator != (const TLattice* l)  const  {  return this != l;  }

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);

protected:
  // removes H2O and R3N from the list of potential hydrogen bond forming atoms
  void RemoveNonHBonding(class TAtomEnvi& envi);
//  void AnalyseHBonding(class TAtomEnvi& Envi);
  bool _AnalyseAtomHAdd(class AConstraintGenerator& cg, TSAtom& atom, TSAtomPList& ProcessingAtoms, 
    int part = -1, TCAtomPList* generated = NULL);
  void _ProcessRingHAdd(AConstraintGenerator& cg, const TPtrList<TBasicAtomInfo>& rcont);
public:
  void AnalyseHAdd(class AConstraintGenerator& cg, const TSAtomPList& atoms);

  DefPropP(double, Delta)
  DefPropP(double, DeltaI)

  void LibGetFragmentCount(const TStrObjList& Params, TMacroError& E);
  void LibGetFragmentAtoms(const TStrObjList& Params, TMacroError& E);
  TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};

EndXlibNamespace()
#endif

