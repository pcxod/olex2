#include "afixgroup.h"
#include "refmodel.h"

void TAfixGroup::Clear()  {  Parent.Delete(Id);  }
//..............................................................................
void TAfixGroup::Assign(const TAfixGroup& ag)  {
  D = ag.D;
  Sof = ag.Sof;
  U = ag.U;
  Afix = ag.Afix;
  
  Pivot = Parent.RM.aunit.FindCAtomByLoaderId(ag.Pivot->GetLoaderId());
  if( Pivot == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
  SetPivot( *Pivot );
  for( int i=0; i < ag.Dependent.Count(); i++ )  {
    Dependent.Add( Parent.RM.aunit.FindCAtomByLoaderId( ag.Dependent[i]->GetLoaderId()) );
    if( Dependent.Last() == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units mismatch");
    Dependent.Last()->SetParentAfixGroup(this);
  }
}
//..............................................................................
void TAfixGroup::ToDataItem(TDataItem& item) const {
  item.AddField("afix", Afix);
  item.AddField("d", D);
  item.AddField("u", U);
  item.AddField("pivot_atom_id", Pivot->GetTag());
  TDataItem& dep = item.AddItem("dependent");
  int dep_id = 0;
  for( int i=0; i < Dependent.Count(); i++ )  {
    if( Dependent[i]->IsDeleted() )  continue;
    dep.AddField(olxstr("atom_id_") << dep_id++, Dependent[i]->GetTag());
  }
}
//..............................................................................
void TAfixGroup::FromDataItem(TDataItem& item) {
  Afix = item.GetRequiredField("afix").ToInt();
  D = item.GetRequiredField("d").ToDouble();
  U = item.GetRequiredField("u").ToDouble();
  SetPivot( Parent.RM.aunit.GetAtom( item.GetRequiredField("pivot_atom_id").ToInt() ) );
  TDataItem& dep = item.FindRequiredItem("dependent");
  for( int i=0; i < dep.FieldCount(); i++ )
    Dependent.Add( &Parent.RM.aunit.GetAtom(dep.GetField(i).ToInt()) )->SetParentAfixGroup(this);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TAfixGroups::ToDataItem(TDataItem& item) {
  int group_id = 0;
  for( int i=0; i < Groups.Count(); i++ )  {
    if( Groups[i].IsEmpty() )  {
      Groups.NullItem(i);
      continue;
    }
    Groups[i].SetId(group_id++);
  }
  Groups.Pack();
  item.AddField("n", Groups.Count());
  for( int i=0; i < Groups.Count(); i++ )
    Groups[i].ToDataItem( item.AddItem(i) );
}
//..............................................................................
void TAfixGroups::FromDataItem(TDataItem& item) {
  Clear();
  int n = item.GetRequiredField("n").ToInt();
  if( n != item.ItemCount() )
    throw TFunctionFailedException(__OlxSourceInfo, "number of items mismatch");
  for( int i=0; i < n; i++ )  {
    Groups.Add( new TAfixGroup(*this) ).SetId(i);
    Groups.Last().FromDataItem(item.GetItem(i) );
  }
}
