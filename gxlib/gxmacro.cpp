#include "gxmacro.h"
#include "sfutil.h"
#include "maputil.h"
#include "beevers-lipson.h"
#include "unitcell.h"
#include "xgrid.h"
#include "gllabels.h"
#include "symmparser.h"
#include "cif.h"
#include "vcov.h"
#include "xline.h"
#include "olxstate.h"
#ifdef __WXWIDGETS__
#include "wx/wx.h"
#endif

#define gxlib_InitMacro(macroName, validOptions, argc, desc)\
  lib.RegisterStaticMacro(\
    new TStaticMacro(&GXLibMacros::mac##macroName, #macroName, (validOptions), argc, desc))
#define gxlib_InitMacroA(macroName, amacroName, validOptions, argc, desc)\
  lib.RegisterStaticMacro(\
    new TStaticMacro(&GXLibMacros::mac##macroName, #amacroName, (validOptions), argc, desc))
#define gxlib_InitFunc(funcName, argc, desc) \
  lib.RegisterStaticFunction(\
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
  TGXApp &app = TGXApp::GetInstance();
  static const short // scale type
    stSimple     = 0x0001,
    stRegression = 0x0002;
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
  SFUtil::ExpandToP1(refs, F, uc.GetMatrixList(), P1SF);
  const double vol = app.XFile().GetLattice().GetUnitCell().CalcVolume();
  BVFourier::MapInfo mi;
// init map
  const vec3i dim(au.GetAxes()*resolution);
  TArray3D<float> map(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  mi = BVFourier::CalcEDM(P1SF, map.Data, dim, vol);
///////////////////////////////////////////////////////////////////////////////
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
    const int PointCount = dim.Prod();
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
  TSpaceGroup* sg = NULL;
  try  { sg = &app.XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "could not locate space group");
    return;
  }
  TUnitCell::SymmSpace sp = app.XFile().GetUnitCell().GetSymmSpace();
  olxstr hklFileName = app.LocateHklFile();
  if( !TEFile::Exists(hklFileName) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate hkl file");
    return;
  }
  TRefList refs;
  RefinementModel::HklStat stats =
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
    short ADS=0, AtomsStart=2;
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
  bool absolute = Options.Contains('a');
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
    Options.Contains('f')
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
  if( Options.Contains("symm") && str_symm_tag.IsEmpty() )
    str_symm_tag = '$';
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
      labels.Add(l);
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
//..............................................................................
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
    if( d_cnt == qpeaks.Count() && wheel > 0 )  return;
    d_cnt += (int)(wheel);
    if( d_cnt < 0 )  d_cnt = 0;
    if( d_cnt > (int)qpeaks.Count() )
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