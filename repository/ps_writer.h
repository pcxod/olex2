/* A simple postscript file interface 
(c) Oleg Dolomanov, 2009
*/
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
  typedef void (PSWriter::*RenderFunc)();
  PSWriter(const olxstr& fileName) {
    CurrentColor = 0;
    CurrentLineWidth = 1;
    out.Open(fileName, "w+b");
    out.Writenl("%!PS-Adobe-2.0");
    out.Writenl( "%%Title: Olex2 2D diagram" );
    out.Writenl( "%%Pages: 1" );
    out.Writenl( "%%Page: 1 1" );
    out.Writenl( olxcstr( "%%CreationDate: ") << TETime::FormatDateTime(TETime::Now()) );
    out.Writenl( "%%Orientation: Portrait" );
    out.Writenl( "%%DocumentPaperSizes: A4" );
    out.Writenl( "%%EndComments" );
    //out.Writenl( "0 0 0 setrgbcolor" );
    //out.Writenl( "1 setlinewidth" );
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
  void closePath() {  out.Writenl("closepath");  }
  //..........................................................................
  void stroke() {  out.Writenl("stroke");  }
  //..........................................................................
  void fill()   {  out.Writenl("fill");  }
  //..........................................................................
  void gsave() {  out.Writenl("gsave");  }
  //..........................................................................
  void grestore() {  out.Writenl("grestore");  }
  //..........................................................................
  // default scale, A4
  int GetWidth() const {  return 596;  }
  //..........................................................................
  // default scale A4
  int GetHeight() const {  return 842;  }
  //..........................................................................
  template <typename vec_t> 
  void line(const vec_t& from, const vec_t& to)  {
    moveto(from);
    lineto(to);
  }
  //..........................................................................
  template <typename vec_t> 
  void moveto(const vec_t& to)  {
    sprintf(bf, "%f %f moveto", (float)to[0], (float)to[1]);
    out.Writenl( bf );
  }
  template <typename float_t> 
  void moveto(const float_t& x, const float_t& y)  {
    sprintf(bf, "%f %f moveto", (float)x, (float)y);
    out.Writenl( bf );
  }
  //..........................................................................
  template <typename vec_t> 
  void lineto(const vec_t& to)  {
    sprintf(bf, "%f %f lineto", (float)to[0], (float)to[1]);
    out.Writenl( bf );
  }
  template <typename float_t> 
  void lineto(const float_t& x, const float_t& y)  {
    sprintf(bf, "%f %f lineto", (float)x, (float)y);
    out.Writenl( bf );
  }
  //..........................................................................
  template <typename vec_t> 
  void drawLine(const vec_t& from, const vec_t& to)  {
    newPath();
    line(from, to);
    stroke();
  }
  //..........................................................................
  template <typename list_t> 
  void lines(const list_t& list, size_t cnt = InvalidSize, bool close_path=false)  {
    if( cnt == InvalidSize )  cnt = list.Count();
    if( cnt < 2 )  return;
    moveto(list[0]);
    for( int i=1; i < cnt; i++ )
      lineto(list[i]);
    if( close_path )
      lineto(list[0]);
  }
  template <typename list_t> 
  void drawLines(const list_t& list, size_t cnt = InvalidSize, bool join=false, RenderFunc rf = &PSWriter::stroke)  {
    newPath();
    lines(list, cnt, join);
    (this->*rf)();
  }
  //..........................................................................
  template <typename list_t> 
  void lines_vp(const list_t& list, size_t cnt = InvalidSize, bool close_path=false)  {
    if( cnt == InvalidSize )  cnt = list.Count();
    if( cnt < 2 )  return;
    moveto(*list[0]);
    for( int i=1; i < cnt; i++ )
      lineto(*list[i]);
    if( close_path )  
      lineto(*list[0]);
  }
  template <typename list_t> 
  void drawLines_vp(const list_t& list, size_t cnt = InvalidSize, bool join=false, RenderFunc rf = &PSWriter::stroke)  {
    newPath();
    lines_vp(list, cnt, join);
    (this->*rf)();
  }
  //..........................................................................
  template <typename vec_t, typename mat_t> 
  void ellipse(const vec_t& center, const mat_t& basis)  {
    out.Writenl("matrix currentmatrix");
    //a b 0
    //c d 0
    //tx ty 1
    //[a b c d tx ty]
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
  void drawEllipse(const vec_t& center, const mat_t& basis, RenderFunc rf = &PSWriter::stroke)  {
    newPath();
    ellipse(center, basis);
    (this->*rf)();
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
  void drawCircle(const vec_t& center, const float_t& rad, RenderFunc rf = &PSWriter::stroke)  {
    newPath();
    circle(center, rad);
    (this->*rf)();
  }
  //..........................................................................
  template <class vec_t>
  void drawText(const olxstr& text, const vec_t& pos)  {
    moveto(pos);
    sprintf(bf, "(%s) show", text.c_str());
    out.Writenl( bf );
  }
  //..........................................................................
  template <class vec_t>
  void quad(const vec_t& p1, const vec_t& p2, const vec_t& p3, const vec_t& p4) {
    moveto(p1);
    lineto(p2);
    lineto(p3);
    lineto(p4);
    lineto(p1);
  }
  //..........................................................................
  template <class vec_t>
  void stippledQuad(const vec_t& p1, const vec_t& p2, 
    const vec_t& p3, const vec_t& p4, size_t div, RenderFunc func ) 
  {
    const float x_inc1 = (p2[0]-p1[0])/div;
    const float x_inc2 = (p3[0]-p4[0])/div;
    const float y_inc1 = (p2[1]-p1[1])/div;
    const float y_inc2 = (p3[1]-p4[1])/div;
    float fx1 = p1[0], fy1 = p1[1], fx2 = p4[0], fy2 = p4[1];
    float tx1 = p1[0]+x_inc1, ty1 = p1[1]+y_inc1, 
          tx2 = p4[0]+x_inc2, ty2 = p4[1]+y_inc2;
    for( size_t i=0; i < div; i+=2 )  {
      newPath();
      moveto(fx1, fy1);
      lineto(tx1, ty1);
      lineto(tx2, ty2);
      lineto(fx2, fy2);
      lineto(fx1, fy1);
      fx1 = tx1+x_inc1;  fy1 = ty1+y_inc1;  
      fx2 = tx2+x_inc2;  fy2 = ty2+y_inc2;
      tx1 += 2*x_inc1;  ty1 += 2*y_inc1;
      tx2 += 2*x_inc2;  ty2 += 2*y_inc2;
      (this->*func)();
    }
  }
  //..........................................................................
  //draws a cone using sidea and sideb points and calling func after every quadraterial
  template <class vec_lt>
  void drawQuads(const vec_lt& sidea, const vec_lt& sideb, RenderFunc func)  {
    if( sidea.Count() != sideb.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, "lists mismatch");
    if( sidea.Count() < 2 )  return;
    for( size_t j=1; j < sidea.Count(); j++ )  {
      newPath();
      quad(sidea[j-1], sideb[j-1], sideb[j], sidea[j]);
      (this->*func)();
    }
    newPath();
    quad(sidea.Last(), sideb.Last(), sideb[0], sidea[0]);
    (this->*func)();
  }
  //..........................................................................
  template <class vec_lt>
  void drawQuadsBiColored(const vec_lt& sidea, const vec_lt& sideb, RenderFunc func, uint32_t cl1, uint32_t cl2)  {
    if( sidea.Count() != sideb.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, "lists mismatch");
    if( sidea.Count() < 2 )  return;
    color(cl1);
    for( size_t j=1; j < sidea.Count(); j++ )  {
      newPath();
      quad(sidea[j-1], (sideb[j-1]+sidea[j-1])/2, (sideb[j]+sidea[j])/2, sidea[j]);
      (this->*func)();
    }
    newPath();
    quad(sidea.Last(), (sideb.Last()+sidea.Last())/2, (sideb[0]+sidea[0])/2, sidea[0]);
    (this->*func)();
    color(cl2);
    for( size_t j=1; j < sidea.Count(); j++ )  {
      newPath();
      quad((sidea[j-1]+sideb[j-1])/2, sideb[j-1], sideb[j], (sidea[j]+sideb[j])/2);
      (this->*func)();
    }
    newPath();
    quad((sidea.Last()+sideb.Last())/2, sideb.Last(), sideb[0], (sidea[0]+sideb[0])/2);
    (this->*func)();
  }
  //..........................................................................
  template <class vec_lt>
  void drawQuads(const vec_lt& sidea, const vec_lt& sideb, 
    size_t parts, RenderFunc func)  
  {
    if( sidea.Count() != sideb.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, "lists mismatch");
    if( sidea.Count() < 2 )  return;
    for( size_t j=1; j < sidea.Count(); j++ )
      stippledQuad(sidea[j-1], sideb[j-1], sideb[j], sidea[j], parts, func);
    stippledQuad(sidea.Last(), sideb.Last(), sideb[0], sidea[0], parts, func);
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
