//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "inscellreader.h"
#include "estrlist.h"
#include "efile.h"

//---------------------------------------------------------------------------
TInsCellReader::TInsCellReader()  {
  Fa = Fb = Fc = Faa = Fab = Fac = 0;
}
bool _fastcall TInsCellReader::LoadFromInsFile(const olxstr& FN)  {
  try  {
    short params_found = 0;
    TStrList S, toks;
    S.LoadFromFile( FN );
    for( size_t i=0; i < S.Count(); i++ )  {
      if( S[i].IsEmpty() )  continue;
      toks.Clear();
      toks.Strtok(S[i].LowerCase(), ' ');
      if( toks.IsEmpty() )  continue;
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
        FLattice = abs(toks[1].ToInt());
        params_found ++;
      }
      if( params_found == 2 )
        return true;
    }
  }
  catch( const TExceptionBase& exc)  {  return false;  }
  return true;
}
//..............................................................................
double TInsCellReader::parse_double(const olxstr& str) const  {
  size_t i = str.IndexOf('(');
  return i == InvalidIndex ? str.ToDouble() : str.SubStringTo(i).ToDouble();
}
//..............................................................................
bool _fastcall TInsCellReader::LoadFromCifFile(const olxstr& FN)  {
  Fa = Fb = Fc = Faa = Fab = Fac = 0;
  FLattice = 0;
  try  {
    TStrList S, toks;
    S.LoadFromFile(FN);
    for( size_t i=0; i < S.Count(); i++ )  {
      if( S[i].IsEmpty() || S[i][0] != '_' )  continue;
      toks.Clear();
      S[i].Replace('\t', ' ');
      toks.Strtok(S[i].LowerCase(), ' ');
      if( toks.Count() < 2 )  continue;
      if( toks[0] == "_cell_length_a" )
        Fa = parse_double(toks[1]);
      else if( toks[0] == "_cell_length_b" )
        Fb = parse_double(toks[1]);
      else if( toks[0] == "_cell_length_c" )
        Fc = parse_double(toks[1]);
      else if( toks[0] == "_cell_angle_alpha" )
        Faa = parse_double(toks[1]);
      else if( toks[0] == "_cell_angle_beta" )
        Fab = parse_double(toks[1]);
      else if( toks[0] == "_cell_angle_gamma" )
        Fac = parse_double(toks[1]);
      else if( toks[0] == "_symmetry_space_group_name_h-m" )  {
        for( size_t j=0; j < toks[1].Length(); j++ )  {
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
      if( Fa && Fb && Fc && Faa && Fab && Fac && FLattice )
        return true;
    }
  }
  catch(const TExceptionBase& exc)  {  }
  return false;
}

#pragma package(smart_init)

