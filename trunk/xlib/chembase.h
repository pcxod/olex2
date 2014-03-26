/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_cl_base_H
#define __olx_cl_base_H

#define BeginChemNamespace()  namespace chemlib {
#define EndChemNamespace()  };\
  using namespace chemlib;
#define UseChemNamespace()  using namespace chemlib;
#define GlobalChemFunction( fun )     chemlib::fun
#define ChemObject( obj )     chemlib::obj

#endif
