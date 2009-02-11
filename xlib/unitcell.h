#ifndef unitcellH
#define unitcellH

#include "xbase.h"
#include "elist.h"
#include "symmat.h"
#include "envilist.h"
#include "ellipsoid.h"
#include "estrbuffer.h"
#include "arrays.h"
#include "lattice.h"
#include "asymmunit.h"
// on Linux its is defined as something...
#undef QLength

BeginXlibNamespace()

class TUnitCell: public IEObject  {
  TNetwork*  Network;  // for internal use only
private:
  smatd_list Matrices;  // list of unique matrices; FMatrices + centering
  TArrayList<TEllpPList> Ellipsoids;  // i - atoms index, j - matrix index 
  class TLattice*  Lattice;    // parent lattice
public:
  TUnitCell(TLattice* L);
  virtual ~TUnitCell();

  double CalcVolume()  const;
  TEValue<double> CalcVolumeEx()  const;
  inline TLattice& GetLattice()    const {  return *Lattice;  }
  void Clear();

  inline int MatrixCount()             const {  return Matrices.Count();  }
  // the identity matrix is always the first
  inline const smatd& GetMatrix(int i) const {  return Matrices[i];  }

  const TEllipsoid& GetEllipsoid(int MatrixId, int AUId) const;
  void AddEllipsoid(); // adds a new row to ellipsoids, intialised with NULLs
  void ClearEllipsoids();
  void UpdateEllipsoids();
  // get an ellipsoid for an atom by asymmetric unit Id and a matrix associated with it

  void InitMatrices();

  // returns the number of symmetrical equivalents present within the atoms
  // in this case Msg will be initialised with a message, set remove to false
  //to leave equivalents, otherwise they will be removed from the content
  int FindSymmEq(TEStrBuffer &Msg, double tol, bool Initialize, bool remove, bool markDeleted) const;

  // the funciton searches a matrix which moves "atom" to "to" so that the
  // distance between them is shortest and return the matrix, which if not NULL
  // has to be deleted with delete
  smatd* GetClosest(const class TCAtom& to, const TCAtom& atom, bool ConsiderOriginal, double* dist=NULL) const;
  smatd* GetClosest(const vec3d& to, const vec3d& from, bool ConsiderOriginal, double* dist=NULL) const;

  smatd_list* GetInRange(const vec3d& to, const vec3d& from, double R, bool IncludeI) const;

  // returns a list of all closest (in all directions) matrices
  smatd_list* GetInRangeEx(const vec3d& to, const vec3d& from, float R, bool IncludeI, const smatd_list& ToSkip) const;
  // returns a list of all biding matrices
  smatd_list* GetBinding(const TCAtom& toA, const TCAtom& fromA,
    const vec3d& to, const vec3d& from, bool IncludeI, bool IncludeHBonds) const;

  double FindClosestDistance(const class TCAtom& to, const TCAtom& atom) const;
  // finds all atoms and their ccordinates inside the sphere of radius R
  void FindInRange(const vec3d& center, double R, 
    TArrayList< AnAssociation2<TCAtom const*, vec3d> >& res) const;
  // finds all atoms and their ccordinates inside the sphere of radius R
  void FindInRange(const vec3d& center, double R, 
    TArrayList< AnAssociation2<TCAtom const*, smatd> >& res) const;
  /* finds all atoms (+symm attached) and Q-peaks, if specfied; if part is not -1, part 0 and 
  the specified part are only placed
  */
  void GetAtomEnviList(TSAtom& atom, TAtomEnvi& envi, bool IncludeQ = false, int part=-1) const;
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
  It uses Van-der-Waals atomic radii by defualt and adds delta to it. The grid points
  belonging to atoms have value 'value', the others - '0'
  Returns the number of grid points occupied by the structure to structurePoinst if not NULL.
  If _template is provided - only these atoms are used int the calculation
  */
  void BuildStructureMap( TArray3D<short>& map, double delta, short value, 
    size_t* structurePoints, TPSTypeList<TBasicAtomInfo*, double>* radii, const TCAtomPList* _template = NULL );
protected:
  // helper function, association should be AnAssociation2+<vec3d,TCAtom*,+>
  template <class Association> 
    static int AtomsSortByDistance(const Association& A1, const Association& A2)  {
      const double d = A1.GetA().QLength() - A2.GetA().QLength();
      return (d < 0 ? -1 : ((d > 0 ) ? 1 : 0));
    }
public:
  // creates the array of matrices for a given aunit and lattice tyoe
  static void GenerateMatrices(smatd_list& out, const TAsymmUnit& au, short lat);
  // association should be AnAssociation2+<vec3d,TCAtom*,+>, generates all atoms of the aunit
  template <class Association> void GenereteAtomCoordinates(TTypeList<Association>& list, bool IncludeH, 
                    const TCAtomPList* _template = NULL) const  {
    TCAtomPList& atoms = (_template == NULL ? *(new TCAtomPList) : *const_cast<TCAtomPList*>(_template));
    if( _template == NULL )  {
      list.SetCapacity(Lattice->GetAsymmUnit().AtomCount()*Matrices.Count());
      for( int i=0; i < Lattice->GetAsymmUnit().AtomCount(); i++ )  {
        TCAtom& ca = Lattice->GetAsymmUnit().GetAtom(i);
        if( ca.GetAtomInfo() == iQPeakIndex || ca.IsDeleted() )  continue;
        if( !IncludeH && ca.GetAtomInfo() == iHydrogenIndex )    continue;
        atoms.Add(&ca);
      }
    }
    list.SetCapacity(atoms.Count()*Matrices.Count());
    const int atom_cnt = atoms.Count();
    for( int i=0; i < atom_cnt; i++ )  {
      TCAtom* ca = atoms[i];
      if( ca->GetAtomInfo() == iQPeakIndex || ca->IsDeleted() )  continue;
      for( int j=0; j < Matrices.Count(); j++ )  {
        vec3d center = Matrices[j] * ca->ccrd();
        for( int k=0; k < 3; k++ )  {
          while( center[k] < 0 )  center[k] += 1;
          while( center[k] > 1 )  center[k] -= 1;
        }
        list.AddNew(center, ca);
      }
    }
    // create a list of unique atoms
    float* distances = new float[ list.Count()+1 ];
    list.QuickSorter.SortSF( list, AtomsSortByDistance);
    const int lc = list.Count();
    for( int i=0; i < lc; i++ )
      distances[i] = (float)list[i].GetA().QLength();

    for( int i=0; i < lc; i++ )  {
      if( list.IsNull(i) )  continue;
      for( int j=i+1; j < lc; j++ )  {
        if( list.IsNull(j) )  continue;
        if( (distances[j] - distances[i]) > 0.1 )  break;
        double d = list[i].GetA().QDistanceTo( list[j].GetA() );
        if( d < 0.00001 )  {
          list.NullItem(j);
          continue;
        }
      }
    }
    delete [] distances;
    list.Pack();
    if( _template == NULL )
      delete &atoms;
  }
  /* expands atom coordinates with +/-1 if one of the fractional coordinates is less the lim or greater than 1-lim
  This function is used in the BuildStructure map function to take ito account atoms which are near the cell sides
  */
  template <class Association> void ExpandAtomCoordinates(TTypeList<Association>& list, const double lim)  {
    list.SetCapacity( list.Count()*7 );
    const double c_min = lim, c_max = 1.0 -lim;
    const int all_ac = list.Count();
    for( int i=0; i < all_ac; i++ )  {
      const vec3d& v = list[i].GetA();
      const double xi = v[0] < c_min ? 1 : (v[0] > c_max ? -1 : 0);
      const double yi = v[1] < c_min ? 1 : (v[1] > c_max ? -1 : 0);
      const double zi = v[2] < c_min ? 1 : (v[2] > c_max ? -1 : 0);
      if( xi != 0 )  {
        list.AddNew(vec3d(v[0]+xi, v[1], v[2]), list[i].B());
        if( yi != 0 )  {
          list.AddNew(vec3d(v[0]+xi, v[1]+yi, v[2]), list[i].B());
          if( zi != 0 )
            list.AddNew(vec3d(v[0]+xi, v[1]+yi, v[2]+zi), list[i].B());
        }
        if( zi != 0 )
          list.AddNew(vec3d(v[0]+xi, v[1], v[2]+zi), list[i].B());
      }
      if( yi != 0 )  {
        list.AddNew(vec3d(v[0], v[1]+yi, v[2]), list[i].B());
        if( zi != 0 )
          list.AddNew(vec3d(v[0], v[1]+yi, v[2]+zi), list[i].B());
      }
      if( zi != 0 )
        list.AddNew(vec3d(v[0], v[1], v[2]+zi), list[i].B());
    }
  }
  // returns true if the atom overlaps with another atom in the unit cell
  bool DoesOverlap(const TCAtom& ca, double R) const;
  // finds an atom at given position
  TCAtom* FindCAtom(const vec3d& center) const;
  /* finds an atom within delta of the given position, ! the position is updated to the closest match found */
  TCAtom* FindOverlappingAtom(vec3d& position, double delta) const;
protected:
  class TSearchSymmEqTask  {
    TPtrList<TCAtom>& Atoms;
    const smatd_list& Matrices;
    TStrList& Report;
    bool Initialise;
    double tolerance;
    TAsymmUnit* AU;
    TLattice* Latt;
  public:
    TSearchSymmEqTask(TPtrList<TCAtom>& atoms, const smatd_list& matrices, TStrList& report,
                      double tol, bool initialise);
    void Run(long ind);
    TSearchSymmEqTask* Replicate() const  {
      return new TSearchSymmEqTask(Atoms, Matrices, Report, tolerance, Initialise);
    }
  };
public:
  void LibVolumeEx(const TStrObjList& Params, TMacroError& E);
  void LibCellEx(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};

EndXlibNamespace()
#endif

