#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "wxhttpfs.h"
#include "actions.h"
#include "bapp.h"
#include "efile.h"
#include "ememstream.h"

//.........................................................................................
bool TwxHttpFileSystem::ReadToStream(IOutputStream& ms, const olxstr& Source)  {
  olxstr o_src(TEFile::UnixPath(Source));
  TOnProgress Progress;
  Progress.SetAction(o_src );
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
      OnProgress->Enter(this, &Progress);
    }
  }
  catch( const TExceptionBase& exc )  {   
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  if( is == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("NULL handle for '") << o_src << '\'');

  char* bf = new char[1024*64];
  try {
    is->Read(bf, 1024*64);
    while( is->LastRead() != 0 )  {
      ms.Write( bf, is->LastRead() );
      if( Progress.GetMax() > 0 )  {
        Progress.SetPos( ms.GetPosition() );
        OnProgress->Execute(this, &Progress);
      }
      if( Break )  {
        delete is;
        delete [] bf;
        return false;
      }
      is->Read(bf, 1024*64);
    }
    ms.SetPosition(0);
    Progress.SetPos( Progress.GetMax() );
    OnProgress->Exit(this, &Progress);
  }
  catch(...)  {
    Progress.SetPos( 0 );
    OnProgress->Execute(this, &Progress);
    OnProgress->Exit(this, &Progress);
  }
  delete is;
  delete [] bf;
  return true;  
}
//.........................................................................................
IInputStream* TwxHttpFileSystem::_DoOpenFile(const olxstr& Source)  {
  TEMemoryStream* ms = new TEMemoryStream();
  try { 
    if( ReadToStream(*ms, Source) )
      return ms;
    delete ms;
    return NULL;
  }  
  catch( const TExceptionBase& exc)  {
    delete ms;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
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
  TEFile* tf = TEFile::TmpFile(EmptyString);
  try { 
    if( ReadToStream(*tf, Source) )
      return tf;
    delete tf;
    return NULL;
  }  
  catch( const TExceptionBase& exc)  {
    delete tf;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
}
//.........................................................................................
bool TwxHttpFileSystem::_DoesExist(const olxstr& f)  {  
  if( Index != NULL )
    return Index->GetRoot().FindByFullName(f) != NULL;
  return false;  
}
//.........................................................................................

