/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "wrldraw.h"
#include "styles.h"
#include "gdrawobject.h"
#include "glgroup.h"

olxstr wrl_deal_with_material(const TGlMaterial& glm,
  olx_cdict<TGlMaterial, olxstr> &materials, bool back)
{
  size_t lmi = materials.IndexOf(glm);
  olxstr name = olxstr("mat") << (back ? 'b' : 'f'),
    mat_str;
  if (lmi == InvalidIndex) {
    name << (materials.Count()+1);
    materials.Add(glm, name);
    mat_str = (olxstr("DEF").stream(' ') << name << glm.ToWRL(back)).dest;
  }
  else {
    olxstr &v = materials.GetValue(lmi);
    if (!v.Contains(name)) {
      name << (materials.Count()+1);
      v << ',' << name;
      materials.Add(glm, name);
      mat_str = (olxstr("DEF").stream(' ') << name << glm.ToWRL(back)).dest;
    }
    else {
      TStrList toks(v, ',');
      if (toks[0].StartsFrom(name)) {
        name = toks[0];
      }
      else if (toks.Count() > 1) {
        name = toks[1];
      }
      mat_str = olxstr("USE ") << name;
    }
  }
  return mat_str;
}

olxstr wrl::get_mat_str(const olxstr &primitive_name,
  TGraphicsStyle &style, olx_cdict<TGlMaterial, olxstr> &materials,
  const AGDrawObject *sender, bool back)
{
  if (sender != NULL && sender->GetParentGroup() != NULL) {
    return get_mat_str(primitive_name, style, materials);
  }
  size_t lmi = style.IndexOfMaterial(primitive_name);
  if (lmi != InvalidIndex) {
    TGlMaterial& glm = style.GetPrimitiveStyle(lmi).GetProperties();
    return wrl_deal_with_material(glm, materials, back);
  }
  return EmptyString();
}

olxstr wrl::get_mat_str(const TGlMaterial& glm,
  olx_cdict<TGlMaterial, olxstr> &materials,
  const AGDrawObject *sender, bool back)
{
  if (sender != NULL && sender->GetParentGroup() != NULL) {
    return get_mat_str(sender->GetParentGroup()->GetActualMaterial(glm),
      materials, 0, back);
  }
  return wrl_deal_with_material(glm, materials, back);
}
