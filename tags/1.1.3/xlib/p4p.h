#ifndef __olx_xl_p4p_H
#define __olx_xl_p4p_H
#include "xfiles.h"

BeginXlibNamespace()

class TP4PFile : public TBasicCFile  {
  olxstr   Color,
           SiteId,
           Morph,
           Mosaic,
           Symm,
           Bravais,
           SGString;
public:
  TP4PFile();
  virtual ~TP4PFile();

  const olxstr& GetColor() const {  return Color;  }
  const olxstr& GetSiteId() const {  return SiteId;  }
  const olxstr& GetMorph() const {  return Morph;  }
  // this is set when the file is manually exported ...
  DefPropC(olxstr, SGString)
  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile&);
  virtual IEObject* Replicate() const {  return new TP4PFile();  }
};

EndXlibNamespace()
#endif