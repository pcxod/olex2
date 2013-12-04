/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_datafile_H
#define __olx_sdl_datafile_H
#include "ebase.h"
#include "estrlist.h"
BeginEsdlNamespace()

class TDataFile: public IEObject  {
protected:
  class TDataItem *FRoot;
  olxstr FileName;
public:
  TDataFile();
  virtual ~TDataFile();
  virtual bool LoadFromTextStream(IInputStream& io, TStrList* Log=NULL);
  virtual bool LoadFromXMLTextStream(IInputStream& io, TStrList* Log = NULL);
  virtual bool LoadFromXLFile(const olxstr& DataFile, TStrList* Log = NULL);
  virtual bool LoadFromXMLFile(const olxstr& DataFile, TStrList* Log = NULL);
  virtual void SaveToXLFile(const olxstr& DataFile);
  virtual void SaveToXMLFile(const olxstr& DataFile);
  virtual void Include(TStrList* Log);
  inline TDataItem& Root() { return *FRoot; }
  const olxstr& GetFileName() const { return FileName; }
};

EndEsdlNamespace()
#endif
