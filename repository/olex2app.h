/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx2_app_H
#define __olx2_app_H
#include "bapp.h"
#include "efile.h"
#include "bapp.h"
#include "datafile.h"
#include "dataitem.h"

#include "../olex/langdict.h"

namespace olex2 {

class AOlex2App : public virtual IOlxObject {
protected:
  static AOlex2App *& GetInstance_() {
    static AOlex2App *instance = 0;
    return instance;
  }
  SortedObjectList<olxstr, olxstrComparator<true> > plugins;
  olxstr PluginFileName;
public:
  AOlex2App() {
    if (GetInstance_() != 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "application has already be initialised");
    }
    GetInstance_() = this;
  }

  virtual ~AOlex2App() {
    GetInstance_() = 0;
  }

  static bool HasInstance() {
    return GetInstance_() != 0;
  }

  static AOlex2App &GetInstance() {
    if (GetInstance_() == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "application instance has not been initialised");
    }
    return *GetInstance_();
  }

  // throws an exception
  void SetCurrentLanguage(const olxstr &l_) {
    olxstr df = TBasicApp::GetInstance().GetBaseDir() + "dictionary.txt";
    olxstr l = l_.IsEmpty() ? olxstr("English") : l_;
    Dictionary.SetCurrentLanguage(df, l);
    if (Dictionary.GetCurrentLanguage().IsEmpty() && !l_.IsEmpty()) {
      Dictionary.SetCurrentLanguage(df, "English");
    }
  }
  
  virtual void InitOlex2App() {
    PluginFileName = TBasicApp::GetInstance().GetBaseDir() + "plugins.xld";
    if (TEFile::Exists(PluginFileName)) {
      TDataFile df;
      df.LoadFromXLFile(PluginFileName, 0);
      TDataItem *pi = df.Root().FindItem("Plugin");
      if (pi == 0) {
        TBasicApp::NewLogEntry(logError) << "Invalid plugins file";
      }
      else {
        plugins.SetCapacity(pi->ItemCount());
        for (size_t i = 0; i < pi->ItemCount(); i++) {
          plugins.Add(pi->GetItemByIndex(i).GetName());
        }
      }
    }
  }

  virtual bool AddPlugin(const olxstr &pn) {
    if (!plugins.AddUnique(pn).b) {
      return false;
    }
    TDataFile df;
    if (TEFile::Exists(PluginFileName)) {
      df.LoadFromXLFile(PluginFileName, 0);
    }
    TDataItem *pi = df.Root().FindItem("Plugin");
    if (pi == 0) {
      pi = &df.Root().AddItem("Plugin");
    }
    pi->AddItem(pn);
    df.SaveToXLFile(PluginFileName);
    return true;
  }

  virtual bool RemovePlugin(const olxstr &pn) {
    if (!plugins.Remove(pn)) {
      return false;
    }
    TDataFile df;
    if (!TEFile::Exists(PluginFileName)) {
      return false;
    }
    df.LoadFromXLFile(PluginFileName, NULL);
    TDataItem *pir = df.Root().FindItem("Plugin");
    if (pir == 0) {
      return false;
    }
    TDataItem *pi = pir->FindItemi(pn);
    if (pi == 0) {
      return false;
    }
    pir->DeleteItem(pi);
    df.SaveToXLFile(PluginFileName);
    return true;
  }

  bool AddPluginField(const olxstr &pn,
    const olxstr &field, const olxstr &value)
  {
    if (!plugins.Contains(pn)) return false;
    TDataFile df;
    if (TEFile::Exists(PluginFileName))
      df.LoadFromXLFile(PluginFileName, NULL);
    TDataItem *pi = df.Root().FindItem("Plugin");
    if (pi == 0) {
      return false;
    }
    pi = pi->FindItemi(pn);
    if (pi == 0) {
      return false;
    }
    pi->AddField(field, value);
    df.SaveToXLFile(PluginFileName);
    return true;
  }
  const olxstr& GetCurrentLanguageEncodingStr() const {
    return Dictionary.GetCurrentLanguageEncodingStr();
  }
  TStrList GetPluginList() const {
    return TStrList::FromAny(plugins);
  }
  bool IsPluginInstalled(const olxstr &pn) const {
    return plugins.Contains(pn);
  }
  olxstr TranslatePhrase(const olxstr &p) const {
    return Dictionary.Translate(p);
  }
  olxstr TranslateString(const olxstr& str) const {
    olxstr phrase = str;
    size_t ind = phrase.FirstIndexOf('%');
    while (ind != InvalidIndex) {
      if (ind + 1 >= phrase.Length())  return phrase;
      // analyse the %..
      size_t pi = ind;
      while (--pi != InvalidIndex &&
        (olxstr::o_isdigit(phrase.CharAt(pi)) || phrase.CharAt(pi) == '.'))
      {
      }
      if (pi != ind - 1 && pi != InvalidIndex) {
        if (phrase.CharAt(pi) == '\'' || phrase.CharAt(pi) == '\"') {
          if (ind < phrase.Length() && phrase.CharAt(pi) == phrase.CharAt(ind + 1)) {
            ind = phrase.FirstIndexOf('%', ind + 1);
            continue;
          }
        }
        if (phrase.CharAt(pi) == '=') {
          ind = phrase.FirstIndexOf('%', ind + 1);
          continue;
        }
      }
      size_t ind1 = phrase.FirstIndexOf('%', ind + 1);
      if (ind1 == InvalidIndex)  return phrase;
      if (ind1 == ind + 1) { // %%
        phrase.Delete(ind1, 1);
        ind = phrase.FirstIndexOf('%', ind1);
        continue;
      }
      olxstr tp = Dictionary.Translate(phrase.SubString(ind + 1, ind1 - ind - 1));
      phrase.Delete(ind, ind1 - ind + 1);
      phrase.Insert(tp, ind);
      ind1 = ind + tp.Length();
      if (ind1 + 1 >= phrase.Length()) {
        return phrase;
      }
      ind = phrase.FirstIndexOf('%', ind1 + 1);
    }
    return phrase;
  }

  TLangDict Dictionary;
};

} // end namesapce olex
#endif
