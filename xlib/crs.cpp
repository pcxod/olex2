#include "crs.h"
#include "ins.h"
#include "asymmunit.h"

#include "symmlib.h"

TCRSFile::TCRSFile()  {
  SGInitialised = false;
}
//..............................................................................
TSpaceGroup* TCRSFile::GetSG()  {
  return SGInitialised ? TSymmLib::GetInstance()->FindSG( GetAsymmUnit() ) : NULL;
}
//..............................................................................
void TCRSFile::SaveToStrings(TStrList& SL)  {
  olxstr Tmp;

  SL.Add(olxstr("TITLE   ") << GetTitle() );

  Tmp = "CELL ";
  Tmp << GetRM().expl.GetRadiation() << ' ' <<
  GetAsymmUnit().Axes()[0].GetV() << ' ' <<
  GetAsymmUnit().Axes()[1].GetV()  << ' ' <<
  GetAsymmUnit().Axes()[2].GetV() << ' ' <<
  GetAsymmUnit().Angles()[0].GetV() << ' ' <<
  GetAsymmUnit().Angles()[1].GetV() << ' ' <<
  GetAsymmUnit().Angles()[2].GetV();
  SL.Add(Tmp);

  Tmp = "ZERR ";
  Tmp << GetAsymmUnit().GetZ() << ' ' <<
  GetAsymmUnit().Axes()[0].GetE() << ' ' <<
  GetAsymmUnit().Axes()[1].GetE() << ' ' <<
  GetAsymmUnit().Axes()[2].GetE() << ' ' <<
  GetAsymmUnit().Angles()[0].GetE() << ' ' <<
  GetAsymmUnit().Angles()[1].GetE() << ' ' <<
  GetAsymmUnit().Angles()[2].GetE();
  SL.Add(Tmp);

  TSpaceGroup* sg = GetSG();
  if( sg != NULL )  {
    Tmp = "LATT ";
    Tmp << sg->GetBravaisLattice().GetName() << ' ' << sg->GetLattice().GetSymbol();
    SL.Add(Tmp);

    Tmp = "SPGR ";
    Tmp << sg->GetName();
    SL.Add(Tmp);
  }
  else
    throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
  SL.Add(EmptyString);
  TIns::SaveSfacUnit(GetRM(), GetRM().GetUserContent(), SL, SL.Count()-1);
}
//..............................................................................
void TCRSFile::LoadFromStrings(const TStrList& Strings)  {
  SGInitialised = false;
  olxstr Tmp, TmpUC, Cell, Zerr, Sg, fcId("FACE"), sfac, unit;
  TStrList toks;
  TStrPObjList<olxstr, olxstr*> params;
  params.Add("TITL", &Title);
  params.Add("CELL", &Cell);
  params.Add("ZERR", &Zerr);
  params.Add("SFAC", &sfac);
  params.Add("UNIT", &unit);
  params.Add("SPGR", &Sg);
  for( size_t i=0; i < Strings.Count(); i++ )  {
    Tmp = olxstr::DeleteSequencesOf<char>( Strings[i], ' ' );
    if( Tmp.IsEmpty() )  continue;
    TmpUC = Tmp.UpperCase();
    for( size_t j=0; j < params.Count(); j++ )  {
      if( TmpUC.StartsFrom( params[j] ) ) {
        *params.GetObject(j) = Tmp.SubStringFrom( params[j].Length() );
        params.Delete(j);
        break;
      }
    }
    // a crystal face
    if( TmpUC.StartsFrom( fcId ) )  {
      toks.Clear();
      toks.Strtok( Tmp.SubStringFrom( fcId.Length() ), ' ');
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
  toks.Strtok( Cell, ' ');
  if( toks.Count() >= 7 )  {
    GetRM().expl.SetRadiation(toks[0].ToDouble());
    GetAsymmUnit().Axes()[0].V() = toks[1].ToDouble();
    GetAsymmUnit().Axes()[1].V() = toks[2].ToDouble();
    GetAsymmUnit().Axes()[2].V() = toks[3].ToDouble();
    GetAsymmUnit().Angles()[0].V() = toks[4].ToDouble();
    GetAsymmUnit().Angles()[1].V() = toks[5].ToDouble();
    GetAsymmUnit().Angles()[2].V() = toks[6].ToDouble();
  }
  toks.Clear();
  toks.Strtok( Zerr, ' ');
  if( toks.Count() >= 7 )  {
    GetAsymmUnit().SetZ( toks[0].ToInt() );
    GetAsymmUnit().Axes()[0].E() = toks[1].ToDouble();
    GetAsymmUnit().Axes()[1].E() = toks[2].ToDouble();
    GetAsymmUnit().Axes()[2].E() = toks[3].ToDouble();
    GetAsymmUnit().Angles()[0].E() = toks[4].ToDouble();
    GetAsymmUnit().Angles()[1].E() = toks[5].ToDouble();
    GetAsymmUnit().Angles()[2].E() = toks[6].ToDouble();
  }

  Sg.DeleteChars(' ');
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindGroup( Sg );
  if( sg != NULL )  {
    GetAsymmUnit().ChangeSpaceGroup( *sg );
    SGInitialised = true;
  }
  GetRM().SetUserContent(sfac, unit);
}
//..............................................................................
bool TCRSFile::Adopt(TXFile& f)  {
  GetAsymmUnit().Angles() = f.GetAsymmUnit().Angles();
  GetAsymmUnit().Axes() = f.GetAsymmUnit().Axes();
  GetAsymmUnit().ChangeSpaceGroup(f.GetLastLoaderSG());
  SGInitialised = true;
  TStrPObjList<olxstr, TBasicAtomInfo*> BasicAtoms;
  Title = f.LastLoader()->GetTitle();
  GetRM().SetHKLSource(f.LastLoader()->GetRM().GetHKLSource());
  return true;
}
//..............................................................................

