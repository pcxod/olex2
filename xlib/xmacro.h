#ifndef xmacroH
#define xmacroH
#include "library.h"
#include "xapp.h"
#include "cif.h"

//simply a macro registry

class XLibMacros  {
  static DefMacro(BrushHkl)
  static DefMacro(SG)
  static DefMacro(GraphSR)
  static DefMacro(Wilson)
  static DefMacro(TestSymm)
  static DefMacro(VATA) // validate atom type assignment
  static DefMacro(Clean)
  static DefMacro(AtomInfo)
  static DefMacro(Compaq)
  static DefMacro(Envi)
  static DefMacro(AddSE)
  static DefMacro(Fuse)
  static DefMacro(VoidE)
  static DefMacro(ChangeSG)

  static void MergePublTableData(TCifLoopTable& to, TCifLoopTable& from); // helper function
  static olxstr CifResolve(const olxstr& func);
  static bool ProcessExternalFunction(olxstr& func);
  static DefMacro(CifExtract)
  static DefMacro(CifMerge)
  static DefMacro(Cif2Tab)
  static DefMacro(Cif2Doc)

  static DefFunc(FileName)
  static DefFunc(FileExt)
  static DefFunc(FilePath)
  static DefFunc(FileFull)
  static DefFunc(FileDrive)
  static DefFunc(IsFileLoaded)
  static DefFunc(IsFileType)

  static DefFunc(BaseDir)
  static DefFunc(HKLSrc)

  static DefFunc(ATA) // atom type assigmnet
  static DefFunc(VSS) // validate structure or a solution, better the analyse!

  static DefFunc(RemoveSE) // SG modification

public:
  static const olxstr NoneString;
  // finds numbers and removes them from the list
  static void ParseDoubles(TStrObjList& Cmds, int number, ...);
  static void Export(class TLibrary& lib);

protected:
  class TEnviComparator  {
  public:
    static int Compare(AnAssociation3<TCAtom*, vec3d, smatd> const& i1, 
      AnAssociation3<TCAtom*, vec3d, smatd> const& i2)  {
        double res = i1.GetB().QLength() - i2.GetB().QLength();
        if( res < 0 ) return -1;
        return (res > 0) ? 1 : 0;
    }
  };

};
//---------------------------------------------------------------------------
#endif
 