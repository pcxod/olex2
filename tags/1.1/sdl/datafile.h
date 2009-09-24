//---------------------------------------------------------------------------

#ifndef datafileH
#define datafileH

#include "ebase.h"
#include "estrlist.h"

BeginEsdlNamespace()

class TDataFile: public IEObject  {
protected:
  class TDataItem *FRoot;
  olxstr FileName;
public:
  TDataFile();
  virtual ~TDataFile();
  virtual bool LoadFromTextStream(IInputStream& io, TStrList* Log=NULL);
  virtual bool LoadFromXLFile(const olxstr& DataFile, TStrList* Log=NULL);
  virtual void SaveToXLFile(const olxstr& DataFile);
  virtual void Include(TStrList* Log);
  inline TDataItem& Root()          {  return *FRoot; }
  const olxstr& GetFileName() const {  return FileName;  }
};

EndEsdlNamespace()
#endif
