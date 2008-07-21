#ifndef xgridH
#define xgridH
#include "gxbase.h"

#include "glmouselistener.h"
//#include "gdrawobject.h"
#include "arrays.h"
#include "evpoint.h"
#include "IsoSurface.h"
#include "gltextbox.h"
#include "macroerror.h"

#include "gltexture.h"
#include "glprimitive.h"

BeginGxlNamespace()

const short planeDrawModeOriginal = 1,
            planeDrawModeGradient = 2;

class TXGrid: public TGlMouseListener  {
  //TVectorDList AllPoints;
  TArray3D<float>* ED;
  CIsoSurface<float>* IS;
  vec3d GridStranslation;
  char *TextData;
  //TGlPrimitive *FPrimitive;
  class TGXApp * XApp;
  void DeleteObjects();
  int TextIndex;
  static TXGrid* Instance;
  void DrawQuad4(double A[4], double B[4], double C[4], double D[4]);
  void DrawQuad16(double points[4][4]);
  void RescaleSurface();
  TGlTextBox* Info;
  int PolygonMode;
  bool Mode3D;
  TGlPrimitive* glpP, *glpN;
  // these will keep the negative surface, the Isosurface - the positive
  TArrayList<vec3f> vertices;
  TArrayList<vec3f> normals;
  TArrayList<IsoTriangle> triangles;
  bool GridMoved;
protected:
  float MaxVal, MinVal, Depth, Size, Scale;
  int MaxX, MaxY, MaxZ, MaxDim; 
  float MinHole, MaxHole;  // the values of scale to skip
  int LastMouseX, LastMouseY;
  void CalcColorRGB(double v, double& R, double& G, double& B);
  void CalcColor(double v);
  bool MouseDown;
  void DoSmooth();
public:
  TXGrid(const olxstr& collectionName, TGXApp* xapp);
  virtual ~TXGrid();
  void Clear();
  inline TArray3D<float>* Data()  {  return ED;  }
  bool LoadFromFile(const olxstr& GridFile);

  void InitIso(bool v = true);
  void InitGrid(int maxX, int maxY, int MaxZ);
  void SetValue(int i, int j, int k, float v);
  inline double GetValue(int i, int j, int k) const {
    return ED->Data[i][j][k];
  }
  template <class T>
    void SetValue(const TVector<T>& ind, float v)  {
      ED->Data[(int)ind[0]][(int)ind[1]][(int)ind[2]] = v;
      if( v > MaxVal )  MaxVal = v;
      if( v < MinVal )  MinVal = v;
    }
  template <class T>
    inline double GetValue(const TVector<T>& v) const {
      return ED->Data[(int)v[0]][(int)v[1]][(int)v[2]];
    }

  void Create(const olxstr& cName = EmptyString);

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min);

  void SetScale(float v);
  inline double GetScale()  const {  return Scale;  }
  
  void SetDepth(float v);
  void SetDepth(const vec3d& v);

  DefPropP(float, MinHole)
  DefPropP(float, MaxHole)

  inline bool IsEmpty()  const  {  return ED == NULL;  }

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);

  inline static TXGrid* GetInstance()  {  return Instance;  }

  void LibDrawStyle3D(const TStrObjList& Params, TMacroError& E);
  void LibScale(const TStrObjList& Params, TMacroError& E);
  void LibSize(const TStrObjList& Params, TMacroError& E);
  void LibGetMin(const TStrObjList& Params, TMacroError& E);
  void LibGetMax(const TStrObjList& Params, TMacroError& E);
  void LibPolygonMode(const TStrObjList& Params, TMacroError& E);
  void LibIsvalid(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
#ifndef _NO_PYTHON
  static void PyInit();
#endif  
};

EndGxlNamespace()
#endif
 
