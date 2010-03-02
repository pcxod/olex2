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
  float FLineSpacing;
  uint16_t Width, Height, Top, Left; // to clip the content
  double GlLeft, GlTop, LineInc;
  TStrPObjList<olxstr,TGlMaterial*> FBuffer;
  TStrList FCommands;   // the content
  TStrList Cmds;        // hyphenated commands
  olxstr FCommand;    // the command
  olxstr InviteStr, PromptStr;   //
  TActionQList Actions; // actions list
  size_t FCmdPos,
         FTxtPos,
         FMaxLines,
         FLinesToShow,
         FStringPos;
  bool FShowBuffer, FScrollDirectionUp, Blend,
    SkipPosting;  // the next pot operation will pass
  uint16_t FontIndex;
  bool PromptVisible;
protected:
  void KeepSize();
  void SetInsertPosition(size_t v);
  size_t GetInsertPosition() const {  return FStringPos;  }
  void UpdateCursorPosition(bool InitCmds);
  class TGlCursor *FCursor;

  // redefine nl - we do not need to write new line 
  virtual size_t Write(const void *Data, size_t size);
  virtual size_t Writenl(const void *Data, size_t size);
  virtual size_t Write(const olxstr& str);
  virtual size_t Writenl(const olxstr& str);
  virtual IOutputStream& operator << (IInputStream& is);
  virtual uint64_t GetSize() const {  return 1;  }
  virtual uint64_t GetPosition() const { return 0;  }
  virtual void SetPosition(uint64_t newPos)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual bool Execute(const IEObject *Sender, const IEObject *Data=NULL);
public:
  TGlConsole(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TGlConsole();

  olxstr GetCommand() const;
  void SetCommand(const olxstr& NewCmd);
  size_t GetCmdInsertPosition() const {  
    return (FCommand.StartsFrom(PromptStr) ? (FStringPos - PromptStr.Length()) : FStringPos);
  }

  inline const TStrPObjList<olxstr,TGlMaterial*>& Buffer()  const  {  return FBuffer;  }
  void ClearBuffer();

  bool IsPromptVisible()  const  {  return PromptVisible;  }
  void SetPromptVisible(bool v);

  inline size_t GetCommandCount() const {  return FCommands.Count();  }
  inline const olxstr& GetCommandByIndex(size_t i) const {  return FCommands[i];  }
  inline size_t GetCommandIndex() const  {  return FCmdPos;  }
  inline void SetCommandIndex(size_t i) {  FCmdPos = i;  }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min);
  bool ProcessKey( int Key, short ShiftState );
  bool WillProcessKey( int Key, short ShiftState );

  DefPropP(uint16_t, Width)
  DefPropP(uint16_t, Height)
  DefPropP(uint16_t, Top)
  DefPropP(uint16_t, Left)
  DefPropBIsSet(Blend)
  DefPropBIsSet(SkipPosting)
  inline bool ShowBuffer() const {  return FShowBuffer; }
  inline void ShowBuffer(bool v)  {  FShowBuffer = v; }
  inline float GetLineSpacing() const {  return FLineSpacing; }
  void SetLineSpacing(float v);
  inline const olxstr& GetInviteString() const { return InviteStr; }
  void SetInviteString(const olxstr& S);

  void PrintText(const olxstr& S, TGlMaterial *M=NULL, bool Hyphenate=true);
  void PrintText(const TStrList& SL, TGlMaterial *M=NULL, bool Hyphenate=true);
  inline void NewLine()  {  FBuffer.Add(EmptyString); }
  size_t MaxLines();
  inline void SetMaxLines(size_t V)  {  FMaxLines = V; };
  inline size_t GetLinesToShow() const {  return FLinesToShow;  }
  void SetLinesToShow(size_t V);

  class TGlFont& GetFont()  const;
  DefPropP(uint16_t, FontIndex)

  inline TGlCursor& Cursor() const {  return *FCursor;  }
  void Visible(bool On);

  inline bool ScrollDirectionUp() const {  return FScrollDirectionUp; }
  inline void ScrollDirectionUp(bool v) {  FScrollDirectionUp = v; }

  TActionQueue &OnCommand, &OnPost;

  void LibClear(const TStrObjList& Params, TMacroError& E);
  void LibLines(const TStrObjList& Params, TMacroError& E);
  void LibShowBuffer(const TStrObjList& Params, TMacroError& E);
  void LibPostText(const TStrObjList& Params, TMacroError& E);
  void LibLineSpacing(const TStrObjList& Params, TMacroError& E);
  void LibInviteString(const TStrObjList& Params, TMacroError& E);
  void LibCommand(const TStrObjList& Params, TMacroError& E);
  void LibBlend(const TStrObjList& Params, TMacroError& E);
  class TLibrary* ExportLibrary(const olxstr& name=EmptyString);
};


EndGlNamespace()
#endif
