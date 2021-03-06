/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gxfiles.h"
#include "gxapp.h"

//..............................................................................
XObjectProvider::XObjectProvider(TGXApp &app)
: ASObjectProvider(
    *(new TXObjectProvider<TSAtom, TXAtom>(app.GetRenderer())),
    *(new TXObjectProvider<TSBond, TXBond>(app.GetRenderer())),
    *(new TXObjectProvider<TSPlane, TXPlane>(app.GetRenderer()))),
    app(app)
{}
//..............................................................................
//..............................................................................
//..............................................................................
TGXFile::TGXFile(XObjectProvider & op)
: TXFile(op)
{
  olxstr extra = (op.app.XFiles().Count() == 0 ? EmptyString() :
    olxstr(op.app.XFiles().Count()));
  DUnitCell = new TDUnitCell(op.app.GetRenderer(),
    olxstr("DUnitCell") << extra);
  DUnitCell->SetVisible(false);
  op.app.OnObjectsCreate.Add(this, GXFILE_EVT_OBJETSCREATE);
  OnFileLoad.Add(this, GXFILE_EVT_FILELOAD);
}
//..............................................................................
TGXFile::~TGXFile() {
  if (TGXApp::HasInstance()) {
    TGXApp::GetInstance().OnObjectsCreate.Remove(this);
  }
  OnFileLoad.Remove(this);
  delete DUnitCell;
}
//..............................................................................
bool TGXFile::Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
  const IOlxObject *Data, TActionQueue *q)
{
  if (MsgId == GXFILE_EVT_OBJETSCREATE) {
    if (MsgSubId == msiExecute) {
      DUnitCell->Create();
    }
  }
  if (MsgId == GXFILE_EVT_FILELOAD) {
    if (MsgSubId == msiExit) {
      DUnitCell->Init(GetAsymmUnit());
    }
  }
  return TXFile::Dispatch(MsgId, MsgSubId, Sender, Data, q);
}
//..............................................................................
void TGXFile::TakeOver(TXFile &f) {
  TXFile::TakeOver(f);
  OnFileLoad.Remove(&f);
  OnFileLoad.Add(this, GXFILE_EVT_FILELOAD);
}
//..............................................................................
void TGXFile::ToDataItem(TDataItem& item) {
  TXFile::ToDataItem(item);
  DUnitCell->ToDataItem(item.AddItem("DUC"));
}
//..............................................................................
void TGXFile::FromDataItem(const TDataItem& item) {
  TXFile::FromDataItem(item);
  DUnitCell->Init(GetAsymmUnit());
  TDataItem *uc = item.FindItem("DUC");
  if (uc != 0) {
    DUnitCell->FromDataItem(*uc);
  }
}
//..............................................................................

