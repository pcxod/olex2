#ifndef _olx_ps_writerH
#define _olx_ps_writerH
#include "efile.h"
#include "glbase.h"
#include "threex3.h"

class PSWriter  {
  TEFile out;
  char bf[80];
  uint32_t CurrentColor;
  float CurrentLineWidth;
public:
  PSWriter(const olxstr& fileName) {
    CurrentColor = 0;
    CurrentLineWidth = 1;
    out.Open(fileName, "w+b");
    out.Writenl("%!PS-Adobe-2.0");
    out.Writenl( "%%Title: Olex2 test" );
    out.Writenl( CString( "%%CreationDate: ") << TETime::FormatDateTime(TETime::Now()) );
    out.Writenl( "%%Orientation: Portrait" );
    out.Writenl( "%%DocumentPaperSizes: A4" );
    out.Writenl( "%%EndComments" );
  }
  //..........................................................................
  void color(uint32_t rgb)  {
    if( CurrentColor == rgb )  return;
      CurrentColor = rgb;
    sprintf(bf, "%f %f %f setrgbcolor", (float)GetRValue(rgb)/255, 
      (float)GetGValue(rgb)/255,
      (float)GetBValue(rgb)/255
    );
    out.Writenl( bf );
  }
  //..........................................................................
  template <typename vec_t> 
  void translate(const vec_t& origin)  {
    sprintf(bf, "%f %f translate", (float)origin[0], (float)origin[1]);
    out.Writenl( bf );
  }
  //..........................................................................
  template <typename float_t> 
  void translate(const float_t& x, const float_t& y)  {
    sprintf(bf, "%f %f translate", (float)x, (float)y);
    out.Writenl( bf );
  }
  //..........................................................................
  template <typename float_t> 
  void lineWidth(const float_t& lw)  {
    if( lw == CurrentLineWidth )  return;
    sprintf(bf, "%f setlinewidth", (float)lw);
    out.Writenl( bf );
    CurrentLineWidth = (float)lw;
  }
  //..........................................................................
  template <typename vec_t> 
  void scale(const vec_t& origin)  {
    sprintf(bf, "%f %f scale", (float)origin[0], (float)origin[1]);
    out.Writenl( bf );
  }
  //..........................................................................
  template <typename float_t> 
  void scale(const float_t& x_scale, const float_t& y_scale)  {
    sprintf(bf, "%f %f scale", (float)x_scale, (float)y_scale);
    out.Writenl( bf );
  }
  //..........................................................................
  void custom(const char* cmd) {  out.Writenl(cmd);  }
  //..........................................................................
  void custom(const olxstr& cmd) {  out.Writenl(cmd);  }
  //..........................................................................
  void newPath() {  out.Writenl("newpath");  }
  //..........................................................................
  void stroke() {  out.Writenl("stroke");  }
  //..........................................................................
  void fill()   {  out.Writenl("fill");  }
  //..........................................................................
  // default scale, A4
  int GetWidth() const {  return 596;  }
  //..........................................................................
  // default scale A4
  int GetHeight() const {  return 842;  }
  //..........................................................................
  template <typename vec_t> 
  void line(const vec_t& from, const vec_t& to)  {
    sprintf(bf, "%f %f moveto", (float)from[0], (float)from[1]);
    out.Writenl( bf );
    sprintf(bf, "%f %f lineto", (float)to[0], (float)to[1]);
    out.Writenl( bf );
  }
  template <typename vec_t> 
  void drawLine(const vec_t& from, const vec_t& to)  {
    newPath();
    line(from, to);
    stroke();
  }
  //..........................................................................
  template <typename list_t> 
  void lines(const list_t& list, int cnt = -1, bool join=false)  {
    if( cnt == -1 )  cnt = list.Count();
    if( cnt < 2 )  return;
    sprintf(bf, "%f %f moveto", (float)list[0][0], (float)list[0][1]);
    out.Writenl( bf );
    for( int i=1; i < cnt; i++ )  {
      sprintf(bf, "%f %f lineto", (float)list[i][0], (float)list[i][1]);
      out.Writenl( bf );
    }
    if( join && cnt > 2 )  {
      sprintf(bf, "%f %f lineto", (float)list[0][0], (float)list[0][1]);
      out.Writenl( bf );
      out.Writenl("closepath");
    }
  }
  template <typename list_t> 
  void drawLines(const list_t& list, int cnt = -1, bool join=false)  {
    newPath();
    lines(list, cnt, join);
    stroke();
  }
  //..........................................................................
  template <typename list_t> 
  void lines_vp(const list_t& list, int cnt = -1, bool join=false)  {
    if( cnt == -1 )  cnt = list.Count();
    if( cnt < 2 )  return;
    sprintf(bf, "%f %f moveto", (float)(*list[0])[0], (float)(*list[0])[1]);
    out.Writenl( bf );
    for( int i=1; i < cnt; i++ )  {
      sprintf(bf, "%f %f lineto", (float)(*list[i])[0], (float)(*list[i])[1]);
      out.Writenl( bf );
    }
    if( join && cnt > 2 )  {
      sprintf(bf, "%f %f lineto", (float)(*list[0])[0], (float)(*list[0])[1]);
      out.Writenl( bf );
      out.Writenl("closepath");
    }
  }
  template <typename list_t> 
  void drawLines_vp(const list_t& list, int cnt = -1, bool join=false)  {
    newPath();
    lines_vp(list, cnt, join);
    stroke();
  }
  //..........................................................................
  template <typename vec_t, typename mat_t> 
  void ellipse(const vec_t& center, const mat_t& basis)  {
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
  }
  template <typename vec_t, typename mat_t> 
  void drawEllipse(const vec_t& center, const mat_t& basis)  {
    newPath();
    ellipse(center, basis);
    stroke();
  }
  //..........................................................................
  template <typename vec_t, typename float_t> 
  void circle(const vec_t& center, const float_t& rad)  {
    sprintf(bf, "%f %f %f 0 360 arc", (float)center[0], 
      (float)center[1], 
      (float)rad 
    );
    out.Writenl( bf );
  }
  template <typename vec_t, typename float_t> 
  void drawCircle(const vec_t& center, const float_t& rad)  {
    newPath();
    circle(center, rad);
    stroke();
  }
  //..........................................................................
  template <class vec_t>
  void drawText(const olxstr& text, const vec_t& pos)  {
    sprintf(bf, "%f %f moveto", (float)pos[0], (float)pos[1]);
    out.Writenl( bf );
    sprintf(bf, "(%s) show", text.c_str());
    out.Writenl( bf );
  }
  //..........................................................................
  
  //..........................................................................
  template <typename vec_t, typename float_t>
  void arc(const vec_t& center, const float_t& rad, const float_t startAngle, const float_t& endAngle)  {
    sprintf(bf, "%f %f %f %f %f arc", (float)center[0], (float)center[1], (float)rad, (float)startAngle, (float)endAngle);
    out.Writenl( bf );
  }
};

#endif
