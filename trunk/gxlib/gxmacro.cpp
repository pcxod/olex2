#include "gxmacro.h"
#include "sfutil.h"
#include "maputil.h"
#include "beevers-lipson.h"
#include "unitcell.h"
#include "dunitcell.h"
#include "xgrid.h"
#include "gllabels.h"
#include "symmparser.h"
#include "cif.h"
#include "vcov.h"
#include "xline.h"
#include "olxstate.h"
#include "estopwatch.h"
#ifdef __WXWIDGETS__
#include "wx/wx.h"
#endif

#define gxlib_InitMacro(macroName, validOptions, argc, desc)\
  lib.Register(\
    new TStaticMacro(&GXLibMacros::mac##macroName, #macroName, (validOptions), argc, desc))
#define gxlib_InitMacroA(macroName, amacroName, validOptions, argc, desc)\
  lib.Register(\
    new TStaticMacro(&GXLibMacros::mac##macroName, #amacroName, (validOptions), argc, desc))
#define gxlib_InitFunc(funcName, argc, desc) \
  lib.Register(\
    new TStaticFunction(&GXLibMacros::fun##funcName, #funcName, argc, desc))

//.............................................................................
void GXLibMacros::Export(TLibrary& lib) {
  gxlib_InitMacro(Grow,
    "s-grow shells vs fragments&;"
    "w-grows the rest of the structure, using already applied generators&;"
    "t-grows only provided atoms/atom types&;"
    "b-grows all visible grow bonds (when in a grow mode)",
    fpAny|psFileLoaded,
    "Grows whole structure or provided atoms only");

  gxlib_InitMacro(Pack,
    "c-specifies if current lattice content should not be deleted",
    fpAny|psFileLoaded,
    "Packs structure within default or given volume(6 or 2 values for "
    "parallelepiped "
    "or 1 for sphere). If atom names/types are provided it only packs the "
    "provided atoms.");

  gxlib_InitMacro(Name,
    "c-enables checking labels for duplications&;"
    "s-simply changes suffix of provided atoms to the provided one (or none)&;"
    "cs-leaves current selection unchanged",
    fpOne|fpTwo,
    "Names atoms. If the 'sel' keyword is used and a number is provided as "
    "second argument the numbering will happen in the order the atoms were "
    "selected (make sure -c option is added)");

  gxlib_InitMacro(CalcPatt, EmptyString(), fpNone|psFileLoaded,
    "Calculates Patterson map");
  gxlib_InitMacro(CalcFourier,
    "fcf-reads structure factors from a fcf file&;"
    "diff-calculates difference map&;"
    "tomc-calculates 2Fo-Fc map&;"
    "obs-calculates observed map&;"
    "calc-calculates calculated map&;"
    "scale-scale to use for difference maps, currently available simple(s) "
    "sum(Fo^2)/sum(Fc^2)) and regression(r)&;"
    "r-resolution in Angstrems&;"
    "i-integrates the map&;"
    "m-mask the structure", fpNone|psFileLoaded,
  "Calculates fourier map");

  gxlib_InitMacro(Qual,
    "h-High&;"
    "m-Medium&;"
    "l-Low", fpNone,
    "Sets drawings quality");

  gxlib_InitMacro(Mask, EmptyString(), fpAny^fpNone, 
    "Sets primitives for atoms or bonds according to provided mask. Accepts "
    "atoms, bonds, hbonds or a name (like from LstGO).\n"
    "Example: 'mask hbonds 2048' - this resets hydrogen bond style to "
    "default");

  gxlib_InitMacro(ARad, EmptyString(), fpAny^fpNone, 
    "Changes how the atoms are drawn [sfil - sphere packing, pers - static "
    "radii, isot - radii proportional to Ueq, isoth - as isot, but applied to "
    "H atoms as well]");
  gxlib_InitMacro(ADS, EmptyString(), fpAny^(fpNone),
    "Changes atom draw style [sph,elp,std]");
  gxlib_InitMacro(AZoom, EmptyString(), fpAny^fpNone,
    "Modifies given atoms [all] radius. The first argument is the new radius "
    "in %");
  gxlib_InitMacro(BRad, "a-specified value is absolute, in A", fpAny^fpNone,
    "Multiplies provided [all] bonds default radius by given number. The"
    " default radius for covalent bonds is 0.1A and for H-bonds is 0.02A. To "
    "set radius for H-bonds use:"
    "\n\tbrad R hbonds"
    "\nAny particula bond type can also be specified like:\n\tbrad 0.5 C-H"
    "\nNote that the heavier atom type is always first"
    );
  gxlib_InitMacro(TelpV, EmptyString(), fpOne,
    "Calculates ADP scale for given thermal probability (in %)");
  gxlib_InitMacro(Labels,
    "p-part&;"
    "l-label&;"
    "v-variables&;"
    "o-occupancy&;"
    "co-chemical occupancy&;"
    "a-afix&;"
    "h-show hydrogen atom labels&;"
    "f-fixed parameters&;"
    "u-Uiso&;"
    "r-Uiso multiplier for riding atoms&;"
    "ao-actual occupancy (as in the ins file)&;"
    "qi-Q peak intensity&;"
    "i-display labels for identity atoms only&;"
    "f-applies given format to the labels like -f=AaBB Aaaa or -f=Aabb etc",
    fpNone,
    "Inverts visibility of atom labels on/off. Look at the options");
  gxlib_InitMacro(Label,
    "type-type of labels to make - subscript, brackers, default&;"
    "symm-symmetry dependent tag type {[$], #, full}&;"
    "cif-creates labels for CIF data a combination of {b,a,t,h}",
    fpAny,
    "Creates moveable labels for provided atoms/bonds/angles (selection)");
  gxlib_InitMacro(ShowH,
    EmptyString(),
    fpNone|fpTwo|psFileLoaded,
    "Changes the H-atom and H-bonds visibility");
  gxlib_InitMacro(Info,
    "s-sorts the atom list&;p-coordinate precision [3]&;f-print fractional "
    "coordinates vs Cartesian [true]&;"
    "c-copy the printed information ito the clipboard",
    fpAny,
    "Prints out information for gived [all] atoms");
  gxlib_InitMacro(ShowQ,
    "wheel-number of peaks to hide (if negative) or to show ",
    fpNone|fpOne|fpTwo|psFileLoaded,
    "Traverses the three states - peaks and peak bonds are visible, only peaks"
    " visible, no peaks or peak bonds. One numeric argument is taken to "
    "increment/decrement the numegbr of visibe peaks. Two aruments are taken "
    "to control the visibility of atoms or bonds, like in: 'showq a true' or "
    "'showq b false'");
  gxlib_InitMacro(Load,
    EmptyString(),
    fpAny^(fpNone),
    "Arguments - textures");
  gxlib_InitMacro(SetView,
    "c-center",
    fpAny,
    "Sets view normal to the normal of the selected plane, to a bond or mean "
    "line");
  gxlib_InitMacro(Matr,
    "r-used reciprocal cell instead",
    fpNone|fpOne|fpTwo|fpThree|fpNine,
    "Displays or sets current orientation matrix. For single argument, 1,2,3 "
    "001, 111, etc values are acceptable, two values taken are of the klm "
    "form, which specify a view from k1*a+l1*b+m1*c to k2*a+l2*b+m2*c, three "
    "values pecify the view normal and nine values provide a full matrix");
  gxlib_InitMacro(Line,
    "n-just sets current view normal to the line without creating the object",
    fpAny,
    "Creates a line or best line for provided atoms");
  gxlib_InitMacro(Mpln,
    "n-just orient, do not create plane&;"
    "r-create regular plane&;"
    "we-use weights proportional to the (atomic mass)^we", 
    fpAny,
    "Sets current view along the normal of the best plane");
  gxlib_InitMacro(Cent,
    EmptyString(),
    fpAny,
    "Creates a centroid for given/selected/all atoms");
  gxlib_InitMacro(Cell, "r-shows reciprocal cell",
    fpNone|fpOne|psFileLoaded,
    "If no arguments provided inverts visibility of unit cell, otherwise sets"
    " it to the boolean value of the parameter");
  gxlib_InitMacro(Basis, EmptyString(), fpNone|fpOne,
    "Shows/hides the orientation basis");
  gxlib_InitMacro(Group,
    "n-a custom name can be provided",
    fpNone|fpOne|psFileLoaded,
  "Groups current visible objects or selection");
  gxlib_InitMacro(Fmol, EmptyString(), fpNone|psFileLoaded,
    "Shows all fragments (as opposite to uniq)");
  gxlib_InitMacro(Sel,
    "a-select all&;"
    "u-unselect all&;"
    "l-consider the list of bonds as independent&;"
    "c-copies printed values to the clipboard&;"
    "i-invert selection&;",
    fpAny,
    "If no arguments provided, prints current selection. This includes "
    "distances, angles and torsion angles and other geometrical parameters. "
    "Selects atoms fulfilling provided conditions, like"
    "\nsel $type - selects any particular atom type; type can be one of the "
    "following shortcuts - * - for all atoms, M - for metals, X - for halogens"
    "\nsel $*,type - selects all but type atoms"
    "\n\n"
    "An extended syntax include keyword 'where' and 'rings' which "
    "allow selecting atoms and bonds according to their properties, like type "
    "and length or rings of particular connectivity like C6 or NC5. If the "
    "'where' keyword is used, logical operators, like and (&&), and or (||) "
    "can be used to refine the selection. For example:"
    "\nsel atoms where xatom.bai.z > 2 - to select all atoms heavier after H"
    "\nsel bonds where xbond.length > 2 - to select all bonds longer than 2 A"
    "\nsel bonds where xbond.b.bai.z == 1 - to select all bonds where the "
    "lightest atoms is H"
    );
  gxlib_InitMacro(Uniq, EmptyString(), fpAny | psFileLoaded,
    "Shows only fragments specified by atom name(s) or selection");
  gxlib_InitMacro(PiM,
    "l-display labels for the created lines",
    fpAny|psFileLoaded,
    "Creates an illustration of a pi-system to metal bonds");
  gxlib_InitMacro(Split, "r-EADP,ISOR or SIMU to be placed for the split atoms",
    fpAny|psCheckFileTypeIns,
    "Splits provided atoms along the longest axis of the ADP");
  gxlib_InitMacro(ShowP, "m-do not modify the display view", fpAny,
    "Shows specified or all parts of the structure");
  gxlib_InitMacro(Undo, EmptyString(), fpNone,
    "Reverts some of the previous operations");
}
//.............................................................................
void GXLibMacros::macGrow(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  if (Options.Contains('b')) {  // grow XGrowBonds only
    app.GrowBonds();
    return;
  }
  bool GrowShells = Options.Contains('s'),
       GrowContent = Options.Contains('w');
  TCAtomPList TemplAtoms;
  if( Options.Contains('t') )
    TemplAtoms = app.FindCAtoms(olxstr(Options['t']).Replace(',', ' '));
  if( Cmds.IsEmpty() )  {  // grow fragments
    if( GrowContent ) 
      app.GrowWhole(TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
    else  {
      TXAtomPList atoms;
      app.GrowFragments(GrowShells, TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
      if( !GrowShells )  {
        const TLattice& latt = app.XFile().GetLattice();
        smatd_list gm;
        /* check if next grow will not introduce simple translations */
        bool grow_next = true;
        while( grow_next )  {
          gm.Clear();
          latt.GetGrowMatrices(gm);
          if( gm.IsEmpty() )  break;
          for( size_t i=0; i < latt.MatrixCount(); i++ )  {
            for( size_t j=0; j < gm.Count(); j++ )  {
              if( latt.GetMatrix(i).r == gm[j].r )  {
                const vec3d df = latt.GetMatrix(i).t - gm[j].t;
                if( (df-df.Round<int>()).QLength() < 1e-6 )  {
                  grow_next = false;
                  break;
                }
              }
            }
            if( !grow_next )  break;
          }
          if( grow_next ) {
            app.GrowFragments(GrowShells,
              TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
          }
        }
      }
    }
  }
  else  {  // grow atoms
    if( GrowContent )
      app.GrowWhole(TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
    else {
      app.GrowAtoms(Cmds.Text(' '), GrowShells,
        TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
    }

  }
}
//.............................................................................
void GXLibMacros::macPack(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  const bool ClearCont = !Options.Contains("c");
  const bool cell = (Cmds.Count() > 0 && Cmds[0].Equalsi("cell"));
  const bool wbox = (Cmds.Count() > 0 && Cmds[0].Equalsi("wbox"));
  if( cell || wbox )
    Cmds.Delete(0);
  const uint64_t st = TETime::msNow();
  if( cell )
    app.XFile().GetLattice().GenerateCell();
  else if( wbox && app.Get3DFrame().IsVisible() )  {
    vec3d_alist norms(6), centres(6);
    for (int i=0; i < 6; i++) {
      norms[i] = app.Get3DFrame().Faces[i].GetN();
      centres[i] = app.Get3DFrame().Faces[i].GetCenter();
    }
    app.XFile().GetLattice().GenerateBox(norms, centres, ClearCont);
  }
  else  {
    vec3d From(-0.5), To(1.5);
    size_t number_count = 0;
    for( size_t i=0; i < Cmds.Count(); i++ )  {
      if( Cmds[i].IsNumber() )  {
        if( !(number_count%2) )
          From[number_count/2] = Cmds[i].ToDouble();
        else
          To[number_count/2]= Cmds[i].ToDouble();
        number_count++;
        Cmds.Delete(i--);
      }
    }

    if( number_count != 0 && !(number_count == 6 || number_count == 1 ||
      number_count == 2) )
    {
      Error.ProcessingError(__OlxSrcInfo, "please provide 6, 2 or 1 number");
      return;
    }

    TCAtomPList TemplAtoms;
    if( !Cmds.IsEmpty() )
      TemplAtoms = app.FindCAtoms(Cmds.Text(' '));

    if( number_count == 6 || number_count == 0 || number_count == 2 )  {
      if( number_count == 2 )  {
        From[1] = From[2] = From[0];
        To[1] = To[2] = To[0];
      }
      app.Generate(From, To, TemplAtoms.IsEmpty() ? NULL
        : &TemplAtoms, ClearCont);
    }
    else  {
      TXAtomPList xatoms = app.FindXAtoms(Cmds, true, true);
      vec3d cent;
      double wght = 0;
      for( size_t i=0; i < xatoms.Count(); i++ )  {
        cent += xatoms[i]->crd()*xatoms[i]->CAtom().GetChemOccu();
        wght += xatoms[i]->CAtom().GetChemOccu();
      }
      if( wght != 0 )
        cent /= wght;
      app.Generate(cent, From[0], TemplAtoms.IsEmpty() ? NULL
        : &TemplAtoms, ClearCont);
    }
  }
  if( TBasicApp::GetInstance().IsProfiling() )  {
    TBasicApp::NewLogEntry(logInfo) <<
      app.XFile().GetLattice().GetObjects().atoms.Count() <<
      " atoms and " <<
      app.XFile().GetLattice().GetObjects().bonds.Count() <<
      " bonds generated in " <<
      app.XFile().GetLattice().FragmentCount() << " fragments ("
      << (TETime::msNow()-st) << "ms)";
  }
  // optimise drawing ...
  //app.GetRender().Compile(true);
}
//.............................................................................
void GXLibMacros::macName(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  bool checkLabels = Options.Contains("c");
  bool changeSuffix = Options.Contains("s");
  if( changeSuffix )  {
    TXAtomPList xatoms = app.FindXAtoms(Cmds, true, !Options.Contains("cs"));
    if (!xatoms.IsEmpty()) {
      app.GetUndo().Push(app.ChangeSuffix(
        xatoms, Options.FindValue("s"), checkLabels));
    }
  }
  else  {
    bool processed = false;
    if( Cmds.Count() == 1 )  { // bug #49
      const size_t spi = Cmds[0].IndexOf(' ');
      if( spi != InvalidIndex )  {
        app.GetUndo().Push(
          app.Name(Cmds[0].SubStringTo(spi),
          Cmds[0].SubStringFrom(spi+1), checkLabels,
          !Options.Contains("cs"))
        );
      }
      else {
        app.GetUndo().Push(
          app.Name("sel", Cmds[0], checkLabels, !Options.Contains("cs")));
      }
      processed = true;
    }
    else if( Cmds.Count() == 2 )  {
      app.GetUndo().Push(app.Name(
        Cmds[0], Cmds[1], checkLabels, !Options.Contains("cs")));
      processed = true;
    }
    if( !processed )  {
      Error.ProcessingError(__OlxSrcInfo,
        olxstr("invalid syntax: ") << Cmds.Text(' '));
    }
  }
}
//.............................................................................
void GXLibMacros::macQual(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  if( Options.IsEmpty() )
    Error.ProcessingError(__OlxSrcInfo, "wrong number of arguments");
  else  {
    int32_t v;
     if( Options.GetName(0)[0] == 'h' ) v = qaHigh;
     else if( Options.GetName(0)[0] == 'm' ) v = qaMedium;
     else if( Options.GetName(0)[0] == 'l' ) v = qaLow;
     else {
       Error.ProcessingError(__OlxSrcInfo, "wrong argument");
       return;
     }
    app.Quality(v);
  }
}
//.............................................................................
void GXLibMacros::macCalcFourier(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TStopWatch st(__FUNC__);
  TGXApp &app = TGXApp::GetInstance();
  double resolution = Options.FindValue("r", "0.25").ToDouble(),
    maskInc = 1.0;
  if( resolution < 0.1 )  resolution = 0.1;
  resolution = 1./resolution;
  short mapType = SFUtil::mapTypeCalc;
  if( Options.Contains("tomc") )
    mapType = SFUtil::mapType2OmC;
  else if( Options.Contains("obs") )
    mapType = SFUtil::mapTypeObs;
  else if( Options.Contains("diff") )
    mapType = SFUtil::mapTypeDiff;
  olxstr strMaskInc = Options.FindValue("m");
  if( !strMaskInc.IsEmpty() )
    maskInc = strMaskInc.ToDouble();
  TRefList refs;
  TArrayList<compd> F;
  st.start("Obtaining structure factors");
  olxstr err = SFUtil::GetSF(refs, F, mapType,
    Options.Contains("fcf") ? SFUtil::sfOriginFcf : SFUtil::sfOriginOlex2,
    (Options.FindValue("scale", "r").ToLowerCase().CharAt(0) == 'r') ?
      SFUtil::scaleRegression : SFUtil::scaleSimple);
  if( !err.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, err);
    return;
  }
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  TUnitCell& uc = app.XFile().GetUnitCell();
  TArrayList<SFUtil::StructureFactor> P1SF;
  st.start("Expanding SF to P1");
  SFUtil::ExpandToP1(refs, F, uc.GetMatrixList(), P1SF);
  const double vol = app.XFile().GetLattice().GetUnitCell().CalcVolume();
  BVFourier::MapInfo mi;
// init map
  const vec3i dim(au.GetAxes()*resolution);
  TArray3D<float> map(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  st.start("Calcuating ED map");
  mi = BVFourier::CalcEDM(P1SF, map.Data, dim, vol);
///////////////////////////////////////////////////////////////////////////////
  st.start("Map operations");
  app.XGrid().InitGrid(dim);

  app.XGrid().SetMaxHole(mi.sigma*1.4);
  app.XGrid().SetMinHole(-mi.sigma*1.4);
  app.XGrid().SetScale(-mi.sigma*6);
  //app.XGrid().SetScale( -(mi.maxVal - mi.minVal)/2.5 );
  app.XGrid().SetMinVal(mi.minVal);
  app.XGrid().SetMaxVal(mi.maxVal);
  // copy map
  MapUtil::CopyMap(app.XGrid().Data()->Data, map.Data, dim);
  app.XGrid().AdjustMap();

  TBasicApp::NewLogEntry() <<
    "Map max val " << olxstr::FormatFloat(3, mi.maxVal) <<
    " min val " << olxstr::FormatFloat(3, mi.minVal) << NewLineSequence() <<
    "Map sigma " << olxstr::FormatFloat(3, mi.sigma);
  // map integration
  if( Options.Contains('i') )  {
    TArrayList<MapUtil::peak> Peaks;
    TTypeList<MapUtil::peak> MergedPeaks;
    vec3d norm(1./dim[0], 1./dim[1], 1./dim[2]);
    MapUtil::Integrate<float>(map.Data, dim, mi.sigma*6, Peaks);
    MapUtil::MergePeaks(uc.GetSymmSpace(), norm, Peaks, MergedPeaks);
    QuickSorter::SortSF(MergedPeaks, MapUtil::PeakSortBySum);
    for( size_t i=0; i < MergedPeaks.Count(); i++ )  {
      const MapUtil::peak& peak = MergedPeaks[i];
      if( peak.count == 0 )  continue;
      vec3d cnt((double)peak.center[0]/dim[0], (double)peak.center[1]/dim[1],
        (double)peak.center[2]/dim[2]); 
      const double ed = (double)((long)((peak.summ*1000)/peak.count))/1000;
      TCAtom& ca = au.NewAtom();
      ca.SetLabel(olxstr("Q") << olxstr((100+i)));
      ca.ccrd() = cnt;
      ca.SetQPeak(ed);
    }
    au.InitData();
    TActionQueue* q_draw = app.FindActionQueue(olxappevent_GL_DRAW);
    bool q_draw_changed = false;
    if( q_draw != NULL )  {
      q_draw->SetEnabled(false);
      q_draw_changed = true;
    }
    app.XFile().GetLattice().Init();
    olex::IOlexProcessor::GetInstance()->processMacro("compaq -q");
    if( q_draw != NULL && q_draw_changed )
      q_draw->SetEnabled(true);
  }  // integration
  if( Options.Contains("m") )  {
    FractMask* fm = new FractMask;
    app.BuildSceneMask(*fm, maskInc);
    app.XGrid().SetMask(*fm);
  }
  app.XGrid().InitIso();
  //TStateChange sc(prsGridVis, true);
  app.ShowGrid(true, EmptyString());
  //OnStateChange.Execute((AEventsDispatcher*)this, &sc);
}
//.............................................................................
void GXLibMacros::macCalcPatt(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TGXApp &app = TGXApp::GetInstance();
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  // space group matrix list
  TUnitCell::SymmSpace sp = app.XFile().GetUnitCell().GetSymmSpace();
  olxstr hklFileName = app.LocateHklFile();
  if( !TEFile::Exists(hklFileName) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate hkl file");
    return;
  }
  TRefList refs;
  app.XFile().GetRM().GetFourierRefList<
      TUnitCell::SymmSpace,RefMerger::StandardMerger>(sp, refs);
  const double vol = app.XFile().GetLattice().GetUnitCell().CalcVolume();
  TArrayList<SFUtil::StructureFactor> P1SF(refs.Count()*sp.Count());
  size_t index = 0;
  for( size_t i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    double sI = sqrt(refs[i].GetI() < 0 ? 0 : refs[i].GetI());
    for( size_t j=0; j < sp.Count(); j++, index++ )  {
      P1SF[index].hkl = ref * sp[j];
      P1SF[index].ps = sp[j].t.DotProd(ref.GetHkl());
      P1SF[index].val = sI;
      P1SF[index].val *= compd::polar(1, 2*M_PI*P1SF[index].ps);
    }
  }
  const double resolution = 5;
  const vec3i dim(au.GetAxes()*resolution);
  app.XGrid().InitGrid(dim);
  BVFourier::MapInfo mi = BVFourier::CalcPatt(
    P1SF, app.XGrid().Data()->Data, dim, vol);
  app.XGrid().AdjustMap();
  app.XGrid().SetMinVal(mi.minVal);
  app.XGrid().SetMaxVal(mi.maxVal);
  app.XGrid().SetMaxHole( mi.sigma*1.4);
  app.XGrid().SetMinHole(-mi.sigma*1.4);
  app.XGrid().SetScale( -(mi.maxVal - mi.minVal)/2.5);
  app.XGrid().InitIso();
  app.ShowGrid(true, EmptyString());
}
//.............................................................................
void GXLibMacros::macMask(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  if (Cmds[0].Equalsi("atoms") && Cmds.Count() > 1) {
    int Mask = Cmds[1].ToInt();
    short AtomsStart=2;
    TXAtomPList Atoms =
      app.FindXAtoms(TStrObjList(Cmds.SubListFrom(AtomsStart)), false, false);
    app.UpdateAtomPrimitives(Mask, Atoms.IsEmpty() ? NULL : &Atoms);
  }
  else if( (Cmds[0].Equalsi("bonds") || Cmds[0].Equalsi("hbonds")) &&
           Cmds.Count() > 1 )
  {
    int Mask = Cmds[1].ToInt();
    TXBondPList Bonds = app.GetBonds(TStrList(Cmds.SubListFrom(2)), false);
    app.UpdateBondPrimitives(Mask, 
      (Bonds.IsEmpty() && app.GetSelection().Count() == 0) ? NULL : &Bonds,
      Cmds[0].Equalsi("hbonds"));
  }
  else  {
    int Mask = Cmds.GetLastString().ToInt();
    Cmds.Delete(Cmds.Count() - 1);
    TGPCollection *GPC = app.GetRender().FindCollection(Cmds.Text(' '));
    if( GPC != NULL )  {
      if( GPC->ObjectCount() != 0 )
        GPC->GetObject(0).UpdatePrimitives(Mask);
    }
    else  {
      Error.ProcessingError(__OlxSrcInfo,"Undefined graphics:").quote() <<
        Cmds.Text(' ');
      return;
    }
  }
}
//.............................................................................
void GXLibMacros::macARad(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  olxstr arad(Cmds[0]);
  Cmds.Delete(0);
  TXAtomPList xatoms = app.FindXAtoms(Cmds, false, false);
  app.AtomRad(arad, xatoms.IsEmpty() ? NULL : &xatoms);
}
//.............................................................................
void GXLibMacros::macADS(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  short ads = -1;
  if( Cmds[0].Equalsi("elp") )
    ads = adsEllipsoid;
  else if( Cmds[0].Equalsi("sph") )
    ads = adsSphere;
  else if( Cmds[0].Equalsi("ort") )
    ads = adsOrtep;
  else if( Cmds[0].Equalsi("std") )
    ads = adsStandalone;
  if( ads == -1 )  {
    Error.ProcessingError(__OlxSrcInfo,
      "unknown atom type (elp/sph/ort/std) supported only");
    return;
  }
  TGXApp &app = TGXApp::GetInstance();
  Cmds.Delete(0);
  TXAtomPList Atoms = app.FindXAtoms(Cmds, false, false);
  app.SetAtomDrawingStyle(ads, Atoms.IsEmpty() ? NULL : &Atoms);
}
//.............................................................................
void GXLibMacros::macAZoom(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if( !Cmds[0].IsNumber() )  {
    Error.ProcessingError(__OlxSrcInfo,
      "a number is expected as first argument");
    return;
  }
  TGXApp &app = TGXApp::GetInstance();
  double zoom = Cmds[0].ToDouble();
  Cmds.Delete(0);
  TXAtomPList Atoms = app.FindXAtoms(Cmds, false, false);
  app.AtomZoom(zoom, Atoms.IsEmpty() ? NULL : &Atoms);
}
//.............................................................................
void GXLibMacros::macBRad(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  double r = Cmds[0].ToDouble();
  Cmds.Delete(0);
  TXBondPList bonds;
  bool absolute = Options.GetBoolOption('a');
  if( Cmds.Count() == 1 && Cmds[0].Equalsi("hbonds") )  {
    if (absolute) r /= 0.02;
    TGXApp::BondIterator bi = app.GetBonds();
    while( bi.HasNext() )  {
      TXBond& xb = bi.Next();
      if( xb.GetType() == sotHBond )
        bonds.Add(xb);
    }
    app.BondRad(r, &bonds);
  }
  else  {
    if (absolute) r /= 0.1;
    bonds = app.GetBonds(Cmds, true);
    if( bonds.IsEmpty() && Cmds.IsEmpty() )  {  // get all non-H
      TGXApp::BondIterator bi = app.GetBonds();
      while( bi.HasNext() )  {
        TXBond& xb = bi.Next();
        if( xb.GetType() != sotHBond )
          bonds.Add(xb);
      }
      TXBond::DefR(r);
    }
    app.BondRad(r, &bonds);
  }
}
//.............................................................................
void GXLibMacros::macTelpV(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp::GetInstance().CalcProbFactor(Cmds[0].ToDouble());
}
//.............................................................................
void GXLibMacros::macInfo(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  TStrList Output;
  app.InfoList(Cmds.IsEmpty() ? EmptyString() : Cmds.Text(' '),
    Output, Options.Contains("p"),
    Options.FindValue('p', "-3").ToInt(),
    Options.GetBoolOption('f')
  );
  TBasicApp::NewLogEntry() << Output;
  if (Options.Contains('c'))
    app.ToClipboard(Output);
}
//.............................................................................
void GXLibMacros::macLabels(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  short lmode = 0;
  if (Options.Contains('p'))   lmode |= lmPart;
  if (Options.Contains('l'))   lmode |= lmLabels;
  if (Options.Contains('v'))   lmode |= lmOVar;
  if (Options.Contains('o'))   lmode |= lmOccp;
  if (Options.Contains("ao"))  lmode |= lmAOcc;
  if (Options.Contains('u'))   lmode |= lmUiso;
  if (Options.Contains('r'))   lmode |= lmUisR;
  if (Options.Contains('a'))   lmode |= lmAfix;
  if (Options.Contains('h'))   lmode |= lmHydr;
  if (Options.Contains('f'))   lmode |= lmFixed;
  if (Options.Contains("qi"))  lmode |= lmQPeakI;
  if (Options.Contains('i'))   lmode |= lmIdentity;
  if (Options.Contains("co"))  lmode |= lmCOccu;
  TGXApp &app = TGXApp::GetInstance();
  if( lmode == 0 )  {
    lmode |= lmLabels;
    lmode |= lmQPeak;
    app.SetLabelsMode(lmode);
    if (Cmds.Count() == 1 && Cmds[0].IsBool())
      app.SetLabelsVisible(Cmds[0].ToBool());
    else
      app.SetLabelsVisible(!app.AreLabelsVisible());
  }
  else  {
    app.SetLabelsMode(lmode |= lmQPeak);
    app.SetLabelsVisible(true);
  }
  TStateRegistry::GetInstance().SetState(app.stateLabelsVisible,
    app.AreLabelsVisible(), EmptyString(), true);
}
//.............................................................................
void GXLibMacros::macLabel(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TGXApp &app = TGXApp::GetInstance();
  TXAtomPList atoms;
  TXBondPList bonds;
  TPtrList<TXLine> lines;
  if( Cmds.IsEmpty() )  {
    TGlGroup& sel = app.GetSelection();
    for( size_t i=0; i < sel.Count(); i++)  {
      if( EsdlInstanceOf(sel[i], TXAtom) )
        atoms.Add((TXAtom&)sel[i]);
      else if( EsdlInstanceOf(sel[i], TXBond) )
        bonds.Add((TXBond&)sel[i]);
      else if( EsdlInstanceOf(sel[i], TXLine) )
        lines.Add((TXLine&)sel[i]);
    }
    if( atoms.IsEmpty() && bonds.IsEmpty() && lines.IsEmpty() )  {
      TGXApp::AtomIterator ai = app.GetAtoms();
      atoms.SetCapacity(ai.count);
      while( ai.HasNext() )  {
        TXAtom& xa = ai.Next();
        if( xa.IsVisible() )
          atoms.Add(xa);
      }
      TGXApp::BondIterator bi = app.GetBonds();
      bonds.SetCapacity(bi.count);
      while( bi.HasNext() )  {
        TXBond& xb = bi.Next();
        if( xb.IsVisible() )
          bonds.Add(xb);
      }
    }
  }
  else
    atoms = app.FindXAtoms(Cmds, true, false);
  short lt = 0, symm_tag = 0;
  const olxstr str_lt = Options.FindValue("type");
  olxstr str_symm_tag = Options.FindValue("symm");
  // enforce the default
  if( str_lt.Equalsi("brackets") )
    lt = 1;
  else if( str_lt.Equalsi("subscript") )
    lt = 2;
  // have to kill labels in this case, for consistency of _$ or ^#
  if( str_symm_tag =='$' || str_symm_tag == '#' )  {
    for( size_t i=0; i < app.LabelCount(); i++ )
      app.GetLabel(i).SetVisible(false);
    symm_tag = (str_symm_tag =='$' ? 1 : 2);
  }
  else if( str_symm_tag.Equals("full") )
    symm_tag = 3;
  TTypeList<uint32_t> equivs;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    TXGlLabel& gxl = atoms[i]->GetGlLabel();
    olxstr lb;
    if( lt != 0 &&
        atoms[i]->GetLabel().Length() > atoms[i]->GetType().symbol.Length() )
    {
      olxstr bcc = atoms[i]->GetLabel().SubStringFrom(
        atoms[i]->GetType().symbol.Length());
      lb = atoms[i]->GetType().symbol;
      if( lt == 1 )
        lb << '(' << bcc << ')';
      else if( lt == 2 )
        lb << "\\-" << bcc;
    }
    else
      lb = atoms[i]->GetLabel();
    if( !atoms[i]->IsAUAtom() )  {
      if( symm_tag == 1 || symm_tag == 2 )  {
        size_t pos = equivs.IndexOf(atoms[i]->GetMatrix().GetId());
        if( pos == InvalidIndex )  {
          equivs.AddCopy(atoms[i]->GetMatrix().GetId());
          pos = equivs.Count()-1;
        }
        if( symm_tag == 1 )
          lb << "_$" << (pos+1);
        else
          lb << "\\+" << (pos+1);
      }
      else if( symm_tag == 3 )
        lb << ' ' << TSymmParser::MatrixToSymmEx(atoms[i]->GetMatrix());
    }
    gxl.SetOffset(atoms[i]->crd());
    gxl.SetLabel(lb);
    gxl.SetVisible(true);
  }
  TPtrList<TXGlLabel> labels;
  if( !bonds.IsEmpty() )  {
    VcoVContainer vcovc(app.XFile().GetAsymmUnit());
    bool have_vcov = false;
    try  {
      olxstr src_mat = app.InitVcoV(vcovc);
      app.NewLogEntry() << "Using " << src_mat <<
        " matrix for the calculation";
      have_vcov = true;
    }
    catch(const TExceptionBase&)  {}
    for( size_t i=0; i < bonds.Count(); i++ )  {
      TXGlLabel& l = bonds[i]->GetGlLabel();
      l.SetOffset(bonds[i]->GetCenter());
      if( have_vcov ) {
        l.SetLabel(vcovc.CalcDistance(bonds[i]->A(),
          bonds[i]->B()).ToString());
      }
      else
        l.SetLabel(olxstr::FormatFloat(3, bonds[i]->Length()));
      labels.Add(l)->SetVisible(true);
      l.TranslateBasis(-l.GetCenter());
    }
  }
  for( size_t i=0; i < lines.Count(); i++ )
    lines[i]->GetGlLabel().SetVisible(true);

  for( size_t i=0; i < equivs.Count(); i++ )  {
    smatd m = app.XFile().GetUnitCell().GetMatrix(
      smatd::GetContainerId(equivs[i]));
    m.t += smatd::GetT(equivs[i]);
    olxstr line("$");
    line << (i+1);
    line.RightPadding(4, ' ', true) << TSymmParser::MatrixToSymmEx(m);
    if( i != 0 && (i%3) == 0 )
      TBasicApp::GetLog() << NewLineSequence();
    TBasicApp::GetLog() << line.RightPadding(26, ' ');
  }
  TBasicApp::GetLog() << NewLineSequence();

  const olxstr _cif = Options.FindValue("cif");
  if( !_cif.IsEmpty() )  {
    if( app.CheckFileType<TCif>() )  {
      const TCifDataManager& cifdn =
        app.XFile().GetLastLoader<TCif>().GetDataManager();
      const TGlGroup& sel = app.GetSelection();
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( EsdlInstanceOf(sel[i], TXBond) )  {
          TXBond& xb = (TXBond&)sel[i];
          ACifValue* v = cifdn.Match(xb.A(), xb.B());
          if( v == NULL )  continue;
          TXGlLabel& l = xb.GetGlLabel();
          l.SetOffset(xb.GetCenter());
          l.SetLabel(v->GetValue().ToString());
          labels.Add(l);
        }
      }
    }
    else  {
      const TGlGroup& sel = app.GetSelection();
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( EsdlInstanceOf(sel[i], TXBond) )  {
          TXBond& xb = (TXBond&)sel[i];
          TXGlLabel& l = xb.GetGlLabel();
          l.SetOffset(xb.GetCenter());
          l.SetLabel(olxstr::FormatFloat(2, xb.Length()));
          labels.Add(l);
        }
      }
    }
  }
  for( size_t i=0; i < labels.Count(); i++ )  {
    TXGlLabel& l = *labels[i];
    vec3d off(-l.GetRect().width/2, -l.GetRect().height/2, 0);
    const double scale1 =
      l.GetFont().IsVectorFont() ? 1.0/app.GetRender().GetScale() : 1.0;
    const double scale = scale1/app.GetRender().GetBasis().GetZoom();
    l.TranslateBasis(off*scale);
    l.SetVisible(true);
  }
  app.SelectAll(false);
}
//.............................................................................
void GXLibMacros::macShowH(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TGXApp &app = TGXApp::GetInstance();
  TEBasis basis = app.GetRender().GetBasis();
  if( Cmds.Count() == 2 )  {
    bool v = Cmds[1].ToBool();
    if( Cmds[0] == 'a' )  {
      if( v && !app.AreHydrogensVisible() )  {
        app.SetHydrogensVisible(true);
      }
      else if( !v && app.AreHydrogensVisible() )  {
        app.SetHydrogensVisible(false);
      }
    }
    else if( Cmds[0] == 'b' )  {
      if( v && !app.AreHBondsVisible() )  {
        app.SetHBondsVisible(true);
      }
      else if( !v && app.AreHBondsVisible() )  {
        app.SetHBondsVisible(false);
      }
    }
  }
  else  {
    if( app.AreHydrogensVisible() && !app.AreHBondsVisible() )  {
      app.SetHBondsVisible(true);
    }
    else if( app.AreHydrogensVisible() && app.AreHBondsVisible() )  {
      app.SetHBondsVisible(false);
      app.SetHydrogensVisible(false);
    }
    else if( !app.AreHydrogensVisible() && !app.AreHBondsVisible() )  {
      app.SetHydrogensVisible(true);
    }
  }
  TStateRegistry::GetInstance().SetState(app.stateHydrogensVisible,
    app.AreHydrogensVisible(), EmptyString(), true);
  TStateRegistry::GetInstance().SetState(app.stateHydrogenBondsVisible,
    app.AreHBondsVisible(), EmptyString(), true);
  app.GetRender().SetBasis(basis);
}
//.............................................................................
int GXLibMacros::QPeakSortA(const TCAtom &a, const TCAtom &b)  {
  double v = a.GetQPeak() - b.GetQPeak();
  if (v == 0 && a.GetLabel().Length() > 1 && b.GetLabel().Length() > 1) {
    if (a.GetLabel().SubStringFrom(1).IsNumber() &&
        b.GetLabel().SubStringFrom(1).IsNumber())
    {
      v = b.GetLabel().SubStringFrom(1).ToInt() -
        a.GetLabel().SubStringFrom(1).ToInt();
    }
  }
  return v < 0 ? 1 : (v > 0 ? -1 : 0);
}
void GXLibMacros::macShowQ(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  double wheel = Options.FindValue("wheel", '0').ToDouble();
  TGXApp &app = TGXApp::GetInstance();
  TEBasis basis = app.GetRender().GetBasis();
  if( wheel != 0 )  {
    //if( !app.QPeaksVisible() )  return;
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    TCAtomPList qpeaks;
    for( size_t i=0; i < au.AtomCount(); i++ )
      if( !au.GetAtom(i).IsDeleted() && au.GetAtom(i).GetType() == iQPeakZ )
        qpeaks.Add(au.GetAtom(i));
    QuickSorter::SortSF(qpeaks, GXLibMacros::QPeakSortA);
    index_t d_cnt = 0;
    for( size_t i=0; i < qpeaks.Count(); i++ )
      if( !qpeaks[i]->IsDetached() )
        d_cnt++;
    if( d_cnt == 0 && wheel < 0 )  return;
    if( d_cnt == (index_t)qpeaks.Count() && wheel > 0 )  return;
    d_cnt += (index_t)(wheel);
    if( d_cnt < 0 )  d_cnt = 0;
    if( d_cnt > (index_t)qpeaks.Count() )
      d_cnt = qpeaks.Count();
    for( size_t i=0; i < qpeaks.Count(); i++ )
      qpeaks[i]->SetDetached(i >= (size_t)d_cnt);
    //app.XFile().GetLattice().UpdateConnectivityInfo();
    app.UpdateConnectivity();
    app.Draw();
  }
  else if( Cmds.Count() == 2 )  {
    bool v = Cmds[1].ToBool();
    if( Cmds[0] == "a" )  {
      if( v && !app.AreQPeaksVisible() )  {
        app.SetQPeaksVisible(true);
      }
      else if( !v && app.AreQPeaksVisible() )  {
        app.SetQPeaksVisible(false);
      }
    }
    else if( Cmds[0] == "b" )  {
      if( v && !app.AreQPeakBondsVisible() )  {
        app.SetQPeakBondsVisible(true);
      }
      else if( !v && app.AreQPeakBondsVisible() )  {
        app.SetQPeakBondsVisible(false);
      }
    }
  }
  else if( Cmds.Count() == 1 && Cmds[0].IsNumber() )  {
    index_t num = Cmds[0].ToInt();
    const bool negative = num < 0;
    if( num < 0 )  num = olx_abs(num);
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    TCAtomPList qpeaks;
    for( size_t i=0; i < au.AtomCount(); i++ )
      if( au.GetAtom(i).GetType() == iQPeakZ )
        qpeaks.Add(au.GetAtom(i));
    QuickSorter::SortSF(qpeaks,
      negative ? GXLibMacros::QPeakSortD : GXLibMacros::QPeakSortA);
    num = olx_min(qpeaks.Count()*num/100, qpeaks.Count());
    for( size_t i=0; i < qpeaks.Count(); i++ )  
      qpeaks[i]->SetDetached( i >= (size_t)num );
    app.GetSelection().Clear();
    app.UpdateConnectivity();
    app.Draw();
  }
  else  {
    if( (!app.AreQPeaksVisible() && !app.AreQPeakBondsVisible()) )  {
      app.SetQPeaksVisible(true);
    }
    else if( app.AreQPeaksVisible() && !app.AreQPeakBondsVisible())  {
      app.SetQPeakBondsVisible(true);
    }
    else if( app.AreQPeaksVisible() && app.AreQPeakBondsVisible() )  {
      app.SetQPeaksVisible(false);
      app.SetQPeakBondsVisible(false);
    }
  }
  TStateRegistry::GetInstance().SetState(app.stateQPeaksVisible,
    app.AreQPeaksVisible(), EmptyString(), true);
  TStateRegistry::GetInstance().SetState(app.stateQPeakBondsVisible,
    app.AreQPeakBondsVisible(), EmptyString(), true);
  app.GetRender().SetBasis(basis);
}
//.............................................................................
void GXLibMacros::macLoad(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  if (Cmds.IsEmpty() || !Cmds[0].Equalsi("textures")) {
    Error.SetUnhandled(true);
    return;
  }
  Cmds.Delete(0);
  TGXApp &app = TGXApp::GetInstance();
  if (Cmds.IsEmpty())
    app.ClearTextures(~0);
  else
    app.LoadTextures(Cmds.Text(' '));
}
//.............................................................................
void GXLibMacros::macMatr(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  if (Cmds.IsEmpty()) {
    TBasicApp::NewLogEntry() << "Current orientation matrix:";
    const mat3d& Matr = app.GetRender().GetBasis().GetMatrix();
    for (size_t i=0; i < 3; i++) {
      olxstr Tmp;
      for (size_t j=0; j < 3; j++) {
        Tmp << olxstr::FormatFloat(4, Matr[j][i]);
        Tmp.RightPadding(7*(j+1), ' ', true);
      }
      TBasicApp::NewLogEntry() << Tmp;
    }
  }
  else {
    const mat3d& M = Options.Contains('r') ?
      app.XFile().GetAsymmUnit().GetHklToCartesian()
      : app.XFile().GetAsymmUnit().GetCellToCartesian();
    if (Cmds.Count() == 1) {
      olxstr arg;
      if (Cmds[0] == '1' || Cmds[0] == 'a')
        arg = "100";
      else if (Cmds[0] == '2' || Cmds[0] == 'b')
        arg = "010";
      else if (Cmds[0] == '3' || Cmds[0] == 'c')
        arg = "001";
      else
        arg = Cmds[0];
      if ((arg.Length()%3) != 0 ) {
        Error.ProcessingError(__OlxSrcInfo,
          "invalid argument, an arguments like 010, 001000, +0-1+1 etc is"
          " expected");
        return;
      }
      vec3d n;
      const size_t s = arg.Length()/3;
      for (int i=0; i < 3; i++)
        n += M[i]*arg.SubString(s*i, s).ToInt();
      if (n.QLength() < 1e-3) {
        Error.ProcessingError(__OlxSrcInfo,
          "non zero expression is expected");
        return;
      }
      app.GetRender().GetBasis().OrientNormal(n);
    }
    else if (Cmds.Count() == 2) {  // from to view
      if ((Cmds[0].Length()%3) != 0 || (Cmds[1].Length()%3) != 0) {
        Error.ProcessingError(__OlxSrcInfo,
          "invalid arguments, a klm, two arguments like 010, 001000, +0-1+1 "
          "etc are expected");
        return;
      }
      vec3d from, to;
      const size_t fs = Cmds[0].Length()/3, ts = Cmds[1].Length()/3;
      for (int i=0; i < 3; i++) {
        from += M[i]*Cmds[0].SubString(fs*i, fs).ToInt();
        to += M[i]*Cmds[1].SubString(ts*i, ts).ToInt();
      }
      vec3d n = from-to;
      if (n.QLength() < 1e-3 ) {
        Error.ProcessingError(__OlxSrcInfo,
          "from and to arguments must be different");
        return;
      }
      app.GetRender().GetBasis().OrientNormal(n);
    }
    else if (Cmds.Count() == 3) {  // view along
      vec3d n = M[0]*Cmds[0].ToDouble() +
        M[1]*Cmds[1].ToDouble() +
        M[2]*Cmds[2].ToDouble();
      if (n.IsNull(1e-6)) {
        Error.ProcessingError(__OlxSrcInfo,
          "a non-singular direction is expected");
        return;
      }
      app.GetRender().GetBasis().OrientNormal(n);
    }
    else if (Cmds.Count() == 9) {
      mat3d M(
        Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble(),
        Cmds[3].ToDouble(), Cmds[4].ToDouble(), Cmds[5].ToDouble(),
        Cmds[6].ToDouble(), Cmds[7].ToDouble(), Cmds[8].ToDouble());
      M.Transpose();
      app.GetRender().GetBasis().SetMatrix(M.Normalise());
    }
    app.Draw();
  }
}
//.............................................................................
void GXLibMacros::macSetView(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  const bool do_center = Options.Contains('c');
  bool process = false, has_center = true;
  vec3d center, normal;
  if (Cmds.IsEmpty()) {
    TGlGroup& g = app.GetSelection();
    if (g.Count() == 1) {
      if (EsdlInstanceOf(g[0], TXPlane)) {
        TXPlane& xp = (TXPlane&)g[0];
        normal = xp.GetNormal();
        center = xp.GetCenter();
        process = true;
      }
      else if (EsdlInstanceOf(g[0], TXBond)) {
        TXBond& xb = (TXBond&)g[0];
        normal = vec3d(xb.B().crd()-xb.A().crd()).Normalise();
        center = (xb.B().crd()+xb.A().crd())/2;
        process = true;
      }
    }
  }
  else  {
    if (Cmds.Count() == 3) {
      for (int i=0; i < 3; i++)
        normal[i] = Cmds[i].ToDouble();
      normal.Normalise();
      process = true;
      has_center = false;
    }
    else if (Cmds.Count() == 6) {
      for (int i=0; i < 3; i++) {
        normal[i] = Cmds[i].ToDouble();
        center[i] = Cmds[3+i].ToDouble();
      }
      normal.Normalise();
      process = true;
    }
  }
  if (!process) {
    TXAtomPList xatoms = app.FindXAtoms(Cmds, true, false);
    if( xatoms.Count() < 2 )  {
      Error.ProcessingError(__OlxSrcInfo, "At least two atoms are required");
      return;
    }
    process = true;
    if (xatoms.Count() == 2) {
      center = (xatoms[0]->crd()+xatoms[1]->crd())/2;
      normal = (xatoms[1]->crd()-xatoms[0]->crd()).Normalise();
    }
    else {
      TSAtomPList satoms(xatoms, StaticCastAccessor<TSAtom>());
      mat3d params;
      vec3d rms;
      TSPlane::CalcPlanes(satoms, params, rms, center);
      normal = params[2];
    }
  }
  if (process) {
    app.GetRender().GetBasis().OrientNormal(normal);
    if (do_center && has_center)
      app.GetRender().GetBasis().SetCenter(-center);
    if (app.XGrid().IsVisible() && has_center)
      app.SetGridDepth(center);
    app.Draw();
  }
}
//.............................................................................
void GXLibMacros::macLine(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  bool process_atoms = true;
  vec3d from, to;
  if (Cmds.IsEmpty() || (Cmds.Count() == 1 && Cmds[0].Equalsi("sel"))) {
    TGlGroup& sel = app.GetSelection();
    if( sel.Count() == 2 )  {
      if (EsdlInstanceOf(sel[0], TXPlane) && EsdlInstanceOf(sel[1], TXPlane)) {
        from = ((TXPlane&)sel[0]).GetCenter();
        to = ((TXPlane&)sel[1]).GetCenter();
        process_atoms = false;
      }
      else if (EsdlInstanceOf(sel[0], TXAtom) &&
        EsdlInstanceOf(sel[1], TXPlane))
      {
        from = ((TXAtom&)sel[0]).crd();
        to = ((TXPlane&)sel[1]).GetCenter();
        process_atoms = false;
      }
      else if (EsdlInstanceOf(sel[0], TXPlane) &&
        EsdlInstanceOf(sel[1], TXAtom))
      {
        from = ((TXPlane&)sel[0]).GetCenter();
        to = ((TXAtom&)sel[1]).crd();
        process_atoms = false;
      }
    }
  }
  TXAtomPList Atoms;
  if (process_atoms)
    Atoms = app.FindXAtoms(Cmds, true, true);
  if (Atoms.Count() > 2) {
    TSAtomPList satoms(Atoms, StaticCastAccessor<TSAtom>());
    mat3d params;
    vec3d rms, center;
    TSPlane::CalcPlanes(satoms, params, rms, center);
    double maxl = -1000, minl = 1000;
    for( size_t i=0; i < satoms.Count(); i++ )  {
      vec3d v = satoms[i]->crd() - center;
      if (v.QLength() < 0.0001) continue;
      const double ca = params[2].CAngle(v);
      const double l = v.Length()*ca;
      if (l > maxl) maxl = l;
      if (l < minl) minl = l;
    }
    from = center+params[2]*minl;
    to = center+params[2]*maxl;
  }
  else if (Atoms.Count() == 2) {
    from = Atoms[0]->crd();
    to = Atoms[1]->crd();
  }

  else if (process_atoms) {
    Error.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
    return;
  }
  olxstr name = Options.FindValue('n');
  if (Options.Contains('n') && name.IsEmpty())
    app.GetRender().GetBasis().OrientNormal(to-from);
  else
    app.AddLine(name, from, to);
  app.Draw();
}
//.............................................................................
void GXLibMacros::macMpln(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  TSPlane* plane = NULL;
  bool orientOnly = Options.Contains('n'),
    rectangular = Options.Contains('r');
  olxstr name = Options.FindValue('n');
  const double weightExtent = olx_abs(Options.FindValue("we", "0").ToDouble());
  olxstr planeName;
  TXAtomPList Atoms = app.FindXAtoms(Cmds, true, true);
  for (size_t i=0; i < Atoms.Count(); i++ ) {
    planeName << Atoms[i]->GetLabel();
    if( i+1 < Atoms.Count() )
      planeName << ' ';
  }

  if (Atoms.Count() < 3) {
    Error.ProcessingError(__OlxSrcInfo, "at least 3 atoms are expected");
    return;
  }
  if (orientOnly && name.IsEmpty()) {
    plane = app.TmpPlane(&Atoms, weightExtent);
    if( plane != NULL )  {
      mat3d m = plane->GetBasis();
      vec3d d1 = (m[2]+m[1]).Normalise();
      vec3d d2 = m[0].XProdVec(d1).Normalise();
      app.GetRender().GetBasis().Orient(d1, d2, m[0]);
      app.SetGridDepth(plane->GetCenter());
      delete plane;
      plane = NULL;
    }
  }
  else  {
    TXPlane* xp = app.AddPlane(name, Atoms, rectangular, weightExtent);
    if (xp != NULL)
      plane = xp;
  }
  if (plane != NULL) {
    const TAsymmUnit& au = app.XFile().GetAsymmUnit();
    size_t colCount = 3;
    TTTable<TStrList> tab(plane->Count()/colCount +
      (((plane->Count()%colCount)==0)?0:1), colCount*3);
    for (size_t i=0; i < colCount; i++) {
      tab.ColName(i*3) = "Label";
      tab.ColName(i*3+1) = "D/A";
    }
    double rmsd = 0;
    for (size_t i=0; i < plane->Count(); i+=colCount) {
      for (size_t j=0; j < colCount; j++) {
        if ((i + j) >= Atoms.Count())
          break;
        tab[i/colCount][j*3] = plane->GetAtom(i+j).GetLabel();
        const double v = plane->DistanceTo(plane->GetAtom(i+j).crd()); 
        rmsd += v*v;
        tab[i/colCount][j*3+1] = olxstr::FormatFloat(3, v);
      }
    }
    rmsd = sqrt(rmsd/Atoms.Count());
    TBasicApp::NewLogEntry() <<
      tab.CreateTXTList(olxstr("Atom-to-plane distances for ") << planeName,
      true, false, " | ");
    TBasicApp::NewLogEntry() << "Plane equation: " << plane->StrRepr();
    TBasicApp::NewLogEntry() << "HKL direction: " <<
      plane->GetCrystallographicDirection().ToString();
    if (weightExtent != 0) {
      TBasicApp::NewLogEntry() << "Weighted RMSD/A: " <<
        olxstr::FormatFloat(3, plane->GetWeightedRMSD());
      TBasicApp::NewLogEntry() << "RMSD/A: " <<
        olxstr::FormatFloat(3, plane->CalcRMSD());
    }
    else {
      TBasicApp::NewLogEntry() << "RMSD/A: " <<
        olxstr::FormatFloat(3, plane->GetWeightedRMSD());
    }
  }
  else if (!orientOnly) {
    TBasicApp::NewLogEntry() <<
      "The plane was not created because it is either not unique or valid";
  }
}
//.............................................................................
void GXLibMacros::macCent(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  app.AddCentroid(app.FindXAtoms(Cmds, true, true).GetObject());
}
//.............................................................................
void GXLibMacros::macUniq(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  TXAtomPList Atoms = app.FindXAtoms(Cmds, false, true);
  if( Atoms.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  TNetPList L(Atoms, FunctionAccessor::MakeConst(&TXAtom::GetNetwork));
  app.FragmentsVisible(app.InvertFragmentsList(
    ACollectionItem::Unique(L)), false);
  app.CenterView(true);
  app.Draw();
}
//.............................................................................
void GXLibMacros::macGroup(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  olxstr name = Options.FindValue('n');
  if (app.GetSelection().IsEmpty())
    app.SelectAll(true);
  if (name.IsEmpty())  {
    name = "group";
    name << (app.GetRender().GroupCount()+1);
  }
  app.GroupSelection(name);
}
//.............................................................................
void GXLibMacros::macFmol(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  app.AllVisible(true);
  app.CenterView();
  app.GetRender().GetBasis().SetZoom(
    app.GetRender().CalcZoom()*app.GetExtraZoom());
}
//.............................................................................
void GXLibMacros::macCell(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  app.SetCellVisible(Cmds.IsEmpty() ? !app.IsCellVisible() : Cmds[0].ToBool());
  if (app.DUnitCell().IsVisible()) {
    bool r = Options.GetBoolOption('r');
    app.DUnitCell().SetReciprocal(r, r ? 100: 1);
  }
  app.CenterView();
}
//.............................................................................
void GXLibMacros::macSel(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  if (TModeRegistry::GetInstance().GetCurrent() != NULL) {
    TBasicApp::NewLogEntry(logError) << "Unavailable in a mode";
    return;
  }
  if (Cmds.Count() >= 1 && Cmds[0].Equalsi("frag")) {
    TNetPList nets(
      app.FindXAtoms(TStrObjList(Cmds.SubListFrom(1)), false, false),
      FunctionAccessor::MakeConst(&TXAtom::GetNetwork));
    app.SelectFragments(ACollectionItem::Unique(nets), !Options.Contains('u'));
  }
  else if( Cmds.Count() == 1 && Cmds[0].Equalsi("res") )  {
    //app.GetRender().ClearSelection();
    //TStrList out;
    //TCAtomPList a_res;
    //TPtrList<TSimpleRestraint> b_res;
    //RefinementModel& rm = app.XFile().GetRM();
    //rm.Describe(out, &a_res, &b_res);
    //app.XFile().GetAsymmUnit().GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    //for( size_t i=0; i < a_res.Count(); i++ )
    //  a_res[i]->SetTag(1);
    //TGXApp::AtomIterator ai = app.GetAtoms();
    //while( ai.HasNext() )  {
    //  TXAtom& xa = ai.Next();
    //  if( xa.CAtom().GetTag() != 1 )  continue;
    //  if( xa.IsSelected() )  continue;
    //  app.GetRender().Select(xa);
    //}
    //for( size_t i=0; i < b_res.Count(); i++ )  {
    //  const TSimpleRestraint& res = *b_res[i];
    //  for( size_t j=0; j < res.AtomCount(); j+=2 )  {
    //    if( res.GetAtom(j).GetMatrix() != NULL || res.GetAtom(j+1).GetMatrix() != NULL )  continue;
    //    const size_t id1 = res.GetAtom(j).GetAtom()->GetId();
    //    const size_t id2 = res.GetAtom(j+1).GetAtom()->GetId();
    //    TGXApp::BondIterator bi = app.GetBonds();
    //    while( bi.HasNext() )  {
    //      TXBond& xb = bi.Next();
    //      if( xb.IsSelected() )  continue;
    //      const TCAtom& ca1 = xb.A().CAtom();
    //      const TCAtom& ca2 = xb.B().CAtom();
    //      if( (ca1.GetId() == id1 && ca2.GetId() == id2) ||
    //          (ca1.GetId() == id2 && ca2.GetId() == id1) )  
    //      {
    //        app.GetRender().Select(xb);
    //        break;
    //      }
    //    }
    //  }
    //}
    //FGlConsole->PrintText(out);
    //app.GetLog() << NewLineSequence();
  }
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("resi")) {
    TAsymmUnit &au = app.XFile().GetAsymmUnit();
    au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    TSizeList nums;
    TStrList names;
    for (size_t i=1; i < Cmds.Count(); i++) {
      if (Cmds[i].IsInt())
        nums << Cmds[i].ToSizeT();
      else
        names << Cmds[i];
    }
    for (size_t i=0; i < au.ResidueCount(); i++) {
      TResidue &r = au.GetResidue(i);
      bool found = false;
      for (size_t j=0; j < nums.Count(); j++) {
        if (nums[j] == r.GetNumber() || nums[j] == r.GetAlias()) {
          found = true;
          break;
        }
      }
      if (!found) {
        for (size_t j=0; j < names.Count(); j++) {
          if (r.GetClassName().Equalsi(names[j])) {
            found = true;
            break;
          }
        }
      }
      if (!found) continue;
      for (size_t j=0; j < r.Count(); j++)
        r[j].SetTag(1);

    }
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext()) {
      TXAtom &a = ai.Next();
      if (a.IsVisible() && a.CAtom().GetTag() == 1)
        app.GetRender().Select(a, true);
    }
  }
  else if (Cmds.Count() == 1 && TSymmParser::IsRelSymm(Cmds[0])) {
    bool invert = Options.Contains('i'), unselect=false;
    if( !invert )
      unselect = Options.Contains('u');
    const smatd matr = TSymmParser::SymmCodeToMatrix(
      app.XFile().GetUnitCell(), Cmds[0]);
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext()) {
      TXAtom& a = ai.Next();
      if (a.IsDeleted() || !a.IsVisible() )  continue;
      if (a.IsGenerator(matr) )  {
        if (invert)
          app.GetRender().Select(a, !a.IsSelected());
        else if (unselect)
          app.GetRender().Select(a, false);
        else
          app.GetRender().Select(a, true);
      }
    }
    TGXApp::BondIterator bi = app.GetBonds();
    while (bi.HasNext()) {
      TXBond& b = bi.Next();
      if (b.IsDeleted() || !b.IsVisible())  continue;
      if (b.A().IsGenerator(matr) && b.B().IsGenerator(matr)) {
        if (invert)
          app.GetRender().Select(b, !b.IsSelected());
        else if (unselect)
          app.GetRender().Select(b, false);
        else
          app.GetRender().Select(b, true);
      }
    }
  }
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("part")) {
    Cmds.Delete(0);
    TIntList parts;
    for (size_t i=0; Cmds.Count(); i++) {
      if (Cmds[i].IsNumber()) {
        parts.Add(Cmds[i].ToInt());
        Cmds.Delete(i--);
      }
      else
        break;
    }
    if (!parts.IsEmpty()) {
      olxstr cond = "xatom.part==";
      cond << parts[0];
      for (size_t i=1; i < parts.Count(); i++)
        cond << "||xatom.part==" << parts[i];
      app.SelectAtomsWhere(cond);
    }
  }
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("fvar")) {
    Cmds.Delete(0);
    TIntList fvars;
    for (size_t i=0; Cmds.Count(); i++) {
      if (Cmds[i].IsNumber()) {
        int &v = fvars.Add(Cmds[i].ToInt());
        v = olx_sign(v)*(olx_abs(v)-1);
        Cmds.Delete(i--);
      }
      else
        break;
    }
    if (!fvars.IsEmpty()) {
      TGXApp::AtomIterator ai = app.GetAtoms();
      while (ai.HasNext()) {
        TXAtom &a = ai.Next();
        XVarReference *ref = a.CAtom().GetVarRef(catom_var_name_Sof);
        if (ref == NULL) continue;
        int v = int(ref->Parent.GetId());
        if (ref->relation_type == relation_AsOneMinusVar)
          v *= -1;
        for (size_t i=0; i < fvars.Count(); i++) {
          if (fvars[i] == v)
            app.GetRender().Select(a, true);
        }
      }
    }
  }
  else if (Cmds.Count() == 1 && Cmds[0].Equalsi("isot")) {
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext())  {
      TXAtom& xa = ai.Next();
      if (xa.GetEllipsoid() == NULL)
        app.GetRender().Select(xa, true);
    }
  }
  else if (Cmds.Count() >= 1 && Cmds[0].Equalsi("wbox")) {
    if (Cmds.Count() >= 2 && Cmds[1].Equalsi("cell")) {
      const mat3d m = app.XFile().GetAsymmUnit().GetCellToCartesian();
      T3DFrameCtrl& fr = app.Get3DFrame();
      fr.SetEdge(0, vec3d());
      fr.SetEdge(1, m[1]);
      fr.SetEdge(2, m[0] + m[1]);
      fr.SetEdge(3, m[0]);
      fr.SetEdge(4, m[2]);
      fr.SetEdge(5, m[1] + m[2]);
      fr.SetEdge(6, m[0] + m[1] + m[2]);
      fr.SetEdge(7, m[0] + m[2]);
      fr.UpdateEdges();
      app.SetGraphicsVisible(&fr, true);
    }
    else {
      TSAtomPList atoms;
      if (app.FindSAtoms(EmptyString(), atoms, true)) {
        const WBoxInfo bs = TXApp::CalcWBox(atoms, NULL, TSAtom::weight_occu);
        T3DFrameCtrl& fr = app.Get3DFrame();
        vec3d nx = bs.normals[0]*bs.s_from[0];
        vec3d px = bs.normals[0]*bs.s_to[0];
        vec3d ny = bs.normals[1]*bs.s_from[1];
        vec3d py = bs.normals[1]*bs.s_to[1];
        vec3d nz = bs.normals[2]*bs.s_from[2];
        vec3d pz = bs.normals[2]*bs.s_to[2];
        if (ny.XProdVec(nx).DotProd(nz) < 0) {
          olx_swap(nx, px);
          olx_swap(ny, py);
          olx_swap(nz, pz);
        }
        fr.SetEdge(0, nx+ny+nz);
        fr.SetEdge(1, nx+py+nz);
        fr.SetEdge(2, px+py+nz);
        fr.SetEdge(3, px+ny+nz);
        fr.SetEdge(4, nx+ny+pz);
        fr.SetEdge(5, nx+py+pz);
        fr.SetEdge(6, px+py+pz);
        fr.SetEdge(7, px+ny+pz);
        fr.UpdateEdges();
        fr.Translate(bs.center);
        app.SetGraphicsVisible(&fr, true);
      }
    }
  }
  else if (!(Options.Contains('a') ||
    Options.Contains('i') ||
    Options.Contains('u')))
  {
    size_t period=5;
    TXAtomPList Atoms = app.FindXAtoms("sel", false, false);
    if (Atoms.IsEmpty() && Cmds.Count() == 1) {
      TGPCollection* gpc = app.GetRender().FindCollection(Cmds[0]);
      if (gpc != NULL) {
        for (size_t i=0; i < gpc->ObjectCount(); i++)
          app.GetRender().Select(gpc->GetObject(i));
        return;
      }
    }
    for (size_t i=0; i <= Atoms.Count(); i+=period) {
      olxstr Tmp;
      for( size_t j=0; j < period; j++ )  {
        if( (j+i) >= Atoms.Count() )  break;
        Tmp << Atoms[i+j]->GetGuiLabel();
        Tmp.RightPadding((j+1)*14, ' ', true);
      }
      if (!Tmp.IsEmpty())
        TBasicApp::NewLogEntry() << Tmp;
    }
    if (!Cmds.IsEmpty()) {
      size_t whereIndex = Cmds.IndexOf("where");
      if (whereIndex >= 1 && whereIndex != InvalidIndex) {
        olxstr Tmp = Cmds[whereIndex-1];
        while (olx_is_valid_index(whereIndex)) { Cmds.Delete(whereIndex--); }
        if (Tmp.Equalsi("atoms"))
          app.SelectAtomsWhere(Cmds.Text(' '));
        else if (Tmp.Equalsi("bonds"))
          app.SelectBondsWhere(Cmds.Text(' '));
        else
          Error.ProcessingError(__OlxSrcInfo, "undefined keyword: " ) << Tmp;
        return;
      }
      else {
        size_t ringsIndex = Cmds.IndexOf("rings");
        if (ringsIndex != InvalidIndex) {
          Cmds.Delete( ringsIndex );
          app.SelectRings(Cmds.Text(' '));
        }
        else
          app.SelectAtoms(Cmds.Text(' '));
        return;
      }
    }
    olxstr seli = app.GetSelectionInfo(Options.Contains('l'));
    if (!seli.IsEmpty()) {
      app.NewLogEntry() << seli;
      if (Options.Contains('c'))
        app.ToClipboard(seli);
    }
  }
  else {
    for (size_t i=0; i < Options.Count(); i++) {
      if (Options.GetName(i)[0] == 'a')
        app.SelectAll(true);
      else if (Options.GetName(i)[0] == 'u')
        app.SelectAll(false);
      else if( Options.GetName(i)[0] == 'i' )  {
        if (Cmds.Count() == 0)
          app.GetRender().InvertSelection();
        else
          app.SelectAtoms(Cmds.Text(' '), true);
      }
      else
        Error.ProcessingError(__OlxSrcInfo, "wrong options");
    }
  }
}
//.............................................................................
void GXLibMacros::macUndo(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TGXApp &app = TGXApp::GetInstance();
  if (!app.GetUndo().isEmpty()) {
    TUndoData* data = app.GetUndo().Pop();
    data->Undo();
    delete data;
  }
}
//.............................................................................
void GXLibMacros::macBasis(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TGXApp &app = TGXApp::GetInstance();
  app.SetBasisVisible(
    Cmds.IsEmpty() ? !app.IsBasisVisible() : Cmds[0].ToBool());
}
//.............................................................................
void GXLibMacros::macPiM(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TGXApp &app = TGXApp::GetInstance();
  TTypeList<TSAtomPList> rings;
  TTypeList<TSAtomPList> ring_M;
  bool check_metal = true;
  ConstPtrList<TXAtom> atoms = app.FindXAtoms(Cmds, false, true);
  if( atoms.IsEmpty() )  {
    app.FindRings("C4", rings);
    app.FindRings("C5", rings);
    app.FindRings("C6", rings);
    for( size_t i=0; i < rings.Count(); i++ )  {
      if( TSPlane::CalcRMSD(rings[i]) > 0.05 ||
          !TNetwork::IsRingRegular(rings[i]) )
      {
        rings.NullItem(i);
      }
    }
    rings.Pack();
  }
  else  {
    rings.Add(new TSAtomPList(atoms, StaticCastAccessor<TSAtom>()));
    TSAtomPList &metals = ring_M.AddNew();
    for( size_t i=0; i < rings[0].Count(); i++ )  {
      if( XElementLib::IsMetal(rings[0][i]->GetType()) )  {
        check_metal = false;
        metals.Add(rings[0][i]);
        rings[0][i] = NULL;
      }
    }
    rings[0].Pack();
    if( rings[0].IsEmpty() )  return;
    if( check_metal )
      ring_M.Delete(0);
  }
  if( check_metal )  {
    ring_M.SetCapacity(rings.Count());
    for( size_t i=0; i < rings.Count(); i++ )  {
      TSAtomPList &metals = ring_M.AddNew();
      for( size_t j=0; j < rings[i].Count(); j++ )  {
        for( size_t k=0; k < rings[i][j]->NodeCount(); k++ )  {
          if( XElementLib::IsMetal(rings[i][j]->Node(k).GetType()) )  {
            metals.Add(rings[i][j]->Node(k));
          }
        }
      }
      if( metals.IsEmpty() )  {
        rings.NullItem(i);
        ring_M.NullItem(ring_M.Count()-1);
      }
    }
    rings.Pack();
    ring_M.Pack();
  }
  // process rings...  
  if( rings.IsEmpty() )  return;
  const bool label = Options.Contains('l');
  TGXApp::AtomIterator ai = app.GetAtoms();
  while( ai.HasNext() )
    ai.Next().SetTag(0);
  for( size_t i=0; i < rings.Count(); i++ )  {
    vec3d c;
    for( size_t j=0; j < rings[i].Count(); j++ )  {
      c += rings[i][j]->crd();
      rings[i][j]->SetTag(1);
    }
    c /= rings[i].Count();
    ACollectionItem::Unique(ring_M[i]);
    for( size_t j=0; j < ring_M[i].Count(); j++ )  {
      TXLine &l = app.AddLine(ring_M[i][j]->GetLabel()+olxstr(i),
        ring_M[i][j]->crd(), c);
      TGraphicsStyle &ms = ((TXAtom*)ring_M[i][j])->GetPrimitives().GetStyle();
      TGlMaterial *sm = ms.FindMaterial("Sphere");
      if( sm != NULL )
        l.GetPrimitives().GetStyle().SetMaterial(TXBond::StaticPrimitives()[9], *sm);
      l.UpdatePrimitives((1<<9)|(1<<10));
      l.SetRadius(0.5);
      ring_M[i][j]->SetTag(2);
      if( !label )
        l.GetGlLabel().SetVisible(false);
    }
    // hide replaced bonds
  }
  TGXApp::BondIterator bi = app.GetBonds();
  while( bi.HasNext() )  {
    TXBond &b = bi.Next();
    if( b.A().GetTag() == 2 && b.B().GetTag() == 1 )
      b.SetVisible(false);
  }
}
//.............................................................................
void GXLibMacros::macShowP(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TIntList parts;
  if (!Cmds.IsEmpty())  {
    for (size_t i=0; i < Cmds.Count(); i++)
      parts.Add(Cmds[i].ToInt());
  }
  TGXApp &app = TGXApp::GetInstance();
  app.ShowPart(parts, true);
  if (!Options.GetBoolOption('m'))
    app.CenterView();
}
//.............................................................................
void GXLibMacros::macSplit(TStrObjList &Cmds, const TParamList &Options,
  TMacroError &E)
{
  TGXApp &app = TGXApp::GetInstance();
  olxstr cr(Options.FindValue("r", EmptyString()).ToLowerCase());
  TCAtomPList Atoms =
    app.FindCAtoms(Cmds.IsEmpty() ? olxstr("sel") : Cmds.Text(' '), true);
  if( Atoms.IsEmpty() )  return;
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  TLattice& latt = app.XFile().GetLattice();
  RefinementModel& rm = app.XFile().GetRM();
  TCAtomPList ProcessedAtoms;
  XVar& var = rm.Vars.NewVar();
  for (size_t i=0; i < Atoms.Count(); i++) {
    TCAtom* CA = Atoms[i];
    if (CA->GetEllipsoid() == NULL) continue;
    vec3d direction;
    double Length = 0;
    olxstr lbl = CA->GetLabel();
    if (CA->GetEllipsoid()->GetSX() > CA->GetEllipsoid()->GetSY()) {
      if (CA->GetEllipsoid()->GetSX() > CA->GetEllipsoid()->GetSZ()) {
        Length = CA->GetEllipsoid()->GetSX();
        direction = CA->GetEllipsoid()->GetMatrix()[0];
      }
      else {
        Length = CA->GetEllipsoid()->GetSZ();
        direction = CA->GetEllipsoid()->GetMatrix()[2];
      }
    }
    else {
      if (CA->GetEllipsoid()->GetSY() > CA->GetEllipsoid()->GetSZ()) {
        Length = CA->GetEllipsoid()->GetSY();
        direction = CA->GetEllipsoid()->GetMatrix()[1];
      }
      else {
        Length = CA->GetEllipsoid()->GetSZ();
        direction = CA->GetEllipsoid()->GetMatrix()[2];
      }
    }
    direction *= Length;
    direction /= 2;
    au.CartesianToCell(direction);
    const double sp = 1./CA->GetDegeneracy();
    TXAtom &XA = app.AddAtom();
    TCAtom& CA1 = XA.CAtom();
    CA1.Assign(*CA);
    CA1.SetSameId(~0);
    CA1.SetPart(1);
    CA1.ccrd() += direction;
    XA.ccrd() = CA1.ccrd();
    CA1.SetLabel(au.CheckLabel(&CA1, lbl+'a'), false);
    // link occupancies
    rm.Vars.AddVarRef(var, CA1, catom_var_name_Sof, relation_AsVar, sp);
    CA1.SetOccu(0.5*sp);
    ProcessedAtoms.Add(CA1);
    TCAtom& CA2 = *CA;
    CA2.SetPart(2);
    CA2.ccrd() -= direction;
    CA2.SetLabel(au.CheckLabel(&CA2, lbl+'b'), false);
    // link occupancies
    rm.Vars.AddVarRef(var, CA2, catom_var_name_Sof, relation_AsOneMinusVar, sp);
    CA2.SetOccu(0.5*sp);
    ProcessedAtoms.Add(CA2);
    TSimpleRestraint* sr = NULL;
    if (cr.IsEmpty())
      ;
    else if (cr == "eadp")
      sr = &rm.rEADP.AddNew();
    else if (cr == "isor")
      sr = &rm.rISOR.AddNew();
    else if (cr == "simu")
      sr = &rm.rSIMU.AddNew();
    if (sr != NULL)
      sr->AddAtomPair(CA1, NULL, CA2, NULL);
  }
  latt.SetAnis(ProcessedAtoms, false);
  latt.Uniq();
}
//.............................................................................
