/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_console_H
#define __olx_gl_console_H
#include "glbase.h"
#include "gdrawobject.h"
#include "glmaterial.h"
#include "actions.h"
#include "macroerror.h"
#include "datastream.h"
BeginGlNamespace()

class TGlConsole: public AGDrawObject, 
                  public AActionHandler, 
                  public IDataOutputStream  {
  double FLineSpacing;
  uint16_t Width, Height, Top, Left; // to clip the content
  double GlLeft, GlTop;
  TStrPObjList<olxstr,TGlMaterial*> FBuffer;
  TStrList FCommands;   // the content
  olxstr FCommand;    // the command
  olxstr InviteStr, PromptStr;   //
  TActionQList Actions; // actions list
  size_t FCmdPos,
         FTxtPos,
         FMaxLines,
         FLinesToShow,
         FStringPos,
         LinesVisible;
  bool FShowBuffer, ScrollDirectionUp,
    SkipPosting;  // the next pot operation will pass
  size_t FontIndex;
  bool PromptVisible;
  TGlMaterial* PrintMaterial;
protected:
  void KeepSize();
  void SetInsertPosition(size_t v);
  size_t GetInsertPosition() const {  return FStringPos;  }
  void UpdateCursorPosition(bool InitCmds);
  size_t CalcScrollDown() const;
  class TGlCursor *FCursor;

  virtual size_t Write(const void *Data, size_t size);
  virtual size_t Write(const olxstr& str)  {
    return Write((const TTIString<olxch>&)str);
  }
  virtual size_t Write(const TTIString<olxch>& str);
  virtual IOutputStream& operator << (IInputStream& is);
  virtual uint64_t GetSize() const {  return 1;  }
  virtual uint64_t GetPosition() const { return 0;  }
  virtual void SetPosition(uint64_t newPos)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual bool Enter(const IEObject *Sender, const IEObject *Data=NULL);
  void OnResize();
public:
  TGlConsole(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName=EmptyString());
  virtual ~TGlConsole();

  olxstr GetCommand() const;
  void SetCommand(const olxstr& NewCmd);
  size_t GetCmdInsertPosition() const {  
    return (FCommand.StartsFrom(PromptStr) ?
      (FStringPos - PromptStr.Length()) : FStringPos);
  }

  const TStrPObjList<olxstr,TGlMaterial*>& Buffer() const {
    return FBuffer;
  }
  void ClearBuffer();

  bool IsPromptVisible() const {  return PromptVisible;  }
  void SetPromptVisible(bool v);
  // returns this objects Z coordinate
  double GetZ() const;
  size_t GetCommandCount() const {  return FCommands.Count();  }
  const olxstr& GetCommandByIndex(size_t i) const {  return FCommands[i];  }
  const olxstr& GetLastCommand(const olxstr &name) const;

  size_t GetCommandIndex() const {  return FCmdPos;  }
  void SetCommandIndex(size_t i) {  FCmdPos = i;  }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min);
  virtual void SetVisible(bool v);
  bool ProcessKey(int Key, short ShiftState);
  bool WillProcessKey(int Key, short ShiftState);

  uint16_t GetLeft() const { return Left; }
  uint16_t GetTop() const { return Top; }
  uint16_t GetWidth() const { return Width; }
  uint16_t GetHeight() const { return Height; }

  void Resize(int l, int t, int w, int h);
  DefPropBIsSet(SkipPosting)
  DefPropP(TGlMaterial*, PrintMaterial)
  bool ShowBuffer() const {  return FShowBuffer; }
  void ShowBuffer(bool v)  {  FShowBuffer = v; }
  double GetLineSpacing() const {  return FLineSpacing; }
  void SetLineSpacing(double v);
  const olxstr& GetInviteString() const { return InviteStr; }
  void SetInviteString(const olxstr& S);

  void PrintText(const olxstr& S, TGlMaterial *M=NULL, bool Hyphenate=true);
  void PrintText(const TStrList& SL, TGlMaterial *M=NULL, bool Hyphenate=true);
  void NewLine()  {  FBuffer.Add(EmptyString()); }
  size_t MaxLines();
  void SetMaxLines(size_t V)  {  FMaxLines = V; }
  size_t GetLinesToShow() const {  return FLinesToShow;  }
  void SetLinesToShow(size_t V);

  class TGlFont& GetFont() const;
  TGlCursor& Cursor() const {  return *FCursor;  }

  DefPropBIsSet(ScrollDirectionUp)

  TActionQueue &OnCommand, &OnPost;

  void LibClear(const TStrObjList& Params, TMacroError& E);
  void LibLines(const TStrObjList& Params, TMacroError& E);
  void LibShowBuffer(const TStrObjList& Params, TMacroError& E);
  void LibPostText(const TStrObjList& Params, TMacroError& E);
  void LibLineSpacing(const TStrObjList& Params, TMacroError& E);
  void LibInviteString(const TStrObjList& Params, TMacroError& E);
  void LibCommand(const TStrObjList& Params, TMacroError& E);
  class TLibrary* ExportLibrary(const olxstr& name=EmptyString());
};

EndGlNamespace()
#endif
