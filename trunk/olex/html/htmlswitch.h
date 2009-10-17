#ifndef __olx_html_switch_H
#define __olx_html_switch_H
#include "typelist.h"
#include "paramlist.h"

class THtml;
class THtmlSwitch {
protected:
  olxstr Name;
  short FileIndex;  // the file index
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

  inline short GetFileIndex() const {  return FileIndex; }
  void  SetFileIndex(short ind);
  void UpdateFileIndex();
  inline int FileCount() const {  return Files.Count(); }
  const olxstr &GetFile(int ind) const {  return Files[ind]; }
  void ClearFiles()  {  Files.Clear(); }
  void AddFile(const olxstr &fn)  {  Files.Add(fn);  }
  const olxstr& GetCurrentFile() const {  return FileIndex == -1 ? EmptyString : Files[FileIndex];  }

  TStrPObjList<olxstr,THtmlSwitch*>& GetStrings()  {  return Strings; }
  const TStrPObjList<olxstr,THtmlSwitch*>& GetStrings() const {  return Strings; }

  inline int SwitchCount() const {  return Switches.Count(); }
  inline THtmlSwitch& GetSwitch(int ind)  {  return Switches[ind]; }
  THtmlSwitch*  FindSwitch(const olxstr &IName);
  int FindSimilar(const olxstr &start, const olxstr &end, TPtrList<THtmlSwitch>& ret);
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
