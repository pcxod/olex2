#ifdef __BORLANC__
  #pragma hdrstop
#endif

#include "xlattice.h"
#include "glrender.h"
#include "styles.h"
#include "gpcollection.h"
#include "glscene.h"
#include "glprimitive.h"

TXLattice::TXLattice(TGlRenderer& Render, const olxstr& collectionName) :
  TGlMouseListener(Render, collectionName) {

  Fixed = false;
  Size = 4;

  SetMove2D(false);
  SetMoveable(true);
  SetZoomable(false);
}
//..............................................................................
TXLattice::~TXLattice()  {
}
//..............................................................................
void TXLattice::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  Lines = &GPC.NewPrimitive("Lines", sgloLines);
  TGlMaterial GlM;
  GlM.SetFlags(sglmAmbientF);
  GlM.AmbientF = 0;
  GlM.SetIdentityDraw(false);
  GlM.SetTransparent(false);

  Lines->SetProperties( GS.GetMaterial("Lines", GlM) );

  // initialise data
  SetSize( GetSize() );

  TGlPrimitive& glpLabel = GPC.NewPrimitive("Label", sgloText);  // labels
  TGlMaterial GlM1;
  GlM1.SetIdentityDraw(true);
  GlM1.SetTransparent(false);
  
  glpLabel.SetProperties( GS.GetMaterial("Label", GlM1) );

  glpLabel.SetFont( Parent.GetScene().DefFont() );
}
//..............................................................................
bool TXLattice::Orient(TGlPrimitive& P)  {
  if( Fixed )  {
    vec3d c = Parent.GetBasis().GetCenter();
    c *= -1;
    Parent.GlTranslate( c );

    mat3d m (Parent.GetBasis().GetMatrix());
    m.Transpose();
    m = Basis.GetMatrix() * m;
    Parent.GlOrient(  m );

    Parent.GlTranslate( Basis.GetCenter() );
  }
  else  {
    Parent.GlTranslate( Basis.GetCenter() );
  }
  vec3d vec;
  if( P.GetType() == sgloLines )  {
    glPointSize(5);
    glBegin(GL_POINTS);
    for( int i=-Size; i  < Size; i++ )  {
      for( int j=-Size; j  < Size; j++ )  {
        for( int k=-Size; k  < Size; k++ )  {
          vec[0] = i;  vec[1] = j;  vec[2] = k;
          vec *= LatticeBasis;
          glVertex3d(vec[0], vec[1], vec[2]);
        }
      }
    }
    glEnd();
  }
  return true;
}
//..............................................................................
bool TXLattice::GetDimensions(vec3d &Max, vec3d &Min)  {
  return false;
}
//..............................................................................
bool TXLattice::OnMouseDown(const IEObject *Sender, const TMouseData *Data)  {
  if( !IsMoveable() )  return true;
  return TGlMouseListener::OnMouseDown(Sender, Data);
}
//..............................................................................
bool TXLattice::OnMouseUp(const IEObject *Sender, const TMouseData *Data)  {
  if( !IsMoveable() )  return true;
  return TGlMouseListener::OnMouseUp(Sender, Data);
}
//..............................................................................
bool TXLattice::OnMouseMove(const IEObject *Sender, const TMouseData *Data)  {
  TGlMouseListener::OnMouseMove(Sender, Data);
  return true;
}
//..............................................................................
void TXLattice::SetSize(short v)  {
  if( v < 1 || v > 10 )  v = 2;
  Size = v;
//  Lines->Data().Resize(3, 24);

}
//..............................................................................
void TXLattice::SetFixed(bool v )  {
  if( v )  {
    vec3d c = Basis.GetCenter();
    Basis.Orient( Parent.GetBasis().GetMatrix() );
    Basis.SetCenter(c);
  }
  else  {
    vec3d c = Basis.GetCenter();
    Basis.Reset();
    Basis.SetCenter(c);
  }
  SetMove2D(v);
  Fixed = v;
}


