/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_p4p_H
#define __olx_xl_p4p_H
#include "xfiles.h"

BeginXlibNamespace()

class TP4PFile : public TBasicCFile  {
  olxstr
    Color,
    SiteId,
    Morph,
    Mosaic,
    Symm,
    Bravais,
    SGString,
    FileId; // FILEID field
public:
  TP4PFile()  {}
  ~TP4PFile()  {}
  void Clear();
  const olxstr& GetColor() const {  return Color;  }
  const olxstr& GetSiteId() const {  return SiteId;  }
  const olxstr& GetMorph() const {  return Morph;  }
  const olxstr& GetFileId() const {  return FileId;  }
  // this is set when the file is manually exported ...
  DefPropC(olxstr, SGString)
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile&);
  virtual IEObject* Replicate() const {  return new TP4PFile();  }
};

EndXlibNamespace()
#endif
