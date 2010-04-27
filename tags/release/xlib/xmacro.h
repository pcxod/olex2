#ifndef __olx_xlib_xmacro_H
#define __olx_xlib_xmacro_H
#include "library.h"
#include "xapp.h"
#include "cif.h"
#include "symmlib.h"
#include <stdarg.h>
// on Linux its is defined as something...
#undef QLength

//simply a macro registry

class XLibMacros  {
  static DefMacro(Run)
  static DefMacro(BrushHkl)
  static DefMacro(HklStat)
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
  static DefMacro(GenDisp)
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
  static DefMacro(Reset)
  static DefMacro(Degen)
  static DefMacro(User)
  static DefMacro(Dir)
  static DefMacro(LstMac)
  static DefMacro(LstFun)
  static DefMacro(LstVar)
  static DefMacro(LstFS)
  static DefMacro(ASR)
  static DefMacro(Describe)
  static DefMacro(Sort)
  static DefMacro(SGInfo)
  static DefMacro(SAInfo)
  static DefMacro(PiPi)

  static DefMacro(Flush)

  static DefMacro(SGS)  // SG settings
  static DefMacro(Inv)  
  static DefMacro(Push)  
  static DefMacro(Transform)  
  static DefMacro(Standardise)  

  static void MergePublTableData(TCifLoopTable& to, TCifLoopTable& from); // helper function
  static olxstr CifResolve(const olxstr& func);
  static bool ProcessExternalFunction(olxstr& func);
  static DefMacro(CifExtract)
  static DefMacro(CifMerge)
  static DefMacro(Cif2Tab)
  static DefMacro(Cif2Doc)
  static DefMacro(CifCreate)

  static DefMacro(CalcCHN)
  static DefMacro(CalcMass)
  static DefMacro(FitCHN)

  static DefMacro(Close)
  static DefMacro(Omit)
  static DefFunc(Lst)

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
  static DefFunc(Run) // SG modification

  static TActionQList Actions;
  static void ChangeCell(const mat3d& tm, const TSpaceGroup& sg, const olxstr& resHKL_FN);
public:
  static const olxstr NoneString;
  static const olxstr NAString;
  static olxstr CurrentDir;
  // finds numbers and removes them from the list and returns the number of found numbers
  template <typename nt, class list> static size_t ParseNumbers(list& Cmds, size_t cnt, ...)  {
    va_list argptr;
    va_start(argptr, cnt);
    if( cnt == 0 )  {  va_end(argptr);  return 0;  }
    size_t fc=0;
    for( size_t i=0; i < Cmds.Count(); i++ )  {
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
  // finds numbers in the list and returns the number of found numbers
  template <typename nt, class list> static size_t ParseOnlyNumbers(const list& Cmds, size_t cnt, int from=0, ...)  {
    va_list argptr;
    va_start(argptr, from);
    if( cnt <= 0 )  {  va_end(argptr);  return 0;  }
    size_t fc=0;
    for( size_t i=from; i < Cmds.Count(); i++ )  {
      *va_arg(argptr, nt*) = (nt)Cmds[i].ToDouble();
      fc++;
      if( fc >= cnt )  break;
    }
    va_end(argptr);
    return fc;
  }
  // finds numbers in the list and returns the number of found numbers
  template <class list> static bool Parse(list& Cmds, const olxstr& format, bool remove, ...)  {
    va_list argptr;
    va_start(argptr, remove);
    try  {
      for( size_t i=0, j=0; i < format.Length(); i++, j++ )  {
        if( format.CharAt(i) == 'v' )  {
          if( format.Length() < (i+1) || Cmds.Count() < (j+3) )
            throw TInvalidArgumentException(__OlxSourceInfo, "invalid format");
          if( format.CharAt(++i) == 'i' )  {
            vec3i* iv = va_arg(argptr, vec3i*);
            for( int k=0; k < 3; k++ )
              (*iv)[k] = Cmds[j+k].ToInt();
            if( remove )  {
              Cmds.DeleteRange(j, j+3);
              j--;
            }
            else
              j+=2;
          }
          else if( format.CharAt(i) == 'd' )  {
            vec3d* dv = va_arg(argptr, vec3d*);
            for( int k=0; k < 3; k++ )
              (*dv)[k] = Cmds[j+k].ToDouble();
            if( remove )  {
              Cmds.DeleteRange(j, j+3);
              j--;
            }
            else
              j+=2;
          }
          else 
            throw TInvalidArgumentException(__OlxSourceInfo, "undefined vector type");
        }
        else if( format.CharAt(i) == 'm' )  {
          if( format.Length() < (i+1) || Cmds.Count() < (j+9) )
            throw TInvalidArgumentException(__OlxSourceInfo, "invalid format");
          if( format.CharAt(++i) == 'i' )  {
            mat3i* im = va_arg(argptr, mat3i*);
            for( int k=0; k < 3; k++ )
              for( int l=0; l < 3; l++ )
                (*im)[k][l] = Cmds[j+k*3+l].ToInt();
            if( remove )  {
              Cmds.DeleteRange(j, j+9);
              j--;
            }
            else
              j+=8;
          }
          else if( format.CharAt(i) == 'd' )  {
            mat3d* dm = va_arg(argptr, mat3d*);
            for( int k=0; k < 3; k++ )
              for( int l=0; l < 3; l++ )
                (*dm)[k][l] = Cmds[j+k*3+l].ToDouble();
            if( remove )  {
              Cmds.DeleteRange(j, j+9);
              j--;
            }
            else
              j+=8;
          }
          else 
            throw TInvalidArgumentException(__OlxSourceInfo, "undefined matrix type");
        }
        else if( format.CharAt(i) == 'i' )  {
          int* iv = va_arg(argptr, int*);
          *iv = Cmds[j].ToInt();
        }
        else if( format.CharAt(i) == 'd' )  {
          double *dv = va_arg(argptr, double*);
          *dv = Cmds[j].ToDouble();
        }
      }
    }
    catch(...)  {
      va_end(argptr);
      return false;
    }
    va_end(argptr);
    return true;
  }
  static void Export(class TLibrary& lib);
  static TActionQueue &OnDelIns, &OnAddIns;

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
