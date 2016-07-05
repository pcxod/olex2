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
#include "bapp.h"

// simple OS utilities...
#ifdef __WIN32__

bool EsdlObject(olx_setenv)(const olxstr& name, const olxstr& val) {
  return SetEnvironmentVariable(name.u_str(), val.u_str()) != 0;
}
//.............................................................................
olxstr EsdlObject(olx_getenv)(const olxstr& name) {
  olxch* val = olx_malloc<olxch>(1024);
  DWORD sz_;
  if ((sz_ = GetEnvironmentVariable(name.u_str(), val, 1024)) > 1024) {
    val = olx_realloc<>(val, sz_);
    GetEnvironmentVariable(name.u_str(), val, sz_);
    return olxstr::FromExternal(val, sz_ - 1);
  }
  else {
    olx_free(val);
    return EmptyString();
  }
}

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
  if (handle == NULL) {
    handle = ::GetModuleHandle(NULL);
  }
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
  return setenv(name.ToMBStr().c_str(), val.ToMBStr().c_str(), 1) == 0;
}
olxstr EsdlObject(olx_getenv)(const olxstr& name)  {
  const char*p = getenv(name.ToMBStr().c_str());
  return p == 0 ? EmptyString() : olxstr::FromCStr(p);
}
#endif

bool EsdlObject(olx_setenv)(const olxstr& v)  {
  size_t ei = v.IndexOf('=');
  if( ei == InvalidIndex )  return false;
  return olx_setenv(v.SubStringTo(ei).u_str(), v.SubStringFrom(ei+1).u_str());
}
