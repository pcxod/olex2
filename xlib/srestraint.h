//---------------------------------------------------------------------------
#ifndef srestraintH
#define srestraintH
#include "catom.h"
// covers DFIX, SADI, DANG
BeginXlibNamespace()
// I am really tired of this bshyt
#undef AddAtom
class TAsymmUnit;
class RefinementModel;
class TSRestraintList;
// restraint atom list types
const short rltNone   = 0, //default value for the constructor...
            rltAtoms  = 1, // set of independent atoms
            rltBonds  = 2, // set of "bonds" - atom pairs
            rltAngles = 3, // set of "angles" - atom triples
            rltGroup  = 4; // atoms represent a group

class TSimpleRestraint : public IEObject, public IXVarReferencer  {
  double Value, Esd, Esd1;
  XVarReference* VarRef;
  bool AllNonHAtoms;
  short ListType;
  TCAtomGroup InvolvedAtoms;
  inline bool AtomsEqual(TCAtom* a1, const smatd* m1, TCAtom* a2, const smatd* m2)  {
    if( a1 == a2 )  {
      if( (m1 == NULL && m2 == NULL) || ((m1 != NULL && m2 != NULL)  &&
        (*m1 == *m2) ) )  {
          return true;
      }
    }
    return false;
  }
  TSRestraintList& Parent;
public:
  TSimpleRestraint(TSRestraintList& parent, const short listType);

  virtual ~TSimpleRestraint();
  void AddAtoms(const TCAtomGroup& atoms);
  void AddAtom(TCAtom& aa, const smatd* ma);
  void AddAtomPair(TCAtom& aa, const smatd* ma,
                   TCAtom& ab, const smatd* mb);

  inline const TSRestraintList& GetParent() const {  return Parent;  }
  inline TSRestraintList& GetParent()             {  return Parent;  }

  void RemoveAtom(int i);
  void Delete();
  void Validate();
  void Clear();

  void OnCAtomCrdChange( TCAtom* ca, const smatd& matr );

  // removes dublicated information depending on the list type
  void Substruct( TSimpleRestraint& sr);

  // copies data from a restraon, but with atoms from the thisAU
  void Assign(const TSimpleRestraint& );
  //const TSimpleRestraint& operator = ( const TSimpleRestraint& );

  inline int AtomCount()  const  {  return InvolvedAtoms.Count();  }
  inline const TGroupCAtom& GetAtom(int i)  const {  return InvolvedAtoms[i];  }
  inline TGroupCAtom& GetAtom(int i)  {  return InvolvedAtoms[i];  }
  bool ContainsAtom(TCAtom* ca) const;

  inline short GetListType()  const  {  return ListType;  }

  DefPropP(double, Value)
  DefPropP(double, Esd)
  DefPropP(double, Esd1)
  DefPropBIsSet(AllNonHAtoms)

  // compares pointer addresses only!
  bool operator == (const TSimpleRestraint& sr) const {  return this == &sr;  }
// IXVarReferencer implementation
  virtual short VarCount() const {  return 1;  }
  virtual const XVarReference* GetVarRef(short var_index) const {  
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return VarRef;  
  }
  virtual XVarReference* GetVarRef(short var_index)  {  
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return VarRef;  
  }
  virtual olxstr GetVarName(short var_index) const;
  virtual void SetVarRef(short var_index, XVarReference* var_ref) {  
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    VarRef = var_ref;  
  }
  virtual IXVarReferencerContainer& GetParentContainer();
  virtual double GetValue(short var_index) const {  
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return Value;  
  }
  virtual void SetValue(short var_index, const double& val) {  
    if( var_index != 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    Value = val;  
  }
  virtual bool IsValid() const {  return true;  }
  virtual olxstr GetIdName() const;
//

  void ToDataItem(TDataItem& item) const;
#ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
#endif
  void FromDataItem(TDataItem& item);
};

class TSRestraintList : public IEObject, public IXVarReferencerContainer  {
  TTypeList<TSimpleRestraint> Restraints;
  short RestraintListType;
  RefinementModel& RefMod;
  olxstr IdName;
public:
  TSRestraintList(RefinementModel& rm, const short restraintListType, const olxstr& id_name) : 
      RefMod(rm), IdName(id_name) {
    RestraintListType = restraintListType;  
  }
  virtual ~TSRestraintList()  {}
  TSimpleRestraint& AddNew()  {  return Restraints.Add( new TSimpleRestraint(*this, RestraintListType));  }
  // function checks uniquesness of the restraint data - previously defined values are removed
  void ValidateRestraint( TSimpleRestraint& sr);
  void ValidateAll()  {
    for( int i=0; i < Restraints.Count(); i++ )
      Restraints[i].Validate();
  }

  TSimpleRestraint& Release(int i);
  void Release(TSimpleRestraint& sr);
  void Restore(TSimpleRestraint& sr);

  const RefinementModel& GetRM() const {  return RefMod;  }
  RefinementModel& GetRM()             {  return RefMod;  }

  void OnCAtomCrdChange( TCAtom* ca, const smatd& matr );
  
  void Assign(const TSRestraintList& rl);
  void Clear();
  inline int Count() const {  return Restraints.Count();  }
  inline TSimpleRestraint& operator [] (int i){  return Restraints[i];  }
  inline short GetRestraintListType()  const  {  return RestraintListType;  }

// IXVarReferencerContainer implementation
  virtual olxstr GetIdName() const {  return IdName;  }
  // note - possibly unsafe, type is not checked
  virtual int GetReferencerId(const IXVarReferencer& vr) const {
    int ind = Restraints.IndexOf((TSimpleRestraint&)vr);
    if( ind == -1 )
      throw TInvalidArgumentException(__OlxSourceInfo, "var referencer");
    return ind;
  }
  // note - possibly unsafe, range is unchecked
  virtual IXVarReferencer* GetReferencer(int id) {
    return &Restraints[id];
  }
  virtual int ReferencerCount() const {  return Restraints.Count();  }
//

  void ToDataItem(TDataItem& item) const;
  #ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms, TPtrList<PyObject>& equiv);
#endif
  void FromDataItem(TDataItem& item);
};

typedef TTypeList<TSimpleRestraint> TSimpleRestraintList;
typedef TPtrList<TSimpleRestraint> TSimpleRestraintPList;

EndXlibNamespace()

#endif
