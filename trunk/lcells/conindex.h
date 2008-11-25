//---------------------------------------------------------------------------

#ifndef conindexH
#define conindexH
#include "datastream.h"
#include "xfiles.h"
#include "lattice.h"
//---------------------------------------------------------------------------
struct TConNode  {
  TTypeList<TConNode*> Nodes;
  int16_t Id;
  bool  Used, Used1;
  void *Data;
  uint8_t AtomType;
  void Analyse(class TNet *Parent, struct TConInfo *CI);
  void FillList(int lastindex, TPtrList<TConNode>& L);
  void FindPaths(TConNode *Parent, TConInfo* CI, TPtrList<TConNode>* Path);
  bool IsSimilar(const TConNode& N) const;
  void SetUsed1True();
};

struct TConInfo  {
  TTypeList< TPtrList<TConNode> > Paths;
  int    GraphRadius;
  short  Rings;
  class  TConNode *Center;
  TConInfo();
  ~TConInfo();
  void Clear();
};

class TNet  {
  TPtrList<TConNode> FNodes;
  void Analyse();
  void Clear();
public:
  TNet();
  ~TNet();
  void Assign(TLattice& latt);
  bool IsSubstructure(TConInfo *CI, TNet *N);
  bool IsSubstructureA(TNet *N);  // an old veriosn
  TPtrList<TConNode>& Nodes() {  return FNodes;  }
  void operator >> (IDataOutputStream &S);
  void operator << (IDataInputStream& S);
  olxstr GetDebugInfo();
};

class TConFile  {
  olxstr FTitle, FSpaceGroup, FInstructions;
  olxstr FFileName;
  int FFileAge;
  TNet  *FNet;
  class TConZip *FParent;
public:
  TConFile();
  ~TConFile();
  __property TNet *Net      = {read = FNet};
  __property olxstr Title    =  {read = FTitle, write = FTitle};
  __property olxstr FileName  =  {read = FFileName, write = FFileName};
  __property olxstr SpaceGroup  =  {read = FSpaceGroup, write = FSpaceGroup};
  __property olxstr Instructions  =  {read = FInstructions, write = FInstructions};
  __property int FileAge      =   {read = FFileAge, write = FFileAge};
  __property TConZip*  Parent    = {read = FParent, write = FParent};
  void operator >> (IDataOutputStream& S);
  void operator << (IDataInputStream& S);
};

class TConZip  {
  class TConIndex *FIndex;
  int FFileAge;
  olxstr FFileName;
public:
  TConZip();
  ~TConZip();
  bool Update(class TZipFile *ZF, TXFile& xf);
  void Assign(class TZipFile *ZF, TXFile& xf);
  void Clear();
  void operator >> (IDataOutputStream& S);
  void operator << (IDataInputStream& S);

  __property TConIndex *Index = {read = FIndex};
  __property int FileAge = {read = FFileAge};
  __property olxstr FileName = {read = FFileName};
};

const short ssTitle = 0x0001,
            ssSG    = 0x0002,
            ssIns   = 0x0004;

class TConIndex  {
  TTypeList<TConFile*> FConFiles;
  TTypeList<TConZip*> FZipFiles;
  TConZip *FParent;
public:
  TConIndex();
  ~TConIndex();
  __property TTypeList<TConFile*> ConFiles  = {read = FConFiles};
  __property TTypeList<TConZip*> ZipFiles  = {read = FZipFiles};
  __property TConZip*  Parent    = {read = FParent, write = FParent};

  void Clear();
  void Update(const TStrList& IndexFiles, TXFile& xf);
  void SaveToFile(const olxstr& FN);
  void LoadFromFile(const olxstr& FN);
  void Search(TConInfo *CI, TNet *N, TPtrList<TConFile>& Results, bool Silent=false);
  void Search(const short What, const olxstr& Text, TPtrList<TConFile>& Results, bool Silent=false);
  TConFile * GetRecord(const olxstr& FileName);
  void operator >> (IDataOutputStream& S);
  void operator << (IDataInputStream& S);
  int GetCount();
};
#endif

