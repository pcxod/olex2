#pragma once
#include "olxmps.h"
#include "asymmunit.h"
#include "lattice.h"

BeginXlibNamespace()

class IBondsSymmEqTask : public TaskBase {
public:
  virtual ~IBondsSymmEqTask() {}
  virtual void Run(size_t ind) const = 0;
  virtual void InitEquiv() const = 0;
  virtual IBondsSymmEqTask* Replicate() const = 0;
};

enum {
  BondsSymmEqTaskDefault = 0,
  BondsSymmEqTaskDirect,
  BondsSymmEqTaskShells,
  BondsSymmEqTaskCellList
};

//..............................................................................
//..............................................................................
//..............................................................................
struct BondsSymmEqTaskFctory {
  typedef olx_object_ptr<IBondsSymmEqTask> ISearchSymmEqTaskPtr;
  static int& default_type() {
    static int v = BondsSymmEqTaskDefault;
    return v;
  }
  static ISearchSymmEqTaskPtr
    build(TPtrList<TCAtom>& atoms, const smatd_list& matrices, int type=BondsSymmEqTaskDefault);
};

class TBondsSymmEqTaskDirect : public IBondsSymmEqTask {
  TPtrList<TCAtom>& Atoms;
  const smatd_list& Matrices;
  TAsymmUnit* AU;
  TLattice* Latt;
public:
  TBondsSymmEqTaskDirect(TPtrList<TCAtom>& atoms, const smatd_list& matrices);
  void Run(size_t ind) const;
  void InitEquiv() const;
  IBondsSymmEqTask* Replicate() const {
    return new TBondsSymmEqTaskDirect(Atoms, Matrices);
  }
};
//..............................................................................
//..............................................................................
//..............................................................................
class TBondsSymmEqTaskShells : public IBondsSymmEqTask {
  struct Shell;
  struct AtomInfo {
    double ql;
    vec3d center;
    TCAtom* atom;
    Shell* parent;
    uint32_t mat_id;
    AtomInfo(const vec3d& c, TCAtom* a, uint32_t mat_id = FirstMatrixRawId)
      : center(c), atom(a), parent(0), mat_id(mat_id)
    {
    }
    int Compare(const AtomInfo& o) const {
      return olx_cmp(this->ql, o.ql);
    }
  };
  struct Shell {
    TPtrList<AtomInfo> data;
    vec3d center;
    static int Compare(const AtomInfo& a1, const AtomInfo& a2) {
      return olx_cmp((a1.center - a1.parent->center).QLength(),
        (a2.center - a2.parent->center).QLength());
    }

  };

  TPtrList<TCAtom>& atoms;
  typedef TTypeList<AtomInfo> data_t;
  olx_object_ptr<data_t> data_;
  typedef TTypeList<Shell> shell_data_t;
  olx_object_ptr<shell_data_t> shells_;
  vec3d min_d;
protected:
  TBondsSymmEqTaskShells(TPtrList<TCAtom>& atoms,
    const olx_object_ptr<data_t>& d,
    const olx_object_ptr<shell_data_t>& s)
    : atoms(atoms), data_(d), shells_(s)
  {}
  void CheckShell(const Shell& shell, const vec3d& v, TCAtom* self_atom,
    double max_b_ql, TLinkedList<AtomInfo*>& matches) const;
  void ProcessMatch(TCAtom* atom, AtomInfo* match, double qd);
public:
  TBondsSymmEqTaskShells(TPtrList<TCAtom>& atoms, const smatd_list& matrices);

  void Run(size_t ind) const;
  void InitEquiv() const;
  IBondsSymmEqTask* Replicate() const {
    return new TBondsSymmEqTaskShells(atoms, data_, shells_);
  }
};
//..............................................................................
//..............................................................................
//..............................................................................
class TBondsSymmEqTaskCellList : public IBondsSymmEqTask {
  TPtrList<TCAtom>& Atoms;
  const smatd_list& Matrices;
  TAsymmUnit* AU;
  TLattice* Latt;
  /* per atom, ascending, the atoms after it close enough to find anything.
  NULL to test every pair, which is quicker for a small structure
  */
  olx_object_ptr<TArrayList<TSizeList> > Neighbours_;
  /* the cell is wide enough that no -1..1 shift but the one already tested
  can reach anything, so the loop goes. Decided once, see FindSymmEq
  */
  bool SkipTranslations;
protected:
  TBondsSymmEqTaskCellList(TPtrList<TCAtom>& atoms, const smatd_list& matrices,
    olx_object_ptr<TArrayList<TSizeList> > Neighbours, bool skip_translations);
public:
  TBondsSymmEqTaskCellList(TPtrList<TCAtom>& atoms, const smatd_list& matrices);
  void Run(size_t ind) const;
  void InitEquiv() const;
  IBondsSymmEqTask* Replicate() const {
    return new TBondsSymmEqTaskCellList(Atoms, Matrices, Neighbours_,
      SkipTranslations);
  }
};

EndXlibNamespace()
