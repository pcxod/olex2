//---------------------------------------------------------------------------

#ifndef sync_dlgH
#define sync_dlgH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <ImgList.hpp>
#include <ToolWin.hpp>
#include "filetree.h"
#include <Menus.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TdlgSync : public TForm
{
__published:	// IDE-managed Components
  TTreeView *tvFrom;
  TToolBar *ToolBar1;
  TImageList *ilImages;
  TStatusBar *sbBar;
  TPopupMenu *pmMenu;
  TMenuItem *miSkip;
  TMenuItem *miUpdate;
  TSpeedButton *sbRun;
  void __fastcall tvFromChange(TObject *Sender, TTreeNode *Node);
  void __fastcall miSkipClick(TObject *Sender);
  void __fastcall miUpdateClick(TObject *Sender);
  void __fastcall tvFromMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
  void __fastcall sbRunClick(TObject *Sender);
private:	// User declarations
  int GetFileNodeState(TFileTree::DiffFolder& fn);
  void FileNodeToTreeNode(TFileTree::DiffFolder& fn, TTreeView& tv, TTreeNode* tn);
  void ClearTree();
  void SetAction(TTreeNode& nd, int action, bool int_call = false);
  void CalcSize();
  int StateToAction(int v);
public:		// User declarations
  __fastcall TdlgSync(TComponent* Owner);
  __fastcall ~TdlgSync();

  void Init(TFileTree& src, TFileTree& dest);
  TFileTree::DiffFolder* Root;
  uint64_t TotalSize;
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgSync *dlgSync;
//---------------------------------------------------------------------------
#endif
