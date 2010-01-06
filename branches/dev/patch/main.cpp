#pragma hdrstop
#include <stdio.h>
#include "bapp.h"
#include "log.h"
#include "efile.h"

#include <Registry.hpp>

#include <windows.h>
#include "winhttpfs.h"
#include "settingsfile.h"
//---------------------------------------------------------------------------

#pragma argsused

bool CheckOlexInstalled(TEString& installPath)  {
  TRegistry* Reg = new TRegistry();
  bool res = false;
  try  {
    Reg->RootKey = HKEY_CLASSES_ROOT;
    AnsiString cmdKey = "Applications\\olex2.dll\\shell\\open\\command";
    res = Reg->OpenKey(cmdKey, false);
    if( res )  {
      installPath = Reg->ReadString("").c_str();
      if( installPath.Length() && installPath[0] == '\"' )  {
        int lind = installPath.LastIndexOf('"');
        if( lind > 0 )
          installPath = TEFile::ExtractFilePath(installPath.SubString(1, lind-1));
      }
      Reg->CloseKey();
    }
  }
  catch( ... )  {    }
  delete Reg;
  return res;
}


int main(int argc, char* argv[])  {
  TBasicApp bapp(argv[0], EmptyString);
  bapp.GetLog()->ConsoleMode( true );

  TEString upSet;
  printf("\nOlex2 patch. Version 1.00.");
  printf("\n(c) Oleg Dolomanov & Horst Puschmann, 2007.\n");

  if( !CheckOlexInstalled(upSet) )  {
    printf("\nCould not locate Olex2 installed on your system. Please press Enter to exit...");
    getchar();
    return 0;
  }
  TEFile::AddTrailingBackslashI(upSet) << "usettings.dat";
  if( !TEFile::Exists(upSet) )  {
    printf("Could not locate Olex2 update settings file. Please reinstall Olexpress Enter to exit...");
    printf("\nPlease reinstall Olex2 in a normal way.\nPress Enter to exit...");
    getchar();
    return 0;
  }
  HWND olxwnd = FindWindowEx(NULL, NULL, "wxWindowClassNR", "OLEX II");
  while( olxwnd != NULL )  {
    printf("Please close Olex2 and hit Enter...");
    getchar();
   olxwnd = FindWindowEx(NULL, NULL, "wxWindowClassNR", "OLEX II");
  }
  TSettingsFile settings;
  TEString Proxy, Repository = "http://dimas.dur.ac.uk/olex-distro-test/update";
  settings.LoadSettings( upSet );
  if( settings.ParamExists("proxy") )
    Proxy = settings.ParamValue("proxy");
  if( settings.ParamExists("repository") )
    Repository = settings.ParamValue("repository");

  if( Proxy.Length() && !Proxy.EndsWith('/') )  Proxy += '/';
  if( Repository.Length() && !Repository.EndsWith('/') )  Repository += '/';

  TUrl url(Proxy + Repository);

  TOSFileSystem osFS; // local file system
  TWinHttpFileSystem SrcFS( url ); // remote FS

  osFS.SetBase( TEFile::ExtractFilePath(upSet) );
  TFSIndex FI( SrcFS );

  if( FI.UpdateFile(osFS, "olex2.exe", true) )
    printf("\nolex2.exe successfully updated.");
  else
    printf("\nFailed to update olex2.exe...");

  printf("\n\nPress Enter to exit...");
  getchar();

  return 0;
}
//---------------------------------------------------------------------------
 