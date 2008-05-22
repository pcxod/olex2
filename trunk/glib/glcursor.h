//---------------------------------------------------------------------------

#ifndef glcursorH
#define glcursorH
#include "glbase.h"
#include "gdrawobject.h"
#include "actions.h"
#include "macroerror.h"

BeginGlNamespace()

class TGlCursor: public AGDrawObject, AEventsDispatcher  {
  float FX, FY;
  bool FTextStyle;
  short FFontIndex;
  class TGlPrimitive *FPrimitive;
  olxch Symbol;
protected:
  bool Dispatch( int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data=NULL);
public:
  TGlCursor(const olxstr& collectionName, TGlRender *Render, bool TextStyle = true);
  void Create(const olxstr& cName = EmptyString);
  virtual ~TGlCursor();

  void SetPosition(float x, float y)  {  FX=x;  FY=y;  }
  float X() const {  return FX;  }
  float Y() const {  return FY;  }

  class TGlFont *Font()  const;
  void FontIndex(short fi) {  FFontIndex = fi; }
  short FontIndex()  const {  return FFontIndex;  }

  bool IsText()  {  return FTextStyle;  }

  inline olxch GetSymbol() const {  return Symbol;  }
  void SetSymbol(olxch v);

  virtual bool Orient(TGlPrimitive *P);
  bool GetDimensions(TVPointD &Max, TVPointD &Min){  return false;};

  void LibSymbol(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name="cursor");
};

EndGlNamespace()
#endif
