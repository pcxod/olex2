//---------------------------------------------------------------------------
#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "erange.h"
#include "estrlist.h"

UseEsdlNamespace()
//---------------------------------------------------------------------------
// TERange implementation
//---------------------------------------------------------------------------
//..............................................................................
void TERange::ListFromString(const olxstr &Range, TIntList &List)  {
  TStrList Toks(Range, ','), Toks1;
  olxstr S1;
  int pos, start, end;
  for( int i=0; i < Toks.Count(); i++ )  {
    const olxstr& S = Toks[i];
    pos = S.FirstIndexOf('-');
    if( pos >=0 )  {
      S1 = S.SubStringTo(pos);
      start = S1.ToInt();
      S1 = S.SubStringFrom(pos+1);
      end = S1.ToInt();
      for( int j=start; j < end; j++ )
        List.Add(j);
    }
    else
      List.Add( S.ToInt() );
  }
}
//..............................................................................
void TERange::StringFromList(olxstr &Range, const TIntList &List)  {
  for( int i=0; i < List.Count(); i++ )  {
    int j = 0;
    while( List[i] == (List[i+j]-j) )  {
      j++;
      if( (j + i) >= (List.Count()-1) )  {
        if( i != (j+i-1) )  {
          Range << List[i] << '-' << List[j+i];
          return;
        }
        else  {
          Range << List[i];
          return;
        }
      }
    }
    if( j == 0 )
      Range << List[i] << ',';
    else  {
      Range << List[i] <<  '-';
      i += (j - 1);
      Range << List[i] << ',';
    }
  }
}

