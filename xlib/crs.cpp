/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "crs.h"
#include "ins.h"
#include "asymmunit.h"
#include "symmlib.h"

TCRSFile::TCRSFile()  {
  SGInitialised = false;
}
//..............................................................................
void TCRSFile::Clear()  {
  GetRM().Clear(rm_clear_ALL);
  GetAsymmUnit().Clear();
  SGInitialised = false;
}
//..............................................................................
TSpaceGroup* TCRSFile::GetSG()  {
  return SGInitialised ? &TSymmLib::GetInstance().FindSG(GetAsymmUnit()) : NULL;
}
//..............................................................................
void TCRSFile::SaveToStrings(TStrList& SL)  {
  SL.Add("TITLE   ") << GetTitle();
  SL.Add("CELL").stream(' ')
    << GetRM().expl.GetRadiation()
    << GetAsymmUnit().GetAxes()[0]
    << GetAsymmUnit().GetAxes()[1]
    << GetAsymmUnit().GetAxes()[2]
    << GetAsymmUnit().GetAngles()[0]
    << GetAsymmUnit().GetAngles()[1]
    << GetAsymmUnit().GetAngles()[2];

  SL.Add("ZERR").stream(' ')
    << GetAsymmUnit().GetZ()
    << GetAsymmUnit().GetAxisEsds()[0]
    << GetAsymmUnit().GetAxisEsds()[1]
    << GetAsymmUnit().GetAxisEsds()[2]
    << GetAsymmUnit().GetAngleEsds()[0]
    << GetAsymmUnit().GetAngleEsds()[1]
    << GetAsymmUnit().GetAngleEsds()[2];

  TSpaceGroup* sg = GetSG();
  if( sg != NULL )  {
    SL.Add("LATT ")  << sg->GetBravaisLattice().GetName() << ' '
      << sg->GetLattice().GetSymbol();
    SL.Add("SPGR ") << sg->GetName();
  }
  else
    throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
  SL.Add();
  TIns::SaveSfacUnit(GetRM(), SL, SL.Count()-1, false);
}
//..............................................................................
void TCRSFile::LoadFromStrings(const TStrList& Strings)  {
  Clear();
  olxstr Tmp, TmpUC, Cell, Zerr, Sg, fcId("FACE"), sfac, unit;
  TStrList toks;
  TStringToList<olxstr, olxstr*> params;
  params.Add("TITL", &Title);
  params.Add("CELL", &Cell);
  params.Add("ZERR", &Zerr);
  params.Add("SFAC", &sfac);
  params.Add("UNIT", &unit);
  params.Add("SPGR", &Sg);
  for( size_t i=0; i < Strings.Count(); i++ )  {
    Tmp = olxstr::DeleteSequencesOf<char>(Strings[i], ' ');
    if( Tmp.IsEmpty() )  continue;
    TmpUC = Tmp.UpperCase();
    for( size_t j=0; j < params.Count(); j++ )  {
      if( TmpUC.StartsFrom( params[j] ) ) {
        *params.GetObject(j) = Tmp.SubStringFrom(params[j].Length());
        params.Delete(j);
        break;
      }
    }
    // a crystal face
    if( TmpUC.StartsFrom( fcId ) )  {
      toks.Clear();
      toks.Strtok(Tmp.SubStringFrom(fcId.Length()), ' ');
      if( toks.Count() == 4 )  {
        evecd& v = Faces.AddNew(4);
        v[0] = toks[0].ToDouble();
        v[1] = toks[1].ToDouble();
        v[2] = toks[2].ToDouble();
        v[3] = toks[3].ToDouble();
      }
    }
  }
  if( Cell.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate CELL");
  toks.Clear();
  toks.Strtok(Cell, ' ');
  if( toks.Count() >= 7 )  {
    GetRM().expl.SetRadiation(toks[0].ToDouble());
    GetAsymmUnit().GetAxes() =
      vec3d(toks[1].ToDouble(), toks[2].ToDouble(), toks[3].ToDouble());
    GetAsymmUnit().GetAngles() =
      vec3d(toks[4].ToDouble(), toks[5].ToDouble(), toks[6].ToDouble());
    GetAsymmUnit().InitMatrices();
    GetAsymmUnit().InitMatrices();
  }
  toks.Clear();
  toks.Strtok( Zerr, ' ');
  if( toks.Count() >= 7 )  {
    GetAsymmUnit().SetZ(static_cast<short>(olx_round(toks[0].ToDouble())));
    GetAsymmUnit().GetAxisEsds() =
      vec3d(toks[1].ToDouble(), toks[2].ToDouble(), toks[3].ToDouble());
    GetAsymmUnit().GetAngleEsds() =
      vec3d(toks[4].ToDouble(), toks[5].ToDouble(), toks[6].ToDouble());
  }

  Sg.DeleteChars(' ');
  TSpaceGroup* sg = TSymmLib::GetInstance().FindGroupByName(Sg);
  if( sg != NULL )  {
    GetAsymmUnit().ChangeSpaceGroup(*sg);
    SGInitialised = true;
  }
  GetRM().SetUserContent(sfac, unit);
}
//..............................................................................
bool TCRSFile::Adopt(TXFile &f, int) {
  GetAsymmUnit().GetAxes() = f.GetAsymmUnit().GetAxes();
  GetAsymmUnit().GetAxisEsds() = f.GetAsymmUnit().GetAxisEsds();
  GetAsymmUnit().GetAngles() = f.GetAsymmUnit().GetAngles();
  GetAsymmUnit().GetAngleEsds() = f.GetAsymmUnit().GetAngleEsds();
  GetAsymmUnit().ChangeSpaceGroup(f.GetLastLoaderSG());
  SGInitialised = true;
  Title = f.LastLoader()->GetTitle();
  GetRM().SetHKLSource(f.LastLoader()->GetRM().GetHKLSource());
  return true;
}
//..............................................................................
