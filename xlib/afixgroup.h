#ifndef __olxs_afix_group_H
#define __olxs_afix_group_H
#include "catom.h"

BeginXlibNamespace()

class TAfixGroup : public ACollectionItem {
  double D, Sof, U;
  int Afix, Id;
  TCAtom* Pivot;
  TCAtomPList Dependent;
  TAfixGroups& Parent;
public:
  TAfixGroup(TAfixGroups& parent) : Parent(parent) {}
  TAfixGroup(TAfixGroups& parent, int id, TCAtom* pivot, int afix, double d = 0, double sof = 0, double u = 0) :
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
    for( int i=0; i < Dependent.Count(); i++ )
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
  DefPropP(int, Id)
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
  bool IsUnbound() const {  return IsUnbound(Afix);  }
  bool HasExcplicitPivot() const {  return HasExcplicitPivot(Afix);  }
  static int GetM(int afix) {  return afix < 10 ? 0 : afix/10;  }
  static int GetN(int afix) {  return afix < 10 ? afix : afix%10;  }
  static bool IsFitted(int afix)  {  
    int m = GetM(afix);
    return (m == 5 || m == 6 || m == 7 || m == 10 || m == 11 || m > 16 );
  }
  static bool HasExcplicitPivot(int afix)  {
    int n = GetN(afix), m = GetM(afix);
    return (n == 6 || n == 9);
  }
  static bool IsRiding(int afix)  {
    int n = GetN(afix);
    return (n == 3 || n == 4 || n == 7 || n == 8);
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
  }
  void Clear();
  bool IsEmpty()  const {  return (Pivot == NULL || Pivot->IsDeleted() || 
    (!IsUnbound() && Dependent.IsEmpty()));  }
  int Count() const {  return Dependent.Count();  }
  TCAtom& operator [] (int i) {  return *Dependent[i];  }
  const TCAtom& operator [] (int i) const {  return *Dependent[i];  }

  void ToDataItem(TDataItem& item) const;
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
  int Count() const {  return Groups.Count();  }
  TAfixGroup& operator [] (int i) {  return Groups[i];  }
  const TAfixGroup& operator [] (int i) const {  return Groups[i];  }
  void Clear() {  Groups.Clear();  }
  void Delete(int i)  {
    Groups.Delete(i);
    for( int j=i; j < Groups.Count(); j++ )
      Groups[j].SetId(j);
  }
  void Assign(const TAfixGroups& ags)  {
    Clear();
    for( int i=0; i < ags.Count(); i++ )  {
      if( !ags[i].IsEmpty() )  {
        Groups.Add( new TAfixGroup(*this, ags[i]) );
        Groups.Last().SetId( Groups.Count() - 1 );
      }
    }
  }
  void ValidateAll() {
    for( int i=0; i < Groups.Count(); i++ )
      if( Groups[i].IsEmpty() )
        Groups.NullItem(i);
    Groups.Pack();
  }
  void ToDataItem(TDataItem& item);
  void FromDataItem(TDataItem& item);
};

EndXlibNamespace()
#endif
