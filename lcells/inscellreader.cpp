//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "inscellreader.h"
#include "estrlist.h"
#include "efile.h"

//---------------------------------------------------------------------------
TInsCellReader::TInsCellReader()
{
  Fa = Fb = Fc = Faa = Fab = Fac = 0;
}
bool _fastcall TInsCellReader::LoadFromInsFile(const olxstr& FN)  {
  TStrList S, toks;
  olxstr Tmp;
  bool res = false;
  short params_found = 0;
  try  {
    S.LoadFromFile( FN );
  }
  catch( const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  for( int i=0; i < S.Count(); i++ )  {
    if( S[i].Length() == 0 )
      continue;
    toks.Clear();
    toks.Strtok(S[i].LowerCase(), ' ');
    if( toks.Count() == 0 )
      continue;
    if( toks[0] == "cell" && toks.Count() == 8 )  {
      Fa = toks[2].ToDouble();
      Fb = toks[3].ToDouble();
      Fc = toks[4].ToDouble();
      Faa = toks[5].ToDouble();
      Fab = toks[6].ToDouble();
      Fac = toks[7].ToDouble();
      params_found ++;
    }
    else if( toks[0] == "latt" && toks.Count() == 2 )  {
      FLattice = abs( toks[1].ToInt() );
      params_found ++;
    }
    if( params_found == 2 )  {
      res = true;
      goto exit;
    }
  }
exit:
  return res;
}
//..............................................................................
bool _fastcall TInsCellReader::LoadFromCifFile(const olxstr& FN)  {
  Fa = Fb = Fc = Faa = Fab = Fac = 0;
  FLattice = 0;
  TStrList S, toks;
  olxstr Tmp;
  bool res = false;
  try  {
    S.LoadFromFile( FN );
  }
  catch( const TExceptionBase& exc)  {
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  for( int i=0; i < S.Count(); i++ )  {
    if( S[i].IsEmpty() || S[i][0] != '_' )  continue;
    toks.Clear();
    toks.Strtok(S[i].LowerCase(), ' ');
    if( toks.Count() < 2 )  continue;

    if( toks[0] == "_cell_length_a" )
      Fa = toks[1].ToDouble();
    else if( toks[0] == "_cell_length_b" )
      Fb = toks[1].ToDouble();
    else if( toks[0] == "_cell_length_c" )
      Fc = toks[1].ToDouble();
    if( toks[0] == "_cell_angle_alpha" )
      Faa = toks[1].ToDouble();
    if( toks[0] == "_cell_angle_beta" )
      Fab = toks[1].ToDouble();
    if( toks[0] == "_cell_angle_gamma" )
      Fac = toks[1].ToDouble();
    if( toks[0] == "_symmetry_space_group_name_h-m" )  {
      for( int j=0; j < toks[1].Length(); j++ )  {
        switch( toks[1].CharAt(j) )  {
          case 'p':  FLattice = 1;    break;
          case 'i':  FLattice = 2;    break;
          case 'r':  FLattice = 3;    break;
          case 'f':  FLattice = 4;    break;
          case 'a':  FLattice = 5;    break;
          case 'b':  FLattice = 6;    break;
          case 'c':  FLattice = 7;    break;
        }
        if( FLattice != 0 ) break;
      }
    }

    if( Fa && Fb && Fc && Faa && Fab && Fac && FLattice )  {
      res = true;
      break;
    }
  }
  return res;
}

#pragma package(smart_init)

