#ifndef olex2vjH
#define olex2vjH
#include <windows.h>
#include "gxapp.h"
#include "integration.h"
#include "exception.h"

#ifdef _MSC_VER
  #define DllImport   __declspec( dllimport )
  #define DllExportA   __declspec( dllexport )
  #define DllExportB
#endif
#ifdef __BORLANDC__
  #define DllExportA
  #define DllExportB __export
  #define DllImport __import
  #ifdef __WXWIDGETS__
    #define HAVE_SNPRINTF_DECL
  #endif
#endif

const short  olxv_MouseUp     = 0x0001,
             olxv_MouseDown   = 0x0002,
             olxv_MouseMove   = 0x0003;
const short  olxv_MouseLeft   = 0x0001,
             olxv_MouseMiddle = 0x0002,
             olxv_MouseRight  = 0x0004;
const short  olxv_ShiftCtrl   = 0x0001,
             olxv_ShiftShift  = 0x0002,
             olxv_ShiftAlt    = 0x0004;

struct DrawContext {
  HDC DC;
  HGLRC GlContext;
  HBITMAP DIBmp;
  int Width, Height;
  char* Buffer;
  olxstr Status;

  DrawContext(int w, int h);
  ~DrawContext();

  char* ReadPixels();

  static DrawContext* Instance;
};

class TOlexViewer : public olex::IOlexProcessor {
  bool Initialised;
  olxstr FileName;
  static TGXApp* GXApp;
  TEBasis Basis;
  bool LabelsVisible;
public:
  TOlexViewer(int w, int h);
  ~TOlexViewer();
  void OnPaint();
  bool OnMouse(int x, int y, short MouseEvent, short MouseButton, short ShiftState);
  void OnFileChanged(const char* fileName);
  olxstr GetSelectionInfo();
  olxstr GetObjectLabelAt(int x, int y);
  void ShowLabels(bool v);
  bool IsInitialised() const {  return Initialised;  }
  olxstr Status;
  static TPSTypeList<int, TOlexViewer*> Instances;
  static TOlexViewer* Locate(int object)  {
    int i = Instances.IndexOfComparable(object);
    return (i==-1) ? NULL : Instances.Object(i);
  }
  const olxstr GetFileName() const {  return FileName;  }
  void SaveState();
  void RestoreState();

  virtual bool executeMacro(const olxstr& cmdLine) {  return false;  }
  virtual void print(const olxstr& Text, const short MessageType = olex::mtNone) {  return;  }
  virtual bool executeFunction(const olxstr& funcName, olxstr& retValue) {  return false;  }
  virtual IEObject* executeFunction(const olxstr& funcName) {  return NULL;  }
  virtual TLibrary&  GetLibrary() {  return GXApp->GetLibrary();  }
  virtual bool registerCallbackFunc(const olxstr& cbEvent, ABasicFunction* fn) {  return false;  }
  virtual void unregisterCallbackFunc(const olxstr& cbEvent, const olxstr& funcName) {  return;  }

  virtual const olxstr& getDataDir() const {  return EmptyString;  }
  virtual const olxstr& getVar(const olxstr &name, const olxstr &defval=NullString) const {  return EmptyString;  }
  virtual void setVar(const olxstr &name, const olxstr &val) const {  return;  }

};

extern "C" {
  JNIEXPORT void JNICALL Java_olex2j_GlWindow_paint(JNIEnv *, jobject, jint o_id, jintArray buffer, jint w, jint h);
  JNIEXPORT jboolean JNICALL Java_olex2j_GlWindow_mouseEvt(JNIEnv *, jobject, jint o_id, jint x, jint y, jshort evt, jshort button, jshort shift);
  JNIEXPORT void JNICALL Java_olex2j_GlWindow_fileChanged(JNIEnv *, jobject, jint o_id, jstring fileName);
  JNIEXPORT jbyteArray JNICALL Java_olex2j_GlWindow_getStatus(JNIEnv *, jobject, jint o_id);
  JNIEXPORT jbyteArray JNICALL Java_olex2j_GlWindow_getSelectionInfo(JNIEnv *, jobject, jint o_id);
  JNIEXPORT jbyteArray JNICALL Java_olex2j_GlWindow_getObjectLabelAt(JNIEnv *, jobject, jint o_id, jint x, jint y);
  JNIEXPORT void JNICALL Java_olex2j_GlWindow_showLabels(JNIEnv *, jobject, jint o_id, jboolean show);
  JNIEXPORT void JNICALL Java_olex2j_GlWindow_finalise(JNIEnv *, jobject, jint o_id);
};
#endif
