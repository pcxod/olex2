//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifndef efileH
#define efileH
#include <stdio.h>
#include "estrlist.h"
#include "efilelist.h"
#include "datastream.h"

BeginEsdlNamespace()

// search flags
const uint16_t sefFile = 0x0001,
               sefDir       = 0x0002,
               sefRelDir    = 0x0004,  // "." and ".." folders
               sefAllDir    = sefDir | sefRelDir,

               sefReadOnly  = 0x0100,
               sefWriteOnly = 0x0200,
               sefReadWrite = sefReadOnly | sefWriteOnly,
               sefExecute   = 0x0400,
               sefHidden    = 0x0800,
               sefSystem    = 0x1000,
               sefRegular   = sefReadWrite | sefExecute,

               sefAll       = 0xFFFF;

// seek constants
const int      sfStart   = SEEK_SET,
               sfCurrent = SEEK_CUR,
               sfEnd     = SEEK_END;
// file open attributes
const uint16_t fofText   = 0x0001,
               fofBinary = 0x0002,
               fofWrite  = 0x0004,
               fofRead   = 0x0008,
               fofAppend = 0x0010,
               fofCreate = 0x0020,
               fofReadWrite = 0x000C;
// access check constants, POSIX values
const short accessExists = 0x0000,
            accessExec   = 0x0001,
            accessWrite  = 0x0002,
            accessRead   = 0x0004;

class TEFile: public IEObject, public IDataInputStream,
                               public IDataOutputStream  {
  FILE *FHandle;
  olxstr FName;
  bool Temporary; //tmpnam or tmpfile to use
  void CheckHandle() const;
protected:
  TEFile(const olxstr& name, FILE* handle)  {  // a temporray file
    FName = name;
    FHandle = handle;
  }
public:
  // class for filtering files by mask. other platforms then win
  class TFileNameMask  {
    TStrList toks;
    olxstr mask;
    int toksEnd, toksStart;
  public:
    TFileNameMask(const olxstr& msk)  { Build(mask);  }
    TFileNameMask() : toksEnd(-1), toksStart(-1)  {  }
    void Build(const olxstr& mask);
    bool DoesMatch(const olxstr& str)  const;
  };
  // a simple file ID, validates file by modification time, name and the size...
  struct FileID {
    olxstr name;
    size_t size;
    time_t timestamp;
    FileID(const olxstr& _name, size_t _size, time_t _timestamp) :
      name(_name), size(_size), timestamp(_timestamp) { }
    // copy constructor
    FileID(const FileID& fi) : name(fi.name), size(fi.size), timestamp(fi.timestamp) {  }
    // comparison operator
    bool operator == (const FileID& fi)  {
      return (timestamp == fi.timestamp && size == fi.size && name == fi.name);
    }
    // comparison operator
    bool operator != (const FileID& fi)  {
      return !(timestamp == fi.timestamp && size == fi.size && name == fi.name);
    }
    // assignment operator
    FileID& operator = (const FileID& fi)  {
      name = fi.name;
      size = fi.size;
      timestamp = fi.timestamp;
      return *this;
    }
  };
protected:
  struct MaskAssociation  {
    TEFile::TFileNameMask ExtMask, NameMask;
    MaskAssociation(const olxstr& fn)  {
      olxstr ext = TEFile::ExtractFileExt( fn );
      // special case when "name*" is specified - the extension can be any...
      if( ext.IsEmpty() && !fn.EndsWith('.') && fn.EndsWith('*') )
        ExtMask.Build('*');
      else
        ExtMask.Build(ext);
      NameMask.Build(fn.SubStringTo(fn.Length() - ext.Length() - (ext.IsEmpty() ? 0 : 1)));
    }
  };
  typedef TTypeList<MaskAssociation> MaskList;
  // validates if given file name matches any of the provided masks buidl by BuildMaskList
  static bool DoesMatchMasks(const olxstr& fn, const MaskList& masks);
  // used to build masks ( ans association of mask by name and extension) from a semicolon separated string of masks
  static void BuildMaskList(const olxstr& mask, MaskList& out);
public:
  TEFile();
  TEFile(const olxstr &F, const olxstr &Attribs);
  TEFile(const olxstr &F, short Attribs);
  virtual ~TEFile();
  bool Open(const olxstr &F, const olxstr &Attribs);
  // closes the file, if was open - returns true, might throw an exception if fclose failed
  bool Close();
  long Length() const;
  virtual void Flush();
  void Seek(long Position, const int From);
  FILE* Handle()  const {  return FHandle; }
  FILE* ReleaseHandle() {
    FILE* rv = FHandle;
    FHandle = NULL;
    return rv;
  }
  int FileNo() const {
    if( FHandle == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL handle");
#ifdef _MSC_VER
    return _fileno(FHandle);
#else
    return fileno(FHandle);
#endif
  }

  virtual void Read(void *Bf, size_t count);
  virtual size_t Write(const void *Bf, size_t count);
  virtual inline size_t Writenl(const void *Data, size_t size)  {
    size_t w = Write(Data, size);
    w += Write(NewLineSequence, NewLineSequenceLength);
    return w;
  }
  template <class T> inline size_t Write(const T& data)    {  return IDataOutputStream::Write(data);  }
  template <class T> inline size_t Writenl(const T& data)  {  return IDataOutputStream::Writenl(data);  }

  virtual inline size_t GetSize() const {  return Length();  }
  virtual void SetPosition(size_t p);
  virtual size_t GetPosition() const;

  inline const olxstr& GetName()  const  {  return FName; }
  bool Delete();

  static bool Access(const olxstr& F, const short Flags);
  // uses access, so is case sensitive
  static bool FileExists(const olxstr &F);
  /* a case insensitive alternative (for windows - same as above) 
    the case sensitive name is stored in res (the first on if there
    are several file names matching
  */
  static bool FileExistsi(const olxstr &F, olxstr& res);
  static bool DelFile(const olxstr &F);
  static bool DelDir(const olxstr &F);
  static olxstr ExtractFileDrive(const olxstr &F);
  static olxstr ExtractFilePath(const olxstr &F);
  static olxstr ParentDir(const olxstr& F);
  static olxstr ExtractFileExt(const olxstr &F);
  static olxstr ExtractFileName(const olxstr &F);
  static olxstr ChangeFileExt(const olxstr &F, const olxstr &Extension);
  static bool ListCurrentDirEx(TFileList &Out, const olxstr &Mask, const unsigned short searchFlags);
  static bool ListDirEx(const olxstr& dir, TFileList &Out, const olxstr &Mask, const unsigned short searchFlags);
  static bool ListCurrentDir(TStrList& Out, const olxstr &Mask, const unsigned short searchFlags);
  static bool ListDir(const olxstr& dir, TStrList& Out, const olxstr &Mask, const unsigned short searchFlags);
  static bool ChangeDir(const olxstr &To);
  static bool MakeDir(const olxstr &Name);
  // the function forces the creation of all dirs upto the last one
  static bool MakeDirs(const olxstr &Name);
  static olxstr CurrentDir();
  static olxstr OSPath(const olxstr &F);
  static olxstr& OSPathI(olxstr &F);
  static olxstr WinPath(const olxstr &F);
  static olxstr UnixPath(const olxstr &F);
  static olxstr& WinPathI(olxstr &F);
  static olxstr& UnixPathI(olxstr &F);
  static olxstr AddTrailingBackslash(const olxstr &Path);
  static olxstr& AddTrailingBackslashI(olxstr &Path);
  static olxstr RemoveTrailingBackslash(const olxstr &Path);
  static olxstr& RemoveTrailingBackslashI(olxstr &Path);
  static bool IsAbsolutePath(const olxstr &Path);
  static char GetPathDelimeter()  {
#ifdef __WIN32__
    return '\\';
#else
    return '/';
#endif
  }
  // copies a file, use overwrite option to modify the behaviour
  static void Copy(const olxstr& From, const olxstr& to, bool overwrite = true);
  // renames a file, if the file with 'to' name exists, behaves according to the overwrite flag
  static bool Rename(const olxstr& from, const olxstr& to, bool overwrite = true);
  /*works for windows only, for other operation systems return LocalFN*/
  static olxstr UNCFileName(const olxstr &LocalFN);
  /* return absolute path as a relative to another path
    so for "c:/windows/win32" and ".." return "c:/windows"
    if the resulting path does not exist - throws an exception
    */
  static olxstr AbsolutePathTo(const olxstr &Path, const olxstr &relPath );
  /* searches given file name in current folder and in path, if found returns full
    path to the file inclusive the file name, otherwise returns empty string.
    if the filename is absolute returns it straight away
  */
  static olxstr Which(const olxstr& filename);
  // returns a new object created with new using tmpnam
  static TEFile* TmpFile(const olxstr& templ);
  // function is based on utime
  static bool SetFileTimes(const olxstr& fileName, uint64_t AccTime, uint64_t ModTime);
  // function is based on stat;
  static time_t FileAge(const olxstr& fileName);
  // function is based on stat;
  static uint64_t FileLength(const olxstr& fileName);
  static FileID GetFileID(const olxstr& name);

  static void CheckFileExists(const olxstr& location, const olxstr& fileName);
  
  static const olxstr AllFilesMask;

  static class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};


EndEsdlNamespace()
#endif
