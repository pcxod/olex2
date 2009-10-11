//---------------------------------------------------------------------------

#ifndef conindexH
#define conindexH
#include "datastream.h"
#include "xfiles.h"
#include "lattice.h"
//---------------------------------------------------------------------------
struct TConNode;
struct TConZip;
struct TConIndex;

typedef TPtrList<TConNode> NodePList;
struct TConNode  {
  NodePList Nodes;
  int16_t Id;
  bool  Used, Used1;
  void *Data;
  uint8_t AtomType;
  void Analyse(class TNet& Parent, struct TConInfo& CI);
  void FillList(int lastindex, NodePList& L);
  void FindPaths(TConNode* Parent, TConInfo& CI, NodePList* Path);
  bool IsSimilar(const TConNode& N) const;
  void SetUsed1True();
};

typedef TTypeList<TConNode> NodeList;
typedef TTypeList<NodePList> PathList;

struct TConInfo  {
  PathList Paths;
  int    GraphRadius;
  short  Rings;
  TConNode *Center;
  TConInfo();
  ~TConInfo();
  void Clear();
};

class TNet  {
  void Analyse();
  void Clear() { Nodes.Clear();  }
public:
  NodeList Nodes;

  TNet()  {}
  ~TNet() {}
  void Assign(TLattice& latt);
  bool IsSubstructure(TConInfo *CI, TNet& N);
  bool IsSubstructureA(TNet& N);  // an old veriosn
  void operator >> (IDataOutputStream &S);
  void operator << (IDataInputStream& S);
  olxstr GetDebugInfo();
};

struct TConFile  {
  olxstr FileName;
  time_t FileAge;
  TNet Net;
  TConZip* Parent;
  olxstr Title,
         SpaceGroup,
         Instructions;

  TConFile(TConZip* parent=NULL) : Parent(parent), FileAge(0) { }
  ~TConFile() {}
  void operator >> (IDataOutputStream& S);
  void operator << (IDataInputStream& S);
};

struct TConZip  {
  TConIndex* Index;
  time_t FileAge;
  olxstr FileName;

  TConZip();
  ~TConZip();
  bool Update(class TZipFile *ZF, TXFile& xf);
  void Assign(class TZipFile *ZF, TXFile& xf);
  void Clear();
  void operator >> (IDataOutputStream& S);
  void operator << (IDataInputStream& S);
};

const short ssTitle = 0x0001,
            ssSG    = 0x0002,
            ssIns   = 0x0004;

struct TConIndex  {
  TTypeList<TConFile> ConFiles;
  TTypeList<TConZip> ZipFiles;
  TConZip* Parent;

  TConIndex(TConZip* parent=NULL) : Parent(parent) {}
  ~TConIndex() {}

  void Clear();
  void Update(const TStrList& IndexFiles, TXFile& xf);
  void SaveToFile(const olxstr& FN);
  void LoadFromFile(const olxstr& FN);
  void Search(TConInfo* CI, TNet& N, TPtrList<TConFile>& Results, bool Silent=false);
  void Search(const short What, const olxstr& Text, TPtrList<TConFile>& Results, bool Silent=false);
  TConFile * GetRecord(const olxstr& FileName);
  void operator >> (IDataOutputStream& S);
  void operator << (IDataInputStream& S);
  int GetCount();
};
#endif

