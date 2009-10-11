//---------------------------------------------------------------------------

#ifndef _xl_dgenoptH
#define _xl_dgenoptH
#include "wx/wx.h"
#include "ctrls.h"
//---------------------------------------------------------------------------

class TdlgGenerate: public TDialog, AActionHandler
{
private:
  wxStaticText *stAFrom, *stBFrom, *stCFrom, *stATo, *stBTo, *stCTo;
  wxTextCtrl *tcAFrom, *tcBFrom, *tcCFrom, *tcATo, *tcBTo, *tcCTo;
  TComboBox *cbA, *cbB, *cbC;

protected:
  void OnOK(wxCommandEvent& event);
  void OnLinkChanges(wxCommandEvent& event);
  void OnAChange();
  void OnBChange();
  void OnCChange();
  bool Execute(const IEObject *Sender, const IEObject *Data=NULL);

  float FAFrom, FBFrom, FCFrom, FATo, FBTo, FCTo;
public:
  TdlgGenerate(TMainFrame *ParentFrame);
  virtual ~TdlgGenerate();

//..............................................................................
// properties

//..............................................................................
// interface
  float AFrom(){  return FAFrom; }
  float BFrom(){  return FBFrom; }
  float CFrom(){  return FCFrom; }
  void AFrom( float a) {  FAFrom = a; }
  void BFrom( float a) {  FBFrom = a; }
  void CFrom( float a) {  FCFrom = a; }

  float ATo(){  return FATo; }
  float BTo(){  return FBTo; }
  float CTo(){  return FCTo; }
  void ATo(float a){  FATo = a; }
  void BTo(float a){  FBTo = a; }
  void CTo(float a){  FCTo = a; }
//..............................................................................
  DECLARE_EVENT_TABLE()
};
#endif
