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
TGlPrimitive::TGlPrimitive(TObjectGroup *ParentG, TGlRender *ParentR):
  AGroupObject(ParentG)
{
  FQuadric = NULL;
  FEval = NULL;
  FParentRender = ParentR;
  FTexture = -1;
  FQuadricDrawStyle = GLU_FILL;
  FQuadricNormals = GLU_SMOOTH;
  FQuadricOrientation = GLU_OUTSIDE;
  FList = false;
  FCompiled = false;
  FParentCollection = NULL;
  FGlClipPlanes = NULL;
  FBasis = NULL;
  FString = NULL;
  FFont = NULL;
}
//..............................................................................
TGlPrimitive::~TGlPrimitive()  {
  if( FQuadric )  gluDeleteQuadric(FQuadric);
  if( FBasis )     delete FBasis;
  if( FGlClipPlanes )  delete FGlClipPlanes;
}
//..............................................................................
AGOProperties *TGlPrimitive::NewProperties()  {
  TGlMaterial *GlM = new TGlMaterial;
  return GlM;
}
//..............................................................................
void TGlPrimitive::CreateQuadric()  {
  if( FQuadric )  return;
  FQuadric = gluNewQuadric();
  if( !FQuadric )
    throw TOutOfMemoryException(__OlxSourceInfo);
  if( FTexture != -1 )  {
    glBindTexture(GL_TEXTURE_2D, FTexture);
    gluQuadricTexture(FQuadric, GL_TRUE);
  }
  else
    gluQuadricTexture(FQuadric, GL_FALSE);

  gluQuadricOrientation( FQuadric, FQuadricOrientation);
  gluQuadricDrawStyle( FQuadric, FQuadricDrawStyle);
  gluQuadricNormals( FQuadric, FQuadricNormals);
}
//..............................................................................
void TGlPrimitive::Type(short T)  {
  FType = T;
  switch( FType )  {
    case sgloText:
      FParams.Resize(4);
      FParams[0] = 1;  FParams[1] = 1;  FParams[2] = 1; FParams[3] = 1;
      FList = false;
      break;
    case sgloSphere:
      FParams.Resize(3);
      FParams[0] = 1;  FParams[1] = 5;  FParams[2] = 5;
      FList = true;
      break;
    case sgloDisk:
      FParams.Resize(4);
      FParams[0] = 0;  FParams[1] = 1;  FParams[2] = 5;  FParams[3] = 5;
      FList = true;
      break;
    case sgloDiskSlice:
      FParams.Resize(6);
      FParams[0] = 0;  FParams[1] = 1;  FParams[2] = 5;  FParams[3] = 5;
      FParams[4] = 0;  FParams[5] = 90;
      FList = true;
      break;
    case sgloCylinder:
      FParams.Resize(5);
      FParams[0] = 0;  FParams[1] = 1;  FParams[2] = 1;  FParams[3] = 5;
      FParams[4] = 5;
      FList = true;
      break;
    case sgloCommandList:
      FList = true;
      FCompiled = true;
      break;
    default:
      FParams.Resize(1); // point size or line width
      FParams[0] = 1;  // default point size and line width
      FList = false;
      break;
  }
  if( FList )  FId = FParentRender->NewListId();

}
//..............................................................................
void TGlPrimitive::ListParams(TStrList &List)  {
  switch( FType )  {
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
      FParentCollection->ListParams(List, this);
      break;    
  }
}
//..............................................................................
void TGlPrimitive::Compile()  {
//  if( Compiled() )  return;
  switch( FType )  {
    case sgloSphere:
      glNewList(FId, GL_COMPILE);
      if( GlClipPlanes() )  GlClipPlanes()->Enable(true);
      if( Basis() )         glMultMatrixf( Basis()->GetMData() );
      CreateQuadric();
      gluSphere(FQuadric, FParams[0], (int)FParams[1], (int)FParams[2]);
      if( GlClipPlanes() )  GlClipPlanes()->Enable(false);
      glEndList();
      FCompiled = true;
      break;
    case sgloDisk:
      glNewList(FId, GL_COMPILE);
      if( GlClipPlanes() )  GlClipPlanes()->Enable(true);
      if( Basis() )         glMultMatrixf( Basis()->GetMData() );
      CreateQuadric();
      gluDisk(FQuadric, FParams[0], FParams[1], (int)FParams[2], (int)FParams[3]);
      if( GlClipPlanes() )  GlClipPlanes()->Enable(false);
      glEndList();
      FCompiled = true;
      break;
    case sgloDiskSlice:
      glNewList(FId, GL_COMPILE);
      if( GlClipPlanes() )  GlClipPlanes()->Enable(true);
      if( Basis() )         glMultMatrixf( Basis()->GetMData() );
      CreateQuadric();
      gluPartialDisk(FQuadric, FParams[0], FParams[1], (int)FParams[2], (int)FParams[3], FParams[4], FParams[5]);
      if( GlClipPlanes() )  GlClipPlanes()->Enable(false);
      glEndList();
      FCompiled = true;
      break;
    case sgloCylinder:
      glNewList(FId, GL_COMPILE);
      if( GlClipPlanes() )  GlClipPlanes()->Enable(true);
      if( Basis() )         glMultMatrixf( Basis()->GetMData() );
      CreateQuadric();
      gluCylinder(FQuadric, FParams[0], FParams[1], FParams[2], (int)FParams[3], (int)FParams[4]);
      if( GlClipPlanes() )  GlClipPlanes()->Enable(false);
      glEndList();
      FCompiled = true;
      break;
    case sgloCommandList:
      FCompiled = true;
      FList = true;
      break;
    default:
      FCompiled = false;
  }
}
//..............................................................................
void TGlPrimitive::Draw()  {
  if( FList )  {  glCallList(FId);  return;  }

  if( Basis() )  glMultMatrixf( Basis()->GetMData() );
  if( GlClipPlanes() )  GlClipPlanes()->Enable(true);
  TGlTexture* currentTexture = NULL;
  if( FTexture != -1 )  {
    TGlTexture* tex = FParentRender->GetTextureManager().FindTexture( FTexture );
    currentTexture = new TGlTexture();
    tex->ReadCurrent( *currentTexture );
    tex->SetCurrent();
  }

  if( FType == sgloText )  {
    if( FString == NULL )   return;
    if( FFont == NULL )     return;
    const int fontbase = FFont->FontBase();
    /* each character of different colour */
    int StrLen = FString->Length();
    if( FData.Elements() == StrLen )  {
      for( int i=0; i < StrLen; i++ )  {
        int Cl = (int)FData[0][i];
        glColor4b(  GetRValue(Cl), GetGValue(Cl), GetBValue(Cl), GetAValue(Cl));
        glCallList( fontbase + FString->CharAt(i) );
      }
    }
    else  {  /* all characters of the same colour */
      for( int i=0; i < StrLen; i++ )  {
        glCallList( fontbase + FString->CharAt(i) );
//        raster_pos[0] += Font()->MaxWidth();  
//        glRasterPos3d(raster_pos[0], raster_pos[1], raster_pos[2]);
      }
      //switch( olxstr::CharSize )  {
      //  case 1:
      //    glCallLists(StrLen, GL_UNSIGNED_BYTE, FString->raw_str() );
      //  case 2:
      //    glCallLists(StrLen, GL_UNSIGNED_SHORT, FString->raw_str() );
      //  case 4:
      //    glCallLists(StrLen, GL_UNSIGNED_INT, FString->raw_str() );
      //  default:
      //    glCallLists(StrLen, GL_UNSIGNED_BYTE, FString->c_str() );
      //}
    }
  }
  else if( FType == sgloPoints )  {
    glPointSize( (float)FParams[0]);
    if( FData.Vectors() == 3 )  {
      glBegin(GL_POINTS);
      for( int  i=0; i < FData.Elements(); i++ )
        glVertex3d( FData[0][i], FData[1][i], FData[2][i] );
      glEnd();
    }
    else if( FData.Vectors() == 4 )  {
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_POINTS);
      for( int i=0; i < FData.Elements(); i++ )  {
        glEnable(GL_COLOR_MATERIAL);
        int Cl = (int)FData[3][i];
        glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
          (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
        glVertex3d( FData[0][i], FData[1][i], FData[2][i] );
      }
      glEnd();
      glDisable(GL_COLOR_MATERIAL);
    }
  }
  else if( FType == sgloLines )  {
    float LW = 0;
    if( FParams[0] != 1 )  {
      glGetFloatv(GL_LINE_WIDTH, &LW);
      glLineWidth( (float)(FParams[0]*LW) );
    }
    if( FData.Vectors() == 3 )  {
      glBegin(GL_LINES);
      for( int i=0; i < FData.Elements(); i++ )
        glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
      glEnd();
    }
    else if( FData.Vectors() == 4 )  {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glDisable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_LINES);
      for( int i=0; i < FData.Elements(); i++ )  {
        if( !(i%2) )  {
          int Cl = (int)FData[3][i];
          glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
            (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
        }
        glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
      }
      glEnd();
      glPopAttrib();
    }
    if( LW != 0 ) glLineWidth( (float)LW );
  }
  else if( FType == sgloLineStrip )  {
    float LW = 0;
    if( FParams[0] != 1 )  {
      glGetFloatv(GL_LINE_WIDTH, &LW);
      glLineWidth( (float)(FParams[0]*LW) );
    }
    glBegin(GL_LINE_STRIP);
    for( int i=0; i < FData.Elements(); i++ )
      glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
    glEnd();
    if( LW != 0 ) glLineWidth( (float)LW );
  }
  else if( FType == sgloLineLoop )  {
    float LW = 0;
    if( FParams[0] != 1 )  {
      glGetFloatv(GL_LINE_WIDTH, &LW);
      glLineWidth( (float)(FParams[0]*LW) );
    }
    if( FData.Vectors() == 3 )  {
      glBegin(GL_LINE_LOOP);
      for( int i=0; i < FData.Elements(); i++ )
        glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
      glEnd();
    }
    else if( FData.Vectors() == 4 )  {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glDisable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_LINE_LOOP);
      for( int i=0; i < FData.Elements(); i++ )  {
        int Cl = (unsigned int)(FData[3][i]);
        glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
          (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
        glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
      }
      glEnd();
      glPopAttrib();
    }
    if( LW != 0 ) glLineWidth( (float)LW );
  }
  else if( FType == sgloTriangles )  {
    glBegin(GL_TRIANGLES);
    if( FData.Vectors() == 3 )  {
      for( int i=0; i < FData.Elements(); i++ )
        glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
    }
    else if( FData.Vectors() == 4 )  {  //+normal
      for( int i=0; i < FData.Elements(); i++ )  {
        if( (i%3) == 0 )  {
          const int ni = i/3;
          glNormal3d(FData[3][ni], FData[3][ni+1], FData[3][ni+2]);
        }
        glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
      }
    }
    glEnd();
  }
  else if( FType == sgloQuads )  {
    if( FData.Vectors() == 3 )  {
      glBegin(GL_QUADS);
      for( int i=0; i < FData.Elements(); i++ )
        glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
      glEnd();
    }
    else  if( FData.Vectors() == 5 )  {
      glBegin(GL_QUADS);
      if( FTexture != -1 )  {
        for( int i=0; i < FData.Elements(); i++ )  {
          glTexCoord2d( FData[3][i], FData[4][i] );
          glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
        }
      }
      else  {
        for( int i=0; i < FData.Elements(); i++ )  {
          glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
        }
      }
      glEnd();
    }
    else  if( FData.Vectors() == 4 )  {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glDisable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_QUADS);
      for( int i=0; i < FData.Elements(); i++ )  {
        int Cl = (int)(FData[3][i]);
        glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
          (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
        glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
      }
      glEnd();
      glPopAttrib();
    }
    else  if( FData.Vectors() == 6 )  {
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glDisable(GL_LIGHTING);
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_QUADS);
      if( FTexture != -1 )  {
        for( int i=0; i < FData.Elements(); i++ )  {
          int Cl = (int)(FData[3][i]);
          glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
            (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
          glTexCoord2d( FData[4][i], FData[5][i] );
          glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
        }
      }
      else  {
        for( int i=0; i < FData.Elements(); i++ )  {
          int Cl = (int)(FData[3][i]);
          glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
            (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
          glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
        }
      }
      glEnd();
      glPopAttrib();
    }
  }
  else if( FType == sgloPolygon )  {
    GLboolean v = glIsEnabled(GL_CULL_FACE);
    if( v )  glDisable(GL_CULL_FACE);
    if( FData.Vectors() == 3 )  {
      glBegin(GL_POLYGON);
      for( int i=0; i < FData.Elements(); i++ )
        glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
      glEnd();
    }
    else if( FData.Vectors() == 4 )  {
      glEnable(GL_COLOR_MATERIAL);
      glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
      glBegin(GL_POLYGON);
      for( int i=0; i < FData.Elements(); i++ )  {
        int Cl = (int)(FData[3][i]);
        glColor4d(  (double)GetRValue(Cl)/255, (double)GetGValue(Cl)/255, 
          (double)GetBValue(Cl)/255, (double)GetAValue(Cl)/255);
        glVertex3d(FData[0][i], FData[1][i], FData[2][i]);
      }
      glDisable(GL_COLOR_MATERIAL);
      glEnd();
    }
    if( v )  glEnable(GL_CULL_FACE);
  }
  if( GlClipPlanes() )  GlClipPlanes()->Enable(false);
  if( currentTexture )  {
    currentTexture->SetCurrent();
    delete currentTexture;
  }
//  glEnable(GL_LIGHTING);
}
//..............................................................................
AGOProperties * TGlPrimitive::SetProperties( const AGOProperties *C)
{
  if( GetProperties() )
  {
    if( !(*C == *GetProperties()) )  // properties will be removed if ObjectCount == 1
    {
      Render()->OnSetProperties((TGlMaterial*)GetProperties());
    }
  }
  TGlMaterial* Props = (TGlMaterial*)AGroupObject::SetProperties(C);
  Render()->SetProperties(Props);
  return Props;
}
//..............................................................................
void TGlPrimitive::StartList(){  glNewList(FId, GL_COMPILE_AND_EXECUTE); }
//..............................................................................
void TGlPrimitive::CallList( TGlPrimitive *GlP ){  
  if( GlP->FList )
    glCallList(GlP->Id()); 
  else
    GlP->Draw();
}
//..............................................................................
void TGlPrimitive::EndList()        {  glEndList(); }
//..............................................................................
bool TGlPrimitive::Compiled() const {  return FCompiled; }
//..............................................................................

