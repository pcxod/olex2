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
void CartoonTests(OlxTests &t) {
  t.Add(test::CartoonTubeTest);
  t.Add(test::CartoonRibbonPlaneTest);
  t.Add(test::CartoonColourTest);
}
};  //namespace test
