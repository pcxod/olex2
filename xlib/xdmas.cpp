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
  TVectorD QE(6);

  for( int i=0; i < Strings.Count(); i++ )  {
    if( Strings[i].StartFromi("CELLSD") )  {
      toks.Clear();
      toks.Strtok( Strings[i], ' ');
      if( toks.Count() < 7 )  
        throw TFunctionFailedException(__OlxSourceInfo, "inlvaid CELLSD instruction");

      GetAsymmUnit().Axes().Value(0).E() = toks[1].ToDouble();
      GetAsymmUnit().Axes().Value(1).E() = toks[2].ToDouble();
      GetAsymmUnit().Axes().Value(2).E() = toks[3].ToDouble();
      GetAsymmUnit().Angles().Value(0).E() = toks[4].ToDouble();
      GetAsymmUnit().Angles().Value(1).E() = toks[5].ToDouble();
      GetAsymmUnit().Angles().Value(2).E() = toks[6].ToDouble();
      int j = 0;
      if( GetAsymmUnit().Axes()[0].GetE() != 0 )  {  Error += GetAsymmUnit().Axes()[0].GetE()/GetAsymmUnit().Axes()[0].GetV(); j++; }
      if( GetAsymmUnit().Axes()[1].GetE() != 0 )  {  Error += GetAsymmUnit().Axes()[1].GetE()/GetAsymmUnit().Axes()[1].GetV(); j++; }
      if( GetAsymmUnit().Axes()[2].GetE() != 0 )  {  Error += GetAsymmUnit().Axes()[2].GetE()/GetAsymmUnit().Axes()[2].GetV(); j++; }
      if( GetAsymmUnit().Angles()[0].GetE() != 0 ){  Error += GetAsymmUnit().Angles()[0].GetE()/GetAsymmUnit().Angles()[0].GetV(); j++; }
      if( GetAsymmUnit().Angles()[1].GetE() != 0 ){  Error += GetAsymmUnit().Angles()[1].GetE()/GetAsymmUnit().Angles()[1].GetV(); j++; }
      if( GetAsymmUnit().Angles()[2].GetE() != 0 ){  Error += GetAsymmUnit().Angles()[2].GetE()/GetAsymmUnit().Angles()[2].GetV(); j++; }
      if( j != 0 )  Error /= j;
    }
    else if( Strings[i].StartFromi("CELL") )  {
      toks.Clear();
      toks.Strtok( Strings[i], ' ');
      if( toks.Count() < 7 )  
        throw TFunctionFailedException(__OlxSourceInfo, "inlvaid CELL instruction");
      GetAsymmUnit().Axes().Value(0) = toks[1];
      GetAsymmUnit().Axes().Value(1) = toks[2];
      GetAsymmUnit().Axes().Value(2) = toks[3];
      GetAsymmUnit().Angles().Value(0) = toks[4];
      GetAsymmUnit().Angles().Value(1) = toks[5];
      GetAsymmUnit().Angles().Value(2) = toks[6];
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

  TMatrixD sm(3,4);
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
        atom.SetLoaderId(GetAsymmUnit().AtomCount()-1);
        atom.SetUiso( 4*caDefIso*caDefIso );
        toks[0].DeleteChars(')');
        toks[0].DeleteChars('(');
        atom.SetLabel( toks[0].Length() > 4 ? toks[0].SubStringTo(4) : toks[0] );
        atom.CCenter().Value(0) = toks.String(12);
        atom.CCenter().Value(1) = toks.String(13);
        atom.CCenter().Value(2) = toks.String(14);
        // initialise uncertanties using average cell error
        atom.CCenter().Value(0).E() = (float)fabs(atom.CCenter()[0].GetV()*Error);
        atom.CCenter().Value(1).E() = (float)fabs(atom.CCenter()[1].GetV()*Error);
        atom.CCenter().Value(2).E() = (float)fabs(atom.CCenter()[2].GetV()*Error);
        if( (i+1) < crds.Count() && crds[i+1].IndexOf('(') == -1 )  {
          toks.Clear();
          toks.Strtok( crds[i+1], ' ');
          if( toks.Count() != 6 )  continue;
          atom.EllpE().Resize(6);
          QE[0] = toks[0].ToDouble();
          QE[1] = toks[1].ToDouble();
          QE[2] = toks[2].ToDouble();
          QE[3] = toks[5].ToDouble();
          QE[4] = toks[4].ToDouble();  // note swaped 3 and 5 vs. SHELX
          QE[5] = toks[3].ToDouble();
          if( QE[1] == 0 && QE[2] == 0 && QE[3] == 0 && QE[4] == 0 && QE[5] == 0  )  {
            atom.SetUiso( QE[0] );
          }
          else  {
            GetAsymmUnit().UcifToUcart(QE);
            atom.UpdateEllp(QE);
            if( atom.GetEllipsoid()->IsNPD() )  {
              TBasicApp::GetLog().Info(olxstr("Not positevely defined: ") << atom.GetLabel());
              atom.SetUiso( 0 );
            }
            else
              atom.SetUiso( (QE[0] +  QE[1] + QE[2])/3);
          }
          i++;
        }
      }
    }
  }

}
