/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "cdsfs.h"
#include "settingsfile.h"

//..............................................................................
bool TSocketFS::_OnReadFailed(const THttpFileSystem::ResponseInfo& info,
  uint64_t position)
{
  if (!info.headers.Find("Server", CEmptyString()).Equals("Olex2-CDS"))
    return false;
  while (--attempts >= 0) {
    try {
      TBasicApp::NewLogEntry(logInfo, true) << "Connection broken at position "
        << position << ", reconnecting to the server";
      DoConnect();  // reconnect
      const olxcstr rq = GenerateRequest("GET", info.source, position);
      return (size_t)_write(rq) == rq.Length();
    }
    catch(...) {
      olx_sleep(1000);
    }
  }
  return false;
}
//..............................................................................
bool TSocketFS::_DoValidate(const THttpFileSystem::ResponseInfo& info,
  TEFile& data) const
{
  bool valid = THttpFileSystem::_DoValidate(info, data);
  // make file pesistent and write file info
  if (BaseValid && info.headers.Find("Server",
    CEmptyString()).Equals("Olex2-CDS"))
  {
    data.SetTemporary(valid);
    olxstr ifn = olxstr(data.GetName()) << ".info";
    if (!valid) {
      olxstr fn = TEFile::OSPath(info.source);
      TSettingsFile sf;
      sf.SetParam("MD5", info.contentMD5);
      sf.SaveSettings(ifn);
    }
    else if (TEFile::Exists(ifn)) {
      TEFile::DelFile(ifn);
    }
  }
  return valid;
}
//..............................................................................
THttpFileSystem::AllocationInfo TSocketFS::_DoAllocateFile(const olxstr& src) {
  if (!BaseValid) {
    return THttpFileSystem::_DoAllocateFile(src);
  }
  try {
    const olxstr fn = olxstr(Base()) << MD5::Digest(TUtf8::Encode(src));
    const olxstr ifn = olxstr(fn) << ".info";
    if (TEFile::Exists(ifn) && GetIndex() != 0) {
      olxstr src_fn = TEFile::OSPath(src);
      TFSItem* fi;
      if (src_fn.StartsFrom(GetBase())) {
        fi = GetIndex()->GetRoot().FindByFullName(
          src_fn.SubStringFrom(GetBase().Length()));
      }
      else
        fi = GetIndex()->GetRoot().FindByFullName(src_fn);
      if (fi != 0) {
        const TSettingsFile sf(ifn);
        if (sf["MD5"] == fi->GetDigest()) {
          return AllocationInfo(new TEFile(fn, "a+b"), fi->GetDigest(), false);
        }
      }
    }
    return AllocationInfo(new TEFile(fn, "w+b"), CEmptyString(), true);
  }
  catch (...) {
    return AllocationInfo(0, CEmptyString(), true);
  }
}
//..............................................................................
THttpFileSystem::AllocationInfo& TSocketFS::_DoTruncateFile(AllocationInfo& file)
{
  if (!file.file.ok()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "file");
  }
  const olxstr fn = file.file->GetName();
  const olxstr ifn = olxstr(fn) << ".info";
  if (TEFile::Exists(ifn)) {
    TEFile::DelFile(ifn);
  }
  file.file->SetTemporary(true);
  file.truncated = true;
  file.digest.SetLength(0);
  file.file = new TEFile(fn, "w+b");
  return file;
}
//..............................................................................
