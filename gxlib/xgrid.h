/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_xgrid_H
#define __olx_gxl_xgrid_H
#include "gxbase.h"
#include "arrays.h"
#include "gltextbox.h"
#include "macroerror.h"
#include "gltexture.h"
#include "glprimitive.h"
#ifdef __WXWIDGETS__
#include "wx/zipstrm.h"
#endif
#include "fracmask.h"
//#include "ps_writer.h"
#include "xblob.h"
#include "symmat.h"
#include "maputil.h"
#include "glbitmap.h"
#include "olxmps.h"
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

//class TXGrid: public TGlMouseListener  {
class TXGrid : public AGDrawObject {
  class TLegend : public AGlMouseHandlerImp {
    int Width, Height;
    int Top, Left;
    GLuint TextureId;
    double Z;
    TGlMaterial GlM;
  protected:
    vec3d Center;
    virtual bool DoTranslate(const vec3d& t) { Center += t;  return true; }
    virtual bool DoRotate(const vec3d&, double) { return false; }
    virtual bool DoZoom(double zoom, bool inc) { return false; }
    const vec3d& GetCenter() const { return Center; }
    void Init(unsigned char* RGB, GLenum format);
  public:
    TLegend(TGlRenderer& Render, const olxstr& collectionName);
    void SetData(unsigned char* RGB, GLsizei width, GLsizei height,
      GLenum format);
    void Create(const olxstr& cName = EmptyString());
    void SetMaterial(const TGlMaterial &m) { GlM = m; }
    void Fit();
    virtual bool Orient(TGlPrimitive& P);
    virtual bool GetDimensions(vec3d &, vec3d &) { return false; }
    bool OnMouseUp(const IOlxObject *Sender, const TMouseData& Data);
    void UpdateLabel() { Fit(); }
    TStrList text;
  };

  TArray3D<float>* ED;
  TArray3D<int>* ColorData;
  // if mask is specified
  FractMask* Mask;
  GLuint PListId, NListId;
  char *TextData, *LegendData;
  array_2d<float> ContourData;
  float *ContourCrds[2], *ContourLevels;
  size_t ContourLevelCount;
  class TGXApp * XApp;
  void DeleteObjects();
  GLuint TextIndex;
  static TXGrid* Instance;
  void RescaleSurface(bool collect_only);
  TGlTextBox *Info;
  TLegend *Legend;
  short RenderMode;
  bool Extended, Boxed, Loading_;
  vec3f ExtMin, ExtMax;
  TGlPrimitive* glpP, *glpN, *glpC;
  // these will keep the masked objects
  TArrayList<vec3f_alist> vertices, normals;
  TArrayList<TArrayList<IsoTriangle> > triangles;
  TArrayList<TArrayList<int> > colors;
  // used by RescaleSurface when collecting rather than rendering
  olx_object_ptr<TTypeList<vec3f> > cp_vertices, cn_vertices;
protected:
  float MaxVal, MinVal, Depth, Size, Scale;
  float box_min, box_step;
  size_t MaxX, MaxY, MaxZ, MaxDim;
  float MinHole, MaxHole;  // the values of scale to skip
  int LastMouseX, LastMouseY;
  bool MouseDown;
  void DoSmooth();
  void GlLine(float x1, float y1, float x2, float y2, float z);
  int GetPolygonMode() const {
    return RenderMode == planeRenderModeFill ? GL_FILL :
      (RenderMode == planeRenderModeLine ? GL_LINE :
      (RenderMode == planeRenderModePoint ? GL_POINT : -1));
  }
  bool Is3D() const {
    return RenderMode == planeRenderModeFill ||
      RenderMode == planeRenderModeLine ||
      RenderMode == planeRenderModePoint;
  }
  // updates the text information regarding current map
  void UpdateInfo();
  class TContextClear : public AActionHandler {
  public:
    TContextClear(TGlRenderer& Render);
    virtual ~TContextClear() {}
    bool Enter(const IOlxObject *Sender, const IOlxObject *Data, TActionQueue *);
    bool Exit(const IOlxObject *Sender, const IOlxObject *Data, TActionQueue *);
  };
  static void _ResetLists() {
    if (Instance != 0) {
      Instance->PListId = Instance->NListId = ~0;
    }
  }
public:
  TXGrid(const olxstr& collectionName, TGXApp* xapp);
  virtual ~TXGrid();
  void Clear();
  TArray3D<float>* Data() { return ED; }
  const TArray3D<float>* Data() const { return ED; }
  bool LoadFromFile(const olxstr& GridFile);

  void InitIso();
  void InitGrid(size_t maxX, size_t maxY, size_t MaxZ, bool colors=false);
  void InitGrid(const vec3s& dim) { InitGrid(dim[0], dim[1], dim[2]); }
  void SetValue(size_t i, size_t j, size_t k, float v,
    uint32_t cl = ~0)
  {
    ED->Data[i][j][k] = v;
    if (ColorData != 0) {
      ColorData->Data[i][j][k] = cl;
    }
  }
  double GetValue(int i, int j, int k) const {
    return ED->Data[i][j][k];
  }
  template <class T>
  void SetValue(const T& ind, float v, uint32_t cl=~0) {
    ED->Data[(int)ind[0]][(int)ind[1]][(int)ind[2]] = v;
    if (ColorData != 0) {
      ColorData->Data[(int)ind[0]][(int)ind[1]][(int)ind[2]] = cl;
    }
  }
  template <class T> float GetValue(const T& v) const {
    return ED->Data[(int)v[0]][(int)v[1]][(int)v[2]];
  }

  // copies the 0yz x0z and xy0 layers to Maxyz xMaxyz and xyMaxZ
  void AdjustMap();
  virtual void Create(const olxstr& cName = EmptyString());

  virtual bool Orient(TGlPrimitive& P);
  virtual bool GetDimensions(vec3d& Max, vec3d& Min);

  void SetScale(float v);
  double GetScale() const { return Scale; }
  // this object will be deleted
  void SetMask(FractMask& fm) { Mask = &fm; }

  // extends the grid by +-1
  bool IsExtended() const { return Extended; }
  void SetExtended(bool v);

  void SetDepth(float v);
  void SetDepth(const vec3d& v);
  float GetDepth() const { return Depth; }
  vec3s GetDimVec() const { return vec3s(MaxX, MaxY, MaxZ); }
  size_t GetPlaneSize() const { return MaxDim; }
  /* v=2^n values are acepted only (64, 128, 256, etc to be compatible with
  textures)
  */
  void SetPlaneSize(size_t v);
  float GetSize() const { return Size; }

  DefPropP(float, MinHole);
  DefPropP(float, MaxHole);
  DefPropP(float, MinVal);
  DefPropP(float, MaxVal);

  bool IsEmpty() const { return ED == 0; }
  short GetRenderMode() const { return RenderMode; }

  size_t GetContourLevelCount() const { return ContourLevelCount; }
  // sets new number of contours...
  void SetContourLevelCount(size_t v);

  virtual void SetVisible(bool On) {
    AGDrawObject::SetVisible(On);
    Info->SetVisible(On);
    Legend->SetVisible(On);
    if (!On) {
      Clear();
    }
  }

  bool OnMouseDown(const IOlxObject *Sender, const TMouseData& Data);
  bool OnMouseUp(const IOlxObject *Sender, const TMouseData& Data);
  bool OnMouseMove(const IOlxObject *Sender, const TMouseData& Data);

  inline static TXGrid* GetInstance() { return Instance; }

  void LibScale(const TStrObjList& Params, TMacroData& E);
  void LibFix(const TStrObjList& Params, TMacroData& E);
  void LibExtended(const TStrObjList& Params, TMacroData& E);
  void LibSize(const TStrObjList& Params, TMacroData& E);
  void LibDepth(const TStrObjList& Params, TMacroData& E);
  void LibMaxDepth(const TStrObjList& Params, TMacroData& E);
  void LibContours(const TStrObjList& Params, TMacroData& E);
  void LibPlaneSize(const TStrObjList& Params, TMacroData& E);
  void LibGetMin(const TStrObjList& Params, TMacroData& E);
  void LibGetMax(const TStrObjList& Params, TMacroData& E);
  void LibRenderMode(const TStrObjList& Params, TMacroData& E);
  void LibIsvalid(const TStrObjList& Params, TMacroData& E);
  void LibSplit(TStrObjList &Cmds, const TParamList &Options,
    TMacroData &Error);
  class TLibrary*  ExportLibrary(const olxstr& name = EmptyString());
#ifdef _PYTHON
  static olxcstr &ModuleName();
  static PyObject *PyInit();
#endif
  void ToDataItem(TDataItem& item, IOutputStream& zos) const;
  void FromDataItem(const TDataItem& item, IInputStream& zis);
  // splits this object into separate blobs (1 - p, 2 - n, 3 - n+p)
  TPtrList<TXBlob>::const_list_type CreateBlobs(int flag);
  const_strlist ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const;
protected:
  struct TPlaneCalculationTask : public TaskBase {
    TXGrid& parent;
    olx_array::array_2d<float> &data;
    olx_array::array_3d<float> &src_data;
    char *text_data;
    const mat3f &proj_m, &c2c;
    const vec3f& center;
    float minVal, maxVal, size, depth, hh;
    size_t max_dim;
    short mode;
    MapUtil::MapGetter<float, 2> map_getter;
    void Run(size_t index);
    TPlaneCalculationTask(TXGrid& _parent,
      olx_array::array_3d<float> &_src_data,
      olx_array::array_2d<float> &_data,
      char* _text_data, size_t _max_dim, float _size,
      float _depth, const mat3f& _proj_m, const mat3f& _c2c,
      const vec3f& _center, short _mode) :
      parent(_parent),
      data(_data), src_data(_src_data), text_data(_text_data),
      proj_m(_proj_m), c2c(_c2c), center(_center),
      minVal(1000), maxVal(-1000),
      size(_size), depth(_depth), hh((float)_max_dim / 2), max_dim(_max_dim),
      mode(_mode),
      map_getter(src_data)
    {}
    TPlaneCalculationTask* Replicate() const {
      return new TPlaneCalculationTask(parent, src_data, data, text_data,
        max_dim, size, depth, proj_m, c2c, center, mode);
    }
  };
};

EndGxlNamespace()
#endif
