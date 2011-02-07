#ifndef __olx_sdl_math_lu_H
#define __olx_sdl_math_lu_H
#include "../ematrix.h"
#include "../bapp.h"
#include "../testsuit.h"
/* most of the procedures are inspired by ALGLIB and LAPACK, some things, the Householder
reflection generation in particular - inspired by smtxb code by Luc Bourhis */

BeginEsdlNamespace()

namespace math  {
  template <typename CT, typename FT> struct mat_col  {
    CT& m;
    const size_t col, start, end, sz;
    mat_col(CT& _m, size_t _col, size_t _start, size_t _end=InvalidIndex) :
      m(_m), col(_col), start(_start),
      end(_end==InvalidIndex ? m.RowCount() : _end), sz(end-start)  {}
    FT& operator () (size_t i)  {  return m(start+i, col);  }
    const FT& operator () (size_t i) const {
      if( i >= sz )  throw 1;
       return m(start+i, col);
    }
    template <typename Functor> mat_col& ForEach(const Functor& f)  {
      for( size_t i=start; i < end; i++ )
        f(m(i,col));
      return *this;
    }
    template <typename Functor> mat_col& ForEach(const Functor& f) const {
      for( size_t i=start; i < end; i++ )
        f(m(i,col));
    }
    size_t Count() const {  return sz;  }
    bool IsEmpty() const {  return sz == 0;  }
  };

  template <typename CT, typename FT> struct mat_row  {
    CT& m;
    const size_t row, start, end, sz;
    mat_row(CT& _m, size_t _row, size_t _start, size_t _end=InvalidIndex) :
      m(_m), row(_row), start(_start),
      end(_end==InvalidIndex ? m.ColCount() : _end), sz(end-start)  {}
    FT& operator () (size_t i)  {  return m(row, start+i);  }
    const FT& operator () (size_t i) const {  return m(row, start+i);  }
    template <typename Functor> mat_row& ForEach(const Functor& f)  {
      for( size_t i=start; i < end; i++ )
        f(m(row, i));
      return *this;
    }
    template <typename Functor> mat_row& ForEach(const Functor& f) const {
      for( size_t i=start; i < end; i++ )
        f(m(row, i));
      return *this;
    }
    size_t Count() const {  return sz;  }
    bool IsEmpty() const {  return sz == 0;  }
  };

  template <typename CT, typename FT> struct vec_row  {
    CT& v;
    const size_t start, end, sz;
    vec_row(CT& _v, size_t _start, size_t _end=InvalidIndex) :
      v(_v), start(_start),
      end(_end==InvalidIndex ? v.Count() : _end), sz(end-start)  {}
    FT& operator () (size_t i)  {  return v(start+i);  }
    const FT& operator () (size_t i) const {  return v(start+i);  }
    template <typename Functor> vec_row& ForEach(const Functor& f)  {
      for( size_t i=start; i < end; i++ )
        f(v(i));
      return *this;
    }
    template <typename Functor> vec_row& ForEach(const Functor& f) const {
      for( size_t i=start; i < end; i++ )
        f(v(i));
      return *this;
    }
    size_t Count() const {  return sz;  }
    bool IsEmpty() const {  return sz == 0;  }
  };

  template <typename CT, typename FT> struct mat_mat  {
    const size_t col_s, col_e, row_s, row_e, row_c, col_c;
    CT& m;
    mat_mat(CT& _m, size_t rows, size_t rowe, size_t cols, size_t cole) :
      m(_m),
      col_s(cols), col_e(cole), row_s(rows), row_e(rowe),
      row_c(rowe-rows), col_c(cole-cols) {}
    mat_mat(CT& _m, size_t colc, size_t rowc) :
      m(_m),
      col_s(0), col_e(colc), row_s(0), row_e(rowc),
      row_c(rowc), col_c(colc) {}
    FT& operator ()(size_t i, size_t j)  {  return m(i,j);  }
    const FT& operator ()(size_t i, size_t j) const {  return m(i,j);  }
    size_t ColCount() const {  return col_c;  }
    size_t RowCount() const {  return row_c;  }
    bool IsEmpty() const {  return ColCount() == 0 || RowCount() == 0;  }
  };
  template <typename FT> struct proxy  {
    template <typename CT> static mat_mat<CT,FT>
    mat(CT& _m, size_t rows, size_t rowe, size_t cols, size_t cole)  {
      return mat_mat<CT,FT>(_m, rows, rowe, cols, cole); 
    }
    template <typename CT> static mat_mat<CT,FT>
    mat_to(CT& _m, size_t rowe, size_t cole)  {
      return mat_mat<CT,FT>(_m, 0, rowe, 0, cole); 
    }
    template <typename CT> static mat_col<CT,FT>
    col(CT& _m, size_t _col, size_t _start, size_t _end=InvalidIndex)  {
      return mat_col<CT,FT>(_m, _col, _start, _end);
    }
    template <typename CT> static mat_row<CT,FT>
    row(CT& _m, size_t _row, size_t _start, size_t _end=InvalidIndex)  {
      return mat_row<CT,FT>(_m, _row, _start, _end);
    }
    template <typename CT> static vec_row<CT,FT>
    vec(CT& _v, size_t _start, size_t _end=InvalidIndex)  {
      return vec_row<CT,FT>(_v, _start, _end);
    }
  };

  namespace alg  {
    template<typename ToT, typename FromT> void copy(ToT& to, const FromT& f)  {
      for( size_t i=0; i < f.Count(); i++ )
        to(i) = f(i);
    }
    template<typename ToT, typename FromT> void copy_1(ToT& to, const FromT& f)  {
      for( size_t i=1; i < f.Count(); i++ )
        to(i) = f(i);
      to(0) = 1;
    }
    // this way the rounding error is smaller than in a simple loop
    template<typename T1, typename T2, typename FT> FT& dot_prod_x(const T1& a, const T2& b, FT& v)  {
      v = 0;
      size_t ce=((a.Count())/4)*4, i=0;
      for( ; i < ce; i+=4 )
        v += a(i)*b(i) + a(i+1)*b(i+1) + a(i+2)*b(i+2) + a(i+3)*b(i+3);
      for( ; i < a.Count(); i++ )
        v += a(i)*b(i);
      return v;
    }
    template <typename T> void swap_rows(T& m, size_t i, size_t j)  {
      const size_t sz = m.ColCount();
      for( size_t x=0; x < sz; x++ )
        olx_swap(m(i,x), m(j,x));
    }
    template <typename T> void swap_cols(T& m, size_t i, size_t j)  {
      const size_t sz = m.RowCount();
      for( size_t x=0; x < sz; x++ )
        olx_swap(m(x,i), m(x,j));
    }
    template <typename T> void print0(const T& a, size_t m, const olxstr& annotation=EmptyString())  {
      if( !annotation.IsEmpty() )
        TBasicApp::NewLogEntry() << annotation;
      olxstr line;
      for( size_t i=0; i < m; i++ )
        line << olxstr::FormatFloat(-16, a(i), true) << ' ';
      TBasicApp::NewLogEntry() << line;
    }
    template <typename T> void print0_1(const T& a, const olxstr& annotation=EmptyString())  {
      print0(a, a.Count(), annotation);
    }
    template <typename T> void print1(const T& a, int m, const olxstr& annotation=EmptyString())  {
      if( !annotation.IsEmpty() )
        TBasicApp::NewLogEntry() << annotation;
      olxstr line;
      for( int i=0; i < m; i++ )
        line << olxstr::FormatFloat(-16, a(i+1), true) << ' ';
      TBasicApp::NewLogEntry() << line;
    }
    template <typename T> void print0(const T& a, size_t m, size_t n,
      const olxstr& annotation=EmptyString())
    {
      if( !annotation.IsEmpty() )
        TBasicApp::NewLogEntry() << annotation;
      for( size_t i=0; i < m; i++ )  {
        olxstr line;
        for( size_t j=0; j < n; j++ )
          line << olxstr::FormatFloat(-16, a(i, j), true) << ' ';
        TBasicApp::NewLogEntry() << line;
      }
    }
    template <typename T> void print0_2(const T& a, const olxstr& annotation=EmptyString())  {
      print0(a, a.RowCount(), a.ColCount(), annotation);
    }
    template <typename T> void print1(const T& a, int m, int n, const olxstr& annotation=EmptyString())  {
      if( !annotation.IsEmpty() )
        TBasicApp::NewLogEntry() << annotation;
      for( int i=1; i <= m; i++ )  {
        olxstr line;
        for( int j=1; j <= n; j++ )
          line << olxstr::FormatFloat(-16, a(i,j), true) << ' ';
        TBasicApp::NewLogEntry() << line;
      }
    }
    template <typename FT> struct Mul  {
      const FT k;
      Mul(FT _k) : k(_k)  {}
      void operator ()(FT& f) const {  f *= k;  }
    };
    template <typename FT> struct Div  {
      const FT k;
      Div(FT _k) : k(_k)  {}
      void operator ()(FT& f) const {  f /= k;  }
    };
    template <typename FT> struct Add  {
      const FT k;
      Add(FT _k) : k(_k)  {}
      void operator ()(FT& f) const {  f += k;  }
    };
    template <typename FT> struct Sub  {
      const FT k;
      Sub(FT _k) : k(_k)  {}
      void operator ()(FT& f) const {  f -= k;  }
    };
    struct ChSig  {
      template <typename FT> void operator ()(FT& f) const {  f = -f;  }
    };
  };  // end namespace alg
  template <typename FT>
  struct LU  {
    template <typename MatT, typename IndexVecT>
    static void Decompose(MatT& m, IndexVecT& pivots)  {
      const size_t
        min_d = olx_min(m.RowCount(), m.ColCount()),
        max_d = olx_max(m.RowCount(), m.ColCount());
      for( size_t i=0; i < min_d; i++ )  {
        size_t pivot_index = i;
        for( size_t j=i+1; j < m.RowCount(); j++ )  {
          if( olx_abs(m(j,i)) > olx_abs(m(pivot_index,i)) )
            pivot_index = j;
        }
        pivots(i) = pivot_index;
        if( m(pivot_index,i) != 0 )  {
          if( pivot_index != i )
            alg::swap_rows(m, pivot_index, i);
          if( i < min_d-1 )
            proxy<FT>::col(m, i, i+1).ForEach(alg::Mul<FT>(1./m(i,i)));
        }
        if( i < m.RowCount()-1 )  {
          for( size_t j=i+1; j < m.RowCount(); j++ )  {
            for( size_t k=i+1; k < m.RowCount(); k++ )
              m(j,k) -= m(j,i)*m(i,k);
          }
        }
      }
    }
    template <typename MatT, typename IndexVecT>
    static bool Invert(MatT& m, const IndexVecT& pivots)  {
      if( !InvertTriagular<MatT, FT>(m, true, false) )
        return false;
      TVector<FT> tmp(m.RowCount());
      for( size_t i=m.RowCount()-1; i != InvalidIndex; i-- )  {
        for( size_t j=i+1; j < m.RowCount(); j++ )  {
          tmp(j) = m(j,i);
          m(j,i) = 0.0;
        }
        if( i < m.RowCount()-1 )  {
          for( size_t j=0; j < m.RowCount(); j++ )  {
            FT v = 0;
            for( size_t k=i+1; k < m.RowCount(); k++ )
              v += m(j,k)*tmp(k);
            m(j,i) -= v;
          }
        }
      }
      for( size_t i=m.RowCount()-1; i != InvalidIndex; i-- )  {
        if( pivots(i) != i )
          alg::swap_cols(m, i, pivots(i));
      }
      return true;
    }
    template <typename MatT>
    static bool Invert(MatT& m)  {
      TVector<size_t> pivots(m.RowCount());
      Decompose(m, pivots);
      return Invert(m, pivots);
    }
  }; // end of LU
  template <typename FT> struct LQ  {
    template <typename MatT,typename VecT>
    static void Decompose(MatT& m, VecT& taus)  {
      const size_t
        min_d = olx_min(m.RowCount(), m.ColCount()),
        max_d = olx_max(m.RowCount(), m.ColCount());
      taus.Resize(min_d);
      TVector<FT> tmp(m.ColCount());
      Reflection<FT> ref(m.RowCount());
      for( size_t i=0; i < min_d; i++ )  {
        mat_row<MatT,FT> mr(m, i, i);
        taus(i) = ref.Generate(mr);
        if( i < m.ColCount()-1 )  {
          alg::copy_1(tmp, mr);
          ref.ApplyFromRight(m, taus(i), tmp, i+1, m.RowCount(), i, m.ColCount());
        }
      }
    }
    template <typename MatIT, typename MatOT, typename VecT>
    static void UnpackQ(const MatIT& m, const VecT& taus, size_t row_cnt, MatOT& q)  {
      if( m.IsEmpty() || row_cnt == 0 )
        return;
      const size_t min_d = olx_min(m.RowCount(), m.ColCount());
      q.Resize(row_cnt, m.ColCount());
      TVector<FT> tmp(m.ColCount());
      Reflection<FT> ref(row_cnt);
      for( size_t i=0; i < row_cnt; i++ )
        for( size_t j=0; j < m.ColCount(); j++ )
          q(i,j) = static_cast<FT>(i==j ? 1 : 0);
      for( size_t i=min_d-1; i != InvalidIndex; i-- )  {
        const size_t sz = m.ColCount()-i;
        for( size_t j=1; j < sz; j++ )
          tmp(j) = m(i, i+j);
        tmp(0) = 1;
        ref.ApplyFromRight(q, taus(i), tmp, 0, row_cnt, i, m.ColCount());
      }
    }
  }; // end of LQ
  // returns false for singular matrix
  template <typename MatT, typename FT>
  static bool InvertTriagular(MatT& m, bool upper, bool hasUnitDiag)  {
    TVector<FT> tmp(m.RowCount());
    if( upper )  {
      for( size_t i=0; i < m.RowCount(); i++ )  {
        FT diag_v = -1;
        if( !hasUnitDiag )  {
          if( m(i,i) == 0 )
            return false;
          diag_v = -(m(i,i) = 1./m(i,i));
        }
        if( i > 0 )  {
          alg::copy(tmp, proxy<FT>::col(m, i, 0, i));
          for( size_t j=0; j < i; j++ )  {
            FT v = 0.0;
            if( j < i-1 )  {
              for( size_t k=j+1; k < i; k++ )
                v += m(j,k)*tmp(k);
            }
            m(j,i) = v + m(j,j)*tmp(j);
          }
          proxy<FT>::col(m, i, 0, i).ForEach(alg::Mul<FT>(diag_v));
        }
      }
    }
    else  {
      for( size_t i=m.RowCount()-1; i != InvalidIndex ; i-- )  {
        FT diag_v = -1;
        if( !hasUnitDiag )  {
          if( m(i,i) == 0 )
            return false;
          diag_v = -(m(i,i) = 1./m(i,i));
        }
        if( i < m.RowCount()-1 )  {
          for( size_t j=i+1; j < m.RowCount(); j++ )
            tmp(j) = m(j,i);
          for( size_t j=i+1; j < m.RowCount(); j++ )  {
            FT v = 0.0;
            if( j > i+1 )  {
              for( size_t k=i+1; k < j; k++ )
                v += m(j,k)*tmp(k);
            }
            m(j,i) = v + m(j,j)*tmp(j);
          }
          proxy<FT>::col(m, i, i+1, m.RowCount()).ForEach(alg::Mul<FT>(diag_v));
        }
      }
    }
    return true;
  }

  template<typename FT> struct Reflection  {
    TVector<FT> tmp;
    Reflection(size_t sz) : tmp(sz) {}
    template <typename vec_t> FT Generate(vec_t& v)  {
      if( v.Count() <= 1 )
        return 0;
      FT mx = olx_abs(v(1));
      for( size_t i=2; i < v.Count(); i++ )
        mx = olx_max(olx_abs(v(i)), mx);
      FT xnorm = 0;
      if( mx != 0 )  {
        for( size_t i=1; i < v.Count(); i++ )
          xnorm += olx_sqr(v(i)/mx);
        xnorm = sqrt(xnorm)*mx;
      }
      else
        return 0;
      const FT alpha = v(0);
      mx = olx_max(olx_abs(alpha), olx_abs(xnorm));
      const FT beta = mx*sqrt(olx_sqr(alpha/mx)+olx_sqr(xnorm/mx));
      const FT v0 = (alpha <= 0 ? alpha - beta : -xnorm/(alpha + beta)*xnorm);
      proxy<FT>::vec(v,1, v.Count()).ForEach(alg::Mul<FT>(1/v0));
      v(0) = beta;
      return 2/(olx_sqr(xnorm/v0)+1);
    }
    template <typename MatT, typename VecT>
    void ApplyFromLeft(MatT& m, FT tau, const VecT& vt,
      size_t row_s, size_t row_e, size_t col_s, size_t col_e)
    {
      if( tau == 0 || row_s >= row_e || col_s >= col_e )
        return;
      for( size_t j=col_s; j < col_e; j++ )
        tmp(j) = 0;
      for( size_t i=row_s; i < row_e; i++ )  {
        const FT v = vt(i-row_s);
        for( size_t j=col_s; j < col_e; j++ )
          tmp(j) += m(i,j)*v;
      }
      for( size_t i=row_s; i < row_e; i++ )  {
        const FT v = vt(i-row_s)*tau;
        for( size_t j=col_s; j < col_e; j++ )
          m(i,j) -= tmp(j)*v;
      }
    }
    template <typename MatT>
    void ApplyFromLeft(MatT& m, FT tau, size_t row_s, size_t col_s)  {
      const FT beta = m(row_s,col_s-1);
      m(row_s,col_s-1) = 1;
      ApplyFromLeft(m, tau, proxy<FT>::col(m, col_s-1, row_s, m.RowCount()),
        row_s, m.RowCount(), col_s, m.ColCount());
      m(row_s,col_s-1) = beta;
    }
    template <typename MatT, typename VecT>
    void ApplyFromRight(MatT& m, FT tau, const VecT& vt,
      size_t row_s, size_t row_e, size_t col_s, size_t col_e)
    {
      if( tau == 0 || row_s >= row_e || col_s >= col_e )
        return;
      for( size_t i=row_s; i < row_e; i++ )  {
        FT v = 0;
        const FT v1 = tau*alg::dot_prod_x(proxy<FT>::row(m, i, col_s, col_e), vt, v);
        for( size_t j=col_s; j < col_e; j++ )
          m(i,j) -= vt(j-col_s)*v1;
      }
    }
    template <typename MatT>
    void ApplyFromRight(MatT& m, FT tau, size_t row_s, size_t col_s)  {
      const FT beta = m(row_s-1,col_s);
      m(row_s-1,col_s) = 1;
      ApplyFromRight(m, tau, proxy<FT>::row(m, row_s-1, col_s, m.ColCount()),
        row_s, m.RowCount(), col_s, m.ColCount());
      m(row_s-1,col_s) = beta;
    }
  };  // end of Reflection
  
  template <class FT>
  struct QR  {
    template <typename MatT, typename VecT>
    static void Decompose(MatT& m, VecT& taus)  {
      const size_t min_d = olx_min(m.ColCount(), m.RowCount());
      Reflection<FT> r(min_d);
      taus.Resize(min_d);
      TVector<FT> tmp(m.RowCount());
      for( size_t i=0; i < min_d; i++ )  {
        mat_col<MatT, FT> mc(m, i, i);
        taus(i) = r.Generate(mc);
        if( i < m.ColCount()-1 )  {
          alg::copy_1(tmp, mc);
          r.ApplyFromLeft(m, taus(i), tmp, i, m.RowCount(), i+1, m.ColCount());
        }
      }
    }

    template <typename MatIT, typename MatOT, typename VecT>
    static void Unpack(const MatIT& qr, const VecT& tau, size_t col_num, MatOT& q)  {
      const size_t min_d = olx_min(qr.ColCount(), qr.RowCount());
      Reflection<FT> ref(col_num);
      q.Resize(qr.RowCount(), col_num);
      for( size_t i=0; i < qr.RowCount(); i++ )  {
        for( size_t j=0; j < col_num; j++ )
          q(i,j) = static_cast<FT>(i==j ? 1 : 0);
      }
      const size_t sz = olx_min(min_d, col_num)-1;
      TVector<FT> v(qr.RowCount());
      for( size_t i=sz; i != InvalidIndex; i-- )  {
        alg::copy_1(v, proxy<FT>::col(qr, i, i, qr.RowCount())); 
        ref.ApplyFromLeft(q, tau(i), v, i, qr.RowCount(), 0, col_num);
      }
    }
  }; // end QR

  template <typename FT> struct Bidiagonal  {
    template <typename MatT, typename VecT>
    static void ToBidiagonal(MatT& m, VecT& qtau, VecT& ptau)  {
      const size_t
        min_d = olx_min(m.ColCount(), m.RowCount()),
        max_d = olx_max(m.ColCount(), m.RowCount());
      Reflection<FT> ref(max_d);
      qtau.Resize(min_d);
      ptau.Resize(min_d);
      if( m.RowCount() >= m.ColCount() )  {
        const size_t k_left = m.ColCount() - (m.RowCount() > m.ColCount() ? 0 : 1);
        const size_t k_right = m.ColCount()-2;
        for( size_t i=0; i < k_left; i++ )  {
          mat_col<MatT, FT> mc(m, i, i);
          qtau(i) = ref.Generate(mc);
          ref.ApplyFromLeft(m, qtau(i), i, i+1);
          if( i < k_right )  {
            mat_row<MatT, FT> mr(m, i, i+1);
            ptau(i) = ref.Generate(mr);
            ref.ApplyFromRight(m, ptau(i), i+1, i+1);
          }
          else
            ptau(i) = 0;
        }
      }
      else  {
        const size_t k_right = m.RowCount();
        const size_t k_left = m.RowCount()-2;
        for( size_t i=0; i < k_right; i++ )  {
          mat_row<MatT, FT> mr(m, i, i);
          ptau(i) = ref.Generate(mr);
          ref.ApplyFromRight(m, ptau(i), i+1, i);
          if( i < k_left )  {
            mat_col<MatT, FT> mc(m, i, i+1);
            qtau(i) = ref.Generate(mc);
            ref.ApplyFromLeft(m, qtau(i), i+1, i+1);
          }
          else
            qtau(i) = 0;
        }
      }
    }
    template <typename MatIT, typename MatOT, typename VecT>
    static void UnpackQ(const MatIT& qp, const VecT& qtau, size_t col_cnt, MatOT& q)  {
      q.Resize(qp.RowCount(), col_cnt);
      Reflection<FT> r(col_cnt);
      TVector<FT> tmp(qp.RowCount());
      for( size_t i=0; i < qp.RowCount(); i++ )  {
        for( size_t j=0; j < col_cnt; j++ )
          q(i,j) = static_cast<FT>(i==j ? 1 : 0);
      }
      if( qp.RowCount() >= qp.ColCount() )  {
        const size_t s = olx_min(qp.ColCount(), col_cnt)-1;
        for( size_t i=s; i != InvalidIndex; i-- )  {
          alg::copy_1(tmp, proxy<FT>::col(qp, i, i, qp.RowCount()));
          r.ApplyFromLeft(q, qtau(i), tmp, i, qp.RowCount(), 0, col_cnt);
        }
      }
      else  {
        const size_t s = olx_min(qp.RowCount(), col_cnt)-1;
        for( size_t i=s; i != InvalidIndex; i-- )  {
          alg::copy_1(tmp, proxy<FT>::col(qp, i, i+1, qp.RowCount()));
          r.ApplyFromLeft(q, qtau(i), tmp, i+1, qp.RowCount(), 0, col_cnt);
        }
      }
    }
    template <typename MatIT, typename MatOT, typename VecT>
    static void UnpackPT(const MatIT& qp, const VecT& ptau, size_t row_cnt, MatOT& pt)  {
      if( qp.IsEmpty() || row_cnt == 0 )
        return;
      pt.Resize(row_cnt, qp.ColCount());
      Reflection<FT> ref(qp.ColCount());
      TVector<FT> tmp(qp.ColCount());
      for( size_t i=0; i < row_cnt; i++ )  {
        for( size_t j=0; j < qp.ColCount(); j++ )
          pt(i,j) = static_cast<FT>(i==j ? 1 : 0);
      }
      if( qp.RowCount() >= qp.ColCount() )  {
        const size_t s = olx_min(qp.ColCount(), row_cnt)-2;
        for( size_t i=s; i != InvalidIndex; i-- )  {
          alg::copy_1(tmp, proxy<FT>::row(qp, i, i+1, qp.ColCount()));
          ref.ApplyFromRight(pt, ptau(i), tmp, 0, row_cnt, i+1, qp.ColCount());
        }
      }
      else  {
        const size_t s = olx_min(qp.RowCount(), row_cnt)-1;
        for( size_t i=s; i != InvalidIndex; i-- )  {
          alg::copy_1(tmp, proxy<FT>::row(qp, i, i, qp.ColCount()));
          ref.ApplyFromRight(pt, ptau(i), tmp, 0, row_cnt, i, qp.ColCount());
        }
      }
    }
    // returns true if the matrix is upper diagonl
    template <typename MatT, typename VecT>
    static bool UnpackDiagonals(const MatT& m, VecT& d, VecT& e)  {
      const bool upper = m.RowCount() >= m.ColCount();
      if( m.IsEmpty() )
        return upper;
      if( upper )  {
        d.Resize(m.ColCount());
        e.Resize(m.ColCount());
        for( size_t i=0; i < m.ColCount()-1; i++ )  {
          d(i) = m(i,i);
          e(i) = m(i,i+1);
        }
        d.GetLast() = m(m.ColCount()-1,m.ColCount()-1);
      }
      else  {
        d.Resize(m.RowCount());
        e.Resize(m.RowCount());
        for( size_t i=0; i < m.RowCount()-1; i++ )  {
          d(i) = m(i,i);
          e(i) = m(i+1,i);
        }
        d.GetLast() = m(m.RowCount()-1,m.RowCount()-1);
      }
      return upper;
    }
    template <typename MatIT, typename MatOT, typename VecT>
    static void MulByQ(const MatIT& qp, const VecT& tauq, MatOT& z, bool from_right, bool dot_trans)
    {
      const size_t mx = olx_max(
        olx_max(qp.ColCount(), qp.RowCount()),
        max(z.ColCount(), z.RowCount()));
      Reflection<FT> ref(mx);
      if( qp.RowCount() >= qp.ColCount() )  {
        size_t i1 = 0, i2 = qp.ColCount();
        int step_inc = 1;
        if( !from_right )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        if( dot_trans )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        TVector<FT> tmp(mx);
        for( size_t i=i1; i < i2; i+= step_inc )  {
          alg::copy_1(tmp, proxy<FT>::col(qp, i, i, qp.RowCount()));
          if( from_right )
            ref.ApplyFromRight(z, tauq(i), tmp, 0, z.RowCount(), i, qp.RowCount());
          else
            ref.ApplyFromLeft(z, tauq(i), tmp, i, qp.RowCount(), 0, z.ColCount());
        }
      }
      else  {
        if( qp.RowCount() == 1 )  return;
        size_t i1 = 1, i2 = qp.RowCount()-1;
        int step_inc = 1;
        if( !from_right )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        if( dot_trans )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        TVector<FT> tmp(mx);
        for( size_t i=i1; i < i2; i += step_inc )  {
          alg::copy_1(tmp, proxy<FT>::col(qp, i, i+1, qp.RowCount()));
          if( from_right )
            ref.ApplyFromRight(z, tauq(i), tmp, 0, z.RowCount(), i+1, qp.RowCount());
          else
            ref.ApplyFromLeft(z, tauq(i), tmp, i+1, qp.RowCount(), 0, z.ColCount());
        }
      }
    }
    template <typename MatIT, typename MatOT, typename VecT>
    static void MulByP(const MatIT& qp, const VecT& tauq, MatOT& z, bool from_right, bool dot_trans)
    {
      const size_t mx = olx_max(
        olx_max(qp.ColCount(), qp.RowCount()),
        max(z.ColCount(), z.RowCount()));
      Reflection<FT> ref(mx);
      if( qp.RowCount() >= qp.ColCount() )  {
        size_t i1 = qp.ColCount()-1, i2 = 0;
        int step_inc = -1;
        if( !from_right )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        if( !dot_trans )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        TVector<FT> tmp(mx);
        while( i1 != i2 )  {
          alg::copy_1(tmp, proxy<FT>::row(qp, i1, i1+1, qp.RowCount()));
          if( from_right )
            ref.ApplyFromRight(z, tauq(i1), tmp, 0, z.RowCount(), i1+1, qp.ColCount());
          else
            ref.ApplyFromLeft(z, tauq(i1), tmp, i1+1, qp.ColCount(), 0, z.ColCount());
          i1 += step_inc;
        }
      }
      else  {
        if( qp.RowCount() == 1 )  return;
        size_t i1 = qp.RowCount(), i2 = 0;
        int step_inc = -1;
        if( !from_right )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        if( !dot_trans )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        TVector<FT> tmp(mx);
        while( i1 != i2 )  {
          alg::copy_1(tmp, proxy<FT>::row(qp, i1, i1, qp.ColCount()));
          if( from_right )
            ref.ApplyFromRight(z, tauq(i1), tmp, 0, z.RowCount(), i1, qp.ColCount());
          else
            ref.ApplyFromLeft(z, tauq(i1), tmp, i1, qp.ColCount(), 0, z.ColCount());
          i1 += step_inc;
        }
      }
    }
    struct SVD  {
      static void do2x2(FT f, FT g, FT h,
        FT& ssmin, FT& ssmax, FT& snr, FT& csr, FT& snl, FT& csl)
      {
        FT abs_f = olx_abs(f), abs_h = olx_abs(h), abs_g = olx_abs(g);
        csl = csr = 1;
        snl = snr = 0;
        const bool swap = abs_h > abs_f;
        if( swap )  {
          olx_swap(f, h);
          olx_swap(abs_f, abs_h);
        }
        if( abs_g != 0 )  {
          bool small_g = true;
          if( abs_g > abs_f )  {
            if( abs_f/abs_g < 1e-16 )  {
              small_g = false;
              ssmax = abs_g;
              if( abs_g > 1 )
                ssmin = abs_f*abs_h/abs_g;
              else
                ssmin = abs_h*abs_g/abs_f;
              csl = snr = 1;
              snl = h/g;
              csr = f/g;
            }
          }
          if( small_g )  {
            const FT d = abs_f - abs_h;
            const FT l = d/abs_f;
            const FT m = g/f, mm = m*m;
            FT t = 2-l, tt = t*t;
            const FT s = sqrt(mm+tt);
            const FT r = (l ==0 ? olx_abs(m) : sqrt(l*l+mm));
            const FT a = (s+r)/2;
            ssmin = abs_h/a;
            ssmax = abs_f*a;
            if( mm == 0 )  {
              if( l == 0 )
                t = olx_copy_sign(2, f)*olx_copy_sign(1, g);
              else
                t = g/olx_copy_sign(d, f) + m/t;
            }
            else
              t = (m/(s+t)+m/(r+l))*(1+a);
            const FT tmp = sqrt(t*t+4);
            csr = 2/tmp;
            snr = t/tmp;
            csl = (csr+snr*m)/a;
            snl = (h/f)*snr/a;
          }
        }
        if( swap )  {
          olx_swap(csr, snl);
          olx_swap(snr, csl);
        }
        ssmax = olx_copy_sign(ssmax, f);
        ssmin = olx_copy_sign(ssmin, h);
      }
      static void do2x2(FT f, FT g, FT h, FT& ssmin, FT& ssmax)  {
        const FT abs_f = olx_abs(f), abs_h = olx_abs(h), abs_g = olx_abs(g);
        const FT min_fh = olx_min(abs_f, abs_h),
          max_fh = olx_max(abs_f, abs_h);
        if( min_fh == 0 )  {
          ssmin = 0;
          const FT mg = olx_max(max_fh, abs_g);
          ssmax = max_fh == 0 ? abs_g : mg*sqrt(1+olx_sqr(olx_min(min_fh, abs_g)/mg));
        }
        else  {
          if( abs_g < max_fh )  {
            const FT a = 1+min_fh/max_fh,
              b = (max_fh-min_fh)/max_fh,
              c = olx_sqr(abs_g/max_fh);
            const FT d = 2/(sqrt(a*a+c)+sqrt(b*b+c));
            ssmin = min_fh*d;
            ssmax = max_fh/d;
          }
          else  {
            const FT _a = max_fh/abs_g;
            if( _a == 0 )  {
              ssmin = min_fh*max_fh/abs_g;
              ssmax = abs_g;
            }
            else  {
              const FT a = 1+min_fh/max_fh,
                b = (max_fh-min_fh)/max_fh,
                c = 1/(sqrt(1+olx_sqr(a*_a))+sqrt(1+olx_sqr(b*_a)));
              ssmin = 2*min_fh*c*_a;
              ssmax = abs_g/(2*c);
            }
          }
        }
      }
      template <typename DiagT1, typename DiagT2, typename UT, typename CT, typename VtT>
      static bool Decompose(DiagT1& d, DiagT2& e, bool upper, bool fract_accu,
        UT& u, CT& c, VtT& vt)
      {
        if( d.Count() == 0 )  return true;
        if( d.Count() == 1 )  {
          if( d(0) < 0 )  {
            d(0) = -d(0);
            if( vt.RowCount() > 0 )
              proxy<FT>::row(vt, 0, 0).ForEach(alg::ChSig());
          }
          return true;
        }
        Rotation<FT> rot;
        int max_itr = 6;
        e.Resize(d.Count());
        const FT _eps = 5e-16, unfl = 1e-300;

        int flag = 0;
        if( !upper )  {
          for( size_t i=0; i < d.Count()-1; i++ )  {
            FT sn, cs, r;
            rot.Generate(d(i), e(i), cs, sn, r);
            d(i) = r;
            e(i) = sn*d(i+1);
            d(i+1) *= cs;
            if( u.RowCount() > 0 )  rot.ApplyToCols(u, i, i+1, cs, sn);
            if( c.ColCount() > 0 )  rot.ApplyToRows(c, i, i+1, cs, sn);
          }
        }
        FT tol = olx_max(10, olx_min(100, pow(_eps, static_cast<FT>(-1./8))))*_eps;
        if( !fract_accu )
          tol = -tol;
        FT smax = 0;
        for( size_t i=0; i < d.Count()-1; i++ )
          smax = olx_max(smax, olx_max(olx_abs(e(i)), olx_abs(d(i))));
        smax = olx_max(smax, olx_abs(d.GetLast()));
        FT sminl = 0, thresh;
        if( tol >=0 )  {
          FT sminoa = olx_abs(d(0));
          if( sminoa != 0 )  {
            FT mu = sminoa;
            for( size_t i=1; i < d.Count(); i++ )  {
              mu = olx_abs(d(i))*(mu/(mu+olx_abs(e(i-1))));
              if( (sminoa = olx_min(sminoa, mu)) == 0 )
                break;
            }
          }
          sminoa /= sqrt((FT)d.Count());
          thresh = olx_max(tol*sminoa, max_itr*olx_sqr(d.Count())*unfl);
        }
        else
          thresh = olx_max(olx_abs(tol)*smax, max_itr*olx_sqr(d.Count())*unfl);
        int max_i = max_itr*olx_sqr((int)d.Count()),
          m = (int)d.Count()-1, iter = 0;
        int oldm = -1, oldll = -1;
        for(;;)  {
          if( m <= 0 )
            break;
          if( iter > max_i )
            return false;
          if( tol < 0 && olx_abs(d(m)) <= thresh )
            d(m) = 0;
          FT smin = (smax = olx_abs(d(m)));
          bool split_flag = false;
          int ll = 0;
          for( int i=1; i <= m; i++ )  {
            ll = m-i;
            const FT
              abs_s = olx_abs(d(ll)),
              abs_e = olx_abs(e(ll));
            if( tol < 0 && abs_s <= thresh )
              d(ll) = 0;
            if( abs_e <= thresh )  {
              split_flag = true;
              break;
            }
            smin = olx_min(smin, abs_s);
            smax = olx_max(smax, olx_max(abs_s, abs_e));
          }
          if( !split_flag )
            ll = -1;
          else  {
            e(ll) = 0;
            if( ll == m-1 )  {
              m--;
              continue;
            }
          }
          ll++;
          if( ll == m-1 )  {
            FT sigmn=0, sigmx=0, sinr, cosr, sinl, cosl;
            do2x2(d(m-1), e(m-1), d(m), sigmn, sigmx, sinr, cosr, sinl, cosl);
            d(m-1) = sigmx;
            e(m-1) = 0;
            d(m) = sigmn;
            if( vt.ColCount() > 0 ) //&& cosr != 1 )
              rot.ApplyToRows(vt, m-1, m, cosr, sinr);
            if( true )  {  //cosl != 1 )  {
              if( u.RowCount() > 0 )  rot.ApplyToCols(u, m-1, m, cosl, sinl);
              if( c.RowCount() > 0 )  rot.ApplyToRows(c, m-1, m, cosl, sinl);
            }
            m -= 2;
            continue;
          }
          bool change_dir = false;
          const FT dm_a = olx_abs(d(m)), dl_a = olx_abs(d(ll));
          if( (flag == 1 && (dl_a < dm_a/1000)) || (flag == 2 && (dm_a < dl_a/1000)) )
            change_dir = true;
          if( ll > oldm || m < oldll || change_dir )
            flag = (olx_abs(d(ll)) >= olx_abs(d(m)) ? 1 : 2);
          if( flag == 1 )  {
            if( olx_abs(e(m-1)) <= olx_abs(tol*d(m)) || (tol < 0 && olx_abs(e(m-1)) < thresh) ) {
              e(m-1) = 0;
              continue;
            }
            if( tol >= 0 )  {  //check if converged
              FT mu = olx_abs(d(ll));
              sminl = mu;
              bool iter_flag = false;
              for( int i=ll; i < m-1; i++ )  {
                if( olx_abs(e(i)) <= tol*mu )  {
                  e(i) = 0;
                  iter_flag = true;
                  break;
                }
                mu = olx_abs(d(i+1))*(mu/(mu+olx_abs(e(i))));
                sminl = olx_min(sminl, mu);
              }
              if( iter_flag )
                continue;
            }
          }
          else  {
            if( olx_abs(e(ll)) <= olx_abs(tol*d(ll)) || (tol < 0 && olx_abs(e(ll)) < thresh) ) {
              e(ll) = 0;
              continue;
            }
            if( tol >= 0 )  {
              FT mu = olx_abs(d(m));
              sminl = mu;
              bool iter_flag = false;
              for( int i=m-1; i >= ll; i-- )  {
                if( olx_abs(e(i)) <= tol*mu )  {
                  e(i) = 0;
                  iter_flag = true;
                  break;
                }
                mu = olx_abs(d(i))*(mu/(mu+olx_abs(e(i))));
                sminl = olx_min(sminl, mu);
              }
              if( iter_flag )
                continue;
            }
          }
          oldll = ll;
          oldm = m;
          FT shift = 0, r = 0;
          if( !(tol >=0 && d.Count()*tol*(sminl/smax) <= olx_max(_eps, 0.01*tol)) )  {
            FT sll;
            if( flag == 1 )  {
              sll = olx_abs(d(ll));
              do2x2(d(m-1), e(m-1), d(m), shift, r);
            }
            else  {
              sll = olx_abs(d(m));
              do2x2(d(ll), e(ll), d(ll+1), shift, r);
            }
            if( sll > 0 && olx_sqr(shift/sll) < _eps )
              shift = 0;
          }
          iter = iter+m-ll;
          if( shift == 0 )  {  // 0 shift - simplified QR iterations
            if( flag == 1 )  {  // chase bulge down
              FT cs = 1, sn, r, oldcs = 1, oldsn = 0, tmp;
              for( int i=ll; i < m; i++ )  {
                rot.Generate(d(i)*cs, e(i), cs, sn, r);
                if( i > ll )
                  e(i-1) = oldsn*r;
                rot.Generate(oldcs*r, d(i+1)*sn, oldcs, oldsn, tmp);
                if( vt.ColCount() > 0 ) // && cs != 1 )
                  rot.ApplyToRows(vt, i, i+1, cs, sn);
                if( true )  {  //oldcs != 1 )  {
                  if( u.RowCount() > 0 )  rot.ApplyToCols(u, i, i+1, oldcs, oldsn);
                  if( c.ColCount() > 0 )  rot.ApplyToRows(c, i, i+1, oldcs, oldsn);
                }
                d(i) = tmp;
              }
              const FT h = d(m)*cs;
              d(m) = h*oldcs;
              if( olx_abs(e(m-1) = h*oldsn) <= thresh )
                e(m-1) = 0;
            }
            else  {  // chase bulge up
              FT cs = 1, sn, r, oldcs = 1, oldsn = 0, tmp;
              for( int i=m; i > ll; i-- )  {
                rot.Generate(d(i)*cs, e(i-1), cs, sn, r);
                if( i < m )
                  e(i) = oldsn*r;
                rot.Generate(oldcs*r, d(i-1)*sn, oldcs, oldsn, tmp);
                d(i) = tmp;
                if( vt.ColCount() > 0 ) // && oldcs != 1 )
                  rot.ApplyToRows(vt, i-1, i, oldcs, -oldsn);
                if( true )  {  //cs != 1 )  {
                  if( u.RowCount() > 0 )  rot.ApplyToCols(u, i-1, i, cs, -sn);
                  if( c.ColCount() > 0 )  rot.ApplyToRows(c, i-1, i, cs, -sn);
                }
              }
              const FT h = d(ll)*cs;
              d(ll) = h*oldcs;
              e(ll) = h*oldsn;
              if( olx_abs(e(ll)) < thresh )
                e(ll) = 0;
            }
          }
          else  {  // non zero shift
            if( flag == 1 )  {  // chanse bulge down
              FT f = (olx_abs(d(ll))-shift)*(olx_copy_sign(1, d(ll))+shift/d(ll));
              FT g = e(ll);
              FT cosr, sinr, cosl, sinl, r;
              for( int i=ll; i < m; i++ )  {
                rot.Generate(f, g, cosr, sinr, r);
                if( i > ll )
                  e(i-1) = r;
                f = cosr*d(i)+sinr*e(i);
                e(i) = cosr*e(i)-sinr*d(i);
                g = sinr*d(i+1);
                d(i+1) *= cosr;
                rot.Generate(f, g, cosl, sinl, r);
                d(i) = r;
                f = cosl*e(i)+sinl*d(i+1);
                d(i+1) = cosl*d(i+1)-sinl*e(i);
                if( i < m-1 ) {
                  g = sinl*e(i+1);
                  e(i+1) = cosl*e(i+1);
                }
                if( vt.ColCount() > 0 )  //&& cosr != 1 )
                  rot.ApplyToRows(vt, i, i+1, cosr, sinr);
                if( true ) {  //cosl != 1 )  {
                  if( u.RowCount() > 0 )  rot.ApplyToCols(u, i, i+1, cosl, sinl);
                  if( c.ColCount() > 0 )  rot.ApplyToRows(c, i, i+1, cosl, sinl);
                }
              }
              if( olx_abs(e(m-1) = f) <= thresh )
                e(m-1) = 0;
            }
            else  { //chase bulge up
              FT f = (olx_abs(d(m))-shift)*(olx_copy_sign(1, d(m))+shift/d(m));
              FT g = e(m-1);
              FT cosr, sinr, cosl, sinl, r;
              for( int i=m; i > ll; i-- )  {
                rot.Generate(f, g, cosr, sinr, r);
                if( i < m )
                  e(i) = r;
                f = cosr*d(i)+sinr*e(i-1);
                e(i-1) = cosr*e(i-1)-sinr*d(i);
                g = sinr*d(i-1);
                d(i-1) *= cosr;
                rot.Generate(f, g, cosl, sinl, r);
                d(i) = r;
                f = cosl*e(i-1)+sinl*d(i-1);
                d(i-1) = cosl*d(i-1)-sinl*e(i-1);
                if( i > ll+1 ) {
                  g = sinl*e(i-2);
                  e(i-2) *= cosl;
                }
                if( vt.ColCount() > 0 )  //&& cosr != 1 )
                  rot.ApplyToRows(vt, i-1, i, cosl, -sinl);
                if( true )  {  //cosl != 1 )  {
                  if( u.RowCount() > 0 )  rot.ApplyToCols(u, i-1, i, cosr, -sinr);
                  if( c.ColCount() > 0 )  rot.ApplyToRows(c, i-1, i, cosr, -sinr);
                }
              }
              if( olx_abs(e(ll) = f) < thresh )
                e(ll) = 0;
            }
          }
        }
        for( size_t i=0; i < d.Count(); i++ )  {
          if( d(i) < 0 )  {
            d(i) = -d(i);
            if( vt.ColCount() > 0 )
              proxy<FT>::row(vt, i, 0).ForEach(alg::ChSig());
          }
        }
        // eventually - sorting...
        for( size_t i=0; i < d.Count()-1; i++ )  {
          FT min_s = d(0);
          size_t min_i = 0;
          for( size_t j=1; j < d.Count()-i; j++ )  {
            if( d(j) <= min_s )  {
              min_i = j;
              min_s = d(j);
            }
          }
          const size_t n = d.Count()-1;
          if( min_i != n-i )  {
            d(min_i) = d(n-i);
            d(n-i) = min_s;
            if( vt.ColCount() > 0 )
              alg::swap_rows(vt, min_i, n-i);
            if( u.ColCount() > 0 )
              alg::swap_cols(u, min_i, n-i);
            if( c.ColCount() > 0 )
              alg::swap_rows(c, min_i, n-i);
          }
        }
        return true;
      }
    };
  }; // end of BiDiagonal
  template <typename FT>
  struct Rotation  {
    Rotation() {}
    static void Generate(FT f, FT g, FT& cs, FT& sn, FT& r)  {
      sn = 0;
      cs = 1;
      if( g == 0 )
        r = f;
      else if( f == 0 )  {
        sn = 1;
        cs = 0;
        r = g;
      }
      else  {
        r = sqrt(f*f+g*g);
        if( olx_abs(f) > olx_abs(g) && f < 0 )  {
          cs = -f/r;
          sn = -g/r;
          r = -r;
        }
        else  {
          cs = f/r;
          sn = g/r;
        }
      }
    }
    template <typename MatT>
    static void ApplyToRows(MatT& m, size_t row_a, size_t row_b, FT cs, FT sn)  {
      for( size_t i=0; i < m.ColCount(); i++ )  {
        const FT tmp = m(row_a,i)*cs + m(row_b,i)*sn;
        m(row_b,i) = m(row_b,i)*cs - m(row_a,i)*sn;
        m(row_a,i) = tmp;
      }
    }
    template <typename MatT>
    void ApplyToCols(MatT& m, size_t col_a, size_t col_b, FT cs, FT sn)  {
      for( size_t i=0; i < m.RowCount(); i++ )  {
        const FT tmp = m(i,col_a)*cs + m(i,col_b)*sn;
        m(i,col_b) = m(i,col_b)*cs - m(i,col_a)*sn;
        m(i,col_a) = tmp;
      }
    }
    template <typename VecT1, typename VecT2>
    static void Apply(VecT1& a, VecT2& b, FT cs, FT sn)  {
      for( size_t i=0; i < a.Count(); i++ )  {
        const FT tmp = a(i)*cs + b(i)*sn;
        b(i) = b(i)*cs - a(i)*sn;
        a(i) = tmp;
      }
    }
  };  // end of struct Rotation
  template <typename FT> struct SVD  {
    TVector<FT> w;
    TMatrix<FT> u, vt;
    /* u_flag = 0 - u is not needed, 1 - only left vectors are needed, 2 - full u is needed
      vt_flag = 0 - vt is not needed, 1 - only right vectors are needed, 2 - full vt is needed
      extra_mem = false - no additional memory will be as of what is required to store requested
      matrices will be needed, for true - an intermediate matric will be allocated, this
      increases the performance
    */
    template <class MatT>
    bool Decompose(MatT& m, int u_flag=2, int vt_flag=2, bool extra_mem = false)  {
      if( m.IsEmpty() )  return true;
      const size_t min_d = olx_min(m.ColCount(), m.RowCount());
      w.Resize(min_d);
      if( u_flag == 0 )
        u.Resize(0, 0);
      else if( u_flag == 1 )
        u.Resize(m.RowCount(), min_d);
      else
        u.Resize(m.RowCount(), m.RowCount());
      if( vt_flag == 0 )
        vt.Resize(0, 0);
      else if( vt_flag == 1 )
        vt.Resize(min_d, m.ColCount());
      else
        vt.Resize(m.ColCount(), m.ColCount());
      
      if( m.RowCount() > 1.6*m.ColCount() )  {
        if( u_flag == 0 )  {
          TVector<FT> taus;
          QR<FT>::Decompose(m, taus);
          for( size_t i=1; i < m.ColCount(); i++ )  {
            for( size_t j=1; j < i; j++ )
              m(i,j) = 0;
          }
          TVector<FT> qtau, ptau;
          mat_mat<MatT,FT> mp(m, m.RowCount(), m.RowCount());
          Bidiagonal<FT>::ToBidiagonal(mp, qtau, ptau);
          Bidiagonal<FT>::UnpackPT(mp, ptau, vt.RowCount(), vt);
          TVector<FT> e;
          const bool upper = Bidiagonal<FT>::UnpackDiagonals(mp, w, e);
          return Bidiagonal<FT>::SVD::Decompose(w, e, upper, false, 
            proxy<FT>::mat_to(u,0,0), proxy<FT>::mat_to(m,0,0), vt);
        }
        else  {
          TVector<FT> taus;
          QR<FT>::Decompose(m, taus);
          QR<FT>::Unpack(m, taus, u.ColCount(), u);
          for( size_t i=1; i < m.ColCount(); i++ )  {
            for( size_t j=0; j < i; j++ )
              m(i,j) = 0;
          }
          TVector<FT> qtau, ptau, e;
          mat_mat<MatT,FT> mp(m, m.ColCount(), m.ColCount());
          Bidiagonal<FT>::ToBidiagonal(mp, qtau, ptau);
          Bidiagonal<FT>::UnpackPT(mp, ptau, vt.RowCount(), vt);
          const bool upper = Bidiagonal<FT>::UnpackDiagonals(mp, w, e);
          if( !extra_mem )  {
            Bidiagonal<FT>::MulByQ(mp, qtau, u, true, false);
            return Bidiagonal<FT>::SVD::Decompose(
              w, e, upper, false, u, proxy<FT>::mat_to(m, 0, 0), vt);
          }
          else  {
            TMatrix<FT> tmp;
            Bidiagonal<FT>::UnpackQ(mp, qtau, m.ColCount(), tmp);
            m.Assign(u, m.RowCount(), m.ColCount());
            tmp.Transpose();
            bool res;
            if( res = Bidiagonal<FT>::SVD::Decompose(w, e, upper, false,
                        proxy<FT>::mat_to(u, 0, 0), tmp, vt) )
            {
              tmp = m*tmp.Transpose();
              u.Assign(tmp, tmp.RowCount(), tmp.ColCount(), false);
            }
            return res;
          }
        }
      }
      else if( m.ColCount() > 1.6*m.RowCount() )  {
        if( u_flag == 0 )  {
          TVector<FT> taus;
          LQ<FT>::Decompose(m, taus);
          for( size_t i=1; i < m.RowCount()-1; i++ )  {
            for( size_t j=i+1; j < m.RowCount(); j++ )
              m(i,j) = 0;
          }
          TVector<FT> qtau, ptau, e;
          Bidiagonal<FT>::ToBidiagonal(m, qtau, ptau);
          Bidiagonal<FT>::UnpackQ(m, ptau, u.RowCount(), u);
          mat_mat<MatT,FT> mp(m, m.RowCount(), m.RowCount());
          const bool upper = Bidiagonal<FT>::UnpackDiagonals(mp, w, e);
          u.Transpose();
          const bool res = Bidiagonal<FT>::SVD::Decompose(w, e, upper, false, u, mp, vt);
          u.Transpose();
          return res;
        }
        else  {
          TVector<FT> taus;
          LQ<FT>::Decompose(m, taus);
          LQ<FT>::UnpackQ(m, taus, vt.ColCount(), vt);
          for( size_t i=0; i < m.RowCount()-1; i++ )  {
            for( size_t j=i+1; j < m.RowCount(); j++ )
              m(i,j) = 0;
          }
          TVector<FT> qtau, ptau, e;
          mat_mat<MatT,FT> mp(m, m.RowCount(), m.RowCount());
          Bidiagonal<FT>::ToBidiagonal(mp, qtau, ptau);
          Bidiagonal<FT>::UnpackQ(mp, qtau, u.RowCount(), u);
          const bool upper = Bidiagonal<FT>::UnpackDiagonals(mp, w, e);
          u.Transpose();
          bool res;
          if( !extra_mem )  {
            Bidiagonal<FT>::MulByP(mp, ptau, vt, false, true);
            res = Bidiagonal<FT>::SVD::Decompose(
              w, e, upper, false, proxy<FT>::mat_to(m, 0, 0), u, vt);
          }
          else  {
            TMatrix<FT> tmp;
            Bidiagonal<FT>::UnpackPT(mp, ptau, m.RowCount(), tmp);
            if( res = Bidiagonal<FT>::SVD::Decompose(
                        w, e, upper, false, proxy<FT>::mat_to(m,0,0), u, tmp) )
            {
              m.Assign(vt, m.RowCount(), m.ColCount());
              tmp = tmp * m;
              vt.Assign(tmp, tmp.RowCount(), tmp.ColCount(), false);
            }
          }
          u.Transpose();
          return res;
        }
      }
      else if( m.RowCount() <= m.ColCount() )  {
        TVector<FT> ptau, qtau, e;
        Bidiagonal<FT>::ToBidiagonal(m, qtau, ptau);
        Bidiagonal<FT>::UnpackQ(m, qtau, u.ColCount(), u);
        Bidiagonal<FT>::UnpackPT(m, ptau, vt.RowCount(), vt);
        const bool upper = Bidiagonal<FT>::UnpackDiagonals(m, w, e);
        u.Transpose();
        const bool res = Bidiagonal<FT>::SVD::Decompose(
          w, e, upper, false, proxy<FT>::mat_to(m, 0, 0), u, vt);
        u.Transpose();
        return res;
      }
      else  {
        TVector<FT> ptau, qtau, e;
        Bidiagonal<FT>::ToBidiagonal(m, qtau, ptau);
        Bidiagonal<FT>::UnpackQ(m, qtau, u.ColCount(), u);
        Bidiagonal<FT>::UnpackPT(m, ptau, vt.RowCount(), vt);
        const bool upper = Bidiagonal<FT>::UnpackDiagonals(m, w, e);
        if( !extra_mem )  {
          return Bidiagonal<FT>::SVD::Decompose(
          w, e, upper, false, u, proxy<FT>::mat_to(m, 0, 0), vt);
        }
        else  {
          TMatrix<FT> tmp(min_d, m.RowCount());
          for( size_t i=0; i < min_d; i++ )
            for( size_t j=0; j < m.RowCount(); j++ )
              tmp(i,j) = u(j,i);
          const bool res = Bidiagonal<FT>::SVD::Decompose(
            w, e, upper, false, proxy<FT>::mat_to(u, 0, 0), tmp, vt);
          for( size_t i=0; i < min_d; i++ )
            for( size_t j=0; j < m.RowCount(); j++ )
              u(j,i) = tmp(i,j);
          return res;
        }
      }
    }
  };
  void TestQR(OlxTests& t);
  void TestLU(OlxTests& t);
  void TestInvert(OlxTests& t);
  void TestSVD(OlxTests& t);
  static void AddTests(OlxTests& t)  {
    t.Add(&TestInvert);
    t.Add(&TestSVD);
    t.Add(&TestQR);
    t.Add(&TestLU);
  }
}; // end namespace math

EndEsdlNamespace()
#endif