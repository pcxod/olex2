#ifndef __olx_xl_unitcell_H
#define __olx_xl_unitcell_H
#include "xbase.h"
#include "symmat.h"
#include "envilist.h"
#include "ellipsoid.h"
#include "estrbuffer.h"
#include "arrays.h"
#include "lattice.h"
#include "asymmunit.h"
#include "symspace.h"
#ifdef QLength  // on Linux it is defined as something...
  #undef QLength
#endif

BeginXlibNamespace()

class TUnitCell: public IEObject  {
  TNetwork*  Network;  // for internal use only
  smatd_list Matrices;  // list of unique matrices; FMatrices + centering
  TArrayList<TEllpPList> Ellipsoids;  // i - atoms index, j - matrix index 
  TTypeList<TArrayList<uint8_t> > MulDest;  // index of r from a product of two matrices
  TArrayList<uint8_t> InvDest;
  class TLattice*  Lattice;    // parent lattice
  /* a macro FindInRange, if the atom list is NULL, atoms are taken from the asymmetric unit,
  atoms are excluded only if deleted, availability is not counted */
  void _FindInRange(const vec3d& center, double R,
    TTypeList<AnAssociation3<TCAtom*, smatd, vec3d> >& res,
    const TCAtomPList* atoms=NULL) const;
  /* macro FindBinding, if the atoms is NULL, atoms of the asymmetric unit are taken and atoms are excluded
  only if deleted, availability is not counted */
  void _FindBinding(const TCAtom& center, const smatd& center_tm, double delta,
    TTypeList<AnAssociation3<TCAtom*, smatd, vec3d> >& res,
    const TCAtomPList* atoms=NULL) const;
  // explicit use of the I matrix 
  void _FindBinding(const TCAtom& center, double delta,
    TTypeList<AnAssociation3<TCAtom*, smatd, vec3d> >& res,
    const TCAtomPList* atoms=NULL) const
  {  _FindBinding(center, smatd().I(), delta, res, atoms);  }
  // taking the transformation from the position generator
  void _FindBinding(const TSAtom& center, double delta,
    TTypeList<AnAssociation3<TCAtom*, smatd, vec3d> >& res,
    const TCAtomPList* atoms=NULL) const
  {  _FindBinding(center.CAtom(), center.GetMatrix(0), delta, res, atoms);  }
  //
  static int _AtomSorter(const AnAssociation3<vec3d,TCAtom*, double>& a1, const AnAssociation3<vec3d,TCAtom*, double>& a2)  {
    const double d = a1.GetA().QLength() - a2.GetA().QLength();
    return d < 0 ? -1 : (d > 0 ? 1 : 0);
  }
public:
  TUnitCell(TLattice* L);
  virtual ~TUnitCell();

  double CalcVolume()  const;
  TEValue<double> CalcVolumeEx() const;
  inline TLattice& GetLattice() const {  return *Lattice;  }
  void Clear();

  inline size_t MatrixCount() const {  return Matrices.Count();  }
  // the identity matrix is always the first
  inline const smatd& GetMatrix(size_t i) const {  return Matrices[i];  }
  // initialises the matrix container id, throws an excpetion if matrix is not found
  smatd& InitMatrixId(smatd& m) const;
  /* if there is a list of transforms calculated in the asymmetric unit and a symmetry operator
  needs to be applied to it, this is the function. Note that the list of input matrices and the
  transformation matrix should have valid Id's. The return value is a new list of matrices
  with new Id's */
  smatd_list MulMatrices(const smatd_list& in, const smatd& transform) const;
  smatd MulMatrix(const smatd& m, const smatd& tr) const {
    smatd rv = m*tr;  // rv*r = tr*(m*r) - this is how it is when applied to a vector....
    const uint8_t index = MulDest[m.GetContainerId()][tr.GetContainerId()];
    rv.SetId(index, (rv.t-Matrices[index].t).Round<int>());
    return rv;
  }
  smatd InvMatrix(const smatd& m) const {
    smatd rv = m.Inverse();
    const uint8_t index = InvDest[m.GetContainerId()];
    rv.SetId(index, (rv.t-Matrices[index].t).Round<int>());
    return rv;
  }
  size_t EllpCount() const {  return Ellipsoids.Count()*Matrices.Count();  }
  const TEllipsoid* GetEllp(size_t i) const {  return Ellipsoids[i/Matrices.Count()][i%Matrices.Count()];  }
  TEllipsoid* GetEllp(size_t i)  {  return Ellipsoids[i/Matrices.Count()][i%Matrices.Count()];  }
  // gets an ellipsoid for an atom by asymmetric unit Id and a matrix associated with it
  const TEllipsoid& GetEllipsoid(size_t MatrixId, size_t AUId) const {  return *Ellipsoids[AUId][MatrixId];  }
  TEllipsoid& GetEllipsoid(size_t MatrixId, size_t AUId)  {  return *Ellipsoids[AUId][MatrixId];  }
  void AddEllipsoid(); // adds a new row to ellipsoids, intialised with NULLs
  void ClearEllipsoids();
  // (re)caches the ellipsoids from the aunit
  void UpdateEllipsoids();
  // expands the lattice centering and '-1' and caches ellipsoids for all symmetry operators
  void InitMatrices();

  /* Removes symm eqivs and initialises symmetry induced connectivity */
  void FindSymmEq() const;

  /* the funciton searches a matrix which moves "atom" to "to" so that the
   distance between them is shortest and return the matrix, which if not NULL
   has to be deleted with delete
  */
  smatd* GetClosest(const class TCAtom& to, const TCAtom& atom, bool ConsiderOriginal, double* dist=NULL) const {
    return GetClosest(to.ccrd(), atom.ccrd(), ConsiderOriginal, dist);
  }
  smatd* GetClosest(const vec3d& to, const vec3d& from, bool ConsiderOriginal, double* dist=NULL) const;
  
  /* Finds matrices UNIQ to the unit cell (besides the I matrix, see below) which move point 
  'from' to 'to' within R. 
  Always returns a valid object to be deleted with delete. For the identity matrix additionally checks the 
  translational symmetry (in some cases the atom within the aunit is closer than the translational 
  symmetry generated one), so there might be a two I matrices with different translations
  */ 
  smatd_list* GetInRange(const vec3d& to, const vec3d& from, double R, bool IncludeI) const;

  /* function operates as the one above, however for each matrix also searches translations
  within [-1..+1] range, so returned list might have multiple matrices of the unit cell with
  different translations. It also checks if the matrix is already in the ToSkip list before adding it
  to the results. The return value is always a valid object to be deleted with a call to delete
  */
  smatd_list* GetInRangeEx(const vec3d& to, const vec3d& from, double R, bool IncludeI, const smatd_list& ToSkip) const;
  
  /* The function finds all atoms and symmetry operators generating them within the sphere of 
  radius R. Note that only matrices unique to the unit cell are used (besides that the I matrics is 
  checked within [-1..+1] range in all directions), so only limited R values are supported. 
  Expects a list of Assiciation2+<TCAtom const*, smatd,...>
  */
  template <class association> void FindInRangeAM(const vec3d& center, double R,
    TArrayList<association>& out, const TCAtomPList* atoms=NULL) const
  {
    TTypeList<AnAssociation3<TCAtom*,smatd,vec3d> > res;
    _FindInRange(center, R, res, atoms);
    out.SetCount(res.Count());
    for( size_t i=0; i < res.Count(); i++ )  {
      out[i].A() = res[i].A();
      out[i].B() = res[i].GetB(); 
    }
  }
  /* finds only bound atoms defined by delta, can take both TCAtom and TSAtom. In first case the
  result will be located at the origin of the atom in the asymmetric unit, in the second - to the 
  center of the TSAtom */
  template <class Atom, class Association> void FindBindingAM(const Atom& center, double delta,
    TArrayList<Association>& out, const TCAtomPList* atoms=NULL) const
  {
    TTypeList<AnAssociation3<TCAtom*,smatd,vec3d> > res;
    _FindBinding(center, delta, res, atoms);
    out.SetCount(res.Count());
    for( size_t i=0; i < res.Count(); i++ )  {
      out[i].A() = res[i].A();
      out[i].B() = res[i].GetB(); 
    }
  }
  /* As above but provides an association of atoms with the cartesian coordinates 
  instead of the matrices 
  Expects a list of Assiciation2+<TCAtom const*, vec3d,...>
  */
  template <class association> void FindInRangeAC(const vec3d& center, double R,
    TArrayList<association>& out, const TCAtomPList* atoms=NULL) const
  {
    TTypeList<AnAssociation3<TCAtom*,smatd,vec3d> > res;
    _FindInRange(center, R, res, atoms);
    out.SetCount(res.Count());
    for( size_t i=0; i < res.Count(); i++ )  {
      out[i].A() = res[i].A();
      out[i].B() = res[i].GetC(); 
    }
  }

  /* returns a list of all matrices which lead to covalent/hydrogen bonds, 
  the return value is always an object to be deleted with delete 
  */
  smatd_list* GetBinding(const TCAtom& toA, const TCAtom& fromA,
    const vec3d& to, const vec3d& from, bool IncludeI, bool IncludeHBonds) const;
  
  typedef MatrixListAdaptor<TUnitCell> MatrixList;
  typedef TSymSpace<TUnitCell::MatrixList> SymSpace;

  MatrixList GetMatrixList() const {  return MatrixList(*this);  }

  SymSpace GetSymSpace() const {
    const TAsymmUnit& au = GetLattice().GetAsymmUnit();
    return SymSpace(MatrixList(*this),
      au.GetCartesianToCell(), au.GetCellToCartesian(), au.GetHklToCartesian(), au.GetLatt() > 0);
  }
  // returns the closest distance between two atoms considering the unit cell symmetry
  double FindClosestDistance(const TCAtom& to, const TCAtom& from) const {
    return FindClosestDistance(GetLattice().GetAsymmUnit(), MatrixList(*this),
      to.ccrd(), from.ccrd());
  }
  double FindClosestDistance(const vec3d& to, const vec3d& from) const {
    return FindClosestDistance(GetLattice().GetAsymmUnit(), MatrixList(*this),
      to, from);
  }

  template <class MatList> static double FindClosestDistance(const TAsymmUnit& au, const MatList& ml,
    const vec3d& to, const vec3d& from)
  {
    vec3d v = au.Orthogonalise(from-to);
    double minD = v.QLength();
    for( size_t i=0; i < ml.Count(); i++ )  {
      v = ml[i]*from - to;
      const double D = au.Orthogonalise(v -= v.Round<int>()).QLength();
      if( D < minD )  
        minD = D;
    }
    return sqrt(minD);
  }
  
  /* finds all atoms (+symm attached) and Q-peaks, if specfied; if part is not -1, part 0 and 
  the specified part are only placed
  */
  void GetAtomEnviList(TSAtom& atom, TAtomEnvi& envi, bool IncludeQ = false, int part=DefNoPart) const;
  
  // finds only q-peaks in the environment of specified atom
  void GetAtomQEnviList(TSAtom& atom, TAtomEnvi& envi);
  
  /* finds "pivoting" atoms for possible h-bonds, considering O and N only with
    distances within 2.9A and angles 89-130 deg
  */
  void GetAtomPossibleHBonds(const TAtomEnvi& atom, TAtomEnvi& envi);

  /* returns a multiplier for the number of matrices in the asymmetruc unit for
    specified LATT instruction
  */
  static size_t GetMatrixMultiplier(short Latt);
  /* This function creates a map of the unit cell with provided partioning.
  It uses Van-der-Waals atomic radii by defualt and adds delta to it. The grid points
  belonging to atoms have value 'value', the others - '0'
  Returns the number of grid points occupied by the structure to structurePoinst if not NULL.
  If _template is provided - only these atoms are used int the calculation
  */
  void BuildStructureMap_Direct(TArray3D<short>& map, double delta, short value, 
    ElementRadii* radii, const TCAtomPList* _template = NULL);
  // much faster, but less 'precise' version
  void BuildStructureMap_Masks(TArray3D<short>& map, double delta, short value, 
    ElementRadii* radii, const TCAtomPList* _template = NULL);
  /**/
  void BuildDistanceMap_Direct(TArray3D<short>& map, double delta, short value, 
    ElementRadii* radii, const TCAtomPList* _template = NULL);
  // much faster, but less 'precise' version
  void BuildDistanceMap_Masks(TArray3D<short>& map, double delta, short value, 
    ElementRadii* radii, const TCAtomPList* _template = NULL);
protected:
  // helper function, association should be AnAssociation2+<vec3d,TCAtom*,+>
  template <class Association> 
    static int AtomsSortByDistance(const Association* A1, const Association* A2)  {
      const double d = A1->GetA().QLength() - A2->GetA().QLength();
      return (d < 0 ? -1 : ((d > 0 ) ? 1 : 0));
    }
public:
  // creates an array of matrices for a given aunit and lattice type
  static void GenerateMatrices(smatd_list& out, const TAsymmUnit& au, short lat);
  // association should be AnAssociation2+<vec3d,TCAtom*,+>, generates all atoms of the aunit
  template <class Association> void GenereteAtomCoordinates(TTypeList<Association>& list, bool IncludeH, 
                    const TCAtomPList* _template = NULL) const  {
    TCAtomPList& atoms = (_template == NULL ? *(new TCAtomPList) : *const_cast<TCAtomPList*>(_template));
    if( _template == NULL )  {
      list.SetCapacity(Lattice->GetAsymmUnit().AtomCount()*Matrices.Count());
      for( size_t i=0; i < Lattice->GetAsymmUnit().AtomCount(); i++ )  {
        TCAtom& ca = Lattice->GetAsymmUnit().GetAtom(i);
        if( ca.GetType() == iQPeakZ || ca.IsDeleted() )  continue;
        if( !IncludeH && ca.GetType() == iHydrogenZ )    continue;
        atoms.Add(&ca);
      }
    }
    list.SetCapacity(atoms.Count()*Matrices.Count());
    const size_t atom_cnt = atoms.Count();
    for( size_t i=0; i < atom_cnt; i++ )  {
      TCAtom* ca = atoms[i];
      if( ca->GetType() == iQPeakZ || ca->IsDeleted() )  continue;
      for( size_t j=0; j < Matrices.Count(); j++ )  {
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
    list.QuickSorter.SortSF(list, AtomsSortByDistance);
    const size_t lc = list.Count();
    for( size_t i=0; i < lc; i++ )
      distances[i] = (float)list[i].GetA().QLength();

    for( size_t i=0; i < lc; i++ )  {
      if( list.IsNull(i) )  continue;
      for( size_t j=i+1; j < lc; j++ )  {
        if( list.IsNull(j) )  continue;
        if( (distances[j] - distances[i]) > 0.1 )  break;
        const double d = list[i].GetA().QDistanceTo( list[j].GetA() );
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
  This function is used in the BuildStructure map function to take into account atoms which are near the cell sides
  */
  template <class Association> void ExpandAtomCoordinates(TTypeList<Association>& list, const double lim)  {
    list.SetCapacity( list.Count()*7 );
    const double c_min = lim, c_max = 1.0 -lim;
    const size_t all_ac = list.Count();
    for( size_t i=0; i < all_ac; i++ )  {
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
  // finds an atom at given position
  TCAtom* FindCAtom(const vec3d& center) const;
  /* finds an atom within delta of the given position */
  TCAtom* FindOverlappingAtom(const vec3d& position, double delta) const;
  size_t GetPositionMultiplicity(const vec3d& p) const {
    return GetPositionMultiplicity(MatrixList(*this), p);
  }
  template <class MatList> static size_t GetPositionMultiplicity(const MatList& ml, const vec3d& p)  {
    size_t m=1;  // identity
    for( size_t i=1; i < ml.Count(); i++ )  {
      vec3d v = p - ml[i]*p;
      v -= v.Round<int>();
      if( v.IsNull(1e-3) )
        m++;
    }
    return m;
  }
protected:
  class TSearchSymmEqTask  {
    TPtrList<TCAtom>& Atoms;
    const smatd_list& Matrices;
    TAsymmUnit* AU;
    TLattice* Latt;
  public:
    TSearchSymmEqTask(TPtrList<TCAtom>& atoms, const smatd_list& matrices);
    void Run(size_t ind) const;
    TSearchSymmEqTask* Replicate() const {
      return new TSearchSymmEqTask(Atoms, Matrices);
    }
  };
  class TBuildDistanceMapTask  {
    float*** map;
    TTypeList<AnAssociation3<vec3f,TCAtom*, float> >& atoms;
    const vec3s& dims;
    const mat3f& tm;
    float** loop_data;
    bool owns_data;
    TBuildDistanceMapTask(TBuildDistanceMapTask& parent) : 
    tm(parent.tm), map(parent.map), atoms(parent.atoms), dims(parent.dims),
      loop_data(parent.loop_data), owns_data(false)  {}
  public:
    TBuildDistanceMapTask(const mat3f& _tm, float*** _map, const vec3s& _dims,
      TTypeList<AnAssociation3<vec3f,TCAtom*, float> >& _atoms) :
      tm(_tm), map(_map), atoms(_atoms), dims(_dims), owns_data(true)
    {
      init_loop_data();
    }
    void init_loop_data();
    void clear_loop_data();
    void Run(size_t ind) const;
    TBuildDistanceMapTask* Replicate() {
      return new TBuildDistanceMapTask(*this);
    }
  };
public:
  void LibVolumeEx(const TStrObjList& Params, TMacroError& E);
  void LibCellEx(const TStrObjList& Params, TMacroError& E);
  void LibMatrixCount(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};

EndXlibNamespace()
#endif
