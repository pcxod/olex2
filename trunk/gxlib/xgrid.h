#ifndef xgridH
#define xgridH
#include "gxbase.h"

#include "glmouselistener.h"
#include "arrays.h"
#include "evpoint.h"
#include "IsoSurface.h"
#include "gltextbox.h"
#include "macroerror.h"

#include "gltexture.h"
#include "glprimitive.h"

#include "wx/zipstrm.h"
#include "fracmask.h"
#include "ps_writer.h"

BeginGxlNamespace()

const short
  planeDrawModeOriginal = 1,
  planeDrawModeGradient = 2;
const short
  planeRenderModePlane   = 0x0001,
  planeRenderModeContour = 0x0002,
  planeRenderModePoint   = 0x0004,
  planeRenderModeFill    = 0x0008,
  planeRenderModeLine    = 0x0010;

class TXGrid: public TGlMouseListener  {
  //TVectorDList AllPoints;
  TArray3D<float>* ED;
  CIsoSurface<float>* IS;
  FractMask* Mask;
  // if mask is specified
  GLuint PListId, NListId;
  char *TextData;
  float **ContourData;
  float *ContourCrds[2], *ContourLevels;
  int ContourLevelCount;
  //TGlPrimitive *FPrimitive;
  class TGXApp * XApp;
  void DeleteObjects();
  GLuint TextIndex;
  static TXGrid* Instance;
  /*currently unused procedure for smoothing polygonised plane by linearly interpolating
  colours. So this procedure will render 4 quads with nice colour gradients */
  void DrawQuad4(double A[4], double B[4], double C[4], double D[4]);
  /* a more detailed procedure based on the above - an original quad is split into 16
  pieces and colours are lineraly interpolated */
  void DrawQuad16(double points[4][4]);
  void RescaleSurface();
  TGlTextBox* Info;
  short RenderMode;
  bool Extended;
  TGlPrimitive* glpP, *glpN, *glpC;
  // these will keep the masked objects
  TTypeList<vec3f> p_vertices, n_vertices;
  TTypeList<vec3f> p_normals, n_normals;
  TTypeList<IsoTriangle> p_triangles, n_triangles;
protected:
  float MaxVal, MinVal, Depth, Size, Scale;
  int MaxX, MaxY, MaxZ, MaxDim; 
  float MinHole, MaxHole;  // the values of scale to skip
  int LastMouseX, LastMouseY;
  void CalcColorRGB(float v, uint8_t& R, uint8_t& G, uint8_t& B);
  void CalcColor(float v);
  bool MouseDown;
  void DoSmooth();
  struct ContourDrawer {
    PSWriter output;
    ContourDrawer(const olxstr& file_name);
    void draw(float x1, float y1, float x2, float y2, float z);
  };
  void GlLine(float x1, float y1, float x2, float y2, float z);
  int GetPolygonMode() const {  return RenderMode == planeRenderModeFill ? GL_FILL : 
    (RenderMode == planeRenderModeLine ? GL_LINE : 
    (RenderMode == planeRenderModePoint ? GL_POINT : -1)); 
  }
  bool Is3D() const {  return RenderMode == planeRenderModeFill || 
    RenderMode == planeRenderModeLine ||
    RenderMode == planeRenderModePoint;
  }
  // updates the text information regarding current map
  void UpdateInfo();
public:
  TXGrid(const olxstr& collectionName, TGXApp* xapp);
  virtual ~TXGrid();
  void Clear();
  inline TArray3D<float>* Data()  {  return ED;  }
  bool LoadFromFile(const olxstr& GridFile);

  void InitIso();
  void InitGrid(int maxX, int maxY, int MaxZ);
  inline void SetValue(int i, int j, int k, float v) {
    ED->Data[i][j][k] = v;
  }
  inline double GetValue(int i, int j, int k) const {
    return ED->Data[i][j][k];
  }
  template <class T>
    void SetValue(const T& ind, float v)  {
      ED->Data[(int)ind[0]][(int)ind[1]][(int)ind[2]] = v;
    }
  template <class T>
    inline double GetValue(const T& v) const {
      return ED->Data[(int)v[0]][(int)v[1]][(int)v[2]];
    }
  
  // copies the 0yz x0z and xy0 layers to Maxyz xMaxyz and xyMaxZ
  void AdjustMap();
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min);

  void SetScale(float v);
  inline double GetScale() const {  return Scale;  }
  // this object will be deleted
  void SetMask(FractMask& fm) {  Mask = &fm;  }

  // extends the grid by +-1
  bool IsExtended() const {  return Extended;  }
  void SetExtended(bool v);

  void SetDepth(float v);
  void SetDepth(const vec3d& v);
  float GetDepth() const {  return Depth;  }
  vec3i GetDim() const {  return vec3i(MaxX, MaxY, MaxZ);  }
  float GetSize() const {  return Size;  }
  
  DefPropP(float, MinHole)
  DefPropP(float, MaxHole)
  DefPropP(float, MinVal)
  DefPropP(float, MaxVal)

  // recreates the lists, for draing on a different context
  void GlContextChange();

  inline bool IsEmpty()  const  {  return ED == NULL;  }
  short GetRenderMode() const {  return RenderMode;  }
  
  int GetContourLevelCount() const {  return ContourLevelCount;  }
  // sets new number of contours...
  void SetContourLevelCount(int v);

  inline virtual void SetVisible(bool On) {  
    AGDrawObject::SetVisible(On);  
    Info->SetVisible(On);
    if( !On )
      Clear();
  }

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);

  inline static TXGrid* GetInstance()  {  return Instance;  }

  void LibScale(const TStrObjList& Params, TMacroError& E);
  void LibExtended(const TStrObjList& Params, TMacroError& E);
  void LibSize(const TStrObjList& Params, TMacroError& E);
  void LibContours(const TStrObjList& Params, TMacroError& E);
  void LibGetMin(const TStrObjList& Params, TMacroError& E);
  void LibGetMax(const TStrObjList& Params, TMacroError& E);
  void LibRenderMode(const TStrObjList& Params, TMacroError& E);
  void LibIsvalid(const TStrObjList& Params, TMacroError& E);
  void LibWritePS(TStrObjList& Params, const TParamList& Options, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
#ifndef _NO_PYTHON
  static void PyInit();
#endif  
  void ToDataItem(TDataItem& item, IOutputStream& zos) const;
  void FromDataItem(const TDataItem& item, IInputStream& zis);
};

EndGxlNamespace()
#endif
 
