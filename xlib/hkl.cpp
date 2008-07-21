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
  try  {
    int fl = TEFile::FileLength(FN);
    TEFile fi(FN, "rb");
    char ch, pch;
    int line_length = 0, new_line_length = 0;
    bool End = false;
    while( (ch = fgetc(fi.Handle())) != EOF )  {
      if( ch == '\n' )  {
        if( line_length > 0 && pch == '\r' )  {
          new_line_length = 2;
          line_length--;
        }
        else
          new_line_length = 1;
        break;
      }
      pch = ch;
      line_length ++;
    }
    fi.Seek(0, SEEK_SET);
    if( line_length < 28 )  throw TFunctionFailedException(__OlxSourceInfo, "invalid file format");
    // preallocate array of refs
    Refs.SetCapacity( fl /(line_length+new_line_length) + 1);
    char format_a[] = "%4i%4i%4i%8lf%8lf";
    char format_b[] = "%4i%4i%4i%8lf%8lf%4i";
    char *format = (line_length == 28) ? format_a : format_b;
    double I, S;
    int h,k,l,batch = NoFlagSet;
#ifdef _MSC_VER
    while( (ch=fscanf_s(fi.Handle(), format, &h, &k, &l, &I, &S, &batch) ) != EOF )  {
#else
    while( (ch=fscanf(fi.Handle(), format, &h, &k, &l, &I, &S, &batch) ) != EOF )  {
#endif
/* After discussion with George, a CELL and SFAC instaructions might be at this point ... */
      if( ch == 0 )  {  // error happened
        if( !End )  {  // this is an error
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong reflection at line ") << (Refs.Count()+1));
        }
        if( ins == NULL )  break;
        ins->Clear();
        ins->SetTitle( TEFile::ChangeFileExt(TEFile::ExtractFileName(FN), EmptyString) << " imported from HKL file" );
        olxstr line;
        TStrList lines, toks;
        lines.LoadFromTextStream(fi);
        lines.CombineLines('=');
        bool cell_found = false;
        ins->SetSfac(EmptyString);
        for( int i=0; i < lines.Count(); i++ )  {
          line = lines[i].Trim(' ');
          if( line.IsEmpty() )  continue;
          toks.Clear();
          if( line.StartFromi("CELL") )  {
            toks.Strtok(line, ' ');
            if( toks.Count() != 8 )
              throw TFunctionFailedException(__OlxSourceInfo, "invalid CELL format");
            ins->SetRadiation( toks[1].ToDouble() );
            ins->GetAsymmUnit().Axes()[0] = toks[2];
            ins->GetAsymmUnit().Axes()[1] = toks[3];
            ins->GetAsymmUnit().Axes()[2] = toks[4];
            ins->GetAsymmUnit().Angles()[0] = toks[5];
            ins->GetAsymmUnit().Angles()[1] = toks[6];
            ins->GetAsymmUnit().Angles()[2] = toks[7];
            cell_found = true;
          }
          else if( line.StartFromi("SFAC") )  {
            toks.Strtok(line, ' ');  // do the validation
            olxstr unit;
            for( int j=1; j < toks.Count(); j++ )  {
              if( !ins->GetAsymmUnit().GetAtomsInfo()->IsAtom(toks[j]) )
                throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid element ") << toks[j]);
              unit << "1 ";
            }
            ins->SetSfac( line.SubStringFrom(5) );
            ins->SetUnit( unit );
          }
          else if( line.StartFromi("TEMP") )
            ins->AddIns(line);
          else if( line.StartFromi("SIZE") )
            ins->AddIns(line);
          else if( line.StartFromi("REM") )
            ins->AddIns(line);
          else if( line.StartFromi("UNIT") )
            ins->SetUnit( line.SubStringFrom(5) );
        }
        if( !cell_found )
          throw TFunctionFailedException(__OlxSourceInfo, "could no locate valid CELL instruction");
        break;
      }

      if( (h|k|l) == 0 )  {  End = true;  continue;  }
      TReflection *ref = new TReflection(h,k,l,I,S,batch);
      End ? ref->SetTag(-1) : ref->SetTag(1);
      UpdateMinMax( *ref );
      Refs.Add(ref);
      fi.Seek(Refs.Count()*(line_length+new_line_length), SEEK_SET);
    }
    for( int i=0; i < Refs.Count(); i++ )
      Refs[i]->SetTag( (i+1) * Sign(Refs[i]->GetTag()));

    if( Refs.IsEmpty() )
      throw TFunctionFailedException(__OlxSourceInfo, "no reflections found");
    return true;
  }
  catch( TExceptionBase& exc )  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  TEFile::CheckFileExists(__OlxSourceInfo, FN);
  TStrList SL, Toks;
  TReflection *ref;
  int h, k, l;
  bool End = false;

  SL.LoadFromFile(FN);
  int bf [] = {4,4,4,8,8};
  TIntList Format(5, bf );
  bool FormatInitialised = false;
  for(int i=0; i < SL.Count(); i++ )  {
    if( SL.String(i).Length() < 28 )  continue;
    if( !FormatInitialised )  {
      if( SL.String(i).Length() >= 32 )
        Format.Add(4);
      FormatInitialised = true;
    }
    Toks.Clear();
    Toks.StrtokF(SL.String(i), Format);
    if( Toks.Count() >= 5 )  {
      h = Toks[0].ToInt();
      k = Toks[1].ToInt();
      l = Toks[2].ToInt();
      if( (h|k|l) == 0 )  {  // end of the reflections included into calculations
        End = true;
        continue;
      }
      if( Toks.Count() > 5 )
        ref = new TReflection( h, k, l, Toks[3].ToDouble(), Toks[4].ToDouble(), Toks[5].ToInt() );
      else
        ref = new TReflection( h, k, l, Toks[3].ToDouble(), Toks[4].ToDouble() );

      End ? ref->SetTag(-1) : ref->SetTag(1);
      UpdateMinMax( *ref );
      Refs.Add(ref);
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
    TRefPList *&rl = hkl3D(r1->GetH(), r1->GetK(), r1->GetL());

    if( rl == NULL )
      rl = new TRefPList();
    rl->Add( r1 );
  }
  Hkl3D = &hkl3D;
}
//..............................................................................
void THklFile::AllRefs(const TReflection& R, const TAsymmUnit& AU, TRefPList& Res)  {
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(AU);
  if( sg == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "Undefined space group");
  smatd_list ml;

  sg->GetMatrices(ml, mattAll);

  if( !sg->IsCentrosymmetric() )  {
    smatd& m = ml.AddNew();
    m.r.I();
    m.r *= -1;
  }

  vec3d hklv;
  vec3d_list ri;

  for( int i=0; i < ml.Count(); i++ )  {
    R.MulHkl(hklv, ml[i]);
    // check if the range of the reflection is valid
    if( hklv[0] < MinHkl[0] || hklv[0] > MaxHkl[0] )  continue;
    if( hklv[1] < MinHkl[1] || hklv[1] > MaxHkl[1] )  continue;
    if( hklv[2] < MinHkl[2] || hklv[2] > MaxHkl[2] )  continue;

    if( ri.IndexOf(hklv) == -1 )  ri.AddCCopy(hklv);
  }

  InitHkl3D();

  for( int j=0; j < ri.Count(); j++ )  {
    TRefPList* r = Hkl3D->Value((int)ri[j][0], (int)ri[j][1], (int)ri[j][2]);
    if( r != NULL )  Res.AddList(*r);
  }
}
//..............................................................................
void THklFile::UpdateMinMax(const TReflection& r)  {
  if( RefCount() == 0 )  {
    // set starting values
    MinHkl[0] = MaxHkl[0] = r.GetH();
    MinHkl[1] = MaxHkl[1] = r.GetK();
    MinHkl[2] = MaxHkl[2] = r.GetL();
    MinI = MaxI = r.GetI();
    MinIS = MaxIS = r.GetS();
  }
  else  {
    if( r.GetH() < MinHkl[0] )  MinHkl[0] = r.GetH();
    if( r.GetH() > MaxHkl[0] )  MaxHkl[0] = r.GetH();
    if( r.GetK() < MinHkl[1] )  MinHkl[1] = r.GetK();
    if( r.GetK() > MaxHkl[1] )  MaxHkl[1] = r.GetK();
    if( r.GetL() < MinHkl[2] )  MinHkl[2] = r.GetL();
    if( r.GetL() > MaxHkl[2] )  MaxHkl[2] = r.GetL();
    if( r.GetI() < MinI )  {  MinI = r.GetI();  MinIS = r.GetS();  }
    if( r.GetI() > MaxI )  {  MaxI = r.GetI();  MaxIS = r.GetS();  }
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
  if( !refs.Count() )  return true;

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
    for( int i=0; i < refs.Count(); i++ )  {
      if( refs[i]->GetTag() > 0 )  
        out.Writenl( CString(refs[i]->ToString()) );
    }
    out.Writenl( CString(NullRef.ToString()) );
    for( int i=0; i < refs.Count(); i++ )  {
      if( refs[i]->GetTag() < 0 )  out.Writenl( CString(refs[i]->ToString()) );
    }
  }
  return true;
}
//..............................................................................
THklFile::MergeStats THklFile::Merge(const TSpaceGroup& sg, bool MergeInverse, TRefList& output)  {
  smatd_list ml;
  sg.GetMatrices(ml, mattAll^mattIdentity);
  if( MergeInverse )  {
    smatd& im = ml.AddNew();
    im.r.I();
    im.r *= -1;
  }
  return Merge<TSimpleMerger>(ml, output);
}
//..............................................................................
void THklFile::AnalyseReflections( const TSpaceGroup& sg )  {
  smatd_list ml;
  sg.GetMatrices(ml, mattAll ^ (mattInversion|mattIdentity) );
  vec3d hklv;
  for( int i=0; i < RefCount(); i++ )  {
    Refs[i]->SetCentric(false);
    Refs[i]->SetDegeneracy(1);
    for( int j=0; j < ml.Count(); j++ )  {
      Refs[i]->MulHkl(hklv, ml[j]);
      if(  Refs[i]->EqHkl(hklv)  )  {
        //if( !Ref(i)->IsCentric() )
        //  Ref(i)->SetCentric(false);
        Refs[i]->IncDegeneracy();
      }
      else if( Refs[i]->EqNegHkl(hklv) )  {
        Refs[i]->SetCentric(true);
        Refs[i]->IncDegeneracy();
      }
    }
  }
}
//..............................................................................


