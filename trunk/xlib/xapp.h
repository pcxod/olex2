#ifndef _xl_xappH
#define _xl_xappH

#include "bapp.h"
#include "xfiles.h"
#include "library.h"
#include "reflection.h"
#include "ecomplex.h"

// program state and some other special checks for functions
const unsigned int   psFileLoaded        = 0x00010000,
                     psCheckFileTypeIns  = 0x00020000,
                     psCheckFileTypeCif  = 0x00040000,
                     psCheckFileTypeP4P  = 0x00080000,
                     psCheckFileTypeCRS  = 0x00100000;
                     
class TXApp : public TBasicApp, public ALibraryContainer  {
protected:
  TXFile *FXFile;
  TAtomsInfo* FAtomsInfo;
  TLibrary Library;
protected:
  virtual bool CheckProgramState(unsigned int specialCheck);
public:
  TXApp(const olxstr &basedir);
  virtual ~TXApp();
  inline TAtomsInfo* AtomsInfo() {  return FAtomsInfo; }
  inline TXFile& XFile()         {  return *FXFile; }

  virtual class TLibrary&  GetLibrary()  {  return Library;  }

  // locates related HKL file, processes raw or hkc file if necessary
  const olxstr& LocateHklFile();

  template <class FT>
    bool CheckFileType()  {
      if( XFile().GetLastLoader() == NULL )  return false;
      return EsdlInstanceOf(*XFile().GetLastLoader(), FT);
    }

  static TXApp& GetInstance()  {
    TBasicApp *bai = TBasicApp::GetInstance();
    if( bai == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "unitialised application");
    TXApp *xai = dynamic_cast<TXApp*>(bai);
    if( xai == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "unsuitabe application instance");
    return *xai;
  }
  // calculates structure factors for current structure, F.Count must be greater or equal to the ref.Count
  void CalcSF(const TRefList& refs, TArrayList<TEComplex<double> >& F);
};



#endif
