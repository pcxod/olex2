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

/* One polymer chain drawn as a compiled display list.

The mesh comes from cartoon_geom and the topology from xlib/protein.h; this
only owns the geometry and hands it to OpenGL once. The primitive is
sgloCommandList rather than sgloTriangles because TGlPrimitive::IsCompilable()
covers only the quadric types: a sgloTriangles primitive re-emits every vertex
and normal through olx_gl on every frame, which at protein scale is hundreds of
thousands of calls per frame. A command list is emitted once and drawn with a
single callList.

One object per chain, not per residue: the display list is the unit of both
drawing and rebuilding, and a per-residue list would give away the whole
advantage in call overhead.
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
  /* The material the ribbon is drawn with, for one colour.

  The legend's swatch is a sphere and the ribbon is a mesh, but they have to
  read as the same colour, and a flat unlit swatch does not. The ribbon carries
  a specular highlight and is lit; a swatch with an ambient term only comes out
  flatter and more saturated, which is far enough away to look like a different
  colour rather than the same one. Built here for the same reason the colours
  above are, so the key and the thing it is a key to cannot drift.

  Colour material is left off: the ribbon turns it on because its list emits a
  glColor per residue, whereas a swatch has nothing to emit one and would take
  whatever colour happened to be current.

  The ambient term is deliberately dark rather than the colour. Under this
  scene's lights an ambient at full colour saturates to white, which on a white
  background is an empty window - so this is the ribbon's material as it stands
  and not a tidied-up version of it.
  */
  TGlMaterial RibbonMaterial(uint32_t colour);
}

class TXCartoon : public AGDrawObject {
  cartoon::Mesh mesh;
  // baked into the list, so a colour change is a rebuild
  uint32_t colour;
  /* Optional, one per residue. When it is the right length the list emits a
  colour change at each residue boundary instead of one for the whole chain,
  which is the per-residue table in the mesh finding its first consumer.
  */
  TArrayList<uint32_t> residue_colours;
  /* TResidue ids, in the same order, so recolouring and the residue subset
  work can reach the model without re-running the topology.
  */
  TArrayList<size_t> residue_ids;
  /* TCAtom ids of the backbone atoms this ribbon stands for: four per residue,
  N, CA, C, O in that order, InvalidIndex where a role was not filled.

  A ribbon is a picture of the backbone, so a focused residue showing its
  sidechain as sticks must not also draw the backbone as sticks - that puts a
  second copy of it inside the ribbon. Ids because the classification is
  topological: crambin's first residue is N3, C6, C7, O3, and a label test
  finds no backbone in it at all.
  */
  TArrayList<size_t> backbone_atom_ids;
  /* CA of each residue, in the same order. The ribbon is a single mesh with no
  per-residue objects in it, so this is what a click has to be resolved
  against, and what a residue's share of the model centre acts at.
  */
  TArrayList<vec3d> residue_ca;
  // the selecting pass's stand-in for the ribbon; see DrawPickProxy
  int pick_slices;
  double pick_radius;
  // xlib::protein::ss_* per residue, in the same order
  TArrayList<short> residue_ss;
  /* Per residue; an empty list draws all of them. This is the substructure
  view: the per-residue triangle table means a subset costs a re-emission of
  the display list and nothing else - no new geometry, no new topology.
  */
  TArrayList<bool> residue_visible;
  /* Per residue, or empty when nothing in this chain is selected. The highlight
  is baked into the same display list as the colours, so selecting a residue
  costs a re-emission and nothing else - no second pass over the ribbon, no
  overlay geometry, and no per-frame work once the click is over.
  */
  TArrayList<bool> residue_selected;
  TGlPrimitive *list;
  vec3d min_dim, max_dim;
  bool has_dims;
  // the driver rejected the list, so nothing of this chain will be drawn
  mutable bool list_failed;
  olxch chain_id;
  /* Which symmetry matrix generated this copy of the chain, 0 for the
  asymmetric unit's own. A generated copy is traced as its own segment and sits
  somewhere else entirely, so a click landing on one is the difference between
  "that residue" and "a residue that looks rotated in space".
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

  /* Replaces the geometry with a cartoon through the segment's CA positions,
  assigning secondary structure on the way. Must be called before Create();
  afterwards use Rebuild().
  */
  void BuildFrom(const xlib::protein::ChainSegment &seg,
    const cartoon::CartoonParams &p);

  /* Re-emits the display list from the current mesh, keeping the same list id.
  This is the path for a colour or subset change; it does not touch the
  collection or the style.
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
  /* Which residue a line through the scene passes closest to, or InvalidIndex
  if it passes no nearer than `tolerance` to any of them.

  Resolved against the CA positions rather than against the mesh. Consecutive
  CA are 3.8 A apart and the ribbon is about 2 A wide, so the nearest CA to the
  line is the residue under the cursor for any view that is not almost edge-on
  along the chain - and it costs a dot product per residue instead of a
  triangle intersection per triangle.

  `from` and `dir` are in the same absolute Cartesian frame the mesh is in;
  `dir` need not be normalised. On a hit, `dist` is how far along the line the
  residue lies, so a caller can keep the nearest of several chains.
  */
  size_t HitResidue(const vec3d &from, const vec3d &dir, double &dist,
    double tolerance = 2.5) const;
  /* The shape the selecting pass sees: a tube of `pick_slices` sides and
  `pick_radius` A along the CA trace. Coarse on purpose - it only has to say
  which chain is under the cursor, and HitResidue settles the residue - so it
  is a fraction of the real mesh and cheap enough to emit per click rather than
  holding a second display list. Turn the slices up if clicks near the edge of
  a wide helix ribbon feel unreliable, or the radius up to make the whole
  ribbon easier to hit.
  */
  void DrawPickProxy() const;
  /* Which residue lies nearest a point on the projection plane, or
  InvalidIndex if none is within `tolerance` of it.

  The other direction from HitResidue: the renderer can project a world point
  but has nothing to unproject a screen one, so the residues are brought to the
  cursor rather than a ray taken into the scene. `q` returns how far away the
  winner was, so a caller comparing chains can keep the closest.
  */
  size_t HitResidueProjected(double px, double py, double &q,
    double tolerance = 1.0) const;
  /* Logs the residues competing for one click and the depth Project gives
  them, with the chosen one starred. Temporary: it is here to settle which way
  round that depth runs, because assuming it made picking worse.
  */
  void ReportHitCandidates(double px, double py, size_t chosen) const;
  /* Mouse-down only claims the event; the selection happens on mouse-up, and
  only if the button came up where it went down.

  Selecting on the press made every drag that started on a ribbon select
  something as well as rotating, which is not what a drag means anywhere else
  in Olex2 - and it put a redraw inside the press handler, which is not where
  the rest of the renderer draws from. TGlMouse's own selection fallback works
  the same way: on release, gated on IsClick.
  */
  virtual bool OnMouseDown(const IOlxObject *sender, const TMouseData &d);
  virtual bool OnMouseUp(const IOlxObject *sender, const TMouseData &d);
  int GetPickSlices() const { return pick_slices; }
  void SetPickSlices(int v) { pick_slices = v; }
  double GetPickRadius() const { return pick_radius; }
  void SetPickRadius(double v) { pick_radius = v; }
  /* Adds one residue of this chain to the selection, or takes it back out if
  it was already in it. Returns true when the state changed, so the caller can
  skip a rebuild that would draw the same thing.

  Toggling rather than replacing, and with no modifier key: that is what
  clicking does everywhere else in Olex2, and a selection built one residue at a
  time is the whole point of picking on a ribbon.

  Selection is per residue and lives here rather than in the renderer's
  selection group because a group holds whole objects: to the renderer this
  chain is one object, and a click on one turn of a helix never meant all of it.
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

  /* Reads and discards every pending GL error, returning how many there were.

  This is the only place in Olex2 that calls glGetError at all, and GL latches
  errors until somebody reads them. So an error raised anywhere, at any earlier
  point, is still sitting there waiting to be blamed on whatever checks first -
  which is exactly what happened when the cartoon started checking. Draining
  before a check is what makes the check mean anything.
  */
  static size_t DrainGlErrors();
};

EndGxlNamespace()
#endif
