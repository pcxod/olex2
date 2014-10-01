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

// a scheduled macro
struct TScheduledTask  {
  bool Repeatable, NeedsGUI;
  olxstr Task;
  time_t Interval, LastCalled;
  TScheduledTask()
    : Repeatable(false), NeedsGUI(false), Interval(0), LastCalled(0)
  {}
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
  olxstr hklsrc;
  vec3d axes, axis_esd, angles, angle_esd;
  CellChangeTask(const olxstr &hklsrc_, const TAsymmUnit &au)
    : hklsrc(hklsrc_),
    axes(au.GetAxes()), axis_esd(au.GetAxisEsds()),
    angles(au.GetAngles()), angle_esd(au.GetAngleEsds())
  {}

  virtual void Run();
};

#endif
