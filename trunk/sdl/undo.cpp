/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "undo.h"

void TUndoStack::Clear() {
  UndoList.DeleteItems(false).Clear();
}
//..............................................................................
TUndoData* TUndoStack::Pop() {
  if (UndoList.IsEmpty()) {
    TUndoData* retVal = UndoList.GetLast();
    UndoList.Delete(UndoList.Count()-1);
    return retVal;
  }
  throw TFunctionFailedException(__OlxSourceInfo, "empty undo stack");
}
//..............................................................................
TUndoData::~TUndoData() {
  UndoList.DeleteItems(false);
  delete UndoAction;
}
