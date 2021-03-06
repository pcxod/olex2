/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "glfont.h"
#include "gltexture.h"

TGlPrimitive::TGlPrimitive(TObjectGroup& ParentG, TGlRenderer& ParentR, short type)
  : AGroupObject(ParentG), Renderer(ParentR)
{
  Quadric = 0;
  Evaluator = 0;
  TextureId = ListId = OwnerId = ~0;
  QuadricDrawStyle = GLU_FILL;
  QuadricNormals = GLU_SMOOTH;
  QuadricOrientation = GLU_OUTSIDE;
  Compiled = false;
  ParentCollection = 0;
  ClipPlanes = 0;
  Basis = 0;
  String = 0;
  Font = 0;
  SetType(type);
}
//..............................................................................
TGlPrimitive::~TGlPrimitive() {
  if (Quadric != 0) {
    gluDeleteQuadric(Quadric);
  }
  if (Basis != 0) {
    delete Basis;
  }
  if (ClipPlanes != 0) {
    delete ClipPlanes;
  }
  if (olx_is_valid_index(ListId)) {
    olx_gl::deleteLists(ListId, 1);
  }
}
//..............................................................................
void TGlPrimitive::CreateQuadric() {
  if (Quadric != 0) {
    return;
  }
  Quadric = gluNewQuadric();
  if (Quadric == 0) {
    throw TOutOfMemoryException(__OlxSourceInfo);
  }
  if (olx_is_valid_index(TextureId)) {
    olx_gl::bindTexture(GL_TEXTURE_2D, TextureId);
    gluQuadricTexture(Quadric, GL_TRUE);
  }
  else {
    gluQuadricTexture(Quadric, GL_FALSE);
  }

  gluQuadricOrientation(Quadric, QuadricOrientation);
  gluQuadricDrawStyle(Quadric, QuadricDrawStyle);
  gluQuadricNormals(Quadric, QuadricNormals);
}
//..............................................................................
void TGlPrimitive::SetType(short T) {
  Type = T;
  switch (Type) {
  case sgloText:
    Params.Resize(4);
    Params[0] = 1;  Params[1] = 1;  Params[2] = 1; Params[3] = 1;
    break;
  case sgloSphere:
    Params.Resize(3);
    Params[0] = 1;  Params[1] = 5;  Params[2] = 5;
    break;
  case sgloDisk:
    Params.Resize(4);
    Params[0] = 0;  Params[1] = 1;  Params[2] = 5;  Params[3] = 5;
    break;
  case sgloDiskSlice:
    Params.Resize(6);
    Params[0] = 0;  Params[1] = 1;  Params[2] = 5;  Params[3] = 5;
    Params[4] = 0;  Params[5] = 90;
    break;
  case sgloCylinder:
    Params.Resize(5);
    Params[0] = 0;  Params[1] = 1;  Params[2] = 1;  Params[3] = 5;
    Params[4] = 5;
    break;
  case sgloCommandList:
    Compiled = true;
    break;
  default:
    Params.Resize(1); // point size or line width
    Params[0] = 1;  // default point size and line width
    break;
  }
  if (Type == sgloDisk || Type == sgloDiskSlice ||
    Type == sgloCylinder || Type == sgloSphere ||
    Type == sgloCommandList)
  {
    ListId = Renderer.NewListId();
  }
}
//..............................................................................
void TGlPrimitive::ListParams(TStrList &List) {
  switch (Type) {
  case sgloSphere:
    List << "Radius" << "Slices" << "Stacks";
    break;
  case sgloDisk:
    List << "Inner radius" << "Outer radius" << "Slices" << "Loops";
    break;
  case sgloDiskSlice:
    List << "Inner radius" << "Outer radius" << "Slices" << "Loops" <<
      "Start angle" << "Sweep angle";
    break;
  case sgloCylinder:
    List << "Base radius" << "Top radius" << "Height" << "Slices" << "Loops";
    break;
  case sgloPoints:
    List << "Point size";
    break;
  case sgloLines:
    List << "Line width";
    break;
  default:
    ParentCollection->ListParams(List, this);
    break;
  }
}
//..............................................................................
bool TGlPrimitive::IsCompilable() const {
  switch (Type) {
  case sgloSphere:
  case sgloDisk:
  case sgloDiskSlice:
  case sgloCylinder:
    return true;
  }
  return false;
}
//..............................................................................
void TGlPrimitive::Compile() {
  if (IsCompiled())  return;
  TGlTexture* currentTexture = 0;
  if (IsCompilable()) {
    olx_gl::newList(ListId, GL_COMPILE);
    if (olx_is_valid_index(TextureId)) {
      TGlTexture* tex = Renderer.GetTextureManager().FindTexture(TextureId);
      currentTexture = new TGlTexture();
      tex->ReadCurrent(*currentTexture);
      tex->SetCurrent();
    }
    if (ClipPlanes != 0) {
      ClipPlanes->Enable(true);
    }
    if (Basis != 0) {
      olx_gl::orient(*Basis);
    }
    CreateQuadric();
  }
  switch (Type) {
  case sgloSphere:
    gluSphere(Quadric, Params[0], (int)Params[1], (int)Params[2]);
    break;
  case sgloDisk:
    gluDisk(Quadric, Params[0], Params[1], (int)Params[2], (int)Params[3]);
    break;
  case sgloDiskSlice:
    gluPartialDisk(Quadric, Params[0], Params[1], (int)Params[2],
      (int)Params[3], Params[4], Params[5]);
    break;
  case sgloCylinder:
    gluCylinder(Quadric, Params[0], Params[1], Params[2], (int)Params[3],
      (int)Params[4]);
    break;
  case sgloCommandList:
    Compiled = true;
    break;
  default:
    Compiled = false;
  }
  if (IsCompilable()) {
    if (ClipPlanes != 0) {
      ClipPlanes->Enable(false);
    }
    olx_gl::endList();
    Compiled = true;
    if (currentTexture != 0) {
      currentTexture->SetCurrent();
      delete currentTexture;
    }
  }
}
//..............................................................................
void TGlPrimitive::PrepareColorRendering(uint16_t _begin) const {
  if (!Renderer.ForcePlain() &&  !Renderer.IsSelecting()) {
    if (_begin == GL_POINT ||
      _begin == GL_LINES ||
      _begin == GL_LINE_STRIP ||
      _begin == GL_LINE_LOOP)
    {
      flagManager.disable(GL_LIGHTING);
    }
    //flagManager.enable(GL_COLOR_MATERIAL);
    //olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  }
  olx_gl::begin(_begin);
}
//..............................................................................
void TGlPrimitive::EndColorRendering() const {
  olx_gl::end();
  flagManager.clear();
}
//..............................................................................
void TGlPrimitive::SetColor(const uint32_t& cl) const {
  if (!Renderer.ForcePlain()) {
    olx_gl::color((float)OLX_GetRValue(cl) / 255, (float)OLX_GetGValue(cl) / 255,
      (float)OLX_GetBValue(cl) / 255, (float)OLX_GetAValue(cl) / 255);
  }
}
//..............................................................................
void TGlPrimitive::Draw() {
#ifdef _DEBUG
  if (olx_is_valid_index(ListId) && !Compiled) {
    throw TInvalidArgumentException(__OlxSourceInfo, "uncompiled complex object");
  }
#endif
  if (Compiled) {
    olx_gl::callList(ListId);
    return;
  }
  if (Basis != 0) {
    olx_gl::orient(*Basis);
  }
  if (ClipPlanes != 0) {
    ClipPlanes->Enable(true);
  }
  TGlTexture* currentTexture = 0;
  if (olx_is_valid_index(TextureId) && !GetRenderer().IsSelecting()) {
    TGlTexture* tex = Renderer.GetTextureManager().FindTexture(TextureId);
    currentTexture = new TGlTexture();
    tex->ReadCurrent(*currentTexture);
    tex->SetCurrent();
  }
  switch (Type) {
  case sgloText: {
#ifdef _DEBUG
    if (Font == 0)
      throw TFunctionFailedException(__OlxSourceInfo, "undefined font");
#endif
    if (!(String == 0 || Font == 0 || String->IsEmpty())) {
      if (Font->IsVectorFont()) {
        Font->DrawVectorText(vec3d(0, 0, 0), *String);
      }
      else {
        /* each character of different colour */
        const size_t StrLen = String->Length();
        if (Colors.Count() == StrLen && !GetRenderer().IsSelecting()) {
          uint32_t prev_color = Colors[0];
          SetColor(prev_color);
          short cstate = 0;
          for (size_t i = 0; i < StrLen; i++) {
            if (prev_color != Colors[i]) {
              SetColor(Colors[i]);
              prev_color = Colors[i];
            }
            Font->DrawRasterChar(i, *String, cstate);
          }
        }
        else {  /* all characters of the same colour */
          Font->DrawRasterText(*String);
        }
      }
    }
    break;
  }
  case sgloPoints: {
    PrepareColorRendering(GL_POINTS);
    olx_gl::pointSize((float)Params[0]);
    if (Colors.IsEmpty() || GetRenderer().IsSelecting()) {
      for (size_t i = 0; i < Vertices.Count(); i++) {
        DrawVertex(Vertices[i]);
      }
    }
    else if (Colors.Count() == Vertices.Count()) {
      for (size_t i = 0; i < Vertices.Count(); i++) {
        DrawVertex(Vertices[i], Colors[i]);
      }
    }
    EndColorRendering();
    break;
  }
  case sgloLines: {
    PrepareColorRendering(GL_LINES);
    float LW = 0;
    if (Params[0] != 1) {
      olx_gl::get(GL_LINE_WIDTH, &LW);
      olx_gl::lineWidth(Params[0] * LW);
    }
    if (Colors.IsEmpty() || GetRenderer().IsSelecting()) {
      for (size_t i = 0; i < Vertices.Count(); i++) {
        DrawVertex(Vertices[i]);
      }
    }
    else if (Colors.Count() == Vertices.Count()) {
      for (size_t i = 0; i < Vertices.Count(); i++) {
        DrawVertex(Vertices[i], Colors[i]);
      }
    }
    else if (Colors.Count() * 2 == Vertices.Count()) {
      for (size_t i = 0; i < Colors.Count(); i++) {
        SetColor(Colors[i]);
        DrawVertex2(i * 2);
      }
    }
    if (LW != 0) {
      olx_gl::lineWidth(LW);
    }
    EndColorRendering();
    break;
  }
  case sgloLineStrip: {
    PrepareColorRendering(GL_LINE_STRIP);
    float LW = 0;
    if (Params[0] != 1) {
      olx_gl::get(GL_LINE_WIDTH, &LW);
      olx_gl::lineWidth(Params[0] * LW);
    }
    for (size_t i = 0; i < Vertices.Count(); i++) {
      if (Colors.Count() == Vertices.Count()) {
        SetColor(Colors[i]);
      }
      DrawVertex(Vertices[i]);
    }
    if (LW != 0) {
      olx_gl::lineWidth(LW);
    }
    EndColorRendering();
    break;
  }
  case sgloLineLoop: {
    PrepareColorRendering(GL_LINE_LOOP);
    float LW = 0;
    if (Params[0] != 1) {
      olx_gl::get(GL_LINE_WIDTH, &LW);
      olx_gl::lineWidth(Params[0] * LW);
    }
    if (Colors.IsEmpty() || GetRenderer().IsSelecting()) {
      for (size_t i = 0; i < Vertices.Count(); i++) {
        DrawVertex(Vertices[i]);
      }
    }
    else if (Colors.Count() == Vertices.Count()) {
      for (size_t i = 0; i < Vertices.Count(); i++) {
        DrawVertex(Vertices[i], Colors[i]);
      }
    }
    if (LW != 0) {
      olx_gl::lineWidth(LW);
    }
    EndColorRendering();
    break;
  }
  case sgloTriangles: {
    PrepareColorRendering(GL_TRIANGLES);
    if (Normals.IsEmpty()) {
      if (Colors.IsEmpty() || GetRenderer().IsSelecting()) {
        for (size_t i = 0; i < Vertices.Count(); i++) {
          DrawVertex(Vertices[i]);
        }
      }
      else if (Colors.Count() == Vertices.Count()) {
        for (size_t i = 0; i < Vertices.Count(); i++) {
          DrawVertex(Vertices[i], Colors[i]);
        }
      }
    }
    else {
      if (Colors.IsEmpty() || GetRenderer().IsSelecting()) {
        if (Normals.Count() == Vertices.Count()) {  //+normal
          for (size_t i = 0; i < Normals.Count(); i++) {
            SetNormal(Normals[i]);
            DrawVertex(Vertices[i]);
          }
        }
        else if (Normals.Count() * 3 == Vertices.Count()) {  //+normal
          for (size_t i = 0; i < Normals.Count(); i++) {
            SetNormal(Normals[i]);
            DrawVertex3(i * 3);
          }
        }
      }
      else if (Colors.Count() == Vertices.Count()) {
        if (Normals.Count() == Vertices.Count()) {  //+normal
          for (size_t i = 0; i < Normals.Count(); i++) {
            SetNormal(Normals[i]);
            DrawVertex(Vertices[i], Colors[i]);
          }
        }
        else if (Normals.Count() * 3 == Vertices.Count()) {  //+normal
          for (size_t i = 0; i < Normals.Count(); i++) {
            SetNormal(Normals[i]);
            DrawVertex3c(i * 3);
          }
        }
      }
    }
    EndColorRendering();
    break;
  }
  case sgloQuads: {
    PrepareColorRendering(GL_QUADS);
    if (TextureCrds.IsEmpty() ||
      !olx_is_valid_index(TextureId) ||
      GetRenderer().IsSelecting())
    {
      if (Colors.IsEmpty() || GetRenderer().IsSelecting()) {
        if (Normals.IsEmpty()) {
          for (size_t i = 0; i < Vertices.Count(); i++) {
            DrawVertex(Vertices[i]);
          }
        }
        else if (Normals.Count() * 4 == Vertices.Count()) {
          for (size_t i = 0; i < Normals.Count(); i++) {
            SetNormal(Normals[i]);
            DrawVertex4(i * 4);
          }
        }
      }
      else if (Colors.Count() == Vertices.Count()) {
        if (Normals.IsEmpty()) {
          for (size_t i = 0; i < Vertices.Count(); i++) {
            DrawVertex(Vertices[i], Colors[i]);
          }
        }
        else if (Normals.Count() * 4 == Vertices.Count()) {
          for (size_t i = 0; i < Normals.Count(); i++) {
            SetNormal(Normals[i]);
            DrawVertex4c(i * 4);
          }
        }
      }
      else if (Colors.Count() * 4 == Vertices.Count()) {
        if (Normals.IsEmpty()) {
          for (size_t i = 0; i < Colors.Count(); i++) {
            SetColor(Colors[i]);
            DrawVertex4(i * 4);
          }
        }
        else if (Normals.Count() * 4 == Vertices.Count()) {
          for (size_t i = 0; i < Normals.Count(); i++) {
            SetNormal(Normals[i]);
            SetColor(Colors[i]);
            DrawVertex4(i * 4);
          }
        }
      }
    }
    else if (TextureCrds.Count() == Vertices.Count()) {
      if (Colors.IsEmpty()) {
        if (Normals.IsEmpty()) {
          for (size_t i = 0; i < Vertices.Count(); i++) {
            DrawVertex(Vertices[i], TextureCrds[i]);
          }
        }
        else if (Normals.Count() * 4 == Vertices.Count()) {
          for (size_t i = 0; i < Normals.Count(); i++) {
            SetNormal(Normals[i]);
            DrawVertex4t(i * 4);
          }
        }
      }
      else if (Colors.Count() == Vertices.Count()) {
        if (Normals.IsEmpty()) {
          for (size_t i = 0; i < Vertices.Count(); i++) {
            DrawVertex(Vertices[i], Colors[i], TextureCrds[i]);
          }
        }
        else if (Normals.Count() * 4 == Vertices.Count()) {
          for (size_t i = 0; i < Normals.Count(); i++) {
            SetNormal(Normals[i]);
            DrawVertex4ct(i * 4);
          }
        }
      }
      else if (Colors.Count() * 4 == Vertices.Count()) {
        if (Normals.IsEmpty()) {
          for (size_t i = 0; i < Colors.Count(); i++) {
            SetColor(Colors[i]);
            DrawVertex4t(i * 4);
          }
        }
        else if (Normals.Count() * 4 == Vertices.Count()) {
          for (size_t i = 0; i < Normals.Count(); i++) {
            SetNormal(Normals[i]);
            SetColor(Colors[i]);
            DrawVertex4t(i * 4);
          }
        }
      }
    }
    EndColorRendering();
    break;
  }
  case sgloPolygon: {
    PrepareColorRendering(GL_POLYGON);
    olx_gl::FlagDisabler fc(GL_CULL_FACE);
    if (Normals.Count() == 1) {
      SetNormal(Normals[0]);
    }
    if (Colors.IsEmpty() || GetRenderer().IsSelecting()) {
      for (size_t i = 0; i < Vertices.Count(); i++) {
        DrawVertex(Vertices[i]);
      }
    }
    else if (Colors.Count() == Vertices.Count()) {
      for (size_t i = 0; i < Vertices.Count(); i++) {
        DrawVertex(Vertices[i], Colors[i]);
      }
    }
    EndColorRendering();
    break;
  }
  } // end of case(Type)
  if (ClipPlanes != 0) {
    ClipPlanes->Enable(false);
  }
  if (currentTexture != 0) {
    currentTexture->SetCurrent();
    delete currentTexture;
  }
  //  olx_gl::enable(GL_LIGHTING);
}
//..............................................................................
AGOProperties& TGlPrimitive::SetProperties(const AGOProperties& C) {
  if (Properties != 0) {
    if (!(C == GetProperties())) { // properties will be removed if ObjectCount == 1
      Renderer.OnSetProperties(GetProperties());
    }
  }
  TGlMaterial& Props = (TGlMaterial&)AGroupObject::SetProperties(C);
  Renderer.SetProperties(Props);
  return Props;
}
//..............................................................................
void TGlPrimitive::TriangularFromEdges(const vec3d *edges, size_t count,
  double sz, const TTypeList<vec3s> &faces)
{
  Vertices.SetCount(faces.Count() * 3);
  Normals.SetCount(faces.Count());
  for (size_t i = 0; i < faces.Count(); i++) {
    vec3d d = edges[faces[i][0]] + edges[faces[i][1]] + edges[faces[i][2]];
    vec3d n = (edges[faces[i][0]] - edges[faces[i][1]])
      .XProdVec(edges[faces[i][2]] - edges[faces[i][1]]).Normalise();
    Vertices[i * 3 + 0] = edges[faces[i][0]] * sz;
    Vertices[i * 3 + 1] = edges[faces[i][1]] * sz;
    Vertices[i * 3 + 2] = edges[faces[i][2]] * sz;
    if (d.DotProd(n) < 1) {
      n *= -1;
    }
    else {
      Vertices.Swap(i * 3 + 0, i * 3 + 2);
    }
    Normals[i] = n;
  }
}
//..............................................................................
void TGlPrimitive::MakeTetrahedron(double sz) {
  if (GetType() != sgloTriangles) {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid object type");
  }
  static vec3d edges [] = {
    vec3d(1, 1, 1),
    vec3d(-1, -1, 1),
    vec3d(-1, 1, -1),
    vec3d(1, -1, -1)
  };
  TTypeList<vec3s> faces;
  faces.AddNew(0, 1, 2);
  faces.AddNew(0, 1, 3);
  faces.AddNew(0, 2, 3);
  faces.AddNew(1, 2, 3);
  TriangularFromEdges(&edges[0], 4, sz, faces);
}
//..............................................................................
void TGlPrimitive::MakeOctahedron(double sz) {
  if (GetType() != sgloTriangles) {
    throw TFunctionFailedException(__OlxSourceInfo, "invlaid object type");
  }
  static vec3d edges [] = {
    vec3d(1, 0, 0),
    vec3d(0, 1, 0),
    vec3d(-1, 0, 0),
    vec3d(0, -1, 0),
    vec3d(0, 0, 1),
    vec3d(0, 0, -1)
  };
  TTypeList<vec3s> faces;
  faces.AddNew(0, 1, 4);
  faces.AddNew(0, 1, 5);
  faces.AddNew(0, 3, 4);
  faces.AddNew(0, 3, 5);
  faces.AddNew(1, 2, 4);
  faces.AddNew(1, 2, 5);
  faces.AddNew(2, 3, 4);
  faces.AddNew(2, 3, 5);
  TriangularFromEdges(&edges[0], 6, sz, faces);
}
//..............................................................................
