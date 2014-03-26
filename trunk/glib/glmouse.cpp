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
  MData.GlMouse = this;
  InMode = FDblClick = false;
  FParent = Parent;
  SetHandler(MouseEvtHandler::New(smbLeft, 0, smeMouseMove, meRotateXY));
  SetHandler(MouseEvtHandler::New(smbLeft, sssCtrl, smeMouseMove, meRotateZ));
  SetHandler(MouseEvtHandler::New(smbLeft, sssShift|sssCtrl, smeMouseMove, meMoveXY));
  SetHandler(MouseEvtHandler::New(smbLeft|smbRight, 0, smeMouseMove, meMoveXY));
  //SetHandler(smbLeft, sssAlt, meMoveZ);
  if( !TBasicApp::GetInstance().GetOptions().FindValue(
    "mouse_invert_zoom", FalseString()).ToBool() )
  {
    SetHandler(MouseEvtHandler::New(smbRight, 0, smeMouseMove, meZoom));
    SetHandler(MouseEvtHandler::New(smbRight, sssAlt, smeMouseMove, meZoom));
  }
  else {
    SetHandler(MouseEvtHandler::New(smbRight, 0, smeMouseMove, meZoomI));
    SetHandler(MouseEvtHandler::New(smbRight, sssAlt, smeMouseMove, meZoomI));
  }
  ClickThreshold = TBasicApp::GetInstance().GetOptions()
    .FindValue("mouse_click_threshold", "2").ToInt();
  // an alternative for MAC...
  SetHandler(MouseEvtHandler::New(smbLeft, sssAlt, smeMouseMove, meZoom));
  SetHandler(MouseEvtHandler::New(smbLeft, sssAlt|sssShift, smeMouseMove, meRotateZ));
  FDFrame = Frame;
  TranslationEnabled = SelectionEnabled = RotationEnabled =
    ZoomingEnabled = true;
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
  MData.Event = smeMouseUp;
  MData.Shift = Shift;
  if( MData.Object != NULL )  {
    const bool is_click = IsClick(MData);
    if( MData.Object == FDFrame )
      res = MData.Object->OnMouseUp(this, MData);
    else  {
      if (InMode && button == smbLeft && Shift == 0 && is_click &&
          OnObject.Execute(this, MData.Object))
      {
        res = true;
      }
      else {
        TGlGroup *PColl = FindObjectGroup(*MData.Object);
        if( PColl != NULL )
          res = PColl->OnMouseUp(this, MData);
        else
          res = MData.Object->OnMouseUp(this, MData);
        if (res == false) {
          for( size_t i=0; i < Handlers.Count(); i++ )  {
            if (Handlers[i]->WillProcess(MData))  {
              Handlers[i]->Process(MData);
              res = true;
              break;
            }
          }
        }
        if (res == false && SelectionEnabled && Shift == 0 && button == smbLeft &&
            is_click )  // right click
        {
          if (PColl != NULL && PColl != &FParent->GetSelection())
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
  MData.Button &= ~button;
  MData.Object = NULL;
  return res;
}
//..............................................................................
bool TGlMouse::DblClick()  {
  FDblClick = true;
  return (MData.Object != NULL) ? MData.Object->OnDblClick(this, MData) : false;
}
//..............................................................................
void TGlMouse::ResetMouseState(short x, short y, short shift, short button) {
  FDblClick = false;
  MData.Button = button;
  MData.Shift = shift;
  FSX = MData.X = x;
  FSY = MData.Y = y;
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
  MData.Button |= button;
  MData.Shift = Shift;
  MData.DownX = x;
  MData.DownY = y;
  MData.Event = smeMouseDown;
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
  if (FDblClick) {
    FDblClick = false;
    MData.Button = MData.Shift = 0;
    MData.Object = NULL;
    return false;
  }
  MData.X = x;
  MData.Y = y;
  MData.Event = smeMouseMove;
  MData.Shift = Shift;
  bool res = false;
  if (MData.Object != NULL) {
    if (MData.Object == FDFrame) {
      res = MData.Object->OnMouseMove(this, MData);
    }
    else {
      TGlGroup *PColl = FindObjectGroup(*MData.Object);
      if (PColl != NULL) {
        if (PColl->OnMouseMove(this, MData))
          res = true;
      }
      else if( MData.Object->OnMouseMove(this, MData) )
        return true;
    }
  }
  if (!res) {
    // default handlers...
    for (size_t i=0; i < Handlers.Count(); i++) {
      if (Handlers[i]->WillProcess(MData)) {
        Handlers[i]->Process(MData);
        res = true;
        break;
      }
    }
  }
  FSX = x;
  FSY = y;
  return res;
}
//..............................................................................
AMouseEvtHandler &TGlMouse::SetHandler(AMouseEvtHandler &eh) {
  bool found = false;
  for (size_t i=0; i < Handlers.Count(); i++ ) {
    if (eh == *Handlers[i]) {
      Handlers.Delete(i);
      break;
    }
  }
  return *Handlers.Add(eh);
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
void GlobalGlFunction( meMoveXY(const TMouseData &md) )  {
  if (!md.GlMouse->IsTranslationEnabled()) return;
  TGlRenderer *R = md.GlMouse->Parent();
  const int dx = (md.X-md.GlMouse->SX()), dy = (md.GlMouse->SY()-md.Y);
  double v = R->GetScale();
  vec3d t = R->GetBasis().GetMatrix()*vec3d(dx*v, dy*v, 0);
  R->Translate(t/R->GetBasis().GetZoom());
  md.GlMouse->SetAction(glmaTranslateXY);
}
//..............................................................................
void GlobalGlFunction( meMoveZ(const TMouseData &md) )  {
  if (!md.GlMouse->IsTranslationEnabled()) return;
  TGlRenderer *R = md.GlMouse->Parent();
  const int dx = (md.X-md.GlMouse->SX()), dy = (md.GlMouse->SY()-md.Y);
  double v = R->GetScale();
  R->TranslateZ(dx*v + dy*v);
  md.GlMouse->SetAction(glmaTranslateZ);
}
//..............................................................................
void GlobalGlFunction( meRotateXY(const TMouseData &md) )  {
  if (!md.GlMouse->IsRotationEnabled()) return;
  TGlRenderer *R = md.GlMouse->Parent();
  const int dx = (md.X-md.GlMouse->SX()), dy = (md.GlMouse->SY()-md.Y);
  double RX = R->GetBasis().GetRX() + (double)dy/FRotationDiv;
  double RY = R->GetBasis().GetRY() + (double)dx/FRotationDiv;
  if (RX > 360)  RX = 0;
  else if (RX < 0)  RX = 360;
  if (RY > 360 )  RY = 0;
  else if (RY < 0 )  RY = 360;
  R->RotateX(RX);
  R->RotateY(RY);
  md.GlMouse->SetAction(glmaRotateXY);
}
//..............................................................................
void GlobalGlFunction( meRotateZ(const TMouseData &md) )  {
  if (!md.GlMouse->IsRotationEnabled()) return;
  TGlRenderer *R = md.GlMouse->Parent();
  const int dx = (md.X-md.GlMouse->SX()), dy = (md.GlMouse->SY()-md.Y);
  double RZ = R->GetBasis().GetRZ();
  if (md.GlMouse->SX() > R->GetWidth()/2)
    RZ -= (double)dy/FRotationDiv;
  else
    RZ += (double)dy/FRotationDiv;

  if (md.GlMouse->SY() > R->GetHeight()/2)
    RZ -= (double)dx/FRotationDiv;
  else
    RZ += (double)dx/FRotationDiv;

  if( RZ > 360 )  RZ = 0;
  if( RZ < 0 )    RZ = 360;
  R->RotateZ(RZ);
  md.GlMouse->SetAction(glmaRotateZ);
}
//..............................................................................
void GlobalGlFunction( meZoom(const TMouseData &md) )  {
  if (!md.GlMouse->IsZoomingEnabled()) return;
  TGlRenderer *R = md.GlMouse->Parent();
  const int dx = (md.X-md.GlMouse->SX()), dy = (md.GlMouse->SY()-md.Y);
  double df = 600;
  if ((md.Shift&sssAlt) != 0)
    df /= R->CalcZoom();
  R->SetZoom(R->GetZoom() + (double)dx/df - (double)dy/df);
  md.GlMouse->SetAction(glmaZoom);
}
//..............................................................................
void GlobalGlFunction( meZoomI(const TMouseData &md) )  {
  if (!md.GlMouse->IsZoomingEnabled()) return;
  TGlRenderer *R = md.GlMouse->Parent();
  const int dx = (md.X-md.GlMouse->SX()), dy = (md.GlMouse->SY()-md.Y);
  double df = 600;
  if ((md.Shift&sssAlt) != 0)
    df /= R->CalcZoom();
  R->SetZoom(R->GetZoom() - (double)dx/df + (double)dy/df);
  md.GlMouse->SetAction(glmaZoom);
}
//..............................................................................
