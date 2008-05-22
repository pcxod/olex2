#ifndef xmacroH
#define xmacroH
#include "library.h"
#include "xapp.h"

//simply a macro registry

class XLibMacros  {
  static void macBrushHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E);
  static void macSG(TStrObjList &Cmds, const TParamList &Options, TMacroError &E);
  static void macGraphSR(TStrObjList &Cmds, const TParamList &Options, TMacroError &E);
  static void macWilson(TStrObjList &Cmds, const TParamList &Options, TMacroError &E);
  static void macTestSymm(TStrObjList &Cmds, const TParamList &Options, TMacroError &E);
public:
  static void Export(class TLibrary& lib);
};
//---------------------------------------------------------------------------
#endif
 