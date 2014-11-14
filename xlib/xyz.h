/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_xyz_H
#define __olx_xl_xyz_H
#include "xfiles.h"

BeginXlibNamespace()

class TXyz: public TBasicCFile  {
private:
  void Clear();
public:
  TXyz();
  virtual ~TXyz();
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile&);
  virtual IOlxObject* Replicate() const {  return new TXyz;  }
};

EndXlibNamespace()
#endif
