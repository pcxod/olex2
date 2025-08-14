/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "estopwatch.h"

//.............................................................................
const_strlist TStopWatchManager::Record::prepareList(size_t level)  {
  volatile olx_scope_cs cs_(GetCriticalSection());
  TStrList out;
  olxstr ident = olxstr::CharStr(' ', level*2);
  (out.Add(ident) << "Profiling information for " << FunctionName)
    .RightPadding(75, ' ', true) << termination_time-creation_time << "ms";
  if (!steps.IsEmpty() && steps.GetLast().GetC() == 0)
    steps.GetLast().c = termination_time;
  for (size_t i=0; i < sequence.Count(); i++) {
    size_t idx = olx_abs(sequence[i])-1;
    if (sequence[i] < 0) {
      out << nodes[idx].prepareList(level+1);
    }
    else {
      (out.Add(ident) << ident << steps[idx].GetB())
        .RightPadding(75, ' ', true) <<
        steps[idx].GetC() - steps[idx].GetA() << "ms";
    }
  }
  return out;
}
//.............................................................................
void TStopWatchManager::print() {
  volatile olx_scope_cs cs_(GetCriticalSection());
  if (!TBasicApp::GetInstance().IsProfiling()) {
    return;
  }
  TBasicApp::NewLogEntry(logInfo) << NewLineSequence() <<
    current_()->prepareList(1) << NewLineSequence();
}
//.............................................................................
TStopWatchManager::Record &TStopWatchManager::Push(
  const olxstr &functionName)
{
  volatile olx_scope_cs cs_(GetCriticalSection());
  if (current_() == 0) {
    return *(current_() = new Record(0, functionName));
  }
  return *(current_() = &current_()->New(functionName));
}
//.............................................................................
void TStopWatchManager::Pop() {
  volatile olx_scope_cs cs_(GetCriticalSection());
  if (current_() == 0) {
    throw TFunctionFailedException(__OlxSourceInfo, "current==NULL");
  }
  if (current_()->parent == 0) {
    print();
    delete current_();
    current_() = 0;
  }
  else {
    current_() = current_()->parent;
  }
}
//.............................................................................
