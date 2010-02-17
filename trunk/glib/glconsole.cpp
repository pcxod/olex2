//----------------------------------------------------------------------------//
// TGlConsole - a console
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#include "glconsole.h"
#include "glrender.h"
#include "actions.h"
#include "glcursor.h"
#include "gpcollection.h"
#include "glfont.h"
#include "glmouse.h"
#include "glscene.h"
#include "glprimitive.h"

#include "styles.h"

#include "integration.h"
#include "bapp.h"

// keyboard constanst, silly to include here, but...
#include "wx/defs.h"

/* There is a slight problem with cursor - depending on object properties it might
  be drawn before the console, and then its position is validated after it is drawn
  in previous position!  ...
*/

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlConsole::TGlConsole(TGlRenderer& R, const olxstr& collectionName) :
  AGDrawObject(R, collectionName),
  OnCommand(Actions.New("ONCOMMAND")),
  OnPost(Actions.New("ONPOST"))
{
  FLineSpacing = 0;
  Left = Top = 0;
  Width = Height = 100;
  PromptVisible = true;
  SkipPosting = false;
  Blend = true;
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FShowBuffer = true;
  SetGroupable(false);
  FontIndex = ~0;
  FTxtPos = ~0;
  FMaxLines = 1000;
  FScrollDirectionUp = true;
  FLinesToShow = ~0;
  FCmdPos = ~0;
  FCursor = new TGlCursor(R, "Cursor");
  //FCursor->Visible(false);

  SetToDelete(false);
  R.BeforeDraw->Add(this);
}
//..............................................................................
TGlConsole::~TGlConsole()  {
  Parent.BeforeDraw->Remove(this);
  ClearBuffer();
  delete FCursor;
}
//..............................................................................
void TGlConsole::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();

  FLinesToShow = GS.GetParam("LinesToShow", FLinesToShow, true).ToInt();
  FLineSpacing = GS.GetParam("LineSpacing", "0", true).ToDouble();
  InviteStr = GS.GetParam("Prompt", ">>", true);

  TGlPrimitive& GlP = GPC.NewPrimitive("Text", sgloText);
  GlP.SetProperties(GS.GetMaterial("Text", GetFont().GetMaterial()));
  GlP.Params[0] = -1;  //bitmap; TTF by default
  FCursor->Create();
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FStringPos = FCommand.Length();
  Cmds.Clear();
  Cmds.Add(FCommand);
}
//..............................................................................
bool TGlConsole::Orient(TGlPrimitive& P)  {
  TGlFont& Fnt = GetFont();
  //Fnt->DrawGlText( vec3d(0,0,0), "HELLW_O", true);
  P.SetFont(&Fnt);
  if( Parent.GetWidth() < 100 )  return true;
  const uint16_t th = Fnt.TextHeight(EmptyString);
  const double Scale = Parent.GetScale(),
               MaxY = ((double)Parent.GetHeight()/2-Top-th)*Scale;
  const double MaxZ = -Parent.GetMaxRasterZ() + 0.02;
  const size_t MaxLineWidth = Fnt.MaxTextLength(Parent.GetWidth());

  olxstr line;
  vec3d T;
  const TGlMaterial& OGlM = P.GetProperties();
  TGlOption CC = Parent.LightModel.GetClearColor();
  TGlMaterial GlM = OGlM;  // copy properties
//  GlM.AmbientF[0] = 1-CC[0];  GlM.AmbientF[1] = 1-CC[1];  GlM.AmbientF[2] = 1-CC[2];
  TGlOption Ambient = GlM.AmbientF;
  if( ShowBuffer() && FLinesToShow != 0 )  {
    float Rk=1, Gk=1, Bk=1;
    index_t lc = (index_t)(((float)Height-0.1)/(th*(FLineSpacing+1))); // calc the number of lines
    if( PromptVisible )  lc -= Cmds.Count();
    if( lc != 0 )  {
      Rk = (CC[0] - Ambient[0])/lc;
      Gk = (CC[1] - Ambient[1])/lc;
      Bk = (CC[2] - Ambient[2])/lc;
    }
    for( index_t i=FTxtPos; i >= olx_max(0, (index_t)(FTxtPos)-lc); i-- )  {
      if( FBuffer[i].IsEmpty() )  continue;
      size_t ii = (FTxtPos-i);
      if( olx_is_valid_size(FLinesToShow) && ii > FLinesToShow )  continue;
      if( IsPromptVisible() )  ii += Cmds.Count();

      T[0] = GlLeft;  T[1] = GlTop + ii*LineInc;
      T *= Scale;
      if( T[1] > MaxY )  break;
      if( FBuffer[i].Length() > MaxLineWidth )  
        line = FBuffer[i].SubStringTo(MaxLineWidth);
      else
        line = FBuffer[i];
      // drawing spaces is not required ...
      size_t stlen = line.Length();
      if( stlen == 0 )  continue;
      while( line.CharAt(stlen-1) == ' ' && stlen > 1 ) stlen--;
      line = line.SubStringTo(stlen);

      glRasterPos3d(T[0], T[1], MaxZ);
      TGlMaterial* GlMP = FBuffer.GetObject(i);
      if( GlMP != NULL ) 
        GlMP->Init(Parent.IsColorStereo());
      else if( IsBlend() )  {
        GlM.AmbientF[0] = Ambient[0] + (float)ii*Rk;
        GlM.AmbientF[1] = Ambient[1] + (float)ii*Gk;
        GlM.AmbientF[2] = Ambient[2] + (float)ii*Bk;
        GlM.Init(Parent.IsColorStereo()); // fading the text
      }
      else
        OGlM.Init(Parent.IsColorStereo());
      P.SetString(&line);
      Parent.DrawText(P, T[0], T[1], MaxZ); 
      P.SetString(NULL);
      if( i== 0 )  break;
    }
  }
  if( PromptVisible )  {
    GlM.AmbientF =Ambient;
    GlM.Init(Parent.IsColorStereo());
    if( Cmds.Count() == 1 )  {
      T[0] = GlLeft;  T[1] = GlTop;
      T *= Scale;
      P.SetString(&FCommand);
      Parent.DrawText(P, T[0], T[1], MaxZ); 
      P.SetString(NULL);
    }
    else  {
      for( size_t i=Cmds.Count()-1; olx_is_valid_index(i); i-- )  {
        const size_t ii = Cmds.Count()-i-1;
        if( olx_is_valid_size(FLinesToShow) && ii > FLinesToShow )  continue;
        T[0] = GlLeft;  T[1] = GlTop + ii*LineInc;
        T *= Scale;
        P.SetString(&Cmds[i]);
        Parent.DrawText(P, T[0], T[1], MaxZ); 
        P.SetString(NULL);
      }
    }
  }
  OGlM.Init(Parent.IsColorStereo()); // restore the material properties
  FCursor->Draw();
  return true;
}
//..............................................................................
bool TGlConsole::WillProcessKey( int Key, short ShiftState )  {
  if( Key == WXK_DELETE )  {
    if( ShiftState == 0 && GetInsertPosition() < FCommand.Length() &&
         GetInsertPosition() >= PromptStr.Length() )  {
      return true;
    }
  }
  else if( Key == WXK_BACK && GetInsertPosition() != PromptStr.Length() )
    return true;
  return false;
}
//..............................................................................
bool TGlConsole::ProcessKey( int Key , short ShiftState)  {
  if( (Key == WXK_UP) && IsPromptVisible() && ShiftState == 0 )  {
    FCmdPos --;
    if( !olx_is_valid_size(FCmdPos) )  FCmdPos = FCommands.Count()-1;
    if( olx_is_valid_size(FCmdPos) && FCmdPos < FCommands.Count() )  {
      olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
      FCommand = PromptStr;
      FCommand << FCommands[FCmdPos];
      SetInsertPosition(FCommand.Length());
    }
    return true;
  }
  if( (Key == WXK_DOWN) && IsPromptVisible() && ShiftState == 0 )  {
    FCmdPos ++;
    if( FCmdPos >= FCommands.Count() )  FCmdPos = 0;
    if( olx_is_valid_size(FCmdPos) && FCmdPos < FCommands.Count() )  {
      olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
      FCommand = PromptStr;
      FCommand << FCommands[FCmdPos];
      SetInsertPosition(FCommand.Length());
    }
    return true;
  }
  if( (Key == WXK_LEFT) && IsPromptVisible() && (GetInsertPosition() > PromptStr.Length()) )  {
    if( ShiftState == 0 )  {
      SetInsertPosition(GetInsertPosition()-1);
      return true;
    }
    else if( ShiftState == sssCtrl  )  {
      size_t ind = FCommand.LastIndexOf(' ', GetInsertPosition()-1);
      if( ind != InvalidIndex )
        SetInsertPosition( ind );
      else
        SetInsertPosition(PromptStr.Length());
      return true;
    }
    return false;
  }
  if( (Key == WXK_RIGHT) && IsPromptVisible() && (GetInsertPosition() < FCommand.Length()) )  {
    if( ShiftState == 0 )  {
      SetInsertPosition(GetInsertPosition()+1);
      return true;
    }
    else if( ShiftState == sssCtrl )  {
      size_t ind = FCommand.FirstIndexOf(' ', GetInsertPosition()+1);
      if( ind != InvalidIndex )
        SetInsertPosition( ind );
      else
        SetInsertPosition(FCommand.Length());
      return true;
    }
    return false;
  }
  if( Key == WXK_DELETE )  {
    if( ShiftState == 0 && GetInsertPosition() < FCommand.Length() &&
         GetInsertPosition() >= PromptStr.Length() )  {
      FCommand.Delete(GetInsertPosition(), 1);
      UpdateCursorPosition(true);
      return true;
    }
    return false;
  }
  if( (Key == WXK_HOME) && !ShiftState  && IsPromptVisible() )  {
    SetInsertPosition(PromptStr.Length());
    return true;
  }
  if( (Key == WXK_END) && !ShiftState  && IsPromptVisible() )  {
    SetInsertPosition(FCommand.Length());
    return true;
  }
  if( Key == WXK_PAGEUP || Key == WXK_PAGEDOWN )  {
    TGlFont& Fnt = GetFont();
    uint16_t th = Fnt.TextHeight(EmptyString);
    size_t lc;
    if( !olx_is_valid_size(FLinesToShow) )
      lc = (size_t)((float)Height/(th*(1.0+FLineSpacing))*Parent.GetViewZoom()); // calc the number of lines
    else
      lc = FLinesToShow;
    if( Key == WXK_PAGEDOWN )  {
      FTxtPos += lc;
      if( FTxtPos >= FBuffer.Count() )
        FTxtPos = FBuffer.Count()-1;
    }
    if( Key == WXK_PAGEUP )  {
      if( FTxtPos < lc )
        FTxtPos = FBuffer.IsEmpty() ? ~0 : 0;
      else
        FTxtPos -= lc;
    }
    SetInsertPosition(FCommand.Length());
    return true;
  }
  if( !Key || Key > 255 || (ShiftState & sssCtrl) || (ShiftState & sssAlt))  return false;
  if( !IsPromptVisible() )  return false;
  
  if( Key == WXK_ESCAPE )  {
    olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
    FCommand = PromptStr;
    SetInsertPosition(FCommand.Length());
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
    OnCommand.Execute(dynamic_cast<IEObject*>((AActionHandler*)this) );
    if( FCommand.IsEmpty() )  {
      olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
      FCommand = PromptStr;
    }
    SetInsertPosition(FCommand.Length());
    return true;
  }
  if( Key == WXK_BACK )  {
    if( FCommand.Length() > PromptStr.Length() )  {
      if( GetInsertPosition() == FCommand.Length() )  {
        FCommand.SetLength(FCommand.Length()-1);
        SetInsertPosition(FCommand.Length());
      }
      else  {  // works like delete
        if( GetInsertPosition() > PromptStr.Length() )  {
          FCommand.Delete(GetInsertPosition()-1, 1);
          SetInsertPosition(GetInsertPosition()-1);
        }
      }
    }
    return true;
  }
  if( GetInsertPosition() == FCommand.Length() )
    FCommand << (olxch)Key;
  else
    FCommand.Insert((olxch)Key, GetInsertPosition());
  SetInsertPosition(GetInsertPosition()+1);
  return true;
}
//..............................................................................
void TGlConsole::PrintText(const olxstr &S, TGlMaterial *M, bool Hyphenate)  {
  if( IsSkipPosting() )  {
    //SetSkipPosting(false);
    return;
  }
  olxstr Tmp(S);
  bool SingleLine = false;
  for( size_t i=0; i < Tmp.Length(); i++ )  {
    if( Tmp[i] == '\t' )  {
      Tmp[i] = ' ';
      int count = 8-i%8-1;
      if( count > 0 ) Tmp.Insert(' ', i, count);
    }
  }
  if( Hyphenate )  {
    const size_t sz = GetFont().MaxTextLength(Parent.GetWidth());
    if( sz <= 0 )  return;
    TStrList Txt;
    Txt.Hyphenate(S, sz, true);
    if( Txt.Count() > 1 )  PrintText(Txt, M);
    else                   SingleLine = true;
  }
  if( !Hyphenate || SingleLine )  {
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
    OnPost.Execute(dynamic_cast<IEObject*>((AActionHandler*)this), &Tmp);
  }

  KeepSize();
  FTxtPos = FBuffer.Count()-1;
  //FBuffer.Add(EmptyString);
}
//..............................................................................
void TGlConsole::PrintText(const TStrList &SL, TGlMaterial *M, bool Hyphenate)  {
  if( IsSkipPosting() )  {
    //SetSkipPosting(false);
    return;
  }
  const size_t sz = GetFont().MaxTextLength(Parent.GetWidth());
  if( sz <= 0 )  return;
  TStrList Txt;
  olxstr Tmp;
  for( size_t i=0; i < SL.Count(); i++ )  {
    Tmp = SL[i];
    for( size_t j=0; j < Tmp.Length(); j++ )  {
      if( Tmp.CharAt(j) == '\t' )  {
        Tmp[j] = ' ';
        int count = 8-j%8-1;
        if( count > 0 ) Tmp.Insert(' ', j, count);
      }
    }
    if( Hyphenate )  {
      Txt.Clear();
      Txt.Hyphenate(Tmp, sz, true);
      for( size_t j=0; j < Txt.Count(); j++ )  {
        TGlMaterial *GlM = NULL;
        if( M != NULL )  {  GlM = new TGlMaterial;  *GlM = *M;  }
        if( j == 0 && !FBuffer.IsEmpty() && FBuffer.LastStr().IsEmpty() )  {
          FBuffer.Last().String = Txt[j];
          FBuffer.Last().Object = GlM;
        }
        else
          FBuffer.Add(Txt[j], GlM);
        OnPost.Execute(dynamic_cast<IEObject*>((AActionHandler*)this), &Txt[j] );
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
      OnPost.Execute(dynamic_cast<IEObject*>((AActionHandler*)this), &Tmp);
    }
  }
  KeepSize();
  FTxtPos = FBuffer.Count()-1;
  FBuffer.Add(EmptyString);
}
//..............................................................................
olxstr TGlConsole::GetCommand() const  {
  return (FCommand.StartsFrom(PromptStr) ? FCommand.SubStringFrom(PromptStr.Length()) : FCommand);
}
//..............................................................................
void TGlConsole::SetCommand(const olxstr& NewCmd)  {
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FCommand << NewCmd;
  SetInsertPosition( FCommand.Length() );
}
//..............................................................................
void TGlConsole::ClearBuffer()  {
  size_t lc = FBuffer.Count();
  for( size_t i=0; i < lc; i++ )
    if( FBuffer.GetObject(i) != NULL )
      delete (TGlMaterial*)FBuffer.GetObject(i);
  FBuffer.Clear();
  //FCommand = FInviteString;
  FTxtPos = ~0;
}
//..............................................................................
void TGlConsole::KeepSize()  {
  size_t lc = FBuffer.Count();
  if( lc > FMaxLines )  {
    for( size_t i = 0; i < lc-FMaxLines; i++ )
      if( FBuffer.GetObject(i) )
        delete FBuffer.GetObject(i);
    FBuffer.DeleteRange(0, lc-FMaxLines);
  }
}
//..............................................................................
void TGlConsole::Visible(bool On)  {
  AGDrawObject::SetVisible(On);
  FCursor->SetVisible(On);
}
//..............................................................................
void TGlConsole::UpdateCursorPosition(bool InitCmds)  {
  if( !IsPromptVisible() || !olx_is_valid_index(FontIndex) || 
    Parent.GetWidth()*Parent.GetHeight() <= 50*50 )  return;
  TGlFont& Fnt = GetFont();
  if( InitCmds )  {
    Cmds.Clear();
    size_t MaxLineWidth = Fnt.MaxTextLength(Parent.GetWidth());
    if( MaxLineWidth == 0 )  return;
    Cmds.Hyphenate(FCommand, MaxLineWidth, true);
  }
  GlLeft = ((double)Left - (double)Parent.GetWidth()/2) + 0.1;
  GlTop = ((double)Parent.GetHeight()/2 - (Height+Top)) + 0.1;
  int th = Fnt.TextHeight(EmptyString);
  LineInc = (th*(1+FLineSpacing))*Parent.GetViewZoom();
  double Scale = Parent.GetScale();
  vec3d T;
  // update cursor position ...
  if( !Cmds.IsEmpty() )   {
    T[0] = GlLeft;
    index_t dxp = GetInsertPosition();
    size_t i;
    for( i=0; i < Cmds.Count(); i++ )  {
      dxp -= Cmds[i].Length();
      if( dxp <= 0 )  break;
    }
    if( i >= Cmds.Count() )  i = Cmds.Count()-1;
    T[1] = GlTop + (Cmds.Count()-1-i)*LineInc;

    if( dxp < 0 )
      T[0] += Fnt.TextWidth( Cmds[i].SubStringTo(Cmds[i].Length() + dxp) );
    else
      T[0] += Fnt.TextWidth(Cmds[i]);
    T[0] -= Fnt.GetMaxWidth()/2;  // move the cursor half a char left
    T *= Scale;
    FCursor->SetPosition(T[0], T[1]);
    FCursor->SetFontIndex(FontIndex);
  }
}
//..............................................................................
void TGlConsole::SetInsertPosition(size_t v)  {
  FStringPos = v;
  UpdateCursorPosition(true);
}
//..............................................................................
TGlFont &TGlConsole::GetFont() const {  
  TGlFont* fnt = Parent.GetScene().GetFont(FontIndex);
  if( fnt == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, "font index");
  return *fnt;
}
//..............................................................................
bool TGlConsole::Execute(const IEObject *Sender, const IEObject *Data)  {

//  TEString cmd = GetCommand();
//  int pos = FStringPos - PromptStr.Length();
//  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
//  FCommand = PromptStr;
//  FCommand << cmd;
//  SetInsertPosition( PromptStr.Length() + pos);
  
  UpdateCursorPosition(false);
  return true;
}
//..............................................................................
void TGlConsole::SetPromptVisible(bool v)  {
  PromptVisible = v;
  FCursor->SetVisible( v );
}
//..............................................................................
void TGlConsole::SetInviteString(const olxstr &S)  {
  InviteStr = S;
  GetPrimitives().GetStyle().SetParam("Prompt", InviteStr.Replace("\\(", '('), true);
  olxstr cmd = GetCommand();
  olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
  FCommand = PromptStr;
  FCommand << cmd;
  SetInsertPosition(  FCommand.Length() );
}
//..............................................................................
void TGlConsole::SetLinesToShow(size_t V)  {
  FLinesToShow = V;
  GetPrimitives().GetStyle().SetParam("LinesToShow", FLinesToShow, true);
}
//..............................................................................
void TGlConsole::SetLineSpacing(float v)  {
  FLineSpacing = olx_max(-0.9, v);
  GetPrimitives().GetStyle().SetParam("LineSpacing", FLineSpacing, true);
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
  for( size_t i=0; i < str.Length(); i++ )  {
    if( str.CharAt(i) == '\n' )
      FBuffer.Add(EmptyString);
    else if( str.CharAt(i) == '\r' )  {
      if( !FBuffer.IsEmpty() )
      FBuffer.Last().String = EmptyString;
    }
    else if( str.CharAt(i) == '\t') {
      int count = 8-FBuffer.Last().String.Length()%8;
      if( count > 0 )
        FBuffer.Last().String.Insert(' ', FBuffer.Last().String.Length(), count);
    }
    else
      FBuffer.Last().String << str.CharAt(i);
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
  if( !Params.IsEmpty() )
    SetLinesToShow(Params[0].ToInt());
  else
    E.SetRetVal<olxstr>(FLinesToShow);
}
//..............................................................................
void TGlConsole::LibShowBuffer(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )
    ShowBuffer( Params[0].ToBool() );
  else
    E.SetRetVal<olxstr>(FShowBuffer);
}
//..............................................................................
void TGlConsole::LibPostText(const TStrObjList& Params, TMacroError& E)  {
  for( size_t i=0; i < Params.Count(); i++ )
    PrintText(Params[i]);
}
//..............................................................................
void TGlConsole::LibLineSpacing(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )
    SetLineSpacing(Params[0].ToDouble());
  else
    E.SetRetVal<olxstr>(FLineSpacing);
}
//..............................................................................
void TGlConsole::LibInviteString(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )
    SetInviteString(Params[0]);
  else
    E.SetRetVal(InviteStr);
}
//..............................................................................
void TGlConsole::LibBlend(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )
    SetBlend(Params[0].ToBool());
  else
    E.SetRetVal(IsBlend());
}
//..............................................................................
void TGlConsole::LibCommand(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )  {
    olex::IOlexProcessor::GetInstance()->executeFunction(InviteStr, PromptStr);
    FCommand = PromptStr;
    FCommand << Params[0];
    SetInsertPosition(FCommand.Length());
  }
  else
    E.SetRetVal(FCommand);
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

