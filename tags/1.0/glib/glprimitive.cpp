//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

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
  TextureId = ListId = OwnerId = -1;
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
  if( TextureId != -1 )  {
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
void TGlPrimitive::Draw()  {
  if( Compiled )  {  
    glCallList(ListId);  
    return;  
  }

  if( Basis != NULL )       glMultMatrixf( Basis->GetMData() );
  if( ClipPlanes != NULL )  ClipPlanes->Enable(true);
  TGlTexture* currentTexture = NULL;
  if( TextureId != -1 )  {
    TGlTexture* tex = Renderer.GetTextureManager().FindTexture( TextureId );
    currentTexture = new TGlTexture();
    tex->ReadCurrent( *currentTexture );
    tex->SetCurrent();
  }

  if( Type == sgloText )  {
    if( String == NULL || Font == NULL )   return;
    const int fontbase = Font->FontBase();
    /* each character of different colour */
    const int StrLen = String->Length();
    if( Data.Elements() == StrLen )  {
      for( int i=0; i < StrLen; i++ )  {
        const int Cl = (int)Data[0][i];
        glColor4b(  GetRValue(Cl), GetGValue(Cl), GetBValue(Cl), GetAValue(Cl));
        glCallList( fontbase + String->CharAt(i) );
      }
    }
    else  {  /* all characters of the same colour */
      for( int i=0; i < StrLen; i++ )
        glCallList( fontbase + String->CharAt(i) );
    }
  }
  else if( Type == sgloPoints )  {
    glPointSize( (float)Params[0]);
    if( Data.Vectors() == 3 )  {
      glBegin(GL_POINTS);
      for( int  i=0; i < Data.Elements(); i++ )
        glVertex3d( Data[0][i], Data[1][i], Data[2][i] );
      glEnd();
    }
    else if( Data.Vectors() == 4 )  {
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_POINTS);
      for( int i=0; i < Data.Elements(); i++ )  {
        glEnable(GL_COLOR_MATERIAL);
        const int Cl = (int)Data[3][i];
        glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
          (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
        glVertex3d( Data[0][i], Data[1][i], Data[2][i] );
      }
      glEnd();
      glDisable(GL_COLOR_MATERIAL);
    }
  }
  else if( Type == sgloLines )  {
    float LW = 0;
    if( Params[0] != 1 )  {
      glGetFloatv(GL_LINE_WIDTH, &LW);
      glLineWidth( (float)(Params[0]*LW) );
    }
    if( Data.Vectors() == 3 )  {
      glBegin(GL_LINES);
      for( int i=0; i < Data.Elements(); i++ )
        glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
      glEnd();
    }
    else if( Data.Vectors() == 4 )  {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glDisable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_LINES);
      for( int i=0; i < Data.Elements(); i++ )  {
        if( !(i%2) )  {
          int Cl = (int)Data[3][i];
          glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
            (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
        }
        glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
      }
      glEnd();
      glPopAttrib();
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
    for( int i=0; i < Data.Elements(); i++ )
      glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
    glEnd();
    if( LW != 0 ) glLineWidth( (float)LW );
  }
  else if( Type == sgloLineLoop )  {
    float LW = 0;
    if( Params[0] != 1 )  {
      glGetFloatv(GL_LINE_WIDTH, &LW);
      glLineWidth( (float)(Params[0]*LW) );
    }
    if( Data.Vectors() == 3 )  {
      glBegin(GL_LINE_LOOP);
      for( int i=0; i < Data.Elements(); i++ )
        glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
      glEnd();
    }
    else if( Data.Vectors() == 4 )  {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glDisable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_LINE_LOOP);
      for( int i=0; i < Data.Elements(); i++ )  {
        int Cl = (unsigned int)(Data[3][i]);
        glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
          (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
        glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
      }
      glEnd();
      glPopAttrib();
    }
    if( LW != 0 ) glLineWidth( (float)LW );
  }
  else if( Type == sgloTriangles )  {
    glBegin(GL_TRIANGLES);
    if( Data.Vectors() == 3 )  {
      for( int i=0; i < Data.Elements(); i++ )
        glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
    }
    else if( Data.Vectors() == 4 )  {  //+normal
      for( int i=0; i < Data.Elements(); i++ )  {
        if( (i%3) == 0 )  {
          const int ni = i/3;
          glNormal3d(Data[3][ni], Data[3][ni+1], Data[3][ni+2]);
        }
        glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
      }
    }
    glEnd();
  }
  else if( Type == sgloQuads )  {
    if( Data.Vectors() == 3 )  {
      glBegin(GL_QUADS);
      for( int i=0; i < Data.Elements(); i++ )
        glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
      glEnd();
    }
    else  if( Data.Vectors() == 5 )  {
      glBegin(GL_QUADS);
      if( TextureId != -1 )  {
        for( int i=0; i < Data.Elements(); i++ )  {
          glTexCoord2d( Data[3][i], Data[4][i] );
          glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
        }
      }
      else  {
        for( int i=0; i < Data.Elements(); i++ )
          glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
      }
      glEnd();
    }
    else  if( Data.Vectors() == 4 )  {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glDisable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_QUADS);
      for( int i=0; i < Data.Elements(); i++ )  {
        int Cl = (int)(Data[3][i]);
        glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
          (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
        glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
      }
      glEnd();
      glPopAttrib();
    }
    else  if( Data.Vectors() == 6 )  {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glDisable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_QUADS);
      if( TextureId != -1 )  {
        for( int i=0; i < Data.Elements(); i++ )  {
          int Cl = (int)(Data[3][i]);
          glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
            (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
          glTexCoord2d( Data[4][i], Data[5][i] );
          glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
        }
      }
      else  {
        for( int i=0; i < Data.Elements(); i++ )  {
          int Cl = (int)(Data[3][i]);
          glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
            (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
          glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
        }
      }
      glEnd();
      glPopAttrib();
    }
  }
  else if( Type == sgloPolygon )  {
    GLboolean v = glIsEnabled(GL_CULL_FACE);
    if( v )  glDisable(GL_CULL_FACE);
    if( Data.Vectors() == 3 )  {
      glBegin(GL_POLYGON);
      for( int i=0; i < Data.Elements(); i++ )
        glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
      glEnd();
    }
    else if( Data.Vectors() == 4 )  {
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_POLYGON);
      for( int i=0; i < Data.Elements(); i++ )  {
        int Cl = (int)(Data[3][i]);
        glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
          (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
        glVertex3d(Data[0][i], Data[1][i], Data[2][i]);
      }
      glDisable(GL_COLOR_MATERIAL);
      glEnd();
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

