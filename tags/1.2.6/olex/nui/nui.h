/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "exception.h"
#include "library.h"
#include "olxth.h"
#include "glpixels.h"
#include "edict.h"

namespace olx_nui {

class NUIInitFaieldEx : public TBasicException {
public:
  NUIInitFaieldEx(const olxstr &location, const olxstr &msg=EmptyString())
    : TBasicException(location, msg) {}
  const char* GetNiceName() const {  return "NUI initialisation failed";  }
  IEObject* Replicate() const {  return new NUIInitFaieldEx(*this);  }
};

class INUI : public IEObject {
public:
  static const short
    processVideo    = 0x0001,
    processSkeleton = 0x0002,
    processDepth    = 0x0004;
  virtual void InitProcessing(short flags) = 0;
  virtual void DoProcessing() = 0;
};

INUI *Initialise();

#if defined __WIN32__ && !defined(_WIN64) && defined(__OLX_USE_NUI__)
#include <MSR_NuiApi.h>

class Skeleton : public AGDrawObject {
protected:
  void DrawQuad(const vec3d &p1, const vec3d &p2,
    const vec3d &p3, const vec3d &p4);
  void DrawLimp(const vec3d &shoulder, const vec3d &wrist,
    const vec3d &body, const vec3d &p_shoulder, const vec3d &p_elbow,
    const vec3d &p_wrist, bool cw);
public:
  Skeleton(TGlRenderer& Renderer, const olxstr& collectionName)
  : AGDrawObject(Renderer, collectionName),
    points(NUI_SKELETON_COUNT)
  {}
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min)  {
    return false;
  }
  TArrayList<vec3d_alist> points;
};

class Kinect : public INUI {
  bool Initialised;
  HANDLE hDepthFrameEvt, hVideoFrameEvt, hSkeletonEvt;
  HANDLE hDepthStream, hVideoStream;
  void throw_failed(const olxstr &src, HRESULT res, const olxstr &msg=EmptyString());
protected:
  class ListenerThread : public AOlxThread {
  Kinect &instance;
  HANDLE events[3];
  public:
    ListenerThread(Kinect &_instance) : instance(_instance)  {
      events[0] = instance.hDepthFrameEvt;
      events[1] = instance.hVideoFrameEvt;
      events[2] = instance.hSkeletonEvt;
    }
    virtual int Run();
  };
  ListenerThread *listener;
  Skeleton *skeleton;
  TGlPixels *pixels;
  olxstr last_command;
  olxstr_dict<olxstr, true> commands;
public:
  Kinect()
  : Initialised(false),
    hDepthFrameEvt(INVALID_HANDLE_VALUE),
    hVideoFrameEvt(INVALID_HANDLE_VALUE),
    hSkeletonEvt(INVALID_HANDLE_VALUE),
    listener(NULL),
    HasVideoFrame(false),
    HasDepthFrame(false),
    HasSkeleton(false),
    skeleton(NULL),
    pixels(NULL)
  {}

  ~Kinect()  {  Finalise();  }

  Kinect* Initialise();
  void Finalise();
  bool HasVideoFrame, HasDepthFrame, HasSkeleton;

  virtual void InitProcessing(short flags);
  virtual void DoProcessing();

  void processVideo();
  void processDepth();
  void processSkeleton();
};
#endif //__WIN32__
};
