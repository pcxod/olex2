/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "emath.h"

/*
Adopted procedure, source:  C derivation from the fortran version of CONREC by Paul Bourke
(http://local.wasp.uwa.edu.au/~pbourke/papers/conrec/)
Contains original documentation.
*/
template <typename Float> struct Contour {
  // feedback support structures
  class IFeedback  {
  public:
    virtual void OnLine(Float x1, Float y1, Float x2, Float y2, Float z) const = 0;
  };
  class StaticFeedback : public IFeedback  {
  protected:
    void (*DrawLine)(Float,Float,Float,Float,Float);
  public:
    StaticFeedback(void (*_DrawLine)(Float,Float,Float,Float,Float)) : DrawLine(_DrawLine)  {}
    virtual void OnLine(Float x1, Float y1, Float x2, Float y2, Float z) const {
      (*DrawLine)(x1, y1, x2, y2, z);
    }
  };

  template <class Base> class MemberFeedback : public IFeedback  {
  protected:
    Base& instance;
    void (Base::*DrawLine)(Float,Float,Float,Float,Float);
  public:
    MemberFeedback(Base& _instance, void (Base::*_DrawLine)(Float,Float,Float,Float,Float)) :
        instance(_instance), DrawLine(_DrawLine)  {}
    virtual void OnLine(Float x1, Float y1, Float x2, Float y2, Float z) const {
      (instance.*DrawLine)(x1, y1, x2, y2, z);
    }
  };

  template <class Base> class MemberFeedback<const Base&> : public IFeedback  {
  protected:
    const Base& instance;
    void (Base::*DrawLine)(Float,Float,Float,Float,Float) const;
  public:
    MemberFeedback(const Base& _instance, void (Base::*_DrawLine)(Float,Float,Float,Float,Float) const) :
        instance(_instance), DrawLine(_DrawLine)  {}
    virtual void OnLine(Float x1, Float y1, Float x2, Float y2, Float z) const {
      (instance.*DrawLine)(x1, y1, x2, y2, z);
    }
  };
protected:
  Float h[5];
  int sh[5];
  Float xh[5], yh[5];
  inline Float xsect(size_t p1, size_t p2) const {  return (h[p2]*xh[p1]-h[p1]*xh[p2])/(h[p2]-h[p1]);  }
  inline Float ysect(size_t p1, size_t p2) const {  return (h[p2]*yh[p1]-h[p1]*yh[p2])/(h[p2]-h[p1]);  }
public:
  void DoContour(Float **d, int x_start, int x_end, int y_start, int y_end,
    Float *x, Float *y, size_t contour_number, Float *z,
    IFeedback& callback)
  {
    static const size_t im[4] = {0,1,1,0}, jm[4]={0,0,1,1};
    static const size_t castab[3][3][3] = {
      { {0,0,8},{0,2,5},{7,6,9} },
      { {0,3,4},{1,3,1},{4,3,0} },
      { {9,6,7},{5,2,0},{8,0,0} }
    };
    for( int j=(y_end-1); j >= y_start; j-- ) {
      for( int i=x_start; i <= x_end-1; i++) {
        Float dmin  = olx_min(olx_min(d[i][j],d[i][j+1]), olx_min(d[i+1][j],d[i+1][j+1]));
        Float dmax  = olx_max(olx_max(d[i][j],d[i][j+1]), olx_max(d[i+1][j],d[i+1][j+1]));
        if (dmax < z[0] || dmin > z[contour_number-1])
          continue;
        for( size_t k=0; k < contour_number; k++ ) {
          if (z[k] < dmin || z[k] > dmax)
            continue;
          for( int m=4; m >= 0; m-- ) {
            if (m > 0) {
              h[m]  = d[i+im[m-1]][j+jm[m-1]]-z[k];
              xh[m] = x[i+im[m-1]];
              yh[m] = y[j+jm[m-1]];
            } else {
              h[0]  = Float(0.25) * (h[1]+h[2]+h[3]+h[4]);
              xh[0] = Float(0.50) * (x[i]+x[i+1]);
              yh[0] = Float(0.50) * (y[j]+y[j+1]);
            }
            if( h[m] > 0 )
              sh[m] = 1;
            else if( h[m] < 0 )
              sh[m] = -1;
            else
              sh[m] = 0;
          }

          /*
          Note: at this stage the relative heights of the corners and the
          centre are in the h array, and the corresponding coordinates are
          in the xh and yh arrays. The centre of the box is indexed by 0
          and the 4 corners by 1 to 4 as shown below.
          Each triangle is then indexed by the parameter m, and the 3
          vertices of each triangle are indexed by parameters m1,m2,and m3.
          It is assumed that the centre of the box is always vertex 2
          though this is important only when all 3 vertices lie exactly on
          the same contour level, in which case only the side of the box
          is drawn.
          vertex 4 +-------------------+ vertex 3
          | \               / |
          |   \    m=3    /   |
          |     \       /     |
          |       \   /       |
          |  m=2    X   m=2   |       the centre is vertex 0
          |       /   \       |
          |     /       \     |
          |   /    m=1    \   |
          | /               \ |
          vertex 1 +-------------------+ vertex 2
          */
          /* Scan each triangle in the box */
          for( size_t m=1; m <= 4; m++ ) {
            size_t m1 = m;
            size_t m2 = 0, m3;
            if( m != 4 )
              m3 = m + 1;
            else
              m3 = 1;
            size_t case_value = castab[sh[m1]+1][sh[m2]+1][sh[m3]+1];
            if( case_value == 0 )
              continue;
            Float x1=0, x2=0, y1=0, y2=0;
            switch( case_value ) {
               case 1: /* Line between vertices 1 and 2 */
                 x1 = xh[m1];  y1 = yh[m1];
                 x2 = xh[m2];  y2 = yh[m2];
                 break;
               case 2: /* Line between vertices 2 and 3 */
                 x1 = xh[m2];  y1 = yh[m2];
                 x2 = xh[m3];  y2 = yh[m3];
                 break;
               case 3: /* Line between vertices 3 and 1 */
                 x1 = xh[m3];  y1 = yh[m3];
                 x2 = xh[m1];  y2 = yh[m1];
                 break;
               case 4: /* Line between vertex 1 and side 2-3 */
                 x1 = xh[m1];  y1 = yh[m1];
                 x2 = xsect(m2,m3);  y2 = ysect(m2,m3);
                 break;
               case 5: /* Line between vertex 2 and side 3-1 */
                 x1 = xh[m2];  y1 = yh[m2];
                 x2 = xsect(m3,m1);  y2 = ysect(m3,m1);
                 break;
               case 6: /* Line between vertex 3 and side 1-2 */
                 x1 = xh[m3];  y1 = yh[m3];
                 x2 = xsect(m3,m2);  y2 = ysect(m3,m2);
                 break;
               case 7: /* Line between sides 1-2 and 2-3 */
                 x1 = xsect(m1,m2);  y1 = ysect(m1,m2);
                 x2 = xsect(m2,m3);  y2 = ysect(m2,m3);
                 break;
               case 8: /* Line between sides 2-3 and 3-1 */
                 x1 = xsect(m2,m3);  y1 = ysect(m2,m3);
                 x2 = xsect(m3,m1);  y2 = ysect(m3,m1);
                 break;
               case 9: /* Line between sides 3-1 and 1-2 */
                 x1 = xsect(m3,m1);  y1 = ysect(m3,m1);
                 x2 = xsect(m1,m2);  y2 = ysect(m1,m2);
                 break;
               default:
                 break;
            }
            /* Finally draw the line */
            callback.OnLine(x1, y1, x2, y2, z[k]);
          } /* m */
        } /* k - contour */
      } /* i */
    } /* j */
  }
};
