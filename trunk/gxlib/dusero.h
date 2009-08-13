#ifndef duserobjH
#define duserobjH
#include "gxbase.h"
#include "glmouselistener.h"
#include "ematrix.h"

BeginGxlNamespace()

class TDUserObj: public TGlMouseListener  {
  short Type;
  TArrayList<vec3f>* Vertices, *Normals;
  TGlMaterial GlM;
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
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;  }
};


EndGxlNamespace()
#endif
