#ifndef __olx_atom_sort_H
#define __olx_atom_sort_H
#include "lattice.h"
#include "edict.h"

const short 
  atom_sort_None      = 0x0000,
  atom_sort_Mw        = 0x0001,
  atom_sort_Label     = 0x0002,  // clever sort C1a1  
  atom_sort_Label1    = 0x0004, // string comparison sort
  atom_sort_MoetySize = 0x0008,  // fgrament size
  atom_sort_KeepH     = 0x0010,  // keeps H atoms after the pivoting one
  moety_sort_Create   = 0x0020,
  moety_sort_Size     = 0x0040,
  moety_sort_Weight   = 0x0080,
  moety_sort_Heaviest = 0x0100;  // sorts moeties by heaviest element of the moety

class AtomSorter {
public:
  static int atom_cmp_Mw(const TCAtom* a1, const TCAtom* a2) {
    const double diff = a2->GetAtomInfo().GetMr() - a1->GetAtomInfo().GetMr();
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
  static int atom_cmp_Label(const TCAtom* a1, const TCAtom* a2)  {
    return TCAtom::CompareAtomLabels(a1->GetLabel(), a2->GetLabel());
  }
  static int atom_cmp_Id(const TCAtom* a1, const TCAtom* a2)  {
    return a1->GetId() - a2->GetId();
  }
  static int atom_cmp_Label1(const TCAtom* a1, const TCAtom* a2) {
    return a1->GetLabel().Comparei(a2->GetLabel());
  }
  static int atom_cmp_MoetySize(const TCAtom* a1, const TCAtom* a2) {
    return a1->GetFragmentId() - a2->GetFragmentId();
  }
  static int atom_cmp_Mw_Label(const TCAtom* a1, const TCAtom* a2) {
    int rv = atom_cmp_Mw(a1,a2);
    return (rv == 0) ? atom_cmp_Label(a1,a2) : rv;
  }

  static void KeepH(TCAtomPList& l1, const TLattice& latt, int (*sort_func)(const TCAtom*, const TCAtom*))  {
    typedef AnAssociation2<TCAtom*,TCAtomPList> tree_node;
    TTypeList<tree_node> atom_tree;
    for( int i=0; i < latt.GetAsymmUnit().AtomCount(); i++ ) // tag all atoms with -1
      latt.GetAsymmUnit().GetAtom(i).SetTag(-1);
    for( int i=0; i < l1.Count(); i++ )  // tag atoms in the list with 0
      l1[i]->SetTag(0);
    int atom_count = 0;
    for( int i=0; i < l1.Count(); i++ )  {
      if( l1[i]->GetAtomInfo() == iHydrogenIndex || l1[i]->GetAtomInfo() == iDeuteriumIndex )
        continue;
      TSAtom* sa = NULL;
      for( int j=0; j < latt.AtomCount(); j++ )  {
        if( latt.GetAtom(j).CAtom().GetId() == l1[i]->GetId() )  {
          sa = &latt.GetAtom(j);
          break;
        }
      }
      TCAtomPList& ca_list = atom_tree.Add( new tree_node(l1[i])).B();
      if( l1[i]->GetAtomInfo() == iQPeakIndex || l1[i]->IsDeleted() )  {
        atom_count++;
        continue;
      }
      if( sa == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "aunit and lattice mismatch");
      for( int j=0; j < sa->NodeCount(); j++ )  {
        // check if the atom in the list
        if( sa->Node(j).CAtom().GetTag() == 0 && 
            (sa->Node(j).GetAtomInfo() == iHydrogenIndex || sa->Node(j).GetAtomInfo() == iDeuteriumIndex) )
          ca_list.Add( &sa->Node(j).CAtom() )->SetTag(1);
      }
      if( ca_list.Count() > 1 && sort_func != NULL )
        TCAtomPList::QuickSorter.SortSF(ca_list, sort_func);
      atom_count += (ca_list.Count() + 1);
    }
    if( atom_count != l1.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, "atom list does not match the lattice");
    atom_count = 0;
    for( int i=0; i < atom_tree.Count(); i++ )  {
      l1[atom_count++] = atom_tree[i].A();
      for( int j=0; j < atom_tree[i].GetB().Count(); j++ )
        l1[atom_count++] = atom_tree[i].GetB()[j];
    }
  }
  static void SyncLists(const TCAtomPList& ref, TCAtomPList& list)  {
    if( ref.Count() != list.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "lists mismatch");
    TCAtomPList list_copy(list);
    for( int i=0; i < ref.Count(); i++ )  {
      bool found = false;
      for( int j=0; j < ref.Count(); j++ )  {
        if( ref[i]->GetId() == list_copy[j]->GetId() )  {
          list[i] = list_copy[j];
          found = true;
          break;
        }
      }
      if( !found )
        throw TInvalidArgumentException(__OlxSourceInfo, "lists content mismatch");
    }
  }
  static void SortByName(TCAtomPList& list, const TStrList& atom_names)  {
    if( atom_names.Count() < 2 )
      return;
    int fi = -1;
    TCAtomPList sorted;
    for( int i=0; i < list.Count(); i++ )  {
      if( list[i]->GetLabel().Comparei(atom_names[0]) == 0 )  {
        fi = i;
        sorted.Add(list[i]);
        list[i] = NULL;
        break;
      }
    }
    if( fi == -1 )
      throw TInvalidArgumentException(__OlxSourceInfo, "atom names");
    for( int i=1; i < atom_names.Count(); i++ )  {
      for( int j=0; j < list.Count(); j++ )  {
        if( list[j] == NULL )  continue;
        if( list[j]->GetLabel().Comparei(atom_names[i]) == 0 )  {
          sorted.Add(list[j]);
          list[j] = NULL;
          break;
        }
      }
    }
    int ins_pos = 0;
    for( int i=0; i < fi; i++ )  {
      if( list[i] != NULL  )  
        ins_pos++;
    }
    list.Pack();
    list.Insert(ins_pos, sorted);
  }
  static void Sort(TCAtomPList& list, int (*sort_func)(const TCAtom*, const TCAtom*))  {
    TCAtomPList::QuickSorter.SortSF(list, sort_func);
  }
};

class MoetySorter {
  static int moety_cmp_Mr(const AnAssociation3<int,double,TCAtomPList>& m1, 
    const AnAssociation3<int,double,TCAtomPList>& m2) 
  {
    const double diff = m2.GetB() - m1.GetB();
    return diff < 0 ? -1 : ( diff > 0 ? 1 : 0);
  }
  static int moety_cmp_size(const AnAssociation2<int,TCAtomPList>& m1, 
    const AnAssociation2<int,TCAtomPList>& m2) 
  {
    return m2.GetB().Count() - m1.GetB().Count();
  }
public:
  static void SortByMoetyAtom(TCAtomPList& list, const TStrList& atom_names)  {
    if( atom_names.Count() < 2 )
      return;
    typedef AnAssociation2<TCAtom*, TCAtomPList> moety;
    TTypeList<moety> moeties;
    for( int i=0; i < list.Count(); i++ )
      list[i]->SetTag(-1);
    for( int i=0; i < atom_names.Count(); i++ )  {
      for( int j=0; j < list.Count(); j++ )  {
        if( list[j]->GetLabel().Comparei(atom_names[i]) == 0 )  {
          if( list[j]->GetTag() == -1 )  {  // avoid duplicated moeties
            list[j]->SetTag(moeties.Count());
            moeties.Add( new moety(list[j]) );
          }
        }
      }
    }
    if( moeties.IsEmpty() )
      return;
    for( int i=0; i < moeties.Count(); i++ )  {
      const int moety_id = moeties[i].GetA()->GetFragmentId();
      for( int j=0; j < list.Count(); j++ )  {
        if( moety_id == list[j]->GetFragmentId() && list[j]->GetTag() == -1 )  {
          list[j]->SetTag(i);
          moeties[i].B().Add(list[j]);
        }
      }
    }
    for( int i=0; i < list.Count(); i++ )  {
      if( list[i]->GetTag() != -1 )
        list[i] = NULL;
    }
    list.Pack();
    for( int i=0; i < moeties.Count(); i++ )  {
      moety& mt = moeties[i];
      list.Add( mt.A() );
      for( int j=0; j < mt.GetB().Count(); j++ )
        list.Add( mt.GetB()[j] );
    }
  }
  // does not do the sorting - just organises atoms into moeties
  static void CreateMoeties(TCAtomPList& list)  {
    // dictionary will not do here - the moety order will be destroyed...
    typedef AnAssociation2<int, TCAtomPList> moety;
    TTypeList<moety> moeties;
    for( int i=0; i < list.Count(); i++ )  {
      TCAtomPList* ca_list = NULL;
      for( int j=0; j < moeties.Count(); j++ )  {
        if( moeties[j].GetA() == list[i]->GetFragmentId() )  {
          ca_list = &moeties[j].B();
          break;
        }
      }
      if( ca_list == NULL )
        ca_list = &moeties.Add( new moety(list[i]->GetFragmentId()) ).B();
      ca_list->Add(list[i]);
    }
    if( moeties.Count() < 2 )  return;
    int atom_cnt = 0;
    for( int i=0; i < moeties.Count(); i++ )  {
      for( int j=0; j < moeties[i].GetB().Count(); j++ )
        list[atom_cnt++] = moeties[i].GetB()[j];
    }
  }
  static void SortByHeaviestElement(TCAtomPList& list)  {
    typedef AnAssociation3<int, double, TCAtomPList> moety;
    TTypeList<moety> moeties;
    for( int i=0; i < list.Count(); i++ )  {
      TCAtomPList* ca_list = NULL;
      for( int j=0; j < moeties.Count(); j++ )  {
        if( moeties[j].GetA() == list[i]->GetFragmentId() )  {
          ca_list = &moeties[j].C();
          if( list[i]->GetAtomInfo().GetMr() > moeties[j].GetB() )
            moeties[j].B() = list[i]->GetAtomInfo().GetMr();
          break;
        }
      }
      if( ca_list == NULL )
        ca_list = &moeties.Add( new moety(list[i]->GetFragmentId(), list[i]->GetAtomInfo().GetMr()) ).C();
      ca_list->Add(list[i]);
    }
    if( moeties.Count() < 2 )  return;
    moeties.QuickSorter.SortSF(moeties, moety_cmp_Mr);
    int atom_cnt = 0;
    for( int i=0; i < moeties.Count(); i++ )  {
      for( int j=0; j < moeties[i].GetC().Count(); j++ )
        list[atom_cnt++] = moeties[i].GetC()[j];
    }
  }
  static void SortByWeight(TCAtomPList& list)  {
    typedef AnAssociation3<int, double, TCAtomPList> moety;
    TTypeList<moety> moeties;
    for( int i=0; i < list.Count(); i++ )  {
      TCAtomPList* ca_list = NULL;
      for( int j=0; j < moeties.Count(); j++ )  {
        if( moeties[j].GetA() == list[i]->GetFragmentId() )  {
          ca_list = &moeties[j].C();
          moeties[j].B() += list[i]->GetAtomInfo().GetMr();
          break;
        }
      }
      if( ca_list == NULL )
        ca_list = &moeties.Add( new moety(list[i]->GetFragmentId(), list[i]->GetAtomInfo().GetMr()) ).C();
      ca_list->Add(list[i]);
    }
    if( moeties.Count() < 2 )  return;
    moeties.QuickSorter.SortSF(moeties, moety_cmp_Mr);
    int atom_cnt = 0;
    for( int i=0; i < moeties.Count(); i++ )  {
      for( int j=0; j < moeties[i].GetC().Count(); j++ )
        list[atom_cnt++] = moeties[i].GetC()[j];
    }
  }
  static void SortBySize(TCAtomPList& list)  {
    typedef AnAssociation2<int, TCAtomPList> moety;
    TTypeList<moety> moeties;
    for( int i=0; i < list.Count(); i++ )  {
      TCAtomPList* ca_list = NULL;
      for( int j=0; j < moeties.Count(); j++ )  {
        if( moeties[j].GetA() == list[i]->GetFragmentId() )  {
          ca_list = &moeties[j].B();
          break;
        }
      }
      if( ca_list == NULL )
        ca_list = &moeties.Add( new moety(list[i]->GetFragmentId()) ).B();
      ca_list->Add(list[i]);
    }
    if( moeties.Count() < 2 )  return;
    moeties.QuickSorter.SortSF(moeties, moety_cmp_size);
    int atom_cnt = 0;
    for( int i=0; i < moeties.Count(); i++ )  {
      for( int j=0; j < moeties[i].GetB().Count(); j++ )
        list[atom_cnt++] = moeties[i].GetB()[j];
    }
  }
};



#endif

