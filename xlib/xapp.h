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
public:
  struct NameRef {
    size_t catom_id;
    olxstr name;
    NameRef(size_t id, const olxstr& n) : catom_id(id), name(n)  {  }
  };

  TNameUndo(IUndoAction* action) : TUndoData(action)  {  }
  
  TTypeList<NameRef> Data;
  
  void AddAtom(TCAtom& A, const olxstr& newName)  {
    Data.Add( new NameRef(A.GetId(), newName) );
  }
  inline size_t AtomCount() const {  return Data.Count();  }
  inline size_t GetCAtomId(size_t i) const {  return  Data[i].catom_id;  }
  inline const olxstr& GetLabel(size_t i) const {  return  Data[i].name;  }
};

                     
class TXApp : public TBasicApp, public ALibraryContainer  {
protected:
  TXFile *FXFile;
  TLibrary Library;
  olxstr CifTemplatesDir;  // the folder with CIF templates/data
  ASelectionOwner* SelectionOwner;
protected:
  virtual bool CheckProgramState(unsigned int specialCheck);
  void ProcessRingAfix(TSAtomPList& ring, int afix, bool pivot_last);
public:
  TXApp(const olxstr &basedir, ASelectionOwner* selOwner=NULL);
  virtual ~TXApp();
  inline TXFile& XFile() const {  return *FXFile; }
  
  DefPropC(olxstr, CifTemplatesDir)

  virtual class TLibrary&  GetLibrary()  {  return Library;  }

  // locates related HKL file, processes raw or hkc file if necessary
  olxstr LocateHklFile();

  // fills the list with the matrices of the UnitCell
  void GetSymm(smatd_list& ml) const;

  template <class FT>
    bool CheckFileType() const {
      if( !FXFile->HasLastLoader() )  return false;
      return EsdlInstanceOf(*FXFile->LastLoader(), FT);
    }

  static TXApp& GetInstance()  {
    TBasicApp& bai = TBasicApp::GetInstance();
    TXApp& xai = dynamic_cast<TXApp&>(bai);
    if( &xai == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "unsuitabe application instance");
    return xai;
  }
  // calculates structure factors for current structure, F.Count must be greater or equal to the ref.Count
  void CalcSF(const TRefList& refs, TArrayList<TEComplex<double> >& F);
  /* function undoes renaming atoms */
  void undoName(TUndoData *data);
  void NameHydrogens(TSAtom& a, TUndoData* ud, bool CheckLabel);
  // fixes hydrogen atom labels
  TUndoData* FixHL();
  void RingContentFromStr(const olxstr& textDescr, ElementPList& ringDesc);
  void FindRings(const olxstr& Condition, TTypeList<TSAtomPList>& rings);
  //
  virtual bool FindSAtoms(const olxstr& condition, TSAtomPList& res, bool ReturnAll = true, bool ClearSelection=true);
  // fins Cp, Ph, Naph and Cp* rings and adds corresponding afixes
  void AutoAfixRings(int afix, TSAtom* sa = NULL, bool TryPyridine = false);
  void SetAtomUiso(TSAtom& sa, double val);

  static ElementRadii ReadVdWRadii(const olxstr& fileName);
  static void PrintVdWRadii(const ElementRadii& radii, const ContentList& au_cont);
  template <class AtomType>  // could be either TCAtom or TSAtom
  static double GetVdWRadius(const AtomType& a, const ElementRadii* radii)  {
    const size_t ei = radii == NULL ? InvalidIndex : radii->IndexOf(&a.GetType());
    return ei == InvalidIndex ? a.GetType().r_sfil : radii->GetValue(ei);
  }

  struct CalcVolumeInfo  {
    double total, overlapping;
    CalcVolumeInfo(double _total, double _overlapping) : total(_total), overlapping(_overlapping)  {}
  };
  CalcVolumeInfo CalcVolume(const ElementRadii* radii);

  template <class atom_list> static void UnifyPAtomList(atom_list& alist) {
    const size_t ac = alist.Count();
    for( size_t i=0; i < ac; i++ )
      alist[i]->SetTag(i);
    for( size_t i=0; i < ac; i++ )
      if( alist[i]->GetTag() != i || alist[i]->IsDeleted() )
        alist[i] = NULL;
    alist.Pack();
  }

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
};



#endif
