//----------------------------------------------------------------------------//
// TBasicAtomsInfo  - basic atom data
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "atominfo.h"
#include "efile.h"
#include "estrlist.h"
#include "exception.h"
#include <stdlib.h>
// RGBA

int Colors[]={
          0xffffff, // white
          0x008000,
          0x0000ff,
          0xff0000,
          0x00ffff,
          0x800080,
          0xffffff,
          0x808080,
          0x808000,
          0x008000,
          0x800000,
          0x00ffff,
          0x0000ff,
          0x000000};

//---------------------------------------------------------------------------
// TBasicAtomInfo function bodies
//---------------------------------------------------------------------------

TBasicAtomInfo::TBasicAtomInfo()  {
  Isotopes = NULL;
}
//..............................................................................
TBasicAtomInfo::~TBasicAtomInfo()  {
  if( Isotopes != NULL )
    delete Isotopes;
}
//..............................................................................
TIsotope& TBasicAtomInfo::NewIsotope()  {
  if( Isotopes == NULL )  Isotopes = new TIsotopeList;
  return Isotopes->AddNew();
}
olxstr TBasicAtomInfo::StrRepr() const  {
  olxstr Tmp;
  olxstr Tmp1;
  Tmp1 = GetSymbol();    Tmp.Format(3, true, ' ');
  Tmp = Tmp1;
  Tmp1 = GetMr();        Tmp << Tmp1.Format(14, true, ' ');
  Tmp1 = GetName();      Tmp << Tmp1.Format(12, true, ' ');
  Tmp1 = GetDefColor();  Tmp << Tmp1.Format(12, true, ' ');
  Tmp1 = GetRad();       Tmp << Tmp1.Format(6, true, ' ');
  Tmp1 = GetRad1();      Tmp << Tmp1.Format(6, true, ' ');
  Tmp1 = GetRad2();      Tmp << Tmp1.Format(6, true, ' ');
  if( Isotopes != NULL )  {
    for( int i=0; i < Isotopes->Count(); i++ )  {
      Tmp1 = Isotopes->Item(i).GetMr();
      Tmp << Tmp1.Format(14, true, ' ');
      Tmp1 = Isotopes->Item(i).GetW()*100;
      Tmp << Tmp1.Format(8, true, ' ');
    }
  }
  return Tmp;
}
//..............................................................................
//---------------------------------------------------------------------------
// TAtomsInfo function bodies
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TAtomsInfo::TAtomsInfo(const olxstr &filename)  {
  olxstr Tmp;
  TCStrList List, Toks;
  int color;
  List.LoadFromFile(filename);
  if( List.Count() < 100 )
    throw TInvalidArgumentException(__OlxSourceInfo, "unexpected end of the stream");
  for( int i=1; i< List.Count(); i++ )  {
    Toks.Clear();
    Toks.Strtok(List[i], ' ');
    if( Toks.Count() >= 7 )  {
      TBasicAtomInfo& I = Data.AddNew();
      I.SetSymbol( Toks.String(0) );
      I.SetMr( Toks[1].ToDouble() );
      I.SetName( Toks[2] );
      color = Toks[3].ToInt();
      if(  color >= 1 && color <= 14 )
        I.SetDefColor( Colors[color] );
      I.SetRad( Toks[4].ToDouble() );
      I.SetRad1( Toks[5].ToDouble() );
      I.SetRad2( Toks[6].ToDouble() );
      I.SetIndex( (short)(i-1) );
      if( Toks.Count() > 7 )  {
        for( int j=7; j < Toks.Count(); j+=2 )  {
          TIsotope& Is = I.NewIsotope();
          Is.SetMr( Toks[j].ToDouble() );
          Is.SetW( Toks[j+1].ToDouble()/100 );
        }
      }
    }
  }
}
//..............................................................................
TAtomsInfo::~TAtomsInfo()  {  }
//..............................................................................
void TAtomsInfo::SaveToFile(const olxstr &filename) const  {
  TCStrList L;
  L.Add("XLIB: (c) Oleg V. Dolomanov 2004");
  for( int i=0; i < Count(); i++ )
    L.Add( GetAtomInfo(i).StrRepr() );
  L.SaveToFile( filename );
}
//..............................................................................
TBasicAtomInfo* TAtomsInfo::FindAtomInfoBySymbol(const olxstr &Symbol) const  {
  for( int i=0; i < Data.Count(); i++ )
    if( Data[i].GetSymbol().Comparei(Symbol) == 0 )
      return &Data[i];
  return NULL;
}
//..............................................................................
TBasicAtomInfo* TAtomsInfo::FindAtomInfoEx(const olxstr &Str) const {
  if( !Str.Length() )  return NULL;

  int l = Str.Length(), dl = abs('A'-'a');
  TBasicAtomInfo *I;
  if( l >= 2 &&
    (( (Str[0]>='a' && Str[0]<='z')    ||
       (Str[0]>='A' && Str[0]<='Z') )  &&
    (  (Str[1]>='a' && Str[1]<='z')    ||
       (Str[1]>='A' && Str[1]<='Z') ))   )  {  // searching between two charachter elements
    for( short j=0; j < Data.Count(); j++ )  {
      I = &GetAtomInfo(j);
      if( I->GetSymbol().Length() == 2 )  {
        short da = abs( I->GetSymbol()[0]- Str[0] ),
              db = abs( I->GetSymbol()[1]- Str[1] );
        if( (!da && !db) || (!da && db==dl) || (da==dl && !db) || (da==dl && db==dl) )
          return I;
      }
    }
  }
  if( (Str[0]>='a' && Str[0]<='z') || (Str[0]>='A' && Str[0]<='Z') )  {
    for( short j=0; j < Data.Count(); j++ )  {
      I = &GetAtomInfo(j);
      if( I->GetSymbol().Length() == 1 )  {
        short da = abs( I->GetSymbol()[0]- Str[0] );
        if( (!da) || (da == dl) )
          return I;
      }
    }
  }
  return NULL;
}
//..............................................................................
void TAtomsInfo::ParseSimpleElementStr(const olxstr& str, TStrList& toks)  const {
  olxstr elm;
  for( int i=0; i < str.Length(); i++ )  {
    if( str[i] >= 'A' && str[i] <= 'Z' )  {
      if( elm.Length() )  {
        if( (i+1) < str.Length() )  {
          if( str[i+1] >= 'A' && str[i+1] <= 'Z' )  {
            if( IsElement( elm + str[i+1] ) )  {
              i++;
              toks.Add(elm);
              if( (i+1) < str.Length() )
                elm = str[i];
            }
          }
        }
        if( !IsElement( elm ) )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Unknown element - ") << elm);
        toks.Add(elm);
        elm = str[i];
      }
      else
        elm = str[i];
    }
    else
      elm << str[i];
  }
  if( elm.Length() )  {
    if( !IsElement( elm ) )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Unknown element - ") << elm);
    toks.Add(elm);
  }
}
//..............................................................................
void TAtomsInfo::ParseElementString(const olxstr& su, TTypeList<AnAssociation2<olxstr, int> >& res)  const {
  olxstr elm, cnt;
  bool nowCnt = false;
  TStrList toks;
  for( int i=0; i < su.Length(); i++ )  {
    if( su[i] == ' ' )  continue;
    if( nowCnt )  {
      if( (su[i] >='0' && su[i] <= '9') || su[i] == '.' )  {
        cnt << su[i];
      }
      else  {
        if( elm.Length() && cnt.Length() )  {
          toks.Clear();
          ParseSimpleElementStr( elm, toks );
          for( int i=0; i < toks.Count()-1; i++ )
            res.AddNew<olxstr, int>(toks.String(i), 1);
          res.AddNew<olxstr, int>(toks.String( toks.Count() -1 ), cnt.ToInt());
          cnt = EmptyString;
        }
        nowCnt = false;
        elm = su[i];
      }
    }
    else  {
      if( !( (su[i] >='0' && su[i] <= '9') || su[i] == '.') )
        elm << su[i];
      else  {
        nowCnt = true;
        cnt = su[i];
      }
    }
  }
  if( elm.Length() )  {
    toks.Clear();
    ParseSimpleElementStr( elm, toks );
    for( int i=0; i < toks.Count()-1; i++ )
      res.AddNew<olxstr, int>(toks.String(i), 1);
    if( cnt.Length() )
      res.AddNew<olxstr, int>(toks.String( toks.Count() -1 ), cnt.ToInt());
    else
      res.AddNew<olxstr, int>(toks.String( toks.Count() -1 ), 1);
  }
}
//..............................................................................

class TLibrary*  ExportLibrary(const olxstr& name);
//..............................................................................

