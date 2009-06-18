//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

//#include "smart/ostring.h"
#include "ebase.h"
#include <string.h>
#include <sys/stat.h>
#include "efile.h"
#ifdef __WIN32__
#include <windows.h>
#endif
#include "filetree.h"

#ifdef __WIN32__

  #include <malloc.h>
  #include <io.h>
  #include <direct.h>
  #define OLXSTR(A) (A).u_str()
  #ifdef _UNICODE
    #define UTIME _wutime
    #define unlink _wunlink
    #define rmdir _wrmdir
    #define STAT _wstat
    #define STAT_STR _stat
    #define chmod _wchmod
    #define chdir _wchdir
    #define access _waccess
    #define getcwd _wgetcwd
    #define makedir _wmkdir
    #define fopen _wfopen
    #define rename _wrename
  #else
    #define STAT stat
    #define STAT_STR stat
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
  #include <windows.h>
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

  #define OLXSTR(A) (A).c_str()

  #define makedir(a) mkdir((a), 0755)
  #define UTIME utime
  #define STAT stat
  #define STAT_STR stat

  #define OLX_PATH_DEL '/'
  #define OLX_ENVI_PATH_DEL ':'
  #define OLX_OS_PATH(A)  TEFile::UnixPath( (A) )
  #define OLX_OS_PATHI(A) TEFile::UnixPathI( (A) )
  #define UTIMBUF utimbuf
#endif


#include "exception.h"
#include "estrlist.h"

#include "library.h"
#include "bapp.h"
#include "log.h"

#ifndef MAX_PATH
  #define MAX_PATH 1024
#endif

UseEsdlNamespace()

const olxstr TEFile::AllFilesMask("*.*");
//----------------------------------------------------------------------------//
// TFileNameMask function bodies
//----------------------------------------------------------------------------//
void TEFile::TFileNameMask::Build(const olxstr& msk )  {
  mask = olxstr::LowerCase(msk);
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
bool TEFile::TFileNameMask::DoesMatch(const olxstr& _str)  const {
  if( mask.IsEmpty() && !_str.IsEmpty() )  return false;
  // this will work for '*' mask
  if( toks.Count() == 0 )  return true;
  // need to check if the mask starts from a '*' or ends with it
  olxstr str = olxstr::LowerCase(_str);
  int off = 0, start = 0, end = str.Length();
  if( mask[0] != '*' )  {
    olxstr& tmp = toks[0];
    if( tmp.Length() > str.Length() )  return false;
    for( int i=0; i < tmp.Length(); i++ )
     if( !(tmp[i] == '?' || tmp[i] == str[i]) )  return false;
    start = tmp.Length();
    if( toks.Count() == 1 && mask[ mask.Length()-1] != '*' )  return true;
  }
  if( mask[ mask.Length()-1] != '*' && toks.Count() > (mask[0]!='*' ? 1 : 0) )  {
    olxstr& tmp = toks[toks.Count()-1];
    if( tmp.Length() > (str.Length()-start) )  return false;
    for( int i=0; i < tmp.Length(); i++ )
     if( !(tmp[i] == '?' || tmp[i] == str[str.Length()-tmp.Length() + i]) )  return false;
    end = str.Length() - tmp.Length();

    if( toks.Count() == 1 )  return true;
  }

  for( int i=toksStart; i < toksEnd; i++ )  {
    olxstr& tmp = toks[i];
    bool found = false;
    for( int j=start; j < end; j++ )  {
      if( (str.Length() - j) < tmp.Length() )  return false;
      if( tmp[off] == '?' || str[j] == tmp[off] )  {
        while( tmp[off] == '?' || tmp[off] == str[j+off] )  {
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
}
//..............................................................................
TEFile::TEFile(const olxstr &F, const olxstr &Attribs)  {
  FHandle = NULL;
  Open(F, Attribs);
}
//..............................................................................
TEFile::TEFile(const olxstr& F, short Attribs)  {
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
    FName = EmptyString;
    throw TFileExceptionBase(__OlxSourceInfo, F, olxstr("NULL handle for '") << F << '\'');
  }
  return true;
}
//..............................................................................
bool TEFile::Close()  {
  if( FHandle != NULL )  {
    if( fclose(FHandle) != 0 )
      throw TFileExceptionBase(__OlxSourceInfo, FName, "fclose failed");
    FHandle = NULL;
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
    throw TFileExceptionBase(__OlxSourceInfo, EmptyString, "Invalid file handle");
}
//..............................................................................
void TEFile::Read(void *Bf, size_t count)  {
  CheckHandle();
  if( count == 0 )  return;
  int res = fread(Bf, count, 1, FHandle);
  if( res != 1 )
    throw TFileExceptionBase(__OlxSourceInfo, FName, "fread failed" );
}
//..............................................................................
void TEFile::SetPosition(size_t p)  {
  CheckHandle();
  if( fseek(FHandle, p, SEEK_SET) != 0 )  
    throw TFileExceptionBase(__OlxSourceInfo, FName, "fseek failed" );
}
//..............................................................................
size_t TEFile::GetPosition() const  {
  CheckHandle();
  long v = ftell(FHandle);
  if( v == -1 )
    throw TFileExceptionBase(__OlxSourceInfo, FName, "ftell failed" );
  return v;
}
//..............................................................................
long TEFile::Length() const  {
  CheckHandle();
  size_t currentPos = GetPosition();
  if( fseek( FHandle, 0, SEEK_END) )  
    throw TFileExceptionBase(__OlxSourceInfo, FName, "fseek failed");
  long length = ftell( FHandle );
  if( length == -1 )
    throw TFileExceptionBase(__OlxSourceInfo, FName, "ftell failed" );
  fseek( FHandle, currentPos, SEEK_SET);
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
    throw TFileExceptionBase(__OlxSourceInfo, FName, "fwrite failed" );
  return res;
}
//..............................................................................
void TEFile::Seek( long Position, const int From)  {
  CheckHandle();
  if( fseek(FHandle, Position, From) != 0 )
    throw TFileExceptionBase(__OlxSourceInfo, FName, "fseek failed");
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
  res = TEFile::AddTrailingBackslashI(path) << files[0];
  return true;
#endif
}
//..............................................................................
bool TEFile::Access(const olxstr& F, const short Flags)  {
  return (access(OLXSTR(OLX_OS_PATH(F)), Flags) != -1);
}
//..............................................................................
olxstr TEFile::ExtractFilePath(const olxstr &F)  {
  olxstr fn = OLX_OS_PATH(F);
  if( TEFile::IsAbsolutePath( F ) )  {
    int i = fn.LastIndexOf( OLX_PATH_DEL );
    if( i > 0 ) return fn.SubStringTo(i+1);
    return EmptyString;
  }
  return EmptyString;
}
//..............................................................................
olxstr TEFile::ParentDir(const olxstr& F) {
  if( F.IsEmpty() )  return F;
  int start = F[F.Length()-1] == OLX_PATH_DEL ? F.Length()-2 : F.Length()-1;
  olxstr fn = OLX_OS_PATH(F);
  int i = fn.LastIndexOf(OLX_PATH_DEL, start);
  if( i > 0 ) return fn.SubStringTo(i+1);
  return EmptyString;
}
//..............................................................................
olxstr TEFile::ExtractFileExt(const olxstr& F)  {
  int i=F.LastIndexOf('.');
  if( i > 0 )  return F.SubStringFrom(i+1);
  return EmptyString;
}
//..............................................................................
olxstr TEFile::ExtractFileName(const olxstr& F)  {
  olxstr fn = OLX_OS_PATH(F);
  int i=fn.LastIndexOf( OLX_PATH_DEL );
  if( i > 0 )  return fn.SubStringFrom(i+1);
  return F;
}
//..............................................................................
olxstr TEFile::ExtractFileDrive(const olxstr& F)  {
#ifdef __WIN32__
  if( F.Length() < 2 )  return EmptyString;
  if( F[1] != ':' )  return EmptyString;
  return F.SubString(0, 2);
#else
  return EmptyString;
#endif
}
//..............................................................................
olxstr TEFile::ChangeFileExt(const olxstr &F, const olxstr &Ext)  {
  if( F.IsEmpty() )  return F;
  olxstr N(F);

  int i=N.LastIndexOf('.');
  if( i > 0 )  N.SetLength(i);
  else  {
    if( N[N.Length()-1] == '.' )
      N.SetLength(N.Length()-1);
  }
  if( Ext.Length() )  {
    if( Ext[0] != '.' )  N << '.';
    N << Ext;
  }
  return N;
}
//..............................................................................
bool TEFile::DelFile(const olxstr& F)  {
  if( !Exists(F) )  return true;
  olxstr fn = OLX_OS_PATH(F);
  if( chmod(OLXSTR(fn), S_IWRITE) == 0 )
    return (unlink(OLXSTR(fn)) == -1) ? false: true;
  return false;
}
//..............................................................................
bool TEFile::RmDir(const olxstr& F)  {
  if( !Exists(F) )  return true;
  if( chmod(OLXSTR(F), S_IWRITE) == 0 )
    return (rmdir(OLXSTR(OLX_OS_PATH(F))) == -1) ?  false : true;
  return false;
}
//..............................................................................
bool TEFile::IsDir(const olxstr& F)  {
  struct STAT_STR the_stat;
  olxstr fn = OLX_OS_PATH(F);
  if( fn.EndsWith(OLX_PATH_DEL) )
    fn = fn.SubStringFrom(0, 1);
  if( STAT(OLXSTR(fn), &the_stat) != 0 )
    return false;
  return (the_stat.st_mode & S_IFDIR) != 0;
}
//..............................................................................
bool TEFile::DeleteDir(const olxstr& F)  {
  olxstr fn = OLX_OS_PATH(F);
  if( !Exists(fn) )  
    return false;
  if( !TEFile::IsDir(fn) )  
    return false;
  TFileTree ft(F);
  try  {
    ft.Root.Expand();
    ft.Root.Delete();
    return true;
  }
  catch( TExceptionBase& )  {    return false;  }
}
//..............................................................................
bool TEFile::DoesMatchMasks(const olxstr& _fn, const MaskList& masks)  {
  olxstr ext = TEFile::ExtractFileExt( _fn );
  olxstr fn = _fn.SubStringTo(_fn.Length() - ext.Length() - (ext.IsEmpty() ? 0 : 1));
  for( int i=0; i < masks.Count(); i++ )
    if( masks[i].ExtMask.DoesMatch(ext) && masks[i].NameMask.DoesMatch(fn) )
      return true;
  return false;
}
//..............................................................................
void TEFile::BuildMaskList(const olxstr& mask, MaskList& masks)  {
  TStrList ml(mask, ';');
  for( int i=0; i < ml.Count(); i++ )
    masks.AddNew(ml[i]);
}
//..............................................................................
#ifdef __WIN32__
bool TEFile::ListCurrentDirEx(TFileList &Out, const olxstr &Mask, const unsigned short sF)  {
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
bool TEFile::ListCurrentDir(TStrList& Out, const olxstr &Mask, const unsigned short sF)  {
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
    if( DoesMatchMasks(sd.cFileName, masks) )
      Out.Add( sd.cFileName );
    done = (FindNextFile(hn, &sd) != 0);
  }
  FindClose(hn);
  return true;
}
#else
//..............................................................................
bool TEFile::ListCurrentDirEx(TFileList &Out, const olxstr &Mask, const unsigned short sF)  {
  DIR *d = opendir( TEFile::CurrentDir().c_str() );
  if( d == NULL ) return false;
  MaskList masks;
  BuildMaskList(Mask, masks);
  int access = 0, faccess;
  unsigned short attrib;
  if( (sF & sefReadOnly) != 0 )  access |= S_IRUSR;
  if( (sF & sefWriteOnly) != 0 ) access |= S_IWUSR;
  if( (sF & sefExecute) != 0 )   access |= S_IXUSR;

  struct stat the_stat;
  struct dirent* de;
  while((de = readdir(d)) != NULL) {
    stat(de->d_name, &the_stat);
    if( sF != sefAll )  {
      faccess = 0;
      if( (the_stat.st_mode & S_IRUSR) != 0 )  faccess |= S_IRUSR;
      if( (the_stat.st_mode & S_IWUSR) != 0 )  faccess |= S_IWUSR;
      if( (the_stat.st_mode & S_IXUSR) != 0 )  faccess |= S_IXUSR;
      if( (faccess & access) != access )  continue;

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
      attrib = 0;
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
bool TEFile::ListCurrentDir(TStrList &Out, const olxstr &Mask, const unsigned short sF)  {
  DIR *d = opendir( TEFile::CurrentDir().c_str() );
  if( d == NULL ) return false;
  MaskList masks;
  BuildMaskList(Mask, masks);
  olxstr tmp, fn;
  int access = 0, faccess;

  if( (sF & sefReadOnly) != 0 )  access |= S_IRUSR;
  if( (sF & sefWriteOnly) != 0 ) access |= S_IWUSR;
  if( (sF & sefExecute) != 0 )   access |= S_IXUSR;

  struct stat the_stat;
  struct dirent* de;
  while((de = readdir(d)) != NULL) {
    if( sF != sefAll )  {
      stat(de->d_name, &the_stat);
      faccess = 0;
      if( (the_stat.st_mode & S_IRUSR) != 0 )  faccess |= S_IRUSR;
      if( (the_stat.st_mode & S_IWUSR) != 0 )  faccess |= S_IWUSR;
      if( (the_stat.st_mode & S_IXUSR) != 0 )  faccess |= S_IXUSR;
      if( (faccess & access) != access )  continue;

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
bool TEFile::ListDirEx(const olxstr& dir, TFileList &Out, const olxstr &Mask, const unsigned short sF)  {
  olxstr cd( TEFile::CurrentDir() );
  TEFile::ChangeDir(dir);
  bool res = ListCurrentDirEx(Out, Mask, sF);
  TEFile::ChangeDir(cd);
  return res;
}
//..............................................................................
bool TEFile::ListDir(const olxstr& dir, TStrList &Out, const olxstr &Mask, const unsigned short sF)  {
  olxstr cd( TEFile::CurrentDir() );
  TEFile::ChangeDir(dir);
  bool res = ListCurrentDir(Out, Mask, sF);
  TEFile::ChangeDir(cd);
  return res;
}
//..............................................................................
bool TEFile::SetFileTimes(const olxstr& fileName, uint64_t AccTime, uint64_t ModTime)  {
  struct UTIMBUF tb;
  tb.actime = AccTime;
  tb.modtime = ModTime;
  return UTIME(OLXSTR(fileName), &tb) == 0 ? true : false;
}
//..............................................................................
// thanx to Luc - I have completely forgotten about stat!
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
  return ( chdir(path.c_str()) == -1 ) ?  false : true;
#endif
}
//..............................................................................
olxstr TEFile::CurrentDir()  {
#ifdef __WIN32__
  olxch bf[MAX_PATH];
  if( GetCurrentDirectory(MAX_PATH, bf) == 0 )
    return EmptyString;
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
  TStrList toks(OLX_OS_PATH(Name), OLX_PATH_DEL);
  olxstr toCreate;
  toCreate.SetCapacity( Name.Length() + 5 );
  for( int i=0; i < toks.Count(); i++ )  {
    toCreate << toks[i] << OLX_PATH_DEL;
    if( !Exists( toCreate ) )
      if( !MakeDir( toCreate ) )
        return false;
  }
  return true;
}
//..............................................................................
bool TEFile::MakeDir(const olxstr& Name)  {
  return ( makedir(OLXSTR(OLX_OS_PATH(Name))) == -1 ) ? false : true;
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
  olxstr T(F);
  T.Replace('/', '\\');
  return T;
}
//..............................................................................
olxstr TEFile::UnixPath(const olxstr& F) {
  olxstr T(F);
  T.Replace('\\', '/');
  return T;
}
//..............................................................................
olxstr& TEFile::WinPathI( olxstr& F )  {
  F.Replace('/', '\\');
  return F;
}
//..............................................................................
olxstr& TEFile::UnixPathI( olxstr& F )  {
  F.Replace('\\', '/');
  return F;
}
//..............................................................................
olxstr TEFile::AddTrailingBackslash( const olxstr& Path )  {
  if( Path.IsEmpty() )  
    return olxstr(OLX_PATH_DEL);
  olxstr T = OLX_OS_PATH(Path);
  if( T[T.Length()-1] != OLX_PATH_DEL )  
    T << OLX_PATH_DEL;
  return T;
}
//..............................................................................
olxstr& TEFile::AddTrailingBackslashI(olxstr &Path)  {
  if( Path.IsEmpty() )  return Path;
  OLX_OS_PATHI(Path);
  if( Path[Path.Length()-1] != OLX_PATH_DEL )  
    Path << OLX_PATH_DEL;
  return Path;
}
//..............................................................................
olxstr TEFile::RemoveTrailingBackslash(const olxstr &Path)  {
  if( Path.IsEmpty() )  return Path;
  olxstr T = OLX_OS_PATH(Path);
  return (T[T.Length()-1] == OLX_PATH_DEL ) ? T.SubStringTo(T.Length()-1) : T;
}
//..............................................................................
olxstr& TEFile::RemoveTrailingBackslashI(olxstr &Path)  {
  if( Path.IsEmpty() )  return Path;
  OLX_OS_PATHI(Path);
  if( Path[Path.Length()-1] == OLX_PATH_DEL )
    Path.SetLength(Path.Length()-1);
  return Path;
}
//..............................................................................
bool TEFile::IsAbsolutePath(const olxstr& Path)  {
  if( Path.Length() < 2 )  return false;
#ifdef __WIN32__
  if( Path[1] == ':' )  return true;
  if( Path[0] == '\\' && Path[1] == '\\' )  return true;
  return false;
#else
  return (Path[0] == '/') ? true : false;
#endif
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
    return new TEFile(EmptyString, tmpfile());
  }
  else  
    throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
bool TEFile::Rename(const olxstr& from, const olxstr& to, bool overwrite)  {
  if( Exists(to) && ! overwrite )  return false;
  return rename( OLXSTR(from), OLXSTR(to) ) != -1;
}
//..............................................................................
void TEFile::Copy(const olxstr& From, const olxstr& To, bool overwrite )  {
  if( Exists(To) && !overwrite )  return;
  // need to check that the files are not the same though...
  TEFile in( From, "rb" );
  TEFile out( To, "w+b" );
  out << in;
}
//..............................................................................
olxstr TEFile::AbsolutePathTo(const olxstr &Path, const olxstr &relPath ) {
  TStrList dirToks(OLX_OS_PATH( Path ), OLX_PATH_DEL),
              relPathToks(OLX_OS_PATH( relPath ), OLX_PATH_DEL);
  for( int i=0; i < relPathToks.Count(); i++ )  {
    if( relPathToks[i] == ".." )
      dirToks.Delete( dirToks.Count()-1 );
    else if( relPathToks[i] == "." )
      ;
    else
      dirToks.Add( relPathToks[i] );
  }
  olxstr res = dirToks.Text(OLX_PATH_DEL);
//  if( !TEFile::FileExists( res ) )
//    throw TFileDoesNotExistException(__OlxSourceInfo, res);
  return res;
}
//..............................................................................
olxstr TEFile::Which(const olxstr& filename)  {
  if( TEFile::IsAbsolutePath(filename) )
    return filename;
  olxstr fn = TEFile::CurrentDir();
  TEFile::AddTrailingBackslashI(fn) << filename;
  // check current folder
  if( Exists(fn) )  return fn;
  // check program folder
  fn = TBasicApp::GetInstance()->BaseDir();
  fn << filename;
  if( Exists(fn) )  return fn;
  // check path then ...
  char* path = getenv("PATH");
  if( path == NULL )  return EmptyString;
  TStrList toks(path, OLX_ENVI_PATH_DEL);
  for( int i=0; i < toks.Count(); i++ )  {
    TEFile::AddTrailingBackslashI(toks[i]) << filename;
    if( Exists(toks[i]) )
      return toks[i];
//    TBasicApp::GetLog() << toks[i] << '\n';
  }
  return EmptyString;
}
//..............................................................................

////////////////////////////////////////////////////////////////////////////////
////////////////////EXTERNAL LIBRRAY FUNCTIONS//////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void FileExists(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::Exists( Params[0] ) );
}

void FileName(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::ExtractFileName( Params[0] ) );
}

void FilePath(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::ExtractFilePath( Params[0] ) );
}

void FileDrive(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::ExtractFileDrive( Params[0] ) );
}

void FileExt(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::ExtractFileExt( Params[0] ) );
}

void ChangeFileExt(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::ChangeFileExt( Params[0], Params[1] ) );
}

void Copy(const TStrObjList& Params, TMacroError& E)  {
  TEFile::Copy( Params[0], Params[1] );
  E.SetRetVal( Params[1] );
}

void Rename(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::Rename( Params[0], Params[1] ) );
}

void Delete(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::DelFile(Params[0]) );
}

void CurDir(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::CurrentDir() );
}

void ChDir(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::ChangeDir(Params[0]) );
}

void MkDir(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::MakeDir(Params[0]) );
}

void OSPath(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::OSPath(Params[0]) );
}

void Which(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( TEFile::Which(Params[0]) );
}

void Age(const TStrObjList& Params, TMacroError& E)  {
  TEFile::CheckFileExists(__OlxSourceInfo, Params[0]);
  time_t v = TEFile::FileAge( Params[0] );
  if( Params.Count() == 1 )
    E.SetRetVal( TETime::FormatDateTime(v) );
  else
    E.SetRetVal( TETime::FormatDateTime(Params[1], v) );
}

void ListDirForGUI(const TStrObjList& Params, TMacroError& E)  {
  TEFile::CheckFileExists(__OlxSourceInfo, Params[0]);
  olxstr cd( TEFile::CurrentDir() );
  olxstr dn( Params[0] );
  TEFile::AddTrailingBackslashI(dn);
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
  TEFile::ListCurrentDir( output, Params[1], attrib );
  TEFile::ChangeDir( cd );
#ifdef __BORLANDC__
  output.QuickSort< TStringWrapperComparator<TSingleStringWrapper<olxstr>,true> >();
#else  // borland dies here...
  output.QSort(false);
#endif
  for(int i=0; i < output.Count(); i++ )  {
   tmp = EmptyString;
    tmp <<  "<-" << dn << output[i];
    output[i] << tmp;
  }
  E.SetRetVal( output.Text(';') );
}

TLibrary*  TEFile::ExportLibrary(const olxstr& name)
{
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
  lib->RegisterStaticFunction( new TStaticFunction( ::OSPath, "ospath", fpOne,
"Returns OS specific path for provided path") );
  lib->RegisterStaticFunction( new TStaticFunction( ::Which, "Which", fpOne,
"Tries to find a particular file looking at current folder, PATH and program folder") );
  lib->RegisterStaticFunction( new TStaticFunction( ::Age, "Age", fpOne|fpTwo,
"Returns file age for provided file using formatting string (if provided)") );
  lib->RegisterStaticFunction( new TStaticFunction( ::ListDirForGUI, "ListDirForGUI", fpTwo|fpThree,
"Returns a ready to use in GUI list of files, matching provided mask(s) separated by semicolon.\
 The third, optional argument [f,d,fd] specifies what should be included into the list") );
  return lib;
}

