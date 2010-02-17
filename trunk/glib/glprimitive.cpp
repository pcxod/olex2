//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "glfont.h"
#include "gltexture.h"

UseGlNamespace();
//..............................................................................
TGlPrimitive::TGlPrimitive(TObjectGroup& ParentG, TGlRenderer& ParentR, short type):
  AGroupObject(ParentG), Renderer(ParentR)
{
  Quadric = NULL;
  Evaluator = NULL;
  Renderer = ParentR;
  TextureId = ListId = OwnerId = ~0;
  QuadricDrawStyle = GLU_FILL;
  QuadricNormals = GLU_SMOOTH;
  QuadricOrientation = GLU_OUTSIDE;
  Compiled = false;
  ParentCollection = NULL;
  ClipPlanes = NULL;
  Basis = NULL;
  String = NULL;
  Font = NULL;
  SetType(type);
}
//..............................................................................
TGlPrimitive::~TGlPrimitive()  {
  if( Quadric != NULL )  
    gluDeleteQuadric(Quadric);
  if( Basis != NULL )     
    delete Basis;
  if( ClipPlanes != NULL )  
    delete ClipPlanes;
}
//..............................................................................
void TGlPrimitive::CreateQuadric()  {
  if( Quadric != NULL )  return;
  Quadric = gluNewQuadric();
  if( Quadric == NULL )
    throw TOutOfMemoryException(__OlxSourceInfo);
  if( olx_is_valid_index(TextureId) )  {
    glBindTexture(GL_TEXTURE_2D, TextureId);
    gluQuadricTexture(Quadric, GL_TRUE);
  }
  else
    gluQuadricTexture(Quadric, GL_FALSE);

  gluQuadricOrientation( Quadric, QuadricOrientation);
  gluQuadricDrawStyle( Quadric, QuadricDrawStyle);
  gluQuadricNormals( Quadric, QuadricNormals);
}
//..............................................................................
void TGlPrimitive::SetType(short T)  {
  Type = T;
  switch( Type )  {
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
  if( Type == sgloDisk || Type == sgloDiskSlice ||
      Type == sgloCylinder || Type == sgloSphere ||
      Type == sgloCommandList )  
    ListId = Renderer.NewListId();

}
//..............................................................................
void TGlPrimitive::ListParams(TStrList &List)  {
  switch( Type )  {
    case sgloSphere:
      List.Add("Radius");
      List.Add("Slices");
      List.Add("Stacks");
      break;
    case sgloDisk:
      List.Add("Inner radius");
      List.Add("Outer radius");
      List.Add("Slices");
      List.Add("Loops");
      break;
    case sgloDiskSlice:
      List.Add("Inner radius");
      List.Add("Outer radius");
      List.Add("Slices");
      List.Add("Loops");
      List.Add("Start angle");
      List.Add("Sweep angle");
      break;
    case sgloCylinder:
      List.Add("Base radius");
      List.Add("Top radius");
      List.Add("Height");
      List.Add("Slices");
      List.Add("Loops");
      break;
    case sgloPoints:
      List.Add("Point size");
      break;
    case sgloLines:
      List.Add("Line width");
      break;
    default:
      ParentCollection->ListParams(List, this);
      break;    
  }
}
//..............................................................................
void TGlPrimitive::Compile()  {
//  if( Compiled() )  return;
  switch( Type )  {
    case sgloSphere:
      glNewList(ListId, GL_COMPILE);
      if( ClipPlanes != NULL )  ClipPlanes->Enable(true);
      if( Basis != NULL  )      glMultMatrixf( Basis->GetMData() );
      CreateQuadric();
      gluSphere(Quadric, Params[0], (int)Params[1], (int)Params[2]);
      if( ClipPlanes != NULL )  ClipPlanes->Enable(false);
      glEndList();
      Compiled = true;
      break;
    case sgloDisk:
      glNewList(ListId, GL_COMPILE);
      if( ClipPlanes != NULL )  ClipPlanes->Enable(true);
      if( Basis != NULL )       glMultMatrixf( Basis->GetMData() );
      CreateQuadric();
      gluDisk(Quadric, Params[0], Params[1], (int)Params[2], (int)Params[3]);
      if( ClipPlanes != NULL )  ClipPlanes->Enable(false);
      glEndList();
      Compiled = true;
      break;
    case sgloDiskSlice:
      glNewList(ListId, GL_COMPILE);
      if( ClipPlanes != NULL )  ClipPlanes->Enable(true);
      if( Basis != NULL )       glMultMatrixf( Basis->GetMData() );
      CreateQuadric();
      gluPartialDisk(Quadric, Params[0], Params[1], (int)Params[2], (int)Params[3], Params[4], Params[5]);
      if( ClipPlanes != NULL )  ClipPlanes->Enable(false);
      glEndList();
      Compiled = true;
      break;
    case sgloCylinder:
      glNewList(ListId, GL_COMPILE);
      if( ClipPlanes != NULL )  ClipPlanes->Enable(true);
      if( Basis != NULL )       glMultMatrixf( Basis->GetMData() );
      CreateQuadric();
      gluCylinder(Quadric, Params[0], Params[1], Params[2], (int)Params[3], (int)Params[4]);
      if( ClipPlanes != NULL )  ClipPlanes->Enable(false);
      glEndList();
      Compiled = true;
      break;
    case sgloCommandList:
      Compiled = true;
      break;
    default:
      Compiled = false;
  }
}
//..............................................................................
void TGlPrimitive::PrepareColorRendering(uint16_t _begin) const {
  if( !Renderer.IsColorStereo() )  {
    glPushAttrib(GL_LIGHTING_BIT);
    glDisable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  }
  glBegin(_begin);
}
//..............................................................................
void TGlPrimitive::EndColorRendering() const {
  glEnd();
  if( !Renderer.IsColorStereo() )
    glPopAttrib();
}
//..............................................................................
void TGlPrimitive::SetColor(const uint32_t& cl) const {
  if( !Renderer.IsColorStereo() )
    glColor4f((float)GetRValue(cl)/255, (float)GetGValue(cl)/255,
    (float)GetBValue(cl)/255, (float)GetAValue(cl)/255);
}
//..............................................................................
void TGlPrimitive::Draw()  {
  if( Compiled )  {  
    glCallList(ListId);  
    return;  
  }
  if( Basis != NULL )
    glMultMatrixf( Basis->GetMData() );
  if( ClipPlanes != NULL )
    ClipPlanes->Enable(true);
  TGlTexture* currentTexture = NULL;
  if( olx_is_valid_index(TextureId) )  {
    TGlTexture* tex = Renderer.GetTextureManager().FindTexture( TextureId );
    currentTexture = new TGlTexture();
    tex->ReadCurrent( *currentTexture );
    tex->SetCurrent();
  }

  if( Type == sgloText )  {
    if( String == NULL || Font == NULL || String->IsEmpty() )   return;
    const GLuint fontbase = Font->GetFontBase();
    /* each character of different colour */
    const size_t StrLen = String->Length();
    if( Colors.Count() == StrLen )  {
      uint32_t prev_color = Colors[0];
      SetColor(prev_color);
      for( size_t i=0; i < StrLen; i++ )  {
        if( prev_color != Colors[i] )  {
          SetColor( Colors[i] );
          prev_color = Colors[i];
        }
        glCallList( fontbase + String->CharAt(i) );
      }
    }
    else  {  /* all characters of the same colour */
      for( size_t i=0; i < StrLen; i++ )
        glCallList( fontbase + String->CharAt(i) );
    }
  }
  else if( Type == sgloPoints )  {
    glPointSize( (float)Params[0]);
    if( Colors.IsEmpty() )  {
      glBegin(GL_POINTS);
      for( size_t  i=0; i < Vertices.Count(); i++ )
        DrawVertex( Vertices[i] );
      glEnd();
    }
    else if( Colors.Count() == Vertices.Count() )  {
      PrepareColorRendering(GL_POINTS);
      for( size_t i=0; i < Vertices.Count(); i++ )
        DrawVertex(Vertices[i], Colors[i]);
      EndColorRendering();
    }
  }
  else if( Type == sgloLines )  {
    float LW = 0;
    if( Params[0] != 1 )  {
      glGetFloatv(GL_LINE_WIDTH, &LW);
      glLineWidth( (float)(Params[0]*LW) );
    }
    if( Colors.IsEmpty() )  {
      glBegin(GL_LINES);
      for( size_t i=0; i < Vertices.Count(); i++ )
        DrawVertex(Vertices[i]);
      glEnd();
    }
    else if( Colors.Count() == Vertices.Count() )  {
      PrepareColorRendering(GL_LINES);
      for( size_t i=0; i < Vertices.Count(); i++ )
        DrawVertex(Vertices[i], Colors[i]);
      EndColorRendering();
    }
    else if( Colors.Count()*2 == Vertices.Count() )  {
      PrepareColorRendering(GL_LINES);
      for( size_t i=0; i < Colors.Count(); i++ )  {
        SetColor(Colors[i]);
        DrawVertex2(i*2);
      }
      EndColorRendering();
    }
    if( LW != 0 ) glLineWidth( (float)LW );
  }
  else if( Type == sgloLineStrip )  {
    float LW = 0;
    if( Params[0] != 1 )  {
      glGetFloatv(GL_LINE_WIDTH, &LW);
      glLineWidth( (float)(Params[0]*LW) );
    }
    glBegin(GL_LINE_STRIP);
    for( size_t i=0; i < Vertices.Count(); i++ )
      DrawVertex(Vertices[i]);
    glEnd();
    if( LW != 0 ) 
      glLineWidth( (float)LW );
  }
  else if( Type == sgloLineLoop )  {
    float LW = 0;
    if( Params[0] != 1 )  {
      glGetFloatv(GL_LINE_WIDTH, &LW);
      glLineWidth( (float)(Params[0]*LW) );
    }
    if( Colors.IsEmpty() )  {
      glBegin(GL_LINE_LOOP);
      for( size_t i=0; i < Vertices.Count(); i++ )
        DrawVertex(Vertices[i]);
      glEnd();
    }
    else if( Colors.Count() == Vertices.Count() )  {
      PrepareColorRendering(GL_LINE_LOOP);
      for( size_t i=0; i < Vertices.Count(); i++ )
        DrawVertex(Vertices[i], Colors[i]);
      EndColorRendering();
    }
    if( LW != 0 ) glLineWidth( (float)LW );
  }
  else if( Type == sgloTriangles )  {
    glBegin(GL_TRIANGLES);
    if( Normals.IsEmpty() )  {
      for( size_t i=0; i < Vertices.Count(); i++ )
        DrawVertex(Vertices[i]);
    }
    else if( Normals.Count()*3 == Vertices.Count() )  {  //+normal
      for( size_t i=0; i < Normals.Count(); i++ )  {
        SetNormal(Normals[i]);
        DrawVertex3(i*3);
      }
    }
    glEnd();
  }
  else if( Type == sgloQuads )  {
    if( TextureCrds.IsEmpty() || !olx_is_valid_index(TextureId) )  {
      if( Colors.IsEmpty() )  {
        if( Normals.IsEmpty() )  {
          glBegin(GL_QUADS);
          for( size_t i=0; i < Vertices.Count(); i++ )
            DrawVertex(Vertices[i]);
          glEnd();
        }
        else if( Normals.Count()*4 == Vertices.Count() ) {
          glBegin(GL_QUADS);
          for( size_t i=0; i < Normals.Count(); i++ )  {
            SetNormal(Normals[i]);
            DrawVertex4(i*4);
          }
          glEnd();
        }
      }
      else if( Colors.Count() == Vertices.Count() )  {
        if( Normals.IsEmpty() )  {
          PrepareColorRendering(GL_QUADS);
          for( size_t i=0; i < Vertices.Count(); i++ )
            DrawVertex(Vertices[i], Colors[i]);
          EndColorRendering();
        }
        else if( Normals.Count()*4 == Vertices.Count() )  {
          PrepareColorRendering(GL_QUADS);
          for( size_t i=0; i < Normals.Count(); i++ )  {
            SetNormal(Normals[i]);
            DrawVertex4c(i*4);
          }
          EndColorRendering();
        }
      }
      else if( Colors.Count()*4 == Vertices.Count() )  {
        if( Normals.IsEmpty() )  {
          PrepareColorRendering(GL_QUADS);
          for( size_t i=0; i < Colors.Count(); i++ )  {
            SetColor(Colors[i]);
            DrawVertex4(i*4);
          }
          EndColorRendering();
        }
        else if( Normals.Count()*4 == Vertices.Count() )  {
          PrepareColorRendering(GL_QUADS);
          for( size_t i=0; i < Normals.Count(); i++ )  {
            SetNormal(Normals[i]);
            SetColor(Colors[i]);
            DrawVertex4(i*4);
          }
          EndColorRendering();
        }
      }
    }
    else if( TextureCrds.Count() == Vertices.Count() ) {
      if( Colors.IsEmpty() )  {
        if( Normals.IsEmpty() )  {
          glBegin(GL_QUADS);
          for( size_t i=0; i < Vertices.Count(); i++ )
            DrawVertex(Vertices[i], TextureCrds[i]);
          glEnd();
        }
        else if( Normals.Count()*4 == Vertices.Count() ) {
          glBegin(GL_QUADS);
          for( size_t i=0; i < Normals.Count(); i++ )  {
            SetNormal(Normals[i]);
            DrawVertex4t(i*4);
          }
          glEnd();
        }
      }
      else if( Colors.Count() == Vertices.Count() )  {
        if( Normals.IsEmpty() )  {
          PrepareColorRendering(GL_QUADS);
          for( size_t i=0; i < Vertices.Count(); i++ )
            DrawVertex(Vertices[i], Colors[i], TextureCrds[i]);
          EndColorRendering();
        }
        else if( Normals.Count()*4 == Vertices.Count() )  {
          PrepareColorRendering(GL_QUADS);
          for( size_t i=0; i < Normals.Count(); i++ )  {
            SetNormal(Normals[i]);
            DrawVertex4ct(i*4);
          }
          EndColorRendering();
        }
      }
      else if( Colors.Count()*4 == Vertices.Count() )  {
        if( Normals.IsEmpty() )  {
          PrepareColorRendering(GL_QUADS);
          for( size_t i=0; i < Colors.Count(); i++ )  {
            SetColor(Colors[i]);
            DrawVertex4t(i*4);
          }
          EndColorRendering();
        }
        else if( Normals.Count()*4 == Vertices.Count() )  {
          PrepareColorRendering(GL_QUADS);
          for( size_t i=0; i < Normals.Count(); i++ )  {
            SetNormal(Normals[i]);
            SetColor(Colors[i]);
            DrawVertex4t(i*4);
          }
          EndColorRendering();
        }
      }
    }
  }
  else if( Type == sgloPolygon )  {
    GLboolean v = glIsEnabled(GL_CULL_FACE);
    if( v )  glDisable(GL_CULL_FACE);
    if( Colors.IsEmpty() )  {
      glBegin(GL_POLYGON);
      for( size_t i=0; i < Vertices.Count(); i++ )
        DrawVertex(Vertices[i]);
      glEnd();
    }
    else if( Colors.Count() == Vertices.Count() )  {
      PrepareColorRendering(GL_POLYGON);
      for( size_t i=0; i < Vertices.Count(); i++ )
        DrawVertex(Vertices[i], Colors[i]);
      EndColorRendering();
    }
    if( v )  glEnable(GL_CULL_FACE);
  }
  if( ClipPlanes != NULL )  ClipPlanes->Enable(false);
  if( currentTexture != NULL )  {
    currentTexture->SetCurrent();
    delete currentTexture;
  }
//  glEnable(GL_LIGHTING);
}
//..............................................................................
AGOProperties& TGlPrimitive::SetProperties(const AGOProperties& C)  {
  if( Properties != NULL )  {
    if( !(C == GetProperties()) )  // properties will be removed if ObjectCount == 1
      Renderer.OnSetProperties(GetProperties());
  }
  TGlMaterial& Props = (TGlMaterial&)AGroupObject::SetProperties(C);
  Renderer.SetProperties(Props);
  return Props;
}
//..............................................................................

