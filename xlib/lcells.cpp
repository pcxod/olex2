#include "lcells.h"
#include "symmparser.h"
#include "symspace.h"
#include "math/composite.h"

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
    else if( ext == "ins" || ext == "res" || ext == "p4p") {
      CellInfo rv;
      size_t found_cnt = 0;
      for( size_t i=0; i < lines.Count(); i++ )  {
        if( lines[i].Equalsi("cell") )  {
          TStrList toks(lines[i], ' ');
          if( toks.Count() == 8 )  {
            rv.cell[0] = toks[2].ToDouble();
            rv.cell[1] = toks[3].ToDouble();
            rv.cell[2] = toks[4].ToDouble();
            rv.cell[3] = toks[5].ToDouble();
            rv.cell[4] = toks[6].ToDouble();
            rv.cell[5] = toks[7].ToDouble();
            found_cnt ++;
          }
        }
        else if( lines[i].StartsFromi("latt") )  {
          TStrList toks(lines[i], ' ');
          if( toks.Count() == 2 )  {
            rv.lattice = toks[1].ToInt();
            found_cnt ++;
          }
        }
        if( found_cnt == 2 )  {
          rvl.AddCCopy(rv);
          return r_;
        }
      }
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
    masks.Add("p4p");
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
  SaveToFile(index_name);
}
//.............................................................................
void Index::SaveToFile(const olxstr &file_name) const {
  TEFile f(file_name + ".tmp", "w+b");
  f.Write(&file_sig[0], 3);
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
  TFileTree ft(ind.root.name);
  TBasicApp::NewLogEntry() << "Searching files in '" << ind.root.name << "' - this may take a while...";
  TBasicApp::GetInstance().Update();
  ft.Expand();
  ind.root.Update(ft.GetRoot());
  ind.SaveToFile(index);
}
//.............................................................................
void Index::FolderEntry::Update(const TFileTree::Folder &folder)  {
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
TTypeList<Index::ResultEntry> Index::Search(const CellInfo &cell) const {
  const double diff = 5;
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
  //
  res.QuickSorter.Sort<TComparablePtrComparator>(res);
  ConstSlice<TTypeList<Index::ResultEntry>, ResultEntry> res_slice(res, 0, res.Count());
  // search by cell volume
  all.QuickSorter.Sort<CellInfo::VolumeComparator>(all);
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
  return res;
}
//.............................................................................
//.............................................................................
//.............................................................................
void Index::Search(TStrObjList &Params, const TParamList &Options, TMacroError &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//.............................................................................
void Index::Search(const TStrObjList &Params, TMacroError &E)  {
  olxstr index = TBasicApp::GetSharedDir() + "lcells.ind";
  if( Params.Count() == 8 )
    index = Params[7];

  CellInfo cell;
  cell.cell[0] = Params[0].ToDouble();
  cell.cell[1] = Params[1].ToDouble();
  cell.cell[2] = Params[2].ToDouble();
  cell.cell[3] = Params[3].ToDouble();
  cell.cell[4] = Params[4].ToDouble();
  cell.cell[5] = Params[5].ToDouble();
  cell.volume = TUnitCell::CalcVolume(cell.cell);
  cell.niggle_volume = TUnitCell::CalcVolume(
    Niggli::reduce_const(cell.lattice, cell.cell));
  TBasicApp::NewLogEntry() << "Searching cell:";
  TBasicApp::NewLogEntry() << "  " << cell.cell.ToString() << ", "
    << TCLattice::SymbolForLatt(cell.lattice);
  TBasicApp::NewLogEntry() << "  Cell volume: " << olx_round(cell.volume, 100);
  TBasicApp::NewLogEntry() << "  Reduced cell volume: " << olx_round(cell.niggle_volume, 100);
  int latt_sig = 1;
  olxch latt = Params[6].CharAt(0);
  if( Params[6].Length() == 2 )  {
    latt_sig = Params[6].CharAt(0) == '-' ? -1 : 1;
    latt = Params[6].CharAt(1);
  }
  cell.lattice = TCLattice::LattForSymbol(latt)*latt_sig;
  TTypeList<ResultEntry> res = Index().LoadFromFile(index).Search(cell);
  
  for( size_t i=0; i < res.Count(); i++ )  {
    TBasicApp::NewLogEntry() << olxstr(i+1).Format(2, false, ' ') << ". " << res[i].file_name;
    TBasicApp::NewLogEntry() << "  " << res[i].cell.ToString()
      << ", " << TCLattice::SymbolForLatt(res[i].lattice);
    TBasicApp::NewLogEntry() << "  Cell volume: " << olxstr::FormatFloat(2, res[i].volume);
    if( olx_abs(res[i].volume -res[i].niggle_volume) > 1.e-3 )
      TBasicApp::NewLogEntry() << "  Reduced cell volume: " << olxstr::FormatFloat(2, res[i].niggle_volume);
  }
}
//.............................................................................
void Index::Update(const TStrObjList &Params, TMacroError &E)  {
  olxstr index = TBasicApp::GetSharedDir() + "lcells.ind";
  if( Params.Count() == 2 )
    index = Params[1];
  if( TEFile::Exists(index) )
    Index::Update(index);
  else
    Index().Create(Params[0], index);
}
//.............................................................................
TLibrary* Index::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("lcells") : name);
  lib->RegisterStaticFunction(
    new TStaticFunction(Index::Search, "Search", fpSeven|fpEight,
    "Searches given cell")
    );
  lib->RegisterStaticFunction(
    new TStaticFunction(Index::Update, "Update", fpOne|fpTwo,
    "Updates index from given/original folder")
    );
  //lib.RegisterStaticMacro(
  //  new TStaticMacro(&Index::Search, "Search", "", fpSix, "")
  //);
  return lib;
}
//.............................................................................
