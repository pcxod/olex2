/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_socketfs_H
#define __olx_socketfs_H
#include "httpfs.h"
#include "md5.h"
#include "eutf8.h"

/* POSIX socket based file fetching utility
*/
class TSocketFS : public THttpFileSystem {
protected:
  static bool& UseLocalFS() {
    static bool use = false;
    return use;
  }
  static olxstr& Base() {
    static olxstr base;
    return base;
  }
  int attempts, max_attempts;
  bool BaseValid;
  virtual olx_object_ptr<IInputStream> _DoOpenFile(const olxstr& src) {
    attempts = max_attempts;
    return THttpFileSystem::_DoOpenFile(src);
  }
  virtual bool _OnReadFailed(const ResponseInfo& info, uint64_t position);
  // this will be useful when Olex2-CDS returns MD5 digest in ETag...
  virtual bool _DoValidate(const ResponseInfo& info, TEFile& data) const;
  virtual AllocationInfo _DoAllocateFile(const olxstr& src);
  virtual AllocationInfo& _DoTruncateFile(AllocationInfo& file);
public:
  TSocketFS(const TUrl& url, int _max_attempts = 100) :
    THttpFileSystem(url), max_attempts(_max_attempts), BaseValid(false)
  {
    if (Base().IsEmpty())
      Base() = TBasicApp::GetInstanceDir() + ".cds/";
    if (UseLocalFS()) {
      try {
        if (!TEFile::Exists(Base())) {
          if (TEFile::MakeDir(Base())) {
            BaseValid = true;
          }
        }
        else {
          BaseValid = true;
        }
      }
      catch (...) {}
    }
    if (max_attempts < 0) {
      max_attempts = 0;
    }
    else if (max_attempts > 32000) {
      max_attempts = 32000;
    }
  }
  virtual ~TSocketFS() {}
  static bool CanUseLocalFS() { return UseLocalFS(); }
  // allows creating temporary files in basedir/.cds/
  static void SetUseLocalFS(bool v) { UseLocalFS() = v; }
  /* sets the dir in which temporary files can be stores and enables
  the use of it as in a call to SetUseLocalFS(true). If the folder
  does not exists an attempt to create one will be made and in the case
  of an error false will be return, tru return otherwise */
  static bool InitLocalFSBase(const olxstr& base) {
    if (!TEFile::Exists(base)) {
      if (!TEFile::MakeDir(base)) {
        return false;
      }
    }
    Base() = base;
    SetUseLocalFS(true);
    return false;
  }
};

#endif
