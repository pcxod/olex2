#ifndef __olx_xlib_Hall_H
#define __olx_xlib_Hall_H
#include "rot_id.h"
#include "edict.h"
#include "symmat.h"

BeginXlibNamespace()

struct HallSymbol  {
protected:
  struct symop {
    int rot_id;
    vec3d t;
  };
  static void init();
  static char LattSymbols[];  // [abs(latt)-1]
  static TTypeList<AnAssociation2<vec3d, olxstr> > trans;
  //const TTypeList<AnAssociation2<mat3d, olxstr> > rotations;
  static TTypeList<AnAssociation2<int,olxstr> >
    rotx, roty, rotz, rotx1, roty1, rotz1, rot3;
  static char GetLatticeSymbol(int latt)  {
    const unsigned al = olx_abs(latt);
    if( al > 7 )
      throw TInvalidArgumentException(__OlxSourceInfo, "latt");
    return LattSymbols[al-1];
  }
  static olxstr FindT(const vec3d& t, int order);
  static olxstr FindTR(const vec3d& t, int order);
  static int FindR(olxstr& hs, TTypeList<symop>& matrs,
    const TTypeList<AnAssociation2<int,olxstr> >& rot, bool full);
public:
  static olxstr Evaluate(int latt, const smatd_list& matrices);
  static olxstr FindCentering(smatd_list& matrices);
};

EndXlibNamespace()
#endif
