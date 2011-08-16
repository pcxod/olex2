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
  if( bFoundSkeleton )  {
    //for( size_t i=0; i < skeleton->points.Count(); i++ )
    //  skeleton->points[i].Clear();
    return;
  }

  // smooth out the skeleton data
  NuiTransformSmooth(&SkeletonFrame, NULL);
  for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )  {
    if( SkeletonFrame.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED )  {
      float x, y;
      USHORT z;
      skeleton->points[i].SetCount(NUI_SKELETON_POSITION_COUNT);
      for( int j=0; j < NUI_SKELETON_POSITION_COUNT; j++ )  {
        NuiTransformSkeletonToDepthImageF(
          SkeletonFrame.SkeletonData[i].SkeletonPositions[j], &x, &y, &z);
        skeleton->points[i][j][0] = x;
        skeleton->points[i][j][1] = 1-y;
        skeleton->points[i][j][2] = (double)z;
      }
    }
    else
      skeleton->points[i].Clear();
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
