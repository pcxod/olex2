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
public:
  TAtomEnvi()  {  Base = NULL;  }
  virtual ~TAtomEnvi()  {  }

  void Add( TCAtom& ca, const smatd& matr, const vec3d& crd )  {
    Envi.AddNew(&ca, matr, crd);
  }

  void Clear()  {  Envi.Clear();  }

  inline int Count()    const {  return Envi.Count();  }
  inline bool IsEmpty() const {  return (Envi.Count() == 0);  }

  inline TSAtom& GetBase()               const {  return *Base;  }
  inline void SetBase(TSAtom& base)            {  Base = &base;  }
  inline const olxstr& GetLabel(int ind) const {  return Envi[ind].A()->Label();  }
  inline TBasicAtomInfo& GetBAI(int ind)       {  return Envi[ind].A()->GetAtomInfo();  }
  inline TCAtom& GetCAtom(int ind)       const {  return *Envi[ind].A();  }
  inline const vec3d& GetCrd(int ind)    const {  return Envi[ind].GetC();  }
  inline const smatd& GetMatrix(int ind) const {  return Envi[ind].GetB();  }
  void Delete(int i)                           {  Envi.Delete(i);  }
  void Exclude(TCAtom& ca )                    {
    for( int i=0; i < Envi.Count(); i++ )
      if( Envi[i].GetA() == &ca )  {
        Envi.Delete(i);
        break;
      }
  }

};

  typedef TPtrList<TAtomEnvi>  TAtomEnviPList;
  typedef TTypeList<TAtomEnvi>  TAtomEnviList;

EndXlibNamespace()
#endif


