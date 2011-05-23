#include "lcells.h"
#include "symmparser.h"
#include "symspace.h"

using namespace lcells;

olx_object_ptr<TTypeList<CellInfo> > CellReader::read(const olxstr &fn)  {
  TStrList lines;
  lines.LoadFromFile(fn);
  olx_object_ptr<TTypeList<CellInfo> > r_(new TTypeList<CellInfo>);
  TTypeList<CellInfo> &rvl = r_;
  if( TEFile::ExtractFileExt(fn).Equalsi("cif") )  {
    cif_dp::TCifDP cif;
    cif.LoadFromStrings(lines);
    for( size_t db=0; db < cif.Count(); db++ )    {
      cif_dp::CifBlock& block = cif[db];
      try {
        olxstr l = GetCifParamAsString(block, "_symmetry_space_group_name_h-m").ToLowerCase();
        char latt = 0;
        if( l.IsEmpty() )  {  // go hard way then...
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
          rv.cell[0] = GetCifParamAsString(block, "_cell_length_a").ToDouble();
          rv.cell[1] = GetCifParamAsString(block, "_cell_length_b").ToDouble();
          rv.cell[2] = GetCifParamAsString(block, "_cell_length_c").ToDouble();
          rv.cell[3] = GetCifParamAsString(block, "_cell_angle_alpha").ToDouble();
          rv.cell[4] = GetCifParamAsString(block, "_cell_angle_beta").ToDouble();
          rv.cell[6] = GetCifParamAsString(block, "_cell_angle_gamma").ToDouble();
          rv.lattice = latt;
          rv.volume = TUnitCell::CalcVolume(rv.cell);
          rvl.AddCCopy(rv);
        }
      }
      catch(...)  {}
    }
  }
  else {
    CellInfo rv;
    size_t found_cnt = 0;
    for( size_t i=0; i < lines.Count(); i++ )  {
      if( lines[i].StartsFromi("cell") )  {
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
  return r_;
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
void Index::Create(const olxstr &folder)  {
  TFileTree ft(folder);
  TBasicApp::NewLogEntry() << "Searching files in '" << folder << "' - this may take a while...";
  TBasicApp::GetInstance().Update();
  ft.Expand();
  TStrList cif_files, ins_files, res_files;
  ft.GetRoot().ListFiles(cif_files, "*.cif");
  ft.GetRoot().ListFiles(ins_files, "*.ins");
  ft.GetRoot().ListFiles(res_files, "*.res");
  TBasicApp::NewLogEntry() << "Found:";
  TBasicApp::NewLogEntry() << "  " << ft.GetRoot().CountFiles("*.cif") << " CIF";
  TBasicApp::NewLogEntry() << "  " << ft.GetRoot().CountFiles("*.ins") << " INS";
  TBasicApp::NewLogEntry() << "  " << ft.GetRoot().CountFiles("*.res") << " RES";
  TBasicApp::NewLogEntry() << "  files";
  root.Init(ft.GetRoot());
  root.name = TEFile::UnixPath(folder);
  root.ToStream(olx_object_ptr<TEFile>(new TEFile("e:/1.tmp", "w+b"))());
}
//.............................................................................
void Index::FolderEntry::Init(const TFileTree::Folder &folder)  {
  entries.Clear();
  folders.Clear();
  name = folder.GetName();
  modified = folder.GetModificationTime();
  for( size_t i=0; i < folder.FileCount(); i++ )  {
    const TFileListItem &f = folder.GetFile(i);
    entries.Add(new FileEntry(*this, f.GetName(), f.GetModificationTime()));
  }
  for( size_t i=0; i < folder.FolderCount(); i++ )  {
    const TFileTree::Folder &f = folder.GetFolder(i);
    folders.Add(new FolderEntry(this, f.GetName(), f.GetModificationTime())).
      Init(f);
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
void Index::Search(TStrObjList &Params, const TParamList &Options, TMacroError &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//.............................................................................
void Index::Search(const TStrObjList &Params, TMacroError &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//.............................................................................
void Index::Update(const TStrObjList &Params, TMacroError &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
  if( Params.IsEmpty() )  {
    TBasicApp &app = TBasicApp::GetInstance();
    olxstr shared_dir = app.GetSharedDir();
  }
  else  {
    Index().Create(Params[0]);
  }
}
//.............................................................................
TLibrary* Index::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("lcells") : name);
  lib->RegisterStaticFunction(
    new TStaticFunction(Index::Search, "Search", fpEight,
    "Searches given cell")
    );
  lib->RegisterStaticFunction(
    new TStaticFunction(Index::Update, "Update", fpNone|fpOne,
    "Updates index from given/original folder")
    );
  //lib.RegisterStaticMacro(
  //  new TStaticMacro(&Index::Search, "Search", "", fpSix, "")
  //);
  return lib;
}
//.............................................................................
