#ifndef __olx_base_H
#define __olx_base_H
#include "estlist.h"
#include "xatom.h"

// mouse modes
const unsigned short mmNone   = 0;
// program states
const uint32_t 
  prsNone            = 0x00000000, // modes
  prsStrVis          = 0x00000001, // Structure visible/invisible
  prsHVis            = 0x00000002, // hydroges visible invisible
  prsHBVis           = 0x00000004, // hydrogen bonds visible/invisible
  prsQVis            = 0x00000008, // qpeaks visible/invisible
  prsQBVis           = 0x00000010, // qpeak bonds visible/invisible
  prsCellVis         = 0x00000020, // cell visible/invisible
  prsBasisVis        = 0x00000040, // basis visible/invisible
  prsHtmlVis         = 0x00000080, // popup window visible/invisible
  prsHtmlTTVis       = 0x00000100, // popup window tooltips visible/invisible
  prsBmpVis          = 0x00000200, // glBitmap visible/invisible
  prsPluginInstalled = 0x00000400, // plugin installed
  prsHelpVis         = 0x00000800, // help window visible
  prsInfoVis         = 0x00001000, // info window visible
  prsCmdlVis         = 0x00002000, // command line visible
  prsGradBG          = 0x00004000, // gradient bg view
  prsLabels          = 0x00008000, // labels vusible/hidden
  prsGLTT            = 0x00010000; // GLTooltip 

//---------------------------------------------------------------------------
class AMode : public IEObject  {
protected:
  size_t Id;
public:
  AMode(size_t id);
  virtual ~AMode();
  // mode initialisation
  virtual bool Initialise(TStrObjList &Cmds, const TParamList &Options) = 0;
  virtual void Finalise() = 0;
  //an action to be exected then any particular object is selected/clicked
  virtual bool OnObject(AGDrawObject &obj) = 0;
  // if the mode holds any reference to graphical objects - this should be cleared
  virtual void OnGraphicsDestroy()  {}
  //if the mode processes the key - true should be returned to skip the event
  virtual bool OnKey(int keyId, short shiftState)  {  return false;  }
  // if the function supported - returns true
  virtual bool AddAtoms(const TPtrList<TXAtom>& atoms) {  return false;  }
  inline size_t GetId() const {  return Id;  }
};
//..............................................................................
class AModeWithLabels : public AMode  {
  short LabelsMode;
  bool LabelsVisible;
public:
  AModeWithLabels(size_t id);
  ~AModeWithLabels();
};
//..............................................................................

class AModeFactory  {
protected:
  size_t modeId;
public:
  AModeFactory(size_t id) : modeId(id)  {}
  virtual ~AModeFactory()  {}
  virtual AMode* New() = 0;
};

template <class ModeClass>
  class TModeFactory : public AModeFactory  {
  public:
    TModeFactory(size_t id) : AModeFactory(id) {}
    virtual AMode* New()  {  return new ModeClass(modeId);  }
  };

class TModes  {
  TSStrPObjList<olxstr,AModeFactory*, true> Modes;
  AMode *CurrentMode;
  static TModes *Instance;
public:
  TModes();
  ~TModes();
  // NULL is returned of no mode foind
  AMode* SetMode(const olxstr& name);
  void ClearMode(bool finalise);
  static size_t DecodeMode( const olxstr& mode );
  AMode* GetCurrent()  {  return CurrentMode;  }
};

class TModeChange: public IEObject  {
  bool FStatus;
  size_t Mode;
public:
  TModeChange(size_t mode, bool status) : FStatus(status), Mode(mode) {}
  ~TModeChange()  {  }
  bool GetStatus() const {  return FStatus;  }
  static bool CheckStatus(const olxstr& mode, const olxstr& modeData=EmptyString);
  static bool CheckStatus(size_t mode, const olxstr& modeData=EmptyString );
};
//..............................................................................
class TStateChange: public IEObject  {
  bool FStatus;
  uint32_t State;
  olxstr Data;
public:
  TStateChange(uint32_t state, bool status, const olxstr& data=EmptyString);
  // string representation of the state
  inline bool GetStatus() const {  return FStatus;  }
  inline uint32_t GetState() const {  return State;  }
  inline const olxstr& GetData() const {  return Data;  }
  static bool CheckStatus(const olxstr& stateName, const olxstr& stateData=EmptyString);
  static bool CheckStatus(uint32_t state, const olxstr& stateData=EmptyString);

  static uint32_t DecodeState( const olxstr& mode );
  static olxstr StrRepr(uint32_t state);
};

#endif
