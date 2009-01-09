//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXGrid
// (c) Oleg V. Dolomanov, 2006
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xgrid.h"
#include "gpcollection.h"

#include "styles.h"

#include "glmaterial.h"
#include "glrender.h"

#include "efile.h"

#include "gxapp.h"

#include "library.h"

#ifndef _NO_PYTHON
  #include "pyext.h"
#endif

  TXGrid* TXGrid::Instance = NULL;

  void TXGrid::DrawQuad16(double points[4][4])  {
    static double p[3][3][4];

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
    static double p[3][3][3], c[2][2];

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
// TXReflection function bodies
//----------------------------------------------------------------------------//
TXGrid::TXGrid(const olxstr& collectionName, TGXApp* xapp) :
                     TGlMouseListener(collectionName, &xapp->GetRender())  {
  if( Instance != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "singleton");

  Instance = this;
  Mode3D = false;
  PolygonMode = GL_FILL;
#ifndef _NO_PYTHON
  PythonExt::GetInstance()->Register( &TXGrid::PyInit );
#endif
  XApp = xapp;
  Move2D(false);
  Moveable(false);
  Zoomable(false);
  Depth = 0;
  ED = NULL;
  IS = NULL;
  MouseDown = false;
  Size = 10;
  TextIndex = -1;
  TextData = NULL;
  Scale = 10;
  //for textures, 2^n+2 (for border)...
  //MaxDim = 128;//olx_max( olx_max(MaxX,MaxY), MaxZ);
  MaxDim = 128;
  Info = new TGlTextBox("XGrid_Legend", FParent);
  MaxX = MaxY = MaxZ = 0;
  MaxVal = MinVal = 0;
  MinHole = MaxHole = 0;
  GridMoved = false;
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
  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  if( !GPC )
    GPC = FParent->NewCollection( GetCollectionName() );
  else  {
    if( GPC->PrimitiveCount() )  {
      GPC->AddObject(this);
      return;
    }
  }
  TGraphicsStyle* GS = GPC->Style();
  GPC->AddObject(this);
  TGlPrimitive* GlP = GPC->NewPrimitive("eMap");
  TGlMaterial* GlM = const_cast<TGlMaterial*>( GS->Material("eMap") );
  if( GlM->Mark() )  {
    GlM->SetFlags(0);
    GlM->ShininessF = 128;
    GlM->SetFlags(sglmAmbientF|sglmDiffuseF|sglmTransparent);
    GlM->AmbientF = 0xD80f0f0f;
    GlM->DiffuseF = 0xD80f0f0f;
  }
  TextIndex = -1;
  GlP->Texture( -1 );

  GlP->SetProperties(GlM);
  GlP->Type(sgloQuads);
  GlP->Data().Resize(5, 4);
  // texture coordinates
  GlP->Data()[3][0] = 0;  GlP->Data()[4][0] = 0;
  GlP->Data()[3][1] = 1;  GlP->Data()[4][1] = 0;
  GlP->Data()[3][2] = 1;  GlP->Data()[4][2] = 1;
  GlP->Data()[3][3] = 0;  GlP->Data()[4][3] = 1;
  Info->Create();
  // create dummy primitives
  glpP = GPC->NewPrimitive("+Surface");
  GlM = const_cast<TGlMaterial*>( GS->Material("+Surface") );
  if( GlM->Mark() )
    GlM->FromString("85;0.000,1.000,0.000,0.850;3632300160;1.000,1.000,1.000,0.500;36");
  glpP->SetProperties(GlM);
  glpP->Type(sgloQuads);
  glpN = GPC->NewPrimitive("-Surface");
  GlM = const_cast<TGlMaterial*>( GS->Material("-Surface") );
  if( GlM->Mark() ) 
    GlM->FromString("85;1.000,0.000,0.000,0.850;3632300160;1.000,1.000,1.000,0.500;36");
  glpN->SetProperties(GlM);
  glpN->Type(sgloQuads);
}
//..............................................................................
void TXGrid::CalcColorRGB(double v, double& R, double& G, double& B) {
  //if( v == 0 )
  // v = MaxVal;
  double cs;
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
  R *=255;
  G *=255;
  B *=255;
}
//..............................................................................
void TXGrid::CalcColor(double v) {
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
bool TXGrid::Orient(TGlPrimitive *GlP)  {
  if( ED == NULL )  return true;

  if( IS != NULL && Mode3D && (IS->TriangleList().Count() > 25 || triangles.Count() > 25) )  {
    if( GlP != glpP && GlP != glpN )  return true;
    const TArrayList<vec3f>& verts( (GlP == glpP) ? IS->VertexList() : vertices );
    const TArrayList<vec3f>& norms( (GlP == glpP) ? IS->NormalList() : normals );
    const TArrayList<IsoTriangle>& trians(  (GlP == glpP) ? IS->TriangleList() : triangles );
    glPolygonMode(GL_FRONT_AND_BACK, PolygonMode);
    glBegin(GL_TRIANGLES);
    for( int i=0; i < trians.Count(); i++ )  {
      for( int j=0; j < 3; j++ )  {
        const vec3f& nr = norms[trians[i].pointID[j]];
        glNormal3f( nr[0], nr[1], nr[2] );
        const vec3f& p = verts[trians[i].pointID[j]];
        glVertex3f(p[0], p[1], p[2]);
      }
    }
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    return true;
  }
  if( GlP == glpP || GlP == glpN )  return true;

//  mat3d bm ( mat3d::Transpose(Parent()->GetBasis().GetMatrix()) );
  mat3d bm( Parent()->GetBasis().GetMatrix() );
  mat3d c2c(  XApp->XFile().GetAsymmUnit().GetCartesianToCell() );

  double R, G, B;
  vec3d p, p1, p2, p3, p4;
  const float hh = (float)MaxDim/2;

  p1[0] = -hh/Size;  p1[1] = -hh/Size;
  p2[0] = hh/Size;   p2[1] = -hh/Size;
  p3[0] = hh/Size;   p3[1] = hh/Size;
  p4[0] = -hh/Size;  p4[1] = hh/Size;
  p1[2] = p2[2] = p3[2] = p4[2] = Depth;
  p1 = bm * p1;  p1 -= Parent()->GetBasis().GetCenter();
  p2 = bm * p2;  p2 -= Parent()->GetBasis().GetCenter();
  p3 = bm * p3;  p3 -= Parent()->GetBasis().GetCenter();
  p4 = bm * p4;  p4 -= Parent()->GetBasis().GetCenter();

  const int maxX = ED->Length1(),
            maxY = ED->Length2(),
            maxZ = ED->Length3();

  for( int i=0; i < MaxDim; i++ )  {
    for( int j=0; j < MaxDim; j++ )  {  // (i,j,Depth)        
      p[0] = (double)(i-hh)/Size;
      p[1] = (double)(j-hh)/Size;
      p[2] = Depth;

      p = bm * p;
      p -= Parent()->GetBasis().GetCenter();
      p *= c2c;
      p -= GridStranslation;
      p[0] *= MaxX;  p[0] = Round(p[0]);
      p[1] *= MaxY;  p[1] = Round(p[1]);
      p[2] *= MaxZ;  p[2] = Round(p[2]);

      while( p[0] < 0 )     p[0] += maxX;
      while( p[0] >= maxX )  p[0] -= maxX;
      while( p[1] < 0 )     p[1] += maxY;
      while( p[1] >= maxY )  p[1] -= maxY;
      while( p[2] < 0 )     p[2] += maxZ;
      while( p[2] >= maxZ )  p[2] -= maxZ;
      float val = ED->Data[(int)p[0]][(int)p[1]][(int)p[2]];
      CalcColorRGB(val, R, G, B);
      const int off = (i+j*MaxDim)*3; 
      TextData[off]     = (char)R;
      TextData[off + 1] = (char)G;
      TextData[off + 2] = (char)B;
    }
  }

  if( TextIndex == -1 )  {
    TextIndex = FParent->GetTextureManager().Add2DTexture("Plane", 0, MaxDim, MaxDim, 0, GL_RGB, TextData);
    TGlTexture* tex = FParent->GetTextureManager().FindTexture(TextIndex);
    tex->SetEnvMode( tpeDecal );
    tex->SetSCrdWrapping( tpCrdClamp );
    tex->SetTCrdWrapping( tpCrdClamp );

    tex->SetMinFilter( tpFilterLinear );
    tex->SetMagFilter( tpFilterLinear );
    tex->SetEnabled(true);
  }
  else
    FParent->GetTextureManager().
      Replace2DTexture(*FParent->GetTextureManager().
      FindTexture(TextIndex), 0, MaxDim, MaxDim, 0, GL_RGB, TextData);

  GlP->Data()[0][0] = p1[0];
  GlP->Data()[1][0] = p1[1];
  GlP->Data()[2][0] = p1[2];

  GlP->Data()[0][1] = p2[0];
  GlP->Data()[1][1] = p2[1];
  GlP->Data()[2][1] = p2[2];

  GlP->Data()[0][2] = p3[0];
  GlP->Data()[1][2] = p3[1];
  GlP->Data()[2][2] = p3[2];

  GlP->Data()[0][3] = p4[0];
  GlP->Data()[1][3] = p4[1];
  GlP->Data()[2][3] = p4[2];

  GlP->Texture( TextIndex );
//  glNormal3d(bm[0][2], bm[1][2], bm[2][2]);
  glNormal3d(0, 0, 1);

  return false;
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
  ED = new TArray3D<float>(0, MaxX-1, 0,MaxY-1, 0, MaxZ-1);
  TextData = new char[MaxDim*MaxDim*3+1];
  GridMoved = false;
  GridStranslation.Null();
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
    triangles.Clear();
    normals.Clear();
    vertices.Clear();
    IS = NULL;
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
        float val = (float)toks.String(vc).ToDouble();
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
  SetDepth( v );
  return true;
}
//..............................................................................
void TXGrid::SetScale(float v)  {
  if( MinHole != MaxHole )  {
    if( v >= MinHole && v <= MaxHole )  return;
  }
  Scale = v;
  Info->Clear();
  Info->PostText( olxstr("Current level is ") << Scale);
  if( IS != NULL && Mode3D )  {
    triangles.Clear();
    normals.Clear();
    vertices.Clear();
    IS->GenerateSurface( Scale );
    if( Scale < 0 )  {
      vertices.Assign( IS->VertexList() );
      normals.Assign( IS->NormalList() );
      triangles.Assign( IS->TriangleList() );
      IS->GenerateSurface( -Scale );
    }
    RescaleSurface();
  }
}
//..............................................................................
void TXGrid::SetDepth(float v)  {  Depth = v;  }
//..............................................................................
void TXGrid::SetDepth(const vec3d& v)  {
  vec3d p(v);
  p += Parent()->GetBasis().GetCenter();
  p = Parent()->GetBasis().GetMatrix()*p;
  SetDepth( (float)p[2] );
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
    float z = (float)FParent->CalcZoom();
    if( Depth < -z/2 )
      Depth = -z/2;
    if( Depth > z/2 )
      Depth = z/2;
  }
  else  {
    if( (Data->Shift & sssShift) != 0 )  {
      Scale += (float)(LastMouseX - Data->X);
      Scale += (float)(LastMouseY - Data->Y);
      if( Scale < MinVal )  Scale = MinVal;
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
  Info->PostText( olxstr("Current level is ") << Scale);
  LastMouseX = Data->X;
  LastMouseY = Data->Y;
  return true;
}
//..............................................................................

void TXGrid::RescaleSurface()  {
  const mat3d& c2ca =  XApp->XFile().GetAsymmUnit().GetCellToCartesian();

  TArrayList<vec3f>& verts = IS->VertexList();
  for( int i=0; i < verts.Count(); i++ )  {
    vec3f& p = verts[i];
    p[0] /= MaxX;
    p[1] /= MaxY;
    p[2] /= MaxZ;
    p += GridStranslation;
    p *= c2ca;
  }
  for( int i=0; i < vertices.Count(); i++ )  {
    vec3f& p = vertices[i];
    p[0] /= MaxX;
    p[1] /= MaxY;
    p[2] /= MaxZ;
    p += GridStranslation;
    p *= c2ca;
  }
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
    if( !GridMoved )  {
      GridMoved = true;
      const TLattice& latt = XApp->XFile().GetLattice();
      vec3d cnt;
      int ac = 0;
      for( int i=0; i < latt.AtomCount(); i++ )  {
        const TSAtom& a = latt.GetAtom(i);
        if( a.IsDeleted() || a.GetAtomInfo() == iQPeakIndex )  continue;
        cnt += a.ccrd();
        ac ++;
      }
      if( ac != 0 )  cnt /= ac;
      //    cnt *= -1;
      cnt -= 0.5;
      GridStranslation = cnt;
      cnt[0] *= MaxX;  cnt[0] = Round(cnt[0]);
      cnt[1] *= MaxY;  cnt[1] = Round(cnt[1]);
      cnt[2] *= MaxZ;  cnt[2] = Round(cnt[2]);
      // expanding the box by 5A of the UC
      TArray3D<float> *nA = new TArray3D<float>(0, MaxX-1, 0, MaxY-1, 0, MaxZ-1);
      // fill the new array with data
      for( int ix=0; ix < MaxX; ix++ )  {
        int x = ix + (int)cnt[0];
        while( x < 0 )  x += MaxX;
        while( x >= MaxX ) x -= MaxX;  
        for( int iy=0; iy < MaxY; iy++ )  {
          int y = iy + (int)cnt[1];
          while( y < 0 )  y += MaxY;
          while( y >= MaxY ) y -= MaxY;  
          for( int iz=0; iz < MaxZ; iz++ )  {
            int z = iz + (int)cnt[2];
            while( z < 0 )  z += MaxZ;
            while( z >= MaxZ ) z -= MaxZ;  
            nA->Value(ix,iy,iz) = ED->Data[x][y][z];
          }
        }
      }
      delete ED;
      ED = nA;
    }
    IS = new CIsoSurface<float>(*ED);
    SetScale(Scale);
  }
  Mode3D = v;
}
//..............................................................................
//..............................................................................
//..............................................................................
void TXGrid::LibDrawStyle3D(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() ) E.SetRetVal( Mode3D );
  else  {
    Mode3D = Params[0].ToBool();
    InitIso(Mode3D);
  }
}
//..............................................................................
void TXGrid::LibScale(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal( Scale );
  else  {
    SetScale( (float)Params[0].ToDouble() );
  }
}
//..............................................................................
void TXGrid::LibSize(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  E.SetRetVal( Size );
  else
    Size = (float)Params[0].ToDouble();
}
//..............................................................................
void TXGrid::LibIsvalid(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( ED != NULL );
}
//..............................................................................
void TXGrid::LibGetMin(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( MinVal );
}
//..............................................................................
void TXGrid::LibGetMax(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal( MaxVal );
}
//..............................................................................
void TXGrid::LibPolygonMode(const TStrObjList& Params, TMacroError& E)  {
  if( Params.IsEmpty() )  {
    if( PolygonMode == GL_FILL )  E.SetRetVal<olxstr>("fill");
    else if( PolygonMode == GL_POINT )  E.SetRetVal<olxstr>("point");
    else if( PolygonMode == GL_LINE )  E.SetRetVal<olxstr>("line");
    return;
  }
  if( Params[0] == "fill" )  PolygonMode = GL_FILL;
  else if( Params[0] == "point" )  PolygonMode = GL_POINT;
  else if( Params[0] == "line" )  PolygonMode = GL_LINE;
  else throw TInvalidArgumentException(__OlxSourceInfo,
         olxstr("incorrect mode value: '") << Params[0] << '\'');
  if( PolygonMode == GL_POINT )  //create the points
    SetScale(Scale);
}
//..............................................................................
void TXGrid::ToDataItem(TDataItem& item, wxOutputStream& zos) const {
  item.AddField("empty", IsEmpty() );
  if( !IsEmpty() )  {
    item.AddField("visible", Visible());
    item.AddField("3D", Mode3D);
    item.AddField("moved", GridMoved);
    item.AddField("draw_mode", PolygonMode);
    item.AddField("max_val", MaxVal);
    item.AddField("min_val", MinVal);
    item.AddField("depth", Depth);
    item.AddField("size", Size);
    item.AddField("scale", Scale);
    item.AddField("max_x", MaxX);
    item.AddField("max_y", MaxY);
    item.AddField("max_z", MaxZ);
    for( int x=0; x < MaxX; x++ )  {
      for( int y=0; y < MaxY; y++ )  {
        zos.Write( ED->Data[x][y], sizeof(float)*MaxZ );
      }
    }
  }
}
//..............................................................................
void TXGrid::FromDataItem(const TDataItem& item, wxInputStream& zis) {
  Clear();
  bool empty = item.GetRequiredField("empty").ToBool();
  if( empty )  return;
  Visible( item.GetRequiredField("visible").ToBool() );
  Mode3D = item.GetRequiredField("3D").ToBool();
  GridMoved = item.GetRequiredField("moved").ToBool();
  PolygonMode = item.GetRequiredField("draw_mode").ToInt();
  MaxVal = item.GetRequiredField("max_val").ToDouble();
  MinVal = item.GetRequiredField("min_val").ToDouble();
  Depth = item.GetRequiredField("depth").ToDouble();
  Size = item.GetRequiredField("size").ToDouble();
  Scale = item.GetRequiredField("scale").ToDouble();
  InitGrid( item.GetRequiredField("max_x").ToInt(), 
            item.GetRequiredField("max_y").ToInt(),
            item.GetRequiredField("max_z").ToInt() );
  for( int x=0; x < MaxX; x++ )
    for( int y=0; y < MaxY; y++ )
      zis.Read( ED->Data[x][y], sizeof(float)*MaxZ );
  InitIso(Mode3D);
}
//..............................................................................
TLibrary*  TXGrid::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("xgrid") : name);
  lib->RegisterFunction<TXGrid>( new TFunction<TXGrid>(this,  &TXGrid::LibGetMin, "GetMin",
    fpNone, "Returns minimum value of the map") );
  lib->RegisterFunction<TXGrid>( new TFunction<TXGrid>(this,  &TXGrid::LibGetMax, "GetMax",
    fpNone, "Returns maximum value of the map") );
  lib->RegisterFunction<TXGrid>( new TFunction<TXGrid>(this,  &TXGrid::LibDrawStyle3D, "3D",
    fpNone|fpOne, "Returns/sets 3D drawing style") );
  lib->RegisterFunction<TXGrid>( new TFunction<TXGrid>(this,  &TXGrid::LibScale, "Scale",
    fpNone|fpOne, "Returns/sets current scale") );
  lib->RegisterFunction<TXGrid>( new TFunction<TXGrid>(this,  &TXGrid::LibSize, "Size",
    fpNone|fpOne, "Returns/sets current size") );
  lib->RegisterFunction<TXGrid>( new TFunction<TXGrid>(this,  &TXGrid::LibIsvalid, "IsValid",
    fpNone|fpOne, "Returns true if grid data is initialised") );
  lib->RegisterFunction<TXGrid>( new TFunction<TXGrid>(this,  &TXGrid::LibPolygonMode, "FillMode",
    fpNone|fpOne, "Returns/sets polygom mode for 3D display. Supported values: point, line, fill") );

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
PyObject* pySetVisible(PyObject* self, PyObject* args)  {
  bool v;
  if( !PyArg_ParseTuple(args, "b", &v) )
    return NULL;
  TXGrid::GetInstance()->Visible(v);
  Py_INCREF(Py_None);
  return Py_None;
}
//..............................................................................
PyObject* pyInitSurface(PyObject* self, PyObject* args)  {
  bool v;
  if( !PyArg_ParseTuple(args, "b", &v) )
    return NULL;
  TXGrid::GetInstance()->InitIso(v);
  Py_INCREF(Py_None);
  return Py_None;
}
//..............................................................................
PyObject* pyIsVisible(PyObject* self, PyObject* args)  {
  return Py_BuildValue("b", TXGrid::GetInstance()->Visible() );
}
//..............................................................................
//..............................................................................

static PyMethodDef XGRID_Methods[] = {
  {"Init", pyInit, METH_VARARGS, "initialises grid memory"},
  {"SetValue", pySetValue, METH_VARARGS, "sets grid value"},
  {"IsVisible", pyIsVisible, METH_VARARGS, "returns grid visibility status"},
  {"SetVisible", pySetVisible, METH_VARARGS, "sets grid visibility"},
  {"InitSurface", pyInitSurface, METH_VARARGS, "inits surface drawing"},
  {NULL, NULL, 0, NULL}
   };

void TXGrid::PyInit()  {
  Py_InitModule( "olex_xgrid", XGRID_Methods );
}
#endif // _NO_PYTHON
