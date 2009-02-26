#ifndef __olx_ort_Draw_H
#define __olx_ort_Draw_H
#include "gxapp.h"
#include "ps_writer.h"

class OrtDraw  {
private:
  struct OrtAtom {
    const TSAtom* atom;
    vec3f crd;
    mat3f *elpm, *ielpm;
    OrtAtom(const TSAtom* a, const vec3f& c, mat3f* em = NULL, mat3f* iem = NULL) :
      atom(a), crd(c), elpm(em), ielpm(iem) {  }
    OrtAtom(const OrtAtom& a) :
      atom(a.atom), crd(a.crd), elpm(a.elpm), ielpm(a.ielpm) {  }
    ~OrtAtom()  {
      if( elpm != NULL )   
        delete elpm;
      if( ielpm != NULL )  
        delete ielpm;
    }
  };
  static int OrtAtomZSort(const OrtAtom& a1, const OrtAtom& a2)  {
    const float diff = a1.crd[2] - a2.crd[2];
    return diff < 0 ? -1 : (diff > 0 ? 1 : 0);
  }
  float AradScale, BradScale, DrawScale, BondRad;
  mat3f ProjMatr, UnProjMatr;
  vec3f DrawOrigin, SceneOrigin;
protected:
  short ElpDiv, PieDiv, BondDiv;
  TArrayList<vec3f> ElpCrd, PieCrd, Arc, BondCrd, Bond;
  TPtrList<const vec3f> FilteredArc;
  float PieLineWidth, 
    ElpLineWidth, 
    QuadLineWidth, 
    BondLineWidth;

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
  void DrawBonds(PSWriter& pw, const OrtAtom& oa) const  {
    pw.lineWidth(BondLineWidth);
    pw.color(0);
    const TSAtom& sa = *oa.atom;
    for( int j=0; j < sa.BondCount(); j++ )  {
      const TSBond& bn = sa.Bond(j);
      vec3f p1 = (bn.Another(sa).crd() + SceneOrigin)*ProjMatr+DrawOrigin;
      if( p1[2] <= oa.crd[2] )  // goes into the plane - drawn
        continue; 
      const vec3f dir_vec( (p1-oa.crd).Normalise() );
      const float pers_scale = sqrt(1.0-sqr(dir_vec[2]));
      float brad = (bn.GetA().GetAtomInfo() < 4 || bn.GetB().GetAtomInfo() < 4) ? 
        BondRad/2 : BondRad;
      vec3f touch_point = (bn.Another(sa).crd()-oa.atom->crd()).Normalise();
      mat3f proj_mat, rot_mat;
      vec3f rot_vec(-touch_point[1], touch_point[0], 0), off_vec;
      CreateRotationMatrix(rot_mat, rot_vec.Normalise(), touch_point[2]);
      proj_mat = rot_mat*ProjMatr;
      float b_len = (p1-oa.crd).Length();
      pw.translate(oa.crd); 
      float off_len = (sa.GetEllipsoid() == NULL) ? AradScale*sa.GetAtomInfo().GetRad1()/2 : 
        (dir_vec*(*oa.ielpm)).Length();
      pw.newPath();
      for( int j=0; j < BondDiv; j++ )  {
        Bond[j] = BondCrd[j]*proj_mat;
        vec3f v1 = Bond[j].NormaliseTo(brad*(1+pers_scale));
        vec3f v2 = Bond[j].NormaliseTo(brad*2);
        if( sa.GetEllipsoid() != NULL )  {
          //vec3f v3 = ((BondCrd[j]*rot_mat+touch_point)*(*oa.elpm));//.NormaliseTo(brad*(1+pers_scale));
          vec3f v3 = (((BondCrd[j]*rot_mat+touch_point)*ProjMatr).Normalise()*(*oa.ielpm));//.NormaliseTo(brad*(1+pers_scale));
          off_vec = dir_vec*v3.Length()*0.95;
        }
        else
          off_vec = dir_vec*off_len;
        pw.line(v1 + off_vec, v2+dir_vec*b_len);
      }
      pw.stroke();
      //pw.drawLines(Bond, BondDiv, true);
      pw.translate(-oa.crd);      
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
    vec3f norm_vec = (ElpCrd[0]*ielpm).XProdVec(ElpCrd[1]*ielpm);
    if( norm_vec[2] < 0 )
      norm_vec *= -1;
    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = ElpCrd[j]*pelpm;
    int pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
    //pw.color(0x0000ff);
    pw.drawLines_vp(FilteredArc, pts_cnt, false);
    pw.drawLine(center, pelpm[0]);

    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = vec3f(ElpCrd[j][1], 0, ElpCrd[j][0])*pelpm;
    pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
    //pw.color(0x00ff00);
    pw.drawLines_vp(FilteredArc, pts_cnt, false);
    pw.drawLine(center, pelpm[1]);

    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = vec3f(0, ElpCrd[j][0], ElpCrd[j][1])*pelpm;
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
    BondDiv = 24;
    PieDiv = 4;
    QuadLineWidth = PieLineWidth = 0.5;
    BondLineWidth = 0.75;
    BondRad = 1.5;
  }
  // create ellipse and pie coordinates
  void Init(const PSWriter& pw)  {
    ElpCrd.SetCount(ElpDiv);
    Arc.SetCount(ElpDiv);
    PieCrd.SetCount(PieDiv);
    FilteredArc.SetCount(ElpDiv);
    BondCrd.SetCount(BondDiv);
    Bond.SetCount(BondDiv);
    double sin_a, cos_a;
    SinCos(2*M_PI/ElpDiv, &sin_a, &cos_a);
    vec3f ps(cos_a, -sin_a, 0);
    for( int i=0; i < ElpDiv; i++ )  {
      ElpCrd[i] = ps;
      const float x = ps[0];
      ps[0] = (float)(cos_a*x + sin_a*ps[1]);
      ps[1] = (float)(cos_a*ps[1] - sin_a*x);
    }
    SinCos(2*M_PI/BondDiv, &sin_a, &cos_a);
    ps = vec3f(cos_a/2, -sin_a/2, 0);
    for( int i=0; i < BondDiv; i++ )  {
      BondCrd[i] = ps;
      const float x = ps[0];
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
        mat3f& elpm = *(new mat3f(sa.GetEllipsoid()->GetMatrix()) );
        elpm[0] *= sa.GetEllipsoid()->GetSX();
        elpm[1] *= sa.GetEllipsoid()->GetSY();
        elpm[2] *= sa.GetEllipsoid()->GetSZ();
        elpm *= TXAtom::TelpProb();
        elpm *= ProjMatr;
        mat3f& ielpm = *(new mat3f( (sa.GetEllipsoid()->GetMatrix() * basis.GetMatrix() ).Inverse()) );
        atoms.AddNew(&sa, (sa.crd() + SceneOrigin)*ProjMatr+DrawOrigin, &elpm, &(ielpm *= elpm));
      }
      else
        atoms.AddNew(&sa, (sa.crd() + SceneOrigin)*ProjMatr+DrawOrigin);
    }
    atoms.QuickSorter.SortSF(atoms, OrtAtomZSort);

    for( int i=0; i < atoms.Count(); i++ )  {
      const TSAtom& sa = *atoms[i].atom;
      const vec3d& p = atoms[i].crd;
      //DrawBottomBonds(pw, atoms[i]);
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
      //DrawUpperBonds(pw, atoms[i]);
      DrawBonds(pw, atoms[i]);
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
