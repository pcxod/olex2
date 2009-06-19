#ifndef __olx_win_zipfs_H
#define __olx_win_zipfs_H
/* windows specific ZIP file extracting, creation (not implemented yet) utility,
(c) O Dolomanov, 2004-2009 */

#include "defs.h"
#ifdef __WIN32__
  #include <windows.h>
  #include "filesystem.h"
  #include "unzip.h"

class TWinZipFileSystem: public AFileSystem, public IEObject  {
  HZIP zip;
  TStrList TmpFiles;
public:
  TWinZipFileSystem(const olxstr& filename, bool unused=false);
  virtual ~TWinZipFileSystem();

  virtual IDataInputStream* OpenFile(const olxstr& zip_name);
  virtual bool FileExists(const olxstr& fn)  {
    ZIPENTRY* ze = NULL;
    int zindex = -1;
    return FindZipItem(zip, fn.u_str(), true, &zindex, ze) == ZR_OK;
  }
  void ExtractAll(const olxstr& dest);

  virtual bool DelFile(const olxstr& FN)     {  throw TNotImplementedException(__OlxSourceInfo);    }
  virtual bool DelDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool AdoptFile(const TFSItem& Source){  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual bool NewDir(const olxstr& DN)      {  throw TNotImplementedException(__OlxSourceInfo);     }
  virtual bool ChangeDir(const olxstr& DN)   {  throw TNotImplementedException(__OlxSourceInfo);  }
  virtual bool AdoptStream(IInputStream& file, const olxstr& name)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }

  TEFile* OpenFileAsFile(const olxstr& Source)  {
    return (TEFile*)OpenFile(Source);
  }
};

#endif // __WIN32__

#endif
 
