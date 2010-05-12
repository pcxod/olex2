#include "pdb.h"
#include "unitcell.h"
#include "bapp.h"
#include "log.h"

void TPdb::Clear()  {
  GetAsymmUnit().Clear();
}
//..............................................................................
void TPdb::SaveToStrings(TStrList& Strings)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TPdb::LoadFromStrings(const TStrList& Strings)  {
  Clear();

  evecd QE(6);
  TStrList toks;
  TSizeList CrystF;
  CrystF.Add(6);
  CrystF.Add(9);
  CrystF.Add(9);
  CrystF.Add(9);
  CrystF.Add(7);
  CrystF.Add(7);
  CrystF.Add(7);

  TSizeList AtomF;
  AtomF.Add(6);  //"ATOM  "
  AtomF.Add(5);  //serial number
  AtomF.Add(1);  //ws
  AtomF.Add(4);  //name
  AtomF.Add(1);  //alternative location indicator
  AtomF.Add(3);  //residue name
  AtomF.Add(1);  //ws
  AtomF.Add(1);  //chain ID
  AtomF.Add(4);  //  residue sequence number
  AtomF.Add(4);  //iCode + 3 ws
  AtomF.Add(8);  // x
  AtomF.Add(8);  // y
  AtomF.Add(8);  // z
  AtomF.Add(6);  // occupancy
  AtomF.Add(6);  // temperature factor
  AtomF.Add(2);  // element
  AtomF.Add(2);  // charge

  TSizeList AnisF;
  AnisF.Add(6); // record name
  AnisF.Add(5);  // serial
  AnisF.Add(1);  // ws
  AnisF.Add(4);  // atom name
  AnisF.Add(1);  // alternative location
  AnisF.Add(3);  // residue name
  AnisF.Add(1);  // ws
  AnisF.Add(1);  // chain ID
  AnisF.Add(4);  // residue sequence number
  AnisF.Add(2);  // iCode +  1 ws
  AnisF.Add(7);  // U00
  AnisF.Add(7);  // U11
  AnisF.Add(7);  // U22
  AnisF.Add(7);  // U01
  AnisF.Add(7);  // U02
  AnisF.Add(7);  // U12
  AnisF.Add(2);  // element
  AnisF.Add(2);  // Charge


  Title = "OLEX: imported from PDB";
  for( size_t i=0; i < Strings.Count(); i++ )  {
    size_t spi = Strings[i].FirstIndexOf(' ');
    if( spi == InvalidIndex || spi == 0 )  continue;
    olxstr line = Strings[i].SubStringTo(spi).UpperCase();
    if( line == "CRYST1" )  {
      toks.StrtokF( Strings[i], CrystF);
      if( toks.Count() < 7 )  
        throw TFunctionFailedException(__OlxSourceInfo, "parsing failed");
      GetAsymmUnit().Axes()[0] = toks[1].ToDouble();
      GetAsymmUnit().Axes()[1] = toks[2].ToDouble();
      GetAsymmUnit().Axes()[2] = toks[3].ToDouble();
      GetAsymmUnit().Angles()[0] = toks[4].ToDouble();
      GetAsymmUnit().Angles()[1] = toks[5].ToDouble();
      GetAsymmUnit().Angles()[2] = toks[6].ToDouble();
      GetAsymmUnit().InitMatrices();
    }
    else if( line == "ATOM" )  {
      toks.Clear();
      toks.StrtokF( Strings[i], AtomF);
      if( toks.Count() < 13 )  
        throw TFunctionFailedException(__OlxSourceInfo, "parsing failed");
      TCAtom& CA = GetAsymmUnit().NewAtom();
      vec3d crd(toks[10].ToDouble(), toks[11].ToDouble(), toks[12].ToDouble());
      GetAsymmUnit().CartesianToCell(crd);
      CA.ccrd() = crd;
      olxstr Tmp = toks[3].Trim(' ');
      if( Tmp == "CA" )
        Tmp = "C";
      else  if( Tmp == "CD" )
        Tmp = "C";
      else  if( Tmp == "CE" )
        Tmp = "C";
      else  if( Tmp == "W" )
        Tmp = "O";
      Tmp << '_' << toks[5].Trim(' ') << '_' << toks[8].Trim(' ');
      CA.SetLabel( Tmp );
    }
    else if( line == "ANISOU" )  {
      toks.Clear();
      toks.StrtokF(Strings[i], AnisF);
      if( toks.Count() < 16 )  
        throw TFunctionFailedException(__OlxSourceInfo, "parsing failed");
      QE[0] = toks[10].ToDouble();
      QE[1] = toks[11].ToDouble();
      QE[2] = toks[12].ToDouble();
      QE[3] = toks[15].ToDouble();
      QE[4] = toks[14].ToDouble();
      QE[5] = toks[13].ToDouble();
      QE /= 10000;
      TCAtom* ca = GetAsymmUnit().FindCAtomById( toks[1].ToInt() );
      if( ca != NULL )  {
        ca->UpdateEllp(QE);
        if( ca->GetEllipsoid()->IsNPD() )
          TBasicApp::GetLog().Error(olxstr("Not positevely defined: ") + ca->GetLabel());
        ca->SetUiso((QE[0] +  QE[1] + QE[2])/3);
      }
    }
  }
}
//..............................................................................
bool TPdb::Adopt(TXFile& XF)  {
  Clear();
  GetAsymmUnit().Assign(XF.GetAsymmUnit());
  GetAsymmUnit().SetZ((short)XF.GetLattice().GetUnitCell().MatrixCount());
  return true;
}
//..............................................................................




