/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_wxFtpFS_H
#define __olx_wxFtpFS_H
#include "filesystem.h"
#include "url.h"
#include "wxzipfs.h"
#include "wx/protocol/ftp.h"
#include "ememstream.h"

class TwxFtpFileSystem : public AFileSystem  {
  TUrl Url;
  wxFTP Ftp;
  TwxZipFileSystem* ZipFS;
  olxstr pwd;
protected:
  olxstr NormaliseName(const olxstr& name) const {
    olxstr fn( TEFile::UnixPath(name) );
    fn = fn.StartsFrom(pwd) ? fn.SubStringFrom(pwd.Length()) : fn;
    return fn.Trim('/');
  }
  virtual IInputStream* _DoOpenFile(const olxstr& Source)  {
    olxstr o_src(NormaliseName(Source));
    olxstr zip_name = Source.SubStringFrom( Url.GetPath().Length()+1 );
    if( ZipFS != NULL && ZipFS->Exists(zip_name) != 0 )  {
      return ZipFS->OpenFile(zip_name);
    }
    TOnProgress Progress;
    Progress.SetMax(1);
    Progress.SetPos(0);
    Progress.SetAction(olxstr("Downloading ") << o_src );
    OnProgress.Enter(this, &Progress);
    wxInputStream* is = NULL;
    try  {
      olxstr src( o_src );
      src.Replace(' ', "%20");
      is = wxOpenFile( src );
      if( is != NULL )  {
        Progress.SetMax( is->GetLength() );
        OnProgress.Execute(this, &Progress);
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
          OnProgress.Execute(this, &Progress);
        }
        if( Break )  {
          delete is;
          delete [] bf;
          delete ms;
          return NULL;
        }
        is->Read(bf, 1024*64);
      }
      ms->SetPosition(0);
      Progress.SetAction("Download complete");
      Progress.SetPos( 0 );
      OnProgress.Exit(this, &Progress);
    }
    catch(...)  {
      Progress.SetAction("Download failed");
      Progress.SetPos( 0 );
      OnProgress.Execute(this, &Progress);
      OnProgress.Exit(this, &Progress);
    }
    delete is;
    delete [] bf;
    return ms;  
    
  }
  virtual bool _DoesExist(const olxstr& DN, bool)  {
    olxstr fn( NormaliseName(DN).u_str() );
    if( Index != NULL )
      return Index->GetRoot().FindByFullName(fn) != NULL;
    return Ftp.FileExists(fn.u_str());  
  }

  virtual bool _DoDelFile(const olxstr& FN)     {  // to dangerous
    return Ftp.RmFile(NormaliseName(FN).u_str());    
  }
  virtual bool _DoDelDir(const olxstr& DN)      {  // too dangerous
    return Ftp.RmDir(NormaliseName(DN).u_str());     
  }
  virtual bool _DoAdoptFile(const TFSItem& src){  
    IInputStream* is = NULL;
    try  {  
      is = src.GetIndexFS().OpenFile(src.GetIndexFS().GetBase() + src.GetFullName() );  
      if( is != NULL )  
        return AdoptStream(*is, TEFile::UnixPath(src.GetFullName()));
    }
    catch(const TExceptionBase& exc)  {
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    if( is == NULL )  return false;
    else
      delete is;
    return true;
  }
  virtual bool _DoAdoptStream(IInputStream& in, const olxstr& as) {  
    olxstr rel_path( NormaliseName(as) );
//    OnAdoptFile->Execute( this, &F__N);
    TOnProgress Progress;
    Progress.SetMax(1);
    Progress.SetPos(0);
    Progress.SetAction(olxstr("Uploading ") << rel_path );
    OnProgress.Enter(this, &Progress);

    Progress.SetMax( in.GetSize() );
    try {
      //TStrList path_toks;
      int ind = rel_path.LastIndexOf('/');
      olxstr path = (ind != -1) ? rel_path.SubStringTo(ind) : EmptyString();
      olxstr fn( ind == 0-1 ? rel_path : rel_path.SubStringFrom(ind+1) );
      int depth = 0;
      if( !path.IsEmpty() && !Ftp.FileExists(path.u_str()) )  {
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
        while( (read = in.SafeRead(bf, bf_sz)) > 0 )  {
          if( Break )  {
            delete [] bf;
            delete out;
            Ftp.RmFile( rel_path.u_str() );
            return false;
          }
          out->Write(bf, read);
          Progress.SetPos( in.GetPosition() );
          OnProgress.Execute(this, &Progress);
        }
        Progress.SetAction("Upload complete");
        Progress.SetPos( 0 );
        OnProgress.Exit(this, &Progress);
      }
      catch(...)  {
        Progress.SetAction("Upload failed");
        Progress.SetPos( 0 );
        OnProgress.Execute(this, &Progress);
        OnProgress.Exit(this, &Progress);
      }
      delete out;
      delete [] bf;
    }
    catch( const TExceptionBase& exc )  {
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    return true;
  }
  virtual bool _DoNewDir(const olxstr& DN)      {  
    olxstr norm_path( NormaliseName(DN) );
    if( norm_path.FirstIndexOf('/') != InvalidIndex )  {
      TStrList toks(norm_path, '/');
      for( size_t i=0; i < toks.Count(); i++ )  {
        if( !Ftp.FileExists( toks[i].u_str() ) )
          if( !Ftp.MkDir(toks[i].u_str()) )
            return false;
        if( !Ftp.ChDir(toks[i].u_str()) )
          return false;
      }
      for( size_t i=0; i < toks.Count(); i++ )  // restore the level
        if( !Ftp.ChDir(wxT("..")) )
          return false;
      return true;
    }
    return Ftp.MkDir(norm_path.u_str());     
  }
public:
  TwxFtpFileSystem(const TUrl& url, TwxZipFileSystem* zipFS=NULL) : Url(url) {
    Access = afs_FullAccess;
    Ftp.SetUser( url.GetUser().u_str() );
    Ftp.SetPassword( url.GetPassword().u_str() );
    Ftp.SetBinary();
    ZipFS = zipFS;
    if( !Ftp.Connect( (url.HasProxy() ? url.GetProxy().GetFullHost() : url.GetHost()).u_str() ) )  {
      throw TFunctionFailedException(__OlxSourceInfo, "connection failed");
    }
    SetBase( url.GetPath() );
    if( !url.GetPath().IsEmpty() )  {
      if( !Exists(url.GetPath()) )
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
  virtual wxInputStream* wxOpenFile(const olxstr& Source) {  return Ftp.GetInputStream(Source.u_str());  }
  bool ChangeDir(const olxstr& DN)   {  
    if( Ftp.ChDir( DN.u_str() ) )  {
      pwd = TEFile::UnixPath(Ftp.Pwd().wx_str());
      return true;
    }
    return false;
  }

};

#endif
