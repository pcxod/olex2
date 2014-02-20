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

olxstr wrl::get_mat_str(const olxstr &primitive_name,
  TGraphicsStyle &style, olxdict<TGlMaterial, olxstr,
  TComparableComparator> &materials,
  const AGDrawObject *sender)
{
  if (sender != NULL && sender->GetParentGroup() != NULL)
    return get_mat_str(primitive_name, style, materials);
  olxstr mat_str;
  size_t lmi = style.IndexOfMaterial(primitive_name);
  if (lmi != InvalidIndex) {
    TGlMaterial& glm = style.GetPrimitiveStyle(lmi).GetProperties();
    lmi = materials.IndexOf(glm);
    if (lmi == InvalidIndex) {
      olxstr name = olxstr("mat") << (materials.Count()+1);
      materials.Add(glm, name);
      mat_str = (olxstr("DEF").stream(' ') << name << glm.ToWRL()).dest;
    }
    else
      mat_str = olxstr("USE ") << materials.GetValue(lmi);
  }
  return mat_str;
}

olxstr wrl::get_mat_str(const TGlMaterial& glm,
  olxdict<TGlMaterial, olxstr, TComparableComparator> &materials,
  const AGDrawObject *sender)
{
  if (sender != NULL && sender->GetParentGroup() != NULL) {
    return get_mat_str(sender->GetParentGroup()->GetActualMaterial(glm),
      materials);
  }
  size_t lmi = materials.IndexOf(&glm);
  if (lmi == InvalidIndex) {
    olxstr name = olxstr("mat") << (materials.Count()+1);
    materials.Add(glm, name);
    return (olxstr("DEF").stream(' ') << name << glm.ToWRL()).dest;
  }
  else
    return olxstr("USE ") << materials.GetValue(lmi);
}

