/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "undo.h"

void TUndoStack::Clear()  {
  for( size_t i=0; i < UndoList.Count(); i++ )
    delete UndoList[i];
  UndoList.Clear();
}
//..............................................................................
TUndoData* TUndoStack::Pop()  {
  if( UndoList.Count() != 0 )  {
    TUndoData* retVal = UndoList[UndoList.Count()-1];
    UndoList.Delete( UndoList.Count()-1 );
    return retVal;
  }
  throw TFunctionFailedException(__OlxSourceInfo, "empty undo stack");
}
//..............................................................................
TUndoData::~TUndoData()  {
  for( size_t i=0; i < UndoList.Count(); i++ )
    delete UndoList[i];

  delete UndoAction;
}
