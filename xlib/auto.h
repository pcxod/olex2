/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_auto_H
#define __olx_xl_auto_H
#include "xbase.h"
#include "estlist.h"
#include "asymmunit.h"
#include "unitcell.h"
#include "lattice.h"
#include "satom.h"
#include "network.h"
#include "xfiles.h"
#include "library.h"
#include "analysis.h"
#include "xapp.h"
#include "olxmps.h"

BeginXlibNamespace()
class TAutoDB;
class TAutoDBNet;
class TAtomTypePermutator;

struct TAutoDBIdObject {
  int32_t id;
  olxstr reference;
  TAutoDBIdObject(int32_t id_)
    : id(id_)
  {}
  TAutoDBIdObject(int32_t id_, const olxstr& ref) : id(id_), reference(ref) {}
  TAutoDBIdObject() : id(-1) {}
  bool operator == (const TAutoDBIdObject& oid) { return id == oid.id; }
  bool operator != (const TAutoDBIdObject& oid) { return id != oid.id; }
};
typedef TPtrList<TAutoDBIdObject>  TAutoDBIdPList;
////////////////////////////////////////////////////////////////////////////////
class TAutoDBRegistry {
  olxstr_dict<TAutoDBIdObject*, true> entries;
public:
  TAutoDBRegistry() {}
  TAutoDBRegistry(const TAutoDBRegistry& dbf)
    : entries(dbf.entries)
  {}
  ~TAutoDBRegistry() {
    Clear();
  }
  void Clear();
  TAutoDBRegistry& operator = (const TAutoDBRegistry& dbf) {
    entries = dbf.entries;
    return *this;
  }
  TAutoDBRegistry(IDataInputStream& in) { LoadFromStream(in); }
  bool Contains(const olxstr& hash) { return entries.HasKey(hash); }
  TAutoDBIdObject& Add(const olxstr& hash, const olxstr& file_name) {
    TAutoDBIdObject* df = new TAutoDBIdObject(-1, file_name),
      * rv;
    if ((rv = entries.Add(hash, df)) != df) {
      delete df;
    }
    return *rv;
  }
  size_t Count() const { return entries.Count(); }
  TAutoDBIdObject& GetIdObject(size_t ind) const {
    return *entries.GetValue(ind);
  }
  const olxstr& GetObjectName(size_t ind) {
    return entries.GetKey(ind);
  }
  void AssignIds() const {
    for (size_t i = 0; i < entries.Count(); i++) {
      entries.GetValue(i)->id = (uint32_t)i;
    }
  }
  void SaveToStream(IDataOutputStream& output) const;
  void LoadFromStream(IDataInputStream& input);
  // saves a map of the hashes to the phisical file names
  void SaveMap(IDataOutputStream& output);
  // loads a map of the hashes to the phisical file names
  void LoadMap(IDataInputStream& input);
};
////////////////////////////////////////////////////////////////////////////////
class TAttachedNode {
  const cm_Element* Element;
  vec3d FCenter;
public:
  TAttachedNode(const cm_Element* element, const vec3d& c)
    : Element(element), FCenter(c) {}
  TAttachedNode(IDataInputStream& in);
  TAttachedNode() : Element(0) {}
  void SaveToStream(IDataOutputStream& output) const;
  void SetType(const cm_Element* e) { Element = e; }
  void SetCenter(const vec3d& c) { FCenter = c; }
  const cm_Element& GetType() const { return *Element; }
  const vec3d& GetCenter() const { return FCenter; }
};

////////////////////////////////////////////////////////////////////////////////
class TAutoDBNode {
  TTypeList<TAttachedNode> AttachedNodes;
  const cm_Element* Element;
  vec3d Center;
  //  int AppendedCount,
  int32_t Id;
  // runtime information
  // this is the "index"
  TTypeList<olx_pair_t<TAutoDBNet*, uint32_t> > Parents;
  evecd Params; // pre-calculated parameters
  void _PreCalc();
  double CalcDistance(size_t i) const {
    return AttachedNodes[i].GetCenter().DistanceTo(Center);
  }
  double CalcAngle(size_t i, size_t j) const;
protected:
  static int SortMetricsFunc(const TAttachedNode& a, const TAttachedNode& b);
  static int SortCAtomsFunc(const olx_pair_t<TCAtom*, vec3d>& a,
    const olx_pair_t<TCAtom*, vec3d>& b);
  static vec3d SortCenter;
  void FromCAtom(const TCAtom& ca, const smatd& m,
    TTypeList<olx_pair_t<TCAtom*, vec3d> >* atoms);
public:
  TAutoDBNode(const TSAtom& sa, TTypeList<olx_pair_t<TCAtom*, vec3d> >* atoms) {
    FromCAtom(sa.CAtom(), sa.GetMatrix(), atoms);
  }
  TAutoDBNode(const TCAtom& ca, const smatd& m,
    TTypeList<olx_pair_t<TCAtom*, vec3d> >* atoms)
  {
    FromCAtom(ca, m, atoms);
  }
  TAutoDBNode(IDataInputStream& in) { LoadFromStream(in); }

  olxstr ToString() const;

  void SaveToStream(IDataOutputStream& output) const;
  void LoadFromStream(IDataInputStream& input);

  void AddParent(TAutoDBNet* net, uint32_t index) {
    Parents.AddNew<TAutoDBNet*, uint32_t>(net, index);
  }
  size_t ParentCount() const { return Parents.Count(); }
  TAutoDBNet* GetParent(size_t i) const { return Parents[i].GetA(); }
  int GetParentIndex(size_t i) const { return Parents[i].GetB(); }

  size_t NodeCount() const { return AttachedNodes.Count(); }
  const TAttachedNode& GetNode(size_t i) const {
    return AttachedNodes[i];
  }
  const cm_Element& GetType() const { return *Element; }
  size_t DistanceCount() const { return AttachedNodes.Count(); }
  double GetDistance(size_t i) const { return Params[i]; }
  double GetAngle(size_t i) const {
    return Params[AttachedNodes.Count() + i];
  }

  void DecodeAngle(size_t index, size_t& nodea, size_t& nodeb) {
    size_t cnt = AttachedNodes.Count(), itr = 0;
    while (index > cnt) {
      index -= cnt;
      cnt--;
      itr++;
    }
    nodea = itr;
    nodeb = index + (AttachedNodes.Count() - cnt);
  }

  DefPropP(int32_t, Id)
    // sorts by connectivity, type and params
    int UpdateCompare(const TAutoDBNode& dbn) const;
  // sorts by connectivity and params
  double SearchCompare(const TAutoDBNode& dbn, double* fom = 0) const;

  bool IsSameType(const TAutoDBNode& dbn) const;
  bool IsSimilar(const TAutoDBNode& dbn) const;
  bool IsMetricSimilar(const TAutoDBNode& dbn, double& fom) const;
};

typedef TPtrList< TAutoDBNode > TAutoDBNodePList;
typedef TTypeList< TAutoDBNode > TAutoDBNodeList;

////////////////////////////////////////////////////////////////////////////////
class TAutoDBNetNode {
  TAutoDBNode* FCenter;
  TPtrList<TAutoDBNetNode>  AttachedNodes;
  int32_t Id, Tag;
public:
  TAutoDBNetNode(TAutoDBNode* node) { FCenter = node; }
  TAutoDBNetNode(IDataInputStream& input) { LoadFromStream(input); }
  void AttachNode(TAutoDBNetNode* node) { AttachedNodes.Add(node); }
  size_t Count() const { return AttachedNodes.Count(); }
  TAutoDBNetNode* Node(size_t i) const { return AttachedNodes[i]; }
  TAutoDBNode* Center() const { return FCenter; }

  bool IsSameType(const TAutoDBNetNode& dbn, bool ExtraLevel) const;
  bool IsMetricSimilar(const TAutoDBNetNode& nd, double& cfom,
    uint16_t* cindexes, bool ExtraLevel) const;

  void SaveToStream(IDataOutputStream& output) const;
  void LoadFromStream(IDataInputStream& input);

  olxstr ToString(int level) const;

  DefPropP(int32_t, Id);
  DefPropP(int32_t, Tag);
};
////////////////////////////////////////////////////////////////////////////////
class TAutoDBNet {
  TTypeList<TAutoDBNetNode> Nodes;
  TAutoDBIdObject* FReference;
protected:
  static TAutoDBNet* CurrentlyLoading;
public:
  TAutoDBNet(TAutoDBIdObject* ref) { FReference = ref; }
  TAutoDBNet(IDataInputStream& input) { LoadFromStream(input); }
  TAutoDBNetNode& NewNode(TAutoDBNode* node) { return Nodes.AddNew(node); }
  const TAutoDBIdObject& GetReference() const { return *FReference; }
  TAutoDBIdObject* Reference() const { return FReference; }
  size_t Count() const { return Nodes.Count(); }
  TAutoDBNetNode& Node(size_t i) { return Nodes[i]; }
  void SaveToStream(IDataOutputStream& output) const;
  void LoadFromStream(IDataInputStream& input);
  bool Contains(TAutoDBNode* nd) const {
    for (size_t i = 0; i < Nodes.Count(); i++)
      if (Nodes[i].Center() == nd) {
        return true;
      }
    return false;
  }
  size_t IndexOf(TAutoDBNode* nd, int start = 0)  const {
    for (size_t i = start; i < Nodes.Count(); i++) {
      if (Nodes[i].Center() == nd) {
        return i;
      }
    }
    return InvalidIndex;
  }
  static TAutoDBNet& GetCurrentlyLoading() {
    return *CurrentlyLoading;
  }
};

////////////////////////////////////////////////////////////////////////////////
class TAutoDBSearchNode {
  TAutoDBNodePList PossibleCentres;
};
////////////////////////////////////////////////////////////////////////////////
class TAutoDB : public IOlxObject {
  // a fixed size list 0 - nodes connected to one other node, 1 - two, etc
  TTypeList< TPtrList<TAutoDBNode> > Nodes;
  TXFile& XFile;
  olxstr src_file;
  //TSStrPObjList<olxstr,TAutoDBFolder*, true> Folders;
  TAutoDBRegistry registry;
  TTypeList<TAutoDBNet> Nets;
  void PrepareForSearch();
protected:
  void ProcessNodes(TAutoDBIdObject* currentFile, TNetwork& net);
  TAutoDBNet* BuildSearchNet(TNetwork& net, TSAtomPList& cas);
  //............................................................................
  TAutoDBNode* LocateNode(size_t index) {
    for (size_t i = 0; i < Nodes.Count(); i++) {
      if (index < Nodes[i].Count()) {
        return Nodes[i][index];
      }
      index -= Nodes[i].Count();
    }
    throw TInvalidArgumentException(__OlxSourceInfo, "index");
  }
  //............................................................................
  void SafeSave(const olxstr& file_name);
public:
  // structre to store analysis statistics
  struct AnalysisStat {
    int AtomTypeChanges, ConfidentAtomTypes, SNAtomTypeAssignments;
    bool FormulaConstrained, AtomDeltaConstrained;
    AnalysisStat() { Clear(); }
    void Clear() {
      AtomTypeChanges = ConfidentAtomTypes = SNAtomTypeAssignments = 0;
      FormulaConstrained = AtomDeltaConstrained = false;
    }
  };
private:
  TDoubleList Uisos;
  AnalysisStat LastStat;
  olxstr LastFileName;
  // maximim element promotion
  int BAIDelta;
  // ratio beyond which search for element promotion
  double URatio, URatioFormula, LengthVar, AngleVar;
  bool EnforceFormula;
public:
  /* the instance must be created with Replicate to avoid problems
   It will be deleted by this object
  */
  TAutoDB(TXFile& xfile, ALibraryContainer& lc);
  virtual ~TAutoDB();

  void ProcessFolder(const olxstr& folder, bool allow_disorder=false,
    double max_r=5.0, double max_shift_over_esd=0.05, double max_GoF_dev=0.1);
  void SaveToStream(IDataOutputStream& output) const;
  void LoadFromStream(IDataInputStream& input);
  void Clear();
protected:
  void AnalyseNet(TNetwork& net, TAtomTypePermutator* permutator,
    double& Uiso, AnalysisStat& stat, bool dry_run,
    ElementPList* proposed_atoms = 0);
  // a helper function to check C-O and C-N
  bool A2Pemutate(TCAtom& a1, TCAtom& a2, const cm_Element& e1,
    const cm_Element& e2, bool dry_run, double threshold);
  // returns bitmask, 1 - a1 should be changed, 2 - a2
  short CheckA2Pemutate(TCAtom& a1, TCAtom& a2, const cm_Element& e1,
    const cm_Element& e2, double threshold);
public:
  void AnalyseStructure(const olxstr& LastFileName, TLattice& latt,
    TAtomTypePermutator* permutator, AnalysisStat& stat, bool dry_run,
    ElementPList* proposed_atoms = 0);

  const TDoubleList& GetUisos() const { return Uisos; }
  const AnalysisStat& GetStats() const { return LastStat; }
  const olxstr& GetLastFileName() const { return LastFileName; }
  void AnalyseNode(TSAtom& sa, TStrList& report);
  static TAutoDB*& GetInstance_() {
    static TAutoDB* inst = 0;
    return inst;
  }
  static TAutoDB& GetInstance();
  TAutoDBNode* Node(size_t i) { return LocateNode(i); }
  TAutoDBIdObject& Reference(size_t i) const { return registry.GetIdObject(i); }
  DefPropP(int, BAIDelta);
  DefPropP(double, URatio);
  DefPropP(double, LengthVar);
  DefPropP(double, AngleVar);
  DefPropBIsSet(EnforceFormula);


  template <class NodeClass>
  struct THitStruct {
    double Fom;
    NodeClass* Node;
    THitStruct(NodeClass* node, double fom) {
      Node = node;
      Fom = fom;
    }
    static int SortByFOMFunc(const THitStruct<NodeClass>& a,
      const THitStruct<NodeClass>& b)
    {
      return olx_cmp(a.Fom, b.Fom);
    }
  };

  template <class NodeClass>
  struct THitList {
    TTypeList< THitStruct<NodeClass> > hits;
    const cm_Element* Type;
    double meanFom;
    static int SortByFOMFunc(const THitList<NodeClass>& a,
      const THitList<NodeClass>& b) {
      const size_t nc = olx_min(a.hits.Count(), b.hits.Count());
      double foma = 0, fomb = 0;
      for (size_t i = 0; i < nc; i++) {
        foma += a.hits[i].Fom;
        fomb += b.hits[i].Fom;
      }
      return olx_cmp(foma, fomb);
    }
    static int SortByCountFunc(const THitList<NodeClass>& a,
      const THitList<NodeClass>& b) {
      return olx_cmp(b.hits.Count(), a.hits.Count());
    }
    THitList(const cm_Element& type, NodeClass* node, double fom) {
      Type = &type;
      hits.AddNew(node, fom);
      meanFom = 0;
    }
    double MeanFom() {
      if (meanFom != 0) {
        return meanFom;
      }
      for (size_t i = 0; i < hits.Count(); i++) {
        meanFom += hits[i].Fom;
      }
      return meanFom /= hits.Count();
    }
    double MeanFomN(size_t cnt) {
      double mf = 0;
      for (size_t i = 0; i < cnt; i++) {
        mf += hits[i].Fom;
      }
      return mf /= cnt;
    }
    void Sort() {
      QuickSorter::SortSF(hits, THitStruct<NodeClass>::SortByFOMFunc);
    }
  };
  struct Type {
    double fom;
    const cm_Element& type;
    Type(double _fom, const cm_Element& _type)
      : fom(_fom), type(_type)
    {}
  };
  struct TGuessCount {
    TCAtom* atom;
    TTypeList<THitList<TAutoDBNode> > list1;
    TTypeList<THitList<TAutoDBNetNode> > list2;
    TTypeList<THitList<TAutoDBNetNode> > list3;
  };
  struct TAnalysisResult {
    TCAtom* atom;
    TTypeList<Type> list1, list2, list3,
      enforced;
  };
  TStrList::const_list_type ValidateResult(const olxstr& fileName,
    const TLattice& latt);

  ConstTypeList<TAnalysisResult> AnalyseStructure(TLattice& latt);
  static bool ChangeType(TCAtom& a, const cm_Element& e, bool dry_run);
protected:
  ConstTypeList<TAnalysisResult> AnalyseNet(TNetwork& net);
  // hits must be sorted beforehand!
  void AnalyseUiso(TCAtom& ca, const TTypeList<THitList<TAutoDBNode> >& list,
    AnalysisStat& stat,
    bool dry_run,
    bool heavier, bool lighter,
    ElementPList* proposed_atoms = 0);

  class TAnalyseNetNodeTask : public TaskBase {
    TTypeList< TPtrList<TAutoDBNode> >& Nodes;
    TAutoDBNet& Network;
    TTypeList<TGuessCount>& Guesses;
    size_t LocateDBNodeIndex(const TPtrList<TAutoDBNode>& segment,
      TAutoDBNode* nd, size_t from = InvalidIndex, size_t to = InvalidIndex);
  public:
    TAnalyseNetNodeTask(TTypeList< TPtrList<TAutoDBNode> >& nodes,
      TAutoDBNet& network, TTypeList<TGuessCount>& guesses) :
      Nodes(nodes), Network(network), Guesses(guesses) {}
    void Run(size_t index);
    TAnalyseNetNodeTask* Replicate() const {
      return new TAnalyseNetNodeTask(Nodes, Network, Guesses);
    }
  };
  //  these are protected, but exposed in the constructor
  void LibBAIDelta(const TStrObjList& Params, TMacroData& E);
  void LibURatio(const TStrObjList& Params, TMacroData& E);
  void LibEnforceFormula(const TStrObjList& Params, TMacroData& E);
  void LibTolerance(const TStrObjList& Params, TMacroData& E);
  void LibLoad(TStrObjList& Cmds, const TParamList& Options, TMacroData& E);
  void LibSave(TStrObjList& Cmds, const TParamList& Options, TMacroData& E);
  void LibDigest(TStrObjList& Cmds, const TParamList& Options, TMacroData& E);
  void LibLock(TStrObjList& Cmds, const TParamList& Options, TMacroData& E);
  class TLibrary* ExportLibrary(const olxstr& name = EmptyString());
  ///////////////////////////////////////////////////////////////////////////////
  template <class NodeType>
  bool AnalyseUiso(TCAtom& ca, const TTypeList< THitList<NodeType> >& list,
    AnalysisStat& stat, bool dry_run, bool heavier, bool lighter,
    ElementPList* proposed_atoms)
  {
    if (!lighter && !heavier) {
      return false;
    }
    bool return_any = !olx_analysis::alg::check_connectivity(ca, ca.GetType());
    olxstr tmp = ca.GetLabel();
    tmp << ' ';
    const cm_Element* type = &ca.GetType();
    if (heavier) {
      if (!dry_run) {
        TBasicApp::NewLogEntry(logVerbose) << "Searching element heavier for " <<
          ca.GetLabel();
      }
      for (size_t j = 0; j < list.Count(); j++) {
        if (list[j].Type->z > type->z) {
          if (proposed_atoms != 0) {
            if (proposed_atoms->IndexOf(list[j].Type) != InvalidIndex) {
              type = list[j].Type;
              break;
            }
          }
          else if (BAIDelta != -1) {
            if ((list[j].Type->z - ca.GetType().z) < BAIDelta) {
              type = list[j].Type;
              break;
            }
          }
          else {
            type = list[j].Type;
            break;
          }
        }
      }
      // last option
      if (type == &ca.GetType() && proposed_atoms != 0) {
        int delta = 100;
        for (size_t i = 0; i < proposed_atoms->Count(); i++) {
          int d = (*proposed_atoms)[i]->z - ca.GetType().z;
          if (d > 0 && d < delta &&
            (return_any ||
              olx_analysis::alg::check_connectivity(ca, *(*proposed_atoms)[i])))
          {
            delta = d;
            type = (*proposed_atoms)[i];
          }
        }
        if (type != &ca.GetType() && BAIDelta != -1 && delta > BAIDelta) {
          type = &ca.GetType();
        }
      }
    }
    else if (lighter) {
      if (!dry_run) {
        TBasicApp::NewLogEntry(logVerbose) << "Searching element lighter for " <<
          ca.GetLabel();
      }
      for (size_t j = 0; j < list.Count(); j++) {
        if (list[j].Type->z < type->z) {
          if (proposed_atoms != 0) {
            if (proposed_atoms->IndexOf(list[j].Type) != InvalidIndex) {
              type = list[j].Type;
              break;
            }
          }
          else if (BAIDelta != -1) {
            if ((ca.GetType().z - list[j].Type->z) < BAIDelta) {
              type = list[j].Type;
              break;
            }
          }
          else {
            type = list[j].Type;
            break;
          }
        }
      }
      // last option
      if (type == &ca.GetType() && proposed_atoms != 0) {
        int delta = 100;
        for (size_t i = 0; i < proposed_atoms->Count(); i++) {
          int d = ca.GetType().z - (*proposed_atoms)[i]->z;
          if (d > 0 && d < delta &&
            (return_any ||
              olx_analysis::alg::check_connectivity(ca, *(*proposed_atoms)[i])))
          {
            delta = d;
            type = (*proposed_atoms)[i];
          }
        }
        if (type != &ca.GetType() && BAIDelta != -1 && delta > BAIDelta) {
          type = &ca.GetType();
        }
      }
    }
    for (size_t j = 0; j < list.Count(); j++) {
      tmp << list[j].Type->symbol << '(' <<
        olxstr::FormatFloat(3, 1.0 / (list[j].MeanFom() + 0.001)) << ")" <<
        list[j].hits[0].Fom;
      if ((j + 1) < list.Count()) {
        tmp << ',';
      }
    }
    if (ChangeType(ca, *type, dry_run)) {
      stat.AtomTypeChanges++;
      if (!dry_run) {
        olx_analysis::helper::reset_u(ca);
        TBasicApp::NewLogEntry(logVerbose) << tmp;
      }
      return true;
    }
    return false;
  }
};

class  TAtomTypePermutator {
public:
  struct TPermutation {
    vec3d AtomCenter;
    TCAtom* Atom;
    TTypeList<AnAssociation3<const cm_Element*, double, double> > Tries;
  };
protected:
  TTypeList<TPermutation> Atoms;
  ElementPList TypeRestraints;
  bool Active;
public:
  TAtomTypePermutator() { Active = true; }
  void Init(ElementPList* typeRestraints = 0);
  void ReInit(const TAsymmUnit& au);
  void InitAtom(TAutoDB::TGuessCount& guess);
  void Permutate();
  DefPropBIsSet(Active);
};

EndXlibNamespace()
#endif
