#ifndef __olx_glutil_H
#define __olx_glutil_H

#include "glbase.h"
#include "threex3.h"
#include "talist.h"

BeginGlNamespace()

struct GlTriangle {
  vec3i verts;
};

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
  void Generate(float rad, int ext, TTypeList<vec3f>& vo, TTypeList<GlTriangle>& to, TArrayList<vec3f>& normals) const {
    int nc = 8;
    for( int i=0; i < ext; i++ )
      nc += nc*3;
    
    to.SetCapacity( nc+1 );
    vo.SetCapacity( nc-1 );
    for( int i=0; i < 6; i++ )
      vo.AddNew( v_oh[i] );
    for( int i=0; i < 8; i++ )
      to.AddNew( t_oh[i] );
    for( int i=0; i < ext; i++ ) {
      const int t_cnt = to.Count();
      for( int j=0; j < t_cnt; j++ )  {
        GlTriangle& t = to[j];
        vo.AddNew( (vo[t.verts[0]]+vo[t.verts[1]])/2 );
        vo.AddNew( (vo[t.verts[0]]+vo[t.verts[2]])/2 );
        vo.AddNew( (vo[t.verts[1]]+vo[t.verts[2]])/2 );
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
    // normalise the vertices
    for( int i=0; i < vo.Count(); i++ )
      vo[i].Normalise();
    // initialise normals
    normals.SetCount(vo.Count());
    for( int i=0; i < vo.Count(); i++ )  {
      normals[i] = vo[i];
      vo[i] *= rad;
    }
  }
  void Render(float rad, int ext) const {
    TTypeList<vec3f> vecs;
    TTypeList<GlTriangle> triags;
    TArrayList<vec3f> norms;
    Generate(rad, ext, vecs, triags, norms);
    const int tc = triags.Count();
    glBegin(GL_TRIANGLES);
    for( int i = 0; i < tc; i++ )  {
      const GlTriangle& t = triags[i];
      for( int j=0; j < 3; j++ )  {
        glNormal3f( norms[t.verts[j]][0], norms[t.verts[j]][1], norms[t.verts[j]][2]); 
        glVertex3f( vecs[t.verts[j]][0], vecs[t.verts[j]][1], vecs[t.verts[j]][2]); 
      }
    }
    glEnd();
  }
  void Render(const TTypeList<vec3f>& vecs, const TTypeList<GlTriangle>& triags, const TArrayList<vec3f>& norms) const {
    const int tc = triags.Count();
    glBegin(GL_TRIANGLES);
    for( int i = 0; i < tc; i++ )  {
      const GlTriangle& t = triags[i];
      for( int j=0; j < 3; j++ )  {
        glNormal3f( norms[t.verts[j]][0], norms[t.verts[j]][1], norms[t.verts[j]][2]); 
        glVertex3f( vecs[t.verts[j]][0], vecs[t.verts[j]][1], vecs[t.verts[j]][2]); 
      }
    }
    glEnd();
  }
  void RenderEx(float rad, int ext, const vec3f& f_mask, const vec3f& t_mask) const {
    TTypeList<vec3f> vecs;
    TTypeList<GlTriangle> triags;
    TArrayList<vec3f> norms;
    Generate(rad, ext, vecs, triags, norms);
    const int tc = triags.Count();
    glBegin(GL_TRIANGLES);
    for( int i = 0; i < tc; i++ )  {
      const GlTriangle& t = triags[i];
      if( vec3f::IsInRangeInc(vecs[t.verts[0]], f_mask, t_mask) &&
          vec3f::IsInRangeInc(vecs[t.verts[1]], f_mask, t_mask) &&
          vec3f::IsInRangeInc(vecs[t.verts[2]], f_mask, t_mask) )
         continue;
      for( int j=0; j < 3; j++ )  {
        glNormal3f( norms[t.verts[j]][0], norms[t.verts[j]][1], norms[t.verts[j]][2]); 
        glVertex3f( vecs[t.verts[j]][0], vecs[t.verts[j]][1], vecs[t.verts[j]][2]); 
      }
    }
    glEnd();
  }
  void RenderEx(const TTypeList<vec3f>& vecs, const TTypeList<GlTriangle>& triags, const TArrayList<vec3f>& norms, 
      const vec3f& f_mask, const vec3f& t_mask) const {
    const int tc = triags.Count();
    glBegin(GL_TRIANGLES);
    for( int i = 0; i < tc; i++ )  {
      const GlTriangle& t = triags[i];
      if( vec3f::IsInRangeInc(vecs[t.verts[0]], f_mask, t_mask) &&
          vec3f::IsInRangeInc(vecs[t.verts[1]], f_mask, t_mask) &&
          vec3f::IsInRangeInc(vecs[t.verts[2]], f_mask, t_mask) )
         continue;
      for( int j=0; j < 3; j++ )  {
        glNormal3f( norms[t.verts[j]][0], norms[t.verts[j]][1], norms[t.verts[j]][2]); 
        glVertex3f( vecs[t.verts[j]][0], vecs[t.verts[j]][1], vecs[t.verts[j]][2]); 
      }
    }
    glEnd();
  }
  void RenderDisks(float rad, int t, float disk_s)  {
    vec3f sf(rad, 0, 0);
    double sa, sma, ca, cma;
    SinCos(2*M_PI/t, &sa, &ca);
    SinCos(-2*M_PI/t, &sma, &cma);
    glBegin(GL_POLYGON);
    glNormal3f(0, 0, -1);
    for( int i=0; i <= t; i++ )  {
      glVertex3f(sf[0], sf[1], -disk_s/2);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
    }
    glEnd();
    sf = vec3f(rad, 0, 0);
    glBegin(GL_POLYGON);
    glNormal3f(0, 0, 1);
    for( int i=0; i <= t; i++ )  {
      glVertex3f(sf[0], sf[1], disk_s/2);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*cma + sf[1]*sma);
      sf[1] = (float)(sf[1]*cma - x*sma);
    }
    glEnd();

    sf = vec3f(rad, 0, 0);
    glBegin(GL_POLYGON);
    glNormal3f(0, -1, 0);
    for( int i=0; i <= t; i++ )  {
      glVertex3f(sf[1], -disk_s/2, sf[0]);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
    }
    glEnd();
    sf = vec3f(rad, 0, 0);
    glBegin(GL_POLYGON);
    glNormal3f(0, 1, 0);
    for( int i=0; i <= t; i++ )  {
      glVertex3f(sf[1], disk_s/2, sf[0]);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*cma + sf[1]*sma);
      sf[1] = (float)(sf[1]*cma - x*sma);
    }
    glEnd();

    sf = vec3f(rad, 0, 0);
    glBegin(GL_POLYGON);
    glNormal3f(-1, 0, 0);
    for( int i=0; i <= t; i++ )  {
      glVertex3f(-disk_s/2, sf[0], sf[1]);
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
    }
    glEnd();
    sf = vec3f(rad, 0, 0);
    glBegin(GL_POLYGON);
    glNormal3f(1, 0, 0);
    for( int i=0; i <= t; i++ )  {
      glVertex3f(disk_s/2, sf[0], sf[1]);
      float x = sf[0];
      sf[0] = (float)(x*cma + sf[1]*sma);
      sf[1] = (float)(sf[1]*cma - x*sma);
    }
    glEnd();
  }
  void RenderRims(float rad, int t, float disk_s)  {
    vec3f sf(rad, 0, 0), pv;
    double sa, ca;
    SinCos(2*M_PI/t, &sa, &ca);
    glBegin(GL_QUADS);
    for( int i=0; i <= t; i++ )  {
      glNormal3f(sf[0], sf[1], 0);
      glVertex3f(sf[0], sf[1], -disk_s/2);
      glVertex3f(sf[0], sf[1], +disk_s/2);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
      glNormal3f(sf[0], sf[1], 0);
      glVertex3f(sf[0], sf[1], +disk_s/2);
      glVertex3f(sf[0], sf[1], -disk_s/2);
    }
    glEnd();

    sf = vec3f(rad, 0, 0);
    glBegin(GL_QUADS);
    for( int i=0; i <= t; i++ )  {
      glNormal3f(sf[1], 0, sf[0]);
      glVertex3f(sf[1], -disk_s/2, sf[0]);
      glVertex3f(sf[1], +disk_s/2, sf[0]);
      if( (i+1) > t )  break;
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
      glNormal3f(sf[1], 0, sf[0]);
      glVertex3f(sf[1], +disk_s/2, sf[0]);
      glVertex3f(sf[1], -disk_s/2, sf[0]);
    }
    glEnd();

    sf = vec3f(rad, 0, 0);
    glBegin(GL_QUADS);
    for( int i=0; i <= t; i++ )  {
      glNormal3f(0, sf[0], sf[1]);
      glVertex3f(-disk_s/2, sf[0], sf[1]);
      glVertex3f(+disk_s/2, sf[0], sf[1]);
      float x = sf[0];
      sf[0] = (float)(x*ca + sf[1]*sa);
      sf[1] = (float)(sf[1]*ca - x*sa);
      glNormal3f(0, sf[0], sf[1]);
      glVertex3f(+disk_s/2, sf[0], sf[1]);
      glVertex3f(-disk_s/2, sf[0], sf[1]);
    }
    glEnd();
  }
};

EndGlNamespace()
#endif
