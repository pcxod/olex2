#include "cifloop.h"
#include "cif.h"
using namespace cif_dp;
//..............................................................................
void TCifLoop::DeleteAtom(TCAtom *A)  {
  for( size_t i=0; i < Table.data.RowCount(); i++ )  {
    for( size_t j=0; j < Table.data.ColCount(); j++ )  {
      ICifEntry* CD = Table.data[i][j];
      if( EsdlInstanceOf(*CD, AtomCifEntry) && ((AtomCifEntry*)CD)->data == A )  {
        for( size_t k=0; k < Table.data.ColCount(); k++ )
          delete Table.data[i][k];
        Table.data.DelRow(i--);
        break;
      }
    }
  }
}
//..............................................................................
int TCifLoop::CifLoopSorter::Compare(const CifTable::TableSort& r1,
                                     const CifTable::TableSort& r2)
{
  uint64_t id1 = 0, id2 = 0;
  size_t ac = 0;
  for( size_t i=r1.data.Count(); i > 0; i-- )  {
    bool atom = false;
    if( EsdlInstanceOf(*r1.data[i-1], AtomCifEntry) )  {
      id1 |= ((uint64_t)(((AtomCifEntry*)r1.data[i-1])->data->GetId()) << ac*16);
      atom = true;
    }
    if( EsdlInstanceOf(*r2.data[i-1], AtomCifEntry) )  {
      id2 |= ((uint64_t)(((AtomCifEntry*)r2.data[i-1])->data->GetId()) << ac*16);
      atom = true;
    }
    if( atom )  ac++;
  }
  return (id1 < id2 ? -1 : (id1 > id2 ? 1 : 0)); 
}
//..............................................................................
void TCifLoop::UpdateTable(const TCif& parent)  {
  if( Table.data.RowCount() == 0 )  return;
  bool update = false;
  for( size_t i=0; i < Table.data.ColCount(); i++ )  {
    if( EsdlInstanceOf(*Table.data[0][i], AtomCifEntry) )  {
      update = true;
      break;
    }
  }
  if( !update  )  return;
  Table.data.SortRows<CifLoopSorter>();
  const size_t pi = Table.data.ColIndex("_atom_site_disorder_group");
  if( pi != InvalidIndex )  {
    for( size_t i=0; i < Table.data.RowCount(); i++ )
      if( EsdlInstanceOf(*Table.data[i][0], AtomCifEntry) &&
        ((AtomCifEntry*)Table.data[i][0])->data->GetPart() != 0 )
      {
        delete Table.data[i][pi];
        Table.data[i][pi] = new cetString((int)((AtomCifEntry*)Table.data[i][0])->data->GetPart());
      }
  }
}
