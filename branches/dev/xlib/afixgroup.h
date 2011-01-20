#ifndef __olxs_afix_group_H
#define __olxs_afix_group_H
#include "catom.h"

BeginXlibNamespace()

class TAfixGroup : public ACollectionItem {
  double D, Sof, U;
  int Afix;
  size_t Id;
  TCAtom* Pivot;
  TCAtomPList Dependent;
  TAfixGroups& Parent;
  static const olxstr n_names[], m_names[];
public:
  TAfixGroup(TAfixGroups& parent) : Parent(parent), D(0), Sof(0), U(0) {}
  TAfixGroup(TAfixGroups& parent, size_t id, TCAtom* pivot, int afix, double d = 0, double sof = 0, double u = 0) :
      Parent(parent), Id(id), Pivot(pivot), D(d), Afix(afix), Sof(sof), U(u)  {  
    if( pivot != NULL )  {
      if( HasExcplicitPivot() || IsUnbound() )
        pivot->SetDependentAfixGroup(this);
      else 
        pivot->AddDependentHfixGroup(*this);
    }
  }
  TAfixGroup(TAfixGroups& parent, const TAfixGroup& ag) : Parent(parent) {  
    Assign(ag);  
  }
  ~TAfixGroup()  {  // note that Clear just removes the item from the parent list, calling this  
    for( size_t i=0; i < Dependent.Count(); i++ )
      Dependent[i]->SetParentAfixGroup(NULL);
    Dependent.Clear();
    if( Pivot == NULL )  return;
    if( HasExcplicitPivot() || IsUnbound() )
      Pivot->SetDependentAfixGroup(NULL);
    else if( Pivot->DependentHfixGroupCount() != 0 ) // might happen at several places 
      Pivot->RemoveDependentHfixGroup(*this);
  }
  DefPropP(double, D)
  DefPropP(double, Sof)
  DefPropP(double, U)
  DefPropP(size_t, Id)
  void Assign(const TAfixGroup& ag);
  TCAtom& GetPivot() {  return *Pivot;  }
  void SetPivot(TCAtom& ca)  {  
    Pivot = &ca;
    if( HasExcplicitPivot() || IsUnbound() )
      Pivot->SetDependentAfixGroup(this);
    else 
      Pivot->AddDependentHfixGroup(*this);
  }
  const TCAtom& GetPivot() const {  return *Pivot;  }
  int GetM() const {  return GetM(Afix);  }
  int GetN() const {  return GetN(Afix);  }
  bool IsFitted() const {  return IsFitted(Afix);  }
  bool IsRiding() const {  return IsRiding(Afix);  }
  bool IsSizable() const {  return IsSizable(Afix);  }
  bool IsRotable() const {  return IsRotable(Afix);  }
  bool IsRefinable() const {  return IsRefinable(Afix);  }
  bool IsUnbound() const {  return IsUnbound(Afix);  }
  bool HasExcplicitPivot() const {  return HasExcplicitPivot(Afix);  }
  static int GetM(int afix) {  return afix/10;  }
  static int GetN(int afix) {  return afix%10;  }
  static bool IsFitted(int afix)  {  
    if( afix > 10 )  {
      const int m = GetM(afix);
      return (m == 5 || m == 6 || m == 7 || m == 10 || m == 11 || m > 16);
    }
    else  // rigid group, no geometrical constraints
      return (afix == 6 || afix == 9 || afix == 7 || afix == 8);
  }
  static bool HasExcplicitPivot(int afix)  {
    const int n = GetN(afix), m = GetM(afix);
    return (n == 6 || n == 9 || (m == 0 && (n == 7 || n == 8)));
  }
  static bool IsRiding(int afix)  {
    const int n = GetN(afix);
    return (n == 3 || n == 4 || n == 7 || n == 8);
  }
  static bool IsSizable(int afix)  {
    const int n = GetN(afix);
    return (n == 4 || n == 8 || n == 9);
  }
  static bool IsRotable(int afix)  {
    const int n = GetN(afix);
    return (n == 6 || n == 7 || n == 8 || n == 9);
  }
  static bool IsRefinable(int afix)  {
    const int n = GetN(afix);
    return (n == 4 || n ==6 || n == 7 || n == 8 || n == 9);
  }
  static bool IsUnbound(int afix)  {
    return (afix == 1 || afix == 2);
  }
  static bool IsDependent(int afix)  {
    return GetN(afix) == 5;
  }
  TCAtom& AddDependent(TCAtom& a)  {
    Dependent.Add(&a);
    a.SetParentAfixGroup(this);
    return a;
  }
  int GetAfix() const {  return Afix;  }
  void SetAfix(int a)  {
    if( a == 0 )
      Clear();
    Afix = a;
  }
  void Clear();
  bool IsEmpty()  const {  return (Pivot == NULL || Pivot->IsDeleted() || 
    (!IsUnbound() && Dependent.IsEmpty()));  }
  size_t Count() const {  return Dependent.Count();  }
  TCAtom& operator [] (size_t i) {  return *Dependent[i];  }
  const TCAtom& operator [] (size_t i) const {  return *Dependent[i];  }
  // describes current group (just the AFIX)
  olxstr Describe() const;
  // returns string describing the group content
  TIString ToString() const;

  void ToDataItem(TDataItem& item) const;
#ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
  void FromDataItem(TDataItem& item);
};
//....................................................................................
class TAfixGroups {
  TTypeList<TAfixGroup> Groups;
public:

  class RefinementModel& RM;

  TAfixGroups(RefinementModel& parent) : RM(parent) {}

  TAfixGroup& New(TCAtom* pivot, int Afix, double d = 0, double sof = 0, double u = 0 )  {
    return Groups.Add( new TAfixGroup(*this, Groups.Count(), pivot, Afix, d, sof, u) );
  }
  size_t Count() const {  return Groups.Count();  }
  TAfixGroup& operator [] (size_t i) {  return Groups[i];  }
  const TAfixGroup& operator [] (size_t i) const {  return Groups[i];  }
  void Clear() {  Groups.Clear();  }
  void Delete(size_t i)  {
    Groups.Delete(i);
    for( size_t j=i; j < Groups.Count(); j++ )
      Groups[j].SetId(j);
  }
  void Assign(const TAfixGroups& ags)  {
    Clear();
    for( size_t i=0; i < ags.Count(); i++ )  {
      if( !ags[i].IsEmpty() )  {
        Groups.Add(new TAfixGroup(*this, ags[i]));
        Groups.GetLast().SetId(Groups.Count() - 1);
      }
    }
  }
  void ValidateAll() {
    for( size_t i=0; i < Groups.Count(); i++ )
      if( Groups[i].IsEmpty() )
        Groups.NullItem(i);
    Groups.Pack();
  }
  void ToDataItem(TDataItem& item);
#ifndef _NO_PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
  void FromDataItem(TDataItem& item);
};

EndXlibNamespace()
#endif
