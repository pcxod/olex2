#ifndef obaseH
#define obaseH
#include "estlist.h"
#include "xatom.h"

// mouse modes
const unsigned short mmNone   = 0;
// program states
const unsigned short prsNone      = 0, // modes
                     prsStrVis    = 0x0001, // Structure visible/invisible
                     prsHVis      = 0x0002,        // hydroges visible invisible
                     prsHBVis     = 0x0004,      // hydrogen bonds visible/invisible
                     prsQVis      = 0x0008,        // qpeaks visible/invisible
                     prsQBVis     = 0x0010,       // qpeak bonds visible/invisible
                     prsCellVis   = 0x0020,     // cell visible/invisible
                     prsBasisVis  = 0x0040,    // basis visible/invisible
                     prsHtmlVis     = 0x0080,       // popup window visible/invisible
                     prsHtmlTTVis   = 0x0100,       // popup window tooltips visible/invisible
                     prsBmpVis    = 0x0200,       // glBitmap visible/invisible
                     prsPluginInstalled = 0x0400,       // plugin installed
                     prsHelpVis = 0x0800,       // help window visible
                     prsInfoVis = 0x1000,       // info window visible
                     prsCmdlVis = 0x2000,       // command line visible
                     prsGradBG  = 0x4000;       // gradient bg view

//---------------------------------------------------------------------------
class AMode : public IEObject  {
protected:
  int Id;
public:
  AMode(int id);
  virtual ~AMode();
  // mode initialisation
  virtual bool Init(TStrObjList &Cmds, const TParamList &Options) = 0;
  //an action to be exected then any particular object is selected/clicked
  virtual bool OnObject(AGDrawObject &obj) = 0;
  // if the mode holds any reference to graphical objects - this should be cleared
  virtual void OnGraphicsDestroy()  {  return;  }
  //if the mode processes the key - true should be returned to skip the event
  virtual bool OnKey(int keyId, short shiftState)  {  return false;  }
  // if the function supported - returns true
  virtual bool AddAtoms(const TPtrList<TXAtom>& atoms) {  return false;  }
  inline int GetId()  const  {  return Id;  }
};
//..............................................................................
class AModeWithLabels : public AMode  {
  short LabelsMode;
  bool LabelsVisible;
public:
  AModeWithLabels(int id);
  ~AModeWithLabels();
};
//..............................................................................

class AModeFactory  {
protected:
  int modeId;
public:
  AModeFactory(int id) : modeId(id)  {}
  virtual ~AModeFactory()  {}
  virtual AMode* New() = 0;
};

template <class ModeClass>
  class TModeFactory : public AModeFactory  {
  public:
    TModeFactory(int id) : AModeFactory(id) {}
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
  static unsigned short DecodeMode( const olxstr& mode );
  AMode* GetCurrent()  {  return CurrentMode;  }
};

class TModeChange: public IEObject  {
  bool FStatus;
  unsigned short Mode;
public:
  TModeChange(const unsigned short mode, bool status) : FStatus(status), Mode(mode) {}
  ~TModeChange()  {  }
  bool GetStatus() const {  return FStatus;  }
  static bool CheckStatus( const olxstr& mode, const olxstr& modeData=EmptyString );
  static bool CheckStatus( unsigned short mode, const olxstr& modeData=EmptyString );
};
//..............................................................................
class TStateChange: public IEObject  {
  bool FStatus;
  unsigned short State;
public:
  TStateChange(unsigned short state, bool status);
  static bool CheckStatus(const olxstr& stateName, const olxstr& stateData=EmptyString);
  static bool CheckStatus(unsigned short state, const olxstr& stateData=EmptyString);

  static unsigned short DecodeState( const olxstr& mode );
};

#endif
