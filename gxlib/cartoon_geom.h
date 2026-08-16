/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_cartoon_geom_H
#define __olx_gxl_cartoon_geom_H
#include "gxbase.h"
#include "esphere.h"
#include "typelist.h"
#include "protein.h"

/* mesh generation for the cartoon representations - topology is in
xlib/protein.h, this only turns backbone positions into triangles. Free of
OpenGL, so another renderer can take it and it tests without a context
*/
BeginGxlNamespace()

namespace cartoon {

/* triangle mesh with per-residue provenance: residue_triangle_offset holds the
first triangle of each residue plus a closing sentinel, so a subset maps to a
contiguous range without searching
*/
struct Mesh {
  TTypeList<vec3f> vertices, normals;
  TTypeList<IndexTriangle> triangles;
  TArrayList<size_t> residue_triangle_offset;

  void Clear() {
    vertices.Clear();
    normals.Clear();
    triangles.Clear();
    residue_triangle_offset.Clear();
  }
  size_t TriangleCount() const { return triangles.Count(); }
  size_t VertexCount() const { return vertices.Count(); }
};

/* cross-section sizes per secondary structure, in A. The section is an ellipse
of width across the ribbon and thickness; a coil sets them equal and is a round
tube, which is why one builder covers both representations. The point count is
fixed across the three types so a chain stays one continuous surface
*/
struct CartoonParams {
  float coil_radius;
  float helix_width, helix_thickness;
  float strand_width, strand_thickness;
  // widest point of the arrowhead, and the point it tapers to
  float arrow_width, arrow_tip;
  size_t steps_per_residue;
  size_t slices;
  /* every residue as coil, whatever it was assigned - the plain CA trace, and
  the honest one when the assignment is not to be trusted
  */
  bool trace_only;
  CartoonParams()
    : coil_radius(0.30f),
      helix_width(1.20f), helix_thickness(0.28f),
      strand_width(1.00f), strand_thickness(0.28f),
      arrow_width(1.70f), arrow_tip(0.10f),
      steps_per_residue(8), slices(12),
      trace_only(false)
  {}
  size_t TriangleEstimate(size_t n_residues) const {
    // the arrowhead shoulders add one ring each, hence the slack
    return (n_residues*steps_per_residue + n_residues)*slices*2;
  }
};

/* The plain CA trace: a tube of circular cross-section along the chain. */
struct TubeParams {
  float radius;
  size_t steps_per_residue;
  size_t slices;
  TubeParams()
    : radius(0.35f), steps_per_residue(6), slices(8)
  {}
  size_t TriangleEstimate(size_t n_residues) const {
    return n_residues*steps_per_residue*slices*2;
  }
};

/* the direction the ribbon lies flat along, from the chain step
A = CA(i+1) - CA(i) and the carbonyl B = O(i) - CA(i). Carson and Bugg (1986)
give D = (A x B) x A, the part of B perpendicular to A. A x B is the surface
normal, and using it turns the ribbon ninety degrees - a helix then shows its
edge. Zero when the two are parallel or either is degenerate
*/
vec3f RibbonWidthDirection(const vec3f &chain_step, const vec3f &carbonyl);

/* a cartoon on a Catmull-Rom spline through the CA positions: ribbon through
helices, arrow through strands, tube through coil. ss holds one protein::ss_*
per residue, and an empty or wrong sized list is taken as all coil rather than
failing. Appends, so several segments share one buffer
*/
void BuildCartoon(const xlib::protein::ChainSegment &seg,
  const TArrayList<short> &ss, const CartoonParams &p, Mesh &m);

/* The trace on its own. The same builder with every residue coil, so there is
one implementation of the spline, the frame and the stitching.
*/
void BuildTube(const xlib::protein::ChainSegment &seg, const TubeParams &p,
  Mesh &m);

/* blue to cyan to green to yellow to red, t clamped to [0,1] - the usual ramp
for a scalar along a chain, as Coot and PyMOL draw it. OLX_RGBA, 0xAABBGGRR
*/
uint32_t RainbowColour(double t);

} // namespace cartoon

EndGxlNamespace()
#endif
