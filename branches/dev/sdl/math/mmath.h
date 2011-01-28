#ifndef __olx_sdl_math_lu_H
#define __olx_sdl_math_lu_H
#include "../ematrix.h"
#include "../bapp.h"
BeginEsdlNamespace()

namespace math  {
  struct mat_col  {
    ematd& m;
    const size_t col, start, end, sz;
    mat_col(ematd& _m, size_t _col, size_t _start, size_t _end=InvalidIndex) :
      m(_m), col(_col), start(_start),
      end(_end==InvalidIndex ? m.RowCount() : _end), sz(end-start)  {}
    double& operator [] (size_t i)  {  return m[start+i][col];  }
    size_t Count() const {  return sz;  }
    mat_col& operator *= (double v)  {
      for( size_t i=start; i < end; i++ )
        m[i][col] *= v;
      return *this;  
    }
  };
  struct vec_row  {
    evecd& v;
    const size_t start, end, sz;
    vec_row(evecd& _v, size_t _start, size_t _end=InvalidIndex) :
      v(_v), start(_start),
      end(_end==InvalidIndex ? v.Count() : _end), sz(end-start)  {}
    double& operator [] (size_t i)  {  return v[start+i];  }
    vec_row& operator *= (double val)  {
      for( size_t i=start; i < end; i++ )
        v[i] *= val;
      return *this;
    }
    size_t Count() const {  return sz;  }
  };
  struct mat_row : public vec_row  {
    mat_row(ematd& _m, size_t _row, size_t _start, size_t _end=InvalidIndex) :
      vec_row(_m[_row], _start, _end)  {}
  };
  namespace alg  {
    template<typename ToT, typename FromT> void copy(ToT& to, const FromT& f)  {
      for( size_t i=0; i < f.Count(); i++ )
        to[i] = f[i];
    }
    template<typename ToT, typename FromT> void copy(ToT& to, const FromT& f, double s)  {
      for( size_t i=0; i < f.Count(); i++ )
        to[i] = f[i]*s;
    }
    template <typename T> void print(const T& a, size_t m, size_t n)  {
      TBasicApp::NewLogEntry() << "++++";
      for( size_t i=0; i < m; i++ )  {
        olxstr line;
        for( size_t j=0; j < n; j++ )
          line << olxstr::FormatFloat(-3, a[i][j], true) << ' ';
        TBasicApp::NewLogEntry() << line;
      }
    }
    template <typename T> void printx(const T& a, size_t m, size_t n)  {
      TBasicApp::NewLogEntry() << "___";
      for( size_t i=1; i <= m; i++ )  {
        olxstr line;
        for( size_t j=1; j <= n; j++ )
          line << olxstr::FormatFloat(-3, a(i,j), true) << ' ';
        TBasicApp::NewLogEntry() << line;
      }
    }
  };  // end namespace alg
  static void LUDecompose(ematd& m, eveci& pivots)  {
    const size_t
      min_d = olx_min(m.RowCount(), m.ColCount()),
      max_d = olx_max(m.RowCount(), m.ColCount());
    for( size_t i=0; i < min_d; i++ )  {
      size_t pivot_index = i;
      for( size_t j=i+1; j < m.RowCount(); j++ )  {
        if( olx_abs(m[j][i]) > olx_abs(m[pivot_index][i]) )
          pivot_index = j;
      }
      pivots[i] = pivot_index;
      if( m[pivot_index][i] != 0 )  {
        if( pivot_index != i )
          m.SwapRows(pivot_index, i);
        if( i < min_d-1 )
          mat_col(m, i, i+1) *= 1./m[i][i];
      }
      if( i < m.RowCount()-1 )  {
        for( size_t j=i+1; j < m.RowCount(); j++ )  {
          for( size_t k=i+1; k < m.RowCount(); k++ )
            m[j][k] -= m[j][i]*m[i][k];
        }
      }
    }
  }
  // returns false for singular matrix
  static bool InvertTriagular(ematd& m, bool upper, bool hasUnitDiag)  {
    evecd tmp(m.RowCount());
    if( upper )  {
      for( size_t i=0; i < m.RowCount(); i++ )  {
        double diag_v = -1;
        if( !hasUnitDiag )  {
          if( m[i][i] == 0 )
            return false;
          m[i][i] = 1./m[i][i];
          diag_v = -m[i][i];
        }
        if( i > 0 )  {
          for( size_t j=0; j < i; j++ )
            tmp[j] = m[j][i];
          for( size_t j=0; j < i; j++ )  {
            double v = 0.0;
            if( j < i-1 )  {
              for( size_t k=j+1; k < i; k++ )
                v += m[j][k]*tmp[k];
            }
            m[j][i] = v + m[j][j]*tmp[j];
          }
          mat_col(m, i, 0, i) *= diag_v;
        }
      }
    }
    else  {
      for( size_t i=m.RowCount()-1; i != InvalidIndex ; i-- )  {
        double diag_v = -1;
        if( !hasUnitDiag )  {
          if( m[i][i] == 0 )
            return false;
          m[i][i] = 1./m[i][i];
          diag_v = -m[i][i];
        }
        if( i < m.RowCount()-1 )  {
          for( size_t j=i+1; j < m.RowCount(); j++ )
            tmp[j] = m[j][i];
          for( size_t j=i+1; j < m.RowCount(); j++ )  {
            double v = 0.0;
            if( j > i+1 )  {
              for( size_t k=i+1; k < j; k++ )
                v += m[j][k]*tmp[k];
            }
            m[j][i] = v + m[j][j]*tmp[j];
          }
          for( size_t j=i+1; j < m.RowCount(); j++ )
            m[j][i] *= diag_v;
        }
      }
    }
    return true;
  }

  static bool InversLU(ematd& m, eveci& pivots)  {
    if( !InvertTriagular(m, true, false) )
      return false;
    evecd tmp(m.RowCount());
    for( size_t i=m.RowCount()-1; i != InvalidIndex; i-- )  {
      for( size_t j=i+1; j < m.RowCount(); j++ )  {
        tmp[j] = m[j][i];
        m[j][i] = 0.0;
      }
      if( i < m.RowCount()-1 )  {
        for( size_t j=0; j < m.RowCount(); j++ )  {
          double v = 0;
          for( size_t k=i+1; k < m.RowCount(); k++ )
            v += m[j][k]*tmp[k];
          m[j][i] -= v;
        }
      }
    }
    for( size_t i=m.RowCount()-1; i != InvalidIndex; i-- )  {
      if( pivots[i] != i )
        m.SwapCols(i, pivots[i]);
    }
    return true;
  }

  static bool Inverse(ematd& m)  {
    eveci pivots(m.RowCount());
    LUDecompose(m, pivots);
    return InversLU(m, pivots);
  }

  struct Reflection  {
    evecd tmp;
    Reflection(size_t sz) : tmp(sz) {}
    template <typename vec_t> double Generate(vec_t& v)  {
      if( v.Count() <= 1 )
        return 0;
      double max_abs_val = olx_abs(v[1]);
      for( size_t i=2; i < v.Count(); i++ )
        max_abs_val = olx_max(olx_abs(v[i]), max_abs_val);
      if( max_abs_val == 0 )
        return 0;
      double xnorm = 0;
      for( size_t i=1; i < v.Count(); i++ )
        xnorm += olx_sqr(v[i]/max_abs_val);
      xnorm = sqrt(xnorm)*max_abs_val;
      if( xnorm == 0 )
        return 0;
      max_abs_val = olx_max(olx_abs(v[0]), olx_abs(xnorm));
      double beta = -max_abs_val*sqrt(olx_sqr(v[0]/max_abs_val)+olx_sqr(xnorm/max_abs_val));
      if( v[0] < 0 )
        beta = -beta;
      const double mf = 1./(v[0]-beta);
      for( size_t i=1; i < v.Count(); i++ )
        v[i] *= mf;
      const double tau = (beta-v[0])/beta;
      v[0] = beta;
      return tau;
    }
    void ApplyFromLeft(ematd& m, double tau, const evecd& vt,
      size_t row_s, size_t row_e, size_t col_s, size_t col_e)
    {
      if( row_s >= row_e || col_s >= col_e )
        return;
      for( size_t i=col_s; i < col_e; i++ )
        tmp[i] = 0;
      for( size_t i=row_s; i < row_e; i++ )  {
        const double v = vt[i-row_s];
        for( size_t j=col_s; j < col_e; j++ )
          tmp[j] += m[i][j]*v;
      }
      for( size_t i=row_s; i < row_e; i++ )  {
        const double v = vt[i-row_s]*tau;
        for( size_t j=col_s; j < col_e; j++ )
          m[i][j] -= tmp[j]*v;
      }
    }
    void ApplyFromRight(ematd& m, double tau, const evecd& vt,
      size_t row_s, size_t row_e, size_t col_s, size_t col_e)
    {
      if( row_s >= row_e || col_s >= col_e )
        return;
      for( size_t i=row_s; i < row_e; i++ )  {
        double v = 0;
        for( size_t j=col_s; j < col_e; j++ )
          v += m[i][j]*vt[j-col_s];
        tmp[i] = v;
      }
      for( size_t i=row_s; i < row_e; i++ )  {
        const double v = tmp[i]*tau;
        for( size_t j=col_s; j < col_e; j++ )
          m[i][j] -= vt[j-col_s]*v;
      }
    }
  };  // end of Reflection

  static void QRDecompose(ematd& m, evecd& taus)  {
    const size_t min_d = olx_min(m.ColCount(), m.RowCount());
    Reflection r(min_d);
    taus.Resize(min_d);
    evecd tmp1(m.RowCount());
    for( size_t i=0; i < min_d; i++ )  {
      const size_t sz = m.RowCount()-i;
      for( size_t j=0; j < sz; j++ )
        tmp1[j] = m[i+j][i];
      taus[i] = r.Generate(vec_row(tmp1, 0, sz));
      for( size_t j=0; j < sz; j++ )
        m[i+j][i] = tmp1[j];
      tmp1[0] = 1;
      if( i < m.ColCount()-1 )
        r.ApplyFromLeft(m, taus[i], tmp1, i, m.RowCount(), i+1, m.ColCount());
    }
  }
  
  static void QRUnpack(const ematd& qr, const evecd& tau, size_t col_num, ematd& q)  {
    const size_t min_d = olx_min(qr.ColCount(), qr.RowCount());
    Reflection ref(col_num);
    q.Resize(qr.RowCount(), col_num);
    for( size_t i=0; i < qr.RowCount(); i++ )  {
      for( size_t j=0; j < col_num; j++ )
        q[i][j] = (i==j ? 1 : 0);
    }
    const size_t sz = olx_min(min_d, col_num)-1;
    evecd v(qr.RowCount());
    for( size_t i=sz; i != InvalidIndex; i-- )  {
      for( size_t j=0; j < qr.RowCount()-i; j++ )
        v[j] = qr[i+j][i];
      v[0] = 1;
      ref.ApplyFromLeft(q, tau[i], v, i, qr.RowCount(), 0, col_num);
    }
  }
  
  struct Bidiagonal  {
    static void ToBidiagonal(ematd& m, evecd& qtau, evecd& ptau)  {
      const size_t
        min_d = olx_min(m.ColCount(), m.RowCount()),
        max_d = olx_max(m.ColCount(), m.RowCount());
      Reflection ref(max_d);
      evecd t(max_d);
      qtau.Resize(min_d);
      ptau.Resize(min_d);
      if( m.RowCount() >= m.ColCount() )  {
        for( size_t i=0; i < m.ColCount(); i++ )  {
          const size_t sz = m.RowCount()-i;
          for( size_t j=0; j < sz; j++ )
            t[j] = m[i+j][i];
          qtau[i] = ref.Generate(vec_row(t, 0, sz));
          for( size_t j=0; j < sz; j++ )
            m[i+j][i] = t[j];
          t[0] = 1;
          ref.ApplyFromLeft(m, qtau[i], t, i, m.RowCount(), i+1, m.ColCount());
          if( i < m.ColCount()-1 )  {
            const size_t sz = m.ColCount()-i-1;
            for( size_t j=0; j < sz; j++ )
              t[j] = m[i][i+j+1];
            ptau[i] = ref.Generate(vec_row(t, 0, sz));
            for( size_t j=0; j < sz; j++ )
              m[i][i+j+1] = t[j];
            t[0] = 1;
            ref.ApplyFromRight(m, ptau[i], t, i+1, m.RowCount(), i+1, m.ColCount());
          }
          else
            ptau[i] = 0;
        }
      }
      else  {
        for( size_t i=0; i < m.RowCount(); i++ )  {
          const size_t sz = m.ColCount()-i;
          for( size_t j=0; j < sz; j++ )
            t[j] = m[i][i+j];
          ptau[i] = ref.Generate(vec_row(t, 0, sz));
          for( size_t j=0; j < sz; j++ )
            m[i][i+j] = t[j];
          t[0] = 1;
          ref.ApplyFromRight(m, ptau[i], t, i+1, m.RowCount(), i, m.ColCount());
          if( i < m.RowCount()-1 )  {
            const size_t sz = m.RowCount()-i;
            for( size_t j=0; j < sz; j++ )
              t[j] = m[i+j][i];
            qtau[i] = ref.Generate(vec_row(t, 0, sz));
            for( size_t j=0; j < sz; j++ )
              m[i+j][i] = t[j];
            t[0] = 1;
            ref.ApplyFromLeft(m, qtau[i], t, i+1, m.RowCount(), i+1, m.ColCount());
          }
          else
            qtau[i] = 0;
        }
      }
    }
    static void UnpackQ(const ematd& qp, const evecd& qtau, size_t col_cnt, ematd& q)  {
      q.Resize(qp.RowCount(), col_cnt);
      Reflection r(col_cnt);
      evecd tmp(qp.RowCount());
      for( size_t i=0; i < qp.RowCount(); i++ )  {
        for( size_t j=0; j < col_cnt; j++ )
          q[i][j] = (i==j ? 1 : 0);
      }
      if( qp.RowCount() >= qp.ColCount() )  {
        const size_t s = olx_min(qp.ColCount(), col_cnt);
        for( size_t i=s; i != InvalidIndex; i-- )  {
          const size_t sz = qp.RowCount()-i;
          for( size_t j=0; j < sz; j++ )
            tmp[j] = qp[i+j][i];
          tmp[0] = 1;
          r.ApplyFromLeft(q, qtau[i], tmp, i, qp.RowCount(), 0, col_cnt);
        }
      }
      else  {
        const size_t s = olx_min(qp.RowCount(), col_cnt)-1;
        for( size_t i=s; i != InvalidIndex; i-- )  {
          const size_t sz = qp.RowCount()-i-1;
          for( size_t j=0; j < sz; j++ )
            tmp[j] = qp[i+j+1][i];
          tmp[0] = 1;
          r.ApplyFromLeft(q, qtau[i], tmp, i+1, qp.RowCount(), 0, col_cnt);
        }
      }
    }
    static void UnpackPT(const ematd& qp, const evecd& ptau, size_t row_cnt, ematd& pt)  {
      pt.Resize(row_cnt, qp.ColCount());
      Reflection r(qp.ColCount());
      evecd tmp(row_cnt);
      for( size_t i=0; i < row_cnt; i++ )  {
        for( size_t j=0; j < qp.ColCount(); j++ )
          pt[i][j] = (i==j ? 1 : 0);
      }
      if( qp.RowCount() >= qp.ColCount() )  {
        const size_t s = olx_min(qp.ColCount(), row_cnt)-1;
        for( size_t i=s; i != InvalidIndex; i-- )  {
          const size_t sz = qp.ColCount()-i-1;
          for( size_t j=0; j < sz; j++ )
            tmp[j] = qp[i][i+j+1];
          tmp[0] = 1;
          r.ApplyFromRight(pt, ptau[i], tmp, 0, row_cnt, i+1, qp.ColCount());
        }
      }
      else  {
        const size_t s = olx_min(qp.RowCount(), row_cnt);
        for( size_t i=s; i != InvalidIndex; i-- )  {
          const size_t sz = qp.ColCount()-i-1;
          for( size_t j=0; j < sz; j++ )
            tmp[j] = qp[i][i+j];
          tmp[0] = 1;
          r.ApplyFromRight(pt, ptau[i], tmp, 0, row_cnt, i, qp.ColCount());
        }
      }
    }
    static void UnpackDiagonals(const ematd& m, bool& upper, evecd& d, evecd& e)  {
      upper = m.RowCount() >= m.ColCount();
      if( m.ColCount() == 0 || m.RowCount() == 0 )
        return;
      if( upper )  {
        d.Resize(m.ColCount());
        e.Resize(m.ColCount());
        for( size_t i=0; i < m.ColCount()-1; i++ )  {
          d[i] = m[i][i];
          e[i] = m[i][i+1];
        }
        d.GetLast() = m[m.ColCount()-1][m.ColCount()-1];
      }
      else  {
        d.Resize(m.RowCount());
        e.Resize(m.RowCount());
        for( size_t i=0; i < m.RowCount()-1; i++ )  {
          d[i] = m[i][i];
          e[i] = m[i][i+1];
        }
        d.GetLast() = m[m.RowCount()-1][m.RowCount()-1];
      }
    }
    static void MulByQ(const ematd& qp, const evecd& tauq, ematd& z, size_t z_m, size_t z_n,
      bool from_right, bool dot_trans)  {
      const size_t mx = olx_max(
        olx_max(qp.ColCount(), qp.RowCount()),
        max(z_m, z_n));
      Reflection ref(mx);
      if( qp.RowCount() >= qp.ColCount() )  {
        int i1, i2, step_inc;
        if( from_right )  {
          i1 = 1;
          i2 = qp.ColCount();
          step_inc = 1;
        }
        else  {
          i1 = qp.ColCount();
          i2 = 1;
          step_inc = -1;
        }
        if( dot_trans )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        evecd tmp(mx);
        for( int i=i1; i < i2; i+= step_inc )  {
          for( size_t j=0; j < qp.RowCount()-i; j++ )
            tmp[j] = qp[i+j][i];
          tmp[0] = 1;
          if( from_right )
            ref.ApplyFromRight(z, tauq[i], tmp, 0, z_m, i, qp.RowCount());
          else
            ref.ApplyFromLeft(z, tauq[i], tmp, i, qp.RowCount(), 0, z_n);
        }
      }
      else  {
        if( qp.RowCount() == 1 )  return;
        int i1 = 1, i2 = qp.RowCount()-1, step_inc = 1;
        if( !from_right )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        if( dot_trans )  {
          olx_swap(i1, i2);
          step_inc = -step_inc;
        }
        evecd tmp(mx);
        for( int i=i1; i < i2; i+= step_inc )  {
          for( size_t j=0; j < qp.RowCount()-i; j++ )
            tmp[j] = qp[i+j][i+1];
          tmp[0] = 1;
          if( from_right )
            ref.ApplyFromRight(z, tauq[i], tmp, 0, z_m, i+1, qp.RowCount());
          else
            ref.ApplyFromLeft(z, tauq[i], tmp, i+1, qp.RowCount(), 0, z_n);
        }
      }
    }
  }; // end of BiDiagonal
  struct Rotation  {
    evecd tmp;
    Rotation(size_t sz) : tmp(sz)  {}
    static void Generate(double f, double g, double& cs, double& sn, double& r)  {
      sn = 0;
      cs = 1;
      if( g == 0 )  {  r = f;  return;  }
      if( f == 0 )  {  r = g;  return;  }
      r = sqrt(f*f+g*g);
      if( olx_abs(f) > olx_abs(g) && f < 0 )  {
        sn = f/r;
        cs = g/r;
      }
      else  {
        sn = -f/r;
        cs = -g/r;
        r = -r;
      }
    }
    void ApplyFromLeft(bool forward, size_t row_s, size_t row_e, size_t col_s, size_t col_e,
      const evecd& c, const evecd& s, ematd& m)
    {
      if( forward )  {
        for( size_t i=row_s; i < row_e-1; i++ )  {
          const double
            cv = c[i-row_s],
            sv = s[i-row_s];
          if( cv == 1 && sv == 0 )  continue;
          for( size_t j=col_s; j < col_e; j++ )  {
            const double tmp = m[i+1][j]*cv - m[i][j]*sv;
            m[i][j] = m[i][j]*cv + m[i+1][j]*sv;
            m[i+1][j] = tmp;
          }
        }
      }
      else  {
        for( size_t i=row_e; i > row_s; i-- )  {
          const double
            cv = c[i-row_s],
            sv = s[i-row_s];
          if( cv == 1 && sv == 0 )  continue;
          for( size_t j=col_s; j < col_e; j++ )  {
            const double tmp = m[i][j]*cv - m[i-1][j]*sv;
            m[i-1][j] = m[i-1][j]*cv + m[i][j]*sv;
            m[i][j] = tmp;
          }
        }
      }
    }
    void ApplyFromRight(bool forward, size_t row_s, size_t row_e, size_t col_s, size_t col_e,
      const evecd& c, const evecd& s, ematd& m)
    {
      if( forward )  {
        for( size_t i=col_s; i < col_e-1; i++ )  {
          const double
            cv = c[i-col_s],
            sv = s[i-col_s];
          if( cv == 1 && sv == 0 )  continue;
          for( size_t j=row_s; j < row_e; j++ )  {
            const double tmp = m[j][i+1]*cv - m[j][i]*sv;
            m[j][i] = m[j][i]*cv + m[j][i+1]*sv;
            m[j][i+1] = tmp;
          }
        }
      }
      else  {
        for( size_t i=col_e; i > col_s; i-- )  {
          const double
            cv = c[i-col_s],
            sv = s[i-col_s];
          if( cv == 1 && sv == 0 )  continue;
          for( size_t j=row_s; j < row_e; j++ )  {
            const double tmp = m[j][i]*cv - m[j][i-1]*sv;
            m[j][i-1] = m[j][i-1]*cv + m[j][i]*sv;
            m[j][i] = tmp;
          }
        }
      }
    }
  };  // end of struct Rotation
  struct SVD  {
    static double ext_sign(double a, double b)  {
      if( b >= 0 )
        return olx_abs(a);
      else
        return a < 0 ? a : -a;
    }
    static void do2x2(double f, double g, double h,
      double& ssmin, double& ssmax, double& snr, double& csr, double& snl, double& csl)
    {
      double abs_f = olx_abs(f), abs_h = olx_abs(h), abs_g = olx_abs(g);
      double clt = 1, srt = 0, slt = 0, crt = 1;
      int flag = 1;
      if( abs_h > abs_f )  {
        flag = 3;
        olx_swap(f, h);
        olx_swap(abs_f, abs_h);
      }
      if( abs_g != 0 )  {
        bool small_g = true;
        if( abs_g > abs_f )  {
          flag = 2;
          if( abs_f/abs_g < 1e-16 )  {
            small_g = false;
            ssmax = abs_g;
            if( abs_g > 1 )
              ssmin = abs_f*abs_h/abs_g;
            else
              ssmin = abs_h*abs_g/abs_f;
            clt = srt = 1;
            slt = h/g;
            crt = f/g;
          }
        }
        if( small_g )  {
          const double d = abs_f - abs_h;
          const double l = d/abs_f;
          const double m = g/f, mm = m*m;
          double t = 2-l, tt = t*t;
          const double s = sqrt(mm+tt);
          const double r = (l ==0 ? olx_abs(m) : sqrt(l*l+mm));
          const double a = 0.5*(s+r);
          ssmin = abs_h/a;
          ssmax = abs_f*a;
          if( mm == 0 )  {
            if( l == 0 )
              t = ext_sign(2, f)*ext_sign(1, g);
            else
              t = g/ext_sign(d, f) + m/t;
          }
          else
            t = (m/(s+t)+m/(r+l))*(1+a);
          const double tmp = sqrt(t*t+4);
          crt = 2./tmp;
          srt = t/tmp;
          clt = (crt+srt*m)/a;
          slt = srt*h/(f*a);
        }
      }
      if( abs_h > abs_f )  {
        csl = srt;
        snl = crt;
        csr = slt;
        snr = clt;
      }
      else  {
        csl = clt;
        snl = slt;
        csr = crt;
        snr = srt;
      }
      double sig = 0;
      if( flag == 1 )
        sig = ext_sign(1, csr)*ext_sign(1, csl)*ext_sign(1, f);
      else if( flag == 2 )
        sig = ext_sign(1, snr)*ext_sign(1, csl)*ext_sign(1, g);
      else if( flag == 2 )
        sig = ext_sign(1, snr)*ext_sign(1, snl)*ext_sign(1, h);
      ssmax = ext_sign(ssmax, sig);
      ssmin = ext_sign(ssmin, sig*ext_sign(1, f)*ext_sign(1, h));
    }
    static void do2x2(double f, double g, double h, double& ssmin, double& ssmax)  {
      const double abs_f = olx_abs(f), abs_h = olx_abs(h), abs_g = olx_abs(g);
      const double min_fh = olx_min(abs_f, abs_h),
        max_fh = olx_max(abs_f, abs_h);
      if( min_fh == 0 )  {
        ssmin = 0;
        const double mg = olx_max(max_fh, abs_g);
        ssmax = max_fh == 0 ? abs_g : mg*sqrt(1+olx_sqr(olx_min(min_fh, abs_g)/mg));
      }
      else  {
        if( abs_g < max_fh )  {
          const double a = 1+min_fh/max_fh,
            b = (max_fh-min_fh)/max_fh,
            c = olx_sqr(abs_g/max_fh);
          const double d = 2./(sqrt(a*a+c)+sqrt(b*b+c));
          ssmin = min_fh*d;
          ssmax = max_fh/d;
        }
        else  {
          const double _a = max_fh/abs_g;
          if( _a == 0 )  {
            ssmin = min_fh*max_fh/abs_g;
            ssmax = abs_g;
          }
          else  {
            const double a = 1+min_fh/max_fh,
              b = (max_fh-min_fh)/max_fh,
              c = 1./(sqrt(1+olx_sqr(a*_a))+sqrt(1+olx_sqr(b*_a)));
            ssmin = 2*min_fh*c*_a;
            ssmax = abs_g/(2*c);
          }
        }
      }
    }
    static bool Decompose(evecd& d, evecd& e, bool upper, bool fract_accu,
      ematd& u, ematd& c, ematd& vt)
    {
      if( d.Count() == 0 )  return true;
      if( d.Count() == 1 )  {
        if( d[0] < 0 )  {
          d[0] = -d[0];
          if( vt.RowCount() > 0 )
            vec_row(vt[0], 0) *= -1;
        }
        return true;
      }
      evecd w0(d.Count()-1), w1(d.Count()-1), w2(d.Count()-1), w3(d.Count()-1);
      //evecd utmp(olx_max(1, u.RowCount())),
      //  vttmp(olx_max(1, vt.RowCount())),
      //  ctmp(olx_max(1, c.RowCount())),
      //  etmp(d.Count());
      Rotation rot(d.Count());
      int max_itr = 6;
      bool forward = true;
      e.Resize(d.Count());
      const double _eps = 1e-16,
        unfl = 1e-300;

      int flag = 0;
      if( !upper )  {
        for( size_t i=0; i < d.Count()-1; i++ )  {
          double sn, cs, r;
          rot.Generate(d[i], e[i], cs, sn, r);
          d[i] = r;
          e[i] = sn*d[i+1];
          d[i+1] *= cs;
          w0[i] = cs;
          w1[i] = sn;
        }
        if( u.ColCount() > 0 )
          rot.ApplyFromRight(forward, 0, u.ColCount(), 0, d.Count(), w0, w1, u);
        if( c.ColCount() > 0 )
          rot.ApplyFromLeft(forward, 0, d.Count(), 0, c.ColCount(), w0, w1, c);
      }
      double tol = olx_max(10, olx_min(100, pow(_eps, -1./8)))*_eps;
      if( !fract_accu )
        tol = -tol;
      double smax = 0;
      for( size_t i=0; i < d.Count()-1; i++ )
        smax = olx_max(smax, olx_max(olx_abs(e[i]), olx_abs(d[i])));
      smax = olx_max(smax, olx_abs(d.GetLast()));
      double sminl = 0, thresh;
      if( tol >=0 )  {
        double sminoa = olx_abs(d[0]);
        if( sminoa != 0 )  {
          double mu = sminoa;
          for( size_t i=1; i < d.Count(); i++ )  {
            mu = olx_abs(d[i])*(mu/(mu+olx_abs(e[i-1])));
            if( (sminoa = olx_min(sminoa, mu)) == 0 )
              break;
          }
        }
        sminoa /= sqrt((double)d.Count());
        thresh = olx_max(tol*sminoa, max_itr*olx_sqr(d.Count())*unfl);
      }
      else
        thresh = olx_max(olx_abs(tol)*smax, max_itr*olx_sqr(d.Count())*unfl);
      int max_i = max_itr*olx_sqr(d.Count()),
        m = d.Count()-1, iter = 0;
      int oldm = -1, oldll = -1;
      for(;;)  {
        if( m <= 0 )
          break;
        if( iter > max_i )
          return false;
        if( tol < 0 && olx_abs(d[m]) <= thresh )
          d[m] = 0;
        double smin = (smax = olx_abs(d[m]));
        bool split_flag = false;
        int ll = 0;
        for( int i=0; i < m-1; i++ )  {
          ll = m-i-1;
          const double
            abs_s = olx_abs(d[ll]),
            abs_e = olx_abs(e[ll]);
          if( tol < 0 && abs_s <= thresh )
            d[ll] = 0;
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
          e[ll] = 0;
          if( ll == m-1 )  {
            m--;
            continue;
          }
        }
        ll++;
        if( ll == m-1 )  {
          double sigmn, sigmx, sinr, cosr, sinl, cosl;
          do2x2(d[m-1], e[m-1], d[m], sigmn, sigmx, sinr, cosr, sinl, cosl);
          d[m-1] = sigmx;
          e[m-1] = 0;
          d[m] = sigmn;
          if( vt.RowCount() > 0 )  {
            for( size_t i=0; i < vt.ColCount(); i++ )  {
              double tmp = vt[m-1][i]*cosr + vt[m][i]*sinr;
              vt[m][i] = vt[m][i]*cosr - vt[m-1][i]*sinr;
              vt[m-1][i] = tmp;
            }
          }
          if( u.RowCount() > 0 )  {
            for( size_t i=0; i < u.ColCount(); i++ )  {
              double tmp = u[m-1][i]*cosl + u[m][i]*sinl;
              u[m][i] = u[m][i]*cosl - u[m-1][i]*sinl;
              u[m-1][i] = tmp;
            }
          }
          if( c.RowCount() > 0 )  {
            for( size_t i=0; i < c.ColCount(); i++ )  {
              double tmp = c[m-1][i]*cosl + c[m][i]*sinl;
              c[m][i] = c[m][i]*cosl - c[m-1][i]*sinl;
              c[m-1][i] = tmp;
            }
          }
          m -= 2;
          continue;
        }
        if( ll > oldm || m < oldll )
          flag = (olx_abs(d[ll]) >= olx_abs(d[m]) ? 1 : 2);
        if( flag == 1 )  {
          if( olx_abs(e[m-1]) <= olx_abs(tol*d[m]) || (tol < 0 && olx_abs(e[m-1]) < thresh) ) {
            e[m-1] = 0;
            continue;
          }
          if( tol >= 0 )  {
            double mu = olx_abs(d[ll]);
            sminl = mu;
            bool iter_flag = false;
            for( int i=ll; i < m-1; i++ )  {
              if( olx_abs(e[i]) <= tol*mu )  {
                e[i] = 0;
                iter_flag = true;
                break;
              }
              mu = olx_abs(d[i+1])*(mu/(mu+olx_abs(e[i])));
              sminl = olx_min(sminl, mu);
            }
            if( iter_flag )
              continue;
          }
        }
        else  {
          if( olx_abs(e[ll]) <= olx_abs(tol*d[ll]) || (tol < 0 && olx_abs(e[ll]) < thresh) ) {
            e[ll] = 0;
            continue;
          }
          if( tol >= 0 )  {
            double mu = olx_abs(d[m]);
            sminl = mu;
            bool iter_flag = false;
            for( int i=m-1; i >= ll; i-- )  {
              if( olx_abs(e[i]) <= tol*mu )  {
                e[i] = 0;
                iter_flag = true;
                break;
              }
              mu = olx_abs(d[i])*(mu/(mu+olx_abs(e[i])));
              sminl = olx_min(sminl, mu);
            }
            if( iter_flag )
              continue;
          }
        }
        oldll = ll;
        oldm = m;
        double shift = 0, r = 0;
        if( !(tol >=0 && d.Count()*tol*(sminl/smax) <= olx_max(_eps, 0.01*tol)) )  {
          double sll;
          if( flag == 1 )  {
            sll = olx_abs(d[ll]);
            do2x2(d[m-1], e[m-1], d[m], shift, r);
          }
          else  {
            sll = olx_abs(d[m]);
            do2x2(d[ll], e[ll], d[ll+1], shift, r);
          }
          if( sll > 0 && olx_sqr(shift/sll) < _eps )
            shift = 0;
        }
        iter = iter+m-ll;
        if( shift == 0 )  {
          if( flag == 1 )  {
            double cs = 1, sn, r, oldcs = 1, oldsn = 0, tmp;
            for( int i=ll; i < m-1; i++ )  {
              rot.Generate(d[i]*cs, e[i], cs, sn, r);
              if( i > ll )
                e[i-1] = oldsn*r;
              rot.Generate(oldcs*r, d[i+1]*sn, oldcs, oldsn, tmp);
              d[i] = tmp;
              w0[i-ll] = cs;
              w1[i-ll] = sn;
              w2[i-ll] = oldcs;
              w3[i-ll] = oldsn;
            }
            const double h = d[m]*cs;
            d[m] = h*oldcs;
            e[m-1] = h*oldsn;
            if( vt.ColCount() > 0 )
              rot.ApplyFromLeft(forward, ll, m, 0, vt.ColCount(), w0, w1, vt);
            if( u.ColCount() > 0 )
              rot.ApplyFromRight(forward, 0, u.ColCount(), ll, m, w2, w3, u);
            if( c.ColCount() > 0 )
              rot.ApplyFromLeft(forward, ll, m, 0, c.ColCount(), w2, w3, c);
            if( olx_abs(e[m-1]) <= thresh )
              e[m-1] = 0;
          }
          else  {
            double cs = 1, sn, r, oldcs = 1, oldsn = 0, tmp;
            for( int i=m; i >= ll+1; i-- )  {
              rot.Generate(d[i]*cs, e[i-1], cs, sn, r);
              if( i < m )
                e[i] = oldsn*r;
              rot.Generate(oldcs*r, d[i-1]*sn, oldcs, oldsn, tmp);
              d[i] = tmp;
              w0[i-ll-1] = cs;
              w1[i-ll-1] = -sn;
              w2[i-ll-1] = oldcs;
              w3[i-ll-1] = -oldsn;
            }
            const double h = d[ll]*cs;
            d[ll] = h*oldcs;
            e[ll] = h*oldsn;
            if( vt.ColCount() > 0 )
              rot.ApplyFromLeft(!forward, ll, m, 0, vt.ColCount(), w2, w3, vt);
            if( u.ColCount() > 0 )
              rot.ApplyFromRight(!forward, 0, u.ColCount(), ll, m, w0, w1, u);
            if( c.ColCount() > 0 )
              rot.ApplyFromLeft(!forward, ll, m, 0, c.ColCount(), w0, w1, c);
            if( olx_abs(e[m-1]) <= thresh )
              e[m-1] = 0;
            if( olx_abs(e[ll]) < thresh )
              e[ll] = 0;
          }
        }
        else  {
          if( flag == 1 )  {
            double f = olx_abs(d[ll]-shift)*(ext_sign(1, d[ll])+shift/d[ll]);
            double g = e[ll];
            double cosr, sinr, cosl, sinl, r;
            for( int i=ll; i < m; i++ )  {
              rot.Generate(f, g, cosr, sinr, r);
              if( i > ll )
                e[i-1] = r;
              f = cosr*d[i]+sinr*e[i];
              e[i] = cosr*e[i]-sinr*d[i];
              g = sinr*d[i+1];
              d[i+1] *= cosr;
              rot.Generate(f, g, cosl, sinl, r);
              d[i] = r;
              f = cosl*e[i]+sinl*d[i+1];
              d[i+1] = cosl*d[i+1]-sinl*e[i];
              if( i < m-1 ) {
                g = sinl*e[i+1];
                e[i+1] = cosl*e[i+1];
              }
              w0[i-ll] = cosr;
              w1[i-ll] = sinr;
              w2[i-ll] = cosl;
              w3[i-ll] = sinl;
            }
            e[m-1] = f;
            if( vt.ColCount() > 0 )
              rot.ApplyFromLeft(forward, ll, m, 0, vt.ColCount(), w0, w1, vt);
            if( u.ColCount() > 0 )
              rot.ApplyFromRight(forward, 0, u.ColCount(), ll, m, w2, w3, u);
            if( c.ColCount() > 0 )
              rot.ApplyFromLeft(forward, ll, m, 0, c.ColCount(), w2, w3, c);
            if( olx_abs(e[m-1]) <= thresh )
              e[m-1] = 0;
          }
          else  {
            double f = olx_abs(d[m]-shift)*(ext_sign(1, d[m])+shift/d[m]);
            double g = e[m-1];
            double cosr, sinr, cosl, sinl, r;
            for( int i=m; i >= ll+1; i-- )  {
              rot.Generate(f, g, cosr, sinr, r);
              if( i < m )
                e[i] = r;
              f = cosr*d[i]+sinr*e[i-1];
              e[i] = cosr*e[i-1]-sinr*d[i];
              g = sinr*d[i-1];
              d[i-1] *= cosr;
              rot.Generate(f, g, cosl, sinl, r);
              d[i] = r;
              f = cosl*e[i-1]+sinl*d[i-1];
              d[i+1] = cosl*d[i-1]-sinl*e[i-1];
              if( i > ll+1 ) {
                g = sinl*e[i-2];
                e[i-2] = cosl*e[i-2];
              }
              w0[i-ll-1] = cosr;
              w1[i-ll-1] = sinr;
              w2[i-ll-1] = cosl;
              w3[i-ll-1] = sinl;
            }
            if( olx_abs(e[ll] = f) < thresh )
              e[ll] = 0;
            if( vt.ColCount() > 0 )
              rot.ApplyFromLeft(!forward, ll, m, 0, vt.ColCount(), w2, w3, vt);
            if( u.RowCount() > 0 )
              rot.ApplyFromRight(!forward, 0, u.ColCount(), ll, m, w0, w1, u);
            if( c.ColCount() > 0 )
              rot.ApplyFromLeft(!forward, ll, m, 0, c.ColCount(), w0, w1, c);
          }
        }
      }
      for( size_t i=0; i < d.Count(); i++ )  {
        if( d[i] < 0 )  {
          d[i] = -d[i];
          if( vt.RowCount() > 0 )
            vec_row(vt[i], 0) *= -1;
        }
      }
      // eventually - sorting...
      for( size_t i=0; i < d.Count()-1; i++ )  {
        double min_s = d[0];
        int min_i = 0;
        for( size_t j=1; j < d.Count()-i; j++ )  {
          if( d[j] <= min_s )  {
            min_i = j;
            min_s = d[j];
          }
        }
        int n = d.Count()-1;
        if( min_i != n-i )  {
          d[min_i] = d[n-i];
          d[n-i] = min_s;
          if( vt.ColCount() > 0 )  {
            for( size_t j=0; j < vt.ColCount(); j++ )
              olx_swap(vt[min_i][j], vt[n-i][j]);
          }
          if( u.ColCount() > 0 )  {
            for( size_t j=0; j < u.ColCount(); j++ )
              olx_swap(u[min_i][j], u[n-i][j]);
          }
          if( c.ColCount() > 0 )  {
            for( size_t j=0; j < c.ColCount(); j++ )
              olx_swap(c[min_i][j], c[n-i][j]);
          }
        }
      }
      return true;
    }
    static bool Decompose(ematd& m, int u_flag, int vt_flag, evecd& w, ematd& u, ematd& vt)  {
      if( m.ColCount() == 0 || m.RowCount() == 0 )
        return true;
      const size_t min_d = olx_min(m.ColCount(), m.RowCount());
      w.Resize(min_d);
      if( u_flag == 1 )
        u.Resize(m.RowCount(), min_d);
      else
        u.Resize(m.RowCount(), m.RowCount());
      if( vt_flag == 1 )
        vt.Resize(min_d, m.ColCount());
      else
        vt.Resize(m.ColCount(), m.ColCount());
      
      if( m.RowCount() > 1.6*m.ColCount() )  {
        if( u_flag == 0 )  {
          evecd taus;
          QRDecompose(m, taus);
          for( size_t i=1; i < m.ColCount(); i++ )  {
            for( size_t j=1; j < i; j++ )
              m[i][j] = 0;
          }
          evecd qtau, ptau;
          Bidiagonal::ToBidiagonal(m, qtau, ptau);
          Bidiagonal::UnpackPT(m, ptau, vt.RowCount(), vt);
          bool upper;
          evecd e;
          Bidiagonal::UnpackDiagonals(m, upper, w, e);
          return Decompose(w, e, upper, false, u, m, vt);
        }
        else  {
          evecd taus;
          QRDecompose(m, taus);
          QRUnpack(m, taus, u.ColCount(), u);
          for( size_t i=1; i < m.ColCount(); i++ )  {
            for( size_t j=0; j < i; j++ )
              m[i][j] = 0;
          }
          evecd qtau, ptau, e;
          m.Resize(m.ColCount(), m.ColCount());
          Bidiagonal::ToBidiagonal(m, qtau, ptau);
          Bidiagonal::UnpackPT(m, ptau, vt.RowCount(), vt);
          bool upper;
          Bidiagonal::UnpackDiagonals(m, upper, w, e);
          Bidiagonal::MulByQ(m, qtau, u, u.RowCount(), m.ColCount(), true, false);
          return Decompose(w, e, upper, false, u, m, vt);
        }
      }
      return true;
    }
  };
}; // end namespace math

EndEsdlNamespace()
#endif