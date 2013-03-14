/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "integration.h"

using namespace olex;

// we need oly one copy of the instance inside the dll; might not work on other compilers
#if defined(__DLL__) || defined(__GNUC__)
    IOlexRunnable* OlexPort::OlexRunnable;
    IOlexProcessor* IOlexProcessor::Instance;
#else
    IOlexRunnable* OlexPort::OlexRunnable;
    IOlexProcessor* IOlexProcessor::Instance;
#endif

//  const olxstr IOlexProcessor::SGListVarName("olx_int_sglist");

IOlexRunnable* OlexPort::GetOlexRunnable()  {  return OlexRunnable;  }
