/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_crs_H
#define __olx_xl_crs_H

#include "xfiles.h"

// STOE "P4P" file
BeginXlibNamespace()

class TCRSFile : public TBasicCFile  {
  evecd_list Faces;
  bool SGInitialised;
public:
  TCRSFile();
  ~TCRSFile()  {}
  void Clear();
  class TSpaceGroup* GetSG();

  inline size_t FacesCount() const {  return Faces.Count();  }
  inline const evecd& GetFace(size_t i) const {  return Faces[i];  }

  bool HasSG() const {  return SGInitialised;  }

  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile&);
  virtual IOlxObject* Replicate() const {  return new TCRSFile();  }
};

EndXlibNamespace()
#endif
