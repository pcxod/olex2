//---------------------------------------------------------------------------

#ifndef dfameH
#define dfameH
#include "glbase.h"
#include "gdrawobject.h"
#include "vpoint.h"
#include "actions.h"

BeginGlNamespace()

// this class is passed a aparameter to the OSelect handlers
class TSelectionInfo: public IEObject  {
public:
  TVPointD From, To;
};

class TDFrame: public AGDrawObject  {
protected:
  class TGlRender *FRender;
  class TGlPrimitive *FPrimitive;
  TVPointD Translation;
  TActionQList *FActions;
public:
  TDFrame(const olxstr& collectionName, TGlRender *Render);
  virtual ~TDFrame();
  void Create(const olxstr& cName = EmptyString);
  bool Orient(TGlPrimitive *P);
  bool GetDimensions(TVPointD &Max, TVPointD &Min){  return false;};

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);
  TActionQueue *OnSelect;
};

EndGlNamespace()
#endif
