#include "tasks.h"
#include "xglapp.h"
#include "mainform.h"
#include "msgbox.h"

void P4PTask::Run() {
  if (file_id.Contains("Rigaku CrystalClear")) {
    TMainForm *mf = TGlXApp::GetMainForm();
    olxstr cmd = "spy.xplain.exists()";
    mf->ProcessFunction(cmd, __OlxSrcInfo);
    if (cmd.IsBool() && cmd.ToBool()) {
      olxstr opt =
        TBasicApp::GetInstance().Options.FindValue("run_rigaku_xplain");
      bool run_xplain=false;
      if (opt.IsEmpty()) {
        olxstr rv = TdlgMsgBox::Execute(mf, "Would you like to run "
          "Rigaku XPlain for the space group determination?",
          "Rigaku P4P file import",
          "Do this for all Rigaku P4P files",
          wxYES|wxNO|wxICON_QUESTION, true);
        run_xplain = rv.Contains('Y');
        if (rv.Containsi('R'))
          mf->UpdateUserOptions("run_rigaku_xplain", run_xplain);
      }
      else {
        run_xplain = opt.ToBool();
      }
      if (run_xplain) {
        if (mf->ProcessMacro("spy.xplain.run(false, true)", __OlxSrcInfo)) {
          mf->ProcessMacro("reap", __OlxSrcInfo);
        }
      }
    }
  }
}

void CellChangeTask::Run() {
  TMainForm *mf = TGlXApp::GetMainForm();
  olxstr opt = TBasicApp::GetInstance().Options.FindValue("use_hkl_cell");
  bool use = false;
  if (opt.IsEmpty()) {
    olxstr rv = TdlgMsgBox::Execute(mf, "Cell parameters in your HKL file"
      " differ from currently used. Would you like to update them?\nNote that "
      "if you answer no, this dialog will not appear until the HKL file is "
      "changed or Olex2 is restarted",
      "Different cells",
      "Always use cell from the HKL file",
      wxYES|wxNO|wxICON_QUESTION, true);
    use = rv.Contains('Y');
    if (rv.Containsi('R'))
      mf->UpdateUserOptions("use_hkl_cell", use);
  }
  if (use) {
    TAsymmUnit &au = TXApp::GetInstance().XFile().GetAsymmUnit();
    TAsymmUnit &au1 =
      TXApp::GetInstance().XFile().LastLoader()->GetAsymmUnit();
    au1.GetAxes() = au.GetAxes() = vec3d(cell[0], cell[1], cell[2]);
    au1.GetAngles() = au.GetAngles() = vec3d(cell[3], cell[4], cell[5]);
    au1.GetAxisEsds() = au.GetAxisEsds() = vec3d(esds[0], esds[1], esds[2]);
    au1.GetAngleEsds() = au.GetAngleEsds() = vec3d(esds[3], esds[4], esds[5]);
    TBasicApp::NewLogEntry() <<
      "Updating cell parameters with value from HKL file";
  }
}
