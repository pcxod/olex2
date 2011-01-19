#ifndef __olx_xl_symspace_H
#define __olx_xl_symspace_H
#include "symmat.h"
#include "rot_id.h"
BeginXlibNamespace()

template <class MatList> class TSymSpace  {
  MatList ml;
  const mat3d &cart2cell, &cell2cart, &hkl2cart;
  bool centrosymmetric;
  size_t start;
protected:
  TSymSpace(const TSymSpace& sp, size_t _start) :
    ml(sp.ml), 
    cart2cell(sp.cart2cell),
    cell2cart(sp.cell2cart),
    hkl2cart(sp.hkl2cart),
    start(sp.start+_start),
    centrosymmetric(sp.centrosymmetric)  {}
public:
  TSymSpace(const MatList& _ml,
    const mat3d& _cart2cell,
    const mat3d& _cell2cart,
    const mat3d& _hkl2cart,
    bool _centrosymmetric) :
      ml(_ml), 
      cart2cell(_cart2cell),
      cell2cart(_cell2cart),
      hkl2cart(_hkl2cart),
      start(0),
      centrosymmetric(_centrosymmetric)  {}

  TSymSpace(const TSymSpace& sp) :
    ml(sp.ml), 
    cart2cell(sp.cart2cell),
    cell2cart(sp.cell2cart),
    hkl2cart(sp.hkl2cart),
    start(sp.start),
    centrosymmetric(sp.centrosymmetric)  {}
    
  inline const smatd& operator [] (size_t i) const {  return ml[i+start];  }
  inline size_t Count() const {  return ml.Count()-start;  }
  inline bool IsEmpty() const {  return Count() == 0;  }
  bool IsCentrosymmetric() const {  return centrosymmetric;  }
  template <typename vec_type> vec3d Orthogonalise(const vec_type& v) const {
    return vec3d(
      v[0]*cell2cart[0][0] + v[1]*cell2cart[1][0] + v[2]*cell2cart[2][0],
      v[1]*cell2cart[1][1] + v[2]*cell2cart[2][1],
      v[2]*cell2cart[2][2]);
  }
  template <typename vec_type> vec_type& OrthogonaliseI(vec_type& v) const {
    v[0] = v[0]*cell2cart[0][0] + v[1]*cell2cart[1][0] + v[2]*cell2cart[2][0];
    v[1] = v[1]*cell2cart[1][1] + v[2]*cell2cart[2][1];
    v[2] = v[2]*cell2cart[2][2];
    return v;
  }
  template <typename vec_type> vec3d Fractionalise(const vec_type& v) const {
    return vec3d(
      v[0]*cart2cell[0][0] + v[1]*cart2cell[1][0] + v[2]*cart2cell[2][0],
      v[1]*cart2cell[1][1] + v[2]*cart2cell[2][1],
      v[2]*cart2cell[2][2]);
  }
  template <typename vec_type> vec_type& FractionaliseI(vec_type& v) const {
    v[0] = v[0]*cart2cell[0][0] + v[1]*cart2cell[1][0] + v[2]*cart2cell[2][0];
    v[1] = v[1]*cart2cell[1][1] + v[2]*cart2cell[2][1];
    v[2] = v[2]*cart2cell[2][2];
    return v;
  }
  template <typename vec_type> vec3d HklToCart(const vec_type& v) const {
    return vec3d(
      v[0]*hkl2cart[0][0],
      v[0]*hkl2cart[0][1] + v[1]*hkl2cart[1][1],
      v[0]*hkl2cart[0][2] + v[1]*hkl2cart[1][2] + v[2]*hkl2cart[2][2]
    );
  }
  TSymSpace<MatList> SubListFrom(size_t _start) const {  return TSymSpace<MatList>(*this, _start);  }
};

// the adaptor for complex classes having MatrixCount()/GetMatrix(size_t) methods
template <class MatrixContainer> class MatrixListAdaptor {
  const MatrixContainer& mc;
public:
  MatrixListAdaptor(const MatrixContainer& _mc) : mc(_mc)  {}
  MatrixListAdaptor(const MatrixListAdaptor& mla) : mc(mla.mc)  {}
  inline size_t Count() const {  return mc.MatrixCount();  }
  inline bool IsEmpty() const {  return Count() == 0;  }
  inline const smatd& operator [](size_t i) const {  return mc.GetMatrix(i);  }
};

namespace SymSpace  {
  struct Info  {
    bool centrosymmetric;
    // holds references of the original object, be careful with const& containers!
    TPtrList<smatd> matrices;
    // always positive
    short latt;
  };
  struct InfoEx  {  // has a list of translation vectors vs latt number
    bool centrosymmetric;
    smatd_list matrices;
    vec3d_list vertices;
  };
  template <class SP> static Info GetInfo(const SP& sp)  {
    Info rv;
    rv.latt = -1;
    rv.centrosymmetric = false;
    if( sp.Count() == 1 )  {
      if( !sp[0].r.IsI() || !sp[0].t.IsNull() )
        throw TInvalidArgumentException(__OlxSourceInfo, "identity matrix");
      rv.latt = 1; //'P';
      rv.matrices.Add(sp[0]);
      return rv;
    }
    olxdict<int, TPtrList<smatd>, TPrimitiveComparator> groups;
    for( size_t i=0; i < sp.Count(); i++ )  {
      TPtrList<smatd>& l = groups.Add(rotation_id::get(sp[i].r));
      l.Add(sp[i]);
    }
    const size_t min_a_group=groups.GetValue(0).Count();
    for( size_t i=0; i < groups.Count(); i++ )  {
      if( groups.GetValue(i).Count() != min_a_group )
        throw TInvalidArgumentException(__OlxSourceInfo, "matrix list");
    }
    if( min_a_group ==  1 )
      rv.latt = 1; //'P';
    else if( min_a_group == 2 )  {
      vec3d t = groups.GetValue(0)[1]->t - groups.GetValue(0)[0]->t;
      t -= t.Floor<int>();
      if( olx_abs(t[0]-0.5) < 1e-6 )  {
        if( olx_abs(t[1]-0.5) < 1e-6 )  {
          if( olx_abs(t[2]-0.5) < 1e-6 )
            rv.latt = 2; //'I';
          else if( olx_abs(t[2]) < 1e-6 )
            rv.latt = 7; //'C';
        }
        else if( olx_abs(t[1]) < 1e-6 && olx_abs(t[2]-0.5) < 1e-6 )
          rv.latt = 6; //'B';
      }
      else if( olx_abs(t[0]) < 1e-6 && olx_abs(t[1]-0.5) < 1e-6 && olx_abs(t[2]-0.5) < 1e-6 )
        rv.latt = 5; //'A';
    }
    else if( min_a_group == 3 )  {
      vec3d t1 = groups.GetValue(0)[1]->t - groups.GetValue(0)[0]->t;
      vec3d t2 = groups.GetValue(0)[2]->t - groups.GetValue(0)[0]->t;
      t1 -= t1.Floor<int>();
      t2 -= t2.Floor<int>();
      if( olx_abs(t1[0]-2./3) < 1e-6 && olx_abs(t1[1]-1./3) < 1e-6 && olx_abs(t1[2]-1./3) < 1e-6  &&
        olx_abs(t2[0]-1./3) < 1e-6 && olx_abs(t2[1]-2./3) < 1e-6 && olx_abs(t2[2]-2./3) < 1e-6 )
        rv.latt = 3; //'R';
      else if( olx_abs(t1[0]-1./3) < 1e-6 && olx_abs(t1[1]-1./3) < 1e-6 && olx_abs(t1[2]-2./3) < 1e-6  &&
        olx_abs(t2[0]-2./3) < 1e-6 && olx_abs(t2[1]-2./3) < 1e-6 && olx_abs(t2[2]-1./3) < 1e-6 )
        rv.latt = 8; //'S';
      else if( olx_abs(t1[0]-1./3) < 1e-6 && olx_abs(t1[1]-2./3) < 1e-6 && olx_abs(t1[2]-1./3) < 1e-6  &&
        olx_abs(t2[0]-2./3) < 1e-6 && olx_abs(t2[1]-1./3) < 1e-6 && olx_abs(t2[2]-2./3) < 1e-6 )
        rv.latt = 9; //'T';
    }
    else if( min_a_group )
      rv.latt = 4; //'F';
    if( rv.latt < 0 )
      throw TFunctionFailedException(__OlxSourceInfo, "could not deduce lattice centering");
    for( size_t i=0; i < groups.Count(); i++ )  {
      groups.GetValue(i).SetCount(1);
      groups.GetValue(i)[0]->t -= groups.GetValue(i)[0]->t.template Floor<int>();
    }
    // find out if centrosymmetric...
    if( groups.Count() > 1 )  {
      const size_t gc = groups.Count();
      for( size_t i=0; i < groups.Count(); i++ )  {
        const size_t ii = groups.IndexOf(rotation_id::negate(groups.GetKey(i)));
        if( ii == InvalidIndex )
          continue;
        groups.GetValue(ii).Add(groups.GetValue(i)[0]);
        groups.Delete(i--);
      }
      rv.centrosymmetric = (groups.Count()*2 == gc);
      if( rv.centrosymmetric )  {
        for( size_t i=0; i < groups.Count(); i++ )  {
          if( groups.GetValue(i)[0]->r.Determinant() < 0 )
            groups.GetValue(i).Delete(0);
          else
            groups.GetValue(i).SetCount(1);
        }
      }
    }
    for( size_t i=0; i < groups.Count(); i++ )  {
      TPtrList<smatd>& l = groups.GetValue(i);
      for( size_t j=0; j < l.Count(); j++ )
        rv.matrices.Add(l[j]);
    }
    return rv;
  }
  // returns a compacted form of symspace
  template <class SP> static InfoEx Compact(const SP& sp)  {
    InfoEx rv;
    rv.centrosymmetric = false;
    int latt = -1;
    if( sp.Count() == 1 )  {  // P?
      if( !sp[0].r.IsI() || !sp[0].t.IsNull() )
        throw TInvalidArgumentException(__OlxSourceInfo, "identity matrix");
      return rv;
    }
    olxdict<int, TPtrList<const smatd>, TPrimitiveComparator> groups;
    for( size_t i=0; i < sp.Count(); i++ )  {
      TPtrList<const smatd>& l = groups.Add(rotation_id::get(sp[i].r));
      l.Add(sp[i]);
    }
    const size_t min_a_group=groups.GetValue(0).Count();
    for( size_t i=0; i < groups.Count(); i++ )  {
      if( groups.GetValue(i).Count() != min_a_group )
        throw TInvalidArgumentException(__OlxSourceInfo, "matrix list");
    }
    if( min_a_group == 1 )
      latt = 1; //'P';
    else if( min_a_group == 2 )  {
      vec3d t = groups.GetValue(0)[1]->t - groups.GetValue(0)[0]->t;
      t -= t.Floor<int>();
      if( olx_abs(t[0]-0.5) < 1e-6 )  {
        if( olx_abs(t[1]-0.5) < 1e-6 )  {
          if( olx_abs(t[2]-0.5) < 1e-6 )
            latt = 2; //'I';
          else if( olx_abs(t[2]) < 1e-6 )
            latt = 7; //'C';
        }
        else if( olx_abs(t[1]) < 1e-6 && olx_abs(t[2]-0.5) < 1e-6 )
          latt = 6; //'B';
      }
      else if( olx_abs(t[0]) < 1e-6 && olx_abs(t[1]-0.5) < 1e-6 && olx_abs(t[2]-0.5) < 1e-6 )
        latt = 5; //'A';
      rv.vertices.AddCCopy(t);
    }
    else if( min_a_group == 3 )  {
      vec3d t1 = groups.GetValue(0)[1]->t - groups.GetValue(0)[0]->t;
      vec3d t2 = groups.GetValue(0)[2]->t - groups.GetValue(0)[0]->t;
      t1 -= t1.Floor<int>();
      t2 -= t2.Floor<int>();
      if( olx_abs(t1[0]-2./3) < 1e-6 && olx_abs(t1[1]-1./3) < 1e-6 && olx_abs(t1[2]-1./3) < 1e-6  &&
        olx_abs(t2[0]-1./3) < 1e-6 && olx_abs(t2[1]-2./3) < 1e-6 && olx_abs(t2[2]-2./3) < 1e-6 )
        latt = 3; //'R';
      else if( olx_abs(t1[0]-1./3) < 1e-6 && olx_abs(t1[1]-1./3) < 1e-6 && olx_abs(t1[2]-2./3) < 1e-6  &&
        olx_abs(t2[0]-2./3) < 1e-6 && olx_abs(t2[1]-2./3) < 1e-6 && olx_abs(t2[2]-1./3) < 1e-6 )
        latt = 8; //'S';
      else if( olx_abs(t1[0]-1./3) < 1e-6 && olx_abs(t1[1]-2./3) < 1e-6 && olx_abs(t1[2]-1./3) < 1e-6  &&
        olx_abs(t2[0]-2./3) < 1e-6 && olx_abs(t2[1]-1./3) < 1e-6 && olx_abs(t2[2]-2./3) < 1e-6 )
        latt = 9; //'T';
      rv.vertices.AddCCopy(t1);
      rv.vertices.AddCCopy(t2);
    }
    else if( min_a_group )  {
      rv.vertices.AddNew(0, 0.5, 0.5);
      rv.vertices.AddNew(0.5, 0, 0.5);
      rv.vertices.AddNew(0.5, 0.5, 0);
      latt = 4; //'F';
    }
    if( latt < 0 )
      throw TFunctionFailedException(__OlxSourceInfo, "could not deduce lattice centering");
    for( size_t i=0; i < groups.Count(); i++ )
      groups.GetValue(i).SetCount(1);
    // find out if centrosymmetric...
    if( groups.Count() > 1 )  {
      const size_t gc = groups.Count();
      for( size_t i=0; i < groups.Count(); i++ )  {
        const size_t ii = groups.IndexOf(rotation_id::negate(groups.GetKey(i)));
        if( ii == InvalidIndex )
          continue;
        groups.GetValue(ii).Add(groups.GetValue(i)[0]);
        groups.Delete(i--);
      }
      rv.centrosymmetric = (groups.Count()*2 == gc);
      if( rv.centrosymmetric )  {
        for( size_t i=0; i < groups.Count(); i++ )  {
          if( groups.GetValue(i)[0]->r.Determinant() < 0 )
            groups.GetValue(i).Delete(0);
          else
            groups.GetValue(i).SetCount(1);
        }
      }
    }
    for( size_t i=0; i < groups.Count(); i++ )  {
      TPtrList<const smatd>& l = groups.GetValue(i);
      for( size_t j=0; j < l.Count(); j++ )  {
        if( !l[j]->r.IsI() )
          rv.matrices.AddCCopy(*l[j]).t -= l[j]->t.Floor<int>();
      }
    }
    return rv;
  }
};  // end namespace SymSpace

EndXlibNamespace()
#endif
