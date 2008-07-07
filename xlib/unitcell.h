#ifndef unitcellH
#define unitcellH

#include "xbase.h"
#include "elist.h"
#include "ematrix.h"
#include "vpoint.h"
#include "envilist.h"
#include "ellipsoid.h"
#include "estrbuffer.h"
#include "arrays.h"

BeginXlibNamespace()

class TUnitCell: public IEObject  {
  TNetwork*  Network;  // for internal use only
private:
  TMatrixDList Matrices;  // list of unique matrices; FMatrices + centering
  TArrayList<TEllpPList> Ellipsoids;  // i - atoms index, j - matrix index 
  class TLattice*  Lattice;    // parent lattice
public:
  TUnitCell(TLattice* L);
  virtual ~TUnitCell();

  double CalcVolume()  const;
  inline TLattice& GetLattice()    const {  return *Lattice;  }
  void Clear();

  inline int MatrixCount()                const {  return Matrices.Count();  }
  inline const TMatrixD& GetMatrix(int i) const {  return Matrices[i];  }

  const TEllipsoid& GetEllipsoid(int MatrixId, int AUId) const;
  void AddEllipsoid(); // adds a new row to ellipsoids, intialised with NULLs
  void ClearEllipsoids();
  // get an ellipsoid for an atom by asymmetric unit Id and a matrix associated with it

  void InitMatrices();

  // returns the number of symmetrical equivalents present within the atoms
  // in this case Msg will be initialised with a message, set remove to false
  //to leave equivalents, otherwise they will be removed from the content
  int FindSymmEq(TEStrBuffer &Msg, double tol, bool Initialize, bool remove, bool markDeleted) const;

  // the funciton searches a matrix which moves "atom" to "to" so that the
  // distance between them is shortest and return the matrix, which if not NULL
  // has to be deleted with delete
  TMatrixD* GetClosest(const class TCAtom& to, const TCAtom& atom, bool ConsiderOriginal) const;

  TMatrixD* GetClosest(const TVPointD& to, const TVPointD& from, bool ConsiderOriginal) const;

  TMatrixDList* GetInRange(const TVPointD& to, const TVPointD& from, double R, bool IncludeI) const;

  // returns a list of all closest (in all directions) matrices
  TMatrixDList* GetInRangeEx(const TVPointD& to, const TVPointD& from, float R, bool IncludeI, const TMatrixDList& ToSkip) const;
  // returns a list of all biding matrices
  TMatrixDList* GetBinding(const TCAtom& toA, const TCAtom& fromA,
    const TVPointD& to, const TVPointD& from, bool IncludeI, bool IncludeHBonds) const;

  double FindClosestDistance(const class TCAtom& to, const TCAtom& atom) const;
  // finds all atoms and their ccordinates inside the sphere of radius R
  void FindInRange(const TCAtom& atom, double R, 
    TArrayList< AnAssociation2<TCAtom const*, TVPointD> >& res) const;
  // finds all atoms (+symm attached) and Q-peaks, if specfied
  void GetAtomEnviList(TSAtom& atom, TAtomEnvi& envi, bool IncludeQ = false) const;
  // finds only q-peaks in the environment of specified atom
  void GetAtomQEnviList(TSAtom& atom, TAtomEnvi& envi);
  /* finds "pivoting" atoms for possible h-bonds, considering O and N only with
    distances within 2.9A and angles 89-130 deg
  */
  void GetAtomPossibleHBonds(const TAtomEnvi& atom, TAtomEnvi& envi);

  /* returns a multiplier for the number of matrices in the asymmetruc unit for
    specified LATT instruction
  */
  static int GetMatrixMultiplier(short Latt);
  /* This function creates a map of the unit cell with provided partioning.
  It uses Van-der-Waals atomic radii and adds delta to it. The grid points
  belonging to atoms have value 'value', the others - '0'
  Returns the number of grid points occupied by the structure to structurePoinst if no NULL
  */
  void BuildStructureMap( TArray3D<short>& map, double delta, short value, long* structurePoints );
  void GenereteAtomCoordinates(TTypeList< AnAssociation2<TVPointD,TCAtom*> >& list, bool IncludeH) const;
  // returns true if the atom overlaps with another atom in the unit cell
  bool DoesOverlap(const TCAtom& ca, double R) const;
  // finds an atom at given position
  TCAtom* FindCAtom(const TVPointD& center) const;
  /* finds an atom within delta of the given position, ! the position is updated to the closest match found */
  TCAtom* FindOverlappingAtom(TVPointD& position, double delta) const;
protected:
  class TSearchSymmEqTask  {
    TPtrList<TCAtom>& Atoms;
    const TMatrixDList& Matrices;
    TStrList& Report;
    bool Initialise;
    double tolerance;
    TAsymmUnit* AU;
    TLattice* Latt;
  public:
    TSearchSymmEqTask(TPtrList<TCAtom>& atoms, const TMatrixDList& matrices, TStrList& report,
                      double tol, bool initialise);
    void Run(long ind);
    TSearchSymmEqTask* Replicate() const  {
      return new TSearchSymmEqTask(Atoms, Matrices, Report, tolerance, Initialise);
    }
  };
};

EndXlibNamespace()
#endif

