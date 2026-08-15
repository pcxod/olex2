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

/* Mesh generation for the polymer cartoon representations.

Topology lives in xlib/protein.h; this only turns a chain of backbone positions
into triangles. Deliberately free of OpenGL, so the same geometry can be handed
to a different renderer without being rewritten, and so it can be tested without
a graphics context.
*/
BeginGxlNamespace()

namespace cartoon {

/* Triangle mesh with per-residue provenance.

`residue_triangle_offset` holds the first triangle of each residue plus a
closing sentinel equal to the triangle count, so a subset of residues maps to a
contiguous triangle range without searching.
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

/* Cross-section sizes per secondary structure, in Angstroms.

The cross-section is an ellipse whose two semi-axes are the width, across the
ribbon, and the thickness. A coil sets them equal and is therefore a round tube,
which is why one builder covers both representations: the tube is the cartoon
with every residue coil.

Keeping the number of points around the cross-section fixed across the three
types is what lets a whole chain stay one continuous surface, morphing between a
tube and a ribbon rather than butting two separately closed pieces together.
*/
struct CartoonParams {
  float coil_radius;
  float helix_width, helix_thickness;
  float strand_width, strand_thickness;
  // widest point of the arrowhead, and the point it tapers to
  float arrow_width, arrow_tip;
  size_t steps_per_residue;
  size_t slices;
  /* Draw every residue as coil, whatever it was assigned. The plain CA trace,
  kept as a representation in its own right: it is the honest one when the
  assignment is not to be trusted, and it is what a low-resolution or partly
  built model deserves.
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

/* The direction the ribbon lies flat along at one residue, from the chain step
A = CA(i+1) - CA(i) and the carbonyl B = O(i) - CA(i).

Carson and Bugg (1986) call this D = (A x B) x A, which is the part of B
perpendicular to A. A x B on its own is the *surface normal*, and using that as
the width instead turns the ribbon through ninety degrees: a helix then presents
its edge and reads as a lumpy tube rather than a broad spiral band. Separated
out and tested because that is exactly the mistake the first version made and
nothing else in the geometry would reveal it.

Returns a zero vector when the two are parallel, or either is degenerate.
*/
vec3f RibbonWidthDirection(const vec3f &chain_step, const vec3f &carbonyl);

/* A cartoon following a Catmull-Rom spline through the CA positions: ribbon
through helices, arrow through strands, round tube through coil.

`ss` holds one xlib::protein::ss_* value per residue; an empty or wrongly sized
list is treated as all coil, which degrades to the plain trace rather than
failing. Appends to the mesh, so several segments share one buffer.
*/
void BuildCartoon(const xlib::protein::ChainSegment &seg,
  const TArrayList<short> &ss, const CartoonParams &p, Mesh &m);

/* The trace on its own. The same builder with every residue coil, so there is
one implementation of the spline, the frame and the stitching.
*/
void BuildTube(const xlib::protein::ChainSegment &seg, const TubeParams &p,
  Mesh &m);

/* Blue to cyan to green to yellow to red, with t clamped to [0,1].

The usual ramp for a scalar along a chain, and the one a reader of Coot or
PyMOL figures already knows: N terminus blue, C terminus red; low displacement
parameter blue, high red. Returned in the OLX_RGBA packing, 0xAABBGGRR.
*/
uint32_t RainbowColour(double t);

} // namespace cartoon

EndGxlNamespace()
#endif
