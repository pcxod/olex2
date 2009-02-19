#ifndef gllabelH
#define gllabelH
#include "gxbase.h"

#include "glmouselistener.h"
#include "glfont.h"
#include "dataitem.h"

BeginGxlNamespace()

class TXGlLabel: public TGlMouseListener  {
  olxstr FLabel;
  short FFontIndex;
  double OffsetX, OffsetY;
public:
  TXGlLabel(const olxstr& collectionName, TGlRenderer *Render);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TXGlLabel();

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;  }
  inline const olxstr& GetLabel() const   {  return FLabel;  }
  void SetLabel(const olxstr& L);

  TGlFont *Font() const;
  inline void FontIndex(short FntIndex)  {  FFontIndex = FntIndex; }
  inline short FontIndex() const         {  return FFontIndex; }

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item);
};

EndGxlNamespace()
#endif
 
