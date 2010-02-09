#ifndef __olx_gl_primitive_H
#define __olx_gl_primitive_H
#include "glmaterial.h"
#include "glclipplane.h"
#include "ematrix.h"
#include "ebasis.h"

BeginGlNamespace()

// primitve types
const short sgloPoints    = 1,
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
class TGlrenderer;

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
  olxstr* String; // for text object, nut be initialised with a proper pointer
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
    DrawVertex(Vertices[++i], Colors[i]);
  }
  void DrawVertex2t(size_t i) const {
    DrawVertex(Vertices[i], TextureCrds[i]);
    DrawVertex(Vertices[++i], TextureCrds[i]);
  }
  void DrawVertex2ct(size_t i) const {
    DrawVertex(Vertices[i], Colors[i], TextureCrds[i]);
    DrawVertex(Vertices[++i], Colors[i], TextureCrds[i]);
  }
  void DrawVertex3(size_t i) const {
    DrawVertex(Vertices[i]);
    DrawVertex(Vertices[i+1]);
    DrawVertex(Vertices[i+2]);
  }
  void DrawVertex3c(size_t i) const {
    DrawVertex(Vertices[i], Colors[i]);
    DrawVertex(Vertices[++i], Colors[i]);
    DrawVertex(Vertices[++i], Colors[i]);
  }
  void DrawVertex3t(size_t i) const {
    DrawVertex(Vertices[i], TextureCrds[i]);
    DrawVertex(Vertices[++i], TextureCrds[i]);
    DrawVertex(Vertices[++i], TextureCrds[i]);
  }
  void DrawVertex3ct(size_t i) const {
    DrawVertex(Vertices[i], Colors[i], TextureCrds[i]);
    DrawVertex(Vertices[++i], Colors[i], TextureCrds[i]);
    DrawVertex(Vertices[++i], Colors[i], TextureCrds[i]);
  }
  void DrawVertex4(size_t i) const {
    DrawVertex(Vertices[i]);
    DrawVertex(Vertices[i+1]);
    DrawVertex(Vertices[i+2]);
    DrawVertex(Vertices[i+3]);
  }
  void DrawVertex4c(size_t i) const {
    DrawVertex(Vertices[i], Colors[i]);
    DrawVertex(Vertices[++i], Colors[i]);
    DrawVertex(Vertices[++i], Colors[i]);
    DrawVertex(Vertices[++i], Colors[i]);
  }
  void DrawVertex4t(size_t i) const {
    DrawVertex(Vertices[i], TextureCrds[i]);
    DrawVertex(Vertices[++i], TextureCrds[i]);
    DrawVertex(Vertices[++i], TextureCrds[i]);
    DrawVertex(Vertices[++i], TextureCrds[i]);
  }
  void DrawVertex4ct(size_t i) const {
    DrawVertex(Vertices[i], Colors[i], TextureCrds[i]);
    DrawVertex(Vertices[++i], Colors[i], TextureCrds[i]);
    DrawVertex(Vertices[++i], Colors[i], TextureCrds[i]);
    DrawVertex(Vertices[++i], Colors[i], TextureCrds[i]);
  }
public:
  TGlPrimitive(TObjectGroup& ParentG, TGlRenderer& ParentR, short type);
  ~TGlPrimitive();

  TGlRenderer& GetRenderer()  const {  return Renderer;  }
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
  DefPropP(olxstr*, String)
  DefPropP(TGlFont*, Font)
  DefPropP(TEBasis*, Basis)
  DefPropP(AEvaluator*, Evaluator)
  DefPropP(TGPCollection*, ParentCollection)

  DefPropC(olxstr, Name)

  TGlMaterial& GetProperties() const {  return (TGlMaterial&)AGroupObject::GetProperties();  }
  AGOProperties& SetProperties(const AGOProperties& C);

  static void PrepareColorRendering(uint16_t _begin)  {
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glBegin(_begin);
  }
  static void EndColorRendering()  {
    glEnd();
    glPopAttrib();
  }

  void CallList(TGlPrimitive* GlP)  {
    if( GlP->IsList() )
      glCallList(GlP->GetListId()); 
    else
      GlP->Draw();
  }
  static void CallList(GLuint i)  {  glCallList(i); };
  void StartList()  {
    if( ListId == ~0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "ListId");
    glNewList(ListId, GL_COMPILE);
  }
  static void EndList()  { glEndList();  }
  bool IsCompiled() const {  return Compiled;  }
  bool IsList() const {  return Type == sgloCommandList;  }

  struct TextureCrd  {
    float s, t, r, q;
    TextureCrd() : s(0), r(0), t(0), q(1)  {}
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

  static void SetColor(const uint32_t& cl)  {
    glColor4f( (float)GetRValue(cl)/255, (float)GetGValue(cl)/255, (float)GetBValue(cl)/255, (float)GetAValue(cl)/255 );
  }
  static void SetNormal(const vec3d& v)   {  glNormal3dv(v.GetData());  }
  static void SetNormal(const vec3f& v)   {  glNormal3fv(v.GetData());  }
  static void SetTexCrd(const TextureCrd& c)  {  glTexCoord4d(c.s, c.t, c.r, c.q);  }

  static void DrawVertex(const vec3d& v)  {  glVertex3dv(v.GetData());  }
  static void DrawVertex(const vec3f& v)  {  glVertex3fv(v.GetData());  }

  template <class vec_class> 
  static void DrawVertex(const vec_class& v, const uint32_t& c)  {  
    SetColor(c);
    DrawVertex(v);
  }
  template <class vec_class> 
  static void DrawVertex(const vec_class& v, const uint32_t& c, const TextureCrd& tc)  {  
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

