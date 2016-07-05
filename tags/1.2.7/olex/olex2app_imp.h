/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx2_app_imp_H
#define __olx2_app_imp_H
#include "olex2app.h"
#include "langdict.h"
#include "olxstate.h"

class Olex2App : public olex2::AOlex2App {
  size_t statePluginInstalled;
  SortedObjectList<olxstr, olxstrComparator<true> > plugins;
  bool CheckState(size_t state, const olxstr& stateData) const {
    return state == statePluginInstalled ? plugins.Contains(stateData) : false;
  }
  olxstr PluginFileName;
public:
  TLangDict Dictionary;

  Olex2App(const olxstr& FileName, AGlScene *scene=NULL)
    : olex2::AOlex2App(FileName, scene)
  {
    TStateRegistry &states = GetStatesRegistry();
    statePluginInstalled = TStateRegistry::GetInstance().Register(
      "pluginInstalled",
      new TStateRegistry::Slot(
        states.NewGetter<Olex2App>(this, &Olex2App::CheckState),
        new TStateRegistry::TMacroSetter("HtmlPanelVisible")
      )
    );
  }
  // throws an exception
  void SetCurrentLanguage(const olxstr &l_) {
    olxstr df=GetBaseDir() + "dictionary.txt";
    olxstr l = l_.IsEmpty() ? olxstr("English") : l_;
    Dictionary.SetCurrentLanguage(df, l);
    if (Dictionary.GetCurrentLanguage().IsEmpty() && !l_.IsEmpty())  {
      Dictionary.SetCurrentLanguage(df, "English");
    }
  }
  void InitOlex2App() {
    PluginFileName=GetBaseDir() + "plugins.xld";
    if (TEFile::Exists(PluginFileName)) {
      TDataFile df;
      df.LoadFromXLFile(PluginFileName, NULL);
      TDataItem *pi=df.Root().FindItem("Plugin");
      if (pi == NULL) {
        NewLogEntry(logError) << "Invalid plugins file";
      }
      else {
        plugins.SetCapacity(pi->ItemCount());
        for (size_t i=0; i < pi->ItemCount(); i++) {
          TStateRegistry::GetInstance().SetState(
            statePluginInstalled, true, pi->GetItemByIndex(i).GetName(), true);
          plugins.Add(pi->GetItemByIndex(i).GetName());
        }
      }
    }
  }
  bool AddPlugin(const olxstr &pn) {
    if (!plugins.AddUnique(pn).b) return false;
    TDataFile df;
    if (TEFile::Exists(PluginFileName)) {
      df.LoadFromXLFile(PluginFileName, NULL);
    }
    TDataItem *pi=df.Root().FindItem("Plugin");
    if (pi == NULL) {
      pi = &df.Root().AddItem("Plugin");
    }
    pi->AddItem(pn);
    df.SaveToXLFile(PluginFileName);
    TStateRegistry::GetInstance().SetState(statePluginInstalled, true,
        pn, true);
    return true;
  }
  bool RemovePlugin(const olxstr &pn) {
    if (!plugins.Remove(pn)) return false;
    TDataFile df;
    if (!TEFile::Exists(PluginFileName)) return false;
    df.LoadFromXLFile(PluginFileName, NULL);
    TDataItem *pir=df.Root().FindItem("Plugin");
    if (pir == NULL) return false;
    TDataItem *pi=pir->FindItemi(pn);
    if (pi == NULL) return false;
    pir->DeleteItem(pi);
    df.SaveToXLFile(PluginFileName);
    TStateRegistry::GetInstance().SetState(statePluginInstalled, false,
        pn, true);
    return true;
  }
  bool AddPluginField(const olxstr &pn,
    const olxstr &field, const olxstr &value)
  {
    if (!plugins.Contains(pn)) return false;
    TDataFile df;
    if (TEFile::Exists(PluginFileName))
      df.LoadFromXLFile(PluginFileName, NULL);
    TDataItem *pi=df.Root().FindItem("Plugin");
    if (pi == NULL) return false;
    pi = pi->FindItemi(pn);
    if (pi == NULL) return false;
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
    olxstr phrase=str;
    size_t ind = phrase.FirstIndexOf('%');
    while( ind != InvalidIndex )  {
      if( ind+1 >= phrase.Length() )  return phrase;
      // analyse the %..
      size_t pi = ind;
      while( --pi != InvalidIndex &&
        (olxstr::o_isdigit(phrase.CharAt(pi)) || phrase.CharAt(pi) == '.') )
        ;
      if( pi != ind-1 && pi != InvalidIndex )  {
        if( phrase.CharAt(pi) == '\'' || phrase.CharAt(pi) == '\"' )  {
          if( ind < phrase.Length() && phrase.CharAt(pi) == phrase.CharAt(ind+1) )  {
            ind = phrase.FirstIndexOf('%', ind+1);
            continue;
          }
        }
        if( phrase.CharAt(pi) == '=' )  {
          ind = phrase.FirstIndexOf('%', ind+1);
          continue;
        }
      }
      size_t ind1 = phrase.FirstIndexOf('%', ind+1);
      if( ind1 == InvalidIndex )  return phrase;
      if( ind1 == ind+1 )  { // %%
        phrase.Delete(ind1, 1);
        ind = phrase.FirstIndexOf('%', ind1);
        continue;
      }
      olxstr tp = Dictionary.Translate( phrase.SubString(ind+1, ind1-ind-1));
      phrase.Delete(ind, ind1-ind+1);
      phrase.Insert(tp, ind);
      ind1 = ind + tp.Length();
      if( ind1+1 >= phrase.Length() )  return phrase;
      ind = phrase.FirstIndexOf('%', ind1+1);
    }
    return phrase;
  }

  static Olex2App &GetInstance() {
    TGXApp& app = TGXApp::GetInstance();
    Olex2App* olx2app = dynamic_cast<Olex2App*>(&app);
    if (olx2app == NULL) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "unsuitable application instance");
    }
    return *olx2app;
  }
};

#endif
