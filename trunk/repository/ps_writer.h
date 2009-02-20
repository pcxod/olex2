#ifndef _olx_ps_writerH
#define _olx_ps_writerH
#include "efile.h"

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
  template <typename vec_t> 
  void line(const vec_t& from, const vec_t& to)  {
    out.Writenl("newpath");
    sprintf(bf, "%f %f moveto", (float)from[0], (float)from[1]);
    out.Writenl( bf, strlen(bf) );
    sprintf(bf, "%f %f lineto", (float)to[0], (float)to[1]);
    out.Writenl( bf, strlen(bf) );
    out.Writenl("stroke");
  }
  template <typename list_t> 
  void lines(const list_t& list, bool join)  {
    if( list.Count() < 2 )  return;
    out.Writenl("newpath");
    sprintf(bf, "%f %f moveto", (float)list[0][0], (float)list[0][1]);
    out.Writenl( bf, strlen(bf) );
    for( int i=1; i < list.Count(); i++ )  {
      sprintf(bf, "%f %f lineto", (float)list[i][0], (float)list[i][1]);
      out.Writenl( bf, strlen(bf) );
    }
    if( join && list.Count() > 2 )  {
      sprintf(bf, "%f %f lineto", (float)list[i][0], (float)list[i][1]);
      out.Writenl( bf, strlen(bf) );
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
    out.Writenl( bf, strlen(bf) );
    out.Writenl( "0 0 1 0 360 arc");
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
    out.Writenl( bf, strlen(bf) );
    out.Writenl("stroke");
  }
};

#endif
