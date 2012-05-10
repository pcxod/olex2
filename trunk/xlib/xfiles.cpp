/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xfiles.h"
#include "efile.h"
#include "xapp.h"
#include "unitcell.h"
#include "catom.h"
#include "library.h"
#include "ins.h"
#include "crs.h"
#include "cif.h"
#include "utf8file.h"
#include "atomsort.h"
#include "infotab.h"
#include "absorpc.h"
#include "analysis.h"

enum {
  XFILE_SG_Change,
  XFILE_UNIQ
};

TBasicCFile::TBasicCFile() : RefMod(AsymmUnit), AsymmUnit(NULL)  {  
  AsymmUnit.SetRefMod(&RefMod);
}
//..............................................................................
TBasicCFile::~TBasicCFile()  {  
  RefMod.Clear(rm_clear_ALL);  // this must be called, as the AU might get destroyed beforehand and then AfixGroups cause crash
}
//..............................................................................
void TBasicCFile::SaveToFile(const olxstr& fn)  {
  TStrList L;
  SaveToStrings(L);
  TUtf8File::WriteLines(fn, L, false); 
  FileName = fn;
};
//..............................................................................
void TBasicCFile::LoadFromFile(const olxstr& _fn)  {
  TXFile::NameArg file_n = TXFile::ParseName(_fn);  
  TEFile::CheckFileExists(__OlxSourceInfo, file_n.file_name);
  TStrList L;
  L.LoadFromFile(file_n.file_name);
  if( L.IsEmpty() )
    throw TEmptyFileException(__OlxSourceInfo, _fn);
  FileName = file_n.file_name;
  try  {
    LoadFromStrings(L);
    if( EsdlInstanceOf(*this, TCif) )  {
      if( !file_n.data_name.IsEmpty() ) {
        if( file_n.is_index )
          ((TCif*)this)->SetCurrentBlock(file_n.data_name.ToSizeT());
        else
          ((TCif*)this)->SetCurrentBlock(file_n.data_name);
      }
      else  {  // set first then
        ((TCif*)this)->SetCurrentBlock(InvalidIndex);
      }
    }
  }
  catch( const TExceptionBase& exc )  {  
    FileName.SetLength(0);
    throw TFunctionFailedException(__OlxSourceInfo, exc);  
  }
  /* fix labels for not native formats, will not help for FE1A, because it could come from
  Fe1A or from Fe1a ... */
  if( !IsNative() )  {   
    for( size_t i=0; i < AsymmUnit.AtomCount(); i++ )  {
      TCAtom& a = AsymmUnit.GetAtom(i);
      if( a.GetType().symbol.Length() == 2 && a.GetLabel().StartsFromi(a.GetType().symbol) )
        a.SetLabel(a.GetType().symbol + a.GetLabel().SubStringFrom(a.GetType().symbol.Length()), false);
    }
  }
  FileName = file_n.file_name;
}
//----------------------------------------------------------------------------//
// TXFile function bodies
//----------------------------------------------------------------------------//
TXFile::TXFile(ASObjectProvider& Objects) :
  Lattice(Objects),
  RefMod(Lattice.GetAsymmUnit()),
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
    if( (size_t)FileFormats.GetObject(i)->GetTag() == i )
      delete FileFormats.GetObject(i);
}
//..............................................................................
void TXFile::RegisterFileFormat(TBasicCFile *F, const olxstr &Ext)  {
  if( FileFormats.IndexOf(Ext) != InvalidIndex )
    throw TInvalidArgumentException(__OlxSourceInfo, "Ext");
  FileFormats.Add(Ext.ToLowerCase(), F);
}
//..............................................................................
TBasicCFile *TXFile::FindFormat(const olxstr &Ext)  {
  const size_t i = FileFormats.IndexOf(Ext.ToLowerCase());
  if( i == InvalidIndex )
    throw TInvalidArgumentException(__OlxSourceInfo, "unknown file format");
  return FileFormats.GetObject(i);
}
//..............................................................................
void TXFile::LastLoaderChanged() {
  if( FLastLoader == NULL )  {
    GetRM().Clear(rm_clear_ALL);
    GetLattice().Clear(true);
    return;
  }
  FSG = &TSymmLib::GetInstance().FindSG(FLastLoader->GetAsymmUnit());
  OnFileLoad.Enter(this, &FLastLoader->GetFileName());
  GetRM().Clear(rm_clear_ALL);
  GetLattice().Clear(true);
  GetRM().Assign(FLastLoader->GetRM(), true);
  OnFileLoad.Execute(this);
  GetLattice().Init();
  OnFileLoad.Exit(this, &FLastLoader->GetFileName());
}
//..............................................................................
bool TXFile::Dispatch(int MsgId, short MsgSubId, const IEObject* Sender,
  const IEObject* Data)
{
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
void TXFile::LoadFromFile(const olxstr & _fn) {
  const NameArg file_n = ParseName(_fn);
  const olxstr ext(TEFile::ExtractFileExt(file_n.file_name));
  // this thows an exception if the file format loader does not exist
  TBasicCFile* Loader = FindFormat(ext);
  bool replicated = false;
  if( FLastLoader == Loader )  {
    Loader = (TBasicCFile*)Loader->Replicate();
    replicated = true;
  }
  try  {
    Loader->LoadFromFile(_fn);
    for (size_t i=0; i < Loader->GetAsymmUnit().AtomCount(); i++) {
      TCAtom &a = Loader->GetAsymmUnit().GetAtom(i);
      if (olx_abs(a.ccrd()[0]) > 127 ||
          olx_abs(a.ccrd()[1]) > 127 ||
          olx_abs(a.ccrd()[2]) > 127)
      {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("atom coordinates for ").quote() << a.GetLabel());
      }
    }
  }
  catch( const TExceptionBase& exc )  {
    if( replicated )  
      delete Loader;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }

  if( !Loader->IsNative() )  {
    OnFileLoad.Enter(this, &_fn);
    try  {
      GetRM().Clear(rm_clear_ALL);
      GetLattice().Clear(true);
      GetRM().Assign(Loader->GetRM(), true);
      OnFileLoad.Execute(this);
      GetLattice().Init();
    }
    catch(const TExceptionBase& exc)  {
      OnFileLoad.Exit(this);
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    OnFileLoad.Exit(this);
  }
  FSG = &TSymmLib::GetInstance().FindSG(Loader->GetAsymmUnit());
  if( replicated )  {
    for( size_t i=0; i < FileFormats.Count(); i++ )
      if( FileFormats.GetObject(i) == FLastLoader )
        FileFormats.GetObject(i) = Loader;
    delete FLastLoader;
  }
  FLastLoader = Loader;
  if( GetRM().GetHKLSource().IsEmpty() ||
     !TEFile::Exists(GetRM().GetHKLSource()) )
  {
    olxstr src = TXApp::GetInstance().LocateHklFile();
    if( !src.IsEmpty() && !TEFile::Existsi(olxstr(src), src) )
      src.SetLength(0);
    GetRM().SetHKLSource(src);
  }
}
//..............................................................................
void TXFile::UpdateAsymmUnit()  {
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
  LL->GetAsymmUnit().SetZ(GetAsymmUnit().GetZ());
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
    if( list[i]->GetType() == iHydrogenZ )  {
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
    if( del_h_cnt != 0 && free_h_cnt != 0 )  {
      TBasicApp::NewLogEntry(logError) << "Hydrogen atoms, which are not "
        "attached using AFIX will not be kept with pivot atom until the file "
        "is reloaded";
    }
  }
  try {
    AtomSorter::CombiSort cs;
    olxstr sort;
    for( size_t i=0; i < ins.Count(); i++ )  {
      if( ins[i].CharAt(0) == '+' )
        sort << ins[i].SubStringFrom(1);
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
       else if( sort.CharAt(i) == 'z' )  
        cs.sequence.Add(&AtomSorter::atom_cmp_Suffix);
       else if( sort.CharAt(i) == 'n' )  
        cs.sequence.Add(&AtomSorter::atom_cmp_Number);
    }
    if( !cs.sequence.IsEmpty() )
      AtomSorter::Sort(list, cs);
    if( !labels.IsEmpty() )  {
      AtomSorter::SortByName(list, labels);
      labels.Clear();
    }
    if( moiety_index != InvalidIndex )  {
      sort.SetLength(0);
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
    TBasicApp::NewLogEntry(logError) << exc.GetException()->GetError();
  }
  if( !FLastLoader->IsNative() )  {
    AtomSorter::SyncLists(list,
      FLastLoader->GetAsymmUnit().GetResidue(0).GetAtomList());
    FLastLoader->GetAsymmUnit().ComplyToResidues();
  }
  // this changes Id's !!! so must be called after the SyncLists
  GetAsymmUnit().ComplyToResidues();
  // 2010.11.29, ASB bug fix for ADPS on H...
  GetUnitCell().UpdateEllipsoids();
  GetLattice().RestoreADPs(false);
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
    ASObjectProvider& objects = Lattice.GetObjects();
    const size_t ac = objects.atoms.Count();
    for( size_t j=0; j < ac; j++ )  {
      TSAtom& sa1 = objects.atoms[j];
      if( sa1.CAtom().GetId() == it.GetAtom(0).GetAtom()->GetId() )  {
        sa = &sa1;
        break;
      }
    }
    if( sa == NULL )  {
      RefMod.DeleteInfoTab(i--);
      continue;
    }
    bool hasH = false;
    for( size_t j=0; j < sa->NodeCount(); j++ )  {
      if( !sa->Node(j).IsDeleted() && sa->Node(j).GetType() == iHydrogenZ )  {
        hasH = true;
        break;
      }
    }
    if( !hasH )  {  
      TBasicApp::NewLogEntry() << "Removing HTAB (donor has no H atoms): "
        << it.InsStr();
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
      TBasicApp::NewLogEntry() << "Removing HTAB (d > 5A): " << it.InsStr();
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
      if( !Loader->Adopt(*this) ) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "could not adopt specified file format");
      }
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
  if( Cause != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, Cause);
}
//..............................................................................
void TXFile::Close()  {
  OnFileClose.Enter(this, FLastLoader);
  FLastLoader = NULL;
  RefMod.Clear(rm_clear_ALL);
  Lattice.Clear(true);
  OnFileClose.Exit(this, NULL);
}
//..............................................................................
IEObject* TXFile::Replicate() const {
  TXFile* xf = new TXFile(*(SObjectProvider*)Lattice.GetObjects().Replicate());
  for( size_t i=0; i < FileFormats.Count(); i++ )  {
    xf->RegisterFileFormat((TBasicCFile*)FileFormats.GetObject(i)->Replicate(),
                              FileFormats[i]);
  }
  return xf;
}
//..............................................................................
void TXFile::EndUpdate()  {
  OnFileLoad.Enter(this, &GetFileName());
  OnFileLoad.Execute(this);
  // we keep the asymmunit but clear the unitcell
  try {
    GetLattice().Init();
    OnFileLoad.Exit(this);
  }
  catch (const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
    Close();
  }
}
//..............................................................................
void TXFile::ToDataItem(TDataItem& item) {
  GetLattice().ToDataItem(item.AddItem("Lattice"));
  GetRM().ToDataItem(item.AddItem("RefModel"));
}
//..............................................................................
void TXFile::FromDataItem(TDataItem& item) {
  GetRM().Clear(rm_clear_ALL);
  GetLattice().FromDataItem(item.FindRequiredItem("Lattice"));
  GetRM().FromDataItem(item.FindRequiredItem("RefModel"));
  GetLattice().FinaliseLoading();
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
    rv << content[i].element.symbol;
    if( list )  rv << ':';
    bool subAdded = false;
    const double dv = content[i].count/GetAsymmUnit().GetZ();
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
      const cm_Element* elm =
        XElementLib::FindBySymbol(toks[i].SubStringTo(ind));
      if( elm == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, "element");
      content.AddNew(*elm,
        toks[i].SubStringFrom(ind+1).ToDouble()*GetAsymmUnit().GetZ());
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
  // needs to be called to assign the loaderIds for new atoms
  UpdateAsymmUnit();
  ins.GetRM().Assign( GetRM(), true );
  ins.AddIns("FMAP 2", ins.GetRM());
  ins.GetRM().SetRefinementMethod("L.S.");
  ins.GetRM().SetIterations(4);
  ins.GetRM().SetPlan(20);
  ins.GetRM().SetUserContent(oins->GetRM().GetUserContent());
  ins.SaveToFile(Params[0]);
}
void TXFile::LibDataCount(const TStrObjList& Params, TMacroError& E)  {
  if( EsdlInstanceOf(*FLastLoader, TCif) )
    E.SetRetVal(((TCif*)FLastLoader)->BlockCount());
  else
    E.SetRetVal(1);
}
//..............................................................................
void TXFile::LibCurrentData(const TStrObjList& Params, TMacroError& E)  {
  TCif &cif = *(TCif*)FLastLoader;
  if( Params.IsEmpty() )
    E.SetRetVal(cif.GetBlockIndex());
  else
    cif.SetCurrentBlock(Params[0].ToInt());
}
//..............................................................................
void TXFile::LibDataName(const TStrObjList& Params, TMacroError& E)  {
  int i = Params[0].ToInt();
  TCif &cif = *(TCif*)FLastLoader;
  if( i < 0 )
    E.SetRetVal(cif.GetDataName());
  else  {
    if( (size_t)i >= cif.BlockCount() )
      throw TIndexOutOfRangeException(__OlxSourceInfo, i, 0, cif.BlockCount());
    E.SetRetVal(cif.GetBlock(i).GetName());
  }
}
//..............................................................................
void TXFile::LibGetMu(const TStrObjList& Params, TMacroError& E)  {
  cm_Absorption_Coefficient_Reg ac;
  ContentList cont = GetAsymmUnit().GetContentList();
  double mu=0;
  for( size_t i=0; i < cont.Count(); i++ )  {
    double v = ac.CalcMuOverRhoForE(
      GetRM().expl.GetRadiationEnergy(), *ac.locate(cont[i].element.symbol));
    mu += (cont[i].count*cont[i].element.GetMr())*v;
  }
  mu *= GetAsymmUnit().GetZ()/GetAsymmUnit().CalcCellVolume()/
    GetAsymmUnit().GetZPrime();
  mu /= 6.022142;
  E.SetRetVal(olxstr::FormatFloat(3,mu));
}
//..............................................................................
TLibrary* TXFile::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("xf") : name );

  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this, &TXFile::LibGetFormula, "GetFormula",
      fpNone|fpOne|fpTwo|psFileLoaded,
      "Returns a string for content of the asymmetric unit. Takes single or "
      "none parameters. If parameter equals 'html' and html formatted string is" 
      " returned, for 'list' parameter a string like 'C:26,N:45' is returned. "
      "If no parameter is specified, just formula is returned")
   );

  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this,  &TXFile::LibSetFormula, "SetFormula",
      fpOne|psCheckFileTypeIns,
      "Sets formula for current file, takes a string of the following form "
      "'C:25,N:4'")
  );

  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this,  &TXFile::LibEndUpdate, "EndUpdate",
      fpNone|psCheckFileTypeIns,
      "Must be called after the content of the asymmetric unit has changed - "
      "this function will update the program state")
  );

  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this,  &TXFile::LibSaveSolution, "SaveSolution",
      fpOne|psCheckFileTypeIns,
      "Saves current Q-peak model to provided file (res-file)")
  );

  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this,  &TXFile::LibDataCount, "DataCount",
      fpNone|psFileLoaded,
      "Returns number of available data sets")
  );

  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this,  &TXFile::LibDataName, "DataName",
      fpOne|psCheckFileTypeCif,
      "Returns data name for given CIF block")
  );
  
  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this,  &TXFile::LibCurrentData, "CurrentData",
      fpNone|fpOne|psCheckFileTypeCif,
      "Returns current data index or changes current data block within the CIF")
  );
  
  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this,  &TXFile::LibGetMu, "GetMu",
      fpNone|psFileLoaded,
      "Changes current data block within the CIF")
  );
  
  lib->AttachLibrary(Lattice.GetAsymmUnit().ExportLibrary());
  lib->AttachLibrary(Lattice.GetUnitCell().ExportLibrary());
  lib->AttachLibrary(Lattice.ExportLibrary());
  lib->AttachLibrary(RefMod.expl.ExportLibrary());
  lib->AttachLibrary(RefMod.ExportLibrary());
  lib->AttachLibrary(olx_analysis::Analysis::ExportLibrary());
  return lib;
}
//..............................................................................
TXFile::NameArg TXFile::ParseName(const olxstr& fn)  {
  TXFile::NameArg rv;
  rv.file_name = fn;
  rv.is_index = false;
  const size_t di = fn.LastIndexOf('.');
  const size_t hi = fn.LastIndexOf('#');
  const size_t ui = fn.LastIndexOf('$');
  if( hi == InvalidIndex && ui == InvalidIndex )
    return rv;
  if( di != InvalidIndex )  {
    if( hi != InvalidIndex && ui != InvalidIndex && di < hi && di < ui ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "only one data ID is allowed");
    }
    if( hi != InvalidIndex && di < hi )  {
      rv.data_name = fn.SubStringFrom(hi+1);
      rv.file_name = fn.SubStringTo(hi);
      rv.is_index = true;
    }
    else if( ui != InvalidIndex && di < ui )  {
      rv.data_name = fn.SubStringFrom(ui+1);
      rv.file_name = fn.SubStringTo(ui);
      rv.is_index = false;
    }
  }
  else  {
    if( hi != InvalidIndex && ui != InvalidIndex ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "only one data ID is allowed");
    }
    if( hi != InvalidIndex )  {
      rv.data_name = fn.SubStringFrom(hi+1);
      rv.file_name = fn.SubStringTo(hi);
      rv.is_index = true;
    }
    else if( ui != InvalidIndex )  {
      rv.data_name = fn.SubStringFrom(ui+1);
      rv.file_name = fn.SubStringTo(ui);
      rv.is_index = false;
    }
  }
  return rv;
}
//..............................................................................

