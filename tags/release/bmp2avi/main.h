//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TdlgMain : public TForm
{
__published:	// IDE-managed Components
  TPanel *Panel1;
  TPanel *Panel2;
  TPanel *Panel3;
  TPanel *Panel4;
  TSplitter *Splitter1;
  TSplitter *Splitter2;
  TListView *lvFiles;
  TBitBtn *bbLoadFileList;
  TImage *iImage;
  TOpenDialog *dlgBmpLoad;
  TBitBtn *bbClearFileList;
  TSaveDialog *dlgSaveAVI;
  TBitBtn *bbSaveAVI;
  TLabeledEdit *leFps;
  void __fastcall bbLoadFileListClick(TObject *Sender);
  void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
  void __fastcall bbClearFileListClick(TObject *Sender);
  void __fastcall bbSaveAVIClick(TObject *Sender);
  void __fastcall lvFilesSelectItem(TObject *Sender, TListItem *Item,
          bool Selected);
private:	// User declarations
  void ClearFileList();
public:		// User declarations
  __fastcall TdlgMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgMain *dlgMain;
//---------------------------------------------------------------------------
#endif
