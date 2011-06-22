/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef xrestraintH
#define xrestraintH
#include "gxbase.h"
BeginGxlNamespace()

/*
class TXConsraint: public AGDrawObject  { // TGlMouseListener
private:
  TSAtomPList Atoms;
  double Value, Esd;

protected:
  TEStringList* FindPrimitiveParams(TGlPrimitive *P);
  static TEList FPrimitiveParams;
  void ValidateRadius(TGraphicsStyle *GS);
  void ValidateDS(TGraphicsStyle *GS);
  static void ValidateAtomParams();
  static TXAtomStylesClear *FXAtomStylesClear;
protected:
  static float FTelpProb, FQPeakScale;
  static TGraphicsStyle *FAtomParams;
public:
  TXAtom(const TEString& collectionName, TSAtom *A, TGlRender *Render);
  virtual ~TXAtom();
  void Create(const TEString& cName = EmptyString);

  static TEStringList FStaticObjects;
  void CreateStaticPrimitives();

  static TEString GetLegend(TSAtom *A, const short Level=2); // returns full legend with symm code
  // returns level of the given legend (number of '.', basically)
  static short LegendLevel(const TEString& legend);
  static TEString GetLabelLegend(TSAtom *A);
  // returns full legend for the label. e.g. "Q.Q1"

  static void GetDefSphereMaterial(TSAtom *A, TGlMaterial &M);
  static void GetDefRimMaterial(TSAtom *A, TGlMaterial &M);

  static void DefRad(short V);
  static void DefDS(short V);
  static void DefSphMask(int V);
  static void DefElpMask(int V);
  static void DefNpdMask(int V);
  static void TelpProb(float V);
  static void DefZoom(float V);

  static short DefRad(); // default radius
  static short DefDS();     // default drawing style
  static int   DefSphMask();  // default mask for spherical atoms
  static int   DefElpMask();  // default mask for elliptical atoms
  static int   DefNpdMask();  // default mas for elliptical atoms eith NPD ellipsoid
  static float TelpProb();    // to use with ellipsoids
  static float DefZoom();    // to use with ellipsoids

  static float QPeakScale();    // to use with q-peaks
  static void QPeakScale(float V);    // to use with q-peaks

  void CalcRad(short DefAtomR);

  inline operator TSAtom* () const {  return FAtom;  }
  
  inline TSAtom* Atom() const  {  return FAtom; }
  void ApplyStyle(TGraphicsStyle *S);
  void UpdateStyle(TGraphicsStyle *S);

  void Zoom(float Z);
  inline double Zoom()  {  return Params()[1]; }

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(TVPointD &Max, TVPointD &Min);

  void ListParams(TEStringList &List, TGlPrimitive *Primitive);
  // for parameters of a specific primitive
  void ListParams(TEStringList &List);
  void ListPrimitives(TEStringList &List) const;
  // fills the list with proposal primitives to construct object
  TGraphicsStyle* Style();
  void UpdatePrimitives(__int32 Mask);

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data);

  inline bool Deleted()  const {  return AGDrawObject::Deleted(); }
  void Deleted(bool v)         {  AGDrawObject::Deleted(v);  FAtom->Deleted(v); }
  void ListDrawingStyles(TEStringList &List);
  inline short DrawStyle() const {  return FDrawStyle; }
  void DrawStyle(short V);

  void UpdatePrimitiveParams(TGlPrimitive *GlP);
  void OnPrimitivesCleared();

  void Quality(const short Val);

};
*/
EndGxlNamespace()
#endif
