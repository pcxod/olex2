#ifndef xmacroH
#define xmacroH
#include "library.h"
#include "xapp.h"
#include "cif.h"
#include "symmlib.h"
#include <stdarg.h>
// on Linux its is defined as something...
#undef QLength

//simply a macro registry

class XLibMacros  {
  static DefMacro(BrushHkl)
  static DefMacro(SG)
  static DefMacro(SGE)
  static DefMacro(GraphSR)
  static DefMacro(GraphPD)
  static DefMacro(Wilson)
  static DefMacro(TestSymm)
  static DefMacro(VATA) // validate atom type assignment
  static DefMacro(Clean)
  static DefMacro(AtomInfo)
  static DefMacro(Compaq)
  static DefMacro(Envi)
  static DefMacro(AddSE)
  static DefMacro(Fuse)
  static DefMacro(EXYZ)
  static DefMacro(EADP)
  static DefMacro(VoidE)
  static DefMacro(ChangeSG)
  static DefMacro(FixUnit)
  static DefMacro(LstIns)
  static DefMacro(DelIns)
  static DefMacro(AddIns)
  static DefMacro(Htab)
  static DefMacro(HAdd)
  static DefMacro(HImp)
  static DefMacro(LS)
  static DefMacro(Plan)
  static DefMacro(UpdateWght)
  static DefMacro(Anis)
  static DefMacro(Isot)
  static DefMacro(FixHL)
  static DefMacro(Fix)
  static DefMacro(Free)
  static DefMacro(File)
  static DefMacro(User)
  static DefMacro(Dir)
  static DefMacro(LstMac)
  static DefMacro(LstFun)
  static DefMacro(LstVar)
  static DefMacro(LstFS)
  static DefMacro(ASR)

  static DefMacro(Flush)

  static DefMacro(SGS)  // SG settings

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
  static DefFunc(Title)
  static DefFunc(IsFileLoaded)
  static DefFunc(IsFileType)
  static DefFunc(Ins)

  static DefFunc(BaseDir)
  static DefFunc(HKLSrc)

  static DefFunc(LSM)
  static DefFunc(SSM)
  static DefFunc(SG)
  static DefFunc(SGS)  // SG settings

  static DefFunc(ATA) // atom type assigmnet
  static DefFunc(VSS) // validate structure or a solution, better the analyse!
  static DefFunc(FATA) // atom type assignment from Fourier map

  static DefFunc(RemoveSE) // SG modification

  static TActionQList Actions;
  static void ChangeCell(const mat3d& tm, const TSpaceGroup& sg);
public:
  static const olxstr NoneString;
  static const olxstr NAString;
  static olxstr CurrentDir;
  // finds numbers and removes them from the list and returns the number of found numbers
  template <typename nt> static int ParseNumbers(TStrObjList& Cmds, int cnt, ...)  {
    va_list argptr;
    va_start(argptr, cnt);
    if( cnt <= 0 )  {  va_end(argptr);  return 0;  }
    int fc=0;
    for( int i=0; i < Cmds.Count(); i++ )  {
      if( Cmds[i].IsNumber() )  {
        *va_arg(argptr, nt*) = (nt)Cmds[i].ToDouble();
        Cmds.Delete(i);
        fc++;
        if( fc >= cnt )  break;
        i--;
      }
    }
    va_end(argptr);
    return fc;
  }
  static void Export(class TLibrary& lib);
  static TActionQueue* OnDelIns;

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
