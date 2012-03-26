/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "efile.h"
#include "filetree.h"
#include <string.h>
#include <sys/stat.h>
#include "exception.h"
#include "estrlist.h"

#include "library.h"
#include "bapp.h"

#ifdef __WIN32__
  #include <winbase.h>
  #include <io.h>
  #include <direct.h>
  #define OLXSTR(A) (A).u_str()
  #ifdef _UNICODE
    #define UTIME _wutime
    #define unlink _wunlink
    #define rmdir _wrmdir
    #define STAT _wstati64
    #define STAT_STR _stati64
    #define chmod _wchmod
    #define chdir _wchdir
    #define access _waccess
    #define getcwd _wgetcwd
    #define makedir _wmkdir
    #define fopen _wfopen
    #define rename _wrename
    #define WinCopyFile CopyFileW
  #else
    #define STAT stat
    #define STAT_STR stat
    #define WinCopyFile CopyFileA
  #endif
  #ifndef __BORLANDC__ // this (as Builder 6.0 has no support for it
  #  define ftell _ftelli64
  #  define fseek _fseeki64
  #endif
  #ifdef _MSC_VER
    #include <sys/utime.h>
    #ifndef _UNICODE
      #define chmod _chmod
      #define chdir _chdir
      #define access _access
      #define getcwd _getcwd
      #define makedir _mkdir
      #define UTIME _utime
      #define unlink _unlink
      #define rmdir _rmdir
    #endif
    #define UTIMBUF _utimbuf
  #else
    #include <utime.h>
    #ifndef _UNICODE
      #define UTIME utime
      #define makedir mkdir
    #endif
    #define UTIMBUF utimbuf
  #endif
  //this is only for UNC file names under windows
  #include <mapiutil.h>

  #define OLX_PATH_DEL '\\'
  #define OLX_ENVI_PATH_DEL ';'
  #define OLX_OS_PATH(A)  TEFile::WinPath( (A) )
  #define OLX_OS_PATHI(A) TEFile::WinPathI( (A) )
#else
  #include <unistd.h>
  #include <stdlib.h>
  #include <dirent.h>
  #include <utime.h>

  #define OLXSTR(A) olxcstr(A).c_str()  //have to make it thread safe
  #ifndef __MAC__  // could not find these on MAC...
    #define ftell ftello64
    #define fseek fseeko64
  #endif
  #define makedir(a) mkdir((a), 0755)
  #define UTIME utime
  #define STAT stat
  #define STAT_STR stat

  #define OLX_PATH_DEL '/'
  #define OLX_ENVI_PATH_DEL ':'
  #define OLX_OS_PATH(A)  TEFile::UnixPath((A))
  #define OLX_OS_PATHI(A) TEFile::UnixPathI((A))
  #define UTIMBUF utimbuf
#endif

#ifndef MAX_PATH
  #define MAX_PATH 1024
#endif

UseEsdlNamespace()

const olxstr TEFile::AllFilesMask("*.*");
//----------------------------------------------------------------------------//
// TFileNameMask function bodies
//----------------------------------------------------------------------------//
void TEFile::TFileNameMask::Build(const olxstr& msk)  {
  mask = msk.ToLowerCase();
  toks.Strtok( mask, '*');
  if( !mask.IsEmpty() )  {
    toksStart = (mask[0] != '*') ? 1 : 0;
    toksEnd = toks.Count() - ((mask[mask.Length()-1] != '*') ? 1 : 0);
  }
  else  {
    toksStart = 0;
    toksEnd = toks.Count();
  }
}
//..............................................................................
bool TEFile::TFileNameMask::DoesMatch(const olxstr& _str) const {
  if( mask.IsEmpty() && !_str.IsEmpty() )  return false;
  // this will work for '*' mask
  if( toks.IsEmpty() )  return true;
  // need to check if the mask starts from a '*' or ends with it
  olxstr str = _str.ToLowerCase();
  size_t off = 0, start = 0, end = str.Length();
  if( mask[0] != '*' )  {
    const olxstr& tmp = toks[0];
    if( tmp.Length() > str.Length() )  return false;
    for( size_t i=0; i < tmp.Length(); i++ )
     if( tmp.CharAt(i) != '?' && tmp.CharAt(i) != str.CharAt(i) )  return false;
    start = tmp.Length();
    if( toks.Count() == 1 )  
      return tmp.Length() == str.Length() ? true : mask[ mask.Length()-1] == '*';
  }
  if( mask.GetLast() != '*' && toks.Count() > (size_t)(mask.CharAt(0) != '*' ? 1 : 0) )  {
    olxstr& tmp = toks[toks.Count()-1];
    if( tmp.Length() > (str.Length()-start) )  return false;
    for( size_t i=0; i < tmp.Length(); i++ )
     if( !(tmp.CharAt(i) == '?' || tmp.CharAt(i) == str.CharAt(str.Length()-tmp.Length() + i)) )  return false;
    end = str.Length() - tmp.Length();

    if( toks.Count() == 1 )  return true;
  }

  for( size_t i=toksStart; i < toksEnd; i++ )  {
    olxstr& tmp = toks[i];
    bool found = false;
    for( size_t j=start; j < end; j++ )  {
      if( (str.Length() - j) < tmp.Length() )  return false;
      if( tmp.CharAt(off) == '?' || str.CharAt(j) == tmp.CharAt(off) )  {
        while( tmp.CharAt(off) == '?' || tmp.CharAt(off) == str.CharAt(j+off) )  {
          off++;
          if( off == tmp.Length() )  break;
        }
        start = j+off;
        if( off == tmp.Length() )  {  // found the mask string
          found = true;
          off = 0;
          break;
        }
        off = 0;
      }
    }
    if( !found )  return false;
  }
  return true;
}



//----------------------------------------------------------------------------//
// TEFile function bodies
//----------------------------------------------------------------------------//
TEFile::TEFile()  {
  FHandle = NULL;
  Temporary = false;
}
//..............................................................................
TEFile::TEFile(const olxstr &F, const olxstr &Attribs)  {
  Temporary = false;
  FHandle = NULL;
  Open(F, Attribs);
}
//..............................................................................
TEFile::TEFile(const olxstr& F, short Attribs)  {
  Temporary = false;
  throw TNotImplementedException(__OlxSourceInfo);
/*
  olxstr attr;
  if( (Attribs & fofRead) != 0 )  {
    attr << 'r';
    if( (Attribs & fofWrite) != 0 )
      attr << 'w';
    if( (Attribs & fofAppend) != 0 )
      attr << 'a';
    if( (Attribs & fofCreate) != 0 )
      attr << '+';
  }
  else if( (Attribs & fofWrite) != 0 )  {
  }
*/
}
//..............................................................................
TEFile::~TEFile()  {  Close();  }
//..............................................................................
bool TEFile::Open(const olxstr& F, const olxstr& Attribs)  {
  Close();
  FName = OLX_OS_PATH(F);
  FHandle = fopen( OLXSTR(FName), OLXSTR(Attribs));
  if( FHandle == NULL )  {
    olxstr fn = FName;
    FName.SetLength(0);
    throw TFileException(__OlxSourceInfo, F, olxstr("NULL handle for '") << F << '\'');
  }
  return true;
}
//..............................................................................
bool TEFile::Close()  {
  if( FHandle != NULL )  {
    if( fclose(FHandle) != 0 )
      throw TFileException(__OlxSourceInfo, FName, "fclose failed");
    FHandle = NULL;
    if( Temporary )  {
      if( !DelFile(FName) )
        throw TFileException(__OlxSourceInfo, FName, "could not remove temporary file");
    }
    return true;
  }
  return false;
}
//..............................................................................
bool TEFile::Delete()  {
  if( FHandle == NULL )  return false;
  Close();
  return TEFile::DelFile(FName);
}
//..............................................................................
void TEFile::CheckHandle() const  {
  if( FHandle == NULL )
    throw TFileException(__OlxSourceInfo, EmptyString(), "Invalid file handle");
}
//..............................................................................
void TEFile::Read(void *Bf, size_t count)  {
  CheckHandle();
  if( count == 0 )  return;
  size_t res = fread(Bf, count, 1, FHandle);
  if( res != 1 )
    throw TFileException(__OlxSourceInfo, FName, "fread failed" );
}
//..............................................................................
void TEFile::_seek(uint64_t off, int origin) const {
  if( fseek(FHandle, off, origin) != 0 )  
    throw TFileException(__OlxSourceInfo, FName, "fseek failed" );
}
//..............................................................................
void TEFile::SetPosition(uint64_t p)  {
  CheckHandle();
  _seek(p, SEEK_SET);
}
//..............................................................................
uint64_t TEFile::GetPosition() const  {
  CheckHandle();
  int64_t v = ftell(FHandle);
  if( v == -1 )
    throw TFileException(__OlxSourceInfo, FName, "ftell failed" );
  return v;
}
//..............................................................................
uint64_t TEFile::Length() const  {
  CheckHandle();
  uint64_t currentPos = GetPosition();
  _seek(0, SEEK_END);
  int64_t length = ftell(FHandle);
  _seek(currentPos, SEEK_SET);
  if( length == -1 )
    throw TFileException(__OlxSourceInfo, FName, "ftell failed");
  return length;
}
//..............................................................................
void TEFile::Flush()  {
  CheckHandle();
  fflush(FHandle);
}
//..............................................................................
size_t TEFile::Write(const void *Bf, size_t count)  {
  CheckHandle();
  if( count == 0 )  return count;
  size_t res = fwrite(Bf, count, 1, FHandle);
  if( res == 0 )
    throw TFileException(__OlxSourceInfo, FName, "fwrite failed");
  return res;
}
//..............................................................................
bool TEFile::Exists(const olxstr& F)  {
  return (access( OLXSTR(OLX_OS_PATH(F)), 0) != -1);
}
//..............................................................................
bool TEFile::Existsi(const olxstr& F, olxstr& res)  {
  if( F.IsEmpty() )
    return false;
#ifdef __WIN32__
  return Exists(F);
#else
  olxstr path = ExtractFilePath(F);
  olxstr name = ExtractFileName(F);
  TStrList files;
  if( path.IsEmpty() )  
    TEFile::ListCurrentDir(files, name, sefAll);
  else
    TEFile::ListDir(path, files, name, sefAll);
  if( files.IsEmpty() )
    return false;
  res = TEFile::AddPathDelimeterI(path) << files[0];
  return true;
#endif
}
//..............................................................................
bool TEFile::Access(const olxstr& F, const short Flags)  {
  return (access(OLXSTR(OLX_OS_PATH(F)), Flags) != -1);
}
//..............................................................................
bool TEFile::Chmod(const olxstr& F, const short Flags)  {
  return (chmod(OLXSTR(OLX_OS_PATH(F)), Flags) != -1);
}
//..............................................................................
olxstr TEFile::ExtractFilePath(const olxstr &F)  {
  olxstr fn = OLX_OS_PATH(F);
  if( TEFile::IsAbsolutePath(fn) )  {
    size_t i = fn.LastIndexOf( OLX_PATH_DEL );
    if( i > 0 && i != InvalidIndex )
      return fn.SubStringTo(i+1);
    return EmptyString();
  }
  return EmptyString();
}
//..............................................................................
olxstr TEFile::ParentDir(const olxstr& name) {
  if( name.IsEmpty() )  return name;
  // normalise path
  olxstr np = OLX_OS_PATH(name);
  size_t start = (np.GetLast() == OLX_PATH_DEL ? np.Length()-2 : np.Length()-1);
  size_t i = np.LastIndexOf(OLX_PATH_DEL, start);
  if( i > 0 && i != InvalidIndex )
    return np.SubStringTo(i+1);
  return EmptyString();
}
//..............................................................................
olxstr TEFile::ExtractFileExt(const olxstr& F)  {
  //if( F.IsEmpty() || IsDir(F) )  return EmptyString();
  if( F.IsEmpty() )  return EmptyString();
  olxstr fn = OLX_OS_PATH(F);
  size_t i = fn.LastIndexOf('.');
  if( i > 0 && i != InvalidIndex )  {
    size_t del_ind = fn.LastIndexOf(OLX_PATH_DEL); 
    if( del_ind != InvalidIndex && del_ind > i )
      return EmptyString();
    return fn.SubStringFrom(i+1);
  }
  return EmptyString();
}
//..............................................................................
olxstr TEFile::ExtractFileName(const olxstr& F)  {
  if( F.IsEmpty() || IsDir(F) )  return EmptyString();
  olxstr fn = OLX_OS_PATH(F);
  size_t i = fn.LastIndexOf(OLX_PATH_DEL);
  if( i > 0 && i != InvalidIndex )
    return fn.SubStringFrom(i+1);
  return F;
}
//..............................................................................
olxstr TEFile::ExtractFileDrive(const olxstr& F)  {
#ifdef __WIN32__
  if( F.Length() < 2 )  return EmptyString();
  if( F[1] != ':' )  return EmptyString();
  return F.SubString(0, 2);
#else
  return EmptyString();
#endif
}
//..............................................................................
olxstr TEFile::ChangeFileExt(const olxstr &F, const olxstr &Ext)  {
  //if( F.IsEmpty() || IsDir(F) )  return EmptyString();
  if( F.IsEmpty() )  return EmptyString();
  olxstr fn = OLX_OS_PATH(F);
  size_t i = fn.LastIndexOf('.');
  size_t d_i = fn.LastIndexOf(OLX_PATH_DEL);
  if( i != InvalidIndex && i > 0 && (d_i == InvalidIndex || d_i < i) )
    fn.SetLength(i);
  else  {
    if( fn.GetLast() == '.' )
      fn.SetLength(fn.Length()-1);
  }
  if( !Ext.IsEmpty() )  {
    if( Ext.CharAt(0) != '.' )  
      fn << '.';
    fn << Ext;
  }
  return fn;
}
//..............................................................................
bool TEFile::DelFile(const olxstr& F)  {
  if( !Exists(F) )  return true;
  olxstr fn = OLX_OS_PATH(F);
  int res = chmod(OLXSTR(fn), S_IWRITE);
#ifdef __WIN32__
  if( res != 0 )
    return false;
#else  // POSIX -1 - no change!
  if( res != 0 && res != -1 )
    return false;
#endif 
  return unlink(OLXSTR(fn)) != -1;
}
//..............................................................................
bool TEFile::RmDir(const olxstr& F)  {
  if( !Exists(F) )  return true;
  return rmdir(OLXSTR(OLX_OS_PATH(F))) != -1;
}
//..............................................................................
bool TEFile::IsDir(const olxstr& F)  {
  if( F.IsEmpty() )  return false;
  struct STAT_STR the_stat;
  olxstr fn = OLX_OS_PATH(F);
  if( fn.EndsWith(OLX_PATH_DEL) )  {
#ifdef __WIN32__
    if( !(fn.Length() == 3 && fn.CharAt(1) == ':' ) ) // e:\ is a dir e: not!
#else
    if( fn.Length() > 1 )  // / is a dir, nothing is not...
#endif
      fn = fn.SubStringFrom(0, 1);
  }
  if( STAT(OLXSTR(fn), &the_stat) != 0 )
    return false;
  return (the_stat.st_mode & S_IFDIR) != 0;
}
//..............................................................................
bool TEFile::DeleteDir(const olxstr& F, bool ContentOnly)  {
  olxstr fn = OLX_OS_PATH(F);
  if( !Exists(fn) || !TEFile::IsDir(fn) )  
    return false;
  try  {
    TFileTree::Delete(fn, ContentOnly);
    return true;
  }
  catch( TExceptionBase& )  {  return false;  }
}
//..............................................................................
bool TEFile::IsEmptyDir(const olxstr& F)  {
  olxstr fn = OLX_OS_PATH(F);
  if( !Exists(fn) || !TEFile::IsDir(fn) )  
    throw TFunctionFailedException(__OlxSourceInfo, "The directory does not exist");
  TStrList out;
  if( !TEFile::ListDir(fn, out, "*", sefAll^sefRelDir) )
    throw TFunctionFailedException(__OlxSourceInfo, "Failed to list the directory");
  return out.IsEmpty();
}
//..............................................................................
bool TEFile::DoesMatchMasks(const olxstr& _fn, const MaskList& masks)  {
  olxstr ext = TEFile::ExtractFileExt( _fn );
  olxstr fn = _fn.SubStringTo(_fn.Length() - ext.Length() - (ext.IsEmpty() ? 0 : 1));
  for( size_t i=0; i < masks.Count(); i++ )
    if( masks[i].ExtMask.DoesMatch(ext) && masks[i].NameMask.DoesMatch(fn) )
      return true;
  return false;
}
//..............................................................................
void TEFile::BuildMaskList(const olxstr& mask, MaskList& masks)  {
  TStrList ml(mask, ';');
  for( size_t i=0; i < ml.Count(); i++ )
    masks.AddNew(ml[i]);
}
//..............................................................................
#ifdef __WIN32__
bool TEFile::ListCurrentDirEx(TFileList &Out, const olxstr &Mask, const uint16_t sF)  {
  MaskList masks;
  BuildMaskList(Mask, masks);

  WIN32_FIND_DATA sd;
  struct STAT_STR the_stat;
  memset(&sd, 0, sizeof(sd));

  int flags = 0;
  unsigned short attrib;
  if( (sF & sefDir) != 0 )       flags |= FILE_ATTRIBUTE_DIRECTORY;
  if( (sF & sefReadOnly) != 0 )  flags |= FILE_ATTRIBUTE_READONLY;
  if( (sF & sefSystem) != 0 )    flags |= FILE_ATTRIBUTE_SYSTEM;
  if( (sF & sefHidden) != 0 )    flags |= FILE_ATTRIBUTE_HIDDEN;
  sd.dwFileAttributes = flags; 
  HANDLE hn = FindFirstFile(AllFilesMask.u_str(), &sd);
  if( hn == INVALID_HANDLE_VALUE )  
    return false;
  bool done = true;
  while( done )  {
    if( (sF & sefDir) != 0 && (sF & sefRelDir) == 0 && (sd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)  {
      size_t len = olxstr::o_strlen(sd.cFileName);
      if( (len == 1 && sd.cFileName[0] == '.') || (len == 2 && sd.cFileName[0] == '.' && sd.cFileName[1] == '.') )  {
        done = (FindNextFile(hn, &sd) != 0);
        continue;
      }
    }
    if( DoesMatchMasks(sd.cFileName, masks) )  {
      TFileListItem& li = Out.AddNew();
      li.SetName( sd.cFileName );
      uint64_t lv = sd.nFileSizeLow;
      lv += sd.nFileSizeHigh * MAXDWORD;
      li.SetSize( lv );
      if( STAT(sd.cFileName, &the_stat) == 0 )  {
        li.SetCreationTime( the_stat.st_ctime );
        li.SetModificationTime( the_stat.st_mtime );
        li.SetLastAccessTime( the_stat.st_atime );
      }
      else
        throw TFunctionFailedException(__OlxSourceInfo, "stat failed");
      attrib = 0;
      if( (sd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 )  
        attrib |= sefDir;
      else                                      attrib |= sefFile;
      if( (sd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0 ) 
        attrib |= sefReadOnly;
      if( (sd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) != 0 ) 
        attrib |= sefSystem;
      if( (sd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0 ) 
        attrib |= sefHidden;
      li.SetAttributes( attrib );
    }
    done = (FindNextFile(hn, &sd) != 0);
  }
  FindClose( hn );
  return true;
}
bool TEFile::ListCurrentDir(TStrList& Out, const olxstr &Mask, const uint16_t sF)  {
  MaskList masks;
  BuildMaskList(Mask, masks);
  WIN32_FIND_DATA sd;
  memset(&sd, 0, sizeof(sd));

  int flags = 0;
  if( (sF & sefDir) != 0 )       flags |= FILE_ATTRIBUTE_DIRECTORY;
  if( (sF & sefReadOnly) != 0 )  flags |= FILE_ATTRIBUTE_READONLY;
  if( (sF & sefSystem) != 0 )    flags |= FILE_ATTRIBUTE_SYSTEM;
  if( (sF & sefHidden) != 0 )    flags |= FILE_ATTRIBUTE_HIDDEN;
  sd.dwFileAttributes = flags; 
  HANDLE hn = FindFirstFile(AllFilesMask.u_str(), &sd);
  if( hn == INVALID_HANDLE_VALUE )  
    return false;
  bool done = true;
  while( done )  {
    if( (sF & sefDir) != 0 && (sF & sefRelDir) == 0 && (sd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)  {
      size_t len = olxstr::o_strlen(sd.cFileName);
      if( (len == 1 && sd.cFileName[0] == '.') || (len == 2 && sd.cFileName[0] == '.' && sd.cFileName[1] == '.') )  {
        done = (FindNextFile(hn, &sd) != 0);
        continue;
      }
    }
    if( DoesMatchMasks(sd.cFileName, masks) )
      Out.Add( sd.cFileName );
    done = (FindNextFile(hn, &sd) != 0);
  }
  FindClose(hn);
  return true;
}
#else
//..............................................................................
bool TEFile::ListCurrentDirEx(TFileList &Out, const olxstr &Mask, const uint16_t sF)  {
  DIR *d = opendir( olxcstr(TEFile::CurrentDir()).c_str() );
  if( d == NULL ) return false;
  MaskList masks;
  BuildMaskList(Mask, masks);
  int access = 0;
  if( (sF & sefReadOnly) != 0 )  access |= S_IRUSR;
  if( (sF & sefWriteOnly) != 0 ) access |= S_IWUSR;
  if( (sF & sefExecute) != 0 )   access |= S_IXUSR;

  struct stat the_stat;
  struct dirent* de;
  while((de = readdir(d)) != NULL) {
    stat(de->d_name, &the_stat);
    if( sF != sefAll )  {
      if( access != 0 )  {
        int faccess = 0;
        if( (the_stat.st_mode & S_IRUSR) != 0 )  faccess |= S_IRUSR;
        if( (the_stat.st_mode & S_IWUSR) != 0 )  faccess |= S_IWUSR;
        if( (the_stat.st_mode & S_IXUSR) != 0 )  faccess |= S_IXUSR;
        if( (faccess & access) == 0 )  continue;
      }
      if( (sF & sefDir) != 0 )  {
        if( S_ISDIR(the_stat.st_mode) )  {
          if( (sF & sefRelDir) == 0 )
            if( strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 )
              continue;
        }
        else if( (sF & sefFile) == 0 )  continue;
      }
      else if( (sF & sefFile) == 0 || S_ISDIR(the_stat.st_mode) )  continue;
    }
    if( DoesMatchMasks(de->d_name, masks) )  {
      TFileListItem& li = Out.AddNew();
      li.SetName( de->d_name );
      li.SetSize( the_stat.st_size );
      li.SetCreationTime( the_stat.st_ctime );
      li.SetModificationTime( the_stat.st_mtime );
      li.SetLastAccessTime( the_stat.st_atime );
      uint16_t attrib = 0;
      if( S_ISDIR(the_stat.st_mode) )           attrib |= sefDir;
      else                                      attrib |= sefFile;
      if( (the_stat.st_mode & S_IRUSR) != 0 )  attrib |= sefReadOnly;
      if( (the_stat.st_mode & S_IWUSR) != 0 )  attrib |= sefWriteOnly;
      if( (the_stat.st_mode & S_IXUSR) != 0 )  attrib |= sefExecute;
      li.SetAttributes( attrib );
    }
  }
  return closedir(d) == 0;
}
//..............................................................................
bool TEFile::ListCurrentDir(TStrList &Out, const olxstr &Mask, const uint16_t sF)  {
  DIR *d = opendir( olxcstr(TEFile::CurrentDir()).c_str() );
  if( d == NULL ) return false;
  MaskList masks;
  BuildMaskList(Mask, masks);
  olxstr tmp, fn;
  int access = 0;
  if( (sF & sefReadOnly) != 0 )  access |= S_IRUSR;
  if( (sF & sefWriteOnly) != 0 ) access |= S_IWUSR;
  if( (sF & sefExecute) != 0 )   access |= S_IXUSR;

  struct stat the_stat;
  struct dirent* de;
  while((de = readdir(d)) != NULL) {
    if( sF != sefAll )  {
      stat(de->d_name, &the_stat);
      if( access != 0 )  {
        int faccess = 0;
        if( (the_stat.st_mode & S_IRUSR) != 0 )  faccess |= S_IRUSR;
        if( (the_stat.st_mode & S_IWUSR) != 0 )  faccess |= S_IWUSR;
        if( (the_stat.st_mode & S_IXUSR) != 0 )  faccess |= S_IXUSR;
        if( (faccess & access) == 0 )  continue;
      }
      if( (sF & sefDir) != 0 )  {
        if( S_ISDIR(the_stat.st_mode) )  {
          if( (sF & sefRelDir) == 0 )
            if( strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0 )
              continue;
        }
        else if( (sF & sefFile) == 0 )  continue;
      }
      else if( (sF & sefFile) == 0 || S_ISDIR(the_stat.st_mode) )  continue;
    }
    if( DoesMatchMasks(de->d_name, masks) )
      Out.Add(de->d_name);
  }
  return closedir(d) == 0;
}
#endif
//..............................................................................
bool TEFile::ListDirEx(const olxstr& dir, TFileList &Out, const olxstr &Mask, const uint16_t sF)  {
  olxstr cd( TEFile::CurrentDir() );
  if( !TEFile::ChangeDir(dir) )
    return false;
  bool res = ListCurrentDirEx(Out, Mask, sF);
  TEFile::ChangeDir(cd);
  return res;
}
//..............................................................................
bool TEFile::ListDir(const olxstr& dir, TStrList &Out, const olxstr &Mask, const uint16_t sF)  {
  olxstr cd( TEFile::CurrentDir() );
  if( !TEFile::ChangeDir(dir) )
    return false;
  bool res = ListCurrentDir(Out, Mask, sF);
  TEFile::ChangeDir(cd);
  return res;
}
//..............................................................................
bool TEFile::SetFileTimes(const olxstr& fileName, uint64_t AccTime, uint64_t ModTime)  {
  struct UTIMBUF tb;
  tb.actime = AccTime;
  tb.modtime = ModTime;
  return UTIME(OLXSTR(OLX_OS_PATH(fileName)), &tb) == 0;
}
//..............................................................................
time_t TEFile::FileAge(const olxstr& fileName)  {
  struct STAT_STR the_stat;
  if( STAT(OLXSTR(OLX_OS_PATH(fileName)), &the_stat) != 0 )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Invalid file '") << fileName << '\'');
#ifdef __BORLANDC__
  return the_stat.st_mtime;
#elif _MSC_VER
  return the_stat.st_mtime;
#elif __GNUC__
  return the_stat.st_mtime;
#else
  struct timespec& t = the_stat.st_mtimespec;
  return t.tv_nsec + t.tv_nsec*1e-9; // number of seconds since last modification
#endif
}
//..............................................................................
uint64_t TEFile::FileLength(const olxstr& fileName)  {
  struct STAT_STR the_stat;
  if( STAT(OLXSTR(OLX_OS_PATH(fileName)), &the_stat) != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "stat failed");
  return the_stat.st_size;
}
//..............................................................................
TEFile::FileID TEFile::GetFileID(const olxstr& fileName)  {
  olxstr _name( OLX_OS_PATH(fileName) );
  time_t _timestamp = 0;
  struct STAT_STR the_stat;
  if( STAT(OLXSTR(_name), &the_stat) != 0 )
    throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Invalid file '") << _name << '\'');
#if defined(__BORLANDC__) || defined(_MSC_VER) || defined(__GNUC__)
  _timestamp = the_stat.st_mtime;
#else
  struct timespec& t = the_stat.st_mtimespec;
  _timestamp = t.tv_nsec + t.tv_nsec*1e-9; // number of seconds since last modification
#endif
  return FileID(ExtractFileName(_name), the_stat.st_size, _timestamp);
}
//..............................................................................
bool TEFile::ChangeDir(const olxstr& To)  {
  if( To.IsEmpty() )  return false;
  olxstr path = OLX_OS_PATH(To);
#ifdef __WIN32__
  return SetCurrentDirectory(path.u_str()) != 0;
#else
  return chdir(olxcstr(path).c_str()) != -1;
#endif
}
//..............................................................................
olxstr TEFile::CurrentDir()  {
#ifdef __WIN32__
  olxch bf[MAX_PATH];
  if( GetCurrentDirectory(MAX_PATH, bf) == 0 )
    return EmptyString();
  return olxstr(bf);
#else
  char *Dp = getcwd(NULL, MAX_PATH);
  olxstr Dir( Dp );
  free(Dp);
  return Dir;
#endif
}
//..............................................................................
bool TEFile::MakeDirs(const olxstr& Name)  {
  const olxstr dn = OLX_OS_PATH(Name);
  if( Exists(dn) )  return true;
  TStrList toks(dn, OLX_PATH_DEL);
  olxstr toCreate;
  toCreate.SetCapacity( Name.Length() + 5 );
  if( Name.StartsFrom('/') ) // !!!! linux
    toCreate << '/';
  else if( Name.StartsFrom("\\\\") ) {  // server name
    toCreate << "\\\\";
    if( !toks.IsEmpty() )  {  // put the server name back
      toCreate << toks[0] << OLX_PATH_DEL;
      toks.Delete(0);
    }
  }
  for( size_t i=0; i < toks.Count(); i++ )  {
    toCreate << toks[i] << OLX_PATH_DEL;
    if( !Exists( toCreate ) )
      if( !MakeDir( toCreate ) )
        return false;
  }
  return true;
}
//..............................................................................
bool TEFile::MakeDir(const olxstr& Name)  {
  return (makedir(OLXSTR(OLX_OS_PATH(Name))) != -1);
}
//..............................................................................
olxstr TEFile::OSPath(const olxstr &F)  {
  return OLX_OS_PATH(F);
}
//..............................................................................
olxstr& TEFile::OSPathI(olxstr &F)  {
  return OLX_OS_PATHI(F);
}
//..............................................................................
olxstr TEFile::WinPath(const olxstr &F)  {
  return olxstr(F).Replace('/', '\\');
}
//..............................................................................
olxstr TEFile::UnixPath(const olxstr& F) {
  return olxstr(F).Replace('\\', '/');
}
//..............................................................................
olxstr& TEFile::WinPathI( olxstr& F )  {
  return F.Replace('/', '\\');
}
//..............................................................................
olxstr& TEFile::UnixPathI( olxstr& F )  {
  return F.Replace('\\', '/');
}
//..............................................................................
olxstr TEFile::AddPathDelimeter( const olxstr& Path )  {
  if( Path.IsEmpty() )  
    return olxstr(OLX_PATH_DEL);
  olxstr T = OLX_OS_PATH(Path);
  if( T[T.Length()-1] != OLX_PATH_DEL )  
    T << OLX_PATH_DEL;
  return T;
}
//..............................................................................
olxstr& TEFile::AddPathDelimeterI(olxstr &Path)  {
  if( Path.IsEmpty() )  return Path;
  OLX_OS_PATHI(Path);
  if( Path[Path.Length()-1] != OLX_PATH_DEL )  
    Path << OLX_PATH_DEL;
  return Path;
}
//..............................................................................
olxstr TEFile::TrimPathDelimeter(const olxstr &Path)  {
  if( Path.IsEmpty() )  return Path;
  olxstr T = OLX_OS_PATH(Path);
  return (T[T.Length()-1] == OLX_PATH_DEL ) ? T.SubStringTo(T.Length()-1) : T;
}
//..............................................................................
olxstr& TEFile::TrimPathDelimeterI(olxstr &Path)  {
  if( Path.IsEmpty() )  return Path;
  OLX_OS_PATHI(Path);
  if( Path[Path.Length()-1] == OLX_PATH_DEL )
    Path.SetLength(Path.Length()-1);
  return Path;
}
//..............................................................................
bool TEFile::IsAbsolutePath(const olxstr& Path)  {
  if( Path.Length() < 1 )  return false;
#ifdef __WIN32__
  if( Path.Length() < 2 )  return false;
  if( Path[1] == ':' )  return true;
  if( Path.Length() < 2 )  return false;
  if( Path[0] == '\\' && Path[1] == '\\' )  return true;
  return false;
#else
  return (Path[0] == '/') ? true : false;
#endif
}
//..............................................................................
bool TEFile::IsSameFolder(const olxstr& _f1, const olxstr& _f2)  {
  olxstr f1 = AddPathDelimeter(_f1);
  olxstr f2 = AddPathDelimeter(_f2);
  bool e1 = Exists(f1),
       e2 = Exists(f2);
  // if one or both of the folders does not exist - we cannot test their equality...
  if( e1 != e2 || !e1 )  return false;
  if( f1 == f2 )  return true;
  static olxstr dn("_OLX_TEST_SAME_DIR.TMP");
  f1 << dn;
  f2 << dn;
  if( makedir(OLXSTR(f1)) != -1 )  {
    bool res = TEFile::Exists(f2);
    rmdir(OLXSTR(f1));
    return res;
  }
  return false;
}
//..............................................................................
bool TEFile::IsSubFolder(const olxstr& _f1, const olxstr& _f2)  {
  olxstr f1 = OLX_OS_PATH(_f1);
  olxstr f2 = OLX_OS_PATH(_f2);
  return f2.IsSubStringAt(f2, 0);
}
//..............................................................................
olxstr TEFile::UNCFileName(const olxstr &LocalFN)  {
#ifdef __WIN32__  //this function does not work on winxp anymore ...
//  char buffer[512];
//  memset( buffer, 0, 512);
//  if(  ScUNCFromLocalPath(LocalFN.c_str(), buffer, 512) == S_OK )
//    return olxstr(buffer);
  return LocalFN;
#else
  return LocalFN;
#endif
}
//..............................................................................
void TEFile::CheckFileExists(const olxstr& location, const olxstr& fileName)  {
  if( !Exists(fileName) )
    throw TFileDoesNotExistException(location, fileName);
}
//..............................................................................
TEFile* TEFile::TmpFile(const olxstr& templ)  {
  if( templ.IsEmpty() )  {
#ifdef __WIN32__
  olxch TmpFN[MAX_PATH];
  GetTempPath(MAX_PATH, TmpFN);
  olxstr Tmp(TmpFN), fn_mask("olex2");
  GetTempFileName(Tmp.u_str(), fn_mask.u_str(), 0, TmpFN);
  TEFile* rv = new TEFile(TmpFN, "w+b");
  rv->Temporary = true;
  return rv;
#else
    return new TEFile(EmptyString(), tmpfile());
#endif
  }
  else  
    throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
bool TEFile::Rename(const olxstr& from, const olxstr& to, bool overwrite)  {
  if( !Exists(from) )  return false;
  if( Exists(to) )  {
    if( !overwrite )  
      return false;
    if( !IsDir(to) )
      DelFile(to);
    else
      return false;
  }
  return rename( OLXSTR(from), OLXSTR(to) ) != -1;
}
//..............................................................................
bool TEFile::Copy(const olxstr& From, const olxstr& To, bool overwrite)  {
  const bool exists = Exists(To);
  if( exists && !overwrite )
    return false;
  try  {
  // need to check that the files are not the same though...
    struct STAT_STR the_stat;
    olxstr from = OLX_OS_PATH(From);
    olxstr to = OLX_OS_PATH(To);
    if( STAT(OLXSTR(from), &the_stat) != 0 )
      return false;
#ifdef __WIN32__ // to make the copying a transaction...
    // some installer user observations that copyign still fails in some cases...
    if( exists && !TEFile::DelFile(To) )
      return false;
    if( WinCopyFile(From.u_str(), To.u_str(), FALSE) == FALSE )
      return false;
#else
    TEFile in(from, "rb");
    TEFile out(to, "w+b");
    out << in;
    out.Close();
    in.Close();
#endif
    chmod(OLXSTR(to), the_stat.st_mode);
    return SetFileTimes(to, the_stat.st_atime, the_stat.st_mtime);
  }
  catch(...)  {  return false;  }
}
//..............................................................................
olxstr TEFile::ExpandRelativePath(const olxstr& path, const olxstr& _base) {
  if( path.IsEmpty() )  return path;
  if( !(path.StartsFrom('.') || path.StartsFrom("..")) )
    return OLX_OS_PATH(path);
  olxstr base = OLX_OS_PATH(_base);
  if( base.IsEmpty() )
    base = TBasicApp::GetBaseDir();
  TStrList baseToks(base, OLX_PATH_DEL),
           pathToks(OLX_OS_PATH(path), OLX_PATH_DEL);
  for( size_t i=0; i < pathToks.Count(); i++ )  {
    if( pathToks[i] == ".." )
      baseToks.Delete(baseToks.Count()-1);
    else if( pathToks[i] == '.' )
      ;
    else
      baseToks.Add(pathToks[i]);
  }
  olxstr res = baseToks.Text(OLX_PATH_DEL);
// make sure absolute paths stay absolute...
  if( base.StartsFrom(OLX_PATH_DEL) )
    res = olxstr(OLX_PATH_DEL) << res;
  return res;
}
//..............................................................................
olxstr TEFile::CreateRelativePath(const olxstr& path, const olxstr& _base)  {
  if( path.IsEmpty() )  return path;
  olxstr base = OLX_OS_PATH(_base);
  if( base.IsEmpty() )
    base = TBasicApp::GetBaseDir();
  TStrList baseToks(base, OLX_PATH_DEL),
           pathToks(OLX_OS_PATH(path), OLX_PATH_DEL);
  size_t match_count=0, max_cnt = olx_min(pathToks.Count(), baseToks.Count());
  while(baseToks[match_count] == pathToks[match_count] && ++match_count < max_cnt )  continue;
  if( match_count == 0 )  return OLX_OS_PATH(path);
  olxstr rv;
  for( size_t i=match_count; i < baseToks.Count(); i++ )  {
    rv << "..";
    if( (i+1) < baseToks.Count() )
      rv  << OLX_PATH_DEL;
  }
  if( rv.IsEmpty() )
    rv << '.';
  for( size_t i=match_count; i < pathToks.Count(); i++ )
    rv << OLX_PATH_DEL << pathToks[i];
  return rv;
}
//..............................................................................
olxstr TEFile::Which(const olxstr& filename)  {
  if( TEFile::IsAbsolutePath(filename) )
    return filename;
  olxstr fn = TEFile::CurrentDir();
  TEFile::AddPathDelimeterI(fn) << filename;
  // check current folder
  if( Exists(fn) )  return fn;
  // check program folder
  fn = TBasicApp::GetBaseDir();
  fn << filename;
  if( Exists(fn) )  return fn;
  // check path then ...
  olxstr path = olx_getenv("PATH");
  if( path.IsEmpty() )  return path;
  TStrList toks(path, OLX_ENVI_PATH_DEL);
  for( size_t i=0; i < toks.Count(); i++ )  {
    TEFile::AddPathDelimeterI(toks[i]) << filename;
    if( Exists(toks[i]) )
      return toks[i];
  }
  return EmptyString();
}
//..............................................................................

////////////////////////////////////////////////////////////////////////////////
////////////////////EXTERNAL LIBRRAY FUNCTIONS//////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void FileExists(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::Exists(Params[0]));
}

void FileName(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::ExtractFileName(Params[0]));
}

void FilePath(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::ExtractFilePath(Params[0]));
}

void FileDrive(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::ExtractFileDrive(Params[0]));
}

void FileExt(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::ExtractFileExt(Params[0]));
}

void ChangeFileExt(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::ChangeFileExt(Params[0], Params[1]));
}

void Copy(const TStrObjList& Params, TMacroError& E)  {
  TEFile::Copy(Params[0], Params[1]);
  E.SetRetVal(Params[1]);
}

void Rename(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::Rename(Params[0], Params[1]));
}

void Delete(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::DelFile(Params[0]));
}

void CurDir(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::CurrentDir());
}

void ChDir(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::ChangeDir(Params[0]));
}

void MkDir(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::MakeDir(Params[0]));
}

void OSPath(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::OSPath(Params[0]));
}

void Which(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::Which(Params[0]));
}

void Age(const TStrObjList& Params, TMacroError& E)  {
  TEFile::CheckFileExists(__OlxSourceInfo, Params[0]);
  time_t v = TEFile::FileAge(Params[0]);
  if( Params.Count() == 1 )
    E.SetRetVal(TETime::FormatDateTime(v));
  else
    E.SetRetVal(TETime::FormatDateTime(Params[1], v));
}

void ListDirForGUI(const TStrObjList& Params, TMacroError& E)  {
  TEFile::CheckFileExists(__OlxSourceInfo, Params[0]);
  olxstr cd( TEFile::CurrentDir() );
  olxstr dn(Params[0]);
  TEFile::AddPathDelimeterI(dn);
  TEFile::ChangeDir( Params[0] );
  short attrib = sefFile;
  if( Params.Count() == 3 )  {
    if( Params[2].Equalsi("fd") )
      attrib |= sefDir;
    else if( Params[2].CharAt(0) == 'd' )
      attrib = sefDir;
  }
  TStrList output;
  olxstr tmp;
  TEFile::ListCurrentDir(output, Params[1], attrib);
  TEFile::ChangeDir(cd);
#ifdef __BORLANDC__
  output.QuickSort< TStringWrapperComparator<TSingleStringWrapper<olxstr>,true> >();
#else  // borland dies here...
  output.QSort(false);
#endif
  for( size_t i=0; i < output.Count(); i++ )  {
   tmp.SetLength(0);
    tmp <<  "<-" << dn << output[i];
    output[i] << tmp;
  }
  E.SetRetVal( output.Text(';') );
}

void CreateRelativePath(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::CreateRelativePath(Params[0], Params.Count() == 2 ? Params[1] : EmptyString()));
}

void ExpandRelativePath(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TEFile::ExpandRelativePath(Params[0], Params.Count() == 2 ? Params[1] : EmptyString()));
}

TLibrary*  TEFile::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("file") : name);
  lib->RegisterStaticFunction( new TStaticFunction( ::FileExists, "Exists", fpOne,
"Returns true if specified file exists") );
  lib->RegisterStaticFunction( new TStaticFunction( ::FileName, "GetName", fpOne,
"Returns name part of the full/partial file name") );
  lib->RegisterStaticFunction( new TStaticFunction( ::FilePath, "GetPath", fpOne,
"Returns path component of the full file name") );
  lib->RegisterStaticFunction( new TStaticFunction( ::FileDrive, "GetDrive", fpOne,
"Returns drive component of the full file name") );
  lib->RegisterStaticFunction( new TStaticFunction( ::FileExt, "GetExt", fpOne,
"Returns file extension") );
  lib->RegisterStaticFunction( new TStaticFunction( ::ChangeFileExt, "ChangeExt", fpTwo,
"Returns file name with changed extension") );
  lib->RegisterStaticFunction( new TStaticFunction( ::Copy, "Copy", fpTwo,
"Copies file provided as first argument into the file provided as second argument") );
  lib->RegisterStaticFunction( new TStaticFunction( ::Delete, "Delete", fpOne,
"Deletes specified file") );
  lib->RegisterStaticFunction( new TStaticFunction( ::Rename, "Rename", fpTwo,
"Renames specified file") );
  lib->RegisterStaticFunction( new TStaticFunction( ::CurDir, "CurDir", fpNone,
"Returns current folder") );
  lib->RegisterStaticFunction( new TStaticFunction( ::ChDir, "ChDir", fpOne,
"Changes current folder to provided folder") );
  lib->RegisterStaticFunction( new TStaticFunction( ::MkDir, "MkDir", fpOne,
"Creates specified folder") );
  lib->RegisterStaticFunction( new TStaticFunction( ::OSPath, "OSPath", fpOne,
"Returns OS specific path for provided path") );
  lib->RegisterStaticFunction( new TStaticFunction( ::Which, "Which", fpOne,
"Tries to find a particular file looking at current folder, PATH and program folder") );
  lib->RegisterStaticFunction( new TStaticFunction( ::Age, "Age", fpOne|fpTwo,
"Returns file age for provided file using formatting string (if provided)") );
  lib->RegisterStaticFunction( new TStaticFunction( ::ListDirForGUI, "ListDirForGUI", fpTwo|fpThree,
"Returns a ready to use in GUI list of files, matching provided mask(s) separated by semicolon.\
 The third, optional argument [f,d,fd] specifies what should be included into the list") );
  lib->RegisterStaticFunction( new TStaticFunction( ::CreateRelativePath, "RelativePath", fpOne|fpTwo,
"Returns a path to a folder relative to basedir; arguments are (base=basedir,path)") );
  lib->RegisterStaticFunction( new TStaticFunction( ::ExpandRelativePath, "AbsolutePath", fpOne|fpTwo,
"Returns an absolute path to a folder relative to the basedir; arguments are (base=basedir,path)") );
  return lib;
}