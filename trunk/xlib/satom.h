#ifndef satomH
#define satomH

#include "xbase.h"
#include "elist.h"
#include "vpoint.h"
#include "atominfo.h"
#include "catom.h"
#include "ematrix.h"
#include "typelist.h"
#include "tptrlist.h"

BeginXlibNamespace()

class TSAtom : public TBasicNode<TSAtom, class TSBond>  {
private:
  TMatrixDPList Matrices;
  // a list of pointers to matrices used for generation of atom
  class TCAtom*  FCAtom;       // basic crystallographic information
//  int FTag; // override TCollectioItem and TGDrawObject tags
  const class TEllipsoid*  FEllipsoid;   // a pointer to TEllipse object
  TVPointD  FCCenter;     // atom center in cell coordinates
  TVPointD  FCenter;          // atom center in cartesian coordinates
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
  inline const TMatrixD& GetMatrix(int i) const {  return *Matrices[i];  }
  inline void AddMatrix(TMatrixD* M)            {  Matrices.Add(M);  }
  inline void AddMatrices(TSAtom *A)            {  Matrices.AddList(A->Matrices); }

  void ChangeType(const olxstr& Type);

  inline const TEllipsoid* GetEllipsoid() const {  return FEllipsoid;  }
  inline void SetEllipsoid(const TEllipsoid* v) {  FEllipsoid = v;  }
  inline TVPointD&  CCenter()            {  return FCCenter;  }
  inline TVPointD&  Center()             {  return FCenter;  }
  inline const TVPointD&  GetCCenter()  const {  return FCCenter;  }
  inline const TVPointD&  GetCenter()   const {  return FCenter;  }
};
  typedef TTypeList<TSAtom> TSAtomList;
  typedef TPtrList<TSAtom> TSAtomPList;

EndXlibNamespace()
#endif

