#ifndef wxzipfsH
#define wxzipfsH

#include "filesystem.h"
#include "fsext.h"

struct TZipEntry  {
  olxstr ZipName;
  olxstr EntryName;
};
//---------------------------------------------------------------------------
class TZipWrapper  {
  wxZipInputStream *FInputStream;
  wxFileInputStream *FFileInputStream;
  TSStrPObjList<olxstr,wxZipEntry*, false> FEntries;
  TSStrPObjList<olxstr,TMemoryBlock*, false> FMemoryBlocks;
protected:
  TMemoryBlock* GetMemoryBlock(const olxstr &EM);
public:
  static olxstr ZipUrlSignature;

  TZipWrapper(const olxstr &zipName);

  ~TZipWrapper();
  IDataInputStream* OpenEntry(const olxstr &EN);
  wxInputStream* OpenWxEntry(const olxstr &EN);
  inline int Count()               const {  return FEntries.Count();  }
  inline const olxstr& Name(int i) const {  return FEntries.GetString(i);  }
  inline time_t Timestamp(int i)   const {  return FEntries.GetObject(i)->GetDateTime().GetTicks();  } 

  static bool IsValidFileName(const olxstr &FN);
  static bool IsZipFile(const olxstr &FN);
  static olxstr ExtractZipName(const olxstr &FN);
  static olxstr ExtractZipEntryName(const olxstr &FN);
  static bool SplitZipUrl(const olxstr &fullName, TZipEntry &ZE);
  static olxstr ComposeFileName(const olxstr &ZipFileNameA, const olxstr &FNA);
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
class TwxZipFileSystem: public AFileSystem, public IEObject  {
TZipWrapper zip;
public:
  TwxZipFileSystem(const olxstr& filename);
  virtual ~TwxZipFileSystem() {}

  virtual IDataInputStream* OpenFile(const olxstr& zip_name);
  virtual bool FileExists(const olxstr& DN)  {  return true;  }

  virtual bool DelFile(const olxstr& FN)     {  throw TNotImplementedException(__OlxSourceInfo);    }
  virtual bool DelDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool AdoptFile(const TFSItem& Source){  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual bool NewDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool ChangeDir(const olxstr& DN)   {  throw TNotImplementedException(__OlxSourceInfo);  }
};

#endif  
