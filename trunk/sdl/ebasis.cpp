//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "ebasis.h"
#include "dataitem.h"

UseEsdlNamespace()
// TBasis function bodies
//----------------------------------------------------------------------------//
TEBasis::TEBasis() {
  Reset();
}
//..............................................................................
TEBasis::TEBasis(const TEBasis &B)  {
  *this = B;
}
//..............................................................................
TEBasis::~TEBasis()  {
  return;
}
//..............................................................................
void TEBasis::SetZoom(double v)   {  FZoom = v; }
//..............................................................................
void TEBasis::CopyMatrix()  {
  FMData[0] = (float)FMatrix[0][0];  FMData[1] = (float)FMatrix[0][1];  FMData[2] = (float)FMatrix[0][2];
  FMData[4] = (float)FMatrix[1][0];  FMData[5] = (float)FMatrix[1][1];  FMData[6] = (float)FMatrix[1][2];
  FMData[8] = (float)FMatrix[2][0];  FMData[9] = (float)FMatrix[2][1];  FMData[10] = (float)FMatrix[2][2];
  FMData[3] = FMData[7] = FMData[11] = 0;
  FMData[12] = (float)FCenter[0];  FMData[13] = (float)FCenter[1];  FMData[14] = (float)FCenter[2];
  FMData[15] = 1;

  FMDataT[0] = (float)FMatrix[0][0];  FMDataT[1] = (float)FMatrix[1][0];  FMDataT[2] = (float)FMatrix[2][0];
  FMDataT[4] = (float)FMatrix[0][1];  FMDataT[5] = (float)FMatrix[1][1];  FMDataT[6] = (float)FMatrix[2][1];
  FMDataT[8] = (float)FMatrix[0][2];  FMDataT[9] = (float)FMatrix[1][2];  FMDataT[10] = (float)FMatrix[2][2];
  FMDataT[3] = FMDataT[7] = FMDataT[11] = 0;
  FMDataT[12] = (float)FCenter[0];  FMDataT[13] = (float)FCenter[1];  FMDataT[14] = (float)FCenter[2];
  FMDataT[15] = 1;
}
//..............................................................................
const TEBasis& TEBasis::operator  = (const TEBasis &B)  {
  FCenter = B.GetCenter();
  FRX = B.GetRX();
  FRY = B.GetRY();
  FRZ = B.GetRZ();
  FMatrix = B.GetMatrix();
  FZoom = B.GetZoom();
  CopyMatrix();
  return B;
}
//..............................................................................
void  TEBasis::Reset()  {
  FMatrix.I();
  FRX = FRY = FRZ = 0;
  FCenter.Null();
  FZoom = 1;
  CopyMatrix();
}
//..............................................................................
void  TEBasis::TranslateX(double x){  FCenter[0] += x; FMData[12] += (float)x; };
//..............................................................................
void  TEBasis::TranslateY(double y){  FCenter[1] += y; FMData[13] += (float)y; };
//..............................................................................
void  TEBasis::TranslateZ(double z){  FCenter[2] += z; FMData[14] += (float)z; };
//..............................................................................
void  TEBasis::RotateX( double A)  {
  if( FRX == A )    return;
  double RA = M_PI*(A-FRX)/180;
  FRX = A;
  mat3d M;  M.I();
  M[1][1] = cos(RA);
  M[1][2] = -sin(RA);
  M[2][1] = -M[1][2];
  M[2][2] = M[1][1];
  FMatrix *= M;
  CopyMatrix();
}
//..............................................................................
void  TEBasis::RotateY( double A)  {
  if( FRY == A )    return;
  double RA = M_PI*(A-FRY)/180;
  FRY = A;
  mat3d M;  M.I();
  M[0][0] = cos(RA);
  M[0][2] = -sin(RA);
  M[2][0] = -M[0][2];
  M[2][2] =  M[0][0];
  FMatrix *= M;
  CopyMatrix();
}
//..............................................................................
void  TEBasis::RotateZ( double A)  {
  if( FRZ == A )    return;
  double RA = M_PI*(A-FRZ)/180;
  FRZ = A;
  mat3d M;  M.I();
  M[0][0] = cos(RA);
  M[0][1] = -sin(RA);
  M[1][0] = -M[0][1];
  M[1][1] =  M[0][0];
  FMatrix *= M;
  CopyMatrix();
}
//..............................................................................
void TEBasis::ToDataItem(TDataItem *Item) const  {
  TDataItem *matr = Item->AddItem("matrix");
  matr->AddField("xx", FMatrix[0][0]);
  matr->AddField("xy", FMatrix[0][1]);
  matr->AddField("xz", FMatrix[0][2]);
  matr->AddField("yx", FMatrix[1][0]);
  matr->AddField("yy", FMatrix[1][1]);
  matr->AddField("yz", FMatrix[1][2]);
  matr->AddField("zx", FMatrix[2][0]);
  matr->AddField("zy", FMatrix[2][1]);
  matr->AddField("zz", FMatrix[2][2]);
  TDataItem *center = Item->AddItem("center");
  center->AddField("x", FCenter[0]);
  center->AddField("y", FCenter[1]);
  center->AddField("z", FCenter[2]);
}
bool TEBasis::FromDataItem(TDataItem *Item)
{
  TDataItem *matr = Item->FindItem("matrix");
  if( matr == NULL )  return false;
  FMatrix[0][0] = matr->GetFieldValue("xx").ToDouble();
  FMatrix[0][1] = matr->GetFieldValue("xy").ToDouble();
  FMatrix[0][2] = matr->GetFieldValue("xz").ToDouble();
  FMatrix[1][0] = matr->GetFieldValue("yx").ToDouble();
  FMatrix[1][1] = matr->GetFieldValue("yy").ToDouble();
  FMatrix[1][2] = matr->GetFieldValue("yz").ToDouble();
  FMatrix[2][0] = matr->GetFieldValue("zx").ToDouble();
  FMatrix[2][1] = matr->GetFieldValue("zy").ToDouble();
  FMatrix[2][2] = matr->GetFieldValue("zz").ToDouble();

  TDataItem *center = Item->FindItem("center");
  if( center == NULL )  return false;
  FCenter[0] = center->GetFieldValue("x").ToDouble();
  FCenter[1] = center->GetFieldValue("y").ToDouble();
  FCenter[2] = center->GetFieldValue("z").ToDouble();
  CopyMatrix();
  return true;
}

