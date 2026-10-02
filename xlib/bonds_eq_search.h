/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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
  static void init_equivs(TPtrList<TCAtom>& Atoms,
    const smatd_list& Matrices, bool check_translations);
  static void process_1(TPtrList<TCAtom>& Atoms, size_t i, size_t j,
    const smatd_list& Matrices, bool check_translations, olx_critical_section *);
  static void process_t(TCAtom& from, TCAtom& to,
    const smatd& M, const vec3i& done_shift, olx_critical_section*);
};

enum {
  BondsSymmEqTaskDefault = 0, // auto-decide depending on the structure
  BondsSymmEqTaskDirect,
  BondsSymmEqTaskShells,
  BondsSymmEqTaskCellList
};

//..............................................................................
//..............................................................................
//..............................................................................
struct BondsSymmEqTaskFactory {
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
  bool SkipTranslations;
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
    double len, slen;
    vec3d center;
    TCAtom* atom;
    Shell* parent;
    uint32_t mat_id;
    AtomInfo(const vec3d& c, TCAtom* a, uint32_t mat_id = FirstMatrixRawId)
      : center(c), atom(a), parent(0), mat_id(mat_id)
    {}
    int Compare(const AtomInfo& o) const {
      return olx_cmp(this->len, o.len);
    }
  };
  struct Shell {
    TPtrList<AtomInfo> data;
    vec3d center;
    static int Compare(const AtomInfo& a1, const AtomInfo& a2) {
      return olx_cmp(a1.slen, a2.slen);
    }
  };

  TPtrList<TCAtom>& atoms;
  const smatd_list& matrices;
  const TUnitCell* UC;
  typedef TTypeList<AtomInfo> data_t;
  olx_object_ptr<data_t> data_;
  olx_object_ptr<TPtrList<AtomInfo> > au_atoms_;
  typedef TTypeList<Shell> shell_data_t;
  olx_object_ptr<shell_data_t> shells_;
  double delta, deltaI, shell_thickness, max_r;
protected:
  TBondsSymmEqTaskShells(const TBondsSymmEqTaskShells &other)
    : atoms(other.atoms), matrices(other.matrices), UC(other.UC),
    data_(other.data_), au_atoms_(other.au_atoms_),
    shells_(other.shells_), delta(other.delta), deltaI(other.deltaI),
    shell_thickness(other.shell_thickness)
  {}
  void CheckShell(const Shell& shell, const vec3d& v, TCAtom* self_atom,
    double max_ql) const;
  void ProcessMatch(TCAtom* atom, AtomInfo* match, double qd) const;
public:
  TBondsSymmEqTaskShells(TPtrList<TCAtom>& atoms, const smatd_list& matrices);

  void Run(size_t ind) const;
  void InitEquiv() const;
  IBondsSymmEqTask* Replicate() const {
    return new TBondsSymmEqTaskShells(*this);
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
