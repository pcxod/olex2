//---------------------------------------------------------------------------
#ifndef srestraintH
#define srestraintH
#include "catom.h"
#include "ematrix.h"
// covers DFIX, SADI, DANG
BeginXlibNamespace()
// I am really tired of this bshyt
#undef AddAtom
class TAsymmUnit;
// restraint atom list types
const short rltNone   = 0, //default value for the constructor...
            rltAtoms  = 1, // set of independent atoms
            rltBonds  = 2, // set of "bonds" - atom pairs
            rltAngles = 3, // set of "angles" - atom triples
            rltGroup  = 4; // atoms represent a group

class TSimpleRestraint : public IEObject  {
  double Value, Esd, Esd1;
  bool AllNonHAtoms;
  short ListType;
  TCAtomGroup InvolvedAtoms;
  bool AtomsEqual(TCAtom* a1, const symmd* m1, TCAtom* a2, const symmd* m2);
  class TSRestraintList* Parent;
public:
  TSimpleRestraint(TSRestraintList* parent, const short listType);

  virtual ~TSimpleRestraint();
  void AddAtoms(const TCAtomGroup& atoms);
  void AddAtom(TCAtom& aa, const symmd* ma);
  void AddAtomPair(TCAtom& aa, const symmd* ma,
                   TCAtom& ab, const symmd* mb);

  inline TSRestraintList* GetParent()  {  return Parent;  }

  void RemoveAtom(int i);
  void Delete();
  void Validate();
  void Clear();

  void OnCAtomCrdChange( TCAtom* ca, const symmd& matr );

  // removes dublicated information depending on the list type
  void Substruct( TSimpleRestraint& sr);

  // copies data from a restraon, but with atoms from the thisAU
  void Assign( TAsymmUnit& thisAU, const TSimpleRestraint& );
  //const TSimpleRestraint& operator = ( const TSimpleRestraint& );

  inline int AtomCount()  const  {  return InvolvedAtoms.Count();  }
  inline const TGroupCAtom& GetAtom(int i)  const {  return InvolvedAtoms[i];  }
  bool ContainsAtom(TCAtom* ca) const;

  inline short GetListType()  const  {  return ListType;  }

  DefPropP(double, Value)
  DefPropP(double, Esd)
  DefPropP(double, Esd1)
  DefPropB(AllNonHAtoms)
};

class TSRestraintList : public IEObject  {
  TTypeList<TSimpleRestraint> Restraints;
  short RestraintListType;
public:
  TSRestraintList(const short restraintListType)  {  RestraintListType = restraintListType;  }
  virtual ~TSRestraintList()  {}
  TSimpleRestraint& AddNew()  {  return Restraints.AddNew(this, RestraintListType);  }
  // function checks uniquesness of the restraint data - previously defined values are removed
  void ValidateRestraint( TSimpleRestraint& sr);

  inline TSimpleRestraint& Release(int i)    {  return Restraints.Release(i);  }
  void Release(TSimpleRestraint& sr);
  inline void Restore(TSimpleRestraint& sr)  {  Restraints.Add(sr);  }

  void OnCAtomCrdChange( TCAtom* ca, const symmd& matr );
  
  void Assign(TAsymmUnit& au, const TSRestraintList& rl);
  inline void Clear()  {  Restraints.Clear();  }
  inline int Count() const {  return Restraints.Count();  }
  inline TSimpleRestraint& operator [] (int i){  return Restraints[i];  }
  inline short GetRestraintListType()  const  {  return RestraintListType;  }
};

typedef TTypeList<TSimpleRestraint> TSimpleRestraintList;
typedef TPtrList<TSimpleRestraint> TSimpleRestraintPList;

EndXlibNamespace()

#endif
