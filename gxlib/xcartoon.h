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

/* one polymer chain as one compiled display list, the unit of drawing and
rebuilding. sgloCommandList, not sgloTriangles: IsCompilable() covers only the
quadric types, so a triangle primitive re-emits every vertex on every frame
*/
BeginGxlNamespace()

// how a chain is coloured: per residue, except ccChain, one for the segment
const short
  ccChain = 0,        // one colour per chain
  ccIndex = 1,        // rainbow along the chain, N terminus blue
  ccPolarity = 2,     // lipophilic or hydrophilic
  ccUeq = 3,          // mean Ueq of the residue, ramped over the model
  ccSecondary = 4,    // helix, strand and coil
  ccCharge = 5,       // acidic, basic or neutral
  ccResidue = 6;      // one colour per amino acid

/* what to do with everything the backbone trace did not claim: waters, ions,
sugars, ligands, and anything with no RESI at all
*/
const short
  cnpHide = 0,        // the cartoon on its own
  cnpShow = 1,        // all of it, as atoms and bonds in the current style
  cnpNoWater = 2;     // ligands and metals, but not the waters

// the colours each mode uses, shared by the ribbon and the legend key
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
  /* the ribbon's material for one colour, shared with the legend key. Colour
  material off: the ribbon's list emits a glColor per residue. The ambient stays
  dark; at full colour it saturates to white under these lights
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
  reach the model without re-running the topology
  */
  TArrayList<size_t> residue_ids;
  /* TCAtom ids (topological, not by label) of the backbone this ribbon stands
  for, N, CA, C, O per residue, InvalidIndex where unfilled; a residue showing
  its sidechain must not also draw these as sticks inside its ribbon
  */
  TArrayList<size_t> backbone_atom_ids;
  /* CA of each residue, in order; the ribbon has no per-residue objects, so
  clicks and the centre resolve against this
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
  // the matrix that generated this copy, 0 for the asymmetric unit's own
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

  // N, CA, C, O per residue in GetResidueIds order, InvalidIndex where unfilled
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
  tolerance. Against the CA, not the mesh: CA are 3.8 A apart and the ribbon
  ~2 A wide, so the nearest CA is the one under the cursor unless edge-on. dir
  need not be normalised; dist is how far along the line, for comparing chains
  */
  size_t HitResidue(const vec3d &from, const vec3d &dir, double &dist,
    double tolerance = 2.5) const;
  /* what the selecting pass sees: a tube of pick_slices sides and pick_radius
  along the CA trace, emitted per click. Coarse on purpose: it only picks the
  chain, HitResidue the residue
  */
  void DrawPickProxy() const;
  /* which residue is nearest a point on the projection plane, InvalidIndex if
  none within tolerance. The other direction from HitResidue: the renderer can
  project a world point but not unproject a screen one. q is how far away
  */
  size_t HitResidueProjected(double px, double py, double &q,
    double tolerance = 1.0) const;
  // logs the residues competing for a click and their Project depth (debug)
  void ReportHitCandidates(double px, double py, size_t chosen) const;
  /* mouse-down only claims the event; release selects, and only if it is a
  click, so a drag starting on a ribbon just rotates (as TGlMouse's IsClick)
  */
  virtual bool OnMouseDown(const IOlxObject *sender, const TMouseData &d);
  virtual bool OnMouseUp(const IOlxObject *sender, const TMouseData &d);
  int GetPickSlices() const { return pick_slices; }
  void SetPickSlices(int v) { pick_slices = v; }
  double GetPickRadius() const { return pick_radius; }
  void SetPickRadius(double v) { pick_radius = v; }
  /* toggles one residue in the selection, true when the state changed. Kept
  here, not in the renderer's selection group, which holds whole chains
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

  /* reads and discards every pending GL error, returning the count; GL latches
  them and nothing else in Olex2 reads them, so earlier errors wait here
  */
  static size_t DrainGlErrors();
};

EndGxlNamespace()
#endif
