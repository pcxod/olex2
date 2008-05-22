//---------------------------------------------------------------------------//
#ifndef xyzH
#define xyzH

#include "xfiles.h"

BeginXlibNamespace()

// linked loop sort type
class TXyz: public TBasicCFile  {
private:
  void Clear();
public:
  TXyz(TAtomsInfo *S);
  virtual ~TXyz();

  virtual void SaveToStrings(TStrList& Strings);
  virtual void LoadFromStrings(const TStrList& Strings);
  virtual bool Adopt(TXFile *XF);
  void DeleteAtom(TCAtom *CA);

  virtual IEObject* Replicate()  const {  return new TXyz(AtomsInfo);  }
};

EndXlibNamespace()
#endif
 
