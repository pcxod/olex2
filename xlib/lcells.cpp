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
#include "symmspace.h"
#include "math/composite.h"
#include "xapp.h"

using namespace lcells;

ConstArrayList<CellInfo> CellReader::read(const olxstr &fn)  {
  TStrList lines;
  lines.LoadFromFile(fn);
  TArrayList<CellInfo> rvl;
  const olxstr ext = TEFile::ExtractFileExt(fn).ToLowerCase();
  try {
    if( ext == "cif" )  {
      cif_dp::TCifDP cif;
      cif.LoadFromStrings(lines);
      for( size_t db=0; db < cif.Count(); db++ )    {
        cif_dp::CifBlock& block = cif[db];
        try {
          olxstr l = GetCifParamAsString(block,
            "_symmetry_space_group_name_h-m").ToLowerCase();
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
            rv.cell[0] =
              ToDouble(GetCifParamAsString(block, "_cell_length_a"));
            rv.cell[1] =
              ToDouble(GetCifParamAsString(block, "_cell_length_b"));
            rv.cell[2] =
              ToDouble(GetCifParamAsString(block, "_cell_length_c"));
            rv.cell[3] =
              ToDouble(GetCifParamAsString(block, "_cell_angle_alpha"));
            rv.cell[4] =
              ToDouble(GetCifParamAsString(block, "_cell_angle_beta"));
            rv.cell[5] =
              ToDouble(GetCifParamAsString(block, "_cell_angle_gamma"));
            rv.lattice = latt;
            rvl.Add(rv);
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
        rvl.Add(rv);
    }
    for( size_t i=0; i < rvl.Count(); i++ )  {
      rvl[i].volume = TUnitCell::CalcVolume(rvl[i].cell);
      rvl[i].niggli_volume = TUnitCell::CalcVolume(
        Niggli::reduce_const(rvl[i].lattice, rvl[i].cell));
    }
  }
  catch(const TExceptionBase &e)  {
    TBasicApp::NewLogEntry(logError) << fn << ": ";
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
  }
  return rvl;
}
//.............................................................................
double CellReader::ToDouble(const olxstr &str)  {
  const size_t i = str.FirstIndexOf('(');
  return (i == InvalidIndex ? str : str.SubStringTo(i)).ToDouble();
}
//.............................................................................
olxstr CellReader::GetCifParamAsString(const cif_dp::CifBlock &block,
  const olxstr &Param)
{
  using namespace cif_dp;
  IStringCifEntry* ce = dynamic_cast<IStringCifEntry*>(
    block.param_map.Find(Param, NULL));
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
        matrices.AddCopy(
          TSymmParser::SymmToMatrix(Loop->Get(i, sindex).GetStringValue()));
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
          matrices.AddCopy(
            TSymmParser::SymmToMatrix(Loop->Get(i, sindex).GetStringValue()));
        }
      }
    }
    if( matrices.IsEmpty() )  return 0;
    return SymmSpace::GetInfo(matrices).latt;
  }
  catch(const TExceptionBase &e)  {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
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
  TBasicApp::NewLogEntry() << "Searching files in '" << folder
    << "' - this may take a while...";
  TBasicApp::GetInstance().Update();
  ft.Expand(TFileTree::efSafe|TFileTree::efReport);
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
    throw TFunctionFailedException(__OlxSourceInfo,
      "File signarure mismatches");
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
olxstr Index::Update(const olxstr &index, const olxstr &root)  {
  if( !root.IsEmpty() && !TEFile::Exists(root) )  {
    TBasicApp::NewLogEntry() <<
      (olxstr("Skipping unavailable mounting point: ").quote() << root);
    return root;
  }
  Index ind;
  ind.LoadFromFile(index);
  if( !root.IsEmpty() )
    ind.root.name = root;
  ind.PrintInfo();
  size_t cnt = ind.root.TotalCount();
  TFileTree ft(ind.root.name);
  TBasicApp::NewLogEntry() << "Searching files in '" << ind.root.name
    << "' - this may take a while...";
  TBasicApp::GetInstance().Update();
  ft.Expand(TFileTree::efSafe|TFileTree::efReport);
  TBasicApp::NewLogEntry() << "Updated: " << ind.root.Update(ft.GetRoot())
    << " entries";
  ind.LastUpdated = TETime::EpochTime();
  ind.SaveToFile(index);
  size_t cnt1 = ind.root.TotalCount();
  if( cnt1 > cnt )
    TBasicApp::NewLogEntry() << "Added: " << cnt1-cnt << " new entries";
  return ind.root.name;
}
//.............................................................................
void Index::PrintInfo() const {
  TBasicApp::NewLogEntry() << "Last updated: "
    << TETime::FormatDateTime(LastUpdated);
  TBasicApp::NewLogEntry() << "Number of entries: " << root.TotalCount();
}
//.............................................................................
size_t Index::FolderEntry::Update(const TFileTree::Folder &folder)  {
  size_t updated_cnt = 0;
  QuickSorter::Sort(folders, Entry::NameComparator());
  QuickSorter::Sort(entries, Entry::NameComparator());
  ConstSlice<TTypeList<FileEntry> > file_entries(entries, 0, entries.Count());
  for( size_t i=0; i < folder.FileCount(); i++ )  {
    const TFileListItem &f = folder.GetFile(i);
    if( !ConsiderFile(f.GetName()) )  continue;
    const size_t fi = sorted::FindIndexOf(
      file_entries, Entry::NameComparator(), f.GetName());
    if( fi == InvalidIndex )  {
      TBasicApp::NewLogEntry() << "New file added: " << folder.GetFullPath()
        << f.GetName();
      entries.Add(new FileEntry(*this, f.GetName(), f.GetModificationTime()))
        .cells = CellReader::read(folder.GetFullPath()+f.GetName());
    }
    else  {
      if( entries[fi].modified != f.GetModificationTime() )  {
        updated_cnt++;
        TBasicApp::NewLogEntry() << "Updated file: " << folder.GetFullPath()
          << f.GetName();
        entries[fi].cells = CellReader::read(folder.GetFullPath()+f.GetName());
        entries[fi].modified = f.GetModificationTime();
      }
    }
  }
  ConstSlice<TTypeList<FolderEntry> >
    folder_entries(folders, 0, folders.Count());
  for( size_t i=0;  i < folder.FolderCount(); i++ )  {
    const TFileTree::Folder &f = folder.GetFolder(i);
    const size_t fi = sorted::FindIndexOf(
      folder_entries, Entry::NameComparator(), f.GetName());
    if( fi == InvalidIndex )  {
      folders.Add(new FolderEntry(this, f.GetName(), f.GetModificationTime()))
        .Init(f);
    }
    else
      updated_cnt += folders[fi].Update(f);
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
    if( !ConsiderFile(f.GetName()) )  continue;
    ConstArrayList<CellInfo> res =
      CellReader::read(folder.GetFullPath()+f.GetName());
    FileEntry &fe = entries.Add(
      new FileEntry(*this, f.GetName(), f.GetModificationTime()));
    fe.cells = res;
  }
  for( size_t i=0; i < folder.FolderCount(); i++ )  {
    const TFileTree::Folder &f = folder.GetFolder(i);
    folders.Add(new FolderEntry(this, f.GetName(), f.GetModificationTime()))
      .Init(f);
  }
}
//.............................................................................
ConstTypeList<Index::ResultEntry> Index::Search(const CellInfo &cell,
  double diff, bool filter_by_dimensions) const
{
  TTypeList<ResultEntry> all, res;
  all.SetCapacity(root.TotalCount());
  root.Expand(all);
  TEBitArray usage(all.Count());
/* alternative to use the usage flags would be something like this:
  res.QuickSorter.Sort<TComparableComparator>(res);
  ConstSlice<TTypeList<Index::ResultEntry>, ResultEntry>
  res_slice(res, 0, res.Count());
...
    if( sorted::FindIndexOf(res_slice, TComparableComparator(), all[j]) == InvalidIndex )
*/
  CellInfo to_search = cell;
  // search by niggli volume
  Niggli::reduce(to_search.lattice, to_search.cell);
  QuickSorter::Sort(all, CellInfo::ReducedVolumeComparator());
  const size_t ni = sorted::FindInsertIndex(
    all, CellInfo::ReducedVolumeComparator(), to_search);
  // go right
  for( size_t j=ni+1; j < all.Count(); j++ )  {
    if( olx_abs(all[j].niggli_volume-cell.niggli_volume) > diff )
      break;
    usage.SetTrue(j);
    res.AddCopy(all[j]);
  }
  // go left
  for( size_t j=ni; j != InvalidIndex; j-- )  {
    if( olx_abs(all[j].niggli_volume-cell.niggli_volume) > diff )
      break;
    usage.SetTrue(j);
    res.AddCopy(all[j]);
  }
  // search by cell volume
  QuickSorter::Sort(all, CellInfo::VolumeComparator(),
    SyncSwapListener::Make(usage));
  const size_t vi = sorted::FindInsertIndex(
    all, CellInfo::VolumeComparator(), to_search);
  // go right
  for( size_t j=vi+1; j < all.Count(); j++ )  {
    if( olx_abs(all[j].volume-cell.volume) > diff )
      break;
    if( !usage[j] )
      res.AddCopy(all[j]);
  }
  // go left
  for( size_t j=vi; j != InvalidIndex; j-- )  {
    if( olx_abs(all[j].volume-cell.volume) > diff )
      break;
    if( !usage[j] )
      res.AddCopy(all[j]);
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
void Index::PrintResults(const TTypeList<ResultEntry> &res)  {
  size_t cell_num=0;
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
    row1[1] << res[i].cell.ToString() << ", "
      << TCLattice::SymbolForLatt(res[i].lattice);
    row1[2] << olxstr::FormatFloat(2, res[i].volume) << "A^3";
    evecd rc = Niggli::reduce_const(res[i].lattice, res[i].cell);
    if( rc.QDistanceTo(res[i].cell) < 1e-6 )  continue;
    TStrList& row2 = tab.AddRow();
    row2[0] = "  Niggli cell";
    row2[1] << rc.ToString();
    row2[2] << olxstr::FormatFloat(2, res[i].niggli_volume) << "A^3";
  }
  TBasicApp::NewLogEntry() <<
    tab.CreateTXTList("Search results", true, false, ' ');
}
//.............................................................................
ConstTypeList<Index::ResultEntry> IndexManager::Search(const olxstr &cfg_name,
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
    cell.niggli_volume = TUnitCell::CalcVolume(reduced);
  }
  else if( Cmds.Count() == 1 )  {
    TArrayList<CellInfo> r = CellReader::read(Cmds[0]);
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
    cell.niggli_volume = TUnitCell::CalcVolume(
      Niggli::reduce_const(cell.lattice, cell.cell));
  }
  TTypeList<Index::ResultEntry> res;
  if( cell.volume == 0 )
    return res;
  TBasicApp::NewLogEntry() << "Searching cell:";
  TBasicApp::NewLogEntry() << "  " << cell.cell.ToString() << ", "
    << TCLattice::SymbolForLatt(cell.lattice);
  TBasicApp::NewLogEntry() << "  Niggli cell: "
    << Niggli::reduce_const(cell.lattice, cell.cell).ToString();
  TBasicApp::NewLogEntry() << "  Cell volume: "
    << olxstr::FormatFloat(2, cell.volume);
  TBasicApp::NewLogEntry() << "  Reduced cell volume: "
    << olxstr::FormatFloat(2, cell.niggli_volume);
  IndexManager im;
  im.LoadConfig(cfg_name);
  for( size_t i=0; i < im.indices.Count(); i++ )  {
    olxstr index_name;
    try  {
      if( !TEFile::Exists(im.indices[i].index_file_name) )  {
        TBasicApp::NewLogEntry(logError) <<
          (olxstr("Skipping incomplete index: ").quote()
            << im.indices[i].index_file_name);
        continue;
      }
      Index ind;
      ind.LoadFromFile(im.indices[i].index_file_name);
      if( !im.indices[i].root.IsEmpty() )  {
        if( !TEFile::Exists(im.indices[i].root) )
          TBasicApp::NewLogEntry(logInfo) <<
            (olxstr("Index root does not exist: ").quote()
              << im.indices[i].root);
        index_name = ind.root.name = im.indices[i].root;
      }
      else
        index_name = ind.root.name;
      res.AddList(ind.Search(cell, vol_diff, true));
    }
    catch(...)  {
      TBasicApp::NewLogEntry(logException) <<
        (olxstr("Failed to search: ").quote() << index_name);
    }
  }
  return res;
}
//.............................................................................
//.............................................................................
void IndexManager::LoadConfig(const olxstr &file_name)  {
  if( !TEFile::Exists(file_name) )  {
    if( TEFile::Exists(Index::DefaultIndex()) )  {
      Item &itm = indices.AddNew();
      itm.index_file_name = Index::DefaultIndex();
      itm.update = true;
    }
  }
  else {
    TDataFile df;
    df.LoadFromXLFile(file_name);
    TDataItem &root = df.Root().GetItemByName("indices");
    for( size_t i=0; i < root.ItemCount(); i++ )  {
      TDataItem &idx = root.GetItemByIndex(i);
      olxstr fn = idx.GetFieldByName("file");
      if( !TEFile::IsAbsolutePath(fn) )
        fn = TEFile::ExpandRelativePath(fn, TBasicApp::GetInstanceDir());
      olxstr root_dir = idx.GetFieldByName("root");
      if( !TEFile::Exists(fn) &&
          (root_dir.IsEmpty() || !TEFile::Exists(root_dir)))
      {
        TBasicApp::NewLogEntry(logError)
          << (olxstr("Missing index file removed: ").quote() << fn);
        continue;
      }
      Item &itm = indices.AddNew();
      itm.index_file_name = fn;
      itm.root = TEFile::UnixPathI(TEFile::TrimPathDelimeterI(root_dir));
      itm.update = idx.GetFieldByName("update").ToBool();
    }
  }
}
//.............................................................................
void IndexManager::SaveConfig(const olxstr &file_name) {
  if( indices.IsEmpty() )  return;
  try {
    TDataFile df;
    TDataItem &root = df.Root().AddItem("indices");
    for( size_t i=0; i < indices.Count(); i++ )  {
      root.AddItem(i+1)
        .AddField("file", TEFile::CreateRelativePath(
          indices[i].index_file_name, TBasicApp::GetInstanceDir()))
        .AddField("root", indices[i].root)
        .AddField("update", indices[i].update);
    }
    df.SaveToXLFile(file_name);
  }
  catch(...)  {
    TBasicApp::NewLogEntry() << "Failed to save configuration";
  }
}
//.............................................................................
void IndexManager::Update(const olxstr &cfg_name, const olxstr &folder_name,
  const olxstr &index_name)
{
  LoadConfig(cfg_name);
  SortedObjectList<olxstr, olxstrComparator<true> > roots;
  for( size_t i=0; i < indices.Count(); i++ )  {
    if( !indices[i].update ) continue;
    try  {
      if( TEFile::Exists(indices[i].index_file_name) )  {
        indices[i].root =
          Index::Update(indices[i].index_file_name, indices[i].root);
        roots.Add(TEFile::UnixPath(indices[i].root));
      }
    }
    catch(...)  {
      TBasicApp::NewLogEntry(logException) <<
        (olxstr("Failed to update: ").quote() << indices[i].index_file_name);
    }
  }
  if( !folder_name.IsEmpty() && TEFile::Exists(folder_name) &&
      !index_name.IsEmpty() )
  {
    if( roots.IndexOf(TEFile::UnixPath(folder_name)) != InvalidIndex )  {
      TBasicApp::NewLogEntry() <<
        (olxstr("Skipping duplicate mounting point: ").quote() << folder_name);
    }
    else  {
      Item &itm = indices.AddNew();
      itm.root = folder_name;
      itm.update = true;
      itm.index_file_name = index_name;
    }
  }
  for( size_t i=0; i < indices.Count(); i++ )  {
    if( !indices[i].update ) continue;
    try  {
      if( !TEFile::Exists(indices[i].index_file_name) )  {
        if( roots.IndexOf(TEFile::UnixPath(indices[i].root)) !=
            InvalidIndex )
        {
          TBasicApp::NewLogEntry() <<
            (olxstr("Skipping duplicate mounting point: ").quote()
              << indices[i].root);
          indices.NullItem(i);
          continue;
        }
        Index().Create(indices[i].root, indices[i].index_file_name);
      }
    }
    catch(const TExceptionBase &e)  {
      TBasicApp::NewLogEntry(logExceptionTrace) << e;
      TBasicApp::NewLogEntry(logException) <<
        (olxstr("Failed to create: ").quote() << indices[i].index_file_name);
    }
  }
  indices.Pack();
}
//.............................................................................
//.............................................................................
void IndexManager::Search(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  const double vd = Options.FindValue('d', '1').ToDouble();
  Index::PrintResults(Search(DefaultCfgName(), Cmds, vd)); 
}
//.............................................................................
void IndexManager::Search(const TStrObjList &Params, TMacroError &E)  {
  TTypeList<Index::ResultEntry> res = Search(DefaultCfgName(), Params, 1);
  if( !Params.IsEmpty() && Params[0].ToBool() )
    Index::PrintResults(res);
  olxstr_buf bf;
  olxstr cs = ',';
  for( size_t i=0; i < res.Count(); i++ )  {
    bf << res[i].file_name;
    if( i+1 < res.Count() )
      bf << cs;
  }
  E.SetRetVal<olxstr>(bf);
}
//.............................................................................
void IndexManager::Update(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  olxstr cfg_name, folder_name, index_name;
  if( Cmds.Count() == 0 )  {
    if( !(TEFile::Exists(DefaultCfgName()) ||
        TEFile::Exists(Index::DefaultIndex())) )
    {
      E.ProcessingError(__OlxSrcInfo, "Please provide a folder to index");
      return;
    }
    cfg_name = DefaultCfgName();
  }
  else if( Cmds.Count() == 1 )  {
    if( TEFile::IsDir(Cmds[0]) )  {
      if( TEFile::Exists(Index::DefaultIndex()) )  {
        E.ProcessingError(__OlxSrcInfo,
          "Default index already exists, please provide new index name");
        return;
      }
      cfg_name = DefaultCfgName();
      folder_name = Cmds[0];
      index_name = Index::DefaultIndex();
    }
    else if( !TEFile::Exists(Cmds[0]) )  {
      E.ProcessingError(__OlxSrcInfo, "Invalid configuration name: ").quote()
        << Cmds[0];
      return;
    }
    else
      cfg_name = Cmds[0]; 
  }
  else if( Cmds.Count() == 2 )  {
    if( TEFile::IsDir(Cmds[0]) )  {
      cfg_name = DefaultCfgName();
      folder_name = Cmds[0];
      index_name = Cmds[1];
    }
    else  {
      E.ProcessingError(__OlxSrcInfo, "A folder and index name are expected");
      return;
    }
  }
  else if( Cmds.Count() == 3 )  {
    cfg_name = Cmds[0];
    folder_name = Cmds[1];
    index_name = Cmds[2];
  }
  if( !index_name.IsEmpty() && !TEFile::IsAbsolutePath(index_name) )  {
    index_name =
      TEFile::ExpandRelativePath(index_name, TBasicApp::GetInstanceDir());
    if( !TEFile::IsAbsolutePath(index_name) )
      index_name = TBasicApp::GetInstanceDir() + index_name;
  }
  IndexManager im;
  im.Update(cfg_name, folder_name, index_name);
  im.SaveConfig(cfg_name);
}
//.............................................................................
TLibrary* IndexManager::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("lcells") : name);
  lib->Register(
    new TStaticFunction(&IndexManager::Search, "Search",
    fpNone|fpOne|fpTwo|fpSeven,
    "Searches given cell")
    );
  lib->Register(
    new TStaticMacro(&IndexManager::Update, "Update", EmptyString(),
    fpThree|fpTwo|fpOne|fpNone,
    "Updates/creates indices using default/given configuration."
    " To create an index, pass a folder name.")
    );
  lib->Register(
    new TStaticMacro(&IndexManager::Search, "Search", "d-deviation [1 A^3]",
    fpNone|fpOne|fpTwo|fpSeven,
    "Searches current cell, cell from given file, or given cell")
  );
  return lib;
}
//.............................................................................
