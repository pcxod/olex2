/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "p4p.h"
#include "asymmunit.h"
#include "unitcell.h"
#include "symmlib.h"
#include "xmacro.h"

void TP4PFile::Clear()  {
  GetRM().Clear(rm_clear_ALL);
  GetAsymmUnit().Clear();
}
//.............................................................................
void TP4PFile::SaveToStrings(TStrList& SL)  {
  SL.Add("FILEID Created by OLEX2: ") << XLibMacros::GetCompilationInfo();
  SL.Add(olxstr("TITLE   ") << GetTitle());
  SL.Add(olxstr("CHEM    ") << GetRM().GetUserContentStr());

  TEValueD vol = TUnitCell::CalcVolumeEx(GetAsymmUnit());
  SL.Add("CELL").stream(' ')
    << GetAsymmUnit().GetAxes()[0]
    << GetAsymmUnit().GetAxes()[1]
    << GetAsymmUnit().GetAxes()[2]
    << GetAsymmUnit().GetAngles()[0]
    << GetAsymmUnit().GetAngles()[1]
    << GetAsymmUnit().GetAngles()[2]
    << vol.GetV();

  SL.Add("CELLSD").stream(' ')
    << GetAsymmUnit().GetAxisEsds()[0]
    << GetAsymmUnit().GetAxisEsds()[1]
    << GetAsymmUnit().GetAxisEsds()[2]
    << GetAsymmUnit().GetAngleEsds()[0]
    << GetAsymmUnit().GetAngleEsds()[1]
    << GetAsymmUnit().GetAngleEsds()[2]
    << vol.GetE();

  SL.Add("MORPH   ") << GetMorph();
  SL.Add("CCOLOR  ") << GetColor();
  SL.Add("CSIZE ").stream(' ')
    << GetRM().expl.GetCrystalSize()[0]
    << GetRM().expl.GetCrystalSize()[0]
    << GetRM().expl.GetCrystalSize()[1]
    << GetRM().expl.GetCrystalSize()[2];
    if( GetRM().expl.IsTemperatureSet() )
      SL.GetLastString() << ' ' << GetRM().expl.GetTempValue().ToString();
    else
      SL.GetLastString() << " 0";
    double tr = olx_round(GetRM().expl.GetRadiation(), 100);
    olxstr str_rad = '?';
    if (tr == 0.71)  str_rad = "Mo";
    else if (tr == 1.54)  str_rad = "Cu";
    else if (tr == 0.56)  str_rad = "Ag";
    SL.Add("SOURCE  ") << str_rad << ' ' << GetRM().expl.GetRadiation();
  // save only if preset
  if( !SGString.IsEmpty() )
    SL.Add("SG  ") << SGString;
}

void TP4PFile::LoadFromStrings(const TStrList& Strings)  {
  Clear();
  olxstr Cell, CellSd, Size, Source, chem;
  TStringToList<olxstr, olxstr*> params;
  params.Add("SITEID",  &SiteId);
  params.Add("MORPH",   &Morph);
  params.Add("CCOLOR",  &Color);
  params.Add("CSIZE",   &Size);
  params.Add("CHEM",    &chem);
  params.Add("MOSAIC",  &Mosaic);
  params.Add("SYMM",    &Symm);
  params.Add("BRAVAIS", &Bravais);
  params.Add("SOURCE",  &Source);
  params.Add("CELL",    &Cell);
  params.Add("CELLSD",  &CellSd);
  params.Add("TITLE",   &Title);
  params.Add("SG",      &SGString);
  params.Add("FILEID",  &FileId);
  for( size_t i=0; i < Strings.Count(); i++ )  {
    olxstr Tmp = olxstr::DeleteSequencesOf<char>(Strings[i], ' ');
    if( Tmp.IsEmpty() )  continue;
    olxstr TmpUC = Tmp.ToUpperCase();
    for( size_t j=0; j < params.Count(); j++ )  {
      if( TmpUC.StartsFrom(params[j] )) {
        *params.GetObject(j) = Tmp.SubStringFrom(params[j].Length()).Trim(' ');
        params.Delete(j);
        break;
      }
    }
    // do not read reflections and the background etc
    if( !params.Count() || TmpUC.StartsFrom("REF") )  break;
  }
  if( Cell.Length() == 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "could not locate CELL");
  params.Clear();
  params.Strtok( Cell, ' ');
  if( params.Count() >= 6 )  {
    GetAsymmUnit().GetAxes()[0] = params[0].ToDouble();
    GetAsymmUnit().GetAxes()[1] = params[1].ToDouble();
    GetAsymmUnit().GetAxes()[2] = params[2].ToDouble();
    GetAsymmUnit().GetAngles()[0] = params[3].ToDouble();
    GetAsymmUnit().GetAngles()[1] = params[4].ToDouble();
    GetAsymmUnit().GetAngles()[2] = params[5].ToDouble();
    GetAsymmUnit().InitMatrices();
  }
  params.Clear();
  params.Strtok( CellSd, ' ');
  if( params.Count() >= 6 )  {
    GetAsymmUnit().GetAxisEsds()[0] = params[0].ToDouble();
    GetAsymmUnit().GetAxisEsds()[1] = params[1].ToDouble();
    GetAsymmUnit().GetAxisEsds()[2] = params[2].ToDouble();
    GetAsymmUnit().GetAngleEsds()[0] = params[3].ToDouble();
    GetAsymmUnit().GetAngleEsds()[1] = params[4].ToDouble();
    GetAsymmUnit().GetAngleEsds()[2] = params[5].ToDouble();
  }
  params.Clear();
  params.Strtok(Bravais, ' ');
  if (!params.IsEmpty()) {
    olxstr c = params.GetLastString();
    if (c.Length() == 1) { // centering?
      try {
        GetAsymmUnit().SetLatt(
          TCLattice::LattForSymbol(c.CharAt(0)));
      }
      catch (const TExceptionBase &e) {}
    }
  }
  params.Clear();
  params.Strtok( Size, ' ');
  if( params.Count() == 5 )  {
    if( !params[4].IsNumber() )  {
      size_t bi = params[4].IndexOf('(');
      if( bi != InvalidIndex && bi > 0 )
        params[4] = params[4].SubStringTo(bi);
    }
    if( params[4].IsNumber() )
      GetRM().expl.SetTemp(params[4]);
    params.Delete(4);
  }
  while (params.Count() > 3)
    params.Delete(params.Count() - 1);

  if( params.Count() == 3 )  {
    if( !params[0].IsNumber() )  params[0] = '0';
    if( !params[1].IsNumber() )  params[1] = '0';
    if( !params[2].IsNumber() )  params[2] = '0';
    GetRM().expl.SetCrystalSize(params[0].ToDouble(),
      params[1].ToDouble(), params[2].ToDouble());
  }

  params.Clear();
  params.Strtok(Source, ' ');
  if( params.Count() > 2 )
    GetRM().expl.SetRadiation(params[1].ToDouble());
  try  {  GetRM().SetUserFormula(chem.DeleteChars('_'));  }
  catch(...)  {  }  // just skip...
}

bool TP4PFile::Adopt(TXFile &f, int) {
  GetRM().Assign(f.GetRM(), false);
  GetAsymmUnit().GetAxes() = f.GetAsymmUnit().GetAxes();
  GetAsymmUnit().GetAxisEsds() = f.GetAsymmUnit().GetAxisEsds();
  GetAsymmUnit().GetAngles() = f.GetAsymmUnit().GetAngles();
  GetAsymmUnit().GetAngleEsds() = f.GetAsymmUnit().GetAngleEsds();
  Title = f.LastLoader()->GetTitle();
  SiteId  = "?";
  Morph   = SiteId;
  Color   = SiteId;
  Mosaic  = SiteId;
  Symm = SiteId;
  SGString = SiteId;
  Bravais = SiteId;
  try  {
    TSpaceGroup& sg = f.GetLastLoaderSG();
    SGString = sg.GetName();
    Bravais = sg.GetBravaisLattice().GetName();
  }
  catch( ... )  {  }
  return true;
}
