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
protected:
  olxstr zip_name;
  virtual bool _DoDelFile(const olxstr& f) {  return false;  }
  virtual bool _DoDelDir(const olxstr& f)  {  return false;  }
  virtual bool _DoNewDir(const olxstr& f)  {  return false;  }
  virtual bool _DoAdoptFile(const TFSItem& Source) {  return false;  }
  virtual bool _DoesExist(const olxstr& fn)  {
    ZIPENTRY* ze = NULL;
    int zindex = -1;
    return FindZipItem(zip, fn.u_str(), true, &zindex, ze) == ZR_OK;
  }
  virtual IInputStream* _DoOpenFile(const olxstr& src);
  virtual bool _DoAdoptStream(IInputStream& file, const olxstr& name) {  return false;  }
public:
  TWinZipFileSystem(const olxstr& filename, bool unused=false);
  virtual ~TWinZipFileSystem();

  void ExtractAll(const olxstr& dest);

  TEFile* OpenFileAsFile(const olxstr& Source)  {
    return (TEFile*)OpenFile(Source);
  }
};

#endif // __WIN32__

#endif
 
