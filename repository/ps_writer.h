#ifndef _olx_ps_writerH
#define _olx_ps_writerH
#include "efile.h"
#include "glbase.h"
#include "threex3.h"

class PSWriter  {
  TEFile out;
  char bf[80];
public:
  PSWriter(const olxstr& fileName) {
    out.Open(fileName, "w+b");
    out.Writenl("%!PS-Adobe-2.0");
    out.Writenl( "%%Title: Olex2 test" );
    out.Writenl( CString( "%%CreationDate: ") << TETime::FormatDateTime(TETime::Now()) );
    out.Writenl( "%%Orientation: Portrait" );
    out.Writenl( "%%DocumentPaperSizes: A4" );
    out.Writenl( "%%EndComments" );
    out.Writenl( "1.0 setlinewidth" );
  }
  void color(uint32_t rgb)  {
    sprintf(bf, "%f %f %f setrgbcolor", (float)GetRValue(rgb)/255, 
      (float)GetGValue(rgb)/255,
      (float)GetBValue(rgb)/255
    );
    out.Writenl( bf );
  }
  template <typename vec_t> 
  void translate(const vec_t& origin)  {
    sprintf(bf, "%f %f translate", (float)origin[0], (float)origin[1]);
    out.Writenl( bf );
  }
  template <typename float_t> 
  void translate(const float_t& x, const float_t& y)  {
    sprintf(bf, "%f %f translate", (float)x, (float)y);
    out.Writenl( bf );
  }
  template <typename vec_t> 
  void scale(const vec_t& origin)  {
    sprintf(bf, "%f %f scale", (float)origin[0], (float)origin[1]);
    out.Writenl( bf );
  }
  template <typename float_t> 
  void scale(const float_t& x_scale, const float_t& y_scale)  {
    sprintf(bf, "%f %f scale", (float)x_scale, (float)y_scale);
    out.Writenl( bf );
  }
  template <typename vec_t> 
  void line(const vec_t& from, const vec_t& to)  {
    out.Writenl("newpath");
    sprintf(bf, "%f %f moveto", (float)from[0], (float)from[1]);
    out.Writenl( bf );
    sprintf(bf, "%f %f lineto", (float)to[0], (float)to[1]);
    out.Writenl( bf );
    out.Writenl("stroke");
  }
  void erasePath() {
    out.Writenl("clippath");
    out.Writenl("1 setgray");
    out.Writenl("fill");
  }
  template <typename list_t> 
  void lines(const list_t& list, int cnt = -1, bool join=false)  {
    if( cnt == -1 )  cnt = list.Count();
    if( cnt < 2 )  return;
    out.Writenl("newpath");
    sprintf(bf, "%f %f moveto", (float)list[0][0], (float)list[0][1]);
    out.Writenl( bf );
    for( int i=1; i < cnt; i++ )  {
      sprintf(bf, "%f %f lineto", (float)list[i][0], (float)list[i][1]);
      out.Writenl( bf );
    }
    if( join && cnt > 2 )  {
      sprintf(bf, "%f %f lineto", (float)list[0][0], (float)list[0][1]);
      out.Writenl( bf );
    }
    out.Writenl("stroke", 6);
  }
  template <typename vec_t, typename mat_t> 
  void ellipse(const vec_t& center, const mat_t& basis)  {
    out.Writenl("newpath");
    out.Writenl("matrix currentmatrix");
    sprintf(bf, "[%f %f %f %f %f %f] concat", (float)basis[0][0], 
      (float)basis[0][1], 
      (float)basis[1][0],
      (float)basis[1][1],
      (float)center[0], 
      (float)center[1] 
    );
    out.Writenl( bf );
    out.Writenl( "0 0 1 0 360 arc");
    out.Writenl("setmatrix");
    out.Writenl("stroke");
  }
  template <typename vec_t, typename mat_t> 
  void arcs(const vec_t& center, const mat_t& basis)  {
    out.Writenl("newpath");
    out.Writenl("matrix currentmatrix");
    sprintf(bf, "[%f %f %f %f %f %f] concat", (float)basis[0][0], 
      (float)basis[0][1], 
      (float)basis[1][0],
      (float)basis[1][1],
      (float)center[0], 
      (float)center[1] 
    );
    out.Writenl( bf );
    if( basis[1][2] >= 0 )
      out.Writenl( "0 0 1 0 180 arc");
    else
      out.Writenl( "0 0 1 180 360 arc");
    //out.Writenl("setmatrix");
    //sprintf(bf, "[%f %f %f %f %f %f] concat", (float)basis[0][0], 
    //  (float)basis[0][1], 
    //  (float)basis[1][0],
    //  (float)basis[1][1],
    //  (float)center[0], 
    //  (float)center[1] 
    //);
    //out.Writenl( bf );
    //out.Writenl( "0 0 1 0 180 arc");
    out.Writenl("setmatrix");
    out.Writenl("stroke");
  }
  template <typename vec_t, typename float_t> 
  void circle(const vec_t& center, const float_t& rad)  {
    out.Writenl("newpath");
    sprintf(bf, "%f %f %f 0 360 arc", (float)center[0], 
      (float)center[1], 
      (float)rad 
    );
    out.Writenl( bf );
    out.Writenl("stroke");
  }
  template <typename vec_t>
  bool arc(const vec_t& p1, const vec_t& p2, const vec_t& p3)  {
    const float  dx1 = p2[0] - p1[0],
                 dx2 = p3[0] - p2[0];
    if( dx1 != 0 && dx2 != 0 )  {
      const float  ma = (p2[1] - p1[1])/dx1,
                   mb = (p3[1] - p2[1])/dx2;
      if( ma != mb )  {
        vec3f center;
        center[0] = (ma*mb*(p1[1]-p3[1]) + mb*(p1[0]+p2[0]) -ma*(p2[0]+p3[0]))/(2*(mb-ma));
        center[1] = ((p1[0]+p2[0])/2-center[0])/ma + (p1[1]+p2[1])/2;
        float rad = center.DistanceTo(p1);
        vec3f r1 = (p1 - center).Normalise(),
              r2 = (p3 - center).Normalise();
        float a1 = acos(r1[0]),
              a2 = acos(r2[0]);
        if( r1[1] < 0 )  a1 += M_PI;
        if( r2[1] < 0 )  a2 += M_PI;
        a1 = a1 * 180/M_PI;
        a2 = a2 * 180/M_PI;
        bool swapped = (r1[0]*r2[1] - r2[0]*r1[1]) > 0;  // triangle "area", xprod in 2d
        out.Writenl("newpath");
        if( !swapped )
          sprintf(bf, "%f %f %f %f %f arc", (float)center[0], (float)center[1], (float)rad, a2, a1);
        else  
          sprintf(bf, "%f %f %f %f %f arc", (float)center[0], (float)center[1], (float)rad, 360.0-a2, a1);
        out.Writenl( bf );
        out.Writenl("stroke");
        return true;
      }
    }
    return false;
  }
  //template <class float_t>
  //void arc()  {
  //}
};

#endif
