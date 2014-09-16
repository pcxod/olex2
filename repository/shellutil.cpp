/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "shellutil.h"
#include "efile.h"
#include "bapp.h"

#ifdef __WIN32__
  #include <objbase.h>
  #include <shlguid.h>
#ifndef __GNUC__
  #include <Softpub.h>
  #include <wintrust.h>
  #include <shobjidl.h>
  #pragma comment (lib, "wintrust")
  #pragma comment (lib, "version")
#endif
  #include <shlobj.h>
  #include <iphlpapi.h>
#else
  #ifdef __WXWIDGETS__
    #include <wx/stdpaths.h>
    #include <wx/dirdlg.h>
    #include <wx/filedlg.h>
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
#ifndef __GNUC__
bool TShellUtil::CreateShortcut(const olxstr& ShortcutPath,
  const olxstr& ObjectPath,const olxstr& description, bool AddRunAs)
{
#ifdef __WIN32__
  IShellLink* psl;
  CoInitialize(NULL);
  // Get a pointer to the IShellLink interface.
  HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
    IID_IShellLink, (LPVOID*)&psl);
  if (SUCCEEDED(hres)) {
    IPersistFile* ppf;
    // Set the path to the shortcut target and add the description.
    psl->SetPath(ObjectPath.u_str());
    psl->SetDescription(description.u_str());
    psl->SetWorkingDirectory(TEFile::ExtractFilePath(ObjectPath).u_str());
    hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
    if( AddRunAs )  {   // set admin rights
      IShellLinkDataList* pdl;
      hres = psl->QueryInterface(IID_IShellLinkDataList, (void**)&pdl);
      if (SUCCEEDED(hres)) {
        DWORD dwFlags = 0;
        hres = pdl->GetFlags(&dwFlags);
        if (SUCCEEDED(hres)) {
          /*
          Only set SLDF_RUNAS_USER if it's not set, otherwise SetFlags
          returns an error
          */
          if ((SLDF_RUNAS_USER & dwFlags) != SLDF_RUNAS_USER)
            hres = pdl->SetFlags(SLDF_RUNAS_USER | dwFlags);
        }
        pdl->Release();
      }
    }
    // Save the link by calling IPersistFile::Save
    if( SUCCEEDED(hres) )  {
#if !defined(_UNICODE )
      olx_array_ptr<WCHAR> wsz( new WCHAR[MAX_PATH]);
      // Ensure that the string is Unicode.
      MultiByteToWideChar(CP_ACP, 0, ShortcutPath.u_str(), -1, wsz(), MAX_PATH);
      hres = ppf->Save(wsz(), TRUE);
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
#endif //__GNUC__
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
        /*
        determine windows version, win2000 and earlier do not support
        KEY_WOW64_64KEY...
        */
        OSVERSIONINFO veri;
        memset(&veri, 0, sizeof(veri));
        veri.dwOSVersionInfoSize = sizeof(veri);
        GetVersionEx(&veri);
        LONG flags = KEY_QUERY_VALUE;
        // is XP or later?
#ifdef KEY_WOW64_64KEY
        if ( veri.dwMajorVersion > 5 ||
            (veri.dwMajorVersion == 5 && veri.dwMinorVersion > 0 ))
        {
          flags |= KEY_WOW64_64KEY;
        }
#endif
        HKEY key;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
              olxT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion"),
              0, flags, &key) != ERROR_SUCCESS)
        {
          return EmptyString();
        }
        DWORD sz = 0;
        if (RegQueryValueEx(key, olxT("ProgramFilesDir"),
          NULL, NULL, NULL, &sz) != ERROR_SUCCESS)
        {
          return EmptyString();
        }
        olx_array_ptr<olxch> data(new olxch[sz/sizeof(olxch)+1]);
        if (RegQueryValueEx(key, olxT("ProgramFilesDir"),
            NULL, NULL, (LPBYTE)data(), &sz) != ERROR_SUCCESS)
        {
          return EmptyString();
        }
        RegCloseKey(key);
        olxstr rv = olxstr::FromExternal(data.release());
        return TEFile::AddPathDelimeterI(rv);
      }
      break;
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "unknown identifier");
  }
  LPITEMIDLIST items;
  if (SHGetSpecialFolderLocation(NULL, FID, &items ) == NOERROR) {
    olx_array_ptr<olxch> bf(new olxch [MAX_PATH]);
    olxstr retVal;
    if (SHGetPathFromIDList(items, bf()))
      retVal = olxstr::FromExternal(bf.release());
    // release memory allocated by the funciton
    LPMALLOC shellMalloc;
    if (SHGetMalloc(&shellMalloc) == NOERROR)
      shellMalloc->Free(items);
    return TEFile::AddPathDelimeterI(retVal);
  }
  return EmptyString();
#else
  #ifdef __WXWIDGETS__
    olxstr retVal;
    switch( folderId )  {
      case fiAppData:
        retVal = wxStandardPaths::Get().GetUserDataDir();
      break;
      case fiMyDocuments:
        retVal = wxStandardPaths::Get().GetDocumentsDir();
      break;
      default:
        throw TInvalidArgumentException(__OlxSourceInfo, "unknown identifier");
    }
    return TEFile::AddPathDelimeterI(retVal);
  #endif
  throw TNotImplementedException(__OlxSourceInfo);
#endif
}
//..............................................................................
#ifdef __WIN32__
int __stdcall BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam,
  LPARAM lpData)
{
  if (uMsg == BFFM_INITIALIZED && lpData)
    SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
  return 0;
}
#endif

olxstr TShellUtil::PickFolder(const olxstr& Title,
  const olxstr& SelectedFolder, const olxstr& RootFolder)
{
#ifdef __WIN32__
  LPMALLOC shellMalloc;
  if (SHGetMalloc(&shellMalloc) != NOERROR)
    return EmptyString();

  LPSHELLFOLDER desktopFolder;
  if (SHGetDesktopFolder(&desktopFolder) != NOERROR)
    return EmptyString();

  LPITEMIDLIST rootFolder= NULL;
  if (TEFile::Exists(RootFolder)) {
    ULONG eaten;
    olx_array_ptr<WCHAR> tmp(new WCHAR[RootFolder.Length()+1]);
    memcpy(tmp(), RootFolder.raw_str(), RootFolder.RawLen());
    tmp[RootFolder.Length()] = '\0';
    desktopFolder->ParseDisplayName(NULL, NULL, tmp(), &eaten,
      &rootFolder, NULL);
  }
  BROWSEINFO bi;
  olxch* path = (olxch*)shellMalloc->Alloc(MAX_PATH*sizeof(TCHAR));
  memset(&bi, 0, sizeof(bi));
  bi.lpszTitle = Title.u_str();
  bi.pszDisplayName = path;
  bi.pidlRoot = rootFolder;
  bi.ulFlags = BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE;
  if (!SelectedFolder.IsEmpty() && TEFile::Exists(SelectedFolder)) {
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)SelectedFolder.u_str();
  }

  LPITEMIDLIST pidlBrowse = SHBrowseForFolder(&bi);

  if (pidlBrowse) {
    olxstr retVal;
    if (SHGetPathFromIDList(pidlBrowse, path))
      retVal = path;
    shellMalloc->Free(pidlBrowse);
    shellMalloc->Free(path);
    return retVal;
  }
  return EmptyString();
#else
  #ifdef __WXWIDGETS__
  wxDirDialog dd(NULL, Title.u_str(), SelectedFolder.u_str());
  if (dd.ShowModal() == wxID_OK)
    return dd.GetPath();
  return EmptyString();
  #endif
  throw TNotImplementedException(__OlxSourceInfo);
#endif
}
//.............................................................................
olxstr TShellUtil::PickFile(const olxstr& Title, const olxstr &Filter,
  bool open,
  const olxstr& DefFolder, const olxstr &DefFile)
{
#ifdef __WIN32__
  OPENFILENAME ofn;
  memset(&ofn, 0, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  olxstr f = olxstr(Filter).Replace('|', '\0') << '\0';
  ofn.lpstrFilter = f.u_str();
  olx_array_ptr<olxch> bf(new olxch[MAX_PATH]);
  if (DefFile.Length()+1 < MAX_PATH) {
    olx_memcpy(bf(), DefFile.u_str(), DefFile.Length());
    bf()[DefFile.Length()] = '\0';
  }
  ofn.lpstrFile = bf();
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrTitle = Title.u_str();
  if (!DefFolder.IsEmpty())
    ofn.lpstrInitialDir = DefFolder.u_str();
  ofn.Flags = OFN_ENABLESIZING;
  if (open) {
    ofn.Flags |= OFN_FILEMUSTEXIST;
    if (GetSaveFileName(&ofn) == TRUE)
      return olxstr::FromExternal(bf.release());
  }
  else {
    if (GetOpenFileName(&ofn) == TRUE)
      return olxstr::FromExternal(bf.release());
  }
  return EmptyString();
#elif __WXWIDGETS__
  int Style = open ? wxFD_OPEN : wxFD_SAVE;
  wxFileDialog dlgFile(NULL, Title.u_str(),
    DefFolder.u_str(), DefFile.u_str(), Filter.u_str(), Style);
  return (dlgFile.ShowModal() == wxID_OK ? olxstr(dlgFile.GetPath())
    : EmptyString());
#else
  throw TNotImplementedException(__OlxSourceInfo);
#endif
}
//.............................................................................
bool TShellUtil::_MACFromArray(const unsigned char* bf, const char* name,
  MACInfo& mi, size_t len, bool accept_empty)
{
  if (!accept_empty) {
    uint32_t sum = 0;
    for (size_t i=0; i < len; i++)
      sum += bf[i];
    if (sum == 0)  return false;
  }
  TArrayList<unsigned char>& MAC = mi.Add(name).Object;
  MAC.SetCount(len);
  for (size_t i=0; i < len; i++)
    MAC[i] = bf[i];
  return true;
}
//.............................................................................
/*
http://www.codeguru.com/cpp/i-n/network/networkinformation/article.php/c5451 win
http://cboard.cprogramming.com/linux-programming/43261-ioctl-request-get-hw-address.html unix/linux
http://othermark.livejournal.com/3005.html mac/freebsd
http://lists.freebsd.org/pipermail/freebsd-hackers/2004-June/007415.html as above
*/
void TShellUtil::ListMACAddresses(TShellUtil::MACInfo& rv) {
#ifdef __WIN32__
  IP_ADAPTER_INFO ai[16];
  ULONG bfLen = sizeof(ai);
  if (GetAdaptersInfo(ai, &bfLen) != ERROR_SUCCESS)
    return;
  PIP_ADAPTER_INFO pai = ai;
  do {
    if (pai->AddressLength == 6) {
      _MACFromArray( (unsigned char*)&pai->Address[0],
        (char*)&(pai->Description[0]), rv, 6, false);
    }
    pai = pai->Next;
  }
  while (pai != NULL);
#elif SIOCGIFHWADDR // I thought this will be enough, but no - Mac is special...
  struct ifconf ifc;
  struct ifreq ifs[32];
  int sckt = socket(AF_INET, SOCK_DGRAM, 0);
  if (sckt == -1)  return;
  ifc.ifc_len = sizeof(ifs);
  ifc.ifc_req = ifs;
  if (ioctl(sckt, SIOCGIFCONF, &ifc) < 0) {
    close(sckt);
    return;
  }
  struct ifreq* ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));
  for (struct ifreq* ifr = ifc.ifc_req; ifr < ifend; ifr++) {
    if (ifr->ifr_addr.sa_family == AF_INET) {
      struct ifreq ifreq;
      strncpy(ifreq.ifr_name, ifr->ifr_name,sizeof(ifreq.ifr_name));
      if (ioctl (sckt, SIOCGIFHWADDR, &ifreq) < 0)
        return;
      _MACFromArray((unsigned char*)&ifreq.ifr_hwaddr.sa_data,
        (char*)&(ifreq.ifr_name[0]), rv, 6, false);
    }
  }
  close(sckt);
#else
  struct ifaddrs* ifaddrs, *tmpia;
  getifaddrs(&ifaddrs);
  tmpia = ifaddrs;
  while (tmpia != NULL) {
    if (tmpia->ifa_addr->sa_family != AF_LINK) {
     tmpia = tmpia->ifa_next;
     continue;
    }
    struct sockaddr_dl* sck_dl = (struct sockaddr_dl*)tmpia->ifa_addr;
    if (sck_dl->sdl_alen != 6) {
      tmpia = tmpia->ifa_next;
      continue;
    }
    _MACFromArray((unsigned char*)LLADDR(sck_dl),
      (char*)&(tmpia->ifa_name[0]), rv, 6, false);
    tmpia = tmpia->ifa_next;
  }
  if (ifaddrs != NULL)
    freeifaddrs(ifaddrs);
#endif
}
//.............................................................................
//http://msdn.microsoft.com/en-us/library/windows/desktop/aa382384(v=vs.85).aspx
#if defined(__WIN32__) && !defined(__GNUC__)
bool TShellUtil::VerifyEmbeddedSignature(const olxstr &file_name) {
  WINTRUST_FILE_INFO FileData;
  memset(&FileData, 0, sizeof(FileData));
  FileData.cbStruct = sizeof(WINTRUST_FILE_INFO);
  FileData.pcwszFilePath = file_name.u_str();
  FileData.hFile = NULL;
  FileData.pgKnownSubject = NULL;

  GUID WVTPolicyGUID = WINTRUST_ACTION_GENERIC_VERIFY_V2;
  WINTRUST_DATA WinTrustData;
  memset(&WinTrustData, 0, sizeof(WinTrustData));
  WinTrustData.cbStruct = sizeof(WinTrustData);
  WinTrustData.pPolicyCallbackData = NULL;
  WinTrustData.pSIPClientData = NULL;
  WinTrustData.dwUIChoice = WTD_UI_NONE;
  WinTrustData.fdwRevocationChecks = WTD_REVOKE_NONE; 
  WinTrustData.dwUnionChoice = WTD_CHOICE_FILE;
  WinTrustData.dwStateAction = 0;
  WinTrustData.hWVTStateData = NULL;
  WinTrustData.pwszURLReference = NULL;
  WinTrustData.dwUIContext = 0;
  WinTrustData.pFile = &FileData;
  return WinVerifyTrust(
    NULL,
    &WVTPolicyGUID,
    &WinTrustData) == ERROR_SUCCESS;
}
#endif // __WIN32__ && !__GNUC__
//.............................................................................
#ifdef __WIN32__
#ifndef __GNUC__
olxstr TShellUtil::GetFileVersion(const olxstr &fn, const olxstr &lang) {
  if (TEFile::Exists(fn)) {
    DWORD len = GetFileVersionInfoSize(fn.u_str(), &len);
    if (len > 0)  {
      olx_array_ptr<olxch> pBuf(new olxch[len+1]);
      olxch *pValue[1];
      UINT pLen;
      GetFileVersionInfo(fn.u_str(), 0, len, pBuf());
      if (VerQueryValue(pBuf(),
            (olxstr("StringFileInfo\\") << lang << "\\ProductVersion").u_str(),
            (void**)&pValue[0], &pLen))
      {
        return pValue[0];
      }
    } 
  }
  return EmptyString();
}
/*
http://msdn.microsoft.com/en-us/library/aa376389(VS.85).aspx
http://stackoverflow.com/questions/581204/how-do-i-check-if-a-user-has-local-admin-privileges-in-win32
*/
bool TShellUtil::IsAdmin() {
  //SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
  //PSID AdministratorsGroup; 
  //b = AllocateAndInitializeSid(
  //  &NtAuthority,
  //  2,
  //  SECURITY_BUILTIN_DOMAIN_RID,
  //  DOMAIN_ALIAS_RID_ADMINS,
  //  0, 0, 0, 0, 0, 0,
  //  &AdministratorsGroup); 
  //if( b == TRUE )   {
  //  if( !CheckTokenMembership(NULL, AdministratorsGroup, &b) )
  //    b = FALSE;
  //  FreeSid(AdministratorsGroup); 
  //}
  //return b == TRUE;
  // check if supports elevation
  OSVERSIONINFO osvi;
  memset(&osvi, 0, sizeof(osvi));
  osvi.dwOSVersionInfoSize = sizeof(osvi);
  if (GetVersionEx(&osvi) && osvi.dwMajorVersion < 6) {
    return ::IsUserAnAdmin() != FALSE;
  }
#if _WIN32_WINNT >= 0x0600
  bool isAdmin = false, res = true;
  DWORD bytesUsed = 0;
  TOKEN_ELEVATION_TYPE tokenElevationType;
  HANDLE m_hToken = NULL;
  if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &m_hToken)) {
    res = false;
  }
  if (res && !::GetTokenInformation(m_hToken, TokenElevationType,
    &tokenElevationType, sizeof(tokenElevationType), &bytesUsed))
  {
    res = false;
  }
  if (res && tokenElevationType == TokenElevationTypeLimited) {
    TOKEN_LINKED_TOKEN lti;
    if (res && !::GetTokenInformation(m_hToken, TokenLinkedToken,
      reinterpret_cast<void *>(&lti), sizeof(lti), &bytesUsed))
    {
      res = false;
    }
    else {
      BYTE adminSID[SECURITY_MAX_SID_SIZE];
      DWORD sidSize = sizeof(adminSID);
      if (!::CreateWellKnownSid(WinBuiltinAdministratorsSid, 0, &adminSID,
        &sidSize))
      {
        res = false;
      }
      BOOL isMember = FALSE;
      if (res && !::CheckTokenMembership(lti.LinkedToken, &adminSID, &isMember))
        res = false;
      else
        isAdmin = (isMember != FALSE);
    }
  }
  else
    isAdmin = ::IsUserAnAdmin() != FALSE;
  if (m_hToken != NULL)
    CloseHandle(m_hToken);
  return res ? isAdmin : false;
#else
  return ::IsUserAnAdmin();
#endif
}
#endif //__GNUC__
//.............................................................................
//http://msdn.microsoft.com/en-us/library/windows/desktop/bb762153(v=vs.85).aspx
bool TShellUtil::RunElevated(const olxstr &fn, const olxstr &args) {
  return ShellExecute(
    NULL,
    olxT("runas"),
    fn.u_str(),
    args.u_str(),
    NULL,
    SW_SHOW
    ) != 0;
}
#endif // __WIN32__
//.............................................................................
/* Ref:
http://blogs.msdn.com/b/twistylittlepassagesallalike/archive/2011/04/23/everyone-quotes-arguments-the-wrong-way.aspx
*/
olxstr TShellUtil::QuoteArg(const olxstr &a) {
  return TEFile::QuotePath(a);
}
//.............................................................................
olxstr TShellUtil::GetCmdLineArgs(const olxstr &fn,
  bool put_args, bool put_opts)
{
  const TBasicApp &a = TBasicApp::GetInstance();
  const TStrList& args = a.GetArguments();
  olxstr s_cmdl = QuoteArg(fn);
  if (put_args) {
    for( size_t i=1; i < args.Count(); i++ )
      s_cmdl << ' ' << QuoteArg(args[i]);
  }
  if (put_opts) {
    for (size_t i=0; i < a.GetOptions().Count(); i++) {
      s_cmdl << ' ' << a.GetOptions().GetName(i);
      const olxstr &v = a.GetOptions().GetValue(i);
      if (v.IsEmpty()) continue;
      s_cmdl << '=' << QuoteArg(v);
    }
  }
  return s_cmdl;
}
