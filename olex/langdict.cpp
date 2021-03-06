/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "langdict.h"
#include "utf8file.h"
#include "egc.h"
#include "bapp.h"
#include "log.h"
#include "etbuffer.h"
#ifdef __WXWIDGETS__
#include "wx/wx.h"
#endif

TLangDict::TLangDict() {
  CurrentLanguageEncodingStr = "ISO8859-1";
}
//..............................................................................
TLangDict::~TLangDict() {
  Clear();
}
//..............................................................................
void TLangDict::Clear() {
  for (size_t i = 0; i < Records.Count(); i++) {
    delete Records.GetValue(i);
  }
  Records.Clear();
}
//..............................................................................
const olxstr& TLangDict::Translate(const olxstr& Phrase) const {
  if (CurrentLanguageIndex == 0) {
    return Phrase;
  }
  size_t ind = Records.IndexOf(Phrase);
  return (ind == InvalidIndex ? Phrase : *Records.GetValue(ind));
}
//..............................................................................
void TLangDict::SetCurrentLanguage(const olxstr& fileName, const olxstr& lang) {
  Clear();
  olx_object_ptr<TUtf8File> f = TUtf8File::Open(fileName, "rb", true);
  TCStrList sl;
  sl.LoadFromTextStream(f);
  if (sl.Count() < 2) {
    return;
  }

  TCStrList toks(sl[0], '\t');  // languages
  if (toks.Count() < 2) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "At least one ID column and one language are expected");
  }

  CurrentLanguageIndex = (lang.IsEmpty() ? 1 : toks.IndexOf(lang));
  if (CurrentLanguageIndex == 0 || CurrentLanguageIndex == InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("Invalid language ").quote() << lang);
  }
  TCStrList toks1(sl[1], '\t');  // encodings
  if (toks.Count() != toks1.Count()) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "Number of languages mismatches the number of encodings");
  }
  CurrentLanguage = toks[CurrentLanguageIndex];
  CurrentLanguageEncodingStr = toks1[CurrentLanguageIndex];

#ifdef __WXWIDGETS__
  wxMBConvUTF8 utf8;
  TTBuffer<wchar_t> wc_bf(4096);
#endif
  size_t cc = toks.Count();
  Records.SetCapacity(sl.Count());
  for (size_t i = 2; i < sl.Count(); i++) {
    if (sl[i].IsEmpty()) {
      continue;
    }
    toks.Clear();
    toks.Strtok(sl[i], '\t');
    if (toks.Count() != cc) {
      TBasicApp::NewLogEntry() << "Error reading dictionary at line " << i;
      continue;
    }
    olx_object_ptr<olxstr> str;
    if (toks[CurrentLanguageIndex].Length() < 2) {  // take language 1
#ifdef __WXWIDGETS__
      wc_bf.SetCapacity(toks[1].Length());
      int c = utf8.MB2WC(wc_bf.Data(), toks[1].c_str(), wc_bf.GetCapacity());
      str = new olxstr((const olxch*)wc_bf.Data(), c);
#else
      try {
        str = new olxstr(olxwstr::FromCStr(toks[1].c_str()));
      }
      catch (...) {
        continue;
      }
#endif
    }
    else {
#ifdef __WXWIDGETS__
      wc_bf.SetCapacity(toks[CurrentLanguageIndex].Length());
      size_t c = utf8.MB2WC(wc_bf.Data(), toks[CurrentLanguageIndex].c_str(),
        wc_bf.GetCapacity());
      str = new olxstr((const olxch *)wc_bf.Data(), c);
#else
      try {
        str = new olxstr(olxwstr::FromCStr(toks[CurrentLanguageIndex].c_str()));
      }
      catch (...) {
        continue;
      }
#endif
    }
    if (str.ok()) {
      size_t idx = Records.IndexOf(*str);
      if (idx == InvalidIndex) {
        Records.Add(toks[0], str.release());
      }
      else {
        *Records.GetValue(idx) = *str;
      }
    }
  }
}
//..............................................................................
