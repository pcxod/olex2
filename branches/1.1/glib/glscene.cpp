//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "glscene.h"
#include "glrender.h"

UseGlNamespace();
//..............................................................................
//..............................................................................
AGlScene::AGlScene() : FParent(NULL) {}
//..............................................................................
AGlScene::~AGlScene()  {
  Fonts.DeleteItems();
  SmallFonts.DeleteItems();
}
//..............................................................................
void AGlScene::StartDraw()  {  olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
//..............................................................................
void AGlScene::EndDraw()  {  olx_gl::flush();  }
//..............................................................................
void AGlScene::StartSelect(int x, int y, GLuint *Bf)  {
  olx_gl::selectBuffer(MAXSELECT, Bf);
  olx_gl::renderMode(GL_SELECT);
  olx_gl::initNames();
  olx_gl::pushName(~0);
  FParent->SetView(x, y, false, true, 1);
}
//..............................................................................
int AGlScene::EndSelect()  {
  int hits = olx_gl::renderMode(GL_RENDER);
  olx_gl::flush();
  FParent->SetView(false, 1);
  return hits;
}
//..............................................................................
TGlFont& AGlScene::CreateFont(const olxstr& name, const olxstr& fntDescription)  {
  const size_t i = FontsDict.IndexOf(name);
  if( i != InvalidIndex ) {
    // overwrite
    FontsDict.GetValue(i)->ClearData();
    FontsDict.GetValue(i)->SetIdString(fntDescription);
    if( olx_is_valid_index(FontsDict.GetValue(i)->SmallId) )  {
      SmallFonts[FontsDict.GetValue(i)->SmallId]->ClearData();
      SmallFonts[FontsDict.GetValue(i)->SmallId]->SetIdString(fntDescription);
    }
    return *FontsDict.GetValue(i);
  }
  TGlFont* fnt = Fonts.Add(new TGlFont(*this, Fonts.Count(), name));
  fnt->SetIdString(fntDescription);
  FontsDict.Add(name, fnt);
  if( !fnt->IsVectorFont() )  {
    fnt->SmallId = SmallFonts.Count();
    SmallFonts.Add(new TGlFont(*this, SmallFonts.Count(), name))->SetIdString(fntDescription);
  }
  return *fnt;
}
//..............................................................................
//..............................................................................
//..............................................................................
olxstr AGlScene::MetaFont::BuildOlexFontId(const olxstr& fileName, short size, bool fixed, bool bold, bool italic)  {
  olxstr prefix, suffix;
  if( !fileName.IsEmpty() )
    prefix << '#' << fileName << ':';
  suffix << (fixed ? "f" : "n" );
  if( italic )  {
    suffix << "i";
    if( bold )
      suffix << "b";
  }
  else if( bold )
    suffix << "rb";
  else
    suffix << "r";
  suffix.Format(4, true, '_');
  return prefix << suffix << size;
}
//..............................................................................
bool AGlScene::MetaFont::SetIdString(const olxstr& idstr)  {
  OriginalId = idstr;
  if( IsOlexFont(idstr) )  {
    size_t ci = idstr.IndexOf(':');
    if( ci == InvalidIndex )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid font ID");
    FileName = idstr.SubStringFrom(1).SubStringTo(ci-1);
    olxstr fntid( idstr.SubStringFrom(ci+1) );
    if( fntid.Length() < 5 )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid font ID");
    Fixed = (fntid.CharAt(0) == 'f');
    Italic = (fntid.CharAt(1) == 'i');
    Bold = (fntid.CharAt(2) == 'b');
    Size = fntid.SubStringFrom(4).ToInt();
  }
  else if( IsVectorFont(idstr) )  {
    Fixed = false;
    Italic = false;
    Bold = false;
    Size = 15;
    if( idstr.SubStringFrom(1).IsNumber() )
      Size = idstr.SubStringFrom(1).ToInt();
  }
  else
    return false;
  return true;
}
//..............................................................................
olxstr AGlScene::MetaFont::GetIdString() const {
  if( IsOlexFont(OriginalId) )
    return BuildOlexFontId(FileName, Size, Fixed, Bold, Italic);
  if( IsVectorFont(OriginalId) )
    return olxstr('@') << Size;
  return EmptyString();
}
//..............................................................................
olxstr AGlScene::MetaFont::GetFileIdString() const {
  if( IsOlexFont(OriginalId) ) 
    return BuildOlexFontId(EmptyString(), Size, Fixed, Bold, Italic);
  throw TInvalidArgumentException(__OlxSourceInfo, "Olex2 font is expected");
}

