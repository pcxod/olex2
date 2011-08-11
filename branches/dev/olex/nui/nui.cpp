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

using namespace olx_nui;
/*****************************************************************************/
INUI *olx_nui::Initialise() {
#if defined (__WIN32__) && !defined(_WIN64) && defined(__OLX_USE_NUI__)
  Kinect *kinect = new Kinect;
  TEGC::AddP(kinect);
  kinect->Initialise();
  return kinect;
#endif
  return NULL;
}
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
#if defined(__WIN32__) && !defined(_WIN64) && defined(__OLX_USE_NUI__)
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
  if( points.Count() != NUI_SKELETON_POSITION_COUNT )
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
  vec3d_alist pts(points.Count());
  for( size_t i=0; i < points.Count(); i++ )  {
    vec3d t = (vs + points[i]*vs1)*scale;
    pts[i] = t;
    olx_gl::translate(t);
    P.Draw();
    olx_gl::translate(-t);
    if( i == NUI_SKELETON_POSITION_HAND_RIGHT )  {
      //vec3d sp = points[i]*vs1;
      //sp[1] = vs1[1] - sp[1];
      //rendering = true;
      //olx_gl::pushMatrix();
      //AGDrawObject *o = Parent.SelectObject(sp[0], sp[1], 0);
      //olx_gl::popMatrix();
      //rendering = false;
      //if( o != NULL )
      //  Parent.Select(*o, sp[2] > 1.4);
      vec3d sp = points[i]*vs1;
      sp[1] = vs1[1] - sp[1];
      if( sp[2] < 1.4 )  {
        if( !right_down )  {
          Parent.Background()->RB(0xff);
          right_down = true;
          TGXApp::GetInstance().MouseDown(sp[0], sp[1], 0, smbRight);
        }
        else  {
          TGXApp::GetInstance().MouseMove(sp[0], sp[1], 0);
        }
      }
      else if( right_down )  {
        Parent.Background()->RB(0xffffff);
        TGXApp::GetInstance().MouseUp(sp[0], sp[1], 0, smbRight);
        right_down = false;
      }
    }
    else if( i == NUI_SKELETON_POSITION_HAND_LEFT )  {
      vec3d sp = points[i]*vs1;
      sp[1] = vs1[1] - sp[1];
      if( sp[2] < 1.4 )  {
        if( !left_down )  {
          Parent.Background()->LT(0xff);
          left_down = true;
          TGXApp::GetInstance().MouseDown(sp[0], sp[1], 0, smbLeft);
        }
        else  {
          TGXApp::GetInstance().MouseMove(sp[0], sp[1], 0);
        }
      }
      else if( left_down )  {
        Parent.Background()->LT(0xffffff);
        TGXApp::GetInstance().MouseUp(sp[0], sp[1], 0, smbLeft);
        left_down = false;
      }

      //rendering = true;
      //olx_gl::pushMatrix();
      //AGDrawObject *o = Parent.SelectObject(sp[0], sp[1], 0);
      //olx_gl::popMatrix();
      //rendering = false;
      //if( o != NULL )
      //  Parent.Select(*o, false);
    }
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
      spine_line.QLength() > 0 &&
      right_shoulder_line.QLength() > 0 &&
      left_shoulder_line.QLength() > 0 &&
      right_wrist_line.QLength() > 0 &&
      left_wrist_line.QLength() > 0 )
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

      vec3d body_left_side = pts[NUI_SKELETON_POSITION_SPINE] +
        half_spine_line_normal - pts[NUI_SKELETON_POSITION_SHOULDER_LEFT];
      vec3d body_right_side = pts[NUI_SKELETON_POSITION_SPINE] -
        half_spine_line_normal - pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT];

      // right arm
      // polygon for right shoulder
      vec3d rsp_n = (right_shoulder_line*mat3d(0,-1,0, 1,0,0, 0,0,1))
        .NormaliseTo(right_shoulder_line.Length()/5);
      if( rsp_n.CAngle(body_right_side) < 0 )
        rsp_n *= -1;

      DrawQuad(
        pts[NUI_SKELETON_POSITION_ELBOW_RIGHT],
        pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT],
        pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT]+body_right_side/3,
        pts[NUI_SKELETON_POSITION_ELBOW_RIGHT]+rsp_n
        );
      // right wrist
      vec3d rwp_n = (right_wrist_line*mat3d(0,-1,0, 1,0,0, 0,0,1))
        .NormaliseTo(right_wrist_line.Length()/5);
      if( rwp_n.CAngle(rsp_n) < 0 )
        rwp_n *= -1;
      DrawQuad(
        pts[NUI_SKELETON_POSITION_ELBOW_RIGHT],
        pts[NUI_SKELETON_POSITION_ELBOW_RIGHT]+rsp_n,
        pts[NUI_SKELETON_POSITION_WRIST_RIGHT]+rwp_n,
        pts[NUI_SKELETON_POSITION_WRIST_RIGHT]
      );

      // left arm
      // polygon for left shoulder
      vec3d lsp_n = (left_shoulder_line*mat3d(0,-1,0, 1,0,0, 0,0,1))
        .NormaliseTo(left_shoulder_line.Length()/5);
      if( lsp_n.CAngle(body_left_side) < 0 )
        lsp_n *= -1;

      DrawQuad(
        pts[NUI_SKELETON_POSITION_SHOULDER_LEFT],
        pts[NUI_SKELETON_POSITION_ELBOW_LEFT],
        pts[NUI_SKELETON_POSITION_ELBOW_LEFT]+lsp_n,
        pts[NUI_SKELETON_POSITION_SHOULDER_LEFT]+body_left_side/3
        );

      vec3d lwp_n = (left_wrist_line*mat3d(0,-1,0, 1,0,0, 0,0,1))
        .NormaliseTo(left_wrist_line.Length()/5);
      if( lwp_n.CAngle(lsp_n) < 0 )
        lwp_n *= -1;
      DrawQuad(
        pts[NUI_SKELETON_POSITION_ELBOW_LEFT],
        pts[NUI_SKELETON_POSITION_WRIST_LEFT],
        pts[NUI_SKELETON_POSITION_WRIST_LEFT]+lwp_n,
        pts[NUI_SKELETON_POSITION_ELBOW_LEFT]+lsp_n
        );

      // right leg
      // polygon for right thight
      vec3d rtp_n = (right_thight_line*mat3d(0,-1,0, 1,0,0, 0,0,1))
        .NormaliseTo(right_thight_line.Length()/10);
      if( rtp_n.CAngle(hip_line) > 0 )
        rtp_n *= -1;
      DrawQuad(
        pts[NUI_SKELETON_POSITION_HIP_RIGHT],
        pts[NUI_SKELETON_POSITION_HIP_RIGHT]-hip_line/2,
        pts[NUI_SKELETON_POSITION_KNEE_RIGHT]+rtp_n,
        pts[NUI_SKELETON_POSITION_KNEE_RIGHT]
        );
      // right shin
      vec3d rshp_n = (right_shin_line*mat3d(0,-1,0, 1,0,0, 0,0,1))
        .NormaliseTo(right_shin_line.Length()/10);
      if( rshp_n.CAngle(rtp_n) < 0 )
        rshp_n *= -1;
      DrawQuad(
        pts[NUI_SKELETON_POSITION_KNEE_RIGHT],
        pts[NUI_SKELETON_POSITION_KNEE_RIGHT]+rtp_n,
        pts[NUI_SKELETON_POSITION_ANKLE_RIGHT]+rshp_n,
        pts[NUI_SKELETON_POSITION_ANKLE_RIGHT]
        );

      // left leg
      // polygon for left thight
      vec3d ltp_n = (left_thight_line*mat3d(0,-1,0, 1,0,0, 0,0,1))
        .NormaliseTo(left_thight_line.Length()/10);
      if( ltp_n.CAngle(hip_line) < 0 )
        ltp_n *= -1;
      DrawQuad(
        pts[NUI_SKELETON_POSITION_HIP_LEFT],
        pts[NUI_SKELETON_POSITION_KNEE_LEFT],
        pts[NUI_SKELETON_POSITION_KNEE_LEFT]+ltp_n,
        pts[NUI_SKELETON_POSITION_HIP_LEFT]+hip_line/2
        );
      // left shin
      vec3d lshp_n = (left_shin_line*mat3d(0,-1,0, 1,0,0, 0,0,1))
        .NormaliseTo(left_shin_line.Length()/5);
      if( lshp_n.CAngle(ltp_n) < 0 )
        lshp_n *= -1;
      DrawQuad(
        pts[NUI_SKELETON_POSITION_KNEE_LEFT],
        pts[NUI_SKELETON_POSITION_ANKLE_LEFT],
        pts[NUI_SKELETON_POSITION_ANKLE_LEFT]+lshp_n,
        pts[NUI_SKELETON_POSITION_KNEE_LEFT]+ltp_n);
      }
  }
  catch(...)  {}
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
/*****************************************************************************/
/*****************************************************************************/
void Kinect::throw_failed(const olxstr &src, HRESULT res, const olxstr &msg)  {
  if( msg.IsEmpty() )
    throw NUIInitFaieldEx(src, olxstr("HRESULT=") << res);
  else
    throw NUIInitFaieldEx(src, olxstr(msg) << " with HRESULT=" << res);
}
/*****************************************************************************/
void Kinect::InitProcessing(short flags)  {
  if( (flags&INUI::processVideo) != 0 )  {
    if( pixels == NULL )  {
      TGXApp &app = TGXApp::GetInstance();
      pixels = new TGlPixels(app.GetRender(), "kinet_pixels", 0, 0, 0, 0, 0, 0);
      pixels->SetVisible(false);
      app.AddObjectToCreate(pixels)->Create();
    }
  }
  if( (flags&INUI::processSkeleton) != 0 )  {
    if( skeleton == NULL )  {
      TGXApp &app = TGXApp::GetInstance();
      skeleton = new Skeleton(app.GetRender(), "kinet_skeleton");
      app.AddObjectToCreate(skeleton)->Create();
    }
  }
}
/*****************************************************************************/
void Kinect::DoProcessing()  {
  if( HasVideoFrame )
    processVideo();
  if( HasDepthFrame )
    processDepth();
  if( HasSkeleton )
    processSkeleton();
}
/*****************************************************************************/
Kinect *Kinect::Initialise()  {
  hDepthFrameEvt = CreateEvent(NULL, TRUE, FALSE, NULL);
  hVideoFrameEvt = CreateEvent(NULL, TRUE, FALSE, NULL);
  hSkeletonEvt = CreateEvent(NULL, TRUE, FALSE, NULL);

  HRESULT hr =
    NuiInitialize(
      NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX |
      NUI_INITIALIZE_FLAG_USES_SKELETON |
      NUI_INITIALIZE_FLAG_USES_COLOR);
  if( FAILED(hr) )
    throw_failed(__OlxSourceInfo, hr);

  Initialised = true;

  hr = NuiImageStreamOpen(
    NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
    NUI_IMAGE_RESOLUTION_320x240,
    0,
    2,
    hDepthFrameEvt,
    &hDepthStream);
  if( FAILED(hr) )
    throw_failed(__OlxSourceInfo, hr, "depth stream");

  hr = NuiSkeletonTrackingEnable(hSkeletonEvt, 0);
  if( FAILED(hr) )
    throw_failed(__OlxSourceInfo, hr, "skeleton tracking");

  hr = NuiImageStreamOpen(
    NUI_IMAGE_TYPE_COLOR,
    NUI_IMAGE_RESOLUTION_640x480,
    0,
    2,
    hVideoFrameEvt,
    &hVideoStream);
  if( FAILED(hr) )
    throw_failed(__OlxSourceInfo, hr, "video stream");

  listener = new ListenerThread(*this);
  listener->Start();
  return this;
}
/*****************************************************************************/
void Kinect::Finalise()  {
  if( Initialised )  {
    NuiShutdown();
    Initialised = false;
  }
  if( hDepthFrameEvt != INVALID_HANDLE_VALUE )  {
    CloseHandle(hDepthFrameEvt);
    hDepthFrameEvt = INVALID_HANDLE_VALUE;
  }
  if( hVideoFrameEvt != INVALID_HANDLE_VALUE )  {
    CloseHandle(hVideoFrameEvt);
    hVideoFrameEvt = INVALID_HANDLE_VALUE;
  }
  if( hSkeletonEvt != INVALID_HANDLE_VALUE )  {
    CloseHandle(hSkeletonEvt);
    hSkeletonEvt = INVALID_HANDLE_VALUE;
  }
  if( listener != NULL )  {
    listener->Join(true);
    delete listener;
    listener = NULL;
  }
}
/*****************************************************************************/
void Kinect::processDepth()  {
  if( !HasDepthFrame )   return;
  HasDepthFrame = false;
  const NUI_IMAGE_FRAME *pImageFrame = NULL;
  HRESULT hr = NuiImageStreamGetNextFrame(hDepthStream, 0, &pImageFrame);
  if( FAILED(hr) )
    return;

  NuiImageBuffer * pTexture = pImageFrame->pFrameTexture;
  KINECT_LOCKED_RECT LockedRect;
  pTexture->LockRect( 0, &LockedRect, NULL, 0 );
  if( LockedRect.Pitch != 0 )  {
    BYTE * pBuffer = (BYTE*) LockedRect.pBits;
    //// draw the bits to the bitmap
    //RGBQUAD * rgbrun = m_rgbWk;
    //USHORT * pBufferRun = (USHORT*) pBuffer;
    //for( int y = 0 ; y < 240 ; y++ )  {
    //  for( int x = 0 ; x < 320 ; x++ ) {
    //    RGBQUAD quad = Nui_ShortToQuad_Depth( *pBufferRun );
    //    pBufferRun++;
    //    *rgbrun = quad;
    //    rgbrun++;
    //  }
    //}
    //m_DrawDepth.DrawFrame( (BYTE*) m_rgbWk );
  }
  NuiImageStreamReleaseFrame(hDepthStream, pImageFrame);
}
/*****************************************************************************/
void Kinect::processVideo()  {
  if( !HasVideoFrame || pixels == NULL )  return;
  HasVideoFrame = false;
  const NUI_IMAGE_FRAME * pImageFrame = NULL;
  HRESULT hr = NuiImageStreamGetNextFrame(hVideoStream, 0, &pImageFrame);
  if( FAILED(hr) )
    return;

  NuiImageBuffer *pTexture = pImageFrame->pFrameTexture;
  KINECT_LOCKED_RECT LockedRect;
  pTexture->LockRect(0, &LockedRect, NULL, 0);
  if( LockedRect.Pitch != 0 )  {
    BYTE *pBuffer = (BYTE*) LockedRect.pBits;
    if( pixels->IsVisible() )
      pixels->SetData(640, 480, pBuffer, GL_RGBA); 
  }
  NuiImageStreamReleaseFrame(hVideoStream, pImageFrame);
  TGXApp::GetInstance().Draw();
}
/*****************************************************************************/
void Kinect::processSkeleton()  {
  if( !HasSkeleton || skeleton == NULL )   return;
  HasSkeleton = false;
  NUI_SKELETON_FRAME SkeletonFrame;
  HRESULT hr = NuiSkeletonGetNextFrame(0, &SkeletonFrame);
  bool bFoundSkeleton = true;
  for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )  {
    if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )  {
      bFoundSkeleton = false;
    }
  }

  //sk.points.Clear();
  if( bFoundSkeleton )
    return;

  // smooth out the skeleton data
  NuiTransformSmooth(&SkeletonFrame, NULL);
  for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )  {
    if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )  {
      float x, y;
      USHORT z;
      skeleton->points.SetCount(NUI_SKELETON_POSITION_COUNT);
      for( int j=0; j < NUI_SKELETON_POSITION_COUNT; j++ )  {
        NuiTransformSkeletonToDepthImageF(
          SkeletonFrame.SkeletonData[i].SkeletonPositions[j], &x, &y, &z);
        skeleton->points[j][0] = x;
        skeleton->points[j][1] = 1-y;
        skeleton->points[j][2] = (double)z/10000;
      }
    }
  }
  TGXApp::GetInstance().Draw();
}
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
int Kinect::ListenerThread::Run()  {
  while( true )  {
    if( Terminate )  break;
    DWORD evt_id = WaitForMultipleObjects(
      sizeof(events)/sizeof(events[0]), events, FALSE, 100);
    switch( evt_id )  {
    case WAIT_OBJECT_0:
      instance.HasDepthFrame = true;
      break;
    case WAIT_OBJECT_0+1:
      instance.HasVideoFrame = true;
      break;
    case WAIT_OBJECT_0+2:
      instance.HasSkeleton = true;
      break;
    }
  }
  return 0;
}
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#endif //__WIN32__
