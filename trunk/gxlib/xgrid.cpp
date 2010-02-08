//----------------------------------------------------------------------------//
// TXGrid
// (c) Oleg V. Dolomanov, 2006
//----------------------------------------------------------------------------//
#include "xgrid.h"
#include "gpcollection.h"

#include "styles.h"
#include "glmaterial.h"
#include "glrender.h"
#include "efile.h"
#include "gxapp.h"
#include "library.h"
#include "conrec.h"

#ifndef _NO_PYTHON
  #include "pyext.h"
#endif

TXGrid* TXGrid::Instance = NULL;

void TXGrid::DrawQuad16(double points[4][4])  {
  double p[3][3][4];

  p[0][0][0] = points[0][0];
  p[0][0][1] = points[0][1];
  p[0][0][2] = points[0][2];
  p[0][0][3] = points[0][3];

  p[0][2][0] = points[1][0];
  p[0][2][1] = points[1][1];
  p[0][2][2] = points[1][2];
  p[0][2][3] = points[1][3];

  p[2][2][0] = points[2][0];
  p[2][2][1] = points[2][1];
  p[2][2][2] = points[2][2];
  p[2][2][3] = points[2][3];

  p[2][0][0] = points[3][0];
  p[2][0][1] = points[3][1];
  p[2][0][2] = points[3][2];
  p[2][0][2] = points[3][3];

  p[1][0][0] = (points[0][0]+points[3][0])/2;
  p[1][0][1] = (points[0][1]+points[3][1])/2;
  p[1][0][2] = (points[0][2]+points[3][2])/2;
  p[1][0][3] = (points[0][3]+points[3][3])/2;

  p[1][1][0] = (points[0][0]+points[1][0]+points[2][0]+points[3][0])/4;
  p[1][1][1] = (points[0][1]+points[1][1]+points[2][1]+points[3][1])/4;
  p[1][1][2] = (points[0][2]+points[1][2]+points[2][2]+points[3][2])/4;
  p[1][1][3] = (points[0][2]+points[1][2]+points[2][2]+points[3][2])/4;

  p[0][1][0] = (points[0][0]+points[1][0])/2;
  p[0][1][1] = (points[0][1]+points[1][1])/2;
  p[0][1][2] = (points[0][2]+points[1][2])/2;
  p[0][1][3] = (points[0][3]+points[1][3])/2;

  p[1][2][0] = (points[1][0]+points[2][0])/2;
  p[1][2][1] = (points[1][1]+points[2][1])/2;
  p[1][2][2] = (points[1][2]+points[2][2])/2;
  p[1][2][3] = (points[1][3]+points[2][3])/2;

  p[2][1][0] = (points[2][0]+points[3][0])/2;
  p[2][1][1] = (points[2][1]+points[3][1])/2;
  p[2][1][2] = (points[2][2]+points[3][2])/2;
  p[2][1][3] = (points[2][3]+points[3][3])/2;

  DrawQuad4( p[0][0], p[0][1], p[1][1], p[1][0] );
  DrawQuad4( p[0][1], p[0][2], p[1][2], p[1][1] );
  DrawQuad4( p[1][0], p[1][1], p[2][1], p[2][0] );
  DrawQuad4( p[1][1], p[1][2], p[2][2], p[2][1] );
}

void TXGrid::DrawQuad4(double A[4], double B[4], double C[4], double D[4])  {
  double p[3][3][3], c[2][2];

  c[0][0] = A[3];
  c[0][1] = B[3];
  c[1][0] = C[3];
  c[1][1] = D[3];

  p[0][0][0] = A[0];
  p[0][0][1] = A[1];
  p[0][0][2] = A[2];

  p[0][2][0] = B[0];
  p[0][2][1] = B[1];
  p[0][2][2] = B[2];

  p[2][2][0] = C[0];
  p[2][2][1] = C[1];
  p[2][2][2] = C[2];

  p[2][0][0] = C[0];
  p[2][0][1] = C[1];
  p[2][0][2] = C[2];

  p[1][0][0] = (A[0]+D[0])/2;
  p[1][0][1] = (A[1]+D[1])/2;
  p[1][0][2] = (A[2]+D[2])/2;

  p[1][1][0] = (A[0]+B[0]+C[0]+D[0])/4;
  p[1][1][1] = (A[1]+B[1]+C[1]+D[1])/4;
  p[1][1][2] = (A[2]+B[2]+C[2]+D[2])/4;

  p[0][1][0] = (A[0]+B[0])/2;
  p[0][1][1] = (A[1]+B[1])/2;
  p[0][1][2] = (A[2]+B[2])/2;

  p[1][2][0] = (B[0]+C[0])/2;
  p[1][2][1] = (B[1]+C[1])/2;
  p[1][2][2] = (B[2]+C[2])/2;

  p[2][1][0] = (C[0]+D[0])/2;
  p[2][1][1] = (C[1]+D[1])/2;
  p[2][1][2] = (C[2]+D[2])/2;

  for(int i=0; i < 2; i++ )  {
    for(int j=0; j < 2; j++ )  {
      CalcColor( c[i][j] );
      glVertex3d(p[i][j][0], p[i][j][1], p[i][j][2]);
      glVertex3d(p[i+1][j][0], p[i+1][j][1], p[i+1][j][2]);
      glVertex3d(p[i+1][j+1][0], p[i+1][j+1][1], p[i+1][j+1][2]);
      glVertex3d(p[i][j+1][0], p[i][j+1][1], p[i][j+1][2]);
    }
  }
}
//----------------------------------------------------------------------------//
TXGrid::TXGrid(const olxstr& collectionName, TGXApp* xapp) :
                     TGlMouseListener(xapp->GetRender(), collectionName)  {
  if( Instance != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  Mask = NULL;
  Instance = this;
  Extended = false;
  RenderMode = planeRenderModeFill;
#ifndef _NO_PYTHON
  PythonExt::GetInstance()->Register(&TXGrid::PyInit);
#endif
  XApp = xapp;
  SetMove2D(false);
  SetMoveable(false);
  SetZoomable(false);
  Depth = 0;
  ED = NULL;
  IS = NULL;
  MouseDown = false;
  Size = 10;
  // texture related data
  TextIndex = ~0;
  TextData = NULL;
  // contour related data
  ContourData = NULL;
  ContourCrds[0] = ContourCrds[1] = NULL;
  ContourLevels = NULL;
  ContourLevelCount = 14;
  Scale = 10;
  //for textures, 2^n+2 (for border)...
  //MaxDim = 128;//olx_max( olx_max(MaxX,MaxY), MaxZ);
  MaxDim = 128;
  Info = new TGlTextBox(Parent, "XGrid_Legend");
  MaxX = MaxY = MaxZ = 0;
  MaxVal = MinVal = 0;
  MinHole = MaxHole = 0;
  PListId = NListId = ~0;
}
//..............................................................................
TXGrid::~TXGrid()  {
  Clear();
  DeleteObjects();
  delete Info;
}
//..............................................................................
void TXGrid::Clear()  {  DeleteObjects();  }
//..............................................................................
void TXGrid::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  TGlPrimitive& GlP = GPC.NewPrimitive("eMap", sgloQuads);
  TGlMaterial GlM;
  GlM.SetFlags(0);
  GlM.ShininessF = 128;
  GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmTransparent);
  GlM.AmbientF = 0xD80f0f0f;
  GlM.DiffuseF = 0xD80f0f0f;
  GlP.SetProperties( GS.GetMaterial(GlP.GetName(), GlM));

  TextIndex = ~0;
  GlP.SetTextureId(~0);

  GlP.Vertices.SetCount(4);
  GlP.TextureCrds.SetCount(4);
  // texture coordinates
  GlP.TextureCrds[0].s = 0;  GlP.TextureCrds[0].t = 0;
  GlP.TextureCrds[1].s = 1;  GlP.TextureCrds[1].t = 0;
  GlP.TextureCrds[2].s = 1;  GlP.TextureCrds[2].t = 1;
  GlP.TextureCrds[3].s = 0;  GlP.TextureCrds[3].t = 1;
  Info->Create();
  // create dummy primitives
  glpP = &GPC.NewPrimitive("+Surface", sgloQuads);
  glpP->SetProperties(GS.GetMaterial("+Surface", 
    TGlMaterial("85;0.000,1.000,0.000,0.850;3632300160;1.000,1.000,1.000,0.500;36")));
  glpN = &GPC.NewPrimitive("-Surface", sgloQuads);
  glpN->SetProperties(GS.GetMaterial("-Surface", 
    TGlMaterial("85;1.000,0.000,0.000,0.850;3632300160;1.000,1.000,1.000,0.500;36")));
}
//..............................................................................
void TXGrid::CalcColorRGB(float v, uint8_t& R, uint8_t& G, uint8_t& B) {
  //if( v == 0 )
  // v = MaxVal;
  float cs;
  if( Scale < 0 )  {  // show both
    cs = v/(-Scale + 0.001);
    if( cs < -0.5 )  {
      R = 255;
      G = (uint8_t)(-sin(M_PI*cs)*255);
      B = G;
    }
    else if( cs < 0 ) {
      R = (uint8_t)(-sin(M_PI*cs/2)*255);
      G = R;
      B = R;
    }
    else if( cs < 0.5 ) {
      G = (uint8_t)(sin(M_PI*cs/2)*255);
      R = G;
      B = G;
    }
    else {
      R = (uint8_t)(sin(M_PI*cs)*255);
      G = 255;
      B = R;
    }
  }
  else  {
    cs = olx_abs(v)/(Scale+0.001);
    if( cs < 0.5 ) {
      R = (uint8_t)(sin(M_PI*cs)*255);
      G = 255;
      B = 255;
    }
    else {
      R = 255;
      G = (uint8_t)(sin(M_PI*cs)*255);
      B = 0;
    }
  }
}
//..............................................................................
void TXGrid::CalcColor(float v) {
  double cs, R, G, B;
  if( v < 0 )  {
    if( v < MinVal/Scale )
      cs = -1;
    else
      cs = -v*(Scale)/MinVal;
  }
  else  {
    if( v > MaxVal/Scale )
      cs = 1;
    else
      cs = v*(Scale)/MaxVal;
  }

  if( cs < -0.5 )  {
    R = 0;
    G = -(float)sin(M_PI*cs);
    B = 1;
  }
  else if( cs < 0 ) {
    R = 0;
    G = 1;
    B = -(float)sin(M_PI*cs);
  }
  else if( cs < 0.5 ) {
    R = (float)sin(M_PI*cs);
    G = 1;
    B = 0;
  }
  else {
    R = 1;
    G = (float)sin(M_PI*cs);
    B = 0;
  }
  glColor3d(R,G,B);
}
//..............................................................................
bool TXGrid::Orient(TGlPrimitive& GlP)  {
  if( ED == NULL )  return true;
  if( IS != NULL && Is3D() )  {
    if( &GlP == glpN )  // draw once only
      glCallList(PListId);
    else if( &GlP == glpP )  // draw once only
      glCallList(NListId);
    return true;
  }
  if( &GlP == glpP || &GlP == glpN )  return true;

  const mat3f bm(Parent.GetBasis().GetMatrix());
  const mat3f c2c(XApp->XFile().GetAsymmUnit().GetCartesianToCell());
  const float hh = (float)MaxDim/2;
  const vec3f center(Parent.GetBasis().GetCenter());
  const vec3i dim(MaxX, MaxY, MaxZ);
  vec3f p1(-hh/Size, -hh/Size, Depth),
        p2(hh/Size, -hh/Size, Depth),
        p3(hh/Size, hh/Size, Depth),
        p4(-hh/Size, hh/Size, Depth);
  p1 = bm * p1;  p1 -= center;
  p2 = bm * p2;  p2 -= center;
  p3 = bm * p3;  p3 -= center;
  p4 = bm * p4;  p4 -= center;
  vec3i aa[8];
  float minVal = 1000, maxVal = -1000;
  for( int i=0; i < MaxDim; i++ )  {
    for( int j=0; j < MaxDim; j++ )  {  // (i,j,Depth)        
      vec3f p((float)(i-hh)/Size, (float)(j-hh)/Size,  Depth);
      p = bm*p;
      p -= center;
      p *= c2c;
      //
      p *= dim;
      aa[0] = vec3i(olx_round(p[0]), olx_round(p[1]), olx_round(p[2])); //x,y,z
      aa[1] = vec3i((int)(p[0]), (int)(p[1]), (int)(p[2]));  //x',y',z'
      aa[2] = vec3i(aa[1][0], aa[0][1], aa[0][2]);  // x',y,z
      aa[3] = vec3i(aa[1][0], aa[1][1], aa[0][2]);  // x',y',z
      aa[4] = vec3i(aa[1][0], aa[0][1], aa[1][2]);  // x',y,z'
      aa[5] = vec3i(aa[0][0], aa[1][1], aa[0][2]);  // x,y',z
      aa[6] = vec3i(aa[0][0], aa[1][1], aa[1][2]);  // x,y',z'
      aa[7] = vec3i(aa[0][0], aa[0][1], aa[1][2]);  // x,y,z'
      float val = 0, wght = 0;
      for( int k=0; k < 8; k++ )  {
        const float w = 1.0f-(p.QDistanceTo(aa[k])/3);
        for( int m=0; m < 3; m++ )  {
          while( aa[k][m] < 0 )
            aa[k][m] += dim[m];
          while( aa[k][m] >= dim[m] )
            aa[k][m] -= dim[m];
        }
        val += w*ED->Data[aa[k][0]][aa[k][1]][aa[k][2]];
        wght += w;
      }
      val /= wght;
      if( RenderMode == planeRenderModePlane )  {
        uint8_t R, G, B;
        CalcColorRGB(val, R, G, B);
        const int off = (i+j*MaxDim)*3; 
        TextData[off] = R;
        TextData[off+1] = G;
        TextData[off+2] = B;
      }
      else if( RenderMode == planeRenderModeContour )  {
        if( val < minVal )
          minVal = val;
        if( val > maxVal )
          maxVal = val;
        ContourData[i][j] = val;
      }
    }
  }
  GlP.Vertices[0] = p1;
  GlP.Vertices[1] = p2;
  GlP.Vertices[2] = p3;
  GlP.Vertices[3] = p4;
  if( RenderMode == planeRenderModePlane )  {
    if( !olx_is_valid_index(TextIndex) )  {
      TextIndex = Parent.GetTextureManager().Add2DTexture("Plane", 0, MaxDim, MaxDim, 0, GL_RGB, TextData);
      TGlTexture* tex = Parent.GetTextureManager().FindTexture(TextIndex);
      tex->SetEnvMode(tpeDecal);
      tex->SetSCrdWrapping(tpCrdClamp);
      tex->SetTCrdWrapping(tpCrdClamp);

      tex->SetMinFilter(tpFilterLinear);
      tex->SetMagFilter(tpFilterLinear);
      tex->SetEnabled(true);
    }
    else
      Parent.GetTextureManager().
      Replace2DTexture(*Parent.GetTextureManager().
      FindTexture(TextIndex), 0, MaxDim, MaxDim, 0, GL_RGB, TextData);

    GlP.SetTextureId(TextIndex);
    //  glNormal3d(bm[0][2], bm[1][2], bm[2][2]);
    glNormal3d(0, 0, 1);
    return false;
  }
  else  {
    glColor3f(0, 0, 0);
    Contour<float> cm;
    Contour<float>::MemberFeedback<TXGrid> mf(*this, &TXGrid::GlLine);
    float contour_step = (maxVal - minVal)/(ContourLevelCount-1);
    ContourLevels[0] = minVal;
    for( int i=1; i < ContourLevelCount; i++ )
      ContourLevels[i] = ContourLevels[i-1]+contour_step;
    cm.DoContour(ContourData, 0, MaxDim-1, 0, MaxDim-1,
      ContourCrds[0], ContourCrds[1], 
      ContourLevelCount, ContourLevels, mf);
    GlP.SetTextureId(~0);
    return true;  // do not render the plane
  }
  return false;
}
//..............................................................................
void TXGrid::GlLine(float x1, float y1, float x2, float y2, float z)  {
  vec3d p1(x1/Size, y1/Size, Depth), p2(x2/Size, y2/Size, Depth);
  p1 = Parent.GetBasis().GetMatrix()*p1 - Parent.GetBasis().GetCenter();
  p2 = Parent.GetBasis().GetMatrix()*p2 - Parent.GetBasis().GetCenter();
  glBegin(GL_LINES);
  glVertex3d(p1[0], p1[1], p1[2]);
  glVertex3d(p2[0], p2[1], p2[2]);
  glEnd();
}
//..............................................................................
bool TXGrid::GetDimensions(vec3d &Max, vec3d &Min)  {
//  Min = FCenter;
//  Max = FCenter;
  return false;
};
//..............................................................................
void TXGrid::InitGrid(int maxX, int maxY, int maxZ)  {
  DeleteObjects();
  MaxX = maxX;
  MaxY = maxY;
  MaxZ = maxZ;
  MaxVal = MinVal = 0;
  if( ED != NULL )
    delete ED;
  ED = new TArray3D<float>(0, MaxX, 0,MaxY, 0, MaxZ);
  TextData = new char[MaxDim*MaxDim*3+1];
  ContourData = new float*[MaxDim];
  ContourCrds[0] = new float[MaxDim];
  ContourCrds[1] = new float[MaxDim];
  for( int i=0; i < MaxDim; i++ )  {
    ContourData[i] = new float[MaxDim];
    ContourCrds[0][i] = ContourCrds[1][i] = (float)(i-MaxDim/2);///MaxDim;
  }
  ContourLevels = new float[ContourLevelCount];
  MaxHole = MinHole = 0;
}
//..............................................................................
void TXGrid::DeleteObjects()  {
  if( ED != NULL )  {
    delete ED;
    ED = NULL;
  }
  if( TextData != NULL )  {
    delete TextData;
    TextData = NULL;
  }
  if( IS != NULL )  {
    delete IS;
    p_triangles.Clear();
    p_normals.Clear();
    p_vertices.Clear();
    n_triangles.Clear();
    n_normals.Clear();
    n_vertices.Clear();
    IS = NULL;
  }
  if( Mask != NULL )  {
    delete Mask;
    Mask = NULL;
  }
  if( olx_is_valid_index(PListId) )  {
    glDeleteLists(PListId, 2);
    PListId = NListId = ~0;
  }
  if( ContourData != NULL )  {
    for( int i=0; i < MaxDim; i++ )
      delete [] ContourData[i];
    delete [] ContourData;
    delete [] ContourCrds[0];
    delete [] ContourCrds[1];
    delete [] ContourLevels;
    ContourData = NULL;
  }
}
//..............................................................................
bool TXGrid::LoadFromFile(const olxstr& GridFile)  {
  TEFile::CheckFileExists(__OlxSourceInfo, GridFile);
  TStrList SL, toks;
  SL.LoadFromFile(GridFile);
  toks.Strtok(SL[0], ' ');

  int vc = 3;

  InitGrid( toks[0].ToInt(),
            toks[1].ToInt(),
            toks[2].ToInt());
    
  for( int i=0; i < MaxX; i++ )  {
    for( int j=0; j < MaxY; j++ )  {
      for( int k=0; k < MaxZ; k++ )  {
        float val = (float)toks[vc].ToDouble();
        if( val > MaxVal ) MaxVal = val;
        if( val < MinVal ) MinVal = val;
        ED->Data[i][j][k] = val;
        vc++;
      }
    }
  }

  // set default depth to center of the asymmetric unit
  vec3d v( XApp->XFile().GetAsymmUnit().GetOCenter(true, true) );
  v = XApp->XFile().GetAsymmUnit().GetCellToCartesian() * v;
  SetDepth(v);
  return true;
}
//..............................................................................
void TXGrid::SetScale(float v)  {
  bool _3d = Is3D();
  if( _3d && MinHole != MaxHole )  {
    if( v >= MinHole && v <= MaxHole )
      return;
  }
  Scale = v;
  Info->Clear();
  Info->PostText( olxstr("Current level is ") << Scale);
  if( IS != NULL && _3d )  {
    p_triangles.Clear();
    p_normals.Clear();
    p_vertices.Clear();
    n_triangles.Clear();
    n_normals.Clear();
    n_vertices.Clear();
    IS->GenerateSurface(Scale);
    p_vertices = IS->VertexList();
    p_normals = IS->NormalList();
    p_triangles = IS->TriangleList();
    if( Scale < 0 )  {
      IS->GenerateSurface( -Scale );
      n_vertices = IS->VertexList();
      n_normals = IS->NormalList();
      n_triangles = IS->TriangleList();
    }
    RescaleSurface();
  }
}
//..............................................................................
void TXGrid::SetExtended(bool v)  {
  if( Extended == v )
    return;
  Extended = v;
  SetScale(Scale);
}
//..............................................................................
void TXGrid::SetDepth(float v)  {  Depth = v;  }
//..............................................................................
void TXGrid::SetDepth(const vec3d& v)  {
  vec3d p = (v + Parent.GetBasis().GetCenter())*Parent.GetBasis().GetMatrix();
  SetDepth((float)p[2]);
}
//..............................................................................
bool TXGrid::OnMouseDown(const IEObject *Sender, const TMouseData *Data)  {
  if( (Data->Shift & sssCtrl) == 0 && (Data->Shift & sssShift) == 0 )
    return false;
  LastMouseX = Data->DownX;
  LastMouseY = Data->DownY;
  MouseDown = true;
  return true;
}
//..............................................................................
bool TXGrid::OnMouseUp(const IEObject *Sender, const TMouseData *Data) {
  MouseDown = false;
  return true;
}
//..............................................................................
bool TXGrid::OnMouseMove(const IEObject *Sender, const TMouseData *Data)  {
  if( !MouseDown )  return false;

  if( (Data->Button & smbLeft) != 0 ) {
    Depth += (float)(LastMouseX - Data->X)/15;
    Depth += (float)(LastMouseY - Data->Y)/15;
    float z = (float)Parent.CalcZoom();
    if( Depth < -z/2 )
      Depth = -z/2;
    if( Depth > z/2 )
      Depth = z/2;
  }
  else  {
    if( (Data->Shift & sssShift) != 0 )  {
      double step = (MaxVal-MinVal)/250.0;
      Scale -= step*(LastMouseX - Data->X);
      Scale += step*(LastMouseY - Data->Y);
      if( olx_abs(Scale) > olx_max(MaxVal,MinVal)  )
        Scale = olx_sign(Scale)*olx_max(MaxVal,MinVal);
    }
    else  {
      Size += (float)(LastMouseX - Data->X)/15;
      Size += (float)(LastMouseY - Data->Y)/15;
      if( Size < 1 )  Size = 1;
      if( Size > 20 )  Size = 20;
    }
  }
  if( IS != NULL )  {
    SetScale(Scale);
  }
  Info->Clear();
  Info->PostText(olxstr("Current level: ") << Scale);
  LastMouseX = Data->X;
  LastMouseY = Data->Y;
  return true;
}
//..............................................................................
void TXGrid::GlContextChange()  {
  if( ED == NULL )
    return;
  if( !olx_is_valid_index(PListId)  )  {
    glDeleteLists(PListId, 2);
    PListId = NListId = ~0;
  }
  SetScale(Scale);
}
//..............................................................................
void TXGrid::RescaleSurface()  {
  const TAsymmUnit& au =  XApp->XFile().GetAsymmUnit();
  if( !olx_is_valid_index(PListId) )  {
    PListId = glGenLists(2);
    NListId = PListId+1;
  }
  if( Mask != NULL )  {
    vec3d pts[3];
    for( int li = 0; li <= 1; li++ )  {
      const TTypeList<vec3f>& verts = (li == 0 ? p_vertices : n_vertices);
      const TTypeList<vec3f>& norms = (li == 0 ? p_normals : n_normals);
      const TTypeList<IsoTriangle>& trians = (li == 0 ? p_triangles : n_triangles);
      glNewList(li == 0 ? PListId : NListId, GL_COMPILE_AND_EXECUTE);
      glPolygonMode(GL_FRONT_AND_BACK, GetPolygonMode());
      glBegin(GL_TRIANGLES);
      for( int x=-1; x <= 1; x++ )  {
        for( int y=-1; y <= 1; y++ )  {
          for( int z=-1; z <= 1; z++ )  {
            for( size_t i=0; i < trians.Count(); i++ )  {
              bool draw = true;
              for( int j=0; j < 3; j++ )  {
                pts[j] = verts[trians[i].pointID[j]];
                pts[j][0] /= MaxX;  pts[j][1] /= MaxY;  pts[j][2] /= MaxZ;
                pts[j][0] += x;     pts[j][1] += y;     pts[j][2] += z;
                if( !Mask->Get(pts[j]) )  {
                  draw = false;
                  break;
                }
                au.CellToCartesian(pts[j]);
              }
              if( !draw )  continue;
              for( int j=0; j < 3; j++ )  {
                const vec3f& nr = norms[trians[i].pointID[j]];
                glNormal3f( nr[0], nr[1], nr[2] );
                glVertex3f(pts[j][0], pts[j][1], pts[j][2]);
              }
            }
          }
        }
      }
      glEnd();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEndList();
    }
    p_vertices.Clear();
    p_triangles.Clear();
    p_normals.Clear();
    n_vertices.Clear();
    n_triangles.Clear();
    n_normals.Clear();
  }
  else  {
    if( Extended )  {
      vec3d pts[3]; // ext drawing
      for( int li = 0; li <= 1; li++ )  {
        TTypeList<vec3f>& verts = (li == 0 ? p_vertices : n_vertices);
        const TTypeList<vec3f>& norms = (li == 0 ? p_normals : n_normals);
        const TTypeList<IsoTriangle>& trians = (li == 0 ? p_triangles : n_triangles);
        glNewList(li == 0 ? PListId : NListId, GL_COMPILE_AND_EXECUTE);
        glPolygonMode(GL_FRONT_AND_BACK, GetPolygonMode());
        glBegin(GL_TRIANGLES);
        for( int x=-1; x <= 1; x++ )  {
          for( int y=-1; y <= 1; y++ )  {
            for( int z=-1; z <= 1; z++ )  {
              for( size_t i=0; i < trians.Count(); i++ )  {
                for( int j=0; j < 3; j++ )  {
                  pts[j] = verts[trians[i].pointID[j]];                      // ext drawing
                  pts[j][0] /= MaxX;  pts[j][1] /= MaxY;  pts[j][2] /= MaxZ; // ext drawing
                  pts[j][0] += x;     pts[j][1] += y;     pts[j][2] += z;    // ext drawing
                  const vec3f& nr = norms[trians[i].pointID[j]];
                  glNormal3f( nr[0], nr[1], nr[2] );
                  au.CellToCartesian(pts[j]);                                // ext drawing
                  glVertex3f(pts[j][0], pts[j][1], pts[j][2]);               // ext drawing
                }
              }
            }
          }
        }
        glEnd();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEndList();
      }
    }
    else  {
      for( int li = 0; li <= 1; li++ )  {
        TTypeList<vec3f>& verts = (li == 0 ? p_vertices : n_vertices);
        const TTypeList<vec3f>& norms = (li == 0 ? p_normals : n_normals);
        const TTypeList<IsoTriangle>& trians = (li == 0 ? p_triangles : n_triangles);
        for( size_t i=0; i < verts.Count(); i++ )  {                           // cell drawing
          verts[i][0] /= MaxX;  verts[i][1] /= MaxY;  verts[i][2] /= MaxZ;  // cell drawing
          au.CellToCartesian(verts[i]);                                     // cell drawing
        }
        glNewList(li == 0 ? PListId : NListId, GL_COMPILE_AND_EXECUTE);
        glPolygonMode(GL_FRONT_AND_BACK, GetPolygonMode());
        glBegin(GL_TRIANGLES);
        for( size_t i=0; i < trians.Count(); i++ )  {
          for( int j=0; j < 3; j++ )  {
            const vec3f& nr = norms[trians[i].pointID[j]];
            glNormal3f( nr[0], nr[1], nr[2] );
            const vec3f& p = verts[trians[i].pointID[j]];  // cell drawing
            glVertex3f(p[0], p[1], p[2]);                  // cell drawing
          }
        }
        glEnd();
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEndList();
      }
    }
    p_vertices.Clear();
    p_triangles.Clear();
    p_normals.Clear();
    n_vertices.Clear();
    n_triangles.Clear();
    n_normals.Clear();
  }
}
//..............................................................................
void TXGrid::AdjustMap()  {
  if( ED == NULL )  
    return;
  for( int i=0; i < MaxX; i++ )
    for( int j=0; j < MaxY; j++ )
        ED->Data[i][j][MaxZ] = ED->Data[i][j][0];
  for( int i=0; i < MaxX; i++ )
    for( int j=0; j < MaxZ; j++ )
        ED->Data[i][MaxY][j] = ED->Data[i][0][j];
  for( int i=0; i < MaxY; i++ )
    for( int j=0; j < MaxZ; j++ )
        ED->Data[MaxX][i][j] = ED->Data[0][i][j];
  ED->Data[MaxX][MaxY][MaxZ] = ED->Data[0][0][0];
}
//..............................................................................
void TXGrid::InitIso(bool v)  {
  if( !v )  {
    if( IS != NULL )  {
      delete IS;
      IS = NULL;
    }
  }
  else  {
    if( ED == NULL )  return;
    if( IS != NULL )  delete IS;
    IS = new CIsoSurface<float>(*ED);
    SetScale(Scale);
  }
  //Mode3D = v;
}
//..............................................................................
//..............................................................................
//..............................................................................
void TXGrid::LibExtended(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Extended);
  else
    SetExtended( Params[0].ToBool() );
}
//..............................................................................
void TXGrid::LibScale(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Scale);
  else
    SetScale(Params[0].ToFloat<float>());
}
//..............................................................................
void TXGrid::LibSize(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Size);
  else
    Size = Params[0].ToFloat<float>();
}
//..............................................................................
void TXGrid::LibIsvalid(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(ED != NULL);
}
//..............................................................................
void TXGrid::LibGetMin(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(MinVal);
}
//..............................................................................
void TXGrid::LibGetMax(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(MaxVal);
}
//..............................................................................
void TXGrid::LibRenderMode(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  {
    if( RenderMode == planeRenderModeFill )
      E.SetRetVal<olxstr>("fill");
    else if( RenderMode == planeRenderModePoint )
      E.SetRetVal<olxstr>("point");
    else if( RenderMode == planeRenderModeLine )
      E.SetRetVal<olxstr>("line");
    else if( RenderMode == planeRenderModePlane )
      E.SetRetVal<olxstr>("plane");
    else if( RenderMode == planeRenderModeContour )
      E.SetRetVal<olxstr>("contour");
    return;
  }
  int pm = RenderMode;
  if( Params[0] == "fill" )
    RenderMode = planeRenderModeFill;
  else if( Params[0] == "point" )
    RenderMode = planeRenderModePoint;
  else if( Params[0] == "line" )
    RenderMode = planeRenderModeLine;
  else if( Params[0] == "plane" )
    RenderMode = planeRenderModePlane;
  else if( Params[0] == "contour" )
    RenderMode = planeRenderModeContour;
  else throw TInvalidArgumentException(__OlxSourceInfo,
         olxstr("incorrect mode value: '") << Params[0] << '\'');
  // have to recreate
  if( pm != RenderMode )  {
    InitIso(Is3D());
  }
}
//..............................................................................
TXGrid::ContourDrawer::ContourDrawer(const olxstr& file_name) : output(file_name)  {
  //output.translate(output.GetWidth()/2, output.GetHeight()/2);
  output.scale(1, 1);
  output.color(0);
  output.lineWidth(0.1);
}
void TXGrid::ContourDrawer::draw(float x1, float y1, float x2, float y2, float z)  {
  output.moveto(x1, y1);
  output.lineto(x2, y2);
  output.stroke();
}
//..............................................................................
void TXGrid::LibWritePS(TStrObjList& Params, const TParamList& Options, TMacroError& E)  {
  if( ED == NULL )  return;
  Contour<float> cm;
  TXGrid::ContourDrawer drawer(Params[0]);
  Contour<float>::MemberFeedback<TXGrid::ContourDrawer> mf(drawer, &TXGrid::ContourDrawer::draw);

  float d_step = (float)drawer.output.GetWidth()/MaxDim;
  float **data = new float*[MaxDim];
  float *x = new float[MaxDim];
  float *y = new float[MaxDim];
  for( int i=0; i < MaxDim; i++ )  {
    data[i] = new float[MaxDim];
    y[i] = x[i] = i*d_step;
  }
  const int contour_cnt = 25;
  float z[contour_cnt], minZ = 1000, maxZ = -1000;
  const vec3i dim(MaxX, MaxY, MaxZ);
  const mat3f bm(Parent.GetBasis().GetMatrix());
  const mat3f c2c(XApp->XFile().GetAsymmUnit().GetCartesianToCell());
  const float hh = (float)MaxDim/2;
  const vec3f center(Parent.GetBasis().GetCenter());
  vec3i aa[8];
  for( int i=0; i < MaxDim; i++ )  {
    for( int j=0; j < MaxDim; j++ )  {  // (i,j,Depth)        
      vec3f p((float)(i-hh)/Size, (float)(j-hh)/Size,  Depth);
      p = bm*p;
      p -= center;
      p *= c2c;
      p *= dim;
      aa[0] = vec3i(olx_round(p[0]), olx_round(p[1]), olx_round(p[2])); //x,y,z
      aa[1] = vec3i((int)(p[0]), (int)(p[1]), (int)(p[2]));  //x',y',z'
      aa[2] = vec3i(aa[1][0], aa[0][1], aa[0][2]);  // x',y,z
      aa[3] = vec3i(aa[1][0], aa[1][1], aa[0][2]);  // x',y',z
      aa[4] = vec3i(aa[1][0], aa[0][1], aa[1][2]);  // x',y,z'
      aa[5] = vec3i(aa[0][0], aa[1][1], aa[0][2]);  // x,y',z
      aa[6] = vec3i(aa[0][0], aa[1][1], aa[1][2]);  // x,y',z'
      aa[7] = vec3i(aa[0][0], aa[0][1], aa[1][2]);  // x,y,z'
      data[i][j] = 0;
      float wght=0;
      for( int k=0; k < 8; k++ )  {
        const float w = 1.0f-(sqrt(p.QDistanceTo(aa[k])/3));
        for( int m=0; m < 3; m++ )  {
          while( aa[k][m] < 0 )
            aa[k][m] += dim[m];
          while( aa[k][m] >= dim[m] )
            aa[k][m] -= dim[m];
        }
        data[i][j] += w*ED->Data[aa[k][0]][aa[k][1]][aa[k][2]];
        wght += w;
      }
      data[i][j] /= wght;
      if( data[i][j] < minZ )  minZ = data[i][j];
      if( data[i][j] > maxZ )  maxZ = data[i][j];
    }
  }
  float contour_step = (maxZ - minZ)/(contour_cnt-1);
  z[0] = minZ;
  for( int i=1; i < contour_cnt; i++ )
    z[i] = z[i-1]+contour_step;
  cm.DoContour(data, 0, MaxDim-1, 0, MaxDim-1, x, y, contour_cnt, z, mf);
  delete [] x;
  delete [] y;
  for( int i=0; i < MaxDim; i++ )
    delete [] data[i];
  delete [] data;
}
//..............................................................................
void TXGrid::ToDataItem(TDataItem& item, IOutputStream& zos) const {
  item.AddField("empty", IsEmpty() );
  if( !IsEmpty() )  {
    //item.AddField("visible", Visible());
    item.AddField("draw_mode", RenderMode);
    item.AddField("max_val", MaxVal);
    item.AddField("min_val", MinVal);
    item.AddField("depth", Depth);
    item.AddField("size", Size);
    item.AddField("extended", Extended);
    item.AddField("scale", Scale);
    item.AddField("max_x", MaxX);
    item.AddField("max_y", MaxY);
    item.AddField("max_z", MaxZ);
    for( int x=0; x < MaxX; x++ )  {
      for( int y=0; y < MaxY; y++ )  {
        zos.Write( ED->Data[x][y], sizeof(float)*MaxZ );
      }
    }
    if( Mask != NULL && Mask->GetMask() != NULL )
      Mask->ToDataItem(item.AddItem("mask"), zos);
  }
}
//..............................................................................
void TXGrid::FromDataItem(const TDataItem& item, IInputStream& zis) {
  Clear();
  bool empty = item.GetRequiredField("empty").ToBool();
  if( empty )  return;
  //Visible( item.GetRequiredField("visible").ToBool() );
  SetVisible(true);
  RenderMode = item.GetRequiredField("draw_mode").ToInt();
  MaxVal = item.GetRequiredField("max_val").ToDouble();
  MinVal = item.GetRequiredField("min_val").ToDouble();
  Depth = item.GetRequiredField("depth").ToDouble();
  Size = item.GetRequiredField("size").ToDouble();
  Extended = item.GetFieldValue("extended", FalseString).ToBool();
  Scale = item.GetRequiredField("scale").ToDouble();
  InitGrid( item.GetRequiredField("max_x").ToInt(), 
            item.GetRequiredField("max_y").ToInt(),
            item.GetRequiredField("max_z").ToInt() );
  for( int x=0; x < MaxX; x++ )
    for( int y=0; y < MaxY; y++ )
      zis.Read( ED->Data[x][y], sizeof(float)*MaxZ );
  TDataItem* maski = item.FindItem("mask");
  if( maski != NULL )  {
    Mask = new FractMask;
    Mask->FromDataItem(*maski, zis);
  }
  InitIso(Is3D());
}
//..............................................................................
TLibrary*  TXGrid::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("xgrid") : name);
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibGetMin, "GetMin",
    fpNone, "Returns minimum value of the map") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibGetMax, "GetMax",
    fpNone, "Returns maximum value of the map") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibExtended, "Extended",
    fpNone|fpOne, "Returns/sets extended size of the grid") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibScale, "Scale",
    fpNone|fpOne, "Returns/sets current scale") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibSize, "Size",
    fpNone|fpOne, "Returns/sets current size") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibIsvalid, "IsValid",
    fpNone|fpOne, "Returns true if grid data is initialised") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibRenderMode, "RenderMode",
    fpNone|fpOne, "Returns/sets grid rendering mode. Supported values: point, line, fill, plane, contour") );
  lib->RegisterMacro<TXGrid>(new TMacro<TXGrid>(this,  &TXGrid::LibWritePS, "WritePS", EmptyString,
    fpOne, "Writes current projection to a postscript file") );

  AGDrawObject::ExportLibrary( *lib );
  return lib;
}
//..............................................................................
//..............................................................................
//..............................................................................

#ifndef _NO_PYTHON
PyObject* pySetValue(PyObject* self, PyObject* args)  {
  int i, j, k;
  float val;
  if( !PyArg_ParseTuple(args, "iiif", &i, &j, &k, &val) )
    return NULL;
  TXGrid::GetInstance()->SetValue(i, j, k, val);
  Py_INCREF(Py_None);
  return Py_None;
}
//..............................................................................
PyObject* pyInit(PyObject* self, PyObject* args)  {
  int i, j, k;
  if( !PyArg_ParseTuple(args, "iii", &i, &j, &k) )
    return NULL;
  TXGrid::GetInstance()->InitGrid(i, j, k);
  return Py_BuildValue("b", true);
}
//..............................................................................
PyObject* pySetMinMax(PyObject* self, PyObject* args)  {
  int min, max;
  if( !PyArg_ParseTuple(args, "ii", &min, &max) )
    return NULL;
  TXGrid::GetInstance()->SetMinVal(min);
  TXGrid::GetInstance()->SetMaxVal(max);
  return Py_BuildValue("b", true);
}
//..............................................................................
PyObject* pySetHole(PyObject* self, PyObject* args)  {
  int min, max;
  if( !PyArg_ParseTuple(args, "ii", &min, &max) )
    return NULL;
  TXGrid::GetInstance()->SetMinHole(min);
  TXGrid::GetInstance()->SetMaxHole(max);
  return Py_BuildValue("b", true);
}
//..............................................................................
PyObject* pySetVisible(PyObject* self, PyObject* args)  {
  bool v;
  if( !PyArg_ParseTuple(args, "b", &v) )
    return NULL;
  TXGrid::GetInstance()->SetVisible(v);
  Py_INCREF(Py_None);
  return Py_None;
}
//..............................................................................
PyObject* pyInitSurface(PyObject* self, PyObject* args)  {
  bool v;
  if( !PyArg_ParseTuple(args, "b", &v) )
    return NULL;
  if( v )
    TXGrid::GetInstance()->AdjustMap();
  TXGrid::GetInstance()->InitIso(v);
  Py_INCREF(Py_None);
  return Py_None;
}
//..............................................................................
PyObject* pyIsVisible(PyObject* self, PyObject* args)  {
  return Py_BuildValue("b", TXGrid::GetInstance()->IsVisible() );
}
//..............................................................................
//..............................................................................

static PyMethodDef XGRID_Methods[] = {
  {"Init", pyInit, METH_VARARGS, "initialises grid memory"},
  {"SetValue", pySetValue, METH_VARARGS, "sets grid iso-level"},
  {"SetMinMax", pySetMinMax, METH_VARARGS, "sets minimum and maximum vaues of the grid"},
  {"SetHole", pySetHole, METH_VARARGS, "sets minimum and maximum vaues of the grid to be avoided"},
  {"IsVisible", pyIsVisible, METH_VARARGS, "returns grid visibility status"},
  {"SetVisible", pySetVisible, METH_VARARGS, "sets grid visibility"},
  {"InitSurface", pyInitSurface, METH_VARARGS, "initialisess surface drawing"},
  {NULL, NULL, 0, NULL}
   };

void TXGrid::PyInit()  {
  Py_InitModule( "olex_xgrid", XGRID_Methods );
}
#endif // _NO_PYTHON
