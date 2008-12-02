#ifdef __BORLANC__
  #pragma hdrstop
#endif

#include "xmacro.h"
#include "xapp.h"

#include "p4p.h"
#include "mol.h"
#include "crs.h"
#include "ins.h"
#include "cif.h"
#include "hkl.h"

#include "unitcell.h"
#include "symmtest.h"
#include "integration.h"
#include "utf8file.h"
#include "datafile.h"
#include "dataitem.h"
#include "fsext.h"
#include "ecast.h"
#include "xlcongen.h"
#include "bitarray.h"
#include "olxvar.h"
#include "strmask.h"
#include "sgset.h"
#include "sfutil.h"

#define xlib_InitMacro(macroName, validOptions, argc, desc)\
  lib.RegisterStaticMacro( new TStaticMacro(&XLibMacros::mac##macroName, #macroName, (validOptions), argc, desc))
#define xlib_InitFunc(funcName, argc, desc) \
  lib.RegisterStaticFunction( new TStaticFunction(&XLibMacros::fun##funcName, #funcName, argc, desc))

const olxstr XLibMacros::NoneString("none");
const olxstr XLibMacros::NAString("n/a");
olxstr XLibMacros::CurrentDir;
TActionQList XLibMacros::Actions;
TActionQueue* XLibMacros::OnDelIns = &XLibMacros::Actions.NewQueue("OnDelIns");

void XLibMacros::Export(TLibrary& lib)  {
  xlib_InitMacro(BrushHkl, "f-consider Friedel law", fpAny, "for high redundancy\
 data sets, removes equivalents with high sigma");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(SG, "a", fpNone|fpOne, "suggest space group");
  xlib_InitMacro(SGE, "", fpNone|fpOne|psFileLoaded, "Extended spacegroup determination. Internal use" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(GraphSR, "b-number of bins", fpNone|fpOne|psFileLoaded,
"Prints a scale vs resolution graph for current file (fcf file must exist in current folder)");
  xlib_InitMacro(GraphPD, "r-resolution&;fcf-take structure factors from the FCF file, otherwise calculate from curent model\
&;s-use simple scale when calculating structure factors from the mode, otherwise regression scaling will be used", fpNone|psFileLoaded,
"Prints a intensity vs. 2 theta graph");
  xlib_InitMacro(Wilson, "b-number of bins&;p-uses linear vins for picture, otherwise uses spherical bins", 
    fpNone|fpOne|psFileLoaded, "Prints Wilson plot data");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(TestSymm, "e-tolerance limit", fpNone|psFileLoaded, "Tests current \
  structure for missing symmetry");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(VATA, EmptyString, fpAny|psFileLoaded,
"Compares current model with the cif file and write the report to provided file (appending)" );
  xlib_InitMacro(Clean, "npd-does not change atom types of NPD atoms\
&;f-does not run 'fuse' after the completion\
&;aq-disables analysis of the Q-peaks based on thresholds\
&;at-disables lonely atom types assignment to O and Cl", fpNone,
"Tidies up current model" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(AtomInfo, "", fpAny|psFileLoaded,
"Searches information for given atoms in the database" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Compaq, "a-analyse connectivity on atom level, bu default fragment level is used", fpNone|psFileLoaded,
"Moves all atoms or fragments of the asymmetric unit as close to each other as possible." );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Envi, "q-adds Q-peaks to the list&;h-adds hydrogen atoms to the list&;cs-leaves selection unchanged",
    fpNone|fpOne|fpTwo,
"This macro prints environment of any particular atom. Default search radius is 2.7A."  );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(AddSE, "", (fpAny^fpNone)|psFileLoaded,
"Tries to add a new symmetry element to current space group to form a new one. [-1] is for center of symmetry" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Fuse, "f-removes symmetrical equivalents", fpNone|psFileLoaded,
"Re-initialises the connectivity list" );
  xlib_InitMacro(Flush, EmptyString, fpNone|fpOne, "Flushes log streams" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(EXYZ, "", fpAny|psCheckFileTypeIns,
"Shares adds a new element to the give site" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(EADP, "", fpAny|psCheckFileTypeIns,
"Forces EADP/Uiso of provided atoms to be constrained the same" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Cif2Doc, "", fpNone|fpOne|psFileLoaded, "converts cif to a document" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Cif2Tab, "", fpAny|psFileLoaded, "creates a table from a cif" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(CifMerge, "", (fpAny^fpNone)|psFileLoaded,
  "Merges loaded or provided as first argument cif with other cif(s)" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(CifExtract, "", fpTwo|psFileLoaded, "extract a list of items from one cif to another" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(VoidE, "", fpNone|psFileLoaded, "calculates number of electrons in the voids area" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(ChangeSG, "", fpOne|fpFour|psFileLoaded, "[shift] SG Changes space group of current structure" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(Htab, "t-adds extra elements (comma separated -t=Br,I) to the donor list. Defaults are [N,O,F,Cl,S]", fpNone|fpOne|fpTwo|psCheckFileTypeIns, 
    "Adds HTBA instructions to the ins file, maximum bond length [2.9] and minimal angle [150] might be provided" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(HAdd, "", fpAny|psCheckFileTypeIns, "Adds hydrogen atoms to all or provided atoms" );
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(FixUnit,"", fpNone|fpOne|psCheckFileTypeIns, " Sets SFAc and UNIT to current content of the asymmetric unit.\
 Takes Z', with default value of 1.");
//_________________________________________________________________________________________________________________________
  xlib_InitMacro(AddIns,"", (fpAny^fpNone)|psCheckFileTypeIns, "Adds an instruction to the INS file" );
  xlib_InitMacro(DelIns, "", fpOne|psCheckFileTypeIns, "A number or the name (will remove all accurances) can be provided" );
  xlib_InitMacro(LstIns, "", fpNone|psCheckFileTypeIns, "Lists all instructions of currently loaded Ins file" );
  xlib_InitMacro(FixHL, "", fpNone|psFileLoaded, "Fixes hydgrogen atom labels" );
  xlib_InitMacro(Fix, "", (fpAny^(fpNone|fpOne))|psCheckFileTypeIns, "Fixes specified parameters of atoms: XYZ, Uiso, Occu" );
  xlib_InitMacro(Free, "", (fpAny^(fpNone|fpOne))|psCheckFileTypeIns, "Frees specified parameters of atoms: XYZ, Uiso, Occu" );
  xlib_InitMacro(Isot,"" , fpAny|psCheckFileTypeIns,
"makes provided atoms isotropic, if no arguments provieded, current selection all atoms become isotropic");
  xlib_InitMacro(Anis,"h-adds hydrogen atoms" , (fpAny) | psCheckFileTypeIns, 
"makes provided atoms anisotropic if no arguments provided current selection or all atoms are considered" );
xlib_InitMacro(File, "s-sort the main residue of the asymmetric unit", fpNone|fpOne|psFileLoaded, 
    "Saves current model to a file. By default an ins file is saved and loaded" );
  xlib_InitMacro(LS, "", fpOne|fpTwo|psCheckFileTypeIns, "Sets refinement method and/or the number of iterations.");
  xlib_InitMacro(Plan, "", fpOne|psCheckFileTypeIns, "Sets the number of Fuorier peaks to be found from the difference map");
  xlib_InitMacro(UpdateWght, "", fpAny|psCheckFileTypeIns, "Copies proposed weight to current");
  xlib_InitMacro(User, "", fpNone|fpOne, "Changes current folder");
  xlib_InitMacro(Dir, "", fpNone|fpOne, "Lists current folder. A file name mask may be provided");
  xlib_InitMacro(LstVar, "", fpAny, "Lists all defined variables. Accepts * based masks" );
  xlib_InitMacro(LstMac, "h-Shows help", fpAny, "Lists all defined macros. Accepts * based masks" );
  xlib_InitMacro(LstFun, "h-Shows help", fpAny, "Lists all defined functions. Accepts * based masks" );
  xlib_InitMacro(LstFS, EmptyString, fpAny, "Prints out detailed content of virtual file system. Accepts * based masks");
  xlib_InitMacro(SGS, EmptyString, fpOne|fpTwo|psFileLoaded, "Changes current space group settings using provided cell setting (if aplicable) and axis");
//_________________________________________________________________________________________________________________________
//_________________________________________________________________________________________________________________________

  xlib_InitFunc(FileName, fpNone|fpOne,
"If no arguments provided, returns file name of currently loaded file, for one\
 argument returns extracted file name");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(FileExt, fpNone|fpOne, "Retursn file extension. If no arguments provided - of currently loaded file");
  xlib_InitFunc(FilePath, fpNone|fpOne, "Returns file path. If no arguments provided - of currently loaded file");
  xlib_InitFunc(FileFull, fpNone, "Returns full path of currently loaded file");
  xlib_InitFunc(FileDrive, fpNone|fpOne, "Returns file drive. If no arguments provided - of currently loaded file");
  xlib_InitFunc(Title, fpNone|fpOne, "If the file is laoded, returns it's title else if a parameter passed, it is returned");
  xlib_InitFunc(IsFileLoaded, fpNone, "Returns true/false");
  xlib_InitFunc(IsFileType, fpOne, "Checks type of currently loaded file [ins,res,ires,cif,mol,xyz]");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(BaseDir, fpNone|fpOne, "Returns the startup folder");
  xlib_InitFunc(HKLSrc, fpNone|fpOne|psFileLoaded, "Returns/sets hkl source for currently loaded file");
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(LSM, fpNone|psCheckFileTypeIns, "Return current refinement method, L.S. or CGLS currently.");
  xlib_InitFunc(SSM, fpNone|fpOne, "Return current structure solution method, TREF or PATT currently. If current method is unknown\
 and an argument is provided, that argument is returned");
  xlib_InitFunc(Ins, fpOne|psCheckFileTypeIns, "Returns instruction value (all data after the instruction). In case the instruction\
 does not exist it return 'n/a' string");
  xlib_InitFunc(SG, fpNone|fpOne|psFileLoaded, "Returns space group of currently loaded file.\
 Also takes a string template, where %# is replaced with SG number, %n - short name,\
 %N - full name, %h - html representation of the short name, %H - same as %h for full name,\
 %s - syngony, %HS -Hall symbol" );
  xlib_InitFunc(SGS, fpNone|fpOne|psFileLoaded, "Returns current space settings" );
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(ATA, fpAny|psFileLoaded, "Test current structure agains database.\
  (Atom Type Assignment). Returns true if any atom type changed" );
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(FATA, fpAny|psFileLoaded, "Calculates the diff fourier map and integrates it to find artefacts around atoms.\
  (Fouriesr Atom Type Assignment). Returns true if any atom type changed" );
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(VSS, fpOne|psFileLoaded, "Validate Structure or Solution.\
  Takes a boolean value. if value is true, the number of tested atoms is limited\
 by the 18A rule. Returns proportion of know atom types to the all atoms number." );
//_________________________________________________________________________________________________________________________
  xlib_InitFunc(RemoveSE, fpOne|psFileLoaded, "Returns a new space group name without provided element");
//_________________________________________________________________________________________________________________________
}
//..............................................................................
//..............................................................................
void XLibMacros::macHtab(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( TXApp::GetInstance().XFile().GetLattice().IsGenerated() )  {
    E.ProcessingError(__OlxSrcInfo, "operation is not applicable to the grown structure");
  }
  double max_d = 2.9, min_ang = 150.0;
  int cnt = XLibMacros::ParseNumbers<double>(Cmds, 2, &max_d, &min_ang);
  if( cnt == 1 )  {
    if( max_d > 100 )  {
      min_ang = max_d;
      max_d = 2.9;
    }
    else if( max_d > 5 )
      max_d = 2.9;
  }
  TIntList bais;
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  bais.Add(iNitrogenIndex);
  bais.Add(iOxygenIndex);
  bais.Add(iFluorineIndex);
  bais.Add(iChlorineIndex);
  bais.Add(iSulphurIndex);
  TBasicApp::GetLog() << "Processing HTAB with max D-A distance " << max_d << " and minimum angle " << min_ang << '\n';
  min_ang = cos(min_ang*M_PI/180.0);
  if( Options.Contains('t') )  {
    TStrList elm(Options.FindValue('t'), ',');
    for( int i=0; i < elm.Count(); i++ )  {
      TBasicAtomInfo* bai = AtomsInfo.FindAtomInfoBySymbol(elm[i]);
      if( bai == NULL )
        TBasicApp::GetLog() << (olxstr("Unknown element type: ") << elm[i] << '\n');
      else if( bais.IndexOf(bai->GetIndex()) == -1 )
          bais.Add(bai->GetIndex());
    }
  }
  TAsymmUnit& au = TXApp::GetInstance().XFile().GetAsymmUnit();
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  TUnitCell& uc = TXApp::GetInstance().XFile().GetUnitCell();
  TLattice& lat = TXApp::GetInstance().XFile().GetLattice();
  TIns& ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  TArrayList< AnAssociation2<TCAtom const*, smatd> > all;
  int h_indexes[4];
  for( int i=0; i < lat.AtomCount(); i++ )  {
    TSAtom& sa = lat.GetAtom(i);
    TBasicAtomInfo& bai = sa.GetAtomInfo();
    if( bai.GetMr() < 3.5 || bai == iQPeakIndex )  continue;
    int hc = 0;
    for( int j=0; j < sa.NodeCount(); j++ )  {
      TBasicAtomInfo& bai1 = sa.Node(j).GetAtomInfo();
      if( bai1 == iHydrogenIndex || bai1 == iDeuteriumIndex )  {
        h_indexes[hc] = j;
        hc++;
        if( hc >= 4 )
          break;
      }
    }
    if( hc == 0 || hc >= 4 )  continue;
    all.Clear();
    uc.FindInRange(sa.ccrd(), max_d+bai.GetRad1()-0.6, all);
    for( int j=0; j < all.Count(); j++ )  {
      const TCAtom& ca = *all[j].GetA();
      const TBasicAtomInfo& bai1 = ca.GetAtomInfo();
      if(  bais.IndexOf(bai1.GetIndex()) == -1 )  continue;
      vec3d cvec( all[j].GetB()*ca.ccrd() ); 
      vec3d bond( cvec );
      bond -= sa.ccrd();
      au.CellToCartesian(bond);
      double d = bond.Length();
      if( (bai.GetRad1() + bai1.GetRad1() + 0.45) > d )  continue;  // coval bond
      // analyse angles
      for( int k=0; k < hc; k++ )  {
        vec3d base = sa.Node(h_indexes[k]).ccrd();
        vec3d v1 = sa.ccrd() - base;
        vec3d v2 = cvec - base;
        au.CellToCartesian(v1);
        au.CellToCartesian(v2);
        double c_a = v1.CAngle(v2);
        if( c_a < min_ang )  {  // > 150 degrees
          if( sa.GetAtomInfo() == iCarbonIndex )  {
            olxstr rtab_d((const char*)"RTAB ", 80);
            rtab_d << sa.GetAtomInfo().GetSymbol() << ca.GetAtomInfo().GetSymbol() << // codename
              ' ' <<sa.GetLabel() << ' ' << ca.GetLabel();
            const smatd& mt = all[j].GetB();
            if( !(mt.t.IsNull() && mt.r.IsI()) )  {
              const smatd& eqiv = rm.AddUsedSymm(mt);
              int ei = rm.UsedSymmIndex(eqiv);
              rtab_d << "_$" << ei+1;
            }
            ins.AddIns(rtab_d, rm);
            TBasicApp::GetLog() << rtab_d << " d=" << olxstr::FormatFloat(3, d) << '\n';

            olxstr rtab_a((const char*)"RTAB ", 80);
            rtab_a << sa.GetAtomInfo().GetSymbol() << ca.GetAtomInfo().GetSymbol() << // codename
              ' ' << sa.GetLabel() << ' ' << sa.Node(h_indexes[k]).GetLabel() << ' ' << ca.GetLabel();
            if( !(mt.t.IsNull() && mt.r.IsI()) )  {
              const smatd& eqiv = rm.AddUsedSymm(mt);
              int ei = rm.UsedSymmIndex(eqiv);
              rtab_a << "_$" << ei+1;
            }
            ins.AddIns(rtab_a, rm);
            TBasicApp::GetLog() << rtab_a << " a=" << olxstr::FormatFloat(3, acos(c_a)*180.0/M_PI) << '\n';
          }
          else  {
            olxstr htab((const char*)"HTAB ", 80);
            htab << sa.GetLabel() << ' ' << ca.GetLabel();
            const smatd& mt = all[j].GetB();
            if( !(mt.t.IsNull() && mt.r.IsI()) )  {
              const smatd& eqiv = rm.AddUsedSymm(mt);
              int ei = rm.UsedSymmIndex(eqiv);
              htab << "_$" << ei+1;
            }
            ins.AddIns(htab, rm);
            TBasicApp::GetLog() << htab << " d=" << olxstr::FormatFloat(3, d) << '\n';
          }
        }
      }
    }
  }
}
//..............................................................................
void XLibMacros::macHAdd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& XApp = TXApp::GetInstance();
  TSAtomPList satoms;
  XApp.FindSAtoms( Cmds.Text(' '), satoms, true );
  TXlConGen xlcg( XApp.XFile().GetRM() );
  XApp.XFile().GetLattice().AnalyseHAdd( xlcg, satoms );
  XApp.XFile().EndUpdate();
  delete XApp.FixHL();
}
//..............................................................................
void XLibMacros::macAnis(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TSAtomPList atoms;
  if( !TXApp::GetInstance().FindSAtoms(Cmds.Text(' '), atoms, true) )  return;
  TCAtomPList catoms;
  TListCaster::POP(atoms, catoms);
  bool useH = Options.Contains("h");
  for( int i=0; i < catoms.Count(); i++ )
    if( !useH && catoms[i]->GetAtomInfo() == iHydrogenIndex )
      catoms[i] = NULL;
  catoms.Pack();
  TXApp::GetInstance().XFile().GetLattice().SetAnis(catoms, true);
}
//..............................................................................
void XLibMacros::macIsot(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TSAtomPList atoms;
  if( !TXApp::GetInstance().FindSAtoms(Cmds.Text(' '), atoms, true) )  return;
  TCAtomPList catoms;
  TListCaster::POP(atoms, catoms);
  TXApp::GetInstance().XFile().GetLattice().SetAnis(catoms, false);
}
//..............................................................................
void XLibMacros::macFix(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  olxstr vars( Cmds[0] );
  Cmds.Delete(0);
  double var_val = 0;
  if( !Cmds.IsEmpty() && Cmds[0].IsNumber() )  {
    var_val = Cmds[0].ToDouble();
    Cmds.Delete(0);
  }
  TSAtomPList atoms;
  if( !TXApp::GetInstance().FindSAtoms(Cmds.Text(' '), atoms, true) )  return;
  if( vars.Comparei( "XYZ" ) == 0 )  {
    for(int i=0; i < atoms.Count(); i++ )  {
      for( int j=0; j < 3; j++ )
        atoms[i]->CAtom().FixedValues()[TCAtom::CrdFixedValuesOffset + j] = 10;
    }
  }
  else if( vars.Comparei( "UISO" ) == 0 )  {
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->GetEllipsoid() == NULL )  {  // isotropic atom
        if( var_val != 0 )  {
          if( var_val < 10 )  {
            atoms[i]->CAtom().SetUiso( var_val );
            atoms[i]->CAtom().SetUisoVar( 10.0 + var_val );
          }
          else  {
            int iv = (int) var_val;
            atoms[i]->CAtom().SetUiso( var_val-iv );
            atoms[i]->CAtom().SetUisoVar( var_val );
          }
        }
        else  if( atoms[i]->CAtom().GetUisoVar() == 0 )  {  // have to skip riding atoms
          atoms[i]->CAtom().SetUisoVar( 10 );
        }
      }
      else  {
        for( int j=0; j < 6; j++ )
          atoms[i]->CAtom().FixedValues()[TCAtom::UisoFixedValuesOffset + j] = 10;
      }
    }
  }
  else if( vars.Comparei( "OCCU" ) == 0 )  {
    for(int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->CAtom().GetPart() == 0 )  {
        atoms[i]->CAtom().SetOccp( 1./atoms[i]->CAtom().GetDegeneracy() );
        atoms[i]->CAtom().SetOccpVar( 10 );
      }
    }
  }
}
//..............................................................................
void XLibMacros::macFree(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  olxstr vars = Cmds[0];
  Cmds.Delete(0);
  TSAtomPList atoms;
  if( !TXApp::GetInstance().FindSAtoms(Cmds.Text(' '), atoms, true) )  return;
  if( vars.Comparei( "XYZ" ) == 0 )  {
    for(int i=0; i < atoms.Count(); i++ )  {
      for( int j=0; j < 3; j++ )
        atoms[i]->CAtom().FixedValues()[TCAtom::CrdFixedValuesOffset + j] = 0;
    }
  }
  else if( vars.Comparei( "UISO" ) == 0 )  {
    for(int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i]->CAtom().GetEllipsoid() == NULL )  {  // isotropic atom
          atoms[i]->CAtom().SetUisoVar( 0 );
      }
      else  {
        for( int j=0; j < 6; j++ )
          atoms[i]->CAtom().FixedValues()[TCAtom::UisoFixedValuesOffset + j] = 0;
      }
    }
  }
  else if( vars.Comparei( "OCCU" ) == 0 )  {
    for(int i=0; i < atoms.Count(); i++ )  {
      atoms[i]->CAtom().SetOccpVar( 0 );
    }
  }
}
//..............................................................................
void XLibMacros::macFixHL(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  delete TXApp::GetInstance().FixHL();
}
//..............................................................................
int macGraphPD_Sort(const AnAssociation2<double,double>& a1, const AnAssociation2<double,double>& a2) {
  const double df = a2.GetA() - a1.GetA();
  if( df < 0 )  return -1;
  if( df > 0 )  return 1;
  return 0;
}
void XLibMacros::macGraphPD(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TRefList refs;
  TArrayList<compd > F;
  olxstr err( SFUtil::GetSF(refs, F, SFUtil::mapTypeObs, 
    Options.Contains("fcf") ? SFUtil::sfOriginFcf : SFUtil::sfOriginOlex2, 
    (Options.FindValue("s", "r").ToLowerCase().CharAt(0) == 'r') ? SFUtil::scaleRegression : SFUtil::scaleSimple) );
  if( !err.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, err);
    return;
  }
  TEFile out( TEFile::ExtractFilePath(xapp.XFile().GetFileName()) << "olx_pd_calc.csv", "w+b");
  const mat3d& hkl2c = xapp.XFile().GetAsymmUnit().GetHklToCartesian();
  const double d_2_sin = xapp.XFile().GetRM().expl.GetRadiation()/2.0;
  TTypeList< AnAssociation2<double,double> > gd;
  gd.SetCapacity( refs.Count() );
  for( int i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    vec3d d_hkl(ref.GetH(), ref.GetK(), ref.GetL());
    vec3d hkl(d_hkl[0]*hkl2c[0][0],
      d_hkl[0]*hkl2c[0][1] + d_hkl[1]*hkl2c[1][1],
      d_hkl[0]*hkl2c[0][2] + d_hkl[1]*hkl2c[1][2] + d_hkl[2]*hkl2c[2][2]);
    const double theta = asin(d_2_sin*hkl.Length());
    gd.AddNew( 360*theta/M_PI, ref.GetI());
  }
  gd.QuickSorter.SortSF(gd, macGraphPD_Sort);
  for( int i=0; i < refs.Count(); i++ )
    out.Writenl( CString(gd[i].GetA(), 40) << ',' << gd[i].GetB());

}
//..............................................................................
void XLibMacros::macFile(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& XApp = TXApp::GetInstance();
  olxstr Tmp;
  if( Cmds.IsEmpty() )  {  // res -> Ins rotation if ins file
    if( XApp.CheckFileType<TIns>() )  {
      Tmp = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "ins");
    }
    else
      Tmp = XApp.XFile().GetFileName();
  }
  else
    Tmp = Cmds[0];

  bool Sort = Options.Contains('s');

  if( TEFile::ExtractFilePath(Tmp).IsEmpty() )
    Tmp = TEFile::AddTrailingBackslash(CurrentDir) + Tmp;
  TEBitArray removedSAtoms, removedCAtoms;
  if( TEFile::ExtractFileExt(Tmp).Comparei("ins") == 0 )  {  // kill Q peak in the ins file
    TLattice& latt = XApp.XFile().GetLattice();
    removedSAtoms.SetSize( latt.AtomCount() );
    removedCAtoms.SetSize( latt.AtomCount() );
    for( int i=0; i < latt.AtomCount(); i++ )  {
      TSAtom& sa = latt.GetAtom(i);
      if( sa.GetAtomInfo() == iQPeakIndex )  {
        if( !sa.IsDeleted() )  {
          sa.SetDeleted(true);
          removedSAtoms.SetTrue(i);
        }
        if( !sa.IsDeleted() )  {
          sa.CAtom().SetDeleted(true);
          removedCAtoms.SetTrue(i);
        }
      }
    }
  }
  
  XApp.XFile().SaveToFile(Tmp, Sort);
  if( XApp.XFile().HasLastLoader() )  {
    olxstr fd = TEFile::ExtractFilePath(Tmp);
    if( !fd.IsEmpty() && (fd.Comparei(CurrentDir)) )  {
      if( !TEFile::ChangeDir(fd) )
        TBasicApp::GetLog().Error("Cannot change current folder...");
      else
        CurrentDir = fd;
    }
  }
  else  if( !Sort )  {
    Sort = true;  // forse reading the file
  }
  if( removedSAtoms.Count() != 0 )  {  // need to restore, abit of mess here...
    TLattice& latt = XApp.XFile().GetLattice();
    for( int i=0; i < latt.AtomCount(); i++ )  {
      TSAtom& sa = latt.GetAtom(i);
      if( removedSAtoms.Get(i) )  sa.SetDeleted(false);
      if( removedCAtoms.Get(i) )  sa.CAtom().SetDeleted(false);
    }
  }
  if( Sort )  {
    olex::IOlexProcessor* op = olex::IOlexProcessor::GetInstance();
      if( op != NULL )
        op->executeMacro(olxstr("reap \'") << Tmp << '\'');
  }
}
//..............................................................................
void XLibMacros::macFuse(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  TXApp::GetInstance().XFile().GetLattice().Uniq( Options.Contains("f") );
}
//..............................................................................
void XLibMacros::macLstIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  bool remarks = Options.Contains("r");
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  TBasicApp::GetLog() << ("List of current instructions:\n");
  olxstr Tmp;
  for( int i=0; i < Ins.InsCount(); i++ )  {
    if( !remarks && !Ins.InsName(i).Comparei("REM") )  continue;
    Tmp = i;  Tmp.Format(3, true, ' ');
    Tmp << Ins.InsName(i) << ' ' << Ins.InsParams(i).Text(' ');
    TBasicApp::GetLog() << (Tmp << '\n');
  }
}
//..............................................................................
void XLibMacros::macAddIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  // if instruction is parsed, it goes to current model, otherwise i stays in the ins file
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  if( !Ins.AddIns(TStrList(Cmds), TXApp::GetInstance().XFile().GetRM()) )  {
    Error.ProcessingError(__OlxSrcInfo, olxstr("could not add instruction: ") << Cmds.Text(' ') );
    return;
  }
}
//..............................................................................
void XLibMacros::macDelIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  bool isOmit = false;
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  if( Cmds[0].IsNumber() )  {
    int insIndex = Cmds[0].ToInt();
    Ins.DelIns(insIndex);
  }
  else  {
    if( Cmds[0].Comparei("OMIT") == 0 )
      TXApp::GetInstance().XFile().GetRM().ClearOmits();
    else  {
      for( int i=0; i < Ins.InsCount(); i++ )  {
        if( Ins.InsName(i).Comparei(Cmds[0]) == 0 )  {
          Ins.DelIns(i);  i--;  continue;
        }
      }
    }
  }
  OnDelIns->Exit(NULL, &Cmds[0]);
}
//..............................................................................
void XLibMacros::macLS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  int ls = -1;
  XLibMacros::ParseNumbers<int>(Cmds, 1, &ls);
  if( ls != -1 )  
    TXApp::GetInstance().XFile().GetRM().SetIterations( (int)ls);
  if( !Cmds.IsEmpty() )
    TXApp::GetInstance().XFile().GetRM().SetRefinementMethod( Cmds[0] );
}
//..............................................................................
void XLibMacros::macUpdateWght(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  if( rm.proposed_weight.Count() == 0 )  return;
  if( Cmds.IsEmpty() )  
    rm.used_weight = rm.proposed_weight;
  else  {
    rm.used_weight.Resize(Cmds.Count());
    for( int i=0; i < Cmds.Count(); i++ )  
      rm.used_weight[i] = Cmds[i].ToDouble();
  }
}
//..............................................................................
void XLibMacros::macUser(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    TBasicApp::GetLog() << TEFile::CurrentDir() << '\n';
  }
  else if( !TEFile::ChangeDir(Cmds[0]) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not change current folder" );
  }
  else
    CurrentDir = Cmds[0]; 
}
//..............................................................................
void XLibMacros::macDir(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr Filter( Cmds.IsEmpty() ? olxstr("*.*")  : Cmds[0] );
  TStrList Output;
  TFileList fl;
  TEFile::ListCurrentDirEx(fl, Filter, sefFile|sefDir);
  TTTable<TStrList> tab(fl.Count(), 4);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Size";
  tab.ColName(2) = "Last Modified";
  tab.ColName(3) = "Attributes";

  TFileListItem::SortListByName(fl);

  for( int i=0; i < fl.Count(); i++ )  {
    tab[i][0] = fl[i].GetName();
    if( (fl[i].GetAttributes() & sefDir) != 0 )
      tab[i][1] = "Folder";
    else
      tab[i][1] = fl[i].GetSize();
    tab[i][2] = TETime::FormatDateTime("yyyy.MM.dd hh:mm:ss", fl[i].GetModificationTime());
    if( (fl[i].GetAttributes() & sefReadOnly) != 0 )
      tab[i][3] << 'r';
    if( (fl[i].GetAttributes() & sefWriteOnly) != 0 )
      tab[i][3] << 'w';
    if( (fl[i].GetAttributes() & sefHidden) != 0 )
      tab[i][3] << 'h';
    if( (fl[i].GetAttributes() & sefSystem) != 0 )
      tab[i][3] << 's';
    if( (fl[i].GetAttributes() & sefExecute) != 0 )
      tab[i][3] << 'e';
  }

  tab.CreateTXTList(Output, "Directory list", true, true, ' ');
  TBasicApp::GetLog() << Output;
}
//..............................................................................
void XLibMacros::macLstMac(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  // create masks
  TTypeList<TStrMask> masks;
  for( int i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString);
  // end masks creation
  TBasicFunctionPList macros;
  TXApp::GetInstance().GetLibrary().ListAllMacros( macros );
  for( int i=0; i < macros.Count(); i++ )  {
    ABasicFunction* func = macros[i];
    bool add = false;
    olxstr fn = func->GetQualifiedName();
    for( int j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(fn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    TBasicApp::GetLog() << (fn << " - " << func->GetSignature() << '\n');
  }
}
//..............................................................................
void XLibMacros::macLstFun(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  // create masks
  TTypeList<TStrMask> masks;
  for( int i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString);
  // end masks creation
  TBasicFunctionPList functions;
  TXApp::GetInstance().GetLibrary().ListAllFunctions( functions );
  for( int i=0; i < functions.Count(); i++ )  {
    ABasicFunction* func = functions[i];
    bool add = false;
    olxstr fn = func->GetQualifiedName();
    for( int j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(fn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    TBasicApp::GetLog() << (fn << " - " << func->GetSignature() << '\n');
  }
}
//..............................................................................
void XLibMacros::ChangeCell(const mat3d& tm, const TSpaceGroup& new_sg)  {
  TXApp& xapp = TXApp::GetInstance();
  TBasicApp::GetLog() << (olxstr("Cell choice trasformation matrix: \n"));
  TBasicApp::GetLog() << tm[0].ToString() << '\n';
  TBasicApp::GetLog() << tm[1].ToString() << '\n';
  TBasicApp::GetLog() << tm[2].ToString() << '\n';
  TBasicApp::GetLog() << ((olxstr("New space group: ") << new_sg.GetName() << '\n'));
  const mat3d tm_t( mat3d::Transpose(tm) );
  xapp.XFile().UpdateAsymmUnit();
  TAsymmUnit& au = xapp.XFile().LastLoader()->GetAsymmUnit();
  const mat3d i_tm( tm.Inverse() );
  mat3d f2c( mat3d::Transpose(xapp.XFile().GetAsymmUnit().GetCellToCartesian())*tm );
  mat3d ax_err;
  ax_err[0] = vec3d(QRT(au.Axes()[0].GetE()), QRT(au.Axes()[1].GetE()), QRT(au.Axes()[2].GetE()));
  ax_err[1] = ax_err[0];  ax_err[2] = ax_err[0];
  mat3d an_err;
  an_err[0] = vec3d(QRT(au.Angles()[0].GetE()), QRT(au.Angles()[1].GetE()), QRT(au.Angles()[2].GetE()));
  an_err[1] = an_err[0];  an_err[2] = an_err[0];
  // prepare positive matrix for error estimation
  mat3d tm_p(tm);
  for( int i=0; i < 3; i++ )
    for( int j=0; j < 3; j++ )
      if( tm_p[i][j] < 0 ) 
        tm_p[i][j] = - tm_p[i][j];
  ax_err *= tm_p;
  an_err *= tm_p;
  f2c.Transpose();
  au.Axes()[0].V() = f2c[0].Length();  au.Axes()[0].E() = sqrt(ax_err[0][0]);
  au.Axes()[1].V() = f2c[1].Length();  au.Axes()[1].E() = sqrt(ax_err[1][1]);
  au.Axes()[2].V() = f2c[2].Length();  au.Axes()[2].E() = sqrt(ax_err[2][2]);
  au.Angles()[0].V() = acos(f2c[1].CAngle(f2c[2]))*180.0/M_PI;  au.Angles()[0].E() = sqrt(an_err[0][0]);
  au.Angles()[1].V() = acos(f2c[0].CAngle(f2c[2]))*180.0/M_PI;  au.Angles()[1].E() = sqrt(an_err[1][1]);
  au.Angles()[2].V() = acos(f2c[0].CAngle(f2c[1]))*180.0/M_PI;  au.Angles()[2].E() = sqrt(an_err[2][2]);
  for( int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    ca.ccrd() = i_tm * ca.ccrd();
    if( ca.GetEllipsoid() != NULL )  { // reset to usio
      au.NullEllp( ca.GetEllipsoid()->GetId() );
      ca.AssignEllp(NULL);
    }
  }
  au.PackEllps();
  TBasicApp::GetLog() << (olxstr("New cell: ") << au.Axes()[0].ToString() << 
    ' ' << au.Axes()[1].ToString() << 
    ' ' << au.Axes()[2].ToString() <<
    ' '  << au.Angles()[0].ToString() << 
    ' '  << au.Angles()[1].ToString() << 
    ' '  << au.Angles()[2].ToString() << '\n'
    );
  TBasicApp::GetLog().Error("Cell esd's are estimated!");
  olxstr hkl_fn( xapp.LocateHklFile() );
  if( !hkl_fn.IsEmpty() )  {
    THklFile hklf;
    hklf.LoadFromFile(hkl_fn);
    for( int i=0; i < hklf.RefCount(); i++ )  {
      vec3d hkl(hklf[i].GetH(), hklf[i].GetK(), hklf[i].GetL());
      hkl = tm_t * hkl;
      hklf[i].SetH(Round(hkl[0]));
      hklf[i].SetK(Round(hkl[1]));
      hklf[i].SetL(Round(hkl[2]));
    }
    olxstr new_hkl_fn( TEFile::ExtractFilePath(hkl_fn) );
    TEFile::AddTrailingBackslashI(new_hkl_fn) << "test.hkl";
    hklf.SaveToFile( new_hkl_fn );
    xapp.XFile().GetRM().SetHKLSource(new_hkl_fn);
  }
  au.ChangeSpaceGroup(new_sg);
  au.InitMatrices();
  xapp.XFile().LastLoaderChanged();
  // keep the settings, as the hkl file has changed, so if user exists... inconsistency
  xapp.XFile().SaveToFile( xapp.XFile().GetFileName(), false );
}
//..............................................................................
TSpaceGroup* XLibMacros_macSGS_FindSG(TPtrList<TSpaceGroup>& sgs, const olxstr& axis)  {
  for( int i=0; i < sgs.Count(); i++ )
    if( sgs[i]->GetAxis().Compare(axis) == 0 )
      return sgs[i];
  return NULL;
}
olxstr XLibMacros_macSGS_SgInfo(const olxstr& caxis)  {
  if( caxis.IsEmpty() )
    return "standard";
  else  {
    if( caxis.Length() == 3 && caxis.CharAt(0) == '-' )    // -axis + cell choice
      return olxstr("axis: -") << caxis.CharAt(1) << ", cell choice " << caxis.CharAt(2);
    else if( caxis.Length() == 2 )    // axis + cell choice
      return olxstr("axis: ") << caxis.CharAt(0) << ", cell choice " << caxis.CharAt(1);
    else  
      return olxstr("axis: ") << caxis;
  }
}
void XLibMacros::macSGS(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup& sg = Cmds.Count() == 1 ? xapp.XFile().GetLastLoaderSG() : *TSymmLib::GetInstance()->FindGroup(Cmds[1]);
  SGSettings sg_set(sg);
  olxstr axis = sg_set.axisInfo.GetAxis();
  TBasicApp::GetLog() << (olxstr("Current setting: ") << XLibMacros_macSGS_SgInfo(axis) << '\n');
  if( axis.IsEmpty() )  {
    TBasicApp::GetLog() << "Nothing to do\n";
    return;
  }
  TSymmLib& sl = *TSymmLib::GetInstance();
  TPtrList<TSpaceGroup> sgs;
  sl.GetGroupByNumber(sg.GetNumber(), sgs);
  for( int i=0; i < sgs.Count(); i++ )  {
    if( &sg != sgs[i] )
      TBasicApp::GetLog() << (olxstr("Possible: ") << XLibMacros_macSGS_SgInfo(sgs[i]->GetAxis()) << '\n');
  }
  AxisInfo n_ai(sg, Cmds[0]);
  if( sg_set.axisInfo.HasMonoclinicAxis() && !n_ai.HasMonoclinicAxis() )
    n_ai.ChangeMonoclinicAxis(sg_set.axisInfo.GetMonoclinicAxis());
  if( sg_set.axisInfo.HasCellChoice() && !n_ai.HasCellChoice() )
    n_ai.ChangeCellChoice(sg_set.axisInfo.GetCellChoice());
  if( sg_set.axisInfo.GetAxis() == n_ai.GetAxis() )  {
    TBasicApp::GetLog() << "Nothing to change\n";
    return;
  }
  mat3d tm;
  if( sg_set.GetTrasformation(n_ai, tm) )  {
    TSpaceGroup* new_sg = XLibMacros_macSGS_FindSG( sgs, n_ai.GetAxis() );
    if( new_sg == NULL && n_ai.GetAxis() == "abc" )
      new_sg = XLibMacros_macSGS_FindSG( sgs, EmptyString );
    if( new_sg == NULL )  {
      E.ProcessingError(__OlxSrcInfo, "Could not locate space group for given settings");
      return;
    }
    ChangeCell(tm, *new_sg);
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "could not find appropriate transformation");
  }
}
//..............................................................................
void XLibMacros::macLstVar(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( TOlxVars::VarCount() == 0 )  return;
  // create masks
  TTypeList<TStrMask> masks;
  for( int i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString);
  // end masks creation
  TTTable<TStrList> tab(TOlxVars::VarCount(), 3);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Value";
#ifndef _NO_PYTHON_
  tab.ColName(2) = "RefCnt";
#endif
  int rowsCount = 0;
  for( int i=0; i < TOlxVars::VarCount(); i++ )  {
    bool add = false;
    const olxstr& vn = TOlxVars::GetVarName(i);
    for( int j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(vn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    tab[rowsCount][0] = vn;
    tab[rowsCount][1] = TOlxVars::GetVarStr(i);
#ifndef _NO_PYTHON
    if( TOlxVars::GetVarWrapper(i) != NULL )
      tab[rowsCount][2] = TOlxVars::GetVarWrapper(i)->ob_refcnt;
    else
      tab[rowsCount][2] = NAString;
#endif
    rowsCount++;
  }
  tab.SetRowCount(rowsCount);
  TStrList Output;
  tab.CreateTXTList(Output, "Variables list", true, true, ' ');
  TBasicApp::GetLog() << Output;
}
//..............................................................................
void XLibMacros::macLstFS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  // create masks
  TTypeList<TStrMask> masks;
  for( int i=0; i < Cmds.Count(); i++ )
    masks.AddNew(Cmds[i]);
  if( masks.IsEmpty() )  masks.AddNew(EmptyString);
  // end masks creation
  double tc = 0;
  TTTable<TStrList> tab(TFileHandlerManager::Count(), 4);
  tab.ColName(0) = "Name";
  tab.ColName(1) = "Size";
  tab.ColName(2) = "Timestamp";
  tab.ColName(3) = "Persistent";
  int rowsAdded = 0;
  for(int i=0; i < TFileHandlerManager::Count(); i++ )  {
    bool add = false;
    const olxstr& bn = TFileHandlerManager::GetBlockName(i);
    for( int j=0; j < masks.Count(); j++ )  {
      if( masks[j].DoesMatchi(bn) )  {
        add = true;
        break;
      }
    }
    if( !add )  continue;
    tab[rowsAdded][0] = bn;
    tab[rowsAdded][1] = TFileHandlerManager::GetBlockSize(i);
    tab[rowsAdded][2] = TFileHandlerManager::GetBlockDateTime(i);
    tab[rowsAdded][3] = TFileHandlerManager::GetPersistenceId(i);
    tc += TFileHandlerManager::GetBlockSize(i);
    rowsAdded++;
  }
  tc /= (1024*1024);
  tab.SetRowCount(rowsAdded);
  TStrList Output;
  tab.CreateTXTList(Output, olxstr("Virtual FS content"), true, false, "|");
  TBasicApp::GetLog() << Output;
  TBasicApp::GetLog() << (olxstr("Content size is ") << olxstr::FormatFloat(3, tc)  << "Mb\n");
}
//..............................................................................
void XLibMacros::macPlan(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
  int plan = Cmds[0].ToInt();
  if( plan == -1 )  return; // leave like it is
  TXApp::GetInstance().XFile().GetRM().SetPlan( plan );
}
//..............................................................................
class TFixUnit_Sorter  {
public:
  static int Compare(TBasicAtomInfo const * s1, 
                    TBasicAtomInfo const * s2)  {
    double diff = s1->GetMr() - s1->GetMr();
    if( diff < 0 )  return -1;
    if( diff > 0 )  return 1;
    return 0;
  }
};
void XLibMacros::macFixUnit(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  double Zp = Cmds.IsEmpty() ? 1 : Cmds[0].ToDouble();
  if( Zp <= 0 )  Zp = 1;
  TIns& Ins = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
  TXApp::GetInstance().XFile().UpdateAsymmUnit();
  TPtrList<TBasicAtomInfo> content;
  TAsymmUnit& au = TXApp::GetInstance().XFile().GetAsymmUnit();
  int nhc = 0;
  TBasicAtomInfo *cBai = NULL, *hBai = NULL;
  for(int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    TBasicAtomInfo& bai = ca.GetAtomInfo();
    if( ca.IsDeleted() || bai == iQPeakIndex )  continue;
    if( bai.GetMr() > 3.5 )
      nhc++;
    int ind = content.IndexOf(&bai);
    if( ind == -1 )  {
      content.Add(&bai);
      bai.SetSumm( ca.GetOccp() );
      if( cBai == NULL && bai == iCarbonIndex )    cBai = &bai;
      if( hBai == NULL && bai == iHydrogenIndex )  hBai = &bai;
    }
    else
      bai.SetSumm( bai.GetSumm() + ca.GetOccp() );
  }
  int Z = Round(au.EstimateZ((int)((double)nhc/Zp)));
  au.SetZ(Z);
  TBasicApp::GetLog() << (olxstr("for Z'=") << olxstr::FormatFloat(2, Zp).TrimFloat() <<
    " and " << nhc << " non hydrogen atoms Z is estimated to be " << Z << '\n');
  olxstr sfac, unit, n_c;
  content.QuickSorter.Sort<TFixUnit_Sorter>(content);
  if( cBai != NULL && content.Count() > 1 )
    content.Swap(0, content.IndexOf(cBai) );
  if( hBai != NULL && content.Count() > 2 )
    content.Swap(1, content.IndexOf(hBai) );

  for( int i=0; i < content.Count(); i++ )  {
    sfac << content[i]->GetSymbol();
    unit << content[i]->GetSumm()*Z;
    n_c << content[i]->GetSymbol() << olxstr::FormatFloat(3,(double)content[i]->GetSumm()/Zp).TrimFloat();
    if( (i+1) < content.Count() )  {
      sfac << ' ';
      unit << ' ';
      n_c << ' ';
    }
  }
  TBasicApp::GetLog() << "New content is: " << n_c << '\n';
  Ins.SetSfac(sfac);
  Ins.SetUnit(unit);
}
//..............................................................................
void XLibMacros::macEXYZ(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  return;
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  xapp.FindSAtoms(Cmds.Text(' '), atoms, false, false);
  if( atoms.Count() != 1 )  {
    E.ProcessingError(__OlxSrcInfo, "please provide one atom exactly" );
    return;
  }
  if( atoms[0]->CAtom().GetExyzGroup() != NULL )  {
  
  }
  else  {
    TExyzGroup& sr = xapp.XFile().GetRM().ExyzGroups.New();
    for( int i=0; i < atoms.Count(); i++ )
      sr.Add(atoms[i]->CAtom());
  }
}
//..............................................................................
void XLibMacros::macEADP(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  xapp.FindSAtoms(Cmds.Text(' '), atoms, false, true);
  if( atoms.Count() < 2 )  {
    E.ProcessingError(__OlxSrcInfo, "not enough atoms provided" );
    return;
  }
  // validate that atoms of the same type
  bool allIso = (atoms[0]->GetEllipsoid() == NULL);
  for( int i=1; i < atoms.Count(); i++ )  {
    if( (atoms[i]->GetEllipsoid() == NULL) != allIso )  {
      E.ProcessingError(__OlxSrcInfo, "mixed atoms types (aniso and iso)" );
      return;
    }
  }
  TSimpleRestraint& sr = xapp.XFile().GetRM().rEADP.AddNew();
  for( int i=0; i < atoms.Count(); i++ )
    sr.AddAtom(atoms[i]->CAtom(), NULL);
  xapp.XFile().GetRM().rEADP.ValidateRestraint(sr);
}
//..............................................................................
void XLibMacros::macAddSE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  TXApp& xapp = TXApp::GetInstance();
  
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  if( au.AtomCount() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "Empty asymmetric unit");
    return;
  }
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "Could not identify current space group");
    return;
  }
  if( sg->IsCentrosymmetric() && Cmds.Count() == 1 )  {
    E.ProcessingError(__OlxSrcInfo, "Centrosymmetric space group");
    return;
  }
  if( Cmds.Count() == 1 )  {
    smatd_list ml;
    sg->GetMatrices(ml, mattAll);
    ml.SetCapacity( ml.Count()*2 );
    int mc = ml.Count();
    for( int i=0; i < mc; i++ )  {
      ml.AddCCopy(ml[i]);
      ml[i+mc] *= -1;
    }
    for( int i=0; i < TSymmLib::GetInstance()->SGCount(); i++ )  {
      if( TSymmLib::GetInstance()->GetGroup(i) == ml )  {
        xapp.GetLog() << "found " << TSymmLib::GetInstance()->GetGroup(i).GetName() << '\n';
        sg = &TSymmLib::GetInstance()->GetGroup(i);
      }
    }
  }
  else if( Cmds.Count() == 2 )  {
    TSAtomPList atoms;
    xapp.FindSAtoms(EmptyString, atoms);
    for( int i=0; i < au.AtomCount(); i++ )  {
      for( int j=i+1; j < au.AtomCount(); j++ )  {
        if( au.GetAtom(j).IsDeleted() )  continue;
        double d = uc.FindClosestDistance(au.GetAtom(i), au.GetAtom(j));
        if( d < 0.5 )  {
          au.GetAtom(i).SetDeleted(true);
          break;
        }
      }
    }
    latt.Init();
    latt.Compaq();
    latt.CompaqAll();
    return;
  }
  TSymmTest st(uc);
  smatd m;
  m.r.I();
  double tol = 0.1;
  st.TestMatrix(m, tol);
  if( !st.GetResults().IsEmpty() )  {
    int ind = st.GetResults().Count()-1;
    double match = st.GetResults()[ind].Count()*200/st.AtomCount();
    while( match > 125 && tol > 1e-4 )  {
      tol /= 4;
      st.TestMatrix(m, tol);
      ind = st.GetResults().Count()-1;
      match = st.GetResults().IsEmpty() ? 0.0 :st.GetResults()[ind].Count()*200/st.AtomCount();
      continue;
    }
    if( st.GetResults().IsEmpty() )  {
      E.ProcessingError(__OlxSrcInfo, "ooops...");
      return;
    }
    TEStrBuffer out;
    vec3d trans( st.GetResults()[ind].Center );
    //TVectorD trans = st.GetGravityCenter();
    trans /= 2;
    trans *= -1;
    m.t = trans;
    TSAtomPList atoms;
    xapp.FindSAtoms(EmptyString, atoms);
    xapp.XFile().GetLattice().TransformFragments(atoms, m);
    au.ChangeSpaceGroup(*sg);
    xapp.XFile().LastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
    latt.Init();
    for( int i=0; i < au.AtomCount(); i++ )  {
      for( int j=i+1; j < au.AtomCount(); j++ )  {
        if( au.GetAtom(j).IsDeleted() )  continue;
        double d = uc.FindClosestDistance(au.GetAtom(i), au.GetAtom(j));
        if( d < 0.5 )  {
          au.GetAtom(i).SetDeleted(true);
          break;
        }
      }
    }
//    latt.OnStructureGrow->SetEnabled(false);
//    latt.OnStructureUniq->SetEnabled(false);
    latt.Init();
    latt.Compaq();
//    latt.OnStructureGrow->SetEnabled(true);
//    latt.OnStructureUniq->SetEnabled(true);
    latt.CompaqAll();
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "could not find interatomic relations");
  }
}
//..............................................................................
void XLibMacros::macCompaq(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Options.Contains("a") )  
    TXApp::GetInstance().XFile().GetLattice().CompaqAll();
  else
    TXApp::GetInstance().XFile().GetLattice().Compaq();
}
//..............................................................................
void XLibMacros::macEnvi(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  double r = 2.7;
  ParseNumbers<double>(Cmds, 1, &r);
  if( r < 1 || r > 10 )  {
    E.ProcessingError(__OlxSrcInfo, "radius must be within [1;10] range" );
    return;
  }
  TSAtomPList atoms;
  TXApp& xapp = TXApp::GetInstance();
  if( !xapp.FindSAtoms(Cmds.Text(' '), atoms, false, false) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }
  TAtomsInfo& AtomsInfo = TAtomsInfo::GetInstance();
  TStrList output;
  TPtrList<TBasicAtomInfo> Exceptions;
  Exceptions.Add(AtomsInfo.FindAtomInfoBySymbol("Q"));
  Exceptions.Add(AtomsInfo.FindAtomInfoBySymbol("H"));
  if( Options.Contains('q') )
    Exceptions.Remove(AtomsInfo.FindAtomInfoBySymbol("Q"));
  if( Options.Contains('h') )
    Exceptions.Remove(AtomsInfo.FindAtomInfoBySymbol("H"));

  TSAtom& SA = *atoms[0];
  TLattice& latt = TXApp::GetInstance().XFile().GetLattice();
  TAsymmUnit& au = latt.GetAsymmUnit();
  vec3d V;
  smatd_list* L;
  TArrayList< AnAssociation3<TCAtom*, vec3d, smatd> > rowData;
  TCAtomPList allAtoms;

  for( int i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    bool skip = false;
    for( int j=0; j < Exceptions.Count(); j++ )  {
      if( au.GetAtom(i).GetAtomInfo() == *Exceptions[j] )
      {  skip = true;  break;  }
    }
    if( !skip )  allAtoms.Add( &au.GetAtom(i) );
  }
  for( int i=0; i < au.CentroidCount(); i++ )  {
    if( au.GetCentroid(i).IsDeleted() )  continue;
    allAtoms.Add( &au.GetCentroid(i) );
  }
  for( int i=0; i < allAtoms.Count(); i++ )  {
    if( SA.CAtom().GetId() == allAtoms[i]->GetId() )
      L = latt.GetUnitCell().GetInRange(SA.ccrd(), allAtoms[i]->ccrd(), r, false);
    else
      L = latt.GetUnitCell().GetInRange(SA.ccrd(), allAtoms[i]->ccrd(), r, true);
    if( L->Count() != 0 )  {
      for( int j=0; j < L->Count(); j++ )  {
        const smatd& m = L->Item(j);
        V = m * allAtoms[i]->ccrd() - SA.ccrd();
        au.CellToCartesian(V);
        if( V.Length() == 0 )  // symmetrical equivalent?
          continue;
        rowData.Add( AnAssociation3<TCAtom*, vec3d, smatd>(allAtoms[i], V, m) );
      }
    }
    delete L;
  }
  TTTable<TStrList> table(rowData.Count(), rowData.Count()+2); // +SYM + LEN
  table.ColName(0) = SA.GetLabel();
  table.ColName(1) = "SYMM";
  rowData.BubleSorter.Sort<XLibMacros::TEnviComparator>(rowData);
  for( int i=0; i < rowData.Count(); i++ )  {
    const AnAssociation3<TCAtom*, vec3d, smatd>& rd = rowData[i];
    table.RowName(i) = rd.GetA()->GetLabel();
    table.ColName(i+2) = table.RowName(i);
    if( rd.GetC().r.IsI() && rd.GetC().t.IsNull() )
     table[i][1] = 'I';  // identity
    else
      table[i][1] = TSymmParser::MatrixToSymm( rd.GetC() );
    table[i][0] = olxstr::FormatFloat(2, rd.GetB().Length());
    for( int j=0; j < rowData.Count(); j++ )  {
      if( i == j )  { table[i][j+2] = '-'; continue; }
      if( i < j )   { table[i][j+2] = '-'; continue; }
      const AnAssociation3<TCAtom*, vec3d, smatd>& rd1 = rowData[j];
      if( rd.GetB().Length() != 0 && rd1.GetB().Length() != 0 )  {
        double angle = rd.GetB().CAngle(rd1.GetB());
        angle = acos(angle)*180/M_PI;
        table[i][j+2] = olxstr::FormatFloat(1, angle);
      }
      else
        table[i][j+2] = '-';
    }
  }
  table.CreateTXTList(output, EmptyString, true, true, ' ');
  TBasicApp::GetLog() << output << '\n';
  //TBasicApp::GetLog() << ("----\n");
  //if( !latt.IsGenerated() )
  //  TBasicApp::GetLog() << ("Use move \"atom_to atom_to_move\" command to move the atom/fragment\n");
  //else
  //  TBasicApp::GetLog() << ("Use move \"atom_to atom_to_move -c\" command to move the atom/fragment\n");
}
//..............................................................................
void XLibMacros::funRemoveSE(const TStrObjList &Params, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "could not identify current space group");
    return;
  }
  if( Params[0] == "-1" )  {
    if( !sg->IsCentrosymmetric() )  {
      E.SetRetVal( sg->GetName() );
      return;
    }
    smatd_list ml;
    sg->GetMatrices(ml, mattAll^mattInversion);
    TPSTypeList<double, TSpaceGroup*> sglist;
    for( int i=0; i < TSymmLib::GetInstance()->SGCount(); i++ )  {
      double st=0;
      if( TSymmLib::GetInstance()->GetGroup(i).Compare(ml, st) )
        sglist.Add(st, &TSymmLib::GetInstance()->GetGroup(i) );
    }
    E.SetRetVal( sglist.IsEmpty() ? sg->GetName() : sglist.Object(0)->GetName() );
  }
}
//..............................................................................
void XLibMacros::funFileName(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    Tmp = TEFile::ExtractFileName(Params[0]);
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      Tmp = TEFile::ExtractFileName( TXApp::GetInstance().XFile().GetFileName() );
    else
      Tmp = NoneString;
  }
  E.SetRetVal( TEFile::ChangeFileExt(Tmp, EmptyString) );
}
//..............................................................................
void XLibMacros::funFileExt(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    E.SetRetVal( TEFile::ExtractFileExt(Params[0]) );
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      E.SetRetVal( TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()) );
    else
      E.SetRetVal( NoneString );
  }
}
//..............................................................................
void XLibMacros::funFilePath(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    Tmp = TEFile::ExtractFilePath( Params[0] );
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      Tmp = TEFile::ExtractFilePath( TXApp::GetInstance().XFile().GetFileName() );
    else
      Tmp = NoneString;
  }
  // see notes in funBaseDir
  TEFile::RemoveTrailingBackslashI(Tmp);
  E.SetRetVal( Tmp );
}
//..............................................................................
void XLibMacros::funFileDrive(const TStrObjList &Params, TMacroError &E)  {
  olxstr Tmp;
  if( !Params.IsEmpty() )
    E.SetRetVal( TEFile::ExtractFileDrive(Params[0]) );
  else  {
    if( TXApp::GetInstance().XFile().HasLastLoader() )
      E.SetRetVal( TEFile::ExtractFileDrive(TXApp::GetInstance().XFile().GetFileName()) );
    else
      E.SetRetVal( NoneString );
  }
}
//..............................................................................
void XLibMacros::funFileFull(const TStrObjList &Params, TMacroError &E)  {
  if( TXApp::GetInstance().XFile().HasLastLoader() )
    E.SetRetVal( TXApp::GetInstance().XFile().GetFileName() );
  else
    E.SetRetVal( NoneString );
}
//..............................................................................
void XLibMacros::funIsFileLoaded(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( TXApp::GetInstance().XFile().HasLastLoader() );
}
//..............................................................................
void XLibMacros::funTitle(const TStrObjList& Params, TMacroError &E)  {
  if( !TXApp::GetInstance().XFile().HasLastLoader() )  {
    if( Params.IsEmpty() )
      E.SetRetVal( olxstr("File is not loaded") );
    else
      E.SetRetVal( Params[0] );
  }
  else
    E.SetRetVal( TXApp::GetInstance().XFile().LastLoader()->GetTitle() );
}
//..............................................................................
void XLibMacros::funIsFileType(const TStrObjList& Params, TMacroError &E) {
  if( Params[0].Comparei("ins") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() && 
      (TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()).Comparei("ins") == 0) );
  }
  else if( Params[0].Comparei("res") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() && 
      (TEFile::ExtractFileExt(TXApp::GetInstance().XFile().GetFileName()).Comparei("res") == 0) );
  }
  else if( Params[0].Comparei("ires") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TIns>() );
  }
  else if( Params[0].Comparei("cif") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TCif>() );
  }
  else if( Params[0].Comparei("p4p") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TP4PFile>() );
  }
  else if( Params[0].Comparei("mol") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TMol>() );
  }
  else if( Params[0].Comparei("xyz") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TMol>() );
  }
  else if( Params[0].Comparei("crs") == 0 )  {
    E.SetRetVal( TXApp::GetInstance().CheckFileType<TCRSFile>() );
  }
  else
    E.SetRetVal( false );
}
//..............................................................................
void XLibMacros::funBaseDir(const TStrObjList& Params, TMacroError &E)  {
  olxstr tmp( TBasicApp::GetInstance()->BaseDir() );
  // remove the trailing backslash, as it causes a lot of problems with
  // passing parameters to other programs:
  // windows parser assumes that \" is " and does wrong parsing...
  if( !tmp.IsEmpty() )  tmp.SetLength( tmp.Length()-1 );
  E.SetRetVal( tmp );
}
//..............................................................................
void XLibMacros::funLSM(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( TXApp::GetInstance().XFile().GetRM().GetRefinementMethod() );
}
//..............................................................................
void XLibMacros::funIns(const TStrObjList& Params, TMacroError &E)  {
  RefinementModel& rm = TXApp::GetInstance().XFile().GetRM();
  olxstr tmp;
  if( Params[0].Comparei("weight") == 0 || Params[0].Comparei("wght") == 0 )  {
    for( int j=0; j < rm.used_weight.Count(); j++ )  {
      tmp << rm.used_weight[j];
      if( (j+1) < rm.used_weight.Count() )  tmp << ' ';
    }
    E.SetRetVal( tmp );
  }
  else if( !Params[0].Comparei("weight1") )  {
    for( int j=0; j < rm.proposed_weight.Count(); j++ )  {
      tmp << rm.proposed_weight[j];
      if( (j+1) < rm.proposed_weight.Count() )  tmp << ' ';
    }
    E.SetRetVal( tmp );
  }
  else if( (Params[0].Comparei("L.S.") == 0) || (Params[0].Comparei("CGLS") == 0) )  {
    for( int i=0; i < rm.LS.Count(); i++ )  {
      tmp << rm.LS[i];
      if( (i+1) < rm.LS.Count() )  tmp << ' ';
    }
    E.SetRetVal( rm.LS.Count() == 0 ? NAString : tmp );
  }
  else if( Params[0].Comparei("ls") == 0 )  {
    olxstr rv = rm.LS.Count() == 0 ? NAString : olxstr(rm.LS[0]) ;
    E.SetRetVal( rv );
  }
  else if( Params[0].Comparei("plan") == 0)  {
    for( int i=0; i < rm.PLAN.Count(); i++ )  {
      tmp << ((i < 1) ? Round(rm.PLAN[i]) : rm.PLAN[i]);
      if( (i+1) < rm.PLAN.Count() )  tmp << ' ';
    }
    E.SetRetVal( rm.PLAN.Count() == 0 ? NAString : tmp );
  }
  else if( Params[0].Comparei("qnum") == 0)  {
    tmp = rm.PLAN.Count() == 0 ? NAString : olxstr(rm.PLAN[0]);
    E.SetRetVal( tmp );
  }
  else  {
    TIns& I = TXApp::GetInstance().XFile().GetLastLoader<TIns>();
    if( Params[0].Comparei("R1") == 0)  {
      olxstr rv = I.GetR1() < 0 ? NAString : olxstr(I.GetR1());
      E.SetRetVal( rv );
    }
    if( !I.InsExists(Params[0]) )  {
      E.SetRetVal( NAString );
      return;
    }
    //  FXApp->XFile().UpdateAsymmUnit();
    //  I->UpdateParams();

    TInsList* insv = I.FindIns( Params[0] );
    if( insv != 0 )
      E.SetRetVal( insv->Text(' ') );
    else
      E.SetRetVal( EmptyString );
  }
}
//..............................................................................
void XLibMacros::funSSM(const TStrObjList& Params, TMacroError &E) {
  RefinementModel& rm  = TXApp::GetInstance().XFile().GetRM();
  if( rm.GetSolutionMethod().IsEmpty() && Params.Count() == 1 )
    E.SetRetVal( Params[0] );
  else
    E.SetRetVal( rm.GetSolutionMethod() );
}
//..............................................................................
olxstr XLibMacros_funSGNameToHtml(const olxstr& name)  {
  olxstr res;
  res.SetCapacity( name.Length() + 20 );
  for( int i=0; i < name.Length(); i++ )  {
    if( (i+1) < name.Length() )  {
      if( (name[i] >= '0' && name[i] <= '9')  &&  (name[i+1] >= '0' && name[i+1] <= '9') )  {
        if( name[i] != '1' )  {
          res << name[i] << "<sub>" << name[i+1] << "</sub>";
          i++;
        } 
        continue;
      }
    }
    res << name[i];
  }
  return res;
}
void XLibMacros::funSG(const TStrObjList &Cmds, TMacroError &E)  {
  TSpaceGroup* sg = NULL;
  try  { sg = &TXApp::GetInstance().XFile().GetLastLoaderSG();  }
  catch(...)  {}
  if( sg != NULL )  {
    olxstr Tmp;
    if( Cmds.IsEmpty() )  {
      Tmp = sg->GetName();
      if( !sg->GetFullName().IsEmpty() )  {
        Tmp << " (" << sg->GetFullName() << ')';
      }
      Tmp << " #" << sg->GetNumber();
    }
    else  {
      Tmp = Cmds[0];
      Tmp.Replace("%#", olxstr(sg->GetNumber()) );
      Tmp.Replace("%n", sg->GetName());
      Tmp.Replace("%N", sg->GetFullName());
      Tmp.Replace("%HS", sg->GetHallSymbol());
      Tmp.Replace("%s", sg->GetBravaisLattice().GetName());
      if( Tmp.IndexOf("%H") != -1 )
        Tmp.Replace("%H", XLibMacros_funSGNameToHtml(sg->GetFullName()));
      if( Tmp.IndexOf("%h") != -1 )
        Tmp.Replace("%h", XLibMacros_funSGNameToHtml(sg->GetName()));
    }
    E.SetRetVal( Tmp );
  }
  else  {
    E.SetRetVal( NAString );
//    E.ProcessingError(__OlxSrcInfo, "could not find space group for the file" );
    return;
  }
}
//..............................................................................
void XLibMacros::funSGS(const TStrObjList &Cmds, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup& sg = xapp.XFile().GetLastLoaderSG();
  const olxstr& axis =  sg.GetAxis();
  if( axis.IsEmpty() )
    E.SetRetVal<olxstr>( "standard" );
  else  {
    if( axis.Length() == 2 )  {  // axis + cell choice
      E.SetRetVal(olxstr(axis.CharAt(0)) << ": cell choice " << axis.CharAt(1));
    }
    else  {
      E.SetRetVal(olxstr("axis: ") << axis);
    }
  }
}
//..............................................................................
void XLibMacros::funHKLSrc(const TStrObjList& Params, TMacroError &E)  {
  if( Params.Count() == 1 )
    TXApp::GetInstance().XFile().GetRM().SetHKLSource( Params[0] );
  else
    E.SetRetVal( TXApp::GetInstance().XFile().GetRM().GetHKLSource() );
}
//..............................................................................
void XLibMacros::macCif2Doc(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr CifDictionaryFile( xapp.GetCifTemplatesDir() + "cifindex.dat");
  if( Cmds.IsEmpty() )  {
    TStrList Output;
    olxstr CDir = TEFile::CurrentDir();
    TEFile::ChangeDir( xapp.GetCifTemplatesDir() );
    TEFile::ListCurrentDir(Output, "*.rtf;*html;*.htm", sefFile);
    TEFile::ChangeDir(CDir);
    xapp.GetLog() << "Templates found: \n";
    xapp.GetLog() << Output << '\n';
    return;
  }

  olxstr TN = Cmds[0];
  if( !TEFile::FileExists(TN) )
    TN = xapp.GetCifTemplatesDir() + TN;
  if( !TEFile::FileExists(TN) )  {
    Error.ProcessingError(__OlxSrcInfo, "template for CIF does not exist: ") << Cmds[0];
    return;
  }
  // resolvind the index file
  if( !TEFile::FileExists(CifDictionaryFile) )  {
    Error.ProcessingError(__OlxSrcInfo, "CIF dictionary does not exist" );
    return;
  }

  TCif *Cif, Cif1;
  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::FileExists( cifFN ) ) 
      Cif1.LoadFromFile( cifFN );
    else  {
      Error.ProcessingError(__OlxSrcInfo, "existing cif is expected");
      return;
    }
    Cif = &Cif1;
  }

  TStrList SL, Dic;
  olxstr RF = TEFile::ChangeFileExt(Cif->GetFileName(), TEFile::ExtractFileExt(TN));
  SL.LoadFromFile( TN );
  Dic.LoadFromFile( CifDictionaryFile );
  for( int i=0; i < SL.Count(); i++ )
    Cif->ResolveParamsFromDictionary(Dic, SL[i], '%', &XLibMacros::CifResolve);
  TUtf8File::WriteLines( RF, SL, false );
  TBasicApp::GetLog().Info(olxstr("Document name: ") << RF);
}
//..............................................................................
void XLibMacros::macCif2Tab(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr CifTablesFile( xapp.GetCifTemplatesDir() + "tables.xlt");
  olxstr CifDictionaryFile( xapp.GetCifTemplatesDir() + "cifindex.dat");
  if( Cmds.IsEmpty() )  {
    if( !TEFile::FileExists(CifTablesFile) )  {
      Error.ProcessingError(__OlxSrcInfo, "tables definition file is not found" );
      return;
    }

    TDataFile DF;
    TStrList SL;
    TDataItem *Root;
    olxstr Tmp;
    DF.LoadFromXLFile(CifTablesFile, &SL);

    Root = DF.Root().FindItemCI("Cif_Tables");
    if( Root != NULL )  {
      xapp.GetLog().Info("Found table definitions:");
      for( int i=0; i < Root->ItemCount(); i++ )  {
        Tmp = "Table "; 
        Tmp << Root->Item(i).GetName()  << "(" << " #" << (int)i+1 <<  "): caption <---";
        xapp.GetLog().Info(Tmp);
        xapp.GetLog().Info(Root->Item(i).GetFieldValueCI("caption"));
        xapp.GetLog().Info("--->");
      }
    }
    else  {
      Error.ProcessingError(__OlxSrcInfo, "tables definition node is not found" );
      return;
    }
    return;
  }
  TCif *Cif, Cif1;

  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::FileExists( cifFN ) )  {
      Cif1.LoadFromFile( cifFN );
    }
    else
        throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif1;
  }

  TStrList SL, SL1, Dic, 
    CLA, // cell attributes
    THA;  // header (th) attributes
  TDataFile DF;
  TDataItem *TD, *Root;
  TTTable<TStrList> DT;
  DF.LoadFromXLFile(CifTablesFile, NULL);
  Dic.LoadFromFile( CifDictionaryFile );

  olxstr RF = TEFile::ExtractFilePath(Cif->GetFileName()) + "tables.html", Tmp;
  Root = DF.Root().FindItemCI("Cif_Tables");
  smatd_list SymmList;
  int tab_count = 1;
  for( int i=0; i < Cmds.Count(); i++ )  {
    TD = NULL;
    if( Cmds[i].IsNumber() )  {
      int index = Cmds[i].ToInt();
      if( index >=0 && index < Root->ItemCount() )
        TD = &Root->Item(index);
    }
    if( TD == NULL  )
      TD = Root->FindItem(Cmds[i]);
    if( TD == NULL )  {
      xapp.GetLog().Warning( olxstr("Could not find table definition: ") << Cmds[i] );
      continue;
    }
    if( !TD->GetName().Comparei("footer") || !TD->GetName().Comparei("header") )  {
      olxstr fn = TD->GetFieldValue("source");
      if( !TEFile::IsAbsolutePath(fn) )
        fn = xapp.GetCifTemplatesDir() + fn;
      SL1.LoadFromFile( fn );
      for( int j=0; j < SL1.Count(); j++ )  {
        Cif->ResolveParamsFromDictionary(Dic, SL1[j], '%', &XLibMacros::CifResolve);
        SL.Add( SL1[j] );
      }
      continue;
    }
    if( Cif->CreateTable(TD, DT, SymmList) )  {
      Tmp = "Table "; Tmp << ++tab_count << ' ' << TD->GetFieldValueCI("caption");
      Tmp.Replace("%DATA_NAME%", Cif->GetDataName());
      if( Tmp.IndexOf("$") >= 0 )
        ProcessExternalFunction( Tmp );
      // attributes of the row names ...
      CLA.Clear();
      THA.Clear();
      CLA.Add( TD->GetFieldValue("tha", EmptyString) );
      THA.Add( TD->GetFieldValue("tha", EmptyString) );
      for( int j=0; j < TD->ItemCount(); j++ )  {
        CLA.Add( TD->Item(j).GetFieldValue("cola", EmptyString) );
        THA.Add( TD->Item(j).GetFieldValue("tha", EmptyString) );
      }

      olxstr footer;
      for(int i=0; i < SymmList.Count(); i++ )  {
        footer << "<sup>" << (i+1) << "</sup>" <<
           TSymmParser::MatrixToSymm(SymmList[i]);
        if( (i+1) < SymmList.Count() )
          footer << "; ";
      }


      DT.CreateHTMLList(SL, Tmp,
                      footer,
                      true, false,
                      TD->GetFieldValue("tita", EmptyString),  // title paragraph attributes
                      TD->GetFieldValue("foota", EmptyString),  // footer paragraph attributes
                      TD->GetFieldValue("taba", EmptyString),  //const olxstr& tabAttr,
                      TD->GetFieldValue("rowa", EmptyString),  //const olxstr& rowAttr,
                      THA, // header attributes
                      CLA, // cell attributes,
                      true,
                      TD->GetFieldValue("coln", "1").ToInt()
                      ); //bool Format) const  {

      //DT.CreateHTMLList(SL, Tmp, true, false, true);
    }
  }
  TUtf8File::WriteLines(RF, SL, false);
  TBasicApp::GetLog().Info(olxstr("Tables file: ") << RF);
}
//..............................................................................
olxstr XLibMacros::CifResolve(const olxstr& func)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )  return func;
  olxstr rv;
  if( op->executeFunction(func, rv) )
    return rv;
  return func;
}
//..............................................................................
bool XLibMacros::ProcessExternalFunction(olxstr& func)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )  return false;
  olxstr rv;
  if( op->executeFunction(func, rv) )  {
    func = rv;
    return true;
  }
  return false;
}
//..............................................................................
void XLibMacros::MergePublTableData(TCifLoopTable& to, TCifLoopTable& from)  {
  if( from.RowCount() == 0 )  return;
  static const olxstr authorNameCN("_publ_author_name");
  // create a list of unique colums, and prepeare them for indexed access
  TSStrPObjList<olxstr, AnAssociation2<int,int>, false> uniqCols;
  for( int i=0; i < from.ColCount(); i++ )  {
    if( uniqCols.IndexOfComparable( from.ColName(i) ) == -1 )  {
      uniqCols.Add( from.ColName(i), AnAssociation2<int,int>(i, -1) );
    }
  }
  for( int i=0; i < to.ColCount(); i++ )  {
    int ind = uniqCols.IndexOfComparable( to.ColName(i) );
    if( ind == -1 )
      uniqCols.Add( to.ColName(i), AnAssociation2<int,int>(-1, i) );
    else
      uniqCols.Object(ind).B() = i;
  }
  // add new columns, if any
  for( int i=0; i < uniqCols.Count(); i++ ) {
    if( uniqCols.Object(i).GetB() == -1 )  {
      to.AddCol( uniqCols.GetComparable(i) );
      uniqCols.Object(i).B() = to.ColCount() - 1;
    }
  }
  /* by this point the uniqCols contains all the column names and the association
  holds corresponding column indexes in from and to tables */
  // the actual merge, by author name
  int authNCI = uniqCols.IndexOfComparable( authorNameCN );
  if( authNCI == -1 )  return;  // cannot do much, can we?
  AnAssociation2<int,int> authCA( uniqCols.Object(authNCI) );
  if( authCA.GetA() == -1 )  return;  // no author?, bad ...
  for( int i=0; i < from.RowCount(); i++ )  {
    int ri = -1;
    for( int j=0; j < to.RowCount(); j++ )  {
      if( to[j][ authCA.GetB() ].Comparei( from[i][ authCA.GetA() ]) == 0 )  {
        ri = j;
        break;
      }
    }
    if( ri == -1 )  {  // add a new row
      to.AddRow( EmptyString );
      ri = to.RowCount()-1;
    }
    for( int j=0; j < uniqCols.Count(); j++ )  {
      AnAssociation2<int,int>& as = uniqCols.Object(j);
      if( as.GetA() == -1 )  continue;
      to[ ri ][as.GetB()] = from[i][ as.GetA() ];
    }
  }
  // null the objects - they must not be here anyway ..
  for( int i=0; i < to.RowCount(); i++ )  {
    for( int j=0; j < to.ColCount(); j++ )  {
      if( to[i].Object(j) == NULL )
        to[i].Object(j) = new TCifLoopData(true);
      to[i].Object(j)->String = true;
    }
  }
}
//..............................................................................
void XLibMacros::macCifMerge(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TCif *Cif, Cif1, Cif2;

  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::FileExists( cifFN ) )  {
      Cif2.LoadFromFile( cifFN );
    }
    else
      throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif2;
  }

  TCifLoop& publ_info = Cif->PublicationInfoLoop();

  for( int i=0; i < Cmds.Count(); i++ )  {
    try {
      IInputStream *is = TFileHandlerManager::GetInputStream(Cmds[i]);
      if( is == NULL )  {
        TBasicApp::GetLog().Error( olxstr("Could not find file: ") << Cmds[i] );
        continue;
      }
      TStrList sl;
      sl.LoadFromTextStream(*is);
      delete is;
      Cif1.LoadFromStrings(sl);
    }
    catch( ... )  {    }  // most like the cif does not have cell, so pass it
    TCifLoop& pil = Cif1.PublicationInfoLoop();
    for( int j=0; j < Cif1.ParamCount(); j++ )  {
      if( !Cif->ParamExists(Cif1.Param(j)) )
        Cif->AddParam(Cif1.Param(j), Cif1.ParamValue(j));
      else
        Cif->SetParam(Cif1.Param(j), Cif1.ParamValue(j));
    }
    // update publication info loop
    MergePublTableData( publ_info.Table(), pil.Table() );
  }
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( Cif->GetAsymmUnit() );
  if( sg != NULL )  {
    if( !Cif->ParamExists("_symmetry_cell_setting") )
      Cif->AddParam("_symmetry_cell_setting", sg->GetBravaisLattice().GetName(), true);
    else  {
      TCifData* cd = Cif->FindParam("_symmetry_cell_setting");
      if( cd->Data->IsEmpty() )
        cd->Data->Add( sg->GetBravaisLattice().GetName() );
      else
        cd->Data->String(0) = sg->GetBravaisLattice().GetName();
      cd->String = true;
    }
    if( !Cif->ParamExists("_symmetry_space_group_name_H-M") )
      Cif->AddParam("_symmetry_space_group_name_H-M", sg->GetFullName(), true);
    else  {
      TCifData* cd = Cif->FindParam("_symmetry_space_group_name_H-M");
      if( cd->Data->IsEmpty() )
        cd->Data->Add( sg->GetFullName() );
      else
        cd->Data->String(0) = sg->GetFullName();
      cd->String = true;
    }
    if( !Cif->ParamExists("_symmetry_space_group_name_Hall") )
      Cif->AddParam("_symmetry_space_group_name_Hall", sg->GetHallSymbol(), true);
    else  {
      TCifData* cd = Cif->FindParam("_symmetry_space_group_name_Hall");
      if( cd->Data->IsEmpty() )
        cd->Data->Add( sg->GetHallSymbol() );
      else
        cd->Data->String(0) = sg->GetHallSymbol();
      cd->String = true;
    }
  }
  else
    TBasicApp::GetLog().Error("Could not locate space group ...");
  Cif->Group();
  Cif->SaveToFile( Cif->GetFileName() );
}
//..............................................................................
void XLibMacros::macCifExtract(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr Dictionary = Cmds[0];
  if( !TEFile::FileExists(Dictionary) )  {  // check if the dictionary exists
    Dictionary = xapp.GetCifTemplatesDir();  Dictionary << Cmds[0];
    if( !TEFile::FileExists(Dictionary) )  {
      Error.ProcessingError(__OlxSrcInfo, "dictionary file does not exists" );
      return;
    }
  }

  TCif In,  Out, *Cif, Cif1;

  if( xapp.CheckFileType<TCif>() )
    Cif = &xapp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr cifFN = TEFile::ChangeFileExt( xapp.XFile().GetFileName(), "cif");
    if( TEFile::FileExists( cifFN ) )  {
      Cif1.LoadFromFile( cifFN );
    }
    else
      throw TFunctionFailedException(__OlxSourceInfo, "existing cif is expected");
    Cif = &Cif1;
  }

  try  {  In.LoadFromFile(Dictionary);  }
  catch( TExceptionBase& )  {
    Error.ProcessingError(__OlxSrcInfo, "could not load dictionary file" );
    return;
  }

  TCifData *CifData;
  for( int i=0; i < In.ParamCount(); i++ )  {
    CifData = Cif->FindParam(In.Param(i));
    if( CifData )
      Out.AddParam(In.Param(i), CifData);
  }
  try  {  Out.SaveToFile(Cmds[1]);  }
  catch( TExceptionBase& )  {
    Error.ProcessingError(__OlxSrcInfo, "could not save file: ") << Cmds[1];
    return;
  }
}
//..............................................................................
struct XLibMacros_StrF  {
  int h, k, l;
  double ps;
  TEComplex<double> v;
};
void XLibMacros::macVoidE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& XApp = TXApp::GetInstance();
  double F000 = 0;
  double factor = 2;
  TRefList refs;
  TArrayList<TEComplex<double> > F;
  TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
  const TUnitCell& uc = XApp.XFile().GetUnitCell();
  // space group matrix list
  TSpaceGroup* sg = NULL;
  try  { sg = &XApp.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "could not locate space group");
    return;
  }
  smatd_list ml;
  sg->GetMatrices(ml, mattAll^mattInversion);
  for( int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  
      continue;
    int ec = (ca.GetAtomInfo() == iDeuteriumIndex) ? 1 : ca.GetAtomInfo().GetIndex()+1;
    F000 += ec*uc.MatrixCount()*ca.GetOccp();
  }
  olxstr fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fcf");
  if( !TEFile::FileExists(fcffn) )  {
    fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fco");
    if( !TEFile::FileExists(fcffn) )  {
      E.ProcessingError(__OlxSrcInfo, "please load fcf file or make sure the one exists in current folder");
      return;
    }
  }
  TCif cif;
  cif.LoadFromFile( fcffn );
//  F000 = cif.GetSParam("_exptl_crystal_F_000").ToDouble();
  TCifLoop* hklLoop = cif.FindLoop("_refln");
  if( hklLoop == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "no hkl loop found");
    return;
  }
  int hInd = hklLoop->Table().ColIndex("_refln_index_h");
  int kInd = hklLoop->Table().ColIndex("_refln_index_k");
  int lInd = hklLoop->Table().ColIndex("_refln_index_l");
  // list 3, F
  int mfInd = hklLoop->Table().ColIndex("_refln_F_meas");
  int sfInd = hklLoop->Table().ColIndex("_refln_F_sigma");
  int aInd = hklLoop->Table().ColIndex("_refln_A_calc");
  int bInd = hklLoop->Table().ColIndex("_refln_B_calc");

  if( hInd == -1 || kInd == -1 || lInd == -1 || 
    mfInd == -1 || sfInd == -1 || aInd == -1 || bInd == -1  ) {
      E.ProcessingError(__OlxSrcInfo, "list 3 fcf file is expected");
      return;
  }
  refs.SetCapacity( hklLoop->Table().RowCount() );
  F.SetCount( hklLoop->Table().RowCount() );
  for( int i=0; i < hklLoop->Table().RowCount(); i++ )  {
    TStrPObjList<olxstr,TCifLoopData*>& row = hklLoop->Table()[i];
    TReflection& ref = refs.AddNew(row[hInd].ToInt(), row[kInd].ToInt(), 
      row[lInd].ToInt(), row[mfInd].ToDouble(), row[sfInd].ToDouble());
    if( ref.GetH() < 0 )
      factor = 4;
//    const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
//    F[i] = TEComplex<double>::polar(ref.GetI(), rv.arg());
//    F[i].A() = row[aInd].ToDouble();
//    F[i].B() = row[bInd].ToDouble();
      const TEComplex<double> rv(row[aInd].ToDouble(), row[bInd].ToDouble());
      double dI = (ref.GetI() - rv.mod());
      F[i] = TEComplex<double>::polar(dI, rv.arg());
  }
  olxstr hklFileName = XApp.LocateHklFile();
  if( !TEFile::FileExists(hklFileName) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate hkl file");
    return;
  }
  double vol = XApp.XFile().GetLattice().GetUnitCell().CalcVolume();
  int minH = 100,  minK = 100,  minL = 100;
  int maxH = -100, maxK = -100, maxL = -100;

  vec3d hkl;
  TArrayList<XLibMacros_StrF> AllF(refs.Count()*ml.Count());
  int index = 0;
  double f000 = 0;
  for( int i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    for( int j=0; j < ml.Count(); j++, index++ )  {
      ref.MulHklT(hkl, ml[j]);
      if( hkl[0] < minH )  minH = (int)hkl[0];
      if( hkl[1] < minK )  minK = (int)hkl[1];
      if( hkl[2] < minL )  minL = (int)hkl[2];
      if( hkl[0] > maxH )  maxH = (int)hkl[0];
      if( hkl[1] > maxK )  maxK = (int)hkl[1];
      if( hkl[2] > maxL )  maxL = (int)hkl[2];
      AllF[index].h = (int)hkl[0];
      AllF[index].k = (int)hkl[1];
      AllF[index].l = (int)hkl[2];
      AllF[index].ps = hkl[0]*ml[j].t[0] + hkl[1]*ml[j].t[1] + hkl[2]*ml[j].t[2];
      AllF[index].v = F[i];
      AllF[index].v *= TEComplex<double>::polar(1, 2*M_PI*AllF[index].ps);
    }
  }
// init map, 0.1A for now
  const int mapX = (int)au.Axes()[0].GetV()*3,
			mapY = (int)au.Axes()[1].GetV()*3,
			mapZ = (int)au.Axes()[2].GetV()*3;
  double mapVol = mapX*mapY*mapZ;
  TArray3D<double> fMap(0, mapX-1, 0, mapY-1, 0, mapZ-1);
//////////////////////////////////////////////////////////////////////////////////////////
  TEComplex<double> ** S, *T;
  int kLen = maxK-minK+1, hLen = maxH-minH+1, lLen = maxL-minL+1;
  S = new TEComplex<double>*[kLen];
  for( int i=0; i < kLen; i++ )
    S[i] = new TEComplex<double>[lLen];
  T = new TEComplex<double>[lLen];
  const double T_PI = 2*M_PI;
// precalculations
  int minInd = olx_min(minH, minK);
  if( minL < minInd )  minInd = minL;
  int maxInd = olx_max(maxH, maxK);
  if( maxL > maxInd )  maxInd = maxL;
  int iLen = maxInd - minInd + 1;
  int mapMax = olx_max(mapX, mapY);
  if( mapZ > mapMax )  mapMax = mapZ;
  TEComplex<double>** sin_cosX = new TEComplex<double>*[mapX],
                      **sin_cosY, **sin_cosZ;
  for( int i=0; i < mapX; i++ )  {
    sin_cosX[i] = new TEComplex<double>[iLen];
    for( int j=minInd; j <= maxInd; j++ )  {
      double rv = (double)(i*j)/mapX, ca, sa;
      rv *= T_PI;
      SinCos(-rv, &sa, &ca);
      sin_cosX[i][j-minInd].SetRe(ca);
      sin_cosX[i][j-minInd].SetIm(sa);
    }
  }
  if( mapX == mapY )  {
    sin_cosY = sin_cosX;
  }
  else  {
    sin_cosY = new TEComplex<double>*[mapY];
    for( int i=0; i < mapY; i++ )  {
      sin_cosY[i] = new TEComplex<double>[iLen];
      for( int j=minInd; j <= maxInd; j++ )  {
        double rv = (double)(i*j)/mapY, ca, sa;
        rv *= T_PI;
        SinCos(-rv, &sa, &ca);
        sin_cosY[i][j-minInd].SetRe(ca);
        sin_cosY[i][j-minInd].SetIm(sa);
      }
    }
  }
  if( mapX == mapZ )  {
    sin_cosZ = sin_cosX;
  }
  else if( mapY == mapZ )  {
    sin_cosZ = sin_cosY;
  }
  else  {
    sin_cosZ = new TEComplex<double>*[mapZ];
    for( int i=0; i < mapZ; i++ )  {
      sin_cosZ[i] = new TEComplex<double>[iLen];
      for( int j=minInd; j <= maxInd; j++ )  {
        double rv = (double)(i*j)/mapZ, ca, sa;
        rv *= T_PI;
        SinCos(-rv, &sa, &ca);
        sin_cosZ[i][j-minInd].SetRe(ca);
        sin_cosZ[i][j-minInd].SetIm(sa);
      }
    }
  }
  TEComplex<double> R;
  double maxMapV = -1000, minMapV = 1000;
  for( int ix=0; ix < mapX; ix++ )  {
    for( int i=0; i < AllF.Count(); i++ )  {
      const XLibMacros_StrF& sf = AllF[i];
      S[sf.k-minK][sf.l-minL] += sf.v*sin_cosX[ix][sf.h-minInd];
    }
    for( int iy=0; iy < mapY; iy++ )  {
      for( int i=minK; i <= maxK; i++ )  {
        for( int j=minL; j <= maxL; j++ )  {
          T[j-minL] += S[i-minK][j-minL]*sin_cosY[iy][i-minInd];
        }
      }
      for( int iz=0; iz < mapZ; iz++ )  {
        R.Null();
        for( int i=minL; i <= maxL; i++ )  {
          R += T[i-minL]*sin_cosZ[iz][i-minInd];
        }
        double val = factor*R.Re()/vol;
        if( val > maxMapV )  maxMapV = val;
        if( val < minMapV )  minMapV = val;
        fMap.Data[ix][iy][iz] = val;
      }
      for( int i=0; i < lLen; i++ )  
        T[i].Null();
    }
    for( int i=0; i < kLen; i++ )  
      for( int j=0; j < lLen; j++ )  
        S[i][j].Null();
  }
  TBasicApp::GetLog() << (olxstr("Map max val ") << olxstr::FormatFloat(3, maxMapV) << " min val " << olxstr::FormatFloat(3, minMapV) << '\n');
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  // calculate the map
  double surfdis = Options.FindValue("d", "1.0").ToDouble();
  long structurePoints = 0;
  vec3d voidCenter;
  TArray3D<short> maskMap(0, mapX-1, 0, mapY-1, 0, mapZ-1);
  short MaxLevel = XApp.CalcVoid(maskMap, surfdis, -101, &structurePoints, voidCenter, NULL);
  XApp.GetLog() << ( olxstr("Cell volume (A^3) ") << olxstr::FormatFloat(3, vol) << '\n');
  XApp.GetLog() << ( olxstr("Max level reached ") << MaxLevel << '\n');
  XApp.GetLog() << ( olxstr("Largest spherical void is (A^3) ") << olxstr::FormatFloat(3, MaxLevel*MaxLevel*MaxLevel*4*M_PI/(3*mapVol)*vol) << '\n');
  XApp.GetLog() << ( olxstr("Structure occupies (A^3) ") << olxstr::FormatFloat(3, structurePoints*vol/mapVol) << '\n');
  int minLevel = Round( pow( 6*mapVol*3/(4*M_PI*vol), 1./3) );
  XApp.GetLog() << ( olxstr("6A^3 level is ") << minLevel << '\n');
  // calculate new structure factors
  double Re = 0, Te=0, F0 = 0;
  int RePointCount = 0, TePointCount = 0;
//  for( int i=0; i < refs.Count(); i++ )  {
//    TReflection& ref = refs[i];
//    double A = 0, B = 0;
    for( int ix=0; ix < mapX; ix++ )  {
      for( int iy=0; iy < mapY; iy++ )  {
        for( int iz=0; iz < mapZ; iz++ )  {
          if( maskMap.Data[ix][iy][iz] <= 0  )  {
//            double tv =  (double)ref.GetH()*ix/mapX;  
//            tv += (double)ref.GetK()*iy/mapY;  
//            tv += (double)ref.GetL()*iz/mapZ;
//            tv *= T_PI;
//            double ca, sa;
//            SinCos(tv, &sa, &ca);
//            A += fMap.Data[ix][iy][iz]*ca;
//            B += fMap.Data[ix][iy][iz]*sa;
//            if( i == 0 )  {
              Te += fMap.Data[ix][iy][iz];
              TePointCount++;
//            }
          }
          else   {
//            if( i == 0 )  {
              Re += fMap.Data[ix][iy][iz];
              RePointCount++;
//            }
          }
//          if( i == 0 )  {
            F0 += fMap.Data[ix][iy][iz];
//          }
        }
      }
    }
//    ref.SetI( sqrt(A*A+B*B)/100 );
//  }
//  TCStrList sl;
//  for( int i=0;  i < refs.Count(); i++ )
//    sl.Add( refs[i].ToString() );
//  sl.SaveToFile( "test.hkl" );
  XApp.GetLog() << "Voids         " << Re*vol/(mapVol) << "e-\n";
//  XApp.GetLog() << "F000 calc     " << Te*vol/(mapVol) << "e-\n";
  XApp.GetLog() << "F000 (formula)" << F000 << "e-\n";
//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  for( int i=0; i < kLen; i++ )
    delete [] S[i];
  delete [] S;
  delete [] T;
  if( sin_cosY == sin_cosX )  sin_cosY = NULL;
  if( sin_cosZ == sin_cosX || sin_cosZ == sin_cosY )  sin_cosZ = NULL;
  for( int i=0; i < mapX; i++ )
    delete [] sin_cosX[i];
  delete [] sin_cosX;
  if( sin_cosY != NULL )  {
    for( int i=0; i < mapY; i++ )
      delete [] sin_cosY[i];
    delete [] sin_cosY;
  }
  if( sin_cosZ != NULL )  {
    for( int i=0; i < mapZ; i++ )
      delete [] sin_cosZ[i];
    delete [] sin_cosZ;
  }
}

//..............................................................................
void XLibMacros::macChangeSG(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  if( au.AtomCount() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "Empty asymmetric unit");
    return;
  }
  TSpaceGroup& from_sg = xapp.XFile().GetLastLoaderSG();
  TSpaceGroup* sg = TSymmLib::GetInstance()->FindGroup(Cmds.Last().String());
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Could not identify given space group");
    return;
  }
  // change centering?
  if( from_sg.GetName().SubStringFrom(1) == sg->GetName().SubStringFrom(1) )  {
    olxch from = from_sg.GetLattice().GetSymbol()[0],
          to = sg->GetLattice().GetSymbol()[0];
    mat3d tm;
    tm.I();
    if( from == 'I' )  {
      if( to == 'P' )
        tm = mat3d(-0.5, 0.5, 0.5, -0.5, 0.5, -0.5);
    }
    else if( from == 'P' )  {
      if( to == 'I' )
        tm = mat3d(0, 1, 1, 0, 1, 0);
      else if( to == 'C' )  {
        tm = mat3d(0, 1, 1, 0, 1, 0);  // P->I
        tm *= mat3d(-1, 0, 1, 0, 1, 0, -1, 0, 0);  // I->C, uniq axis b
      }
      else if( to == 'F' )
        tm = mat3d(-1, 1, 1, -1, 1, 1);
    }
    else if( from == 'C' )  {
      if( to == 'P' )  {
        tm = mat3d(0, 0, -1, 0, 1, 0, 1, 0, -1);  // C->I, uniq axis b
        tm *= mat3d(-0.5, 0.5, 0.5, -0.5, 0.5, -0.5);  // I->P 
      }
    }
    else if( from == 'F')  {
      if( to == 'P' )
        tm = mat3d(0, 0.5, 0.5, 0, 0.5, 0);
    }
    if( !tm.IsI() )  {
      TBasicApp::GetLog() << "EXPERIMENTAL: transformations considering b unique\n";
      ChangeCell(tm, *sg);
    }
    else  {
      TBasicApp::GetLog() << "The transformation is not supported\n";
    }
    return;
  }
  smatd_list ml;
  sg->GetMatrices(ml, mattAll );
  TTypeList< AnAssociation3<vec3d,TCAtom*, int> > list;
  uc.GenereteAtomCoordinates(list, true);
  if( Cmds.Count() == 4 )  {
    vec3d trans( Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble());
    for( int i=0; i < list.Count(); i++ )  {
      list[i].A() += trans;
      list[i].SetC(1);
    }
  }
  else   {
    for( int i=0; i < list.Count(); i++ )  { 
      list[i].SetC(1);
    }
  }
  vec3d v;
  for( int i=0; i < list.Count(); i++ )  {
    if( list[i].GetC() == 0 )  continue;
    for( int j=i+1; j < list.Count(); j++ )  {
      if( list[j].GetC() == 0 )  continue;
      for( int k=1; k < ml.Count(); k++ )  {
        v = ml[k] * list[i].GetA();
        v -= list[j].GetA();
        v[0] -= Round(v[0]);  v[1] -= Round(v[1]);  v[2] -= Round(v[2]);
        au.CellToCartesian(v);
        if( v.QLength() < 0.01 )  {
          list[i].C() ++;
          list[j].SetC(0);
        }
      }
    }
  }
  for( int i=0; i < au.AtomCount(); i++ )
    au.GetAtom(i).SetTag(0);
  TCAtomPList newAtoms;
  for( int i=0; i < list.Count(); i++ )  {
    if( list[i].GetC() == 0 )  continue;
    TCAtom* ca;
    if( list[i].GetB()->GetTag() > 0 )  {
      ca = &au.NewAtom();
      ca->Assign( *list[i].GetB() );
      ca->SetLoaderId(liNewAtom);
    }
    else  {
      ca = list[i].GetB();
      ca->SetTag( ca->GetTag() + 1 );
    }
    ca->ccrd() = list[i].GetA();
    ca->AssignEllp(NULL);
  }
  for( int i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetTag() == 0 )
      au.GetAtom(i).SetDeleted(true);
  }
  au.InitAtomIds();
  au.ChangeSpaceGroup(*sg);
  xapp.XFile().LastLoader()->GetAsymmUnit().ChangeSpaceGroup(*sg);
  latt.Init();
  latt.CompaqAll();
}
//..............................................................................
void XLibMacros::macFlush(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TBasicApp::GetLog().Flush();
}
//..............................................................................
void XLibMacros::macSGE(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  using namespace olex;
  IOlexProcessor* op = IOlexProcessor::GetInstance();
  if( op == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "this function requires Olex2 processor implementation");
  TPtrList<TSpaceGroup> sgs;
  TSpaceGroup* sg = NULL;
  bool cntro = false;
  E.SetRetVal(&sgs);
  op->executeMacroEx("SG", E);
  E.SetRetVal<bool>(false);
  if( sgs.Count() == 0 )  {
    TBasicApp::GetLog().Error( "Could not find any suitable spacegroup. Terminating ... " );
    return;
  }
  else if( sgs.Count() == 1 )  {
    sg = sgs[0];
    TBasicApp::GetLog() << "Univocal spacegroup choice: " << sg->GetName() << '\n';
  }
  else  {
    E.Reset();
    op->executeMacroEx("Wilson", E);
    bool centro = E.GetRetVal().ToBool();
    TBasicApp::GetLog() << "Searching for centrosymmetric group: " << centro << '\n';
    for( int i=0; i < sgs.Count(); i++ )  {
      if( centro )  {
        if( sgs[i]->IsCentrosymmetric() )  {
          sg = sgs[i];
          break;
        }
      }
      else  {
        if( !sgs[i]->IsCentrosymmetric() )  {
          sg = sgs[i];
          break;
        }
      }
    }
    if( sg == NULL )  {  // no match to centre of symmetry found
      sg = sgs[0];
      TBasicApp::GetLog() << "Could not match, choosing: " << sg->GetName() << '\n';
    }
    else  {
      TBasicApp::GetLog() << "Chosen: " << sg->GetName() << '\n';
    }
  }
  olxstr fn( Cmds.IsEmpty() ? TEFile::ChangeFileExt(TXApp::GetInstance().XFile().GetFileName(), "ins") : Cmds[0] );
  op->executeMacroEx(olxstr("reset -s=") << sg->GetName() << " -f='" << fn << '\'', E);
  if( E.IsSuccessful() )  {
    op->executeMacroEx(olxstr("reap '") << fn << '\'', E);
    if( E.IsSuccessful() )  
      op->executeMacroEx(olxstr("solve"), E);
    E.SetRetVal<bool>(E.IsSuccessful());
  }
}
//..............................................................................




#ifdef __BORLANC__
  #pragma package(smart_init)
#endif

