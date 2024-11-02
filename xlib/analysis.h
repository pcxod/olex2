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
#include "egraph.h"

BeginXlibNamespace()
namespace olx_analysis {
namespace alg {
  double mean_peak(const TCAtomPList &peaks);
  double mean_u_eq(const TCAtomPList &atoms);
  olxstr formula(const TCAtomPList &atoms, double mult=1);
  olxstr label(const TCAtomPList &atoms, const olxstr &sp=EmptyString());
  olxstr label(const TCAtomGroup &atoms, const olxstr &sp=EmptyString());
  // if the atoms share a single residue -it is returned, 0 otherwise
  template <class atom_plist_t, class accessor_t>
  TResidue* get_resi(const atom_plist_t& atoms, const accessor_t& ac=DummyAccessor()) {
    if (atoms.IsEmpty()) {
      return 0;
    }
    size_t r_id = ac(atoms[0]).GetResiId();
    for (size_t i = 1; i < atoms.Count(); i++) {
      if (ac(atoms[i]).GetResiId() != r_id) {
        return 0;
      }
    }
    return &ac(atoms[0]).GetParent()->GetResidue(r_id);
  }
  // applicable to TS/XAtom containers
  template <class atom_plist_t>
  olxstr label(const atom_plist_t &atoms, bool add_sym, const olxstr &sp) {
    olxstr_buf rv;
    for( size_t i=0; i < atoms.Count(); i++ ) {
      rv << sp << (add_sym ? atoms[i]->GetGuiLabelEx() : atoms[i]->GetGuiLabel());
    }
    return rv.IsEmpty() ? EmptyString() : olxstr(rv).SubStringFrom(sp.Length());
  }
  // returns true if any of the atoms is from the original asymmetric unit
  template <class atom_plist_t> bool has_I(const atom_plist_t &atoms) {
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (atoms[i]->GetMatrix().IsFirst()) {
        return true;
      }
    }
    return false;
  }
  // returns true if all atoms are from the original asymmetric unit
  template <class atom_plist_t> bool are_all_I(const atom_plist_t &atoms) {
    for (size_t i = 0; i < atoms.Count(); i++) {
      if (!atoms[i]->GetMatrix().IsFirst()) {
        return false;
      }
    }
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
    const cm_Element *re=0);
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
  bool delete_atom(TCAtom &a);
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
    bool *all_peaks=0);
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
  // identifies an atomic site, matrix*atom.ccrd()
  struct node : public ACollectionItem {
    TCAtom& atom;
    smatd matrix;
    TTypeList<node> nodes;
    node(TCAtom &a, const smatd& m) : atom(a), matrix(m) {}
    node(const TCAtom::Site &s) : atom(*s.atom), matrix(s.matrix)
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
    ring(const TCAtomPList &atoms_)
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
    void reverse() {
      olx_reverse(atoms);
    }
    cart_ring to_cart() const;
  };
  struct fragment {
    typedef TEGraphNode<uint64_t, TCAtom *> node_t;
  protected:
    mutable double u_eq;
    TCAtomPList atoms_;
    smatd_list generators;
    bool polymeric;
    void init_generators();
    mutable TTypeList<olx_pair_t<vec3d, size_t> > crds_;
    // traces a ring from breadth-first expansion
    TTypeList<TCAtomPList>::const_list_type trace_ring_b(TQueue<TCAtom*> &q,
      TCAtomPList &ring, bool flag=false) const;
    // traces a ring from breadth-first expansion
    TTypeList<TCAtomPList>::const_list_type trace_ring_b(TCAtom &a) const;
    /* traces a ring from depth-first expansion. The ring tags must go in
    descending order
    */
    static ConstPtrList<TCAtom> trace_ring_d(TCAtom &a);
    void trace_substituent(ring::substituent &s) const;
    static ConstPtrList<TCAtom> ring_sorter(const TCAtomPList &r);
    void init_ring(size_t i, TTypeList<ring> &rings) const;
    /* expects graph prepared by trace substituent or by the breadth-first
    expansion, returns the atom where the branch's trunk stops.
    a - the atom at the far end of the branch
    */
    static TCAtom *trace_branch(TCAtom *a, tree_node &b);
    static tree_node &trace_tree(TCAtomPList &atoms, tree_node &root);
    uint64_t calc_node_hash(node_t& node) const;
    static uint64_t mix_node_hash(node_t& node,
      const olxdict<TCAtom*, node_t *, TPointerComparator>& map);
    static void mix_hashes(node_t& node,
      const olxdict<TCAtom *, node_t *, TPointerComparator>& map,
      olxdict<node_t *, uint64_t, TPointerComparator>& vs)
    {
      vs(&node, mix_node_hash(node, map));
      for (size_t i = 0; i < node.Count(); i++) {
        mix_hashes(node[i], map, vs);
      }
    }
    void calc_hashes(node_t& node) const {
      node.SetData(calc_node_hash(node));
      for (size_t i = 0; i < node.Count(); i++) {
        calc_hashes(node[i]);
      }
      //graphNode.GetObject()->CAtom().SetLabel(graphNode.GetData(), false);
    }
    static void assign_hashes(node_t& node,
      const olxdict<node_t *, uint64_t, TPointerComparator>& vs)
    {
      node.SetData(vs.Get(&node));
      for (size_t i = 0; i < node.Count(); i++) {
        assign_hashes(node[i], vs);
      }
      //graphNode.GetObject()->CAtom().SetLabel(graphNode.GetData(), false);
    }
    /* assumes that all atoms are masked with 0, atoms of B - 2, neighbours of
    B - 1. After finishing - matching set is marked with 3, atoms of B - with 4
    */
    static bool does_match(TCAtom &a, TCAtom &b, TCAtomPList &p);
  public:
    fragment() : u_eq(0), polymeric(false)
    {}
    fragment(const fragment &f)
      : u_eq(f.u_eq),
      atoms_(f.atoms_),
      generators(f.generators),
      polymeric(f.polymeric)
    {}
    fragment(const TCAtomPList &atoms) : u_eq(0), polymeric(false) {
      set_atoms(atoms);
    }
    double get_mean_u_eq(bool update = false) const {
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
    // index refers to the atom Id in the AU
    const TTypeList<olx_pair_t<vec3d, size_t> > &build_coordinates_ext() const;
    ConstTypeList<vec3d> build_coordinates() const {
      vec3d_list rv(build_coordinates_ext(),
        FunctionAccessor::MakeConst(&olx_pair_t<vec3d,size_t>::GetA));
      return rv;
    }
    bool is_disjoint() const;
    bool is_regular() const;
    bool is_flat() const;
    // checks if symmetry generators is empty
    bool is_complete() const { return generators.Count() < 2; }
    bool is_polymeric() const { return polymeric; }
    fragment &pack() {
      atoms_.Pack(TCAtom::FlagsAnalyser(catom_flag_Deleted));
      return *this;
    }
    // works only for a group of atoms distributed around the central one
    size_t find_central_index() const;
    olxstr formula() const { return alg::formula(atoms_); }
    void breadth_first_tags(size_t start = InvalidIndex,
      TCAtomPList *ring_atoms = 0) const
    {
      breadth_first_tags(atoms(), start, ring_atoms);
    }
    void depth_first_tags() const {
      depth_first_tags(atoms());
    }
    /* traces the rings back from the breadth-first tag assignment */
    ConstTypeList<ring> get_rings(const TCAtomPList &r_atoms) const;
    /* finds ring substituents and sorts substituents and the rings */
    void init_rings(TTypeList<ring> &rings) const;
    tree_node build_tree();
    void mask_neighbours(index_t a_tag, index_t nbh_tag) const {
      mask_neighbours(atoms(), a_tag, nbh_tag);
    }
    /* recursively sets atom's tag to the tag value and then sets incremented
    value of the tag to the neighbpurs of the atom. Only affects atoms with tag
    value of -1
    */
    static void depth_first_tag(TCAtom &a, index_t tag);
    /* sets all atoms tags to -1 and then does depth first expansion incremental
    assignment of tags starting from 0.
    */
    static void depth_first_tags(const TCAtomPList &atoms);
    /* atoms should reprent connected set, the expansion starts 0th atom or
    atom given by start.
    */
    static void breadth_first_tags(const TCAtomPList &atoms,
      size_t start = InvalidIndex, TCAtomPList *ring_atoms = NULL);
    olx_object_ptr<node_t> build_graph() const;
    /* orders the list in such a way elemets with larger Z come and more bonds
    come first
    */
    static void order_list(TCAtomPList &l);
    /* builds a graph matching the given one and based on the root atom. The
    function succeeds if the returned list item count equals to the number of
    atoms in the reference graph. For efficiency, the atoms of the reference
    fragment should be ordered using order_list function and should represent
    a connected set of atoms.
    */
    static TCAtomPList::const_list_type get_matching_set(TCAtom &root,
      const fragment &f);
    /* counts the number of valid (real type and not deleted) neighbours with
    the given tag the atom has
    */
    static size_t get_neighbour_count(const TCAtom &a, index_t tag);
    // as above - tag is ignored
    static size_t get_neighbour_count(const TCAtom& a);
    /* sets tags for the atoms and their neighbours - this is enought to mask
    the atoms out of the whole AU
    */
    static void mask_neighbours(const TCAtomPList &atoms,
      index_t a_tag, index_t nbh_tag);
    friend struct fragments;
  };
  static ConstTypeList<fragment> extract(TAsymmUnit &au);
  /* extracts all given fragments from the given atoms
  */
  static ConstTypeList<fragment> extract(const TCAtomPList &atoms,
    const fragment &f, const olx_pset<int> *parts=0);
  static ConstTypeList<fragment> extract(TAsymmUnit &au, const fragment &f,
    const olx_pset<int>* parts=0)
  {
    return extract(au.GetAtoms(), f, parts);
  }
};

class Framework {
public:
  struct Edge {
    class Vertex* that;
    smatd matrix;
    Edge()
      : that(0)
    {}
  };
  struct Vertex : public ACollectionItem {
    Vertex(size_t id)
      : id(id)
    {}

    size_t id;
    vec3d center;
    TTypeList<Edge> neighbours;
    TCAtomPList atoms;
  };

  struct Linker {
    TCAtomPList atoms;
    olxset<Vertex*, TPointerComparator> vertices;
  };

  TCAtomPList atoms;
  TTypeList<Vertex> vertices;
  TTypeList<Linker> linkers;
  // node tags must be gt 1
  Framework(const fragments::fragment& frag,
    const olx_pset<index_t> &nodes);

  typedef AnAssociation3<Vertex*, smatd, bool> conn_atom_t;
  TTypeList<TTypeList<conn_atom_t> > conn_info;
  void set_tags(Vertex &v);
  void set_tags_(Vertex& v,
    TCAtom& a, const smatd& m,
    TTypeList<conn_atom_t>& dest);
  olxdict<index_t, Vertex*, TPrimitiveComparator> vertex_map;

  void describe() const;
};

class NetTools {
public:
  typedef AnAssociation3<TCAtom*, smatd, bool> conn_atom_t;
  typedef olx_pair_t<TCAtom*, vec3d> atom_t;
  // atom and its position in the fragment
  NetTools(fragments::fragment &frag, const TCAtomPList &nodes, bool verbose=false);
  
  TTypeList<TTypeList<atom_t> >::const_list_type extract_triangles();
private:
  bool verbose;
  TCAtomPList nodes;
  olxdict<size_t, size_t, TPrimitiveComparator> conn_map;
  TTypeList<TTypeList<conn_atom_t> > conn_info;
  void set_tags(TCAtom& a);
  void set_tags_(TCAtom& a, const smatd& m,
    TTypeList<conn_atom_t>& dest);
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

  static void funTrim(const TStrObjList& Params, TMacroData& E) {
    E.SetRetVal(trim_18(TXApp::GetInstance().XFile().GetAsymmUnit()));
  }

  static void funAnaluseUeq(const TStrObjList& Params, TMacroData& E) {
    E.SetRetVal(analyse_u_eq(TXApp::GetInstance().XFile().GetAsymmUnit()));
  }

  static void funFindScale(const TStrObjList& Params, TMacroData& E);

  static TLibrary *ExportLibrary(const olxstr& name = "analysis");
};

namespace chirality {
  /* analysis R/S chirality of the atoms and if the atom is chiral - 
  returns a string of substituents. Forn no chiral atoms returns an empty
  string.
  */
  olxstr rsa_analyse(TCAtom &a, bool debug=false);
}; // namespace chirality
}; // end namespace olx_analysis
EndXlibNamespace()
#endif
