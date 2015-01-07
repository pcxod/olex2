/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_analysis_H
#define __olx_xlib_analysis_H
#include "lattice.h"
#include "symmlib.h"
#include "xapp.h"
#include "filetree.h"

BeginXlibNamespace()
namespace olx_analysis {
namespace alg {
  double mean_peak(const TCAtomPList &peaks);
  double mean_u_eq(const TCAtomPList &atoms);
  olxstr formula(const TCAtomPList &atoms, double mult=1);
  olxstr label(const TCAtomPList &atoms, const olxstr &sp=EmptyString());
  olxstr label(const TCAtomGroup &atoms, const olxstr &sp=EmptyString());
  // applicable to TS/XAtom containers
  template <class atom_plist_t>
  olxstr label(const atom_plist_t &atoms, bool add_sym, const olxstr &sp) {
    olxstr_buf rv;
    for( size_t i=0; i < atoms.Count(); i++ ) {
      if (!rv.IsEmpty())  rv << sp;
      rv << atoms[i]->GetGuiLabel();
    }
    return rv;
  }
  // returns true if any of the atoms is from the original asymmetric unit
  template <class atom_plist_t> bool has_I(const atom_plist_t &atoms) {
    for( size_t i=0; i < atoms.Count(); i++ )
      if (atoms[i]->GetMatrix().IsFirst())
        return true;
    return false;
  }
  // returns true if all atoms are from the original asymmetric unit
  template <class atom_plist_t> bool are_all_I(const atom_plist_t &atoms) {
    for( size_t i=0; i < atoms.Count(); i++ )
      if (!atoms[i]->GetMatrix().IsFirst())
        return false;
    return true;
  }
  const cm_Element &find_heaviest(const TCAtomPList &atoms);
  /* checks if new enviroment is better or as good as the old one
  */
  bool check_geometry(const TCAtom &a, const cm_Element &e);
  /* does basic chemical connectivity checks
  */
  bool check_connectivity(const TCAtom &a, const cm_Element &e);
  ConstArrayList<size_t> find_hetero_indices(const TCAtomPList &atoms,
    const cm_Element *re=NULL);
  /* locates an evironment around the given point (in fractional coordinates)
  and rates it according to simple geometrical measurements as distances and
  angles
  */
  double rate_envi(const TUnitCell &uc, const vec3d& fcrd, double r);
}; // end namespace alg
namespace helper {
  ConstSortedElementPList get_user_elements();
  // resets the atom ADP
  void reset_u(TCAtom &a, double r=0.025);
  // also deletes any riding groups
  void delete_atom(TCAtom &a);
  size_t get_demoted_index(const cm_Element &e, const SortedElementPList &elms);
  bool can_demote(const cm_Element &e, const SortedElementPList &elms);
  // as above, but also checks if the demoted type conenctivity is OK
  bool can_demote(const TCAtom &e, const SortedElementPList &elms);
}; // end namespace helper
struct peaks {
  static int peak_sort(const TCAtom &a1, const TCAtom &a2) {
    return olx_cmp(a2.GetQPeak(), a1.GetQPeak());
  }
  static ConstPtrList<TCAtom> extract(TAsymmUnit &au,
    bool *all_peaks=NULL);
  struct range {
  protected:
    mutable double mean;
  public:
    TCAtomPList peaks;
    range() : mean(0) {}
    double get_mean(bool update=false) const {
      return (mean != 0 && !update) ? mean : (mean=alg::mean_peak(peaks));
    }
    void delete_all();
  };
  struct result {
    bool only_peaks;
    size_t peak_count;
    double mean_peak;
    TTypeList<range> peak_ranges;
  };
  static ConstTypeList<range> analyse(const TCAtomPList &peaks);
  static result analyse_full(TAsymmUnit &au);
  static TCAtomPList &proximity_clean(TCAtomPList &peaks);
};

struct fragments {
protected:
  // recursive exansion helper function
  static void expand_node(TCAtom &a, TCAtomPList &atoms);
public:
  struct tree_node {
    TCAtomPList trunk;
    TTypeList<tree_node> branches;
    int Compare(const tree_node &b) const {
      return olx_cmp(trunk[0]->GetTag(), b.trunk[0]->GetTag());
    }
    void sort() {
      trunk.ForEach(ACollectionItem::IndexTagSetter());
      QuickSorter::Sort(branches, TComparableComparator());
    }
    // reverses the trunk direction and re-sorts the branches
    void reverse();
  };
  struct cart_atom {
    TCAtom &atom;
    vec3d xyz;
    smatd matrix;
    cart_atom(TCAtom *a, const vec3d &c, const smatd &m)
      : atom(*a), xyz(c), matrix(m)
    {}
  };
  struct cart_plane {
    vec3d center, normal;
    double rmsd;
    cart_plane() : rmsd(0) {}
    cart_plane(const vec3d &c, const vec3d &n, double rmsd_=0)
      : center(c), normal(n), rmsd(rmsd_)
    {}
    double distance_to(const vec3d &v) const {
      return normal.DotProd(v-center);
    }
    double angle(const vec3d &v) const {
      return acos(normal.CAngle(v)) * 180 / M_PI;
    }
  };
  struct cart_ring {
    TTypeList<cart_atom> atoms;
    cart_atom &operator [] (size_t i) { return atoms[i];  }
    const cart_atom &operator [] (size_t i) const { return atoms[i]; }
    vec3d calc_center() const;
    cart_plane calc_plane() const;
    bool is_regular() const;
  };
  struct ring {
    struct substituent {
      // always start from the ring atom
      TCAtomPList atoms;
      tree_node tree;
      ring &parent;
      size_t ring_count;
      substituent(ring &parent_, TCAtom &a)
        : parent(parent_), ring_count(0)
      {
        atoms.Add(a);
      }
      int Compare(const substituent &s) const;
      struct atom_cmp {
        atom_cmp() {}
        template <class item_a_t, class item_b_t>
        int Compare(const item_a_t &a1_, const item_b_t &a2_) const {
          const TCAtom &a1 = olx_ref::get(a1_), &a2 = olx_ref::get(a2_);
          int r = olx_cmp(a1.GetTag(), a2.GetTag());
          if (r == 0) r = olx_cmp(a1.GetType().z, a2.GetType().z);
          return r;
        }
      };
    };
    size_t fused_count;
    ring(ConstPtrList<TCAtom> atoms_)
      : fused_count(0), atoms(atoms_)
    {}
    TTypeList<substituent> substituents;
    TCAtomPList atoms;
    bool is_leq(const ring &r) const;
    // checks if the rings shares two subsequent atoms with another ring
    bool is_fused_with(const ring &r) const;
    // merges fused ring into one, big ring, r is merged into this ring
    bool merge(ring &r);
    int Compare(const ring &r) const;
    static int SizeCompare(const ring &r1, const ring &r2) {
      return olx_cmp(r1.atoms.Count(), r2.atoms.Count());
    }
    void reverse();
    cart_ring to_cart() const;
  };
  struct fragment {
  protected:
    mutable double u_eq;
    TCAtomPList atoms_;
    smatd_list generators;
    void init_generators();
    static void build_coordinate(
      TCAtom &a, const smatd &m, vec3d_list &res);
    ConstTypeList<vec3d> build_coordinates() const;
    ConstPtrList<TCAtom> trace_ring(TCAtom &a);
    void trace_substituent(ring::substituent &s);
    static ConstPtrList<TCAtom> ring_sorter(const TCAtomPList &r);
    void init_ring(size_t i, TTypeList<ring> &rings);
    /* expects graph prepared by trace substituent or by the breadth-first
    expansion, returns the atom where the branch's trunk stops.
    a - the atom at the far end of the branch
    */
    static TCAtom *trace_branch(TCAtom *a, tree_node &b);
    static tree_node &trace_tree(TCAtomPList &atoms, tree_node &root);
  public:
    fragment() : u_eq(0) {}
    double get_mean_u_eq(bool update=false) const {
      return (u_eq != 0 && !update) ? u_eq : (u_eq=alg::mean_u_eq(atoms_));
    }
    TCAtomPList &atoms() { return atoms_; }
    const TCAtomPList &atoms() const { return atoms_; }
    size_t count() const { return atoms_.Count(); }
    TCAtom &operator [] (size_t i) const { return *atoms_[i]; }
    void set_atoms(const TCAtomPList &atoms) {
      atoms_ = atoms;
      init_generators();
    }
    bool is_regular() const;
    bool is_flat() const;
    bool is_polymeric() const;
    fragment &pack() {
      atoms_.Pack(TCAtom::FlagsAnalyser(catom_flag_Deleted));
      return *this;
    }
    // works only for a group of atoms distributed around the central one
    size_t find_central_index() const;
    olxstr formula() const { return alg::formula(atoms_); }
    void breadth_first_tags(size_t start=InvalidIndex,
      TCAtomPList *ring_atoms=NULL);
    /* traces the rings back from the breadth-first tag assignment */
    ConstTypeList<ring> get_rings(const TCAtomPList &r_atoms);
    /* finds ring substituents and sorts substituents and the rings */
    void init_rings(TTypeList<ring> &rings);
    tree_node build_tree();
  };
  static ConstTypeList<fragment> extract(TAsymmUnit &au);
};

class Analysis {
  static int hr_sort(
    const olx_pair_t<double, olxstr> *a1,
    const olx_pair_t<double, olxstr> *a2)
  {
    return olx_cmp(a1->GetA(), a2->GetA());
  }
public:
  static bool trim_18(TAsymmUnit &au);

  static bool analyse_u_eq(TAsymmUnit &au);

  static double find_scale(TLattice &latt);

  static const cm_Element &check_proposed_element(
    TCAtom &a, const cm_Element &e, ElementPList *set=NULL);

  static const cm_Element &check_atom_type(TSAtom &a);

  static void funTrim(const TStrObjList& Params, TMacroError& E)  {
    E.SetRetVal(trim_18(TXApp::GetInstance().XFile().GetAsymmUnit()));
  }

  static void funAnaluseUeq(const TStrObjList& Params, TMacroError& E)  {
    E.SetRetVal(analyse_u_eq(TXApp::GetInstance().XFile().GetAsymmUnit()));
  }

  static void funFindScale(const TStrObjList& Params, TMacroError& E)  {
    bool apply = Params.IsEmpty() ? false : Params[0].ToBool();
    TLattice &latt = TXApp::GetInstance().XFile().GetLattice();
    double scale = find_scale(latt);
    if (scale > 0) {
      for (size_t i=0; i < latt.GetObjects().atoms.Count(); i++) {
        TSAtom &a = latt.GetObjects().atoms[i];
        if (a.GetType() == iQPeakZ && apply) {
          int z = olx_round(a.CAtom().GetQPeak()*scale);
          cm_Element *tp = XElementLib::FindByZ(z),
             *tp1 = NULL;
          // find previous halogen vs noble gas or alkaline metal
          if (tp != NULL) {
            if (XElementLib::IsGroup8(*tp) ||
                XElementLib::IsGroup1(*tp) ||
                XElementLib::IsGroup2(*tp))
            {
              tp1 = XElementLib::PrevGroup(7, tp);
            }
            a.CAtom().SetType(tp1 == NULL ? *tp : *tp1);
          }
        }
      }
    }
    E.SetRetVal(scale);
  }

  static TLibrary *ExportLibrary(const olxstr& name="analysis")  {
    TLibrary* lib = new TLibrary(name);
    lib->Register(
      new TStaticFunction(&Analysis::funTrim, "Trim", fpNone|fpOne,
      "Trims the size of the assymetric unit according to the 18 A^3 rule."
      "Returns true if any atoms were deleted")
    );
    lib->Register(
      new TStaticFunction(&Analysis::funFindScale, "Scale", fpNone|fpOne,
      "Scales the Q-peaks according to found fragments."
      "Returns the scale or 0")
    );
    lib->Register(
      new TStaticFunction(&Analysis::funAnaluseUeq, "AnalyseUeq", fpNone|fpOne,
      ""
      "")
    );
    return lib;
  }
};
}; // end namespace olx_analysis
EndXlibNamespace()
#endif
