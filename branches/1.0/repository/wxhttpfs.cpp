#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "wxhttpfs.h"
#include "actions.h"
#include "bapp.h"
#include "efile.h"
#include "ememstream.h"

TwxHttpFileSystem::~TwxHttpFileSystem()  {  
  if( ZipFS != NULL )  delete ZipFS;
}
//.........................................................................................
IInputStream* TwxHttpFileSystem::OpenFile(const olxstr& Source)  {
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
    is = Http.GetInputStream( src.u_str() );
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
//.........................................................................................
wxInputStream* TwxHttpFileSystem::wxOpenFile(const olxstr& Source)  {
  olxstr o_src(TEFile::UnixPath(Source));
  wxInputStream* is = NULL;
  try  {
    olxstr src;
    if( Url.HasProxy() )
      src << Url.GetFullHost();
    src << o_src;
    src.Replace(' ', "%20");
    is = Http.GetInputStream( src.u_str() );
  }
  catch( ... )  {   return NULL;  }
  if( is == NULL )  {
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("NULL handle for '") << o_src << '\'');
    return NULL;
  }
  return is;  
}
//.........................................................................................
TEFile* TwxHttpFileSystem::SaveFile(const olxstr& Source)  {
  TOnProgress Progress;
  Progress.SetMax(1);
  Progress.SetPos(0);
  Progress.SetAction(olxstr("Downloading ") << Source );
  TBasicApp::GetInstance()->OnProgress->Enter(this, &Progress);
  olxstr o_src(TEFile::UnixPath(Source));
  wxInputStream* is = NULL;
  try  {
    olxstr src;
    if( Url.HasProxy() )
      src << Url.GetFullHost();
    src << o_src;
    src.Replace(' ', "%20");
    is = Http.GetInputStream( src.u_str() );
    if( is != NULL )  {
      Progress.SetMax( is->GetLength() );
      TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
    }
  }
  catch( ... )  {   return NULL;  }
  if( is == NULL )  {
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("NULL handle for '") << o_src << '\'');
  }
  TEFile* tf = TEFile::TmpFile(EmptyString);
  char* bf = new char [1024*64];
  is->Read(bf, 1024*64);
  while( is->LastRead() != 0 )  {
    tf->Write(bf, is->LastRead());
    Progress.SetPos( tf->GetPosition() );
    TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
    is->Read(bf, 1024*64);
  }
  Progress.SetAction("Download complete");
  Progress.SetPos( 0 );
  TBasicApp::GetInstance()->OnProgress->Exit(this, &Progress);

  delete [] bf;
  tf->Seek(0, 0);
  return tf;  
}
//.........................................................................................

