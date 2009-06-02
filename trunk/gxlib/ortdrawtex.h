#ifndef __olx_ort_Draw_Tex_H
#define __olx_ort_Draw_Tex_H
#include "gxapp.h"
#include "ps_writer.h"
#include <math.h>

  
class OrtDrawTex  {
      TEFile test;
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
  float AradScale, DrawScale, BondRad, LinearScale;
  mat3f ProjMatr, UnProjMatr;
  vec3f DrawOrigin, SceneOrigin;
  TGXApp& app;
  const vec3f NullVec;
  short ColorMode;
protected:
  short ElpDiv, PieDiv, BondDiv;
  TArrayList<vec3f> ElpCrd, PieCrd, Arc, BondCrd, BondProjF, BondProjT;
  TPtrList<const vec3f> FilteredArc;
  float PieLineWidth, 
    ElpLineWidth, 
    QuadLineWidth,
    HBondScale;

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
  void DoDrawBonds(PSWriter& pw, const OrtAtom& oa, uint32_t color, float scalex, const vec3d& p)  {
    pw.color(color);
                              char bf[250]; //pascal
      float xa, xb, ya, yb, angle, xc, yc, xd, yd, xe, ye, xf, yf;
    const TSAtom& sa = *oa.atom;
    vec3f dir_vec, touch_point, touch_point_proj, off_vec, bproj_cnt;
    mat3f proj_mat, rot_mat;
    for( int j=0; j < sa.BondCount(); j++ )  {
      const TSBond& bn = sa.Bond(j);
      //const TSBond atom1 = app.GetBond( bn.GetTag() );
      //char Atom1Symbol = bn.A().GetAtomInfo().GetSymbol().c_str();
      //char Atom2Symbol = bn.B().GetAtomInfo().GetSymbol().c_str();
      //const char atom2 = bn.FB.GetAtomInfo().GetSymbol().c_str();
      if( app.GetBond( bn.GetTag() ).IsDeleted() || !app.GetBond( bn.GetTag() ).IsVisible() )
        continue;
      vec3f p1 = (bn.Another(sa).crd() + SceneOrigin)*ProjMatr+DrawOrigin;
      if( p1[2] <= oa.crd[2] )  // goes into the plane - drawn
        continue; 
      dir_vec = (p1-oa.crd).Normalise();
      const float pers_scale = 1.0-sqr(dir_vec[2]);
      float brad = (bn.A().GetAtomInfo() < 4 || bn.B().GetAtomInfo() < 4) ? 
        BondRad*HBondScale : BondRad;
      if( bn.GetType() == sotHBond )  //even thiner H-bonds
        brad *= HBondScale;
      touch_point = (bn.Another(sa).crd()-oa.atom->crd()).Normalise();
      vec3f rot_vec(-touch_point[1], touch_point[0], 0);
      CreateRotationMatrix(rot_mat, rot_vec.Normalise(), touch_point[2]);
      proj_mat = rot_mat*ProjMatr;
      const float b_len = (p1-oa.crd).Length();
      if( sa.GetEllipsoid() != NULL )  {
        bproj_cnt.Null();
        touch_point_proj = dir_vec*(*oa.ielpm);
        for( int j=0; j < BondDiv; j++ )  {
          BondProjF[j] = (((BondCrd[j]*rot_mat+touch_point)*ProjMatr).Normalise()*(*oa.ielpm));
          bproj_cnt += BondProjF[j];
        }
        bproj_cnt /= BondDiv;
        for( int j=0; j < BondDiv; j++ )  {
          BondProjF[j] = (BondProjF[j]-bproj_cnt).NormaliseTo(brad*(1+pers_scale)*scalex) + bproj_cnt;
          BondProjT[j] = (BondCrd[j]*proj_mat).NormaliseTo(brad*2*scalex) + dir_vec*b_len;
        }
      }
      else  {
        const float off_len = AradScale*sa.GetAtomInfo().GetRad1()/2;
        for( int j=0; j < BondDiv; j++ )  {
          BondProjT[j] = BondProjF[j] = (BondCrd[j]*proj_mat).NormaliseTo(brad*(1+pers_scale)*scalex); 
          BondProjT[j].NormaliseTo(brad*2*scalex) += dir_vec*b_len;
          BondProjF[j] += dir_vec*off_len;
        }
      }
      if( bn.GetType() == sotHBond )
        pw.drawQuads(BondProjF, BondProjT, 16, &PSWriter::fill);
      else
      {
        pw.drawQuads(BondProjF, BondProjT, &PSWriter::fill);
        if( BondProjF.Count() != BondProjT.Count() )
          throw TFunctionFailedException(__OlxSourceInfo, "lists mismatch");
        if( BondProjF.Count() > 2 )
        {
            //Calculation of the centre of the intersection ellipsis    
            xa = p[0]+(BondProjT[1][0]+BondProjT[2][0]+BondProjT[3][0]+BondProjT[4][0]+BondProjT[5][0]+BondProjT[6][0]+BondProjT[7][0]+BondProjT[8][0]+BondProjT[9][0]+BondProjT[10][0]+BondProjT[11][0]+BondProjT[12][0])/12;
            ya = p[1]+(BondProjT[1][1]+BondProjT[2][1]+BondProjT[3][1]+BondProjT[4][1]+BondProjT[5][1]+BondProjT[6][1]+BondProjT[7][1]+BondProjT[8][1]+BondProjT[9][1]+BondProjT[10][1]+BondProjT[11][1]+BondProjT[12][1])/12;
            xb = p[0]+(BondProjF[1][0]+BondProjF[2][0]+BondProjF[3][0]+BondProjF[4][0]+BondProjF[5][0]+BondProjF[6][0]+BondProjF[7][0]+BondProjF[8][0]+BondProjF[9][0]+BondProjF[10][0]+BondProjF[11][0]+BondProjF[12][0])/12;
            yb = p[1]+(BondProjF[1][1]+BondProjF[2][1]+BondProjF[3][1]+BondProjF[4][1]+BondProjF[5][1]+BondProjF[6][1]+BondProjF[7][1]+BondProjF[8][1]+BondProjF[9][1]+BondProjF[10][1]+BondProjF[11][1]+BondProjF[12][1])/12;
            
            //calculation of the angle between the bond and the vertical. Needed to applys a shading on the bond
            if(xb>xa && (xb-xa)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya))<1 && (xb-xa)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya))>-1) 
            {
                angle = asin((xb-xa)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya)))*180/M_PI;
                if(ya<yb)
                    angle = 180 - asin((xb-xa)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya)))*180/M_PI;
            }
            else if(xa>xb && (xa-xb)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya))<1 && (xa-xb)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya))>-1) 
            {
                angle = - asin((xa-xb)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya)))*180/M_PI;
                if(ya<yb)
                    angle= asin((xa-xb)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya)))*180/M_PI;
            }
            else
            {
                angle = 360;
            }
                    
            //Front coordinate of the rectangle for the bond
            yc = 0.08*DrawScale*(xa-xb)/sqrt(xa*xa-2*xa*xb+xb*xb+(ya-yb)*(ya-yb))+ya;
            xc = -(yb-ya)*(yc-ya)/(xb-xa)+xa;
            ye = ya-(yc-ya);
            xe = xa-(xc-xa);

            //Back coordinate of the recangle for the bond
            yd = 0.055*DrawScale*(xa-xb)/sqrt(xa*xa-2*xa*xb+xb*xb+(ya-yb)*(ya-yb))+yb;
            xd = -(ya-yb)*(yd-yb)/(xa-xb)+xb;
            yf = yb-(yd-yb);
            xf = xb-(xd-xb);

            //white perimeter around the bond
            sprintf(bf, "\\draw[white,very thick, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
                xc/5,yc/5,
                xe/5, ye/5,
                xf/5, yf/5, 
                xd/5, yd/5);
            test.Writenl(bf);
            
            //Fix me!!! Assign the right color to each end of the bond
            //A is the heaviest atom
            //draw the shading
            if(((bn.A().crd() + SceneOrigin)*ProjMatr+DrawOrigin)[2]-((bn.B().crd() + SceneOrigin)*ProjMatr+DrawOrigin)[2] >0)
            {
                sprintf(bf, "\\shade[top color=\\color%s, bottom color=\\color%s, shading angle=%f, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
                    bn.B().GetAtomInfo().GetSymbol().c_str(), bn.A().GetAtomInfo().GetSymbol().c_str(), angle,
                    xc/5,yc/5,
                    xe/5, ye/5,
                    xf/5, yf/5, 
                    xd/5, yd/5);
            }
            else
            {
                sprintf(bf, "\\shade[top color=\\color%s, bottom color=\\color%s, shading angle=%f, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
                    bn.A().GetAtomInfo().GetSymbol().c_str(), bn.B().GetAtomInfo().GetSymbol().c_str(), angle,
                    xc/5,yc/5,
                    xe/5, ye/5,
                    xf/5, yf/5, 
                    xd/5, yd/5);
            }                                    
            test.Writenl(bf);
            
            //Draw the black stroke around the bond
            sprintf(bf, "\\draw[rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
                xc/5,yc/5,
                xe/5, ye/5,
                xf/5, yf/5, 
                xd/5, yd/5);
            test.Writenl(bf);
                    
        }
      }
    }
  }
  void DrawBonds(PSWriter& pw, const OrtAtom& oa, const vec3d& p) {
    //DoDrawBonds(pw, oa, ~0, 1.2, p);
    DoDrawBonds(pw, oa, 0, 1, p);
  }
  void DrawRimsAndQuad(PSWriter& pw, const OrtAtom& oa, bool drawQuad)  {
    pw.lineWidth(QuadLineWidth);
    const mat3f& elpm = *oa.elpm;
    const mat3f& ielpm = *oa.ielpm;
    char bf[200];
    //pelpm - projection ellipsoid matrix
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
    
    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = vec3f(ElpCrd[j][1], 0, ElpCrd[j][0])*pelpm;
    pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
    //pw.color(0x00ff00);
    pw.drawLines_vp(FilteredArc, pts_cnt, false);

    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = vec3f(0, ElpCrd[j][0], ElpCrd[j][1])*pelpm;
    pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
    //pw.color(0xff0000);
    pw.drawLines_vp(FilteredArc, pts_cnt, false);

    //pw.color(0);
    if( drawQuad )  {
      pw.drawLine(NullVec, pelpm[0]);
      pw.drawLine(NullVec, pelpm[1]);
      pw.drawLine(NullVec, pelpm[2]);
      for( int j=0; j < PieDiv; j++ )
        pw.drawLine( PieCrd[j]*pelpm, pelpm[0]*((float)(PieDiv-j)/PieDiv));
      for( int j=0; j < PieDiv; j++ )
        pw.drawLine( vec3f(0, PieCrd[j][0], PieCrd[j][1])*pelpm, pelpm[1]*((float)(PieDiv-j)/PieDiv));
      for( int j=0; j < PieDiv; j++ )
        pw.drawLine( vec3f(PieCrd[j][1], 0, PieCrd[j][0])*pelpm, pelpm[2]*((float)(PieDiv-j)/PieDiv));
    }
  }
public:
  OrtDrawTex() : app(TGXApp::GetInstance()) {  
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
        test.Open("/home/pascal/test.tex", "w");
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
    LinearScale = olx_min((float)pw.GetWidth()/vp[2], (double)pw.GetHeight()/vp[3]);

    if( app.LabelCount() != 0 )  {
      CString fnt("/Verdana findfont ");
      fnt << Round(app.GetLabel(0).Font()->GetPointSize()/LinearScale) << " scalefont setfont";
      pw.custom(fnt.c_str());
    }

    pw.scale(LinearScale, LinearScale);
    LinearScale = 1; // reset now
    DrawScale = LinearScale/app.GetRender().GetScale();
    AradScale = 0.5*DrawScale;///app.GetRender().GetScale(),
    BondRad = 0.05*DrawScale;///app.GetRender().GetScale();
    SceneOrigin = basis.GetCenter();
    //DrawOrigin = vec3f(pw.GetWidth()/2, pw.GetHeight()/2, 0);
    DrawOrigin = vec3f(vp[2]/2, vp[3]/2, 0);
    ProjMatr = basis.GetMatrix()*DrawScale;  
    UnProjMatr = ProjMatr.Inverse();
  }
  void Render(const olxstr& fileName)  {
    PSWriter pw(fileName);
    Init(pw);
              char bf[200];

    const TEBasis& basis = app.GetRender().GetBasis();
    TTypeList<OrtDrawTex::OrtAtom> atoms;
    atoms.SetCapacity(app.AtomCount());
    for( int i=0; i < app.AtomCount(); i++ )  {
      if( app.GetAtom(i).IsDeleted() || !app.GetAtom(i).IsVisible() )
        continue;
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
    for( int i=0; i < app.BondCount(); i++ )  {
      app.GetBond(i).SetTag(i);
      app.GetBond(i).Bond().SetTag(i);
    }
    atoms.QuickSorter.SortSF(atoms, OrtAtomZSort);

    for( int i=0; i < atoms.Count(); i++ )  {
      const TSAtom& sa = *atoms[i].atom;
      const vec3d& p = atoms[i].crd;
      pw.translate(p);
      pw.lineWidth(0.5);
      pw.color(~0);
      if( sa.GetEllipsoid() == NULL )  {
        pw.drawCircle(NullVec, AradScale*sa.GetAtomInfo().GetRad1()*1.05, &PSWriter::fill);
        if( ColorMode == ortep_color_None )  {
          pw.color(0);
          pw.drawCircle(NullVec, AradScale*sa.GetAtomInfo().GetRad1());
        }
        else if( ColorMode == ortep_color_Fill )  {
          pw.newPath();
          pw.circle(NullVec, AradScale*sa.GetAtomInfo().GetRad1());
          pw.gsave();
          pw.color(sa.GetAtomInfo().GetDefColor());
          pw.fill();
          pw.grestore();
          pw.color(0);
          pw.stroke();
        }
        else if( ColorMode == ortep_color_Lines )  {
          pw.color(  sa.GetAtomInfo().GetDefColor() == 0xffffff ? 0 : sa.GetAtomInfo().GetDefColor() );
          pw.drawCircle(NullVec, AradScale*sa.GetAtomInfo().GetRad1());
        }
        //draw the atom
        //draw circle, centre, radius
        sprintf(bf, "\\pgfpathcircle{\\pgfpoint{%fmm}{%fmm}}{%fmm}\n\\pgfshadepath{ballshading%s}{0}\n\\pgfusepath{draw}",         
            p[0]/5, p[1]/5,
            AradScale*sqrt(sa.GetAtomInfo().GetRad1())/5,
            sa.GetAtomInfo().GetSymbol().c_str()
        );        
        test.Writenl(bf);
      }
      else  {
        const mat3f& ielpm = *atoms[i].ielpm;
        //Draw the atom
        //centre, basis
        //drawing centered on the origin, then the ellipse is translated.
        pw.drawEllipse(NullVec, ielpm*1.05, &PSWriter::fill);
        sprintf(bf, "\\pgfpathellipse{\\pgfpoint{%fmm}{%fmm}}\n{\\pgfpoint{%fmm}{%fmm}}\n{\\pgfpoint{%fmm}{%fmm}}\n\\pgfshadepath{ballshading%s}{0}\n\\pgfusepath{draw}", 
            p[0]/5, p[1]/5,
            ielpm[0][0]/5, ielpm[0][1]/5, 
            ielpm[1][0]/5, ielpm[1][1]/5,
            sa.GetAtomInfo().GetSymbol().c_str()
            );
        test.Writenl(bf);
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
      DrawBonds(pw, atoms[i], p);
      pw.translate(-p);
    }
    if( app.LabelCount() != 0 )  {
      for( int i=0; i < app.LabelCount(); i++ )  {
        const TXGlLabel& glxl = app.GetLabel(i);
        vec3d rp = glxl.GetRasterPosition();
        rp[1] += 4;
        rp *= (DrawScale*app.GetRender().GetScale());
        pw.drawText(glxl.GetLabel(), rp+DrawOrigin);
      }
    }
  }

  DefPropP(short, ElpDiv)
  DefPropP(short, PieDiv) 
  DefPropP(short, BondDiv) 
  DefPropP(short, ColorMode)
  DefPropP(float, HBondScale)
  DefPropP(float, BondRad)
};


#endif
