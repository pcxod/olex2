/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "povdraw.h"
#include "styles.h"
#include "gdrawobject.h"
#include "glgroup.h"

olxstr pov::get_mat_name(const olxstr &primitive_name,
  TGraphicsStyle &style, olxdict<TGlMaterial, olxstr,
  TComparableComparator> &materials,
  const AGDrawObject *sender)
{
  if (sender != NULL && sender->GetParentGroup() != NULL)
    return get_mat_name(primitive_name, style, materials);
  olxstr mat_name;
  size_t lmi = style.IndexOfMaterial(primitive_name);
  if( lmi != InvalidIndex )  {
    TGlMaterial& glm = style.GetPrimitiveStyle(lmi).GetProperties();
    lmi = materials.IndexOf(glm);
    if( lmi == InvalidIndex )
      mat_name = materials.Add(glm, olxstr("mat") << (materials.Count()+1));
    else
      mat_name = materials.GetValue(lmi);
  }
  return mat_name;
}

olxstr pov::get_mat_name(const TGlMaterial& glm,
  olxdict<TGlMaterial, olxstr, TComparableComparator> &materials,
  const AGDrawObject *sender)
{
  if (sender != NULL && sender->GetParentGroup() != NULL) {
    return get_mat_name(sender->GetParentGroup()->GetActualMaterial(glm),
      materials);
  }
  size_t lmi = materials.IndexOf(&glm);
  return (lmi == InvalidIndex ? materials.Add(glm, olxstr("mat") <<
    (materials.Count()+1)) : materials.GetValue(lmi));
}
