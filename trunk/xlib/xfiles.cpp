//---------------------------------------------------------------------------//
// namespace TXFiles: TBasicCFile - basic chemical file (INS, CIF, MOL, PDB, etc)
// TXFile - format independent crsytallographic file
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

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

//---------------------------------------------------------------------------
// TBasicCFile function bodies
//---------------------------------------------------------------------------
TBasicCFile::TBasicCFile() : RefMod(AsymmUnit), AsymmUnit(NULL)  {  
  AsymmUnit.SetRefMod(&RefMod);
}
//..............................................................................
TBasicCFile::~TBasicCFile()  {  }
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
  L.LoadFromFile( fn );
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
    if( !TEFile::FileExists(src) )
      src = EmptyString;
    GetRM().SetHKLSource(src);
  }
  else  {
   /*
    if( !TEFile::FileExists(FHKLSource) )
    {
      BasicApp->Log->Error(olxstr("TBasicCFile::HKL source file does not exist: ") += FHKLSource);
      FHKLSource = TEFile::ChangeFileExt(A, "hkl");
      if( !TEFile::FileExists(FHKLSource) )
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
TXFile::TXFile() : RefMod(Lattice.GetAsymmUnit())  {
  Lattice.GetAsymmUnit().SetRefMod(&RefMod);
  AActionHandler::SetToDelete(false);
  Lattice.GetAsymmUnit().OnSGChange->Add(this); 
  OnFileLoad = &Actions.NewQueue("XFILELOAD");
  OnFileSave = &Actions.NewQueue("XFILESAVE");
  FLastLoader = NULL;
  FSG = NULL;
}
//..............................................................................
TXFile::~TXFile()  {
// finding uniq objects and deleting them
  for( int i=0; i < FileFormats.Count(); i++ )
    FileFormats.Object(i)->SetTag(i);
  for( int i=0; i < FileFormats.Count(); i++ )
    if( FileFormats.Object(i)->GetTag() == i )
      delete FileFormats.Object(i);
}
//..............................................................................
void TXFile::RegisterFileFormat(TBasicCFile *F, const olxstr &Ext)  {
  if( FileFormats.IndexOf(Ext) >=0 )
    throw TInvalidArgumentException(__OlxSourceInfo, "Ext");
  FileFormats.Add(olxstr::LowerCase(Ext), F);
}
//..............................................................................
TBasicCFile *TXFile::FindFormat(const olxstr &Ext)  {
  int i= FileFormats.IndexOf(olxstr::LowerCase(Ext));
  if( i == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "unknown file format");
  return FileFormats.Object(i);
}
//..............................................................................
void TXFile::LastLoaderChanged() {
  if( FLastLoader == NULL )  return;
  FSG = TSymmLib::GetInstance()->FindSG(FLastLoader->GetAsymmUnit());
  OnFileLoad->Enter(this);
  GetRM().ClearAll();
  GetLattice().Clear(true);
  GetRM().Assign(FLastLoader->GetRM(), true);
  GetLattice().Init();
  OnFileLoad->Exit(this);
}
//..............................................................................
bool TXFile::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( Data == NULL || !EsdlInstanceOf(*Data, TSpaceGroup) )
    throw TInvalidArgumentException(__OlxSourceInfo, "space group");
  FSG = const_cast<TSpaceGroup*>( dynamic_cast<const TSpaceGroup*>(Data) );
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
    OnFileLoad->Enter(this, &FN);
    GetRM().ClearAll();
    GetLattice().Clear(true);
    GetRM().Assign(Loader->GetRM(), true);
    GetLattice().Init();
    OnFileLoad->Exit(this);
  }
  FSG = TSymmLib::GetInstance()->FindSG(Loader->GetAsymmUnit());
  if( replicated )  {
    for( int i=0; i < FileFormats.Count(); i++ )
      if( FileFormats.Object(i) == FLastLoader )
        FileFormats.Object(i) = Loader;
    delete FLastLoader;
  }

  FLastLoader = Loader;
}
//..............................................................................
void TXFile::UpdateAsymmUnit()  {
  //TLattice *L = GetLattice();
  TBasicCFile* LL = FLastLoader;
  GetLattice().UpdateAsymmUnit();
  LL->GetAsymmUnit().ClearResidues(false);
  LL->GetAsymmUnit().ClearEllps();
  for( int i=0; i < GetAsymmUnit().EllpCount(); i++ )
    LL->GetAsymmUnit().NewEllp() = GetAsymmUnit().GetEllp(i);
  for( int i=0; i < GetAsymmUnit().ResidueCount(); i++ )  {
    TAsymmUnit::TResidue& resi = GetAsymmUnit().GetResidue(i);
    LL->GetAsymmUnit().NewResidue(resi.GetClassName(), resi.GetNumber(), resi.GetAlias() );
  }
  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& CA = GetAsymmUnit().GetAtom(i);
    TCAtom& CA1 = LL->GetAsymmUnit().AtomCount() <= i ? 
      LL->GetAsymmUnit().NewAtom() : LL->GetAsymmUnit().GetAtom(i);
    CA1.Assign(CA);
    //LL->GetAsymmUnit().GetResidue(CA.GetResiId()).AddAtom(&CA1);
  }
  //keep the resudue atoms order
  for( int i=-1; i < GetAsymmUnit().ResidueCount(); i++ )  {
    const TAsymmUnit::TResidue& resi = GetAsymmUnit().GetResidue(i);
    TAsymmUnit::TResidue& resi1 = LL->GetAsymmUnit().GetResidue(i);
    for( int j=0; j < resi.Count(); j++ )  
      resi1.AddAtom( &LL->GetAsymmUnit().GetAtom(resi[j].GetId()) );
  }
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
  TCAtomPList &list = GetAsymmUnit().GetResidue(-1).AtomList();
  int moety_index = -1;
  bool keeph = true;
  try {
    for( int i=0; i < ins.Count(); i++ )  {
      if( ins[i].Comparei("Mw") == 0 )
        AtomSorter::Sort(list, AtomSorter::atom_cmp_Mw);
      else if( ins[i].Comparei("l") == 0 )
        AtomSorter::Sort(list, AtomSorter::atom_cmp_Label);
      else if( ins[i].Comparei("MwL") == 0 )
        AtomSorter::Sort(list, AtomSorter::atom_cmp_Mw_Label);
      else if( ins[i].Comparei("l1") == 0 )
        AtomSorter::Sort(list, AtomSorter::atom_cmp_Label1);
      else if( ins[i].Comparei("h-") == 0 )
        keeph = false;
      else if( ins[i].Comparei("moety") == 0 )  {
        moety_index = i;
        break;
      }
      else
        labels.Add(ins[i]);
    }
    if( !labels.IsEmpty() )  {
      AtomSorter::SortByName(list, labels);
      labels.Clear();
    }
    if( moety_index != -1 )  {
      if( moety_index+1 < ins.Count() )  {
        for( int i=moety_index+1; i < ins.Count(); i++ )  {
          if( ins[i].Comparei("s") == 0 )  
            MoetySorter::SortBySize(list);
          else if( ins[i].Comparei("h") == 0 )  
            MoetySorter::SortByHeaviestElement(list);
          else if( ins[i].Comparei("Mw") == 0 )  
            MoetySorter::SortByWeight(list);
          else
            labels.Add(ins[i]);
        }
        if( !labels.IsEmpty() )
          MoetySorter::SortByMoetyAtom(list, labels);
      }
      else
        MoetySorter::CreateMoeties(list);
    }
    if( keeph )
      AtomSorter::KeepH(list,GetLattice(),  NULL);
  }
  catch(const TExceptionBase& exc)  {
    TBasicApp::GetLog().Error( exc.GetException()->GetError() );
  }
  if( !FLastLoader->IsNative() )
    AtomSorter::SyncLists(list, FLastLoader->GetAsymmUnit().GetResidue(-1).AtomList());
}
//..............................................................................
void TXFile::ValidateTabs()  {
  for( int i=0; i < RefMod.InfoTabCount(); i++ )  {
    if( RefMod.GetInfoTab(i).GetType() != infotab_htab )
      continue;
    if( RefMod.GetInfoTab(i).Count() != 2 )  // already invalid
      continue;
    TSAtom* sa = NULL;
    for( int j=0; j < Lattice.AtomCount(); j++ )  {
      if( Lattice.GetAtom(j).CAtom().GetId() == RefMod.GetInfoTab(i).GetAtom(0).GetAtom()->GetId() )  {
        sa = &Lattice.GetAtom(j);
        break;
      }
    }
    if( sa == NULL )  {
      RefMod.DeleteInfoTab(i--);
      continue;
    }
    bool hasH = false;
    for( int j=0; j < sa->NodeCount(); j++ )  {
      if( sa->Node(j).GetAtomInfo() == iHydrogenIndex || sa->Node(j).GetAtomInfo() == iDeuteriumIndex )  {
        hasH = true;
        break;
      }
    }
    if( !hasH )  {
      RefMod.DeleteInfoTab(i--);
      continue;
    }
  }
}
//..............................................................................
void TXFile::SaveToFile(const olxstr &FN, bool Sort)  {
  olxstr Ext = TEFile::ExtractFileExt(FN);
  TBasicCFile *Loader = FindFormat(Ext);

  TBasicCFile *LL = FLastLoader;
  if( !Loader->IsNative() )  {
    if( LL != Loader ) {
      if( !Loader->Adopt(this) )
        throw TFunctionFailedException(__OlxSourceInfo, "could not adopt specified file format");
    }
    else
      UpdateAsymmUnit();
    if( Sort )  
      Loader->GetAsymmUnit().Sort();
  }
  OnFileSave->Enter(this);
  IEObject* Cause = NULL;
  try  {  Loader->SaveToFile(FN);  }
  catch( const TExceptionBase &exc )  {
    Cause = exc.Replicate();
  }
  OnFileSave->Exit(this);
  if( Cause != NULL )  {
    throw TFunctionFailedException(__OlxSourceInfo, Cause);
  }
}
//..............................................................................
IEObject* TXFile::Replicate() const  {
  TXFile* xf = new TXFile;
  for( int i=0; i < FileFormats.Count(); i++ )  {
    xf->RegisterFileFormat( (TBasicCFile*)FileFormats.Object(i)->Replicate(), 
                              FileFormats.String(i) );
  }
  return xf;
}
//..............................................................................
void TXFile::EndUpdate()  {
  OnFileLoad->Enter(this);
  // we keep the asymmunit but clear the unitcell
  GetLattice().Clear(false);
  GetLattice().GetUnitCell().Clear();
  GetLattice().Init();
  OnFileLoad->Exit(this);
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
  if( FLastLoader == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "no file is loaded");
    E.SetRetVal( E.GetInfo() );
    return;
  }
  olxstr as, us;
  if( EsdlInstanceOf( *FLastLoader, TIns) )  {
    TIns* ins = (TIns*)FLastLoader;
    as = ins->GetSfac();
    us = ins->GetUnit();
  }
  else if( EsdlInstanceOf( *FLastLoader, TCRSFile) )  {
    TCRSFile* crs = (TCRSFile*)FLastLoader;
    as = crs->GetSfac();
    us = crs->GetUnit();
  }
  else  {
    TStrPObjList<olxstr,TBasicAtomInfo*> sl;
    GetAsymmUnit().SummFormula(sl, as, us, false);
  }
  bool list = false, html = false;
  int digits = -1;
  if( Params.Count() > 0 )  {
    if( Params.String(0).Comparei("list") == 0 )
      list = true;
    else if( Params.String(0).Comparei("html") == 0 )
      html = true;
  }
  if( Params.Count() == 2 )
    digits = Params[1].ToInt();

  TCStrList atoms(as, ' '),
           units(us, ' ');
  olxstr rv, tmp;
  int len = olx_min( atoms.Count(), units.Count() );
  for( int i=0; i < len; i++) {
    rv << atoms[i].SubStringTo(1).UpperCase();
    rv << atoms[i].SubStringFrom(1).LowerCase();
    if( list )
      rv << ':';

    bool subAdded = false;
    double dv = units[i].ToDouble()/GetAsymmUnit().GetZ();
    tmp = (digits > 0) ? olxstr::FormatFloat(digits, dv) : olxstr(dv);
    if( tmp.IndexOf('.') != -1 )
      tmp.TrimFloat();
    if( html )  {
      if( olx_abs(dv-1) > 0.01 && olx_abs(dv) > 0.01 )  {
        rv << "<sub>" << tmp;
        subAdded = true;
      }
    }
    else
      rv << tmp;

    if( (i+1) < len )  {
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
  E.SetRetVal( rv );
}
//..............................................................................
void TXFile::LibSetFormula(const TStrObjList& Params, TMacroError& E) {
  if( FLastLoader == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "no file is loaded");
    E.SetRetVal( E.GetInfo() );
    return;
  }
  if( !EsdlInstanceOf( *FLastLoader, TIns) )  {
    E.ProcessingError(__OlxSrcInfo, "operation only valid for ins files");
    E.SetRetVal( E.GetInfo() );
    return;
  }
  TIns* ins = (TIns*)FLastLoader;
  olxstr Sfac, Unit;
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  if( Params[0].IndexOf(':') == -1 )  {
    TTypeList<AnAssociation2<olxstr, int> > res;
    AtomsInfo.ParseElementString( Params[0], res);
    if( res.IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "empty formula is not allowed" );
      return;
    }
    for( int i=0; i < res.Count(); i++ )  {
      Sfac << res[i].GetA();
      Unit << res[i].GetB()*GetAsymmUnit().GetZ();
      if( (i+1) < res.Count() )  {
        Sfac << ' ';
        Unit << ' ';
      }
    }
  }
  else  {
    TCStrList toks(Params[0], ',');
    for( int i=0; i < toks.Count(); i++ )  {
      int ind = toks.String(i).FirstIndexOf(':');
      if( ind == -1 )  {
        E.ProcessingError(__OlxSrcInfo, "invalid formula syntax" );
        return;
      }
      Sfac << toks[i].SubStringTo(ind);
      Unit << toks[i].SubStringFrom(ind+1).ToDouble()*GetAsymmUnit().GetZ();
      if( (i+1) < toks.Count() )  {
        Sfac << ' ';
        Unit << ' ';
      }
    }
    if( !Sfac.Length() )  {
      E.ProcessingError(__OlxSrcInfo, "empty SFAC - check formula syntax");
      return;
    }
  }
  ins->SetSfac( Sfac );
  ins->SetUnit( Unit );
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
  ins.SetSfac( oins->GetSfac());
  ins.SetUnit( oins->GetUnit() );
  ins.SaveToFile( Params[0] );
}
//..............................................................................
TLibrary*  TXFile::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("xf") : name );
  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this,  &TXFile::LibGetFormula, "GetFormula", fpNone|fpOne|fpTwo,
"Returns a string for content of the asymmetric unit. Takes single or none parameters.\
 If parameter equals 'html' and html formatted string is returned, for 'list' parameter\
 a string like 'C:26,N:45' is returned. If no parameter is specified, just formula is returned") );

  lib->RegisterFunction<TXFile>( new TFunction<TXFile>(this,  &TXFile::LibSetFormula, "SetFormula", fpOne,
"Sets formula for current file, takes a string of the following form 'C:25,N:4'") );

  lib->RegisterFunction<TXFile>( new TFunction<TXFile>(this,  &TXFile::LibEndUpdate, "EndUpdate", fpNone,
"Must be called after the content of the asymmetric unit has changed - this function will\
 update the program state") );
  lib->RegisterFunction<TXFile>( new TFunction<TXFile>(this,  &TXFile::LibSaveSolution, "SaveSolution", fpOne|psCheckFileTypeIns,
"Saves current Q-peak model to provided file (res-file)") );
  lib->AttachLibrary( Lattice.GetAsymmUnit().ExportLibrary() );
  lib->AttachLibrary( Lattice.GetUnitCell().ExportLibrary() );
  lib->AttachLibrary( Lattice.ExportLibrary() );
  return lib;
}
//..............................................................................

