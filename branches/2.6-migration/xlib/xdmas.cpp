#include "xdmas.h"
#include "symmlib.h"
#include "symmparser.h"

void TXDMas::LoadFromStrings(const TStrList& Strings)  {
  olxstr crdfn( TEFile::ChangeFileExt(GetFileName(), "inp") );
  if( !TEFile::FileExists(crdfn) )
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate coordinates file");

  GetAsymmUnit().Clear();

  TStrList crds, toks, symm;
  crds.LoadFromFile( crdfn );
  bool CellFound = false, LattFound = false;
  double Q[6];

  for( int i=0; i < Strings.Count(); i++ )  {
    if( Strings[i].StartFromi("CELLSD") )  {
      toks.Clear();
      toks.Strtok( Strings[i], ' ');
      if( toks.Count() < 7 )  
        throw TFunctionFailedException(__OlxSourceInfo, "inlvaid CELLSD instruction");

      GetAsymmUnit().Axes()[0].E() = toks[1].ToDouble();
      GetAsymmUnit().Axes()[1].E() = toks[2].ToDouble();
      GetAsymmUnit().Axes()[2].E() = toks[3].ToDouble();
      GetAsymmUnit().Angles()[0].E() = toks[4].ToDouble();
      GetAsymmUnit().Angles()[1].E() = toks[5].ToDouble();
      GetAsymmUnit().Angles()[2].E() = toks[6].ToDouble();
    }
    else if( Strings[i].StartFromi("CELL") )  {
      toks.Clear();
      toks.Strtok( Strings[i], ' ');
      if( toks.Count() < 7 )  
        throw TFunctionFailedException(__OlxSourceInfo, "inlvaid CELL instruction");
      GetAsymmUnit().Axes()[0] = toks[1];
      GetAsymmUnit().Axes()[1] = toks[2];
      GetAsymmUnit().Axes()[2] = toks[3];
      GetAsymmUnit().Angles()[0] = toks[4];
      GetAsymmUnit().Angles()[1] = toks[5];
      GetAsymmUnit().Angles()[2] = toks[6];
      CellFound = true;
      GetAsymmUnit().InitMatrices();
    }
    else if( Strings[i].StartFromi("WAVE") )  {
      SetRadiation( Strings[i].SubStringFrom(4).Trim(' ').ToDouble() );
    }
    else if( Strings[i].StartFromi("SYMM") )  {
      symm.Add( Strings[i] );
    }
    else if( Strings[i].StartFromi("LATT") )  {
      toks.Clear();
      toks.Strtok( Strings[i], ' ');
      if( toks.Count() < 3 )  
        throw TFunctionFailedException(__OlxSourceInfo, "inlvaid LATT instruction");
      olxch centroflag = olxstr::o_toupper(toks[1][0]); 
      olxch centering = olxstr::o_toupper(toks[2][0]); 
      TCLattice* latt = TSymmLib::GetInstance()->FindLattice(centering);
      if( latt == NULL )  
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("inlvaid lattice symbol '") << centering << '\'');
      int ilatt = latt->GetLatt();
      if( centroflag == 'A' ) 
        ilatt *= -1;
      else if( centroflag == 'C' ) 
        ;
      else
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("inlvaid centering symbol '") << centroflag << '\'');
      GetAsymmUnit().SetLatt( ilatt );
      LattFound = true;
    }
  }
  if( !CellFound || !LattFound )
    throw TFunctionFailedException(__OlxSourceInfo, "CELL or LATT are missing");

  smatd sm;
  for( int i=0; i < symm.Count(); i++ )  {
    if( TSymmParser::SymmToMatrix(symm[i], sm) )
      GetAsymmUnit().AddMatrix(sm);
  }
  
  for( int i=0; i < crds.Count(); i++ )  {
    if( crds[i].IndexOf('(' ) != -1 )  {
      toks.Clear();
      toks.Strtok(crds[i], ' ');
      if( toks.Count() == 16 )  {
        TCAtom& atom = GetAsymmUnit().NewAtom();
        atom.SetUiso( 4*caDefIso*caDefIso );
        toks[0].DeleteChars(')');
        toks[0].DeleteChars('(');
        atom.SetLabel( toks[0].Length() > 4 ? toks[0].SubStringTo(4) : toks[0] );
        atom.ccrd()[0] = toks[12].ToDouble();
        atom.ccrd()[1] = toks[13].ToDouble();
        atom.ccrd()[2] = toks[14].ToDouble();
        if( (i+1) < crds.Count() && crds[i+1].IndexOf('(') == -1 )  {
          toks.Clear();
          toks.Strtok( crds[i+1], ' ');
          if( toks.Count() != 6 )  continue;
          Q[0] = toks[0].ToDouble();
          Q[1] = toks[1].ToDouble();
          Q[2] = toks[2].ToDouble();
          Q[3] = toks[5].ToDouble();
          Q[4] = toks[4].ToDouble();  // note swaped 3 and 5 vs. SHELX
          Q[5] = toks[3].ToDouble();
          if( Q[1] == 0 && Q[2] == 0 && Q[3] == 0 && Q[4] == 0 && Q[5] == 0  )  {
            atom.SetUiso( Q[0] );
          }
          else  {
            GetAsymmUnit().UcifToUcart(Q);
            atom.AssignEllp(& GetAsymmUnit().NewEllp().Initialise(Q) );
            if( atom.GetEllipsoid()->IsNPD() )  {
              TBasicApp::GetLog().Info(olxstr("Not positevely defined: ") << atom.GetLabel());
              atom.SetUiso( 0 );
            }
            else
              atom.SetUiso( (Q[0] +  Q[1] + Q[2])/3);
          }
          i++;
        }
      }
    }
  }

}
