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

//---------------------------------------------------------------------------
// TBasicCFile function bodies
//---------------------------------------------------------------------------
TBasicCFile::TBasicCFile(TAtomsInfo *S)  {
  FAsymmUnit   = new TAsymmUnit(NULL, S);
  AtomsInfo = S;
}
//..............................................................................
TBasicCFile::~TBasicCFile()  {
  delete FAsymmUnit;
}
//..............................................................................
void TBasicCFile::SaveToFile(const olxstr &A)  {
  TStrList L;
  FAsymmUnit->KeepH();
  SaveToStrings( L );
#ifdef _UNICODE
  TCStrList(L).SaveToFile( A );
#else
  L.SaveToFile( A );
#endif
  FFileName = A;
};
//..............................................................................
void TBasicCFile::LoadFromFile(const olxstr &A)  {
  TEFile::CheckFileExists(__OlxSourceInfo, A);
  TStrList L;
  L.LoadFromFile( A );
  if( L.IsEmpty() )
    throw TEmptyFileException(__OlxSourceInfo, A);
  FFileName = A;
  FHKLSource = EmptyString;
  try  {  LoadFromStrings(L);  }
  catch( const TExceptionBase& exc )  {  
    FFileName = EmptyString;
    throw TFunctionFailedException(__OlxSourceInfo, exc);  
  }
  FFileName = A;
  if( FHKLSource.IsEmpty() )  {
    FHKLSource = TEFile::ChangeFileExt(A, "hkl");
    if( !TEFile::FileExists(FHKLSource) )
      FHKLSource = EmptyString;
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
TXFile::TXFile(TAtomsInfo *S)  {
  AtomsInfo = S;
  FLattice = new TLattice(S);
  FAsymmUnit = &GetLattice().GetAsymmUnit();
  TBasicApp::GetInstance()->NewActionQueue("XFILELOAD");
  TBasicApp::GetInstance()->NewActionQueue("XFILESAVE");
  FLastLoader = NULL;
}
//..............................................................................
TXFile::~TXFile()  {
////////////////////////////////////////////
// finding uniq objects and deleting them
  for( int i=0; i < FileFormats.Count(); i++ )  {
    FileFormats.Object(i)->SetTag(i);
  }
  for( int i=0; i < FileFormats.Count(); i++ )  {
    if( FileFormats.Object(i)->GetTag() == i )
      delete FileFormats.Object(i);
  }
////////////////////////////////////////////
  delete FLattice;
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
void TXFile::LoadFromFile(const olxstr & FN) {
  olxstr Ext = TEFile::ExtractFileExt(FN);
  // this thows an exception if the file format loader does not exist
  TBasicCFile *Loader = FindFormat(Ext);
  Loader->LoadFromFile(FN);

  FFileName = FN;
  TBasicApp::GetInstance()->ActionQueue("XFILELOAD")->Enter(this);
  GetLattice().Clear(true);
  GetAsymmUnit().Assign(Loader->GetAsymmUnit());
  GetLattice().Init();
  TBasicApp::GetInstance()->ActionQueue("XFILELOAD")->Exit(this);
  FLastLoader = Loader;
}
//..............................................................................
void TXFile::UpdateAsymmUnit()  {
  //TLattice *L = GetLattice();
  TBasicCFile *LL = GetLastLoader();
  LL->GetAsymmUnit().ExpandExyz(); // should not have a any 'new' atoms
  GetLattice().UpdateAsymmUnit();
  TCAtomPList* NewAtoms = GetAsymmUnit().ExpandExyz();
  if( NewAtoms == NULL )
    NewAtoms = new TCAtomPList;
  LL->GetAsymmUnit().ClearResidues();
  for( int i=0; i < GetAsymmUnit().ResidueCount(); i++ )  {
    TAsymmUnit::TResidue& resi = GetAsymmUnit().GetResidue(i);
    LL->GetAsymmUnit().NewResidue(resi.GetClassName(), resi.GetNumber(), resi.GetAlias() );
  }
  /* find new atoms */
  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& CA1 = GetAsymmUnit().GetAtom(i);
    if( CA1.GetLoaderId() == liNewAtom && CA1.GetAtomInfo() != iQPeakIndex )
      NewAtoms->Add(&CA1);
  }
  for(int i=0; i < LL->GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& CA = LL->GetAsymmUnit().GetAtom(i);
    for( int j=0; j < GetAsymmUnit().AtomCount(); j++ )  {
      TCAtom& CA1 = GetAsymmUnit().GetAtom(j);
      if( (CA1.GetLoaderId() == CA.GetLoaderId()) && (CA1.GetLoaderId() != liNewAtom) )  {
        if( CA1.GetAtomInfo() == iQPeakIndex )  { // automatic delete Q-peaks
          CA.SetDeleted(true);  break;
        }
        CA.Assign(CA1);
        if( CA1.GetResiId() != -1 )  // main residue
          LL->GetAsymmUnit().GetResidue(CA1.GetResiId()).AddAtom(&CA);
        break;
      }
    }
  }
  int loaderid = GetAsymmUnit().GetMaxLoaderId();
  for( int i=0; i < NewAtoms->Count(); i++ )  {
    TCAtom* CA = NewAtoms->Item(i);
    TCAtom& CA1 = LL->GetAsymmUnit().NewAtom();
    CA1.Assign(*CA);
    CA1.SetLoaderId(loaderid+i);
    CA->SetLoaderId(loaderid+i);
    if( CA->GetResiId() != -1 )  // main residue
      LL->GetAsymmUnit().GetResidue(CA->GetResiId()).AddAtom(&CA1);
  }
  delete NewAtoms;

  LL->GetAsymmUnit().RestrainedDistances().Assign(GetAsymmUnit(), GetAsymmUnit().RestrainedDistances());
  LL->GetAsymmUnit().RestrainedAngles().Assign(GetAsymmUnit(), GetAsymmUnit().RestrainedAngles());
  LL->GetAsymmUnit().SimilarDistances().Assign(GetAsymmUnit(), GetAsymmUnit().SimilarDistances());
  LL->GetAsymmUnit().RestrainedVolumes().Assign(GetAsymmUnit(), GetAsymmUnit().RestrainedVolumes());
  LL->GetAsymmUnit().RestrainedPlanarity().Assign(GetAsymmUnit(), GetAsymmUnit().RestrainedPlanarity());
  LL->GetAsymmUnit().RestranedUaAsUi().Assign(GetAsymmUnit(), GetAsymmUnit().RestranedUaAsUi());
  LL->GetAsymmUnit().RigidBonds().Assign(GetAsymmUnit(), GetAsymmUnit().RigidBonds());
  LL->GetAsymmUnit().SimilarU().Assign(GetAsymmUnit(), GetAsymmUnit().SimilarU());
  LL->GetAsymmUnit().EquivalentU().Assign(GetAsymmUnit(), GetAsymmUnit().EquivalentU());
  LL->GetAsymmUnit().SimilarFragments().Assign(GetAsymmUnit(), GetAsymmUnit().SimilarFragments());
  LL->GetAsymmUnit().ClearUsedSymm();
  for( int i=0; i < GetAsymmUnit().UsedSymmCount(); i++ )
    LL->GetAsymmUnit().AddUsedSymm( GetAsymmUnit().GetUsedSymm(i) );

  LL->GetAsymmUnit().SetZ( GetAsymmUnit().GetZ() );

  //AsymmUnit()->CollapseExyz();
}
//..............................................................................
void TXFile::SaveToFile(const olxstr & FN, bool Sort)  {
  olxstr Ext = TEFile::ExtractFileExt(FN);
  TBasicCFile *Loader = FindFormat(Ext);

  TBasicCFile *LL = GetLastLoader();

  if( LL != Loader )  {
    if( !Loader->Adopt(this) )
      throw TFunctionFailedException(__OlxSourceInfo, "could not adopt specified file format");
  }
  else
  {
    UpdateAsymmUnit();
  }
  if( Sort )  Loader->GetAsymmUnit().Sort();
  TBasicApp::GetInstance()->ActionQueue("XFILESAVE")->Enter(this);
  IEObject* Cause = NULL;
  try  {
    Loader->SaveToFile(FN);
    FFileName = FN;
  }
  catch( const TExceptionBase &exc )  {
    Cause = exc.Replicate();
  }
  TBasicApp::GetInstance()->ActionQueue("XFILESAVE")->Exit(this);
  GetAsymmUnit().CollapseExyz();
  if( LL == Loader )  LL->GetAsymmUnit().CollapseExyz();
  if( Cause != NULL )  {
    throw TFunctionFailedException(__OlxSourceInfo, Cause);
  }
}
//..............................................................................
IEObject* TXFile::Replicate() const  {
  TXFile* xf = new TXFile( this->AtomsInfo );
  for( int i=0; i < FileFormats.Count(); i++ )  {
    xf->RegisterFileFormat( (TBasicCFile*)FileFormats.Object(i)->Replicate(), 
                              FileFormats.String(i) );
  }
  return xf;
}
//..............................................................................
void TXFile::EndUpdate()  {
  TBasicApp::GetInstance()->ActionQueue("XFILELOAD")->Enter(this);
  // we keep the asymmunit but clear the unitcell
  GetLattice().Clear(false);
  GetLattice().GetUnitCell().Clear();
  GetLattice().Init();
  TBasicApp::GetInstance()->ActionQueue("XFILELOAD")->Exit(this);
}




void TXFile::LibGetFormula(const TStrObjList& Params, TMacroError& E)  {
  if( GetLastLoader() == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "no file is loaded");
    E.SetRetVal( E.GetInfo() );
    return;
  }
  olxstr as, us;
  if( EsdlInstanceOf( *GetLastLoader(), TIns) )  {
    TIns* ins = (TIns*)GetLastLoader();
    as = ins->GetSfac();
    us = ins->GetUnit();
  }
  else if( EsdlInstanceOf( *GetLastLoader(), TCRSFile) )  {
    TCRSFile* crs = (TCRSFile*)GetLastLoader();
    as = crs->GetSfac();
    us = crs->GetUnit();
  }
  else  {
    TStrPObjList<olxstr,TBasicAtomInfo*> sl;
    GetAsymmUnit().SummFormula(sl, as, us, false);
  }
  bool list = false, html = false;
  if( Params.Count() != 0 )  {
    if( Params.String(0).Comparei("list") == 0 )
      list = true;
    else if( Params.String(0).Comparei("html") == 0 )
      html = true;
  }

  TCStrList atoms(as, ' '),
           units(us, ' ');
  olxstr rv;
  int len = olx_min( atoms.Count(), units.Count() );
  for( int i=0; i < len; i++) {
    rv << atoms[i].SubStringTo(1).UpperCase();
    rv << atoms[i].SubStringFrom(1).LowerCase();
    if( list )
      rv << ':';

    bool subAdded = false;
    double dv = units.String(i).ToDouble();
    if( html )  {
      if( dv/GetAsymmUnit().GetZ() != 1 )  {
        rv << "<sub>" << dv/GetAsymmUnit().GetZ();
        subAdded = true;
      }
    }
    else
      rv << dv/GetAsymmUnit().GetZ();

    if( rv.EndsWith(".0") ) //
      rv.SetLength( rv.Length() -2 );
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
  if( GetLastLoader() == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "no file is loaded");
    E.SetRetVal( E.GetInfo() );
    return;
  }
  if( !EsdlInstanceOf( *GetLastLoader(), TIns) )  {
    E.ProcessingError(__OlxSrcInfo, "operation only valid for ins files");
    E.SetRetVal( E.GetInfo() );
    return;
  }
  TIns* ins = (TIns*)GetLastLoader();
  olxstr Sfac, Unit;
  if( Params[0].IndexOf(':') == -1 )  {
    TTypeList<AnAssociation2<olxstr, int> > res;
    AtomsInfo->ParseElementString( Params[0], res);
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
  TIns* oins = (TIns*)GetLastLoader();
  TIns ins(AtomsInfo);
  ins.GetAsymmUnit().Assign( *FAsymmUnit );
  ins.AddIns("FMAP 2");
  ins.SetSfac( oins->GetSfac());
  ins.SetUnit( oins->GetUnit() );
  ins.SaveToFile( Params[0] );
}
//..............................................................................
TLibrary*  TXFile::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("xf") : name );
  lib->RegisterFunction<TXFile>(
    new TFunction<TXFile>(this,  &TXFile::LibGetFormula, "GetFormula", fpNone|fpOne,
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
  lib->AttachLibrary( FLattice->GetAsymmUnit().ExportLibrary() );
  lib->AttachLibrary( FLattice->ExportLibrary() );
  return lib;
}
//..............................................................................

