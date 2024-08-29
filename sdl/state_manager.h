/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
BeginEsdlNamespace()

// a simple scalar state manager
template <typename T>
struct StateManager {
  T& value;
  T start, end;
  StateManager(T& value, const T& start_value, const T& end_value)
    : value(value), start(start_value), end(end_value)
  {
    value = start;
  }
  ~StateManager() {
    value = end;
  }
};
EndEsdlNamespace()
