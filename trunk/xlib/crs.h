#ifndef crsFileH
#define crsFileH

#include "xfiles.h"

// STOE "P4P" file
BeginXlibNamespace()

class TCRSFile : public TBasicCFile  {
  evecd_list Faces;
  double Radiation;
  olxstr Sfac, Unit;
  bool SGInitialised;
public:
  TCRSFile();
  virtual ~TCRSFile();

  const double& GetRadiation()  const  {  return Radiation;  }

  class TSpaceGroup* GetSG();

  inline const olxstr& GetSfac()  const  {  return Sfac;  }
  inline const olxstr& GetUnit()  const  {  return Unit;  }

  inline size_t FacesCount()  const  {  return Faces.Count();  }
  inline const evecd& GetFace(size_t i)  const  {  return Faces[i];  }

  bool IsSGInitialised() const {  return SGInitialised;  }

  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *);
  virtual IEObject* Replicate() const {  return new TCRSFile();  }
  virtual void DeleteAtom(TCAtom *A)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
};

EndXlibNamespace()
#endif
