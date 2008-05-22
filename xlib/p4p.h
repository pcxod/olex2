#ifndef p4pH
#define p4pH

#include "xfiles.h"

BeginXlibNamespace()

class TP4PFile : public TBasicCFile  {
  olxstr Size,
           Color,
           SiteId,
           Morph,
           Chem,
           Mosaic,
           Source,
           Symm,
           Temp,
           Bravais,
           SG;
  double Radiation;
public:
  TP4PFile();
  virtual ~TP4PFile();

  const olxstr& GetSize()    const {  return Size;  }
  const olxstr& GetColor()   const {  return Color;  }
  const olxstr& GetSiteId()  const {  return SiteId;  }
  const olxstr& GetMorph()   const {  return Morph;  }
  const olxstr& GetTemp()    const {  return Temp;  }
  const olxstr& GetSource()  const {  return Source;  }
  const double GetRadiation()  const {  return Radiation;  }
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
