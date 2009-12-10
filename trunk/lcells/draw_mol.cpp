//---------------------------------------------------------------------------


#pragma hdrstop

#include "draw_mol.h"
#include "cplanes.h"
#include "main.h"
#include "conindex.h"
#include "moldraw.h"
#include "bapp.h"
#include "cif.h"
#include "ins.h"
#include "mol.h"


#include "efile.h"

// cif parsing
//---------------------------------------------------------------------------
int DrawPointsSortZ(const TDrawSort& I, const TDrawSort& I1)  {  // sorting atoms reverse to Z
  return I.P[2] - I1.P[2];
}
int DrawPointsSortX(TDrawSort* const &I, TDrawSort* const &I1)  {  // sorting atoms reverse to Z
  return I->P[0] - I1->P[0];
}
int DrawPointsSortY(TDrawSort* const &I, TDrawSort* const &I1)  // sorting atoms reverse to Z
{
  return I->P[1] - I1->P[1];
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TOrganiser::TOrganiser(Graphics::TBitmap *Bmp)  {
  MouseDown = false;
  FBitmap = Bmp;

  FAtomsInfo = new TAtomsInfo( TBasicApp::GetBaseDir() + "ptablex.dat" );
  FXFile = new TXFile();
  FXFile->RegisterFileFormat( new xlib::TCif, "cif");
  FXFile->RegisterFileFormat( new xlib::TMol, "mol");
  FXFile->RegisterFileFormat( new xlib::TIns, "ins");
}
//..............................................................................
__fastcall TOrganiser::~TOrganiser()  {
  delete FXFile;
  delete FAtomsInfo;
}
//..............................................................................
void _fastcall TOrganiser::CalcZoom()  {
  if( XFile->LastLoader() == NULL )  return;
  TTypeList<TDrawSort*> DrawSort;
  vec3d Min, Max, Center;
  double w = FBitmap->Width, h = FBitmap->Height;
  size_t ac = 0;
  for( size_t i=0; i < XFile->GetLattice().AtomCount(); i++ )  {
    TSAtom& sa = XFile->GetLattice().GetAtom(i);
    if( !sa.IsAvailable() )  continue;
    ac++;
    TDrawSort* DS = new TDrawSort;
    DS->A = &sa;
    DS->P = DS->A->crd();
    DS->P *= Basis.GetMatrix();
    DrawSort.AddACopy(DS);
    Center += DS->A->crd();
  }
  if( ac != 0 )  Center /= ac;
  Basis.SetCenter(Center);
///////////////////////////////////////////////////////////////////////////////
// zoom calculation
  DrawSort.QuickSorter.SortSF( DrawSort, DrawPointsSortX);
  Min[0] = DrawSort[0]->P[0];
  Max[0] = DrawSort[DrawSort.Count()-1]->P[0];

  DrawSort.QuickSorter.SortSF( DrawSort, DrawPointsSortY);
  Min[1] = DrawSort[0]->P[1];
  Max[1] = DrawSort[DrawSort.Count()-1]->P[1];

  if( (Max[0]-Min[0]) && (Max[1]-Min[1]) )  {
    double k1 = fabs(w/(Max[0]-Min[0])),
           k2 = fabs(h/(Max[1]-Min[1]));
    if( k1 < k2 )  Basis.SetZoom(k1);
    else           Basis.SetZoom(k2);
  }
  else  {
    if( (Max[1]-Min[1]) )      Basis.SetZoom( fabs(h/(Max[1]-Min[1])) );
    if( (Max[0]-Min[0]) )      Basis.SetZoom( fabs(w/(Max[0]-Min[0])) );
  }
  Basis.SetZoom( Basis.GetZoom() / 1.5 );
  if( Basis.GetZoom() < 0 )        Basis.SetZoom(10);
  if( Basis.GetZoom() > 100 )      Basis.SetZoom(100);
  for( size_t i=0; i < DrawSort.Count(); i++ )
    delete DrawSort[i];
}
//..............................................................................
void _fastcall TOrganiser::Update()  {
  if( FBitmap == NULL )  return;
  vec3d Z(0,0,1);
  vec3d_list Crd;
  for( size_t i=0; i < XFile->GetLattice().AtomCount(); i++ )  {
    if( !XFile->GetLattice().GetAtom(i).IsAvailable() ) continue;
    Crd.AddNew(XFile->GetLattice().GetAtom(i).crd());
  }
  if( Crd.Count() > 3 )  {
    GetPlane(Crd, Z);
    Basis.OrientNormal(Z);
  }
  CalcZoom();
}
//..............................................................................
void _fastcall TOrganiser::Draw()  {
  if( XFile->LastLoader() == NULL )  return;

  TSAtom *A;
  TSBond *B;
  TCanvas *C = FBitmap->Canvas;

  TRect R(0, 0, FBitmap->Width, FBitmap->Height);
  C->Brush->Color = clWhite;
  C->FillRect(R);

  vec3d P;
  double w = FBitmap->Width, h = FBitmap->Height;
  double hw = w/2, hh = h/2;
  mat3d m( mat3d::Transpose(Basis.GetMatrix()) );
  TTypeList<TDrawSort> DrawSort;

  for( size_t i=0; i < XFile->GetLattice().AtomCount(); i++ )  {
    if( !XFile->GetLattice().GetAtom(i).IsAvailable() )  continue;
    TDrawSort& DS = DrawSort.AddNew();
    DS.A = &XFile->GetLattice().GetAtom(i);
    DS.P = DS.A->crd();
    DS.P -= Basis.GetCenter();
    DS.P = m * DS.P;
    DS.P *= Basis.GetZoom();
    DS.P[0] += hw;
    DS.P[1] += hh;
  }
  DrawSort.QuickSorter.SortSF(DrawSort, DrawPointsSortZ);

  for( size_t i=0; i < DrawSort.Count(); i++ )
    DrawSort[i].A->SetTag(i);

  for( size_t i=0; i < XFile->GetLattice().BondCount(); i++ )  {
    TSBond* B = &XFile->GetLattice().GetBond(i);
    if( B->A().GetAtomInfo().GetMr() > B->B().GetAtomInfo().GetMr() )
      C->Pen->Color = (TColor)B->A().GetAtomInfo().GetDefColor();
    else
      C->Pen->Color = (TColor)B->B().GetAtomInfo().GetDefColor();
    C->MoveTo(DrawSort[B->A().GetTag()].P[0], DrawSort[B->A().GetTag()].P[1]);
    C->LineTo(DrawSort[B->B().GetTag()].P[0], DrawSort[B->B().GetTag()].P[1]);
  }

  if( Basis.GetZoom() < 15 )
    goto exit;
  C->Pen->Color = clBlack;

//  DrawSort->Sort(DrawPointsSortZ);
  for( size_t i=0; i < DrawSort.Count(); i++ )  {
    TDrawSort& DS = DrawSort[i];
    C->Brush->Color = (TColor)DS.A->GetAtomInfo().GetDefColor();
    double rad = DS.A->GetAtomInfo().GetRad() * Basis.GetZoom();
    C->Ellipse(
      (DS.P[0] - rad),
      (DS.P[1] - rad),
      (DS.P[0] + rad),
      (DS.P[1] + rad)
      );
    rad = rad/2 + 0.3;
    if( DS.A->GetAtomInfo() != iHydrogenIndex )  {
      C->Brush->Color = clWhite;
      C->TextOut(DS.P[0]+rad, DS.P[1]+rad, DS.A->GetLabel().c_str());
    }
  }
exit:
  dlgMolDraw->FormPaint(NULL);
}
//..............................................................................
void __fastcall TOrganiser::OnResize(TObject *Sender)  {
  CalcZoom();
  Draw();
}
//..............................................................................
void __fastcall TOrganiser::OnClick(TObject *Sender)  {
  MouseDown = false;
}
//..............................................................................
bool __fastcall TOrganiser::OnMouseDown(TObject *Sender, TMouseButton B, TShiftState Shift, int X, int Y)  {
  MouseX = X;
  MouseY = Y;
  MouseDown = true;
  Button = B;
  return false;
}
//..............................................................................
bool __fastcall TOrganiser::OnMouseMove(TObject *Sender, TShiftState Shift, int X, int Y)  {
  if( MouseDown && (Button == mbLeft) )  {
    int xinc = X - MouseX;
    int yinc = (Y - MouseY);
    Basis.RotateY(Basis.GetRY() + xinc);
    Basis.RotateX(Basis.GetRX() + yinc);
    Draw();
    MouseX = X;
    MouseY = Y;
    return true;
  }
  if( MouseDown && (Button == mbRight) )  {
    double yinc = (double)(Y - MouseY)/10;

    if( ((Basis.GetZoom() + yinc) > 0) &&
        ((Basis.GetZoom() + yinc) < 100) )
      Basis.SetZoom( Basis.GetZoom() + yinc );

    Draw();
    MouseX = X;
    MouseY = Y;
    return true;
  }
  MouseX = X;
  MouseY = Y;
  return false;
}
//..............................................................................
bool __fastcall TOrganiser::OnMouseUp(TObject *Sender, TMouseButton Button,  TShiftState Shift, int X, int Y)  {
  MouseDown = false;
  return false;
}
#pragma package(smart_init)






