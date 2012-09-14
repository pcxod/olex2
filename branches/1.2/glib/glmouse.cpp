/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glmouse.h"
#include "bapp.h"
#include "glrender.h"
#include "dframe.h"
#include "glgroup.h"

TGlMouse::TGlMouse(TGlRenderer *Parent, TDFrame *Frame)
  : OnObject(Actions.New("ObObject"))
{
  FSX = FSY = 0;
  InMode = FDblClick = FButtonDown = false;
  FParent = Parent;
  SetHandler(smbLeft, 0, meRotateXY);
  SetHandler(smbLeft, sssCtrl, meRotateZ);
  SetHandler(smbLeft, sssShift|sssCtrl, meMoveXY);
  SetHandler(smbLeft|smbRight, 0, meMoveXY);
  //SetHandler(smbLeft, sssAlt, meMoveZ);
  if( !TBasicApp::GetInstance().GetOptions().FindValue(
    "mouse_invert_zoom", FalseString()).ToBool() )
  {
    SetHandler(smbRight, 0, meZoom);
  }
  else
    SetHandler(smbRight, 0, meZoomI);
  ClickThreshold = TBasicApp::GetInstance().GetOptions()
    .FindValue("mouse_click_threshold", "2").ToInt();
  // an alternative for MAC...
  SetHandler(smbLeft, sssAlt, meZoom);
  FDFrame = Frame;
  SelectionEnabled = RotationEnabled = ZoomingEnabled = true;
}
//..............................................................................
TGlMouse::~TGlMouse()  {
  Handlers.DeleteItems(false);
}
//..............................................................................
bool TGlMouse::MouseUp(int x, int y, short Shift, short button)  {
  bool res = false;
  MData.UpX = x;
  MData.UpY = y;
  MData.Shift = Shift; 
  MData.Button = button;
  if( MData.Object != NULL )  {
    bool is_click = (olx_abs(MData.DownX-MData.UpX) <= ClickThreshold) &&
          (olx_abs(MData.DownY-MData.UpY) <= ClickThreshold);
    if( MData.Object == FDFrame )
      res = MData.Object->OnMouseUp(this, MData);
    else  {
      if (InMode && button == smbLeft && Shift == 0 && is_click &&
          OnObject.Execute(this, MData.Object))
      {
        ;
      }
      else {
        TGlGroup *PColl = FindObjectGroup(*MData.Object);
        if( PColl != NULL ) 
          res = PColl->OnMouseUp(this, MData);
        else
          res = MData.Object->OnMouseUp(this, MData);
        if( res == false && SelectionEnabled && Shift == 0 && button == smbLeft &&
            is_click )  // right click
        {
          if( PColl != NULL && PColl != &FParent->GetSelection() ) 
            FParent->Select(*PColl); 
          else 
            FParent->Select(*MData.Object);
          FParent->Draw();
          res = true;
        }
      }
    }
  }
  else  {
    if( Action == glmaRotateXY )  {
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
  MData.Object = NULL;
  FButtonDown = false;
  return res;
}
//..............................................................................
bool TGlMouse::DblClick()  {
  FDblClick = true;
  return (MData.Object != NULL) ? MData.Object->OnDblClick(this, MData) : false;
}
//..............................................................................
void TGlMouse::ResetMouseState()  {
  FButtonDown = false;
  FDblClick = false;
  MData.Object = NULL;
}
//..............................................................................
TGlGroup* TGlMouse::FindObjectGroup(const AGDrawObject& obj)  {
  if( obj.IsSelected() )
    return &FParent->GetSelection();
  else  {
    TGlGroup* p = FParent->FindObjectGroup(obj);
    if( FParent->GetSelection().Contains(*p) ) 
      return &FParent->GetSelection();
    return p;
  }
  return NULL;
}
//..............................................................................
bool TGlMouse::MouseDown(int x, int y, short Shift, short button)  {
  bool res = false;
  FButtonDown = true;
  MData.Button = button;
  MData.Shift = Shift;
  MData.DownX = x;
  MData.DownY = y;
  MData.Object = FParent->SelectObject(x, y);
  if( MData.Object != NULL )  {
    TGlGroup *PColl = FindObjectGroup(*MData.Object);
    if( PColl != NULL )
      res = PColl->OnMouseDown(this, MData);
    else 
      res = MData.Object->OnMouseDown(this, MData);
  }
  if( res == false && Shift == sssShift )  {
    MData.Object = FDFrame;
    res = MData.Object->OnMouseDown(this, MData);
  }
  FSX = x;
  FSY = y;
  return res;
}
//..............................................................................
bool TGlMouse::MouseMove(int x, int y, short Shift)  {
  if( FDblClick )  {
    FButtonDown = false;
    FDblClick = false;
    MData.Object = NULL;
    return false;
  }
  const int DX = (x-FSX), DY = (FSY-y);
  FSX = x;
  FSY = y;
  MData.X = x;
  MData.Y = y;
  MData.Shift = Shift;
  if( MData.Object != NULL)  {
    if( MData.Object == FDFrame )  {
      return MData.Object->OnMouseMove(this, MData);
    }
    else  {
      TGlGroup *PColl = FindObjectGroup(*MData.Object);
      if( PColl != NULL )  {
        if( PColl->OnMouseMove(this, MData) )
          return true;
      }
      else if( MData.Object->OnMouseMove(this, MData) )
        return true;
    }
  }
  // default handlers...
  for( size_t i=0; i < Handlers.Count(); i++ )  {
    TGlMMoveEvent* ME = Handlers[i];
    if( (ME->Button == MData.Button) && (ME->Shift == MData.Shift) )  {
      if( FButtonDown && ME->ButtonDown )  {
        ME->Handler(this, DX, DY);
        return true;
      }
    }
  }
  return false;
}
//..............................................................................
void TGlMouse::SetHandler(short Button, short Shift, MMoveHandler MH)  {
  bool found = false;
  for( size_t i=0; i < Handlers.Count(); i++ )  {
    TGlMMoveEvent* ME = Handlers[i];
    if( (ME->Button == Button) && (ME->Shift == Shift) )  {
      ME->Handler = MH;
      found = true;
      break;
    }
  }
  if( !found )  {
    TGlMMoveEvent* ME = new TGlMMoveEvent;
    ME->Button = Button;
    ME->Shift = Shift;
    ME->Handler = MH;
    Handlers.Add(ME);
  }
}
//..............................................................................
void TGlMouse::process_command_list(TStrObjList& Cmds, bool enable) {
  for (size_t i=0; i < Cmds.Count(); i++) {
    if (Cmds[i].Equalsi("selection"))
      SetSelectionEnabled(enable);
    else if (Cmds[i].Equalsi("rotation"))
      SetRotationEnabled(enable);
    else if (Cmds[i].Equalsi("translation"))
      SetTranslationEnabled(enable);
    else if (Cmds[i].Equalsi("zooming"))
      SetZoomingEnabled(enable);
  }
}
//..............................................................................
void TGlMouse::LibEnable(TStrObjList& Cmds, const TParamList& Options,
  TMacroError &E)
{
  process_command_list(Cmds, true);
}
//..............................................................................
void TGlMouse::LibDisable(TStrObjList& Cmds, const TParamList& Options,
  TMacroError &E)
{
  process_command_list(Cmds, false);
}
//..............................................................................
void TGlMouse::LibLock(TStrObjList& Cmds, const TParamList& Options,
  TMacroError &E)
{
  bool v = Cmds.IsEmpty() ? false : !Cmds[0].ToBool();
  SetRotationEnabled(v);
  SetTranslationEnabled(v);
  SetZoomingEnabled(v);
}
//..............................................................................
void TGlMouse::LibIsEnabled(const TStrObjList& Cmds, TMacroError& E) {
  if (Cmds[0].Equalsi("selection"))
    E.SetRetVal(IsSelectionEnabled());
  else if (Cmds[0].Equalsi("rotation"))
    E.SetRetVal(IsRotationEnabled());
  else if (Cmds[0].Equalsi("translation"))
    E.SetRetVal(IsTranslationEnabled());
  else if (Cmds[0].Equalsi("zooming"))
    E.SetRetVal(IsZoomingEnabled());
}
//..............................................................................
TLibrary *TGlMouse::ExportLib(const olxstr &name) {
  TLibrary *lib = new TLibrary(name);
  lib->Register(
    new TMacro<TGlMouse>(this,  &TGlMouse::LibEnable, "Enable",
      EmptyString(), fpAny^fpNone,
      "Enables one of the following operations: rotation, zooming, translation"
      ", selection")
  );
  lib->Register(
    new TMacro<TGlMouse>(this,  &TGlMouse::LibDisable, "Disable",
      EmptyString(), fpAny^fpNone,
      "Disables one of the following operations: rotation, zooming, translation"
      ", selection")
  );
  lib->Register(
    new TMacro<TGlMouse>(this,  &TGlMouse::LibLock, "Lock",
      EmptyString(), fpNone|fpOne,
      "[Disables]/enables rotation, zooming and translation")
  );
  lib->Register(
    new TFunction<TGlMouse>(this,  &TGlMouse::LibIsEnabled, "IsEnabled",
      fpOne,
      "Returns current status for rotation, zooming, translation or selection")
  );
  return lib;
}
//..............................................................................
//..............................................................................
//..............................................................................
void GlobalGlFunction( meMoveXY(TGlMouse *G, int dx, int dy) )  {
  if (!G->IsTranslationEnabled()) return;
  TGlRenderer *R = G->Parent();
  double v = R->GetScale();
  vec3d t = R->GetBasis().GetMatrix()*vec3d(dx*v, dy*v, 0);
  R->Translate(t/R->GetBasis().GetZoom());
  G->SetAction(glmaTranslateXY);
}
//..............................................................................
void GlobalGlFunction( meMoveZ(TGlMouse *G, int dx, int dy) )  {
  if (!G->IsTranslationEnabled()) return;
  TGlRenderer *R = G->Parent();
  double v = R->GetScale();
  R->TranslateZ(dx*v + dy*v);
  G->SetAction(glmaTranslateZ);
}
//..............................................................................
void GlobalGlFunction( meRotateXY(TGlMouse *G, int dx, int dy) )  {
  if (!G->IsRotationEnabled()) return;
  TGlRenderer *R = G->Parent();
  double RX = R->GetBasis().GetRX() + (double)dy/FRotationDiv;;
  double RY = R->GetBasis().GetRY() + (double)dx/FRotationDiv;;
  if( RX > 360 )  RX = 0;
  if( RX < 0 )    RX = 360;
  if( RY > 360 )  RY = 0;
  if( RY < 0 )    RY = 360;
  R->RotateX(RX);
  R->RotateY(RY);
  G->SetAction(glmaRotateXY);
}
//..............................................................................
void GlobalGlFunction( meRotateZ(TGlMouse *G, int dx, int dy) )  {
  if (!G->IsRotationEnabled()) return;
  TGlRenderer *R = G->Parent();
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
  G->SetAction(glmaRotateZ);
}
//..............................................................................
void GlobalGlFunction( meZoom(TGlMouse *G, int dx, int dy) )  {
  if (!G->IsZoomingEnabled()) return;
  TGlRenderer *R = G->Parent();
  static const double df = 600;
  R->SetZoom(R->GetZoom() + (double)dx/df - (double)dy/df);
  G->SetAction(glmaZoom);
}
//..............................................................................
void GlobalGlFunction( meZoomI(TGlMouse *G, int dx, int dy) )  {
  if (!G->IsZoomingEnabled()) return;
  TGlRenderer *R = G->Parent();
  static const double df = 600;
  R->SetZoom(R->GetZoom() - (double)dx/df + (double)dy/df);
  G->SetAction(glmaZoom);
}
//..............................................................................
