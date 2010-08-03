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
#include "olxmps.h"
#include "pers_util.h"

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
      olx_gl::vertex(p[i][j]);
      olx_gl::vertex(p[i+1][j]);
      olx_gl::vertex(p[i+1][j+1]);
      olx_gl::vertex(p[i][j+1]);
    }
  }
}
//----------------------------------------------------------------------------//
TXGrid::TXGrid(const olxstr& collectionName, TGXApp* xapp) :
                     AGDrawObject(xapp->GetRender(), collectionName),
                     ExtMin(-1,-1,-1),
                     ExtMax(1,1,1)
{
  if( Instance != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");
  AGDrawObject::SetSelectable(false);
  Mask = NULL;
  Instance = this;
  Extended = false;
  RenderMode = planeRenderModeFill;
#ifndef _NO_PYTHON
  PythonExt::GetInstance()->Register(&TXGrid::PyInit);
#endif
  XApp = xapp;
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
  glpC = glpN = glpP = NULL;
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
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
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

  glpC = &GPC.NewPrimitive("Contour plane", sgloQuads);
  glpC->SetProperties(GS.GetMaterial("Contour plane", 
    TGlMaterial("1029;3628944717;645955712")));
  glpC->Vertices.SetCount(4);
}
//..............................................................................
void TXGrid::CalcColorRGB(float v, uint8_t& R, uint8_t& G, uint8_t& B) const {
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
void TXGrid::CalcColor(float v) const {
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
  olx_gl::color(R,G,B);
}
//..............................................................................
void TXGrid::TPlaneCalculationTask::Run(size_t ind)  {
  for( size_t j=0; j < max_dim; j++ )  {  // (i,j,Depth)        
    vec3f p((float)(ind-hh)/size, (float)(j-hh)/size,  depth);
    p = proj_m*p;
    p -= center;
    p *= c2c;
    p *= dim;
    float val = 0;
    vec3i fp((int)(p[0]), (int)(p[1]), (int)(p[2]));  //x',y',z'
    const float _p = p[0]-fp[0], _pc = _p*_p*_p, _ps = _p*_p;
    const float _q = p[1]-fp[1], _qc = _q*_q*_q, _qs = _q*_q;
    const float _r = p[2]-fp[2], _rc = _r*_r*_r, _rs = _r*_r;
    const float vx[4] = {-_pc/6 + _ps/2 -_p/3, (_pc-_p)/2 - _ps + 1, (-_pc + _ps)/2 + _p, (_pc - _p)/6 };
    const float vy[4] = {-_qc/6 + _qs/2 -_q/3, (_qc-_q)/2 - _qs + 1, (-_qc + _qs)/2 + _q, (_qc - _q)/6 };
    const float vz[4] = {-_rc/6 + _rs/2 -_r/3, (_rc-_r)/2 - _rs + 1, (-_rc + _rs)/2 + _r, (_rc - _r)/6 };
    for( int dx=-1; dx <= 2; dx++ )  {
      const float _vx = vx[dx+1];
      const int n_x = fp[0]+dx;
      for( int dy=-1; dy <= 2; dy++ )  {
        const float _vxy = vy[dy+1]*_vx;
        const int n_y = fp[1]+dy;
        for( int dz=-1; dz <= 2; dz++ )  {
          const float _vxyz = vz[dz+1]*_vxy;
          vec3i ijk(n_x, n_y, fp[2]+dz);
          for( int m=0; m < 3; m++ )  {
            while( ijk[m] < 0 )
              ijk[m] += (int)dim[m];
            while( ijk[m] >= (int)dim[m] )
              ijk[m] -= (int)dim[m];
          }
          val += src_data[ijk[0]][ijk[1]][ijk[2]]*_vxyz;
        }
      }
    }
    if( init_text )  {
      uint8_t R, G, B;
      parent.CalcColorRGB(val, R, G, B);
      const size_t off = (ind+j*max_dim)*3; 
      text_data[off] = R;
      text_data[off+1] = G;
      text_data[off+2] = B;
    }
    if( init_data )  {
      if( val < minVal )
        minVal = val;
      if( val > maxVal )
        maxVal = val;
      data[ind][j] = val;
    }
  }
}
//..............................................................................
bool TXGrid::Orient(TGlPrimitive& GlP)  {
  if( ED == NULL )  return true;
  if( Is3D() )  {
    if( IS == NULL )  return true;
    if( &GlP == glpN )  // draw once only
      olx_gl::callList(PListId);
    else if( &GlP == glpP )  // draw once only
      olx_gl::callList(NListId);
    return true;
  }
  if( &GlP == glpP || &GlP == glpN )  return true;
  if( &GlP == glpC )  {
    if( (RenderMode&planeRenderModePlane) != 0 )
      return true;
  }
  else  {
    if( (RenderMode&planeRenderModeContour) != 0 && (RenderMode&planeRenderModePlane) == 0 )
      return true;
  }

  const mat3f bm(Parent.GetBasis().GetMatrix());
  const mat3f c2c(XApp->XFile().GetAsymmUnit().GetCartesianToCell());
  const float hh = (float)MaxDim/2;
  const vec3f center(Parent.GetBasis().GetCenter());
  const vec3s dim(MaxX, MaxY, MaxZ);
  double Z;
  // if only contours are drawn - render plane at the background
  if( (RenderMode&planeRenderModeContour) != 0 )  {
    if( (RenderMode&planeRenderModePlane) == 0 )
      Z = -Parent.GetMaxRasterZ() + 0.003;
    else  // render the plane just a bit behind
      Z = Depth - 0.001;
  }
  else  // no adjustment is required
    Z = Depth;
  GlP.Vertices[0] = bm*vec3f(-hh/Size, -hh/Size, Z)-center;
  GlP.Vertices[1] = bm*vec3f(hh/Size, -hh/Size, Z)-center;
  GlP.Vertices[2] = bm*vec3f(hh/Size, hh/Size, Z)-center;
  GlP.Vertices[3] = bm*vec3f(-hh/Size, hh/Size, Z)-center;

  TPlaneCalculationTask calc_task(*this, ED->Data, ContourData, TextData, MaxDim, Size, Depth, bm, c2c, center, dim, RenderMode);
  TListIteratorManager<TPlaneCalculationTask> tasks(calc_task, MaxDim, tLinearTask, MaxDim > 64);
  float minVal = 1000, maxVal = -1000;
  for( size_t i = 0; i < tasks.Count(); i++ )  {
    if( tasks[i].minVal < minVal )
      minVal = tasks[i].minVal;
    if( tasks[i].maxVal > maxVal )
      maxVal = tasks[i].maxVal;
  }
  olx_gl::normal(bm[0][2], bm[1][2], bm[2][2]);
  if( (RenderMode&planeRenderModePlane) != 0 )  {
    if( !olx_is_valid_index(TextIndex) )  {
      TextIndex = Parent.GetTextureManager().Add2DTexture("Plane", 0, (GLsizei)MaxDim, (GLsizei)MaxDim, 0, GL_RGB, TextData);
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
      FindTexture(TextIndex), 0, (GLsizei)MaxDim, (GLsizei)MaxDim, 0, GL_RGB, TextData);

    GlP.SetTextureId(TextIndex);
  }
  if( (RenderMode&planeRenderModeContour) != 0 )  {
    Contour<float> cm;
    Contour<float>::MemberFeedback<TXGrid> mf(*this, &TXGrid::GlLine);
    float contour_step = (maxVal - minVal)/(ContourLevelCount-1);
    ContourLevels[0] = minVal;
    for( size_t i=1; i < ContourLevelCount; i++ )
      ContourLevels[i] = ContourLevels[i-1]+contour_step;

    GlP.PrepareColorRendering(GL_LINES);
    olx_gl::color(0, 0, 0);
    cm.DoContour(ContourData, 0, (int)MaxDim-1, 0, (int)MaxDim-1,
      ContourCrds[0], ContourCrds[1], 
      ContourLevelCount, ContourLevels, mf);
    GlP.EndColorRendering();
  }
  return false;
}
//..............................................................................
void TXGrid::GlLine(float x1, float y1, float x2, float y2, float z)  {
  vec3d p1(x1/Size, y1/Size, Depth), p2(x2/Size, y2/Size, Depth);
  p1 = Parent.GetBasis().GetMatrix()*p1 - Parent.GetBasis().GetCenter();
  p2 = Parent.GetBasis().GetMatrix()*p2 - Parent.GetBasis().GetCenter();
  if( z < 0 )  // render just a half of the segment
    p2 = (p1 + p2)*0.5;
  
  olx_gl::vertex(p1);
  olx_gl::vertex(p2);
}
//..............................................................................
bool TXGrid::GetDimensions(vec3d &Max, vec3d &Min)  {
//  Min = FCenter;
//  Max = FCenter;
  return false;
};
//..............................................................................
void TXGrid::InitGrid(size_t maxX, size_t maxY, size_t maxZ)  {
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
  for( int i=0; i < (int)MaxDim; i++ )  {
    ContourData[i] = new float[MaxDim];
    ContourCrds[0][i] = ContourCrds[1][i] = (float)(i-(int)MaxDim/2);
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
    olx_gl::deleteLists(PListId, 2);
    PListId = NListId = ~0;
  }
  if( ContourData != NULL )  {
    for( size_t i=0; i < MaxDim; i++ )
      delete [] ContourData[i];
    delete [] ContourData;
    delete [] ContourCrds[0];
    delete [] ContourCrds[1];
    delete [] ContourLevels;
    ContourData = NULL;
    ContourLevels = NULL;
  }
}
//..............................................................................
bool TXGrid::LoadFromFile(const olxstr& GridFile)  {
  TEFile::CheckFileExists(__OlxSourceInfo, GridFile);
  TStrList SL, toks;
  SL.LoadFromFile(GridFile);
  toks.Strtok(SL[0], ' ');

  int vc = 3;
  InitGrid(toks[0].ToSizeT(), toks[1].ToSizeT(), toks[2].ToSizeT());
  for( size_t i=0; i < MaxX; i++ )  {
    for( size_t j=0; j < MaxY; j++ )  {
      for( size_t k=0; k < MaxZ; k++ )  {
        const float val = toks[vc].ToFloat<float>();
        if( val > MaxVal ) MaxVal = val;
        if( val < MinVal ) MinVal = val;
        ED->Data[i][j][k] = val;
        vc++;
      }
    }
  }

  // set default depth to center of the asymmetric unit
  vec3d v(XApp->XFile().GetAsymmUnit().GetOCenter(true, true));
  v = XApp->XFile().GetAsymmUnit().GetCellToCartesian() * v;
  SetDepth(v);
  return true;
}
//..............................................................................
void TXGrid::SetScale(float v)  {
  bool _3d = Is3D();
  if( _3d && MinHole != MaxHole )  {
    if( v >= MinHole && v <= MaxHole )  {
      Info->Clear();
      Info->PostText("Locked");
      Info->Fit();
      return;
    }
  }
  Scale = v;
  UpdateInfo();
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
      IS->GenerateSurface(-Scale);
      n_vertices = IS->VertexList();
      n_normals = IS->NormalList();
      n_triangles = IS->TriangleList();
    }
    RescaleSurface();
  }
}
//..............................................................................
void TXGrid::SetExtended(bool v)  {
  if( Extended == v )  return;
  Extended = v;
  SetScale(Scale);
}
//..............................................................................
void TXGrid::SetDepth(float v)  {
  Depth = v;
  const float z = Parent.MaxDim().DistanceTo(Parent.MinDim())/2;
  if( Depth < -z )
    Depth = -z;
  if( Depth > z )
    Depth = z;
}
//..............................................................................
void TXGrid::SetDepth(const vec3d& v)  {
  vec3d p = (v + Parent.GetBasis().GetCenter())*Parent.GetBasis().GetMatrix();
  SetDepth((float)p[2]);
}
//..............................................................................
void TXGrid::SetPlaneSize(size_t _v)  {
  size_t v = _v; 
  while( (v&1) == 0 )
    v = v >> 1;
  if( v != 1 )
    throw TInvalidArgumentException(__OlxSrcInfo, "PlaneSize must be a power of 2");
  if( _v < 64 || _v > 1024 || _v == MaxDim )
    return;
  if( TextData != NULL )  {
    delete TextData;
    TextData = new char[_v*_v*3+1];
  }
  if( ContourData != NULL )  {
    for( size_t i=0; i < MaxDim; i++ )
      delete [] ContourData[i];
    delete [] ContourData;
    delete [] ContourCrds[0];
    delete [] ContourCrds[1];
    ContourData = new float*[_v];
    ContourCrds[0] = new float[_v];
    ContourCrds[1] = new float[_v];
    for( size_t i=0; i < _v; i++ )  {
      ContourData[i] = new float[_v];
      ContourCrds[0][i] = ContourCrds[1][i] = (float)(i-_v/2);///MaxDim;
    }
  }
  MaxDim = _v;   
  Parent.Draw();
}
//..............................................................................
void TXGrid::SetContourLevelCount(size_t v)  {
  if( v <= 2 || v > 30 )  return;
  if( ContourLevels != NULL )
    delete [] ContourLevels;
  ContourLevelCount = v;
  ContourLevels = new float[ContourLevelCount];
}
//..............................................................................
bool TXGrid::OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {
  if( (Data.Shift & sssCtrl) == 0 && (Data.Shift & sssShift) == 0 )
    return false;
  LastMouseX = Data.DownX;
  LastMouseY = Data.DownY;
  MouseDown = true;
  return true;
}
//..............................................................................
bool TXGrid::OnMouseUp(const IEObject *Sender, const TMouseData& Data) {
  MouseDown = false;
  return !((Data.Shift & sssCtrl) == 0 && (Data.Shift & sssShift) == 0);
}
//..............................................................................
bool TXGrid::OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {
  if( !MouseDown )  return false;
  if( (Data.Button & smbLeft) != 0 ) {
    SetDepth(Depth+(float)((LastMouseX - Data.X)+(LastMouseY - Data.Y))/15);
  }
  else  {
    if( (Data.Shift & sssShift) != 0 )  {
      if( RenderMode == planeRenderModeContour )  {
        const int v =  -(LastMouseX - Data.X) + (LastMouseY - Data.Y);
        SetContourLevelCount(GetContourLevelCount()+v/2);
      }
      else  {
        const double step = (MaxVal-MinVal)/250.0;
        Scale -= step*(LastMouseX - Data.X);
        Scale += step*(LastMouseY - Data.Y);
        if( olx_abs(Scale) > olx_max(MaxVal,MinVal)  )
          Scale = olx_sign(Scale)*olx_max(MaxVal,MinVal);
      }
    }
    else  {
      Size += (float)(LastMouseX - Data.X)/15;
      Size += (float)(LastMouseY - Data.Y)/15;
      if( Size < 1 )  Size = 1;
      if( Size > 20 )  Size = 20;
    }
  }
  if( IS != NULL )  {
    SetScale(Scale);
  }
  UpdateInfo();
  LastMouseX = Data.X;
  LastMouseY = Data.Y;
  return true;
}
//..............................................................................
void TXGrid::UpdateInfo()  {
  Info->Clear();
  if( RenderMode == planeRenderModeContour )
    Info->PostText(olxstr("Contours number: ") << GetContourLevelCount());
  else
    Info->PostText(olxstr("Current level: ") << Scale);
  Info->Fit();
}
//..............................................................................
void TXGrid::GlContextChange()  {
  if( ED == NULL )
    return;
  if( !olx_is_valid_index(PListId)  )  {
    olx_gl::deleteLists(PListId, 2);
    PListId = NListId = ~0;
  }
  SetScale(Scale);
}
//..............................................................................
void TXGrid::RescaleSurface()  {
  const TAsymmUnit& au =  XApp->XFile().GetAsymmUnit();
  if( !olx_is_valid_index(PListId) )  {
    PListId = olx_gl::genLists(2);
    NListId = PListId+1;
  }
  if( Mask != NULL )  {
    vec3d pts[3];
    for( int li = 0; li <= 1; li++ )  {
      const TTypeList<vec3f>& verts = (li == 0 ? p_vertices : n_vertices);
      const TTypeList<vec3f>& norms = (li == 0 ? p_normals : n_normals);
      const TTypeList<IsoTriangle>& trians = (li == 0 ? p_triangles : n_triangles);
      glNewList(li == 0 ? PListId : NListId, GL_COMPILE);
      olx_gl::polygonMode(GL_FRONT_AND_BACK, GetPolygonMode());
      olx_gl::begin(GL_TRIANGLES);
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
                olx_gl::normal(norms[trians[i].pointID[j]]);
                olx_gl::vertex(pts[j]);
              }
            }
          }
        }
      }
      olx_gl::end();
      olx_gl::polygonMode(GL_FRONT_AND_BACK, GL_FILL);
      olx_gl::endList();
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
        glNewList(li == 0 ? PListId : NListId, GL_COMPILE);
        olx_gl::polygonMode(GL_FRONT_AND_BACK, GetPolygonMode());
        olx_gl::begin(GL_TRIANGLES);
        for( float x=ExtMin[0]; x < ExtMax[0]; x++ )  {
          for( float y=ExtMin[1]; y < ExtMax[1]; y++ )  {
            for( float z=ExtMin[2]; z < ExtMax[2]; z++ )  {
              for( size_t i=0; i < trians.Count(); i++ )  {
                bool draw = true;
                for( int j=0; j < 3; j++ )  {
                  pts[j] = verts[trians[i].pointID[j]];                      // ext drawing
                  pts[j][0] /= MaxX;  pts[j][1] /= MaxY;  pts[j][2] /= MaxZ; // ext drawing
                  pts[j][0] += x;     pts[j][1] += y;     pts[j][2] += z;    // ext drawing
                  if( pts[j][0] > ExtMax[0] || pts[j][1] > ExtMax[1] || pts[j][2] > ExtMax[2] )  {
                    draw = false;
                    break;
                  }
                  au.CellToCartesian(pts[j]);                                // ext drawing
                }
                if( !draw )  continue;
                for( int j=0; j < 3; j++ )  {
                  olx_gl::normal(norms[trians[i].pointID[j]]);
                  olx_gl::vertex(pts[j]);               // ext drawing
                }
              }
            }
          }
        }
        olx_gl::end();
        olx_gl::polygonMode(GL_FRONT_AND_BACK, GL_FILL);
        olx_gl::endList();
      }
    }
    else  {
      for( int li = 0; li <= 1; li++ )  {
        TTypeList<vec3f>& verts = (li == 0 ? p_vertices : n_vertices);
        const TTypeList<vec3f>& norms = (li == 0 ? p_normals : n_normals);
        const TTypeList<IsoTriangle>& trians = (li == 0 ? p_triangles : n_triangles);
        for( size_t i=0; i < verts.Count(); i++ )  {                           // cell drawing
          verts[i][0] /= (MaxX-1);  verts[i][1] /= (MaxY-1);  verts[i][2] /= (MaxZ-1);  // cell drawing
          au.CellToCartesian(verts[i]);                                     // cell drawing
        }
        glNewList(li == 0 ? PListId : NListId, GL_COMPILE);
        olx_gl::polygonMode(GL_FRONT_AND_BACK, GetPolygonMode());
        olx_gl::begin(GL_TRIANGLES);
        for( size_t i=0; i < trians.Count(); i++ )  {
          for( int j=0; j < 3; j++ )  {
            olx_gl::normal(norms[trians[i].pointID[j]]);
            olx_gl::vertex(verts[trians[i].pointID[j]]);  // cell drawing
          }
        }
        olx_gl::end();
        olx_gl::polygonMode(GL_FRONT_AND_BACK, GL_FILL);
        olx_gl::endList();
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
  if( ED == NULL )  return;
  for( size_t i=0; i < MaxX; i++ )
    for( size_t j=0; j < MaxY; j++ )
      ED->Data[i][j][MaxZ] = ED->Data[i][j][0];
  for( size_t i=0; i < MaxX; i++ )
    for( size_t j=0; j < MaxZ; j++ )
      ED->Data[i][MaxY][j] = ED->Data[i][0][j];
  for( size_t i=0; i < MaxY; i++ )
    for( size_t j=0; j < MaxZ; j++ )
      ED->Data[MaxX][i][j] = ED->Data[0][i][j];

  for( size_t i=0; i < MaxX; i++ )
    ED->Data[i][MaxY][MaxZ] = ED->Data[i][0][0];
  for( size_t i=0; i < MaxY; i++ )
    ED->Data[MaxX][i][MaxZ] = ED->Data[0][i][0];
  for( size_t i=0; i < MaxZ; i++ )
    ED->Data[MaxX][MaxY][i] = ED->Data[0][0][i];
  ED->Data[MaxX][MaxY][MaxZ] = ED->Data[0][0][0];
}
//..............................................................................
void TXGrid::InitIso()  {
  if( !Is3D() )  {
    if( IS != NULL )  {
      delete IS;
      IS = NULL;
    }
  }
  else  {
    if( ED == NULL )  return;
    if( IS != NULL )  delete IS;
    IS = new CIsoSurface(*ED);
    SetScale(Scale);
  }
}
//..............................................................................
TXBlob* TXGrid::CreateBlob(int x, int) const {
  if( IS == NULL || !Is3D() )  return NULL;
  const TAsymmUnit& au =  XApp->XFile().GetAsymmUnit();
  TXBlob* xb = new TXBlob(Parent, "blob");
  //IS->GenerateSurface(Scale);
  TPtrList<IsoTriangle> triags;
  const TArrayList<vec3f>& vertices = IS->VertexList();
  const TArrayList<vec3f>& normals = IS->NormalList();
  const TArrayList<IsoTriangle>& triangles = IS->TriangleList();
  TEBitArray verts(vertices.Count()), used_triags(triangles.Count());
  verts.SetTrue(triangles[0].pointID[0]);
  verts.SetTrue(triangles[0].pointID[1]);
  verts.SetTrue(triangles[0].pointID[2]);
  triags.Add(triangles[0]);
  bool added = true;
  size_t vec_cnt = 0;
  while( added )  {
    added = false;
    for( size_t i=1; i < triangles.Count(); i++ )  {
      if( used_triags[i] )  continue;
      IsoTriangle& t = triangles[i];
      bool has_shared_point = false;
      for( int j=0; j < 3; j++ )  {
        if( verts[t.pointID[j]] )  {
          has_shared_point = true;
          break;
        }
      }
      if( has_shared_point )  {
        added = true;
        used_triags.SetTrue(i);
        triags.Add(t);
        for( int j=0; j < 3; j++ )  {
          if( !verts[t.pointID[j]] )  {
            verts.SetTrue(t.pointID[j]);
            vec_cnt++;
          }
        }
      }
    }
  }
  xb->vertices.SetCapacity(vec_cnt);
  xb->normals.SetCapacity(vec_cnt);
  TArrayList<size_t> new_ids(vertices.Count());
  for( size_t i = 0; i < verts.Count(); i++ )  {
    if( verts[i] )  {
      new_ids[i] = xb->vertices.Count();
      vec3f& v = xb->vertices.AddCCopy(vertices[i]);
      au.CellToCartesian<vec3f,vec3f>(vec3f(v[0]/MaxX, v[1]/MaxY, v[2]/MaxZ), v);
      xb->normals.AddCCopy(normals[i]);
    }
    else
      new_ids[i] = ~0;
  }
  xb->triangles.SetCapacity(triags.Count());
  for( size_t i=0; i < triags.Count(); i++ )  {
    IsoTriangle& t = xb->triangles.Add(new IsoTriangle(*triags[i]));
    t.pointID[0] = new_ids[t.pointID[0]];
    t.pointID[1] = new_ids[t.pointID[1]];
    t.pointID[2] = new_ids[t.pointID[2]];
  }
  return xb;
}
//..............................................................................
//..............................................................................
//..............................................................................
void TXGrid::LibExtended(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Extended);
  else if( Params.Count() == 1 )
    SetExtended(Params[0].ToBool());
  else  {
    ExtMin = vec3d(Params[0].ToDouble(), Params[1].ToDouble(), Params[2].ToDouble());
    ExtMax = vec3d(Params[3].ToDouble(), Params[4].ToDouble(), Params[5].ToDouble());
    SetExtended(true);
  }
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
void TXGrid::LibPlaneSize(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(MaxDim);
  else
    SetPlaneSize(Params[0].ToSizeT());
}
//..............................................................................
void TXGrid::LibDepth(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(Depth);
  else
    Depth = Params[0].ToFloat<float>();
}
//..............................................................................
void TXGrid::LibMaxDepth(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(Parent.MaxDim().DistanceTo(Parent.MinDim())/2);
}
//..............................................................................
void TXGrid::LibContours(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal(ContourLevelCount);
  else
    SetContourLevelCount(Params[0].ToSizeT());
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
    else if( RenderMode == (planeRenderModeContour|planeRenderModePlane) )
      E.SetRetVal<olxstr>("contour+plane");
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
  else if( Params[0] == "contour+plane" )
    RenderMode = planeRenderModeContour|planeRenderModePlane;
  else throw TInvalidArgumentException(__OlxSourceInfo,
         olxstr("incorrect mode value: '") << Params[0] << '\'');
  InitIso();
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
    item.AddField("ext_min", PersUtil::VecToStr(ExtMin));
    item.AddField("ext_max", PersUtil::VecToStr(ExtMin));
    item.AddField("scale", Scale);
    item.AddField("max_x", MaxX);
    item.AddField("max_y", MaxY);
    item.AddField("max_z", MaxZ);
    for( size_t x=0; x < MaxX; x++ )  {
      for( size_t y=0; y < MaxY; y++ )  {
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
  ExtMin = vec3f(-1,-1,-1);
  ExtMax = vec3f(1,1,1);
  const size_t ed_i = item.FieldIndex("ext_max");
  if( ed_i != InvalidIndex )  {
    ExtMin = PersUtil::FloatVecFromStr(item.GetField(ed_i));
    ExtMax = PersUtil::FloatVecFromStr(item.GetRequiredField("ext_min"));
  }
  Scale = item.GetRequiredField("scale").ToDouble();
  InitGrid( item.GetRequiredField("max_x").ToInt(), 
            item.GetRequiredField("max_y").ToInt(),
            item.GetRequiredField("max_z").ToInt() );
  for( size_t x=0; x < MaxX; x++ )
    for( size_t y=0; y < MaxY; y++ )
      zis.Read(ED->Data[x][y], sizeof(float)*MaxZ);
  TDataItem* maski = item.FindItem("mask");
  if( maski != NULL )  {
    Mask = new FractMask;
    Mask->FromDataItem(*maski, zis);
  }
  InitIso();
}
//..............................................................................
TLibrary*  TXGrid::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("xgrid") : name);
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibGetMin, "GetMin",
    fpNone, "Returns minimum value of the map") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibGetMax, "GetMax",
    fpNone, "Returns maximum value of the map") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibExtended, "Extended",
    fpNone|fpOne|fpSix, "Returns/sets extended size of the grid") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibScale, "Scale",
    fpNone|fpOne, "Returns/sets current scale") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibSize, "Size",
    fpNone|fpOne, "Returns/sets current size") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibPlaneSize, "PlaneSize",
    fpNone|fpOne, "Returns/sets current size") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibDepth, "Depth",
    fpNone|fpOne, "Returns/sets current depth") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibDepth, "MaxDepth",
    fpNone, "Returns maximum available depth") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibContours, "Contours",
    fpNone|fpOne, "Returns/sets number of contour levels") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibIsvalid, "IsValid",
    fpNone|fpOne, "Returns true if grid data is initialised") );
  lib->RegisterFunction<TXGrid>(new TFunction<TXGrid>(this,  &TXGrid::LibRenderMode, "RenderMode",
    fpNone|fpOne, "Returns/sets grid rendering mode. Supported values: point, line, fill, plane, contour") );

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
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iiif");
  TXGrid::GetInstance()->SetValue(i, j, k, val);
  return PythonExt::PyNone();
}
//..............................................................................
PyObject* pySetData(PyObject* self, PyObject* args)  {
  int di, dj, dk;
  PyObject *grid;
  if( !PyArg_ParseTuple(args, "iiiO", &di, &dj, &dk, &grid) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iiiO");
  TXGrid& g = *TXGrid::GetInstance();
  g.InitGrid(di, dj, dk);
  PyObject* arglist = PyTuple_New(3);
  bool error = false;
  int max = olx_max(dk, olx_max(di,dj));
  TPtrList<PyObject> indexes(max);
  for( int i=0; i < max; i++ )
    indexes[i] = PyInt_FromLong(i);
  for( int i=0; i < di; i++ )  {
    Py_INCREF(indexes[i]);
    PyTuple_SetItem(arglist, 0, indexes[i]);
    for( int j=0; j < dj; j++ )  {
      Py_INCREF(indexes[j]);
      PyTuple_SetItem(arglist, 1, indexes[j]);
      for( int k=0; k < dk; k++ )  {
        Py_INCREF(indexes[k]);
        PyTuple_SetItem(arglist, 2, indexes[k]);
        PyObject* result = PyObject_GetItem(grid, arglist);
        if( result != NULL )  {
          g.SetValue(i,j,k, (float)PyFloat_AsDouble(result));
          Py_DECREF(result);
        }
        if( PyErr_Occurred() )  {
          error = true;
          break;
        }
      }
      if( error )  break;
    }
    if( error )  break;
  }
  if( error )  PyErr_Print();
  for( int i=0; i < max; i++ )
    Py_DECREF(indexes[i]);
  Py_DECREF(arglist);
  return PythonExt::PyNone();
}
//..............................................................................
PyObject* pyInit(PyObject* self, PyObject* args)  {
  int i, j, k;
  if( !PyArg_ParseTuple(args, "iii", &i, &j, &k) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "iii");
  TXGrid::GetInstance()->InitGrid(i, j, k);
  return PythonExt::PyTrue();
}
//..............................................................................
PyObject* pySetMinMax(PyObject* self, PyObject* args)  {
  float min, max;
  if( !PyArg_ParseTuple(args, "ff", &min, &max) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "ff");
  TXGrid::GetInstance()->SetMinVal(min);
  TXGrid::GetInstance()->SetMaxVal(max);
  return PythonExt::PyTrue();
}
//..............................................................................
PyObject* pySetHole(PyObject* self, PyObject* args)  {
  float min, max;
  if( !PyArg_ParseTuple(args, "ff", &min, &max) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "ff");
  TXGrid::GetInstance()->SetMinHole(min);
  TXGrid::GetInstance()->SetMaxHole(max);
  return PythonExt::PyTrue();
}
//..............................................................................
PyObject* pySetVisible(PyObject* self, PyObject* args)  {
  bool v;
  if( !PyArg_ParseTuple(args, "b", &v) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "b");
  TXGrid::GetInstance()->SetVisible(v);
  return PythonExt::PyNone();
}
//..............................................................................
PyObject* pyInitSurface(PyObject* self, PyObject* args)  {
  bool v;
  if( !PyArg_ParseTuple(args, "b", &v) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "b");
  if( v )
    TXGrid::GetInstance()->AdjustMap();
  TXGrid::GetInstance()->InitIso();
  return PythonExt::PyNone();
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
  {"SetData", pySetData, METH_VARARGS, "sets grid data, dimensions and a callable accessor are required"},
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
