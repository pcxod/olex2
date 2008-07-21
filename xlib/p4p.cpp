#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "p4p.h"
#include "asymmunit.h"
#include "symmlib.h"

TP4PFile::TP4PFile():TBasicCFile(NULL)  {
  Radiation = 0.71;
}

TP4PFile::~TP4PFile()  {  }

void TP4PFile::SaveToStrings(TStrList& SL)  {
  olxstr Tmp;

  SL.Add("FILEID Created by OLEX2");
  SL.Add(olxstr("TITLE   ") << GetTitle() );
  SL.Add(olxstr("CHEM    ") << Chem );

  Tmp = "CELL ";
              Tmp << GetAsymmUnit().Axes()[0].GetV();
  Tmp << ' '; Tmp << GetAsymmUnit().Axes()[1].GetV();
  Tmp << ' '; Tmp << GetAsymmUnit().Axes()[2].GetV();
  Tmp << ' '; Tmp << GetAsymmUnit().Angles()[0].GetV();
  Tmp << ' '; Tmp << GetAsymmUnit().Angles()[1].GetV();
  Tmp << ' '; Tmp << GetAsymmUnit().Angles()[2].GetV();
  SL.Add(Tmp);

  Tmp = "CELLSD "; Tmp << GetAsymmUnit().GetZ();
  Tmp << ' '; Tmp << GetAsymmUnit().Axes()[0].GetE();
  Tmp << ' '; Tmp << GetAsymmUnit().Axes()[1].GetE();
  Tmp << ' '; Tmp << GetAsymmUnit().Axes()[2].GetE();
  Tmp << ' '; Tmp << GetAsymmUnit().Angles()[0].GetE();
  Tmp << ' '; Tmp << GetAsymmUnit().Angles()[1].GetE();
  Tmp << ' '; Tmp << GetAsymmUnit().Angles()[2].GetE();
  SL.Add(Tmp);

  SL.Add(olxstr("MORPH   ") << GetMorph() );
  SL.Add(olxstr("CCOLOR  ") << GetColor() );
  SL.Add(olxstr("CSIZE  ") << GetSize() << ' '  << GetTemp() );
  SL.Add(olxstr("SOURCE  ") << GetSource() );
  // save only if preset
  if( !SG.IsEmpty() )
    SL.Add(olxstr("SG  ") << GetSG() );
}

void TP4PFile::LoadFromStrings(const TStrList& Strings)  {
  olxstr Tmp, TmpUC, Cell, CellSd;
  TStrPObjList<olxstr,olxstr*> params;
  params.Add("SITEID",  &SiteId);
  params.Add("MORPH",   &Morph);
  params.Add("CCOLOR",  &Color);
  params.Add("CSIZE",    &Size);
  params.Add("CHEM",    &Chem);
  params.Add("MOSAIC",  &Mosaic);
  params.Add("SYMM",    &Symm);
  params.Add("BRAVAIS", &Bravais);
  params.Add("SOURCE",   &Source);
  params.Add("CELL",   &Cell);
  params.Add("CELLSD",   &CellSd);
  params.Add("TITLE",   &FTitle);
  params.Add("SG",   &SG);
  for( int i=0; i < Strings.Count(); i++ )  {
    Tmp = olxstr::DeleteSequencesOf<char>( Strings[i], ' ' );
    if( Tmp.IsEmpty() )  continue;
    TmpUC = Tmp.UpperCase();
    for( int j=0; j < params.Count(); j++ )  {
      if( TmpUC.StartsFrom( params.String(j) ) ) {
        *params.Object(j) = Tmp.SubStringFrom( params.String(j).Length() ).Trim(' ');
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
    GetAsymmUnit().Axes()[0].V() = params.String(0).ToDouble();
    GetAsymmUnit().Axes()[1].V() = params.String(1).ToDouble();
    GetAsymmUnit().Axes()[2].V() = params.String(2).ToDouble();
    GetAsymmUnit().Angles()[0].V() = params.String(3).ToDouble();
    GetAsymmUnit().Angles()[1].V() = params.String(4).ToDouble();
    GetAsymmUnit().Angles()[2].V() = params.String(5).ToDouble();
  }
  params.Clear();
  params.Strtok( CellSd, ' ');
  if( params.Count() >= 6 )  {
    GetAsymmUnit().Axes()[0].E() = params.String(0).ToDouble();
    GetAsymmUnit().Axes()[1].E() = params.String(1).ToDouble();
    GetAsymmUnit().Axes()[2].E() = params.String(2).ToDouble();
    GetAsymmUnit().Angles()[0].E() = params.String(3).ToDouble();
    GetAsymmUnit().Angles()[1].E() = params.String(4).ToDouble();
    GetAsymmUnit().Angles()[2].E() = params.String(5).ToDouble();
  }
  params.Clear();
  params.Strtok( Size, ' ');
  if( params.Count() == 5 )  {
    Temp = params.String(4);
    params.Delete(4);
  }
  while( params.Count() > 3 )
    params.Delete( params.Count()-1 );

  if( params.Count() == 3 )
    Size = params.Text(' ');

  params.Clear();
  params.Strtok( Source, ' ');
  if( params.Count() > 2 )  {
    Radiation = params.String(1).ToDouble();
  }

  Chem.DeleteChars('_');
}

bool TP4PFile::Adopt(TXFile* f)  {

  Chem = f->GetAsymmUnit().SummFormula(' ');
  GetAsymmUnit().Angles() = f->GetAsymmUnit().Angles();
  GetAsymmUnit().Axes()   = f->GetAsymmUnit().Axes();
  if( f->GetLastLoader() )  {
    FTitle = f->GetLastLoader()->GetTitle();
    SetHKLSource( f->GetLastLoader()->GetHKLSource() );
  }
  else  {
    FTitle = "?";
    SetHKLSource( EmptyString );
  }

  SiteId  = "?";
  Morph   = "?";
  Color   = "?";
  Size    = "? ? ?";
  Mosaic  = "?";

  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(f->GetAsymmUnit());
  if( sg != NULL )  {
    Symm = sg->GetName();
    Bravais = sg->GetBravaisLattice().GetName();
  }
  else  {
    Symm    = "?";
    Bravais = "?";
  }

  return true;
}

