//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <OleCtrls.hpp>
#include <Buttons.hpp>
#include <Dialogs.hpp>
#include <Menus.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include "zips.h"
#include "ciffile.h"
#include "draw_mol.h"
#include "actions.h"
//---------------------------------------------------------------------------
class LogListener : public AEventsDispatcher  {
protected:
  bool Dispatch( int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data=NULL);
  class TdlgMain* form;
public:
  LogListener(TdlgMain* f)  { form = f;  }
  virtual ~LogListener()  {}
  //ImplementICollectionItem()
};
class TdlgMain : public TForm
{
__published:  // IDE-managed Components
  TMainMenu *mMenu;
  TPanel *Panel1;
  TSplitter *Splitter1;
  TListView *lvList;
  TPanel *Panel2;
  TLabel *lFound;
  TMenuItem *miAbout;
  TMenuItem *miInfo;
  TPanel *Panel3;
  TLabel *Label1;
  TLabel *Label2;
  TLabel *Label3;
  TEdit *eC;
  TEdit *eB;
  TEdit *eA;
  TLabel *Label4;
  TLabel *Label5;
  TLabel *Label6;
  TLabel *Label7;
  TEdit *eDev;
  TEdit *eAC;
  TEdit *eAB;
  TEdit *eAA;
  TBitBtn *bbSearch;
  TSpeedButton *sbSaveAs;
  TSaveDialog *dlgSave;
  TLabel *Label8;
  TComboBox *cbLattice;
  TLabel *lNiggli;
  TSpeedButton *sbUpdateCell;
  TMenuItem *miPreferences;
  TMenuItem *miExit;
  TSpeedButton *sbView;
  TPanel *Panel5;
  TMenuItem *miSearch;
  TSpeedButton *sbOlex2;
  TPanel *Panel4;
  TRichEdit *mMemo;
  TTreeView *tvTree;
  TSplitter *Splitter2;
  TPopupMenu *pmTree;
  TMenuItem *Expandall1;
  TMenuItem *Collapseall1;
  void __fastcall bbSearchClick(TObject *Sender);
  void __fastcall lvListSelectItem(TObject *Sender, TListItem *Item,
      bool Selected);
  void __fastcall miAboutClick(TObject *Sender);
  void __fastcall miInfoClick(TObject *Sender);
  void __fastcall lvListDblClick(TObject *Sender);
  void __fastcall sbSaveAsClick(TObject *Sender);
  void __fastcall sbUpdateCellClick(TObject *Sender);
  void __fastcall miPreferencesClick(TObject *Sender);
  void __fastcall miExitClick(TObject *Sender);
  void __fastcall sbViewClick(TObject *Sender);
  void __fastcall miSearchClick(TObject *Sender);
  void __fastcall sbOlex2Click(TObject *Sender);
  void __fastcall tvTreeGetSelectedIndex(TObject *Sender, TTreeNode *Node);
  void __fastcall Expandall1Click(TObject *Sender);
  void __fastcall Collapseall1Click(TObject *Sender);
private:  // User declarations
  olxstr CurrentFile, Olex2Path;
  TStrList UpdatePaths,
           IndexFiles;
  void ClearTree();
  void InitTree();
  void LoadCurrentFile();
public:    // User declarations
  Graphics::TBitmap *Bitmap;
  TOrganiser *Organiser;
  TZipShell *Zip;
  TCifIndex *Index;
  __fastcall TdlgMain(TComponent* Owner);
  __fastcall ~TdlgMain();
  olxstr CurrentDir, TmpDir, IndexFile, IniFile, CIndexFile;
  void _fastcall DeleteDir(const olxstr& Dir, TStrList *SubDirs = NULL);
  bool _fastcall ReduceCell(TCell &C);
  void _fastcall ClearMessages();
  void _fastcall AddMessage(const olxstr& A);
  void _fastcall AddMessage(const TStrList& sl);
  __property TStrList Paths = {read = UpdatePaths};
  void _fastcall PostMsg(HWND H, const olxstr& Msg);
  void _fastcall AddPath(const olxstr& T);
  __property TStrList Indexes = {read = IndexFiles};
  void _fastcall AddResult(TCifFile *C);
  void _fastcall AddDummy(class TConFile *C);
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgMain *dlgMain;
//---------------------------------------------------------------------------
#endif

