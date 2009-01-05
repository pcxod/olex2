//---------------------------------------------------------------------------
#ifndef autoH
#define autoH
#include "xbase.h"
#include "typelist.h"
#include "estlist.h"
#include "asymmunit.h"
#include "unitcell.h"
#include "lattice.h"
#include "satom.h"
#include "network.h"
#include "atominfo.h"
#include "xfiles.h"
#include "tptrlist.h"
#include "library.h"
#include "xapp.h"

BeginXlibNamespace()
class TAutoDB;
class TAutoDBNet;
class TAtomTypePermutator;

class TAutoDBIdObject  {
  int32_t Id;
public:
  TAutoDBIdObject(int32_t id )  {  Id = id;  }
  TAutoDBIdObject(const TAutoDBIdObject& ido )  {  Id = ido.GetId();  }
  TAutoDBIdObject()          {  Id = -1;   }
  inline const TAutoDBIdObject& operator = (const TAutoDBIdObject& oid)  {
    Id = oid.GetId();
    return oid;
  }
  inline bool operator == (const TAutoDBIdObject& oid) {  return Id == oid.GetId();  }
  inline bool operator != (const TAutoDBIdObject& oid) {  return Id != oid.GetId();  }
  DefPropP(int32_t, Id)
};
  typedef TPtrList<TAutoDBIdObject>  TAutoDBIdPList;
////////////////////////////////////////////////////////////////////////////////
class TAutoDBFolder {
  TSStrPObjList<olxstr,TAutoDBIdObject*, true> Files;
public:
  TAutoDBFolder()  {  ;  }
  TAutoDBFolder(const TAutoDBFolder& dbf)  {  *this = dbf;  }
  ~TAutoDBFolder()  {
    for( int i=0; i < Files.Count(); i++ )
      delete Files.Object(i);
  }
  inline const TAutoDBFolder& operator = (const TAutoDBFolder& dbf)  {
    Files.SetCapacity( dbf.Files.Count() );
    for( int i=0; i < dbf.Files.Count(); i++ )
      Files.Add( dbf.Files.GetComparable(i), dbf.Files.GetObject(i) );
    return dbf;
  }
  TAutoDBFolder(IDataInputStream& in)     {  LoadFromStream(in);  }
  inline bool Contains(const olxstr& fileName )  {  return Files.IndexOfComparable(fileName) != -1;  }
  TAutoDBIdObject& Add(const olxstr& fileName)  {
    TAutoDBIdObject* df = new TAutoDBIdObject();
    Files.Add(fileName, df );
    return *df;
  }
  inline long Count() const  {  return Files.Count();  }
  inline TAutoDBIdObject& GetIdObject(int ind)  {  return *Files.Object(ind);  }
  inline const olxstr& GetObjectName(int ind) {  return Files.GetComparable(ind);  }
  void AssignIds( long base )  {
    for( int i=0; i < Files.Count(); i++ )
      Files.GetObject(i)->SetId( base + i );
  }
  void SaveToStream( IDataOutputStream& output ) const;
  void LoadFromStream( IDataInputStream& input );
};

////////////////////////////////////////////////////////////////////////////////
class TAttachedNode  {
  TBasicAtomInfo* BasicAtomInfo;
  vec3d FCenter;
public:
  TAttachedNode( TBasicAtomInfo* bai, const vec3d& c)  {
    BasicAtomInfo = bai;
    FCenter = c;
  }
  TAttachedNode( IDataInputStream& in );
  TAttachedNode()  {
    BasicAtomInfo = NULL;
  }
  void SaveToStream( IDataOutputStream& output ) const;
  inline void SetAtomInfo(TBasicAtomInfo* ai)      {  BasicAtomInfo = ai;  }
  inline void SetCenter(const vec3d& c)            {  FCenter = c;  }
  inline const TBasicAtomInfo& GetAtomInfo() const {  return *BasicAtomInfo;  }
  inline TBasicAtomInfo* BAI()               const {  return BasicAtomInfo;  }
  inline const vec3d& GetCenter()         const {  return FCenter;  }
  inline vec3d& Center()                        {  return FCenter;  }
};

////////////////////////////////////////////////////////////////////////////////
class TAutoDBNode  {
  TTypeList<TAttachedNode> AttachedNodes;
  TBasicAtomInfo* BasicAtomInfo;
  vec3d Center;
//  int AppendedCount,
  int32_t Id;
  // runtime information
  // this is the "index"
  TTypeList< AnAssociation2<TAutoDBNet*,int> > Parents;
  evecd Params; // pre-calculated parameters
  void _PreCalc();
  inline double CalcDistance(int i)  const {  return AttachedNodes[i].GetCenter().DistanceTo(Center);  }
  double CalcAngle(int i, int j)  const;
protected:
  static int SortMetricsFunc(const TAttachedNode& a, const TAttachedNode& b );
  static int SortCAtomsFunc(const AnAssociation2<TCAtom*, vec3d>& a,
                            const AnAssociation2<TCAtom*, vec3d>& b );
  static vec3d SortCenter;
public:
  TAutoDBNode(TSAtom& sa, TTypeList<AnAssociation2<TCAtom*, vec3d> >* atoms);
  TAutoDBNode(IDataInputStream& in)  {  LoadFromStream(in);  }

  const olxstr& ToString() const;

  void SaveToStream( IDataOutputStream& output ) const;
  void LoadFromStream( IDataInputStream& input );

  inline void AddParent(TAutoDBNet* net, int index) {  Parents.AddNew<TAutoDBNet*,int>(net,index);  }
  inline int ParentCount()                    const {  return Parents.Count();  }
  inline TAutoDBNet* GetParent(int i)         const {  return Parents[i].GetA();  }
  inline int GetParentIndex(int i)            const {  return Parents[i].GetB();  }

  inline int NodeCount()                     const {  return AttachedNodes.Count();  }
  inline const TAttachedNode& GetNode(int i) const {  return AttachedNodes[i];  }
  inline TBasicAtomInfo& GetAtomInfo()       const {  return *BasicAtomInfo;  }
  inline TBasicAtomInfo* BAI()               const {  return BasicAtomInfo;  }
  inline int DistanceCount()                 const {  return AttachedNodes.Count();  }
  inline double GetDistance(int i)           const {  return Params[i];  }
  inline double GetAngle(int i)              const {  return Params[AttachedNodes.Count()+i];  }

  void DecodeAngle(int index, int& nodea, int& nodeb)  {
    int cnt = AttachedNodes.Count(), itr = 0;
    while( index > cnt )  {
      index -= cnt;
      cnt --;
      itr++;
    }
    nodea = itr;
    nodeb = index + (AttachedNodes.Count()-cnt);
  }

  DefPropP(int32_t, Id)
  // sorts by connectivity, type and params
  int UpdateCompare(const TAutoDBNode& dbn) const;
  // sorts by connectivity and params
  double SearchCompare(const TAutoDBNode& dbn, double* fom=NULL) const;

  bool IsSameType(const TAutoDBNode& dbn) const;
  bool IsSimilar(const TAutoDBNode& dbn) const;
  bool IsMetricSimilar(const TAutoDBNode& dbn, double& fom) const;
};
  typedef TPtrList< TAutoDBNode > TAutoDBNodePList;
  typedef TTypeList< TAutoDBNode > TAutoDBNodeList;

////////////////////////////////////////////////////////////////////////////////
class TAutoDBNetNode  {
  TAutoDBNode* FCenter;
  TPtrList<TAutoDBNetNode>  AttachedNodes;
  int32_t Id, Tag;
public:
  TAutoDBNetNode( TAutoDBNode* node )     {  FCenter = node;  }
  TAutoDBNetNode( IDataInputStream& input )  {  LoadFromStream(input);  }
  void AttachNode( TAutoDBNetNode* node ) {  AttachedNodes.Add(node);  }
  inline int Count()                const {  return AttachedNodes.Count();  }
  TAutoDBNetNode* Node(int i)       const {  return AttachedNodes[i];  }
  TAutoDBNode* Center()             const {  return FCenter;  }

  bool IsSameType(const TAutoDBNetNode& dbn, bool ExtraLevel) const;
  bool IsMetricSimilar(const TAutoDBNetNode& nd, double& cfom, int* cindexes, bool ExtraLevel)  const;

  void SaveToStream( IDataOutputStream& output ) const;
  void LoadFromStream( IDataInputStream& input );

  const olxstr& ToString(int level) const;

  DefPropP(int32_t, Id)
  DefPropP(int32_t, Tag)
};
////////////////////////////////////////////////////////////////////////////////
class TAutoDBNet  {
  TTypeList<TAutoDBNetNode> Nodes;
  TAutoDBIdObject* FReference;
protected:
  static TAutoDBNet* CurrentlyLoading;
public:
  TAutoDBNet(TAutoDBIdObject* ref)                    {  FReference = ref;  }
  TAutoDBNet(IDataInputStream& input)                    {  LoadFromStream(input);  }
  TAutoDBNetNode& NewNode(TAutoDBNode* node)          {  return Nodes.AddNew(node);  }
  inline const TAutoDBIdObject& GetReference()  const {  return *FReference;  }
  inline TAutoDBIdObject* Reference()           const {  return FReference;  }
  inline int Count()                            const {  return Nodes.Count();  }
  inline TAutoDBNetNode& Node(int i)                  {  return Nodes[i];  }
  void SaveToStream( IDataOutputStream& output ) const;
  void LoadFromStream( IDataInputStream& input );
  bool Contains(TAutoDBNode* nd )  const {
    for( int i=0; i < Nodes.Count(); i++ )
      if( Nodes[i].Center() == nd )  return true;
    return false;
  }
  int IndexOf(TAutoDBNode* nd, int start=0 )  const {
    for( int i=start; i < Nodes.Count(); i++ )
      if( Nodes[i].Center() == nd )  return i;
    return -1;
  }
  static inline TAutoDBNet& GetCurrentlyLoading()  {  return *CurrentlyLoading;  }
};

////////////////////////////////////////////////////////////////////////////////
class TAutoDBSearchNode  {
  TAutoDBNodePList PossibleCentres;

};
////////////////////////////////////////////////////////////////////////////////
class TAutoDB : public IEObject  {
  // a fixed size list 0 - nodes connected to one other node, 1 - two, etc
  TTypeList< TPtrList<TAutoDBNode> > Nodes;
  TXFile& XFile;
private:
  static TAutoDB* Instance;
//  TAutoDBFileList References;
  TSStrPObjList<olxstr,TAutoDBFolder*, true> Folders;
//  TAutoDBFolderList Folders;
  TTypeList<TAutoDBNet> Nets;
  void PrepareForSearch();
protected:
  void ProcessNodes( TAutoDBIdObject* currentFile, TNetwork& net );
  TAutoDBNet* BuildSearchNet( TNetwork& net, TSAtomPList& cas );
  //............................................................................
  TAutoDBNode* LocateNode(int index) {
    for( int i=0; i < Nodes.Count(); i++ )  {
      index -= Nodes[i].Count();
      if( index < 0 )  {
        TPtrList<TAutoDBNode>& ndl = Nodes[i];
        int ind = ndl.Count() + index;
        return ndl[ind];
      }
    }
    throw TInvalidArgumentException(__OlxSourceInfo, "index");
  }
  //............................................................................
  TAutoDBIdObject& LocateFile(int index) {
    for( int i=0; i < Folders.Count(); i++ )  {
      index -= Folders.Object(i)->Count();
      if( index < 0 )  {
        TAutoDBFolder* fld = Folders.Object(i);
        int ind = fld->Count() + index;
        return fld->GetIdObject( ind );
      }
    }
    throw TInvalidArgumentException(__OlxSourceInfo, "index");
  }
  //............................................................................
  const olxstr& LocateFileName(const TAutoDBIdObject& file) {
    int32_t index = file.GetId();
    for( int i=0; i < Folders.Count(); i++ )  {
      index -= Folders.Object(i)->Count();
      if( index < 0 )  {
        TAutoDBFolder* fld = Folders.Object(i);
        int ind = fld->Count() + index;
        return fld->GetObjectName( ind );
      }
    }
    throw TInvalidArgumentException(__OlxSourceInfo, "index");
  }
  //............................................................................
  TAutoDBFolder& LocateFileFolder(const TAutoDBIdObject& file) {
    int32_t index = file.GetId();
    for( int i=0; i < Folders.Count(); i++ )  {
      index -= Folders.Object(i)->Count();
      if( index < 0 )  return *Folders.Object(i);
    }
    throw TInvalidArgumentException(__OlxSourceInfo, "index");
  }
public:
  // structre to store analysis statistics
  struct AnalysisStat {
    int AtomTypeChanges, ConfidentAtomTypes, SNAtomTypeAssignments;
    bool FormulaConstrained, AtomDeltaConstrained;
    AnalysisStat()  {  Clear();  }
    void Clear()  {
      AtomTypeChanges = ConfidentAtomTypes = SNAtomTypeAssignments = 0;
      FormulaConstrained = AtomDeltaConstrained;
    }
  };
private:
  TDoubleList Uisos;
  AnalysisStat LastStat;
  olxstr LastFileName;
  int BAIDelta; // maximim element promotion
  double URatio; // ratio beyond which search for element promotion
  TAtomsInfo& AtomsInfo; 
public:
  /* the instance must be created with Replcate to aoid any problems.
   It will be deleted by this object
  */
  TAutoDB(TXFile& xfile, ALibraryContainer& lc);
  virtual ~TAutoDB();

  void ProcessFolder(const olxstr& folder);
  void SaveToStream( IDataOutputStream& output ) const;
  void LoadFromStream( IDataInputStream& input );
protected:
  void AnalyseNet(TNetwork& net, TAtomTypePermutator* permutator, 
    double& Uiso, AnalysisStat& stat, TBAIPList* proposed_atoms = NULL);
public:
  void AnalyseStructure(const olxstr& LastFileName, TLattice& latt, 
    TAtomTypePermutator* permutator, AnalysisStat& stat, TBAIPList* proposed_atoms = NULL);

  inline const TDoubleList& GetUisos()   const {  return Uisos;  }
  const AnalysisStat& GetStats()         const {  return LastStat;  }
  inline const olxstr& GetLastFileName() const {  return LastFileName;  }
  void AnalyseNode(TSAtom& sa, TStrList& report);
  inline static TAutoDB* GetInstance()     {  return Instance;  }
  inline static TAtomsInfo& GetAtomsInfo() {  return Instance->AtomsInfo;  }
  inline TAutoDBIdObject& Reference(int i) {  return LocateFile(i);  }
//  inline TAutoDBFolder& Folder(int i)      {  return Folders.Object(i);  }
  inline TAutoDBNode* Node(int i)          {  return LocateNode(i);  }
  DefPropP(int, BAIDelta)
  DefPropP(double, URatio)

  template <class NodeClass>
    struct THitStruct {
      double Fom;
      NodeClass* Node;
      THitStruct( NodeClass* node, double fom )  {
        Node = node;
        Fom = fom;
      }
      static int SortByFOMFunc( const THitStruct<NodeClass>& a,
                                const THitStruct<NodeClass>& b )  {
        double d = a.Fom - b.Fom;
        if( d < 0 )  return -1;
        if( d > 0 )  return 1;
        return 0;
      }
    };
  template <class NodeClass>
    struct THitList {
      TTypeList< THitStruct<NodeClass> > hits;
      TBasicAtomInfo* BAI;
      double meanFom;
      static int SortByFOMFunc( const THitList<NodeClass>& a,
                                const THitList<NodeClass>& b )  {
        int nc = olx_min( a.hits.Count(), b.hits.Count() );
        //nc = olx_min(1, nc);
        double foma = 0, fomb = 0;
        for( int i=0; i < nc; i++ )  {
          foma += a.hits[i].Fom;
          fomb += b.hits[i].Fom;
        }
        foma -= fomb;
        if( foma < 0 )  return -1;
        if( foma > 0 )  return 1;
        return 0;
      }
      static int SortByCountFunc( const THitList<NodeClass>& a,
                                const THitList<NodeClass>& b )  {
        return b.hits.Count() - a.hits.Count();
      }
      THitList( TBasicAtomInfo* bai, NodeClass* node, double fom )  {
        BAI = bai;
        hits.AddNew(node, fom);
        meanFom = 0;
      }
      double MeanFom()  {
        if( meanFom != 0 )  return meanFom;
        for( int i=0; i < hits.Count(); i++ )
          meanFom += hits[i].Fom;
        return meanFom /= hits.Count();
      }
      double MeanFomN(int cnt)  {
        double mf = 0;
        for( int i=0; i < cnt; i++ )
          mf += hits[i].Fom;
        return mf /= cnt;
      }
      void Sort()  {  hits.QuickSorter.SortSF( hits, THitStruct<NodeClass>::SortByFOMFunc);  }
    };
  struct TGuessCount  {
    TCAtom* atom;
    TTypeList< THitList<TAutoDBNode> >* list1;
    TTypeList< THitList<TAutoDBNetNode> >* list2;
    TTypeList< THitList<TAutoDBNetNode> >* list3;
  };
  void ValidateResult(const olxstr& fileName, const TLattice& au, TStrList& report);
protected:
  // hits must be sorted beforehand!
  void AnalyseUiso(TCAtom& ca, const TTypeList< THitList<TAutoDBNode> >& list, AnalysisStat& stat, 
    bool heavier, bool lighter, TBAIPList* proposed_atoms = NULL);
  
  class TAnalyseNetNodeTask  {
    TTypeList< TPtrList<TAutoDBNode> >& Nodes;
    TTypeList<TGuessCount>& Guesses;
    TAutoDBNet& Network;
    long LocateDBNodeIndex(const TPtrList<TAutoDBNode>& segment, TAutoDBNode* nd,
      long from=-1, long to=-1);
  public:
    TAnalyseNetNodeTask(TTypeList< TPtrList<TAutoDBNode> >& nodes,
                        TAutoDBNet& network, TTypeList<TGuessCount>& guesses) :
      Nodes(nodes), Network(network), Guesses(guesses)  { }
    void Run( long index );
    inline TAnalyseNetNodeTask* Replicate()  const  {
      return new TAnalyseNetNodeTask(Nodes, Network, Guesses);
    }
  };
//  these are protected, but exposed in the constructor
  void LibBAIDelta(const TStrObjList& Params, TMacroError& E);
  void LibURatio(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class NodeType>  bool AnalyseUiso(TCAtom& ca, const TTypeList< THitList<NodeType> >& list, AnalysisStat& stat, 
                          bool heavier, bool lighter, TBAIPList* proposed_atoms)  {
  if( !lighter && !heavier )  return false;
  olxstr tmp( ca.GetLabel() );
  tmp << ' ';
  TBasicAtomInfo* type = &ca.GetAtomInfo();
  if( heavier )  {
    TBasicApp::GetLog().Info( olxstr("Searching element heavier for ") << ca.GetLabel() );
    for( int j=0; j < list.Count(); j++ )  {
      if( list[j].BAI->GetIndex() > type->GetIndex() )  {
        if( proposed_atoms != NULL )  {
          if( proposed_atoms->IndexOf( list[j].BAI ) != -1 )  {
            type = list[j].BAI;
            break;
          }
        }
        else if( BAIDelta != -1 )  {
          if( (list[j].BAI->GetIndex() - ca.GetAtomInfo().GetIndex()) < BAIDelta )  {
            type = list[j].BAI;
            break;
          }
        }
        //              else  {
        //              }
      }
    }
  }
  else if( lighter )  {
    TBasicApp::GetLog().Info( olxstr("Searching element lighter for ") << ca.GetLabel() );
    for( int j=0; j < list.Count(); j++ )  {
      if( list[j].BAI->GetIndex() < type->GetIndex() )  {
        if( proposed_atoms != NULL )  {
          if( proposed_atoms->IndexOf( list[j].BAI ) != -1 )  {
            type = list[j].BAI;
            break;
          }
        }
        else if( BAIDelta != -1 )  {
          if( (ca.GetAtomInfo().GetIndex() - list[j].BAI->GetIndex()) < BAIDelta  )  {
            type = list[j].BAI;
            break;
          }
        }
        //              else  {
        //              }
      }
    }
  }
  for( int j=0; j < list.Count(); j++ )  {
    tmp << list[j].BAI->GetSymbol() << '(' <<
      olxstr::FormatFloat(3,1.0/(list[j].MeanFom()+0.001)) << ")" << list[j].hits[0].Fom;
    if( (j+1) < list.Count() )  tmp << ',';
  }
  if( type == NULL || *type == ca.GetAtomInfo() )  return false;
  int atc = stat.AtomTypeChanges;
  if( proposed_atoms != NULL )  {
    if( proposed_atoms->IndexOf( type ) != -1 )  {
      stat.AtomTypeChanges++;
      ca.Label() =  type->GetSymbol();
      ca.SetAtomInfo( type );
    }
  }
  else if( BAIDelta != -1 )  {
    if( abs(type->GetIndex() - ca.GetAtomInfo().GetIndex()) < BAIDelta )  {
      stat.AtomTypeChanges++;
      ca.Label() =  type->GetSymbol();
      ca.SetAtomInfo( type );
    }
  }
  else  {
    stat.AtomTypeChanges++;
    ca.Label() =  type->GetSymbol();
    ca.SetAtomInfo( type );
  }
  TBasicApp::GetLog().Info( tmp );
  return atc != stat.AtomTypeChanges;
}

};

class  TAtomTypePermutator {
public:
  struct TPermutation  {
    vec3d AtomCenter;
    TCAtom* Atom;
    TTypeList< AnAssociation3<TBasicAtomInfo*,double, double> > Tries;
  };
protected:
  TTypeList<TPermutation> Atoms;
  TPtrList<TBasicAtomInfo> TypeRestraints;
  bool Active;
public:
  TAtomTypePermutator()  {  Active = true;  }
  void Init(TPtrList<TBasicAtomInfo>* typeRestraints = NULL);
  void ReInit(const TAsymmUnit& au);
  void InitAtom(TAutoDB::TGuessCount& guess);
  void Permutate();
  DefPropB(Active)
};

EndXlibNamespace()
#endif
