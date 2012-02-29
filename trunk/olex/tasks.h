/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_tasks_H
#define __olx_tasks_H
#include "evector.h"
#include "p4p.h"

// a schecduled macro
struct TScheduledTask  {
  bool Repeatable;
  olxstr Task;
  long Interval, LastCalled;
};
/* some tasks which can be executed at startup, but the main window must be
visible
*/
struct IOlxTask {
  virtual ~IOlxTask() {}
  virtual void Run() = 0;
};

struct P4PTask : public IOlxTask {
  olxstr file_id;
  P4PTask(const TP4PFile &p4p)
    : file_id(p4p.GetFileId())
  {}
  virtual void Run();
};

struct CellChangeTask : public IOlxTask {
  evecd cell, esds;
  CellChangeTask(const evecd &cell_, const evecd &esds_)
    : cell(cell_), esds(esds_)
  {}
  
  virtual void Run();
};

#endif
