#ifndef envilistH
#define envilistH

#include "xbase.h"

#include "atominfo.h"
#include "symmat.h"
#include "typelist.h"
#include "tptrlist.h"
#include "satom.h"

BeginXlibNamespace()

class TAtomEnvi  {
  TTypeList< AnAssociation3<TCAtom*, smatd, vec3d> >  Envi;
  TSAtom* Base;
  int _SortByDistance( const AnAssociation3<TCAtom*, smatd, vec3d>& i1, const 
    AnAssociation3<TCAtom*, smatd, vec3d>& i2) const 
  {
    const double diff = i1.GetC().QDistanceTo(Base->crd()) - i2.GetC().QDistanceTo(Base->crd());
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
public:
  TAtomEnvi()  {  Base = NULL;  }
  virtual ~TAtomEnvi()  {  }

  void Add( TCAtom& ca, const smatd& matr, const vec3d& crd )  {
    Envi.AddNew(&ca, matr, crd);
  }

  void Clear()  {  Envi.Clear();  }

  inline size_t Count() const {  return Envi.Count();  }
  inline bool IsEmpty() const {  return (Envi.Count() == 0);  }

  inline TSAtom& GetBase()               const {  return *Base;  }
  inline void SetBase(TSAtom& base)            {  Base = &base;  }
  inline const olxstr& GetLabel(size_t ind) const {  return Envi[ind].A()->Label();  }
  inline TBasicAtomInfo& GetBAI(size_t ind)  {  return Envi[ind].A()->GetAtomInfo();  }
  inline TCAtom& GetCAtom(size_t ind) const {  return *Envi[ind].A();  }
  inline const vec3d& GetCrd(size_t ind) const {  return Envi[ind].GetC();  }
  inline const smatd& GetMatrix(size_t ind) const {  return Envi[ind].GetB();  }
  void Delete(size_t i)  {  Envi.Delete(i);  }
  void SortByDistance()  {
    Envi.QuickSorter.SortMF(Envi, *this, &TAtomEnvi::_SortByDistance);
  }
  void Exclude(TCAtom& ca )                    {
    for( size_t i=0; i < Envi.Count(); i++ )
      if( Envi[i].GetA() == &ca )  {
        Envi.Delete(i);
        break;
      }
  }
  // applies a symmetry operation to all matrices and recalculates the coordinates
  void ApplySymm(const smatd& sym);
#ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
};

  typedef TPtrList<TAtomEnvi>  TAtomEnviPList;
  typedef TTypeList<TAtomEnvi>  TAtomEnviList;

EndXlibNamespace()
#endif


