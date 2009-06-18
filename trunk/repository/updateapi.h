#ifndef __olx_update_api_H
#define __olx_update_api_H

#include "filesystem.h"
#include "bapp.h"
#ifdef __WXWIDGETS__
  #include "wxftpfs.h"
#endif

/* a usettings.dat file processor since the launch and unirun do not their original
job now, their API is here, night be useful in the future... */
namespace updater  {

const short
  uapi_OK = 0,
  uapi_NoSettingsFile = 1,
  uapi_FTP_Error = 2;

class UpdateAPI  {
  bool UpdateInstallation(AFileSystem& SrcFS, const TStrList& properties,
    const TFSItem::SkipOptions* toSkip, AActionHandler* file_lsnr, AActionHandler* p_lsnr)
  {
    try  {
      TOSFileSystem DestFS(TBasicApp::GetInstance()->BaseDir()); // local file system
      TFSIndex FI( SrcFS );
      if( p_lsnr != NULL )
        FI.OnProgress->Add(p_lsnr);  
      if( file_lsnr != NULL )
        SrcFS.OnProgress->Add( file_lsnr );
      return FI.Synchronise(DestFS, properties, toSkip);
    }
    catch( TExceptionBase& exc )  {
      TStrList out;
      exc.GetException()->GetStackTrace(out);
      log.AddList(out);
      return false;
    }
  }
#ifdef __WXWIDGETS__
  bool UpdateMirror(AFileSystem& src, TwxFtpFileSystem& dest, 
    AActionHandler* file_lsnr, AActionHandler* p_lsnr)  
  {
    try  {
      TFSIndex FI( src );
      if( p_lsnr != NULL )
        FI.OnProgress->Add(p_lsnr);  
      if( file_lsnr != NULL )
        dest.OnProgress->Add( file_lsnr );
      TStrList empty;
      return FI.Synchronise(dest, empty, NULL);
    }
    catch( TExceptionBase& exc )  {
      TStrList out;
      exc.GetException()->GetStackTrace(out);
      log.AddList(out);
      return false;
    }
  }
#endif
  TStrList log;
public:
  short DoUpdate(AActionHandler* file_slnr, AActionHandler* p_lsnr);
  const TStrList& GetLog() const {  return log;  }
};

};

#endif
