/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_shared_H
#define __olx_sdl_shared_H
#include "ebase.h"
BeginEsdlNamespace()

// not synchronised, i.e. thread safe yet
template <class Data> class Shared  {
  struct Share {
    mutable int ref_cnt;
    Data data;
    Share() : ref_cnt(0) {}
  };
  Share* shared;
protected:
  /* must be implementd in the derived classes if custom cleanuo 
  is required preceding the call to delete */
  virtual void CleanUp()  {}
public:
  Shared()  {
    shared = new Share();
    shared->ref_cnt++;
  }
  Shared(const Shared& s) : shared(s.shared)  {  shared->ref_cnt++;  }
  virtual ~Shared()  {
    if( --shared->ref_cnt == 0 )
      delete shared;
  }
  Shared& operator = (const Shared& s)  {
    if( shared != NULL && --shared->ref_cnt == 0 )
      delete shared;
    shared = s.shared;
    shared->ref_cnt++;
    return *this;
  }
  Data& GetData() const {  return share->data;  }
};

EndEsdlNamespace()
#endif
