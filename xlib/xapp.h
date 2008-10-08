#ifndef _xl_xappH
#define _xl_xappH

#include "bapp.h"
#include "xfiles.h"
#include "library.h"
#include "reflection.h"
#include "ecomplex.h"
#include "undo.h"
#include "arrays.h"
#include "atomref.h"

// program state and some other special checks for functions
const unsigned int   psFileLoaded        = 0x00010000,
                     psCheckFileTypeIns  = 0x00020000,
                     psCheckFileTypeCif  = 0x00040000,
                     psCheckFileTypeP4P  = 0x00080000,
                     psCheckFileTypeCRS  = 0x00100000;

class TNameUndo : public TUndoData  {
  TTypeList< AnAssociation2<TSAtom*, olxstr> >  Data;
public:
  TNameUndo(IUndoAction* action) : TUndoData(action)  {  }

  void AddAtom( TSAtom& A, const olxstr& newName )  {
    Data.AddNew( &A, newName );
  }
  inline int AtomCount()                   const {  return Data.Count();  }
  inline TSAtom& GetAtom(int i)            const {  return  *Data[i].GetA();  }
  inline const olxstr& GetLabel(int i)   const {  return  Data[i].GetB();  }
};

                     
class TXApp : public TBasicApp, public ALibraryContainer  {
protected:
  TXFile *FXFile;
  TAtomsInfo* FAtomsInfo;
  TLibrary Library;
  olxstr CifTemplatesDir;  // the folder with CIF templates/data
  ASelectionOwner* SelectionOwner;
protected:
  virtual bool CheckProgramState(unsigned int specialCheck);
  void ProcessRingAfix(TSAtomPList& ring, int afix, bool pivot_last);
public:
  TXApp(const olxstr &basedir, ASelectionOwner* selOwner=NULL);
  virtual ~TXApp();
  inline TAtomsInfo* AtomsInfo() {  return FAtomsInfo; }
  inline TXFile& XFile()         {  return *FXFile; }
  
  DefPropC(olxstr, CifTemplatesDir)

  virtual class TLibrary&  GetLibrary()  {  return Library;  }

  // locates related HKL file, processes raw or hkc file if necessary
  const olxstr& LocateHklFile();

  template <class FT>
    bool CheckFileType()  {
      if( !XFile().HasLastLoader() )  return false;
      return EsdlInstanceOf(*XFile().LastLoader(), FT);
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
  /*calculates a grid for the voids/squeeze analysis, extraR is the extra atomic radius 
  The resulting map contains Levels, 0 for the surface, 1 - one pixel off the survface etc.
  returns the maximum level reached.  The void center is assigned to the point coordinates with
  largest level*/
  short CalcVoid(TArray3D<short>& map, double extraR, short val, long* structurePoints, 
    vec3d& voidCenter, TPSTypeList<TBasicAtomInfo*, double>* radii);
  /* function undoes renaming atoms */
  void undoName(TUndoData *data);
  void NameHydrogens(TSAtom& a, TUndoData* ud, bool CheckLabel);
  // fixes hydrogen atom labels
  TUndoData* FixHL();
  void RingContentFromStr(const olxstr& textDescr, TPtrList<TBasicAtomInfo>& ringDesc);
  void FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings);
  // 
  virtual bool FindSAtoms(const olxstr& condition, TSAtomPList& res, bool ReturnAll = true, bool ClearSelection=true);
  // fins Cp, Ph, Naph and Cp* rings and adds corresponding afixes
  void AutoAfixRings(int afix, TSAtom* sa = NULL, bool TryPyridine = false);

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
};



#endif
