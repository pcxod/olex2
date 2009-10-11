#ifndef p4pH
#define p4pH

#include "xfiles.h"

BeginXlibNamespace()

class TP4PFile : public TBasicCFile  {
  olxstr   Color,
           SiteId,
           Morph,
           Chem,
           Mosaic,
           Symm,
           Bravais,
           SG;
public:
  TP4PFile();
  virtual ~TP4PFile();

  const olxstr& GetColor()   const {  return Color;  }
  const olxstr& GetSiteId()  const {  return SiteId;  }
  const olxstr& GetMorph()   const {  return Morph;  }
  const olxstr& GetChem()    const {  return Chem;  }
  void SetChem(const olxstr& c)    {  Chem = c;  }
  // this is set when the file is manually exported ...
  const olxstr& GetSG()     const {  return SG;  }
  void SetSG(const olxstr& c)     {  SG = c;  }

  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *);
  virtual IEObject* Replicate() const {  return new TP4PFile();  }
  virtual void DeleteAtom(TCAtom *A)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
};

EndXlibNamespace()
#endif
