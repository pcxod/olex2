//---------------------------------------------------------------------------
#ifndef wxFtpFSH
#define wxFtpFSH
#include "filesystem.h"
#include "url.h"
#include "wxzipfs.h"
#include "wx/protocol/ftp.h"
#include "ememstream.h"
//---------------------------------------------------------------------------
class TwxFtpFileSystem: public AFileSystem, public IEObject  {
  TUrl Url;
  wxFTP Ftp;
  TwxZipFileSystem* ZipFS;
  olxstr pwd;
public:
  TwxFtpFileSystem(const TUrl& url, const olxstr& userName, const olxstr& pswd, TwxZipFileSystem* zipFS=NULL) : Url(url) {
    Ftp.SetUser( userName.u_str() );
    Ftp.SetPassword( pswd.u_str() );
    Ftp.SetBinary();
    ZipFS = zipFS;
    if( !Ftp.Connect( (url.HasProxy() ? url.GetProxy().GetFullHost() : url.GetHost()).u_str() ) )  {
      throw TFunctionFailedException(__OlxSourceInfo, "connection failed");
    }
    SetBase( url.GetPath() );
    if( !url.GetPath().IsEmpty() )  {
      if( !FileExists(url.GetPath()) )
        NewDir(url.GetPath());
      ChangeDir(url.GetPath());
      pwd = url.GetPath();
    }
  }
  virtual ~TwxFtpFileSystem() {}
  // saves stream to a temprray file and returs the object which must be deleted manually
  TEFile* SaveFile(const olxstr& fn);
  // zip is as primary source of the files, if a file is not in the zip - Url is used
  void SetZipFS( TwxZipFileSystem* zipFS )  {
    if( ZipFS != NULL )  delete ZipFS;
      ZipFS = zipFS;
  }
  virtual IInputStream* OpenFile(const olxstr& Source)  {
    olxstr o_src(TEFile::UnixPath(Source));
    olxstr zip_name = Source.SubStringFrom( Url.GetPath().Length()+1 );
    if( ZipFS != NULL && ZipFS->FileExists(zip_name) != 0 )  {
      return ZipFS->OpenFile(zip_name);
    }
    TOnProgress Progress;
    Progress.SetMax(1);
    Progress.SetPos(0);
    Progress.SetAction(olxstr("Downloading ") << o_src );
    TBasicApp::GetInstance()->OnProgress->Enter(this, &Progress);
    wxInputStream* is = NULL;
    try  {
      olxstr src( Url.GetFullHost() );
      //if( Url.HasProxy() )  // 2008-10-13
      //  src << Url.GetFullHost();
      src << o_src;
      src.Replace(' ', "%20");
      is = wxOpenFile( src );
      if( is != NULL )  {
        Progress.SetMax( is->GetLength() );
        TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
      }
    }
    catch( ... )  {   return NULL;  }

    if( is == NULL )  {
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("NULL handle for '") << o_src << '\'');
      return NULL;
    }
    TEMemoryStream* ms = new TEMemoryStream();
    char* bf = new char[1024*64];
    try {
      is->Read(bf, 1024*64);
      while( is->LastRead() != 0 )  {
        ms->Write( bf, is->LastRead() );
        if( Progress.GetMax() > 0 )  {
          Progress.SetPos( ms->GetPosition() );
          TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
        }
        is->Read(bf, 1024*64);
      }
      ms->SetPosition(0);
      Progress.SetAction("Download complete");
      Progress.SetPos( 0 );
      TBasicApp::GetInstance()->OnProgress->Exit(this, &Progress);
    }
    catch(...)  {
      Progress.SetAction("Download failed");
      Progress.SetPos( 0 );
      TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
      TBasicApp::GetInstance()->OnProgress->Exit(this, &Progress);
    }
    delete is;
    delete [] bf;
    return ms;  
    
  }
  virtual wxInputStream* wxOpenFile(const olxstr& Source) {  return Ftp.GetInputStream(Source.u_str());  }
  virtual bool FileExists(const olxstr& DN)  {  return Ftp.FileExists(DN.u_str());  }

  virtual bool DelFile(const olxstr& FN)     {  return Ftp.RmFile(FN.u_str());    }
  virtual bool DelDir(const olxstr& DN)      {  return Ftp.RmDir(DN.u_str());     }
  virtual bool AdoptFile(const TFSItem& src){  
    olxstr rel_path( TEFile::UnixPath(src.GetFullName()) );
    olxstr fn( GetBase() + rel_path );
//    OnAdoptFile->Execute( this, &F__N);
    TOnProgress Progress;
    Progress.SetMax(1);
    Progress.SetPos(0);
    Progress.SetAction(olxstr("Uploading ") << rel_path );
    TBasicApp::GetInstance()->OnProgress->Enter(this, &Progress);

    IInputStream* is = NULL;
    try  {  is = src.GetFileSystem().OpenFile(src.GetFileSystem().GetBase() + rel_path );  }
    catch(const TExceptionBase& exc)  {
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    if( is == NULL )  return false;
    Progress.SetMax( is->GetSize() );
    try {
      //TStrList path_toks;
      int ind = rel_path.LastIndexOf('/');
      olxstr path = (ind != -1) ? rel_path.SubStringTo(ind) : EmptyString;
      olxstr fn( ind == 0-1 ? rel_path : rel_path.SubStringFrom(ind+1) );
      int depth = 0;
      if( !path.IsEmpty() && !FileExists(path) )  {
        if( !NewDir(path) )  
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Mkdir \'") << path << '\'');
      }

      wxOutputStream* out = Ftp.GetOutputStream( rel_path.u_str() );
      if( out == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "could not open output stream");
      const int bf_sz = 64*1024;
      char* bf = new char [bf_sz];
      size_t read = 0;
      try {
        while( (read = is->SafeRead(bf, bf_sz)) > 0 )  {
          out->Write(bf, read);
          Progress.SetPos( is->GetPosition() );
          TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
        }
        Progress.SetAction("Upload complete");
        Progress.SetPos( 0 );
        TBasicApp::GetInstance()->OnProgress->Exit(this, &Progress);
      }
      catch(...)  {
        Progress.SetAction("Upload failed");
        Progress.SetPos( 0 );
        TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
        TBasicApp::GetInstance()->OnProgress->Exit(this, &Progress);
      }
      delete out;
      delete [] bf;
      delete is;
    }
    catch( const TExceptionBase& exc )  {
      delete is;
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    return true;
  }
  virtual bool NewDir(const olxstr& DN)      {  
    olxstr norm_path( TEFile::UnixPath(DN) ),
           rel_path(norm_path);
    if( norm_path.StartsFrom(pwd) )  {  // absolute location
      rel_path = norm_path.SubStringFrom(pwd.Length());
      rel_path.Trim('/');
      if( rel_path.FirstIndexOf('/') != -1 )  {
        TStrList toks(rel_path, '/');
        for( int i=0; i < toks.Count(); i++ )  {
          if( !Ftp.FileExists( toks[i].u_str() ) )
            if( !Ftp.MkDir(toks[i].u_str()) )
              return false;
          if( !Ftp.ChDir(toks[i].u_str()) )
            return false;
        }
        for( int i=0; i < toks.Count(); i++ )  // restore the level
          if( !Ftp.ChDir(wxT("..")) )
            return false;
        return true;
      }
    }
    return Ftp.MkDir(rel_path.u_str());     
  }
  virtual bool ChangeDir(const olxstr& DN)   {  
    if( Ftp.ChDir( DN.u_str() ) )  {
      pwd = TEFile::UnixPath(Ftp.Pwd().wx_str());
      return true;
    }
    return false;
  }
};

#endif
