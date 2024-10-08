#include "refinement_listener.h"
#include "../xlib/xapp.h"
#include "integration.h"

namespace olex2 {

  RefinementListener::RefinementListener() {
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
        fin_fn = TEFile::JoinPath(x) << ".fin";
      }
    }
  }

  bool& RefinementListener::Continue() {
    static bool c = true;
    return c;
  }

  bool RefinementListener::OnProgress(size_t max, size_t pos) {
    /* this block is only called form the thread in which the refinement has
    been started    */
    if (max == ~0 && pos == ~0) {
      volatile olx_scope_cs cs(get_critical_section());
      RefinementListener*& i = GetInstance();
      if (i == 0) {
        i = new RefinementListener();
      }
      // TEFile::Exists creates a copy of a string it is not thread-safe!
      if (!i->fin_fn.IsEmpty() && TEFile::Exists(i->fin_fn)) {
        DoBreak();
        TEFile::DelFile(i->fin_fn);
      }
      IOlex2Processor* ip = IOlex2Processor::GetInstance();
      if (ip != 0) {
        TLog::Disabler ld = TBasicApp::GetInstance().GetLog().Disable();
        ip->processMacro("refresh");
      }
    }
    return Continue();
  }

}
