#ifndef fsextH
#define fsextH

#include <wx/mstream.h>
#include "efile.h"

#include "wx/zipstrm.h"
#include "wx/image.h"
#include "wx/wfstream.h"
#include "wx/filesys.h"
#include "library.h"

class TZipWrapper;
struct TMemoryBlock  {
  char * Buffer;
  uint32_t Length;
  uint64_t DateTime;
  short PersistenceId; // dynamic property - not saved to a file
  TMemoryBlock()  {
    Buffer = NULL;
    Length = 0;
    DateTime = 0;
    PersistenceId = 0;  // not persistent
  }
};
/*____________________________________________________________________________*/
class TFileHandlerManager : public IEObject  {
  TSStrPObjList<olxstr,TZipWrapper*, false> FZipFiles;
  TSStrPObjList<olxstr,TMemoryBlock*, false> FMemoryBlocks;
  static const int16_t FVersion;
  static const char FSignature[];
protected:
  TMemoryBlock *GetMemoryBlock( const olxstr &FN );
public:
  TFileHandlerManager();
  ~TFileHandlerManager();
protected:
  static TFileHandlerManager *FHandler;
  static TStrList BaseDirs;
  void _Clear();
  IDataInputStream *_GetInputStream(const olxstr &FN);
  wxFSFile *_GetFSFileHandler( const olxstr &FN );
  void _AddMemoryBlock(const olxstr& name, char *bf, int length, short persistenceId);
  static olxstr LocateFile( const olxstr& fn );
  void _SaveToStream(IDataOutputStream& os, short persistenceMask);
  void _LoadFromStream(IDataInputStream& is, short persistenceId);
  inline bool IsMemoryBlock(const olxstr &EM) const {  
    return FMemoryBlocks[TEFile::UnixPath(EM)] != NULL;  
  }
public:
  static IDataInputStream *GetInputStream(const olxstr &FN);
  static wxFSFile *GetFSFileHandler( const olxstr &FN );
  static void Clear(short persistenceMask = ~0);
  static void AddBaseDir(const olxstr& bd);
  static void AddMemoryBlock(const olxstr& name, char *bf, int length, short persistenceId);

  static void SaveToStream(IDataOutputStream& os, short persistenceMask);
  static void LoadFromStream(IDataInputStream& is, short persistenceId);
  
  static const TMemoryBlock* FindMemoryBlock(const olxstr& bn);
  static int Count();
  static const olxstr& GetBlockName(int i);
  static long GetBlockSize(int i);
  static const olxstr& GetBlockDateTime(int i);
  static short GetPersistenceId(int i);
  static bool Exists(const olxstr& fn);

  void LibExists(const TStrObjList& Params, TMacroError& E);
  static TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
protected:
  static void PyInit();
};
#endif
