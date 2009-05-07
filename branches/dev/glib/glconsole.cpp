//----------------------------------------------------------------------------//
// namespace TEXLib
// TGlConsole - a console
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "glconsole.h"
#include "glrender.h"
#include "actions.h"
#include "glcursor.h"
#include "gpcollection.h"
#include "glfont.h"
#include "glmouse.h"
#include "glscene.h"

#include "styles.h"

#include "integration.h"
#include "bapp.h"

// keyboard constanst
#include "wx/defs.h"

/*
  There is a slight problem with cursor - depending on object properties it might
  be drawn before the console, and then its position is validated after it is drawn
  in previous position!  ...
*/

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlConsole::TGlConsole(const olxstr& collectionName, TGlRenderer *Render) :
  AGDrawObject(collectionName)  {

  AGDrawObject::Parent(Render);
  FLineSpacing = 0;
  Left = Top = 0;
  Width = Height = 100;
  PromptVisible = true;
  SkipPosting = false;
  Blend = true;
  FActions = new TActionQList;
  OnCommand = &FActions->NewQueue("ONCOMMAND");
  OnPost = &FActions->NewQueue("ONPOST");
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FShowBuffer = true;
  Groupable(false);
  FontIndex = -1;
  FTxtPos = -1;
  FMaxLines = 1000;
  MaxLineWidth = FLineWidth = 80;
  FScrollDirectionUp = true;
  FLinesToShow = -1;
  FCmdPos = -1;
  FCursor = new TGlCursor("Cursor", Render);
  //FCursor->Visible(false);

  SetToDelete(false);
  Render->BeforeDraw->Add(this);
}
//..............................................................................
TGlConsole::~TGlConsole()  {
  FParent->BeforeDraw->Remove(this);
  ClearBuffer();
  delete FActions;
  delete FCursor;
}
//..............................................................................
void TGlConsole::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  if( GPC == NULL )    
    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGraphicsStyle *GS = GPC->Style();

  FLineWidth = GS->GetParam("LineWidth", FLineWidth, true).ToInt();
  FLinesToShow = GS->GetParam("LinesToShow", FLinesToShow, true).ToInt();
  FLineSpacing = GS->GetParam("LineSpacing", "0", true).ToDouble();
  InviteStr = GS->GetParam("Prompt", ">>", true);

  TGlMaterial* GlM = const_cast<TGlMaterial*>(GS->Material("Text"));
  if( GlM->Mark() )
    *GlM = Font()->GetMaterial();

  TGlPrimitive* GlP = GPC->NewPrimitive("Text", sgloText);
  GlP->SetProperties(GlM);
  GlP->Params[0] = -1;  //bitmap; TTF by default
  FCursor->Create();
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FStringPos = FCommand.Length();
  Cmds.Clear();
  Cmds.Add( FCommand );
}
//..............................................................................
bool TGlConsole::Orient(TGlPrimitive *P)  {
  TGlFont *Fnt = Font();
  if( Fnt == NULL )  return true;
//  Fnt->DrawGlText( vec3d(0,0,0), "HELLW_O", true);
  P->SetFont(Fnt);
  if( FParent->GetWidth() < 100 )  return true;
  int th = Fnt->TextHeight(EmptyString), lc, ii;
  double Scale = FParent->GetScale(),
         MaxY = ((double)FParent->GetHeight()/2-Top-th)*Scale;
  double MaxZ = -FParent->GetMaxRasterZ();

  MaxZ += 0.02;
  
  olxstr line;

  vec3d T;

  TGlMaterial *OGlM = (TGlMaterial*)P->GetProperties();
  TGlOption CC, Ambient;
  CC = FParent->LightModel.ClearColor();
  TGlMaterial GlM;
  GlM = *(TGlMaterial*)P->GetProperties();  // copy properties
//  GlM.AmbientF[0] = 1-CC[0];  GlM.AmbientF[1] = 1-CC[1];  GlM.AmbientF[2] = 1-CC[2];
  Ambient = GlM.AmbientF;
  if( ShowBuffer() )  {
    float Rk=1, Gk=1, Bk=1;
    lc = (int)(((float)Height-0.1)/(th*(FLineSpacing+1))); // calc the number of lines
    if( PromptVisible )  lc -= Cmds.Count();
    if( lc != 0 )  {
      Rk = (CC[0] - Ambient[0])/lc;
      Gk = (CC[1] - Ambient[1])/lc;
      Bk = (CC[2] - Ambient[2])/lc;
    }
    for( int i=FTxtPos; i >= olx_max(0, FTxtPos-lc); i-- )  {
      if( FBuffer[i].IsEmpty() )  continue;
      ii = (FTxtPos-i);
      if( FLinesToShow >= 0 && ii > FLinesToShow )  continue;
      if( IsPromptVisible() )  ii += Cmds.Count();

      T[0] = GlLeft;  T[1] = GlTop + ii*LineInc;
      T *= Scale;
      if( T[1] > MaxY )  break;
      if( FBuffer[i].Length() > MaxLineWidth )  
        line = FBuffer[i].SubStringTo(MaxLineWidth);
      else
        line = FBuffer[i];
      // drawing spaces is not required ...
      int stlen = line.Length();
      if( stlen == 0 )  continue;
      while( line.CharAt(stlen-1) == ' ' && stlen > 1 ) stlen--;
      line = line.SubStringTo(stlen);

      glRasterPos3d(T[0], T[1], MaxZ);
      TGlMaterial* GlMP = FBuffer.GetObject(i);
      if( GlMP != NULL ) 
        GlMP->Init();
      else if( IsBlend() )  {
        GlM.AmbientF[0] = Ambient[0] + (float)ii*Rk;
        GlM.AmbientF[1] = Ambient[1] + (float)ii*Gk;
        GlM.AmbientF[2] = Ambient[2] + (float)ii*Bk;
        GlM.Init(); // fading the text
      }
      else
        OGlM->Init();
      P->SetString( &line );
      FParent->DrawText(*P, T[0], T[1], MaxZ); 
      P->SetString(NULL);
    }
  }
  if( PromptVisible )  {
    GlM.AmbientF =Ambient;
    GlM.Init();
    if( Cmds.Count() == 1 )  {
      T[0] = GlLeft;  T[1] = GlTop;
      T *= Scale;
      P->SetString(&FCommand);
      FParent->DrawText(*P, T[0], T[1], MaxZ); 
      P->SetString(NULL);
    }
    else  {
      for( int i=Cmds.Count()-1; i >= 0 ; i-- )  {
        if( FLinesToShow >= 0 && ii > FLinesToShow )  continue;
        ii = Cmds.Count()-i-1;
        T[0] = GlLeft;  T[1] = GlTop + ii*LineInc;
        T *= Scale;
        P->SetString(&Cmds[i]);
        FParent->DrawText(*P, T[0], T[1], MaxZ); 
        P->SetString(NULL);
      }
    }
  }
  OGlM->Init(); // restore the material properties
  FCursor->Draw();
  return true;
}
//..............................................................................
bool TGlConsole::WillProcessKey( int Key, short ShiftState )  {
  if( Key == WXK_DELETE )  {
    if( ShiftState == 0 && StringPosition() < FCommand.Length() &&
         StringPosition() >= PromptStr.Length() )  {
      return true;
    }
  }
  else if( Key == WXK_BACK && StringPosition() != PromptStr.Length() )
    return true;
  return false;
}
//..............................................................................
bool TGlConsole::ProcessKey( int Key , short ShiftState)  {
  if( (Key == WXK_UP) && IsPromptVisible() && ShiftState == 0 )  {
    FCmdPos --;
    if( FCmdPos < 0 )  FCmdPos = FCommands.Count()-1;
    if( FCmdPos >= 0 && FCmdPos < FCommands.Count() )  {
      olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
      FCommand = PromptStr;
      FCommand << FCommands[FCmdPos];
      StringPosition(  FCommand.Length() );
    }
    return true;
  }
  if( (Key == WXK_DOWN) && IsPromptVisible() && ShiftState == 0 )  {
    FCmdPos ++;
    if( FCmdPos >= FCommands.Count() )  FCmdPos = 0;
    if( FCmdPos >= 0 && FCmdPos < FCommands.Count() )  {
      olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
      FCommand = PromptStr;
      FCommand << FCommands[FCmdPos];
      StringPosition(  FCommand.Length() );
    }
    return true;
  }
  if( (Key == WXK_LEFT) && IsPromptVisible() && (StringPosition() > PromptStr.Length()) )  {
    if( ShiftState == 0 )  {
      StringPosition( StringPosition()-1 );
      return true;
    }
    else if( ShiftState == sssCtrl  )  {
      int ind = FCommand.LastIndexOf(' ', StringPosition()-1);
      if( ind >= 0 )
        StringPosition( ind );
      else
        StringPosition( PromptStr.Length() );
      return true;
    }
    return false;
  }
  if( (Key == WXK_RIGHT) && IsPromptVisible() && (StringPosition() < FCommand.Length()) )  {
    if( ShiftState == 0 )  {
      StringPosition( StringPosition()+1 );
      return true;
    }
    else if( ShiftState == sssCtrl )  {
      int ind = FCommand.FirstIndexOf(' ', StringPosition()+1);
      if( ind >= 0 )
        StringPosition( ind );
      else
        StringPosition( FCommand.Length() );
      return true;
    }
    return false;
  }
  if( Key == WXK_DELETE )  {
    if( ShiftState == 0 && StringPosition() < FCommand.Length() &&
         StringPosition() >= PromptStr.Length() )  {
      FCommand.Delete(StringPosition(), 1);
      UpdateCursorPosition(true);
      return true;
    }
    return false;
  }
  if( (Key == WXK_HOME) && !ShiftState  && IsPromptVisible() )  {
    StringPosition( PromptStr.Length() );
    return true;
  }
  if( (Key == WXK_END) && !ShiftState  && IsPromptVisible() )  {
    StringPosition( FCommand.Length() );
    return true;
  }
  if( Key == WXK_PAGEUP || Key == WXK_PAGEDOWN )  {
    TGlFont *Fnt = Font();
    if( !Fnt )  return true;
    int th = Fnt->TextHeight(""), lc;
    if( FLinesToShow == -1 )
      lc = (int)((float)Height/(th*(1.0+FLineSpacing))*FParent->GetViewZoom()); // calc the number of lines
    else
      lc = FLinesToShow;
    if( Key == WXK_PAGEDOWN )  {
      FTxtPos += lc;
      if( FTxtPos >= FBuffer.Count() )
        FTxtPos = FBuffer.Count()-1;
    }
    if( Key == WXK_PAGEUP )  {
      FTxtPos -= lc;
      if( FTxtPos < 0 )
        FTxtPos = FBuffer.Count()!=0 ? 0 : -1;
    }
    StringPosition( FCommand.Length() );
    return true;
  }
  if( !Key || Key > 255 || (ShiftState & sssCtrl) || (ShiftState & sssAlt))  return false;
  if( !IsPromptVisible() )  return false;
  
  if( Key == WXK_ESCAPE )  {
    olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
    FCommand = PromptStr;
    StringPosition( FCommand.Length() );
    return true;
  }
  if( Key == WXK_RETURN )  {
    FCommand = GetCommand();
    if( FCommand.Length() )  {
      if( FCommands.Count() != 0 && FCommands[FCommands.Count()-1] == FCommand)  {
        ;
      }
      else  {
        FCommands.Add(FCommand);
      }
      FCmdPos = FCommands.Count();
    }
    OnCommand->Execute(dynamic_cast<IEObject*>((AActionHandler*)this) );
    if( FCommand.IsEmpty() )  {
      olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
      FCommand = PromptStr;
    }
    StringPosition( FCommand.Length() );
    return true;
  }
  if( Key == WXK_BACK )  {
    if( FCommand.Length() > PromptStr.Length() )  {
      if( StringPosition() == FCommand.Length() )  {
        FCommand.SetLength(FCommand.Length()-1);
        StringPosition( FCommand.Length() );
      }
      else  {  // works like delete
        if( StringPosition() > PromptStr.Length() )  {
          FCommand.Delete(StringPosition()-1, 1);
          StringPosition( StringPosition()-1 );
        }
      }
    }
    return true;
  }
  if( StringPosition() == FCommand.Length() )
    FCommand << (char)Key;
  else
    FCommand.Insert((char)Key, StringPosition() );
  StringPosition( StringPosition()+1 );
  return true;
}
//..............................................................................
void TGlConsole::PrintText(const olxstr &S, TGlMaterial *M, bool Hypernate)  {
  if( IsSkipPosting() )  {
    //SetSkipPosting(false);
    return;
  }
  olxstr Tmp(S);
  bool SingleLine = false;
  for(int i=0; i < Tmp.Length(); i++ )  {
    if( Tmp[i] == '\t' )  {
      Tmp[i] = ' ';
      int count = 8-i%8-1;
      if( count > 0 ) Tmp.Insert(' ', i, count);
    }
  }
  if( Hypernate )  {
    TStrList Txt;
    Txt.Hypernate(S, FLineWidth, true);
    if( Txt.Count() > 1 )  PrintText(Txt, M);
    else                   SingleLine = true;
  }
  if( !Hypernate || SingleLine )  {
    TGlMaterial *GlM = NULL;
    if( M != NULL )  {  GlM = new TGlMaterial;  *GlM = *M;  }
    if( !FBuffer.IsEmpty() && FBuffer.LastStr().IsEmpty() )  {
      FBuffer.Last().String = Tmp;
      /* this line is added after memory leak analysis by Compuware DevPartner 8.2 trial */
      if( FBuffer.Last().Object != NULL )
        delete FBuffer.Last().Object;
      FBuffer.Last().Object = GlM;
    }
    else
      FBuffer.Add(Tmp, GlM);
    OnPost->Execute(dynamic_cast<IEObject*>((AActionHandler*)this), &Tmp );
  }

  KeepSize();
  FTxtPos = FBuffer.Count()-1;
  //FBuffer.Add(EmptyString);
}
//..............................................................................
void TGlConsole::PrintText(const TStrList &SL, TGlMaterial *M, bool Hypernate)  {
  if( IsSkipPosting() )  {
    //SetSkipPosting(false);
    return;
  }
  TStrList Txt;
  olxstr Tmp;
  for( int i=0; i < SL.Count(); i++ )  {
    Tmp = SL[i];
    for(int j=0; j < Tmp.Length(); j++ )  {
      if( Tmp.CharAt(j) == '\t' )  {
        Tmp[j] = ' ';
        int count = 8-j%8-1;
        if( count > 0 ) Tmp.Insert(' ', j, count);
      }
    }
    if( Hypernate )  {
      Txt.Clear();
      Txt.Hypernate(Tmp, FLineWidth, true);
      for( int j=0; j < Txt.Count(); j++ )  {
        TGlMaterial *GlM = NULL;
        if( M != NULL )  {  GlM = new TGlMaterial;  *GlM = *M;  }
        if( j == 0 && !FBuffer.IsEmpty() && FBuffer.LastStr().IsEmpty() )  {
          FBuffer.Last().String = Txt[j];
          FBuffer.Last().Object = GlM;
        }
        else
          FBuffer.Add(Txt[j], GlM);
        OnPost->Execute(dynamic_cast<IEObject*>((AActionHandler*)this), &Txt[j] );
      }
    }
    else  {
      TGlMaterial *GlM = NULL;
      if( M != NULL )  {  GlM = new TGlMaterial;  *GlM = *M;  }
      if( !FBuffer.IsEmpty() && FBuffer.LastStr().IsEmpty() )  {
        FBuffer.Last().String = Tmp;
        FBuffer.Last().Object = GlM;
      }
      else
        FBuffer.Add(Tmp, GlM);
      OnPost->Execute(dynamic_cast<IEObject*>((AActionHandler*)this), &Tmp );
    }
  }
  KeepSize();
  FTxtPos = FBuffer.Count()-1;
  FBuffer.Add(EmptyString);
}
//..............................................................................
olxstr TGlConsole::GetCommand() const  {
  if( FCommand.StartsFrom( PromptStr ) )
    return  FCommand.SubStringFrom( PromptStr.Length() );
  return (!FCommand.IsEmpty()) ? FCommand :EmptyString;
}
//..............................................................................
void TGlConsole::SetCommand(const olxstr &NewCmd)  {
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FCommand << NewCmd;
  StringPosition( FCommand.Length() );
}
//..............................................................................
void TGlConsole::ClearBuffer()  {
  int lc = FBuffer.Count();
  for(int i=0; i < lc; i++ )
    if( FBuffer.GetObject(i) != NULL )
      delete (TGlMaterial*)FBuffer.GetObject(i);
  FBuffer.Clear();
  //FCommand = FInviteString;
  FTxtPos = -1;
}
//..............................................................................
void TGlConsole::KeepSize()  {
  int lc = FBuffer.Count();
  if( lc > FMaxLines )  {
    for( int i = 0; i < lc-FMaxLines; i++ )
      if( FBuffer.GetObject(i) )
        delete FBuffer.GetObject(i);
    FBuffer.DeleteRange(0, lc-FMaxLines);
  }
}
//..............................................................................
void TGlConsole::Visible(bool On)  {
  AGDrawObject::Visible(On);
  FCursor->Visible(On);
}
//..............................................................................
void TGlConsole::UpdateCursorPosition(bool InitCmds)  {
  if( !IsPromptVisible() || FontIndex == -1 || 
    FParent->GetWidth()*FParent->GetHeight() <= 50*50 )  return;
  TGlFont* Fnt = Font();
  if( InitCmds )  {
    Cmds.Clear();
    MaxLineWidth = Fnt->MaxTextLength(FParent->GetWidth());
    if( MaxLineWidth == 0 )  return;
    Cmds.Hypernate(FCommand, MaxLineWidth, true);
  }
  GlLeft = ((double)Left - (double)FParent->GetWidth()/2) + 0.1;
  GlTop = ((double)FParent->GetHeight()/2 - (Height+Top)) + 0.1;
  int th = Fnt->TextHeight(EmptyString);
  LineInc = (th*(1+FLineSpacing))*FParent->GetViewZoom();
  double Scale = FParent->GetScale();
  vec3d T;
  // update cursor position ...
  if( !Cmds.IsEmpty() )   {
    T[0] = GlLeft;
    int dxp = StringPosition(), i;
    for( i=0; i < Cmds.Count(); i++ )  {
      dxp -= Cmds[i].Length();
      if( dxp <= 0 )  break;
    }
    if( i >= Cmds.Count() )  i = Cmds.Count()-1;
    T[1] = GlTop + (Cmds.Count()-1-i)*LineInc;

    if( dxp < 0 )
      T[0] += Fnt->TextWidth( Cmds[i].SubStringTo( Cmds[i].Length() + dxp ) );
    else
      T[0] += Fnt->TextWidth(Cmds[i]);
    T[0] -= Fnt->MaxWidth()/2;  // move the cursor half a char left
    T *= Scale;
    FCursor->SetPosition(T[0], T[1]);
    FCursor->FontIndex( FontIndex );
  }
}
//..............................................................................
void TGlConsole::StringPosition(int v)  {
  FStringPos = v;
  UpdateCursorPosition(true);
}
//..............................................................................
TGlFont *TGlConsole::Font()  const   {  return FParent->Scene()->Font(FontIndex); }
//..............................................................................
bool TGlConsole::Execute(const IEObject *Sender, const IEObject *Data)  {

//  TEString cmd = GetCommand();
//  int pos = FStringPos - PromptStr.Length();
//  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
//  FCommand = PromptStr;
//  FCommand << cmd;
//  StringPosition( PromptStr.Length() + pos);
  
  UpdateCursorPosition(false);
  return true;
}
//..............................................................................
void TGlConsole::SetPromptVisible(bool v)  {
  PromptVisible = v;
  FCursor->Visible( v );
}
//..............................................................................
void TGlConsole::SetInviteString(const olxstr &S)  {
  InviteStr = S;
  InviteStr.Replace("\\(", '(');
  Primitives()->Style()->SetParam("Prompt", InviteStr, true);
  olxstr cmd = GetCommand();
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FCommand << cmd;
  StringPosition(  FCommand.Length() );
}
//..............................................................................
void TGlConsole::SetLinesToShow(int V)  {
  FLinesToShow = V;
  Primitives()->Style()->SetParam("LinesToShow", FLinesToShow, true);
}
//..............................................................................
void TGlConsole::SetLineSpacing(float v)  {
  FLineSpacing = olx_max(-0.9, v);
  Primitives()->Style()->SetParam("LineSpacing", FLineSpacing, true);
}
//..............................................................................
void TGlConsole::SetLineWidth(int V)  {
  if( V < 0 )
    V = Font()->MaxTextLength(FParent->GetWidth() - 5);
  FLineWidth = V;
  Primitives()->Style()->SetParam("LineWidth", FLineWidth, true);
}
//..............................................................................
size_t TGlConsole::Write(const void *Data, size_t size)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
size_t TGlConsole::Write(const olxstr& str)  {
  if( IsSkipPosting() )  {
    SetSkipPosting(false);
    return 1;
  }
  if( FBuffer.IsEmpty() )  
    FBuffer.Add(EmptyString);
  FBuffer.Last().String.SetCapacity( FBuffer.Last().String.Length() + str.Length());
  for( int i=0; i < str.Length(); i++ )  {
    if( str[i] == '\n' )  {
      FBuffer.Add(EmptyString);
    }
    else if( str[i] == '\r' )  {
      if( !FBuffer.IsEmpty() )
      FBuffer.Last().String = EmptyString;
    }
    else if( str[i] == '\t') {
      int count = 8-FBuffer.Last().String.Length()%8;
      if( count > 0 )
        FBuffer.Last().String.Insert(' ', FBuffer.Last().String.Length(), count);
    }
    else
      FBuffer.Last().String << str[i];
  }
  KeepSize();
  FTxtPos = FBuffer.Count()-1;
  return 1;
}
//..............................................................................
size_t TGlConsole::Writenl(const void *Data, size_t size)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
size_t TGlConsole::Writenl(const olxstr& str)  {
  if( IsSkipPosting() )  {
    SetSkipPosting(false);
    return 1;
  }
  if( !str.IsEmpty() )  Write(str);
  FBuffer.Add(EmptyString);
  FTxtPos = FBuffer.Count()-1;
  return 1;
}
//..............................................................................
IOutputStream& TGlConsole::operator << (IInputStream &is)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
size_t TGlConsole::GetSize() const  {
  return 1;
}
//..............................................................................
size_t TGlConsole::GetPosition() const  {
  return 0;
}
//..............................................................................
void TGlConsole::SetPosition(size_t newPos)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
bool TGlConsole::GetDimensions(vec3d &Max, vec3d &Min)  {
  Max = vec3d(0.5, 0.5, 0);
  Min = vec3d(-0.5, -0.5, 0);
  return true;
}
//..............................................................................
//..............................................................................
//..............................................................................


void TGlConsole::LibClear(const TStrObjList& Params, TMacroError& E)  {
  ClearBuffer();
}
//..............................................................................
void TGlConsole::LibLines(const TStrObjList& Params, TMacroError& E)  {
  if( Params.Count() )  SetLinesToShow(Params[0].ToInt());
  else
    E.SetRetVal<olxstr>(FLinesToShow);
}
//..............................................................................
void TGlConsole::LibShowBuffer(const TStrObjList& Params, TMacroError& E)  {
  if( Params.Count() )  ShowBuffer( Params[0].ToBool() );
  else
    E.SetRetVal<olxstr>( FShowBuffer );
}
//..............................................................................
void TGlConsole::LibPostText(const TStrObjList& Params, TMacroError& E)  {
  for( int i=0; i < Params.Count(); i++ )
    PrintText( Params[i] );
}
//..............................................................................
void TGlConsole::LibLineSpacing(const TStrObjList& Params, TMacroError& E)  {
  if( Params.Count() )  SetLineSpacing( Params[0].ToDouble() );
  else
    E.SetRetVal<olxstr>( FLineSpacing );
}
//..............................................................................
void TGlConsole::LibInviteString(const TStrObjList& Params, TMacroError& E)  {
  if( Params.Count() )  SetInviteString( Params[0] );
  else
    E.SetRetVal( InviteStr );
}
//..............................................................................
void TGlConsole::LibBlend(const TStrObjList& Params, TMacroError& E)  {
  if( Params.Count() )  SetBlend( Params[0].ToBool() );
  else
    E.SetRetVal( IsBlend() );
}
//..............................................................................
void TGlConsole::LibCommand(const TStrObjList& Params, TMacroError& E)  {
  if( Params.Count() )  {
    olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
    FCommand = PromptStr;
    FCommand << Params[0];
    StringPosition( FCommand.Length() );
  }
  else
    E.SetRetVal( FCommand );
}
//..............................................................................
TLibrary* TGlConsole::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary( (name.IsEmpty() ? olxstr("console") : name) );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibClear, "Clear",
    fpNone, "Clears the content of the output buffer") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibLines, "Lines",
    fpNone|fpOne, "Sets/returns the number of lines to display") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibShowBuffer, "ShowBuffer",
    fpNone|fpOne, "Shows/hides the output buffer or returns current status") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibPostText, "Post",
    fpAny^fpNone, "Adds provided text to the output buffer") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibLineSpacing, "LineSpacing",
    fpNone|fpOne, "Changes/returns current line spacing") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibInviteString, "PromptString",
    fpNone|fpOne, "Changes/returns current prompt string") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibCommand, "Command",
    fpNone|fpOne, "Changes/returns current command") );
  lib->RegisterFunction<TGlConsole>( new TFunction<TGlConsole>(this,  &TGlConsole::LibBlend, "Blend",
    fpNone|fpOne, "Changes/returns text blending status") );

  AGDrawObject::ExportLibrary( *lib );
  lib->AttachLibrary( FCursor->ExportLibrary() );
  return lib;
}

