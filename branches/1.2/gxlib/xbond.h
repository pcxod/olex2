/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_xbond_H
#define __olx_gxl_xbond_H
#include "glrender.h"
#include "gdrawobject.h"
#include "glprimitive.h"
#include "gllabel.h"
#include "styles.h"
#include "sbond.h"
#include "talist.h"
BeginGxlNamespace()

class TXAtom;

class TXBond: public TSBond, public AGDrawObject  {
private:
  TXGlLabel* Label;
  bool label_forced;
  short FDrawStyle;
  static double FDefR;
  static int FDefM;
  static bool DefSelectable;
protected:
  void GetDefSphereMaterial(TGlMaterial &M);
  void GetDefRimMaterial(TGlMaterial &M);
  static void ValidateBondParams();
  static TGraphicsStyle *FBondParams;
  virtual bool IsMaskSaveable() const {  return false;  }
  virtual bool IsStyleSaveable() const {  return false; }
  virtual bool IsRadiusSaveable() const {  return false; }
  class TStylesClear: public AActionHandler  {
  public:
    TStylesClear(TGlRenderer& Render)  {  Render.OnStylesClear.Add(this);  }
    bool Enter(const IEObject *Sender, const IEObject *Data, TActionQueue *);
    bool Exit(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  };
  static TStylesClear *OnStylesClear;
  class TContextClear: public AActionHandler {
  public:
    TContextClear(TGlRenderer& Render);
    bool Enter(const IEObject *Sender, const IEObject *Data, TActionQueue *);
    bool Exit(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  };
  static void ClearStaticObjects()  {  GetStaticPrimitives().Clear();  }
public:
  TXBond(TNetwork* net, TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName=EmptyString());
  virtual ~TXBond();

  // multiple inheritance...
  void SetTag(index_t v) {  TSBond::SetTag(v);  }
  index_t GetTag() const {  return TSBond::GetTag();  }
  index_t IncTag()  {  return TSBond::IncTag();  }
  index_t DecTag()  {  return TSBond::DecTag();  }

  TXAtom& A() const {  return (TXAtom&)TSBond::A();  }
  TXAtom& B() const {  return (TXAtom&)TSBond::B();  }
  TXAtom& Another(const TSAtom& a) const {
    return (TXAtom&)TSBond::Another(a);
  }

  TXGlLabel& GetGlLabel() const {  return *Label;  }
  void UpdateLabel()  {  GetGlLabel().UpdateLabel();  }
  // creates legend up three levels (0 to 2)
  static olxstr GetLegend(const TSBond& B, const short level);

  void SetRadius(double V);
  double GetRadius() const {  return FParams[4]; }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &, vec3d &)  {  return false; }

  // for parameters of a specific primitive
  void ListParams(TStrList &List, TGlPrimitive *Primitive);
  // for internal object parameters
  void ListParams(TStrList &List);
  // fills the list with proposal primitives to construct object
  void ListPrimitives(TStrList &List) const;
  /* returns a list of static primitives. This list has the same order as
  primtives masks, so primitives names can be obtained for any particular mask
  */
  static TStringToList<olxstr, TGlPrimitive*> &GetStaticPrimitives() {
    //static TArrayList<TGlPrimitiveParams> FPrimitiveParams;
    static TStringToList<olxstr, TGlPrimitive*> sp;
    return sp;
  }
  // updates primitive properties from atoms
  void UpdateStyle();

  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data);

  void SetVisible(bool v)  {
    AGDrawObject::SetVisible(v);
    if (!v) {
      if (Label->IsVisible()) {
        Label->SetVisible(false);
        label_forced = true;
      }
    }
    else if (label_forced) {
      Label->SetVisible(true);
      label_forced = false;
    }
  }
  void Delete()  {
    TSBond::SetDeleted(true);
    SetVisible(false);
  }
  void ListDrawingStyles(TStrList &List);

  uint32_t GetPrimitiveMask() const;
  static void DefMask(int V);
  static int DefMask();
  static void DefR(double V);
  static double DefR();
  static bool IsSelectableByDef() { return DefSelectable; }
  static void SetSelectableByDef(bool v) { DefSelectable=v; }
  short DrawStyle() const {  return FDrawStyle; }

  void UpdatePrimitiveParams(TGlPrimitive *Primitive);
  void OnPrimitivesCleared();
  static int16_t Quality(int16_t Val);
  // should be called when atom coordinates have changed
  virtual void Update();

  const_strlist ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const;
  static const_strlist PovDeclare();

  const_strlist ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const;
  static const_strlist WrlDeclare();

  virtual const vec3d &GetBaseCrd() const;
  static TGraphicsStyle* GetParamStyle()  {  return FBondParams;  }
  static void CreateStaticObjects(TGlRenderer& parent);

  static olxstr_dict<olxstr> &NamesRegistry() {
    static olxstr_dict<olxstr> nr;
    return nr;
  }
};

EndGxlNamespace()
#endif
