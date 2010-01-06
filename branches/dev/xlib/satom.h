#ifndef __olx_xl_satom_H
#define __olx_xl_satom_H
#include "xbase.h"
#include "atominfo.h"
#include "catom.h"
#include "symmat.h"
#include "typelist.h"
#include "tptrlist.h"

BeginXlibNamespace()

const short 
  satom_Deleted    = 0x0001,
  satom_Grown      = 0x0002,
  satom_Standalone = 0x0004;

class TSAtom : public TBasicNode<class TNetwork, TSAtom, class TSBond>  {
private:
  smatd_plist Matrices;
  // a list of pointers to matrices used for generation of atom
  TCAtom*  FCAtom;       // basic crystallographic information
//  int FTag; // override TCollectioItem and TGDrawObject tags
  class TEllipsoid*  FEllipsoid;   // a pointer to TEllipsoid object
  vec3d  FCCenter;     // atom center in cell coordinates
  vec3d  FCenter;          // atom center in cartesian coordinates
protected:
  mutable short Flags;
  int _SortNodesByDistanceAsc(const TSAtom* a1, const TSAtom* a2)  {
    const double diff = FCenter.DistanceTo(a1->FCenter) - FCenter.DistanceTo(a2->FCenter);
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
  int _SortNodesByDistanceDsc(const TSAtom* a1, const TSAtom* a2)  {
    const double diff = FCenter.DistanceTo(a2->FCenter) - FCenter.DistanceTo(a1->FCenter);
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
  static int _SortBondsByLengthAsc(const TSBond* b1, const TSBond* b2);
  static int _SortBondsByLengthDsc(const TSBond* b1, const TSBond* b2);
public:
  TSAtom(TNetwork *N);
  virtual ~TSAtom();
  void Assign(const TSAtom& S);
  
  // Is/Set
  DefPropBFIsSet(Deleted, Flags, satom_Deleted)
  DefPropBFIsSet(Standalone, Flags, satom_Standalone)

  bool IsAvailable() const {  return !(IsDeleted() || FCAtom->IsDetached());  }
  bool IsGrown() const;
  void SetGrown(bool v)  {  SetBit(v, Flags, satom_Grown);  }

  operator TCAtom* () const {  return FCAtom;  }

  TCAtom& CAtom() const {  return *FCAtom; }
  void CAtom(TCAtom& CA);

  void AtomInfo(TBasicAtomInfo& AI);
  const cm_Element& GetType() const {  return FCAtom->GetType(); }

  void SetLabel(const olxstr& L)  { FCAtom->SetLabel(L); }
  const olxstr& GetLabel() const {  return FCAtom->GetLabel(); }
  // returns a label plus (if not identity) first matrix like label_resi.2_556
  olxstr GetGuiLabel() const;
  // returns a label plus (if not identity) first matrix like label_resi(-2/3+X,Y,2-Z)
  olxstr GetGuiLabelEx() const;

  size_t MatrixCount() const {  return Matrices.Count();  }
  const smatd& GetMatrix(size_t i) const {  return *Matrices[i];  }
  // this also makes sure that the identity releated matrix is coming first in the list
  void AddMatrix(smatd* M) {  
    Matrices.Add(M);  
    if( M->IsFirst() && Matrices.Count() > 1 )
      Matrices.Swap(0, Matrices.Count()-1);
  }
  void AddMatrices(TSAtom* A)  {  
    const size_t cnt = Matrices.Count();
    Matrices.AddList(A->Matrices); 
    if( cnt != 0 && Matrices.Count() > 1 )  {
      for( size_t i=0; i < A->Matrices.Count(); i++ )  {
        if( A->Matrices[i]->IsFirst() )  {
          Matrices.Swap(0, cnt+i);
          break;
        }
      }
    }
  }
  void ClearMatrices()  {  Matrices.Clear();  }
  void ChangeType(const olxstr& Type);
  // beware that underlying objkect might be shared by several atoms!
  TEllipsoid* GetEllipsoid() const {  return FEllipsoid;  }
  void SetEllipsoid(TEllipsoid* v) {  FEllipsoid = v;  }
  vec3d& ccrd()  {  return FCCenter;  }
  vec3d& crd()  {  return FCenter;  }
  vec3d const& ccrd() const {  return FCCenter;  }
  vec3d const& crd() const {  return FCenter;  }

  void SortNodesByDistanceAsc()  {
    Nodes.QuickSorter.SortMF(Nodes, *this, &TSAtom::_SortNodesByDistanceAsc);
  }
  void SortNodesByDistanceDsc()  {
    Nodes.QuickSorter.SortMF(Nodes, *this, &TSAtom::_SortNodesByDistanceDsc);
  }
  void SortBondsByLengthAsc()  {
    Bonds.QuickSorter.SortSF(Bonds, &TSAtom::_SortBondsByLengthAsc);
  }
  void SortBondsByLengthDsc()  {
    Bonds.QuickSorter.SortSF(Bonds, &TSAtom::_SortBondsByLengthDsc);
  }
  // allows to trim the number of nodes
  void SetNodeCount(size_t cnt);
  // removes specified node from the list of nodes
  void RemoveNode(TSAtom& node);

  struct Ref  {
    size_t catom_id;
    uint32_t matrix_id;
    TArrayList<uint32_t>* matrices; 
    Ref(size_t a_id, const smatd_plist& symm) : catom_id(a_id), 
      matrix_id(symm[0]->GetId()), matrices(NULL) 
    {
      if( symm.Count() > 1 )  {
        matrices = new TArrayList<uint32_t>(symm.Count()-1);
        for( size_t i=1; i < symm.Count(); i++ )
          (*matrices)[i-1] = symm[i]->GetId();
      }
    } 
    Ref(const Ref& r) : catom_id(r.catom_id), 
      matrix_id(r.matrix_id), 
      matrices(r.matrices == NULL ? NULL : new TArrayList<uint32_t>(*r.matrices))  
    {}
    ~Ref()  {
      if( matrices != NULL )
        delete matrices;
    }
    Ref& operator = (const Ref& r)  {
      catom_id = r.catom_id;
      matrix_id = r.matrix_id;
      matrices = (r.matrices == NULL ? NULL : new TArrayList<uint32_t>(*r.matrices));
      return *this;
    }
    bool operator == (const Ref& r) const {
      if( catom_id == r.catom_id )  {
        if( matrix_id != r.matrix_id )  {
          if( matrices == NULL || r.matrices == NULL )
            return false;
          for( size_t i=0; i < r.matrices->Count(); i++ )
            if( matrix_id == (*r.matrices)[i] )
              return true;
        }
      }
      return false;
    }
  };

  bool operator == (const Ref& id) const {
    if( FCAtom->GetId() == id.catom_id )  {
      for( size_t i=0; i < Matrices.Count(); i++ )
        if( Matrices[i]->GetId() == id.matrix_id )
          return true;
    }
    return false;
  }
  Ref GetRef() const {  return Ref(FCAtom->GetId(), Matrices);  }

  virtual void ToDataItem(TDataItem& item) const;
  virtual void FromDataItem(const TDataItem& item, class TLattice& parent);
};

typedef TTypeList<TSAtom> TSAtomList;
typedef TPtrList<TSAtom> TSAtomPList;

EndXlibNamespace()
#endif

