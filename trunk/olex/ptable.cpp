/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "ptable.h"

TPTableDlg::TPTableDlg(TMainFrame *Parent) :
  TDialog(Parent, wxT("Periodic table"), EsdlClassName(TPTableDlg).u_str() )
{
  AActionHandler::SetToDelete(false);
  for( int i=0; i < 9; i++ )  {
    for( int j=0; j < 18; j++ )  {
      if( i == 0 )  {
        if( j == 0 )
          CreateButton(i,j,0);
        if( j == 17 )
          CreateButton(i,j,0);
      }
      else if( i < 3 )  {
        if( j <= 1 || j >= 12 )
          CreateButton(i,j,0);
      }
      else if( i < 6 )  {
        CreateButton(i,j,0);
      }
      else if( i == 6 )  {
        if( j <= 3  )
          CreateButton(i,j,0);
      }
      else if( i >= 6 )  {  // lantinides and actinides
        if( j <= 14  )
          CreateButton(i,j,5);
      }
    }
  }
  CreateButton(8, 17, 5 );  // q peak

  for( size_t i=iQPeakIndex+1; i <= iMaxElementIndex; i++ )
    CreateButton(9, i-iQPeakIndex-1, 5);  // q peak
  wxDialog::Fit();
  wxDialog::Center();
}
TPTableDlg::~TPTableDlg()  {
  for( size_t i =0; i < ButtonsList.Count(); i++ )  {
    ButtonsList[i]->OnClick.Clear();
    ButtonsList[i];
  }
}
//..............................................................................
void TPTableDlg::CreateButton(int i, int j, int offset )  {
  TButton *B = new TButton(this); // q peak
#ifndef __WIN32__
  short bd = 36;
#else
  short bd = 22;
#endif
  B->WI.SetLeft( j*(bd+1));
  B->WI.SetTop(   i*(bd+1)+offset);
  B->WI.SetWidth( bd );
  B->WI.SetHeight( bd );
  ButtonsList.Add(B);
  int ii = ButtonsList.Count();
  B->SetTag( ii-1 );
  if( ii == 57 || ii == 75 )
    return;
  if( ii > 57 && ii < 75 )  {
    ii += 14;
      goto data;
  }
  if( ii > 75  && ii <= 76 )  {  // 76 - last element Ku
    ii += 28;
    goto data;
  }
  if( ii > 76 && ii <= 91)  // lanthanides
    ii -= 20;
  if( ii > 91 && ii < 107)  // lanthanides and q peak
    ii -= 3;

data:
  if( ii >= 107 )
    ii-=2;
  B->SetCaption(XElementLib::GetByIndex(ii-1).symbol);
//  B->Font->Color = A->Color;
  B->OnClick.Add(this);
  B->SetTag( ii );
}
//..............................................................................
bool TPTableDlg::Execute(const IEObject *Sender, const IEObject *Data,
  TActionQueue *)
{
  TButton *S = (TButton*)(AOlxCtrl*)Sender;
  Selected = &XElementLib::GetByIndex(S->GetTag()-1);
  wxDialog::EndModal(wxID_OK);
  return true;
}
