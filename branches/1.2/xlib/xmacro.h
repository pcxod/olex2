/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_xmacro_H
#define __olx_xlib_xmacro_H
#include "library.h"
#include "xapp.h"
#include "symmlib.h"
#include <stdarg.h>
// on Linux its is defined as something...
#undef QLength

//simply a macro registry
class XLibMacros : public IEObject {
protected:
  static bool ParseResParam(TStrObjList &Cmds, double& esd, double* len=NULL,
    double* len1=NULL, double* ang=NULL);
  struct MacroInput {
    ConstPtrList<TSAtom> atoms;
    ConstPtrList<TSBond> bonds;
    ConstPtrList<TSPlane> planes;
    MacroInput(TSAtomPList &a, TSBondPList &b, TSPlanePList &p)
      : atoms(a), bonds(b), planes(p)
    {}
  };
  static MacroInput ExtractSelection(const TStrObjList &Cmds,
    bool unselect);
public:
  static DefMacro(Run)
  static DefMacro(HklBrush)
  static DefMacro(HklStat)
  static DefMacro(HklMerge)
  static DefMacro(HklAppend)
  static DefMacro(HklExclude)
  static DefMacro(HklImport)
  static DefMacro(HklSplit)
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
  static DefMacro(PiSig)
  static DefMacro(Split)
  static DefMacro(Bang)

  static DefMacro(Flush)

  static DefMacro(SGS)  // SG settings
  static DefMacro(Inv)
  static DefMacro(Push)
  static DefMacro(Transform)
  static DefMacro(Standardise)

  static olxstr CifResolve(const olxstr& func);
  static bool ProcessExternalFunction(olxstr& func);
  static DefMacro(CifExtract)
  static DefMacro(CifMerge)
  static DefMacro(Cif2Tab)
  static DefMacro(Cif2Doc)
  static DefMacro(CifCreate)
  static DefMacro(FcfCreate)

  static DefMacro(CalcCHN)
  static DefMacro(CalcMass)
  static DefMacro(FitCHN)
  static DefMacro(Tolman)
  static DefFunc(VVol)
  static DefMacro(CalcVol)

  static DefMacro(Close)
  static DefMacro(Omit)
  static DefMacro(MolInfo)
  static DefMacro(RTab)
  static DefMacro(Update)
  static DefMacro(Move)
  static DefMacro(Fvar)
  static DefMacro(Sump)
  static DefMacro(Afix)
  static DefMacro(Part)
  static DefMacro(Spec)

  static DefMacro(Dfix)
  static DefMacro(Dang)
  static DefMacro(Tria)
  static DefMacro(Sadi)
  static DefMacro(RRings)
  static DefMacro(Flat)
  static DefMacro(Chiv)
  static DefMacro(SIMU)
  static DefMacro(DELU)
  static DefMacro(ISOR)
  static DefMacro(Same)
  static DefMacro(RIGU)
  static DefMacro(Delta)
  static DefMacro(DeltaI)
  static DefMacro(AddBond)
  static DefMacro(DelBond)
  static DefMacro(RESI)
  static DefMacro(Restrain)
  static DefMacro(Constrain)
  static DefMacro(Conn)
  static DefMacro(TLS)
  static DefMacro(Export)
  static DefMacro(Sgen)
  static DefMacro(LstSymm)
  static DefMacro(RSA)
  static DefMacro(CONF)
  static DefMacro(D2CG)
  static DefMacro(TestR)
  static DefMacro(CalcVars)
  static DefMacro(Pack)
  static DefMacro(Grow)

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
  static DefFunc(Cell)
  static DefFunc(Cif)
  static DefFunc(P4p)
  static DefFunc(Crs)
  static DefFunc(Env)

  static DefFunc(BaseDir)
  static DefFunc(DataDir)
  static DefFunc(HKLSrc)

  static DefFunc(LSM)
  static DefFunc(SSM)
  static DefFunc(SG)
  static DefFunc(SGS)  // SG settings

  static DefFunc(ATA) // atom type assigmnet
  static DefFunc(VSS) // validate structure or a solution, better the analyse!
  static DefFunc(FATA) // atom type assignment from Fourier map

  static DefFunc(RemoveSE) // SG modification
  static DefFunc(Run)

  static DefFunc(CCrd) // cell coordinates of atoms
  static DefFunc(Crd) // cartesian coordinates of atoms
  static DefFunc(CalcR) // calculates R factors
  static DefFunc(GetCompilationInfo)
  static DefFunc(SGList)
  static DefFunc(HKLF)
  static DefFunc(StrDir)

  static void ChangeCell(const mat3d& tm, const TSpaceGroup& sg,
    const olxstr& resHKL_FN);
  static const olxstr &NAString() {
    static olxstr NAString("n/a");
    return NAString;
  }
  static const olxstr &NoneString() {
    static olxstr NoneString("none");
    return NoneString;
  }
  static olxstr& CurrentDir() {
    static olxstr cd;
    return cd;
  }
  /* finds numbers and removes them from the list and returns the number of
  found numbers
  */
  template <class list>
  static size_t Parse(list& Cmds, const char* format, ...) {
    size_t cnt=0;
    va_list argptr;
    va_start(argptr, format);
    try  {
      const size_t len = olxstr::o_strlen(format);
      for( size_t i=0, j=0; i < len; i++, j++ )  {
        while (j < Cmds.Count() && !Cmds[j].IsNumber())
          j++;
        if (j>=Cmds.Count()) break;
        if( format[i] == 'v' )  {
          if( len < (i+1) || Cmds.Count() < (j+3) ) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "invalid format");
          }
          if( format[++i] == 'i' )  {
            vec3i* iv = va_arg(argptr, vec3i*);
            for( int k=0; k < 3; k++ ) (*iv)[k] = Cmds[j+k].ToInt();
          }
          else if( format[i] == 'd' )  {
            vec3d* dv = va_arg(argptr, vec3d*);
            for( int k=0; k < 3; k++ ) (*dv)[k] = Cmds[j+k].ToDouble();
          }
          else {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "undefined vector type");
          }
          Cmds.DeleteRange(j--, 3);
          cnt++;
        }
        else if( format[i] == 'm' )  {
          if( len < (i+1) || Cmds.Count() < (j+9) ) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "invalid format");
          }
          if( format[++i] == 'i' )  {
            mat3i* im = va_arg(argptr, mat3i*);
            for( int k=0; k < 3; k++ )
              for( int l=0; l < 3; l++ )
                (*im)[k][l] = Cmds[j+k*3+l].ToInt();
          }
          else if( format[i] == 'd' )  {
            mat3d* dm = va_arg(argptr, mat3d*);
            for( int k=0; k < 3; k++ )
              for( int l=0; l < 3; l++ )
                (*dm)[k][l] = Cmds[j+k*3+l].ToDouble();
          }
          else {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "undefined matrix type");
          }
          Cmds.DeleteRange(j--, 9);
          cnt++;
        }
        else if( format[i] == 'i' )  {
          int* iv = va_arg(argptr, int*);
          *iv = Cmds[j].ToInt();
          Cmds.Delete(j--);
          cnt++;
        }
        else if( format[i] == 'd' )  {
          double *dv = va_arg(argptr, double*);
          *dv = Cmds[j].ToDouble();
          Cmds.Delete(j--);
          cnt++;
        }
      }
    }
    catch(const TExceptionBase &e)  {
      va_end(argptr);
      throw TFunctionFailedException(__OlxSourceInfo, e);
    }
    va_end(argptr);
    return cnt;
  }
  /* finds numbers and removes them from the list and returns the number of
  found numbers
  */
  template <class list>
  static size_t ParseOnly(const list& Cmds, const char *format, ...) {
    size_t cnt=0;
    va_list argptr;
    va_start(argptr, format);
    try  {
      const size_t len = olxstr::o_strlen(format);
      for( size_t i=0, j=0; i < len; i++, j++ )  {
        while (j < Cmds.Count() && !Cmds[j].IsNumber())
          j++;
        if (j>=Cmds.Count()) break;
        if( format[i] == 'v' )  {
          if( len < (i+1) || Cmds.Count() < (j+3) ) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "invalid format");
          }
          if( format[++i] == 'i' )  {
            vec3i* iv = va_arg(argptr, vec3i*);
            for( int k=0; k < 3; k++ ) (*iv)[k] = Cmds[j+k].ToInt();
          }
          else if( format[i] == 'd' )  {
            vec3d* dv = va_arg(argptr, vec3d*);
            for( int k=0; k < 3; k++ ) (*dv)[k] = Cmds[j+k].ToDouble();
          }
          else {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "undefined vector type");
          }
          j+=2;
          cnt++;
        }
        else if( format[i] == 'm' )  {
          if( len < (i+1) || Cmds.Count() < (j+9) ) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "invalid format");
          }
          if( format[++i] == 'i' )  {
            mat3i* im = va_arg(argptr, mat3i*);
            for( int k=0; k < 3; k++ )
              for( int l=0; l < 3; l++ )
                (*im)[k][l] = Cmds[j+k*3+l].ToInt();
          }
          else if( format[i] == 'd' )  {
            mat3d* dm = va_arg(argptr, mat3d*);
            for( int k=0; k < 3; k++ )
              for( int l=0; l < 3; l++ )
                (*dm)[k][l] = Cmds[j+k*3+l].ToDouble();
          }
          else {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "undefined matrix type");
          }
          j+=8;
          cnt++;
        }
        else if( format[i] == 'i' )  {
          int* iv = va_arg(argptr, int*);
          *iv = Cmds[j].ToInt();
          cnt++;
        }
        else if( format[i] == 'd' )  {
          double *dv = va_arg(argptr, double*);
          *dv = Cmds[j].ToDouble();
          cnt++;
        }
      }
    }
    catch(const TExceptionBase &e)  {
      va_end(argptr);
      throw TFunctionFailedException(__OlxSourceInfo, e);
    }
    va_end(argptr);
    return cnt;
  }
  static void Export(class TLibrary& lib);

  static TActionQList &Actions() {
    static TActionQList Actions_;
    return Actions_;
  }
  static TActionQueue &OnDelIns() {
    static TActionQueue *OnDelIns=NULL;
    if (OnDelIns == NULL) {
      OnDelIns = &XLibMacros::Actions().New("OnDelIns");
    }
    return *OnDelIns;
  }
  static TActionQueue &OnAddIns() {
    static TActionQueue *OnAddIns=NULL;
    if (OnAddIns == NULL) {
      OnAddIns = &XLibMacros::Actions().New("OnAddIns");
    }
    return *OnAddIns;
  }
  static olxstr VarName_ResetLock()  {  return "olx_lock_reset";  }
  static olxstr VarName_InternalTref()  {  return "olx_internal_tref";  }
  static olxstr GetCompilationInfo();
protected:
  class TEnviComparator {
  public:
    template <class item_a_t, class item_b_t>
    int Compare(item_a_t const& i1, item_b_t const& i2) const
    {
      return olx_cmp(olx_ref::get(i1).GetC().QLength(),
        olx_ref::get(i2).GetC().QLength());
    }
  };
};
//---------------------------------------------------------------------------
#endif
