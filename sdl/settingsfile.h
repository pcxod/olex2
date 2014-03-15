/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_settingsfile_H
#define __olx_sdl_settingsfile_H
#include "edict.h"
#include "estrlist.h"
BeginEsdlNamespace()

/* reads settings file, considering lines having no assignment operator or
lines starting from '#' as comments. The white spaces will be trimmed for
parameter names and values. The file structure will be peserved upon saving
(besides white spaces for param name/value). If there are duplicate entries,
the most recent value will be kept
*/
class TSettingsFile: public IEObject  {
  olxstr_dict<olxstr, true> Params;
  TStringToList<olxstr, bool> Lines;
protected:
  void Clear()  {
    Params.Clear();
    Lines.Clear();
  }
public:
  TSettingsFile(){}
  TSettingsFile(const olxstr& fileName)  {  LoadSettings(fileName);  }
  void LoadSettings(const olxstr& fileName);
  void SaveSettings(const olxstr& fileName);
  virtual ~TSettingsFile() {}
  // iteration functions
  inline size_t ParamCount() const {  return Params.Count();  }
  const olxstr& ParamName(size_t i) const {  return Params.GetKey(i);  }
  const olxstr& ParamValue(size_t i) const {  return Params.GetValue(i);  }
  // 
  template <class SC>
  inline const olxstr& GetParam(const SC& paramName,
    const olxstr& defVal=EmptyString()) const
  {
    return Params.Find(paramName, defVal);
  }
  // convinience operator, same as GetParam
  template <class SC>
  inline const olxstr& operator [] (const SC& pn) const {
    return GetParam(pn);
  }
  // creates a param if does not exist
  template <class SC>
  inline olxstr& operator [] (const SC& paramName) {
    size_t index = Params.IndexOf(paramName);
    if( index != InvalidIndex )
      return Params.GetValue(index);
    else  {
      Lines.Add(paramName, true);
      return Params.Add(paramName, EmptyString());
    }
  }
  
  template <class SC>
  inline bool HasParam(const SC& paramName) const {
    return Params.HasKey(paramName);
  }
  // updates or creates new parameter
  template <class SC>
  void SetParam(const SC& paramName, const olxstr& val)  {
    size_t index = Params.IndexOf(paramName);
    if( index != InvalidIndex )
      Params.GetValue(index) = val;
    else  {
      Lines.Add(paramName, true);
      Params.Add(paramName, val);
    }
  }
};

EndEsdlNamespace()
#endif
