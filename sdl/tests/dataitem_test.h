/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../dataitem.h"
namespace test {
  void DataItemTests(OlxTests& t) {
    t.description = __FUNC__;
    /* DeleteByName used to pass a field index to DeleteItemByIndex, which
    throws on a leaf item and removes an unrelated item otherwise
    */
    {
      TDataItem di(0, "root");
      di.SetFieldValue("scope.a", "1");
      di.SetFieldValue("scope.b", "2");
      di.SetFieldValue("scope.c", "3");
      if (!di.DeleteByName("scope.b")) {
        throw TFunctionFailedException(__OlxSourceInfo, "delete reported failure");
      }
      TDataItem* s = di.FindItem("scope");
      if (s == 0 || s->FieldCount() != 2) {
        throw TFunctionFailedException(__OlxSourceInfo, "field was not deleted");
      }
      if (s->FindField("a") != "1" || s->FindField("c") != "3") {
        throw TFunctionFailedException(__OlxSourceInfo, "wrong field deleted");
      }
      if (!s->FindField("b").IsEmpty()) {
        throw TFunctionFailedException(__OlxSourceInfo, "field still present");
      }
    }
    // a field must not be deleted in place of a sibling item of the same index
    {
      TDataItem di(0, "root");
      TDataItem& s = di.AddItem("scope");
      s.AddField("f", "v");
      s.AddItem("child");
      if (!di.DeleteByName("scope.f")) {
        throw TFunctionFailedException(__OlxSourceInfo, "delete reported failure");
      }
      if (s.ItemCount() != 1 || s.FindItem("child") == 0) {
        throw TFunctionFailedException(__OlxSourceInfo, "item was removed");
      }
      if (s.FieldCount() != 0) {
        throw TFunctionFailedException(__OlxSourceInfo, "field was not deleted");
      }
    }
    // items are still deleted by name
    {
      TDataItem di(0, "root");
      di.AddItem("a");
      di.AddItem("b");
      if (!di.DeleteByName("a") || di.ItemCount() != 1 || di.FindItem("b") == 0) {
        throw TFunctionFailedException(__OlxSourceInfo, "item delete broken");
      }
    }
  }
};  //namespace test
