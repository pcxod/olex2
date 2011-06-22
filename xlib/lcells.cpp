/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "lcells.h"
#include "symmparser.h"
#include "symspace.h"
#include "math/composite.h"
#include "xapp.h"

using namespace lcells;

olx_object_ptr<TTypeList<CellInfo> > CellReader::read(const olxstr &fn)  {
  TStrList lines;
  lines.LoadFromFile(fn);
  olx_object_ptr<TTypeList<CellInfo> > r_(new TTypeList<CellInfo>);
  TTypeList<CellInfo> &rvl = r_;
  const olxstr ext = TEFile::ExtractFileExt(fn).ToLowerCase();
  try {
    if( ext == "cif" )  {
      cif_dp::TCifDP cif;
      cif.LoadFromStrings(lines);
      for( size_t db=0; db < cif.Count(); db++ )    {
        cif_dp::CifBlock& block = cif[db];
        try {
          olxstr l = GetCifParamAsString(block, "_symmetry_space_group_name_h-m").ToLowerCase();
          char latt = 0;
          if( l.IsEmpty() || l == '?' )  {  // go hard way then...
            latt = ExtractLattFromSymmetry(block);
          }
          else  {
            for( size_t j=0; j < l.Length(); j++ )  {
              switch( l.CharAt(j) )  {
              case 'p':  latt = 1;    break;
              case 'i':  latt = 2;    break;
              case 'r':  latt = 3;    break;
              case 'f':  latt = 4;    break;
              case 'a':  latt = 5;    break;
              case 'b':  latt = 6;    break;
              case 'c':  latt = 7;    break;
              }
              if( latt != 0 )   break;
            }
          }
          if( latt != 0 )  {
            CellInfo rv;
            rv.cell[0] = ToDouble(GetCifParamAsString(block, "_cell_length_a"));
            rv.cell[1] = ToDouble(GetCifParamAsString(block, "_cell_length_b"));
            rv.cell[2] = ToDouble(GetCifParamAsString(block, "_cell_length_c"));
            rv.cell[3] = ToDouble(GetCifParamAsString(block, "_cell_angle_alpha"));
            rv.cell[4] = ToDouble(GetCifParamAsString(block, "_cell_angle_beta"));
            rv.cell[5] = ToDouble(GetCifParamAsString(block, "_cell_angle_gamma"));
            rv.lattice = latt;
            rvl.AddCCopy(rv);
          }
        }
        catch(...)  {}
      }
    }
    else if( ext == "ins" || ext == "res") {
      CellInfo rv;
      bool cell_found = false, latt_found = false;
      for( size_t i=0; i < lines.Count(); i++ )  {
        if( !cell_found && lines[i].StartsFromi("cell") )  {
          TStrList toks(lines[i], ' ');
          if( toks.Count() == 8 )  {
            const size_t st = 2;
            rv.cell[0] = toks[st].ToDouble();
            rv.cell[1] = toks[st+1].ToDouble();
            rv.cell[2] = toks[st+2].ToDouble();
            rv.cell[3] = toks[st+3].ToDouble();
            rv.cell[4] = toks[st+4].ToDouble();
            rv.cell[5] = toks[st+5].ToDouble();
            cell_found = true;
          }
        }
        else if( cell_found && lines[i].StartsFromi("latt") )  {
          TStrList toks(lines[i], ' ');
          if( toks.Count() == 2 )  {
            rv.lattice = olx_abs(toks[1].ToInt());
            latt_found = true;
            break;
          }
        }
      }
      if( cell_found && latt_found )
        rvl.AddCCopy(rv);
    }
  }
  catch(const TExceptionBase &e)  {
    TBasicApp::NewLogEntry(logError) << fn << ": ";
    TBasicApp::NewLogEntry() << e.GetException()->GetStackTrace<TStrList>();
  }
  for( size_t i=0; i < rvl.Count(); i++ )  {
    rvl[i].volume = TUnitCell::CalcVolume(rvl[i].cell);
    rvl[i].niggle_volume = TUnitCell::CalcVolume(
      Niggli::reduce_const(rvl[i].lattice, rvl[i].cell));
  }
  return r_;
}
//.............................................................................
double CellReader::ToDouble(const olxstr &str)  {
  const size_t i = str.FirstIndexOf('(');
  return (i == InvalidIndex ? str : str.SubStringTo(i)).ToDouble();
}
//.............................................................................
olxstr CellReader::GetCifParamAsString(const cif_dp::CifBlock &block, const olxstr &Param)  {
  using namespace cif_dp;
  IStringCifEntry* ce = dynamic_cast<IStringCifEntry*>(block.param_map.Find(Param, NULL));
  if( ce == NULL || ce->Count() == 0 )  return EmptyString();
  olxstr rv = (*ce)[0];
  for( size_t i = 1; i < ce->Count(); i++ )
    rv << '\n' << (*ce)[i];
  return rv;
}
//.............................................................................
int CellReader::ExtractLattFromSymmetry(const cif_dp::CifBlock &block)  {
  try  {
    smatd_list matrices;
    cif_dp::cetTable *Loop = block.table_map.Find("_space_group_symop", NULL);
    if( Loop == NULL )
      Loop = block.table_map.Find("_space_group_symop_operation_xyz", NULL);
    if( Loop != NULL  )  {
      size_t sindex = Loop->ColIndex("_space_group_symop_operation_xyz");
      if( sindex == InvalidIndex )  return 0;
      for( size_t i=0; i < Loop->RowCount(); i++ )  {
        if( TSymmParser::SymmToMatrix(Loop->Get(i, sindex).GetStringValue(), matrices.AddNew()) )
          return 1;
      }
    }
    else  {
      Loop = block.table_map.Find("_symmetry_equiv_pos", NULL);
      if( Loop == NULL )
        Loop = block.table_map.Find("_symmetry_equiv_pos_as_xyz", NULL);
      if( Loop != NULL  )  {
        size_t sindex = Loop->ColIndex("_symmetry_equiv_pos_as_xyz");
        if( sindex == InvalidIndex )  return 0;
        for( size_t i=0; i < Loop->RowCount(); i++ )  {
          if( !TSymmParser::SymmToMatrix(Loop->Get(i, sindex).GetStringValue(), matrices.AddNew()) )
            return 0;
        }
      }
    }
    if( matrices.IsEmpty() )  return 0;
    return SymSpace::GetInfo(matrices).latt;
  }
  catch(const TExceptionBase &e)  {
    TBasicApp::NewLogEntry() << e.GetException()->GetFullMessage();
    return 0;
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
SortedObjectList<olxstr, olxstrComparator<false> > Index::masks;
char Index::file_sig[] = "LCI";
Index::Index()  {
  if( masks.IsEmpty() )  {
    masks.Add("cif");
    masks.Add("ins");
    masks.Add("res");
  }
}
//.............................................................................
void Index::Create(const olxstr &folder, const olxstr& index_name)  {
  TFileTree ft(folder);
  TBasicApp::NewLogEntry() << "Searching files in '" << folder << "' - this may take a while...";
  TBasicApp::GetInstance().Update();
  ft.Expand();
  TBasicApp::NewLogEntry() << "Found:";
  for( size_t i=0; i < masks.Count(); i++ )  {
    TBasicApp::NewLogEntry() << "  " << ft.GetRoot().CountFiles(olxstr("*.")
      << masks[i]) << ' ' << masks[i].ToUpperCase();
  }
  TBasicApp::NewLogEntry() << "  files";
  root.name = TEFile::UnixPath(folder);
  root.Init(ft.GetRoot());
  LastUpdated = TETime::EpochTime();
  SaveToFile(index_name);
}
//.............................................................................
void Index::SaveToFile(const olxstr &file_name) const {
  TEFile f(file_name + ".tmp", "w+b");
  f.Write(&file_sig[0], 3);
  f << LastUpdated;
  uint16_t stream_flags=0;
  f << stream_flags;
  root.ToStream(f);
  f.Close();
  TEFile::Rename(file_name + ".tmp", file_name);
}
//.............................................................................
Index &Index::LoadFromFile(const olxstr &file_name) {
  TEFile in(file_name, "rb");
  char sig[3];
  in.Read(&sig[0], 3);
  if( olxstr::o_strcmp(sig, 3, file_sig, 3) != 0 )  {
    throw TFunctionFailedException(__OlxSourceInfo, "File signarure mismatches");
  }
  in >> LastUpdated;
  // does nothing for now
  uint16_t stream_flags=0;
  in >> stream_flags;
  //
  root.FromStream(in);
  return *this;
}
//.............................................................................
void Index::Update(const olxstr &index)  {
  Index ind;
  ind.LoadFromFile(index);
  ind.PrintInfo();
  size_t cnt = ind.root.TotalCount();
  TFileTree ft(ind.root.name);
  TBasicApp::NewLogEntry() << "Searching files in '" << ind.root.name << "' - this may take a while...";
  TBasicApp::GetInstance().Update();
  ft.Expand();
  TBasicApp::NewLogEntry() << "Updated: " << ind.root.Update(ft.GetRoot()) << " entries";
  ind.LastUpdated = TETime::EpochTime();
  ind.SaveToFile(index);
  size_t cnt1 = ind.root.TotalCount();
  if( cnt1 > cnt )
    TBasicApp::NewLogEntry() << "Added: " << cnt1-cnt << " new entries";
}
//.............................................................................
void Index::PrintInfo() const {
  TBasicApp::NewLogEntry() << "Last updated: " << TETime::FormatDateTime(LastUpdated);
  TBasicApp::NewLogEntry() << "Number of entries: " << root.TotalCount();
}
//.............................................................................
size_t Index::FolderEntry::Update(const TFileTree::Folder &folder)  {
  size_t updated_cnt = 0;
  folders.QuickSorter.Sort<Entry::NameComparator>(folders);
  entries.QuickSorter.Sort<Entry::NameComparator>(entries);
  ConstSlice<TTypeList<FileEntry>, FileEntry>
    file_entries(entries, 0, entries.Count());
  for( size_t i=0; i < folder.FileCount(); i++ )  {
    const TFileListItem &f = folder.GetFile(i);
    if( !ConsiderFile(f.GetName()) )  continue;
    const size_t fi = sorted::FindIndexOf(
      file_entries, Entry::NameComparator(), f.GetName());
    if( fi == InvalidIndex )  {
      TBasicApp::NewLogEntry() << "New file added: " << folder.GetFullPath() << f.GetName();
      entries.Add(new FileEntry(*this, f.GetName(), f.GetModificationTime())).cells =
        CellReader::read(folder.GetFullPath()+f.GetName())();
    }
    else  {
      if( entries[fi].modified != f.GetModificationTime() )  {
        updated_cnt++;
        TBasicApp::NewLogEntry() << "Updated file: " << folder.GetFullPath() << f.GetName();
        entries[fi].cells = CellReader::read(folder.GetFullPath()+f.GetName())();
        entries[fi].modified = f.GetModificationTime();
      }
    }
  }
  ConstSlice<TTypeList<FolderEntry>, FolderEntry>
    folder_entries(folders, 0, folders.Count());
  for( size_t i=0;  i < folder.FolderCount(); i++ )  {
    const TFileTree::Folder &f = folder.GetFolder(i);
    const size_t fi = sorted::FindIndexOf(
      folder_entries, Entry::NameComparator(), f.GetName());
    if( fi == InvalidIndex )  {
      folders.Add(new FolderEntry(this, f.GetName(), f.GetModificationTime())).
        Init(f);
    }
    else
      folders[fi].Update(f);
  }
  return updated_cnt;
}
//.............................................................................
void Index::FolderEntry::Init(const TFileTree::Folder &folder)  {
  if( parent != NULL )
    name = folder.GetName();
  modified = folder.GetModificationTime();
  for( size_t i=0; i < folder.FileCount(); i++ )  {
    const TFileListItem &f = folder.GetFile(i);
    TTypeList<CellInfo> res = CellReader::read(folder.GetFullPath()+f.GetName())();
    if( res.IsEmpty() )  continue;
    FileEntry &fe = entries.Add(
      new FileEntry(*this, f.GetName(), f.GetModificationTime()));
    fe.cells = res;
  }
  for( size_t i=0; i < folder.FolderCount(); i++ )  {
    const TFileTree::Folder &f = folder.GetFolder(i);
    folders.Add(new FolderEntry(this, f.GetName(), f.GetModificationTime())).
      Init(f);
  }
}
//.............................................................................
TTypeList<Index::ResultEntry> Index::Search(const CellInfo &cell, double diff,
  bool filter_by_dimensions) const
{
  TTypeList<ResultEntry> all, res;
  all.SetCapacity(root.TotalCount());
  root.Expand(all);
  TEBitArray usage(all.Count());
/* alternative to use the usgae flags would be something like this:
  res.QuickSorter.Sort<TComparablePtrComparator>(res);
  ConstSlice<TTypeList<Index::ResultEntry>, ResultEntry> res_slice(res, 0, res.Count());
...
    if( sorted::FindIndexOf(res_slice, TComparablePtrComparator(), all[j]) == InvalidIndex )
*/
  CellInfo to_search = cell;
  // search by niggli volume
  Niggli::reduce(to_search.lattice, to_search.cell);
  all.QuickSorter.Sort<CellInfo::ReducedVolumeComparator>(all);
  const size_t ni = sorted::FindInsertIndex(all, CellInfo::ReducedVolumeComparator(), to_search);
  // go right
  for( size_t j=ni+1; j < all.Count(); j++ )  {
    if( olx_abs(all[j].niggle_volume-cell.niggle_volume) > diff )
      break;
    usage.SetTrue(j);
    res.AddCCopy(all[j]);
  }
  // go left
  for( size_t j=ni; j != InvalidIndex; j-- )  {
    if( olx_abs(all[j].niggle_volume-cell.niggle_volume) > diff )
      break;
    usage.SetTrue(j);
    res.AddCCopy(all[j]);
  }
  // search by cell volume
  all.QuickSorter.Sort(all, CellInfo::VolumeComparator(), SyncSwapListener<TEBitArray>(usage));
  const size_t vi = sorted::FindInsertIndex(all, CellInfo::VolumeComparator(), to_search);
  // go right
  for( size_t j=vi+1; j < all.Count(); j++ )  {
    if( olx_abs(all[j].volume-cell.volume) > diff )
      break;
    if( !usage[j] )
      res.AddCCopy(all[j]);
  }
  // go left
  for( size_t j=vi; j != InvalidIndex; j-- )  {
    if( olx_abs(all[j].volume-cell.volume) > diff )
      break;
    if( !usage[j] )
      res.AddCCopy(all[j]);
  }
  if( filter_by_dimensions )  {
    const double dd = pow(diff, 1./3);
    for( size_t i=0; i < res.Count(); i++ )  {
      evecd c = Niggli::reduce_const(cell.lattice, cell.cell);
      bool match = true;
      for( int j=0; j < 6; j++ )  {
        if( olx_abs(to_search.cell[j]-c[j]) > (j < 3 ? dd : dd/90) ) {
          match = false;
          break;
        }
      }
      if( !match )
        res.NullItem(i);
    }
    res.Pack();
  }
  return res;
}
//.............................................................................
void Index::PrintResults(const TTypeList<ResultEntry> & res)  {
  size_t cell_num=0, rows_processes=0;
  TTable tab(0, 3);
  for( size_t i=0; i < res.Count(); i++ )  {
    if( i > 0 && res[i].cell == res[i-1].cell )  {
      if( TEFile::ChangeFileExt(res[i].file_name, EmptyString()).Equalsi(
        TEFile::ChangeFileExt(res[i-1].file_name, EmptyString())) )
        continue;
    }
    TStrList& row = tab.AddRow();
    row[0] = ++cell_num;
    row[1] = res[i].file_name;
    TStrList& row1 = tab.AddRow();
    row1[1] << res[i].cell.ToString() << ", " << TCLattice::SymbolForLatt(res[i].lattice);
    row1[2] << olxstr::FormatFloat(2, res[i].volume) << "A^3";
    evecd rc = Niggli::reduce_const(res[i].lattice, res[i].cell);
    if( rc.QDistanceTo(res[i].cell) < 1e-6 )  continue;
    TStrList& row2 = tab.AddRow();
    row2[0] = "  Niggle cell";
    row2[1] << rc.ToString();
    row2[2] << olxstr::FormatFloat(2, res[i].niggle_volume) << "A^3";
  }
  TBasicApp::GetLog() << tab.CreateTXTList("Search results", true, false, ' ');
}
//.............................................................................
TTypeList<Index::ResultEntry> IndexSearch(const olxstr &index_name,
  const TStrObjList &Cmds, double vol_diff)
{
  CellInfo cell;
  if( Cmds.IsEmpty() )  {
    TXApp& app = TXApp::GetInstance();
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    for( int i=0; i < 3; i++ )  {
      cell.cell[i] = au.GetAxes()[i];
      cell.cell[3+i] = au.GetAngles()[i];
    }
    cell.lattice = olx_abs(au.GetLatt());
    cell.volume = TUnitCell::CalcVolume(cell.cell);
    const evecd reduced = Niggli::reduce_const(cell.lattice, cell.cell);
    cell.niggle_volume = TUnitCell::CalcVolume(reduced);
  }
  else if( Cmds.Count() == 1 )  {
    TTypeList<CellInfo> r = CellReader::read(Cmds[0])();
    if( r.Count() >= 1 )
      cell = r[0];
  }
  else if( Cmds.Count() == 7 )  {
    cell.cell[0] = Cmds[0].ToDouble();
    cell.cell[1] = Cmds[1].ToDouble();
    cell.cell[2] = Cmds[2].ToDouble();
    cell.cell[3] = Cmds[3].ToDouble();
    cell.cell[4] = Cmds[4].ToDouble();
    cell.cell[5] = Cmds[5].ToDouble();
    cell.lattice = TCLattice::LattForSymbol(Cmds[6].CharAt(0));
    cell.volume = TUnitCell::CalcVolume(cell.cell);
    cell.niggle_volume = TUnitCell::CalcVolume(
      Niggli::reduce_const(cell.lattice, cell.cell));
  }
  if( cell.volume == 0 )
    return TTypeList<Index::ResultEntry>();
  TBasicApp::NewLogEntry() << "Searching cell:";
  TBasicApp::NewLogEntry() << "  " << cell.cell.ToString() << ", "
    << TCLattice::SymbolForLatt(cell.lattice);
  TBasicApp::NewLogEntry() << "  Niggli cell: "
    << Niggli::reduce_const(cell.lattice, cell.cell).ToString();
  TBasicApp::NewLogEntry() << "  Cell volume: " << olxstr::FormatFloat(2, cell.volume);
  TBasicApp::NewLogEntry() << "  Reduced cell volume: " << olxstr::FormatFloat(2, cell.niggle_volume);
  return Index().LoadFromFile(index_name).Search(cell, vol_diff, true);
}
//.............................................................................
//.............................................................................
//.............................................................................
void Index::Search(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  const double vd = Options.FindValue('d', '1').ToDouble();
  Index::PrintResults(IndexSearch(DefaultIndex(), Cmds, vd)); 
}
//.............................................................................
void Index::Search(const TStrObjList &Params, TMacroError &E)  {
  Index::PrintResults(IndexSearch(DefaultIndex(), Params, 1)); 
}
//.............................................................................
void Index::Update(const TStrObjList &Params, TMacroError &E)  {
  const olxstr index = DefaultIndex();
  if( TEFile::Exists(index) )
    Index::Update(index);
  else  {
    if( Params.IsEmpty() )
      E.ProcessingError(__OlxSrcInfo, "please provide a folder name to create index from");
    else
      Index().Create(Params[0], index);
  }
}
//.............................................................................
TLibrary* Index::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("lcells") : name);
  lib->RegisterStaticFunction(
    new TStaticFunction(Index::Search, "Search", fpSeven|fpEight,
    "Searches given cell")
    );
  lib->RegisterStaticFunction(
    new TStaticFunction(Index::Update, "Update", fpOne|fpNone,
    "Updates index from given/original folder")
    );
  lib->RegisterStaticMacro(
    new TStaticMacro(&Index::Search, "Search", "d-deviation [1 A^3]", fpNone|fpOne|fpTwo|fpSeven,
    "Searches current cell, cell from given file, or given cell")
  );
  return lib;
}
//.............................................................................
