//---------------------------------------------------------------------------//
#ifndef xfilesH
#define xfilesH

#include "xbase.h"
#include "symmlib.h"

#include "asymmunit.h"
#include "lattice.h"

BeginXlibNamespace()

// linked loop sort type
const short slltLength = 0x0001,  // sorts by bond length
            slltName   = 0x0002,  // sorts by atom name
            slltMw     = 0x0004;  // sorts by atom moleculr weight
            
class TXFile;
class TBasicCFile;
class TCif;
class TLinkedLoopTable;
class TIns;
class THklFile;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TXFile: public AActionHandler {
private:
  class TLattice *FLattice;
protected:
  TActionQList Actions;
  TStrPObjList<olxstr,TBasicCFile*> FileFormats;
  class TAtomsInfo *AtomsInfo;
  olxstr FFileName;
  TBasicCFile *FLastLoader;
  TAsymmUnit *FAsymmUnit; // just a pointer to FLAttice->AsymmUnit
  TSpaceGroup* FSG;
  // on SG change of the asymmetric unit
  virtual bool Execute(const IEObject *Sender, const IEObject *Data);
public:
  TXFile(TAtomsInfo *S);
  virtual ~TXFile();

  TActionQueue* OnFileLoad, *OnFileSave;

  TAtomsInfo& GetAtomsInfo()                 const {  return *AtomsInfo;  }
  inline TLattice& GetLattice()              const {  return *FLattice;  }
  inline TUnitCell& GetUnitCell()            const {  return FLattice->GetUnitCell(); }
  /* a propper pointer, created with new should be passed
   the object will be deleted in the destructor !! */
  void RegisterFileFormat(TBasicCFile *F, const olxstr &Ext);

  virtual IEObject* Replicate() const;
  /* the space group is initialised upon file loading
   if the space group is unknow, TFunctionFailedException is thrown
  */
  inline TSpaceGroup& GetLastLoaderSG() {  
    if( FSG == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
    return *FSG; 
  }
  inline TAsymmUnit& GetAsymmUnit()          const {  return *FAsymmUnit; }
  TBasicCFile* FindFormat(const olxstr &Ext);
  inline TBasicCFile* GetLastLoader()        const {  return FLastLoader; }
  inline void SetLastLoader(TBasicCFile* ll)       {  FLastLoader = ll; }
  void UpdateAsymmUnit();
  void LoadFromFile(const olxstr & FN);
  void SaveToFile(const olxstr & FN, bool Sort);
  inline const olxstr& GetFileName()       const {  return FFileName; }

  void EndUpdate();
  void LastLoaderChanged();  // performs complete reinitialisation

  void LibGetFormula(const TStrObjList& Params, TMacroError& E);
  void LibSetFormula(const TStrObjList& Params, TMacroError& E);
  void LibEndUpdate(const TStrObjList& Params, TMacroError& E);
  void LibSaveSolution(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class TBasicCFile: public ACollectionItem  {
private:
protected:
  TAtomsInfo *AtomsInfo;
  olxstr FFileName;
  olxstr FHKLSource; // could not store unicode in ascii file ..., and so for title
  TAsymmUnit* FAsymmUnit;
  olxstr  FTitle;    // title of the file
public:
  TBasicCFile(TAtomsInfo* S);
  virtual ~TBasicCFile();

  TAsymmUnit& GetAsymmUnit()   const {  return *FAsymmUnit; }
  const olxstr& GetTitle()     const {  return FTitle; }
  void SetTitle(const olxstr& t)     {  FTitle = t;  }
  const olxstr& GetFileName()  const {  return FFileName; }
  TAtomsInfo& GetAtomsInfo()   const {  return *AtomsInfo; }
  const olxstr& GetHKLSource() const {  return FHKLSource; }
  void SetHKLSource(const olxstr &FN){  FHKLSource = FN; }

  virtual void SaveToStrings(TStrList& Strings) = 0;
  virtual void LoadFromStrings(const TStrList& Strings) = 0;
  void SaveToFile(const olxstr &A);
  void LoadFromFile(const olxstr &A);
  // adopts the content of the AsemmUnit to the virtual format
  virtual bool Adopt(class TXFile *) = 0;
  virtual void DeleteAtom(TCAtom *A) = 0;
};

EndXlibNamespace()
#endif
