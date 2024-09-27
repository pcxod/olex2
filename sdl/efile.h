/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_file_H
#define __olx_sdl_file_H
#include <stdio.h>
#include "estrlist.h"
#include "efilelist.h"
#include "datastream.h"
#include "wildcard.h"
BeginEsdlNamespace()

// search flags
const uint16_t
  sefFile = 0x0001,
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
const int
  sfStart   = SEEK_SET,
  sfCurrent = SEEK_CUR,
  sfEnd     = SEEK_END;
// file open attributes
const uint16_t
  fofText   = 0x0001,
  fofBinary = 0x0002,
  fofWrite  = 0x0004,
  fofRead   = 0x0008,
  fofAppend = 0x0010,
  fofCreate = 0x0020,
  fofReadWrite = 0x000C;
// access check constants, POSIX values
const short
  accessExists = 0x0000,
  accessExec   = 0x0001,
  accessWrite  = 0x0002,
  accessRead   = 0x0004;

class TEFile: public IOlxObject, public IDataInputStream,
  public IDataOutputStream
{
  FILE *FHandle;
  olxstr FName;
  bool Temporary; //tmpnam or tmpfile to use
  void CheckHandle() const;
protected:
  TEFile(const olxstr& name, FILE* handle)  {  // a temporary file
    FName = name;
    FHandle = handle;
    Temporary = true;
  }
public:
  // class for filtering files by mask.
  class TFileNameMask : public Wildcard {
  public:
    TFileNameMask() {}
    TFileNameMask(const olxstr& msk) : Wildcard(msk) {}
  };
  // a simple file ID, validates file by modification time, name and the size..
  struct FileID {
    olxstr name;
    uint64_t size;
    time_t timestamp;
    FileID() : size(InvalidSize), timestamp(-1) {}
    FileID(const olxstr& _name, uint64_t _size, time_t _timestamp)
      : name(_name), size(_size), timestamp(_timestamp) {}
    // comparison operator
    bool operator == (const FileID& fi) const {
      return (timestamp == fi.timestamp && size == fi.size && name == fi.name);
    }
    // comparison operator
    bool operator != (const FileID& fi) const { return !(*this == fi); }
    // assignment operator
    FileID& operator = (const FileID& fi) {
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
      // yet another special case...
      if( fn.Length() == 1 && fn.CharAt(0) == '*' )  {
        ExtMask.Build('*');
        NameMask.Build('*');
        return;
      }
      olxstr ext = TEFile::ExtractFileExt( fn );
      // special case when "name*" is specified - the extension can be any...
      if( ext.IsEmpty() && !fn.EndsWith('.') && fn.EndsWith('*') )
        ExtMask.Build('*');
      else
        ExtMask.Build(ext);
      NameMask.Build(
        fn.SubStringTo(fn.Length() - ext.Length() - (ext.IsEmpty() ? 0 : 1)));
    }
  };
  typedef TTypeList<MaskAssociation> MaskList;
  /* validates if given file name matches any of the provided masks buidl by
  BuildMaskList
  */
  static bool DoesMatchMasks(const olxstr& fn, const MaskList& masks);
  /* used to build masks ( ans association of mask by name and extension) from
  a semicolon separated string of masks
  */
  static void BuildMaskList(const olxstr& mask, MaskList& out);
  // a convinience, const, function
  void _seek(uint64_t off, int origin) const;
public:
  TEFile();
  TEFile(const olxstr& F, const olxstr& Attribs);
  TEFile(const olxstr& F, short Attribs);
  virtual ~TEFile();
  bool Open(const olxstr& F, const olxstr& Attribs);
  /* closes the file, if was open - returns true, might throw an exception if
  fclose failed
  */
  virtual bool Close();
  uint64_t Length() const;
  virtual void Flush();
  FILE* Handle() const {  return FHandle;  }
  FILE* ReleaseHandle() {
    FILE* rv = FHandle;
    FHandle = 0;
    return rv;
  }
  int FileNo() const {
    if (FHandle == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL handle");
    }
#ifdef _MSC_VER
    return _fileno(FHandle);
#else
    return fileno(FHandle);
#endif
  }
  virtual void Read(void *Bf, size_t count);
  virtual size_t Write(const void *Bf, size_t count);
  template <class T> inline size_t Write(const T& data) {
    return IDataOutputStream::Write(data);
  }

  virtual inline uint64_t GetSize() const {  return Length();  }
  virtual void SetPosition(uint64_t p);
  virtual uint64_t GetPosition() const;
  size_t GetAvailableSizeT(bool do_throw=true) const {
    return ((IDataInputStream*)this)->GetAvailableSizeT(do_throw);
  }
  inline const olxstr& GetName() const {  return FName; }
  // closes the file handle and deletes the file
  bool Delete();
  /* temporary file will be deleted on close, if fails - exception will be
  thrown
  */
  DefPropBIsSet(Temporary)
  // tests one or several of the accessXXXX flags
  static bool Access(const olxstr& F, const short Flags);
  // uses access, so is case sensitive, works for both dirs and files
  static bool Exists(const olxstr& F);
  // just a wrapper for chmod...
  static bool Chmod(const olxstr& f, const short Flags);
  /* a case insensitive alternative (for windows - same as above)
    the case sensitive name is stored in res (the first one if there
    are several file names matching
  */
  static bool Existsi(const olxstr& F, olxstr& res);
  // deletes the file and returns true on success
  static bool DelFile(const olxstr& F);
  // returns true if the name refers to a folder
  static bool IsDir(const olxstr& F);
  // deletes the folder if empty and returns true on succsess
  static bool RmDir(const olxstr& F);
  /* deletes the folder recursively, returns true on success, false if the name
  is a file or the deletion was not successful. If ContentOnly is provided -
  the top dir is not deleted. Returns true on success and false if an error has
  occured. If rethrow is true and an exception occurs - it will be rethrown,
  use this flag to get the details of the error
  */
  static bool DeleteDir(const olxstr& F, bool ContentOnly=false,
    bool rethrow=false);
  /* checks if the folder is empty, in case of error (non existing dir etc)
  returns false
  */
  static bool IsEmptyDir(const olxstr& F);
  // returns file drive on windows ans empty string for other platforms
  static olxstr ExtractFileDrive(const olxstr& F);
  static olxstr ExtractFilePath(const olxstr& F);
  static olxstr ParentDir(const olxstr& F);
  static olxstr ExtractFileExt(const olxstr& F);
  static olxstr ExtractFileName(const olxstr& F);
  static olxstr ChangeFileExt(const olxstr& F, const olxstr& Extension);
  static bool ListCurrentDirEx(TFileList& Out, const olxstr& Mask,
    const uint16_t searchFlags);
  static TFileList::const_list_type ListCurrentDirEx(const olxstr& Mask,
    const uint16_t searchFlags)
  {
    TFileList l;
    ListCurrentDirEx(l, Mask, searchFlags);
    return l;
  }
  static bool ListDirEx(const olxstr& dir, TFileList& Out, const olxstr& Mask,
    const uint16_t searchFlags);
  static TFileList::const_list_type ListDirEx(const olxstr& dir,
    const olxstr& Mask, const uint16_t searchFlags)
  {
    TFileList l;
    ListDirEx(dir, l, Mask, searchFlags);
    return l;
  }
  static bool ListCurrentDir(TStrList& Out, const olxstr& Mask,
    const uint16_t searchFlags);
  static TStrList::const_list_type ListCurrentDir(const olxstr& Mask,
    const uint16_t searchFlags)
  {
    TStrList l;
    ListCurrentDir(l, Mask, searchFlags);
    return l;
  }
  static bool ListDir(const olxstr& dir, TStrList& Out, const olxstr& Mask,
    const uint16_t searchFlags);
  static TStrList::const_list_type ListDir(const olxstr& dir,
    const olxstr& Mask, const uint16_t searchFlags)
  {
    TStrList l;
    ListDir(dir, l, Mask, searchFlags);
    return l;
  }
  static bool ChangeDir(const olxstr& To);
  static bool MakeDir(const olxstr& Name);
  // the function forces the creation of all dirs upto the last one
  static bool MakeDirs(const olxstr& Name);
  static olxstr CurrentDir();
  static olxstr OSPath(const olxstr& F);
  static olxstr& OSPathI(olxstr& F);
  static olxstr WinPath(const olxstr& F);
  static olxstr UnixPath(const olxstr& F);
  static olxstr& WinPathI(olxstr& F);
  static olxstr& UnixPathI(olxstr& F);
  static olxstr AddPathDelimeter(const olxstr& Path);
  static olxstr& AddPathDelimeterI(olxstr& Path);
  static olxstr TrimPathDelimeter(const olxstr& Path);
  static olxstr& TrimPathDelimeterI(olxstr& Path);
  static bool IsAbsolutePath(const olxstr& Path);
  static olxstr JoinPath(const IStrList &l);
  static char GetPathDelimeter() {
#ifdef __WIN32__
    return '\\';
#else
    return '/';
#endif
  }
  /* copies a file, use overwrite option to modify the behaviour, if overwrite
  is false and file exists the return value is false
  */
  static bool Copy(const olxstr& From, const olxstr& to,
    bool overwrite=true);
  /* renames a file, if the file with 'to' name exists, behaves according to
  the overwrite flag
  */
  static bool Rename(const olxstr& from, const olxstr& to,
    bool overwrite=true);
  /* works by creating a 'random' directory inside one folder and checking if
  it is in the other... If the function fails to create the test folder it
  returns false
  */
  static bool IsSameFolder(const olxstr& f1, const olxstr& f2);
  // a weak function - makes the decision only on the name...
  static bool IsSubFolder(const olxstr& which, const olxstr& in_what);
  /* works for windows only, for other operation systems return LocalFN
  */
  static olxstr UNCFileName(const olxstr& LocalFN);
  /* return absolute path constructued from a relative path and a base so for
  ".." and "c:/windows/win32" return "c:/windows". If path does not start from
  "." or ".. " thr normalised original is returned. If base is empty, it is
  assumed to be TBasicApp::GetBaseDir(). Fro empty path path is returned
  */
  static olxstr ExpandRelativePath(const olxstr& path,
    const olxstr& base=EmptyString());
  /* return a string like '../win32' for path='c:/windows' and
  base='c:/windows/win32' or the original path string if the paths do not share
  a common base.  If base is empty, it is assumed to be
  TBasicApp::GetBaseDir(). For empty path path is returned.
  */
  static olxstr CreateRelativePath(const olxstr& path,
    const olxstr& base=EmptyString());
  /* searches given file name in current folder and in path, if found returns
  full path to the file inclusive the file name, otherwise returns empty
  string. If the filename is absolute returns it straight away.
  */
  static olxstr Which(const olxstr& filename);
  static olxstr Which(const olxstr& filename, const TStrList &paths);
  /* returns a new object created with new using tmpnam (on non-windows
  systems), or properly named file object which is deleted upon the object
  deletion
  */
  static TEFile* TmpFile(const olxstr& templ=EmptyString());
  // function is based on utime
  static bool SetFileTimes(const olxstr& fileName, uint64_t AccTime,
    uint64_t ModTime);
  // function is based on stat;
  static time_t FileAge(const olxstr& fileName);
  // function is based on stat;
  static uint64_t FileLength(const olxstr& fileName);
  static FileID GetFileID(const olxstr& name);

  static void CheckFileExists(const olxstr& location, const olxstr& fileName);

  static olxstr QuotePath(const olxstr &p);
  static olxstr UnquotePath(const olxstr &p);

  static const olxstr& AllFilesMask() {
    static olxstr m = "*.*";
    return m;
  };

  static const_cstrlist ReadCLines(const olxstr &fn) {
    TCStrList l;
    TEFile f(fn, "rb");
    return l.LoadFromTextStream(f);
  }
  static const_wstrlist ReadWLines(const olxstr &fn) {
    TWStrList l;
    TEFile f(fn, "rb");
    return l.LoadFromTextStream(f);
  }
  static const_strlist ReadLines(const olxstr &fn) {
    TStrList l;
    TEFile f(fn, "rb");
    return l.LoadFromTextStream(f);
  }
  template <class list_t>
  static void ReadLines(const olxstr &fn, list_t &l) {
    TEFile f(fn, "rb");
    l.LoadFromTextStream(f);
  }
  template <class str_t>
  static const TTStrList<str_t> & WriteLines(const olxstr &fn,
    const TTStrList<str_t> &lines)
  {
    TEFile out(fn, "wb");
    return lines.SaveToTextStream(out);
  }

  struct Path: public IOlxObject {
    Path(const Path& p)
    : path(p.path)
    {}
    Path(const olxstr& p)
      : path(TEFile::OSPath(p))
    {}
    TStrList::const_list_type Split(const olxstr& path);
    Path GetParent() const;
    bool ChDir() const;
    Path& operator << (const olxstr& t);

    virtual TIString ToString() const { return path; }

    olxstr path;
  };
};

EndEsdlNamespace()
#endif
