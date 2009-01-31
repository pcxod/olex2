//---------------------------------------------------------------------------//
// namespace TXFiles: THkl - basic procedures for the SHELX HKL files
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdlib.h>

#include "hkl.h"
#include "lst.h"
#include "ins.h"
#include "emath.h"
#include "efile.h"
#include "estrlist.h"

#include "exception.h"
#include "ematrix.h"

#include "symmlib.h"

//..............................................................................
//----------------------------------------------------------------------------//
// THklFil function bodies
//----------------------------------------------------------------------------//
THklFile::THklFile()  {
  Hkl3D = NULL;
}
//..............................................................................
THklFile::~THklFile()  {
  Clear();
}
//..............................................................................
void THklFile::Clear()  {
  for( int i=0; i < Refs.Count(); i++ )
    delete Refs[i];
  Refs.Clear();
  Clear3D();
}
//..............................................................................
void THklFile::Clear3D()  {
  if( Hkl3D == NULL )  return;
  for( int i=MinHkl[0]; i <= MaxHkl[0]; i++ ) {
    for( int j=MinHkl[1]; j <= MaxHkl[1]; j++ )
      for( int k=MinHkl[2]; k <= MaxHkl[2]; k++ )
        if( Hkl3D->Value(i,j,k) != NULL )
          delete Hkl3D->Value(i,j,k);
  }
  delete Hkl3D;
  Hkl3D = NULL;
}

bool THklFile::LoadFromFile(const olxstr& FN, TIns* ins)  {
  Clear();
  TEFile::CheckFileExists(__OlxSourceInfo, FN);
  TCStrList SL, Toks;
  bool ZeroRead = false, 
       HklFinished = false,
       HasBatch = false,
       FormatInitialised = false;
  int Tag = 1;
  SL.LoadFromFile(FN);

  const int line_cnt = SL.Count();
  Refs.SetCapacity( line_cnt );
  for(int i=0; i < line_cnt; i++ )  {
    const CString& line = SL[i];
    if( line.Length() < 28 )  continue;
    if( !FormatInitialised )  {
      if( line.Length() >= 32 && line.SubString(28,4).IsNumber() )
        HasBatch = true;
      FormatInitialised = true;
    }
		if( ZeroRead && !HklFinished )  {
      if( !line.SubString(0,4).IsNumber() || 
          !line.SubString(4,4).IsNumber() || 
          !line.SubString(8,4).IsNumber() || 
          !line.SubString(12,8).IsNumber() || 
          !line.SubString(20,8).IsNumber() )
			  HklFinished = true; 
		}
		if( !HklFinished )  {
      const int h = line.SubString(0,4).ToInt(),
                k = line.SubString(4,4).ToInt(),
                l = line.SubString(8,4).ToInt();
      if( (h|k|l) == 0 )  {  // end of the reflections included into calculations
        ZeroRead = true;
        Tag = -1;
        continue;
      }
      TReflection* ref = HasBatch ? 
        new TReflection( h, k, l, line.SubString(12,8).ToDouble(), line.SubString(20,8).ToDouble(), line.SubString(28,4).ToInt() )
        :
        new TReflection( h, k, l, line.SubString(12,8).ToDouble(), line.SubString(20,8).ToDouble() );
      ref->SetTag( Tag );
      UpdateMinMax( *ref );
      Refs.Add(ref);
		}
		else  {
			if( ins == NULL )  break;
        ins->Clear();
        ins->SetTitle( TEFile::ChangeFileExt(TEFile::ExtractFileName(FN), EmptyString) << " imported from HKL file" );
        olxstr line;
        bool cell_found = false;
        ins->SetSfac(EmptyString);
        for( int j=i; i < SL.Count(); j++ )  {
          line = SL[j].Trim(' ');
          if( line.IsEmpty() )  continue;
          Toks.Clear();
          if( line.StartFromi("CELL") )  {
            Toks.Strtok(line, ' ');
            if( Toks.Count() != 8 )
              throw TFunctionFailedException(__OlxSourceInfo, "invalid CELL format");
            ins->GetRM().expl.SetRadiation( Toks[1].ToDouble() );
            ins->GetAsymmUnit().Axes()[0] = Toks[2];
            ins->GetAsymmUnit().Axes()[1] = Toks[3];
            ins->GetAsymmUnit().Axes()[2] = Toks[4];
            ins->GetAsymmUnit().Angles()[0] = Toks[5];
            ins->GetAsymmUnit().Angles()[1] = Toks[6];
            ins->GetAsymmUnit().Angles()[2] = Toks[7];
            cell_found = true;
          }
          else if( line.StartFromi("SFAC") )  {
            Toks.Strtok(line, ' ');  // do the validation
            olxstr unit;
            for( int k=1; k < Toks.Count(); k++ )  {
              if( !ins->GetAsymmUnit().GetAtomsInfo()->IsAtom(Toks[k]) )
                throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid element ") << Toks[k]);
              unit << "1 ";
            }
            ins->SetSfac( line.SubStringFrom(5) );
            ins->SetUnit( unit );
          }
          else if( line.StartFromi("TEMP") )
            ins->AddIns(line, ins->GetRM());
          else if( line.StartFromi("SIZE") )
            ins->AddIns(line, ins->GetRM());
          else if( line.StartFromi("REM") )
            ins->AddIns(line, ins->GetRM());
          else if( line.StartFromi("UNIT") )
            ins->SetUnit( line.SubStringFrom(5) );
        }
        if( !cell_found )
          throw TFunctionFailedException(__OlxSourceInfo, "could no locate valid CELL instruction");
        break;
		}
  }
  for( int i=0; i < Refs.Count(); i++ )
    Refs[i]->SetTag( (i+1) * Sign(Refs[i]->GetTag()));

  if( Refs.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "no reflections found");
  return true;
}
//..............................................................................
bool THklFile::SaveToFile(const olxstr& FN)  {  return THklFile::SaveToFile(FN, Refs, false);  }
//..............................................................................
void THklFile::UpdateRef(const TReflection& R)  {
  int ind = abs(R.GetTag())-1;
  if( ind < 0 || ind >= Refs.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "reflection tag");
  Refs[ind]->SetTag( R.GetTag() );
}
//..............................................................................
int THklFile::HklCmp(const TReflection* R1, const TReflection* R2)  {
  int r = R1->CompareTo(*R2);
  if( r == 0 )  {  // for unmerged data ...
    if( R1->GetI() < R2->GetI() )  return -1;
    if( R1->GetI() > R2->GetI() )  return 1;
  }
  return r;
}
//..............................................................................
void THklFile::InitHkl3D()  {
  if( Hkl3D != NULL )  return;
  TArray3D<TRefPList*> &hkl3D = *(new TArray3D<TRefPList*>(
                                     MinHkl[0], MaxHkl[0],
                                     MinHkl[1], MaxHkl[1],
                                     MinHkl[2], MaxHkl[2]) );

  for( int i=0; i < Refs.Count(); i++ )  {
    TReflection *r1 = Refs[i];
    TRefPList *&rl = hkl3D(r1->GetHkl());

    if( rl == NULL )
      rl = new TRefPList();
    rl->Add( r1 );
  }
  Hkl3D = &hkl3D;
}
//..............................................................................
void THklFile::AllRefs(const TReflection& R, const smatd_list& ml, TRefPList& Res)  {
  vec3i hklv;
  vec3i_list ri;
  for( int i=0; i < ml.Count(); i++ )  {
    R.MulHkl(hklv, ml[i]);
    // check if the range of the reflection is valid
    if( !vec3i::IsInRangeExc(hklv, MinHkl, MaxHkl) )  continue;
    if( ri.IndexOf(hklv) == -1 )  ri.AddCCopy(hklv);
  }

  InitHkl3D();

  for( int j=0; j < ri.Count(); j++ )  {
    TRefPList* r = Hkl3D->Value(ri[j]);
    if( r != NULL )  
      Res.AddList(*r);
  }
}
//..............................................................................
void THklFile::Append(TReflection& hkl)  {
  UpdateMinMax( hkl );
  Refs.Add( &hkl );
  hkl.SetTag( Refs.Count() );
}
//..............................................................................
void THklFile::EndAppend()  {
  //Refs.QuickSorter.SortSF(Refs, HklCmp);
}
//..............................................................................
void THklFile::Append(const TRefPList& hkls)  {
  if( hkls.IsEmpty() )  return;

  for( int i=0; i < hkls.Count(); i++ )  {
    // call it before new - in case of exception
    UpdateMinMax( *hkls[i] );

    TReflection* r = new TReflection( *hkls[i] );
    r->SetTag( Refs.Count() );
    Refs.Add(r);
  }
  EndAppend();
}
//..............................................................................
void THklFile::Append(const THklFile& hkls)  {
  if( !hkls.RefCount() )  return;
  Append( hkls.Refs );
}
//..............................................................................
bool THklFile::SaveToFile(const olxstr& FN, const TRefPList& refs, bool Append)  {
  if( refs.IsEmpty() )  return true;

  if( Append && TEFile::FileExists(FN) )  {
    THklFile F;
    F.LoadFromFile(FN);
    F.Append(refs);
    F.SaveToFile(FN);
  }
  else  {
    TEFile out(FN, "w+b");
    TReflection NullRef(0, 0, 0, 0, 0);
    if( refs[0]->GetFlag() != NoFlagSet )
      NullRef.SetFlag(0);
    const int ref_str_len = NullRef.ToString().Length();
    char* ref_bf = new char[ref_str_len+1];
    for( int i=0; i < refs.Count(); i++ )  {
      if( refs[i]->GetTag() > 0 )  
        out.Writenl( refs[i]->ToCBuffer(ref_bf), ref_str_len );
    }
    out.Writenl( NullRef.ToCBuffer(ref_bf), ref_str_len );
    for( int i=0; i < refs.Count(); i++ )  {
      if( refs[i]->GetTag() < 0 )  
        out.Writenl( refs[i]->ToCBuffer(ref_bf), ref_str_len );
    }
    delete [] ref_bf;
  }
  return true;
}
//..............................................................................
bool THklFile::SaveToFile(const olxstr& FN, const TRefList& refs)  {
  if( refs.IsEmpty() )  return true;
  TEFile out(FN, "w+b");
  TReflection NullRef(0, 0, 0, 0, 0);
  if( refs[0].GetFlag() != NoFlagSet )
    NullRef.SetFlag(0);
  const int ref_str_len = NullRef.ToString().Length();
  char* ref_bf = new char[ref_str_len+1];
  for( int i=0; i < refs.Count(); i++ )  {
    if( refs[i].GetTag() > 0 )  
      out.Writenl( refs[i].ToCBuffer(ref_bf), ref_str_len );
  }
  out.Writenl( NullRef.ToCBuffer(ref_bf), ref_str_len );
  for( int i=0; i < refs.Count(); i++ )  {
    if( refs[i].GetTag() < 0 )  
      out.Writenl( refs[i].ToCBuffer(ref_bf), ref_str_len );
  }
  delete [] ref_bf;
  return true;
}
//..............................................................................


