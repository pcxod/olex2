//---------------------------------------------------------------------------

#ifndef dfameH
#define dfameH
#include "glbase.h"
#include "gdrawobject.h"
#include "actions.h"

BeginGlNamespace()

// this class is passed a aparameter to the OSelect handlers
class TSelectionInfo: public IEObject  {
public:
  vec3d From, To;
};

class TDFrame: public AGDrawObject  {
protected:
  class TGlRenderer *FRender;
  class TGlPrimitive *FPrimitive;
  vec3d Translation;
  TActionQList *FActions;
public:
  TDFrame(const olxstr& collectionName, TGlRenderer *Render);
  virtual ~TDFrame();
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;};

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);
  TActionQueue *OnSelect;
};

EndGlNamespace()
#endif
