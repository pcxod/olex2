#include "ebase.h"
#include <GL/gl.h>
#include <GL/glu.h>

#include "win32/jawt_md.h"
#include "olex2vji.h"

#include "ins.h"
#include "cif.h"
#include "olxvar.h"
#include "xatom.h"
#include "xbond.h"
#include "gllabels.h"

TPSTypeList<int, TOlexViewer*> TOlexViewer::Instances;
TGXApp* TOlexViewer::GXApp = NULL;
DrawContext* DrawContext::Instance = NULL;


TOlexViewer::TOlexViewer(int w, int h) {
  Initialised = false;
  if( DrawContext::Instance == NULL )  {
    new DrawContext(w, h);
  }
  bool inited = GXApp != NULL;
  if( GXApp == NULL ) 
    GXApp = new TGXApp(EmptyString());
  if( !Status.IsEmpty() )  {
    Initialised = false;
    return;
  }
  if( inited )  {
    if( !Instances.IsEmpty() )
      Initialised = Instances.GetObject(0)->Initialised;
    return;
  }
  olx_gl::clearColor(0.5, 0.5, 0.0, 0.0);
  GXApp->GetRender().Resize(0, 0, w, h, 1);
  TGlMaterial glm;
  glm.SetFlags(sglmAmbientF|sglmEmissionF|sglmIdentityDraw);
  glm.AmbientF = 0x7fff7f;
  glm.EmissionF = 0x1f2f1f;
  TIns *Ins = new TIns;
  GXApp->RegisterXFileFormat(Ins, "ins");
  GXApp->RegisterXFileFormat(Ins, "res");
  GXApp->RegisterXFileFormat(new TCif, "cif");
  GXApp->SetMainFormVisible(true);
  GXApp->AtomRad("isot");
  //GXApp->SetAtomDrawingStyle(adsEllipsoid);
  TXAtom::DefMask(5);
  TXAtom::DefDS(adsEllipsoid);
  TXBond::DefMask(48);
  GXApp->SetLabelsVisible(false);
  GXApp->CalcProbFactor(50);
  GXApp->SetLabelsMode(lmLabels);
  Initialised = Status.IsEmpty();
}
TOlexViewer::~TOlexViewer()  {
  if( GXApp && Instances.Count() == 1 )  {
    delete GXApp;
    GXApp = NULL;
  }
  TOlxVars::Finalise();
}
void TOlexViewer::OnPaint()  {
  GXApp->Draw();
  GdiFlush();
  olx_gl::flush();
}
bool TOlexViewer::OnMouse(int x, int y, short MouseEvent, short MouseButton, short ShiftState)  {
  bool res = false;
  short btn = 0, shift = 0;
  if( MouseButton & olxv_MouseLeft )   btn |= smbLeft;
  else if( MouseButton & olxv_MouseRight )  btn |= smbRight;
  else if( MouseButton & olxv_MouseMiddle ) btn |= smbMiddle;
  if( ShiftState & olxv_ShiftShift )  shift |= sssShift;
  if( ShiftState & olxv_ShiftAlt )    shift |= sssAlt;
  if( ShiftState & olxv_ShiftCtrl )   shift |= sssCtrl;

  if( MouseEvent == olxv_MouseDown ) 
    res = GXApp->MouseDown(x, y, shift, btn);
  else if( MouseEvent == olxv_MouseUp ) 
    res = GXApp->MouseUp(x, y, shift, btn);
  else if( MouseEvent == olxv_MouseMove ) 
    res = GXApp->MouseMove(x, y, shift);
  return res;
}
void TOlexViewer::OnFileChanged(const char* fileName)  {
  try  {
    GXApp->LoadXFile(fileName);
    GXApp->SetAtomDrawingStyle(adsEllipsoid);
    GXApp->Uniq();
    Status = fileName;
    FileName = fileName;
  }
  catch(...)  {
    Status = "Failed on: ";  
    Status << fileName;
  }
}

olxstr TOlexViewer::GetSelectionInfo()  {
  return GXApp->GetSelectionInfo();
}

olxstr TOlexViewer::GetObjectLabelAt(int x, int y)  {
  AGDrawObject *G = GXApp->SelectObject(x, y, 0);
  olxstr Tip;
  if( G != NULL )  {
    if( EsdlInstanceOf( *G, TXAtom) )  {
      TXAtom& xa = *(TXAtom*)G;
      Tip = xa.GetLabel();
      if( xa.GetType() == iQPeakZ )  {
        Tip << ':' << xa.CAtom().GetQPeak();
      }
    }
    else  if( EsdlInstanceOf( *G, TXBond) )  {
      Tip = ((TXBond*)G)->A().GetLabel();
      Tip << '-' << ((TXBond*)G)->B().GetLabel() << ": ";
      Tip << olxstr::FormatFloat(3, ((TXBond*)G)->Length());
    } 
  }
  return Tip;
}

void TOlexViewer::ShowLabels(bool v)  {
  GXApp->SetLabelsVisible(v);
}
void TOlexViewer::SaveState()  {
  Basis = GXApp->GetRender().GetBasis();
  LabelsVisible = GXApp->AreLabelsVisible();
}
void TOlexViewer::RestoreState()  {
  GXApp->GetRender().SetBasis(Basis);
  GXApp->SetLabelsVisible( LabelsVisible );
}



DrawContext::DrawContext(int w, int h)  {
  Instance = this;
  Buffer = NULL;
  DIBmp = NULL;
  DC = CreateCompatibleDC(NULL);
  if( DC == NULL )  {
    Status = "CreateCompatibleDC failed";
    return;
  }
  Width = w;
  Height = h;
  // intialise bitmap header
  BITMAPINFO BmpInfo;
  BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
  BmpInfo.bmiHeader.biWidth = Width;
  BmpInfo.bmiHeader.biHeight = Height;
  BmpInfo.bmiHeader.biPlanes = 1;
  BmpInfo.bmiHeader.biBitCount = (WORD) 24;
  BmpInfo.bmiHeader.biCompression = BI_RGB;
  BmpInfo.bmiHeader.biSizeImage = 0;
  BmpInfo.bmiHeader.biXPelsPerMeter = 0;
  BmpInfo.bmiHeader.biYPelsPerMeter = 0;
  BmpInfo.bmiHeader.biClrUsed = 0;
  BmpInfo.bmiHeader.biClrImportant = 0;

  char* DIBits = NULL;
  DIBmp = CreateDIBSection(DC, &BmpInfo, DIB_RGB_COLORS, (void **)&DIBits, NULL, 0);
  SelectObject(DC, DIBmp);
  int PixelFormat;
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
	  PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP, // support OpenGL
	  PFD_TYPE_RGBA,
    24,  // 24 bit
    0,0,0,0,0,0,
    0,0,
    0,0,0,0,0,
    32,
    0,
    0,
    PFD_MAIN_PLANE,
    0,
    0,0,
  };
  PixelFormat = ChoosePixelFormat(DC, &pfd);
  if( PixelFormat == 0 )  {
    Status = "ChoosePixelFormat failed";
    return;
  }
  if( !SetPixelFormat(DC, PixelFormat, &pfd) )  {
    Status = "SetPixelFormat failed";
    return;
  }
  GlContext = wglCreateContext(DC);
  if( GlContext == NULL )  {
    Status = "wglCreateContext failed";
    return;
  }
  if( !wglMakeCurrent(DC, GlContext) )   {
    Status = "wglMakeCurrent failed";
    return;
  }
  Buffer = (char*)realloc(Buffer, Width*(Height+1)*4);
}
DrawContext::~DrawContext()  {
  Instance = NULL;
  if( GlContext != NULL && DC != NULL )  {
    wglMakeCurrent(DC, NULL); 
    wglDeleteContext(GlContext);
  }
  if( DC != NULL ) DeleteDC(DC);
  if( Buffer != NULL )  free(Buffer);
  if( DIBmp != NULL )  DeleteObject(DIBmp);
}
char* DrawContext::ReadPixels()  {
  if( Buffer != NULL )  {
    olx_gl::readBuffer(GL_FRONT);
    olx_gl::pixelStore(GL_PACK_ALIGNMENT, 1);
    olx_gl::readPixels(0, 0, Width, Height, GL_RGBA, GL_UNSIGNED_BYTE, Buffer);
    const int sz = Width*Height*4;
    for( int i=0; i < Width; i++ )  {  // mirrow by Heght and inverrt
      for( int j=0; j < Height/2; j++ )  {
        int ind1 = (j*Width+i)*4;
        int ind2 = ((Height-j-1)*Width + i)*4;
        char v[4] = {Buffer[ind1], Buffer[ind1+1], Buffer[ind1+2], Buffer[ind1+3]};
        Buffer[ind1] = Buffer[ind2+2];
        Buffer[ind1+1] = Buffer[ind2+1];
        Buffer[ind1+2] = Buffer[ind2];
        Buffer[ind1+3] = Buffer[ind2+3];
        Buffer[ind2] = v[2];
        Buffer[ind2+1] = v[1];
        Buffer[ind2+2] = v[0];
        Buffer[ind2+3] = v[3];
      }
    }
  }
  return Buffer;
}


JNIEXPORT void JNICALL Java_olex2j_GlWindow_paint(JNIEnv* env, jobject this_object, jint o_id, jintArray bf, jint w, jint h)  {
  static int pvId = -1;
  bool empty = false;
  TOlexViewer* ov = TOlexViewer::Locate(o_id);
  if( ov == NULL )  {  
    try  {
      TOlexViewer::Instances.Add(o_id, NULL);
      ov = new TOlexViewer(w, h);  
      TOlexViewer::Instances[o_id] = ov;
    }
    catch( ... )  { }
  }
  if( ov == NULL || !ov->IsInitialised() )  return;
  if( pvId != o_id && pvId != -1 )  {
    TOlexViewer* pov = TOlexViewer::Locate(pvId);
    if( pov != NULL && !pov->GetFileName().IsEmpty() )
      pov->SaveState();
  }
  if( TGXApp::GetInstance().XFile().GetFileName() != ov->GetFileName() && !ov->GetFileName().IsEmpty() )  {
    ov->OnFileChanged(ov->GetFileName().c_str());
  }
  else if( ov->GetFileName().IsEmpty() )
    empty = true;
  if( pvId != o_id && pvId != -1 && !ov->GetFileName().IsEmpty() )
    ov->RestoreState();
  pvId = o_id;
  if( !empty )  {
    ov->OnPaint();
    DrawContext::Instance->ReadPixels();
  }
  if( empty )  {
    const float* cc = TGXApp::GetInstance().GetRender().LightModel.GetClearColor().Data();
    char clrs[4] = {
      char(cc[0]*255), char(cc[1]*255), char(cc[2]*255), char(cc[4]*255)};
    int sz = DrawContext::Instance->Height * DrawContext::Instance->Width * 4;
    for( int i = 0; i < sz; i +=4 )  {
      DrawContext::Instance->Buffer[i]   = clrs[0];
      DrawContext::Instance->Buffer[i+1] = clrs[1];
      DrawContext::Instance->Buffer[i+2] = clrs[2];
      DrawContext::Instance->Buffer[i+3] = clrs[3];
    }
  }
  env->SetIntArrayRegion(bf, 0, w*h, (const jint* )DrawContext::Instance->Buffer); 
}
JNIEXPORT jboolean JNICALL Java_olex2j_GlWindow_mouseEvt(JNIEnv* env, jobject this_object, jint o_id, jint x, jint y, jshort evt, jshort button, jshort shift)  {
  TOlexViewer* ov = TOlexViewer::Locate(o_id);
  if( ov == NULL || !ov->IsInitialised())  return false;
  return ov->OnMouse(x, y, evt, button, shift);
}
JNIEXPORT void JNICALL Java_olex2j_GlWindow_fileChanged(JNIEnv* env, jobject this_object, jint o_id, jstring fileName)  {
  TOlexViewer* ov = TOlexViewer::Locate(o_id);
  if( ov == NULL || !ov->IsInitialised() )  return;
  const char* str = env->GetStringUTFChars(fileName, NULL);
  if( str == NULL )  return;
  ov->OnFileChanged( str );
  env->ReleaseStringUTFChars(fileName, str);
}
JNIEXPORT jbyteArray JNICALL Java_olex2j_GlWindow_getStatus(JNIEnv* env, jobject this_object, jint o_id)  {
  TOlexViewer* ov = TOlexViewer::Locate(o_id);
  if( ov == NULL || !ov->IsInitialised() )  return false;
  jbyteArray rv = env->NewByteArray( ov->Status.Length() );
  if( !ov->Status.IsEmpty() )
    env->SetByteArrayRegion(rv, 0, ov->Status.Length(), (const jbyte*)ov->Status.c_str() );
  return rv;
}
JNIEXPORT void JNICALL Java_olex2j_GlWindow_finalise(JNIEnv *, jobject this_object, jint o_id)  {
  size_t i = TOlexViewer::Instances.IndexOf(o_id);
  if( i == InvalidIndex )  return;
  delete TOlexViewer::Instances.GetObject(i);
  TOlexViewer::Instances.Delete(i);
  if( TOlexViewer::Instances.IsEmpty() )
    delete DrawContext::Instance;
}
JNIEXPORT jbyteArray JNICALL Java_olex2j_GlWindow_getSelectionInfo(JNIEnv* env, jobject this_object, jint o_id)  {
  TOlexViewer* ov = TOlexViewer::Locate(o_id);
  if( ov == NULL || !ov->IsInitialised() )  return env->NewByteArray(0);
  olxstr si( ov->GetSelectionInfo() );
  jbyteArray rv = env->NewByteArray( si.Length() );
  if( !si.IsEmpty() )
    env->SetByteArrayRegion(rv, 0, si.Length(), (const jbyte*)si.c_str() );
  return rv;
}
JNIEXPORT jbyteArray JNICALL Java_olex2j_GlWindow_getObjectLabelAt(JNIEnv* env, jobject this_object, jint o_id, jint x, jint y)  {
  TOlexViewer* ov = TOlexViewer::Locate(o_id);
  if( ov == NULL  || !ov->IsInitialised() )  return env->NewByteArray(0);
  olxstr si( ov->GetObjectLabelAt(x, y) );
  jbyteArray rv = env->NewByteArray( si.Length() );
  if( !si.IsEmpty() )
    env->SetByteArrayRegion(rv, 0, si.Length(), (const jbyte*)si.c_str() );
  return rv;
}
JNIEXPORT void JNICALL Java_olex2j_GlWindow_showLabels(JNIEnv* env, jobject this_object, jint o_id, jboolean show)  {
  TOlexViewer* ov = TOlexViewer::Locate(o_id);
  if( ov == NULL  || !ov->IsInitialised() )  return;
  ov->ShowLabels(show != 0);
}

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)  {
    return TRUE;
}

