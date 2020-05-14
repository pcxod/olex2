/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "solid_angles.h"
#include "gxapp.h"

//.............................................................................
APointAnalyser *APointAnalyser::FromDataItem(const TDataItem &di) {
  olxstr id = di.GetFieldByName("IdString");
  APointAnalyser *(*f)(const TDataItem &di) = Registry().Find(id, NULL);
  if (f == 0) {
    return 0;
  }
  return (*f)(di.GetItemByName("Object"));
}
//.............................................................................
void APointAnalyser::ToDataItem(TDataItem &di) const {
  di.AddField("IdString", GetIdString());
  ToDataItem_(di.AddItem("Object"));
}
//.............................................................................
//.............................................................................
//.............................................................................
PointAnalyser::PointAnalyser(TXAtom &c)
: center(&c), clone(false), cs(*(new olx_critical_section())),
  areas(*(new olxstr_dict<size_t, false>()))
{
  if (!c.IsCreated()) {
    return;
  }
  colors.SetCount(7);
  colors[0] = 0XFF0000;
  colors[1] = 0X009933;
  colors[2] = 0xFFFF00;
  colors[3] = 0x0000FF;
  colors[4] = 0xFF33CC;
  colors[5] = 0xFF9966;
  colors[6] = 0x00FFFF;

  alpha = 0x9c;
  dry_run = false;
  olx_pdict<uint32_t, int> highest;
  const TLattice &latt = ((TSAtom &)center()).GetParent();
  for (size_t i = 0; i < latt.GetObjects().atoms.Count(); i++) {
    TXAtom &a = (TXAtom &)latt.GetObjects().atoms[i];
    if (!a.IsAvailable()) {
      continue;
    }
    size_t net_id = a.GetNetwork().GetOwnerId();
    if (net_id == ~0) {
      continue;
    }
    if (net_id >= colors.Count()) {
      colors.SetCount(net_id + 1, olx_list_init::zero());
    }
    if (a.IsGrouped()) {
      colors[net_id] = a.GetParentGroup()->GetGlM().AmbientF.GetRGB();
    }
    else {
      if (net_id < 7) {
        continue;
      }
      if (a.GetType().z > highest.Find(net_id, 0)) {
        TGlMaterial glm;
        colors[net_id] = a.GetPrimitives().GetStyle().GetMaterial("Sphere", glm)
          .AmbientF.GetRGB();
        highest.Add(net_id, a.GetType().z);
      }
    }
  }
  areas.Clear();
}
//.............................................................................
uint32_t PointAnalyser::Analyse(vec3f &p_) {
  int r = 0, g = 0, b = 0;
  vec3f p = p_;
  float maxd = 1;
  sorted::ObjectPrimitive<uint32_t> added;
  const TLattice &latt = ((TSAtom &)center()).GetParent();
  const TSAtom &cnt = center();
  for (size_t i = 0; i < latt.GetObjects().atoms.Count(); i++) {
    TSAtom &a = latt.GetObjects().atoms[i];
    if (&a == &cnt || !a.IsAvailable())
      continue;
    vec3f v = a.crd() - cnt.crd();
    float dp = p.DotProd(v);
    if (dp < 0) {
      continue;
    }
    float d = (v - p*dp).Length();
    size_t net_id = a.GetNetwork().GetOwnerId();
    if (d < a.GetType().r_custom) {
      if (!added.AddUnique(net_id).b) {
        continue;
      }
      if (net_id < colors.Count()) {
        uint32_t c = colors[net_id];
        r += OLX_GetRValue(c);
        g += OLX_GetGValue(c);
        b += OLX_GetBValue(c);
      }
      if (dp > maxd)
        maxd = dp;
    }
  }
  // update area info
  if (!dry_run) {
    cs.enter();
    areas.Add(olxstr(';').Join(added), 0)++;
    cs.leave();
  }
  if (emboss)
    p_.NormaliseTo(maxd);
  if (added.IsEmpty()) {
    return 0x00ffffff | ((uint32_t)alpha << 24);
  }
  else {
    int s = (int)added.Count();
    if (s > 1) { // make darker
      s++;
    }
    r /= s;
    g /= s;
    b /= s;
    return OLX_RGBA(
      r > 255 ? 255 : r,
      g > 255 ? 255 : g,
      b > 255 ? 255 : b,
      alpha);
  }
}
//.............................................................................
void PointAnalyser::ToDataItem_(TDataItem &di) const {
  if (!center.is_valid()) {
    return;
  }
  center().GetRef().ToDataItem(di.AddItem("AtomRef"));
  di.AddField("Colors", olxstr(',').Join(colors))
    .AddField("emboss", emboss).
    AddField("alpha", alpha);
  ContentList cl = center().CAtom().GetParent()->GetContentList();
  TDataItem &rdi = di.AddItem("Radii");
  for (size_t i = 0; i < cl.Count(); i++) {
    rdi.AddField(cl[i].element->symbol, cl[i].element->r_custom);
  }
}
//.............................................................................
APointAnalyser *PointAnalyser::Load(const TDataItem &di) {
  TXApp &app = TXApp::GetInstance();
  TSAtom::Ref ref(di.GetItemByName("AtomRef"), TGXApp::GetInstance());
  TSAtom * sa = app.XFile().GetLattice().FindSAtom(ref);
  if (sa == 0) {
    return 0;
  }
  PointAnalyser *rv = new PointAnalyser((TXAtom &)*sa);
  rv->emboss = di.GetFieldByName("emboss").ToBool();
  rv->alpha = di.GetFieldByName("alpha").ToUInt();
  TStrList cls(di.GetFieldByName("Colors"), ',');
  if (cls.Count() > rv->colors.Count())
    rv->colors.SetCount(cls.Count());
  for (size_t i = 0; i < cls.Count(); i++) {
    rv->colors[i] = cls[i].ToUInt();
  }
  TDataItem &rdi = di.GetItemByName("Radii");
  for (size_t i = 0; i < rdi.FieldCount(); i++) {
    rv->radii.Add(XElementLib::FindBySymbol(rdi.GetFieldName(i)),
      rdi.GetFieldByIndex(i).ToDouble());
  }
  return rv;
}
//.............................................................................
void PointAnalyser::GetReady() {
  for (size_t i = 0; i < radii.Count(); i++) {
    radii.GetKey(i)->r_custom = radii.GetValue(i);
  }
}
//.............................................................................
