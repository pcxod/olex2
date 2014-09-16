/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#ifndef __sdl_xml_H
#define __sdl_xml_H
#include "ebase.h"
BeginEsdlNamespace()

struct XML {
  static olxstr encode(const olxstr &s) {
    return olxstr(s).Replace('&', "&amp;")
      .Replace('"', "&quot;")
      .Replace('>', "&gt;")
      .Replace('<', "&lt;");
  }

  static olxstr decode(const olxstr &s) {
    return olxstr(s).Replace("&quot;", '"')
      .Replace("&gt;", '>')
      .Replace("&lt;", '<')
      .Replace("&amp;", '&');
  }
};

EndEsdlNamespace()
#endif
