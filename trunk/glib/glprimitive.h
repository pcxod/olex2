//---------------------------------------------------------------------------

#ifndef glprimitiveH
#define glprimitiveH
#include "glbase.h"
#include "groupobj.h"
#include "glclipplane.h"
#include "ematrix.h"
#include "vpoint.h"
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

class AEvaluator // a prototype for the calculation of expressions
{
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

class TGlPrimitive: public AGroupObject
{
  class TGlRender *FParentRender;
  olxstr  FName;
protected:
  GLUquadricObj *FQuadric;
  bool FList, FCompiled;
  short FType;
  AGOProperties *NewProperties();
  TGlClipPlanes *FGlClipPlanes;
  AEvaluator *FEval;
  class TGPCollection *FParentCollection;
  olxstr *FString; // for text object, nut be initialised with a proper pointer
  TEBasis *FBasis;
  class TGlFont *FFont;
  /* if Basis is NULL, then the orientation od the primitive is default
   to use asign it a value Basis = new TEBasis; the object will free memory
   automatically
  */
  void CreateQuadric();
  /* the data and parameters of the primitive */
  TMatrixD FData;
  /* Params:
   Disk: inner radius, outer radius, slices, loops
   DiskSlice: inner radius, outer radius, slices, loops, start angle, sweep angle
   Cylinder:  base radius, top radius, height, slicesm stacks
   Sphere: radius, slices, stacks
   Text(4): [0]<0 - bitmap font, [0] > 0 - ttf, [1]-zoom x, [2]-zoom y, [3]-zoom z
   note that the String must be initialised with a pointer to a proper string
  */
  TVectorD FParams;
  int FId, FTexture, FQuadricDrawStyle, FQuadricNormals, FQuadricOrientation;
public:
  TGlPrimitive(TObjectGroup *ParentG, TGlRender *ParentR);
  ~TGlPrimitive();

  TGlRender* Render()       {  return FParentRender; };
  void Compile();
  void Draw();

  /* sets the type and expands Params vector*/
  void Type(short T);

  int Id() const            {  return FId;  }
  void Id(int v)            {  FId = v;  }
  int Texture() const       {  return FTexture;  }
  void Texture(int v)       {  FTexture = v;  }
  int QuadricDrawStyle() const      {  return FQuadricDrawStyle;  }
  void QuadricDrawStyle(int v)      {  FQuadricDrawStyle = v;  }
  int QuadricNormals() const        {  return FQuadricNormals;  }
  void QuadricNormals(int v)        {  FQuadricNormals = v;  }
  int QuadricOrientation() const    {  return FQuadricOrientation;  }
  void QuadricOrientation(int v)    {  FQuadricOrientation = v;  }

  TMatrixD& Data()    {  return FData;  }
  TVectorD& Params()  {  return FParams;  }

  /* fills the list woth parameter names */
  virtual void ListParams(TStrList &List);

  short Type() const        {  return FType; };

  void GlClipPlanes(TGlClipPlanes *GCP)  {  FGlClipPlanes = GCP; }
  TGlClipPlanes *GlClipPlanes()          {  return FGlClipPlanes; }

  void String(olxstr *P)               { FString = P; }
  olxstr *String()                     {  return FString; }

  void Font(TGlFont *Fnt)                { FFont = Fnt; }
  TGlFont *Font()                  const {  return FFont; }

  void Basis(TEBasis *p)                 {  FBasis = p; }
  TEBasis *Basis()                       { return FBasis; }

  void Evaluator(AEvaluator *Ev)         {  FEval = Ev; }
  AEvaluator *Evaluator()                {  return FEval; }

  void ParentCollection(TGPCollection *PGC){ FParentCollection = PGC; }
  TGPCollection *ParentCollection()        {  return FParentCollection; }

  const olxstr&  Name()         {  return FName; }
  void Name(const olxstr &name) {  FName = name; }

  AGOProperties * SetProperties( const AGOProperties *C);
  // commandList interface
  void GlTranslate( float _x, float _y, float _z);

  template <class T> void GlTranslate( const TVPoint<T>& trans)  {
    FParentRender->GlTranslate(trans);
  }

  void GlRotate( float Angle, float _x, float _y, float _z);
  void GlOrient(const float *m);
  void CallList(TGlPrimitive *GlP);
  void CallList( int i )          {  glCallList(i); };
  void StartList();
  void EndList();
  bool Compiled() const;
};

// a structure to describe primitive parameters
struct TGlPrimitiveParams  {
  TGlPrimitive *GlP;
  TStrList Params;
};

EndGlNamespace()
#endif

