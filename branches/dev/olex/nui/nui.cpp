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
#if defined (__WIN32__) && !defined(_WIN64)
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
#if defined(__WIN32__) && !defined(_WIN64)
void Skeleton::Create(const olxstr& cName) {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  TGlMaterial GlM;
  GlM.SetFlags(0);   
  GlM.ShininessF = 128;
  GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmSpecularF|sglmIdentityDraw);
  //GlM.SetFlags(sglmAmbientF|sglmDiffuseF);
  GlM.AmbientF = 0x800f0f0f;
  GlM.DiffuseF = 0x800f0f0f;
  GlM.SpecularF = 0x808080;

  TGlPrimitive& GlP = GPC.NewPrimitive("Sphere", sgloSphere);
  GlP.SetProperties(GlM);
  //GlP.SetProperties(GS.GetMaterial(GlP.GetName(), GlM));
  GlP.Params[0] = 0.01;  GlP.Params[1] = 10;  GlP.Params[2] = 10;
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
  olx_gl::begin(GL_TRIANGLES);
  olx_gl::vertex(pts[NUI_SKELETON_POSITION_HIP_RIGHT]);
  olx_gl::vertex(pts[NUI_SKELETON_POSITION_HIP_CENTER]);
  olx_gl::vertex(pts[NUI_SKELETON_POSITION_HIP_LEFT]);
  olx_gl::end();

  olx_gl::begin(GL_POLYGON);
  olx_gl::vertex(pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT]);
  olx_gl::vertex(pts[NUI_SKELETON_POSITION_SHOULDER_CENTER]);
  olx_gl::vertex(pts[NUI_SKELETON_POSITION_SHOULDER_LEFT]);
  olx_gl::vertex(pts[NUI_SKELETON_POSITION_SPINE]);
  olx_gl::end();

  // right arm
  // polygon for right shoulder
  vec3d rsp = (pts[NUI_SKELETON_POSITION_ELBOW_RIGHT] - 
    pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT]);
  vec3d rsp_n = (rsp*mat3d(0,-1,0, 1,0,0, 0,0,1)).NormaliseTo(
    rsp.Length()/5);

  DrawQuad(
    pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT]+rsp_n,
    pts[NUI_SKELETON_POSITION_ELBOW_RIGHT]+rsp_n,
    pts[NUI_SKELETON_POSITION_ELBOW_RIGHT]-rsp_n,
    pts[NUI_SKELETON_POSITION_SHOULDER_RIGHT]-rsp_n);
  // right wrist
  vec3d rwp = (pts[NUI_SKELETON_POSITION_WRIST_RIGHT] - 
    pts[NUI_SKELETON_POSITION_ELBOW_RIGHT]);
  vec3d rwp_n = (rwp*mat3d(0,-1,0, 1,0,0, 0,0,1)).NormaliseTo(
    rwp.Length()/5);

  DrawQuad(
    pts[NUI_SKELETON_POSITION_ELBOW_RIGHT]+rsp_n,
    pts[NUI_SKELETON_POSITION_WRIST_RIGHT]+rwp_n,
    pts[NUI_SKELETON_POSITION_WRIST_RIGHT]-rwp_n,
    pts[NUI_SKELETON_POSITION_ELBOW_RIGHT]-rsp_n);

  // left arm
  // polygon for left shoulder
  vec3d lsp = (pts[NUI_SKELETON_POSITION_ELBOW_LEFT] -
    pts[NUI_SKELETON_POSITION_SHOULDER_LEFT]);
  vec3d lsp_n = (lsp*mat3d(0,-1,0, 1,0,0, 0,0,1)).NormaliseTo(
    lsp.Length()/5);

  DrawQuad(
    pts[NUI_SKELETON_POSITION_SHOULDER_LEFT]+lsp_n,
    pts[NUI_SKELETON_POSITION_ELBOW_LEFT]+lsp_n,
    pts[NUI_SKELETON_POSITION_ELBOW_LEFT]-lsp_n,
    pts[NUI_SKELETON_POSITION_SHOULDER_LEFT]-lsp_n);

  vec3d lwp = (pts[NUI_SKELETON_POSITION_WRIST_LEFT] -
    pts[NUI_SKELETON_POSITION_ELBOW_LEFT]);
  vec3d lwp_n = (rwp*mat3d(0,-1,0, 1,0,0, 0,0,1)).NormaliseTo(
    lwp.Length()/5);
  DrawQuad(pts[NUI_SKELETON_POSITION_ELBOW_LEFT]+lsp_n,
    pts[NUI_SKELETON_POSITION_WRIST_LEFT]+lwp_n,
    pts[NUI_SKELETON_POSITION_WRIST_LEFT]-lwp_n,
    pts[NUI_SKELETON_POSITION_ELBOW_LEFT]-lsp_n);

  rendering = false;
  return true;
}
/*****************************************************************************/
void Skeleton::DrawQuad(const vec3d &p1, const vec3d &p2,
    const vec3d &p3, const vec3d &p4)
{
  TPSTypeList<double, const vec3d*> sorted;
  vec3d center = (p1+p2+p3+p4)/4;
  sorted.Add(0, &p1);
  vec3d org(p1-center);
  const vec3d *pts[3] = {&p2, &p3, &p4};
  for( int i=0; i < 3; i++ )  {
    vec3d vec = *pts[i] - center;
    double ca = org.CAngle(vec);
    vec = org.XProdVec(vec);
    // negative - vec is on the right, positive - on the left, if ca == 0, vec == (0,0,0)
    double vo = (ca <= 0.999 ? 0 : vec.Normalise()[2]);
    if( ca >= 0 )  { // -90 to 90
      if( vo < 0 )  // -90 to 0 3->4
        sorted.Add(3.0 + ca, pts[i]);
      else  // 0 to 90 0->1
        sorted.Add(1.0 - ca, pts[i]);
    }
    else if( ca > -1 ) {  // 90-270
      if( vo < 0 )  // 180 to 270 2->3
        sorted.Add(3.0 + ca, pts[i]);
      else  // 90 to 180 1->2
        sorted.Add(1.0 - ca, pts[i]);
    }
    else  {  //-1, special case
      sorted.Add(2, pts[i]);
    }
  }
  olx_gl::begin(GL_QUADS);
  for( int i=0; i < 4; i++ )
    olx_gl::vertex(*sorted.GetObject(i));
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
