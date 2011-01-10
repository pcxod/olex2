#ifndef duserobjH
#define duserobjH
#include "gxbase.h"
#include "glmousehandler.h"
#include "ematrix.h"

BeginGxlNamespace()

class TDUserObj: public AGlMouseHandlerImp  {
  short Type;
  TArrayList<vec3f>* Vertices, *Normals;
  TGlMaterial GlM;
protected:
  TEBasis Basis;
  virtual bool DoTranslate(const vec3d& t) {  Basis.Translate(t);  return true;  }
  virtual bool DoRotate(const vec3d& vec, double angle) {  Basis.Rotate(vec, angle);  return true;  }
  virtual bool DoZoom(double zoom, bool inc)  {
    if( inc )  Basis.SetZoom(ValidateZoom(Basis.GetZoom() + zoom));
    else       Basis.SetZoom(ValidateZoom(zoom));
    return true;
  }
public:
  TDUserObj(TGlRenderer& Render, short type, const olxstr& collectionName);
  virtual ~TDUserObj()  {  
    if( Vertices != NULL )   delete Vertices;
    if( Normals != NULL )   delete Normals;
  }
  void SetVertices(TArrayList<vec3f>* vertices)  {  Vertices = vertices;  }
  void SetNormals(TArrayList<vec3f>* normals)    {  Normals = normals;  }
  void SetMaterial(const olxstr& mat)  {  GlM.FromString(mat);  }
  void SetMaterial(const TGlMaterial& glm)  {  GlM = glm;  }
  void Create(const olxstr& cName = EmptyString);
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;  }
};

EndGxlNamespace()
#endif
