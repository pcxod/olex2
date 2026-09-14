/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../cartoon_geom.h"

namespace test {

/* The mesh generator is deliberately free of OpenGL, which is what makes these
cases possible at all: the shipped code runs here rather than a copy of it, with
no graphics context and no model.

The properties checked are the ones that have actually gone wrong: appending to
a shared mesh, the per-residue index table, and the triangle winding.
*/
namespace cartoon_test_data {
  using namespace xlib::protein;

  // a straight run of CA positions along x at peptide spacing
  ChainSegment StraightSegment(size_t n, double spacing = 3.8) {
    ChainSegment s;
    s.chain_id = 'A';
    for (size_t i = 0; i < n; i++) {
      BackboneResidue r;
      r.number = (int)(i + 1);
      r.ca = vec3d(spacing*i, 0, 0);
      s.residues.AddCopy(r);
    }
    return s;
  }
}

//.............................................................................
void CartoonTubeTest(OlxTests &t) {
  using namespace gxlib::cartoon;
  t.description = __FUNC__;

  TubeParams p;
  const float r = p.radius;

  // a segment too short to have a direction produces nothing, and does not throw
  {
    Mesh m;
    BuildTube(cartoon_test_data::StraightSegment(1), p, m);
    if (m.TriangleCount() != 0 || m.VertexCount() != 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "a one-residue segment produced geometry");
    }
  }

  /* A straight trace is its own Catmull-Rom spline, so every vertex must sit
  exactly the tube radius from the x axis. This catches a frame that collapses
  or a radius applied to the wrong vector.
  */
  {
    Mesh m;
    BuildTube(cartoon_test_data::StraightSegment(5), p, m);
    if (m.TriangleCount() == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "a five-residue segment produced no geometry");
    }
    if (m.normals.Count() != m.vertices.Count()) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "one normal per vertex is expected");
    }
    for (size_t i = 0; i < m.vertices.Count(); i++) {
      const vec3f &v = m.vertices[i];
      const float d = sqrt(v[1]*v[1] + v[2]*v[2]);
      if (olx_abs(d - r) > 1e-3f) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("vertex ") << i << " is " << d << " from the axis, expected "
          << r);
      }
      if (olx_abs(m.normals[i].Length() - 1) > 1e-3f) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "a normal is not of unit length");
      }
    }
  }

  /* The winding must put the outward face at the front. It is invisible while
  the material is opaque, because culling is only enabled for transparent ones,
  so nothing in the GUI would report it until the cartoon is made transparent.
  */
  {
    Mesh m;
    BuildTube(cartoon_test_data::StraightSegment(4), p, m);
    for (size_t i = 0; i < m.triangles.Count(); i++) {
      const IndexTriangle &tr = m.triangles[i];
      const vec3f &a = m.vertices[tr.vertices[0]],
        &b = m.vertices[tr.vertices[1]],
        &c = m.vertices[tr.vertices[2]];
      const vec3f face = (b - a).XProdVec(c - a);
      // outward is away from the x axis, which the trace runs along
      const vec3f out(0, a[1], a[2]);
      if (face.DotProd(out) <= 0) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("triangle ") << i << " faces inwards");
      }
    }
  }

  /* Several chains share one mesh, so BuildTube appends. An earlier version
  cleared the triangle list on entry and silently dropped every chain but the
  last.
  */
  {
    Mesh one, both;
    BuildTube(cartoon_test_data::StraightSegment(5), p, one);
    BuildTube(cartoon_test_data::StraightSegment(5), p, both);
    const size_t after_first = both.TriangleCount(),
      vertices_first = both.VertexCount();
    BuildTube(cartoon_test_data::StraightSegment(7), p, both);
    if (both.TriangleCount() <= after_first ||
      after_first != one.TriangleCount())
    {
      throw TFunctionFailedException(__OlxSourceInfo,
        "appending a second segment did not preserve the first");
    }
    // the second segment must index its own vertices, not the first's
    bool references_new = false;
    for (size_t i = after_first; i < both.triangles.Count(); i++) {
      for (size_t j = 0; j < 3; j++) {
        if (both.triangles[i].vertices[j] < vertices_first) {
          throw TFunctionFailedException(__OlxSourceInfo,
            "the second segment indexes the first segment's vertices");
        }
        references_new = true;
      }
    }
    if (!references_new) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "the second segment produced no triangles");
    }
  }

  /* The per-residue table is what a residue subset will index through, so it
  must cover the triangles exactly once, in order, with a closing sentinel.
  */
  {
    Mesh m;
    const size_t n = 6;
    BuildTube(cartoon_test_data::StraightSegment(n), p, m);
    if (m.residue_triangle_offset.Count() != n + 1) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("expected ") << (n + 1) << " residue offsets, got "
        << m.residue_triangle_offset.Count());
    }
    if (m.residue_triangle_offset[0] != 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "the first residue does not start at the first triangle");
    }
    if (m.residue_triangle_offset[n] != m.TriangleCount()) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "the closing sentinel does not equal the triangle count");
    }
    for (size_t i = 1; i <= n; i++) {
      if (m.residue_triangle_offset[i] < m.residue_triangle_offset[i-1]) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "the residue offsets are not monotonic");
      }
    }
  }

  // the triangle estimate is what the caller budgets memory with
  {
    Mesh m;
    const size_t n = 20;
    BuildTube(cartoon_test_data::StraightSegment(n), p, m);
    const size_t estimate = p.TriangleEstimate(n);
    if (m.TriangleCount() > estimate) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("the triangle estimate ") << estimate << " is below the actual "
        << m.TriangleCount());
    }
  }
}
//.............................................................................
void CartoonColourTest(OlxTests &t) {
  using namespace gxlib::cartoon;
  t.description = __FUNC__;

  // the ends of the ramp are what a reader keys on, so they are asserted exactly
  if (RainbowColour(0) != 0xffff0000u) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "the low end of the ramp is not blue");
  }
  if (RainbowColour(1) != 0xff0000ffu) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "the high end of the ramp is not red");
  }
  // out of range must clamp rather than wrap, which would invert the meaning
  if (RainbowColour(-5) != RainbowColour(0) ||
    RainbowColour(5) != RainbowColour(1))
  {
    throw TFunctionFailedException(__OlxSourceInfo,
      "the ramp does not clamp");
  }
  // every step opaque, and the ramp actually varies rather than saturating
  uint32_t first = RainbowColour(0);
  size_t changes = 0;
  for (size_t i = 1; i <= 100; i++) {
    const uint32_t c = RainbowColour(double(i)/100);
    if ((c >> 24) != 0xff) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "a ramp colour is not opaque");
    }
    if (c != first) {
      changes++;
      first = c;
    }
  }
  if (changes < 50) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("the ramp is nearly flat: ") << changes << " changes in 100 steps");
  }
}
//.............................................................................
/* The ribbon plane. This is the one line of the geometry that cannot be checked
by looking at a wireframe or a triangle count: get it wrong and the surface is
still closed, still smooth and still the right length, but every helix is turned
through ninety degrees and reads as a lumpy tube. It shipped wrong once.
*/
void CartoonRibbonPlaneTest(OlxTests &t) {
  using namespace gxlib::cartoon;
  t.description = __FUNC__;

  /* One residue of an ideal alpha helix about the z axis: the chain step is
  mostly tangential, and the carbonyl points along the axis toward the C
  terminus, which is what makes the ribbon lie flat facing outwards.
  */
  const double turn = 100*M_PI/180, radius = 2.3, rise = 1.5;
  const vec3f ca((float)radius, 0, 0);
  const vec3f ca_next((float)(radius*cos(turn)), (float)(radius*sin(turn)),
    (float)rise);
  // O about 2.4 A from CA, mostly axial with a little radial
  const vec3f o(ca[0] + 0.5f, ca[1], ca[2] + 2.3f);

  const vec3f d = RibbonWidthDirection(ca_next - ca, o - ca);
  if (d.Length() < 0.5f) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "no ribbon direction was produced for an ordinary residue");
  }
  // it must be perpendicular to the chain, or the ribbon shears along the chain
  const vec3f step = (ca_next - ca)/(ca_next - ca).Length();
  if (olx_abs(d.DotProd(step)) > 1e-3f) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "the ribbon direction is not perpendicular to the chain");
  }
  /* And it must lie along the helix axis, not radially. The surface normal is
  the radial one; swapping the two is the ninety-degree error, and it is the
  only assertion here that would have caught it.
  */
  if (olx_abs(d[2]) < 0.7f) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("the ribbon of a helix is not lying along its axis: z component ")
      << d[2]);
  }
  // the surface normal, which is what the cross product gives, must be radial
  const vec3f normal = step.XProdVec(d);
  if (olx_abs(normal[2]) > 0.5f) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "the ribbon surface of a helix is not facing outwards");
  }

  // degenerate input must be reported, not turned into a random direction
  if (RibbonWidthDirection(vec3f(0, 0, 0), vec3f(1, 0, 0)).Length() > 1e-6f ||
    RibbonWidthDirection(vec3f(1, 0, 0), vec3f(2, 0, 0)).Length() > 1e-6f)
  {
    throw TFunctionFailedException(__OlxSourceInfo,
      "a degenerate residue produced a ribbon direction");
  }
}
//.............................................................................
/* the extent of one cross-section along a given axis. The segments below run
along x with their carbonyls along z, so the ribbon lies flat with its width
along z and its thickness along y - taking whichever extent is larger instead
reads the arrowhead tip, which is thicker than it is wide, off the wrong axis
*/
float RingExtent(const gxlib::cartoon::Mesh &m, size_t slices, size_t ring,
  int axis)
{
  vec3f mn = m.vertices[ring*slices], mx = mn;
  for (size_t k = 1; k < slices; k++) {
    vec3f::UpdateMinMax(m.vertices[ring*slices + k], mn, mx);
  }
  return mx[axis] - mn[axis];
}
//.............................................................................
/* a strand must end in an arrowhead: wider than the strand at the shoulder and
tapering to the tip. It used to stop a step short, at the coil radius, so the
arrow merged into the coil after it and no head was visible
*/
void CartoonArrowheadTest(OlxTests &t) {
  t.description = __FUNC__;
  using namespace gxlib::cartoon;
  using namespace xlib::protein;
  // an extended strand of 8, then coil - the carbonyls alternate as they do
  ChainSegment seg;
  seg.chain_id = 'A';
  const size_t rc = 11, ns = 8;
  for (size_t i = 0; i < rc; i++) {
    BackboneResidue r;
    r.number = (int)(i + 1);
    r.ca = vec3d(3.3*i, (i % 2) ? 0.4 : -0.4, 0);
    r.o = r.ca + vec3d(0, 0, (i % 2) ? 1.2 : -1.2);
    r.has_o = true;
    seg.residues.AddCopy(r);
  }
  TArrayList<short> ss(rc);
  for (size_t i = 0; i < rc; i++) {
    ss[i] = (i < ns) ? ss_strand : ss_coil;
  }
  CartoonParams p;
  Mesh m;
  BuildCartoon(seg, ss, p, m);
  const size_t rings = m.vertices.Count()/p.slices;
  float widest = 0, narrowest = 1e6f;
  for (size_t r = 0; r < rings; r++) {
    const float a = RingExtent(m, p.slices, r, 2);
    widest = olx_max(widest, a);
    narrowest = olx_min(narrowest, a);
  }
  if (widest < 2*p.arrow_width - 0.01f) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("no arrowhead shoulder: widest ring is ") << widest);
  }
  if (narrowest > 2*p.arrow_tip + 0.01f) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("the arrowhead never reaches its tip: narrowest ring is ")
      << narrowest);
  }
  // and a strand running to the end of the segment gets one too
  for (size_t i = 0; i < rc; i++) {
    ss[i] = ss_strand;
  }
  Mesh m2;
  BuildCartoon(seg, ss, p, m2);
  const size_t rings2 = m2.vertices.Count()/p.slices;
  float widest2 = 0;
  for (size_t r = 0; r < rings2; r++) {
    widest2 = olx_max(widest2, RingExtent(m2, p.slices, r, 2));
  }
  if (widest2 < 2*p.arrow_width - 0.01f) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("a strand ending the segment has no arrowhead: widest ring is ")
      << widest2);
  }
}
//.............................................................................
/* the flat sections must keep their thickness out to the edge. An ellipse comes
to a knife edge and reads as much thinner than its nominal thickness
*/
void CartoonSectionTest(OlxTests &t) {
  t.description = __FUNC__;
  using namespace gxlib::cartoon;
  using namespace xlib::protein;
  ChainSegment seg = cartoon_test_data::StraightSegment(6);
  for (size_t i = 0; i < seg.residues.Count(); i++) {
    seg.residues[i].o = vec3d(seg.residues[i].ca) + vec3d(0, 0, 1.2);
    seg.residues[i].has_o = true;
  }
  TArrayList<short> ss(seg.residues.Count());
  for (size_t i = 0; i < ss.Count(); i++) {
    ss[i] = ss_helix;
  }
  /* of the points on one section, the one nearest the edge across the ribbon,
  and how much of the thickness it still carries
  */
  struct Edge {
    static float Of(const CartoonParams &p, const ChainSegment &seg,
      const TArrayList<short> &ss)
    {
      Mesh m;
      BuildCartoon(seg, ss, p, m);
      if (m.vertices.Count() < 2*p.slices) {
        throw TFunctionFailedException(__OlxSourceInfo, "no ribbon");
      }
      const size_t ring = p.slices;  // the second section, past the start
      float w_max = 0;
      for (size_t k = 0; k < p.slices; k++) {
        w_max = olx_max(w_max, olx_abs(m.vertices[ring + k][2]));
      }
      float th = 0;
      for (size_t k = 0; k < p.slices; k++) {
        const vec3f &v = m.vertices[ring + k];
        if (olx_abs(v[2]) > 0.8f*w_max) {
          th = olx_max(th, olx_abs(v[1]));
        }
      }
      return th;
    }
  };
  CartoonParams p;
  const float flat = Edge::Of(p, seg, ss);
  if (flat < 0.5f*p.helix_thickness) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("the ribbon edge is knife thin: ") << flat <<
      " of " << p.helix_thickness);
  }
  // and that this is the section shape doing it, not the thickness alone
  CartoonParams e2 = p;
  e2.edge = 2;
  if (Edge::Of(e2, seg, ss) >= flat) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "the superellipse section is no fuller at the edge than an ellipse");
  }
}
//.............................................................................
/* every strand of a sheet ends in its own arrowhead, not just the one that
ends the chain. A hairpin has two: one into the turn and one at the end
*/
void CartoonSheetArrowTest(OlxTests &t) {
  t.description = __FUNC__;
  using namespace gxlib::cartoon;
  using namespace xlib::protein;
  /* the two-stranded antiparallel sheet of SecondaryStructureTest: zigzag
  strands 4.8 A apart, which is what makes them a sheet and not two pieces of
  extended chain, joined by a three residue turn
  */
  ChainSegment seg;
  seg.chain_id = 'A';
  const size_t strand_len = 10;
  for (size_t i = 0; i < strand_len; i++) {
    BackboneResidue r;
    r.number = (int)(i + 1);
    r.ca = vec3d(3.3*i, 0, (i % 2) ? 0.9 : -0.9);
    seg.residues.AddCopy(r);
  }
  for (size_t i = 0; i < 3; i++) {
    BackboneResidue r;
    r.number = (int)(strand_len + i + 1);
    r.ca = vec3d(3.3*(strand_len - 1) + 2.0, 1.2*(i + 1), 0);
    seg.residues.AddCopy(r);
  }
  for (size_t i = 0; i < strand_len; i++) {
    BackboneResidue r;
    r.number = (int)(strand_len + 4 + i);
    r.ca = vec3d(3.3*(strand_len - 1 - i), 4.8,
      ((strand_len - 1 - i) % 2) ? 0.9 : -0.9);
    seg.residues.AddCopy(r);
  }
  TArrayList<short> ss;
  AssignSecondaryStructure(seg, ss);
  size_t n_strand_runs = 0;
  for (size_t i = 0; i < ss.Count(); i++) {
    if (ss[i] == ss_strand && (i == 0 || ss[i-1] != ss_strand)) {
      n_strand_runs++;
    }
  }
  if (n_strand_runs != 2) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("the hairpin was not assigned two strands but ") << n_strand_runs);
  }
  CartoonParams p;
  Mesh m;
  BuildCartoon(seg, ss, p, m);
  /* one shoulder per strand: a ring wider than the strand whose predecessor
  was not. The arrowhead is the only thing that widens the ribbon
  */
  const size_t rings = m.vertices.Count()/p.slices;
  const float flare = 2*(p.strand_width + p.arrow_width)/2;
  size_t n_heads = 0;
  bool was_wide = false;
  for (size_t r = 0; r < rings; r++) {
    const bool wide = RingExtent(m, p.slices, r, 1) > flare ||
      RingExtent(m, p.slices, r, 2) > flare;
    if (wide && !was_wide) {
      n_heads++;
    }
    was_wide = wide;
  }
  if (n_heads != n_strand_runs) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("a sheet of ") << n_strand_runs << " strands drew " << n_heads
      << " arrowheads");
  }
}
//.............................................................................
void CartoonTests(OlxTests &t) {
  t.Add(test::CartoonTubeTest);
  t.Add(test::CartoonRibbonPlaneTest);
  t.Add(test::CartoonColourTest);
  t.Add(test::CartoonArrowheadTest);
  t.Add(test::CartoonSectionTest);
  t.Add(test::CartoonSheetArrowTest);
}
};  //namespace test
