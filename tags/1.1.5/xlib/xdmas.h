/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_xd_mas_file_H
#define __olx_xl_xd_mas_file_H
#include "xbase.h"
#include "estrlist.h"
#include "xfiles.h"
#include "catom.h"

BeginXlibNamespace()

class TXDMas: public TBasicCFile  {
public:
  TXDMas()  {  }
  virtual void SaveToStrings(TStrList& Strings)  {  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile&)  {  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual IEObject* Replicate() const {  return new TXDMas;  }
};

EndXlibNamespace()
#endif
