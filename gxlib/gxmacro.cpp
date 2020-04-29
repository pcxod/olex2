#include "gxmacro.h"
#include "sfutil.h"
#include "maputil.h"
#include "beevers-lipson.h"
#include "unitcell.h"
#include "symmparser.h"
#include "vcov.h"
#include "dunitcell.h"
#include "dusero.h"
#include "xgrid.h"
#include "gllabels.h"
#include "xline.h"
#include "olxstate.h"
#include "estopwatch.h"
#include "analysis.h"
#include "planesort.h"
#include "xmacro.h"
#include "cif.h"
#include "dsphere.h"
#include "dring.h"
#include "roman.h"
#include "olxsurf.h"
#include "atomlegend.h"
#include "xtls.h"
#include "rmsds_adp.h"

#define gxlib_InitMacro(macroName, validOptions, argc, desc)\
  lib.Register(\
    new TMacro<GXLibMacros>(this, &GXLibMacros::mac##macroName, #macroName,\
      (validOptions), argc, desc))
#define gxlib_InitMacroA(macroName, amacroName, validOptions, argc, desc)\
  lib.Register(\
    new TMacro<GXLibMacros>(this, &GXLibMacros::mac##macroName, #amacroName,\
      (validOptions), argc, desc))
#define gxlib_InitFunc(funcName, argc, desc) \
  lib.Register(\
    new TFunction<GXLibMacros>(this, &GXLibMacros::fun##funcName, #funcName, argc,\
      desc))

//.............................................................................
void GXLibMacros::Export(TLibrary& lib) {
  lib.Register(new TMacro<GXLibMacros>(this, &GXLibMacros::macGrow,
    "Grow",
    "b-grows all visible grow bonds (when in a grow mode)",
    fpAny | psFileLoaded,
    EmptyString()),
    libChain
    );

  lib.Register(new TMacro<GXLibMacros>(this, &GXLibMacros::macPack,
    "Pack", EmptyString(),
    fpAny|psFileLoaded,
    "Extends the default macro by keyword 'wbox'"),
    libChain
    );

  gxlib_InitMacro(Name,
    "s-simply changes suffix of provided atoms to the provided one (or none)&;"
    "cs-leaves current selection unchanged&;"
    "r-synchronise names in the residues"
    ,
    fpNone|fpOne|fpTwo|fpThree,
    "Names atoms. If the 'sel' keyword is used and a number (or just the number)"
    " is provided as second argument the numbering will happen in the order the"
    " atoms were selected. Can be used as alternative to name and rename residues:"
    "\nname resi 1 or name resi 1 2");

  gxlib_InitMacro(CalcPatt, EmptyString(), fpNone|psFileLoaded,
    "Calculates Patterson map");
  gxlib_InitMacro(CalcFourier,
    "fcf-reads structure factors from a fcf file&;"
    "diff-calculates difference map&;"
    "tomc-calculates 2Fo-Fc map&;"
    "obs-calculates observed map&;"
    "calc-calculates calculated map&;"
    "fcfmc-calculates FCF Fc-Fc map&;"
    "scale-scale to use for difference maps, currently available simple(s) "
    "sum(Fo^2)/sum(Fc^2)) and regression(r)&;"
    "r-resolution in Angstrems&;"
    "i-integrates the map&;"
    "m-mask the structure&;"
    "map-show map[true]"
    , fpNone | psFileLoaded,
  "Calculates fourier map");

  gxlib_InitMacro(Qual,
    EmptyString(),
    fpOne,
    "Sets drawings quality, 1 - low, 2 - medium, 3 - high");

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
    "s-SPEC&;"
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
    "b-bond lengths&;"
    "rn-residue number&;"
    "rc-residue class&;"
    ,
    fpAny,
    "Shows/hides atom labels. Takes no argument is given to invert current "
    "labels visibility or a boolean value");
  gxlib_InitMacro(Label,
    "type-type of labels to make - subscript, brackers, default&;"
    "symm-symmetry dependent tag type {[$], #, X, full}&;"
    "resi-add residue number(#) or name (n) like -resi=_# or -resi=@n&;"
    "a-aligns labels trying to avoid overlapping&;",
    fpAny,
    "Creates moveable labels for provided atoms/bonds/angles (selection)");
  gxlib_InitMacro(ShowH,
    EmptyString(),
    fpNone|fpTwo|psFileLoaded,
    "Changes the H-atom and H-bonds visibility");
  gxlib_InitMacro(Detach,
    "u-re-attaches all or given atoms",
    fpAny | psFileLoaded,
    "Detaches/re-attaches given/selected atoms from/to the model");
  gxlib_InitMacro(Info,
    "s-sorts the atom list&;p-coordinate precision [3]&;f-print fractional "
    "coordinates vs Cartesian [true]&;"
    "c-copy the printed information ito the clipboard",
    fpAny,
    "Prints out information for gived [all] atoms");
  gxlib_InitMacro(ShowQ,
    "wheel-number of peaks to hide (if negative) or to show ",
    fpNone|fpOne|fpTwo,
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
    "n-just sets current view normal to the line without creating the object&;"
    "f-consider input in fractional coordinates vs Cartesian",
    fpAny,
    "Creates a line or best line for provided atoms");
  gxlib_InitMacro(Mpln,
    "n-just orient, do not create plane&;"
    "r-create regular plane&;"
    "we-use weights proportional to the (atomic mass)^we&;"
    "rings-creates planes for rings template, like NC5&;",
    fpAny,
    "Sets current view along the normal of the best plane");
  gxlib_InitMacro(Lpln,
    EmptyString(),
    fpThree,
    "Creates a lattice plane for the given Miller index");
  gxlib_InitMacro(Cent,
    "rings-finds rings specified by template and add centroids for each of them"
    ". For example cent -rings=C6",
    fpAny,
    "Creates a centroid for given/selected/all atoms");
  gxlib_InitMacro(Cell, "r-shows reciprocal cell",
    fpNone|fpOne|psFileLoaded,
    "If no arguments provided inverts visibility of unit cell, otherwise sets"
    " it to the boolean value of the parameter");
  gxlib_InitMacro(Basis, EmptyString(), fpNone|fpOne,
    "Shows/hides the orientation basis");
  gxlib_InitMacro(Group,
    "n-a custom name can be provided&;"
    "u-ungroups given group"
    ,
    fpNone|fpOne|psFileLoaded,
  "Groups current visible objects or selection");
  gxlib_InitMacro(Fmol, EmptyString(), fpNone|psFileLoaded,
    "Shows all fragments (as opposite to uniq)");
  gxlib_InitMacro(Sel,
    "a-select all&;"
    "u-unselect all&;"
    "l-consider the list of bonds as independent when printing info&;"
    "c-copies printed values to the clipboard&;"
    "i-invert selection&;",
    fpAny,
    "If no arguments provided, prints current selection. This includes "
    "distances, angles and torsion angles and other geometrical parameters. "
    "Selects atoms fulfilling provided conditions, like"
    "\nsel $type - selects any particular atom type; type can be one of the "
    "following shortcuts - * - for all atoms, M - for metals, X - for halogens"
    "\nsel $*,type - selects all but specified type atoms"
    "\n sel isot/anis - selects isotropic or anisotropic atoms"
    "\n sel fvar 1 - selects all atoms parameters of which depend on the fvar 1"
    "\n\n"
    "An extended syntax include keyword 'where' and 'rings' which "
    "allow selecting atoms and bonds according to their properties, like type "
    "and length or rings of particular connectivity like C6 or NC5. If the "
    "'where' keyword is used, logical operators, like and (&&), and or (||) "
    "can be used to refine the selection. For example:"
    "\nsel atoms where xatom.bai.z > 2 - to select all atoms heavier after H"
    "\nsel bonds where xbond.length > 2 - to select all bonds longer than 2 A"
    "\nsel bonds where xbond.b.bai.z == 1 - to select all bonds where the "
    "lightest atoms is H. Other extensions include 'sel atom bonds' and "
    "'sel bond atoms' and 'sel collection [wildcards]'"
    );
  gxlib_InitMacro(Uniq, EmptyString(), fpAny | psFileLoaded,
    "Shows only fragments specified by atom name(s) or selection");
  gxlib_InitMacro(PiM,
    "l-display labels for the created lines&;",
    fpAny|psFileLoaded,
    "Creates an illustration of a pi-system to metal bonds");
  gxlib_InitMacro(ShowP,
    "m-do not modify the display view&;"
    "v-operate only on currently visible atoms/fragments;"
    ,
    fpAny,
    "Shows specified or all parts of the structure");
  gxlib_InitMacro(ShowR,
    "m-do not modify the display view&;"
    "v-operate only on currently visible atoms/fragments;"
    ,
    fpAny,
    "Shows residues by number or name");
  gxlib_InitMacro(Undo, EmptyString(), fpNone,
    "Reverts some of the previous operations");
  gxlib_InitMacro(Esd,
    "label-creates a graphics label&;"
    "l-consider the list of bonds as independent&;"
    "c-copies printed values to the clipboard",
    fpAny|psFileLoaded,
    "This procedure calculates possible parameters for the selection and "
    "evaluates their esd using the variance-covariance matrix coming from the "
    "ShelXL refinement with negative 'MORE' like 'MORE -1' option or from the "
    "olex2.refine");
  gxlib_InitMacro(CalcVoid,
    "d-distance from Van der Waals surface [0]&;r-resolution[0.2]&;"
    "p-precise calculation&;"
    "i-invert the map for rendering",
    fpNone|fpOne|psFileLoaded,
      "Calculates solvent accessible void and packing parameters; optionally "
      "accepts a file with space separated values of Atom Type and radius, an "
      "entry a line");
  gxlib_InitMacro(LstGO,
    EmptyString(),
    fpNone,
    "List current graphical objects");
  gxlib_InitMacro(WBox,
    "w-use atomic mass instead of unit weights for atoms&;"
    "s-create separate boxes for fragments",
    (fpAny)|psFileLoaded,
    "Calculates wrapping box around provided box using the set of best, "
    "intermidiate and worst planes");
  gxlib_InitMacro(Center,
    "z-also recalculates the scene zoom",
    (fpAny)|psFileLoaded,
    "Sets the centre of rotation to given point");
  gxlib_InitMacro(ChemDraw,
    "u-clear the drawing; be careful as it also clears model customisations",
    fpAny|psFileLoaded,
    "Changes the view to show aromatic rings and double/tripple bonds.");
  gxlib_InitMacro(Direction,
    EmptyString(),
    fpNone,
    "Prints current orientation of the model in factional coordinates");
  gxlib_InitMacro(Individualise,
    EmptyString(),
    fpAny,
    "Moves provided atoms to individual collections, so that the atom "
    "properties, such as draw style and appearance can be changed separately "
    "of the group. The first call to this macro creates a group unique to the"
    " asymmetric unit, the second call makes the atom unique to the lattice");
  gxlib_InitMacro(Collectivise,
    EmptyString(),
    fpAny,
    "Does the opposite to the Individialise. If provided atoms are unique to "
    "the lattice a call to this function makes them uniq to the asymmetric "
    "unit, the following call makes the uniq to the element type");
  gxlib_InitMacro(Poly,
    EmptyString(),
    (fpAny^fpNone)|psFileLoaded,
    "Sets polyhedra type for all/selected/given atoms. Last argument specifies"
    " the type - none, auto, regular");
  gxlib_InitMacro(Kill,
    "au-kill atoms in the asymmetric unit (disregarding visibility/"
    "availability). This option is intended for the internal use - the model is"
    " not rebuilt after its execution",
    fpAny,
    "Deletes provided/selected atoms, bonds and other objects. 'kill labels' "
    "hides all atom and bond labels");
  lib.Register(new TMacro<GXLibMacros>(this, &GXLibMacros::macInv,
    "Inv", EmptyString(), fpAny, "Inverts selected plane normals"),
    libChain);
  gxlib_InitMacro(Match,
    "s-subgraph match&;"
    "w-use non-unit weights [ZO - atomic number X occupancy], Z - atomic number"
    ", EM - atomic mass, AM - atomic mass X occupancy, O - occupancy&;"
    "n-naming. If the value a symbol [or set of] this is appended to the "
    "label, '$xx' replaces the symbols after the atom type symbol with xx, "
    "leaving the ending, '-xx' - changes the ending of the label with xx&;"
    "a-align&;"
    "i-try inversion&;"
    "u-unmatch&;"
    "esd-calculate esd (works for pairs only)&;"
    "h-excludes H atoms from matching and the RMSD calculation&;"
    "cm-copies the transformation matrix suitable for sgen to clipboard&;"
    "o-matches overlayed lattices&;"
    "m-moves the atoms to the overlayed position permanently. Only valid for "
    "two atoms groups and selection of at least three atom pairs&;"
    ,
    fpNone|fpOne|fpTwo,
    "Fragment matching, alignment and label transfer routine");
  gxlib_InitMacro(SetMaterial, EmptyString(), fpTwo | fpThree,
    "Assigns provided value to specified material");
  gxlib_InitMacro(DefineVar, EmptyString(), fpOne|psFileLoaded,
    "Defines a variable to be calculated with CalcVars. The argument is "
    "the variable name.");

  gxlib_InitMacro(ProjSph,
    "g-sphere quality [6]&;"
    "e-emboss the sphere&;"
    "a-transparency level [0x9c] 0 - 255&;"
    "group-group the ligands into same-colour groups [false]&;"
    ,
    fpAny | psFileLoaded,
    "Creates a projection from the selected atom onto a sphere, coloring each "
    "point on the sphere with a unique color corresponding to fragments. For "
    "reference see Guzei, I.A., Wendt, M.Dalton Trans., 2006, 3991-3999.");

  gxlib_InitMacro(OFileDel, EmptyString(), fpOne,
    "Deletes overlayed file specified by index");
  gxlib_InitMacro(OFileSwap, EmptyString(), fpNone | fpOne,
    "Sets current file to which all commands are applied");

  gxlib_InitMacro(TwinView,
    "c-Cartesian matrix is provided[true]&;"
    "t-transpose the input matrix [false]"
    ,
    fpAny,
    "Sets special rendering mode");
  gxlib_InitMacro(CalcSurf,
    "pr-probe radius [1.2 A]&;"
    "c-surface colour [m],d,a,e&;"
    ,
    fpAny,
    "For testing only for now");
  gxlib_InitMacro(Legend,
    "r-reset the position",
    fpNone | fpOne,
    "Shows/hides atom legend");
  gxlib_InitMacro(AdjustStyle,
    "a-apply to atoms [true]&;"
    "b-apply to bonds[true]&;",
    fpAny^(fpNone|fpOne),
    "Shows/hides atom legend");

  lib.Register(
    new TMacro<GXLibMacros>(this, &GXLibMacros::macTLS, "TLS",
      "b-three first selected atoms are the ADP basis&;"
      "v-shows the difference {Uobs, Utls}&;"
      "o-the povray output file name and type, like: -o=output,Utls. The "
      "supported types are Utls and Uobs. When Utls is given the difference "
      "between the Utls and Uobs will be rendered and for Uobs, the difference "
      "between Uobs and Utls will be rendered&;"
      "s-scale [125]&;"
      "g-do not use gradient&;"
      "start_color-starting colur for the gradient&;"
      "end_color-final gradient color&;"
      "quality-the sphere quality [5]&;"
      "q-quiet&;"
      "type-object type [diff], rmsds&;",
      0),
    libReplace
  );
  lib.Register(
    new TMacro<GXLibMacros>(this, &GXLibMacros::macUdiff, "Udiff",
      "s-scale [125]&;"
      "start_color-start gradient color [0xff0000]&;"
      "g-do not use gradient&;"
      "end_color-end gradient color [0x0000ff]&;"
      "quality-the sphere quality [5]&;"
      "type-object type [diff], rmsd&;"
      "r-reset the object style",
      fpAny,
      "Renders a difference between two sets of ADPs")
  );
  
  gxlib_InitMacro(MSDSView,
    "s-scale [1]&;"
    "t-type [rmsd], msd&;"
    "q-quality [5], max is set to 7&;"
    "a-anharmonicity display [all], anh, none",
    fpNone,
    "Shows/hides atom legend");

  gxlib_InitFunc(ExtraZoom, fpNone|fpOne,
    "Sets/reads current extra zoom (default zoom correction)");
  gxlib_InitFunc(MatchFiles, fpTwo|fpThree,
    "Matches given files");
  gxlib_InitFunc(SelName, fpNone,
    "Returns name for the selected object group");
  gxlib_InitFunc(GetMaterial, fpOne | fpTwo,
    "Returns material of specified object");
  gxlib_InitFunc(FBond, fpNone | fpOne,
    "Sets/prints bond unit length");
  gxlib_InitFunc(ObjectSettings, fpTwo|fpThree,
    "Gets/sets aobject settings for atom or bond");
}
//.............................................................................
bool InvertFittedFragment() {
  TModeRegistry &mr = TModeRegistry::GetInstance();
  if (mr.CheckMode(mr.DecodeMode("fit"))) {
    TXAtomPList atoms = TGXApp::GetInstance()
      .GetSelection().Extract<TXAtom>();
    if (atoms.IsEmpty()) {
      return false;
    }
    vec3d cnt;
    for (size_t i = 0; i < atoms.Count(); i++) {
      cnt += atoms[i]->crd();
    }
    cnt /= atoms.Count();
    for (size_t i = 0; i < atoms.Count(); i++) {
      atoms[i]->crd() = -atoms[i]->crd() + cnt + cnt;
      atoms[i]->CAtom().ccrd() = atoms[i]->CAtom()
        .GetParent()->Fractionalise(atoms[i]->crd());
    }
    return true;
  }
  return false;
}

void GXLibMacros::macGrow(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (Options.Contains('b')) {  // grow XGrowBonds only
    app.GrowBonds();
  }
  else {
    Error.SetUnhandled(true);
  }
}
//.............................................................................
void GXLibMacros::macPack(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  const bool ClearCont = !Options.Contains("c");
  const bool wbox = (Cmds.Count() > 0 && Cmds[0].Equalsi("wbox"));
  if (wbox) {
    Cmds.Delete(0);
    if (app.Get3DFrame().IsVisible()) {
      if (app.Get3DFrame().IsSpherical()) {
        app.XFile().GetLattice().GenerateSphere(app.Get3DFrame().GetCenter(),
          app.Get3DFrame().GetZoom(), ClearCont);
      }
      else {
        vec3d_alist norms(6), centres(6);
        for (int i = 0; i < 6; i++) {
          norms[i] = app.Get3DFrame().Faces[i].GetN();
          centres[i] = app.Get3DFrame().Faces[i].GetCenter();
        }
        app.XFile().GetLattice().GenerateBox(norms, centres, ClearCont);
      }
    }
  }
  else {
    Error.SetUnhandled(true);
  }
}
//.............................................................................
void GXLibMacros::macName(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (Cmds.Count() == 2 && Cmds[0].Equalsi("collection")) {
    sorted::PointerPointer<TGPCollection> old;
    TGlGroup &sel = app.GetSelection();
    bool reset = Cmds[1].Equalsi("none");
    for (size_t i = 0; i < sel.Count(); i++) {
      if (old.AddUnique(&sel[i].GetPrimitives()).b) {
        // check type of the objects
        if (!reset && old.Count() > 1) {
          size_t idx = (old[0] == &sel[i].GetPrimitives() ? 1 : 0);
          if (typeid(old[idx]->GetObject(0)) != typeid(sel[i])) {
            TBasicApp::NewLogEntry(logError) << "Mixed object collections";
            return;
          }
        }
      }
    }
    if (old.IsEmpty()) {
      return;
    }
    // restore default collections
    if (reset) {
      for (size_t i = 0; i < old.Count(); i++) {
        bool bond = false;
        if (old[i]->GetObject(0).Is<TXBond>()) {
          bond = true;
          for (size_t j = 0; j < TXBond::NamesRegistry().Count(); j++) {
            if (TXBond::NamesRegistry().GetValue(j) == old[i]->GetName()) {
              TXBond::NamesRegistry().Delete(j--);
            }
          }
        }
        else if (old[i]->GetObject(0).Is<TXAtom>()) {
          TXAtom::NamesRegistry().Remove(old[i]->GetName());
          for (size_t j = 0; j < TXAtom::NamesRegistry().Count(); j++) {
            if (TXAtom::NamesRegistry().GetValue(j) == old[i]->GetName()) {
              TXAtom::NamesRegistry().Delete(j--);
            }
          }
        }
        else if (old[i]->GetObject(0).Is<TXPlane>()) {
          for (size_t j = 0; j < TXPlane::NamesRegistry().Count(); j++) {
            if (TXPlane::NamesRegistry().GetValue(j) == old[i]->GetName()) {
              TXPlane::NamesRegistry().Delete(j--);
            }
          }
        }
        else {
          TBasicApp::NewLogEntry(logError) << "Operation is not applicable";
          continue;
        }
        AGDObjList objects(old[i]->GetObjects());
        old[i]->ClearObjects();
        old[i]->ClearPrimitives();
        for (size_t j = 0; j < objects.Count(); j++) {
          olxstr l;
          if (bond) {
            l = TXBond::GetLegend(*(TXBond*)objects[j], 0);
          }
          else {
            l = TXAtom::GetLegend(*(TXAtom*)objects[j], 0);
          }
          if (l == old[i]->GetName()) {
            continue;
          }
          objects[j]->Create();
        }
      }
      return;
    }
    TGPCollection *gpc = app.GetRenderer().FindCollection(Cmds[1]);
    if (gpc != 0 && gpc->ObjectCount() != 0) {
      if (typeid(old[0]->GetObject(0)) != typeid(gpc->GetObject(0))) {
        TBasicApp::NewLogEntry(logError) << "Destination collection is used "
          "by different object type";
        return;
      }
    }
    else {
      if (gpc != 0) {
        gpc->ClearPrimitives();
        for (size_t i = 0; i < sel.Count(); i++) {
          gpc->RemoveObject(sel[i]);
        }
      }
      else {
        gpc = &app.GetRenderer().NewCollection(Cmds[1]);
      }
    }
    for (size_t i = 0; i < sel.Count(); i++) {
      sel[i].GetPrimitives().RemoveObject(sel[i]);
    }
    if (sel[0].Is<TXAtom>()) {
      for (size_t i = 0; i < sel.Count(); i++) {
        TXAtom &a = dynamic_cast<TXAtom &>(sel[i]);
        TXAtom::NamesRegistry().Add(a.GetRef().ToString(), Cmds[1]);
        for (size_t j = 0; j < a.BondCount(); j++) {
          a.Bond(j).UpdateStyle();
        }
      }
    }
    else if (sel[0].Is<TXBond>()) {
      for (size_t i = 0; i < sel.Count(); i++) {
        TXBond &b = dynamic_cast<TXBond &>(sel[i]);
        TXBond::NamesRegistry().Add(b.GetRef().ToString(), Cmds[1]);
      }
    }
    else if (sel[0].Is<TXPlane>()) {
      for (size_t i = 0; i < sel.Count(); i++) {
        TXPlane &p = dynamic_cast<TXPlane &>(sel[i]);
        TXPlane::NamesRegistry().Add(p.GetDefId(), Cmds[1]);
      }
    }
    for (size_t i = 0; i < sel.Count(); i++) {
      sel[i].Create(Cmds[1]);
    }
    sel.Clear();
    return;
  }
  // special keyword
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("resi")) {
    if (Cmds.Count() == 3 && Cmds[1].IsNumber() && Cmds[2].IsNumber()) {
      TAsymmUnit & au = app.XFile().GetAsymmUnit();
      TResidue *r = au.FindResidue(Cmds[1]);
      if (r == 0) {
        Error.ProcessingError(__OlxSrcInfo, "could not locate specified residue");
        return;
      }
      int nn = Cmds[2].ToInt();
      TResidue& resi = au.NewResidue(r->GetClassName(), nn, nn, r->GetChainId());
      if (!resi.IsEmpty()) {
        TBasicApp::NewLogEntry(logWarning) <<
          "Warning - appending atoms to existing, non-empty residue!";
      }
      resi.SetCapacity(r->Count());
      while(r->Count() != 0) {
        resi.Add((*r)[0]);
      }
      TBasicApp::NewLogEntry(logInfo) << "The residue has been renamed";
      return;
    }
    XLibMacros::macRESI(Cmds, Options, Error);
    return;
  }
  bool changeSuffix = Options.Contains('s');
  bool nameResi = Options.GetBoolOption('r', false, true);
  if (changeSuffix) {
    TXAtomPList xatoms = app.FindXAtoms(Cmds, true, !Options.Contains("cs"));
    if (!xatoms.IsEmpty()) {
      TUndoData *ud = app.GetUndo().Push(app.ChangeSuffix(
        xatoms, Options.FindValue('s')));
      if (nameResi) {
        TUndoData *rud = app.SynchroniseResidues(
          TCAtomPList(xatoms, FunctionAccessor::MakeConst(&TXAtom::CAtom)));
        if (ud != 0) {
          ud->AddAction(rud);
        }
        else {
          app.GetUndo().Push(rud);
        }
      }
    }
  }
  else {
    bool processed = false;
    if (Cmds.Count() == 1) { // bug #49
      const size_t spi = Cmds[0].IndexOf(' ');
      if (spi != InvalidIndex) {
        app.GetUndo().Push(
          app.Name(Cmds[0].SubStringTo(spi),
          Cmds[0].SubStringFrom(spi+1), !Options.Contains("cs"), nameResi)
        );
      }
      else {
        app.GetUndo().Push(
          app.Name("sel", Cmds[0], !Options.Contains("cs"),
          nameResi));
      }
      processed = true;
    }
    else if (Cmds.Count() == 2) {
      app.GetUndo().Push(app.Name(Cmds[0], Cmds[1], !Options.Contains("cs"),
        nameResi));
      processed = true;
    }
    if (!processed) {
      Error.ProcessingError(__OlxSrcInfo,
        olxstr("invalid syntax: ") << Cmds.Text(' '));
    }
  }
  if (TXApp::DoUseSafeAfix()) {
    app.GetUndo().Push(
      app.XFile().GetLattice().ValidateHGroups(true, true));
  }
}
//.............................................................................
void GXLibMacros::macQual(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  int v = Cmds[0].ToInt();
  if (v == 1)
    v = qaLow;
  else if (v == 2)
    v = qaMedium;
  else if (v == 3)
    v = qaHigh;
  else {
    Error.ProcessingError(__OlxSrcInfo, "1, 2 or 3 is expected");
    return;
  }
  app.Quality(v);
}
//.............................................................................
void GXLibMacros::macCalcFourier(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TStopWatch st(__FUNC__);
  double resolution = Options.FindValue("r", "0.25").ToDouble(),
    maskInc = 1.0;
  if (resolution < 0.005) {
    resolution = 0.05;
  }
  resolution = 1./resolution;
  short mapType = SFUtil::mapTypeCalc;
  if (Options.Contains("tomc")) {
    mapType = SFUtil::mapType2OmC;
  }
  else if (Options.Contains("obs")) {
    mapType = SFUtil::mapTypeObs;
  }
  else if (Options.Contains("diff")) {
    mapType = SFUtil::mapTypeDiff;
  }
  olxstr strMaskInc = Options.FindValue("m");
  if (!strMaskInc.IsEmpty()) {
    maskInc = strMaskInc.ToDouble();
  }

  TOnProgress pg;
  // link the two
  st.SetProgress(&pg);
  pg.SetMax(4);
  TBasicApp::GetInstance().OnProgress.Enter(0, &pg);

  TRefList refs;
  TArrayList<compd> F;
  st.start("Obtaining structure factors");
  short scale = SFUtil::scaleSimple;
  double scale_value = 0;
  {
    olxstr str_scale = Options.FindValue("scale", "ext")
      .ToLowerCase();
    if (!str_scale.IsEmpty()) {
      if (str_scale.CharAt(0) == 'r') {
        scale = SFUtil::scaleRegression;
      }
      else if (str_scale.CharAt(0) == 'e') {
        scale = SFUtil::scaleExternal;
        if (app.XFile().GetRM().Vars.VarCount() > 0) {
          scale_value = 1./app.XFile().GetRM().Vars.GetVar(0).GetValue();
        }
        else {
          TBasicApp::NewLogEntry(logWarning) << "Could not locate external "
            "scale - using the default";
          scale = SFUtil::scaleSimple;
        }
      }
    }
  }
  short src = Options.GetBoolOption("fcf") ? SFUtil::sfOriginFcf
    : SFUtil::sfOriginOlex2;
  olxstr err = SFUtil::GetSF(refs, F, mapType, src,
    scale, scale_value, SFUtil::fpMerge);
  if (!err.IsEmpty()) {
    E.ProcessingError(__OlxSrcInfo, err);
    return;
  }
  if (Options.GetBoolOption("fcfmc")) {
    if (src != SFUtil::sfOriginFcf || mapType != SFUtil::mapTypeCalc) {
      E.ProcessingError(__OlxSrcInfo, "Invalid combination of arguments");
      return;
    }
    TArrayList<compd> F1(refs.Count());
    SFUtil::CalcSF(app.XFile(), refs, F1);
    // this is a reference implementation for tests
    //app.CalcSF(refs, F1);
    if (F1.Count() != F.Count()) {
      E.ProcessingError(__OlxSrcInfo, "F arrays mismatch");
      return;
    }
    for (size_t i = 0; i < F1.Count(); i++) {
      F[i] -= F1[i];
    }
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
  mi = BVFourier::CalcEDM(P1SF, map.Data, vol);
  ///////////////////////////////////////////////////////////////////////////////
  st.start("Map operations");
  app.XGrid().InitGrid(dim);

  app.XGrid().SetMaxHole((float)(mi.sigma*1.4));
  app.XGrid().SetMinHole((float)(-mi.sigma*1.4));
  app.XGrid().SetScale((float)(-mi.sigma*6));
  //app.XGrid().SetScale( -(mi.maxVal - mi.minVal)/2.5 );
  app.XGrid().SetMinVal((float)mi.minVal);
  app.XGrid().SetMaxVal((float)mi.maxVal);
  // copy map
  olx_array::copy_map_segment_3(map.Data, app.XGrid().Data()->Data,
    vec3s(0), map.GetSize());
  app.XGrid().AdjustMap();

  TBasicApp::NewLogEntry() <<
    "Map max val " << olxstr::FormatFloat(3, mi.maxVal) <<
    " min val " << olxstr::FormatFloat(3, mi.minVal) << NewLineSequence() <<
    "Map sigma " << olxstr::FormatFloat(3, mi.sigma);
  // map integration
  if (Options.GetBoolOption('i')) {
    TArrayList<MapUtil::peak> Peaks;
    TTypeList<MapUtil::peak> MergedPeaks;
    vec3d norm(1. / dim[0], 1. / dim[1], 1. / dim[2]);
    MapUtil::Integrate<float>(map.Data, (float)(mi.sigma * 6), Peaks);
    MapUtil::MergePeaks(uc.GetSymmSpace(), norm, Peaks, MergedPeaks);
    QuickSorter::SortSF(MergedPeaks, MapUtil::PeakSortBySum);
    for (size_t i = 0; i < MergedPeaks.Count(); i++) {
      const MapUtil::peak& peak = MergedPeaks[i];
      if (peak.count == 0)  continue;
      vec3d cnt((double)peak.center[0] / dim[0], (double)peak.center[1] / dim[1],
        (double)peak.center[2] / dim[2]);
      const double ed = (double)((long)((peak.summ * 1000) / peak.count)) / 1000;
      TCAtom& ca = au.NewAtom();
      ca.SetLabel(olxstr("Q") << olxstr((100 + i)));
      ca.ccrd() = cnt;
      ca.SetQPeak(ed);
    }
    au.InitData();
    TActionQueue* q_draw = app.FindActionQueue(olxappevent_GL_DRAW);
    bool q_draw_changed = false;
    if (q_draw != 0) {
      q_draw->SetEnabled(false);
      q_draw_changed = true;
    }
    app.XFile().GetLattice().Init();
    olex2::IOlex2Processor::GetInstance()->processMacro("compaq -q");
    if (q_draw != 0 && q_draw_changed) {
      q_draw->SetEnabled(true);
    }
  }  // integration
  if (Options.Contains("m")) {
    FractMask* fm = new FractMask;
    app.BuildSceneMask(*fm, maskInc);
    app.XGrid().SetMask(*fm);
  }
  app.XGrid().InitIso();
  //TStateChange sc(prsGridVis, true);
  app.ShowGrid(Options.GetBoolOption("map", false, true), EmptyString());
  //OnStateChange.Execute((AEventsDispatcher*)this, &sc);
  pg.SetPos(pg.GetMax());
  TBasicApp::GetInstance().OnProgress.Exit(NULL, &pg);
}
//.............................................................................
void GXLibMacros::macCalcPatt(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  // space group matrix list
  TUnitCell::SymmSpace sp = app.XFile().GetUnitCell().GetSymmSpace();
  olxstr hklFileName = app.XFile().LocateHklFile();
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
  const double resolution = 10;
  const vec3i dim(au.GetAxes()*resolution);
  app.XGrid().InitGrid(dim);
  BVFourier::MapInfo mi = BVFourier::CalcPatt(
    P1SF, app.XGrid().Data()->Data, vol);
  app.XGrid().AdjustMap();
  app.XGrid().SetMinVal((float)mi.minVal);
  app.XGrid().SetMaxVal((float)mi.maxVal);
  app.XGrid().SetMaxHole((float)(mi.sigma*1.4));
  app.XGrid().SetMinHole((float)(-mi.sigma*1.4));
  app.XGrid().SetScale((float)(-(mi.maxVal - mi.minVal)/2.5));
  app.XGrid().InitIso();
  app.ShowGrid(true, EmptyString());
}
//.............................................................................
void GXLibMacros::macMask(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (Cmds[0].Equalsi("atoms") && Cmds.Count() > 1) {
    int Mask = Cmds[1].ToInt();
    short AtomsStart = 2;
    TXAtomPList Atoms =
      app.FindXAtoms(TStrObjList(Cmds.SubListFrom(AtomsStart)), false, false);
    app.UpdateAtomPrimitives(Mask, Atoms.IsEmpty() ? NULL : &Atoms);
  }
  else if ((Cmds[0].Equalsi("bonds") || Cmds[0].Equalsi("hbonds")) &&
           Cmds.Count() > 1)
  {
    int Mask = Cmds[1].ToInt();
    TXBondPList Bonds = app.GetBonds(TStrList(Cmds.SubListFrom(2)), false);
    app.UpdateBondPrimitives(Mask,
      (Bonds.IsEmpty() && app.GetSelection().Count() == 0) ? NULL : &Bonds,
      Cmds[0].Equalsi("hbonds"));
  }
  else {
    int Mask = Cmds.GetLastString().ToInt();
    Cmds.Delete(Cmds.Count() - 1);
    olxstr name = Cmds.Text(' ');
    if (!name.IsEmpty()) {
      TGPCollection *GPC = app.GetRenderer().FindCollection(name);
      if (GPC != NULL) {
        if (GPC->ObjectCount() != 0)
          GPC->GetObject(0).UpdatePrimitives(Mask);
      }
      else {
        Error.ProcessingError(__OlxSrcInfo, "Undefined graphics: ").quote() <<
          Cmds.Text(' ');
        return;
      }
    }
    else {
      TGlGroup &sel = app.GetSelection();
      sorted::PointerPointer<TGPCollection> colls;
      for (size_t i = 0; i < sel.Count(); i++) {
        if (colls.AddUnique(&sel[i].GetPrimitives()).b) {
          sel[i].UpdatePrimitives(Mask);
        }
      }
    }
  }
}
//.............................................................................
void GXLibMacros::macARad(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  olxstr arad(Cmds[0]);
  Cmds.Delete(0);
  TXAtomPList xatoms = app.FindXAtoms(Cmds, false, false);
  app.AtomRad(arad, xatoms.IsEmpty() ? NULL : &xatoms);
}
//.............................................................................
void GXLibMacros::macADS(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  short ads = -1;
  if (Cmds[0].Equalsi("elp")) {
    ads = adsEllipsoid;
  }
  else if (Cmds[0].Equalsi("sph")) {
    ads = adsSphere;
  }
  else if (Cmds[0].Equalsi("ort")) {
    ads = adsOrtep;
  }
  else if (Cmds[0].Equalsi("std")) {
    ads = adsStandalone;
  }
  if (ads == -1) {
    Error.ProcessingError(__OlxSrcInfo,
      "unknown atom type (elp/sph/ort/std) supported only");
    return;
  }
  Cmds.Delete(0);
  TXAtomPList Atoms = app.FindXAtoms(Cmds, false, false);
  app.SetAtomDrawingStyle(ads, Atoms.IsEmpty() ? 0 : &Atoms);
}
//.............................................................................
void GXLibMacros::macAZoom(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if( !Cmds[0].IsNumber() )  {
    Error.ProcessingError(__OlxSrcInfo,
      "a number is expected as first argument");
    return;
  }
  float zoom = Cmds[0].ToFloat();
  Cmds.Delete(0);
  TXAtomPList Atoms = app.FindXAtoms(Cmds, false, false);
  app.AtomZoom(zoom, Atoms.IsEmpty() ? NULL : &Atoms);
}
//.............................................................................
void GXLibMacros::macBRad(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  float r = Cmds[0].ToFloat();
  Cmds.Delete(0);
  TXBondPList bonds;
  bool absolute = Options.GetBoolOption('a');
  if (Cmds.Count() == 1 && Cmds[0].Equalsi("hbonds")) {
    if (absolute) r /= 0.02;
    TGXApp::BondIterator bi = app.GetBonds();
    bonds.SetCapacity(bi.count/10);
    while (bi.HasNext()) {
      TXBond& xb = bi.Next();
      if (xb.GetType() == sotHBond)
        bonds.Add(xb);
    }
    app.BondRad(r, &bonds);
  }
  else {
    if (absolute) r /= 0.1f;
    bonds = app.GetBonds(Cmds, true);
    if (bonds.IsEmpty() && Cmds.IsEmpty()) {  // get all non-H
      TGXApp::BondIterator bi = app.GetBonds();
      bonds.SetCapacity(bi.count);
      while (bi.HasNext()) {
        TXBond& xb = bi.Next();
        if (xb.GetType() != sotHBond)
          bonds.Add(xb);
      }
      TXBond::GetSettings(app.GetRenderer()).SetRadius(r);
      TDRing::GetSettings(app.GetRenderer()).SetTubeR(r / 13.3);
      TDRing::GetSettings(app.GetRenderer()).ClearPrimitives();
      app.GetRings().ForEach(ACollectionItem::IndexTagSetter(
        FunctionAccessor::MakeConst(&TDRing::GetPrimitives)));
      for (size_t i = 0; i < app.GetRings().Count(); i++) {
        if (app.GetRings()[i].GetPrimitives().GetTag() != (index_t)i) {
          continue;
        }
        app.GetRings()[i].GetPrimitives().ClearPrimitives();
        app.GetRings()[i].GetPrimitives().ClearObjects();
      }
      for (size_t i = 0; i < app.GetRings().Count(); i++) {
        app.GetRings()[i].Create();
      }
      TGXApp::PlaneIterator pi = app.GetPlanes();
      while (pi.HasNext()) {
        TXPlane &p = pi.Next();
        p.GetPrimitives().ClearPrimitives();
        p.GetPrimitives().ClearObjects();
      }
      pi.Reset();
      while (pi.HasNext()) {
        TXPlane &p = pi.Next();
        p.Create();
      }
    }
    app.BondRad(r, &bonds);
  }
}
//.............................................................................
void GXLibMacros::macTelpV(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  float p = Cmds[0].ToFloat();
  if (p > 0) {
    app.CalcProbFactor(p);
  }
  else {
    TXAtom::GetSettings(app.GetRenderer()).SetTelpProb(-p);
    app.Draw();
  }
}
//.............................................................................
void GXLibMacros::macInfo(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TStrList Output;
  app.InfoList(Cmds.IsEmpty() ? EmptyString() : Cmds.Text(' '),
    Output, Options.Contains("p"),
    Options.FindValue('p', "-3").ToInt(),
    Options.GetBoolOption('f', false, false)
  );
  TBasicApp::NewLogEntry() << Output;
  if (Options.Contains('c')) {
    app.ToClipboard(Output);
  }
}
//.............................................................................
void GXLibMacros::macLabels(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  uint32_t lmode = 0;
  if (Options.IsEmpty()) {
    TGlGroup &sel = app.GetSelection();
    bool bonds_only = true;
    for (size_t i = 0; i < sel.Count(); i++) {
      if (!sel[i].Is<TXBond>()) {
        bonds_only = false;
        break;
      }
    }
    if (!sel.IsEmpty() && bonds_only && !app.AreLabelsVisible())
      lmode = lmBonds;
  }
  else {
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
    if (Options.Contains("s"))  lmode |= lmSpec;
    if (Options.Contains("b"))  lmode |= lmBonds;
    if (Options.Contains("rn"))  lmode |= lmResiNumber;
    if (Options.Contains("rc"))  lmode |= lmResiName;
  }
  if (lmode == 0) {
    lmode |= lmLabels;
    lmode |= lmQPeak;
    app.SetLabelsMode(lmode);
    if (!Cmds.IsEmpty() == 1 && Cmds[0].IsBool()) {
      app.SetLabelsVisible(Cmds[0].ToBool());
      Cmds.Delete(0);
    }
    else
      app.SetLabelsVisible(!app.AreLabelsVisible());
  }
  else {
    app.SetLabelsMode(lmode |= lmQPeak);
    app.SetLabelsVisible(true);
  }
  if (app.GetLabels().IsVisible()) {
    TGlGroup &sel = app.GetSelection();
    if ((app.GetLabels().GetMode()&lmBonds) == lmBonds) {
      TXBondPList bonds = sel.Extract<TXBond>();
      if (bonds.IsEmpty()) {
        app.GetLabels().Init(true, lmiDefault);
      }
      else {
        app.GetLabels().Init(true, lmiMasked);
        for (size_t i=0; i < bonds.Count(); i++) {
          app.GetLabels().SetMaterialIndex(bonds[i]->GetOwnerId(), lmiDefault);
        }
      }
    }
    else {
      TXAtomPList atoms = sel.Extract<TXAtom>();
      if (atoms.IsEmpty() && Cmds.IsEmpty()) {
        app.GetLabels().Init(true, lmiDefault);
      }
      else {
        app.GetLabels().Init(true, lmiMasked);
        TXAtomPList atoms = app.FindXAtoms(Cmds, true, false);
        for (size_t i = 0; i < atoms.Count(); i++) {
          app.GetLabels().SetMaterialIndex(atoms[i]->GetOwnerId(), lmiDefault);
        }
      }
    }
  }
  TStateRegistry::GetInstance().SetState(app.stateLabelsVisible,
    app.AreLabelsVisible(), EmptyString(), true);
}
//.............................................................................
void GXLibMacros::macLabel(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
  {
  TXAtomPList atoms;
  TXBondPList bonds;
  TPtrList<TXLine> lines;
  if (Cmds.IsEmpty()) {
    TGlGroup& sel = app.GetSelection();
    for (size_t i=0; i < sel.Count(); i++) {
      const std::type_info &oi = typeid(sel[i]);
      if (oi == typeid(TXAtom)) {
        atoms.Add((TXAtom&)sel[i]);
      }
      else if (oi == typeid(TXBond)) {
        bonds.Add((TXBond&)sel[i]);
      }
      else if (oi == typeid(TXLine)) {
        lines.Add((TXLine&)sel[i]);
      }
    }
    if (atoms.IsEmpty() && bonds.IsEmpty() && lines.IsEmpty()) {
      TGXApp::AtomIterator ai = app.GetAtoms();
      atoms.SetCapacity(ai.count);
      while (ai.HasNext()) {
        TXAtom& xa = ai.Next();
        if (xa.IsVisible()) {
          atoms.Add(xa);
        }
      }
      TGXApp::BondIterator bi = app.GetBonds();
      bonds.SetCapacity(bi.count);
      while (bi.HasNext()) {
        TXBond& xb = bi.Next();
        if (xb.IsVisible()) {
          bonds.Add(xb);
        }
      }
    }
  }
  else if (Cmds.Count() == 1 && Cmds[0].Equalsi("gbonds")) {
    app.LabelGrowBonds();
  }
  else {
    atoms = app.FindXAtoms(Cmds, true, false);
  }
  short lt = 0, symm_tag = 0, resi = 0;
  const olxstr str_lt = Options.FindValue("type");
  olxstr str_symm_tag = Options.FindValue("symm");
  // enforce the default
  if (str_lt.Containsi("brackets")) {
    lt = 1;
  }
  if (str_lt.Containsi("subscript") || str_lt.Containsi("sb")) {
    lt |= 2;
  }
  if (str_lt.Containsi("superscript") || str_lt.Containsi("sp")) {
    lt |= 4;
  }
  olxstr resi_sep = Options.FindValue("resi");
  if (!resi_sep.IsEmpty()) {
    if (resi_sep.GetLast() == '#') {
      resi = 1;
    }
    else {
      resi = 2;
    }
    resi_sep.SetLength(resi_sep.Length() - 1);
  }
  // have to kill labels in this case, for consistency of _$ or ^#
  if (str_symm_tag == '$' || str_symm_tag == '#' || str_symm_tag == 'X') {
    for (size_t i = 0; i < app.LabelCount(); i++) {
      app.GetLabel(i).SetVisible(false);
    }
    symm_tag = (str_symm_tag == '$' ? 1 : (str_symm_tag == '#' ? 2 : 3));
  }
  else if (str_symm_tag.Equals("full")) {
    symm_tag = 4;
  }
  TTypeList<uint32_t> equivs;
  for (size_t i = 0; i < atoms.Count(); i++) {
    TXGlLabel& gxl = atoms[i]->GetGlLabel();
    olxstr lb;
    if (lt != 0 &&
      atoms[i]->GetLabel().Length() > atoms[i]->GetType().symbol.Length())
    {
      olxstr bcc = atoms[i]->GetLabel().SubStringFrom(
        atoms[i]->GetType().symbol.Length());
      lb = atoms[i]->GetType().symbol;
      if ((lt & 1) == 1) {
        bcc = olxstr('(') << bcc << ')';
      }
      if ((lt & 2) == 2) {
        lb << "\\-" << bcc;
      }
      else if ((lt & 4) == 4) {
        lb << "\\+" << bcc;
      }
      else {
        lb << bcc;
      }
    }
    else {
      lb = atoms[i]->GetLabel();
    }
    if (resi != 0 && atoms[i]->CAtom().GetResiId() != 0) {
      lb << resi_sep;
      if (resi == 1) {
        lb << atoms[i]->CAtom().GetParent()->GetResidue(
          atoms[i]->CAtom().GetResiId()).GetNumber();
      }
      else {
        lb << atoms[i]->CAtom().GetParent()->GetResidue(
          atoms[i]->CAtom().GetResiId()).GetClassName();
      }
    }
    if (!atoms[i]->IsAUAtom()) {
      if (symm_tag >= 1  && symm_tag <= 3) {
        size_t pos = equivs.IndexOf(atoms[i]->GetMatrix().GetId());
        if (pos == InvalidIndex)  {
          equivs.AddCopy(atoms[i]->GetMatrix().GetId());
          pos = equivs.Count() - 1;
        }
        if (symm_tag == 1) {
          lb << "_$" << (pos + 1);
        }
        else if (symm_tag == 2) {
          lb << "\\+" << (pos + 1);
        }
        else {
          lb << "\\+" << RomanNumber::To(pos + 1).ToLowerCase();
        }
      }
      else if (symm_tag == 4) {
        lb << ' ' << TSymmParser::MatrixToSymmEx(atoms[i]->GetMatrix());
      }
    }
    gxl.SetOffset(atoms[i]->crd());
    gxl.SetLabel(lb);
    gxl.SetVisible(true);
  }
  TPtrList<TXGlLabel> labels;
  if (!bonds.IsEmpty()) {
    VcoVContainer vcovc(app.XFile().GetAsymmUnit());
    bool have_vcov = false;
    try {
      olxstr src_mat = app.InitVcoV(vcovc);
      app.NewLogEntry() << "Using " << src_mat <<
        " matrix for the calculation";
      have_vcov = true;
    }
    catch (const TExceptionBase&) {}
    const TCifDataManager *dm = 0;
    if (app.CheckFileType<TCif>()) {
      dm = &app.XFile().GetLastLoader<TCif>().GetDataManager();
    }
    for (size_t i = 0; i < bonds.Count(); i++) {
      TXGlLabel& l = bonds[i]->GetGlLabel();
      l.SetOffset(bonds[i]->GetCenter());
      bool matched = false;
      if (dm != 0) {
        ACifValue *v = dm->Match(bonds[i]->A(), bonds[i]->B());
        if (v != 0) {
          matched = true;
          l.SetLabel(v->GetValue().ToString());
        }
      }
      if (!matched) {
        if (have_vcov) {
          l.SetLabel(vcovc.CalcDistance(bonds[i]->A(),
            bonds[i]->B()).ToString());
        }
        else {
          l.SetLabel(olxstr::FormatFloat(3, bonds[i]->Length()));
        }
      }
      labels.Add(l)->SetVisible(true);
      l.TranslateBasis(-l.GetCenter());
    }
  }
  for (size_t i = 0; i < lines.Count(); i++) {
    lines[i]->GetGlLabel().SetVisible(true);
  }
  TStrList l_out;
  l_out.Add();
  for (size_t i = 0; i < equivs.Count(); i++) {
    smatd m = app.XFile().GetUnitCell().GetMatrix(
      smatd::GetContainerId(equivs[i]));
    m.t += smatd::GetT(equivs[i]);
    olxstr line;
    if (symm_tag == 1) {
      line << "$" << (i + 1);
    }
    else if (symm_tag == 2) {
      line << (i + 1);
    }
    else {
      line << RomanNumber::To(i + 1).ToLowerCase();
    }
    line.RightPadding(4, ' ', true) << TSymmParser::MatrixToSymmEx(m);
    if (i != 0 && (i % 3) == 0) {
      l_out.Add();
    }
    l_out.GetLastString() << line.RightPadding(26, ' ');
  }
  TBasicApp::NewLogEntry() << l_out << NewLineSequence();

  for (size_t i = 0; i < labels.Count(); i++) {
    TXGlLabel& l = *labels[i];
    vec3d off(-l.GetRect().width / 2, -l.GetRect().height / 2, 0);
    const double scale1 =
      l.GetFont().IsVectorFont() ? 1.0 / app.GetRenderer().GetScale() : 1.0;
    const double scale = scale1 / app.GetRenderer().GetBasis().GetZoom();
    l.TranslateBasis(off*scale);
    l.SetVisible(true);
  }
  if (Options.GetBoolOption('a') && !atoms.IsEmpty()) {
    TGlRenderer &r = app.GetRenderer();
    TEBitArray vis = r.GetVisibility();
    for (size_t i = 0; i < r.ObjectCount(); i++) {
      if (!r.GetObject(i).IsPrintable()) {
        r.GetObject(i).SetVisible(false);
      }
    }
    TGXApp::AtomIterator ai = app.GetAtoms();
    TGXApp::BondIterator bi = app.GetBonds();
    while (ai.HasNext()) {
      TXAtom &a = ai.Next();
      if (a.GetType() == iQPeakZ) {
        a.SetVisible(false);
      }
    }
    while (bi.HasNext()) {
      TXBond &b = bi.Next();
      if (b.A().GetType() == iQPeakZ || b.B().GetType() == iQPeakZ) {
        b.SetVisible(false);
      }
    }
    TXAtomLabelAligner la(atoms);
    la.Align();
    r.SetVisibility(vis);
  }
  app.SelectAll(false);
}
//.............................................................................
void GXLibMacros::macShowH(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TEBasis basis = app.GetRenderer().GetBasis();
  if (Cmds.Count() == 2) {
    bool v = Cmds[1].ToBool();
    if (Cmds[0] == 'a') {
      if (v && !app.AreHydrogensVisible()) {
        app.SetHydrogensVisible(true);
      }
      else if (!v && app.AreHydrogensVisible()) {
        app.SetHydrogensVisible(false);
      }
      TGXApp::AtomIterator ai = app.GetAtoms();
      while (ai.HasNext()) {
        TXAtom &a = ai.Next();
        if (a.GetType() == iHydrogenZ && !a.IsDeleted()) {
          a.SetVisible(v);
        }
      }
      TGXApp::BondIterator bi = app.GetBonds();
      while (bi.HasNext()) {
        TXBond &b = bi.Next();
        if (!b.IsDeleted() && 
          (b.A().GetType() == iHydrogenZ || b.B().GetType() == iHydrogenZ))
        {
          b.SetVisible(v);
        }
      }
    }
    else if (Cmds[0] == 'b') {
      if (v && !app.AreHBondsVisible()) {
        app.SetHBondsVisible(true);
      }
      else if (!v && app.AreHBondsVisible()) {
        app.SetHBondsVisible(false);
      }
    }
  }
  else {
    if (app.AreHydrogensVisible() && !app.AreHBondsVisible()) {
      app.SetHBondsVisible(true);
    }
    else if (app.AreHydrogensVisible() && app.AreHBondsVisible()) {
      app.SetHBondsVisible(false);
      app.SetHydrogensVisible(false);
    }
    else if (!app.AreHydrogensVisible() && !app.AreHBondsVisible()) {
      app.SetHydrogensVisible(true);
    }
  }
  TStateRegistry::GetInstance().SetState(app.stateHydrogensVisible,
    app.AreHydrogensVisible(), EmptyString(), true);
  TStateRegistry::GetInstance().SetState(app.stateHydrogenBondsVisible,
    app.AreHBondsVisible(), EmptyString(), true);
  app.GetRenderer().SetBasis(basis);
}
//.............................................................................
void GXLibMacros::macDetach(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TEBasis basis = app.GetRenderer().GetBasis();
  bool reattach = Options.GetBoolOption('u');
  TCAtomPList atoms = app.FindCAtoms(Cmds.Text(' '));
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i]->SetDetached(!reattach);
  }
  app.UpdateConnectivity();
  if (app.AtomLegend().IsVisible()) {
    app.AtomLegend().Update();
  }
  app.GetRenderer().SetBasis(basis);
}
//.............................................................................
int GXLibMacros::QPeakSortA(const TCAtom &a, const TCAtom &b)  {
  int v = olx_cmp(a.GetQPeak(), b.GetQPeak());
  if (v == 0 && a.GetLabel().Length() > 1 && b.GetLabel().Length() > 1) {
    if (a.GetLabel().SubStringFrom(1).IsNumber() &&
        b.GetLabel().SubStringFrom(1).IsNumber())
    {
      // larger the number - 'smaller' the peak
      v = olx_cmp(b.GetLabel().SubStringFrom(1).ToInt(),
        a.GetLabel().SubStringFrom(1).ToInt());
    }
  }
  return v;
}
void GXLibMacros::macShowQ(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (!app.XFile().HasLastLoader()) {
    return; // quiet this down
  }
  double wheel = Options.FindValue("wheel", '0').ToDouble();
  TEBasis basis = app.GetRenderer().GetBasis();
  if (wheel != 0) {
    //if( !app.QPeaksVisible() )  return;
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    TCAtomPList qpeaks;
    for (size_t i=0; i < au.AtomCount(); i++) {
      if (!au.GetAtom(i).IsDeleted() && au.GetAtom(i).GetType() == iQPeakZ)
        qpeaks.Add(au.GetAtom(i));
    }
    QuickSorter::SortSF(qpeaks, &GXLibMacros::QPeakSortD);
    index_t d_cnt = 0;
    for (size_t i = 0; i < qpeaks.Count(); i++) {
      if (!qpeaks[i]->IsDetached()) {
        d_cnt++;
      }
    }
    if (d_cnt == 0 && wheel < 0) {
      return;
    }
    if (d_cnt == (index_t)qpeaks.Count() && wheel > 0) {
      return;
    }
    d_cnt += (index_t)(wheel);
    if (d_cnt < 0)  d_cnt = 0;
    if (d_cnt > (index_t)qpeaks.Count())
      d_cnt = qpeaks.Count();
    for (size_t i = 0; i < qpeaks.Count(); i++) {
      qpeaks[i]->SetDetached(i >= (size_t)d_cnt);
    }
    app.UpdateConnectivity();
    app.Draw();
  }
  else if (Cmds.Count() == 2) {
    bool v = Cmds[1].ToBool();
    if (Cmds[0] == 'a') {
      if (v && !app.AreQPeaksVisible()) {
        app.SetQPeaksVisible(true);
      }
      else if (!v && app.AreQPeaksVisible()) {
        app.SetQPeaksVisible(false);
      }
    }
    else if (Cmds[0] == 'b') {
      if (v && !app.AreQPeakBondsVisible()) {
        app.SetQPeakBondsVisible(true);
      }
      else if (!v && app.AreQPeakBondsVisible()) {
        app.SetQPeakBondsVisible(false);
      }
    }
  }
  else if (Cmds.Count() == 1 && Cmds[0].IsNumber()) {
    index_t num = Cmds[0].ToInt();
    const bool negative = num < 0;
    num = olx_abs(num);
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    TCAtomPList qpeaks;
    for (size_t i=0; i < au.AtomCount(); i++) {
      if (au.GetAtom(i).GetType() == iQPeakZ)
        qpeaks.Add(au.GetAtom(i));
    }
    QuickSorter::SortSF(qpeaks,
      &(negative ? GXLibMacros::QPeakSortD : GXLibMacros::QPeakSortA));
    num = olx_min(qpeaks.Count()*num/100, qpeaks.Count());
    for (size_t i=0; i < qpeaks.Count(); i++)
      qpeaks[i]->SetDetached(i >= (size_t)num);
    app.GetSelection().Clear();
    app.UpdateConnectivity();
    app.Draw();
  }
  else {
    if ((!app.AreQPeaksVisible() && !app.AreQPeakBondsVisible())) {
      app.SetQPeaksVisible(true);
    }
    else if (app.AreQPeaksVisible() && !app.AreQPeakBondsVisible()) {
      app.SetQPeakBondsVisible(true);
    }
    else if (app.AreQPeaksVisible() && app.AreQPeakBondsVisible()) {
      app.SetQPeaksVisible(false);
      app.SetQPeakBondsVisible(false);
    }
  }
  TStateRegistry::GetInstance().SetState(app.stateQPeaksVisible,
    app.AreQPeaksVisible(), EmptyString(), true);
  TStateRegistry::GetInstance().SetState(app.stateQPeakBondsVisible,
    app.AreQPeakBondsVisible(), EmptyString(), true);
  app.GetRenderer().SetBasis(basis);
}
//.............................................................................
void GXLibMacros::macLoad(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (Cmds.IsEmpty()) {
    Error.SetUnhandled(true);
    return;
  }
  if (Cmds[0].Equalsi("textures")) {
    Cmds.Delete(0);
    if (Cmds.IsEmpty())
      app.ClearTextures(~0);
    else
      app.LoadTextures(Cmds.Text(' '));
  }
  else {
    Error.SetUnhandled(true);
  }
}
//.............................................................................
void GXLibMacros::macMatr(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (Cmds.IsEmpty()) {
    TBasicApp::NewLogEntry() << "Current orientation matrix:";
    const mat3d& Matr = app.GetRenderer().GetBasis().GetMatrix();
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
      app.GetRenderer().GetBasis().OrientNormal(n);
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
      app.GetRenderer().GetBasis().OrientNormal(n);
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
      app.GetRenderer().GetBasis().OrientNormal(n);
    }
    else if (Cmds.Count() == 9) {
      mat3d M(
        Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble(),
        Cmds[3].ToDouble(), Cmds[4].ToDouble(), Cmds[5].ToDouble(),
        Cmds[6].ToDouble(), Cmds[7].ToDouble(), Cmds[8].ToDouble());
      M.Transpose();
      app.GetRenderer().GetBasis().SetMatrix(M.Normalise());
    }
    app.Draw();
  }
}
//.............................................................................
void GXLibMacros::macSetView(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  const bool do_center = Options.Contains('c');
  bool process = false, has_center = true;
  vec3d center, normal;
  if (Cmds.IsEmpty()) {
    TGlGroup& g = app.GetSelection();
    if (g.Count() == 1) {
      if (g[0].Is<TXPlane>()) {
        TXPlane& xp = (TXPlane&)g[0];
        normal = xp.GetNormal();
        center = xp.GetCenter();
        process = true;
      }
      else if (g[0].Is<TXBond>()) {
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
    app.GetRenderer().GetBasis().OrientNormal(normal);
    if (do_center && has_center)
      app.GetRenderer().GetBasis().SetCenter(-center);
    if (app.XGrid().IsVisible() && has_center)
      app.SetGridDepth(center);
    app.Draw();
  }
}
//.............................................................................
void GXLibMacros::macLine(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  bool process_atoms = true;
  vec3d from, to;
  if (Cmds.IsEmpty() || (Cmds.Count() == 1 && Cmds[0].Equalsi("sel"))) {
    TGlGroup& sel = app.GetSelection();
    if( sel.Count() == 2 )  {
      if (sel[0].Is<TXPlane>() && sel[1].Is<TXPlane>()) {
        from = ((TXPlane&)sel[0]).GetCenter();
        to = ((TXPlane&)sel[1]).GetCenter();
        process_atoms = false;
      }
      else if (sel[0].Is<TXAtom>() &&
        sel[1].Is<TXPlane>())
      {
        from = ((TXAtom&)sel[0]).crd();
        to = ((TXPlane&)sel[1]).GetCenter();
        process_atoms = false;
      }
      else if (sel[0].Is<TXPlane>() &&
        sel[1].Is<TXAtom>())
      {
        from = ((TXPlane&)sel[0]).GetCenter();
        to = ((TXAtom&)sel[1]).crd();
        process_atoms = false;
      }
    }
  }
  if (Cmds.Count() == 6 && olx_list_and(Cmds, &olxstr::IsNumber)) {
    TDoubleList a = TDoubleList::FromList(Cmds,
      FunctionAccessor::MakeConst(&olxstr::ToDouble));
    from = vec3d(a[0], a[1], a[2]);
    to = vec3d(a[3], a[4], a[5]);
    if (Options.GetBoolOption('f')) {
      from = app.XFile().GetAsymmUnit().Orthogonalise(from);
      to = app.XFile().GetAsymmUnit().Orthogonalise(to);
    }
    process_atoms = false;
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
    double rmsd = 0;
    for (size_t i = 0; i < Atoms.Count(); i++) {
      vec3d v = Atoms[i]->crd() - center;
      rmsd += (v - params[2]*params[2].DotProd(v)).QLength();
    }
    TBasicApp::NewLogEntry() << "RMSD/A: " <<
      olxstr::FormatFloat(3, sqrt(rmsd/Atoms.Count()));
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
    app.GetRenderer().GetBasis().OrientNormal(to-from);
  else
    app.AddLine(name, from, to);
  app.Draw();
}
//.............................................................................
void GXLibMacros::macMpln(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  olxstr rings_name = Options.FindValue("rings");
  if (rings_name.IsEmpty()) {
    TSPlane* plane = NULL;
    bool orientOnly = Options.Contains('n'),
      reqular = Options.Contains('r');
    size_t sides_n = 0;
    if (reqular) {
      olxstr v = Options.FindValue('r');
      sides_n = (v.IsEmpty()) ? 4 : v.ToSizeT();
      if (sides_n < 4) {
        sides_n = 4;
      }
    }
    olxstr name = Options.FindValue('n');
    const double weightExtent = olx_abs(Options.FindValue("we", "0").ToDouble());
    olxstr planeName;
    TXAtomPList Atoms = app.FindXAtoms(Cmds, true, true);
    for (size_t i=0; i < Atoms.Count(); i++) {
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
        app.GetRenderer().GetBasis().Orient(d1, d2, m[0]);
        app.SetGridDepth(plane->GetCenter());
        delete plane;
        plane = 0;
      }
    }
    else {
      TXPlane* xp = app.AddPlane(name, Atoms, sides_n, weightExtent);
      if (xp != 0) {
        plane = xp;
      }
    }
    if (plane != 0) {
      const vec3d& Z = app.GetRenderer().GetBasis().GetMatrix()[2];
      if (plane->GetNormal().DotProd(Z) < 0) {
        plane->Invert();
      }
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
          if ((i + j) >= Atoms.Count()) {
            break;
          }
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
        "The plane was not created because it is either not unique or invalid";
    }
  }
  else {
    TTypeList<TSAtomPList> rings = app.FindRings(rings_name);
    olxstr name = "Plane";
    name << app.XFile().GetLattice().GetObjects().planes.Count() << '_';
    size_t cnt=0;
    for (size_t i=0; i < rings.Count(); i++) {
      if( app.AddPlane(name + olxstr(cnt+1),
         TXAtomPList(rings[i], DynamicCastAccessor<TXAtom>()), false) != NULL)
      {
        cnt++;
      }
    }
    if (cnt != 0) {
      TBasicApp::NewLogEntry() << "Created " << cnt << " rings";
    }
  }
}
//.............................................................................
void GXLibMacros::macCent(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  olxstr rings_name = Options.FindValue("rings");
  if (rings_name.IsEmpty()) {
    app.AddCentroid(app.FindXAtoms(Cmds, true, true).GetObject());
  }
  else {
    TTypeList<TSAtomPList> rings = app.FindRings(rings_name);
    for (size_t i = 0; i < rings.Count(); i++) {
      app.AddCentroid(TXAtomPList(rings[i], DynamicCastAccessor<TXAtom>()));
    }
  }
}
//.............................................................................
void GXLibMacros::macUniq(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXAtomPList Atoms = app.FindXAtoms(Cmds, false, true);
  if (Atoms.IsEmpty()) {
    olex2::IOlex2Processor::GetInstance()->processMacro("fmol");
    return;
  }
  TNetPList L(Atoms, FunctionAccessor::MakeConst(&TXAtom::GetNetwork));
  app.FragmentsVisible(app.InvertFragmentsList(
    ACollectionItem::Unify(L)), false);
  app.CenterView(true);
  app.Draw();
}
//.............................................................................
void GXLibMacros::macGroup(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (Options.GetBoolOption('u')) {
    if (app.GetSelection().IsEmpty()) {
      app.UngroupAll();
    }
    else {
      app.UngroupSelection();
    }
    return;
  }
  olxstr name = Options.FindValue('n');
  if (app.GetSelection().IsEmpty()) {
    app.SelectAll(true);
  }
  if (name.IsEmpty())  {
    name = "group";
    name << (app.GetRenderer().GroupCount()+1);
  }
  app.GroupSelection(name);
}
//.............................................................................
void GXLibMacros::macFmol(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  app.AllVisible(true);
  app.CenterView();
  app.GetRenderer().GetBasis().SetZoom(
    app.GetRenderer().CalcZoom()*app.GetExtraZoom());
}
//.............................................................................
void GXLibMacros::macCell(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  app.SetCellVisible(Cmds.IsEmpty() ? !app.IsCellVisible() : Cmds[0].ToBool());
  if (app.XFile().DUnitCell->IsVisible()) {
    bool r = Options.GetBoolOption('r');
    app.XFile().DUnitCell->SetReciprocal(r, r ? 100: 1);
  }
  for (size_t i = 1; i < app.XFiles().Count(); i++) {
    app.XFile(i).DUnitCell->SetVisible(
      app.XFile().DUnitCell->IsVisible());
  }
  app.CenterView();
}
//.............................................................................
void GXLibMacros::macSel(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (TModeRegistry::GetInstance().GetCurrent() != 0) {
    if (Options.GetBoolOption('i')) {
      if (InvertFittedFragment()) {
        return;
      }
    }
    TBasicApp::NewLogEntry(logError) << "Unavailable in a mode";
    return;
  }
  glSelectionFlag flag=glSelectionNone;
  if (Options.GetBoolOption('a')) {
    flag = glSelectionSelect;
  }
  else if (Options.GetBoolOption('u')) {
    flag = glSelectionUnselect;
  }
  else if (Options.GetBoolOption('i')) {
    flag = glSelectionInvert;
  }

  if (Cmds.Count() >= 1 && Cmds[0].Equalsi("frag")) {
    TNetPList nets(
      app.FindXAtoms(TStrObjList(Cmds.SubListFrom(1)), false, false),
      FunctionAccessor::MakeConst(&TXAtom::GetNetwork));
    app.SelectFragments(ACollectionItem::Unify(nets), !Options.Contains('u'));
  }
  else if (Cmds.Count() == 1 && Cmds[0].Equalsi("collections")) {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    olxstr_set<> cols;
    for (size_t i = 0; i < TXBond::NamesRegistry().Count(); i++) {
      cols.Add(TXBond::NamesRegistry().GetValue(i));
    }
    for (size_t i = 0; i < TXAtom::NamesRegistry().Count(); i++) {
      cols.Add(TXAtom::NamesRegistry().GetValue(i));
    }
    for (size_t i = 0; i < TXPlane::NamesRegistry().Count(); i++) {
      cols.Add(TXPlane::NamesRegistry().GetValue(i));
    }
    for (size_t i = 0; i < cols.Count(); i++) {
      TGPCollection *c = app.GetRenderer().FindCollection(cols[i]);
      if (c == 0) {
        continue;
      }
      for (size_t j = 0; j < c->ObjectCount(); j++) {
        app.GetRenderer().Select(c->GetObject(j), flag);
      }
    }
  }
  else if (Cmds.Count() > 0 && Cmds[0].Equalsi("sel")) {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    TSAtomPList atoms = app.FindXAtoms(Cmds, false, true);
    if (atoms.IsEmpty()) {
      return;
    }
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    ACollectionItem::Unify(atoms);
    using namespace olx_analysis;
    TCAtomPList fa(atoms, FunctionAccessor::MakeConst(&TSAtom::CAtom));
    fragments::fragment fr(fa);
    TTypeList<fragments::fragment> frags =
      fragments::extract(app.XFile().GetAsymmUnit(), fr);
    app.XFile().GetAsymmUnit().GetAtoms().ForEach(
      ACollectionItem::TagSetter(0));
    for (size_t fi = 0; fi <= frags.Count(); fi++) {
      fragments::fragment *f = (fi == 0 ? &fr : &frags[fi - 1]);
      for (size_t j = 0; j < f->count(); j++) {
        (*f)[j].SetTag(1);
      }
    }
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext()) {
      TXAtom &a = ai.Next();
      if (a.IsVisible() && a.CAtom().GetTag() == 1) {
        app.GetRenderer().Select(a, flag);
      }
    }
  }
  else if (Cmds.Count() == 1 && Cmds[0].Equalsi("res")) {
    //app.GetRenderer().ClearSelection();
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
    //  app.GetRenderer().Select(xa);
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
    //        app.GetRenderer().Select(xb);
    //        break;
    //      }
    //    }
    //  }
    //}
    //FGlConsole->PrintText(out);
    //app.GetLog() << NewLineSequence();
  }
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("resi")) {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    TAsymmUnit &au = app.XFile().GetAsymmUnit();
    au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
    TSizeList nums;
    TStrList names;
    for (size_t i=1; i < Cmds.Count(); i++) {
      if (Cmds[i].IsInt()) {
        nums << Cmds[i].ToSizeT();
      }
      else {
        names << Cmds[i];
      }
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
      if (!found) {
        continue;
      }
      for (size_t j = 0; j < r.Count(); j++) {
        r[j].SetTag(1);
      }
    }
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext()) {
      TXAtom &a = ai.Next();
      if (a.IsVisible() && a.CAtom().GetTag() == 1) {
        app.GetRenderer().Select(a, flag);
      }
    }
  }
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("ofile")) {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    TSizeList fi;
    for (size_t i = 1; i < Cmds.Count(); i++) {
      fi << Cmds[i].ToSizeT();
    }
    for (size_t i=0; i < fi.Count(); i++) {
      TXFile *f = 0;
      if (fi[i] < app.XFiles().Count()) {
        f = &app.XFiles()[fi[i]];
      }
      if (f == 0) {
        continue;
      }
      ASObjectProvider &op = f->GetLattice().GetObjects();
      for (size_t j=0; j < op.atoms.Count(); j++) {
        TXAtom &a = (TXAtom &)op.atoms[j];
        if (!a.IsAvailable()) continue;
        app.GetRenderer().Select(a, flag);
      }
      for (size_t j=0; j < op.bonds.Count(); j++) {
        TXBond &b = (TXBond &)op.bonds[j];
        if (!b.IsAvailable()) {
          continue;
        }
        app.GetRenderer().Select(b, flag);
      }
    }
  }
  else if (Cmds.Count() == 1 && TSymmParser::IsRelSymm(Cmds[0])) {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    const smatd matr = TSymmParser::SymmCodeToMatrix(
      app.XFile().GetUnitCell(), Cmds[0]);
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext()) {
      TXAtom& a = ai.Next();
      if (a.IsDeleted() || !a.IsVisible()) {
        continue;
      }
      if (a.IsGenerator(matr)) {
        app.GetRenderer().Select(a, flag);
      }
    }
    TGXApp::BondIterator bi = app.GetBonds();
    while (bi.HasNext()) {
      TXBond& b = bi.Next();
      if (b.IsDeleted() || !b.IsVisible()) {
        continue;
      }
      if (b.A().IsGenerator(matr) && b.B().IsGenerator(matr)) {
        app.GetRenderer().Select(b, flag);
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
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("afix")) {
    Cmds.Delete(0);
    SortedObjectList<int, TPrimitiveComparator> afixes;
    for (size_t i=0; Cmds.Count(); i++) {
      if (Cmds[i].IsNumber()) {
        afixes.AddUnique(Cmds[i].ToInt());
        Cmds.Delete(i--);
      }
      else
        break;
    }
    if (!afixes.IsEmpty()) {
      if (flag == glSelectionNone) {
        flag = glSelectionSelect;
      }
      TGXApp::AtomIterator ai = app.GetAtoms();
      while (ai.HasNext()) {
        TXAtom& xa = ai.Next();
        if (afixes.Contains(xa.CAtom().GetAfix()))
          app.GetRenderer().Select(xa, flag);
      }
    }
  }
  else if (Cmds.Count() > 1 && Cmds[0].Equalsi("fvar")) {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
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
        if (!a.IsVisible()) {
          continue;
        }
        XVarReference *ref = a.CAtom().GetVarRef(catom_var_name_Sof);
        if (ref == 0) {
          continue;
        }
        int v = int(ref->Parent.GetId());
        if (ref->relation_type == relation_AsOneMinusVar)
          v *= -1;
        for (size_t i=0; i < fvars.Count(); i++) {
          if (fvars[i] == v) {
            app.GetRenderer().Select(a, flag);
          }
        }
      }
    }
  }
  else if (Cmds.Count() == 1 && Cmds[0].Equalsi("isot")) {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext()) {
      TXAtom& xa = ai.Next();
      if (!xa.IsVisible()) {
        continue;
      }
      if (xa.GetEllipsoid() == 0) {
        app.GetRenderer().Select(xa, flag);
      }
    }
  }
  else if (Cmds.Count() == 1 && Cmds[0].Equalsi("anis")) {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext()) {
      TXAtom& xa = ai.Next();
      if (!xa.IsVisible()) {
        continue;
      }
      if (xa.GetEllipsoid() != 0) {
        app.GetRenderer().Select(xa, flag);
      }
    }
  }
  else if (Cmds.Count() == 1 && Cmds[0].Equalsi("atoms")) {
    TGXApp::AtomIterator ai = app.GetAtoms();
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    while (ai.HasNext()) {
      TXAtom &a = ai.Next();
      if (a.IsVisible()) {
        app.GetRenderer().Select(a, flag);
      }
    }
  }
  else if (Cmds.Count() == 1 && Cmds[0].Equalsi("bonds")) {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    TGXApp::BondIterator bi = app.GetBonds();
    while (bi.HasNext()) {
      TXBond &b = bi.Next();
      if (b.IsVisible()) {
        app.GetRenderer().Select(b, flag);
      }
    }
  }
  else if (Cmds.Count() == 2 && Cmds[0].Equalsi("bond") &&
    Cmds[1].Equalsi("atoms"))
  {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    TGXApp::BondIterator bi = app.GetBonds();
    while (bi.HasNext()) {
      TXBond &b = bi.Next();
      if (b.IsSelected()) {
        app.GetRenderer().Select(b.A(), flag);
        app.GetRenderer().Select(b.B(), flag);
      }
    }
  }
  else if (Cmds.Count() == 2 && Cmds[0].Equalsi("atom") &&
    Cmds[1].Equalsi("bonds"))
  {
    bool both = false;;
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
      both = true;
    }
    TGXApp::BondIterator bi = app.GetBonds();
    while (bi.HasNext()) {
      TXBond &b = bi.Next();
      if (both) {
        if (b.A().IsSelected() && b.B().IsSelected()) {
          app.GetRenderer().Select(b, flag);
        }
      }
      else {
        if (b.A().IsSelected() || b.B().IsSelected()) {
          app.GetRenderer().Select(b, flag);
        }
      }
    }
  }
  else if (Cmds.Count() > 0 && Cmds[0].Equalsi("planes")) {
    if (flag == glSelectionNone) {
      flag = glSelectionSelect;
    }
    int pc = -1;
    if (Cmds.Count() > 1 && Cmds[1].IsNumber()) {
      pc = Cmds[1].ToInt();
    }
    TGXApp::PlaneIterator pi = app.GetPlanes();
    while (pi.HasNext()) {
      TXPlane &p = pi.Next();
      if (p.IsVisible()) {
        if (pc < 0 || pc == p.Count()) {
          app.GetRenderer().Select(p, flag);
        }
      }
    }
  }
  else if (Cmds.Count() >= 1 && Cmds[0].Equalsi("wbox")) {
    if (app.Get3DFrame().IsVisible()) {
      app.Get3DFrame().SetVisible(false);
    }
    else {
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
        app.GetUndo().Push(app.SetGraphicsVisible(&fr, true));
      }
      else {
        TSAtomPList atoms;
        if (app.FindSAtoms(EmptyString(), atoms, true)) {
          const WBoxInfo bs = TXApp::CalcWBox(atoms, NULL, TSAtom::weight_occu);
          T3DFrameCtrl& fr = app.Get3DFrame();
          vec3d nx = bs.normals[0] * bs.s_from[0];
          vec3d px = bs.normals[0] * bs.s_to[0];
          vec3d ny = bs.normals[1] * bs.s_from[1];
          vec3d py = bs.normals[1] * bs.s_to[1];
          vec3d nz = bs.normals[2] * bs.s_from[2];
          vec3d pz = bs.normals[2] * bs.s_to[2];
          if (ny.XProdVec(nx).DotProd(nz) < 0) {
            olx_swap(nx, px);
            olx_swap(ny, py);
            olx_swap(nz, pz);
          }
          fr.SetEdge(0, nx + ny + nz);
          fr.SetEdge(1, nx + py + nz);
          fr.SetEdge(2, px + py + nz);
          fr.SetEdge(3, px + ny + nz);
          fr.SetEdge(4, nx + ny + pz);
          fr.SetEdge(5, nx + py + pz);
          fr.SetEdge(6, px + py + pz);
          fr.SetEdge(7, px + ny + pz);
          fr.UpdateEdges();
          fr.Translate(bs.center);
          app.GetUndo().Push(app.SetGraphicsVisible(&fr, true));
        }
      }
    }
  }
  else if (Cmds.Count() >= 2 && Cmds[0].Equalsi("cif")) {
    if (Cmds[1].Equalsi("bonds")) {
      app.XFile().GetRM().GetSelectedTableRows().AddSelectedBonds(
        TStrList(Cmds.Text(' ', 2), ';'), app.XFile().GetAsymmUnit());
    }
    else if (Cmds[1].Equalsi("angles")) {
      app.XFile().GetRM().GetSelectedTableRows().AddSelectedAngles(
        TStrList(Cmds.Text(' ', 2), ';'), app.XFile().GetAsymmUnit());
    }
    else if (Cmds[1].Equalsi("torsions") || Cmds[1].Equalsi("dihedrals")) {
      app.XFile().GetRM().GetSelectedTableRows().AddSelectedDihedrals(
        TStrList(Cmds.Text(' ', 2), ';'), app.XFile().GetAsymmUnit());
    }
  }
  else if (flag == glSelectionNone) {
    if (Cmds.IsEmpty()) {
      size_t period = 5;
      TXAtomPList Atoms = app.FindXAtoms("sel", false, false);
      if (Atoms.IsEmpty() && Cmds.Count() == 1) {
        TGPCollection* gpc = app.GetRenderer().FindCollection(Cmds[0]);
        if (gpc != NULL) {
          for (size_t i = 0; i < gpc->ObjectCount(); i++) {
            app.GetRenderer().Select(gpc->GetObject(i));
          }
          return;
        }
      }
      for (size_t i = 0; i <= Atoms.Count(); i += period) {
        olxstr Tmp;
        for (size_t j = 0; j < period; j++) {
          if ((j + i) >= Atoms.Count()) {
            break;
          }
          Tmp << Atoms[i + j]->GetGuiLabel();
          Tmp.RightPadding((j + 1) * 14, ' ', true);
        }
        if (!Tmp.IsEmpty()) {
          TBasicApp::NewLogEntry() << Tmp;
        }
      }
    }
    else { // !Cmds.IsEmpty()
      size_t whereIndex = Cmds.IndexOf("where");
      if (whereIndex >= 1 && whereIndex != InvalidIndex) {
        olxstr Tmp = Cmds[whereIndex-1];
        while (olx_is_valid_index(whereIndex)) { Cmds.Delete(whereIndex--); }
        if (Tmp.Equalsi("atoms")) {
          app.SelectAtomsWhere(Cmds.Text(' '));
        }
        else if (Tmp.Equalsi("bonds")) {
          app.SelectBondsWhere(Cmds.Text(' '));
        }
        else {
          Error.ProcessingError(__OlxSrcInfo, "undefined keyword: ") << Tmp;
        }
        return;
      }
      else {
        size_t idx;
        if ((idx = Cmds.IndexOf("rings")) != InvalidIndex) {
          Cmds.Delete(idx);
          app.SelectRings(Cmds.Text(' '));
        }
        else if ((idx = Cmds.IndexOf("collection")) != InvalidIndex) {
          TTypeList<Wildcard> masks;
          for (size_t i = 0; i < Cmds.Count(); i++) {
            if (i == idx) {
              continue;
            }
            masks.AddNew(Cmds[i]);
          }
          for (size_t i = 0; i < app.GetRenderer().CollectionCount(); i++) {
            TGPCollection &c = app.GetRenderer().GetCollection(i);
            for (size_t j = 0; j < masks.Count(); j++) {
              if (masks[j].DoesMatch(c.GetName())) {
                for (size_t oi = 0; oi < c.ObjectCount(); oi++) {
                  app.GetRenderer().Select(c.GetObject(oi));
                }
              }
            }
          }
        }
        else {
          app.SelectAtoms(Cmds.Text(' '));
        }
        return;
      }
    }
    olxstr seli = app.GetSelectionInfo(Options.Contains('l'));
    if (!seli.IsEmpty()) {
      app.NewLogEntry() << seli;
      if (Options.Contains('c')) {
        app.ToClipboard(seli);
      }
    }
  }
  else {
    if (Cmds.IsEmpty()) {
      if (flag == glSelectionSelect) {
        app.SelectAll(true);
      }
      else if (flag == glSelectionUnselect) {
        app.SelectAll(false);
      }
      else if (flag == glSelectionInvert) {
        app.GetRenderer().InvertSelection();
      }
    }
    else {
      TXAtomPList atoms = app.FindXAtoms(Cmds.Text(' '), false, false);
      TGlRenderer &r = app.GetRenderer();
      for (size_t i=0; i < atoms.Count(); i++) {
        if (flag == glSelectionSelect) {
          r.Select(*atoms[i], true);
        }
        else if (flag == glSelectionUnselect) {
          r.Select(*atoms[i], false);
        }
        else if (flag == glSelectionInvert || flag == glSelectionNone) {
          r.Select(*atoms[i], !atoms[i]->IsSelected());
        }
      }
    }
  }
  // sort the selection
  {
    TGlGroup &glg = app.GetRenderer().GetSelection();
    for (size_t i = 0; i < glg.Count(); i++) {
      AGDrawObject & o = glg[i];
      if (o.Is<TXAtom>()) {
        o.SetTag(0);
      }
      else {
        o.SetTag(1);
      }
    }
    glg.SortObjectsByTag();
  }
}
//.............................................................................
void GXLibMacros::macUndo(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (!app.GetUndo().isEmpty()) {
    TUndoData* data = app.GetUndo().Pop();
    data->Undo();
    delete data;
  }
}
//.............................................................................
void GXLibMacros::macBasis(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  app.SetBasisVisible(
    Cmds.IsEmpty() ? !app.IsBasisVisible() : Cmds[0].ToBool());
}
//.............................................................................
void GXLibMacros::macPiM(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TTypeList<TSAtomPList> rings;
  TTypeList<TSAtomPList> ring_M;
  bool check_metal = true;
  ConstPtrList<TXAtom> atoms = app.FindXAtoms(Cmds, false, true);
  if (atoms.IsEmpty()) {
    app.FindRings("C4", rings);
    app.FindRings("C5", rings);
    app.FindRings("C6", rings);
    app.FindRings("NC5", rings);
    for (size_t i = 0; i < rings.Count(); i++) {
      if (TSPlane::CalcRMSD(rings[i]) > 0.05 ||
          !TNetwork::IsRingRegular(rings[i]))
      {
        rings.NullItem(i);
      }
    }
    rings.Pack();
  }
  else {
    rings.Add(new TSAtomPList(atoms, StaticCastAccessor<TSAtom>()));
    TSAtomPList &metals = ring_M.AddNew();
    for (size_t i=0; i < rings[0].Count(); i++) {
      if (XElementLib::IsMetal(rings[0][i]->GetType())) {
        check_metal = false;
        metals.Add(rings[0][i]);
        rings[0][i] = NULL;
      }
    }
    rings[0].Pack();
    if (rings[0].IsEmpty()) {
      return;
    }
    if (check_metal) {
      ring_M.Delete(0);
    }
  }
  if (check_metal) {
    ring_M.SetCapacity(rings.Count());
    for (size_t i=0; i < rings.Count(); i++) {
      TSAtomPList &metals = ring_M.AddNew();
      for (size_t j=0; j < rings[i].Count(); j++) {
        for (size_t k=0; k < rings[i][j]->NodeCount(); k++) {
          if (XElementLib::IsMetal(rings[i][j]->Node(k).GetType())) {
            metals.Add(rings[i][j]->Node(k));
          }
        }
      }
      if (metals.IsEmpty()) {
        rings.NullItem(i);
        ring_M.NullItem(ring_M.Count()-1);
      }
    }
    rings.Pack();
    ring_M.Pack();
  }
  // process rings...
  if (rings.IsEmpty()) {
    return;
  }
  TGXApp::AtomIterator ai = app.GetAtoms();
  while (ai.HasNext()) {
    ai.Next().SetTag(0);
  }
  bool create_c = Options.GetBoolOption("c");
  bool label = Options.GetBoolOption("l");

  const olxstr sb_name = TXBond::GetSettings(app.GetRenderer())
    .GetPrimitives(true)[9];
  for (size_t i=0; i < rings.Count(); i++) {
    vec3d c;
    for (size_t j=0; j < rings[i].Count(); j++) {
      c += rings[i][j]->crd();
      rings[i][j]->SetTag(1);
    }
    c /= rings[i].Count();
    ACollectionItem::Unify(ring_M[i]);
    for (size_t j = 0; j < ring_M[i].Count(); j++) {
      TXLine *l = app.AddLine(ring_M[i][j]->GetLabel() + olxstr(i),
        ring_M[i][j]->crd(), c);
      if (l == 0) {
        continue;
      }
      TGraphicsStyle &ms = ((TXAtom*)ring_M[i][j])->GetPrimitives().GetStyle();
      TGlMaterial *sm = ms.FindMaterial("Sphere");
      if (sm != 0) {
        l->GetPrimitives().GetStyle().SetMaterial(sb_name, *sm);
      }
      l->UpdatePrimitives((1 << 9) | (1 << 10));
      l->SetRadius(0.5);
      ((AGlMouseHandler *)l)->SetMoveable(false);
      ring_M[i][j]->SetTag(2);
      if (!label) {
        l->GetGlLabel().SetVisible(false);
      }
    }
  }
  // hide replaced bonds
  TGXApp::BondIterator bi = app.GetBonds();
  while (bi.HasNext()) {
    TXBond &b = bi.Next();
    if (b.A().GetTag() == 2 && b.B().GetTag() == 1) {
      b.SetVisible(false);
    }
  }
}
//.............................................................................
void GXLibMacros::macShowP(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TIntList parts;
  for (size_t i=0; i < Cmds.Count(); i++) {
    parts.Add(Cmds[i].ToInt());
  }
  app.ShowPart(parts, true, Options.GetBoolOption('v'));
  if (!Options.GetBoolOption('m')) {
    app.CenterView();
  }
}
//.............................................................................
void GXLibMacros::macShowR(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TIntList numbers;
  TStrList names;
  for (size_t i = 0; i < Cmds.Count(); i++) {
    if (Cmds[i].IsNumber()) {
      numbers.Add(Cmds[i].ToInt());
    }
    else {
      names.Add(Cmds[i]);
    }
  }
  app.ShowResi(numbers, names, true, Options.GetBoolOption('v'));
  if (!Options.GetBoolOption('m'))
    app.CenterView();
}
//.............................................................................
void main_CreateWBox(TGXApp& app, const TSAtomPList& atoms,
  double (*weight_c)(const TSAtom&),
  const TDoubleList& all_radii, bool print_info)
{
  const WBoxInfo bs = TXApp::CalcWBox(atoms, &all_radii, weight_c);
  static int obj_cnt = 0;
  if (print_info) {
    app.NewLogEntry() << "Wrapping box dimension: " <<
      olxstr::FormatFloat(3, bs.r_to[0] - bs.r_from[0]) << " x " <<
      olxstr::FormatFloat(3, bs.r_to[1] - bs.r_from[1]) << " x " <<
      olxstr::FormatFloat(3, bs.r_to[2] - bs.r_from[2]) << " A";
    app.NewLogEntry() << "Wrapping box volume: " <<
      olxstr::FormatFloat(3, (bs.r_to - bs.r_from).Prod()) << " A^3";
  }
  const vec3d nx = bs.normals[0]*bs.s_from[0];
  const vec3d px = bs.normals[0]*bs.s_to[0];
  const vec3d ny = bs.normals[1]*bs.s_from[1];
  const vec3d py = bs.normals[1]*bs.s_to[1];
  const vec3d nz = bs.normals[2]*bs.s_from[2];
  const vec3d pz = bs.normals[2]*bs.s_to[2];

  vec3d faces[] = {
    px + py + pz, px + ny + pz, px + ny + nz, px + py + nz, // normals[0]
    nx + py + pz, nx + py + nz, nx + ny + nz, nx + ny + pz, // -normals[0]
    px + py + pz, px + py + nz, nx + py + nz, nx + py + pz, // normals[1]
    px + ny + nz, px + ny + pz, nx + ny + pz, nx + ny + nz, // -normals[1]
    nx + py + pz, nx + ny + pz, px + ny + pz, px + py + pz, // normals[2]
    nx + py + nz, px + py + nz, px + ny + nz, nx + ny + nz // -normals[2]
  };

  TArrayList<vec3f>& poly_d = *(new TArrayList<vec3f>(24));
  TArrayList<vec3f>& poly_n = *(new TArrayList<vec3f>(6));
  for( int i=0; i < 6; i++ )  {
    const vec3d cnt((faces[i*4]+faces[i*4+1]+faces[i*4+2]+faces[i*4+3])/4);
    const vec3f norm = (faces[i*4+1]-faces[i*4]).XProdVec(
      faces[i*4+3]-faces[i*4]);
    if( norm.DotProd(cnt) < 0 )  {  // does normal look inside?
      poly_n[i] = -norm;
      olx_swap(faces[i*4+1], faces[i*4+3]);
    }
    else
      poly_n[i] = norm;
  }
  for( int i=0; i < 24; i++ )
    poly_d[i] = bs.center + faces[i];
  TDUserObj* uo = new TDUserObj(app.GetRenderer(), sgloQuads,
    olxstr("wbox") << obj_cnt++);
  uo->SetVertices(&poly_d);
  uo->SetNormals(&poly_n);
  app.AddObjectToCreate(uo);
  uo->SetMaterial("1029;2566914048;2574743415");
  uo->Create();
  if (print_info) {
    app.NewLogEntry() << "Please note that displayed and used atomic radii "
      "might be DIFFERENT";
  }
}
void GXLibMacros::macWBox(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  ElementRadii radii;
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  if( Cmds.Count() == 1 && TEFile::Exists(Cmds[0]) )
    radii = TXApp::ReadRadii(Cmds[0]);
  TXApp::PrintVdWRadii(radii, au.GetContentList());
  const bool use_aw = Options.Contains('w');
  if (Options.GetBoolOption('s')) {
    TLattice& latt = app.XFile().GetLattice();
    for (size_t i=0; i < latt.FragmentCount(); i++) {
      TSAtomPList satoms;
      TNetwork& f = latt.GetFragment(i);
      for (size_t j=0; j < f.NodeCount(); j++) {
        if (f.Node(j).IsDeleted()) continue;
        satoms.Add(f.Node(j));
      }
      if (satoms.Count() < 3) continue;
      TArrayList<double> all_radii(satoms.Count());
      for (size_t j=0; j < satoms.Count(); j++) {
        const size_t ri = radii.IndexOf(&satoms[j]->GetType());
        if (ri == InvalidIndex)
          all_radii[j] = satoms[j]->GetType().r_vdw;
        else
          all_radii[j] = radii.GetValue(ri);
      }
      main_CreateWBox(app, satoms, use_aw ?
        TSAtom::weight_occu_z : TSAtom::weight_occu, all_radii, false);
    }
  }
  else {
    TXAtomPList xatoms = app.FindXAtoms(Cmds, true, true);
    if (xatoms.Count() < 3) {
      E.ProcessingError(__OlxSrcInfo, "no enough atoms provided");
      return;
    }
    TArrayList<double> all_radii(xatoms.Count());
    for (size_t i=0; i < xatoms.Count(); i++) {
      const size_t ri = radii.IndexOf(&xatoms[i]->GetType());
      if (ri == InvalidIndex)
        all_radii[i] = xatoms[i]->GetType().r_vdw;
      else
        all_radii[i] = radii.GetValue(ri);
    }
    main_CreateWBox(app, TSAtomPList(xatoms, StaticCastAccessor<TSAtom>()),
      use_aw ? TSAtom::weight_occu_z : TSAtom::weight_occu, all_radii, true);
  }
}
//..............................................................................
void GXLibMacros::macCenter(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (Cmds.Count() == 3 && olx_list_and(Cmds, &olxstr::IsNumber)) {
    app.GetRenderer().GetBasis().SetCenter(
     -vec3d(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble()));
  }
  else  {
    if (Options.GetBoolOption('z')) {
      vec3d miv(100,100,100), mav(-100,-100,-100), miv_, mav_;
      for (size_t i=0; i < app.GetRenderer().ObjectCount(); i++) {
        AGDrawObject &o = app.GetRenderer().GetObject(i);
        if (!o.IsVisible()) continue;
        if (o.GetDimensions(mav_, miv_)) {
          vec3d::UpdateMinMax(miv_, miv, mav);
          vec3d::UpdateMinMax(mav_, miv, mav);
        }
      }
      double sd = olx_max(mav.DistanceTo(miv), 1.0);
      app.GetRenderer().GetBasis().SetZoom(app.GetExtraZoom()/sd);
    }
    TXAtomPList atoms = app.FindXAtoms(Cmds, true, true);
    vec3d center;
    double sum = 0;
    for (size_t i=0; i < atoms.Count(); i++) {
      center += atoms[i]->crd()*atoms[i]->CAtom().GetChemOccu();
      sum += atoms[i]->CAtom().GetChemOccu();
    }
    if (sum != 0) {
      center /= sum;
      app.GetRenderer().GetBasis().SetCenter(-center);
    }
  }
}
//.............................................................................
void GXLibMacros::macCalcVoid(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  ElementRadii radii;
  TAsymmUnit& au = app.XFile().GetAsymmUnit();
  if( Cmds.Count() == 1 && TEFile::Exists(Cmds[0]) )
    radii = TXApp::ReadRadii(Cmds[0]);
  TXApp::PrintVdWRadii(radii, au.GetContentList());
  TCAtomPList catoms;
  // consider the selection if any
  TGlGroup& sel = app.GetSelection();
  for (size_t i = 0; i < sel.Count(); i++) {
    if (sel[i].Is<TXAtom>()) {
      ((TXAtom&)sel[i]).CAtom().SetTag(catoms.Count());
      catoms.Add(((TXAtom&)sel[i]).CAtom());
    }
  }
  catoms.Pack(olx_alg::olx_not(ACollectionItem::IndexTagAnalyser()));
  if( catoms.IsEmpty() ) {
    TBasicApp::NewLogEntry() <<
     "Calculating for all atoms of the asymmetric unit";
  }
  else {
    TBasicApp::NewLogEntry() << "Calculating for " <<
      olx_analysis::alg::label(catoms);
  }
  olxstr_dict<olxstr> rv;
  double surfdis = rv('d', Options.FindValue('d', '0')).ToDouble();

  TBasicApp::NewLogEntry() << "Extra distance from the surface: " << surfdis;

  float resolution = Options.FindValue("r", "0.2").ToFloat();
  if( resolution < 0.005 )
    resolution = 0.005;
  rv('r', resolution);
  resolution = 1.0f/resolution;
  const vec3i dim(au.GetAxes()*resolution);
  const double mapVol = dim.Prod();
  const double vol = app.XFile().GetLattice().GetUnitCell().CalcVolume();
  const int minLevel = olx_round(pow(6*mapVol*3/(4*M_PI*vol), 1./3));
  app.XGrid().Clear();  // release the occupied memory
  TArray3D<short> map(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  map.FastInitWith(10000);
  if( Options.Contains('p') )
    app.XFile().GetUnitCell().BuildDistanceMap_Direct(map, surfdis, -1,
      radii.IsEmpty() ? NULL : &radii, catoms.IsEmpty() ? NULL : &catoms);
  else  {
    app.XFile().GetUnitCell().BuildDistanceMap_Masks(map, surfdis, -1,
      radii.IsEmpty() ? NULL : &radii, catoms.IsEmpty() ? NULL : &catoms);
  }
  size_t structureGridPoints = 0;
  TIntList levels;
  short*** amap = map.Data.data;
  short MaxLevel = 0;
  for( int i=0; i < dim[0]; i++ )  {
    for( int j=0; j < dim[1]; j++ )  {
      for( int k=0; k < dim[2]; k++ )  {
        if( amap[i][j][k] > MaxLevel )
          MaxLevel = amap[i][j][k];
        if( amap[i][j][k] < 0 )
          structureGridPoints++;
        else  {
          while( levels.Count() <= (size_t)amap[i][j][k] )
            levels.Add(0);
          levels[amap[i][j][k]]++;
        }
      }
    }
  }
  vec3d_list void_centers =
    MapUtil::FindLevelCenters<short>(map.Data, MaxLevel, -MaxLevel);
  const vec3i MaxXCh = MapUtil::AnalyseChannels1(map.Data, MaxLevel);
  for (int i = 0; i < 3; i++) {
    if (MaxXCh[i] != 0) {
      TBasicApp::NewLogEntry() << (olxstr((olxch)('a' + i)) <<
        " direction can be penetrated by a sphere of " <<
        olxstr::FormatFloat(2, MaxXCh[i] / resolution) << "A radius");
    }
  }
  TBasicApp::NewLogEntry() <<
    "Cell volume (A^3) " << olxstr::FormatFloat(3, vol);
  TBasicApp::NewLogEntry() <<
    "Radius [ volume ] of the largest spherical void is " <<
    olxstr::FormatFloat(2, (double)MaxLevel/resolution) <<
    " A [ " << olxstr::FormatFloat(2,
      olx_sphere_volume((double)MaxLevel/resolution)) << " A^3 ]";
  TBasicApp::NewLogEntry() << "The void center(s) are at (fractional):";
  for (size_t i=0; i < void_centers.Count(); i++) {
    olxstr l1;
    for (int j=0; j < 3; j++)
      l1 << ' ' << olxstr::FormatFloat(-3, void_centers[i][j]);
    TBasicApp::NewLogEntry() << l1;
  }
  TBasicApp::NewLogEntry() << (catoms.IsEmpty() ? "Structure occupies"
    : "Selected atoms occupy")
    << " (A^3) " << olxstr::FormatFloat(2, structureGridPoints*vol/mapVol)
    << " (" << olxstr::FormatFloat(2, structureGridPoints*100/mapVol) << "%)";

  double totalVol = 0;
  for( size_t i=levels.Count()-1; olx_is_valid_index(i); i-- )  {
    totalVol += levels[i];
    TBasicApp::NewLogEntry() << "Level " << i << " is " <<
      olxstr::FormatFloat(1, (double)i/resolution) << "A away from the surface "
      << olxstr::FormatFloat(3, totalVol*vol/mapVol) << "(A^3)";
  }
  if( Options.Contains('i') )  {
    for( int i=0; i < dim[0]; i++ )  {
      for( int j=0; j < dim[1]; j++ )  {
        for( int k=0; k < dim[2]; k++ )
          amap[i][j][k] = MaxLevel-amap[i][j][k];
      }
    }
  }
  //// set map to view voids
  app.XGrid().InitGrid(dim[0], dim[1], dim[2]);
  app.XGrid().SetMinVal(0);
  app.XGrid().SetMaxVal((float)MaxLevel/resolution);
  for( int i=0; i < dim[0]; i++ )  {
    for( int j=0; j < dim[1]; j++ )  {
      for( int k=0; k < dim[2]; k++ )
        app.XGrid().SetValue(i, j, k, (float)map.Data[i][j][k]/resolution);
    }
  }
  app.XGrid().AdjustMap();
  app.XGrid().InitIso();
  app.ShowGrid(true, EmptyString());
  TBasicApp::NewLogEntry();
  //E.SetRetVal(XLibMacros::NAString());
}
//.............................................................................
void GXLibMacros::macDirection(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  const mat3d Basis = app.GetRenderer().GetBasis().GetMatrix();
  const vec3d Z(Basis[0][2], Basis[1][2], Basis[2][2]);
  if( app.XFile().HasLastLoader() )  {
    TAsymmUnit &au = app.XFile().GetAsymmUnit();
    mat3d m = au.GetCellToCartesian();
    vec3d fZ = au.Fractionalise(Z).Normalise();
    double min_v = 100;
    for (int i=0; i < 3; i++) {
      if (fZ[i] != 0 && olx_abs(fZ[i]) < min_v)
        min_v = olx_abs(fZ[i]);
    }
    if (min_v >= 1e-4)
      fZ /= min_v;
    olxstr Tmp =  "Direction: (";
    Tmp << olxstr::FormatFloat(3, fZ[0]) << "*A, " <<
      olxstr::FormatFloat(3, fZ[1]) << "*B, " <<
      olxstr::FormatFloat(3, fZ[2]) << "*C)";
    TBasicApp::NewLogEntry() << Tmp;
    const char *Dir[] = {"000", "100", "010", "001", "110", "101", "011", "111"};
    TTypeList<vec3d> Points;
    Points.AddNew();
    Points.AddCopy(m[0]);
    Points.AddCopy(m[1]);
    Points.AddCopy(m[2]);
    Points.AddCopy(m[0] + m[1]);
    Points.AddCopy(m[0] + m[2]);
    Points.AddCopy(m[1] + m[2]);
    Points.AddCopy(m[0] + m[1] + m[2]);
    for( size_t i=0; i < Points.Count(); i++ )  {
      for( size_t j=i+1; j < Points.Count(); j++ )  {
        double d = (Points[j]-Points[i]).Normalise().DistanceTo(Z)/2;
        if (d < 0.05) {
          Tmp = "View along ";
          Tmp << Dir[i] <<  '-' <<  Dir[j] << ' ' << '(' <<
            "normalised deviation: " <<  olxstr::FormatFloat(3, d) << "A)";
          TBasicApp::NewLogEntry() << Tmp;
        }
      }
    }
    if( !app.XGrid().IsEmpty() && app.XGrid().IsVisible() &&
      (app.XGrid().GetRenderMode()&(planeRenderModeContour|planeRenderModePlane)) != 0 )
    {
      const vec3d center(app.GetRenderer().GetBasis().GetCenter());
      vec3d p(0, 0, app.XGrid().GetDepth());
      p = au.Fractionalise(Basis*p - center);
      olxstr Tmp =  "Grid center: (";
      Tmp << olxstr::FormatFloat(3, p[0]) << "*A, " <<
             olxstr::FormatFloat(3, p[1]) << "*B, " <<
             olxstr::FormatFloat(3, p[2]) << "*C)";
      TBasicApp::NewLogEntry() << Tmp;
    }
  }
  else  {
    olxstr Tmp =  "Normal: (";
    Tmp << olxstr::FormatFloat(3, Z[0]) << ", " <<
           olxstr::FormatFloat(3, Z[1]) << ", " <<
           olxstr::FormatFloat(3, Z[2]) << ')';
    TBasicApp::NewLogEntry() << Tmp;
  }
}
//.............................................................................
void GXLibMacros::macIndividualise(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  short level = -1;
  int32_t mask = -1;
  if (!Cmds.IsEmpty() && Cmds[0].IsNumber()) {
    level = Cmds[0].ToInt();
    Cmds.Delete(0);
  }
  if (!Cmds.IsEmpty() && Cmds[0].IsNumber()) {
    mask = Cmds[0].ToInt();
    Cmds.Delete(0);
  }
  if (Cmds.IsEmpty()) {
    TXAtomPList atoms;
    TXBondPList bonds;
    TGlGroup& sel = app.GetSelection();
    for (size_t i=0; i < sel.Count(); i++) {
      if (sel[i].Is<TXAtom>()) {
        atoms.Add((TXAtom&)sel[i]);
      }
      else if (sel[i].Is<TXBond>()) {
        bonds.Add((TXBond&)sel[i]);
      }
    }
    app.Individualise(atoms, level, mask);
    app.Individualise(bonds, level, mask);
  }
  else {
    app.Individualise(app.FindXAtoms(Cmds, false, false).GetObject(),
      level, mask);
  }
}
//..............................................................................
void GXLibMacros::macCollectivise(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (Cmds.IsEmpty()) {
    TGlGroup& glg = app.GetSelection();
    TXAtomPList atoms;
    TXBondPList bonds;
    for (size_t i=0; i < glg.Count(); i++) {
      if (glg[i].Is<TXAtom>())
        atoms.Add((TXAtom&)glg[i]);
      else if (glg[i].Is<TXBond>())
        bonds.Add((TXBond&)glg[i]);
    }
    if (atoms.IsEmpty() && bonds.IsEmpty()) {
      app.ClearIndividualCollections();
    }
    app.Collectivise(atoms);
    app.Collectivise(bonds);
  }
  else {
    app.Collectivise(app.FindXAtoms(Cmds, false, false).GetObject());
  }
}
//.............................................................................
void GXLibMacros::macLstGO(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  TStrList output;
  output.SetCapacity(app.GetRenderer().CollectionCount());
  for (size_t i=0; i < app.GetRenderer().CollectionCount(); i++) {
    TGPCollection& gpc = app.GetRenderer().GetCollection(i);
    output.Add( gpc.GetName() ) << '[';
    for (size_t j=0; j < gpc.PrimitiveCount(); j++) {
      output.GetLastString() << gpc.GetPrimitive(j).GetName();
      if ((j + 1) < gpc.PrimitiveCount()) {
        output.GetLastString() << ';';
      }
    }
    output.GetLastString() << "]->" << gpc.ObjectCount();
  }
  olxstr_dict<const olxstr_dict<olxstr> *> regs;
  regs.Add("atoms", &TXAtom::NamesRegistry());
  regs.Add("bonds", &TXBond::NamesRegistry());
  for (int ri = 0; ri < regs.Count(); ri++) {
    const olxstr_dict<olxstr> &reg = *regs.GetValue(ri);
    if (!reg.IsEmpty()) {
      output.Add("Named collections, ") << regs.GetKey(ri);
      olxstr_dict<int> unq;
      for (size_t i = 0; i < reg.Count(); i++) {
        size_t idx = unq.IndexOf(reg.GetValue(i));
        if (idx == InvalidIndex) {
          unq.Add(reg.GetValue(i), 1);
        }
        else {
          unq.GetValue(idx)++;
        }
      }
      for (size_t i = 0; i < unq.Count(); i++) {
        output.Add(unq.GetKey(i)) << ": " << unq.GetValue(i);
      }
    }
  }
  if (!TXPlane::NamesRegistry().IsEmpty()) {
    output.Add("Named collections, planes:");
    for (size_t i = 0; i < TXPlane::NamesRegistry().Count(); i++) {
      output.Add(TXPlane::NamesRegistry().GetKey(i)) << ": " <<
        TXPlane::NamesRegistry().GetValue(i);
    }
  }
  TBasicApp::NewLogEntry() << output;
}
//.............................................................................
class Esd_Tetrahedron  {
  TSAtomPList atoms;
  olxstr Name;
  TEValue<double> Volume;
  VcoVContainer& vcov;
protected:
  void CalcVolume()  {
    Volume =
      vcov.CalcTetrahedronVolume(*atoms[0], *atoms[1], *atoms[2], *atoms[3]);
  }
public:
  Esd_Tetrahedron(const olxstr& name, const VcoVContainer& _vcov)
    : Volume(-1,0), vcov(const_cast<VcoVContainer&>(_vcov))
  {
    Name = name;
  }
  void Add( TSAtom* a )  {
    atoms.Add( a );
    if( atoms.Count() == 4 )
      CalcVolume();
  }
  const olxstr& GetName() const  {  return Name;  }
  double GetVolume() const {  return Volume.GetV();  }
  double GetEsd() const {  return Volume.GetE();  }
};
int Esd_ThSort(const Esd_Tetrahedron &th1, const Esd_Tetrahedron &th2)  {
  return olx_cmp(th1.GetVolume(), th2.GetVolume());
}
olx_pair_t<size_t, size_t> Esd_FindOppositePair(TSAtomPList &atoms) {
  double ma = 0;
  size_t idx = InvalidIndex, for_ = InvalidIndex;
  for (size_t i = 1; i < atoms.Count(); i++) {
    if (atoms[i]->GetTag() != 0) continue;
    if (for_ == InvalidIndex) {
      for_ = i;
      atoms[i]->SetTag(1);
      continue;
    }
    double ang = olx_angle(
      atoms[i]->crd(), atoms[0]->crd(), atoms[for_]->crd());
    if (ang > ma) {
      idx = i;
      ma = ang;
    }
  }
  if (idx == InvalidIndex) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "could not locate the opposite atom");
  }
  atoms[idx]->SetTag(1);
  return olx_pair::make(for_-1, idx-1);
}

void GXLibMacros::macEsd(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  VcoVContainer vcovc(app.XFile().GetAsymmUnit());
  try {
    olxstr src_mat = app.InitVcoV(vcovc);
    app.NewLogEntry() << "Using " << src_mat << " matrix for the calculation";
  }
  catch (TExceptionBase& e) {
    Error.ProcessingError(__OlxSrcInfo, e.GetException()->GetError());
    return;
  }
  TStrList values;
  TGlGroup& sel = app.GetSelection();
  if (Options.Contains('l')) {
    for (size_t i = 0; i < sel.Count(); i++) {
      if (sel[i].Is<TXBond>()) {
        TXBond &xb = (TXBond&)sel[i];
        values.Add(xb.A().GetLabel()) << " to " << xb.B().GetLabel() <<
          " distance: " << vcovc.CalcDistance(xb.A(), xb.B()).ToString() <<
          " A";
      }
    }
  }
  else {
    if (sel.Count() == 1) {
      if (sel[0].Is<TXAtom>()) {
        TSAtomPList atoms;
        TXAtom& xa = (TXAtom&)sel[0];
        for (size_t i = 0; i < xa.NodeCount(); i++) {
          TSAtom& A = xa.Node(i);
          if (A.IsDeleted() || (A.GetType() == iQPeakZ)) {
            continue;
          }
          atoms.Add(A);
        }
        if (atoms.Count() == 3) {
          atoms.Add(xa);
        }
        if (atoms.Count() < 4) {
          Error.ProcessingError(__OlxSrcInfo,
            "An atom with at least four bonds is expected");
          return;
        }
        TTypeList<Esd_Tetrahedron> tetrahedra;
        // special case for 4 nodes
        if (atoms.Count() == 4) {
          Esd_Tetrahedron& th = tetrahedra.AddNew(
            olxstr(atoms[0]->GetLabel()) << '-'
            << atoms[1]->GetLabel() << '-'
            << atoms[2]->GetLabel() << '-'
            << atoms[3]->GetLabel(), vcovc);
          th.Add(atoms[0]);
          th.Add(atoms[1]);
          th.Add(atoms[2]);
          th.Add(atoms[3]);
        }
        else {
          for (size_t i = 0; i < atoms.Count(); i++) {
            for (size_t j = i + 1; j < atoms.Count(); j++) {
              for (size_t k = j + 1; k < atoms.Count(); k++) {
                Esd_Tetrahedron& th = tetrahedra.AddNew(
                  olxstr(xa.GetLabel()) << '-'
                  << atoms[i]->GetLabel() << '-'
                  << atoms[j]->GetLabel() << '-'
                  << atoms[k]->GetLabel(), vcovc);
                th.Add(&xa);
                th.Add(atoms[i]);
                th.Add(atoms[j]);
                th.Add(atoms[k]);
              }
            }
          }
        }
        const size_t thc = (atoms.Count() - 2) * 2;
        QuickSorter::SortSF(tetrahedra, &Esd_ThSort);
        bool removed = false;
        while (tetrahedra.Count() > thc) {
          TBasicApp::NewLogEntry() << "Removing tetrahedron " <<
            tetrahedra[0].GetName() << " with volume " <<
            tetrahedra[0].GetVolume();
          tetrahedra.Delete(0);
          removed = true;
        }
        double v = 0, esd = 0;
        for (size_t i = 0; i < tetrahedra.Count(); i++) {
          v += tetrahedra[i].GetVolume();
          esd += tetrahedra[i].GetEsd()*tetrahedra[i].GetEsd();
        }
        TEValue<double> ev(v, sqrt(esd));
        if (removed) {
          values.Add("The volume for remaining tetrahedra is ") <<
            ev.ToString() << " A^3";
        }
        else {
          values.Add("The tetrahedra volume is ") << ev.ToString() << " A^3";
        }
      }
      else if (sel[0].Is<TXPlane>()) {
        TSAtomCPList atoms;
        TXPlane& xp = (TXPlane&)sel[0];
        olxstr pld;
        for (size_t i = 0; i < xp.Count(); i++) {
          atoms.Add(xp.GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        values.Add("Plane ") << pld <<
          (values.Add("RMS: ") << vcovc.CalcPlane(atoms).ToString()) << " A";
        TEPoint3<double> c_cent(vcovc.CalcCentroid(atoms));
        values.Add("Plane ") << pld << "Cartesian centroid : {" <<
          c_cent[0].ToString() << ", " << c_cent[1].ToString() << ", " <<
          c_cent[2].ToString() << "}";
        TEPoint3<double> f_cent(vcovc.CalcCentroidF(atoms));
        values.Add("Plane ") << pld << "fractional centroid : {" <<
          f_cent[0].ToString() << ", " << f_cent[1].ToString() << ", " <<
          f_cent[2].ToString() << "}";
      }
      else if (sel[0].Is<TXBond>()) {
        TXBond& xb = (TXBond&)sel[0];
        values.Add(xb.A().GetLabel()) << " to " << xb.B().GetLabel() <<
          " distance: " << vcovc.CalcDistance(xb.A(), xb.B()).ToString() <<
          " A";
      }
    }
    else if (sel.Count() == 2) {
      if (olx_list_and(sel, &IOlxObject::Is<TXAtom>)) {
        values.Add(((TXAtom&)sel[0]).GetLabel()) << " to " <<
          ((TXAtom&)sel[1]).GetLabel() << " distance: " <<
          vcovc.CalcDistance((TXAtom&)sel[0], (TXAtom&)sel[1]).ToString() <<
          " A";
      }
      else if (olx_list_and(sel, &IOlxObject::Is<TXBond>)) {
        TSBond& b1 = ((TXBond&)sel[0]);
        TSBond& b2 = ((TXBond&)sel[1]);
        TEValue<double> v(vcovc.CalcB2BAngle(b1.A(), b1.B(), b2.A(), b2.B())),
          v1(180 - v.GetV(), v.GetE());
        values.Add(b1.A().GetLabel()) << '-' << b1.B().GetLabel() << " to " <<
          b2.A().GetLabel() << '-' << b2.B().GetLabel() << " angle: " <<
          v.ToString() << '(' << v1.ToString() << ')';
      }
      else if ((sel[0].Is<TXBond>() && sel[1].Is<TXAtom>()) ||
        (sel[1].Is<TXBond>() && sel[0].Is<TXAtom>()))
      {
        const TXBond &b = (TXBond&)(sel[0].Is<TXBond>() ? sel[0] : sel[1]);
        const TXAtom &a = (TXAtom&)(sel[0].Is<TXBond>() ? sel[1] : sel[0]);
        values.Add(a.GetLabel()) << " to " <<
          b.A().GetLabel() << '-' << b.B().GetLabel() << " distance: " <<
          vcovc.CalcAtomToVectorDistance(a, b.A(), b.B()).ToString();
      }
      else if ((sel[0].Is<TXAtom>() && sel[1].Is<TXPlane>()) ||
        (sel[1].Is<TXAtom>() && sel[0].Is<TXPlane>()))
      {
        TSAtomCPList atoms;
        TXPlane& xp = (TXPlane&)sel[sel[0].Is<TXPlane>() ? 0 : 1];
        olxstr pld;
        for (size_t i = 0; i < xp.Count(); i++) {
          atoms.Add(xp.GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        TSAtom& sa = ((TXAtom&)sel[sel[0].Is<TXAtom>() ? 0 : 1]);
        values.Add(sa.GetLabel()) << " to plane " << pld << "distance: " <<
          vcovc.CalcP2ADistance(atoms, sa).ToString() << " A";
        values.Add(sa.GetLabel()) << " to plane " << pld <<
          "centroid distance: " <<
          vcovc.CalcPC2ADistance(atoms, sa).ToString() << " A";
      }
      else if ((sel[0].Is<TXBond>() && sel[1].Is<TXPlane>()) ||
        (sel[1].Is<TXBond>() && sel[0].Is<TXPlane>()))
      {
        TSAtomCPList atoms;
        TXPlane& xp = (TXPlane&)sel[sel[0].Is<TXPlane>() ? 0 : 1];
        olxstr_buf pld;
        for (size_t i = 0; i < xp.Count(); i++) {
          atoms.Add(xp.GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        TSBond& sb = ((TXBond&)sel[sel[0].Is<TXBond>() ? 0 : 1]);
        TEValue<double> v(vcovc.CalcP2VAngle(atoms, xp.GetNormal(), sb.A(), sb.B()));
        values.Add(sb.A().GetLabel()) << '-' << sb.B().GetLabel() <<
          " to plane normal " << olxstr(pld) << "angle: " << v.ToString();
      }
      else if (olx_list_and(sel, &IOlxObject::Is<TXPlane>)) {
        TSAtomCPList p1, p2;
        TXPlane& xp1 = (TXPlane&)sel[0];
        TXPlane& xp2 = (TXPlane&)sel[1];
        olxstr pld1, pld2;
        for (size_t i = 0; i < xp1.Count(); i++) {
          p1.Add(xp1.GetAtom(i));
          pld1 << p1.GetLast()->GetLabel() << ' ';
        }
        for (size_t i = 0; i < xp2.Count(); i++) {
          p2.Add(xp2.GetAtom(i));
          pld2 << p2.GetLast()->GetLabel() << ' ';
        }
        const TEValue<double> angle = vcovc.CalcP2PAngle(p1, xp1.GetNormal(),
          p2, xp2.GetNormal());
        values.Add("Plane ") << pld1 << "normal to plane normal angle: " <<
          angle.ToString();
        values.Add("Plane centroid to plane centroid distance: ") <<
          vcovc.CalcPC2PCDistance(p1, p2).ToString() << " A";
        values.Add("Plane [") << pld1 << "] to plane centroid distance: " <<
          vcovc.CalcP2PCDistance(p1, p2).ToString() << " A";
        values.Add("Plane [") << pld1 << "] to plane shift: " <<
          vcovc.CalcP2PShiftDistance(p1, p2).ToString() << " A";
        if (olx_abs(angle.GetV()) > 1e-6) {
          values.Add("Plane [") << pld2 << "] to plane centroid distance: " <<
            vcovc.CalcP2PCDistance(p2, p1).ToString() << " A";
          values.Add("Plane [") << pld2 << "] to plane shift: " <<
            vcovc.CalcP2PShiftDistance(p2, p1).ToString() << " A";
        }
        try {
          values.Add("Plane ") << pld1 << "to plane twist angle: " <<
            vcovc.CalcP2PTAngle(p1, xp1.GetNormal(),
              p2, xp2.GetNormal()).ToString();
        }
        catch (const TExceptionBase &) {
          values.Add("Twist angle is undefined");
        }
        try {
          values.Add("Plane ") << pld1 << "to plane fold angle: " <<
            vcovc.CalcP2PFAngle(p1, xp1.GetNormal(),
              p2, xp2.GetNormal()).ToString();
        }
        catch (const TExceptionBase &) {
          values.Add("Fold angle is undefined");
        }
        if (xp1.Count() == xp2.Count() && xp1.Count() == 3) {
          TSAtomPList atoms(6), sorted_atoms;
          for (size_t i = 0; i < 3; i++) {
            (atoms[i] = &xp1.GetAtom(i))->SetTag(0);
            (atoms[i + 3] = &xp2.GetAtom(i))->SetTag(1);
          }
          olx_pdict<index_t, vec3d> transforms;
          transforms.Add(1, -xp2.GetCenter());
          transforms.Add(0, -xp1.GetCenter());
          PlaneSort::Sorter::DoSort(atoms, transforms, vec3d(),
            xp2.GetNormal(), sorted_atoms);
          vec3d_alist pts;
          TEValueD v = vcovc.CalcTraingluarTwist(sorted_atoms);
          if (v.GetV() > 60) {
            v.V() = 120 - v.GetV();
          }
          values.Add("Mean triangle twist angle: ") << v.ToString();
        }
      }
    }
    else if (sel.Count() == 3) {
      if (olx_list_and(sel, &IOlxObject::Is<TXAtom>)) {
        TSAtom& a1 = (TXAtom&)sel[0];
        TSAtom& a2 = (TXAtom&)sel[1];
        TSAtom& a3 = (TXAtom&)sel[2];
        values.Add(a1.GetLabel()) << '-' << a2.GetLabel() << '-' << a3.GetLabel()
          << " angle (numerical): " << vcovc.CalcAngle(a1, a2, a3).ToString();
        values.Add(a1.GetLabel()) << '-' << a2.GetLabel() << '-' << a3.GetLabel()
          << " angle (analytical): " << vcovc.CalcAngleA(a1, a2, a3).ToString();

        values.Add(a1.GetLabel()) << " to " <<
          a2.GetLabel() << '-' << a3.GetLabel() << " distance: " <<
          vcovc.CalcAtomToVectorDistance(a1, a2, a3).ToString();
      }
      else if (
        (sel[0].Is<TXPlane>() && sel[1].Is<TXAtom>() &&
          sel[2].Is<TXAtom>()) ||
        (sel[1].Is<TXPlane>() && sel[0].Is<TXAtom>() &&
          sel[2].Is<TXAtom>()) ||
        (sel[2].Is<TXPlane>() && sel[1].Is<TXAtom>() &&
          sel[0].Is<TXAtom>()))
      {
        TSAtom* a1 = NULL, *a2 = NULL;
        TXPlane* xp = NULL;
        TSAtomCPList atoms;
        for (size_t i = 0; i < 3; i++) {
          if (sel[i].Is<TXPlane>())
            xp = &(TXPlane&)sel[i];
          else {
            if (a1 == 0) {
              a1 = &((TXAtom&)sel[i]);
            }
            else {
              a2 = &((TXAtom&)sel[i]);
            }
          }
        }
        olxstr pld;
        for (size_t i = 0; i < xp->Count(); i++) {
          atoms.Add(xp->GetAtom(i));
          pld << atoms.GetLast()->GetLabel() << ' ';
        }
        values.Add(a1->GetLabel()) << '-' << a2->GetLabel() << " to plane normal " <<
          pld << "angle: " << vcovc.CalcP2VAngle(atoms, xp->GetNormal(),
            *a1, *a2).ToString();
        if (sel[0].Is<TXPlane>() || sel[2].Is<TXPlane>()) {
          values.Add("Plane centroid-") << a1->GetLabel() << '-' << a2->GetLabel()
            << " angle: " << vcovc.CalcPCAngle(atoms, *a1, *a2).ToString();
        }
        else {
          values.Add(a1->GetLabel()) << "-plane centroid -" << a2->GetLabel()
            << " angle: " << vcovc.CalcPCAngle(*a1, atoms, *a2).ToString();
        }
      }
      else if (olx_list_and(sel, &IOlxObject::Is<TXPlane>)) {
        TSPlane& p1 = (TXPlane&)sel[0];
        TSPlane& p2 = (TXPlane&)sel[1];
        TSPlane& p3 = (TXPlane&)sel[2];
        TSAtomCPList a1, a2, a3;
        for (size_t i = 0; i < p1.Count(); i++) {
          a1.Add(p1.GetAtom(i));
        }
        for (size_t i = 0; i < p2.Count(); i++) {
          a2.Add(p2.GetAtom(i));
        }
        for (size_t i = 0; i < p3.Count(); i++) {
          a3.Add(p3.GetAtom(i));
        }
        values.Add("Angle between plane centroids: ") <<
          vcovc.Calc3PCAngle(a1, a2, a3).ToString();
      }
      else if (sel[0].Is<TXPlane>() &&
        sel[1].Is<TXAtom>() &&
        sel[2].Is<TXPlane>())
      {
        TSPlane& p1 = (TXPlane&)sel[0];
        TSAtom& a = (TXAtom&)sel[1];
        TSPlane& p2 = (TXPlane&)sel[2];
        TSAtomCPList a1, a2;
        for (size_t i = 0; i < p1.Count(); i++)  a1.Add(p1.GetAtom(i));
        for (size_t i = 0; i < p2.Count(); i++)  a2.Add(p2.GetAtom(i));
        values.Add("Angle between plane centroid - atom - plane centroid: ") <<
          vcovc.CalcPCAPCAngle(a1, a, a2).ToString();
      }
    }
    else if (sel.Count() == 4 &&
      olx_list_and_st(sel, &olx_is<TXAtom, TGlGroup::list_item_type>))
    {
      TSAtomPList a(sel, DynamicCastAccessor<TSAtom>());
      olxstr lbl = olx_analysis::alg::label(a, true, '-');
      values.Add(lbl) << " torsion angle (numerical): " <<
        vcovc.CalcTAngle(*a[0], *a[1], *a[2], *a[3]).ToString();
      values.Add(lbl) << " torsion angle (analytical): " <<
        vcovc.CalcTAngleA(*a[0], *a[1], *a[2], *a[3]).ToString();
      values.Add(lbl) << " tetrahedron volume: " <<
        vcovc.CalcTetrahedronVolume(*a[0], *a[1], *a[2], *a[3]).ToString() <<
        " A^3";
    }
    else if (sel.Count() == 7 &&
      olx_list_and_st(sel, &olx_is<TXAtom, TGlGroup::list_item_type>))
    {
      TSAtomPList atoms(sel, DynamicCastAccessor<TSAtom>());
      values.Add("Octahedral distortion is (for the selection): ")
        << vcovc.CalcOHDistortionBP(TSAtomCPList(atoms)).ToString();
      atoms.ForEach(ACollectionItem::TagSetter(0));
      olx_pair_t<size_t, size_t> opposites[3] = {
        Esd_FindOppositePair(atoms),
        Esd_FindOppositePair(atoms),
        Esd_FindOppositePair(atoms)
      };
      TSAtom* central_atom = atoms[0];
      atoms.Delete(0);
      size_t faces[8][6] = {
        { opposites[0].a, opposites[1].a, opposites[2].a,
          opposites[0].b, opposites[1].b, opposites[2].b },
        { opposites[0].a, opposites[1].a, opposites[2].b,
          opposites[0].b, opposites[1].b, opposites[2].a },
        { opposites[0].a, opposites[1].b, opposites[2].a,
          opposites[0].b, opposites[1].a, opposites[2].b },
        { opposites[0].a, opposites[1].b, opposites[2].b,
          opposites[0].b, opposites[1].a, opposites[2].a },
      };
      double total_val_bp = 0, total_esd_bp = 0;
      for (size_t i = 0; i < 4; i++) {
        TSAtomPList sorted_atoms;
        olx_pdict<index_t, vec3d> transforms;
        vec3d face_center, face1_center;
        for (size_t j = 0; j < 3; j++) {
          atoms[faces[i][j]]->SetTag(1);
          face_center += atoms[faces[i][j]]->crd();
          atoms[faces[i][j + 3]]->SetTag(0);
          face1_center += atoms[faces[i][j + 3]]->crd();
        }
        face_center /= 3;
        face1_center /= 3;
        const vec3d normal = vec3d::Normal(atoms[faces[i][0]]->crd(),
          atoms[faces[i][1]]->crd(), atoms[faces[i][2]]->crd());
        transforms.Add(1, central_atom->crd() - face_center);
        transforms.Add(0, central_atom->crd() - face1_center);
        PlaneSort::Sorter::DoSort(atoms, transforms, central_atom->crd(),
          normal, sorted_atoms);
        if (sorted_atoms[0]->GetTag() != 1) {
          sorted_atoms.ShiftR(1);
        }
        values.Add("Face ") << (i + 1) << ": " <<
          olx_analysis::alg::label(sorted_atoms, true, ' ') << ' ';
        sorted_atoms.Insert(0, central_atom);
        TEValue<double> rv = vcovc.CalcOHDistortionBP(
          TSAtomCPList(sorted_atoms));
        total_val_bp += rv.GetV() * 3;
        total_esd_bp += olx_sqr(3 * rv.GetE());
        values.GetLastString() << rv.ToString();
      }
      values.Add("Combined distortion: ") <<
        TEValue<double>(2 * total_val_bp, 2 * sqrt(total_esd_bp)).ToString() <<
        ", mean: " <<
        TEValue<double>(total_val_bp / 12, sqrt(total_esd_bp) / 12).ToString()
        << " degrees";
      olxstr_dict<TEValue<double> > od_c = vcovc.CalcOctahedralDistortion(
        TSAtomCPList() << central_atom << atoms);
      for (size_t ci = 0; ci < od_c.Count(); ci++) {
        values.Add(od_c.GetKey(ci)) << ": " << od_c.GetValue(ci).ToString();
      }
    }
  }
  TBasicApp::NewLogEntry() << values;
  if (Options.Contains('c')) {
    app.ToClipboard(values);
  }
  if (Options.Contains("label")) {
    vec3d cent;
    size_t cnt = 0;
    TGlGroup& gl = app.GetSelection();
    for (size_t i = 0; i < gl.Count(); i++) {
      if (gl[i].Is<TXAtom>()) {
        cent += ((TXAtom&)gl[i]).crd();
        cnt++;
      }
      else if (gl[i].Is<TXBond>()) {
        cent += ((TXBond&)gl[i]).GetCenter();
        cnt++;
      }
      else if (gl[i].Is<TXPlane>()) {
        cent += ((TXPlane&)gl[i]).GetCenter();
        cnt++;
      }
    }
    if (cnt != 0) {
      cent /= cnt;
    }
    app.CreateLabel(cent, values.Text('\n'), 4);
  }
}
//.............................................................................
void GXLibMacros::macChemDraw(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  if (Options.GetBoolOption('u')) {
    olex2::IOlex2Processor::GetInstance()->processMacro("clear groups");
    return;
  }
  app.CreateRings(true, true);
  TGXApp::BondIterator bi = app.GetBonds();
  sorted::PointerPointer<TGPCollection> cols;
  TPtrList<TXBond> changed;
  while (bi.HasNext()) {
    TXBond &b = bi.Next();
    if (!b.IsVisible() ||
      (b.A().CAtom().IsRingAtom() && b.B().CAtom().IsRingAtom()))
    {
      continue;
    }
    short order = TSBond::PercieveOrder(b.A().GetType(),
      b.B().GetType(), b.Length());
    if (order == 0) {
      order = 1;
    }
    b.SetOrder(order);
    if (order > 1) {
      changed << b;
    }
    else if (b.B().GetType() == iCarbonZ &&
      (b.A().GetType() == iCarbonZ || b.A().GetType() == iNitrogenZ)) {
      double l = b.Length();
      if (l < 1.2) {
        order = 3;
      }
      else if (l < 1.30) {
        order = 2;
      }
    }
    b.SetOrder(order);
    if (order > 1) {
      changed << b;
    }
  }
  for (size_t i = 0; i < changed.Count(); i++) {
    changed[i]->GetPrimitives().RemoveObject(*changed[i]);
    changed[i]->Create(TXBond::GetLegend(*changed[i], 0));
    if (cols.Contains(&changed[i]->GetPrimitives())) {
      continue;
    }
    cols.AddUnique(&changed[i]->GetPrimitives());
    short order = changed[i]->GetOrder();
    if (order == 2) {
      changed[i]->UpdatePrimitives((1 << 14) | (1 << 15));
    }
    else if (order == 3) {
      changed[i]->UpdatePrimitives((1 << 16) | (1 << 17));
    }
  }
}
//.............................................................................
void GXLibMacros::macPoly(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  short pt = 0;
  const olxstr &str_t = Cmds.GetLastString();
  if (str_t.Equalsi("none")) {
    pt = 0;
  }
  else if (str_t.Equalsi("auto")) {
    pt = polyAuto;
  }
  else if (str_t.Equalsi("regular")) {
    pt = polyRegular;
  }
  else if (str_t.Equalsi("pyramid")) {
    pt = polyPyramid;
  }
  else if (str_t.Equalsi("bipyramid")) {
    pt = polyBipyramid;
  }
  else {
    E.ProcessingError(__OlxSrcInfo, "Undefined poly type: ").quote() << str_t;
    return;
  }
  Cmds.Delete(Cmds.Count()-1);
  TXAtomPList atoms = app.FindXAtoms(Cmds, true, true);
  atoms.ForEach(ACollectionItem::IndexTagSetter(
    FunctionAccessor::MakeConst(&TXAtom::GetPrimitives)));
  atoms.Pack(olx_alg::olx_not(ACollectionItem::IndexTagAnalyser(
    FunctionAccessor::MakeConst(&TXAtom::GetPrimitives))));
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i]->SetPolyhedronType(pt);
  }
}
//.............................................................................
void GXLibMacros::funExtraZoom(const TStrObjList& Params, TMacroData &E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(app.GetExtraZoom());
  }
  else {
    app.SetExtraZoom(Params[0].ToDouble());
  }
}
//.............................................................................
void GXLibMacros::macKill(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (TModeRegistry::GetInstance().GetCurrent() != NULL) {
    Error.ProcessingError(__OlxSrcInfo, "Kill inaccessible from within a mode");
    return;
  }
  if (Cmds.IsEmpty() || (Cmds.Count() == 1 && Cmds[0].Equalsi("sel"))) {
      AGDObjList Objects;
    TGlGroup& sel = app.GetSelection();
    olxstr_buf out;
    bool group_deletion = false;
    for (size_t i=0; i < sel.Count(); i++) {
      if (sel[i].Is<TXAtom>()) {
        out << ((TXAtom&)sel[i]).GetLabel();
      }
      if (sel[i].Is<TGlGroup>()) {
        if (!group_deletion) {
          group_deletion = true;
          TBasicApp::NewLogEntry() << "Please use 'ungroup' to delete groups";
        }
        continue;
      }
      else {
        out << sel[i].GetPrimitives().GetName();
      }
      out << ' ';
      Objects.Add(sel[i]);
    }
    if (!out.IsEmpty()) {
      TBasicApp::NewLogEntry() << "Deleting " << out;
      app.GetUndo().Push(app.DeleteXObjects(Objects));
      sel.Clear();
    }
  }
  else if (Cmds.Count() == 1 && Cmds[0].Equalsi("labels")) {
    TBasicApp::NewLogEntry() << "Deleting labels";
    for (size_t i = 0; i < app.LabelCount(); i++) {
      app.GetLabel(i).SetVisible(false);
    }
    TGXApp::AtomIterator ai = app.GetAtoms();
    while (ai.HasNext()) {
      ai.Next().GetGlLabel().SetVisible(false);
    }
    TGXApp::BondIterator bi = app.GetBonds();
    while (bi.HasNext()) {
      bi.Next().GetGlLabel().SetVisible(false);
    }
  }
  else {
    if (Options.GetBoolOption("au")) {
      TCAtomPList atoms = app.FindCAtoms(Cmds.Text(' '), false);
      for (size_t i = 0; i < atoms.Count(); i++) {
        atoms[i]->SetDeleted(true);
      }
    }
    else {
      TXAtomPList Atoms = app.FindXAtoms(Cmds.Text(' '), true, false),
        Selected;
      if (Atoms.IsEmpty() && Cmds.Count() == 1) {
        TGPCollection *col = app.GetRenderer().FindCollection(Cmds[0]);
        if (col != 0) {
          for (size_t i = 0; i < col->ObjectCount(); i++) {
            col->GetObject(i).SetVisible(false);
          }
        }
        return;
      }
      olx_list_filter::Filter(Atoms, Selected,
        AGDrawObject::FlagsAnalyser(sgdoSelected));
      TXAtomPList& todel = Selected.IsEmpty() ? Atoms : Selected;
      TLattice &latt = app.XFile().GetLattice();
      for (size_t i=0; i < todel.Count(); i++) {
        if (((TSAtom*)todel[i])->GetParent() != latt) {
          todel[i] = 0;
        }
      }
      todel.Pack();
      if (todel.IsEmpty()) {
        return;
      }
      if (!Options.GetBoolOption("au")) {
        olxstr_buf log = olxstr("Deleting");
        for (size_t i = 0; i < todel.Count(); i++) {
          log << ' ' << todel[i]->GetLabel();
        }
        app.NewLogEntry() << log;
      }
      app.GetUndo().Push(app.DeleteXAtoms(todel));
    }
  }
}
//.............................................................................
void GXLibMacros::macInv(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXPlanePList planes = app.GetSelection().Extract<TXPlane>();
  if (!planes.IsEmpty()) {
    for (size_t i = 0; i < planes.Count(); i++) {
      planes[i]->Invert();
    }
    return;
  }
  if (InvertFittedFragment()) {
    return;
  }
  Error.SetUnhandled(true);
  return;
}
//.............................................................................
TNetwork::AlignInfo GXLibMacros_MatchAtomPairsQT(
  const TTypeList<olx_pair_t<TSAtom*,TSAtom*> >& atoms,
  bool TryInversion, double (*weight_calculator)(const TSAtom&),
  bool print = true)
{
  TNetwork::AlignInfo rv = TNetwork::GetAlignmentRMSD(atoms, TryInversion,
    weight_calculator);
  if (print) {
    const size_t cnt = olx_min(3, atoms.Count());
    olxstr f1 = '{', f2 = '{';
    for (size_t i=0; i < cnt; i++) {
      f1 << atoms[i].GetA()->GetLabel() << ',';
      f2 << atoms[i].GetB()->GetLabel() << ',';
    }
    TBasicApp::NewLogEntry() << "Alignment RMSD " << f2 << "...} to " << f1 <<
      "...} " << (TryInversion ? "with" : "without") << " inversion) is " <<
      olxstr::FormatFloat(3, rv.rmsd.GetV()) << " A";
  }
  return rv;
}
//..............................................................................
TNetwork::AlignInfo GXLibMacros_MatchAtomPairsQTEsd(
  const TTypeList< olx_pair_t<TSAtom*,TSAtom*> >& atoms,
  bool TryInversion, double (*weight_calculator)(const TSAtom&))
{
  TXApp& xapp = TXApp::GetInstance();
  VcoVContainer vcovc(xapp.XFile().GetAsymmUnit());
  xapp.NewLogEntry() << "Using " << xapp.InitVcoV(vcovc) <<
    " matrix for the calculation";
  TSAtomPList atoms_out;
  vec3d_alist crds_out;
  TDoubleList wghts_out;
  TNetwork::PrepareESDCalc(atoms, TryInversion, atoms_out, crds_out, wghts_out,
    weight_calculator);
  TEValue<double> rv = vcovc.CalcAlignmentRMSD(
    TSAtomCPList(atoms_out), crds_out, wghts_out);
  TBasicApp::NewLogEntry() << (olxstr("RMSD is ") << rv.ToString() << " A");
  return TNetwork::GetAlignmentRMSD(atoms, TryInversion, weight_calculator);
}
//..............................................................................
void GXLibMacros_CallMatchCallbacks(TNetwork& netA, TNetwork& netB, double RMS)  {
  static const olxstr OnMatchCBName("onmatch");
  olxstr arg;
  TStrList callBackArg;
  for( size_t i=0; i < netA.NodeCount(); i++ )  {
    arg << netA.Node(i).GetLabel();
    if( (i+1) < netA.NodeCount() )  arg << ',';
  }
  callBackArg.Add(RMS);
  callBackArg.Add(arg);
  arg.SetLength(0);
  for( size_t i=0; i < netB.NodeCount(); i++ )  {
    arg << netB.Node(i).GetLabel();
    if( (i+1) < netB.NodeCount() )  arg << ',';
  }
  callBackArg.Add(arg);
  olex2::IOlex2Processor::GetInstance()->callCallbackFunc(
    OnMatchCBName, callBackArg);
}
//..............................................................................
void GXLibMacros::macMatch(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  static const olxstr StartMatchCBName("startmatch");
  TActionQueueLock __queuelock(app.FindActionQueue(olxappevent_GL_DRAW));
  // restore if already applied
  TLattice& latt = app.XFile().GetLattice();
  // note that this may be different to the overlayed file!!
  const TAsymmUnit &au = latt.GetAsymmUnit();
  for (size_t i = 0; i < app.XFiles().Count(); i++) {
    app.XFile(i).GetLattice().RestoreADPs();
  }
  if (app.XFiles().Count() > 1) {
    app.AlignXFiles();
    app.CenterView(true);
  }
  else {
    app.CenterView();
  }
  app.UpdateBonds();
  if (Cmds.Count() == 1 && Cmds[0].Equalsi("cell")) {
    if (app.XFiles().Count() < 2) {
      E.ProcessingError(__OlxSrcInfo, "an overlayed file is expected");
      return;
    }
    mat3d m1 = app.XFile().GetAsymmUnit().GetCellToCartesian();
    mat3d m2 = app.XFile(1).GetAsymmUnit().GetCellToCartesian();
    vec3d_alist points(16);
    points[1] = m1[0]; //a
    points[2] = m1[1]; //b
    points[3] = m1[2]; //c
    points[4] = points[1] + points[2]; //a+b
    points[5] = points[1] + points[3]; //a+c
    points[6] = points[2] + points[3]; //b+c
    points[7] = points[1] + points[2] + points[3]; //a+b+c
    points[9] = m2[0]; //a
    points[10] = m2[1]; //b
    points[11] = m2[2]; //c
    points[12] = points[9] + points[10]; //a+b
    points[13] = points[9] + points[11]; //a+c
    points[14] = points[10] + points[11]; //b+c
    points[15] = points[9] + points[10] + points[11]; //a+b+c

    smatdd tm;
    align::out ao = align::FindAlignmentQuaternions(
      align::ListToPairAdaptor::Make(points));
    QuaternionToMatrix(ao.quaternions[0], tm.r);
    tm.t = ao.center_a - tm.r * ao.center_b;
    TLattice &latt = app.XFile(1).GetLattice();
    {
      olxstr cname = "DUnitCell1";
      TDUnitCell* duc = app.XFile(1).DUnitCell;
      TGPCollection &gpc = app.GetRenderer().FindOrCreateCollection(cname);
      TGlMaterial dm("85;2147483392;4286611584;41975936;32");
      gpc.GetStyle().SetMaterial("Sphere", dm);
      gpc.GetStyle().SetMaterial("Cylinder", dm);
      gpc.GetStyle().SetMaterial("Lines", dm);
      duc->Init(latt.GetAsymmUnit());
      duc->Create(cname);
      for (size_t i = 0; i < duc->EdgeCount(); i++) {
        duc->GetEdge(i) = tm*duc->GetEdge(i);
      }
      duc->Update();
      duc->SetVisible(true);
    }
    for (size_t i = 0; i < latt.GetObjects().atoms.Count(); i++) {
      TSAtom &a = latt.GetObjects().atoms[i];
      a.crd() = tm*latt.GetAsymmUnit().Orthogonalise(a.ccrd());
    }
    for (size_t i = 0; i < latt.GetObjects().bonds.Count(); i++) {
      dynamic_cast<TXBond &>(latt.GetObjects().bonds[i]).Update();
    }
    //v = tm*(v-tr);
    return;
  }
  const bool exclude_h = Options.GetBoolOption('h');
  if (Options.GetBoolOption('u')) { // do nothing...
    return;
  }
  olex2::IOlex2Processor::GetInstance()->callCallbackFunc(
    StartMatchCBName, TStrList() << EmptyString());
  const bool TryInvert = Options.GetBoolOption('i');
  double(*weight_calculator)(const TSAtom&) = &TSAtom::weight_unit;

  if (Options.Contains('w')) {
    olxstr w = Options.FindValue('w', "zo").ToLowerCase();
    if (w == 'o') {
      weight_calculator = &TSAtom::weight_occu;
    }
    else if (w == "zo") {
      weight_calculator = &TSAtom::weight_occu_z;
    }
    else if (w == 'z') {
      weight_calculator = &TSAtom::weight_z;
    }
    else if (w == "em") {
      weight_calculator = &TSAtom::weight_element_mass;
    }
    else if (w == "am") {
      weight_calculator = &TSAtom::weight_atom_mass;
    }
  }
  const bool subgraph = Options.GetBoolOption('s');
  olxstr suffix = Options.FindValue('n');
  const bool name = Options.Contains('n');
  const bool align = Options.GetBoolOption('a');
  size_t group_cnt = 2;
  if (!Cmds.IsEmpty() && Cmds[0].IsNumber()) {
    group_cnt = Cmds[0].ToSizeT();
    Cmds.Delete(0);
  }
  TXAtomPList atoms = app.FindXAtoms(Cmds, false, true);
  size_t match_cnt = 0;
  if (!atoms.IsEmpty()) {
    if (atoms.Count() == 2 &&
      (&atoms[0]->GetNetwork() != &atoms[1]->GetNetwork()))
    {
      TTypeList<olx_pair_t<size_t, size_t> > res;
      TSizeList sk;
      TNetwork &netA = atoms[0]->GetNetwork(),
        &netB = atoms[1]->GetNetwork();
      bool match = subgraph ? netA.IsSubgraphOf(netB, res, sk) :
        netA.DoMatch(netB, res, TryInvert, weight_calculator);
      TBasicApp::NewLogEntry() << "Graphs match: " << match;
      if (match) {
        match_cnt++;
        // restore the other unit cell, if any...
        if (&latt != &netA.GetLattice() || &latt != &netB.GetLattice()) {
          TLattice& latt1 = (&latt == &netA.GetLattice()) ? netB.GetLattice()
            : netA.GetLattice();
          latt1.RestoreADPs();
        }
        TTypeList< olx_pair_t<TSAtom*, TSAtom*> > satomp;
        TSAtomPList atomsToTransform;
        for (size_t i = 0; i < res.Count(); i++) {
          if (!atomsToTransform.Contains(&netB.Node(res[i].GetB()))) {
            atomsToTransform.Add(netB.Node(res[i].GetB()));
            if (exclude_h && netB.Node(res[i].GetB()).GetType().z == 1) {
              continue;
            }
            satomp.AddNew<TSAtom*, TSAtom*>(&netA.Node(res[i].GetA()),
              &netB.Node(res[i].GetB()));
          }
        }
        if (name) {
          // change CXXX to CSuffix+whatever left of XXX
          if (suffix.Length() > 1 && suffix.CharAt(0) == '$') {
            olxstr new_l;
            const olxstr l_val = suffix.SubStringFrom(1);
            for (size_t i = 0; i < res.Count(); i++) {
              const olxstr& old_l = netA.Node(res[i].GetA()).GetLabel();
              const cm_Element& elm = netA.Node(res[i].GetA()).GetType();
              const index_t l_d = old_l.Length() - elm.symbol.Length();
              if (l_d <= (index_t)l_val.Length())
                new_l = elm.symbol + l_val;
              else if (l_d > (index_t)l_val.Length()) {
                new_l = olxstr(elm.symbol) << l_val <<
                  old_l.SubStringFrom(elm.symbol.Length() + l_val.Length());
              }
              netB.Node(res[i].GetB()).CAtom().SetLabel(new_l, false);
            }
          }
          // change the ending
          else if (suffix.Length() > 1 && suffix.CharAt(0) == '-') {
            olxstr new_l;
            const olxstr l_val = suffix.SubStringFrom(1);
            for (size_t i = 0; i < res.Count(); i++) {
              const olxstr& old_l = netA.Node(res[i].GetA()).GetLabel();
              const cm_Element& elm = netA.Node(res[i].GetA()).GetType();
              const index_t l_d = old_l.Length() - elm.symbol.Length();
              if (l_d <= (index_t)l_val.Length()) {
                new_l = elm.symbol + l_val;
              }
              else if (l_d > (index_t)l_val.Length()) {
                new_l = old_l.SubStringTo(old_l.Length() - l_val.Length()) << l_val;
              }
              netB.Node(res[i].GetB()).CAtom().SetLabel(new_l, false);
            }
          }
          else {
            for (size_t i = 0; i < res.Count(); i++) {
              netB.Node(res[i].GetB()).CAtom().SetLabel(
                netA.Node(res[i].GetA()).GetLabel() + suffix, false);
            }
          }
        }
        TNetwork::AlignInfo align_info;
        if (Options.GetBoolOption("esd")) {
          align_info = GXLibMacros_MatchAtomPairsQTEsd(
            satomp, TryInvert, weight_calculator);
        }
        else {
          align_info = GXLibMacros_MatchAtomPairsQT(
            satomp, TryInvert, weight_calculator);
        }
        // execute callback function
        GXLibMacros_CallMatchCallbacks(netA, netB, align_info.rmsd.GetV());
        // ends execute callback
        { // print pairs
          mat3d m;
          QuaternionToMatrix(align_info.align_out.quaternions[0], m);
          vec3d center = align_info.align_out.center_b;
          if (align_info.inverted) {
            m *= -1;
            center *= -1;
          }
          vec3d total_t = align_info.align_out.center_a - center*m;
          sorted::PrimitiveAssociation<double, size_t> sorted_pairs;
          for (size_t i = 0; i < res.Count(); i++) {
            vec3d v = netB.GetLattice().GetAsymmUnit()
              .Orthogonalise(netB.Node(res[i].GetB()).ccrd());
            v = v*m + total_t;
            v -= netA.GetLattice().GetAsymmUnit()
              .Orthogonalise(netA.Node(res[i].GetA()).ccrd());
            sorted_pairs.Add(v.Length(), i);
          }
          TTable tab(res.Count() / 4 + ((res.Count() % 4) != 0 ? 1 : 0), 12);
          tab.ColName(0) = tab.ColName(3) = tab.ColName(6) = tab.ColName(9) =
            "Atom A";
          tab.ColName(1) = tab.ColName(4) = tab.ColName(7) = tab.ColName(10) =
            "Atom B";
          tab.ColName(2) = tab.ColName(5) = tab.ColName(8) = tab.ColName(11) =
            "Dist/A";
          TBasicApp::NewLogEntry() << "Matching pairs:";
          for (size_t i = 0; i < sorted_pairs.Count(); i++) {
            size_t idx = sorted_pairs.GetValue(i);
            size_t c_off = (i % 4) * 3, r_i = i / 4;
            tab[r_i][c_off + 0] = netA.Node(res[idx].GetA()).GetLabel();
            tab[r_i][c_off + 1] = netB.Node(res[idx].GetB()).GetLabel();
            tab[r_i][c_off + 2] = olxstr::FormatFloat(3, sorted_pairs.GetKey(i));
          }
          TBasicApp::NewLogEntry() <<
            tab.CreateTXTList("Matching pairs", true, false, "  ");
          {
            TBasicApp::NewLogEntry() << NewLineSequence() <<
              "Transformation matrix (fractional):";
            /* AtxBtxCt = (CxBxA)t
            the matrix wil be used on fractional coordinates like R*x + t and
            knowing that m-1 = mt :
            */
            mat3d m1 = mat3d::Transpose(
              au.GetCellToCartesian()*m*au.GetCartesianToCell());
            olxstr_buf mo;
            for (int i = 0; i < 3; i++) {
              TLog::LogEntry e = TBasicApp::NewLogEntry();
              for (int j = 0; j < 3; j++) {
                olxstr s = olxstr::FormatFloat(-3, m1[i][j]) << ' ';
                mo << s;
                e << s;
              }
            }
            TLog::LogEntry log_entry = TBasicApp::NewLogEntry();
            log_entry << NewLineSequence() << "Translation (fractional): ";
            vec3d center1 = au.Fractionalise(total_t);
            for (int i = 0; i < 3; i++) {
              olxstr s = olxstr::FormatFloat(-3, center1[i]) << ' ';
              mo << s;
              log_entry << s;
            }
            if (Options.GetBoolOption("cm"))
              app.ToClipboard(olxstr(mo));
          }
          {
            mat3d_list pms;
            pms.AddCopy(m);
            vec3d_list o_crd;
            const TSpaceGroup &sg = app.XFile().GetLastLoaderSG();
            if (sg.MatrixCount() > 0) {
              o_crd.SetCount(atomsToTransform.Count());
              for (size_t i = 0; i < atomsToTransform.Count(); i++) {
                o_crd[i] = atomsToTransform[i]->crd();
              }
            }
            for (size_t mi = 0; mi < sg.MatrixCount(); mi++) {
              for (size_t ai = 0; ai < atomsToTransform.Count(); ai++) {
                atomsToTransform[ai]->crd() = au.Orthogonalise(
                  sg.GetMatrix(mi) * atomsToTransform[ai]->ccrd());
              }
              TNetwork::AlignInfo ai = TNetwork::GetAlignmentRMSD(satomp,
                TryInvert, weight_calculator, false);
              mat3d tm;
              QuaternionToMatrix(ai.align_out.quaternions[0], tm);
              vec3d center = ai.align_out.center_b;
              if (ai.inverted) {
                tm *= -1;
                center *= -1;
              }
              bool found = false;
              for (size_t j = 0; j < pms.Count(); j++) {
                double d = 0;
                for (int j1 = 0; j1 < 3; j1++) {
                  for (int j2 = 0; j2 < 3; j2++)
                    d += olx_abs(pms[j][j1][j2] - tm[j][j2]);
                }
                if (d < 1e-2) {
                  found = true;
                  break;
                }
              }
              if (found) {
                continue;
              }
              pms.AddCopy(tm);
              mat3d m1 = mat3d::Transpose(
                au.GetCellToCartesian()*tm*au.GetCartesianToCell());
              TBasicApp::NewLogEntry() << NewLineSequence() <<
                "Transformation matrix (fractional) for " <<
                TSymmParser::MatrixToSymmEx(sg.GetMatrix(mi)) << ':';
              for (int i = 0; i < 3; i++) {
                TLog::LogEntry e = TBasicApp::NewLogEntry();
                for (int j = 0; j < 3; j++) {
                  e << olxstr::FormatFloat(-3, m1[i][j]) << ' ';
                }
              }
            }
            // rest the coordinates if needed
            if (sg.MatrixCount() > 0) {
              for (size_t i = 0; i < atomsToTransform.Count(); i++)
                atomsToTransform[i]->crd() = o_crd[i];
            }
          }
        }
        if (align) {
          if (Options.GetBoolOption('o')) {
            if (netA.GetLattice() == netB.GetLattice()) {
              TBasicApp::NewLogEntry() << "Skipping -o option - the fragments "
                "belong to the same lattice";
            }
            else {
              atomsToTransform.Clear();
              atomsToTransform.AddAll(netB.GetLattice().GetObjects().atoms);
            }
          }
          TNetwork::DoAlignAtoms(atomsToTransform, align_info);
          app.UpdateBonds();
          app.CenterView();
        }
        if (subgraph) {
          sk.Add(res[0].GetB());
          res.Clear();
          while (atoms[0]->GetNetwork().IsSubgraphOf(
            atoms[1]->GetNetwork(), res, sk))
          {
            sk.Add(res[0].GetB());
            olxstr tmp = '=';
            for (size_t i = 0; i < res.Count(); i++) {
              tmp << '{' <<
                atoms[0]->GetNetwork().Node(res[i].GetA()).GetLabel() << ',' <<
                atoms[1]->GetNetwork().Node(res[i].GetB()).GetLabel() << '}';
            }
            TBasicApp::NewLogEntry() << tmp;
            res.Clear();
          }
        }
      }
    }
    // a full basis provided
    else if (atoms.Count() >= 3 * group_cnt && (atoms.Count() % group_cnt) == 0) {
      const size_t atom_a_group = atoms.Count() / group_cnt;
      TTypeList<olx_pair_t<TSAtom*, TSAtom*> > satomp(atom_a_group);
      // fill the reference group
      for (size_t i = 0; i < atom_a_group; i++)
        satomp[i].SetA(atoms[i]);
      TNetwork &netA = atoms[0]->GetNetwork();
      for (size_t gi = 1; gi < group_cnt; gi++) {
        for (size_t i = 0; i < atom_a_group; i++)
          satomp[i].SetB(atoms[i + gi*atom_a_group]);
        TNetwork &netB = satomp[0].GetB()->GetNetwork();
        bool valid = true;
        for (size_t i = 1; i < satomp.Count(); i++) {
          if (satomp[i].GetA()->GetNetwork() != netA ||
            satomp[i].GetB()->GetNetwork() != netB)
          {
            valid = false;
            E.ProcessingError(__OlxSrcInfo,
              "Atoms should belong to two distinct fragments or the same"
              " fragment. Skipping group #") << gi;
            break;
          }
        }
        if (valid) {
          match_cnt++;
          // restore the other unit cell, if any...
          if (&latt != &netA.GetLattice() || &latt != &netB.GetLattice()) {
            TLattice& latt1 = (&latt == &netA.GetLattice()) ? netB.GetLattice()
              : netA.GetLattice();
            latt1.RestoreADPs();
          }
          TSAtomPList atomsToTransform;
          if (netA != netB) // collect all atoms
            atomsToTransform = netB.GetNodes();
          else
            atomsToTransform = atoms.SubList(gi*atom_a_group, atom_a_group);
          TNetwork::AlignInfo align_info =
            GXLibMacros_MatchAtomPairsQT(satomp, TryInvert, weight_calculator);
          TNetwork::DoAlignAtoms(atomsToTransform, align_info);
          if (Options.GetBoolOption('m')) {
            for (size_t ai = 0; ai < atomsToTransform.Count(); ai++) {
              atomsToTransform[ai]->ccrd() =
                netB.GetLattice().GetAsymmUnit().Fractionalise(
                  atomsToTransform[ai]->crd());
              atomsToTransform[ai]->CAtom().ccrd() =
                atomsToTransform[ai]->ccrd();
            }
          }
          GXLibMacros_CallMatchCallbacks(netA, netB, align_info.rmsd.GetV());
          if (group_cnt == 2 && Options.GetBoolOption("cm")) {
            mat3d m;
            QuaternionToMatrix(align_info.align_out.quaternions[0], m);
            vec3d center = align_info.align_out.center_b;
            if (align_info.inverted) {
              m *= -1;
              center *= -1;
            }
            vec3d total_t = align_info.align_out.center_a - center*m;
            mat3d m1 = mat3d::Transpose(
              au.GetCellToCartesian()*m*au.GetCartesianToCell());
            olxstr_buf mo;
            for (int i = 0; i < 3; i++) {
              for (int j = 0; j < 3; j++) {
                mo << olxstr::FormatFloat(-3, m1[i][j]) << ' ';
              }
            }
            vec3d center1 = au.Fractionalise(total_t);
            for (int i = 0; i < 3; i++) {
              mo << olxstr::FormatFloat(-3, center1[i]) << ' ';
            }
            app.ToClipboard(olxstr(mo));
          }
        }
      }
    }
  }
  else {
    TNetPList nets;
    app.GetNetworks(nets);
    // restore the other unit cell, if any...
    for (size_t i = 0; i < nets.Count(); i++) {
      if (&latt != &nets[i]->GetLattice()) {
        nets[i]->GetLattice().RestoreADPs();
      }
    }
    TEBitArray matched(nets.Count());
    for (size_t i = 0; i < nets.Count(); i++) {
      if (!nets[i]->IsSuitableForMatching() || matched[i]) {
        continue;
      }
      for (size_t j = i + 1; j < nets.Count(); j++) {
        if (!nets[j]->IsSuitableForMatching() || matched[j]) {
          continue;
        }
        TTypeList<olx_pair_t<size_t, size_t> > res;
        if (!nets[i]->DoMatch(*nets[j], res, false, weight_calculator)) {
          continue;
        }
        match_cnt++;
        matched.SetTrue(j);
        TTypeList<olx_pair_t<TSAtom*, TSAtom*> > ap;
        for (size_t k = 0; k < res.Count(); k++) {
          if (exclude_h && nets[i]->Node(res[k].GetA()).GetType() == 1) {
            continue;
          }
          ap.AddNew<TSAtom*, TSAtom*>(
            &nets[i]->Node(res[k].GetA()), &nets[j]->Node(res[k].GetB()));
        }
        TNetwork::AlignInfo a_i =
          GXLibMacros_MatchAtomPairsQT(ap, false, weight_calculator);
        // get info for the inverted fragment
        res.Clear();
        ap.Clear();
        nets[i]->DoMatch(*nets[j], res, true, weight_calculator);
        for (size_t k = 0; k < res.Count(); k++) {
          if (exclude_h && nets[i]->Node(res[k].GetA()).GetType() == 1) {
            continue;
          }
          ap.AddNew<TSAtom*, TSAtom*>(
            &nets[i]->Node(res[k].GetA()), &nets[j]->Node(res[k].GetB()));
        }
        TNetwork::AlignInfo ia_i =
          GXLibMacros_MatchAtomPairsQT(ap, true, weight_calculator);
        TNetwork::AlignInfo& ra_i =
          (a_i.rmsd.GetV() < ia_i.rmsd.GetV() ? a_i : ia_i);
        GXLibMacros_CallMatchCallbacks(*nets[i], *nets[j], ra_i.rmsd.GetV());
        //
                //inertia<>::out io1 = inertia<>::calc(nets[i]->GetNodes(),
                //  FunctionAccessor::Make(&TSAtom::crd),
                //  FunctionAccessor::MakeStatic(&TSAtom::weight_occu_z));
                //  io1.sort();
                //inertia<>::out io2;
                //if (ra_i.inverted) {
                //  io2 = inertia<>::calc(nets[j]->GetNodes(),
                //    olx_alg::olx_chsig(FunctionAccessor::Make(&TSAtom::crd)),
                //    FunctionAccessor::MakeStatic(&TSAtom::weight_occu_z));
                //    io2.center *= -1;
                //    io2.axis *= -1;
                //}
                //else {
                //  io2 = inertia<>::calc(nets[j]->GetNodes(),
                //    FunctionAccessor::Make(&TSAtom::crd),
                //    FunctionAccessor::MakeStatic(&TSAtom::weight_occu_z));
                //}
                //io2.sort();
                //mat3d tm = mat3d::Transpose(io2.axis)*io1.axis;
                //TNetwork::DoAlignAtoms(nets[j]->GetNodes(), io2.center,
                //  tm,
                //  io1.center);
        //
        TNetwork::DoAlignAtoms(nets[j]->GetNodes(), ra_i);
      }
    }
    // do match all possible fragments with similar number of atoms
  }
  if (match_cnt != 0) {
    app.UpdateBonds();
    app.CenterView();
  }
  else {
    TBasicApp::NewLogEntry() << "No matching fragments found";
  }
}
//..............................................................................
olxstr GXLibMacros_funMatchNets(TNetwork& netA, TNetwork& netB, bool invert,
  bool verbose,
  double (*w_c)(const TSAtom&))
{
  TTypeList<olx_pair_t<size_t, size_t> > res;
  TSizeList sk;
  bool match = false;
  try  {  match = netA.DoMatch(netB, res, invert, w_c);  }
  catch(...)  {
    return "exception";
  }
  if( match )  {
    olxstr rv;
    TTypeList<olx_pair_t<TSAtom*,TSAtom*> > satomp;
    for( size_t i=0; i < res.Count(); i++ ) {
      satomp.AddNew<TSAtom*,TSAtom*>(
        &netA.Node(res[i].GetA()), &netB.Node(res[i].GetB()));
    }
    TNetwork::AlignInfo align_info =
      GXLibMacros_MatchAtomPairsQT(satomp, invert, w_c, false);
    rv = olxstr::FormatFloat(3, align_info.rmsd.GetV());
    mat3d m;
    QuaternionToMatrix(align_info.align_out.quaternions[0], m);
    sorted::PrimitiveAssociation<double, olx_pair_t<TSAtom*,TSAtom*>*> pairs;
    pairs.SetCapacity(satomp.Count());
    const TAsymmUnit& au2 =
      satomp[0].GetB()->GetNetwork().GetLattice().GetAsymmUnit();
    for( size_t i=0; i < satomp.Count(); i++ )  {
      vec3d v = satomp[i].GetB()->ccrd();
      if( invert )  v *= -1;
      au2.CellToCartesian(v);
      const double d = (satomp[i].GetA()->crd()-align_info.align_out.center_a)
        .DistanceTo((v-align_info.align_out.center_b)*m);
      pairs.Add(d, &satomp[i]);
    }
    rv << ",{";
    if( verbose )  {
      for( size_t i=0; i < pairs.Count(); i++ )  {
        size_t index = pairs.Count() - i -1;
        rv << olxstr::FormatFloat(3, pairs.GetKey(index)) << "(";
        rv << pairs.GetValue(index)->GetA()->GetLabel() << ',' <<
          pairs.GetValue(index)->GetB()->GetLabel() << ')';
        if( i+1 < pairs.Count() )
          rv << ',';
      }
    }
    else  {
      rv << olxstr::FormatFloat(3, pairs.GetLastKey()) << "(";
      rv << pairs.GetLastValue()->GetA()->GetLabel() << ',' <<
        pairs.GetLastValue()->GetB()->GetLabel() << ')';
    }
    return rv << '}';
  }
  return XLibMacros::NAString();
}
TStrList TMainForm_funMatchNets1(TNetwork& netA, TNetwork& netB, bool verbose)
{
  TStrList rv;
  if( !netA.IsSuitableForMatching() || !netB.IsSuitableForMatching() )
    return rv;
  const olxstr r = GXLibMacros_funMatchNets(netA, netB, false, verbose,
    TSAtom::weight_unit);
  if( r == XLibMacros::NAString() )  return rv;
  rv.Add("Fragment content: ") << netA.GetFormula();
  rv.Add("inverted:false,weight:unit=") << r;
  rv.Add("inverted:false,weight:Z=") <<
    GXLibMacros_funMatchNets(netA, netB, false, verbose, TSAtom::weight_z);
  rv.Add("inverted:true,weight:unit=") <<
    GXLibMacros_funMatchNets(netA, netB, true, verbose, TSAtom::weight_unit);
  rv.Add("inverted:true,weight:Z=") <<
    GXLibMacros_funMatchNets(netA, netB, true, verbose, TSAtom::weight_z);
  return rv;
}
TStrList GXLibMacros_funMatchLatts(TLattice& lattA, TLattice& lattB,
  const TStrObjList& Params)
{
  TStrList rv;
  const bool verbose = (Params.Count() == 3 && Params[2].Equalsi("verbose"));
  if( lattA.FragmentCount() > 1 )  {
    rv.Add("Comparing '") << TEFile::ExtractFileName(Params[0]) <<
      "' to itself";
    for( size_t i=0; i < lattA.FragmentCount(); i++ )  {
      if( !lattA.GetFragment(i).IsSuitableForMatching() )  continue;
      for( size_t j=i+1; j < lattA.FragmentCount(); j++ )  {
        if( !lattA.GetFragment(j).IsSuitableForMatching() )  continue;
        rv.AddAll(
          TMainForm_funMatchNets1(
            lattA.GetFragment(i), lattA.GetFragment(j), verbose));
      }
    }
  }
  if( lattB.FragmentCount() > 1 )  {
    rv.Add("Comparing '") << TEFile::ExtractFileName(Params[1]) <<
      "' to itself";
    for( size_t i=0; i < lattB.FragmentCount(); i++ )  {
      if( !lattB.GetFragment(i).IsSuitableForMatching() )  continue;
      for( size_t j=i+1; j < lattB.FragmentCount(); j++ )  {
        if( !lattB.GetFragment(j).IsSuitableForMatching() )  continue;
        rv.AddAll(
          TMainForm_funMatchNets1(lattB.GetFragment(i),
            lattB.GetFragment(j), verbose));
      }
    }
  }
  rv.Add("Comparing the two files");
  for( size_t i=0; i < lattA.FragmentCount(); i++ )  {
    if( !lattA.GetFragment(i).IsSuitableForMatching() )  continue;
    for( size_t j=0; j < lattB.FragmentCount(); j++ )  {
      if( !lattB.GetFragment(j).IsSuitableForMatching() )  continue;
      rv.AddAll(
        TMainForm_funMatchNets1(
          lattA.GetFragment(i), lattB.GetFragment(j), verbose));
    }
  }
  return rv;
}
//..............................................................................
void GXLibMacros::funMatchFiles(const TStrObjList& Params, TMacroData &E)  {
  double (*weight_calculator)(const TSAtom&) = &TSAtom::weight_occu;
  try  {
    TXFile* f1 = dynamic_cast<TXFile *>(app.XFile().Replicate());
    TXFile* f2 = dynamic_cast<TXFile *>(app.XFile().Replicate());
    try  {
      f1->LoadFromFile(Params[0]);
      f2->LoadFromFile(Params[1]);
      TStrList rv;
      TLattice& lattA = f1->GetLattice();
      TLattice& lattB = f2->GetLattice();
      rv.Add("!H atoms included");
      rv.AddAll(GXLibMacros_funMatchLatts(lattA, lattB, Params));
      rv.Add("!H atoms excluded");
      lattA.GetAsymmUnit().DetachAtomType(iHydrogenZ, true);
      lattA.UpdateConnectivity();
      lattB.GetAsymmUnit().DetachAtomType(iHydrogenZ, true);
      lattB.UpdateConnectivity();
      rv.AddAll(GXLibMacros_funMatchLatts(lattA, lattB, Params));
      delete f1;
      delete f2;
      E.SetRetVal(rv.Text(NewLineSequence()));
      return;
    }
    catch(...)  {
      delete f1;
      delete f2;
    }
  }
  catch(...)  {}
  E.SetRetVal(XLibMacros::NAString());
}
//..............................................................................
void GXLibMacros::funSelName(const TStrObjList& Params, TMacroData &E)  {
  TGlGroup& sel = app.GetSelection();
  if (sel.Count() == 1) {
    E.SetRetVal(sel[0].GetCollectionName());
  }
  else {
    E.SetRetVal(EmptyString());
  }
}
//..............................................................................
struct SetMaterialUndo : public TUndoData {
  olxstr_dict<TGlMaterial> mat_list;
  SetMaterialUndo() :
    TUndoData(UndoAction::New(&SetMaterialUndo::undo))
  {}

  void push(const olxstr &obj, const TGlMaterial &m) {
    mat_list.Add(obj, m);
  }
  static void undo(TUndoData *d) {
    SetMaterialUndo *m = dynamic_cast<SetMaterialUndo *>(d);
    if (d == 0) {
      return;
    }
    for (size_t i = 0; i < m->mat_list.Count(); i++) {

    }
  }
};
void GXLibMacros::macSetMaterial(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  olx_object_ptr<SetMaterialUndo> undo = new SetMaterialUndo();
  olx_object_ptr<TGlMaterial> glm;
  if (!Cmds[1].Equalsi("None")) {
    try {
      glm = new TGlMaterial();
      glm().FromString(Cmds[1], true);
    }
    catch (...) {
      glm = 0;
    }
  }
  sorted::PointerPointer<TGPCollection> colls;
  size_t di = Cmds[0].IndexOf('.');
  olxstr col_name = di != InvalidIndex ? Cmds[0].SubStringTo(di) : Cmds[0];
  olxstr prm_name = di != InvalidIndex ? Cmds[0].SubStringFrom(di + 1)
    : EmptyString();
  if (!glm.is_valid() && !prm_name.Equals('*')) {
    E.ProcessingError(__OlxSrcInfo,
      "The style can be reset to all promtives only");
    return;
  }
  if (col_name.Equalsi("sel")) {
    TGlGroup &g = app.GetSelection();
    for (size_t i = 0; i < g.Count(); i++) {
      colls.AddUnique(&g[i].GetPrimitives());
    }
  }
  else {
    TGPCollection* gpc = app.GetRenderer().FindCollection(col_name);
    if (gpc != 0) {
      colls.Add(gpc);
    }
    else {  // try to modify the style then, if exists
      bool found = false;
      TGraphicsStyle* gs = app.GetRenderer().GetStyles().FindStyle(col_name);
      if (gs != 0) {
        if (prm_name == '*') {
          if (!glm.is_valid()) {
            gs->Clear();
          }
          else {
            for (size_t pi = 0; pi < gs->PrimitiveStyleCount(); pi++) {
              gs->GetPrimitiveStyle(pi).SetProperties(glm());
            }
          }
          found = true;
        }
        else {
          TGlMaterial* mat = gs->FindMaterial(prm_name);
          if (mat != 0) {
            *mat = glm();
            found = true;
          }

        }
      }
      if (!found) {
        E.ProcessingError(__OlxSrcInfo, "Undefined style ").quote() <<
          Cmds[0];
      }
      return;
    }
  }
  for (size_t ci = 0; ci < colls.Count(); ci++) {
    bool found = false;
    if (!prm_name.IsEmpty()) {
      if (prm_name == '*') {
        if (!glm.is_valid()) {
          colls[ci]->ClearPrimitives();
          colls[ci]->GetStyle().Clear();
        }
        else {
          for (int pi = 0; pi < colls[ci]->PrimitiveCount(); pi++) {
            TGlPrimitive &glp = colls[ci]->GetPrimitive(pi);
            glp.SetProperties(glm());
            colls[ci]->GetStyle().SetMaterial(glp.GetName(), glm());
          }
        }
        found = true;
      }
      else {
        TGlPrimitive* glp = colls[ci]->FindPrimitiveByName(prm_name);
        if (glp != 0) {
          glp->SetProperties(glm());
          colls[ci]->GetStyle().SetMaterial(prm_name, glm());
          found = true;
        }
      }
      if (!glm.is_valid() && colls[ci]->ObjectCount() > 0) {
        colls[ci]->GetObject(0).Create(colls[ci]->GetName());
      }
    }
    else {
      for (size_t i = 0; i < colls[ci]->ObjectCount(); i++)  {
        TGlGroup *glg = dynamic_cast<TGlGroup*>(&colls[ci]->GetObject(i));
        if (glg != 0) {
          glg->SetGlM(glm());
          found = true;
        }
      }
    }
    if (!found) {
      TBasicApp::NewLogEntry(logError) << "Collection '" << colls[ci]->GetName()
        << "' is not processed";
    }
  }
  if (!undo().mat_list.IsEmpty()) {
    app.GetUndo().Push(undo.release());
  }
}
//..............................................................................
void GXLibMacros::funGetMaterial(const TStrObjList &Params, TMacroData &E) {
  const TGlMaterial* mat = 0;
  size_t di = Params[0].IndexOf('.');
  olxstr col_name = di != InvalidIndex ? Params[0].SubStringTo(di) : Params[0];
  olxstr prm_name = di != InvalidIndex ? Params[0].SubStringFrom(di + 1)
    : EmptyString();
  TGPCollection* gpc = 0;
  if (col_name.Equalsi("sel")) {
    TGlGroup &g = app.GetSelection();
    if (g.Count() != 1) {
      E.ProcessingError(__OlxSrcInfo, "Please select one object only");
      return;
    }
    gpc = &g[0].GetPrimitives();
  }
  if (gpc == 0) {
    gpc = app.GetRenderer().FindCollection(col_name);
  }
  if (gpc != 0) {
    if (prm_name.IsEmpty()) {
      if (gpc->Is<TGlGroup>()) {
        mat = &((TGlGroup*)gpc)->GetGlM();
      }
    }
    else {
      TGlPrimitive* glp = gpc->FindPrimitiveByName(prm_name);
      if (glp != 0) {
        mat = &glp->GetProperties();
      }
    }
  }
  else {  // check if the style exists
    TGraphicsStyle* gs = app.GetRenderer().GetStyles().FindStyle(col_name);
    if (gs != 0) {
      if (prm_name.IsEmpty()) {
        mat = gs->FindMaterial("mat");
      }
      else {
        mat = gs->FindMaterial(prm_name);
      }
    }
  }
  if (mat == 0) {
    E.ProcessingError(__OlxSrcInfo, "Undefined material ").quote() <<
      Params[0];
    return;
  }
  else {
    if (Params.Count() == 2) {
      E.SetRetVal(mat->ToPOV());
    }
    else {
      E.SetRetVal(mat->ToString());
    }
  }
}
//..............................................................................
void GXLibMacros::macDefineVar(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  RefinementModel &rm = app.XFile().GetRM();
  CalculatedVars &cv = rm.CVars;
  TGlGroup &g = app.GetSelection();
  if (g.Count() == 2) {
    if (g[0].Is<TXPlane>() && g[1].Is<TXPlane>()) {
      CalculatedVars::Object
        *p1 = CalculatedVars::Object::create((TXPlane&)g[0], cv),
        *p2 = CalculatedVars::Object::create((TXPlane&)g[1], cv);
      cv.AddVar(new CalculatedVars::Var(Cmds[0] + "cc"))
        .AddRef(*p1, "c").AddRef(*p2, "c").type = cv_vt_distance;
      cv.AddVar(new CalculatedVars::Var(Cmds[0] + "pc"))
        .AddRef(*p1, "c").AddRef(*p2).type = cv_vt_distance;
      cv.AddVar(new CalculatedVars::Var(Cmds[0] + "a"))
        .AddRef(*p1).AddRef(*p2).type = cv_vt_angle;
      cv.AddVar(new CalculatedVars::Var(Cmds[0] + "sa"))
        .AddRef(*p1, "c").AddRef(*p2).type = cv_vt_shift;
      cv.AddVar(new CalculatedVars::Var(Cmds[0] + "sb"))
        .AddRef(*p1).AddRef(*p2, "c").type = cv_vt_shift;
    }
    else if ((g[0].Is<TXPlane>() && g[1].Is<TXBond>()) ||
      (g[1].Is<TXPlane>() && g[0].Is<TXBond>()))
    {
      TXPlane &p = (TXPlane &)(g[g[0].Is<TXPlane>() ? 0 : 1]);
      TXBond &b = (TXBond &)(g[g[0].Is<TXPlane>() ? 1 : 0]);
      CalculatedVars::Object
        *p1 = CalculatedVars::Object::create(p, cv),
        *p2 = CalculatedVars::Object::create(b, cv);
      cv.AddVar(new CalculatedVars::Var(Cmds[0]))
        .AddRef(*p1, "n").AddRef(*p2).type = cv_vt_angle;
    }
    else if (g[0].Is<TXAtom>() && g[1].Is<TXAtom>()) {
      CalculatedVars::Object
        *p1 = CalculatedVars::Object::create((TXAtom &)g[0], cv),
        *p2 = CalculatedVars::Object::create((TXAtom &)g[1], cv);
      cv.AddVar(new CalculatedVars::Var(Cmds[0]))
        .AddRef(*p1).AddRef(*p2).type = cv_vt_distance;
    }
  }
  else if (g.Count() == 3 &&
    olx_list_and_st(g, &olx_is<TXAtom, TGlGroup::list_item_type>))
  {
    CalculatedVars::Object
      *p1 = CalculatedVars::Object::create((TXAtom &)g[0], cv),
      *p2 = CalculatedVars::Object::create((TXAtom &)g[1], cv),
      *p3 = CalculatedVars::Object::create((TXAtom &)g[2], cv);
    cv.AddVar(new CalculatedVars::Var(Cmds[0]))
      .AddRef(*p1).AddRef(*p2).AddRef(*p3).type = cv_vt_angle;

  }
  //
}
//..............................................................................
void GXLibMacros::macProjSph(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  {
    ElementRadii radii;
    if (Cmds.Count() == 1 && TEFile::Exists(Cmds[0])) {
      radii = TXApp::ReadRadii(Cmds[0]);
    }
    ContentList cl = app.XFile().GetAsymmUnit().GetContentList();
    for (size_t i = 0; i < cl.Count(); i++) {
      cl[i].element->r_custom = radii.Find(cl[i].element, cl[i].element->r_vdw);
    }
    TXApp::PrintCustomRadii(radii, cl);
  }
  TArrayList<uint32_t> colors;
  for (size_t i = 0; i < Cmds.Count(); i++) {
    if (Cmds[i].IsNumber())  {
      colors.Add(Cmds[i].SafeUInt<uint32_t>());
      Cmds.Delete(i--);
    }
  }
  TXAtomPList xatoms = app.FindXAtoms(Cmds, false, true);
  if (xatoms.Count() != 1) {
    E.ProcessingError(__OlxSourceInfo, "one atom is expected");
    return;
  }
  TBasicApp::NewLogEntry() << "Analysing : " << xatoms[0]->GetGuiLabel() <<
    " environment";
  static size_t counter = 0;
  PointAnalyser &pa = *new PointAnalyser(*xatoms[0]);
  pa.alpha = Options.FindValue('a', "0x9c").SafeUInt<uint8_t>();
  pa.emboss = Options.GetBoolOption('e');
  {
    if (pa.colors.Count() == 1 && !colors.IsEmpty()) {
      pa.colors[0] = colors[0];
    }
    else {
      size_t cr = 0;
      for (size_t i = 0; i < colors.Count(); i++) {
        if (xatoms[0]->GetNetwork().GetOwnerId() == i) {
          cr = 1;
        }
        if (i + cr >= pa.colors.Count()) {
          break;
        }
        pa.colors[i + cr] = colors[i];
      }
    }
  }
  size_t g = Options.FindValue('g', 6).ToSizeT();
  if (g > 10) {
    g = 10;
  }
  TDSphere &sph = app.DSphere();
  if (sph.IsCreated()) {
    sph.GetPrimitives().ClearPrimitives();
  }
  sph.SetAnalyser(&pa);
  sph.SetGeneration(g);
  sph.SetVisible(true);
  sph.Create();
  sph.Basis.SetCenter(xatoms[0]->crd());
  sph.Basis.SetZoom(2);
  if (Options.GetBoolOption("group")) {
    olx_pdict<size_t, TGlGroup *> groups;
    TGXApp::AtomIterator atoms = app.GetAtoms();
    while (atoms.HasNext()) {
      TXAtom &a = atoms.Next();
      if (&a == xatoms[0] || !a.IsAvailable() || a.IsGrouped()) {
        continue;
      }
      TGlGroup *glg = groups.Find(a.GetNetwork().GetOwnerId(), 0);
      if (glg == 0) {
        glg = &app.GetRenderer().NewGroup(
          olxstr("Ligand") << (groups.Count() + 1));
        groups.Add(a.GetNetwork().GetOwnerId(), glg)->Create();
      }
      glg->Add(a);
    }
    TGXApp::BondIterator bonds = app.GetBonds();
    while (bonds.HasNext()) {
      TXBond &b = bonds.Next();
      if (!b.IsAvailable() || b.IsGrouped()) {
        continue;
      }
      TGlGroup *glg = groups.Find(b.A().GetNetwork().GetOwnerId(), 0);
      if (glg == 0) {
        glg = &app.GetRenderer().NewGroup(
          olxstr("Ligand") << (groups.Count() + 1));
        groups.Add(b.A().GetNetwork().GetOwnerId(), glg)->Create();
      }
      glg->Add(b);
    }
    for (size_t i = 0; i < groups.Count(); i++) {
      TGlMaterial glm = groups.GetValue(i)->GetGlM();
      glm.AmbientF = pa.colors[groups.GetKey(i)];
      glm.DiffuseF = pa.colors[groups.GetKey(i)];
      glm.SetTransparent(false);
      groups.GetValue(i)->SetGlM(glm);
    }
  }
  TTable table_l(0, 3), table_o(0, 2);
  table_l.ColName(0) = "Ligand id";
  table_l.ColName(1) = "Area total, %";
  table_l.ColName(2) = "Area individual, %";
  olxdict<TNetwork *, olx_pair_t<double, TSAtom *>, TPointerComparator> li;
  olxdict<TNetwork *, size_t, TPointerComparator> ri;
  for (size_t i = 0; i < pa.areas.Count(); i++) {
    olxstr legend;
    double prc = (double)(pa.areas.GetValue(i) * 100) / sph.GetVectorCount();
    if (pa.areas.GetKey(i).IsEmpty()) {
      legend = "Accessible";
    }
    else {
      legend.SetLength(0);
      TStrList toks(pa.areas.GetKey(i), ';');
      for (size_t j = 0; j < toks.Count(); j++) {
        TNetwork &n = app.XFile().GetLattice().GetFragment(toks[j].ToSizeT());
        if (toks.Count() == 1) {
          ri(&n, table_l.RowCount());
        }
        if (n.NodeCount() == 0) {
          continue;
        }
        TSAtom *a = &n.Node(0);
        for (size_t j = 1; j < n.NodeCount(); j++) {
          if (a->GetType() < n.Node(j).GetType()) {
            a = &n.Node(j);
          }
        }
        if (!legend.IsEmpty()) {
          legend << ',';
        }
        legend << a->GetGuiLabel();
        li(&n, olx_pair_t<double, TSAtom *>(0, a)).a += prc;
      }
    }
    bool lo = !pa.areas.GetKey(i).Contains(';');
    TStrList &r = (lo ? table_l : table_o).AddRow();
    r[0] = legend;
    r[lo ? 2 : 1] = olxstr::FormatFloat(2, prc);
  }
  app.XFile().GetLattice().GetObjects().atoms.ForEach(
    ACollectionItem::TagSetter(-1));
  for (size_t i = 0; i < ri.Count(); i++) {
    if (ri.GetValue(i) < table_l.RowCount()) {
      size_t ix = li.IndexOf(ri.GetKey(i));
      table_l[ri.GetValue(i)][1] = olxstr::FormatFloat(2, li.GetValue(ix).a);
      li.GetValue(ix).b->SetTag(0);
    }
  }
  for (size_t i = 0; i < li.Count(); i++) {
    if (li.GetValue(i).b->GetTag() == 0) {
      continue;
    }
    TStrList &row = table_l.AddRow();
    row[0] = li.GetValue(i).b->GetGuiLabel();
    row[1] = olxstr::FormatFloat(2, li.GetValue(i).a);
    row[2] = '-';
  }
  TBasicApp::NewLogEntry() <<
    table_l.CreateTXTList("Area coverage by ligand, unique (%)", true, false, ' ');
  TBasicApp::NewLogEntry() <<
    table_o.CreateTXTList("Overlapping area (%)", false, false, ' ');
  TBasicApp::NewLogEntry() << "For the use of solid angles, see: "
    "Guzei, I.A., Wendt, M. Dalton Trans., 2006, 3991-3999.";
}
//..............................................................................
void GXLibMacros::macOFileDel(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  if (app.XFiles().Count() == 1) {
    app.XFile().GetLattice().Uniq();
    return;
  }
  int ind = Cmds[0].ToInt();
  if (ind <= -1) {
    if (app.XFiles().Count() > 1)
      app.DeleteXFile(app.XFiles().Count() - 1);
  }
  else {
    if ((size_t)ind < app.XFiles().Count())
      app.DeleteXFile(ind);
    else {
      Error.ProcessingError(__OlxSrcInfo,
        "no overlayed files at given position");
    }
  }
}
//..............................................................................
void GXLibMacros::macOFileSwap(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  app.SetActiveXFile(Cmds.IsEmpty() ? 0 : Cmds[0].ToSizeT());
}
//.............................................................................
void GXLibMacros::macTwinView(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  mat3d m(0, 1, 0, -1, 0, 0, 0, 0, 1);
  vec3d t;
  if (Cmds.Count() > 8 && olx_list_and(Cmds, &olxstr::IsNumber)) {
    for (int i = 0; i < 9; i++) {
      m[i / 3][i % 3] = Cmds[i].ToDouble();
    }
    if (Options.GetBoolOption('t')) {
      m = mat3d::Transpose(m);
    }
  }
  if (!Options.GetBoolOption('c', true, true)) {
    const TAsymmUnit &au = app.XFile().GetAsymmUnit();
    m = au.GetCartesianToCell()*m*au.GetCellToCartesian();
  }
  if (m.Determinant() < 1) {
    m *= -1;
  }
  m.Normalise();
  app.GetRenderer().SetStereoMatrix(m);
  app.GetRenderer().SetStereoTranslation(t);
  app.GetRenderer().SetStereoFlag(glStereoMatrix);
}
//..............................................................................
void GXLibMacros::funFBond(const TStrObjList &Params, TMacroData &E) {
  if (Params.IsEmpty()) {
    TXBond::Settings &st = TXBond::GetSettings(app.GetRenderer());
    E.SetRetVal(st.GetUnitLength());
  }
  else {
    TXBondPList bonds = app.GetRenderer().GetSelection().Extract<TXBond>();
    if (bonds.IsEmpty()) {
      E.ProcessingError(__OlxSrcInfo, "Some bonds are expected");
      return;
    }
    double v = Params[0].ToDouble();
    if (v < 0 || v > 1) {
      E.ProcessingError(__OlxSrcInfo, "Value in range of [0-1] is expected");
      return;
    }
    if (v == 1) {
      TXBond::GetSettings(app.GetRenderer()).SetUnitLength(
        bonds[0]->Length());
    }
    smatd cm = bonds[0]->B().GetMatrix();
    if (!bonds[0]->A().GetMatrix().IsFirst()) {
      cm = bonds[0]->A().GetMatrix().Inverse()*cm;
    }
    TGXApp::BondIterator bi = app.GetBonds();
    while (bi.HasNext()) {
      TXBond &b = bi.Next();
      if (&b == bonds[0] || b.A().GetType() != bonds[0]->A().GetType() ||
        b.B().GetType() != bonds[0]->B().GetType())
      {
        continue;
      }
      smatd m = b.B().GetMatrix();
      if (!b.A().GetMatrix().IsFirst()) {
        m = b.A().GetMatrix().Inverse()*m;
      }
      if (m.Equals(cm)) {
        b.Params()[5] = v;
      }
    }
    bonds[0]->Params()[5] = v;
  }
}
//..............................................................................

void GXLibMacros::macCalcSurf(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &E)
{
  const float pr = Options.FindValue("pr", "1.2").ToFloat();
  TGXApp::AtomIterator ai = app.GetAtoms();
  TXAtomPList atoms;
  atoms.SetCapacity(ai.count);
  while (ai.HasNext()) {
    TXAtom &a = ai.Next();
    if (a.IsDeleted() || a.GetType() == iQPeakZ) {
      continue;
    }
    atoms.Add(a);
  }
  TMolSurf ms(atoms, pr);
  olx_object_ptr<CIsoSurface> sf = ms.Calculate(
    Cmds.IsEmpty() ? -0.1f : Cmds[0].ToFloat());

  TXBlob *blob = new TXBlob(app.GetRenderer(), "Blob");
  blob->vertices = sf().VertexList();
  blob->normals = sf().NormalList();
  blob->triangles = sf().TriangleList();
  TArrayList<int> owners = sf().GetVertexData();
  if (true) {
    for (int i = 0; i < 2; i++) {
      olx_grid_util::smoother sm(blob->vertices, blob->triangles);
      sm.smooth(0.5f);
    }
    TArrayList<size_t> indices = olx_grid_util::reducer(
      blob->vertices, blob->triangles).reduce(0.05);
    TArrayList<int> owners_o = owners;
    owners.SetCount(blob->vertices.Count());
    for (size_t j = 0; j < owners_o.Count(); j++) {
      if (indices[j] < owners.Count()) {
        owners[indices[j]] = owners_o[j];
      }
    }
    {
      olx_grid_util::smoother sm(blob->vertices, blob->triangles);
      for (int i = 0; i < 10; i++) {
        sm.smooth(0.8f);
      }
    }
  }
  blob->UpdateNormals();
  blob->colors.SetCount(blob->vertices.Count());

  olxstr mc = Options.FindValue('c', "m");
  if (mc.Equals('a')) {
    for (size_t i = 0; i < blob->vertices.Count(); i++) {
      if (owners[i] == -1) {
        continue;
      }
      TGlMaterial *m = atoms[owners[i]]->GetPrimitives()
        .GetStyle().FindMaterial("Sphere");
      if (m != 0) {
        blob->colors[i] = m->AmbientF;
      }
    }
  }
  else  {
    array_1d<float> values(blob->vertices.Count());
    TUnitCell& uc = app.XFile().GetUnitCell();
    if (mc.Equals('e')) {
      TRefList refs;
      TArrayList<compd> F;
      olxstr err = SFUtil::GetSF(refs, F, SFUtil::mapTypeDiff,
        Options.Contains("fcf") ? SFUtil::sfOriginFcf : SFUtil::sfOriginOlex2,
        (Options.FindValue("scale", "r").ToLowerCase().CharAt(0) == 'r') ?
        SFUtil::scaleRegression : SFUtil::scaleSimple);
      if (!err.IsEmpty()) {
        E.ProcessingError(__OlxSrcInfo, err);
        return;
      }
      TAsymmUnit& au = app.XFile().GetAsymmUnit();
      TArrayList<SFUtil::StructureFactor> P1SF;
      SFUtil::ExpandToP1(refs, F, uc.GetMatrixList(), P1SF);
      const double vol = app.XFile().GetLattice().GetUnitCell().CalcVolume();
      BVFourier::MapInfo mi;
      // init map
      const vec3i dim(au.GetAxes() * 5);
      TArray3D<float> map(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
      mi = BVFourier::CalcEDM(P1SF, map.Data, vol);
      MapUtil::MapGetter<float, 1> mg(map.Data);

      for (size_t i = 0; i < blob->vertices.Count(); i++) {
        values[i] = mg.Get(au.Fractionalise(blob->vertices[i]));
      }
    }
    else if (mc.Equals('d')) {
      for (size_t i = 0; i < blob->vertices.Count(); i++) {
        values[i] = blob->vertices[i].QDistanceTo(ms.GetCenter());
      }
    }
    else if (mc.Equals('m')) {
      TSurfCalculationTask1 calc_task1(blob->vertices, values, uc);
      TListIteratorManager<TSurfCalculationTask1> tasks(calc_task1,
        blob->vertices.Count(), tLinearTask, 8);
    }
    float min_qd = 1e6, max_qd = 0;
    for (size_t i = 0; i < values.width; i++) {
      if (values[i] > max_qd) {
        max_qd = values[i];
      }
      if (values[i] < min_qd) {
        min_qd = values[i];
      }
    }
    float mid_dq = (max_qd + min_qd) / 2,
      dq_max = (max_qd - min_qd) / 2;
    vec3f mc(1.f), sp(0, 1, 0), ep(1, 0, 0),
      nc1(sp - mc), nc2(ep - mc);
    for (size_t i = 0; i < blob->vertices.Count(); i++) {
      float qd = values[i];
      float v = (qd - mid_dq) / dq_max;
      const vec3f *c;
      float s;
      if (v < 0) {
        s = -v;
        c = &nc1;
      }
      else {
        s = v;
        c = &nc2;
      }
      for (int ci = 0; ci < 3; ci++) {
        blob->colors[i][ci] = mc[ci] + s*(*c)[ci];
      }
    }
  }
  blob->Create();
  app.AddObjectToCreate(blob);
}
//..............................................................................
void GXLibMacros::macLegend(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  bool v = !app.AtomLegend().IsVisible();
  if (!Cmds.IsEmpty()) {
    v = Cmds[0].ToBool();
  }
  if (!v) {
    app.AtomLegend().SetVisible(false);
  }
  else {
    if (Options.GetBoolOption('r')) {
      app.AtomLegend().SetPosition(0, 0);
    }
    app.AtomLegend().Update();
    app.AtomLegend().SetVisible(true);
  }
  
}
//.............................................................................
template <typename functor_t>
void adjust_style(TGXApp &app, const functor_t & functor,
  bool apply_to_atoms, bool apply_to_bonds,
  const olxstr_set<true> &primitives, const olxstr &src=EmptyString())
{
  for (size_t i = 0; i < app.GetRenderer().CollectionCount(); i++) {
    TGPCollection &c = app.GetRenderer().GetCollection(i);
    if (c.IsEmpty()) {
      continue;
    }
    if ((apply_to_atoms && c.GetObject(0).Is<TXAtom>()) ||
      (apply_to_bonds && c.GetObject(0).Is<TXBond>()))
    {
      for (size_t j = 0; j < c.PrimitiveCount(); j++) {
        TGlPrimitive &p = c.GetPrimitive(j);
        if (!primitives.IsEmpty() && !primitives.Contains(p.GetName())) {
          continue;
        }
        if (!src.IsEmpty()) {
          TGlPrimitive *sp = c.FindPrimitiveByName(src);
          if (sp != 0) {
            p.SetProperties(sp->GetProperties());
          }
          continue;
        }
        TGlMaterial m = p.GetProperties();
        // fix unnecessary properties
        m.SetAmbientB(false);
        m.SetDiffuseB(false);
        m.SetEmissionB(false);
        m.SetShininessB(false);
        m.SetSpecularB(false);
        functor(m);
        p.SetProperties(m);
        c.GetStyle().SetMaterial(p.GetName(), m);
      }
    }
  }
}

void sta_adjust_overflow(TGlOption &o) {
  float ovf = 0;
  for (int i = 0; i < 4; i++) {
    if (o[i] > 1) {
      if (ovf < o[i]) {
        ovf = o[i];
      }
      o[i] = 1;
    }
  }
  if (ovf > 1) {
    ovf -= 1;
    for (int i = 0; i < 4; i++) {
      if (o[i] < 1) {
        if ((o[i] += ovf) > 1) {
          o[i] = 1;
        }
      }
    }
  }
}
struct sta_adjust_pc {
  double pc;
  int what;
  sta_adjust_pc(double pc, int what)
    : pc(pc), what(what)
  {}
  TGlMaterial &operator()(TGlMaterial &m) const {
    if (what == 0) {
      m.SetAmbientF(true);
      m.AmbientF *= pc;
      sta_adjust_overflow(m.AmbientF);
    }
    else if (what == 1) {
      m.SetDiffuseF(true);
      m.DiffuseF = m.AmbientF * pc;
      sta_adjust_overflow(m.DiffuseF);
    }
    else if (what == 2) {
      m.SetSpecularF(true);
      m.SpecularF = m.AmbientF * pc;
      sta_adjust_overflow(m.SpecularF);
    }
    else if (what == 3) {
      m.SetEmissionF(true);
      m.EmissionF = m.AmbientF * pc;
      sta_adjust_overflow(m.EmissionF);
    }
    return m;
  }
};

struct sta_adjust_cl {
  uint32_t cl;
  int what;
  sta_adjust_cl(uint32_t cl, int what)
    : cl(cl), what(what)
  {}
  TGlMaterial &operator()(TGlMaterial &m) const {
    if (what == 0) {
      m.SetAmbientF(true);
      m.AmbientF = cl;
    }
    else if (what == 1) {
      m.SetDiffuseF(cl != 0);
      m.DiffuseF = cl;
    }
    else if (what == 2) {
      m.SetSpecularF(cl != 0);
      m.SpecularF = cl;
    }
    else if (what == 3) {
      m.SetEmissionF(cl != 0);
      m.EmissionF = cl;
    }
    return m;
  }
};

struct sta_adjust_shininess {
  short value;
  sta_adjust_shininess(short value)
    : value(value)
  {}
  TGlMaterial &operator()(TGlMaterial &m) const {
    m.SetShininessF(true);
    m.SetSpecularF(true);
    m.ShininessF = value;
    return m;
  }
};

struct sta_adjust_dummy {
  sta_adjust_dummy()
  {}
  TGlMaterial &operator()(TGlMaterial &m) const {
    return m;
  }
};

void GXLibMacros::macAdjustStyle(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  bool apply_to_atoms = Options.GetBoolOption("a", false, true),
    apply_to_bonds = Options.GetBoolOption("b", false, true);
  olxstr_set<true> primitives;
  {
    size_t idx = Cmds[0].IndexOf('.');
    if (idx != InvalidIndex) {
      TStrList toks(Cmds[0].SubStringFrom(idx + 1), ',');
      for (size_t i = 0; i < toks.Count(); i++) {
        primitives.Add(toks[i].TrimWhiteChars());
      }
    }
  }
  if (Cmds[0].StartsFromi("diffuse") ||
    Cmds[0].StartsFromi("specular") ||
    Cmds[0].StartsFromi("ambient") ||
    Cmds[0].StartsFromi("emission"))
  {
    int dest = Cmds[0].StartsFromi("ambient") ? 0
      : (Cmds[0].StartsFromi("diffuse") ? 1 :
        (Cmds[0].StartsFromi("specular") ? 2 : 3));
    if (Cmds[1].EndsWith('%')) {
      double pc = Cmds[1].SubStringFrom(0, 1).ToDouble()/100;
      if (pc < 0) {
        pc = 0;
      }
      adjust_style(app,
        sta_adjust_pc(pc, dest),
        apply_to_atoms, apply_to_bonds, primitives);
    }
    else if (Cmds[1].IsNumber()) {
      adjust_style(app,
        sta_adjust_cl(Cmds[1].SafeUInt<uint32_t>(), dest),
        apply_to_atoms, apply_to_bonds, primitives);
    }
    // copy from another primitive
    else {
      adjust_style(app,
        sta_adjust_dummy(),
        apply_to_atoms, apply_to_bonds, primitives, Cmds[1]);
    }
  }
  else if (Cmds[0].StartsFromi("shininess")) {
    adjust_style(app,
      sta_adjust_shininess(Cmds[1].ToInt()),
      apply_to_atoms, apply_to_bonds, primitives);
  }
}
//..............................................................................
void GXLibMacros::funObjectSettings(const TStrObjList &Params, TMacroData &E) {
  if (Params[0].Equalsi("atom")) {
    TXAtom::Settings &st = TXAtom::GetSettings(app.GetRenderer());
    if (Params[1].Equalsi("RimW")) {
      if (Params.Count() == 3) {
        st.SetRimW(Params[2].ToDouble());
      }
      else {
        E.SetRetVal(st.GetRimW());
      }
    }
    else if (Params[1].Equalsi("RimR")) {
      if (Params.Count() == 3) {
        st.SetRimR(Params[2].ToDouble());
      }
      else {
        E.SetRetVal(st.GetRimR());
      }
    }
    else if (Params[1].Equalsi("DiskS")) {
      if (Params.Count() == 3) {
        st.SetDiskS(Params[2].ToDouble());
      }
      else {
        E.SetRetVal(st.GetDiskS());
      }
    }
    else if (Params[1].Equalsi("DiskOR")) {
      if (Params.Count() == 3) {
        st.SetDiskOR(Params[2].ToDouble());
      }
      else {
        E.SetRetVal(st.GetDiskOR());
      }
    }
    else if (Params[1].Equalsi("DiskIR")) {
      if (Params.Count() == 3) {
        st.SetDiskIR(Params[2].ToDouble());
      }
      else {
        E.SetRetVal(st.GetDiskIR());
      }
    }
    app.GetRenderer().Clear();
    app.CreateObjects(false);
  }
}
//..............................................................................
void GXLibMacros::macTLS(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  TXAtomPList xatoms = app.FindXAtoms(Cmds, true, true);
  for (size_t i = 0; i < xatoms.Count(); i++) {
    if (xatoms[i]->GetEllipsoid() == 0) {
      xatoms[i] = 0;
    }
  }
  if (xatoms.Pack().Count() < 4) {
    Error.ProcessingError(__OlxSrcInfo, "at least 4 anisotropic atoms expected");
    return;
  }
  evecd_list original_q;
  vec3d_list original_crds;
  vec3d center = olx_mean(xatoms,
    FunctionAccessor::MakeConst<vec3d, TSAtom>(&TXAtom::crd));
  mat3d basis;
  basis.I();
  TAsymmUnit &au = app.XFile().GetAsymmUnit();
  if (Options.Contains('b') && xatoms.Count() > 2) {
    evecd Q(6);
    TBasicApp::NewLogEntry() << "Aligning x axis to: " <<
      xatoms[1]->GetGuiLabelEx() << " - " <<
      xatoms[0]->GetGuiLabelEx();
    basis[0] = (xatoms[1]->crd() - xatoms[0]->crd());
    TBasicApp::NewLogEntry() << "Aligning z axis perpendicular to plane formed"
      " by given atoms";
    basis[2] = basis[0].XProdVec(xatoms[2]->crd() - xatoms[0]->crd());
    basis[1] = basis[2].XProdVec(basis[0]);
    basis.Normalise();
    original_q.SetCapacity(xatoms.Count());
    original_crds.SetCapacity(xatoms.Count());
    sorted::PointerPointer<TEllipsoid> processed;
    for (size_t i = 0; i < xatoms.Count(); i++) {
      xatoms[i]->GetEllipsoid()->GetShelxQuad(original_q.AddNew(6));
      if (processed.AddUnique(xatoms[i]->GetEllipsoid()).b)
        xatoms[i]->GetEllipsoid()->Mult(basis);
      original_crds.AddCopy(xatoms[i]->crd());
      xatoms[i]->crd() = basis * (xatoms[i]->crd() - center) + center;
    }
  }
  mat3d basis_t = mat3d::Transpose(basis);

  xlib::TLS tls(TSAtomPList(xatoms, StaticCastAccessor<TSAtom>()));
  if (!Options.GetBoolOption('q')) {
    tls.printTLS(olxstr("TLS analysis for: ") << app.Label(xatoms));
    tls.printFOM();
    tls.printDiff();
  }

  if (!original_q.IsEmpty()) {
    for (size_t i = 0; i < xatoms.Count(); i++) {
      *xatoms[i]->GetEllipsoid() = original_q[i];
      xatoms[i]->crd() = original_crds[i];
    }
    tls.RotateElps(basis_t);
  }

  olxstr show = Options.FindValue('v');
  int q = Options.FindValue("quality", "5").ToUInt();
  if (q > 7) q = 7;
  glx_ext::XTLS xtls(
    Options.FindValue("start_color", "0xff0000").SafeUInt<uint32_t>(),
    Options.FindValue("end_color", "0x0000ff").SafeUInt<uint32_t>(),
    q,
    Options.FindValue("g", TrueString()).ToBool()
  );
  if (!show.IsEmpty()) {
    olxstr obj_type_str = Options.FindValue("type", "diff");
    short obj_type = obj_type_str.Equalsi("diff") ? glx_ext::xtls_obj_diff
      : glx_ext::xtls_obj_rmsd;
    xtls.CreateTLSObject(xatoms, tls,
      show.Equalsi("Uobs") ? glx_ext::xtls_diff_Obs_Tls
      : glx_ext::xtls_diff_Tls_Obs,
      Options.FindValue('s', "125").ToFloat(),
      obj_type
    );
  }
  olxstr out = Options.FindValue("o");
  if (!out.IsEmpty()) {
    TStrList toks(out, ',');
    if (toks.Count() != 2 ||
      !(toks[1].Equalsi("Utls") || toks[1].Equalsi("Uobs")))
    {
      Error.ProcessingError(__OlxSrcInfo, "Invalid -o option");
    }
    else {
      if (!toks[0].EndsWithi(".pov")) {
        toks[0] << ".pov";
      }
      xtls.CreatePovRayFile(xatoms, tls,
        toks[1].Equalsi("Uobs") ? glx_ext::xtls_diff_Obs_Tls
        : glx_ext::xtls_diff_Tls_Obs, toks[0]);
    }
  }
  if (Options.Contains('a') &&
    tls.GetElpList().Count() == xatoms.Count())
  {
    for (size_t i = 0; i < xatoms.Count(); i++) {
      xatoms[i]->GetEllipsoid()->Initialise(tls.GetElpList()[i]);
      if (xatoms[i]->GetMatrix().IsFirst())
        *xatoms[i]->CAtom().GetEllipsoid() = tls.GetElpList()[i];
    }
  }
}
//.............................................................................
void GXLibMacros::macUdiff(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TGXApp &app = TGXApp::GetInstance();
  TXAtomPList xatoms = app.FindXAtoms(Cmds, true, true);
  for (size_t i = 0; i < xatoms.Count(); i++) {
    if (xatoms[i]->GetEllipsoid() == 0) {
      xatoms[i] = 0;
    }
  }
  if (xatoms.Pack().Count() < 6 || (xatoms.Count() % 2) != 0) {
    Error.ProcessingError(__OlxSrcInfo,
      "at least 3 pairs of anisotropic atoms expected");
    return;
  }
  size_t aag = xatoms.Count() / 2;
  vec3f_alist crds(aag);
  TTypeList<olx_pair_t<TSAtom*, TSAtom*> > satomp(aag);
  TEllpPList u_from(aag), u_to(aag);
  for (size_t i = 0; i < aag; i++) {
    satomp[i].a = xatoms[i];
    crds[i] = xatoms[i]->crd();
    satomp[i].b = xatoms[i + aag];
    u_from[i] = xatoms[i]->GetEllipsoid();
    u_to[i] = new TEllipsoid(*xatoms[i + aag]->GetEllipsoid());
  }
  TNetwork::AlignInfo rv = TNetwork::GetAlignmentRMSD(satomp, false,
    TSAtom::weight_unit);
  mat3d m;
  QuaternionToMatrix(rv.align_out.quaternions[0], m);
  for (size_t i = 0; i < aag; i++) {
    u_to[i]->Mult(m);
  }
  int q = Options.FindValue("quality", "5").ToUInt();
  if (q > 7) {
    q = 7;
  }
  glx_ext::XTLS xtls(
    Options.FindValue("start_color", "0xff0000").SafeUInt<uint32_t>(),
    Options.FindValue("end_color", "0x0000ff").SafeUInt<uint32_t>(),
    q,
    Options.FindValue("g", TrueString()).ToBool()
  );
  olxstr obj_type_str = Options.FindValue("type", "diff");
  short obj_type = obj_type_str.Equalsi("diff") ? glx_ext::xtls_obj_diff
    : glx_ext::xtls_obj_rmsd;
  TDUserObj *obj = xtls.CreateUdiffObject(crds,
    u_from, u_to,
    Options.FindValue('s', obj_type == glx_ext::xtls_obj_diff ? "125" : "3").ToFloat(),
    "udiff",
    obj_type);
  if (Options.GetBoolOption('r')) {
    TGlMaterial m("85;0;4286611584;4290822336;64");
    m.SetColorMaterial(true);
    obj->SetMaterial(m);
    obj->GetPrimitives().GetStyle().SetMaterial("Object", m);
    obj->GetPrimitives().GetPrimitive(0).SetProperties(m);
  }
  u_to.DeleteItems(false);
}
//.............................................................................
void GXLibMacros::macMSDSView(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  olxstr col_name = "MSDS";
  TRMDSADP *obj = 0;
  bool add = false;
  TGPCollection *col = app.GetRenderer().FindCollection(col_name);
  if (col != 0 && col->ObjectCount() > 0) {
    col->ClearPrimitives();
    obj = dynamic_cast<TRMDSADP *>(&col->GetObject(0));
  }
  if (obj == 0) {
    obj = new TRMDSADP(app.GetRenderer(), col_name);
    add = true;
  }
  else {
    obj->SetVisible(true);
  }
  obj->SetScale(Options.FindValue("s", "1").ToDouble());
  obj->SetQuality(Options.FindValue("q", "5").ToInt());
  int otype = Options.FindValue('t', "rmsd").Equalsi("rmsd") ? TRMDSADP::type_rmsd
    : TRMDSADP::type_msd;
  obj->SetType(otype);
  olxstr anh_str = Options.FindValue("a");
  int anh_type = TRMDSADP::anh_all;
  if (anh_str.Equalsi("anh")) {
    anh_type = TRMDSADP::anh_anh;
  }
  else if (anh_str.Equalsi("none")) {
    anh_type = TRMDSADP::anh_none;
  }
  obj->SetAnhType(anh_type);
  obj->Create();
  if (add) {
    app.AddObjectToCreate(obj);
  }
}
//.............................................................................
void GXLibMacros::macLpln(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  vec3d idx(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble());
  const TAsymmUnit &au = app.XFile().GetAsymmUnit();
  mat3d cc = au.GetCellToCartesian();
  vec3d o(0);
  vec3d cps[6][4] = {
    {0, cc[0], cc[0] + cc[1], cc[1]}, // a-b
    {0, cc[0], cc[0] + cc[2], cc[2]}, // a-c
    {0, cc[1], cc[1] + cc[2], cc[2]}, // b-c
    {cc[2], cc[0] + cc[2], cc[0] + cc[1] + cc[2], cc[1] + cc[2]}, // a-b
    {cc[1], cc[0]+ cc[1], cc[0] + cc[1] + cc[2], cc[2] + cc[1]}, // a-c
    {cc[0], cc[1] + cc[0], cc[0] + cc[1] + cc[2], cc[2] + cc[0]}, // b-c
  };

  vec3d pn, p;
  if (idx[0] == 0) {
    if (idx[1] == 0) {
      p = cc[2] / idx[2];
      pn = cc[0].XProdVec(cc[1]);
    }
    else if (idx[2] == 0) {
      p = cc[1] / idx[1];
      pn = cc[0].XProdVec(cc[2]);
    }
    else {
      p = cc[1] / idx[1];
      pn = (cc[2] / idx[2] - p).XProdVec((cc[2] / idx[2] + cc[0]) - p);
    }
  }
  else if (idx[1] == 0) {
    if (idx[2] == 0) {
      p = cc[0] / idx[0];
      pn = cc[1].XProdVec(cc[2]);
    }
    else {
      p = cc[0] / idx[0];
      pn = (cc[2] / idx[2] - p).XProdVec((cc[2] / idx[0] + cc[1]) - p);
    }
  }
  else if (idx[2] == 0) {
    p = cc[0] / idx[0];
    pn = (cc[1] / idx[1] - p).XProdVec((cc[1] / idx[1] + cc[2]) - p);
  }
  else {
    p = cc[1] / idx[1];
    pn = (cc[2]/idx[2] - p).XProdVec(cc[0]/idx[0] - p);
  }
  vec3d_list ipts;
  for (size_t i = 0; i < 6; i++) {
    vec3d cpn = (cps[i][2] - cps[i][1]).XProdVec(cps[i][0] - cps[i][1]);
    double cdn = cpn.DotProd(cps[i][3]);
    vec3d lo, ld;
    if (!gl_alg<>::PlanePlaneIntersect(p, pn, cdn, cpn, lo, ld)) {
      continue;
    }
    for (size_t j = 0; j < 5; j++) {
      size_t k = j == 4 ? 0 : j + 1;
      vec3d clo = cps[i][j];
      vec3d cld = cps[i][k] - cps[i][j];
      vec3d pa, pb;
      if (!gl_alg<>::LineLineIntersect(lo, ld, clo, cld, pa, pb)) {
        continue;
      }
      if (pa.Equals(pb, 1e-6)) {
        double qd1 = olx_sqr(pa.DistanceTo(cps[i][j]) + pa.DistanceTo(cps[i][k]));
        double qd2 = cps[i][j].QDistanceTo(cps[i][k]);
        if (olx_feq(qd1, qd2, 1e-6)) {
          ipts.AddCopy(pa);
        }
      }
    }
  }
  QuickSorter::Sort(ipts, TComparableComparator());
  for (size_t i = 0; i < ipts.Count(); i++) {
    size_t j = i;
    while (++i < ipts.Count() && ipts[i].Equals(ipts[j], 1e-6)) {
      ipts.NullItem(i);
    }
  }
  ipts.Pack();
  if (ipts.Count() > 2) {
    vec3d centre = olx_mean(ipts);
    olx_object_ptr<vec3f_alist> normals = new vec3f_alist(1);
    normals()[0] = pn.Normalise();
    olx_plane::Sort(ipts, DummyAccessor(), centre, pn);
    TDUserObj *obj = new TDUserObj(app.GetRenderer(), sgloPolygon,
      olxstr("lpln_") << idx.ToString());
    obj->SetVertices(&vec3f_alist::FromList(ipts, DummyAccessor()).Release());
    obj->SetNormals(normals.release());
    olxstr dm = "255;4290805760;4290805760;4286611584;4286611584;4290822336;4290822336;24;24";
    obj->SetMaterial(TGlMaterial(dm));
    obj->Create();
    app.AddObjectToCreate(obj);
  }
}
