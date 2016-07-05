/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "os_util.h"
#include "exception.h"

// simple OS utilities...
#ifdef __WIN32__
#  if defined(_MSC_VER)
#    ifdef _UNICODE
#      define OLX_GETENV _wgetenv_s
#      define OLX_PUTENV _wputenv_s
#    else
#      define OLX_GETENV getenv_s
#      define OLX_PUTENV _putenv_s
#    endif
   bool EsdlObject(olx_setenv)(const olxstr& name, const olxstr& val)  {
     return OLX_PUTENV(name.u_str(), val.u_str()) == 0;
   }
   //.............................................................................
   //.............................................................................
   olxstr EsdlObject(olx_getenv)(const olxstr& name)  {
     olxch* val=NULL;
     size_t sz;
     OLX_GETENV(&sz, NULL, 0, name.u_str());
     if( sz == 0 )  return EmptyString();
     val = olx_malloc<olxch>(sz);
     OLX_GETENV(&sz, val, sz, name.u_str());
     return olxstr::FromExternal(val, sz-1);
   }
#  else // not MSVC
#    ifdef _UNICODE
#      define OLX_GETENV _wgetenv
#      define OLX_PUTENV _wputenv
#    else
#      define OLX_GETENV getenv
#      define OLX_PUTENV putenv
#    endif
  bool EsdlObject(olx_setenv)(const olxstr& name, const olxstr& val)  {
     return OLX_GETENV((olxstr(name) << '=' << val).u_str()) == 0;
   }
   olxstr EsdlObject(olx_getenv)(const olxstr& name)  {
     return OLX_PUTENV(name.u_str());
   }
#  endif  // MSVC and others WIN compilers
/*http://msdn.microsoft.com/en-us/library/ms684139.aspx
return true if the process is running under wow64 - ie on x64 bit Windows,
might throw an exception... */
bool EsdlObject(IsWow64)()  {
  typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
  LPFN_ISWOW64PROCESS fnIsWow64Process;
  BOOL bIsWow64 = FALSE;
  fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
    GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
  if( fnIsWow64Process != NULL )  {
    if( !fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
      throw TFunctionFailedException(__OlxSourceInfo, "to call IsWow64Process");
  }
  return bIsWow64 == TRUE;
}
//.............................................................................
//.............................................................................
HANDLE EsdlObject(Module::handle) = NULL;
HANDLE Module::GetHandle() const {
  if (handle == NULL)
    handle = ::GetModuleHandle(NULL);
  return handle;
}
//.............................................................................
HANDLE Module::SetHandle(HANDLE h) {
  HANDLE r = handle;
  handle = h;
  return r;
}
//.............................................................................
//.............................................................................
#else  // not WIN
bool EsdlObject(olx_setenv)(const olxstr& name, const olxstr& val)  {
  return setenv(name.c_str(), val.c_str(), 1) == 0;
}
olxstr EsdlObject(olx_getenv)(const olxstr& name)  {
  return getenv(name.c_str());
}
#endif

bool EsdlObject(olx_setenv)(const olxstr& v)  {
  size_t ei = v.IndexOf('=');
  if( ei == InvalidIndex )  return false;
  return olx_setenv(v.SubStringTo(ei).u_str(), v.SubStringFrom(ei+1).u_str());
}
