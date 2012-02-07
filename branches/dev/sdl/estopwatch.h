/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_stop_watch_h
#define __olx_stop_watch_h
#include "etime.h"
#include "bapp.h"
#include "etable.h"
BeginEsdlNamespace()

class TStopWatch  {
  TTypeList<AnAssociation3<uint64_t,olxstr, uint64_t> > steps;
  olxstr FunctionName;
public:
  TStopWatch(const olxstr& functionName) : FunctionName(functionName) {
    steps.AddNew( TETime::msNow(), EmptyString(), 0);
  }
  void start(const olxstr& name)  {
    steps.AddNew(TETime::msNow(), name, 0);
  }
  void stop()  {
    steps.GetLast().C() = TETime::msNow();
  }
protected:
  TStrList prepareList()  {
    TStrList out;
    if( steps.GetLast().GetC() == 0 )
      steps.GetLast().C() = TETime::msNow();
    TTTable<TStrList> tb(steps.Count()-1, 3);
    tb.ColName(0) = "Stop name";
    tb.ColName(1) = "Time till stop (ms)";
    tb.ColName(2) = "Time till next (ms)";
    for( size_t i=1; i < steps.Count(); i++ )  {
      tb[i-1][0] = steps[i].GetB();
      if( steps[i].GetC() != 0 )
        tb[i-1][1] = steps[i].GetC() - steps[i].GetA();
      if( (i+1) < steps.Count() )
        tb[i-1][2] = steps[i+1].GetA() - steps[i].GetA();
    }
    tb.CreateTXTList(out, olxstr("Profiling information for " ) << FunctionName, true, false, ' ');
    out.Add("Total time ") << (steps.GetLast().GetC() - steps[0].GetA()) << "ms";
    return out;
  }
public:
  template <class T> void print(T out)  {
    if( TBasicApp::GetInstance().IsProfiling() )
      out << prepareList();
  }
  template <class T> void print(T& inst, void (T::*func)(const olxstr& str) )  {
    if( !TBasicApp::GetInstance().IsProfiling() )  return;
    TStrList lst = prepareList();
    for( size_t i=0; i < lst.Count(); i++ )
      (inst.*func)( lst[i] );
  }
};

EndEsdlNamespace()
#endif