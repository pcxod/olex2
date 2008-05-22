//---------------------------------------------------------------------------

#ifndef glconsoleH
#define glconsoleH
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
  int Width, Height, Top, Left; // to clip the content
  double GlLeft, GlTop, LineInc;
  TStrPObjList<olxstr,TGlMaterial*> FBuffer;
  TStrList FCommands;   // the content
  TStrList Cmds;        // hypernated commands
  olxstr FCommand;    // the command
  olxstr InviteStr, PromptStr;   //
  TActionQList *FActions; // actions list
  int FCmdPos,
      FTxtPos,
      FMaxLines,
      FLinesToShow,
      FStringPos,
      FLineWidth,
      MaxLineWidth;
  bool FShowBuffer, FScrollDirectionUp, Blend,
    SkipPosting;  // the next pot operation will pass
  short FontIndex;
  bool PromptVisible;
protected:
  void KeepSize();
  void StringPosition(int v);
  inline int  StringPosition()  const {  return FStringPos;  }
  void UpdateCursorPosition(bool InitCmds);
  class TGlCursor *FCursor;

  // redefine nl - we do not need to write new line 
  virtual size_t Write(const void *Data, size_t size);
  virtual size_t Writenl(const void *Data, size_t size);
  virtual size_t Write(const olxstr& str);
  virtual size_t Writenl(const olxstr& str);
  virtual IOutputStream& operator << (IInputStream &is);
  virtual size_t GetSize() const;
  virtual size_t GetPosition() const;
  virtual void SetPosition(size_t newPos);

  virtual bool Execute(const IEObject *Sender, const IEObject *Data=NULL);
public:
  TGlConsole(const olxstr& collectionName, TGlRender *Render);
  void Create(const olxstr& cName = EmptyString);
  virtual ~TGlConsole();

  olxstr GetCommand() const;
  void SetCommand(const olxstr &NewCmd);

  inline const TStrPObjList<olxstr,TGlMaterial*>& Buffer()  const  {  return FBuffer;  }
  void ClearBuffer();

  bool IsPromptVisible()  const  {  return PromptVisible;  }
  void SetPromptVisible(bool v);

  inline int GetCommandCount()  const  {  return FCommands.Count();  }
  inline const olxstr& GetCommandByIndex(int i)  {  return FCommands.String(i);  }
  inline int GetCommandIndex() const  {  return FCmdPos;  }
  inline void SetCommandIndex( int i) {  FCmdPos = i;  }

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(TVPointD &Max, TVPointD &Min){  return false;};
  bool ProcessKey( int Key, short ShiftState );
  bool WillProcessKey( int Key, short ShiftState );

  DefPropP(int, Width)
  DefPropP(int, Height)
  DefPropP(int, Top)
  DefPropP(int, Left)
  DefPropB(Blend)
  DefPropB(SkipPosting)
  inline bool ShowBuffer() const                  {  return FShowBuffer; }
  inline void ShowBuffer(bool v)                  {  FShowBuffer = v; }
  inline float GetLineSpacing()  const            {  return FLineSpacing; }
  void SetLineSpacing(float v);
  inline const olxstr& GetInviolxstr()  const { return InviteStr; }
  void SetInviteString(const olxstr &S);

  void PrintText(const olxstr &S, TGlMaterial *M=NULL, bool Hypernate=true);
  void PrintText(const TStrList &SL, TGlMaterial *M=NULL, bool Hypernate=true);
  inline void NewLine()           {  FBuffer.Add(EmptyString); }
  int  MaxLines();
  inline void SetMaxLines(int V)     {  FMaxLines = V; };
  inline int  GetLinesToShow() const {  return FLinesToShow;  }
  void SetLinesToShow(int V);

  inline int  GetLineWidth()   const {  return FLineWidth;  }
  void SetLineWidth(int V);

  class TGlFont* Font()  const;
  DefPropP(short, FontIndex)

  inline TGlCursor* Cursor()const {  return FCursor;  }
  void Visible(bool On);

  inline bool ScrollDirectionUp() const {  return FScrollDirectionUp; }
  inline void ScrollDirectionUp( bool v){  FScrollDirectionUp = v; }

  TActionQueue *OnCommand, *OnPost;

  void LibClear(const TStrObjList& Params, TMacroError& E);
  void LibLines(const TStrObjList& Params, TMacroError& E);
  void LibShowBuffer(const TStrObjList& Params, TMacroError& E);
  void LibPostText(const TStrObjList& Params, TMacroError& E);
  void LibLineSpacing(const TStrObjList& Params, TMacroError& E);
  void LibInviteString(const TStrObjList& Params, TMacroError& E);
  void LibCommand(const TStrObjList& Params, TMacroError& E);
  void LibBlend(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name=EmptyString);
};


EndGlNamespace()
#endif
