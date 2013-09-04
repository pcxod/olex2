/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_html_switch_H
#define __olx_html_switch_H
#include "typelist.h"
#include "paramlist.h"

class THtml;
class THtmlSwitch {
protected:
  olxstr Name;
  size_t FileIndex;  // the file index
  TStrList  Files;
  TStrPObjList<olxstr,THtmlSwitch*> Strings;  // represents current content of the switch
  TParamList Params;   // parameters to be replaced with their values param=ll use #param
  TTypeList<THtmlSwitch> Switches; // a list of subitems
  THtmlSwitch *ParentSwitch;
protected:
  bool UpdateSwitch;
  THtml* ParentHtml;
public:
  THtmlSwitch(THtml *_parent_html, THtmlSwitch *_parent_switch) : 
      ParentHtml(_parent_html), 
        ParentSwitch(_parent_switch),
        UpdateSwitch(true),
        FileIndex(0) {}
  virtual ~THtmlSwitch() {  Clear();  }
  void Clear();

  DefPropC(olxstr, Name)

  inline size_t GetFileIndex() const {  return FileIndex; }
  void  SetFileIndex(size_t ind);
  void UpdateFileIndex();
  inline size_t FileCount() const {  return Files.Count(); }
  const olxstr &GetFile(size_t ind) const {  return Files[ind]; }
  void ClearFiles()  {  Files.Clear(); }
  void AddFile(const olxstr &fn)  {  Files.Add(fn);  }
  const olxstr& GetCurrentFile() const {
    return olx_is_valid_index(FileIndex) ? Files[FileIndex] : EmptyString();
  }

  TStrPObjList<olxstr,THtmlSwitch*>& GetStrings()  {  return Strings; }
  const TStrPObjList<olxstr,THtmlSwitch*>& GetStrings() const {  return Strings; }

  inline size_t SwitchCount() const {  return Switches.Count(); }
  inline THtmlSwitch& GetSwitch(size_t ind)  {  return Switches[ind]; }
  THtmlSwitch*  FindSwitch(const olxstr &IName);
  size_t FindSimilar(const olxstr &start, const olxstr &end, TPtrList<THtmlSwitch>& ret);
  void Expand(TPtrList<THtmlSwitch>& ret);
  THtmlSwitch& NewSwitch();

  void AddParam(const olxstr &name, const olxstr &value){  Params.AddParam(name, value);  };
  void AddParam(const olxstr &nameEqVal)  {  Params.FromString(nameEqVal, '=');  };
  TParamList& GetParams()  {  return Params;  }
  const TParamList& GetParams() const {  return Params;  }

  bool IsToUpdateSwitch() const {  return UpdateSwitch; };
  void SetUpdateSwitch(bool v)  {  UpdateSwitch = v; };

  void ToStrings(TStrList &List);
  bool ToFile();
};
#endif
