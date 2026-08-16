/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xcartoon_H
#define __olx_xcartoon_H
#include "gxbase.h"
#include "gdrawobject.h"
#include "glmaterial.h"
#include "cartoon_geom.h"

/* one polymer chain as a compiled display list. sgloCommandList and not
sgloTriangles: IsCompilable() covers only the quadric types, so a triangle
primitive re-emits every vertex through olx_gl on every frame - hundreds of
thousands of calls at protein scale. One object per chain rather than per
residue, the list being the unit of both drawing and rebuilding
*/
BeginGxlNamespace()

/* How a chain is coloured. Applied per residue except for ccChain, which is
one colour for the whole segment.
*/
const short
  ccChain = 0,        // one colour per chain
  ccIndex = 1,        // rainbow along the chain, N terminus blue
  ccPolarity = 2,     // lipophilic or hydrophilic
  ccUeq = 3,          // mean Ueq of the residue, ramped over the model
  ccSecondary = 4,    // helix, strand and coil
  ccCharge = 5,       // acidic, basic or neutral
  ccResidue = 6;      // one colour per amino acid

/* What to do with everything the backbone trace did not claim: waters, ions,
sugars, ligands, and anything with no RESI at all.
*/
const short
  cnpHide = 0,        // the cartoon on its own
  cnpShow = 1,        // all of it, as atoms and bonds in the current style
  cnpNoWater = 2;     // ligands and metals, but not the waters

/* The colours each mode uses, shared so that the key drawn in the legend is
the same code that colours the ribbon rather than a copy of it that can drift.
*/
namespace cartoon_colour {
  // one per chain, cycled; keyed on the chain letter by the caller
  uint32_t Chain(size_t index);
  size_t ChainCount();
  // xlib::protein::ResiduePolarity
  uint32_t Polarity(short p);
  // xlib::protein::ResidueCharge
  uint32_t Charge(short c);
  // xlib::protein::ResidueIndex, or InvalidIndex for anything else
  uint32_t Residue(size_t index);
  // xlib::protein::ss_*
  uint32_t Secondary(short ss);
  /* the ribbon's material for one colour, built here so the legend key and the
  ribbon cannot drift - an unlit swatch reads as a different colour. Colour
  material off: the ribbon's list emits a glColor per residue, a swatch has
  nothing to emit one. The ambient stays dark; at full colour it saturates to
  white under these lights
  */
  TGlMaterial RibbonMaterial(uint32_t colour);
}

class TXCartoon : public AGDrawObject {
  cartoon::Mesh mesh;
  // baked into the list, so a colour change is a rebuild
  uint32_t colour;
  /* optional, one per residue; at the right length the list emits a colour
  change per residue boundary instead of one for the chain
  */
  TArrayList<uint32_t> residue_colours;
  /* TResidue ids, in the same order, so recolouring and the residue subset
  work can reach the model without re-running the topology.
  */
  TArrayList<size_t> residue_ids;
  /* TCAtom ids of the backbone this ribbon stands for, four per residue -
  N, CA, C, O - InvalidIndex where a role was not filled. A residue showing its
  sidechain must not draw the backbone as sticks too, inside its own ribbon.
  Ids because the classification is topological, not by label
  */
  TArrayList<size_t> backbone_atom_ids;
  /* CA of each residue, in order. The ribbon is one mesh with no per-residue
  objects, so a click and a residue's share of the centre resolve against this
  */
  TArrayList<vec3d> residue_ca;
  // the selecting pass's stand-in for the ribbon; see DrawPickProxy
  int pick_slices;
  double pick_radius;
  // xlib::protein::ss_* per residue, in the same order
  TArrayList<short> residue_ss;
  /* per residue, empty draws all - the substructure view. The per-residue
  triangle table makes a subset a re-emission and nothing else
  */
  TArrayList<bool> residue_visible;
  /* per residue, empty when nothing here is selected. Baked into the same
  list as the colours, so a selection costs a re-emission and no per-frame work
  */
  TArrayList<bool> residue_selected;
  TGlPrimitive *list;
  vec3d min_dim, max_dim;
  bool has_dims;
  // the driver rejected the list, so nothing of this chain will be drawn
  mutable bool list_failed;
  olxch chain_id;
  /* which matrix generated this copy, 0 for the asymmetric unit's own. A
  generated copy is its own segment and sits elsewhere, which a click has to
  distinguish
  */
  uint32_t matrix_id;
  void EmitList(TGlPrimitive &p) const;
  void EmitTriangles(size_t from, size_t to) const;
  static void EmitColour(uint32_t cl);
  void UpdateDimensions();
public:
  TXCartoon(TGlRenderer &Render, const olxstr &collectionName);
  virtual ~TXCartoon() {}

  void Create(const olxstr &cName = EmptyString());
  bool Orient(TGlPrimitive &P);
  bool GetDimensions(vec3d &Max, vec3d &Min);
  vec3d CalcCenter() const;

  /* replaces the geometry with a cartoon through the segment's CA, assigning
  secondary structure on the way. Before Create(); after it use Rebuild()
  */
  void BuildFrom(const xlib::protein::ChainSegment &seg,
    const cartoon::CartoonParams &p);

  /* re-emits the list from the current mesh, same list id - the path for a
  colour or subset change, touching neither collection nor style
  */
  void Rebuild();

  const cartoon::Mesh &GetMesh() const { return mesh; }
  cartoon::Mesh &GetMesh() { return mesh; }
  olxch GetChainId() const { return chain_id; }
  uint32_t GetMatrixId() const { return matrix_id; }
  size_t TriangleCount() const { return mesh.TriangleCount(); }

  /* Four per residue, in the order of GetResidueIds: N, CA, C, O, with
  InvalidIndex where a role was not filled.
  */
  const TArrayList<size_t>& GetBackboneAtomIds() const {
    return backbone_atom_ids;
  }
  uint32_t GetColour() const { return colour; }
  // takes effect at the next Create() or Rebuild(), the colour being baked in
  void SetColour(uint32_t v) { colour = v; }

  // one entry per residue of the segment; an empty list restores the flat colour
  void SetResidueColours(const TArrayList<uint32_t> &c) {
    residue_colours = c;
  }
  size_t ResidueCount() const {
    return mesh.residue_triangle_offset.IsEmpty()
      ? 0 : mesh.residue_triangle_offset.Count() - 1;
  }
  const TArrayList<size_t>& GetResidueIds() const { return residue_ids; }
  const TArrayList<short>& GetResidueSS() const { return residue_ss; }
  const TArrayList<vec3d>& GetResidueCA() const { return residue_ca; }
  /* which residue a line passes closest to, InvalidIndex if none within
  tolerance. Against the CA rather than the mesh: consecutive CA are 3.8 A
  apart and the ribbon about 2 A wide, so the nearest CA is the residue under
  the cursor unless the view is edge-on, at a dot product per residue. dir need
  not be normalised; dist is how far along the line, for comparing chains
  */
  size_t HitResidue(const vec3d &from, const vec3d &dir, double &dist,
    double tolerance = 2.5) const;
  /* what the selecting pass sees: a tube of pick_slices sides and pick_radius
  along the CA trace. Coarse on purpose - it only says which chain, HitResidue
  settles the residue - so it is emitted per click rather than held as a second
  list. More slices if clicks near a wide ribbon's edge feel unreliable
  */
  void DrawPickProxy() const;
  /* which residue is nearest a point on the projection plane, InvalidIndex if
  none within tolerance. The other direction from HitResidue: the renderer can
  project a world point but not unproject a screen one. q is how far away
  */
  size_t HitResidueProjected(double px, double py, double &q,
    double tolerance = 1.0) const;
  /* logs the residues competing for a click and the depth Project gives them.
  Temporary, to settle which way round that depth runs
  */
  void ReportHitCandidates(double px, double py, size_t chosen) const;
  /* mouse-down only claims the event; the selection happens on release, and
  only if the button came up where it went down - otherwise every drag starting
  on a ribbon selects as well as rotating. TGlMouse's own fallback does the
  same, gated on IsClick
  */
  virtual bool OnMouseDown(const IOlxObject *sender, const TMouseData &d);
  virtual bool OnMouseUp(const IOlxObject *sender, const TMouseData &d);
  int GetPickSlices() const { return pick_slices; }
  void SetPickSlices(int v) { pick_slices = v; }
  double GetPickRadius() const { return pick_radius; }
  void SetPickRadius(double v) { pick_radius = v; }
  /* toggles one residue in the selection, returning true when the state
  changed so the caller can skip a rebuild. Kept here rather than in the
  renderer's selection group: a group holds whole objects, and to the renderer
  this chain is one - a click on a turn of a helix never meant all of it
  */
  bool ToggleResidue(size_t i);
  // true when there was something to clear
  bool ClearResidueSelection();
  size_t SelectedResidueCount() const;
  const TArrayList<bool>& GetResidueSelection() const {
    return residue_selected;
  }
  // one entry per residue, or empty to draw the whole chain again
  void SetResidueVisibility(const TArrayList<bool> &v) {
    residue_visible = v;
  }
  bool HasVisibleResidues() const;
  bool DidListFail() const { return list_failed; }

  /* reads and discards every pending GL error, returning the count. GL latches
  them and this is the only place in Olex2 that reads them, so an error raised
  anywhere earlier is waiting to be blamed on whatever checks first
  */
  static size_t DrainGlErrors();
};

EndGxlNamespace()
#endif
