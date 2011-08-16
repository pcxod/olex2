/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "nui.h"
#include "bapp.h"
#include "glrender.h"
#include "glbackground.h"
#include "glprimitive.h"
#include "styles.h"
#include "gxapp.h"

#if defined(__WIN32__) && !defined(_WIN64) && defined(__OLX_USE_NUI__)
using namespace olx_nui;

void Skeleton::Create(const olxstr& cName) {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  TGlMaterial GlM;
  GlM.SetFlags(0);   
  //GlM.ShininessF = 128;
  GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|
    sglmAmbientB|sglmDiffuseB|sglmSpecularB|sglmIdentityDraw);
  //GlM.SetFlags(sglmAmbientF|sglmDiffuseF);
  GlM.AmbientF = GlM.AmbientB = 0x800f0f0f;
  GlM.DiffuseF = GlM.DiffuseB = 0x800f0f0f;
  GlM.SpecularF = GlM.SpecularB = 0x808080;

  TGlPrimitive& GlP = GPC.NewPrimitive("Sphere", sgloSphere);
  GlP.SetProperties(GlM);
  //GlP.SetProperties(GS.GetMaterial(GlP.GetName(), GlM));
  GlP.Params[0] = 0.01;  GlP.Params[1] = 8;  GlP.Params[2] = 8;
  Compile();
}
/*****************************************************************************/
bool Skeleton::Orient(TGlPrimitive& P)  {
  if( points.Count() != NUI_SKELETON_COUNT )
    return true;
  static bool rendering = false, left_down = false, right_down = false;
  if( rendering )  return true;
  rendering = true;
  double scale = Parent.GetScale();
  vec3d vs1(Parent.GetWidth(), Parent.GetHeight(), 0);
  vec3d vs(-vs1/2);
  vs1[2] = 1;
  //int left_elbow_y = 0, right_elbow_y = 0;
  //if( points.Count() > NUI_SKELETON_POSITION_ELBOW_RIGHT )
  //  right_elbow_y = points[NUI_SKELETON_POSITION_ELBOW_RIGHT][1];
  //if( points.Count() > NUI_SKELETON_POSITION_ELBOW_LEFT )
  //  left_elbow_y = points[NUI_SKELETON_POSITION_ELBOW_LEFT][1];
  for( size_t si = 0; si < points.Count(); si++ )  {
    if( points[si].Count() != NUI_SKELETON_POSITION_COUNT )
      continue;
    vec3d_alist pts(points[si].Count());
    vec3d lh_p, rh_p, h_p;
    for( size_t i=0; i < points[si].Count(); i++ )  {
      pts[i] = (vs + points[si][i]*vs1)*scale;
      if( i == NUI_SKELETON_POSITION_HAND_RIGHT )
        rh_p = pts[i]*vs1;
      else  if( i == NUI_SKELETON_POSITION_HAND_LEFT )
        lh_p = pts[i]*vs1;
      else  if( i == NUI_SKELETON_POSITION_HEAD )
        h_p = pts[i]*vs1;
      pts[i][2] = 0;
    }
    lh_p[1] = vs[1] - lh_p[1];
    rh_p[1] = vs[1] - rh_p[1];
    h_p[1] = vs[1] - h_p[1];

    if( h_p[2]-rh_p[2] > 2 )  {
      if( !right_down )  {
        Parent.Background()->RB(0xff);
        right_down = true;
        TGXApp::GetInstance().MouseDown(rh_p[0], rh_p[1], 0, smbRight);
      }
      else  {
        TGXApp::GetInstance().MouseMove(rh_p[0], rh_p[1], 0);
      }
    }
    else if( right_down )  {
      Parent.Background()->RB(0xffffff);
      TGXApp::GetInstance().MouseUp(rh_p[0], rh_p[1], 0, smbRight);
      right_down = false;
    }
    if( h_p[2]-lh_p[2] > 2 )  {
      if( !left_down )  {
        Parent.Background()->LT(0xff);
        left_down = true;
        TGXApp::GetInstance().MouseDown(lh_p[0], lh_p[1], 0, smbLeft);
      }
      else  {
        TGXApp::GetInstance().MouseMove(lh_p[0], lh_p[1], 0);
      }
    }
    else if( left_down )  {
      Parent.Background()->LT(0xffffff);
      TGXApp::GetInstance().MouseUp(lh_p[0], lh_p[1], 0, smbLeft);
      left_down = false;
    }

    for( size_t i=0; i < points[si].Count(); i++ )  {
      olx_gl::translate(pts[i]);
      P.Draw();
      olx_gl::translate(-pts[i]);
    }
    try  {
      double head_r = (pts[NUI_SKELETON_POSITION_HEAD]-
        pts[NUI_SKELETON_POSITION_SHOULDER_CENTER]).Length()/2;
      const int head_sec = 25;
      vec3d head_v = pts[NUI_SKELETON_POSITION_HEAD];
      head_v[0] += head_r;
      olx_gl::begin(GL_TRIANGLE_FAN);
      olx_gl::vertex(pts[NUI_SKELETON_POSITION_HEAD]);
      for( int i=0; i <= head_sec; i++ )  {
        double ang = i*2*M_PI/head_sec;
        vec3d v(head_r*cos(ang), head_r*sin(ang), 0);
        olx_gl::vertex(pts[NUI_SKELETON_POSITION_HEAD]+v);
      }
      olx_gl::end();

      vec3d shoulder_line(pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT]-
        pts[NUI_SKELETON_POSITION_SHOULDER_LEFT]);
      vec3d hip_line(pts[NUI_SKELETON_POSITION_HIP_RIGHT]-
        pts[NUI_SKELETON_POSITION_HIP_LEFT]);
      vec3d spine_line(pts[NUI_SKELETON_POSITION_SPINE]-
        pts[NUI_SKELETON_POSITION_SHOULDER_CENTER]);
      vec3d right_shoulder_line(pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT]-
        pts[NUI_SKELETON_POSITION_ELBOW_RIGHT]);
      vec3d right_wrist_line(pts[NUI_SKELETON_POSITION_ELBOW_RIGHT]-
        pts[NUI_SKELETON_POSITION_WRIST_RIGHT]);
      vec3d left_shoulder_line(pts[NUI_SKELETON_POSITION_SHOULDER_LEFT]-
        pts[NUI_SKELETON_POSITION_ELBOW_LEFT]);
      vec3d left_wrist_line(pts[NUI_SKELETON_POSITION_ELBOW_LEFT]-
        pts[NUI_SKELETON_POSITION_WRIST_LEFT]);

      vec3d right_thight_line(pts[NUI_SKELETON_POSITION_HIP_RIGHT]-
        pts[NUI_SKELETON_POSITION_KNEE_RIGHT]);
      vec3d right_shin_line(pts[NUI_SKELETON_POSITION_KNEE_RIGHT]-
        pts[NUI_SKELETON_POSITION_ANKLE_RIGHT]);

      vec3d left_thight_line(pts[NUI_SKELETON_POSITION_HIP_LEFT]-
        pts[NUI_SKELETON_POSITION_KNEE_LEFT]);
      vec3d left_shin_line(pts[NUI_SKELETON_POSITION_KNEE_LEFT]-
        pts[NUI_SKELETON_POSITION_ANKLE_LEFT]);

      if( shoulder_line.QLength() > 0 &&
        hip_line.QLength() > 0 &&
        spine_line.QLength() > 0 )
      {
        olx_gl::begin(GL_TRIANGLES);
        olx_gl::vertex(pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT]);
        olx_gl::vertex(pts[NUI_SKELETON_POSITION_SHOULDER_CENTER]);
        olx_gl::vertex(pts[NUI_SKELETON_POSITION_SHOULDER_LEFT]);
        // neck
        olx_gl::vertex(pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT] +
          (pts[NUI_SKELETON_POSITION_SHOULDER_CENTER]-
          pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT])*0.75);
        olx_gl::vertex(pts[NUI_SKELETON_POSITION_HEAD]);
        olx_gl::vertex(pts[NUI_SKELETON_POSITION_SHOULDER_LEFT] +
          (pts[NUI_SKELETON_POSITION_SHOULDER_CENTER]-
          pts[NUI_SKELETON_POSITION_SHOULDER_LEFT])*0.75);
        olx_gl::end();

        vec3d half_spine_line_normal =
          (spine_line*mat3d(0,-1,0, 1,0,0, 0,0,1)).NormaliseTo(
          (shoulder_line.Length()+hip_line.Length())/8); // make a bit slimmer

        // body bottom
        DrawQuad(
          pts[NUI_SKELETON_POSITION_SPINE] - half_spine_line_normal,
          pts[NUI_SKELETON_POSITION_SPINE] + half_spine_line_normal,
          pts[NUI_SKELETON_POSITION_HIP_LEFT],
          pts[NUI_SKELETON_POSITION_HIP_RIGHT]
        );
        // body top
        DrawQuad(
          pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT],
          pts[NUI_SKELETON_POSITION_SHOULDER_LEFT],
          pts[NUI_SKELETON_POSITION_SPINE] + half_spine_line_normal,
          pts[NUI_SKELETON_POSITION_SPINE] - half_spine_line_normal
          );

        vec3d left_body_side = pts[NUI_SKELETON_POSITION_SPINE] +
          half_spine_line_normal - pts[NUI_SKELETON_POSITION_SHOULDER_LEFT];
        vec3d right_body_side = pts[NUI_SKELETON_POSITION_SPINE] -
          half_spine_line_normal - pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT];

        // right arm
        DrawLimp(right_shoulder_line, right_wrist_line, right_body_side/3,
          pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT],
          pts[NUI_SKELETON_POSITION_ELBOW_RIGHT],
          pts[NUI_SKELETON_POSITION_WRIST_RIGHT],
          true
          );
        // left arm
        DrawLimp(left_shoulder_line, left_wrist_line, left_body_side/3,
          pts[NUI_SKELETON_POSITION_SHOULDER_LEFT],
          pts[NUI_SKELETON_POSITION_ELBOW_LEFT],
          pts[NUI_SKELETON_POSITION_WRIST_LEFT],
          false
          );

        // right leg
        DrawLimp(right_thight_line, right_shin_line, -hip_line/2,
          pts[NUI_SKELETON_POSITION_HIP_RIGHT],
          pts[NUI_SKELETON_POSITION_KNEE_RIGHT],
          pts[NUI_SKELETON_POSITION_ANKLE_RIGHT],
          true
        );

        // left leg
        // polygon for left thight
        DrawLimp(left_thight_line, left_shin_line, hip_line/2,
          pts[NUI_SKELETON_POSITION_HIP_LEFT],
          pts[NUI_SKELETON_POSITION_KNEE_LEFT],
          pts[NUI_SKELETON_POSITION_ANKLE_LEFT],
          false
        );
      }
    }
    catch(...)  {}
  }
  rendering = false;
  return true;
}
/*****************************************************************************/
void Skeleton::DrawQuad(const vec3d &p1, const vec3d &p2,
    const vec3d &p3, const vec3d &p4)
{
  olx_gl::begin(GL_QUADS);
  for( int i=0; i < 4; i++ )  {
    olx_gl::vertex(p1);
    olx_gl::vertex(p2);
    olx_gl::vertex(p3);
    olx_gl::vertex(p4);
  }
  olx_gl::end();
}
/*****************************************************************************/
void Skeleton::DrawLimp(const vec3d &shoulder, const vec3d &wrist,
  const vec3d &body, const vec3d &p_shoulder, const vec3d &p_elbow,
  const vec3d &p_wrist, bool cw)
{
  vec3d sp_n = (shoulder*mat3d(0,-1,0, 1,0,0, 0,0,1))
    .NormaliseTo(shoulder.Length()/5);
  const double sp_n_ql = sp_n.QLength();
  if( sp_n_ql > 1e-6 && body.QLength() > 1e-6 )  {
    if( sp_n.CAngle(body) < 0 )
      sp_n *= -1;
    if( cw )
      DrawQuad(p_elbow, p_shoulder, p_shoulder+body, p_elbow+sp_n);
    else
      DrawQuad(p_shoulder, p_elbow, p_elbow+sp_n, p_shoulder+body);
  }
  // right wrist
  vec3d wp_n = (wrist*mat3d(0,-1,0, 1,0,0, 0,0,1))
    .NormaliseTo(wrist.Length()/5);
  if( sp_n_ql > 1e-6 && wp_n.QLength() > 1e-6 )  {
    if( wp_n.CAngle(sp_n) < 0 )
      wp_n *= -1;
    if( cw )
      DrawQuad(p_elbow, p_elbow+sp_n, p_wrist+wp_n, p_wrist);
    else
      DrawQuad(p_elbow, p_wrist, p_wrist+wp_n, p_elbow+sp_n);
  }
}

/*****************************************************************************/

#endif //__WIN32__
