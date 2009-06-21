#ifndef __olx_wxzip_fs_H
#define __olx_wxzip_fs_H

#include "filesystem.h"

#ifdef __WXWIDGETS__

#include <wx/zipstrm.h>
#include <wx/mstream.h>
#include <wx/wfstream.h>
#include "fsext.h"

struct TZipEntry  {
  olxstr ZipName;
  olxstr EntryName;
};
//---------------------------------------------------------------------------
class TZipWrapper  {
  wxZipInputStream *FInputStream;
  wxFile *wxfile;
//  wxFileInputStream *FFileInputStream;
  TSStrPObjList<olxstr,wxZipEntry*, false> FEntries;
  TSStrPObjList<olxstr,TMemoryBlock*, false> FMemoryBlocks;
  TActionQList Actions;
protected:
  TMemoryBlock* GetMemoryBlock(const olxstr &EM);
  olxstr zip_name;
  bool UseCache;
public:
  static olxstr ZipUrlSignature;

  TZipWrapper(const olxstr &zipName, bool useCache);
  TZipWrapper(TEFile* zipName, bool useCache);
  
  TActionQueue* OnProgress;

  ~TZipWrapper();
  IDataInputStream* OpenEntry(const olxstr& EN);
  wxInputStream* OpenWxEntry(const olxstr& EN);
  void ExtractAll(const olxstr& dest);
  inline int Count()               const {  return FEntries.Count();  }
  inline const olxstr& Name(int i) const {  return FEntries.GetString(i);  }
  inline time_t Timestamp(int i)   const {  return FEntries.GetObject(i)->GetDateTime().GetTicks();  } 
  inline bool FileExists(const olxstr& fn) const {  return FEntries[TEFile::UnixPath(fn)] != NULL;  }

  static bool IsValidFileName(const olxstr &FN);
  static bool IsZipFile(const olxstr &FN);
  static olxstr ExtractZipName(const olxstr &FN);
  static olxstr ExtractZipEntryName(const olxstr &FN);
  static bool SplitZipUrl(const olxstr &fullName, TZipEntry &ZE);
  static olxstr ComposeFileName(const olxstr &ZipFileNameA, const olxstr &FNA);
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TwxZipFileSystem: public AFileSystem, public AActionHandler  {
  TZipWrapper zip;
protected:
  // proxying functions
  virtual bool Enter(const IEObject *Sender, const IEObject *Data) {  
    OnProgress->Enter(this, Data);
    return true;
  }
  virtual bool Exit(const IEObject *Sender, const IEObject *Data=NULL)  {  
    OnProgress->Exit(this, Data);
    return true; 
  }
  virtual bool Execute(const IEObject *Sender, const IEObject *Data=NULL) {  
    OnProgress->Execute(this, Data);
    return false; 
  }
public:
  TwxZipFileSystem(const olxstr& filename, bool UseCache=false);
  TwxZipFileSystem(TEFile* file, bool UseCache);
  virtual ~TwxZipFileSystem() {}

  virtual IDataInputStream* OpenFile(const olxstr& zip_name);
  virtual bool FileExists(const olxstr& DN)  {  return zip.FileExists(DN);  }
  void ExtractAll(const olxstr& dest);

  virtual bool DelFile(const olxstr& FN)     {  throw TNotImplementedException(__OlxSourceInfo);    }
  virtual bool DelDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool AdoptFile(const TFSItem& Source){  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual bool AdoptStream(IInputStream& in, const olxstr& as){  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual bool NewDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool ChangeDir(const olxstr& DN)   {  throw TNotImplementedException(__OlxSourceInfo);  }
};

#endif  // __WXWIDGETS__
#endif  
