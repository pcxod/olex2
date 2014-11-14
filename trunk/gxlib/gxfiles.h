/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxlib_gxfiles_H
#define __olx_gxlib_gxfiles_H
//#include "atomregistry.h"
#include "xfiles.h"
#include "xatom.h"
#include "xbond.h"
#include "xplane.h"
#include "dunitcell.h"

BeginGxlNamespace()

enum {
  GXFILE_EVT_OBJETSCREATE = XFILE_EVT_LAST,
  GXFILE_EVT_LAST
};

class TGXFile : public TXFile {
protected:
  virtual bool Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
    const IOlxObject *Data, TActionQueue *);
public:
  TGXFile(struct XObjectProvider &);
  ~TGXFile();

  TDUnitCell *DUnitCell;
};

template <class obj_t, class act_t> class TXObjectProvider
: public TObjectProvider<obj_t>
{
  TGlRenderer& renderer;
public:
  TXObjectProvider(TGlRenderer& _renderer) : renderer(_renderer)  {}
  virtual obj_t& New(TNetwork* n)  {
    return TObjectProvider<obj_t>::AddNew(
      new act_t(n, renderer, EmptyString()));
  }
};

struct XObjectProvider : public ASObjectProvider {
  class TGXApp &app;
  XObjectProvider(TGXApp &);
  ~XObjectProvider() {
    atoms.Clear();
    bonds.Clear();
    planes.Clear();
    delete &atoms;
    delete &bonds;
    delete &planes;
  }
  virtual TXFile *CreateXFile() {
    return new TGXFile(*this);
  }
  virtual IOlxObject* Replicate() const { return new XObjectProvider(app); }
};

EndGxlNamespace()
#endif
