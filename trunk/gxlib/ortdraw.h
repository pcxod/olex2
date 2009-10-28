#ifndef __olx_ort_Draw_H
#define __olx_ort_Draw_H
#include "gxapp.h"
#include "ps_writer.h"
static const short
  ortep_color_None = 0,
  ortep_color_Lines = 1,
  ortep_color_Fill = 2;

class OrtDraw  {
private:
  struct OrtAtom {
    const TSAtom* atom;
    vec3f crd;
    mat3f *elpm, *ielpm;
    double draw_rad;
    OrtAtom(const TSAtom* a, const vec3f& c, double _draw_rad, mat3f* em = NULL, mat3f* iem = NULL) :
      atom(a), draw_rad(_draw_rad), crd(c), elpm(em), ielpm(iem) {}
    OrtAtom(const OrtAtom& a) :
      atom(a.atom), draw_rad(a.draw_rad), crd(a.crd), elpm(a.elpm), ielpm(a.ielpm) {}
    ~OrtAtom()  {
      if( elpm != NULL )  delete elpm;
      if( ielpm != NULL )  delete ielpm;
    }
  };
  static int OrtAtomZSort(const OrtAtom& a1, const OrtAtom& a2)  {
    const float diff = a1.crd[2] - a2.crd[2];
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
  float DrawScale, BondRad, LinearScale;
  mat3f ProjMatr, UnProjMatr;
  vec3f DrawOrigin, SceneOrigin;
  TGXApp& app;
  const vec3f NullVec;
  short ColorMode;
protected:
  uint16_t ElpDiv, PieDiv, BondDiv;
  TArrayList<vec3f> ElpCrd, PieCrd, Arc, BondCrd, BondProjF, BondProjT;
  TPtrList<const vec3f> FilteredArc;
  float PieLineWidth, 
    ElpLineWidth, 
    QuadLineWidth,
    HBondScale;

  size_t PrepareArc(const TArrayList<vec3f>& in, TPtrList<const vec3f>& out, const vec3f& normal)  {
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
  void DoDrawBonds(PSWriter& pw, const OrtAtom& oa, uint32_t color, float scalex) const  {
    pw.color(color);
    const TSAtom& sa = *oa.atom;
    vec3f dir_vec, touch_point, touch_point_proj, off_vec, bproj_cnt;
    mat3f proj_mat, rot_mat;
    for( size_t j=0; j < sa.BondCount(); j++ )  {
      const TSBond& bn = sa.Bond(j);
      if( app.GetBond( bn.GetTag() ).IsDeleted() || !app.GetBond( bn.GetTag() ).IsVisible() )
        continue;
      vec3f p1 = (bn.Another(sa).crd() + SceneOrigin)*ProjMatr+DrawOrigin;
      if( p1[2] <= oa.crd[2] )  // goes into the plane - drawn
        continue; 
      dir_vec = (p1-oa.crd).Normalise();
      const float pers_scale = 1.0-olx_sqr(dir_vec[2]);
      float brad = (bn.A().GetAtomInfo() < 4 || bn.B().GetAtomInfo() < 4) ? 
        BondRad*HBondScale : BondRad;
      if( bn.GetType() == sotHBond )  //even thiner H-bonds
        brad /= 4;
      brad *= app.GetBond(bn.GetTag()).GetRadius();
      touch_point = (bn.Another(sa).crd()-oa.atom->crd()).Normalise();
      vec3f rot_vec(-touch_point[1], touch_point[0], 0);
      CreateRotationMatrix(rot_mat, rot_vec.Normalise(), touch_point[2]);
      proj_mat = rot_mat*ProjMatr;
      const float b_len = (p1-oa.crd).Length();
      if( sa.GetEllipsoid() != NULL )  {
        bproj_cnt.Null();
        touch_point_proj = dir_vec*(*oa.ielpm);
        for( uint16_t j=0; j < BondDiv; j++ )  {
          BondProjF[j] = (((BondCrd[j]*rot_mat+touch_point)*ProjMatr).Normalise()*(*oa.ielpm));
          bproj_cnt += BondProjF[j];
        }
        bproj_cnt /= BondDiv;
        for( uint16_t j=0; j < BondDiv; j++ )  {
          BondProjF[j] = (BondProjF[j]-bproj_cnt).NormaliseTo(brad*(1+pers_scale)*scalex) + bproj_cnt;
          BondProjT[j] = (BondCrd[j]*proj_mat).NormaliseTo(brad*2*scalex) + dir_vec*b_len;
        }
      }
      else  {
        const float off_len = oa.draw_rad/2;
        for( uint16_t j=0; j < BondDiv; j++ )  {
          BondProjT[j] = BondProjF[j] = (BondCrd[j]*proj_mat).NormaliseTo(brad*(1+pers_scale)*scalex); 
          BondProjT[j].NormaliseTo(brad*2*scalex) += dir_vec*b_len;
          BondProjF[j] += dir_vec*off_len;
        }
      }
      if( bn.GetType() == sotHBond )
        pw.drawQuads(BondProjF, BondProjT, 16, &PSWriter::fill);
      else
        pw.drawQuads(BondProjF, BondProjT, &PSWriter::fill);
    }
  }
  void DrawBonds(PSWriter& pw, const OrtAtom& oa) const  {
    DoDrawBonds(pw, oa, ~0, 1.2);
    DoDrawBonds(pw, oa, 0, 1);
  }
  void DrawRimsAndQuad(PSWriter& pw, const OrtAtom& oa, bool drawQuad)  {
    pw.lineWidth(QuadLineWidth);
    const mat3f& elpm = *oa.elpm;
    const mat3f& ielpm = *oa.ielpm;
    mat3f pelpm(elpm[0][2] < 0 ? -elpm[0] : elpm[0],
      elpm[1][2] < 0 ? -elpm[1] : elpm[1],
      elpm[2][2] < 0 ? -elpm[2] : elpm[2]);
    vec3f norm_vec = (ElpCrd[0]*ielpm).XProdVec(ElpCrd[1]*ielpm);
    if( norm_vec[2] < 0 )
      norm_vec *= -1;
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

    if( drawQuad )  {
      pw.drawLine(NullVec, pelpm[0]);
      pw.drawLine(NullVec, pelpm[1]);
      pw.drawLine(NullVec, pelpm[2]);
      for( uint16_t j=0; j < PieDiv; j++ )
        pw.drawLine( PieCrd[j]*pelpm, pelpm[0]*((float)(PieDiv-j)/PieDiv));
      for( uint16_t j=0; j < PieDiv; j++ )
        pw.drawLine( vec3f(0, PieCrd[j][0], PieCrd[j][1])*pelpm, pelpm[1]*((float)(PieDiv-j)/PieDiv));
      for( uint16_t j=0; j < PieDiv; j++ )
        pw.drawLine( vec3f(PieCrd[j][1], 0, PieCrd[j][0])*pelpm, pelpm[2]*((float)(PieDiv-j)/PieDiv));
    }
  }
public:
  OrtDraw() : app(TGXApp::GetInstance()) {  
    ElpDiv = 36;
    BondDiv = 12;
    PieDiv = 4;
    QuadLineWidth = PieLineWidth = 0.5;
    BondRad = 1;
    ColorMode = ortep_color_None;
    HBondScale = 0.5;
  }
  // create ellipse and pie coordinates
  void Init(PSWriter& pw)  {
    ElpCrd.SetCount(ElpDiv);
    Arc.SetCount(ElpDiv);
    PieCrd.SetCount(PieDiv);
    FilteredArc.SetCount(ElpDiv);
    BondCrd.SetCount(BondDiv);
    BondProjF.SetCount(BondDiv);
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
      olxcstr fnt("/Verdana findfont ");
      fnt << olx_round(app.GetLabel(0).GetFont().GetPointSize()/LinearScale) << " scalefont setfont";
      pw.custom(fnt.c_str());
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
  void Render(const olxstr& fileName)  {
    PSWriter pw(fileName);
    Init(pw);
    const TEBasis& basis = app.GetRender().GetBasis();
    TTypeList<OrtDraw::OrtAtom> atoms;
    atoms.SetCapacity(app.AtomCount());
    for( size_t i=0; i < app.AtomCount(); i++ )  {
      if( app.GetAtom(i).IsDeleted() || !app.GetAtom(i).IsVisible() )
        continue;
      const TSAtom& sa = app.GetAtom(i).Atom();
      app.GetAtom(i).SetTag(i);
      app.GetAtom(i).Atom().SetTag(i);
      if( sa.GetEllipsoid() != NULL )  {
        mat3f& elpm = *(new mat3f(sa.GetEllipsoid()->GetMatrix()));
        elpm[0] *= sa.GetEllipsoid()->GetSX();
        elpm[1] *= sa.GetEllipsoid()->GetSY();
        elpm[2] *= sa.GetEllipsoid()->GetSZ();
        elpm *= app.GetAtom(i).GetDrawScale();
        elpm *= ProjMatr;
        mat3f& ielpm = *(new mat3f( (sa.GetEllipsoid()->GetMatrix() * basis.GetMatrix() ).Inverse()) );
        atoms.AddNew(&sa, (sa.crd() + SceneOrigin)*ProjMatr+DrawOrigin, 1.0, &elpm, &(ielpm *= elpm));
      }
      else
        atoms.AddNew(&sa, (sa.crd() + SceneOrigin)*ProjMatr+DrawOrigin, app.GetAtom(i).GetDrawScale()*DrawScale);
    }
    for( size_t i=0; i < app.BondCount(); i++ )  {
      app.GetBond(i).SetTag(i);
      app.GetBond(i).Bond().SetTag(i);
    }
    atoms.QuickSorter.SortSF(atoms, OrtAtomZSort);

    for( size_t i=0; i < atoms.Count(); i++ )  {
      const TSAtom& sa = *atoms[i].atom;
      const vec3d& p = atoms[i].crd;
      pw.translate(p);
      pw.lineWidth(0.5);
      pw.color(~0);
      if( sa.GetEllipsoid() == NULL )  {
        pw.drawCircle(NullVec, atoms[i].draw_rad*1.05, &PSWriter::fill);
        if( ColorMode == ortep_color_None )  {
          pw.color(0);
          pw.drawCircle(NullVec, atoms[i].draw_rad);
        }
        else if( ColorMode == ortep_color_Fill )  {
          pw.newPath();
          pw.circle(NullVec, atoms[i].draw_rad);
          pw.gsave();
          pw.color(sa.GetAtomInfo().GetDefColor());
          pw.fill();
          pw.grestore();
          pw.color(0);
          pw.stroke();
        }
        else if( ColorMode == ortep_color_Lines )  {
          pw.color(  sa.GetAtomInfo().GetDefColor() == 0xffffff ? 0 : sa.GetAtomInfo().GetDefColor() );
          pw.drawCircle(NullVec, atoms[i].draw_rad);
        }
      }
      else  {
        const mat3f& ielpm = *atoms[i].ielpm;
        pw.drawEllipse(NullVec, ielpm*1.05, &PSWriter::fill);
        if( ColorMode == ortep_color_None )  {
          pw.color(0);
          pw.drawEllipse(NullVec, ielpm);
        }
        else if( ColorMode == ortep_color_Fill )  {
          pw.newPath();
          pw.ellipse(NullVec, ielpm);
          pw.gsave();
          pw.color(sa.GetAtomInfo().GetDefColor());
          pw.fill();
          pw.grestore();
          pw.color(0);
          pw.stroke();
        }
        else if( ColorMode == ortep_color_Lines )  {
          pw.color(  sa.GetAtomInfo().GetDefColor() == 0xffffff ? 0 : sa.GetAtomInfo().GetDefColor() );
          pw.drawEllipse(NullVec, ielpm);
        }
        DrawRimsAndQuad(pw, atoms[i], sa.GetAtomInfo() != iCarbonIndex);
      }
      DrawBonds(pw, atoms[i]);
      pw.translate(-p);
    }
    if( app.LabelCount() != 0 )  {
      for( size_t i=0; i < app.LabelCount(); i++ )  {
        const TXGlLabel& glxl = app.GetLabel(i);
        vec3d rp = glxl.GetRasterPosition();
        rp[1] += 4;
        pw.drawText(glxl.GetLabel(), rp+DrawOrigin);
      }
    }
  }

  DefPropP(uint16_t, ElpDiv)
  DefPropP(uint16_t, PieDiv) 
  DefPropP(uint16_t, BondDiv) 
  DefPropP(short, ColorMode)
  DefPropP(float, HBondScale)
  DefPropP(float, BondRad)
};


#endif
