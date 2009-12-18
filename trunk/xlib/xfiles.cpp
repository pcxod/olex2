//---------------------------------------------------------------------------//
// namespace TXFiles: TBasicCFile - basic chemical file (INS, CIF, MOL, PDB, etc)
// TXFile - format independent crsytallographic file
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "xfiles.h"
#include "efile.h"
#include "xapp.h"
#include "unitcell.h"

#include "catom.h"
#include "library.h"

#include "ins.h"
#include "crs.h"
#include "utf8file.h"
#include "atomsort.h"
#include "infotab.h"

enum {
  XFILE_SG_Change,
  XFILE_UNIQ
};
//---------------------------------------------------------------------------
// TBasicCFile function bodies
//---------------------------------------------------------------------------
TBasicCFile::TBasicCFile() : RefMod(AsymmUnit), AsymmUnit(NULL)  {  
  AsymmUnit.SetRefMod(&RefMod);
}
//..............................................................................
TBasicCFile::~TBasicCFile()  {  
  RefMod.ClearAll();  // this must be called, as the AU might get destroyed beforehand and then AfixGroups cause crash
}
//..............................................................................
void TBasicCFile::SaveToFile(const olxstr& fn)  {
  TStrList L;
  SaveToStrings( L );
  TUtf8File::WriteLines(fn, L, false); 
  FileName = fn;
};
//..............................................................................
void TBasicCFile::LoadFromFile(const olxstr& fn)  {
  TEFile::CheckFileExists(__OlxSourceInfo, fn);
  TStrList L;
  L.LoadFromFile(fn);
  if( L.IsEmpty() )
    throw TEmptyFileException(__OlxSourceInfo, fn);
  FileName = fn;
  try  {  LoadFromStrings(L);  }
  catch( const TExceptionBase& exc )  {  
    FileName = EmptyString;
    throw TFunctionFailedException(__OlxSourceInfo, exc);  
  }
  FileName = fn;
  if( GetRM().GetHKLSource().IsEmpty() )  {
    olxstr src = TEFile::ChangeFileExt(fn, "hkl");
    if( !TEFile::Existsi(olxstr(src), src) )
      src = EmptyString;
    GetRM().SetHKLSource(src);
  }
  else  {
   /*
    if( !TEFile::Exists(FHKLSource) )
    {
      BasicApp->Log->Error(olxstr("TBasicCFile::HKL source file does not exist: ") += FHKLSource);
      FHKLSource = TEFile::ChangeFileExt(A, "hkl");
      if( !TEFile::Exists(FHKLSource) )
      {  FHKLSource = "";  }
      else
      {  BasicApp->Log->Error(olxstr("TBasicCFile::HKL source file reset to: ") += FHKLSource);  }
    }
    */
  }
}
//----------------------------------------------------------------------------//
// TXFile function bodies
//----------------------------------------------------------------------------//
TXFile::TXFile() : RefMod(Lattice.GetAsymmUnit()),
  OnFileLoad(Actions.New("XFILELOAD")),
  OnFileSave(Actions.New("XFILESAVE")),
  OnFileClose(Actions.New("XFILECLOSE"))
{
  Lattice.GetAsymmUnit().SetRefMod(&RefMod);
  Lattice.GetAsymmUnit().OnSGChange.Add(this, XFILE_SG_Change); 
  Lattice.OnStructureUniq.Add(this, XFILE_UNIQ);
  FLastLoader = NULL;
  FSG = NULL;
}
//..............................................................................
TXFile::~TXFile()  {
// finding uniq objects and deleting them
  for( size_t i=0; i < FileFormats.Count(); i++ )
    FileFormats.GetObject(i)->SetTag(i);
  for( size_t i=0; i < FileFormats.Count(); i++ )
    if( FileFormats.GetObject(i)->GetTag() == i )
      delete FileFormats.GetObject(i);
}
//..............................................................................
void TXFile::RegisterFileFormat(TBasicCFile *F, const olxstr &Ext)  {
  if( FileFormats.IndexOf(Ext) != InvalidIndex )
    throw TInvalidArgumentException(__OlxSourceInfo, "Ext");
  FileFormats.Add(olxstr::LowerCase(Ext), F);
}
//..............................................................................
TBasicCFile *TXFile::FindFormat(const olxstr &Ext)  {
  size_t i= FileFormats.IndexOf(olxstr::LowerCase(Ext));
  if( i == InvalidIndex )
    throw TInvalidArgumentException(__OlxSourceInfo, "unknown file format");
  return FileFormats.GetObject(i);
}
//..............................................................................
void TXFile::LastLoaderChanged() {
  if( FLastLoader == NULL )  {
    GetRM().ClearAll();
    GetLattice().Clear(true);
    return;
  }
  FSG = TSymmLib::GetInstance().FindSG(FLastLoader->GetAsymmUnit());
  OnFileLoad.Enter(this, &FLastLoader->GetFileName());
  GetRM().ClearAll();
  GetLattice().Clear(true);
  GetRM().Assign(FLastLoader->GetRM(), true);
  OnFileLoad.Execute(this);
  GetLattice().Init();
  OnFileLoad.Exit(this, &FLastLoader->GetFileName());
}
//..............................................................................
bool TXFile::Dispatch(int MsgId, short MsgSubId, const IEObject* Sender, const IEObject* Data) {
  if( MsgId == XFILE_SG_Change )  {
    if( Data == NULL || !EsdlInstanceOf(*Data, TSpaceGroup) )
      throw TInvalidArgumentException(__OlxSourceInfo, "space group");
    FSG = const_cast<TSpaceGroup*>( dynamic_cast<const TSpaceGroup*>(Data) );
  }
  else if( MsgId == XFILE_UNIQ && MsgSubId == msiEnter )  {
    //RefMod.Validate();
    //UpdateAsymmUnit();
    //GetAsymmUnit().PackAtoms();
    //if( !FLastLoader->IsNative() )  {
    //  FLastLoader->GetRM().Validate();
    //  FLastLoader->GetAsymmUnit().PackAtoms();
    //}
  }
  else
    return false;
  return true;
}
//..............................................................................
void TXFile::LoadFromFile(const olxstr & FN) {
  olxstr Ext( TEFile::ExtractFileExt(FN) );
  // this thows an exception if the file format loader does not exist
  TBasicCFile* Loader = FindFormat(Ext);
  bool replicated = false;
  if( FLastLoader == Loader )  {
    Loader = (TBasicCFile*)Loader->Replicate();
    replicated = true;
  }
  try  {  Loader->LoadFromFile(FN);  }
  catch( const TExceptionBase& exc )  {
    if( replicated )  
      delete Loader;
    throw TFunctionFailedException(__OlxSourceInfo, exc.Replicate() );
  }

  if( !Loader->IsNative() )  {
    OnFileLoad.Enter(this, &FN);
    try  {  GetRM().ClearAll();  }
    catch(const TExceptionBase& exc)  {
      TBasicApp::GetLog() << (olxstr("An error occured: ") << exc.GetException()->GetError());
    }
    GetLattice().Clear(true);
    GetRM().Assign(Loader->GetRM(), true);
    OnFileLoad.Execute(this);
    GetLattice().Init();
    OnFileLoad.Exit(this);
  }
  FSG = TSymmLib::GetInstance().FindSG(Loader->GetAsymmUnit());
  if( replicated )  {
    for( size_t i=0; i < FileFormats.Count(); i++ )
      if( FileFormats.GetObject(i) == FLastLoader )
        FileFormats.GetObject(i) = Loader;
    delete FLastLoader;
  }
  FLastLoader = Loader;
}
//..............................................................................
void TXFile::UpdateAsymmUnit()  {
  //TLattice *L = GetLattice();
  TBasicCFile* LL = FLastLoader;
  if( LL->IsNative() )
    return;
  GetLattice().UpdateAsymmUnit();
  LL->GetAsymmUnit().ClearEllps();
  for( size_t i=0; i < GetAsymmUnit().EllpCount(); i++ )
    LL->GetAsymmUnit().NewEllp() = GetAsymmUnit().GetEllp(i);
  for( size_t i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& CA = GetAsymmUnit().GetAtom(i);
    TCAtom& CA1 = LL->GetAsymmUnit().AtomCount() <= i ? 
      LL->GetAsymmUnit().NewAtom() : LL->GetAsymmUnit().GetAtom(i);
    CA1.Assign(CA);
  }
  LL->GetAsymmUnit().AssignResidues(GetAsymmUnit());
  RefMod.Validate();
  ValidateTabs();
  LL->GetRM().Assign(RefMod, false);
  LL->GetAsymmUnit().SetZ( GetAsymmUnit().GetZ() );
}
//..............................................................................
void TXFile::Sort(const TStrList& ins)  {
  if( FLastLoader == NULL )  return;
  if( !FLastLoader->IsNative() )
    UpdateAsymmUnit();
  TStrList labels;
  TCAtomPList &list = GetAsymmUnit().GetResidue(0).GetAtomList();
  size_t moiety_index = InvalidIndex, h_cnt=0, del_h_cnt = 0, free_h_cnt = 0;
  bool keeph = true;
  for( size_t i=0; i < list.Count(); i++ )  {
    if( list[i]->GetAtomInfo() == iHydrogenIndex || list[i]->GetAtomInfo() == iHydrogenIndex )  {
      if( !list[i]->IsDeleted() )  {
        h_cnt++;
        if( list[i]->GetParentAfixGroup() == NULL )
          free_h_cnt++;
      }
      else
        del_h_cnt++;
    }
  }
  if( h_cnt == 0 || del_h_cnt != 0 )  {
    keeph = false;
    if( del_h_cnt != 0 && free_h_cnt != 0 )
      TBasicApp::GetLog().Error("Hydrogen atoms, which are not attached using AFIX will not be kept with pivot atom until the file is reloaded");
  }
  try {
    AtomSorter::CombiSort cs;
    olxstr sort;
    for( size_t i=0; i < ins.Count(); i++ )  {
      if( ins[i].CharAt(0) == '+' )
        sort << ins[i].SubStringFrom(i);
      else if( ins[i].Equalsi("moiety") )  {
        moiety_index = i;
        break;
      }
      else
        labels.Add(ins[i]);
    }
    for( size_t i=0; i < sort.Length(); i++ )  {
      if( sort.CharAt(i) == 'm' )
        cs.sequence.Add(&AtomSorter::atom_cmp_Mw);
      else if( sort.CharAt(i) == 'l' )
        cs.sequence.Add(&AtomSorter::atom_cmp_Label);
      else if( sort.CharAt(i) == 'p' )
        cs.sequence.Add(&AtomSorter::atom_cmp_Part);
      else if( sort.CharAt(i) == 'h' )
        keeph = false;
    }
    if( !cs.sequence.IsEmpty() )
      AtomSorter::Sort(list, cs);
    if( !labels.IsEmpty() )  {
      AtomSorter::SortByName(list, labels);
      labels.Clear();
    }
    if( moiety_index != InvalidIndex )  {
      sort = EmptyString;
      if( moiety_index+1 < ins.Count() )  {
        for( size_t i=moiety_index+1; i < ins.Count(); i++ )  {
          if( ins[i].CharAt(0) == '+' )
            sort << ins[i].SubStringFrom(1);
          else
            labels.Add(ins[i]);
        }
        for( size_t i=0; i < sort.Length(); i++ )  {
          if( sort.CharAt(i) == 's' )
            MoietySorter::SortBySize(list);
          else if( sort.CharAt(i) == 'h' )
            MoietySorter::SortByHeaviestElement(list);
          else if( sort.CharAt(i) == 'm' )  
            MoietySorter::SortByWeight(list);
        }
        if( !labels.IsEmpty() )
          MoietySorter::SortByMoietyAtom(list, labels);
      }
      else
        MoietySorter::CreateMoieties(list);
    }
    if( keeph )
      AtomSorter::KeepH(list,GetLattice(), AtomSorter::atom_cmp_Label);
  }
  catch(const TExceptionBase& exc)  {
    TBasicApp::GetLog().Error( exc.GetException()->GetError() );
  }
  if( !FLastLoader->IsNative() )
    AtomSorter::SyncLists(list, FLastLoader->GetAsymmUnit().GetResidue(0).GetAtomList());
}
//..............................................................................
void TXFile::ValidateTabs()  {
  for( size_t i=0; i < RefMod.InfoTabCount(); i++ )  {
    if( RefMod.GetInfoTab(i).GetType() != infotab_htab )
      continue;
    if( RefMod.GetInfoTab(i).Count() != 2 )  // already invalid
      continue;
    TSAtom* sa = NULL;
    InfoTab& it = RefMod.GetInfoTab(i);
    for( size_t j=0; j < Lattice.AtomCount(); j++ )  {
      if( Lattice.GetAtom(j).CAtom().GetId() == it.GetAtom(0).GetAtom()->GetId() )  {
        sa = &Lattice.GetAtom(j);
        break;
      }
    }
    if( sa == NULL )  {
      RefMod.DeleteInfoTab(i--);
      continue;
    }
    bool hasH = false;
    for( size_t j=0; j < sa->NodeCount(); j++ )  {
      if( !sa->Node(j).IsDeleted() && 
        (sa->Node(j).GetAtomInfo() == iHydrogenIndex || sa->Node(j).GetAtomInfo() == iDeuteriumIndex) )  
      {
        hasH = true;
        break;
      }
    }
    if( !hasH )  {  
      TBasicApp::GetLog() << (olxstr("Removing HTAB (donor has no H atoms): ") << it.InsStr() << '\n');
      RefMod.DeleteInfoTab(i--);  
      continue;  
    }
    // validate the distance makes sense
    const TAsymmUnit& au = *it.GetAtom(0).GetAtom()->GetParent();
    vec3d v1 = it.GetAtom(0).GetAtom()->ccrd();
    if( it.GetAtom(0).GetMatrix() != NULL )
      v1  = *it.GetAtom(0).GetMatrix()*v1;
    vec3d v2 = it.GetAtom(1).GetAtom()->ccrd();
    if( it.GetAtom(1).GetMatrix() != NULL )
      v2  = *it.GetAtom(1).GetMatrix()*v2;
    const double dis = au.CellToCartesian(v1).DistanceTo(au.CellToCartesian(v2));
    if( dis > 5 )  {
      TBasicApp::GetLog() << (olxstr("Removing HTAB (d > 5A): ") << it.InsStr() << '\n');
      RefMod.DeleteInfoTab(i--);  
      continue;
    }
  }
}
//..............................................................................
void TXFile::SaveToFile(const olxstr& FN, bool Sort)  {
  olxstr Ext = TEFile::ExtractFileExt(FN);
  TBasicCFile *Loader = FindFormat(Ext);

  TBasicCFile *LL = FLastLoader;
  if( !Loader->IsNative() )  {
    if( LL != Loader ) {
      if( !Loader->Adopt(*this) )
        throw TFunctionFailedException(__OlxSourceInfo, "could not adopt specified file format");
    }
    else
      UpdateAsymmUnit();
    if( Sort )  
      Loader->GetAsymmUnit().Sort();
  }
  OnFileSave.Enter(this);
  IEObject* Cause = NULL;
  try  {  Loader->SaveToFile(FN);  }
  catch(const TExceptionBase& exc)  {
    Cause = exc.Replicate();
  }
  OnFileSave.Exit(this);
  if( Cause != NULL )  {
    throw TFunctionFailedException(__OlxSourceInfo, Cause);
  }
}
//..............................................................................
void TXFile::Close()  {
  OnFileClose.Enter(this, FLastLoader);
  FLastLoader = NULL;
  RefMod.ClearAll();
  Lattice.Clear(true);
  OnFileClose.Exit(this, NULL);
}
//..............................................................................
IEObject* TXFile::Replicate() const  {
  TXFile* xf = new TXFile;
  for( size_t i=0; i < FileFormats.Count(); i++ )  {
    xf->RegisterFileFormat( (TBasicCFile*)FileFormats.GetObject(i)->Replicate(), 
                              FileFormats[i] );
  }
  return xf;
}
//..............................................................................
void TXFile::EndUpdate()  {
  OnFileLoad.Enter(this, &GetFileName());
  OnFileLoad.Execute(this);
  // we keep the asymmunit but clear the unitcell
  GetLattice().Init();
  OnFileLoad.Exit(this);
}
//..............................................................................
void TXFile::ToDataItem(TDataItem& item) {
  GetLattice().ToDataItem(item.AddItem("Lattice"));
  GetRM().ToDataItem(item.AddItem("RefModel"));
}
//..............................................................................
void TXFile::FromDataItem(TDataItem& item) {
  GetRM().ClearAll();
  GetLattice().FromDataItem( item.FindRequiredItem("Lattice"));
  GetRM().FromDataItem(item.FindRequiredItem("RefModel"));
  //if( FLastLoader != NULL )  {
  //  FLastLoader->
  //}
}
//..............................................................................
//..............................................................................
//..............................................................................
void TXFile::LibGetFormula(const TStrObjList& Params, TMacroError& E)  {
  bool list = false, html = false;
  int digits = -1;
  if( Params.Count() > 0 )  {
    if( Params[0].Equalsi("list") )
      list = true;
    else if( Params[0].Equalsi("html") )
      html = true;
  }
  if( Params.Count() == 2 )
    digits = Params[1].ToInt();

  const ContentList& content = GetRM().GetUserContent();
  olxstr rv;
  for( size_t i=0; i < content.Count(); i++) {
    rv << content[i].GetA().SubStringTo(1).UpperCase();
    rv << content[i].GetA().SubStringFrom(1).LowerCase();
    if( list )  rv << ':';
    bool subAdded = false;
    const double dv = content[i].GetB()/GetAsymmUnit().GetZ();
    olxstr tmp = (digits > 0) ? olxstr::FormatFloat(digits, dv) : olxstr(dv);
    if( tmp.IndexOf('.') != InvalidIndex )
      tmp.TrimFloat();
    if( html )  {
      if( olx_abs(dv-1) > 0.01 && olx_abs(dv) > 0.01 )  {
        rv << "<sub>" << tmp;
        subAdded = true;
      }
    }
    else
      rv << tmp;

    if( (i+1) <  content.Count() )  {
      if( list )
        rv << ',';
      else
        if( html )  {
          if( subAdded )
            rv << "</sub>";
        }
        else
          rv << ' ';
    }
    else  // have to close the html tag
      if( html && subAdded )
        rv << "</sub>";

  }
  E.SetRetVal(rv);
}
//..............................................................................
void TXFile::LibSetFormula(const TStrObjList& Params, TMacroError& E) {
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  if( Params[0].IndexOf(':') == InvalidIndex )
    GetRM().SetUserFormula(Params[0]);
  else  {
    ContentList content;
    TStrList toks(Params[0], ',');
    for( size_t i=0; i < toks.Count(); i++ )  {
      size_t ind = toks[i].FirstIndexOf(':');
      if( ind == InvalidIndex )  {
        E.ProcessingError(__OlxSrcInfo, "invalid formula syntax" );
        return;
      }
      content.AddNew(toks[i].SubStringTo(ind), toks[i].SubStringFrom(ind+1).ToDouble()*GetAsymmUnit().GetZ());
    }
    if( content.IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "empty SFAC - check formula syntax");
      return;
    }
    GetRM().SetUserContent(content);
  }
}
//..............................................................................
void TXFile::LibEndUpdate(const TStrObjList& Params, TMacroError& E)  {
  EndUpdate();
}
//..............................................................................
void TXFile::LibSaveSolution(const TStrObjList& Params, TMacroError& E)  {
  TIns* oins = (TIns*)FLastLoader;
  TIns ins;
  UpdateAsymmUnit();  // needs to be called to assign the loaderIds for new atoms
  ins.GetRM().Assign( GetRM(), true );
  ins.AddIns("FMAP 2", ins.GetRM());
  ins.GetRM().SetRefinementMethod("L.S.");
  ins.GetRM().SetIterations(4);
  ins.GetRM().SetPlan(20);
  ins.GetRM().SetUserContent(oins->GetRM().GetUserContent());
  ins.SaveToFile(Params[0]);
}
//..............................................................................
TLibrary*  TXFile::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("xf") : name );
  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this,  &TXFile::LibGetFormula, "GetFormula", fpNone|fpOne|fpTwo|psCheckFileTypeIns,
"Returns a string for content of the asymmetric unit. Takes single or none parameters.\
 If parameter equals 'html' and html formatted string is returned, for 'list' parameter\
 a string like 'C:26,N:45' is returned. If no parameter is specified, just formula is returned") );

  lib->RegisterFunction<TXFile>( new TFunction<TXFile>(this,  &TXFile::LibSetFormula, "SetFormula", fpOne|psCheckFileTypeIns,
"Sets formula for current file, takes a string of the following form 'C:25,N:4'") );

  lib->RegisterFunction<TXFile>( new TFunction<TXFile>(this,  &TXFile::LibEndUpdate, "EndUpdate", fpNone|psCheckFileTypeIns,
"Must be called after the content of the asymmetric unit has changed - this function will\
 update the program state") );
  lib->RegisterFunction<TXFile>( new TFunction<TXFile>(this,  &TXFile::LibSaveSolution, "SaveSolution", fpOne|psCheckFileTypeIns,
"Saves current Q-peak model to provided file (res-file)") );
  lib->AttachLibrary(Lattice.GetAsymmUnit().ExportLibrary());
  lib->AttachLibrary(Lattice.GetUnitCell().ExportLibrary());
  lib->AttachLibrary(Lattice.ExportLibrary());
  lib->AttachLibrary(RefMod.expl.ExportLibrary());
  return lib;
}
//..............................................................................

