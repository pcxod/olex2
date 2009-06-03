/* A simple postscript file interface 
(c) Oleg Dolomanov, 2009
*/
#ifndef _olx_tex_writerH
#define _olx_tex_writerH
#include "efile.h"
#include "glbase.h"
#include "threex3.h"

class TEXWriter  {
  TEFile out;
  char bf[80];
  uint32_t CurrentColor;
  float CurrentLineWidth;
public:
  typedef void (TEXWriter::*RenderFunc)();
  TEXWriter(const olxstr& fileName) {
    CurrentColor = 0;
    CurrentLineWidth = 1;
    out.Open(fileName, "w+b");
      
    out.Writenl("\\documentclass[a4paper,10pt]{article}");
    out.Writenl("\\usepackage{color}");
    out.Writenl("\\usepackage{graphicx}");
    out.Writenl("\\usepackage{tikz}");
    out.Writenl("\\usetikzlibrary{trees}");
    out.Writenl("\\usetikzlibrary{shapes}");
    out.Writenl("\\usetikzlibrary{patterns}");
    out.Writenl("\\usetikzlibrary{calc,through,backgrounds}");
    out.Writenl("\\usetikzlibrary{arrows,decorations.pathmorphing,backgrounds,positioning,fit}");
    out.Writenl("\\usepackage[english]{babel}");
    //out.Writenl("\\begin{document}");
    out.Writenl("");
    //white border width around elements
    out.Writenl("\\newcommand{\\whitespace}{1pt}");
    //radius for the end of the bond
    out.Writenl("\\newcommand{\\cornerradius}{0.6mm}");
    out.Writenl("");   
  }
  //..........................................................................
  void Writenl(char bf[250])  {
    out.Writenl( bf );
  }  
};

#endif
