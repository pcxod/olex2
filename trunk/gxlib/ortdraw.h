#ifndef __olx_ort_Draw_H
#define __olx_ort_Draw_H
#include "gxapp.h"
#include "ps_writer.h"

class OrtDraw  {
private:
  struct OrtAtom {
    const TSAtom* atom;
    vec3f crd;
    mat3f *elpm, *ielpm, *raw_elpm;
    OrtAtom(const TSAtom* a, const vec3f& c, mat3f* em = NULL, mat3f* iem = NULL, mat3f* rem=NULL) :
      atom(a), crd(c), elpm(em), ielpm(iem), raw_elpm(rem) {  }
    OrtAtom(const OrtAtom& a) :
      atom(a.atom), crd(a.crd), elpm(a.elpm), ielpm(a.ielpm), raw_elpm(a.raw_elpm) {  }
    ~OrtAtom()  {
      if( elpm != NULL )   
        delete elpm;
      if( ielpm != NULL )  
        delete ielpm;
      if( raw_elpm != NULL )  
        delete raw_elpm;
    }
  };
  static int OrtAtomZSort(const OrtAtom& a1, const OrtAtom& a2)  {
    const float diff = a1.crd[2] - a2.crd[2];
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
  float AradScale, BradScale, DrawScale;
  mat3f ProjMatr, UnProjMatr;
  vec3f DrawOrigin, SceneOrigin;
protected:
  short ElpDiv, PieDiv;
  TArrayList<vec3f> ElpCrd, PieCrd, Arc;
  TPtrList<const vec3f> FilteredArc;
  float PieLineWidth, ElpLineWidth, QuadLineWidth, BondLineWidth;

  int PrepareArc(const TArrayList<vec3f>& in, TPtrList<const vec3f>& out, const vec3f& normal)  {
    int start=-1, cnt=0;
    for( int i=0; i < in.Count(); i++ )  {
      const int next_i = ((i+1) >= in.Count() ? i-in.Count() : i) + 1;
      if( in[i].DotProd(normal) < 0 && in[next_i].DotProd(normal) >= 0 )  {
        start = next_i;
        break;
      }
    }
    if( start == -1 )
      return 0;
    for( int i=start; i < start+in.Count(); i++ )  {
      const int ind = (i >= in.Count() ? i-in.Count() : i);
      if( cnt+1 >= in.Count() )  break;
      if( in[ind].DotProd(normal) >= 0  )
        out[cnt++] = &in[ind];
      else
        break;
    }
    return cnt;
  }
  void DrawBottomBonds(PSWriter& pw, const OrtAtom& oa) const  {
    pw.lineWidth(BondLineWidth);
    pw.color(0);
    const TSAtom& sa = *oa.atom;
    for( int j=0; j < sa.BondCount(); j++ )  {
      const TSBond& bn = sa.Bond(j);
      vec3f p1 = (bn.Another(sa).crd() + SceneOrigin)*ProjMatr+DrawOrigin;
      if( p1[2] >= oa.crd[2] )  // goes out of the plane - will be drawn
        continue; 
      float scale_f = (sa.GetAtomInfo().GetMr() < 4 || bn.Another(sa).GetAtomInfo().GetMr() < 4 ) ? 30 : 10;
      vec3f p2,
        dir_vec( (p1-oa.crd).Normalise() ),
        off( vec3f(-dir_vec[1], dir_vec[0], 0).NormaliseTo(BradScale/scale_f) );
      vec3f touch_point;
      if( sa.GetEllipsoid() != NULL )
        touch_point = (bn.Another(sa).crd()-oa.atom->crd()).Normalise()*(*oa.elpm);
      float pers_scale = sqrt(1.0-sqr(dir_vec[2]))/2;
      if( sa.GetEllipsoid() == NULL )
        p2 = dir_vec*AradScale*sa.GetAtomInfo().GetRad1();
      else  {
        p2 = dir_vec*touch_point.Length();
      }
      pw.newPath();
      for( int k=-5; k <= 5; k++ )
        pw.line(oa.crd + p2 + off*k*2, (p1+oa.crd)/2 + off*k*(3+pers_scale)/2);
      pw.stroke();
    }
  }
  void DrawUpperBonds(PSWriter& pw, const OrtAtom& oa) const  {
    pw.lineWidth(BondLineWidth);
    pw.color(0);
    const TSAtom& sa = *oa.atom;
    for( int j=0; j < sa.BondCount(); j++ )  {
      const TSBond& bn = sa.Bond(j);
      vec3f p1 = (bn.Another(sa).crd() + SceneOrigin)*ProjMatr+DrawOrigin;
      if( p1[2] < oa.crd[2] )  // goes into the plane - drawn
        continue; 
      float scale_f = (sa.GetAtomInfo().GetMr() < 4 || bn.Another(sa).GetAtomInfo().GetMr() < 4 ) ? 30 : 10;
      vec3f p2,
        dir_vec( (p1-oa.crd).Normalise() ),
        off( vec3f(-dir_vec[1], dir_vec[0], 0).NormaliseTo(BradScale/scale_f) );
      vec3f touch_point;
      if( sa.GetEllipsoid() != NULL )
        touch_point = (bn.Another(sa).crd()-oa.atom->crd()).Normalise()*(*oa.elpm);
      off[2] = touch_point[2];
      float pers_scale = sqrt(1.0-sqr(dir_vec[2]))/2;
      pw.newPath();
      for( int k=-5; k <= 5; k++ )  {
        if( sa.GetEllipsoid() == NULL )
          p2 = dir_vec*AradScale*sa.GetAtomInfo().GetRad1();
        else  {
          //vec3f p3 = (off*(k*(1+pers_scale))-p2)*UnProjMatr;
          //p2 = dir_vec*(p3.Normalise()*(*oa.elpm)).Length();
          p2 = dir_vec*touch_point.Length();
        }
        pw.line(oa.crd + p2 + off*k*(1+pers_scale), (p1+oa.crd)/2 + off*k*(3+pers_scale)/2);
      }
      pw.stroke();
    }
  }
  void DrawRimsAndQuad(PSWriter& pw, const OrtAtom& oa)  {
    pw.lineWidth(QuadLineWidth);
    pw.translate(oa.crd);
    static const vec3f center(0,0,0);
    const mat3f& elpm = *oa.elpm;
    const mat3f& ielpm = *oa.ielpm;
    mat3f pelpm(elpm[0][2] < 0 ? -elpm[0] : elpm[0],
      elpm[1][2] < 0 ? -elpm[1] : elpm[1],
      elpm[2][2] < 0 ? -elpm[2] : elpm[2]);
    vec3f norm_vec = (ElpCrd[0]*ielpm).XProdVec(ElpCrd[ElpDiv/2]*ielpm);
    if( norm_vec[2] < 0 )
      norm_vec *= -1;
    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = ElpCrd[j]*elpm;
    int pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
    //pw.color(0x0000ff);
    pw.drawLines_vp(FilteredArc, pts_cnt, false);
    pw.drawLine(center, pelpm[0]);

    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = vec3f(ElpCrd[j][1], 0, ElpCrd[j][0])*elpm;
    pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
    //pw.color(0x00ff00);
    pw.drawLines_vp(FilteredArc, pts_cnt, false);
    pw.drawLine(center, pelpm[1]);

    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = vec3f(0, ElpCrd[j][0], ElpCrd[j][1])*elpm;
    pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
    //pw.color(0xff0000);
    pw.drawLines_vp(FilteredArc, pts_cnt, false);
    pw.drawLine(center, pelpm[2]);

    //pw.color(0);

    for( int j=0; j < PieDiv; j++ )
      pw.drawLine( PieCrd[j]*pelpm, pelpm[0]*((float)(PieDiv-j)/PieDiv));
    for( int j=0; j < PieDiv; j++ )
      pw.drawLine( vec3f(0, PieCrd[j][0], PieCrd[j][1])*pelpm, pelpm[1]*((float)(PieDiv-j)/PieDiv));
    for( int j=0; j < PieDiv; j++ )
      pw.drawLine( vec3f(PieCrd[j][1], 0, PieCrd[j][0])*pelpm, pelpm[2]*((float)(PieDiv-j)/PieDiv));
    pw.translate(-oa.crd);
  }
public:
  OrtDraw() {  
    ElpDiv = 36;
    PieDiv = 4;
    QuadLineWidth = PieLineWidth = 0.5;
    BondLineWidth = 1;
  }
  // create ellipse and pie coordinates
  void Init(const PSWriter& pw)  {
    ElpCrd.SetCount(ElpDiv);
    PieCrd.SetCount(PieDiv);
    Arc.SetCount(ElpDiv);
    FilteredArc.SetCount(ElpDiv);
    double sin_a, cos_a;
    SinCos(2*M_PI/ElpDiv, &sin_a, &cos_a);
    vec3f ps(cos_a, -sin_a, 0);
    for( int i=0; i < ElpDiv; i++ )  {
      ElpCrd[i] = ps;
      float x = ps[0];
      ps[0] = (float)(cos_a*x + sin_a*ps[1]);
      ps[1] = (float)(cos_a*ps[1] - sin_a*x);
    }
    ps = vec3f(1, 0, 0);
    for( int i=0; i < PieDiv; i++ )  {
      PieCrd[i] = ps;
      ps[0] = (float)(PieDiv-i-1)/PieDiv;
      ps[1] = (float)sqrt(1.0-ps[0]*ps[0]);
    }
    float vp[4];
    glGetFloatv(GL_VIEWPORT, vp);
    TGXApp& app = TGXApp::GetInstance();
    const TEBasis& basis = app.GetRender().GetBasis();
    DrawScale = olx_min((float)pw.GetWidth()/vp[2], (double)pw.GetHeight()/vp[3]) /app.GetRender().GetScale();
    AradScale = 0.25/app.GetRender().GetScale(),
    BradScale = 0.05/app.GetRender().GetScale();
    SceneOrigin = basis.GetCenter();
    DrawOrigin = vec3f(pw.GetWidth()/2, pw.GetHeight()/2, 0);
    ProjMatr = basis.GetMatrix()*DrawScale;  
    UnProjMatr = ProjMatr.Inverse();
  }
  void Render(const olxstr& fileName)  {
    PSWriter pw(fileName);
    pw.custom("/Helvetica findfont 12 scalefont setfont");
    Init(pw);
    TGXApp& app = TGXApp::GetInstance();
    const TEBasis& basis = app.GetRender().GetBasis();
    TTypeList<OrtDraw::OrtAtom> atoms;
    atoms.SetCapacity(app.AtomCount());
    for( int i=0; i < app.AtomCount(); i++ )  {
      const TSAtom& sa = app.GetAtom(i).Atom();
      if( sa.GetEllipsoid() != NULL )  {
        mat3f& raw_elpm = *(new mat3f(sa.GetEllipsoid()->GetMatrix()) );
        raw_elpm[0] *= sa.GetEllipsoid()->GetSX();
        raw_elpm[1] *= sa.GetEllipsoid()->GetSY();
        raw_elpm[2] *= sa.GetEllipsoid()->GetSZ();
        raw_elpm *= TXAtom::TelpProb();
        mat3f& elpm = *(new mat3f(raw_elpm * ProjMatr));
        mat3f& ielpm = *(new mat3f( (sa.GetEllipsoid()->GetMatrix() * basis.GetMatrix() ).Inverse()) );
        atoms.AddNew(&sa, (sa.crd() + SceneOrigin)*ProjMatr+DrawOrigin, &elpm, &(ielpm *= elpm), &raw_elpm);
      }
      else
        atoms.AddNew(&sa, (sa.crd() + SceneOrigin)*ProjMatr+DrawOrigin);
    }
    atoms.QuickSorter.SortSF(atoms, OrtAtomZSort);

    for( int i=0; i < atoms.Count(); i++ )  {
      const TSAtom& sa = *atoms[i].atom;
      const vec3d& p = atoms[i].crd;
      DrawBottomBonds(pw, atoms[i]);
      //pw.color( sa.GetAtomInfo().GetDefColor() != ~1 ? sa.GetAtomInfo().GetDefColor() : 0 );
      pw.lineWidth(0.5);
      pw.color(~0);
      if( sa.GetEllipsoid() == NULL )  {
        pw.newPath();
        pw.circle(p, AradScale*sa.GetAtomInfo().GetRad1()*1.05);
        pw.fill();
        pw.color(0);
        pw.drawCircle(p, AradScale*sa.GetAtomInfo().GetRad1());
      }
      else  {
        const mat3f& elpm = *atoms[i].elpm;
        const mat3f& ielpm = *atoms[i].ielpm;
        pw.newPath();
        pw.ellipse(p, ielpm*1.05);
        pw.fill();
        pw.color(0);
        pw.drawEllipse(p, ielpm);
        DrawRimsAndQuad(pw, atoms[i]);
      }
      DrawUpperBonds(pw, atoms[i]);
    }
    for( int i=0; i < app.LabelCount(); i++ )  {
      const TXGlLabel& glxl = app.GetLabel(i);
      vec3d rp = glxl.GetRasterPosition();
      rp *= (DrawScale*app.GetRender().GetScale());
      pw.drawText(glxl.GetLabel(), rp+DrawOrigin);
    }
  }

  DefPropP(short, ElpDiv)
  DefPropP(short, PieDiv) 

};


#endif
