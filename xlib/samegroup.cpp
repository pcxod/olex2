#include "samegroup.h"
#include "asymmunit.h"

void TSameGroup::Assign(TAsymmUnit& tau, const TSameGroup& sg)  {
  Clear();
  if( sg.Count() == 0 )  return;

  TAsymmUnit * au = sg[0].GetParent();

  if( au == &tau )  {
    for(int i=0; i < sg.Count(); i++ )
      Add( const_cast<TCAtom&>(sg[i]) );
  }
  else  {
    for(int i=0; i < sg.Count(); i++ )  {
      TCAtom* aa = tau.FindCAtomByLoaderId( sg[i].GetLoaderId() );
      if( aa == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "asymmetric units do not match");
      Add( *aa );
    }
  }
  Esd12 = sg.Esd12;  
  Esd13 = sg.Esd13;  
  for( int i=0; i < sg.Dependent.Count(); i++ )
    Dependent.Add( &Parent[ sg.Dependent[i]->Id ] );
}
//..........................................................................................
void TSameGroup::ToDataItem(TDataItem& item) const {
  item.AddField("esd12", Esd12);
  item.AddField("esd13", Esd13);
  int atom_id = 0;
  for( int i=0; i < Atoms.Count(); i++ )  {
    if( Atoms[i]->IsDeleted() ) continue;
    item.AddItem(atom_id++, Atoms[i]->GetTag() );
  }
  TDataItem& dep = item.AddItem("dependent");
  for( int i=0; i < Dependent.Count(); i++ )  {
    item.AddItem(atom_id++, Dependent[i]->GetId() );
  }
}
//..........................................................................................
void TSameGroup::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..........................................................................................
//..........................................................................................
//..........................................................................................
void TSameGroupList::ToDataItem(TDataItem& item) const {
  int group_id = 0;
  for( int i=0; i < Groups.Count(); i++ )  {
    Groups[i].ToDataItem( item.AddItem(group_id++) );
  }
}
//..........................................................................................
void TSameGroupList::FromDataItem(TDataItem& item) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..........................................................................................
