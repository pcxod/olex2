/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_glutil_H
#define __olx_glutil_H
#include "glalg.h"
#include "bapp.h"
#include "sortedlist.h"
BeginGlNamespace()

template <typename float_type, class FaceProvider> class GlSphereEx :
  public OlxSphere<float_type,FaceProvider>
{
  typedef TVector3<float_type> vec_t;
public:
  static void Render(float_type rad, size_t ext)  {
    TTypeList<vec_t> vecs;
    TTypeList<IndexTriangle> triags;
    TArrayList<vec_t> norms;
    OlxSphere<float_type,FaceProvider>::Generate(rad, ext, vecs, triags, norms);
    Render(vecs, triags, norms);
  }
  static void Render(const TTypeList<vec_t>& vecs,
    const TTypeList<IndexTriangle>& triags,
    const TArrayList<vec_t>& norms)
  {
    const size_t tc = triags.Count();
    olx_gl::begin(GL_TRIANGLES);
    for( size_t i = 0; i < tc; i++ )  {
      const IndexTriangle& t = triags[i];
      for( size_t j=0; j < 3; j++ )  {
        olx_gl::normal(norms[t.vertices[j]]);
        olx_gl::vertex(vecs[t.vertices[j]]);
      }
    }
    olx_gl::end();
  }
  static void RenderEx(float_type rad, size_t ext,
    const vec_t& f_mask, const vec_t& t_mask)
  {
    TTypeList<vec_t> vecs;
    TTypeList<IndexTriangle> triags;
    TArrayList<vec_t> norms;
    OlxSphere<float_type,FaceProvider>::Generate(rad, ext, vecs, triags, norms);
    RenderEx(vecs, triags, norms, f_mask, t_mask);
  }
  static void RenderEx(const TTypeList<vec_t>& vecs,
    const TTypeList<IndexTriangle>& triags,
    const TArrayList<vec_t>& norms,
    const vec_t& f_mask, const vec_t& t_mask)
  {
    const size_t tc = triags.Count();
    olx_gl::begin(GL_TRIANGLES);
    for( size_t i = 0; i < tc; i++ )  {
      const IndexTriangle& t = triags[i];
      if( vec_t::IsInRangeInc(vecs[t.vertices[0]], f_mask, t_mask) &&
          vec_t::IsInRangeInc(vecs[t.vertices[1]], f_mask, t_mask) &&
          vec_t::IsInRangeInc(vecs[t.vertices[2]], f_mask, t_mask) )
         continue;
      for( int j=0; j < 3; j++ )  {
        olx_gl::normal(norms[t.vertices[j]]);
        olx_gl::vertex(vecs[t.vertices[j]]);
      }
    }
    olx_gl::end();
  }
  void RenderDisks(float_type rad, size_t t, float_type disk_s)  {
    vec_t sf(rad, 0, 0);
    float_type sa, sma, ca, cma;
    olx_sincos(2*M_PI/t, &sa, &ca);
    olx_sincos(-2*M_PI/t, &sma, &cma);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(0, 0, -1);
    for( int i=0; i <= t; i++ )  {
      olx_gl::vertex(sf[0], sf[1], -disk_s/2);
      if( (i+1) > t )  break;
      float_type x = sf[0];
      sf[0] = (x*ca + sf[1]*sa);
      sf[1] = (sf[1]*ca - x*sa);
    }
    olx_gl::end();
    sf = vec_t(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(0, 0, 1);
    for( size_t i=0; i <= t; i++ )  {
      olx_gl::vertex(sf[0], sf[1], disk_s/2);
      if( (i+1) > t )  break;
      float_type x = sf[0];
      sf[0] = (x*cma + sf[1]*sma);
      sf[1] = (sf[1]*cma - x*sma);
    }
    olx_gl::end();

    sf = vec_t(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(0, -1, 0);
    for( size_t i=0; i <= t; i++ )  {
      olx_gl::vertex(sf[1], -disk_s/2, sf[0]);
      if( (i+1) > t )  break;
      float_type x = sf[0];
      sf[0] = (x*ca + sf[1]*sa);
      sf[1] = (sf[1]*ca - x*sa);
    }
    olx_gl::end();
    sf = vec_t(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(0, 1, 0);
    for( size_t i=0; i <= t; i++ )  {
      olx_gl::vertex(sf[1], disk_s/2, sf[0]);
      if( (i+1) > t )  break;
      float_type x = sf[0];
      sf[0] = (x*cma + sf[1]*sma);
      sf[1] = (sf[1]*cma - x*sma);
    }
    olx_gl::end();

    sf = vec_t(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(-1, 0, 0);
    for( size_t i=0; i <= t; i++ )  {
      olx_gl::vertex(-disk_s/2, sf[0], sf[1]);
      float_type x = sf[0];
      sf[0] = (x*ca + sf[1]*sa);
      sf[1] = (sf[1]*ca - x*sa);
    }
    olx_gl::end();
    sf = vec_t(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(1, 0, 0);
    for( size_t i=0; i <= t; i++ )  {
      olx_gl::vertex(disk_s/2, sf[0], sf[1]);
      float_type x = sf[0];
      sf[0] = (x*cma + sf[1]*sma);
      sf[1] = (sf[1]*cma - x*sma);
    }
    olx_gl::end();
  }
  void RenderRims(float_type rad, size_t t, float_type disk_s)  {
    vec_t sf(rad, 0, 0), pv;
    float_type sa, ca;
    olx_sincos(2*M_PI/t, &sa, &ca);
    olx_gl::begin(GL_QUADS);
    for( size_t i=0; i <= t; i++ )  {
      olx_gl::normal(sf[0], sf[1], 0.0f);
      olx_gl::vertex(sf[0], sf[1], -disk_s/2);
      olx_gl::vertex(sf[0], sf[1], +disk_s/2);
      if( (i+1) > t )  break;
      float_type x = sf[0];
      sf[0] = (x*ca + sf[1]*sa);
      sf[1] = (sf[1]*ca - x*sa);
      olx_gl::normal(sf[0], sf[1], 0.0f);
      olx_gl::vertex(sf[0], sf[1], +disk_s/2);
      olx_gl::vertex(sf[0], sf[1], -disk_s/2);
    }
    olx_gl::end();

    sf = vec_t(rad, 0, 0);
    olx_gl::begin(GL_QUADS);
    for( size_t i=0; i <= t; i++ )  {
      olx_gl::normal(sf[1], 0.0f, sf[0]);
      olx_gl::vertex(sf[1], -disk_s/2, sf[0]);
      olx_gl::vertex(sf[1], +disk_s/2, sf[0]);
      if( (i+1) > t )  break;
      float_type x = sf[0];
      sf[0] = (x*ca + sf[1]*sa);
      sf[1] = (sf[1]*ca - x*sa);
      olx_gl::normal(sf[1], 0.0f, sf[0]);
      olx_gl::vertex(sf[1], +disk_s/2, sf[0]);
      olx_gl::vertex(sf[1], -disk_s/2, sf[0]);
    }
    olx_gl::end();

    sf = vec_t(rad, 0, 0);
    olx_gl::begin(GL_QUADS);
    for( size_t i=0; i <= t; i++ )  {
      olx_gl::normal(0.0f, sf[0], sf[1]);
      olx_gl::vertex(-disk_s/2, sf[0], sf[1]);
      olx_gl::vertex(+disk_s/2, sf[0], sf[1]);
      float_type x = sf[0];
      sf[0] = (x*ca + sf[1]*sa);
      sf[1] = (sf[1]*ca - x*sa);
      olx_gl::normal(0.0f, sf[0], sf[1]);
      olx_gl::vertex(+disk_s/2, sf[0], sf[1]);
      olx_gl::vertex(-disk_s/2, sf[0], sf[1]);
    }
    olx_gl::end();
  }
};

struct GlTorus {
  //reference: www.opengl.org/resources/code/samples/redbook/torus.c
  // tr - tube radius, r - torus radius
  static void Render(double tr, double r, size_t sc, size_t tsc)  {
    static const double tpi = 2*M_PI;
    for( size_t i = 0; i < sc; i++ ) {
      olx_gl::begin(GL_QUAD_STRIP);
      for( size_t j = 0; j <= tsc; j++ ) {
        for( int k = 1; k >= 0; k-- ) {
          double s = (i + k) % sc + 0.5;
          double t = static_cast<double>(j % tsc);
          double a = cos(s*tpi/sc),
            b = cos(t*tpi/tsc),
            c = sin(t*tpi/tsc),
            d = sin(s*tpi/sc);
          olx_gl::normal(a*b, a*c, d);
          olx_gl::vertex((r+tr*a)*b, (r+tr*a)*c, tr*d);
        }
      }
      olx_gl::end();
    }
  }
};
EndGlNamespace()
#endif
