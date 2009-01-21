//----------------------------------------------------------------------------//
// namespace TGlObj: TGlMouse - basic implementation of mouse behaviour
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifdef _MSC_VER
  #define MSVCC(a) ((double)(a))
#else
  #define MSVCC(a) (a)
#endif

#include "glmouse.h"
#include "elist.h"
#include "glrender.h"
#include "dframe.h"
#include "glgroup.h"

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlMouse::TGlMouse(TGlRender *Parent, TDFrame *Frame)  {
  FSX = FSY = 0;
  FDblClick = FButtonDown = false;
  FParent = Parent;
  SetHandler( smbLeft, 0, meRotateXY);
  SetHandler( smbLeft, sssCtrl, meRotateZ);
  //SetHandler( smbLeft, sssShift, meMoveXY);
  //SetHandler( smbLeft, sssAlt, meMoveZ);

  SetHandler( smbRight, 0, meZoom);
  Handler = NULL;
  MData = new TMouseData;
  FDFrame = Frame;
  SelectionEnabled = true;
}
//..............................................................................
TGlMouse::~TGlMouse()  {
  for( int i=0; i < Handlers.Count(); i++ )
    delete Handlers[i];
  delete MData;
}
//..............................................................................
bool TGlMouse::MouseUp(int x, int y, short Shift, short button)  {
  bool res = false;
  TGlGroup *PColl;
  MData->UpX = x; MData->UpY = y;
  MData->Shift = Shift; MData->Button = button;
  if( Handler != NULL )  {
    if( Handler == FDFrame )  {
      FDFrame->OnMouseUp(this, MData);
    }
    else  {
      PColl = FParent->FindObjectGroup(Handler);
      if( PColl ) { PColl->OnMouseUp(this, MData);    }
      else        { Handler->OnMouseUp(this, MData);  }
      if( (olx_abs(MSVCC(MData->DownX-MData->UpX))<=2) &&
          (olx_abs(MSVCC(MData->DownY-MData->UpY))<=2) && (Shift==0) )  // click
      {
        if( PColl ) { FParent->Select(PColl);      }
        else {  FParent->Select(Handler); }
        FParent->Draw();
        res = true;
      }
    }
  }
  else  {
    if( FAction == glmaRotateXY )  {
/*      float Length = 24;
      float Accel=2, Time=1, Path = Length;
      while( Path > 0 )
      {
        Path -= Accel*Accel*Time;
        Time++;
        Parent()->RotateX(Parent()->Basis().RX() + Path);
        Parent()->RotateY(Parent()->Basis().RY() + Path);
        Parent()->Draw();
      }                  */
      ;
    }
  }
  Handler = NULL;
  FButtonDown = false;
  return res;
}
//..............................................................................
bool TGlMouse::DblClick()  {
  FDblClick = true;
  return (Handler!=NULL) ? Handler->OnDblClick(this, MData) : false;
}
//..............................................................................
void TGlMouse::ResetMouseState()  {
  FButtonDown = false;
  FDblClick = false;
  Handler = NULL;
}
//..............................................................................
bool TGlMouse::MouseDown(int x, int y, short Shift, short button)  {
  TGlGroup *PColl;
  FButtonDown = true;
  MData->Button = button; MData->Shift = Shift;
  MData->DownX = x;       MData->DownY = y;
  if( SelectionEnabled )  {
    Handler = FParent->SelectObject(x, y);
    if( Handler )  {
      if( Handler->Selected() )
        PColl = FParent->Selection();
      else  {
        PColl = FParent->FindObjectGroup(Handler);
        if( FParent->Selection()->Contains(PColl) ) PColl = FParent->Selection();
      }
      if( PColl )  {
        PColl->OnMouseDown(this, MData);
      }
      else  {
        if( !Handler->OnMouseDown(this, MData) )
          Handler = NULL;
      }
    }
    if( !Handler && Shift == sssShift )  {
      Handler = FDFrame;
      //    FParent->UpdateGlImage();
      Handler->OnMouseDown(this, MData);
    }
  }
  FSX = x;
  FSY = y;
  return false;
}
//..............................................................................
bool TGlMouse::MouseMove(int x, int y, short Shift)  {
  if( FDblClick )  {
    FButtonDown = false;
    FDblClick = false;
    Handler = NULL;
    return false;
  }
  TGlGroup *PColl;
  TGlMMoveEvent *ME;
  int DX = (x-FSX), DY = (FSY-y);
  FSX = x;      FSY = y;
  MData->X = x; MData->Y = y; MData->Shift = Shift;
  if( Handler )  {
    if( Handler == FDFrame )  {
      FDFrame->OnMouseMove(this, MData);
//      FParent->DrawObject(FDFrame, true);
//      FParent->DrawObject();
      return true;
    }
    else  {
      if( Handler->Selected() )
        PColl = FParent->Selection();
      else  {
        PColl = FParent->FindObjectGroup(Handler);
        if( FParent->Selection()->Contains(PColl) ) PColl = FParent->Selection();
      }
      if( PColl )  {
        PColl->OnMouseMove(this, MData);
        return true;
      }
      else  {
        if( Handler->OnMouseMove(this, MData) )      return true;
      }
    }
  }
  // default handlers...
  for( int i=0; i < Handlers.Count(); i++ )  {
    ME = Handlers[i];
    if( (ME->Button == MData->Button) && (ME->Shift == MData->Shift) )  {
      if( FButtonDown && ME->ButtonDown )  {
        ME->Handler(this, DX, DY);
        return true;
      }
    }
  }
  return false;
}
//..............................................................................
void TGlMouse::SetHandler( const short Button, const short Shift, MMoveHandler MH)  {
  bool found = false;
  TGlMMoveEvent *ME;
  for( int i=0; i < Handlers.Count(); i++ )  {
    ME = Handlers[i];
    if( (ME->Button == Button) && (ME->Shift == Shift) )  {
      ME->Handler = MH;
      found = true;
      break;
    }
  }
  if( !found )  {
    ME = new TGlMMoveEvent;
    ME->Button = Button;
    ME->Shift = Shift;
    ME->Handler = MH;
    Handlers.Add(ME);
  }
}
//..............................................................................
void GlobalGlFunction( meMoveXY(TGlMouse *G, int dx, int dy) )  {
  TGlRender *R = G->Parent();
  double v = R->GetScale();
  R->TranslateX(dx*v);
  R->TranslateY(dy*v);
  G->Action(glmaTranslateXY);
}
//..............................................................................
void GlobalGlFunction( meMoveZ(TGlMouse *G, int dx, int dy) )  {
  TGlRender *R = G->Parent();
  double v = R->GetScale();
  R->TranslateZ(dx*v + dy*v);
  G->Action(glmaTranslateZ);
}
//..............................................................................
void GlobalGlFunction( meRotateXY(TGlMouse *G, int dx, int dy) )  {
  TGlRender *R = G->Parent();
  double RX = R->GetBasis().GetRX() + (double)dy/FRotationDiv;;
  double RY = R->GetBasis().GetRY() + (double)dx/FRotationDiv;;
  if( RX > 360 )  RX = 0;
  if( RX < 0 )    RX = 360;
  if( RY > 360 )  RY = 0;
  if( RY < 0 )    RY = 360;
  R->RotateX(RX);
  R->RotateY(RY);
  G->Action(glmaRotateXY);
}
//..............................................................................
void GlobalGlFunction( meRotateZ(TGlMouse *G, int dx, int dy) )  {
  TGlRender *R = G->Parent();
  double RZ = R->GetBasis().GetRZ();
  if( G->SX() > R->GetWidth()/2 )
    RZ -= (double)dy/FRotationDiv;
  else
    RZ += (double)dy/FRotationDiv;

  if( G->SY() > R->GetHeight()/2 )
    RZ -= (double)dx/FRotationDiv;
  else
    RZ += (double)dx/FRotationDiv;

  if( RZ > 360 )  RZ = 0;
  if( RZ < 0 )    RZ = 360;
  R->RotateZ(RZ);
  G->Action(glmaRotateZ);
}
//..............................................................................
void GlobalGlFunction( meZoom(TGlMouse *G, int dx, int dy) )  {
  TGlRender *R = G->Parent();
  R->SetZoom( R->GetZoom() + (double)dx/FZoomDiv - (double)dy/FZoomDiv);
  G->Action(glmaZoom);
}
//..............................................................................

 
