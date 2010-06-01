/* Graphics extensions, (c) Oleg Dolomanov 2009 */
#ifndef __olx_glutil_H
#define __olx_glutil_H
#include "threex3.h"
#include "talist.h"

BeginGlNamespace()

struct GlTriangle {  TVector3<size_t> verts;  };
/* a class to build sphere by partitioning an octahedron. Alows custom masks to be aplied when
rendering the sphere in OpenGL or other rendering engine. All methods and members can be made static... */
class GlSphereEx {
  vec3d v_oh[6];
  GlTriangle t_oh[8];
public:
  GlSphereEx() {
    v_oh[0][2] = 1;  // vec3f(0, 0, 1);
    v_oh[1][0] = 1;  // vec3f(1, 0, 0);
    v_oh[2][1] = 1;  // vec3f(0, 1, 0);
    v_oh[3][0] = -1; // vec3f(-1, 0, 0);
    v_oh[4][1] = -1; // vec3f(0, -1, 0);
    v_oh[5][2] = -1; // vec3f(0, 0, -1);
    t_oh[0].verts = vec3i(0, 1, 2);
    t_oh[1].verts = vec3i(0, 2, 3);
    t_oh[2].verts = vec3i(0, 3, 4);
    t_oh[3].verts = vec3i(0, 4, 1);
    t_oh[4].verts = vec3i(5, 2, 1);
    t_oh[5].verts = vec3i(5, 3, 2);
    t_oh[6].verts = vec3i(5, 4, 3);
    t_oh[7].verts = vec3i(5, 1, 4);
  }
  /* rad - radius of the sphere, ext - the generation to generate, vo - vector output,
  to - triangles output, normals - the normals output (one a vertex, not triangle). Since
  there is a huge redundancy, a unique set of vecteces is generated and triangles
  contain inly indeces to the verteces they are based on */
  void Generate(float rad, size_t ext,
    TTypeList<vec3f>& vo, TTypeList<GlTriangle>& to, TArrayList<vec3f>& normals) const
  {
    Generate(rad, ext, vo, to, &normals);
  }
  void Generate(float rad, size_t ext,
    TTypeList<vec3f>& vo, TTypeList<GlTriangle>& to, TArrayList<vec3f>* _normals=NULL) const
  {
    size_t nc = 8;
    for( size_t i=0; i < ext; i++ )
      nc += nc*3;
    
    to.SetCapacity(nc+1);
    vo.SetCapacity(nc-1);
    for( int i=0; i < 6; i++ )
      vo.AddNew(v_oh[i]);
    for( int i=0; i < 8; i++ )
      to.AddNew(t_oh[i]);
    for( size_t i=0; i < ext; i++ ) {
      const size_t t_cnt = to.Count();
      for( size_t j=0; j < t_cnt; j++ )  {
        GlTriangle& t = to[j];
        vo.AddNew((vo[t.verts[0]]+vo[t.verts[1]])/2);
        vo.AddNew((vo[t.verts[0]]+vo[t.verts[2]])/2);
        vo.AddNew((vo[t.verts[1]]+vo[t.verts[2]])/2);
        // new triangle
        GlTriangle& nt1 = to.AddNew();
        nt1.verts[0] = vo.Count()-3;  //12
        nt1.verts[1] = vo.Count()-1;  // 23
        nt1.verts[2] = vo.Count()-2;  // 13
        // new tringles partially based on old vertices
        GlTriangle& nt2 = to.AddNew();
        nt2.verts[0] = t.verts[1];
        nt2.verts[1] = vo.Count()-1;  // 23
        nt2.verts[2] = vo.Count()-3;  // 12
        // new tringles partially based on old vertices
        GlTriangle& nt3 = to.AddNew();
        nt3.verts[0] = t.verts[2];
        nt3.verts[1] = vo.Count()-2;  // 13
        nt3.verts[2] = vo.Count()-1;  // 23
        // modify the original triangle to take two new points
        t.verts[1] = vo.Count()-3;  // 12
        t.verts[2] = vo.Count()-2;  // 13
      }
    }
    if( _normals != NULL )  {
      TArrayList<vec3f>& normals = *_normals;
      for( size_t i=0; i < vo.Count(); i++ )  // normalise the vertices
        vo[i].Normalise();
      // initialise normals
      normals.SetCount(vo.Count());  
      for( size_t i=0; i < vo.Count(); i++ )  {
        normals[i] = vo[i];
        vo[i] *= rad;
      }
    }
    else  {
      for( size_t i=0; i < vo.Count(); i++ )
        vo[i].NormaliseTo(rad);
    }
  }
  void Render(float rad, int ext) const {
    TTypeList<vec3f> vecs;
    TTypeList<GlTriangle> triags;
    TArrayList<vec3f> norms;
    Generate(rad, ext, vecs, triags, norms);
    const size_t tc = triags.Count();
    olx_gl::begin(GL_TRIANGLES);
    for( size_t i = 0; i < tc; i++ )  {
      const GlTriangle& t = triags[i];
      for( size_t j=0; j < 3; j++ )  {
        olx_gl::normal(norms[t.verts[j]]); 
        olx_gl::vertex(vecs[t.verts[j]]); 
      }
    }
    olx_gl::end();
  }
  void Render(const TTypeList<vec3f>& vecs, const TTypeList<GlTriangle>& triags, const TArrayList<vec3f>& norms) const {
    const size_t tc = triags.Count();
    olx_gl::begin(GL_TRIANGLES);
    for( size_t i = 0; i < tc; i++ )  {
      const GlTriangle& t = triags[i];
      for( size_t j=0; j < 3; j++ )  {
        olx_gl::normal(norms[t.verts[j]]);
        olx_gl::vertex(vecs[t.verts[j]]);
      }
    }
    olx_gl::end();
  }
  void RenderEx(float rad, int ext, const vec3f& f_mask, const vec3f& t_mask) const {
    TTypeList<vec3f> vecs;
    TTypeList<GlTriangle> triags;
    TArrayList<vec3f> norms;
    Generate(rad, ext, vecs, triags, norms);
    const size_t tc = triags.Count();
    olx_gl::begin(GL_TRIANGLES);
    for( size_t i = 0; i < tc; i++ )  {
      const GlTriangle& t = triags[i];
      if( vec3f::IsInRangeInc(vecs[t.verts[0]], f_mask, t_mask) &&
          vec3f::IsInRangeInc(vecs[t.verts[1]], f_mask, t_mask) &&
          vec3f::IsInRangeInc(vecs[t.verts[2]], f_mask, t_mask) )
         continue;
      for( size_t j=0; j < 3; j++ )  {
        olx_gl::normal(norms[t.verts[j]]);
        olx_gl::vertex(vecs[t.verts[j]]); 
      }
    }
    olx_gl::end();
  }
  void RenderEx(const TTypeList<vec3f>& vecs, const TTypeList<GlTriangle>& triags, const TArrayList<vec3f>& norms, 
      const vec3f& f_mask, const vec3f& t_mask) const {
    const size_t tc = triags.Count();
    olx_gl::begin(GL_TRIANGLES);
    for( size_t i = 0; i < tc; i++ )  {
      const GlTriangle& t = triags[i];
      if( vec3f::IsInRangeInc(vecs[t.verts[0]], f_mask, t_mask) &&
          vec3f::IsInRangeInc(vecs[t.verts[1]], f_mask, t_mask) &&
          vec3f::IsInRangeInc(vecs[t.verts[2]], f_mask, t_mask) )
         continue;
      for( int j=0; j < 3; j++ )  {
        olx_gl::normal(norms[t.verts[j]]);
        olx_gl::vertex(vecs[t.verts[j]]);
      }
    }
    olx_gl::end();
  }
  void RenderDisks(float rad, int t, float disk_s)  {
    vec3f sf(rad, 0, 0);
    double sa, sma, ca, cma;
    SinCos(2*M_PI/t, &sa, &ca);
    SinCos(-2*M_PI/t, &sma, &cma);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(0, 0, -1);
    for( int i=0; i <= t; i++ )  {
      olx_gl::vertex(sf[0], sf[1], -disk_s/2);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
    }
    olx_gl::end();
    sf = vec3f(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(0, 0, 1);
    for( int i=0; i <= t; i++ )  {
      olx_gl::vertex(sf[0], sf[1], disk_s/2);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*cma + sf[1]*sma);
      sf[1] = (float)(sf[1]*cma - x*sma);
    }
    olx_gl::end();

    sf = vec3f(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(0, -1, 0);
    for( int i=0; i <= t; i++ )  {
      olx_gl::vertex(sf[1], -disk_s/2, sf[0]);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
    }
    olx_gl::end();
    sf = vec3f(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(0, 1, 0);
    for( int i=0; i <= t; i++ )  {
      olx_gl::vertex(sf[1], disk_s/2, sf[0]);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*cma + sf[1]*sma);
      sf[1] = (float)(sf[1]*cma - x*sma);
    }
    olx_gl::end();

    sf = vec3f(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(-1, 0, 0);
    for( int i=0; i <= t; i++ )  {
      olx_gl::vertex(-disk_s/2, sf[0], sf[1]);
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
    }
    olx_gl::end();
    sf = vec3f(rad, 0, 0);
    olx_gl::begin(GL_POLYGON);
    olx_gl::normal(1, 0, 0);
    for( int i=0; i <= t; i++ )  {
      olx_gl::vertex(disk_s/2, sf[0], sf[1]);
      float x = sf[0];
      sf[0] = (float)(x*cma + sf[1]*sma);
      sf[1] = (float)(sf[1]*cma - x*sma);
    }
    olx_gl::end();
  }
  void RenderRims(float rad, int t, float disk_s)  {
    vec3f sf(rad, 0, 0), pv;
    double sa, ca;
    SinCos(2*M_PI/t, &sa, &ca);
    olx_gl::begin(GL_QUADS);
    for( int i=0; i <= t; i++ )  {
      olx_gl::normal(sf[0], sf[1], 0.0f);
      olx_gl::vertex(sf[0], sf[1], -disk_s/2);
      olx_gl::vertex(sf[0], sf[1], +disk_s/2);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
      olx_gl::normal(sf[0], sf[1], 0.0f);
      olx_gl::vertex(sf[0], sf[1], +disk_s/2);
      olx_gl::vertex(sf[0], sf[1], -disk_s/2);
    }
    olx_gl::end();

    sf = vec3f(rad, 0, 0);
    olx_gl::begin(GL_QUADS);
    for( int i=0; i <= t; i++ )  {
      olx_gl::normal(sf[1], 0.0f, sf[0]);
      olx_gl::vertex(sf[1], -disk_s/2, sf[0]);
      olx_gl::vertex(sf[1], +disk_s/2, sf[0]);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
      olx_gl::normal(sf[1], 0.0f, sf[0]);
      olx_gl::vertex(sf[1], +disk_s/2, sf[0]);
      olx_gl::vertex(sf[1], -disk_s/2, sf[0]);
    }
    olx_gl::end();

    sf = vec3f(rad, 0, 0);
    olx_gl::begin(GL_QUADS);
    for( int i=0; i <= t; i++ )  {
      olx_gl::normal(0.0f, sf[0], sf[1]);
      olx_gl::vertex(-disk_s/2, sf[0], sf[1]);
      olx_gl::vertex(+disk_s/2, sf[0], sf[1]);
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
      olx_gl::normal(0.0f, sf[0], sf[1]);
      olx_gl::vertex(+disk_s/2, sf[0], sf[1]);
      olx_gl::vertex(-disk_s/2, sf[0], sf[1]);
    }
    olx_gl::end();
  }
};
EndGlNamespace()
#endif
