/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xcartoon.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "glmouse.h"
#include "gpcollection.h"
#include "styles.h"
// for the residue's name and number, and to clear the other chains' selections
#include "gxapp.h"
#include "residue.h"

// the member definitions below resolve through the header's using-directive,
// but a free function has to be put in the namespace explicitly
BeginGxlNamespace()
namespace cartoon_colour {
  // clear of the background and of the CPK colours, so sidechain sticks drawn
  // over the ribbon stay readable. OLX_RGBA packs as 0xAABBGGRR
  static const uint32_t chain_colours[] = {
    0xffcc9966, 0xff6699cc, 0xff66cc99, 0xff9966cc,
    0xff99cccc, 0xffcc6699, 0xff99cc66, 0xffcccc66
  };

  size_t ChainCount() {
    return sizeof(chain_colours)/sizeof(chain_colours[0]);
  }
  uint32_t Chain(size_t index) {
    return chain_colours[index % ChainCount()];
  }
  // warm for the interior, cool for the solvent, as hydrophobicity figures are
  // drawn elsewhere
  uint32_t Polarity(short p) {
    using namespace xlib::protein;
    switch (p) {
      case rpLipophilic:  return 0xff78bee6;   // warm sand
      case rpHydrophilic: return 0xffe6aa6e;   // cool blue
      default:            return 0xff909090;   // unknown, mid grey
    }
  }
  // acidic red, basic blue, neutral pale
  uint32_t Charge(short c) {
    using namespace xlib::protein;
    switch (c) {
      case rcAcidic:  return 0xff4040e0;   // red
      case rcBasic:   return 0xffe06040;   // blue
      case rcNeutral: return 0xffc8c8c8;   // pale grey
      default:        return 0xff909090;   // unknown, mid grey
    }
  }
  /* One per amino acid, in the alphabetical order xlib::protein indexes them.
  Chosen for separation rather than for meaning; none is dark, the ribbon's
  ambient term being dark enough to take a dark diffuse colour to near black.
  */
  static const uint32_t residue_colours[] = {
    0xff4b19e6, 0xff4bb43c, 0xff19e1ff, 0xffc88200, 0xff3082f5,
    0xffb41e91, 0xfff0f046, 0xffe632f0, 0xff3cf5d2, 0xffd4befa,
    0xff808000, 0xffffbedc, 0xff286eaa, 0xffc8faff, 0xff5050c8,
    0xffc3ffaa, 0xff3cb4b4, 0xffb4d7ff, 0xffdc6464, 0xff808080
  };
  uint32_t Residue(size_t index) {
    const size_t n = sizeof(residue_colours)/sizeof(residue_colours[0]);
    return index < n ? residue_colours[index] : 0xff909090;
  }
  // helix red, strand yellow, coil grey
  uint32_t Secondary(short ss) {
    using namespace xlib::protein;
    if (ss == ss_helix)  { return 0xff4040e0; }
    if (ss == ss_strand) { return 0xff40e0f0; }
    return 0xffc0c0c0;
  }
  TGlMaterial RibbonMaterial(uint32_t colour) {
    TGlMaterial m;
    m.SetFlags(sglmAmbientF | sglmDiffuseF | sglmSpecularF | sglmShininessF |
      sglmAmbientB | sglmDiffuseB | sglmSpecularB | sglmShininessB);
    // only the diffuse carries the colour: raising the ambient to it as well
    // saturates the ribbon to white under this scene's lights
    m.AmbientF = 0xff202020;
    m.DiffuseF = colour;
    m.SpecularF = 0xff909090;
    m.ShininessF = 32;
    m.AmbientB = m.AmbientF;
    m.DiffuseB = m.DiffuseF;
    m.SpecularB = m.SpecularF;
    m.ShininessB = m.ShininessF;
    m.SetTransparent(false);
    return m;
  }
}
EndGxlNamespace()
//.............................................................................
TXCartoon::TXCartoon(TGlRenderer &R, const olxstr &collectionName)
  : AGDrawObject(R, collectionName),
    // OLX_RGBA packs as 0xAABBGGRR, so this is a muted steel blue
    colour(0xffcc9966),
    list(0),
    min_dim(0), max_dim(0),
    has_dims(false),
    list_failed(false),
    // wider than the coil tube, narrower than a helix ribbon: the middle of any
    // secondary structure is hit, only the outer edge of a wide ribbon missed
    pick_slices(6),
    pick_radius(1.6),
    chain_id(' '),
    matrix_id(0)
{
  /* Clicks still arrive: SelectObject maps the picking colour back to an object
  without consulting either flag, and OnMouseUp resolves the hit to a residue.
  What selectable would add is the whole chain entering the renderer's selection
  group, which selects 140 residues at once and, since ~AGDrawObject is empty and
  RemoveObject touches only TGlRenderer::FGObjects, leaves a freed pointer there
  whenever a focus, colour or representation change rebuilds the object.
  TGlGroup::Add refuses a non-selectable object, closing the click fallback,
  'sel -a' and box select alike.
  */
  SetSelectable(false);
  SetGroupable(false);
}
//.............................................................................
void TXCartoon::BuildFrom(const xlib::protein::ChainSegment &seg,
  const cartoon::CartoonParams &p)
{
  mesh.Clear();
  residue_colours.Clear();
  residue_ids.SetCount(seg.residues.Count());
  residue_ca.SetCount(seg.residues.Count());
  backbone_atom_ids.SetCount(seg.residues.Count()*4);
  for (size_t i = 0; i < seg.residues.Count(); i++) {
    const xlib::protein::BackboneResidue &r = seg.residues[i];
    residue_ids[i] = r.resi_id;
    residue_ca[i] = r.ca;
    backbone_atom_ids[i*4 + 0] = r.n_id;
    backbone_atom_ids[i*4 + 1] = r.ca_id;
    backbone_atom_ids[i*4 + 2] = r.c_id;
    backbone_atom_ids[i*4 + 3] = r.o_id;
  }
  chain_id = seg.chain_id;
  matrix_id = seg.matrix_id;
  xlib::protein::AssignSecondaryStructure(seg, residue_ss);
  cartoon::BuildCartoon(seg, residue_ss, p, mesh);
  UpdateDimensions();
}
//.............................................................................
void TXCartoon::UpdateDimensions() {
  has_dims = false;
  if (mesh.vertices.IsEmpty()) {
    return;
  }
  vec3f mn = mesh.vertices[0], mx = mesh.vertices[0];
  for (size_t i = 1; i < mesh.vertices.Count(); i++) {
    vec3f::UpdateMinMax(mesh.vertices[i], mn, mx);
  }
  min_dim = mn;
  max_dim = mx;
  has_dims = true;
}
//.............................................................................
void TXCartoon::EmitColour(uint32_t cl) {
  olx_gl::color((float)OLX_GetRValue(cl) / 255,
    (float)OLX_GetGValue(cl) / 255,
    (float)OLX_GetBValue(cl) / 255,
    (float)OLX_GetAValue(cl) / 255);
}
//.............................................................................
void TXCartoon::EmitTriangles(size_t from, size_t to) const {
  for (size_t i = from; i < to; i++) {
    const IndexTriangle &t = mesh.triangles[i];
    for (size_t j = 0; j < 3; j++) {
      const size_t vi = t.vertices[j];
      olx_gl::normal(mesh.normals[vi]);
      olx_gl::vertex(mesh.vertices[vi]);
    }
  }
}
//.............................................................................
size_t TXCartoon::DrainGlErrors() {
  size_t n = 0;
  // bounded: a broken or lost context can report an error indefinitely
  while (n < 64 && glGetError() != GL_NO_ERROR) {
    n++;
  }
  return n;
}
//.............................................................................
void TXCartoon::EmitList(TGlPrimitive &p) const {
  // so that what is read afterwards is this list's doing and nothing else's
  DrainGlErrors();
  list_failed = false;
  const size_t rc = ResidueCount();
  const bool per_residue = (residue_colours.Count() == rc && rc != 0);
  p.StartList();
  olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  // one begin/end for the whole chain: a colour change between glBegin and
  // glEnd is legal, so the per-residue modes stay a single primitive
  const bool subset = (residue_visible.Count() == rc && rc != 0);
  const bool selected = (residue_selected.Count() == rc && rc != 0);
  /* The renderer's own selection colour, and its ambient rather than its
  diffuse: the selection group is drawn semi-transparent with both set, and it
  is the ambient that carries the green: the diffuse is red (glgroup.cpp:44-45).
  */
  const uint32_t sel_colour =
    Parent.GetSelection().GetGlM().AmbientF.GetRGB() | 0xff000000;
  olx_gl::begin(GL_TRIANGLES);
  if (per_residue || subset || selected) {
    uint32_t emitted = 0;
    bool any_emitted = false;
    if (!per_residue && !selected) {
      EmitColour(colour);
    }
    for (size_t i = 0; i < rc; i++) {
      if (subset && !residue_visible[i]) {
        continue;
      }
      const size_t from = mesh.residue_triangle_offset[i],
        to = mesh.residue_triangle_offset[i+1];
      if (from >= to) {
        continue;      // the last residue closes the tube and owns no stretch
      }
      if (per_residue || selected) {
        const uint32_t c = (selected && residue_selected[i])
          ? sel_colour : (per_residue ? residue_colours[i] : colour);
        // a run of same-coloured residues is the common case, so do not repeat
        if (!any_emitted || c != emitted) {
          EmitColour(c);
          emitted = c;
          any_emitted = true;
        }
      }
      EmitTriangles(from, to);
    }
  }
  else {
    EmitColour(colour);
    EmitTriangles(0, mesh.triangles.Count());
  }
  olx_gl::end();
  p.EndList();
  /* A list that fails to compile draws nothing, which is indistinguishable from
  a fault in the geometry unless it is reported. GL_INVALID_VALUE means the list
  id was rejected - a renderer problem - while a size problem gives
  GL_OUT_OF_MEMORY.
  */
  const GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    TBasicApp::NewLogEntry(logError) << "Cartoon: chain " << chain_id <<
      " could not compile its display list of " << mesh.triangles.Count() <<
      " triangles, GL error " << (int)err <<
      (err == GL_OUT_OF_MEMORY
        ? olxstr(" (out of memory: try 'cartoon -r=trace', about half the"
          " triangles)")
        : olxstr(" (the list id was rejected, which is a renderer problem"
          " rather than a size one)"));
    list_failed = true;
  }
}
//.............................................................................
void TXCartoon::Create(const olxstr &cName) {
  SetCreated(true);
  if (!cName.IsEmpty()) {
    SetCollectionName(cName);
  }
  olxstr NewL;
  TGPCollection *GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if (GPC == 0) {
    GPC = &Parent.NewCollection(NewL);
  }
  GPC->AddObject(*this);
  if (GPC->PrimitiveCount() != 0) {
    /* A collection left over from a previous model still carries a list built
    from that model's coordinates. Re-emitting into it is both correct and
    cheaper than allocating another one: taking it as it stands would draw the
    old structure.
    */
    list = &GPC->GetPrimitive(0);
    if (!mesh.triangles.IsEmpty()) {
      EmitList(*list);
    }
    return;
  }
  if (mesh.triangles.IsEmpty()) {
    return;
  }
  TGraphicsStyle &GS = GPC->GetStyle();
  // the mesh must not reach the style file - TDUserObj writes every vertex as
  // text - so the geometry is regenerated from the model instead
  GS.SetSaveable(false);

  TGlMaterial m = cartoon_colour::RibbonMaterial(colour);
  // the list emits a glColor per residue, which is what this lets through
  m.SetColorMaterial(true);

  list = &GPC->NewPrimitive("Cartoon", sgloCommandList);
  list->SetProperties(m);
  EmitList(*list);
}
//.............................................................................
size_t TXCartoon::HitResidue(const vec3d &from, const vec3d &dir, double &dist,
  double tolerance) const
{
  const double dd = dir.QLength();
  if (dd < 1e-12 || residue_ca.IsEmpty()) {
    return InvalidIndex;
  }
  const bool subset = (residue_visible.Count() == residue_ca.Count());
  const double t2 = tolerance*tolerance;
  size_t best = InvalidIndex;
  double best_q = 0;
  for (size_t i = 0; i < residue_ca.Count(); i++) {
    // not drawn, so not on screen to be clicked
    if (subset && !residue_visible[i]) {
      continue;
    }
    const vec3d v = residue_ca[i] - from;
    const double t = v.DotProd(dir) / dd;
    if (t <= 0) {
      continue;                   // behind the eye
    }
    const double q = (v - dir*t).QLength();
    if (q > t2) {
      continue;
    }
    // nearest along the line, not nearest to it: where the chain doubles back
    // two residues fall within the tolerance and the front one was clicked
    if (best == InvalidIndex || t < best_q) {
      best = i;
      best_q = t;
    }
  }
  if (best != InvalidIndex) {
    dist = best_q;
  }
  return best;
}
//.............................................................................
bool TXCartoon::HasVisibleResidues() const {
  if (residue_visible.IsEmpty()) {
    return !mesh.triangles.IsEmpty();
  }
  for (size_t i = 0; i < residue_visible.Count(); i++) {
    if (residue_visible[i]) {
      return true;
    }
  }
  return false;
}
//.............................................................................
void TXCartoon::Rebuild() {
  UpdateDimensions();
  if (list == 0) {
    return;
  }
  // redefining an existing list id replaces its contents
  EmitList(*list);
}
//.............................................................................
size_t TXCartoon::HitResidueProjected(double px, double py, double &q,
  double tolerance) const
{
  const bool subset = (residue_visible.Count() == residue_ca.Count());
  const double t2 = tolerance*tolerance;
  /* Nearest CA on the projection plane. Where a chain doubles back two
  stretches of ribbon share a pixel and the frontmost is the one clicked, but
  the third component Project() returns is in projection units rather than
  angstroems and its sign convention is not established, so depth is reported
  by ReportHitCandidates rather than used here.
  */
  size_t best = InvalidIndex;
  double best_q = 0;
  for (size_t i = 0; i < residue_ca.Count(); i++) {
    if (subset && !residue_visible[i]) {
      continue;
    }
    const vec3d p = Parent.Project(residue_ca[i]);
    const double dx = p[0] - px, dy = p[1] - py;
    const double d2 = dx*dx + dy*dy;
    if (d2 > t2) {
      continue;
    }
    if (best == InvalidIndex || d2 < best_q) {
      best = i;
      best_q = d2;
    }
  }
  if (best != InvalidIndex) {
    q = best_q;
  }
  return best;
}
//.............................................................................
void TXCartoon::ReportHitCandidates(double px, double py, size_t chosen) const {
  // the residues competing for one click and the depth Project reports, so that
  // the sign and scale of that third component can be established from real use
  const bool subset = (residue_visible.Count() == residue_ca.Count());
  TArrayList<olx_pair_t<double, size_t> > cand;
  for (size_t i = 0; i < residue_ca.Count(); i++) {
    if (subset && !residue_visible[i]) {
      continue;
    }
    const vec3d p = Parent.Project(residue_ca[i]);
    const double dx = p[0] - px, dy = p[1] - py;
    cand.Add(olx_pair_t<double, size_t>(dx*dx + dy*dy, i));
  }
  QuickSorter::Sort(cand, ComplexComparator::Make(
    FunctionAccessor::MakeConst(&olx_pair_t<double, size_t>::GetA),
    TPrimitiveComparator()));
  olxstr line = "Cartoon: candidates for this click (2D distance, depth) -";
  for (size_t i = 0; i < olx_min(cand.Count(), (size_t)4); i++) {
    const size_t ri = cand[i].b;
    const vec3d p = Parent.Project(residue_ca[ri]);
    line << ' ' << (ri == chosen ? '*' : ' ') << '#' << ri << ':' <<
      olxstr::FormatFloat(2, sqrt(cand[i].a)) << '/' <<
      olxstr::FormatFloat(1, p[2]);
  }
  TBasicApp::NewLogEntry(logInfo) << line;
}
//.............................................................................
bool TXCartoon::ToggleResidue(size_t i) {
  const size_t rc = ResidueCount();
  if (i >= rc) {
    return false;
  }
  if (residue_selected.Count() != rc) {
    residue_selected.SetCount(rc);
    for (size_t j = 0; j < rc; j++) {
      residue_selected[j] = false;
    }
  }
  residue_selected[i] = !residue_selected[i];
  return true;
}
//.............................................................................
bool TXCartoon::ClearResidueSelection() {
  if (residue_selected.IsEmpty()) {
    return false;
  }
  residue_selected.Clear();
  return true;
}
//.............................................................................
size_t TXCartoon::SelectedResidueCount() const {
  size_t n = 0;
  for (size_t i = 0; i < residue_selected.Count(); i++) {
    if (residue_selected[i]) {
      n++;
    }
  }
  return n;
}
//.............................................................................
bool TXCartoon::OnMouseDown(const IOlxObject *, const TMouseData &) {
  /* Claimed, but nothing is decided yet - see OnMouseUp. Returning true keeps
  the default handling off this object; the drag that may follow still rotates
  the scene, because OnMouseMove is not overridden and the renderer's own
  handlers pick it up.
  */
  return true;
}
//.............................................................................
bool TXCartoon::OnMouseUp(const IOlxObject *, const TMouseData &d) {
  // a drag rotates and selects nothing, by the renderer's own click threshold
  if (d.GlMouse != 0 && !d.GlMouse->IsClick(d)) {
    return true;
  }
  // pixels to the plane Project() returns, as the legend does it; screen y runs
  // down and the plane's up, hence the subtraction
  const double s = Parent.GetScale();
  const double px = (d.DownX - Parent.GetWidth() / 2.0)*s;
  const double py = (Parent.GetHeight() / 2.0 - d.DownY)*s;
  double q = 0;
  /* No tolerance: the renderer has already resolved this chain as the object
  under the cursor, so the nearest of its residues is the answer whatever the
  distance. A finite limit can only reject it, leaving the click to fall through
  to the default and select the whole chain.
  */
  const size_t i = HitResidueProjected(px, py, q, 1e30);
  if (i == InvalidIndex || i >= residue_ids.Count()) {
    return false;
  }
  // every click toggles and nothing is cleared, as clicking behaves elsewhere
  // in Olex2; 'sel -u' and 'isolate off' clear it
  if (ToggleResidue(i)) {
    Rebuild();
  }
  TGXApp &app = TGXApp::GetInstance();
  olxstr label = olxstr(chain_id) << ':';
  const TAsymmUnit &au = app.XFile().GetAsymmUnit();
  const size_t rid = residue_ids[i];
  if (rid != InvalidIndex && rid < au.ResidueCount()) {
    const TResidue &r = au.GetResidue(rid);
    label << r.GetClassName() << ' ' << r.GetNumber();
  }
  else {
    // no RESI behind it, so the position in the trace is all there is to name
    label << "residue " << (i + 1);
  }
  const bool now = (i < residue_selected.Count() && residue_selected[i]);
  // the identity is not id 0 but the encoded 0x808080: matrix index 0 with a
  // zero translation biased by 128
  const bool generated = (matrix_id != 0 && matrix_id != 0x808080);
  TBasicApp::NewLogEntry(logInfo) << "Cartoon: " <<
    (now ? "selected " : "deselected ") << label <<
    (generated ? (olxstr(" [symmetry copy, matrix ") << matrix_id << ']')
      : EmptyString());
  ReportHitCandidates(px, py, i);
  // from the release, as TGlMouse's own selection fallback does (glmouse.cpp:98)
  Parent.Draw();
  // consumed, so the default handling does not select the whole chain
  return true;
}
//.............................................................................
void TXCartoon::DrawPickProxy() const {
  const size_t n = residue_ca.Count();
  if (n < 2 || pick_slices < 3) {
    return;
  }
  const bool subset = (residue_visible.Count() == n);
  const double two_pi = 6.283185307179586;
  olx_gl::begin(GL_TRIANGLES);
  for (size_t i = 0; i + 1 < n; i++) {
    // a segment is clickable if either end is being drawn
    if (subset && !(residue_visible[i] || residue_visible[i+1])) {
      continue;
    }
    const vec3d &a = residue_ca[i], &b = residue_ca[i+1];
    vec3d ax = b - a;
    const double len = ax.Length();
    if (len < 1e-6) {
      continue;
    }
    ax /= len;
    // any axis not along the segment, so the cross product cannot vanish
    const vec3d up = (olx_abs(ax[2]) < 0.9) ? vec3d(0, 0, 1) : vec3d(1, 0, 0);
    vec3d u(ax[1]*up[2] - ax[2]*up[1],
      ax[2]*up[0] - ax[0]*up[2],
      ax[0]*up[1] - ax[1]*up[0]);
    u.Normalise();
    const vec3d v(ax[1]*u[2] - ax[2]*u[1],
      ax[2]*u[0] - ax[0]*u[2],
      ax[0]*u[1] - ax[1]*u[0]);
    for (int s = 0; s < pick_slices; s++) {
      const double a0 = two_pi*s / pick_slices,
        a1 = two_pi*(s + 1) / pick_slices;
      const vec3d p0 = (u*cos(a0) + v*sin(a0))*pick_radius;
      const vec3d p1 = (u*cos(a1) + v*sin(a1))*pick_radius;
      olx_gl::vertex(a + p0);  olx_gl::vertex(b + p0);  olx_gl::vertex(b + p1);
      olx_gl::vertex(a + p0);  olx_gl::vertex(b + p1);  olx_gl::vertex(a + p1);
    }
  }
  olx_gl::end();
}
//.............................................................................
bool TXCartoon::Orient(TGlPrimitive &P) {
  /* Picking re-renders the scene with one colour per object and reads the pixel
  back to identify it, so the compiled list must not run: it emits a glColor at
  every residue boundary, which would decode as some other object entirely. A
  coarse tube along the CA trace is drawn instead - no colour changes, few
  enough triangles to re-emit on a click, and fat enough that the edges of a
  ribbon still register. It is drawn although the object is not selectable,
  this pass being what tells a click which chain it landed on.
  */
  if (Parent.IsSelecting()) {
    DrawPickProxy();
    return true;
  }
  // the mesh carries absolute Cartesian coordinates, so no transform is needed
  return false;
}
//.............................................................................
bool TXCartoon::GetDimensions(vec3d &Max, vec3d &Min) {
  if (!has_dims || !IsVisible()) {
    return false;
  }
  Min = min_dim;
  Max = max_dim;
  return true;
}
//.............................................................................
vec3d TXCartoon::CalcCenter() const {
  if (!has_dims) {
    return vec3d(0);
  }
  return (min_dim + max_dim) / 2;
}
//.............................................................................
