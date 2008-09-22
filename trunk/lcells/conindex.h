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
  void __fastcall Analyse(class TNet *Parent, struct TConInfo *CI);
  void _fastcall FillList(int lastindex, TTypeList<TConNode*>& L);
  void _fastcall FindPaths(TConNode *Parent, TConInfo* CI, TTypeList<TConNode*>* Path);
  bool _fastcall IsSimilar(TConNode *N);
  void _fastcall SetUsed1True();
};

struct TConInfo  {
  TTypeList< TTypeList<TConNode*>* > Paths;
  int    GraphRadius;
  short  Rings;
  class  TConNode *Center;
  _fastcall TConInfo();
  _fastcall ~TConInfo();
  void _fastcall Clear();
};

class TNet  {
  TTypeList<TConNode*> FNodes;
  void _fastcall Analyse();
  void _fastcall Clear();
public:
  _fastcall TNet();
  _fastcall ~TNet();
  void _fastcall Assign(TLattice& latt);
  bool _fastcall IsSubstructure(TConInfo *CI, TNet *N);
  bool _fastcall IsSubstructureA(TNet *N);  // an old veriosn

  __property TTypeList<TConNode*> Nodes     = {read = FNodes};
  void _fastcall operator >> (IDataOutputStream &S);
  void _fastcall operator << (IDataInputStream& S);
  olxstr GetDebugInfo();
};

class TConFile  {
  olxstr FTitle, FSpaceGroup, FInstructions;
  olxstr FFileName;
  int FFileAge;
  TNet  *FNet;
  class TConZip *FParent;
public:
  _fastcall TConFile();
  _fastcall ~TConFile();
  __property TNet *Net      = {read = FNet};
  __property olxstr Title    =  {read = FTitle, write = FTitle};
  __property olxstr FileName  =  {read = FFileName, write = FFileName};
  __property olxstr SpaceGroup  =  {read = FSpaceGroup, write = FSpaceGroup};
  __property olxstr Instructions  =  {read = FInstructions, write = FInstructions};
  __property int FileAge      =   {read = FFileAge, write = FFileAge};
  __property TConZip*  Parent    = {read = FParent, write = FParent};
  void _fastcall operator >> (IDataOutputStream& S);
  void _fastcall operator << (IDataInputStream& S);
};

class TConZip  {
  class TConIndex *FIndex;
  int FFileAge;
  olxstr FFileName;
public:
  _fastcall TConZip();
  _fastcall ~TConZip();
  bool _fastcall Update(class TZipFile *ZF, TXFile& xf);
  void _fastcall Assign(class TZipFile *ZF, TXFile& xf);
  void _fastcall Clear();
  void _fastcall operator >> (IDataOutputStream& S);
  void _fastcall operator << (IDataInputStream& S);

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
  __fastcall TConIndex();
  __fastcall ~TConIndex();
  __property TTypeList<TConFile*> ConFiles  = {read = FConFiles};
  __property TTypeList<TConZip*> ZipFiles  = {read = FZipFiles};
  __property TConZip*  Parent    = {read = FParent, write = FParent};

  void _fastcall Clear();
  void _fastcall Update(const TStrList& IndexFiles, TXFile& xf);
  void _fastcall SaveToFile(const olxstr& FN);
  void _fastcall LoadFromFile(const olxstr& FN);
  void _fastcall Search(TConInfo *CI, TNet *N, TTypeList<TConFile*>& Results, bool Silent=false);
  void _fastcall Search(const short What, const olxstr& Text, TTypeList<TConFile*>& Results, bool Silent=false);
  TConFile * _fastcall GetRecord(const olxstr& FileName);
  void _fastcall operator >> (IDataOutputStream& S);
  void _fastcall operator << (IDataInputStream& S);
  int GetCount();
};
#endif

