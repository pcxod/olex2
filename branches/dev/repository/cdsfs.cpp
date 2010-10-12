#include "cdsfs.h"
#include "settingsfile.h"

bool TSocketFS::UseLocalFS = false;
olxstr TSocketFS::Base;
//.................................................................................................
bool TSocketFS::_OnReadFailed(const THttpFileSystem::ResponseInfo& info, uint64_t position) {
  if( !info.headers.Find("Server", CEmptyString).Equals("Olex2-CDS") )  return false;
  while( --attempts >= 0 )  {
    try  {
      DoConnect();  // reconnect
      _write(GenerateRequest(GetUrl(), "GET", info.source, position));
      return true;
    }
    catch(...)  {
      olx_sleep(1000);
    }
  }
  return false;
}
//.................................................................................................
bool TSocketFS::_DoValidate(const THttpFileSystem::ResponseInfo& info, TEFile& data,
  uint64_t toBeread) const
{
  bool valid = THttpFileSystem::_DoValidate(info, data, toBeread);
  if( BaseValid && info.headers.Find("Server", CEmptyString).Equals("Olex2-CDS") )  {  // make file pesistent and write file info
    data.SetTemporary(valid);
    olxstr ifn = olxstr(data.GetName()) << ".info";
    if( !valid && GetIndex() != NULL )  {
      olxstr fn = TEFile::OSPath(info.source);
      TFSItem* fi;
      if( fn.StartsFrom(GetBase()) )
        fi = GetIndex()->GetRoot().FindByFullName(fn.SubStringFrom(GetBase().Length()));
      else
        fi = GetIndex()->GetRoot().FindByFullName(fn);
      if( fi != NULL )  {
        TSettingsFile sf;
        sf.SetParam("MD5", fi->GetDigest());
        sf.SetParam("Size", fi->GetSize());
        sf.SetParam("Age", fi->GetDateTime());
        sf.SaveSettings(ifn); 
      }
      else  {
        olxcstr digest = info.headers.Find("Content-MD5", CEmptyString);
        if( !digest.IsEmpty() )  {
          TSettingsFile sf;
          sf.SetParam("MD5", digest);
          sf.SaveSettings(ifn); 
        }
      }
    }
    if( valid && TEFile::Exists(ifn) )
      TEFile::DelFile(ifn);
  }
  return valid;
}
//.................................................................................................
TEFile* TSocketFS::_DoAllocateFile(const olxstr& src)  {
  if( !BaseValid )
    return THttpFileSystem::_DoAllocateFile(src);
  try  {
    const olxstr fn = olxstr(Base) << MD5::Digest(TUtf8::Encode(src)); 
    const olxstr ifn = olxstr(fn) << ".info"; 
    if( TEFile::Exists(ifn) && GetIndex() != NULL )  {
      olxstr src_fn = TEFile::OSPath(src);
      TFSItem* fi;
      if( src_fn.StartsFrom(GetBase()) )
        fi = GetIndex()->GetRoot().FindByFullName(src_fn.SubStringFrom(GetBase().Length()));
      else
        fi = GetIndex()->GetRoot().FindByFullName(src_fn);
      if( fi != NULL )  {
        const TSettingsFile sf(ifn);
        if( sf["MD5"] == fi->GetDigest() )
          return new TEFile(fn, "a+b");
      }
    }
    return new TEFile(fn, "w+b");
  }
  catch(...)  {  return NULL;  }
}
//.................................................................................................
