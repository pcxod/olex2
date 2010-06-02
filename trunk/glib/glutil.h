/* Graphics extensions, (c) Oleg Dolomanov 2009 */
#ifndef __olx_glutil_H
#define __olx_glutil_H
#include "esphere.h"

BeginGlNamespace()

template <typename float_type, class FaceProvider> class GlSphereEx :
  public OlxSphere<float_type,FaceProvider>
{
public:
  static void Render(float_type rad, size_t ext)  {
    TTypeList<TVector3<float_type> > vecs;
    TTypeList<IndexTriangle> triags;
    TArrayList<TVector3<float_type> > norms;
    OlxSphere<float_type,FaceProvider>::Generate(rad, ext, vecs, triags, norms);
    Render(vecs, triags, norms);
  }
  static void Render(const TTypeList<TVector3<float_type> >& vecs,
    const TTypeList<IndexTriangle>& triags,
    const TArrayList<TVector3<float_type> >& norms)
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
    const TVector3<float_type>& f_mask, const TVector3<float_type>& t_mask)
  {
    TTypeList<TVector3<float_type> > vecs;
    TTypeList<IndexTriangle> triags;
    TArrayList<TVector3<float_type> > norms;
    OlxSphere<float_type,FaceProvider>::Generate(rad, ext, vecs, triags, norms);
    RenderEx(vecs, triags, norms, f_mask, t_mask);
  }
  static void RenderEx(const TTypeList<TVector3<float_type> >& vecs,
    const TTypeList<IndexTriangle>& triags,
    const TArrayList<TVector3<float_type> >& norms, 
    const vec3f& f_mask, const vec3f& t_mask)
  {
    const size_t tc = triags.Count();
    olx_gl::begin(GL_TRIANGLES);
    for( size_t i = 0; i < tc; i++ )  {
      const IndexTriangle& t = triags[i];
      if( vec3f::IsInRangeInc(vecs[t.vertices[0]], f_mask, t_mask) &&
          vec3f::IsInRangeInc(vecs[t.vertices[1]], f_mask, t_mask) &&
          vec3f::IsInRangeInc(vecs[t.vertices[2]], f_mask, t_mask) )
         continue;
      for( int j=0; j < 3; j++ )  {
        olx_gl::normal(norms[t.vertices[j]]);
        olx_gl::vertex(vecs[t.vertices[j]]);
      }
    }
    olx_gl::end();
  }
  void RenderDisks(float_type rad, size_t t, float_type disk_s)  {
    vec3f sf(rad, 0, 0);
    float_type sa, sma, ca, cma;
    SinCos(2*M_PI/t, &sa, &ca);
    SinCos(-2*M_PI/t, &sma, &cma);
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
    sf = vec3f(rad, 0, 0);
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

    sf = vec3f(rad, 0, 0);
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
    sf = vec3f(rad, 0, 0);
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

    sf = vec3f(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(-1, 0, 0);
    for( size_t i=0; i <= t; i++ )  {
      olx_gl::vertex(-disk_s/2, sf[0], sf[1]);
      float_type x = sf[0];
      sf[0] = (x*ca + sf[1]*sa);
      sf[1] = (sf[1]*ca - x*sa);
    }
    olx_gl::end();
    sf = vec3f(rad, 0, 0);
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
    vec3f sf(rad, 0, 0), pv;
    float_type sa, ca;
    SinCos(2*M_PI/t, &sa, &ca);
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

    sf = vec3f(rad, 0, 0);
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

    sf = vec3f(rad, 0, 0);
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
EndGlNamespace()
#endif
