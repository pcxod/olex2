/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef _olx_ps_writerH
#define _olx_ps_writerH
#include "efile.h"
#include "glbase.h"
#include "threex3.h"
// mind to change the length of the buffer if changes!!!
#ifdef _MSC_VER
#  define psw_sprintf(bf, format, ...)  sprintf_s(bf, 80, format, __VA_ARGS__)
#else
#  define psw_sprintf sprintf
#endif

/* A simple postscript file interface 
*/
class PSWriter  {
  TEFile out;
  char bf[80];
  uint32_t CurrentColor;
  float CurrentLineWidth;
  uint64_t bounding_box_pos;
public:
  typedef void (PSWriter::*RenderFunc)();
  PSWriter(const olxstr& fileName, bool write_bounding_box)
    : bounding_box_pos(InvalidIndex)
  {
    CurrentColor = 0;
    CurrentLineWidth = 1;
    out.Open(fileName, "w+b");
    out.Writeln("%!PS-Adobe-2.0 EPSF-1.2");
    if( write_bounding_box )  {
      bounding_box_pos = out.GetPosition();
      olxcstr bb("%%BoundingBox: ");
      bounding_box_pos += bb.Length();
      bb << olxcstr::CharStr(' ', 4*12);
      out.Writeln(bb);
    }
    out.Writeln("%%Title: Olex2 2D diagram");
    out.Writeln("%%Pages: 1");
    out.Writeln("%%Page: 1 1");
    out.Writeln(olxcstr("%%CreationDate: ") <<
      TETime::FormatDateTime(TETime::Now()));
    out.Writeln("%%Orientation: Portrait");
    out.Writeln("%%DocumentPaperSizes: A4");
    out.Writeln("%%EndComments");
  }
  //..........................................................................
  void color(uint32_t rgb)  {
    if( CurrentColor == rgb )  return;
      CurrentColor = rgb;
    out.Writeln(color_str(rgb));
  }
  //..........................................................................
  const char* color_str(uint32_t rgb)  {
    psw_sprintf(bf, "%f %f %f setrgbcolor", (float)OLX_GetRValue(rgb)/255, 
      (float)OLX_GetGValue(rgb)/255,
      (float)OLX_GetBValue(rgb)/255
    );
    return bf;
  }
  //..........................................................................
  template <typename vec_t> 
  void translate(const vec_t& origin)  {
    psw_sprintf(bf, "%f %f translate", (float)origin[0], (float)origin[1]);
    out.Writeln(bf);
  }
  //..........................................................................
  template <typename float_t> 
  void translate(const float_t& x, const float_t& y)  {
    psw_sprintf(bf, "%f %f translate", (float)x, (float)y);
    out.Writeln(bf);
  }
  //..........................................................................
  template <typename float_t> 
  void lineWidth(const float_t& lw)  {
    if( olx_abs(lw-CurrentLineWidth) < 1e-6 )  return;
    psw_sprintf(bf, "%f setlinewidth", (float)lw);
    out.Writeln(bf);
    CurrentLineWidth = (float)lw;
  }
  //..........................................................................
  template <typename vec_t> 
  void scale(const vec_t& origin)  {
    psw_sprintf(bf, "%f %f scale", (float)origin[0], (float)origin[1]);
    out.Writeln(bf);
  }
  //..........................................................................
  template <typename float_t> 
  void scale(const float_t& x_scale, const float_t& y_scale)  {
    psw_sprintf(bf, "%f %f scale", (float)x_scale, (float)y_scale);
    out.Writeln(bf);
  }
  //..........................................................................
  void custom(const char* cmd)  {  out.Writeln(cmd);  }
  //..........................................................................
  void custom(const olxstr& cmd)  {  out.Writeln(cmd.c_str());  }
  //..........................................................................
  template <class List> void custom(const List& cmds)  {
    for( size_t i=0; i < cmds.Count(); i++ )
      out.Writeln(cmds[i].c_str());
  }
  //..........................................................................
  void newPath()  {  out.Writeln("newpath");  }
  //..........................................................................
  void closePath()  {  out.Writeln("closepath");  }
  //..........................................................................
  void stroke()  {  out.Writeln("stroke");  }
  //..........................................................................
  void fill()  {  out.Writeln("fill");  }
  //..........................................................................
  void gsave()  {  out.Writeln("gsave");  }
  //..........................................................................
  void grestore()  {  out.Writeln("grestore");  }
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
    psw_sprintf(bf, "%f %f moveto", (float)to[0], (float)to[1]);
    out.Writeln(bf);
  }
  template <typename float_t> 
  void moveto(const float_t& x, const float_t& y)  {
    psw_sprintf(bf, "%f %f moveto", (float)x, (float)y);
    out.Writeln(bf);
  }
  //..........................................................................
  template <typename vec_t> 
  void lineto(const vec_t& to)  {
    psw_sprintf(bf, "%f %f lineto", (float)to[0], (float)to[1]);
    out.Writeln(bf);
  }
  template <typename float_t> 
  void lineto(const float_t& x, const float_t& y)  {
    psw_sprintf(bf, "%f %f lineto", (float)x, (float)y);
    out.Writeln(bf);
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
  void lines(const list_t& list, size_t cnt = InvalidSize,
    bool close_path=false)
  {
    if( cnt == InvalidSize )  cnt = list.Count();
    if( cnt < 2 )  return;
    moveto(list[0]);
    for( size_t i=1; i < cnt; i++ )
      lineto(list[i]);
    if( close_path )
      lineto(list[0]);
  }
  template <typename list_t> 
  void drawLines(const list_t& list, size_t cnt = InvalidSize, bool join=false,
    RenderFunc rf = &PSWriter::stroke)
  {
    newPath();
    lines(list, cnt, join);
    (this->*rf)();
  }
  //..........................................................................
  template <typename list_t> 
  void lines_vp(const list_t& list, size_t cnt = InvalidSize,
    bool close_path=false)
  {
    if( cnt == InvalidSize )  cnt = list.Count();
    if( cnt < 2 )  return;
    moveto(*list[0]);
    for( size_t i=1; i < cnt; i++ )
      lineto(*list[i]);
    if( close_path )  
      lineto(*list[0]);
  }
  template <typename list_t> 
  void drawLines_vp(const list_t& list, size_t cnt = InvalidSize,
    bool join=false, RenderFunc rf = &PSWriter::stroke)
  {
    newPath();
    lines_vp(list, cnt, join);
    (this->*rf)();
  }
  //..........................................................................
  template <typename vec_t, typename mat_t> 
  void ellipse(const vec_t& center, const mat_t& basis)  {
    out.Writeln("matrix currentmatrix");
    //a b 0
    //c d 0
    //tx ty 1
    //[a b c d tx ty]
    psw_sprintf(bf, "[%f %f %f %f %f %f] concat", (float)basis[0][0], 
      (float)basis[0][1], 
      (float)basis[1][0],
      (float)basis[1][1],
      (float)center[0], 
      (float)center[1] 
    );
    out.Writeln(bf);
    out.Writeln( "0 0 1 0 360 arc");
    out.Writeln("setmatrix");
  }
  template <typename vec_t, typename mat_t> 
  void drawEllipse(const vec_t& center, const mat_t& basis,
    RenderFunc rf = &PSWriter::stroke)
  {
    newPath();
    ellipse(center, basis);
    (this->*rf)();
  }
  //..........................................................................
  template <typename vec_t, typename float_t> 
  void circle(const vec_t& center, const float_t& rad)  {
    psw_sprintf(bf, "%f %f %f 0 360 arc", (float)center[0], 
      (float)center[1], 
      (float)rad 
    );
    out.Writeln(bf);
  }
  template <typename vec_t, typename float_t> 
  void drawCircle(const vec_t& center, const float_t& rad,
    RenderFunc rf=&PSWriter::stroke)
  {
    newPath();
    circle(center, rad);
    (this->*rf)();
  }
  //..........................................................................
  template <class vec_t>
  void drawText(const olxstr& text, const vec_t& pos)  {
    moveto(pos);
    psw_sprintf(bf, "(%s) show", text.c_str());
    out.Writeln(bf);
  }
  //..........................................................................
  template <class vec_t>
  void quad(const vec_t& p1, const vec_t& p2, const vec_t& p3,
    const vec_t& p4)
  {
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
  /*draws a cone using sidea and sideb points and calling func after every
  quadraterial
  */
  template <class vec_lt>
  void drawQuads(const vec_lt& sidea, const vec_lt& sideb, RenderFunc func)  {
    if (sidea.Count() != sideb.Count())
      throw TFunctionFailedException(__OlxSourceInfo, "lists mismatch");
    if (sidea.Count() < 2) return;
    for (size_t j = 1; j < sidea.Count(); j++) {
      newPath();
      quad(sidea[j-1], sideb[j-1], sideb[j], sidea[j]);
      (this->*func)();
    }
    newPath();
    quad(sidea.GetLast(), sideb.GetLast(), sideb[0], sidea[0]);
    (this->*func)();
  }
  //..........................................................................
  /*draws a cone using sidea and sideb points and calling func after every
  quadraterial
  */
  template <class vec_lt>
  void drawOuterQuads(const vec_lt& sidea, const vec_lt& sideb, RenderFunc func)  {
    if (sidea.Count() != sideb.Count())
      throw TFunctionFailedException(__OlxSourceInfo, "lists mismatch");
    if (sidea.Count() < 2) return;
    vec3f ca, cb;
    for (size_t j = 0; j < sidea.Count(); j++) {
      ca += sidea[j];
      cb += sideb[j];
    }
    ca /= sidea.Count();
    cb /= sidea.Count();
    vec3f n = (cb - ca).XProdVec(vec3f(0, 0, 1));
    float nl = n.Length();
    if (nl > 0) {
      n *= 1.0f / nl;
      size_t tl = 0, tr = 0, bl = 0, br = 0;
      float mtl = 0, mtr = 0, mbl = 0, mbr = 0;
      for (size_t j = 0; j < sidea.Count(); j++) {
        float v = sidea[j].DotProd(n);
        if (v < 0) {
          if (-v > mtl) {
            tl = j;
            mtl = -v;
          }
        }
        else {
          if (v > mtr) {
            tr = j;
            mtr = v;
          }
        }
        v = sideb[j].DotProd(n);
        if (v < 0) {
          if (-v > mbl) {
            bl = j;
            mbl = -v;
          }
        }
        else {
          if (v > mbr) {
            br = j;
            mbr = v;
          }
        }
      }
      newPath();
      quad(sidea[tl], sidea[tl] + (sidea[tr] - sidea[tl]) / 4,
        sideb[bl] + (sideb[br] - sideb[bl]) / 4, sideb[bl]);
      (this->*func)();
      newPath();
      quad(sidea[tr] - (sidea[tr] - sidea[tl]) / 4, sidea[tr],
        sideb[br], sideb[br] - (sideb[br] - sideb[bl]) / 4);
      (this->*func)();
    }
  }
  //..........................................................................
  template <class vec_lt>
  void drawQuadsBiColored(const vec_lt& sidea, const vec_lt& sideb,
    RenderFunc func, uint32_t cl1, uint32_t cl2)
  {
    if( sidea.Count() != sideb.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, "lists mismatch");
    if( sidea.Count() < 2 )  return;
    color(cl1);
    for( size_t j=1; j < sidea.Count(); j++ )  {
      newPath();
      quad(sidea[j-1], (sideb[j-1]+sidea[j-1])/2, (sideb[j]+sidea[j])/2,
        sidea[j]);
      (this->*func)();
    }
    newPath();
    quad(sidea.GetLast(), (sideb.GetLast()+sidea.GetLast())/2,
      (sideb[0]+sidea[0])/2, sidea[0]);
    (this->*func)();
    color(cl2);
    for( size_t j=1; j < sidea.Count(); j++ )  {
      newPath();
      quad((sidea[j-1]+sideb[j-1])/2, sideb[j-1], sideb[j],
        (sidea[j]+sideb[j])/2);
      (this->*func)();
    }
    newPath();
    quad((sidea.GetLast()+sideb.GetLast())/2, sideb.GetLast(), sideb[0],
      (sidea[0]+sideb[0])/2);
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
    stippledQuad(sidea.GetLast(), sideb.GetLast(), sideb[0], sidea[0], parts,
      func);
  }
  //..........................................................................
  template <typename vec_t, typename float_t>
  void arc(const vec_t& center, const float_t& rad, const float_t startAngle,
    const float_t& endAngle)
  {
    psw_sprintf(bf, "%f %f %f %f %f arc", (float)center[0], (float)center[1],
      (float)rad, (float)startAngle, (float)endAngle);
    out.Writeln(bf);
  }
  //..........................................................................
  void writeBoundingBox(const evecf &b)  {
    if( !olx_is_valid_index(bounding_box_pos) )  {
      throw TFunctionFailedException(__OlxSourceInfo,
        "bounding box space must be reserved");
    }
    out.SetPosition(bounding_box_pos);
    psw_sprintf(bf, "%ld %ld %ld %ld",
      olx_round(b[0]),
      olx_round(b[1]),
      olx_round(b[2]),
      olx_round(b[3]));
    out.Writeln(bf);
  }
};

#endif
