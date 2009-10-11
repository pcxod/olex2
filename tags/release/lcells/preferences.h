//---------------------------------------------------------------------------

#ifndef preferencesH
#define preferencesH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TdlgPref : public TForm
{
__published:	// IDE-managed Components
	TListView *lvPaths;
	TBitBtn *bbRemove;
	TBitBtn *bbAdd;
	TListView *lvFiles;
	TBitBtn *bbRemoveF;
	TBitBtn *bbAddF;
	TBitBtn *bbOk;
	TOpenDialog *dlgOpen;
	TBitBtn *bbCancel;
	void __fastcall lvPathsSelectItem(TObject *Sender, TListItem *Item,
          bool Selected);
	void __fastcall lvFilesSelectItem(TObject *Sender, TListItem *Item,
          bool Selected);
	void __fastcall bbAddFClick(TObject *Sender);
	void __fastcall bbAddClick(TObject *Sender);
	void __fastcall bbRemoveClick(TObject *Sender);
	void __fastcall bbRemoveFClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TdlgPref(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TdlgPref *dlgPref;
//---------------------------------------------------------------------------
#endif
