/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glclipplane.h"
#include "glrender.h"
UseGlNamespace();

TGlClipPlane::TGlClipPlane( int Id, TGlClipPlanes *Parent, float A, float B, float C, float D ) : FEq(4)  {
  FParent = Parent;
  FEq[0] = A;
  FEq[1] = B;
  FEq[2] = C;
  FEq[3] = D;
  FId = Id;
  FEnabled = false;
}
TGlClipPlane::~TGlClipPlane()  {  }
//..............................................................................
void TGlClipPlane::Enabled(bool v)  {
//  FParent->EnableClipPlane(this, v);
  FEnabled = v;
}
//----------------------------------------------------------------------------//
// TGlClipPlanes
//----------------------------------------------------------------------------//
TGlClipPlanes::TGlClipPlanes(TGlRenderer *R)  {
  FParent = R;
  for( GLuint i=0; i < GL_MAX_CLIP_PLANES; i++ )  {
    FPlanes.Add( new TGlClipPlane(GL_CLIP_PLANE0 +i, this, 0, 0, 0, 0) );
  }
}
//..............................................................................
TGlClipPlanes::~TGlClipPlanes()  {
  for( size_t i=0; i < FPlanes.Count(); i++ )
    delete FPlanes[i];
}
//..............................................................................
void TGlClipPlanes::Enable(bool v)  {
  for( size_t i=0; i < PlaneCount(); i++ )  {
    if( v )      FParent->EnableClipPlane(FPlanes[i], true);
    else         FParent->EnableClipPlane(FPlanes[i], false);
  }
}
