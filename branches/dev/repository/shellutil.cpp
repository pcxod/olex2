#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "shellutil.h"
#include "efile.h"
#include "exception.h"

#ifdef __WIN32__
  #include <objbase.h>
  #include <shlguid.h>
  #include <shobjidl.h>
  #include <shlobj.h>
  #include <iphlpapi.h>
#else
  #ifdef __WXWIDGETS__
    #include <wx/stdpaths.h>
    #include <wx/dirdlg.h>
  #endif
  #include <unistd.h>
  #ifdef __MAC__
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <ifaddrs.h>
    #include <net/if_dl.h>
  #else
    #include <net/if.h>
    #include <sys/ioctl.h>
  #endif
#endif

//#undef __WIN32__  // compilation test for wxWidgets


//..............................................................................
bool TShellUtil::CreateShortcut(const olxstr& ShortcutPath,
       const olxstr& ObjectPath,const olxstr& description, bool AddRunAs)  {
#ifdef __WIN32__
  IShellLink* psl;
  CoInitialize(NULL);
  // Get a pointer to the IShellLink interface.
  HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                          IID_IShellLink, (LPVOID*)&psl);
  if( SUCCEEDED(hres) )  {
    IPersistFile* ppf;
    // Set the path to the shortcut target and add the description.
    psl->SetPath( ObjectPath.u_str() );
    psl->SetDescription( description.u_str() );
    psl->SetWorkingDirectory( TEFile::ExtractFilePath(ObjectPath).u_str() );
    //psl->Set
    // Query IShellLink for the IPersistFile interface for saving the
    // shortcut in persistent storage.
    hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
    if( AddRunAs )  {   // set admin rights
      // Look for IShellLinkDataList interface
      IShellLinkDataList* pdl;
      hres = psl->QueryInterface(IID_IShellLinkDataList, (void**)&pdl);
      if( SUCCEEDED(hres) ) {
        DWORD dwFlags = 0;
        hres = pdl->GetFlags(&dwFlags);
        if( SUCCEEDED(hres) ) {
          // Only set SLDF_RUNAS_USER if it's not set, otherwise SetFlags returns an error.
          if ((SLDF_RUNAS_USER & dwFlags) != SLDF_RUNAS_USER)
            hres = pdl->SetFlags(SLDF_RUNAS_USER | dwFlags);
        }
        pdl->Release();
      }
    }
    // Save the link by calling IPersistFile::Save.
    if( SUCCEEDED(hres) )  {
#if !defined(_UNICODE )
      WCHAR wsz[MAX_PATH];
      // Ensure that the string is Unicode.
      MultiByteToWideChar(CP_ACP, 0, ShortcutPath.u_str(), -1, wsz, MAX_PATH);
      hres = ppf->Save(wsz, TRUE);
#else
      hres = ppf->Save(ShortcutPath.u_str(), TRUE);
#endif
      ppf->Release();
    }
    psl->Release();
  }
  CoUninitialize();
  return (hres != NULL);
#endif
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
olxstr TShellUtil::GetSpecialFolderLocation(short folderId)  {
#ifdef __WIN32__
  int FID = 0;
  switch( folderId )  {
    case fiDesktop:       FID = CSIDL_DESKTOP;  break;
    case fiStartMenu:     FID = CSIDL_STARTMENU;  break;
    case fiPrograms:      FID = CSIDL_PROGRAMS;  break;
    case fiStartup:       FID = CSIDL_STARTUP;  break;
    case fiControls:      FID = CSIDL_CONTROLS;  break;
    case fiProgramFiles:  FID = CSIDL_PROGRAM_FILES;  break;
    case fiMyDocuments:   FID = CSIDL_PERSONAL;  break;
    case fiAppData:       FID = CSIDL_APPDATA;  break;
    case fiCommonAppData: FID = CSIDL_COMMON_APPDATA;  break;
    case fiCommonStartMenu: FID = CSIDL_COMMON_STARTMENU;  break;
    case fiCommonDesktop: FID = CSIDL_COMMON_DESKTOPDIRECTORY;  break;
    case fiCommonPrograms: FID = CSIDL_COMMON_PROGRAMS;  break;
    case fiSysProgramFiles:
      {
        HKEY key;
        if( RegOpenKeyEx(HKEY_LOCAL_MACHINE, olxT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion"),
              0, KEY_QUERY_VALUE|KEY_WOW64_64KEY, &key) != ERROR_SUCCESS )
          return EmptyString;
        DWORD sz = 0;
        if( RegQueryValueEx(key, olxT("ProgramFilesDir"), NULL, NULL, NULL, &sz) != ERROR_SUCCESS )
          return EmptyString;
        olxch* data = new olxch[sz/sizeof(olxch)+1];
        if( RegQueryValueEx(key, olxT("ProgramFilesDir"), NULL, NULL, (LPBYTE)data, &sz) != ERROR_SUCCESS )
        {
          delete [] data;
          return EmptyString;
        }
        RegCloseKey(key);
        olxstr rv = olxstr::FromExternal(data, sz/sizeof(olxch)-1);
        return TEFile::AddPathDelimeterI(rv);
      }
      break;
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "unknown identifier");
  }
  LPITEMIDLIST items;
  if( SHGetSpecialFolderLocation(NULL, FID, &items ) == NOERROR )  {
    TCHAR bf[MAX_PATH];
    olxstr retVal;
    if( SHGetPathFromIDList(items, bf) )
      retVal = bf;
    // release memory allocated by the funciton
    LPMALLOC shellMalloc;
    if( SHGetMalloc(& shellMalloc ) == NOERROR )
      shellMalloc->Free(items);
    return TEFile::AddPathDelimeterI(retVal);
  }
  return EmptyString;
#else
  #ifdef __WXWIDGETS__
    olxstr retVal;
    switch( folderId )  {
      case fiAppData:
        retVal = wxStandardPaths().GetUserDataDir().c_str();
      break;
      case fiMyDocuments:
        retVal = wxStandardPaths().GetDocumentsDir().c_str();
      break;
      default:
        throw TInvalidArgumentException(__OlxSourceInfo, "unknown identifier");
    }
    return TEFile::AddPathDelimeterI( retVal );
  #endif
  throw TNotImplementedException(__OlxSourceInfo);
#endif
}
//..............................................................................
#ifdef __WIN32__
int __stdcall BrowseCallbackProc( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)  {
  if( uMsg == BFFM_INITIALIZED && lpData )
    SendMessage( hwnd, BFFM_SETSELECTION, TRUE, lpData);
  return 0;
}
#endif

olxstr TShellUtil::PickFolder( const olxstr& Title,
  const olxstr& SelectedFolder, const olxstr& RootFolder )  {
#ifdef __WIN32__
  LPMALLOC shellMalloc;
  if( SHGetMalloc(& shellMalloc ) != NOERROR )  return EmptyString;

  LPSHELLFOLDER desktopFolder;
  if( SHGetDesktopFolder(&desktopFolder) != NOERROR )  return EmptyString;

  LPITEMIDLIST rootFolder= NULL;
  if( TEFile::Exists( RootFolder ) )  {
    WCHAR wsz[MAX_PATH];
    unsigned long eaten = 0;
    MultiByteToWideChar(CP_ACP, 0, RootFolder.c_str(), -1, wsz, MAX_PATH);
    desktopFolder->ParseDisplayName(NULL, NULL, wsz, &eaten, &rootFolder, NULL);
  }
  BROWSEINFO bi;
  TCHAR* path = (TCHAR*)shellMalloc->Alloc( MAX_PATH*sizeof(TCHAR) );
  memset( &bi, 0, sizeof(bi) );
  bi.lpszTitle = Title.u_str();
  bi.pszDisplayName = path;
  bi.pidlRoot = rootFolder;
  bi.ulFlags = BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;
  if( !SelectedFolder.IsEmpty() && TEFile::Exists(SelectedFolder) )  {
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)SelectedFolder.u_str();
  }

  LPITEMIDLIST pidlBrowse = SHBrowseForFolder(&bi);

  if(  pidlBrowse )  {
    olxstr retVal;
    if( SHGetPathFromIDList( pidlBrowse, path ) )
      retVal = path;

    shellMalloc->Free( pidlBrowse );
    shellMalloc->Free( path );
    return retVal;
  }
  return EmptyString;
#else
  #ifdef __WXWIDGETS__
  wxDirDialog dd(NULL, Title.u_str(), SelectedFolder.u_str());
  if( dd.ShowModal() == wxID_OK )
    return dd.GetPath().c_str();
  return EmptyString;
  #endif
  throw TNotImplementedException(__OlxSourceInfo);
#endif
}
//.....................................................................................
bool TShellUtil::_MACFromArray(const unsigned char* bf, const char* name, MACInfo& mi, size_t len ,bool accept_empty)  {
  if( !accept_empty )  {
    uint32_t sum = 0;
    for( size_t i=0; i < len; i++ )
      sum += bf[i];
    if( sum == 0 )  return false;
  }
  TArrayList<unsigned char>& MAC = mi.Add(name).Object;
  MAC.SetCount(len);
  for( size_t i=0; i < len; i++ )
    MAC[i] = bf[i];
  return true;
}
//.....................................................................................
//http://www.codeguru.com/cpp/i-n/network/networkinformation/article.php/c5451 win
//http://cboard.cprogramming.com/linux-programming/43261-ioctl-request-get-hw-address.html unix/linux
//http://othermark.livejournal.com/3005.html mac/freebsd
//http://lists.freebsd.org/pipermail/freebsd-hackers/2004-June/007415.html as above
void TShellUtil::ListMACAddresses( TShellUtil::MACInfo& rv )  {
#ifdef __WIN32__
  IP_ADAPTER_INFO ai[16];
  ULONG bfLen = sizeof(ai);
  if( GetAdaptersInfo(ai, &bfLen) != ERROR_SUCCESS )
    return;
  PIP_ADAPTER_INFO pai = ai;
  do {
    if( pai->AddressLength == 6 )
      _MACFromArray( (unsigned char*)&pai->Address[0], (char*)&(pai->Description[0]), rv, 6, false);
    pai = pai->Next;
  }
  while( pai != NULL );
#elif SIOCGIFHWADDR // I thought this will be enough, but no - Mac is special...
  struct ifconf ifc;
  struct ifreq ifs[32];
  int sckt = socket(AF_INET, SOCK_DGRAM, 0);
  if( sckt == -1 )  return;
  ifc.ifc_len = sizeof(ifs);
  ifc.ifc_req = ifs;
  if( ioctl(sckt, SIOCGIFCONF, &ifc) < 0 )  {
    close(sckt);
    return;
  }
  struct ifreq* ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));
  for( struct ifreq* ifr = ifc.ifc_req; ifr < ifend; ifr++ )  {
    if( ifr->ifr_addr.sa_family == AF_INET )  {
      struct ifreq ifreq;
      strncpy(ifreq.ifr_name, ifr->ifr_name,sizeof(ifreq.ifr_name));
      if( ioctl (sckt, SIOCGIFHWADDR, &ifreq) < 0 )
        return;
      _MACFromArray( (unsigned char*)&ifreq.ifr_hwaddr.sa_data, (char*)&(ifreq.ifr_name[0]), rv, 6, false);
    }
  }
	close(sckt);
#else
  struct ifaddrs* ifaddrs, *tmpia;
  getifaddrs(&ifaddrs);
  tmpia = ifaddrs;
  while( tmpia != NULL )  {
    if( tmpia->ifa_addr->sa_family != AF_LINK)  {
     tmpia = tmpia->ifa_next;
     continue;
    }
    struct sockaddr_dl* sck_dl = (struct sockaddr_dl*)tmpia->ifa_addr;
    if( sck_dl->sdl_alen != 6 )  {
      tmpia = tmpia->ifa_next;
      continue;
    }
    _MACFromArray( (unsigned char*)LLADDR(sck_dl), (char*)&(tmpia->ifa_name[0]), rv, 6, false);
    tmpia = tmpia->ifa_next;
  }
  if( ifaddrs != NULL )
  freeifaddrs(ifaddrs);
#endif
}

