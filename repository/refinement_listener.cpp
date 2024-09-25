#include "refinement_listener.h"
#include "../xlib/xapp.h"
#include "integration.h"
#include <thread>

namespace olex2 {

  RefinementListener::RefinementListener()
    : valid(false)
  {
    IOlex2Processor* ip = IOlex2Processor::GetInstance();
    if (ip == 0) {
      return;
    }
    olxstr cmd = "spy.GetParam(snum.refinement.program)";
    if (!ip->processFunction(cmd, EmptyString(), true)) {
      return;
    }
    if (cmd == "olex2.refine") {
      cmd = "StrDir()\nFileName()";
      if (ip->processFunction(cmd, EmptyString(), true)) {
        TStrList x(cmd, '\n');
        fin_fn = TEFile::AddPathDelimeterI(x[0])
          << TEFile::TEFile::AddPathDelimeter("temp")
          << x[1].Replace(' ', EmptyString()) << ".fin";
      }
    }
  }

  bool& RefinementListener::Continue() {
    static bool c = true;
    return c;
  }

  bool RefinementListener::OnProgress(size_t max, size_t pos) {
    if ((pos % 100) == 0) {
      RefinementListener*& i = GetInstance();
      if (i == 0) {
        i = new RefinementListener();
      }
      if (!i->fin_fn.IsEmpty() && TEFile::Exists(i->fin_fn)) {
        DoBreak();
      }
      IOlex2Processor* ip = IOlex2Processor::GetInstance();
      if (ip != 0) {
        ip->processMacro("refresh");
      }
    }
    if (max == ~0 && pos == ~0) {
      IOlex2Processor* ip = IOlex2Processor::GetInstance();
      if (ip != 0) {
        ip->processMacro("refresh");
      }
    }
    return Continue();
  }

}