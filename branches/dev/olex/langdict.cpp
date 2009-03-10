#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "langdict.h"
#include "efile.h"
#include "egc.h"
#include "bapp.h"
#include "log.h"

#include "etbuffer.h"

#ifndef uiStr  // ansi string to wxString in unicode
  #define uiStr(v)  (wxString((v).u_str()))
  #define uiStrT(v) (wxString(v, *wxConvUI))
#endif

//..............................................................................
TLangDict::TLangDict()  {
  CurrentLanguageEncodingStr = "ISO8859-1";
}
//..............................................................................
TLangDict::~TLangDict()  {  Clear();  }
//..............................................................................
void TLangDict::Clear()  {
  for( int i=0; i < Records.Count(); i++ )
    delete Records.Object(i);
}
//..............................................................................
const olxstr& TLangDict::Translate(const olxstr& Phrase) const  {
  if( CurrentLanguageIndex == 0 )  return Phrase;
  int ind = Records.IndexOf( Phrase );
  if( ind == -1 )  return Phrase;
//  olxstr debug_str = *Records.GetObject(ind);
//  int l = debug_str.Length();
  return *Records.GetObject(ind);
}
//..............................................................................
void TLangDict::SetCurrentLanguage(const olxstr& fileName, const olxstr& lang)  {
  TEFile f( fileName, "rb" );
  int Utf8Encoding = 0;
  f.Read( &Utf8Encoding, 3);
  if( Utf8Encoding != 0x00BFBBEF )
    throw TFunctionFailedException(__OlxSourceInfo, "UTF8 dictionary is expected");

  TCStrList sl, toks, toks1;
  sl.LoadFromTextStream( f );
  if( sl.Count() < 2 )  return;

  toks.Strtok( sl[0], '\t');  // languages
  if( toks.Count() < 2 )
    throw TFunctionFailedException(__OlxSourceInfo, "At least one ID column and one language are expected");

  CurrentLanguageIndex = (lang.IsEmpty() ? 1 : toks.IndexOf(lang) );
  if(  CurrentLanguageIndex <= 0 )
    throw TInvalidArgumentException(__OlxSourceInfo,  olxstr("Invalid language '") << lang << '\'');
  toks1.Strtok( sl[1], '\t');  // encodings
  if( toks.Count() != toks1.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "Number of languages mismatches the number of encodings");

#ifndef _UNICODE
  wxCSConv csc( toks1[CurrentLanguageIndex].u_str() );
  if( !csc.IsOk() )  {
    throw TFunctionFailedException(__OlxSourceInfo, (olxstr("Could not locate the following encoding ") <<
      toks1[CurrentLanguageIndex] << '\n'));
  }
  else  {
    CurrentLanguage = toks[CurrentLanguageIndex];
    CurrentLanguageEncodingStr = toks1[CurrentLanguageIndex];
  }
  TTBuffer<char>    c_bf(1024);
#else
  CurrentLanguage = toks[CurrentLanguageIndex];
  CurrentLanguageEncodingStr = toks1[CurrentLanguageIndex];
#endif

  TTBuffer<wchar_t> wc_bf(4096);
  wxMBConvUTF8 utf8;

  int cc = toks.Count();

  Records.SetCapacity( sl.Count() );

  for( int i=2; i < sl.Count(); i++ )  {
    if( sl[i].IsEmpty() )  continue;
    toks.Clear();
    toks.Strtok( sl[i], '\t');
    if( toks.Count() != cc )  {
      TBasicApp::GetLog() << (olxstr("Error reading dictionary at line ") << i << '\n');
      continue;
    }
#ifdef _UNICODE
    wc_bf.SetCapacity(toks[CurrentLanguageIndex].Length());
    int c = utf8.MB2WC( wc_bf.Data(), toks[CurrentLanguageIndex].c_str(), wc_bf.GetCapacity());
     Records.Add( toks[0], new olxstr((const olxch *)wc_bf.Data(), c) );
#else
    wc_bf.SetCapacity(toks[CurrentLanguageIndex].Length());
    c_bf.SetCapacity(toks[CurrentLanguageIndex].Length());
    int c = utf8.MB2WC( wc_bf.Data(), toks[CurrentLanguageIndex].c_str(), wc_bf.GetCapacity());
    csc.FromWChar(c_bf.Data(), c_bf.GetCapacity(), wc_bf.Data(), c);
    Records.Add( toks[0], new olxstr(c_bf.Data()) );
#endif
  }
}
//..............................................................................

