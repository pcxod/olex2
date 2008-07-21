#ifdef __BORLANC__
  #pragma hdrstop
#endif

#include "xlattice.h"
#include "glrender.h"
#include "styles.h"
#include "gpcollection.h"
#include "glscene.h"

TXLattice::TXLattice(const olxstr& collectionName, TGlRender *Render) :
  TGlMouseListener(collectionName, Render) {

  Fixed = false;
  Size = 4;

  Move2D(false);
  Moveable(true);
  Zoomable(false);

}
//..............................................................................
TXLattice::~TXLattice()  {
}
//..............................................................................
void TXLattice::Create(const olxstr& cName)  {
  if( cName.Length() != 0 )  SetCollectionName(cName);
  TGPCollection *GPC;
  TGraphicsStyle *GS;
  TGlMaterial GlM;
  const TGlMaterial *SGlM;
  GlM.SetFlags(sglmAmbientF);
  GlM.AmbientF = 0;
  GPC = Parent()->NewCollection( GetCollectionName() );
  GS = GPC->Style();
  GPC->AddObject(this);

  Lines = GPC->NewPrimitive("Lines");
  SGlM = GS->Material("Lines");
  if( !SGlM->Mark() )  Lines->SetProperties(SGlM);
  else  {
    GlM.SetIdentityDraw(false);
    GlM.SetTransparent(false);
    Lines->SetProperties(&GlM);
  }

  Lines->Type(sgloLines);
  // initialise data
  SetSize( GetSize() );

  TGlPrimitive* GlP = GPC->NewPrimitive("Label");  // labels
  SGlM = GS->Material("Label");
  if( !SGlM->Mark() )  GlP->SetProperties(SGlM);
  else  {
    GlM.SetIdentityDraw(true);
    GlM.SetTransparent(false);
    GlP->SetProperties(&GlM);
  }
  GlP->Type(sgloText);
  GlP->Font( Parent()->Scene()->DefFont() );
}
//..............................................................................
bool TXLattice::Orient(TGlPrimitive *P)  {
  if( Fixed )  {

    vec3d c = Parent()->GetBasis().GetCenter();
    c *= -1;
    Parent()->GlTranslate( c );

    mat3d m (Parent()->GetBasis().GetMatrix());
    m.Transpose();
    m = Basis.GetMatrix() * m;
    Parent()->GlOrient(  m );

    Parent()->GlTranslate( Basis.GetCenter() );
  }
  else  {
    Parent()->GlTranslate( Basis.GetCenter() );
  }
  vec3d vec;
  if( P->Type() == sgloLines )  {
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
  if( !Moveable() )  return true;
  return TGlMouseListener::OnMouseDown(Sender, Data);
}
//..............................................................................
bool TXLattice::OnMouseUp(const IEObject *Sender, const TMouseData *Data)  {
  if( !Moveable() )  return true;
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
    Basis.Orient( Parent()->GetBasis().GetMatrix() );
    Basis.SetCenter(c);
  }
  else  {
    vec3d c = Basis.GetCenter();
    Basis.Reset();
    Basis.SetCenter(c);
  }
  Move2D(v);
  Fixed = v;
}


