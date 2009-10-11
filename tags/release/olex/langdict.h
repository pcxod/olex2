#ifndef langdictH
#define langdictH

#include "estlist.h"
#include "estrlist.h"
#include "wx/wx.h"

class TLangDict  {
  TSStrPObjList<olxstr,olxstr*, true> Records;
  olxstr CurrentLanguage, CurrentLanguageEncodingStr;
  int CurrentLanguageIndex;
protected:
  void Clear();
public:
  TLangDict();
  virtual ~TLangDict();

  const olxstr& Translate( const olxstr& Phrase) const;
  const olxstr& GetCurrentLanguageEncodingStr() const {  return CurrentLanguageEncodingStr;  }
  void SetCurrentLanguage(const olxstr& fileName, const olxstr& lang);
  const olxstr& GetCurrentLanguage()  const {  return CurrentLanguage;  }

};

#endif


