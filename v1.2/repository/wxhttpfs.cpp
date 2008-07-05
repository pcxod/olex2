#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "wxhttpfs.h"
#include "actions.h"
#include "bapp.h"
#include "efile.h"
#include "ememstream.h"

IInputStream* TwxHttpFileSystem::OpenFile(const olxstr& Source)  {
  olxstr o_src(TEFile::UnixPath(Source));
  TOnProgress Progress;
  Progress.SetMax(1);
  Progress.SetPos(0);
  Progress.SetAction(olxstr("Downloading ") << o_src );
  TBasicApp::GetInstance()->OnProgress->Enter(this, &Progress);
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
    return NULL;
  }
  TEMemoryStream* ms = new TEMemoryStream();
  char* bf = new char[1024];
  try {
    is->Read(bf, 1024);
    while( is->LastRead() != 0 )  {
      ms->Write( bf, is->LastRead() );
      if( Progress.GetMax() > 0 )  {
        Progress.SetPos( ms->GetPosition() );
        TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
      }
      is->Read(bf, 1024);
    }
    ms->SetPosition(0);
    Progress.SetAction("Download complete");
    Progress.SetPos( 0 );
    TBasicApp::GetInstance()->OnProgress->Exit(this, &Progress);
  }
  catch(...)  {
    delete ms;
    ms = NULL;
    Progress.SetAction("Download failed");
    Progress.SetPos( 0 );
    TBasicApp::GetInstance()->OnProgress->Execute(this, &Progress);
    TBasicApp::GetInstance()->OnProgress->Exit(this, &Progress);
  }
  delete is;
  delete [] bf;
  return ms;  
}


