#ifndef envilistH
#define envilistH

#include "xbase.h"

#include "atominfo.h"
#include "vpoint.h"
#include "ematrix.h"
#include "typelist.h"
#include "tptrlist.h"
#include "catom.h"

BeginXlibNamespace()

class TAtomEnvi  {
  TTypeList< AnAssociation3<TCAtom*, TMatrixD, TVPointD> >  Envi;
  TSAtom* Base;
public:
  TAtomEnvi()  {  Base = NULL;  }
  virtual ~TAtomEnvi()  {  }

  void Add( TCAtom& ca, const TMatrixD& matr, const TVPointD& crd )  {
    Envi.AddNew(&ca, matr, crd);
  }

  void Clear()  {  Envi.Clear();  }

  inline int Count()    const {  return Envi.Count();  }
  inline bool IsEmpty() const {  return (Envi.Count() == 0);  }

  inline TSAtom& GetBase()                  const {  return *Base;  }
  inline void SetBase(TSAtom& base)               {  Base = &base;  }
  inline const olxstr& GetLabel(int ind)  const {  return Envi[ind].A()->Label();  }
  inline TBasicAtomInfo& GetBAI(int ind)          {  return Envi[ind].A()->GetAtomInfo();  }
  inline TCAtom& GetCAtom(int ind)          const {  return *Envi[ind].A();  }
  inline const TVPointD& GetCrd(int ind)    const {  return Envi[ind].GetC();  }
  inline const TMatrixD& GetMatrix(int ind) const {  return Envi[ind].GetB();  }

  void Exclude(TCAtom& ca )  {
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


