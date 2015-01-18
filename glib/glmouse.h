/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_glmouse_H
#define __olx_gl_glmouse_H
#include "glbase.h"
#include "tptrlist.h"
#include "library.h"
BeginGlNamespace()

// mouse listner constants
const short
  smbLeft   = 0x0001,
  smbMiddle = 0x0002,
  smbRight  = 0x0004,
  sssCtrl   = 0x0001,
  sssShift  = 0x0002,
  sssAlt    = 0x0004,
#ifdef __MAC__
  sssCmd    = 0x008
#else
  sssCmd    = sssCtrl
#endif
  ;
//  mouse move events
const short
  smeMoveXY   = 1,
  smeMoveZ    = 2,
  smeRotateXY = 3,
  smeRotateZ  = 4,
  smeZoom     = 5,
  smeSelect   = 6;
const short
  smeMouseUp = 1,
  smeMouseDown = 2,
  smeMouseMove = 4;
// mouse actions
const short
  glmaTranslateXY  = 1,
  glmaRotateXY     = 2,
  glmaRotateZ      = 3,
  glmaTranslateZ   = 4,
  glmaZoom         = 5;

//typedef void (*MMoveHandler)(class TGlMouse *, int dx, int dy);

struct TMouseData: public IOlxObject  {
  TMouseData() :
    Button(0), Shift(0), Event(0),
    DownX(0), DownY(0), UpX(0), UpY(0),
    Object(NULL)  {}
  virtual ~TMouseData()  {}
  short Button, // mouse button
    Shift,      // shift state
    Event;
  short DownX, DownY, // position of mouse when pressed down
        UpX, UpY,     // position of mouse when released
        X, Y;         // current position
  class AGDrawObject *Object;  // object under the mouse
  class TGlMouse *GlMouse;
};

struct TMouseRegion {
  int x, y;
  TMouseRegion(int x, int y) : x(x), y(y)
  {}
  int Compare(const TMouseRegion &r) const {
    int d = x - r.x;
    if (olx_abs(d) > 2)
      return d;
    d = y - r.y;
    if (olx_abs(d) > 2)
      return d;
    return 0;
  }
};

struct AMouseEvtHandler {
  short Button, Shift, Event;
  AMouseEvtHandler(short btn, short shift, short evt)
    : Button(btn), Shift(shift), Event(evt)
  {}
  virtual ~AMouseEvtHandler() {}
  virtual bool WillProcess(const TMouseData &md) const {
    return md.Button == Button && md.Shift == Shift && (md.Event&Event) != 0;
  }
  virtual bool operator == (const AMouseEvtHandler &eh) const {
    bool r = eh.Button == Button && eh.Shift == Shift && (Event&eh.Event) != 0;
    if (r && Event != eh.Event) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "overlapping mouse handlers");
    }
    return r;
  }
  virtual void Process(const TMouseData &md) = 0;
};

struct MouseEvtHandler {
  struct StaticMouseEvtHandler : public AMouseEvtHandler {
    void (*sf)(const TMouseData &md);
    StaticMouseEvtHandler(short btn, short shift, short evt,
      void (*sf_)(const TMouseData &))
      : AMouseEvtHandler(btn, shift, evt),
      sf(sf_)
    {}
    virtual void Process(const TMouseData &md) { (*sf)(md); }
  };

  template <class base_t>
  struct MemberMouseEvtHandler : public AMouseEvtHandler {
    void (base_t::*sf)(const TMouseData &md);
    base_t &instance;
    MemberMouseEvtHandler(short btn, short shift, short evt,
      base_t &inst, void (base_t::*sf_)(const TMouseData &))
      : AMouseEvtHandler(btn, shift, evt),
      instance(inst),
      sf(sf_)
    {}
    virtual void Process(const TMouseData &md) { (instance.*sf)(md); }
  };

  static StaticMouseEvtHandler &New(short btn, short shift, short evt,
      void (*sf)(const TMouseData &))
  {
    return *(new StaticMouseEvtHandler(btn, shift, evt, sf));
  }
  template <class base_t>
  static MemberMouseEvtHandler<base_t> &New(short btn, short shift, short evt,
    base_t &inst, void (base_t::*sf)(const TMouseData &))
  {
    return *(new MemberMouseEvtHandler<base_t>(btn, shift, evt, inst, sf));
  }
};

class TGlMouse: public IOlxObject {
  class TGlRenderer *FParent;
  class TDFrame *FDFrame;
  TActionQList Actions;
  static TGlMouse *Instance;
protected:
  int FSX, FSY;
  bool FDblClick;
  TPtrList<AMouseEvtHandler> Handlers;
  TMouseData MData;
  short Action;
  bool SelectionEnabled,
    RotationEnabled,
    TranslationEnabled,
    ZoomingEnabled,
    InMode;
  // to distinguish clicking on an object
  int ClickThreshold;
  void process_command_list(TStrObjList& Cmds, bool enable);
  olx_cdict<TMouseRegion, AGDrawObject *> object_cache;
  void OnObjectDelete(APerishable *o);
  AGDrawObject *find_object(int x, int y);
  void ClearObjectCache(IOlxObject *caller=NULL);
public:
  TGlMouse(TGlRenderer *Parent, TDFrame *Frame);
  virtual ~TGlMouse();

  bool MouseUp(int x, int y, short Shift, short button);
  bool DblClick();
  void ResetMouseState(short x, short y, short shift = 0, short button = 0,
    bool keep_object = false);
  bool MouseDown(int x, int y, short Shift, short button);
  bool MouseMove(int x, int y, short Shift);
  TGlRenderer* Parent() const {  return FParent;  }
  int SX() const {  return FSX;  }
  int SY() const {  return FSY;  }
  bool IsClick(const TMouseData &md) {
    return (olx_abs(md.DownX-md.UpX) <= ClickThreshold) &&
          (olx_abs(md.DownY-md.UpY) <= ClickThreshold);
  }
  const TMouseData &GetMouseData() { return MData; }
  // the pointer created with new is expected (use MouseEvtHandler)
  AMouseEvtHandler &SetHandler(AMouseEvtHandler &eh);
  // is set by handlers
  void SetAction(short A)  {  Action = A;  }
  /* find objects group. If selected or the parent group is selected - the
  selection group is returned
  */
  class TGlGroup* FindObjectGroup(const AGDrawObject& obj);
  DefPropBIsSet(SelectionEnabled)
  DefPropBIsSet(RotationEnabled)
  DefPropBIsSet(TranslationEnabled)
  DefPropBIsSet(ZoomingEnabled)
  DefPropBIsSet(InMode)
  static TGlMouse &GetInstance() {
    if (Instance == NULL) {
      throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
    }
    return *Instance;
  }
  TActionQueue &OnObject;
  void LibEnable(TStrObjList& Cmds, const TParamList& Options,
    TMacroData &E);
  void LibDisable(TStrObjList& Cmds, const TParamList& Options,
    TMacroData &E);
  void LibLock(TStrObjList& Cmds, const TParamList& Options,
    TMacroData &E);
  void LibIsEnabled(const TStrObjList& Params, TMacroData& E);
TLibrary *ExportLib(const olxstr &name="mouse");
};

// default behaviour to mouse events
void meMoveXY(const TMouseData &);
void meMoveZ(const TMouseData &);
void meRotateXY(const TMouseData &);
void meRotateZ(const TMouseData &);
void meZoom(const TMouseData &);
void meZoomI(const TMouseData &);

EndGlNamespace()
#endif
