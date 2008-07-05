//---------------------------------------------------------------------------

#ifndef msearchH
#define msearchH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ExtCtrls.hpp>
#include <ActnList.hpp>
#include <Dialogs.hpp>
#include "estrlist.h"
#include "typelist.h"
#include "conindex.h"
//---------------------------------------------------------------------------
class TdlgSearch : public TForm
{
__published:  // IDE-managed Components
  TLabel *Label1;
  TSpeedButton *sbPaste;
  TBitBtn *bbSearch;
  TPanel *Panel1;
  TImage *iQuery;
  TActionList *ActionList1;
  TAction *aPaste;
  TBitBtn *bbLoad;
  TOpenDialog *dlgOpen;
  TBitBtn *bbUpdate;
  TBitBtn *bbClose;
  TLabeledEdit *eTitle;
  TBitBtn *bbSearchTitle;
  TLabel *Label2;
  TStaticText *stSize;
  TLabeledEdit *eSG;
  TBitBtn *bbSearchSG;
  TBitBtn *bbSearchIns;
  TLabeledEdit *eIns;
  void __fastcall sbPasteClick(TObject *Sender);
  void __fastcall bbSearchClick(TObject *Sender);
  void __fastcall aPasteExecute(TObject *Sender);
  void __fastcall bbLoadClick(TObject *Sender);
  void __fastcall bbUpdateClick(TObject *Sender);
  void __fastcall bbCloseClick(TObject *Sender);
  void __fastcall iQueryMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall iQueryMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y);
  void __fastcall iQueryMouseUp(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall iQueryClick(TObject *Sender);
  void __fastcall bbSearchTitleClick(TObject *Sender);
private:  // User declarations
  void _fastcall AddResults(const TTypeList<TConFile*>& Res);
public:    // User declarations
  class TOrganiser *Organiser;
  class TConIndex *CIndex;
  bool LoadMolecule(const olxstr& FN);
  __fastcall TdlgSearch(TComponent* Owner);
  __fastcall ~TdlgSearch();
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgSearch *dlgSearch;
//---------------------------------------------------------------------------
#endif

