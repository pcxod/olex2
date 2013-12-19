/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "ortdraw.h"
#include "xatom.h"
#include "xline.h"
#include "dring.h"
#include "styles.h"
#include "gllabel.h"
#include "dunitcell.h"
#include "dbasis.h"
#include "xgrid.h"
#include "conrec.h"
#include "sfutil.h"
#include "unitcell.h"
#include "maputil.h"
#include "arrays.h"

ort_atom::ort_atom(const OrtDraw& parent, const TXAtom& a) :
  a_ort_object(parent),
  atom(a),
  p_elpm(NULL), p_ielpm(NULL), elpm(NULL),
draw_style(0)
{
  const TSAtom& sa = atom;
  if( sa.GetEllipsoid() != NULL )  {
    mat3f& _elpm = *(new mat3f(sa.GetEllipsoid()->GetMatrix()));
    _elpm[0] *= sa.GetEllipsoid()->GetSX();
    _elpm[1] *= sa.GetEllipsoid()->GetSY();
    _elpm[2] *= sa.GetEllipsoid()->GetSZ();
    _elpm *= (float)atom.GetDrawScale();
    elpm = new mat3f(_elpm);
    p_elpm = &(_elpm *= parent.ProjMatr);
    mat3f& ielpm = *(new mat3f((sa.GetEllipsoid()->GetMatrix()*parent.basis.GetMatrix()).Inverse()));
    p_ielpm = &(ielpm *= _elpm);
  }
  draw_rad = (float)atom.GetDrawScale()*parent.DrawScale;
  crd = parent.ProjectPoint(sa.crd());
  sphere_color = atom.GetType().def_color;
  const TGraphicsStyle& style = atom.GetPrimitives().GetStyle();
  size_t lmi = style.IndexOfMaterial("Sphere");
  if( lmi != InvalidIndex )  {
    TGlMaterial& glm = style.GetPrimitiveStyle(lmi).GetProperties();
    sphere_color = glm.AmbientF.GetRGB();
  }
  rim_color = atom.GetType().def_color;
  lmi = style.IndexOfMaterial("Rims");
  if( lmi != InvalidIndex )  {
    TGlMaterial& glm = style.GetPrimitiveStyle(lmi).GetProperties();
    rim_color = glm.AmbientF.GetRGB();
  }
  if( atom.IsVisible() )
    mask = atom.GetPrimitiveMask();
  else
    mask = 0;
}

void ort_atom::render_elp(PSWriter& pw) const {
  const mat3f& ielpm = *p_ielpm;
  if( parent.GetAtomOutlineSize() > 0 )  {
    pw.color(parent.GetAtomOutlineColor());
    pw.drawEllipse(NullVec, ielpm*(parent.GetAtomOutlineSize()+1.0f), &PSWriter::fill);
  }
  if( (draw_style&ortep_color_fill) != 0 )  {
    pw.newPath();
    pw.ellipse(NullVec, ielpm);
    pw.gsave();
    pw.color(sphere_color);
    pw.fill();
    pw.grestore();
    pw.color(0);
    pw.stroke();
  }
  else if( (draw_style&ortep_color_lines) != 0 )  {
    pw.color(sphere_color == 0xffffff ? 0 : sphere_color);
    pw.drawEllipse(NullVec, ielpm);
  }
  else  {
    pw.color(0);
    pw.drawEllipse(NullVec, ielpm);
  }
  if( (draw_style&ortep_atom_rims) != 0 )
    render_rims(pw);
}

void ort_atom::render_sph(PSWriter& pw) const {
  if( parent.GetAtomOutlineSize() > 0 )  {
    pw.color(parent.GetAtomOutlineColor());
    pw.drawCircle(NullVec, draw_rad*(parent.GetAtomOutlineSize()+1.0f), &PSWriter::fill);
  }
  if( (draw_style&ortep_color_fill) != 0 )  {
    pw.newPath();
    pw.circle(NullVec, draw_rad);
    pw.gsave();
    pw.color(sphere_color);
    pw.fill();
    pw.grestore();
    pw.color(0);
    pw.stroke();
  }
  else if( (draw_style&ortep_color_lines) != 0 )  {
    pw.color(sphere_color == 0xffffff ? 0 : sphere_color);
    pw.drawCircle(NullVec, draw_rad);
  }
  else  {
    pw.color(0);
    pw.drawCircle(NullVec, draw_rad);
  }
}

void ort_atom::render_standalone(PSWriter& pw) const {
  if( (draw_style&ortep_color_lines) != 0 )
    pw.color(sphere_color);
  else
    pw.color(0);
  pw.drawLine(vec3f(-draw_rad, 0, 0), vec3f(draw_rad, 0, 0));
  pw.drawLine(vec3f(0, -draw_rad, 0), vec3f(0, draw_rad, 0));
}

bool ort_atom::IsSpherical() const {
  bool res = !(p_elpm != NULL &&
    (atom.DrawStyle() == adsEllipsoid ||
    atom.DrawStyle() == adsOrtep));
  if( !res && atom.GetEllipsoid()->IsNPD() )
    return true;
  return res;
}

void ort_atom::render(PSWriter& pw) const {
  if( mask == 0 )  return;
  pw.translate(crd);
  pw.lineWidth(parent.ElpLineWidth);
  if( mask == 16 && atom.DrawStyle() == adsStandalone )  {
    if( atom.IsStandalone() )
      render_standalone(pw);
  }
  else if( !IsSpherical() )
    render_elp(pw);
  else
    render_sph(pw);
  pw.translate(-crd);
}

void ort_atom::update_size(evecf &size) const {
  if( mask == 0 )  return;
  float mw, mh;
  if( mask == 16 && atom.DrawStyle() == adsStandalone )  {
    mw = mh = draw_rad*(parent.GetAtomOutlineSize()+1.0f);
  }
  else if( !IsSpherical() )  {
    mw = sqrt(olx_sqr((*p_ielpm)[0][0])+olx_sqr((*p_ielpm)[0][1]))
      *(parent.GetAtomOutlineSize()+1.0f);
    mh = sqrt(olx_sqr((*p_ielpm)[1][0])+olx_sqr((*p_ielpm)[1][1]))
      *(parent.GetAtomOutlineSize()+1.0f);
  }
  else  {
    mw = mh = draw_rad*(parent.GetAtomOutlineSize()+1.0f);
  }
  if( size[0] > crd[0]-mw )  size[0] = crd[0]-mw;
  if( size[1] > crd[1]-mh )  size[1] = crd[1]-mh;
  if( size[2] < crd[0]+mw )  size[2] = crd[0]+mw;
  if( size[3] < crd[1]+mh )  size[3] = crd[1]+mh;
}


void ort_atom::render_rims(PSWriter& pw) const {
  if( (draw_style&(ortep_color_lines|ortep_color_fill)) != 0 )
    pw.color(rim_color);
  pw.lineWidth(parent.QuadLineWidth);
  const mat3f& elpm = *p_elpm;
  const mat3f& ielpm = *p_ielpm;
  mat3f pelpm(elpm[0][2] < 0 ? -elpm[0] : elpm[0],
    elpm[1][2] < 0 ? -elpm[1] : elpm[1],
    elpm[2][2] < 0 ? -elpm[2] : elpm[2]);
  vec3f norm_vec = (parent.ElpCrd[0]*ielpm).XProdVec(parent.ElpCrd[1]*ielpm);
  if( norm_vec[2] < 0 )  norm_vec *= -1;

  parent.RenderRims(pw, pelpm, norm_vec);
  if( (draw_style&ortep_atom_quads) != 0 )  {
    pw.lineWidth(parent.PieLineWidth);
    parent.RenderQuads(pw, pelpm);
  }
}

template <class draw_t>
ort_bond<draw_t>::ort_bond(const OrtDraw& parent,
  const draw_t &object,
  const ort_atom& a1, const ort_atom& a2)
  : a_ort_object(parent),
  object(object),
  atom_a(a1.get_z() < a2.get_z() ? a1 : a2),
  atom_b(a1.get_z() < a2.get_z() ? a2 : a1),
  swapped(a1.get_z() > a2.get_z())
{
  draw_style = 0;
}


template <class draw_t>
uint32_t ort_bond<draw_t>::get_color(int primitive, uint32_t def) const {
  TGlPrimitive *glp = object.GetPrimitives().FindPrimitiveByName(
    TXBond::StaticPrimitives()[primitive]);
  if( (draw_style&ortep_color_bond) == 0 )  {
    return glp == NULL ? 0 :
    (glp->GetProperties().AmbientF.GetRGB() == def ? 0
    : glp->GetProperties().AmbientF.GetRGB());
  }
  return glp == NULL ? def : glp->GetProperties().AmbientF.GetRGB();
}
template <class draw_t>
void ort_bond<draw_t>::render(PSWriter& pw) const {
  uint32_t mask = object.GetPrimitiveMask();
  if( mask == 0 )  return;
  pw.lineWidth(1.0f);
  pw.translate(atom_a.crd);
  if( parent.GetBondOutlineSize() > 0 )  {
    pw.color(parent.GetBondOutlineColor());
    _render(pw, (parent.GetBondOutlineSize()+1.0f), mask);
  }
  if( (draw_style&ortep_color_bond) == 0 )
    pw.color(0);
  else if( (mask&((1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<10))) == 0 )  {
    uint32_t def = (atom_a.atom.GetType() > atom_b.atom.GetType() ?
      atom_a.sphere_color : atom_b.sphere_color);
    int pi[] = {0,1,2,3,8,11,12,13};
    for( size_t i=0; i < sizeof(pi)/sizeof(pi[0]); i++ )  {
      if( (mask&(1<<pi[i])) != 0 )  {
        pw.color(get_color(pi[i], def));
        break;
      }
    }
  }
  _render(pw, 1, mask);
  pw.translate(-atom_a.crd);
}

template <class draw_t>
void ort_bond<draw_t>::_render(PSWriter& pw, float scalex, uint32_t mask) const {
  if (mask == (1<<12) || mask == (1<<13) || (mask&((1<<6)|(1<<7))) != 0 ) {
    pw.lineWidth(scalex);
    if ((mask&(1<<6)) !=0) {
      pw.color(get_color(6, atom_b.sphere_color));
      pw.drawLine(NullVec, (atom_b.crd-atom_a.crd)/2);
    }
    if ((mask&(1<<7)) !=0) {
      pw.color(get_color(7, atom_a.sphere_color));
      pw.drawLine((atom_b.crd-atom_a.crd)/2, (atom_b.crd-atom_a.crd));
    }
    if (mask == (1<<13))
      pw.custom("[8 8] 0 setdash");
    if ((mask&((1<<12)|(1<<13))) != 0)
      pw.drawLine(NullVec, atom_b.crd-atom_a.crd);
    if (mask == (1<<13))
      pw.custom("[] 0 setdash");
    return;
  }
  vec3f dir_vec = atom_b.crd-atom_a.crd;
  const float b_len = dir_vec.Length();
  if( b_len < 1e-3 )  return;
  const float brad = parent.GetBondRad(*this, mask)*(float)object.GetRadius();
  dir_vec.Normalise();
  const float pers_scale = 1.0f-olx_sqr(dir_vec[2]);
  mat3f rot_mat;
  const vec3f touch_point = (atom_b.atom.crd() - atom_a.atom.crd()).Normalise();
  if( olx_abs(1.0f-olx_abs(touch_point[2])) < 1e-3 )  // degenerated case...
    rot_mat.I();
  else
    olx_create_rotation_matrix_(rot_mat,
      vec3f(-touch_point[1], touch_point[0], 0).Normalise(), touch_point[2]);
  const mat3f proj_mat = rot_mat*parent.ProjMatr;
  const float _brad = brad*(1+pers_scale)*scalex;
  if( !atom_a.IsSpherical() && atom_a.IsSolid() )  {
    mat3f elm = *atom_a.elpm;
    mat3f ielm = mat3f(elm).Normalise().Transpose();
    /* etm projects to ellipsoid and un-projects back to the cartesian frame
    with the ellipsoid scale accumulated - this is the quadractic form of the
    ellipsoid (QLQt)
    */
    mat3f erm, etm = ielm*elm;// ietm=etm.Inverse();
    // below is same as: vec3f pv = (touch_point*ietm).Normalise();
    const vec3f pv = mat3f::CramerSolve(etm, touch_point).Normalise();
    // this is there the touch_point ends up after projecting onto the ellipsoid
    const vec3f tp = etm*touch_point;
    // create rotation to compensate for the elliptical distrortion
    const float erm_ca = tp.CAngle(touch_point);
    if( erm_ca != 1 )  {
      olx_create_rotation_matrix_(
        erm, tp.XProdVec(touch_point).Normalise(), tp.CAngle(touch_point));
    }
    else
      erm.I();
    rot_mat *= erm;
    // this is the corrected location of the touch point...
    const vec3f off = (etm*pv)*parent.ProjMatr;
    for( uint16_t j=0; j < parent.BondDiv; j++ )  {
      parent.BondProjF[j] = (etm*(parent.BondCrd[j]*rot_mat)*parent.ProjMatr).
        NormaliseTo(_brad) + off;
      parent.BondProjT[j] = (parent.BondCrd[j]*proj_mat).
        NormaliseTo(brad*2*scalex) + dir_vec*b_len;
    }
  }
  else  {
    const float off_len = !atom_a.IsSolid() ? 0 :
      (atom_a.draw_rad > _brad ?
        sqrt(olx_sqr(atom_a.draw_rad)-olx_sqr(_brad)) : atom_a.draw_rad);
    for( uint16_t j=0; j < parent.BondDiv; j++ )  {
      parent.BondProjT[j] = parent.BondProjF[j] =
        (parent.BondCrd[j]*proj_mat).NormaliseTo(_brad);
      parent.BondProjT[j].NormaliseTo(brad*2*scalex) += dir_vec*b_len;
      parent.BondProjF[j] += dir_vec*off_len;
    }
  }
  if( scalex < 1.1 &&
    (mask&((1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 9) | (1 << 10)
    | (1 << 14) | (1 << 15) | (1 << 16) | (1 << 17))) != 0)
  {
    for( uint16_t i=0; i < parent.BondDiv; i++ )  {
      parent.BondProjM[i][0] = (parent.BondProjT[i][0]+parent.BondProjF[i][0])/2;
      parent.BondProjM[i][1] = (parent.BondProjT[i][1]+parent.BondProjF[i][1])/2;
      parent.BondProjM[i][2] = (parent.BondProjT[i][2]+parent.BondProjF[i][2])/2;
    }
    if (!swapped) { // normal rendering
      if ((mask&((1 << 4) | (1 << 6) | (1 << 9) | (1 << 14)
          | (1 << 16))) != 0)
      {
        if ((mask&(1<<4)) != 0)
          pw.color(get_color(4, atom_a.sphere_color));
        else if ((mask&(1<<6)) != 0)
          pw.color(get_color(6, atom_a.sphere_color));
        else if ((mask&(1<<9)) != 0)
          pw.color(get_color(9, atom_a.sphere_color));
        else if ((mask&(1 << 14)) != 0)
          pw.color(get_color(14, atom_a.sphere_color));
        else if ((mask&(1 << 16)) != 0)
          pw.color(get_color(16, atom_a.sphere_color));
        if ((mask&((1 << 14) | (1 << 16))) != 0) {
          pw.drawOuterQuads(parent.BondProjF, parent.BondProjM, &PSWriter::fill,
            (mask&(1 << 16)) != 0, parent.GetMultipleBondsWidth(),
            parent.GetStippleCount(*this, true));
        }
        else {
          pw.drawQuads(parent.BondProjF, parent.BondProjM, &PSWriter::fill,
            parent.GetStippleCount(*this, true));
        }
      }
      if ((mask&((1 << 5) | (1 << 7) | (1 << 10) | (1 << 15)
          | (1 << 17))) != 0)
      {
        if ((mask&(1<<5)) != 0)
          pw.color(get_color(5, atom_b.sphere_color));
        else if ((mask&(1<<7)) != 0)
          pw.color(get_color(7, atom_b.sphere_color));
        else if ((mask&(1<<10)) != 0)
          pw.color(get_color(10, atom_b.sphere_color));
        else if ((mask&(1 << 15)) != 0)
          pw.color(get_color(15, atom_b.sphere_color));
        else if ((mask&(1 << 17)) != 0)
          pw.color(get_color(17, atom_b.sphere_color));
        if ((mask&((1 << 15) | (1 << 17))) != 0) {
          pw.drawOuterQuads(parent.BondProjM, parent.BondProjT, &PSWriter::fill,
            (mask&(1 << 17)) != 0, parent.GetMultipleBondsWidth(),
            parent.GetStippleCount(*this, true));
        }
        else {
          pw.drawQuads(parent.BondProjM, parent.BondProjT, &PSWriter::fill,
            parent.GetStippleCount(*this, true));
        }
      }
    }
    else  {  // reverse rendering
      if ((mask&((1 << 5) | (1 << 7) | (1 << 10) | (1 << 15)
          | (1 << 17))) != 0)
      {
        if( (mask&(1<<5)) != 0 )
          pw.color(get_color(5, atom_a.sphere_color));
        else if ((mask&(1<<7)) != 0)
          pw.color(get_color(7, atom_a.sphere_color));
        else if ((mask&(1<<10)) != 0)
          pw.color(get_color(10, atom_a.sphere_color));
        else if ((mask&(1 << 15)) != 0)
          pw.color(get_color(15, atom_a.sphere_color));
        else if ((mask&(1 << 17)) != 0)
          pw.color(get_color(17, atom_a.sphere_color));
        if ((mask&((1 << 15) | (1 << 17))) != 0) {
          pw.drawOuterQuads(parent.BondProjF, parent.BondProjM, &PSWriter::fill,
            (mask&(1 << 17)) != 0, parent.GetMultipleBondsWidth(),
            parent.GetStippleCount(*this, true));
        }
        else {
          pw.drawQuads(parent.BondProjF, parent.BondProjM, &PSWriter::fill,
            parent.GetStippleCount(*this, true));
        }
      }
      if ((mask&((1 << 4) | (1 << 6) | (1 << 9) | (1 << 14)
          | (1 << 16))) != 0)
      {
        if ((mask&(1<<4)) != 0)
          pw.color(get_color(4, atom_b.sphere_color));
        else if ((mask&(1<<6)) != 0)
          pw.color(get_color(6, atom_b.sphere_color));
        else if ((mask&(1<<9)) != 0)
          pw.color(get_color(9, atom_b.sphere_color));
        else if ((mask&(1 << 14)) != 0)
          pw.color(get_color(14, atom_b.sphere_color));
        else if ((mask&(1 << 16)) != 0)
          pw.color(get_color(16, atom_b.sphere_color));
        if ((mask&((1 << 14) | (1 << 16))) != 0) {
          pw.drawOuterQuads(parent.BondProjM, parent.BondProjT, &PSWriter::fill,
            (mask&(1 << 16)) != 0, parent.GetMultipleBondsWidth(),
            parent.GetStippleCount(*this, true));
        }
        else {
          pw.drawQuads(parent.BondProjM, parent.BondProjT, &PSWriter::fill,
            parent.GetStippleCount(*this, true));
        }
      }
    }
  }
  else {
    if ((mask& ((1 << 14) | (1 << 15))) != 0) {
      pw.drawOuterQuads(parent.BondProjF, parent.BondProjT, &PSWriter::fill,
        false, parent.GetMultipleBondsWidth(),
        parent.GetStippleCount(*this, false));
    }
    else if ((mask& ((1 << 16) | (1 << 17))) != 0) {
      pw.drawOuterQuads(parent.BondProjF, parent.BondProjT, &PSWriter::fill,
        true, parent.GetMultipleBondsWidth(),
        parent.GetStippleCount(*this, false));
    }
    else {
      pw.drawQuads(parent.BondProjF, parent.BondProjT, &PSWriter::fill,
        parent.GetStippleCount(*this, false));
    }
  }
}
//.............................................................................
ort_bond_line::ort_bond_line(const OrtDraw& parent, const TXLine& line,
  const vec3f& from, const vec3f& to)
  : a_ort_object(parent),
  line(line),
  from(from), to(to),
  p_from(parent.ProjectPoint(from)), p_to(parent.ProjectPoint(to))
{
  draw_style = 0;
  a1 = a2 = NULL;
  float min_d1, min_d2;
  for (size_t i=0; i < parent.atoms.Count(); i++) {
    float d1 = (parent.atoms[i]->crd-p_from).QLength();
    float dr = parent.atoms[i]->draw_rad;
    if (parent.atoms[i]->elpm != NULL) {
      dr = olx_max((*parent.atoms[i]->elpm)[0].QLength(),
        (*parent.atoms[i]->elpm)[1].QLength());
      dr = olx_max(dr, (*parent.atoms[i]->elpm)[2].QLength());
    }
    else
      dr *= dr;
    if (d1 < dr) {
      if (a1 == NULL || (a1 != NULL && d1 < min_d1)) {
        a1 = parent.atoms[i];
        min_d1 = d1;
      }
    }
    float d2 = (parent.atoms[i]->crd-p_to).QLength();
    if (d2 < dr) {
      if (a2 == NULL || (a2 != NULL && d2 < min_d2)) {
        a2 = parent.atoms[i];
        min_d2 = d2;
      }
    }
  }
}

uint32_t ort_bond_line::get_color(int primitive, uint32_t def) const {
  TGlPrimitive *glp = line.GetPrimitives().FindPrimitiveByName(
    TXBond::StaticPrimitives()[primitive]);
  if( (draw_style&ortep_color_bond) == 0 )  {
    return glp == NULL ? 0 :
    (glp->GetProperties().AmbientF.GetRGB() == def ? 0
    : glp->GetProperties().AmbientF.GetRGB());
  }
  return glp == NULL ? def : glp->GetProperties().AmbientF.GetRGB();
}
void ort_bond_line::render(PSWriter& pw) const {
  if (a1 != NULL && a2 != NULL) {
    ort_bond<TXLine>(parent, line, *a1, *a2).render(pw);
    return;
  }
  uint32_t mask = line.GetPrimitiveMask();
  if( mask == 0 )  return;
  pw.lineWidth(1.0f);
  vec3f shift = parent.ProjectPoint(from);
  pw.translate(shift);
  if( parent.GetBondOutlineSize() > 0 )  {
    pw.color(parent.GetBondOutlineColor());
    _render(pw, (parent.GetBondOutlineSize()+1.0f), mask);
  }
  if( (draw_style&ortep_color_bond) == 0 )
    pw.color(0);
  else if( (mask&((1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<10))) == 0 )  {
    int pi[] = {0,1,2,3,8,11,12,13};
    for( size_t i=0; i < sizeof(pi)/sizeof(pi[0]); i++ )  {
      if( (mask&(1<<pi[i])) != 0 )  {
        pw.color(get_color(pi[i]));
        break;
      }
    }
  }
  _render(pw, 1, mask);
  pw.translate(-shift);
}
void ort_bond_line::_render(PSWriter& pw, float scalex, uint32_t mask) const {
  if( mask == (1<<12) || mask == (1<<13) || (mask&((1<<6)|(1<<7))) != 0 )  {
    pw.lineWidth(scalex);
    if( (mask&(1<<6)) !=0 )  {
      pw.color(get_color(6));
      pw.drawLine(NullVec, (p_to-p_from)/2);
    }
    if( (mask&(1<<7)) !=0 )  {
      pw.color(get_color(7));
      pw.drawLine((to-from)/2, (p_to-p_from));
    }
    if( mask == (1<<13) )
      pw.custom("[8 8] 0 setdash");
    if( (mask&((1<<12)|(1<<13))) != 0 )
      pw.drawLine(NullVec, p_to-p_from);
    if( mask == (1<<13) )
      pw.custom("[] 0 setdash");
    return;
  }
  vec3f dir_vec = p_to-p_from;
  const float b_len = dir_vec.Length();
  if( b_len < 1e-3 )  return;
  const float brad = parent.GetLineRad(*this, mask)*(float)line.GetRadius();
  const float pers_scale = 1.0f-olx_sqr(dir_vec[2]/b_len);
  mat3f rot_mat;
  const vec3f touch_point = vec3f(to-from).Normalise();
  if( olx_abs(1.0f-olx_abs(touch_point[2])) < 1e-3f )  // degenerated case...
    rot_mat.I();
  else
    olx_create_rotation_matrix_(rot_mat,
      vec3f(-touch_point[1], touch_point[0], 0).Normalise(), touch_point[2]);
  const mat3f proj_mat = rot_mat*parent.ProjMatr;
  const float _brad = brad*(1+pers_scale)*scalex;
  float off1=0, off2=0;
  if (a1 != NULL) {
    off1 = !a1->IsSolid() ? 0 :
      (a1->draw_rad > _brad ?
      sqrt(olx_sqr(a1->draw_rad)-olx_sqr(_brad)) : a1->draw_rad);
  }
  if (a2 != NULL) {
    off2 = !a2->IsSolid() ? 0 :
      (a2->draw_rad > _brad ?
      sqrt(olx_sqr(a2->draw_rad)-olx_sqr(_brad)) : a2->draw_rad);
  }
  if (a1 != NULL && !a1->IsSpherical() && a1->IsSolid()) {
    mat3f elm = *a1->elpm;
    mat3f ielm = mat3f(elm).Normalise().Inverse();
    mat3f erm, etm = ielm*elm;
    vec3f pv = (touch_point*etm.Inverse()).Normalise();
    const vec3f tp = etm*touch_point;
    const float erm_ca = tp.CAngle(touch_point);
    if (erm_ca != 1) {
      olx_create_rotation_matrix_(
        erm, tp.XProdVec(touch_point).Normalise(), tp.CAngle(touch_point));
    }
    else
      erm.I();
    rot_mat *= erm;
    const vec3f off = (etm*pv)*parent.ProjMatr;
    for( uint16_t j=0; j < parent.BondDiv; j++ )  {
      parent.BondProjF[j] = (etm*(parent.BondCrd[j]*rot_mat)*parent.ProjMatr).
        NormaliseTo(_brad) + off;
      parent.BondProjT[j] = (parent.BondCrd[j]*proj_mat).
        NormaliseTo(brad*2*scalex) + dir_vec;
    }
  }
  else {
    for( uint16_t j=0; j < parent.BondDiv; j++ )  {
      parent.BondProjT[j] = parent.BondProjF[j] =
        (parent.BondCrd[j]*proj_mat).NormaliseTo(_brad);
      parent.BondProjT[j].NormaliseTo(brad*2*scalex) += dir_vec*(1-off2/b_len);
      parent.BondProjF[j] += dir_vec*off1/b_len;
    }
  }
  if( scalex < 1.1 &&
    (mask&((1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<10))) != 0 )
  {
    for( uint16_t i=0; i < parent.BondDiv; i++ )  {
      parent.BondProjM[i][0] = (parent.BondProjT[i][0]+parent.BondProjF[i][0])/2;
      parent.BondProjM[i][1] = (parent.BondProjT[i][1]+parent.BondProjF[i][1])/2;
      parent.BondProjM[i][2] = (parent.BondProjT[i][2]+parent.BondProjF[i][2])/2;
    }
    if( (mask&((1<<4)|(1<<6)|(1<<9))) != 0 )  {
      if( (mask&(1<<4)) != 0 )
        pw.color(get_color(4));
      else if( (mask&(1<<6)) != 0 )
        pw.color(get_color(6));
      else if( (mask&(1<<9)) != 0 )
        pw.color(get_color(9));
        pw.drawQuads(parent.BondProjF, parent.BondProjM, &PSWriter::fill,
          parent.GetStippleCount(*this, true));
    }
    if( (mask&((1<<5)|(1<<7)|(1<<10))) != 0 )  {
      if( (mask&(1<<5)) != 0 )
        pw.color(get_color(5));
      else if( (mask&(1<<7)) != 0 )
        pw.color(get_color(7));
      else if( (mask&(1<<10)) != 0 )
        pw.color(get_color(10));
        pw.drawQuads(parent.BondProjM, parent.BondProjT, &PSWriter::fill,
          parent.GetStippleCount(*this, true));
    }
  }
  else  {
    pw.drawQuads(parent.BondProjF, parent.BondProjT, &PSWriter::fill,
      parent.GetStippleCount(*this, false));
  }
}
//.............................................................................
void ort_line::render(PSWriter& pw) const {
  pw.lineWidth(0.1);
  pw.color(color);
  pw.drawLine(from, to);
}

void ort_poly::render(PSWriter& pw) const {
  if( fill && points.Count() > 2 )  {
    pw.color(color);
    pw.drawLines(points, InvalidSize, true, &PSWriter::fill);
  }
  else  {
    if( color != 0xffffffff )  {
      pw.lineWidth(line_width*1.2);
      pw.color(0xffffffff);
      pw.drawLines(points, InvalidSize, true);
    }
    pw.lineWidth(line_width);
    pw.color(color);
    pw.drawLines(points, InvalidSize, true);
  }
}

void ort_circle::render(PSWriter& pw) const {
  pw.color(color);
  if( basis != NULL )  {
    if( fill )  {
      pw.drawEllipse(center, *basis*r, &PSWriter::fill);
      pw.color(0x0);
      pw.drawEllipse(center, *basis*r, &PSWriter::stroke);
    }
    else
      pw.drawEllipse(center, *basis*r, &PSWriter::stroke);
  }
  else  {
    if( fill )  {
      pw.drawCircle(center, r, &PSWriter::fill);
      pw.color(0x0);
      pw.drawCircle(center, r, &PSWriter::stroke);
    }
    else
      pw.drawCircle(center, r, &PSWriter::stroke);
  }
}

void ort_circle::update_size(evecf &sz) const {
  if( basis == NULL )  {
    a_ort_object::update_min_max(sz, vec3f(center[0]+r, center[1]+r, 0));
    a_ort_object::update_min_max(sz, vec3f(center[0]-r, center[1]-r, 0));
  }
  else  {
    mat3f b = *basis*r;
    float hmx = sqrt(olx_sqr(b[0][0])+olx_sqr(b[0][1]))/2;
    float hmy = sqrt(olx_sqr(b[1][0])+olx_sqr(b[1][1]))/2;
    a_ort_object::update_min_max(sz, vec3f(center[0]+hmx, center[1]+hmy, 0));
    a_ort_object::update_min_max(sz, vec3f(center[0]-hmx, center[1]-hmy, 0));
  }
}


void ort_cone::render(PSWriter& pw) const {
  vec3f n = (top-bottom).Normalise();
  mat3f rm, basis = TEBasis::CalcBasis<vec3f,mat3f>(n);
  olx_create_rotation_matrix_<float, mat3f, vec3f>(rm, n,
    (float)cos(M_PI*2/divs));
  TArrayList<vec3f> t_crd(divs), b_crd(divs);
  vec3f ps = basis[1];
  for( uint16_t i=0; i < divs; i++ )  {
    t_crd[i] = b_crd[i] = ps;
    t_crd[i].NormaliseTo(top_r) += top;
    b_crd[i].NormaliseTo(bottom_r) += bottom;
    ps *= rm;
  }
  pw.color(color);
  pw.drawQuads(b_crd, t_crd, &PSWriter::fill);
}

void ort_cone::update_size(evecf &sz) const {
  try {
    vec3f _n = (top-bottom),
      tr = vec3f(_n[0], -_n[1], 0).NormaliseTo(top_r),
      tb = vec3f(_n[0], -_n[1], 0).NormaliseTo(bottom_r);
    a_ort_object::update_min_max(sz, top+tr);
    a_ort_object::update_min_max(sz, top-tr);
    a_ort_object::update_min_max(sz, bottom+tb);
    a_ort_object::update_min_max(sz, bottom-tb);
  }
  catch(const TDivException &) {
    return;
  }
}


size_t OrtDraw::GetStippleCount(const ort_bond_line &l, bool half) const {
  return stipples_for_mask(l.line.GetPrimitiveMask(), half);
}

void OrtDraw::RenderRims(PSWriter& pw, const mat3f& pelpm,
  const vec3f& norm_vec) const
{
  for( uint16_t j=0; j < ElpDiv; j++ )
    Arc[j] = ElpCrd[j]*pelpm;
  size_t pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
  pw.drawLines_vp(FilteredArc, pts_cnt, false);

  for( uint16_t j=0; j < ElpDiv; j++ )
    Arc[j] = vec3f(ElpCrd[j][1], 0, ElpCrd[j][0])*pelpm;
  pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
  pw.drawLines_vp(FilteredArc, pts_cnt, false);

  for( uint16_t j=0; j < ElpDiv; j++ )
    Arc[j] = vec3f(0, ElpCrd[j][0], ElpCrd[j][1])*pelpm;
  pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
  pw.drawLines_vp(FilteredArc, pts_cnt, false);
}

void OrtDraw::RenderQuads(PSWriter& pw, const mat3f& pelpm) const {
  pw.drawLine(NullVec, pelpm[0]);
  pw.drawLine(NullVec, pelpm[1]);
  pw.drawLine(NullVec, pelpm[2]);
  for( uint16_t j=0; j < PieDiv; j++ )
    pw.drawLine(PieCrd[j]*pelpm, pelpm[0]*((float)(PieDiv-j)/PieDiv));
  for( uint16_t j=0; j < PieDiv; j++ ) {
    pw.drawLine(vec3f(0, PieCrd[j][0], PieCrd[j][1])*pelpm,
      pelpm[1]*((float)(PieDiv-j)/PieDiv));
  }
  for( uint16_t j=0; j < PieDiv; j++ ) {
    pw.drawLine(vec3f(PieCrd[j][1], 0, PieCrd[j][0])*pelpm,
      pelpm[2]*((float)(PieDiv-j)/PieDiv));
  }
}

size_t OrtDraw::PrepareArc(const TArrayList<vec3f>& in,
  TPtrList<const vec3f>& out, const vec3f& normal) const
{
  size_t start = InvalidIndex, cnt = 0;
  for( size_t i=0; i < in.Count(); i++ )  {
    const size_t next_i = ((i+1) >= in.Count() ? i-in.Count() : i) + 1;
    if( in[i].DotProd(normal) < 0 && in[next_i].DotProd(normal) >= 0 )  {
      start = next_i;
      break;
    }
  }
  if( start == InvalidIndex )  return 0;
  for( size_t i=start; i < start+in.Count(); i++ )  {
    const size_t ind = (i >= in.Count() ? i-in.Count() : i);
    if( cnt+1 >= in.Count() )  break;
    if( in[ind].DotProd(normal) >= 0  )
      out[cnt++] = &in[ind];
    else
      break;
  }
  return cnt;
}

void OrtDraw::Init(PSWriter& pw)  {
  ElpCrd.SetCount(ElpDiv);
  Arc.SetCount(ElpDiv);
  PieCrd.SetCount(PieDiv);
  FilteredArc.SetCount(ElpDiv);
  BondCrd.SetCount(BondDiv);
  BondProjF.SetCount(BondDiv);
  BondProjM.SetCount(BondDiv);
  BondProjT.SetCount(BondDiv);
  float sin_a, cos_a;
  olx_sincos((float)(2*M_PI/ElpDiv), &sin_a, &cos_a);
  vec3f ps(cos_a, -sin_a, 0);
  for( uint16_t i=0; i < ElpDiv; i++ )  {
    ElpCrd[i] = ps;
    const float x = ps[0];
    ps[0] = (float)(cos_a*x + sin_a*ps[1]);
    ps[1] = (float)(cos_a*ps[1] - sin_a*x);
  }
  olx_sincos((float)(2*M_PI/BondDiv), &sin_a, &cos_a);
  ps = vec3f(cos_a/2, -sin_a/2, 0);
  for( uint16_t i=0; i < BondDiv; i++ )  {
    BondCrd[i] = ps;
    const float x = ps[0];
    ps[0] = (float)(cos_a*x + sin_a*ps[1]);
    ps[1] = (float)(cos_a*ps[1] - sin_a*x);
  }
  ps = vec3f(1, 0, 0);
  for( uint16_t i=0; i < PieDiv; i++ )  {
    PieCrd[i] = ps;
    ps[0] = (float)(PieDiv-i-1)/PieDiv;
    ps[1] = (float)sqrt(1.0-ps[0]*ps[0]);
  }
  GLfloat vp[4];
  olx_gl::get(GL_VIEWPORT, vp);
  TGXApp& app = TGXApp::GetInstance();
  const TEBasis& basis = app.GetRender().GetBasis();
  LinearScale = olx_min((float)pw.GetWidth()/vp[2],
    (float)pw.GetHeight()/vp[3]);
  
  {
    olxcstr fnt("/Verdana findfont ");
    AGlScene& sc = app.GetRender().GetScene();
    fnt << olx_round(
      sc.GetFont(sc.FindFontIndexForType<TXAtom>(), true).
        GetPointSize()/sqrt(LinearScale))
      << " scalefont setfont";
    pw.custom(fnt.c_str());
  }

  YOffset = ((float)pw.GetHeight()-LinearScale*vp[3])/2;
  pw.translate(0.0f, YOffset);
  pw.scale(LinearScale, LinearScale);
  DrawScale = (float)(app.GetRender().GetBasis().GetZoom()
    /(app.GetRender().GetScale()));
  BondRad = 0.03f*DrawScale;
  SceneOrigin = basis.GetCenter();
  DrawOrigin = vec3f(vp[2]/2, vp[3]/2, 0);
  ProjMatr = basis.GetMatrix()*DrawScale;
  UnProjMatr = ProjMatr.Inverse();
}
/*
Grid interpolation stuff...
http://xtal.sourceforge.net/man/slant-desc.html
the cubic interpolation does a good job on coarse grids (compared with direct
calculations)
for linear:
    float _p = p[0]-fp[0];
    float _q = p[1]-fp[1];
    float _r = p[2]-fp[2];
    float vx[2] = {1-_p, _p};
    float vy[2] = {1-_q, _q};
    float vz[2] = {1-_r, _r};
    for( int dx=0; dx <= 1; dx++ )  {
      float _vx = vx[dx];
      for( int dy=0; dy <= 1; dy++ )  {
        float _vy = vy[dy];
        for( int dz=0; dz <= 1; dz++ )  {
          float _vz = vz[dz];
          vec3i ijk(fp[0]+dx, fp[1]+dy, fp[2]+dz);
          for( int m=0; m < 3; m++ )  {
            while( ijk[m] < 0 )
              ijk[m] += dim[m];
            while( ijk[m] >= dim[m] )
              ijk[m] -= dim[m];
          }
          val += grid.GetValue(ijk)*_vx*_vy*_vz;
        }
      }
    }
Direct calculation:
    TRefList refs;
    TArrayList<compd> F;
    SFUtil::GetSF(refs, F, SFUtil::mapTypeCalc, SFUtil::sfOriginOlex2,
      SFUtil::scaleSimple);
    TSpaceGroup* sg = NULL;
    try  { sg = &app.XFile().GetLastLoaderSG();  }
    catch(...)  {  return;  }
    TArrayList<SFUtil::StructureFactor> P1SF;
    TArrayList<vec3i> hkl(refs.Count());
    for( size_t i=0; i < refs.Count(); i++ )
      hkl[i] = refs[i].GetHkl();
    SFUtil::ExpandToP1(hkl, F, *sg, P1SF);
    float cell_vol = (float)app.XFile().GetUnitCell().CalcVolume();
    for( int i=0; i < MaxDim; i++ )  {
      for( int j=0; j < MaxDim; j++ )  {  // (i,j,Depth)        
        vec3f p((float)(i-hh)/Size, (float)(j-hh)/Size,  Depth);
        p = bm*p;
        p -= center;
        p *= c2c;
        compd _val=0;
        for( size_t k=0; k < P1SF.Count(); k++ )  {
          double tv = -2*M_PI*(p.DotProd(P1SF[k].hkl)+P1SF[k].ps), ca, sa;
          olx_sincos(tv, &sa, &ca);
          _val += P1SF[k].val*compd(ca,sa);
        }
        data[i][j] = (float)_val.Re()/cell_vol;
        if( data[i][j] < minZ )  minZ = data[i][j];
        if( data[i][j] > maxZ )  maxZ = data[i][j];
      }
    }
*/
void OrtDraw::Render(const olxstr& fileName)  {
  PSWriter pw(fileName, true);
  Init(pw);
  TTypeList<a_ort_object> objects;
  TPtrList<vec3f> all_points;
  TGXApp::AtomIterator ai = app.GetAtoms();
  TGXApp::BondIterator bi = app.GetBonds();
  objects.SetCapacity(ai.count+bi.count);
  atoms.Clear();
  while (ai.HasNext()) {
    TXAtom& xa = ai.Next();
    // have to keep hidden atoms, as those might be used by bonds!
    if (xa.IsDeleted()) continue;
    xa.SetTag(objects.Count());
    ort_atom *a = new ort_atom(*this, xa);
    a->draw_style |= ortep_atom_rims;
    if (xa.DrawStyle() == adsOrtep)
      a->draw_style |= ortep_atom_quads;
    if ((ColorMode&ortep_color_lines))
      a->draw_style |= ortep_color_lines;
    if ((ColorMode&ortep_color_fill))
      a->draw_style |= ortep_color_fill;
    objects.Add(a);
    all_points.Add(&a->crd);
    atoms.Add(a);
  }
  {
    const TTypeListExt<TDRing, AGDrawObject> &rings = app.GetRings();
    for (size_t i = 0; i < rings.Count(); i++) {
      if (!rings[i].IsVisible()) continue;
      vec3f cnt = ProjectPoint(rings[i].Basis.GetCenter());
      ort_circle *c = new ort_circle(
        *this, cnt, rings[i].Basis.GetZoom()*DrawScale, false);
      c->basis = new mat3f(
        mat3d::Transpose(rings[i].Basis.GetMatrix())
          *app.GetRender().GetBasis().GetMatrix());
      objects.Add(c);
      all_points.Add(&c->center);
    }
  }
  if (app.DUnitCell().IsVisible()) {
    const TDUnitCell& uc = app.DUnitCell();
    for (size_t i=0; i < uc.EdgeCount(); i+=2) {
      ort_poly* l = new ort_poly(*this, false);
      l->points.AddNew(ProjectPoint(uc.GetEdge(i)));
      l->points.AddNew(ProjectPoint(uc.GetEdge(i+1)));
      objects.Add(l);
      _process_points(all_points, *l);
    }
  }
  if (app.DBasis().IsVisible()) {
    const TDBasis& b = app.DBasis();
    mat3f cm = app.XFile().GetAsymmUnit().GetCellToCartesian();
    vec3f len(cm[0].Length(), cm[1].Length(), cm[2].Length());
    cm[0].Normalise();  cm[1].Normalise();  cm[2].Normalise();
    cm *= ProjMatr;
    vec3d T = app.GetRender().GetBasis().GetMatrix()*b.GetCenter();
    T /= app.GetRender().GetZoom();
    T *= app.GetRender().GetScale();
    T -= app.GetRender().GetBasis().GetCenter();
    vec3f cnt = ProjectPoint(T);
    const float sph_rad = (float)(0.2*DrawScale*b.GetZoom());
    ort_circle* center = new ort_circle(*this, cnt, sph_rad, true);
    all_points.Add(center->center);
    center->color = 0xffffffff;
    objects.Add(center);
    for (int i=0; i < 3; i++) {
      vec3f mp = cm[i]*((float)(0.2*len[i]*b.GetZoom())),
        ep = cm[i]*((float)((0.2*len[i]+0.8)*b.GetZoom()));
      
      ort_cone* arrow_cone = new ort_cone(*this, cnt+mp, cnt+ep,
        (float)(0.2*DrawScale*b.GetZoom()), 0, 0);
      objects.Add(arrow_cone);
      all_points.Add(arrow_cone->bottom);
      all_points.Add(arrow_cone->top);

      const float z = cm[i][2]/cm[i].Length();
      const float pscale = 1+olx_sign(z)*sqrt(olx_abs(z))/2;
      const float base_r = (float)(0.075*DrawScale*b.GetZoom());
      ort_cone* axis_cone = new ort_cone(*this,
        cnt+vec3f(cm[i]).NormaliseTo(// extra 3D effect for the central sphere
          sqrt(olx_sqr(sph_rad)-olx_sqr(base_r))),
        cnt+mp, 
        base_r, 
        base_r*pscale,
        0); 
      objects.Add(axis_cone);
      all_points.Add(axis_cone->bottom);
      all_points.Add(axis_cone->top);
    }
  }
  if (Perspective && !all_points.IsEmpty()) {
    vec3f _min, _max;
    _min  = _max = (*all_points[0]);
    for (size_t i=1; i < all_points.Count(); i++)
      vec3f::UpdateMinMax(*all_points[i], _min, _max);
    vec3f center((_min+_max)/2);
    center[2] = (_max[2] - _min[2])*10;
    for (size_t i=0; i < all_points.Count(); i++) {
      vec3f& crd = *all_points[i];
      vec3f v(crd - center);
      v.NormaliseTo(center[2]);
      crd[0] = v[0] + center[0];
      crd[1] = v[1] + center[1];
    }
  }

  while (bi.HasNext()) {
    const TXBond& xb = bi.Next();
    if (xb.IsDeleted() || !xb.IsVisible())
      continue;
    const ort_atom& a1 = (const ort_atom&)objects[xb.A().GetTag()];
    const ort_atom& a2 = (const ort_atom&)objects[xb.B().GetTag()];
    ort_bond<TXBond> *b = new ort_bond<TXBond>(*this, xb, a1, a2);
    if ((ColorMode&ortep_color_bond) != 0)
      b->draw_style |= ortep_color_bond;
    objects.Add(b);
  }

  for (size_t i=0; i < app.LineCount(); i++) {
    TXLine &l = app.GetLine(i);
    if (!l.IsVisible()) continue;
    ort_bond_line *ol = new ort_bond_line(*this,
      l, l.Base(), l.Edge());
    objects.Add(ol);
  }
  const TXGrid& grid = app.XGrid();
  if (!grid.IsEmpty() && (grid.GetRenderMode()&planeRenderModeContour) != 0) {
    Contour<float> cm;
    ContourDrawer drawer(*this, objects, 0);
    Contour<float>::MemberFeedback<OrtDraw::ContourDrawer>
      mf(drawer, &OrtDraw::ContourDrawer::draw);
    const size_t MaxDim = grid.GetPlaneSize();
    const float hh = (float)MaxDim/2;
    const float Size = grid.GetSize();
    const float Depth = grid.GetDepth();
    olx_array_ptr<float*> data(new float*[MaxDim]);
    olx_array_ptr<float> x(new float[MaxDim]);
    olx_array_ptr<float> y(new float[MaxDim]);
    for (size_t i=0; i < MaxDim; i++) {
      data[i] = new float[MaxDim];
      y[i] = x[i] = (float)i - hh;
    }
    const size_t contour_cnt = grid.GetContourLevelCount();
    olx_array_ptr<float> z(new float[contour_cnt]);
    float minZ = 1000, maxZ = -1000;
    const mat3f bm(app.GetRender().GetBasis().GetMatrix());
    const mat3f c2c(app.XFile().GetAsymmUnit().GetCartesianToCell());
    const vec3f center(app.GetRender().GetBasis().GetCenter());
    MapUtil::MapGetter<float, 2>
      map_getter(grid.Data()->Data, grid.Data()->GetSize());
    for (size_t i=0; i < MaxDim; i++) {
      for (size_t j=0; j < MaxDim; j++) {
        vec3f p(((float)i-hh)/Size, ((float)j-hh)/Size,  Depth);
        p = bm*p;
        p -= center;
        p *= c2c;
        data[i][j] = map_getter.Get(p);
        if (data[i][j] < minZ) minZ = data[i][j];
        if (data[i][j] > maxZ) maxZ = data[i][j];
      }
    }
    float contour_step = (maxZ - minZ)/(contour_cnt-1);
    z[0] = minZ;
    for (size_t i=1; i < contour_cnt; i++)
      z[i] = z[i-1]+contour_step;
    cm.DoContour(data, 0, (int)MaxDim-1, 0, (int)MaxDim-1, x, y,
      contour_cnt, z, mf);
    for (size_t i=0; i < MaxDim; i++)
      delete [] data[i];
  }
  QuickSorter::SortSF(objects, OrtObjectsZSort);
  
  for (size_t i=0; i < objects.Count(); i++)
    objects[i].render(pw);

  TPtrList<const TXGlLabel> Labels;
  for (size_t i=0; i < app.LabelCount(); i++) {
    const TXGlLabel& glxl = app.GetLabel(i);
    if (glxl.IsVisible())
      Labels.Add(glxl);
  }
  ai.Reset();
  while (ai.HasNext()) {
    TXAtom& xa = ai.Next();
    if (xa.GetGlLabel().IsVisible())
      Labels.Add(xa.GetGlLabel());
  }
  bi.Reset();
  while (bi.HasNext()) {
    TXBond& xb = bi.Next();
    if (xb.GetGlLabel().IsVisible())
      Labels.Add(xb.GetGlLabel());
  }
  if (app.DUnitCell().IsVisible()) {
    for (size_t i=0; i < app.DUnitCell().LabelCount(); i++) {
      const TXGlLabel& glxl = app.DUnitCell().GetLabel(i);
      if (glxl.IsVisible())
        Labels.Add(glxl);
    }
  }
  if (app.DBasis().IsVisible()) {
    for (size_t i=0; i < app.DBasis().LabelCount(); i++) {
      const TXGlLabel& glxl = app.DBasis().GetLabel(i);
      if (glxl.IsVisible())
        Labels.Add(glxl);
    }
  }
  for (size_t i=0; i < app.LineCount(); i++) {
    TXLine &l = app.GetLine(i);
    if (l.GetGlLabel().IsVisible())
      Labels.Add(l.GetGlLabel());
  }
  evecf boundary(4);
  boundary[0] = 596;
  boundary[1] = 842;
  boundary[2] = -596;
  boundary[3] = -842;
  for (size_t i=0; i < objects.Count(); i++)
    objects[i].update_size(boundary);
  // labels rendering block
  {
    TGlFont::PSRenderContext context;
    TCStrList output;
    uint32_t prev_ps_color = 0;
    output.Add(pw.color_str(prev_ps_color));
    const float vector_scale = (float)(1./app.GetRender().GetScale());
    for (size_t i=0; i < Labels.Count(); i++) {
      const TGlFont& glf = Labels[i]->GetFont();
      uint32_t color = 0;
      TGlMaterial* glm =
        Labels[i]->GetPrimitives().GetStyle().FindMaterial("Text");
      if( glm != NULL )
        color = glm->AmbientF.GetRGB();
      pw.color(color);
      if (glf.IsVectorFont()) {
        const float font_scale = (float)(DrawScale/app.GetRender().CalcZoom());
        vec3f crd = Labels[i]->GetVectorPosition()*vector_scale + DrawOrigin;
        const TTextRect &r = Labels[i]->GetRect();
        a_ort_object::update_min_max(boundary,
          vec3f((float)(crd[0]+r.left*font_scale),
            (float)(crd[1]+r.top*vector_scale), 0));
        a_ort_object::update_min_max(boundary,
          vec3f((float)(crd[0]+(r.left+r.width)*font_scale),
            (float)(crd[1]+(r.top+r.height)*font_scale), 0));
        if (color != prev_ps_color) {
          output.Add(pw.color_str(color));
          prev_ps_color = color;
        }
        output.AddList(
          glf.RenderPSLabel(
            crd, Labels[i]->GetLabel(), font_scale, context)
        );
      }
      else {
        pw.color(color);
        vec3f rp = Labels[i]->GetRasterPosition();
        rp[1] += 4;
        pw.drawText(Labels[i]->GetLabel(), rp+DrawOrigin);
      }
    }
    if (!output.IsEmpty()) {
      for (size_t i=0; i < context.definitions.Count(); i++)
        pw.custom(context.definitions[i].definition);
      pw.lineWidth(FontLineWidth);
      pw.custom(output);
    }
  }
  // scale a bit up
  float scale_up = 1.10;
  float mx = (boundary[2]+boundary[0])/2;
  float my = (boundary[3]+boundary[1])/2;
  boundary[0] = (boundary[0]-mx)*scale_up + mx;
  boundary[1] = (boundary[1]-my)*scale_up + my;
  boundary[2] = (boundary[2]-mx)*scale_up + mx;
  boundary[3] = (boundary[3]-my)*scale_up + my;
  boundary *= LinearScale;
  boundary[1] += YOffset;
  boundary[3] += YOffset;
  pw.writeBoundingBox(boundary);
}

void OrtDraw::ContourDrawer::draw(float x1, float y1, float x2, float y2,
  float z)
{
  const float Size = parent.app.XGrid().GetSize();
  const float Depth = parent.app.XGrid().GetDepth();
  vec3d p1(x1/Size, y1/Size, Depth), p2(x2/Size, y2/Size, Depth);
  p1 = parent.basis.GetMatrix()*p1 - parent.basis.GetCenter();
  p2 = parent.basis.GetMatrix()*p2 - parent.basis.GetCenter();
  if( z < 0 )
    p2 = (p1+p2)*0.5;
  objects.Add(new ort_line(parent, parent.ProjectPoint(p1),
    parent.ProjectPoint(p2), color));
}

float OrtDraw::GetLineRad(const ort_bond_line& b, uint32_t mask) const {
  float r = BondRad;
  //even thinner for line or "balls" bond
  if( (mask&((1<<13)|(1<<12)|(1<<11)|(1<<7)|(1<<6))) != 0 )
    r /= 4;
  return r;
}
