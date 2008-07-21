#ifndef satomH
#define satomH

#include "xbase.h"
#include "elist.h"
#include "atominfo.h"
#include "catom.h"
#include "symmat.h"
#include "typelist.h"
#include "tptrlist.h"

BeginXlibNamespace()

class TSAtom : public TBasicNode<TSAtom, class TSBond>  {
private:
  symmd_plist Matrices;
  // a list of pointers to matrices used for generation of atom
  class TCAtom*  FCAtom;       // basic crystallographic information
//  int FTag; // override TCollectioItem and TGDrawObject tags
  const class TEllipsoid*  FEllipsoid;   // a pointer to TEllipse object
  vec3d  FCCenter;     // atom center in cell coordinates
  vec3d  FCenter;          // atom center in cartesian coordinates
protected:
  bool Deleted, Grown;
public:
  TSAtom(TNetwork *N);
  virtual ~TSAtom();
  void Assign(TSAtom *S);

  DefPropB(Deleted)

  bool IsGrown();
  inline void SetGrown(bool v)  {  Grown = v;  }

  inline operator TCAtom* () const {  return FCAtom;  }

  inline TCAtom& CAtom()     const {  return *FCAtom; }
  void CAtom(TCAtom& CA);

  void AtomInfo(TBasicAtomInfo *AI);
  inline TBasicAtomInfo& GetAtomInfo()    const {  return FCAtom->GetAtomInfo(); }

  inline void SetLabel(const olxstr &L)       { FCAtom->SetLabel(L); }
  inline const olxstr& GetLabel() const       {  return FCAtom->GetLabel(); }

  inline int MatrixCount() const         {  return Matrices.Count();  }
  inline const symmd& GetMatrix(int i) const {  return *Matrices[i];  }
  inline void AddMatrix(symmd* M)            {  Matrices.Add(M);  }
  inline void AddMatrices(TSAtom *A)         {  Matrices.AddList(A->Matrices); }

  void ChangeType(const olxstr& Type);

  inline const TEllipsoid* GetEllipsoid() const {  return FEllipsoid;  }
  inline void SetEllipsoid(const TEllipsoid* v) {  FEllipsoid = v;  }
  inline vec3d&  ccrd()             {  return FCCenter;  }
  inline vec3d&  crd()              {  return FCenter;  }
  inline vec3d const&  ccrd() const {  return FCCenter;  }
  inline vec3d const&  crd()  const {  return FCenter;  }
};
  typedef TTypeList<TSAtom> TSAtomList;
  typedef TPtrList<TSAtom> TSAtomPList;

EndXlibNamespace()
#endif

