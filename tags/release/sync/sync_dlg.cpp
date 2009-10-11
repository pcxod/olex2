//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "sync_dlg.h"
#include "etime.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TdlgSync *dlgSync;

const int
  iiUptoDate = 0,
  iiNew      = 1,
  iiOutdated = 2,
  iiUpdated  = 3,
  iiSkip     = 4,
  iiUpdate   = 5;

struct NodeData  {
  TFileTree::DiffFolder &folder;
  int index, action;
  NodeData(TFileTree::DiffFolder& _folder, int ind = -1, int _action = iiUptoDate) :
    folder(_folder), index(ind), action(_action)  {}
};
//---------------------------------------------------------------------------
__fastcall TdlgSync::TdlgSync(TComponent* Owner)
  : TForm(Owner)
{
  Root = NULL;
  TotalSize = 0;
}
//---------------------------------------------------------------------------
__fastcall TdlgSync::~TdlgSync()  {
  ClearTree();
}
//---------------------------------------------------------------------------
int TdlgSync::StateToAction(int v)  {
  return v == iiNew ? iiUpdate : iiSkip;
}
//---------------------------------------------------------------------------
int TdlgSync::GetFileNodeState(TFileTree::DiffFolder& fn)  {
  if( fn.Src != NULL && fn.Dest == NULL )
    return iiNew;
  else  {
    if( fn.Src == NULL )
      return iiUptoDate;
    if( fn.Dest->IsEmpty() && !fn.Src->IsEmpty() )
      return iiNew;
    if( fn.Dest->IsEmpty() && fn.Src->IsEmpty() )
      return iiUptoDate;
    for( int i=0; i < fn.SrcFiles.Count(); i++ )  {
      if( fn.SrcFiles[i] != NULL )  {
        if( fn.DestFiles[i] != NULL )  {
          if (fn.SrcFiles[i]->GetModificationTime() < fn.DestFiles[i]->GetModificationTime())
            return iiUpdated;  //iiOutdated;
          if (fn.SrcFiles[i]->GetModificationTime() > fn.DestFiles[i]->GetModificationTime())
            return iiUpdated;
        }
        else
          return iiUpdated;
      }
      else
        return iiUpdated; //iiOutdated;
    }
    for( int i=0; i < fn.Folders.Count(); i++ )  {
      int rv = GetFileNodeState(fn.Folders[i]);
      if( rv != iiUptoDate )
        return rv;
    }
  }
  return iiUptoDate;
}
//---------------------------------------------------------------------------
void TdlgSync::FileNodeToTreeNode(TFileTree::DiffFolder& fn, TTreeView& tv, TTreeNode* tn)  {
  TTreeNode* this_node = (tn != NULL) ?
   this_node = tv.Items->AddChild(tn, (fn.Src != NULL) ? fn.Src->GetName().u_str() : fn.Dest->GetName().u_str())
  :
   this_node = tv.Items->Add(tn, (fn.Src != NULL) ? fn.Src->GetName().u_str() : fn.Dest->GetName().u_str());
  this_node->ImageIndex = GetFileNodeState(fn);
  this_node->SelectedIndex = this_node->ImageIndex;
  this_node->Data = new NodeData(fn, -1, this_node->ImageIndex);
  for( int i=0; i < fn.Folders.Count(); i++ )
    FileNodeToTreeNode(fn.Folders[i], tv, this_node);
  for( int i=0; i < fn.SrcFiles.Count(); i++ )  {
    TTreeNode* nn = tv.Items->AddChild(this_node,
      (fn.SrcFiles[i] != NULL) ? fn.SrcFiles[i]->GetName().u_str() : fn.DestFiles[i]->GetName().u_str());
    if( fn.SrcFiles[i] != NULL )  {
      if( fn.DestFiles[i] != NULL )  {
        if (fn.SrcFiles[i]->GetModificationTime() < fn.DestFiles[i]->GetModificationTime())
          nn->ImageIndex = iiOutdated;
        else if (fn.SrcFiles[i]->GetModificationTime() != fn.DestFiles[i]->GetModificationTime())
          nn->ImageIndex = iiUpdated;
        else
          nn->ImageIndex = iiUptoDate;
      }
      else
        nn->ImageIndex = iiNew;
    }
    else
      nn->ImageIndex = -1;
    nn->SelectedIndex = nn->ImageIndex;
    nn->Data = new NodeData(fn, i, StateToAction(nn->ImageIndex));
  }
}
//---------------------------------------------------------------------------
void TdlgSync::ClearTree()  {
  for( int i=0; i < tvFrom->Items->Count; i++ )
    delete (NodeData*)tvFrom->Items->Item[i]->Data;
  tvFrom->Items->Clear();
  if( Root != NULL )  {
    delete Root;
    Root = NULL;
  }
}
//---------------------------------------------------------------------------
void TdlgSync::Init(TFileTree& src, TFileTree& dest)  {
  ClearTree();
  Root = new TFileTree::DiffFolder();
  src.CalcMergedTree(dest, *Root);
  FileNodeToTreeNode(*Root, *tvFrom, NULL);
  CalcSize();
}
//---------------------------------------------------------------------------
void TdlgSync::CalcSize()  {
  uint64_t sz = 0;
  for( int i=0; i < tvFrom->Items->Count; i++ )  {
    NodeData& nd = *(NodeData*)tvFrom->Items->Item[i]->Data;
    if( nd.index != -1 && (nd.action == iiNew || nd.action == iiUpdate) )
      sz += nd.folder.Src->GetFile(nd.index).GetSize();
  }
  double sz_mb = sz/(1024*1024);
  sbBar->Panels->Items[0]->Text = AnsiString("Merge size: ") + AnsiString::FormatFloat("0.00", sz_mb) + " Mb";
  TotalSize = sz;
}
//---------------------------------------------------------------------------
void __fastcall TdlgSync::tvFromChange(TObject *Sender, TTreeNode *Node)  {
  NodeData* nd = (NodeData*)Node->Data;
  if( nd->index == -1 )
    return;
  if( Node->ImageIndex == iiUpdated )  {
    sbBar->Panels->Items[1]->Text =
      AnsiString("Updated: ") +
      (TETime::FormatDateTime("yyyy-MM-dd hh:mm:ss", nd->folder.SrcFiles[nd->index]->GetModificationTime()) +
      " -> " +
      TETime::FormatDateTime("yyyy-MM-dd hh:mm:ss", nd->folder.DestFiles[nd->index]->GetModificationTime())).u_str();
  }
  else if( Node->ImageIndex == iiOutdated )  {
    sbBar->Panels->Items[1]->Text =
      AnsiString("Outdated: ") +
      (TETime::FormatDateTime("yyyy-MM-dd hh:mm:ss", nd->folder.SrcFiles[nd->index]->GetModificationTime()) +
      " -> " +
      TETime::FormatDateTime("yyyy-MM-dd hh:mm:ss", nd->folder.DestFiles[nd->index]->GetModificationTime())).u_str();
  }
  else
    sbBar->Panels->Items[1]->Text = "Upto date";
}
//---------------------------------------------------------------------------
void TdlgSync::SetAction(TTreeNode& node, int action, bool int_call)  {
  NodeData* nd = (NodeData*)node.Data;
  if( node.ImageIndex == iiUptoDate )
    return;
  if( !int_call )
    tvFrom->Items->BeginUpdate();
  nd->action = action;
  node.ImageIndex = action;
  node.SelectedIndex = action;
  if( nd->index == -1 )  {  // folder?
    for( int i=0; i < node.Count; i++ )
      SetAction( *node.Item[i], action, true );
  }
  if( action == iiUpdate )  {  // modify the parents as well
    TTreeNode* p = &node;
    while( (p = p->Parent) != NULL )  {
      NodeData* d = (NodeData*)p->Data;
      d->action = action;
      p->ImageIndex = action;
      p->SelectedIndex = action;
    }
  }
  if( !int_call )  {
    CalcSize();
    tvFrom->Items->EndUpdate();
  }
}
//---------------------------------------------------------------------------
void __fastcall TdlgSync::miSkipClick(TObject *Sender)  {
  tvFrom->Items->BeginUpdate();
  for( int i=0; i < tvFrom->SelectionCount; i++ )
    SetAction(*tvFrom->Selections[i], iiSkip, true);
  CalcSize();
  tvFrom->Items->EndUpdate();
}
//---------------------------------------------------------------------------
void __fastcall TdlgSync::miUpdateClick(TObject *Sender)  {
  tvFrom->Items->BeginUpdate();
  for( int i=0; i < tvFrom->SelectionCount; i++ )
    SetAction(*tvFrom->Selections[i], iiUpdate, true);
  CalcSize();
  tvFrom->Items->EndUpdate();
}
//---------------------------------------------------------------------------
void __fastcall TdlgSync::tvFromMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if( Button == mbRight )  {
    TTreeNode* node = tvFrom->GetNodeAt(X,Y);
    if( node )
      node->Selected = true;
  }
}
//---------------------------------------------------------------------------
void __fastcall TdlgSync::sbRunClick(TObject *Sender)  {
  // prepare data
  for( int i=0; i < tvFrom->Items->Count; i++ )  {
    TTreeNode* node = tvFrom->Items->Item[i];
    NodeData& nd = *(NodeData*)node->Data;
    if( nd.folder.Src == NULL )
      continue;
    if( nd.index == -1 )  {
      if( nd.index == iiUptoDate )  // folder?
        nd.folder.Src = NULL;
    }
    else if( nd.action != iiUpdate )  {
      nd.folder.Src->NullFileEntry(nd.index);
    }
  }
  ModalResult = mrOk;
}
//---------------------------------------------------------------------------
