/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "ebasis.h"
#include "dataitem.h"
UseEsdlNamespace()

// static data
float TEBasis::FMData[16];
float TEBasis::FMDataT[16];

TEBasis::TEBasis() {  Reset();  }
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
const TEBasis& TEBasis::operator = (const TEBasis &B)  {
  FCenter = B.GetCenter();
  FRX = B.GetRX();
  FRY = B.GetRY();
  FRZ = B.GetRZ();
  FMatrix = B.GetMatrix();
  FZoom = B.GetZoom();
  return B;
}
//..............................................................................
void  TEBasis::Reset()  {
  FMatrix.I();
  FRX = FRY = FRZ = 0;
  FCenter.Null();
  FZoom = 1;
}
//..............................................................................
void  TEBasis::TranslateX(double x){  FCenter[0] += x; FMData[12] += (float)x; };
//..............................................................................
void  TEBasis::TranslateY(double y){  FCenter[1] += y; FMData[13] += (float)y; };
//..............................................................................
void  TEBasis::TranslateZ(double z){  FCenter[2] += z; FMData[14] += (float)z; };
//..............................................................................
void  TEBasis::RotateX(double A)  {
  if( FRX == A )    return;
  const double RA = M_PI*(A-FRX)/180;
  FRX = A;
  mat3d M;  M.I();
  M[1][1] = cos(RA);
  M[1][2] = -sin(RA);
  M[2][1] = -M[1][2];
  M[2][2] = M[1][1];
  FMatrix *= M;
}
//..............................................................................
void  TEBasis::RotateY(double A)  {
  if( FRY == A )    return;
  const double RA = M_PI*(A-FRY)/180;
  FRY = A;
  mat3d M;  M.I();
  M[0][0] = cos(RA);
  M[0][2] = -sin(RA);
  M[2][0] = -M[0][2];
  M[2][2] =  M[0][0];
  FMatrix *= M;
}
//..............................................................................
void  TEBasis::RotateZ(double A)  {
  if( FRZ == A )    return;
  const double RA = M_PI*(A-FRZ)/180;
  FRZ = A;
  mat3d M;  M.I();
  M[0][0] = cos(RA);
  M[0][1] = -sin(RA);
  M[1][0] = -M[0][1];
  M[1][1] =  M[0][0];
  FMatrix *= M;
}
//..............................................................................
void TEBasis::ToDataItem(TDataItem& Item) const  {
  TDataItem& matr = Item.AddItem("matrix");
  matr.AddField("xx", FMatrix[0][0]);
  matr.AddField("xy", FMatrix[0][1]);
  matr.AddField("xz", FMatrix[0][2]);
  matr.AddField("yx", FMatrix[1][0]);
  matr.AddField("yy", FMatrix[1][1]);
  matr.AddField("yz", FMatrix[1][2]);
  matr.AddField("zx", FMatrix[2][0]);
  matr.AddField("zy", FMatrix[2][1]);
  matr.AddField("zz", FMatrix[2][2]);
  TDataItem& center = Item.AddItem("center");
  center.AddField("x", FCenter[0]);
  center.AddField("y", FCenter[1]);
  center.AddField("z", FCenter[2]);
  Item.AddField("zoom", FZoom);
}
bool TEBasis::FromDataItem(const TDataItem& Item)  {
  TDataItem *matr = Item.FindItem("matrix");
  if( matr == NULL )  return false;
  FMatrix[0][0] = matr->GetFieldByName("xx").ToDouble();
  FMatrix[0][1] = matr->GetFieldByName("xy").ToDouble();
  FMatrix[0][2] = matr->GetFieldByName("xz").ToDouble();
  FMatrix[1][0] = matr->GetFieldByName("yx").ToDouble();
  FMatrix[1][1] = matr->GetFieldByName("yy").ToDouble();
  FMatrix[1][2] = matr->GetFieldByName("yz").ToDouble();
  FMatrix[2][0] = matr->GetFieldByName("zx").ToDouble();
  FMatrix[2][1] = matr->GetFieldByName("zy").ToDouble();
  FMatrix[2][2] = matr->GetFieldByName("zz").ToDouble();

  TDataItem *center = Item.FindItem("center");
  if( center == NULL )  return false;
  FCenter[0] = center->GetFieldByName("x").ToDouble();
  FCenter[1] = center->GetFieldByName("y").ToDouble();
  FCenter[2] = center->GetFieldByName("z").ToDouble();
  const double z = Item.FindField("zoom", "-1").ToDouble();
  if( z != -1 )
    FZoom = z;
  return true;
}
