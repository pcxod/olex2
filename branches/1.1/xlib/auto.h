#ifndef __olx_xl_auto_H
#define __olx_xl_auto_H
#include "xbase.h"
#include "typelist.h"
#include "estlist.h"
#include "asymmunit.h"
#include "unitcell.h"
#include "lattice.h"
#include "satom.h"
#include "network.h"
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
    for( size_t i=0; i < Files.Count(); i++ )
      delete Files.GetObject(i);
  }
  inline const TAutoDBFolder& operator = (const TAutoDBFolder& dbf)  {
    Files.SetCapacity( dbf.Files.Count() );
    for( size_t i=0; i < dbf.Files.Count(); i++ )
      Files.Add( dbf.Files.GetComparable(i), dbf.Files.GetObject(i) );
    return dbf;
  }
  TAutoDBFolder(IDataInputStream& in)     {  LoadFromStream(in);  }
  inline bool Contains(const olxstr& fileName )  {  return Files.IndexOfComparable(fileName) != InvalidIndex;  }
  TAutoDBIdObject& Add(const olxstr& fileName)  {
    TAutoDBIdObject* df = new TAutoDBIdObject();
    Files.Add(fileName, df );
    return *df;
  }
  inline size_t Count() const {  return Files.Count();  }
  inline TAutoDBIdObject& GetIdObject(size_t ind) const {  return *Files.GetObject(ind);  }
  inline const olxstr& GetObjectName(size_t ind) {  return Files.GetComparable(ind);  }
  void AssignIds(uint32_t base)  {
    for( size_t i=0; i < Files.Count(); i++ )
      Files.GetObject(i)->SetId((uint32_t)(base + i));
  }
  void SaveToStream( IDataOutputStream& output ) const;
  void LoadFromStream( IDataInputStream& input );
};

////////////////////////////////////////////////////////////////////////////////
class TAttachedNode  {
  const cm_Element* Element;
  vec3d FCenter;
public:
  TAttachedNode(const cm_Element* element, const vec3d& c) : Element(element), FCenter(c)  {}
  TAttachedNode(IDataInputStream& in);
  TAttachedNode()  {  Element = NULL;  }
  void SaveToStream(IDataOutputStream& output ) const;
  inline void SetType(const cm_Element* e)  {  Element = e;  }
  inline void SetCenter(const vec3d& c)  {  FCenter = c;  }
  inline const cm_Element& GetType() const {  return *Element;  }
  inline const vec3d& GetCenter() const {  return FCenter;  }
};

////////////////////////////////////////////////////////////////////////////////
class TAutoDBNode  {
  TTypeList<TAttachedNode> AttachedNodes;
  const cm_Element* Element;
  vec3d Center;
//  int AppendedCount,
  int32_t Id;
  // runtime information
  // this is the "index"
  TTypeList<AnAssociation2<TAutoDBNet*,uint32_t> > Parents;
  evecd Params; // pre-calculated parameters
  void _PreCalc();
  inline double CalcDistance(size_t i) const {  return AttachedNodes[i].GetCenter().DistanceTo(Center);  }
  double CalcAngle(size_t i, size_t j) const;
protected:
  static int SortMetricsFunc(const TAttachedNode& a, const TAttachedNode& b);
  static int SortCAtomsFunc(const AnAssociation2<TCAtom*, vec3d>& a,
                            const AnAssociation2<TCAtom*, vec3d>& b);
  static vec3d SortCenter;
public:
  TAutoDBNode(TSAtom& sa, TTypeList<AnAssociation2<TCAtom*, vec3d> >* atoms);
  TAutoDBNode(IDataInputStream& in)  {  LoadFromStream(in);  }

  const olxstr& ToString() const;

  void SaveToStream(IDataOutputStream& output) const;
  void LoadFromStream(IDataInputStream& input);

  inline void AddParent(TAutoDBNet* net, uint32_t index) {  Parents.AddNew<TAutoDBNet*,uint32_t>(net,index);  }
  inline size_t ParentCount() const {  return Parents.Count();  }
  inline TAutoDBNet* GetParent(size_t i) const {  return Parents[i].GetA();  }
  inline int GetParentIndex(size_t i) const {  return Parents[i].GetB();  }

  inline size_t NodeCount() const {  return AttachedNodes.Count();  }
  inline const TAttachedNode& GetNode(size_t i) const {  return AttachedNodes[i];  }
  inline const cm_Element& GetType() const {  return *Element;  }
  inline size_t DistanceCount() const {  return AttachedNodes.Count();  }
  inline double GetDistance(size_t i) const {  return Params[i];  }
  inline double GetAngle(size_t i) const {  return Params[AttachedNodes.Count()+i];  }

  void DecodeAngle(size_t index, size_t& nodea, size_t& nodeb)  {
    size_t cnt = AttachedNodes.Count(), itr = 0;
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
  TAutoDBNetNode(TAutoDBNode* node)  {  FCenter = node;  }
  TAutoDBNetNode(IDataInputStream& input)  {  LoadFromStream(input);  }
  void AttachNode(TAutoDBNetNode* node) {  AttachedNodes.Add(node);  }
  inline size_t Count() const {  return AttachedNodes.Count();  }
  TAutoDBNetNode* Node(size_t i) const {  return AttachedNodes[i];  }
  TAutoDBNode* Center() const {  return FCenter;  }

  bool IsSameType(const TAutoDBNetNode& dbn, bool ExtraLevel) const;
  bool IsMetricSimilar(const TAutoDBNetNode& nd, double& cfom, uint16_t* cindexes, bool ExtraLevel)  const;

  void SaveToStream(IDataOutputStream& output) const;
  void LoadFromStream(IDataInputStream& input);

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
  TAutoDBNet(TAutoDBIdObject* ref)  {  FReference = ref;  }
  TAutoDBNet(IDataInputStream& input)  {  LoadFromStream(input);  }
  TAutoDBNetNode& NewNode(TAutoDBNode* node)  {  return Nodes.AddNew(node);  }
  inline const TAutoDBIdObject& GetReference() const {  return *FReference;  }
  inline TAutoDBIdObject* Reference() const {  return FReference;  }
  inline size_t Count() const {  return Nodes.Count();  }
  inline TAutoDBNetNode& Node(size_t i)  {  return Nodes[i];  }
  void SaveToStream(IDataOutputStream& output) const;
  void LoadFromStream(IDataInputStream& input);
  bool Contains(TAutoDBNode* nd )  const {
    for( size_t i=0; i < Nodes.Count(); i++ )
      if( Nodes[i].Center() == nd )  return true;
    return false;
  }
  size_t IndexOf(TAutoDBNode* nd, int start=0 )  const {
    for( size_t i=start; i < Nodes.Count(); i++ )
      if( Nodes[i].Center() == nd )  return i;
    return InvalidIndex;
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
  void ProcessNodes(TAutoDBIdObject* currentFile, TNetwork& net);
  TAutoDBNet* BuildSearchNet(TNetwork& net, TSAtomPList& cas);
  //............................................................................
  TAutoDBNode* LocateNode(size_t index) {
    for( size_t i=0; i < Nodes.Count(); i++ )  {
      if( index < Nodes[i].Count() )
        return Nodes[i][index];
      index -= Nodes[i].Count();
    }
    throw TInvalidArgumentException(__OlxSourceInfo, "index");
  }
  //............................................................................
  TAutoDBIdObject& LocateFile(size_t index) {
    for( size_t i=0; i < Folders.Count(); i++ )  {
      if( index < Folders.GetObject(i)->Count() )
        return Folders.GetObject(i)->GetIdObject(index);
      index -= Folders.GetObject(i)->Count();
    }
    throw TInvalidArgumentException(__OlxSourceInfo, "index");
  }
  //............................................................................
  const olxstr& LocateFileName(const TAutoDBIdObject& file) {
    size_t index = file.GetId();
    for( size_t i=0; i < Folders.Count(); i++ )  {
      if( index < Folders.GetObject(i)->Count() )
        return Folders.GetObject(i)->GetObjectName(index);
      index -= Folders.GetObject(i)->Count();
    }
    throw TInvalidArgumentException(__OlxSourceInfo, "index");
  }
  //............................................................................
  TAutoDBFolder& LocateFileFolder(const TAutoDBIdObject& file) {
    size_t index = file.GetId();
    for( size_t i=0; i < Folders.Count(); i++ )  {
      if( index < Folders.GetObject(i)->Count() )  
        return *Folders.GetObject(i);
      index -= Folders.GetObject(i)->Count();
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
    double& Uiso, AnalysisStat& stat, ElementPList* proposed_atoms = NULL);
  // a helper function to check C-O and C-N
  void A2Pemutate(TCAtom& a1, TCAtom& a2, const cm_Element& e1, const cm_Element& e2, double threshold);
public:
  void AnalyseStructure(const olxstr& LastFileName, TLattice& latt, 
    TAtomTypePermutator* permutator, AnalysisStat& stat, ElementPList* proposed_atoms = NULL);

  inline const TDoubleList& GetUisos() const {  return Uisos;  }
  const AnalysisStat& GetStats() const {  return LastStat;  }
  inline const olxstr& GetLastFileName() const {  return LastFileName;  }
  void AnalyseNode(TSAtom& sa, TStrList& report);
  inline static TAutoDB* GetInstance()  {  return Instance;  }
  inline TAutoDBIdObject& Reference(size_t i)  {  return LocateFile(i);  }
  inline TAutoDBNode* Node(size_t i)  {  return LocateNode(i);  }
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
      const cm_Element* Type;
      double meanFom;
      static int SortByFOMFunc( const THitList<NodeClass>& a,
                                const THitList<NodeClass>& b )  {
        const size_t nc = olx_min( a.hits.Count(), b.hits.Count() );
        //nc = olx_min(1, nc);
        double foma = 0, fomb = 0;
        for( size_t i=0; i < nc; i++ )  {
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
        return olx_cmp_size_t(b.hits.Count(), a.hits.Count());
      }
      THitList(const cm_Element& type, NodeClass* node, double fom)  {
        Type = &type;
        hits.AddNew(node, fom);
        meanFom = 0;
      }
      double MeanFom()  {
        if( meanFom != 0 )  return meanFom;
        for( size_t i=0; i < hits.Count(); i++ )
          meanFom += hits[i].Fom;
        return meanFom /= hits.Count();
      }
      double MeanFomN(size_t cnt)  {
        double mf = 0;
        for( size_t i=0; i < cnt; i++ )
          mf += hits[i].Fom;
        return mf /= cnt;
      }
      void Sort()  {  hits.QuickSorter.SortSF(hits, THitStruct<NodeClass>::SortByFOMFunc);  }
    };
  struct TGuessCount  {
    TCAtom* atom;
    TTypeList<THitList<TAutoDBNode> >* list1;
    TTypeList<THitList<TAutoDBNetNode> >* list2;
    TTypeList<THitList<TAutoDBNetNode> >* list3;
  };
  void ValidateResult(const olxstr& fileName, const TLattice& au, TStrList& report);
protected:
  // hits must be sorted beforehand!
  void AnalyseUiso(TCAtom& ca, const TTypeList<THitList<TAutoDBNode> >& list, AnalysisStat& stat, 
    bool heavier, bool lighter, ElementPList* proposed_atoms = NULL);
  
  class TAnalyseNetNodeTask  {
    TTypeList< TPtrList<TAutoDBNode> >& Nodes;
    TTypeList<TGuessCount>& Guesses;
    TAutoDBNet& Network;
    size_t LocateDBNodeIndex(const TPtrList<TAutoDBNode>& segment, TAutoDBNode* nd,
      size_t from=InvalidIndex, size_t to=InvalidIndex);
  public:
    TAnalyseNetNodeTask(TTypeList< TPtrList<TAutoDBNode> >& nodes,
                        TAutoDBNet& network, TTypeList<TGuessCount>& guesses) :
      Nodes(nodes), Network(network), Guesses(guesses)  { }
    void Run(size_t index);
    TAnalyseNetNodeTask* Replicate()  const  {
      return new TAnalyseNetNodeTask(Nodes, Network, Guesses);
    }
  };
//  these are protected, but exposed in the constructor
  void LibBAIDelta(const TStrObjList& Params, TMacroError& E);
  void LibURatio(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <class NodeType>  bool AnalyseUiso(TCAtom& ca, const TTypeList< THitList<NodeType> >& list, AnalysisStat& stat, 
                          bool heavier, bool lighter, ElementPList* proposed_atoms)  {
  if( !lighter && !heavier )  return false;
  olxstr tmp( ca.GetLabel() );
  tmp << ' ';
  const cm_Element* type = &ca.GetType();
  if( heavier )  {
    TBasicApp::GetLog().Info(olxstr("Searching element heavier for ") << ca.GetLabel());
    for( size_t j=0; j < list.Count(); j++ )  {
      if( list[j].Type->z > type->z )  {
        if( proposed_atoms != NULL )  {
          if( proposed_atoms->IndexOf(list[j].Type) != InvalidIndex )  {
            type = list[j].Type;
            break;
          }
        }
        else if( BAIDelta != -1 )  {
          if( (list[j].Type->z - ca.GetType().z) < BAIDelta )  {
            type = list[j].Type;
            break;
          }
        }
        //              else  {
        //              }
      }
    }
  }
  else if( lighter )  {
    TBasicApp::GetLog().Info(olxstr("Searching element lighter for ") << ca.GetLabel());
    for( size_t j=0; j < list.Count(); j++ )  {
      if( list[j].Type->z < type->z )  {
        if( proposed_atoms != NULL )  {
          if( proposed_atoms->IndexOf(list[j].Type) != InvalidIndex )  {
            type = list[j].Type;
            break;
          }
        }
        else if( BAIDelta != -1 )  {
          if( (ca.GetType().z - list[j].Type->z) < BAIDelta  )  {
            type = list[j].Type;
            break;
          }
        }
        //              else  {
        //              }
      }
    }
  }
  for( size_t j=0; j < list.Count(); j++ )  {
    tmp << list[j].Type->symbol << '(' <<
      olxstr::FormatFloat(3,1.0/(list[j].MeanFom()+0.001)) << ")" << list[j].hits[0].Fom;
    if( (j+1) < list.Count() )  tmp << ',';
  }
  if( type == NULL || *type == ca.GetType() )  return false;
  int atc = stat.AtomTypeChanges;
  if( proposed_atoms != NULL )  {
    if( proposed_atoms->IndexOf(type) != InvalidIndex )  {
      stat.AtomTypeChanges++;
      ca.SetLabel(type->symbol, false);
      ca.SetType(*type);
    }
  }
  else if( BAIDelta != -1 )  {
    if( abs(type->z - ca.GetType().z) < BAIDelta )  {
      stat.AtomTypeChanges++;
      ca.SetLabel(type->symbol, false);
      ca.SetType(*type);
    }
  }
  else  {
    stat.AtomTypeChanges++;
    ca.SetLabel(type->symbol, false);
    ca.SetType(*type);
  }
  TBasicApp::GetLog().Info(tmp);
  return atc != stat.AtomTypeChanges;
}

};

class  TAtomTypePermutator {
public:
  struct TPermutation  {
    vec3d AtomCenter;
    TCAtom* Atom;
    TTypeList<AnAssociation3<const cm_Element*,double, double> > Tries;
  };
protected:
  TTypeList<TPermutation> Atoms;
  ElementPList TypeRestraints;
  bool Active;
public:
  TAtomTypePermutator()  {  Active = true;  }
  void Init(ElementPList* typeRestraints = NULL);
  void ReInit(const TAsymmUnit& au);
  void InitAtom(TAutoDB::TGuessCount& guess);
  void Permutate();
  DefPropBIsSet(Active)
};

EndXlibNamespace()
#endif
