//---------------------------------------------------------------------------//
#ifndef xfilesH
#define xfilesH

#include "xbase.h"
#include "symmlib.h"

#include "refmodel.h"
#include "lattice.h"

BeginXlibNamespace()

// linked loop sort type
const short slltLength = 0x0001,  // sorts by bond length
            slltName   = 0x0002,  // sorts by atom name
            slltMw     = 0x0004;  // sorts by atom moleculr weight
            
class TBasicCFile: public ACollectionItem  {
private:
protected:
  olxstr FileName,  // file name if file is loaded
         Title;     // title of the file
  RefinementModel RefMod;
  TAsymmUnit AsymmUnit;
public:
  TBasicCFile();
  virtual ~TBasicCFile();

  const TAsymmUnit& GetAsymmUnit() const {  return AsymmUnit; }
  TAsymmUnit& GetAsymmUnit()             {  return AsymmUnit; }
  const RefinementModel& GetRM()   const {  return RefMod;  }
  RefinementModel& GetRM()               {  return RefMod;  }
  DefPropC(olxstr, Title)
  const olxstr& GetFileName()  const {  return FileName; }

  virtual void SaveToStrings(TStrList& Strings) = 0;
  virtual void LoadFromStrings(const TStrList& Strings) = 0;
  void SaveToFile(const olxstr &A);
  void LoadFromFile(const olxstr &A);
  // adopts the content of the AsemmUnit to the virtual format
  virtual bool Adopt(class TXFile *) = 0;
  virtual void DeleteAtom(TCAtom *A) = 0;
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TXFile: public AActionHandler {
private:
  TLattice Lattice;
  RefinementModel RefMod;
protected:
  TActionQList Actions;
  TStrPObjList<olxstr,TBasicCFile*> FileFormats;
  TBasicCFile *FLastLoader;
  TSpaceGroup* FSG;
  // on SG change of the asymmetric unit
  virtual bool Execute(const IEObject *Sender, const IEObject *Data);
public:
  TXFile();
  virtual ~TXFile();

  TActionQueue* OnFileLoad, *OnFileSave;

  inline const TLattice& GetLattice() const {  return Lattice;  }
  inline TLattice& GetLattice()             {  return Lattice;  }
  inline TUnitCell& GetUnitCell()     const {  return Lattice.GetUnitCell(); }
  const RefinementModel& GetRM()   const {  return RefMod;  }
  RefinementModel& GetRM()               {  return RefMod;  }
  inline TAsymmUnit& GetAsymmUnit()          const {  return Lattice.GetAsymmUnit(); }
  /* a propper pointer, created with new should be passed
   the object will be deleted in the destructor !! */
  void RegisterFileFormat(TBasicCFile *F, const olxstr &Ext);

  virtual IEObject* Replicate() const;
  /* the space group is initialised upon file loading
   if the space group is unknow, TFunctionFailedException is thrown
  */
  inline TSpaceGroup& GetLastLoaderSG() const {  
    if( FSG == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
    return *FSG; 
  }
  // returns file loader associated with given file extension
  TBasicCFile* FindFormat(const olxstr& Ext);
  // returns a reference to the last loader (type safe)
  template <class LoaderClass>
  inline LoaderClass& GetLastLoader()        const {  
    if( FLastLoader == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "no last loader");
    if( !EsdlInstanceOf(*FLastLoader, LoaderClass) )
      throw TInvalidArgumentException(__OlxSourceInfo, "wrong last loader type");
    return *(LoaderClass*)FLastLoader; 
  }
  inline void SetLastLoader(TBasicCFile* ll)       {  FLastLoader = ll; }
  // returns true if a file is loaded
  inline bool HasLastLoader()                const {  return FLastLoader != NULL; }
  // returns lat loader object to access properties of the base class if type is not required
  TBasicCFile* LastLoader()                  const {  return FLastLoader;  }
  void UpdateAsymmUnit();
  void LoadFromFile(const olxstr & FN);
  void SaveToFile(const olxstr & FN, bool Sort);
  // return 
  inline const olxstr& GetFileName() const {  return FLastLoader != NULL ? FLastLoader->GetFileName() : EmptyString; }

  void EndUpdate();
  void LastLoaderChanged();  // performs complete reinitialisation

  void ToDataItem(TDataItem& item);
  void FromDataItem(TDataItem& item);

  void LibGetFormula(const TStrObjList& Params, TMacroError& E);
  void LibSetFormula(const TStrObjList& Params, TMacroError& E);
  void LibEndUpdate(const TStrObjList& Params, TMacroError& E);
  void LibSaveSolution(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};
//---------------------------------------------------------------------------
EndXlibNamespace()
#endif
