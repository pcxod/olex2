/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_fsext_H
#define __olx_fsext_H

#include "efile.h"
#include "library.h"

#ifdef __WXWIDGETS__
  #include <wx/wx.h>
  #include <wx/filesys.h>
#endif

class TZipWrapper;
struct TMemoryBlock {
  char *Buffer;
  uint32_t Length;
  uint64_t DateTime;
  short PersistenceId; // dynamic property - not saved to a file
  TMemoryBlock() {
    Buffer = NULL;
    Length = 0;
    DateTime = 0;
    PersistenceId = 0;  // not persistent
  }
};
/*____________________________________________________________________________*/
class TFileHandlerManager : public IOlxObject {
#ifdef __WXWIDGETS__
  olxstr_dict<TZipWrapper*, false> FZipFiles;
#endif
  olxstr_dict<TMemoryBlock*, false> FMemoryBlocks;
  static const int16_t Version() { return 0x0001; }
  static const char *Signature() { return "ODF_"; }
  static size_t SignatureLength() { return 4; }
protected:
  TMemoryBlock *GetMemoryBlock(const olxstr &FN);
public:
  TFileHandlerManager();
  ~TFileHandlerManager();
protected:
  static TFileHandlerManager *&Handler();
  static TStrList &BaseDirs() {
    static TStrList l;
    return l;
  }

  void _Clear();
  olx_object_ptr<IDataInputStream> _GetInputStream(const olxstr &FN);
#ifdef __WXWIDGETS__
  olx_object_ptr<wxFSFile> _GetFSFileHandler( const olxstr &FN );
#endif
  TMemoryBlock *_AddMemoryBlock(const olxstr& name, const char *bf,
    size_t length, short persistenceId);
  static olxstr LocateFile(const olxstr& fn);
  void _SaveToStream(IDataOutputStream& os, short persistenceMask);
  void _LoadFromStream(IDataInputStream& is, short persistenceId);
  inline bool IsMemoryBlock(const olxstr &EM) const {
    return FMemoryBlocks.HasKey(TEFile::UnixPath(EM));
  }
public:
  static olx_object_ptr<IDataInputStream> GetInputStream(const olxstr &FN);
#ifdef __WXWIDGETS__
  static olx_object_ptr<wxFSFile> GetFSFileHandler(const olxstr &FN);
#endif
  static void Clear(short persistenceMask = ~0);
  static void AddBaseDir(const olxstr& bd);
  static void AddMemoryBlock(const olxstr& name, const char *bf, size_t length,
    short persistenceId);
  /* adds or replaces existing memory block and allocates an uninitialised
  storage for it
  */
  static TMemoryBlock& AddMemoryBlock(const olxstr& name, size_t length,
    short persistenceId);

  static void SaveToStream(IDataOutputStream& os, short persistenceMask);
  static void LoadFromStream(IDataInputStream& is, short persistenceId);

  static const TMemoryBlock* FindMemoryBlock(const olxstr& bn);
  static size_t Count();
  static const olxstr& GetBlockName(size_t i);
  static size_t GetBlockSize(size_t i);
  static olxstr GetBlockDateTime(size_t i);
  static short GetPersistenceId(size_t i);
  static bool Exists(const olxstr& fn);
  // does not affect the file system - can only remove a cached file
  static bool Remove(const olxstr& fn);

  void LibExists(const TStrObjList& Params, TMacroData& E);
  void LibDump(TStrObjList &Cmds, const TParamList &Options,
    TMacroData &Error);
  static void LibClear(TStrObjList &Cmds, const TParamList &Options,
    TMacroData &Error);
  static TLibrary* ExportLibrary(const olxstr& name=EmptyString());
protected:
#ifdef _PYTHON
  static PyObject *PyInit();
  static olxcstr &ModuleName();
#endif

};
#endif
