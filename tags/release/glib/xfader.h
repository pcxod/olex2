#ifndef xfader_H
#define xfader_H

#include "glbase.h"
#include "gdrawobject.h"

#include "library.h"

BeginGlNamespace()

class TXFader: public AGDrawObject  {
  char *Background, *Foreground;
  int BGWidth, BGHeight;
  int FGWidth, FGHeight;
  double Step, Position;
public:
  TXFader(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TXFader();
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }

  void InitBG(bool init=true);
  void InitFG(bool init=true);
  void BG2FG(bool zeropos=true);
  DefPropP(double, Step)
  DefPropP(double, Position)
  bool Increment()  {
    Position += Step;
    if( Position > 1 )  {
      Position = 1;
      return false;
    }
    return true;
  }

  void LibStep(const TStrObjList& Params, TMacroError& E);
  void LibPosition(const TStrObjList& Params, TMacroError& E);
  void LibInitFG(const TStrObjList& Params, TMacroError& E);
  void LibInitBG(const TStrObjList& Params, TMacroError& E);
  void LibBG2FG(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name="fader");
};

EndGlNamespace()
#endif
