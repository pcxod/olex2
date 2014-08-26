/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_primitive_H
#define __olx_gl_primitive_H
#include "glmaterial.h"
#include "glclipplane.h"
#include "ematrix.h"
#include "ebasis.h"
BeginGlNamespace()

// primitve types
const short
  sgloPoints    = 1,
  sgloLines     = 2,
  sgloLineStrip = 3,
  sgloLineLoop  = 4,
  sgloTriangles = 5,
  sgloQuads     = 6,
  sgloPolygon   = 7,
  sgloEval      = 8,
  sgloDisk      = 9,
  sgloDiskSlice = 10,
  sgloCylinder  = 11,
  sgloSphere    = 12,
  sgloText      = 13,
  sgloMacro     = 14,
  sgloCommandList = 15;
// data format
const short
  glpdVertexCrd    = 0x0001,
  glpdVertexColor  = 0x0002,
  glpdEdgeColor    = 0x0002,
  glpdNormalCrd    = 0x0004,
  glpdTextureCrd   = 0x0008;


class TGlFont;
class TGPCollection;
class TGlRenderer;

class AEvaluator  { // a prototype for the calculation of expressions
  int FMinX, FMaxX, FMinY, FMaxY;
public:
  virtual void Eval(double X, double Y, double &Z, int &Color) = 0;

  int MaxX() const {  return FMaxX;  }
  int MaxY() const {  return FMaxY;  }
  int MinX() const {  return FMinX;  }
  int MinY() const {  return FMinY;  }

  void MaxX(int v)  {  FMaxX = v;  }
  void MaxY(int v)  {  FMaxY = v;  }
  void MinX(int v)  {  FMinX = v;  }
  void MinY(int v)  {  FMinY = v;  }

};

class TGlPrimitive: public AGroupObject  {
  TGlRenderer& Renderer;
  olxstr  Name;
protected:
  GLUquadricObj* Quadric;
  bool Compiled;
  short Type;
  virtual AGOProperties* NewProperties() const {  return new TGlMaterial;  }
  TGlClipPlanes* ClipPlanes;
  AEvaluator* Evaluator;
  TGPCollection* ParentCollection;
  const olxstr* String; // for text object
  TGlFont* Font;
  /* if Basis is NULL, then the orientation of the primitive is default
   to use asign it a value Basis = new TEBasis; the object will free memory
   automatically
  */
  TEBasis* Basis;
  void CreateQuadric();
  GLuint ListId,
         TextureId,
         OwnerId,
         QuadricDrawStyle,
         QuadricNormals,
         QuadricOrientation;

  void SetType(short T);

  // helper functions
  void DrawVertex2(size_t i) const {
    DrawVertex(Vertices[i]);
    DrawVertex(Vertices[i+1]);
  }
  void DrawVertex2c(size_t i) const {
    DrawVertex(Vertices[i], Colors[i]);
    DrawVertex(Vertices[i+1], Colors[i+1]);
  }
  void DrawVertex2t(size_t i) const {
    DrawVertex(Vertices[i], TextureCrds[i]);
    DrawVertex(Vertices[i+1], TextureCrds[i+1]);
  }
  void DrawVertex2ct(size_t i) const {
    DrawVertex(Vertices[i], Colors[i], TextureCrds[i]);
    DrawVertex(Vertices[i+1], Colors[i+1], TextureCrds[i+1]);
  }
  void DrawVertex3(size_t i) const {
    for (int j=0; j < 3; j++)
      DrawVertex(Vertices[i+j]);
  }
  void DrawVertex3c(size_t i) const {
    for (int j=0; j < 3; j++)
      DrawVertex(Vertices[i+j], Colors[i+j]);
  }
  void DrawVertex3t(size_t i) const {
    for (int j=0; j < 3; j++)
      DrawVertex(Vertices[i+j], TextureCrds[i+j]);
  }
  void DrawVertex3ct(size_t i) const {
    for (int j=0; j < 3; j++)
      DrawVertex(Vertices[i+j], Colors[i+j], TextureCrds[i+j]);
  }
  void DrawVertex4(size_t i) const {
    for (int j=0; j < 4; j++)
      DrawVertex(Vertices[i+j]);
  }
  void DrawVertex4c(size_t i) const {
    for (int j=0; j < 4; j++)
      DrawVertex(Vertices[i+j], Colors[i+j]);
  }
  void DrawVertex4t(size_t i) const {
    for (int j=0; j < 4; j++)
      DrawVertex(Vertices[i+j], TextureCrds[i+j]);
  }
  void DrawVertex4ct(size_t i) const {
    for (int j=0; j < 4; j++)
      DrawVertex(Vertices[i+j], Colors[i+j], TextureCrds[i+j]);
  }
  // creates a triangle based polyhedra
  void TriangularFromEdges(const vec3d *edges, size_t count, double sz,
    const TTypeList<vec3s> &faces);
public:
  TGlPrimitive(TObjectGroup& ParentG, TGlRenderer& ParentR, short type);
  ~TGlPrimitive();

  TGlRenderer& GetRenderer() const {  return Renderer;  }
  void Compile();
  void Draw();

  DefPropP(GLuint, ListId)
  DefPropP(GLuint, TextureId)
  DefPropP(GLuint, OwnerId) // to be used by the owner of the object

  DefPropP(GLuint, QuadricDrawStyle)
  DefPropP(GLuint, QuadricNormals)
  DefPropP(GLuint, QuadricOrientation)


  /* fills the list woth parameter names */
  virtual void ListParams(TStrList& List);

  /* sets the type and expands Params vector*/
  short GetType() const {  return Type;  }

  DefPropP(TGlClipPlanes*, ClipPlanes)
  DefPropP(const olxstr*, String)
  DefPropP(TGlFont*, Font)
  DefPropP(TEBasis*, Basis)
  DefPropP(AEvaluator*, Evaluator)
  DefPropP(TGPCollection*, ParentCollection)

  DefPropC(olxstr, Name)

  TGlMaterial& GetProperties() const {
    TGlMaterial &m = (TGlMaterial&)AGroupObject::GetProperties();
    if (&m == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "uninitialised properties");
    }
    return m;
  }
  AGOProperties& SetProperties(const AGOProperties& C);

  void PrepareColorRendering(uint16_t _begin) const;
  void EndColorRendering() const;
  /* the primitive must have sgloTriangles type. sz = height of the tetrahedron
  */
  void MakeTetrahedron(double sz);
  void MakeOctahedron(double sz);
  static void CallList(TGlPrimitive* GlP)  {
    if( GlP->IsList() )
      olx_gl::callList(GlP->GetListId());
    else
      GlP->Draw();
  }
  static void CallList(GLuint i)  {  olx_gl::callList(i);  }
  void StartList()  {
    if( !olx_is_valid_index(ListId) )
      throw TInvalidArgumentException(__OlxSourceInfo, "ListId");
    olx_gl::newList(ListId, GL_COMPILE);
  }
  static void EndList()  { olx_gl::endList();  }
  bool IsCompiled() const {  return Compiled;  }
  // for internal use mostly
  bool IsCompilable() const;
  bool IsList() const {  return Type == sgloCommandList;  }

  struct TextureCrd  {
    float s, t, r, q;
    TextureCrd() : s(0), t(0), r(0), q(1)  {}
  };
  TArrayList<vec3f> Vertices, Normals;
  TArrayList<TextureCrd> TextureCrds;
  TArrayList<uint32_t> Colors;
  /* Params:
   Disk: inner radius, outer radius, slices, loops
   DiskSlice: inner radius, outer radius, slices, loops, start angle, sweep angle
   Cylinder:  base radius, top radius, height, slicesm stacks
   Sphere: radius, slices, stacks
   Text(4): [0]<0 - bitmap font, [0] > 0 - ttf, [1]-zoom x, [2]-zoom y, [3]-zoom z
   note that the String must be initialised with a pointer to a proper string
  */
  evecd Params;

  void SetColor(const uint32_t& cl) const;
  static void SetNormal(const vec3d& v)   {  olx_gl::normal(v);  }
  static void SetNormal(const vec3f& v)   {  olx_gl::normal(v);  }
  static void SetTexCrd(const TextureCrd& c)  {
    olx_gl::texCoord(c.s, c.t, c.r, c.q);
  }

  static void DrawVertex(const vec3d& v)  {  olx_gl::vertex(v);  }
  static void DrawVertex(const vec3f& v)  {  olx_gl::vertex(v);  }

  template <class vec_class>
  void DrawVertex(const vec_class& v, const uint32_t& c) const {
    SetColor(c);
    DrawVertex(v);
  }
  template <class vec_class>
  void DrawVertex(const vec_class& v, const uint32_t& c,
    const TextureCrd& tc) const
  {
    SetColor(c);
    SetTexCrd(tc);
    DrawVertex(v);
  }
  template <class vec_class>
  static void DrawVertex(const vec_class& v, const TextureCrd& tc)  {
    SetTexCrd(tc);
    DrawVertex(v);
  }
};

// a structure to describe primitive parameters
struct TGlPrimitiveParams  {
  TGlPrimitive *GlP;
  TStrList Params;
};

EndGlNamespace()
#endif
