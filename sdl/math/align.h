// (c) O. Dolomanov, 2010
#ifndef __olx_sdl_align_H
#define __olx_sdl_align_H
#include "../threex3.h"
#include "../ematrix.h"
BeginEsdlNamespace()

namespace align  {

  struct out  {
    // a 4x4 matrix of the quaternions
    ematd quaternions;
    /* root mean square distances, note that if non-unit weights are used, these values
    will differe from the RMSD calculated directly:
    (sum(distance^2*w^2)/sum(w^2)^0.5 vs. (sum(distance^2)/n)^0.5
    */
    evecd rmsd;
    vec3d center_a, center_b;
    out() : quaternions(4,4), rmsd(4)  {}
  };
  struct point  {
    vec3d value;
    double weight;
    point() : weight(1.0)  {}
    point(const vec3d& p, double w=1.0) : value(p), weight(w)  {}
    point(const point& ap) : value(ap.value), weight(ap.weight)  {}
    point& operator = (const point& ap)  {
      value = ap.value;
      weight = ap.weight;
      return *this;
    }
    vec3d GetWeightedValue() const {  return value*weight;  }
  };
  struct pair  {
    point a, b;
    pair()  {}
    pair(const pair& p) : a(p.a), b(p.b)  {}
    pair(const point& _a, const point& _b) : a(_a), b(_b)  {}
    pair& operator = (const pair& p)  {
      a = p.a;
      b = p.b;
      return *this;
    }
  };
  /* finds allignment quaternions for given coordinates and their weights
  the rmsds (and the quaternions) are sorted ascending
  Acta A45 (1989), 208.
  The resulting matrices map {b} set to {a} */
  template <class List>
  out FindAlignmentQuaternions(const List& pairs)  {
    out ao;
    double swa = 0, swb = 0, sws = 0;
    for( size_t i=0; i < pairs.Count(); i++ )  {
      ao.center_a += pairs[i].a.GetWeightedValue();
      ao.center_b += pairs[i].b.GetWeightedValue();
      swa += pairs[i].a.weight;
      swb += pairs[i].b.weight;
      sws += pairs[i].a.weight*pairs[i].b.weight;
    }
    ao.center_a /= swa;
    ao.center_b /= swb;
    ematd evm(4,4);
    for( size_t i=0; i < pairs.Count(); i++ )  {
      //const vec3d v1 = (pairs[i].a.value - ao.center_a)*pairs[i].a.weight;
      //const vec3d v2 = (pairs[i].b.value - ao.center_b)*pairs[i].b.weight;
      //const vec3d p = v1+v2;
      //const vec3d m = v1-v2;
      //evm[0][0] += (m[0]*m[0] + m[1]*m[1] + m[2]*m[2]);
      //evm[0][1] += (p[1]*m[2] - m[1]*p[2]);
      //evm[0][2] += (m[0]*p[2] - p[0]*m[2]);
      //evm[0][3] += (p[0]*m[1] - m[0]*p[1]);
      //evm[1][1] += (p[1]*p[1] + p[2]*p[2] + m[0]*m[0]);
      //evm[1][2] += (m[0]*m[1] - p[0]*p[1]);
      //evm[1][3] += (m[0]*m[2] - p[0]*p[2]);
      //evm[2][2] += (p[0]*p[0] + p[2]*p[2] + m[1]*m[1]);
      //evm[2][3] += (m[1]*m[2] - p[1]*p[2]);
      //evm[3][3] += (p[0]*p[0] + p[1]*p[1] + m[2]*m[2]);
      vec3d v1 = pairs[i].a.value - ao.center_a;
      vec3d v2 = pairs[i].b.value - ao.center_b;
      const double 
        xm = v1[0] - v2[0],
        xp = v1[0] + v2[0],
        yp = v1[1] + v2[1],
        ym = v1[1] - v2[1],
        zm = v1[2] - v2[2],
        zp = v1[2] + v2[2];
      evm[0][0] += (xm*xm + ym*ym + zm*zm);
      evm[0][1] += (yp*zm - ym*zp);
      evm[0][2] += (xm*zp - xp*zm);
      evm[0][3] += (xp*ym - xm*yp);
      evm[1][0] = evm[0][1];
      evm[1][1] += (yp*yp + zp*zp + xm*xm);
      evm[1][2] += (xm*ym - xp*yp);
      evm[1][3] += (xm*zm - xp*zp);
      evm[2][0] = evm[0][2];
      evm[2][1] = evm[1][2];
      evm[2][2] += (xp*xp + zp*zp + ym*ym);
      evm[2][3] += (ym*zm - yp*zp);
      evm[3][0] = evm[0][3];
      evm[3][1] = evm[1][3];
      evm[3][2] = evm[2][3];
      evm[3][3] += (xp*xp + yp*yp + zm*zm);
    }
    //evm[1][0] = evm[0][1];
    //evm[2][0] = evm[0][2];
    //evm[2][1] = evm[1][2];
    //evm[3][0] = evm[0][3];
    //evm[3][1] = evm[1][3];
    //evm[3][2] = evm[2][3];
    ematd::EigenValues(evm /= sws, ao.quaternions.I());
    for( int i=0; i < 4; i++ )
      ao.rmsd[i] = (evm[i][i] <= 0 ? 0 : sqrt(evm[i][i]));
    bool changes = true;
    while( changes )  {
      changes = false;
      for( int i=0; i < 3; i++ )  {
        if( ao.rmsd[i+1] < ao.rmsd[i] )  {
          ao.quaternions.SwapRows(i, i+1);
          olx_swap(ao.rmsd[i], ao.rmsd[i+1]);
          changes = true;
        }
      }
    }
    return ao;
  }
};  // end namespace align
EndEsdlNamespace()
#endif
