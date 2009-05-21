//---------------------------------------------------------------------------

#ifndef glprimitiveH
#define glprimitiveH
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

class TGlFont;
class TGPCollection;
class TGlrenderer;

class AEvaluator  { // a prototype for the calculation of expressions
  int FMinX, FMaxX, FMinY, FMaxY;
public:
  virtual void Eval(double X, double Y, double &Z, int &Color) = 0;

  inline int MaxX() const {  return FMaxX;  }
  inline int MaxY() const {  return FMaxY;  }
  inline int MinX() const {  return FMinX;  }
  inline int MinY() const {  return FMinY;  }

  inline void MaxX(int v)  {  FMaxX = v;  }
  inline void MaxY(int v)  {  FMaxY = v;  }
  inline void MinX(int v)  {  FMinX = v;  }
  inline void MinY(int v)  {  FMinY = v;  }

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
  /* if Basis is NULL, then the orientation od the primitive is default
   to use asign it a value Basis = new TEBasis; the object will free memory
   automatically
  */
  TEBasis* Basis;
  void CreateQuadric();
  int ListId, 
      TextureId, 
      OwnerId, 
      QuadricDrawStyle, 
      QuadricNormals, 
      QuadricOrientation;
  void SetType(short T);
public:
  TGlPrimitive(TObjectGroup& ParentG, TGlRenderer& ParentR, short type);
  ~TGlPrimitive();

  TGlRenderer& GetRenderer()  const {  return Renderer;  }
  void Compile();
  void Draw();

  DefPropP(int, ListId)
  DefPropP(int, TextureId)
  DefPropP(int, OwnerId) // to be used by the owner of the object

  DefPropP(int, QuadricDrawStyle)
  DefPropP(int, QuadricNormals)
  DefPropP(int, QuadricOrientation)


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
  
  inline void CallList(TGlPrimitive* GlP)  {
    if( GlP->IsList() )
      glCallList(GlP->GetListId()); 
    else
      GlP->Draw();
  }
  inline void CallList(int i)  {  glCallList(i); };
  inline void StartList()  {
    if( ListId == -1 )
      throw TInvalidArgumentException(__OlxSourceInfo, "ListId");
    glNewList(ListId, GL_COMPILE);
  }
  inline void EndList()  { glEndList();  }
  inline bool IsCompiled() const {  return Compiled;  }
  inline bool IsList()     const {  return Type == sgloCommandList;  }

  /* the data and parameters of the primitive */
  ematd Data;
  /* Params:
   Disk: inner radius, outer radius, slices, loops
   DiskSlice: inner radius, outer radius, slices, loops, start angle, sweep angle
   Cylinder:  base radius, top radius, height, slicesm stacks
   Sphere: radius, slices, stacks
   Text(4): [0]<0 - bitmap font, [0] > 0 - ttf, [1]-zoom x, [2]-zoom y, [3]-zoom z
   note that the String must be initialised with a pointer to a proper string
  */
  evecd Params;
};

// a structure to describe primitive parameters
struct TGlPrimitiveParams  {
  TGlPrimitive *GlP;
  TStrList Params;
};

EndGlNamespace()
#endif

