/******************************************************************************
* Copyright (c) 2009 Pascal Parois and O. Dolomanov (OlexSys)                 *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ort_Draw_Tex_H
#define __olx_ort_Draw_Tex_H
#include "gxapp.h"
#include "tex_writer.h"

class OrtDrawTex  {
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
  static int OrtAtomZSort(const OrtAtom &a1, const OrtAtom &a2)  {
    return olx_cmp_float(a1.crd[2], a2.crd[2], 1e-3f);
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

  size_t PrepareArc(const TArrayList<vec3f>& in, TPtrList<const vec3f>& out, const vec3f& normal)  {
    size_t start = InvalidIndex, cnt=0;
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
  void DoDrawBonds(TEXWriter& pw, const OrtAtom& oa, uint32_t color, float scalex, const vec3d& p)  {
    char bf[250]; //pascal
    float xa, xb, ya, yb, za, zb, angle, xc, yc, xd, yd, xe, ye, xf, yf;
    static int AtomCount;
    const TSAtom& sa = *oa.atom;
    vec3f dir_vec, touch_point, touch_point_proj, off_vec, bproj_cnt;
    mat3f proj_mat, rot_mat;

    for( size_t j=0; j < sa.BondCount(); j++ )  {
      const TXBond& bn = static_cast<TXBond&>(sa.Bond(j));
      if( bn.IsDeleted() || !bn.IsVisible() )
        continue;
      vec3f p1 = (bn.Another(sa).crd() + SceneOrigin)*ProjMatr+DrawOrigin;
      if( p1[2] <= oa.crd[2] )  // goes into the plane - drawn
        continue; 
      dir_vec = (p1-oa.crd).Normalise();
      const float pers_scale = 1.0-olx_sqr(dir_vec[2]);
      float brad = (bn.A().GetType() == iHydrogenZ || bn.B().GetType() == iHydrogenZ) ? 
        BondRad*HBondScale : BondRad;
      if( bn.GetType() == sotHBond )  //even thiner H-bonds
        brad *= HBondScale;      
      touch_point = (bn.Another(sa).crd()-oa.atom->crd()).Normalise();
      vec3f rot_vec(-touch_point[1], touch_point[0], 0);
      olx_create_rotation_matrix(rot_mat, rot_vec.Normalise(), touch_point[2]);
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
        const float off_len = AradScale*sa.GetType().r_bonding/2;
        for( int j=0; j < BondDiv; j++ )  {
          BondProjT[j] = BondProjF[j] = (BondCrd[j]*proj_mat).NormaliseTo(brad*(1+pers_scale)*scalex); 
          BondProjT[j].NormaliseTo(brad*2*scalex) += dir_vec*b_len;
          BondProjF[j] += dir_vec*off_len;
        }
      }

      //pw.drawQuads(BondProjF, BondProjT, &TEXWriter::fill);
      if( BondProjF.Count() != BondProjT.Count() )
        throw TFunctionFailedException(__OlxSourceInfo, "lists mismatch");
      if( BondProjF.Count() > 2 )
      {
        //Calculation of the centre of the intersection ellipsis 
        za = zb = ya = yb = xa = xb = 0;
        for( size_t bpi=0; bpi < BondProjT.Count(); bpi++ )  {
          xa += BondProjT[bpi][0];
          ya += BondProjT[bpi][1];
          za += BondProjT[bpi][2];
          xb += BondProjF[bpi][0];
          yb += BondProjF[bpi][1];
          zb += BondProjF[bpi][2];
        }            
        xa = xa/BondProjT.Count() + p[0];
        xb = xb/BondProjT.Count() + p[0];
        ya = ya/BondProjT.Count() + p[1];
        yb = yb/BondProjT.Count() + p[1];
        za = za/BondProjT.Count() + p[2];
        zb = zb/BondProjT.Count() + p[2];
        //calculation of the angle between the bond and the vertical. Needed to apply a shading on the bond
        if(xb>xa && (xb-xa)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya))<1 && (xb-xa)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya))>-1) 
        {
          angle = asin((xb-xa)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya)))*180/M_PI;
          if(ya<yb)
            angle = - asin((xb-xa)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya)))*180/M_PI;
        }
        else if(xa>xb && (xa-xb)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya))<1 && (xa-xb)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya))>-1) 
        {
          angle = - asin((xa-xb)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya)))*180/M_PI;
          if(ya<yb)
            angle= asin((xa-xb)/sqrt((xb-xa)*(xb-xa)+(yb-ya)*(yb-ya)))*180/M_PI;
        }
        else
        {
          angle = 0;
        }

        //To coordinate of the rectangle for the bond
        yc = brad*(xa-xb)/sqrt(xa*xa-2*xa*xb+xb*xb+(ya-yb)*(ya-yb))+ya;
        xc = -(yb-ya)*(yc-ya)/(xb-xa)+xa;
        ye = ya-(yc-ya);
        xe = xa-(xc-xa);

        //From coordinate of the recangle for the bond
        //0.7, this end is a bit smaller
        yd = 0.7*brad*(xa-xb)/sqrt(xa*xa-2*xa*xb+xb*xb+(ya-yb)*(ya-yb))+yb;
        xd = -(ya-yb)*(yd-yb)/(xa-xb)+xb;
        yf = yb-(yd-yb);
        xf = xb-(xd-xb);

        //white perimeter around the bond
        sprintf(bf, "\\draw[white,very thick, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
          xc/5,yc/5,
          xe/5, ye/5,
          xf/5, yf/5, 
          xd/5, yd/5);
        pw.Writenl(bf);

        //draw the shading on the bond
        //I need the projection coordinate of the atom
        vec3f p2 = (bn.A().crd() + SceneOrigin)*ProjMatr+DrawOrigin;
        vec3f p3 = (bn.B().crd() + SceneOrigin)*ProjMatr+DrawOrigin;

        //All this, just to have a correct shading...
        if(ya > yb && xa < xb) 
        {
          if(p2[2] < p3[2])
            sprintf(bf, "\\shade[top color=\\color%s, bottom color=\\color%s, shading angle=%f, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
            bn.B().GetType().symbol.c_str(), bn.A().GetType().symbol.c_str(), angle,
            xc/5,yc/5,
            xe/5, ye/5,
            xf/5, yf/5, 
            xd/5, yd/5);
          else
            sprintf(bf, "\\shade[top color=\\color%s, bottom color=\\color%s, shading angle=%f, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
            bn.A().GetType().symbol.c_str(), bn.B().GetType().symbol.c_str(), angle,
            xc/5,yc/5,
            xe/5, ye/5,
            xf/5, yf/5, 
            xd/5, yd/5);
        }
        else if(ya > yb && xa > xb) 
        {
          if(p2[2] < p3[2])
            sprintf(bf, "\\shade[top color=\\color%s, bottom color=\\color%s, shading angle=%f, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
            bn.B().GetType().symbol.c_str(), bn.A().GetType().symbol.c_str(), angle,
            xc/5,yc/5,
            xe/5, ye/5,
            xf/5, yf/5, 
            xd/5, yd/5);
          else
            sprintf(bf, "\\shade[top color=\\color%s, bottom color=\\color%s, shading angle=%f, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
            bn.A().GetType().symbol.c_str(), bn.B().GetType().symbol.c_str(), angle,
            xc/5,yc/5,
            xe/5, ye/5,
            xf/5, yf/5, 
            xd/5, yd/5);                    
        }
        else if(ya <= yb && xa <= xb) 
        {
          if(p2[2] < p3[2])
            sprintf(bf, "\\shade[top color=\\color%s, bottom color=\\color%s, shading angle=%f, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
            bn.A().GetType().symbol.c_str(), bn.B().GetType().symbol.c_str(), angle,
            xc/5,yc/5,
            xe/5, ye/5,
            xf/5, yf/5, 
            xd/5, yd/5);
          else
            sprintf(bf, "\\shade[top color=\\color%s, bottom color=\\color%s, shading angle=%f, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
            bn.B().GetType().symbol.c_str(), bn.A().GetType().symbol.c_str(), angle,
            xc/5,yc/5,
            xe/5, ye/5,
            xf/5, yf/5, 
            xd/5, yd/5);                    
        }
        else //if(ya <= yb && xa >= xb)
        {
          if(p2[2] < p3[2])
            sprintf(bf, "\\shade[top color=\\color%s, bottom color=\\color%s, shading angle=%f, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
            bn.A().GetType().symbol.c_str(), bn.B().GetType().symbol.c_str(), angle,
            xc/5,yc/5,
            xe/5, ye/5,
            xf/5, yf/5, 
            xd/5, yd/5);
          else
            sprintf(bf, "\\shade[top color=\\color%s, bottom color=\\color%s, shading angle=%f, rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
            bn.B().GetType().symbol.c_str(), bn.A().GetType().symbol.c_str(), angle,
            xc/5,yc/5,
            xe/5, ye/5,
            xf/5, yf/5, 
            xd/5, yd/5);                    
        }

        pw.Writenl(bf);

        //Draw the black stroke around the bond
        sprintf(bf, "\\draw[rounded corners=\\cornerradius] (%fmm,%fmm) --  (%fmm,%fmm) -- (%fmm,%fmm) -- (%fmm,%fmm) -- cycle;",
          xc/5,yc/5,
          xe/5, ye/5,
          xf/5, yf/5, 
          xd/5, yd/5);
        pw.Writenl(bf);

      }

    }
  }
  void DrawBonds(TEXWriter& pw, const OrtAtom& oa, const vec3d& p) {
    //DoDrawBonds(pw, oa, ~0, 1.2, p);
    DoDrawBonds(pw, oa, 0, 1, p);
  }
  void DrawRimsAndQuad(TEXWriter& pw, const OrtAtom& oa, bool drawQuad)  {
    const mat3f& elpm = *oa.elpm;
    const mat3f& ielpm = *oa.ielpm;
    //pelpm - projection ellipsoid matrix
    mat3f pelpm(elpm[0][2] < 0 ? -elpm[0] : elpm[0],
      elpm[1][2] < 0 ? -elpm[1] : elpm[1],
      elpm[2][2] < 0 ? -elpm[2] : elpm[2]);
    vec3f norm_vec = (ElpCrd[0]*ielpm).XProdVec(ElpCrd[1]*ielpm);

    if( norm_vec[2] < 0 )
      norm_vec *= -1;
    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = ElpCrd[j]*pelpm;
    size_t pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
    //pw.color(0x0000ff);
    //pw.drawLines_vp(FilteredArc, pts_cnt, false);

    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = vec3f(ElpCrd[j][1], 0, ElpCrd[j][0])*pelpm;
    pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
    //pw.color(0x00ff00);
    //pw.drawLines_vp(FilteredArc, pts_cnt, false);

    for( int j=0; j < ElpDiv; j++ )
      Arc[j] = vec3f(0, ElpCrd[j][0], ElpCrd[j][1])*pelpm;
    pts_cnt = PrepareArc(Arc, FilteredArc, norm_vec);
    //pw.color(0xff0000);
    //pw.drawLines_vp(FilteredArc, pts_cnt, false);

    //pw.color(0);
    /*
    if( drawQuad )  {
    //pw.drawLine(NullVec, pelpm[0]);
    //pw.drawLine(NullVec, pelpm[1]);
    //pw.drawLine(NullVec, pelpm[2]);
    for( int j=0; j < PieDiv; j++ )
    //pw.drawLine( PieCrd[j]*pelpm, pelpm[0]*((float)(PieDiv-j)/PieDiv));
    for( int j=0; j < PieDiv; j++ )
    //pw.drawLine( vec3f(0, PieCrd[j][0], PieCrd[j][1])*pelpm, pelpm[1]*((float)(PieDiv-j)/PieDiv));
    for( int j=0; j < PieDiv; j++ )
    //pw.drawLine( vec3f(PieCrd[j][1], 0, PieCrd[j][0])*pelpm, pelpm[2]*((float)(PieDiv-j)/PieDiv));
    }*/
  }
public:
  OrtDrawTex() : app(TGXApp::GetInstance()) {  
    ElpDiv = 36;
    BondDiv = 12;
    PieDiv = 4;
    QuadLineWidth = PieLineWidth = 0.5;
    BondRad = 4;
    ColorMode = 0;
    HBondScale = 0.8;
  }
  // create ellipse and pie coordinates
  void Init(TEXWriter& pw)  {
    ElpCrd.SetCount(ElpDiv);
    Arc.SetCount(ElpDiv);
    PieCrd.SetCount(PieDiv);
    FilteredArc.SetCount(ElpDiv);
    BondCrd.SetCount(BondDiv);
    BondProjF.SetCount(BondDiv);
    BondProjT.SetCount(BondDiv);
    double sin_a, cos_a;
    olx_sincos(2*M_PI/ElpDiv, &sin_a, &cos_a);
    vec3f ps(cos_a, -sin_a, 0);
    for( int i=0; i < ElpDiv; i++ )  {
      ElpCrd[i] = ps;
      const float x = ps[0];
      ps[0] = (float)(cos_a*x + sin_a*ps[1]);
      ps[1] = (float)(cos_a*ps[1] - sin_a*x);
    }
    olx_sincos(2*M_PI/BondDiv, &sin_a, &cos_a);
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
    olx_gl::get(GL_VIEWPORT, vp);
    TGXApp& app = TGXApp::GetInstance();
    const TEBasis& basis = app.GetRender().GetBasis();
    LinearScale = 1; // reset now
    DrawScale = app.GetRender().GetBasis().GetZoom()/(LinearScale*app.GetRender().GetScale());
    AradScale = 0.5*DrawScale;///app.GetRender().GetScale(),
    BondRad = 0.1*DrawScale;///app.GetRender().GetScale();
    SceneOrigin = basis.GetCenter();
    //DrawOrigin = vec3f(pw.GetWidth()/2, pw.GetHeight()/2, 0);
    DrawOrigin = vec3f(vp[2]/2, vp[3]/2, 0);
    ProjMatr = basis.GetMatrix()*DrawScale;  
    UnProjMatr = ProjMatr.Inverse();
  }
  void Render(const olxstr& fileName)  {
    TEXWriter pw(fileName);
    Init(pw);
    char bf[200];
    bool CurrentAtom;

    //temporary colour dictionary
    //map<olxstr,string> ColourDic;
    olxdict <olxstr,olxcstr,olxstrComparator<false> > ColourDic;
    ColourDic("H","white");
    ColourDic("D","white");
    ColourDic("He","white");
    ColourDic("Li","white");
    ColourDic("Be","white");
    ColourDic("B","white");
    ColourDic("C","gray");
    ColourDic("N","blue");
    ColourDic("O","red");
    ColourDic("F","darkgreen");
    ColourDic("Na","white");
    ColourDic("Mg","white");
    ColourDic("Al","white");
    ColourDic("Si","orange");
    ColourDic("P","purple");
    ColourDic("S","yellow");
    ColourDic("Cl","white");
    ColourDic("K","white");
    ColourDic("Ca","white");
    ColourDic("Sc","white");
    ColourDic("Ti","white");
    ColourDic("V","white");
    ColourDic("Cr","white");
    ColourDic("Mn","white");
    ColourDic("Fe","white");
    ColourDic("Co","white");
    ColourDic("Ni","white");
    ColourDic("Cu","white");
    ColourDic("Zn","white");
    ColourDic("Ga","white");
    ColourDic("Ge","white");
    ColourDic("As","white");
    ColourDic("Se","white");
    ColourDic("Br","white");
    ColourDic("Kr","white");
    ColourDic("Rb","white");
    ColourDic("Sr","white");
    ColourDic("Y","white");
    ColourDic("Zr","white");
    ColourDic("Nb","white");
    ColourDic("Mo","white");
    ColourDic("Tc","white");
    ColourDic("Ru","white");
    ColourDic("Rh","white");
    ColourDic("Pd","white");
    ColourDic("Ag","white");
    ColourDic("Cd","white");
    ColourDic("In","white");
    ColourDic("Sn","white");
    ColourDic("Sb","white");
    ColourDic("Te","white");
    ColourDic("I","white");
    ColourDic("Xe","white");
    ColourDic("Cs","white");
    ColourDic("Ba","white");
    ColourDic("La","white");
    ColourDic("Hf","white");
    ColourDic("Ta","white");
    ColourDic("W","white");
    ColourDic("Re","white");
    ColourDic("Os","white");
    ColourDic("Ir","white");
    ColourDic("Pt","white");
    ColourDic("Au","white");
    ColourDic("Hg","white");
    ColourDic("Tl","white");
    ColourDic("Pb","white");
    ColourDic("Bi","white");
    ColourDic("Po","white");
    ColourDic("At","white");
    ColourDic("Rn","white");
    ColourDic("Fr","white");
    ColourDic("Ra","white");
    ColourDic("Ac","white");
    ColourDic("Ce","white");
    ColourDic("Pr","white");
    ColourDic("Nd","white");
    ColourDic("Pm","white");
    ColourDic("Sm","white");
    ColourDic("Eu","white");
    ColourDic("Gd","white");
    ColourDic("Tb","white");
    ColourDic("Dy","white");
    ColourDic("Ho","white");
    ColourDic("Er","white");
    ColourDic("Tm","white");
    ColourDic("Yb","white");
    ColourDic("Lu","white");
    ColourDic("Th","white");
    ColourDic("Pa","white");
    ColourDic("U","white");
    ColourDic("Np","white");
    ColourDic("Pu","white");
    ColourDic("Am","white");
    ColourDic("Cm","white");
    ColourDic("Bk","white");
    ColourDic("Cf","white");
    ColourDic("Es","white");
    ColourDic("Fm","white");
    ColourDic("Md","white");
    ColourDic("No","white");
    ColourDic("Lr","white");
    ColourDic("Ne","white");
    ColourDic("Ar","white");
    olxcstr def_color = "white";

    const TEBasis& basis = app.GetRender().GetBasis();
    TTypeList<OrtDrawTex::OrtAtom> atoms;
    TGXApp::AtomIterator ai = app.GetAtoms();
    atoms.SetCapacity(ai.count);

    //radius for the end of the bond
    sprintf(bf, "\\newcommand{\\cornerradius}{%fmm}", 
      0.12*0.1*DrawScale);
    pw.Writenl(bf);

    while( ai.HasNext() ) {
      TXAtom& sa = ai.Next();
      if( sa.IsDeleted() || !sa.IsVisible() )
        continue;

      //write latex colors atoms commands and shading
      CurrentAtom=false;
      //We check first if a similar atom already get processed.
      for( size_t j=0; j < atoms.Count(); j++ )  {
        const TSAtom& sa2 = *atoms[j].atom;
        if(sa.GetType() == sa2.GetType())
          CurrentAtom=true;
      }
      //writing of the color definition of the atom
      if(CurrentAtom == false)
      {
        sprintf(bf, "\\newcommand{\\color%s}{%s!90!black}",
          sa.GetType().symbol.c_str(),
          ColourDic.Find(sa.GetType().symbol, def_color).c_str()
          );
        pw.Writenl(bf);

        sprintf(bf, "\\pgfdeclareradialshading{ballshading%s}{\\pgfpoint{-10bp}{10bp}}",
          sa.GetType().symbol.c_str());
        pw.Writenl(bf);
        sprintf(bf, "{color(0bp)=(\\color%s!15!white); color(9bp)=(\\color%s!75!white);",
          sa.GetType().symbol.c_str(), sa.GetType().symbol.c_str());
        pw.Writenl(bf);
        sprintf(bf, "color(18bp)=(\\color%s!70!black); color(25bp)=(\\color%s!50!black); color(50bp)=(black)} ",
          sa.GetType().symbol.c_str(), sa.GetType().symbol.c_str());          
        pw.Writenl(bf);
      }

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

    //tex header is finished, start the body
    pw.Writenl("\\begin{document}");
    pw.Writenl("\\begin{tikzpicture}");
    QuickSorter::SortSF(atoms, OrtAtomZSort);

    for( size_t i=0; i < atoms.Count(); i++ )  {
      const TSAtom& sa = *atoms[i].atom;
      const vec3d& p = atoms[i].crd;
      if( sa.GetEllipsoid() == NULL )  {
        //draw the atom as a circle, done twice, to make a white border arounf it
        //draw circle, centre, radius
        sprintf(bf, "\\begin{pgfscope}\\pgfsetstrokecolor{white}\\pgfsetlinewidth{\\whitespace}\\pgfpathcircle{\\pgfpoint{%fmm}{%fmm}}{%fmm}\n\\pgfshadepath{ballshading%s}{0}\n\\pgfusepath{draw}\\end{pgfscope}",         
          p[0]/5, p[1]/5,
          AradScale*sqrt(sa.GetType().r_pers)/5,
          sa.GetType().symbol.c_str()
          );        
        pw.Writenl(bf);
        sprintf(bf, "\\pgfpathcircle{\\pgfpoint{%fmm}{%fmm}}{%fmm}\n\\pgfshadepath{ballshading%s}{0}\n\\pgfusepath{draw}",         
          p[0]/5, p[1]/5,
          AradScale*sqrt(sa.GetType().r_bonding)/5,
          sa.GetType().symbol.c_str()
          );
        pw.Writenl(bf);
        //write a node to make life easier later
        sprintf(bf, "\\node (atom%i) at (%fmm,%fmm) {};",
          i,
          p[0]/5, p[1]/5);
        pw.Writenl(bf);
      }
      else  {
        const mat3f& ielpm = *atoms[i].ielpm;
        //Draw the atom as an ellipse
        //centre, basis
        //draw a white border around the ellipse
        pw.Writenl("\\begin{pgfscope}\n\\pgfsetstrokecolor{white}\\pgfsetlinewidth{\\whitespace}");
        sprintf(bf, "\\pgfpathellipse{\\pgfpoint{%fmm}{%fmm}}\n{\\pgfpoint{%fmm}{%fmm}}\n{\\pgfpoint{%fmm}{%fmm}}\n\\pgfusepath{draw}", 
          p[0]/5, p[1]/5,
          ielpm[0][0]/5, ielpm[0][1]/5, 
          ielpm[1][0]/5, ielpm[1][1]/5,
          sa.GetType().symbol.c_str()
          );
        pw.Writenl(bf);
        pw.Writenl("\\end{pgfscope}");
        //the ellipse
        sprintf(bf, "\\pgfpathellipse{\\pgfpoint{%fmm}{%fmm}}\n{\\pgfpoint{%fmm}{%fmm}}\n{\\pgfpoint{%fmm}{%fmm}}\n\\pgfshadepath{ballshading%s}{0}\n\\pgfusepath{draw}", 
          p[0]/5, p[1]/5,
          ielpm[0][0]/5, ielpm[0][1]/5, 
          ielpm[1][0]/5, ielpm[1][1]/5,
          sa.GetType().symbol.c_str()
          );
        pw.Writenl(bf);
        //write a node to make life easier later
        sprintf(bf, "\\node (atom%i) at (%fmm,%fmm) {};",
          i,
          p[0]/5, p[1]/5);
        pw.Writenl(bf);

        //DrawRimsAndQuad(pw, atoms[i], sa.GetAtomInfo() != iCarbonIndex);
      }
      DrawBonds(pw, atoms[i], p);
    }
    //write labels
    if( app.LabelCount() != 0 )  {
      for( size_t i=0; i < app.LabelCount(); i++ )  {
        const TXGlLabel& glxl = app.GetLabel(i);
        vec3d rp = glxl.GetRasterPosition();
        rp[1] += 4;
        rp *= (DrawScale*app.GetRender().GetScale());
        //pw.drawText(glxl.GetLabel(), rp+DrawOrigin);
      }
    }

    //document close
    pw.Writenl("\\end{tikzpicture}");
    pw.Writenl("\\end{document}");

  }

  DefPropP(short, ElpDiv)
    DefPropP(short, PieDiv) 
    DefPropP(short, BondDiv) 
    DefPropP(short, ColorMode)
    DefPropP(float, HBondScale)
    DefPropP(float, BondRad)
};

//to be done: labels
//\node [circle,draw,label=60:$60^\circ$,label=below:$-90^\circ$] {my circle};

#endif
