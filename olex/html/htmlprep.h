/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#pragma once
#include "edict.h"
#include "paramlist.h"
class THtmlSwitch;

class THtmlPreprocessor {
public:
  static const size_t UnknownSwitchState = (size_t)(-2);

  THtmlPreprocessor(const olxstr& popup_name = EmptyString())
    : PopupName(popup_name)
  {}

  const olxstr& GetPopupName() const { return PopupName; }
  void CheckForSwitches(THtmlSwitch& Sender, bool IsZip);
  size_t GetSwitchState(const olxstr& switchName) {
    return SwitchStates.Find(switchName, UnknownSwitchState);
  }
  void SetSwitchState(THtmlSwitch& sw, size_t state);
  void UpdateSwitchState(THtmlSwitch& Switch, olxstr& String);

  olxstr Preprocess(const olxstr& html, const TParamList& params);
  olxstr Preprocess(const olxstr& html) {
    return Preprocess(html, TParamList());
  }


protected:
  void ClearSwitchStates() { SwitchStates.Clear(); }
protected:
  olxstr PopupName;
  olxstr_dict<size_t, true> SwitchStates;
};