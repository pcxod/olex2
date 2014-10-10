/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olxs_afix_group_H
#define __olxs_afix_group_H
#include "catom.h"

BeginXlibNamespace()

class TAfixGroup : public ACollectionItem {
  TAfixGroups& Parent;
  size_t Id;
  TCAtom* Pivot;
  double D, Sof, U;
  int Afix;
  TCAtomPList Dependent;
  static const olxstr n_names[], m_names[];
public:
  TAfixGroup(TAfixGroups& parent)
    : Parent(parent), D(0), Sof(0), U(0) {}
  TAfixGroup(TAfixGroups& parent, size_t id, TCAtom* pivot,
    int afix, double d = 0, double sof = 0, double u = 0)
    : Parent(parent), Id(id), Pivot(pivot), D(d), Sof(sof), U(u), Afix(afix)
  {
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
  // note that Clear just removes the item from the parent list, calling this
  ~TAfixGroup()  {
    for( size_t i=0; i < Dependent.Count(); i++ )
      Dependent[i]->SetParentAfixGroup(NULL);
    Dependent.Clear();
    if( Pivot == NULL )  return;
    if( HasExcplicitPivot() || IsUnbound() )
      Pivot->SetDependentAfixGroup(NULL);
    // might happen at several places
    else if( Pivot->DependentHfixGroupCount() != 0 )
      Pivot->RemoveDependentHfixGroup(*this);
  }
  const TAfixGroups &GetParent() const { return Parent; }
  TAfixGroups &GetParent() { return Parent; }
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
  bool IsSizable() const {  return IsSizable(Afix);  }
  bool IsRotating() const {  return IsRotating(Afix);  }
  bool IsRefinable() const {  return IsRefinable(Afix);  }
  bool IsUnbound() const {  return IsUnbound(Afix);  }
  bool HasExcplicitPivot() const {  return HasExcplicitPivot(Afix);  }
  bool HasImplicitPivot() const {  return HasImplicitPivot(Afix);  }
  bool IsFixedGroup() const {  return IsFixedGroup(Afix);  }
  bool IsFittedRing() const {  return IsFittedRing(Afix);  }
  bool IsFittedGroup() const {  return IsFittedGroup(Afix);  }
  bool IsRiding() const {  return IsRiding(Afix);  }
  static int GetM(int afix) {  return afix/10;  }
  static int GetN(int afix) {  return afix%10;  }
  /** returns true if the grous is well defined and the number of atoms of
  the group is fixed
  */
  static bool IsFixedGroup(int afix)  {
    const int m = GetM(afix);
    return !(m == 0 || m > 16);
  }
  static bool IsFittedRing(int afix)  {
    const int m = GetM(afix);
    return (m == 5 || m == 6 || m == 7 || m == 10 || m == 11);
  }
  static bool IsFittedGroup(int afix)  {  return GetM(afix) > 16;  }
  /** returns true for groups where pivot atom is a part of the group and
  not a preceeding (like afix n=3 vs n=6)
  */
  static bool HasExcplicitPivot(int afix)  {
    const int n = GetN(afix);
    return (n == 6 || n == 9);
  }
  // these require an implicit pivot (outside the group)
  static bool HasImplicitPivot(int afix)  {
    const int n = GetN(afix);
    return (n == 3 || n == 4 || n == 7 || n == 8 || afix == -1);
  }
  // these are just 'service' AFIX...
  static bool HasPivot(int afix)  {
    const int n = GetN(afix);
    return !(n == 1 || n == 2);
  }
  static bool IsRiding(int afix)  {
    return (HasImplicitPivot(afix) || IsDependent(afix));
  }
  //bond lengths/size can are uniformly sizeable
  static bool IsSizable(int afix)  {
    const int n = GetN(afix);
    return (n == 4 || n == 8 || n == 9);
  }
  //free rotating or pivoted rotating
  static bool IsRotating(int afix)  {
    const int n = GetN(afix);
    return (n == 6 || n == 7 || n == 8 || n == 9);
  }
  // if the group has a refineable parameter (size, angle etc)
  static bool IsRefinable(int afix)  {
    const int n = GetN(afix);
    return (n == 4 || n ==6 || n == 7 || n == 8 || n == 9);
  }
  // these require no pivot atom
  static bool IsUnbound(int afix)  {
    return (afix == 1 || afix == 2);
  }
  static bool IsDependent(int afix)  {
    return GetN(afix) == 5;
  }
  TCAtom& AddDependent(TCAtom& a) {
    Dependent.Add(&a);
    if (a.GetParentAfixGroup() != NULL)
      a.GetParentAfixGroup()->RemoveDependent(a);
    a.SetParentAfixGroup(this);
    return a;
  }
  TCAtom& RemoveDependent(TCAtom& a)  {
    Dependent.Remove(&a);
    a.SetParentAfixGroup(NULL);
    return a;
  }
  int GetAfix() const {  return Afix;  }
  void SetAfix(int a)  {
    if( a == 0 )
      Clear();
    Afix = a;
  }
  void Clear();
  void Sort();
  bool IsEmpty() const;
  size_t Count() const {  return Dependent.Count();  }
  TCAtom& operator [] (size_t i) const {  return *Dependent[i];  }
  // describes current group (just the AFIX)
  olxstr Describe() const;
  // returns string describing the group content
  TIString ToString() const;

  void ToDataItem(TDataItem& item) const;
#ifdef _PYTHON
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
  TAfixGroup& New(TCAtom* pivot, int Afix, double d = 0, double sof = 0,
    double u = 0 )
  {
    return Groups.Add(
      new TAfixGroup(*this, Groups.Count(), pivot, Afix, d, sof, u));
  }
  size_t Count() const {  return Groups.Count();  }
  TAfixGroup& operator [] (size_t i) {  return Groups[i];  }
  const TAfixGroup& operator [] (size_t i) const {  return Groups[i];  }
  void Clear() {  Groups.Clear();  }
  void Delete(size_t i)  {
    Groups.Delete(i);
    for (size_t j=i; j < Groups.Count(); j++)
      Groups[j].SetId(j);
  }
  void Assign(const TAfixGroups& ags) {
    Clear();
    for (size_t i=0; i < ags.Count(); i++ ) {
      if (!ags[i].IsEmpty()) {
        Groups.Add(new TAfixGroup(*this, ags[i]));
        Groups.GetLast().SetId(Groups.Count() - 1);
      }
    }
  }
  void ValidateAll() {
    for (size_t i = 0; i < Groups.Count(); i++) {
      if (Groups[i].IsEmpty())
        Groups.NullItem(i);
    }
    Groups.Pack();
    for (size_t i = 0; i < Groups.Count(); i++) {
      Groups[i].SetId(i);
    }
  }
  void SortGroupContent();
  void Release(TAfixGroup &ag);
  void Restore(TAfixGroup &ag);
  void ToDataItem(TDataItem& item);
#ifdef _PYTHON
  PyObject* PyExport(TPtrList<PyObject>& atoms);
#endif
  void FromDataItem(TDataItem& item);
};

EndXlibNamespace()
#endif
