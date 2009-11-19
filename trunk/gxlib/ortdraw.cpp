#include "ortdraw.h"
#include "xatom.h"
#include "xbond.h"
#include "gllabel.h"
#include "styles.h"

ort_atom::ort_atom(const OrtDraw& parent, const TXAtom& a) :
a_ort_object(parent), atom(a), p_elpm(NULL), p_ielpm(NULL),
draw_style(0)
{
  const TSAtom& sa = atom.Atom();
  if( sa.GetEllipsoid() != NULL )  {
    mat3f& elpm = *(new mat3f(sa.GetEllipsoid()->GetMatrix()));
    elpm[0] *= sa.GetEllipsoid()->GetSX();
    elpm[1] *= sa.GetEllipsoid()->GetSY();
    elpm[2] *= sa.GetEllipsoid()->GetSZ();
    elpm *= (float)atom.GetDrawScale();
    elpm *= parent.ProjMatr;
    mat3f& ielpm = *(new mat3f( (sa.GetEllipsoid()->GetMatrix() * parent.basis.GetMatrix() ).Inverse()) );
    p_elpm = &elpm;
    p_ielpm = &(ielpm *= elpm);
  }
  draw_rad = (float)atom.GetDrawScale()*parent.DrawScale;
  crd = parent.ProjectPoint(sa.crd());
  sphere_color = atom.Atom().GetAtomInfo().GetDefColor();
  const TGraphicsStyle& style = atom.GetPrimitives().GetStyle();
  size_t lmi = style.IndexOfMaterial("Sphere");
  if( lmi != InvalidIndex )  {
    TGlMaterial& glm = style.GetPrimitiveStyle(lmi).GetProperties();
    sphere_color = glm.AmbientF.GetRGB();
  }
  rim_color = atom.Atom().GetAtomInfo().GetDefColor();
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
  pw.color(~0);
  pw.drawEllipse(NullVec, ielpm*1.05f, &PSWriter::fill);
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
  pw.color(~0);
  pw.drawCircle(NullVec, draw_rad*1.05, &PSWriter::fill);
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
  bool res = !(p_elpm != NULL && (atom.DrawStyle() == adsEllipsoid || atom.DrawStyle() == adsOrtep));
  if( !res && atom.Atom().GetEllipsoid()->IsNPD() )
    return true;
  return res;
}

void ort_atom::render(PSWriter& pw) const {
  if( mask == 0 )  return;
  pw.translate(crd);
  pw.lineWidth(parent.ElpLineWidth);
  if( mask == 16 && atom.DrawStyle() == adsStandalone )  {
    if( atom.Atom().IsStandalone() )
      render_standalone(pw);
  }
  else if( !IsSpherical() )
    render_elp(pw);
  else
    render_sph(pw);
  pw.translate(-crd);
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

ort_bond::ort_bond(const OrtDraw& parent, const TXBond& _bond, const ort_atom& a1, const ort_atom& a2) :
  a_ort_object(parent),  
  bond(_bond),
  atom_a(a1.get_z() < a2.get_z() ? a1 : a2), 
  atom_b(a1.get_z() < a2.get_z() ? a2 : a1)
{
  draw_style = 0;
}

                                                  
void ort_bond::render(PSWriter& pw) const {
  uint32_t mask = bond.GetPrimitiveMask();
  if( mask == 0 )  return;
  pw.lineWidth(1.0f);
  pw.translate(atom_a.crd);
  pw.color(~0);
  _render(pw, 1.2, mask);
  if( (draw_style&ortep_color_bond) == 0 )
    pw.color(0);
  else if( (mask&((1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<10))) == 0 )
    pw.color(atom_a.atom.Atom().GetAtomInfo() > atom_b.atom.Atom().GetAtomInfo() ?
      atom_a.sphere_color : atom_b.sphere_color);
  _render(pw, 1, mask);
  pw.translate(-atom_a.crd);
}
void ort_bond::_render(PSWriter& pw, float scalex, uint32_t mask) const {
  if( mask == (1<<12) || mask == (1<<13) || (mask&((1<<6)|(1<<7))) != 0 )  {
    pw.lineWidth(scalex);
    if( (draw_style&ortep_color_bond) != 0 )  {
      if( (mask&(1<<6)) !=0 )  {
        pw.color(atom_a.sphere_color);
        pw.drawLine(NullVec, (atom_b.crd-atom_a.crd)/2);
      }
      if( (mask&(1<<7)) !=0 )  {
        pw.color(atom_b.sphere_color);
        pw.drawLine((atom_b.crd-atom_a.crd)/2, (atom_b.crd-atom_a.crd));
      }
    }
    if( mask == (1<<13) )
      pw.custom("[8 8] 0 setdash");
    if( (mask&((1<<12)|(1<<13))) != 0 || (draw_style&ortep_color_bond) == 0 )
      pw.drawLine(NullVec, atom_b.crd-atom_a.crd);
    if( mask == (1<<13) )
      pw.custom("[] 0 setdash");
    return;
  }
  vec3f dir_vec = atom_b.crd-atom_a.crd;
  const float b_len = dir_vec.Length();
  const float brad = parent.GetBondRad(*this, mask)*bond.GetRadius();
  dir_vec.Normalise();
  const float pers_scale = 1.0-olx_sqr(dir_vec[2]);
  mat3f rot_mat;
  vec3f touch_point = (atom_b.atom.Atom().crd() - atom_a.atom.Atom().crd()).Normalise();
  vec3f rot_vec(-touch_point[1], touch_point[0], 0);
  CreateRotationMatrixEx<float,mat3f,vec3f>(rot_mat, rot_vec.Normalise(), touch_point[2]);
  mat3f proj_mat = rot_mat*parent.ProjMatr;
  if( !atom_a.IsSpherical() && atom_a.IsSolid() )  {
    const mat3f& ielpm = *atom_a.p_ielpm;
    vec3f bproj_cnt;
    vec3f touch_point_proj = dir_vec*ielpm;
    for( uint16_t j=0; j < parent.BondDiv; j++ )  {
      parent.BondProjF[j] = (((parent.BondCrd[j]*rot_mat+touch_point)*parent.ProjMatr).Normalise()*ielpm);
      bproj_cnt += parent.BondProjF[j];
    }
    bproj_cnt /= parent.BondDiv;
    for( uint16_t j=0; j < parent.BondDiv; j++ )  {
      parent.BondProjF[j] = (parent.BondProjF[j]-bproj_cnt).NormaliseTo(brad*(1+pers_scale)*scalex) + bproj_cnt;
      parent.BondProjT[j] = (parent.BondCrd[j]*proj_mat).NormaliseTo(brad*2*scalex) + dir_vec*b_len;
    }
  }
  else  {
    const float off_len = atom_a.IsSolid() ? atom_a.draw_rad/2 : 0;
    for( uint16_t j=0; j < parent.BondDiv; j++ )  {
      parent.BondProjT[j] = parent.BondProjF[j] = (parent.BondCrd[j]*proj_mat).NormaliseTo(brad*(1+pers_scale)*scalex); 
      parent.BondProjT[j].NormaliseTo(brad*2*scalex) += dir_vec*b_len;
      parent.BondProjF[j] += dir_vec*off_len;
    }
  }
  if( scalex < 1.1 && (draw_style&ortep_color_bond) != 0 &&
    (mask&((1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<10))) != 0 )
  {
    for( uint16_t i=0; i < parent.BondDiv; i++ )  {
      parent.BondProjM[i][0] = (parent.BondProjT[i][0]+parent.BondProjF[i][0])/2;
      parent.BondProjM[i][1] = (parent.BondProjT[i][1]+parent.BondProjF[i][1])/2;
      parent.BondProjM[i][2] = (parent.BondProjT[i][2]+parent.BondProjF[i][2])/2;
    }
    if( (mask&((1<<4)|(1<<6)|(1<<9))) != 0 )  {
      pw.color(atom_a.sphere_color);
      if( (mask&(1<<9)) != 0 )
        pw.drawQuads(parent.BondProjF, parent.BondProjM, 8, &PSWriter::fill);
      else
        pw.drawQuads(parent.BondProjF, parent.BondProjM, &PSWriter::fill);
    }
    if( (mask&((1<<5)|(1<<7)|(1<<10))) != 0 )  {
      pw.color(atom_b.sphere_color);
      if( (mask&(1<<10)) != 0 )
        pw.drawQuads(parent.BondProjM, parent.BondProjT, 8, &PSWriter::fill);
      else
        pw.drawQuads(parent.BondProjM, parent.BondProjT, &PSWriter::fill);
    }
  }
  else  {
    if( (mask&((1 << 13)|(1<<11)|(1<<10)|(1<<9)|(1<<8))) != 0)
      pw.drawQuads(parent.BondProjF, parent.BondProjT, 16, &PSWriter::fill);
    else
      pw.drawQuads(parent.BondProjF, parent.BondProjT, &PSWriter::fill);
  }
}

void OrtDraw::RenderRims(PSWriter& pw, const mat3f& pelpm, const vec3f& norm_vec) const {
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
  for( uint16_t j=0; j < PieDiv; j++ )
    pw.drawLine(vec3f(0, PieCrd[j][0], PieCrd[j][1])*pelpm, pelpm[1]*((float)(PieDiv-j)/PieDiv));
  for( uint16_t j=0; j < PieDiv; j++ )
    pw.drawLine(vec3f(PieCrd[j][1], 0, PieCrd[j][0])*pelpm, pelpm[2]*((float)(PieDiv-j)/PieDiv));
}

size_t OrtDraw::PrepareArc(const TArrayList<vec3f>& in, TPtrList<const vec3f>& out, const vec3f& normal) const {
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
  double sin_a, cos_a;
  SinCos(2*M_PI/ElpDiv, &sin_a, &cos_a);
  vec3f ps(cos_a, -sin_a, 0);
  for( uint16_t i=0; i < ElpDiv; i++ )  {
    ElpCrd[i] = ps;
    const float x = ps[0];
    ps[0] = (float)(cos_a*x + sin_a*ps[1]);
    ps[1] = (float)(cos_a*ps[1] - sin_a*x);
  }
  SinCos(2*M_PI/BondDiv, &sin_a, &cos_a);
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
  float vp[4];
  glGetFloatv(GL_VIEWPORT, vp);
  TGXApp& app = TGXApp::GetInstance();
  const TEBasis& basis = app.GetRender().GetBasis();
  LinearScale = olx_min((float)pw.GetWidth()/vp[2], (double)pw.GetHeight()/vp[3]);

  if( app.LabelCount() != 0 )  {
    if( !app.GetLabel(0).GetFont().IsVectorFont() )  {
      olxcstr fnt("/Verdana findfont ");
      fnt << olx_round(app.GetLabel(0).GetFont().GetPointSize()/sqrt(LinearScale)) << " scalefont setfont";
      pw.custom(fnt.c_str());
    }
  }

  pw.scale(LinearScale, LinearScale);
  LinearScale = 1; // reset now
  DrawScale = 1./(LinearScale*app.GetRender().GetScale());
  BondRad = 0.05*DrawScale;
  SceneOrigin = basis.GetCenter();
  DrawOrigin = vec3f(vp[2]/2, vp[3]/2, 0);
  ProjMatr = basis.GetMatrix()*DrawScale;  
  UnProjMatr = ProjMatr.Inverse();
}

void OrtDraw::Render(const olxstr& fileName)  {
  PSWriter pw(fileName);
  Init(pw);
  TTypeList<a_ort_object> objects;
  objects.SetCapacity(app.AtomCount()+app.BondCount());
  for( size_t i=0; i < app.AtomCount(); i++ )  {
    if( app.GetAtom(i).IsDeleted() ) // have to keep hidden atoms, as those might be used by bonds!
      continue;
    app.GetAtom(i).Atom().SetTag(objects.Count());
    ort_atom *a = new ort_atom(*this, app.GetAtom(i));
    a->draw_style |= ortep_atom_rims;
    if( a->atom.Atom().GetAtomInfo() != iCarbonIndex )
      a->draw_style |= ortep_atom_quads;
    if( (ColorMode&ortep_color_lines) )
      a->draw_style |= ortep_color_lines;
    if( (ColorMode&ortep_color_fill) )
      a->draw_style |= ortep_color_fill;
    objects.Add(a);
  }
  if( Perspective && !objects.IsEmpty() )  {
    float min_z, max_z;
    min_z  = max_z = objects[0].get_z();
    vec3f center = ((ort_atom&)objects[0]).crd;
    for( size_t i=1; i < objects.Count(); i++ )  {
      const ort_atom& a = (const ort_atom&)objects[i];
      if( a.crd[2] < min_z )  min_z = a.crd[2];
      if( a.crd[2] > max_z )  max_z = a.crd[2];
      center += a.crd;
    }
    center /= objects.Count();
    center[2] = (max_z - min_z)*10;
    for( size_t i=0; i < objects.Count(); i++ )  {
      ort_atom& oa = (ort_atom&)objects[i];
      vec3f v(oa.crd - center);
      v.NormaliseTo(center[2]);
      oa.crd[0] = v[0]+center[0];
      oa.crd[1] = v[1]+center[1];
    }
  }
  for( size_t i=0; i < app.BondCount(); i++ )  {
    const TXBond& xb = app.GetBond(i);
    if( xb.IsDeleted() || !xb.IsVisible() )
      continue;
    const ort_atom& a1 = (const ort_atom&)objects[xb.Bond().A().GetTag()];
    const ort_atom& a2 = (const ort_atom&)objects[xb.Bond().B().GetTag()];
    ort_bond *b = new ort_bond(*this, app.GetBond(i), a1, a2);
    if( (ColorMode&ortep_color_bond) != 0 )
      b->draw_style |= ortep_color_bond;
    objects.Add(b);
  }
  objects.QuickSorter.SortSF(objects, OrtObjectsZSort);

  for( size_t i=0; i < objects.Count(); i++ )
    objects[i].render(pw);

  if( app.LabelCount() != 0 )  {
    TGlFont& glf = app.GetLabel(0).GetFont();
    uint32_t color = 0;
    TGlMaterial* glm = app.GetLabel(0).GetPrimitives().GetStyle().FindMaterial("Text");
    if( glm != NULL )
      color = glm->AmbientF.GetRGB();
    pw.color(color);
    if( glf.IsVectorFont() )  {
      TStrList out;
      olxdict<size_t, olxstr, TPrimitiveComparator> defs;
      for( size_t i=0; i < app.LabelCount(); i++ )  {
        const TXGlLabel& glxl = app.GetLabel(i);
        if( glxl.IsDeleted() || !glxl.IsVisible() )  continue;
        vec3d crd = glxl.GetVectorPosition()*DrawScale + DrawOrigin;
        glf.RenderPSLabel(crd, glxl.GetLabel(), out, DrawScale, defs);
      }
      pw.lineWidth(FontLineWidth);
      for( size_t i=0; i < out.Count(); i++ )
        pw.custom(out[i].c_str());
    }
    else  {
      for( size_t i=0; i < app.LabelCount(); i++ )  {
        const TXGlLabel& glxl = app.GetLabel(i);
        if( glxl.IsDeleted() || !glxl.IsVisible() )  continue;
        vec3f rp = glxl.GetRasterPosition();
        rp[1] += 4;
        pw.drawText(glxl.GetLabel(), rp+DrawOrigin);
      }
    }
  }
}

float OrtDraw::GetBondRad(const ort_bond& b, uint32_t mask) const {
  float r = (b.bond.Bond().A().GetAtomInfo() < 4 || b.bond.Bond().B().GetAtomInfo() < 4) ? 
    BondRad*HBondScale : BondRad;
  if( (mask&((1<<13)|(1<<12)|(1<<11)|(1<<7)|(1<<6))) != 0 )  //even thinner for line or "balls" bond
    r /= 4;
  return r;
}

