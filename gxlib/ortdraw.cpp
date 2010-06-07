#include "ortdraw.h"
#include "xatom.h"
#include "xbond.h"
#include "styles.h"
#include "gllabel.h"
#include "dunitcell.h"
#include "dbasis.h"
#include "xgrid.h"
#include "conrec.h"
//
#include "sfutil.h"
#include "unitcell.h"

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
  sphere_color = atom.Atom().GetType().def_color;
  const TGraphicsStyle& style = atom.GetPrimitives().GetStyle();
  size_t lmi = style.IndexOfMaterial("Sphere");
  if( lmi != InvalidIndex )  {
    TGlMaterial& glm = style.GetPrimitiveStyle(lmi).GetProperties();
    sphere_color = glm.AmbientF.GetRGB();
  }
  rim_color = atom.Atom().GetType().def_color;
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
  if( parent.GetBondOutlineSize() > 0 )  {
    pw.color(parent.GetBondOutlineColor());
    _render(pw, (parent.GetBondOutlineSize()+1.0f), mask);
  }
  if( (draw_style&ortep_color_bond) == 0 )
    pw.color(0);
  else if( (mask&((1<<4)|(1<<5)|(1<<6)|(1<<7)|(1<<9)|(1<<10))) == 0 )
    pw.color(atom_a.atom.Atom().GetType() > atom_b.atom.Atom().GetType() ?
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
  const vec3f touch_point = (atom_b.atom.Atom().crd() - atom_a.atom.Atom().crd()).Normalise();
  if( olx_abs(1.0f-olx_abs(touch_point[2])) < 1e-3 )  // degenerated case...
    CreateRotationMatrixEx<float,mat3f,vec3f>(rot_mat, vec3f(0, 1, 0).Normalise(), touch_point[2]);
  else
    CreateRotationMatrixEx<float,mat3f,vec3f>(rot_mat, vec3f(-touch_point[1], touch_point[0], 0).Normalise(), touch_point[2]);
  const mat3f proj_mat = rot_mat*parent.ProjMatr;
  const float _brad = brad*(1+pers_scale)*scalex;
  if( !atom_a.IsSpherical() && atom_a.IsSolid() )  {
    const mat3f& ielpm = *atom_a.p_ielpm;
    vec3f bproj_cnt;
    for( uint16_t j=0; j < parent.BondDiv; j++ )  {
      parent.BondProjF[j] = (((parent.BondCrd[j]*rot_mat+touch_point)*parent.ProjMatr).Normalise()*ielpm);
      bproj_cnt += parent.BondProjF[j];
    }
    bproj_cnt /= parent.BondDiv;
    for( uint16_t j=0; j < parent.BondDiv; j++ )  {
      parent.BondProjF[j] = (parent.BondProjF[j]-bproj_cnt).NormaliseTo(_brad) + bproj_cnt;
      parent.BondProjT[j] = (parent.BondCrd[j]*proj_mat).NormaliseTo(brad*2*scalex) + dir_vec*b_len;
    }
  }
  else  {
    const float off_len = !atom_a.IsSolid() ? 0 : 
      (atom_a.draw_rad > _brad ? sqrt(olx_sqr(atom_a.draw_rad)-olx_sqr(_brad)) : atom_a.draw_rad);
    for( uint16_t j=0; j < parent.BondDiv; j++ )  {
      parent.BondProjT[j] = parent.BondProjF[j] = (parent.BondCrd[j]*proj_mat).NormaliseTo(_brad); 
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

    // renders an intersection bond and ellipsoid ellipse
    //if( !atom_a.IsSpherical() && atom_a.IsSolid() )  {
    //  pw.color(0xff);
    //  mat3f pm = proj_mat;
    //  pm[0].Normalise();
    //  pm[1].Normalise();
    //  pm[2].Normalise();
    //  pm *= *atom_a.p_ielpm;
    //  pw.drawEllipse(NullVec, pm);
    //}
  }
}

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

void ort_cone::render(PSWriter& pw) const {
  vec3f n = (top-bottom).Normalise();
  mat3f rm, basis = TEBasis::CalcBasis<vec3f,mat3f>(n);
  CreateRotationMatrixEx<float, mat3f, vec3f>(rm, n, (float)cos(M_PI*2/divs));
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
  GLfloat vp[4];
  olx_gl::get(GL_VIEWPORT, vp);
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
  pw.translate(0.0f, ((float)pw.GetHeight()-LinearScale*vp[3])/2);
  pw.scale(LinearScale, LinearScale);
  LinearScale = 1; // reset now
  DrawScale = app.GetRender().GetBasis().GetZoom()/(LinearScale*app.GetRender().GetScale());
  BondRad = 0.05*DrawScale;
  SceneOrigin = basis.GetCenter();
  DrawOrigin = vec3f(vp[2]/2, vp[3]/2, 0);
  ProjMatr = basis.GetMatrix()*DrawScale;  
  UnProjMatr = ProjMatr.Inverse();
}
/*
Grid interpolation stuff...
http://xtal.sourceforge.net/man/slant-desc.html
the cubic interpolation does a good job on coarse grids (compared with direct calculations)
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
    SFUtil::GetSF(refs, F, SFUtil::mapTypeCalc, SFUtil::sfOriginOlex2, SFUtil::scaleSimple);
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
          SinCos(tv, &sa, &ca);
          _val += P1SF[k].val*compd(ca,sa);
        }
        data[i][j] = (float)_val.Re()/cell_vol;
        if( data[i][j] < minZ )  minZ = data[i][j];
        if( data[i][j] > maxZ )  maxZ = data[i][j];
      }
    }
*/
void OrtDraw::Render(const olxstr& fileName)  {
  PSWriter pw(fileName);
  Init(pw);
  TTypeList<a_ort_object> objects;
  TPtrList<vec3f> all_points;
  objects.SetCapacity(app.AtomCount()+app.BondCount());
  for( size_t i=0; i < app.AtomCount(); i++ )  {
    if( app.GetAtom(i).IsDeleted() ) // have to keep hidden atoms, as those might be used by bonds!
      continue;
    app.GetAtom(i).Atom().SetTag(objects.Count());
    ort_atom *a = new ort_atom(*this, app.GetAtom(i));
    a->draw_style |= ortep_atom_rims;
    if( app.GetAtom(i).DrawStyle() == adsOrtep )
      a->draw_style |= ortep_atom_quads;
    if( (ColorMode&ortep_color_lines) )
      a->draw_style |= ortep_color_lines;
    if( (ColorMode&ortep_color_fill) )
      a->draw_style |= ortep_color_fill;
    objects.Add(a);
    all_points.Add(&a->crd);
  }
  if( app.DUnitCell().IsVisible() )  {
    const TDUnitCell& uc = app.DUnitCell();
    for( size_t i=0; i < uc.EdgeCount(); i+=2 )  {
      ort_poly* l = new ort_poly(*this, false);
      l->points.AddNew(ProjectPoint(uc.GetEdge(i)));
      l->points.AddNew(ProjectPoint(uc.GetEdge(i+1)));
      objects.Add(l);
      _process_points(all_points, *l);
    }
  }
  if( app.DBasis().IsVisible() )  {
    const TDBasis& b = app.DBasis();
    mat3f cm = app.XFile().GetAsymmUnit().GetCellToCartesian();
    vec3f len(cm[0].Length(), cm[1].Length(), cm[2].Length());
    cm[0].Normalise();  cm[1].Normalise();  cm[2].Normalise();
    cm *= ProjMatr;
    vec3d T = app.GetRender().GetBasis().GetMatrix()*b.Basis.GetCenter();
    T /= app.GetRender().GetZoom();
    T *= app.GetRender().GetScale();
    T -= app.GetRender().GetBasis().GetCenter();
    vec3f cnt = ProjectPoint(T);
    const float sph_rad = 0.2*DrawScale*b.Basis.GetZoom();
    ort_circle* center = new ort_circle(*this, cnt, sph_rad, true);
    all_points.Add(center->center);
    center->color = 0xffffffff;
    objects.Add(center);
    for( int i=0; i < 3; i++ )  {
      vec3f mp = cm[i]*((float)(0.2*len[i]*b.Basis.GetZoom())), 
        ep = cm[i]*((float)((0.2*len[i]+0.8)*b.Basis.GetZoom()));
      
      ort_cone* arrow_cone = new ort_cone(*this, cnt+mp, cnt+ep, 0.2*DrawScale*b.Basis.GetZoom(), 0, 0); 
      objects.Add(arrow_cone);
      all_points.Add(arrow_cone->bottom);
      all_points.Add(arrow_cone->top);

      const float z = cm[i][2]/cm[i].Length();
      const float pscale = 1+olx_sign(z)*sqrt(olx_abs(z))/2;
      const float base_r = 0.075*DrawScale*b.Basis.GetZoom();
      ort_cone* axis_cone = new ort_cone(*this,
        cnt+vec3f(cm[i]).NormaliseTo(sqrt(olx_sqr(sph_rad)-olx_sqr(base_r))), // extra 3D effect for the central sphere
        cnt+mp, 
        base_r, 
        base_r*pscale,
        0); 
      objects.Add(axis_cone);
      all_points.Add(axis_cone->bottom);
      all_points.Add(axis_cone->top);
    }
  }
  if( Perspective && !all_points.IsEmpty() )  {
    vec3f _min, _max;
    _min  = _max = (*all_points[0]);
    for( size_t i=1; i < all_points.Count(); i++ )
      vec3f::UpdateMinMax(*all_points[i], _min, _max);
    vec3f center((_min+_max)/2);
    center[2] = (_max[2] - _min[2])*10;
    for( size_t i=0; i < all_points.Count(); i++ )  {
      vec3f& crd = *all_points[i];
      vec3f v(crd - center);
      v.NormaliseTo(center[2]);
      crd[0] = v[0] + center[0];
      crd[1] = v[1] + center[1];
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
  const TXGrid& grid = app.XGrid();
  if( !grid.IsEmpty() && (grid.GetRenderMode()&planeRenderModeContour) != 0 )  {
    Contour<float> cm;
    ContourDrawer drawer(*this, objects, 0);
    Contour<float>::MemberFeedback<OrtDraw::ContourDrawer> mf(drawer, &OrtDraw::ContourDrawer::draw);
    int MaxDim = grid.GetPlaneSize();
    float Size = grid.GetSize();
    float Depth = grid.GetDepth();
    float **data = new float*[MaxDim];
    float *x = new float[MaxDim];
    float *y = new float[MaxDim];
    for( int i=0; i < MaxDim; i++ )  {
      data[i] = new float[MaxDim];
      y[i] = x[i] = i - MaxDim/2;
    }
    const int contour_cnt = grid.GetContourLevelCount();
    float* z = new float[contour_cnt];
    float minZ = 1000, maxZ = -1000;
    const vec3i dim = grid.GetDimVec();
    const mat3f bm(app.GetRender().GetBasis().GetMatrix());
    const mat3f c2c(app.XFile().GetAsymmUnit().GetCartesianToCell());
    const float hh = (float)MaxDim/2;
    const vec3f center(app.GetRender().GetBasis().GetCenter());
    for( int i=0; i < MaxDim; i++ )  {
      for( int j=0; j < MaxDim; j++ )  {
        vec3f p((float)(i-hh)/Size, (float)(j-hh)/Size,  Depth);
        p = bm*p;
        p -= center;
        p *= c2c;
        p *= dim;
        vec3i fp((int)(p[0]), (int)(p[1]), (int)(p[2]));
        float val = 0;
        float _p = p[0]-fp[0], _pc = _p*_p*_p, _ps = _p*_p;
        float _q = p[1]-fp[1], _qc = _q*_q*_q, _qs = _q*_q;
        float _r = p[2]-fp[2], _rc = _r*_r*_r, _rs = _r*_r;
        const float vx[4] = {-_pc/6 + _ps/2 -_p/3, (_pc-_p)/2 - _ps + 1, (-_pc + _ps)/2 + _p, (_pc - _p)/6 };
        const float vy[4] = {-_qc/6 + _qs/2 -_q/3, (_qc-_q)/2 - _qs + 1, (-_qc + _qs)/2 + _q, (_qc - _q)/6 };
        const float vz[4] = {-_rc/6 + _rs/2 -_r/3, (_rc-_r)/2 - _rs + 1, (-_rc + _rs)/2 + _r, (_rc - _r)/6 };
        for( int dx=-1; dx <= 2; dx++ )  {
          const float _vx = vx[dx+1];
          const int n_x = fp[0]+dx;
          for( int dy=-1; dy <= 2; dy++ )  {
            const float _vxy = vy[dy+1]*_vx;
            const int n_y = fp[1]+dy;
            for( int dz=-1; dz <= 2; dz++ )  {
              const float _vxyz = vz[dz+1]*_vxy;
              vec3i ijk(n_x, n_y, fp[2]+dz);
              for( int m=0; m < 3; m++ )  {
                while( ijk[m] < 0 )
                  ijk[m] += dim[m];
                while( ijk[m] >= dim[m] )
                  ijk[m] -= dim[m];
              }
              val += grid.GetValue(ijk)*_vxyz;
            }
          }
        }
        data[i][j] = val;
        if( data[i][j] < minZ )  minZ = data[i][j];
        if( data[i][j] > maxZ )  maxZ = data[i][j];
      }
    }
    float contour_step = (maxZ - minZ)/(contour_cnt-1);
    z[0] = minZ;
    for( int i=1; i < contour_cnt; i++ )
      z[i] = z[i-1]+contour_step;
    cm.DoContour(data, 0, MaxDim-1, 0, MaxDim-1, x, y, contour_cnt, z, mf);
    for( int i=0; i < MaxDim; i++ )
      delete [] data[i];
    delete [] data;
    delete [] x;
    delete [] y;
    delete [] z;
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
        glf.RenderPSLabel(crd, glxl.GetLabel(), out, DrawScale/app.GetRender().CalcZoom(), defs);
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

void OrtDraw::ContourDrawer::draw(float x1, float y1, float x2, float y2, float z)  {
  const float Size = parent.app.XGrid().GetSize();
  const float Depth = parent.app.XGrid().GetDepth();
  vec3d p1(x1/Size, y1/Size, Depth), p2(x2/Size, y2/Size, Depth);
  p1 = parent.basis.GetMatrix()*p1 - parent.basis.GetCenter();
  p2 = parent.basis.GetMatrix()*p2 - parent.basis.GetCenter();
  if( z < 0 )
    p2 = (p1+p2)*0.5;
  objects.Add(new ort_line(parent, parent.ProjectPoint(p1), parent.ProjectPoint(p2), color));
}

float OrtDraw::GetBondRad(const ort_bond& b, uint32_t mask) const {
  float r = (b.bond.Bond().A().GetType() == iHydrogenZ || b.bond.Bond().B().GetType() < iHydrogenZ) ? 
    BondRad*HBondScale : BondRad;
  if( (mask&((1<<13)|(1<<12)|(1<<11)|(1<<7)|(1<<6))) != 0 )  //even thinner for line or "balls" bond
    r /= 4;
  return r;
}

