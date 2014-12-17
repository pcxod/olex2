/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glrender.h"
#include "povdraw.h"
#include "glscene.h"

AGlScene::~AGlScene()  {
  Fonts.DeleteItems();
  SmallFonts.DeleteItems();
}
//.............................................................................
bool AGlScene::StartDraw() {
  if (!MakeCurrent()) {
    return false;
  }
  olx_gl::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  return true;
}
//.............................................................................
void AGlScene::EndDraw()  {  olx_gl::flush();  }
//.............................................................................
bool AGlScene::StartSelect(int x, int y, GLuint *Bf) {
  if (!MakeCurrent())
    return false;
  olx_gl::selectBuffer(MAXSELECT, Bf);
  olx_gl::renderMode(GL_SELECT);
  olx_gl::initNames();
  olx_gl::pushName(~0);
  FParent->SetView(x, y, false, true, 1);
  return true;
}
//.............................................................................
int AGlScene::EndSelect()  {
  int hits = olx_gl::renderMode(GL_RENDER);
  olx_gl::flush();
  FParent->SetView(false, 1);
  return hits;
}
//.............................................................................
TGlFont& AGlScene::CreateFont(const olxstr& name, const olxstr& fntDescription)
{
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
    SmallFonts.Add(
      new TGlFont(*this, SmallFonts.Count(), name))->SetIdString(
        fntDescription);
  }
  return *fnt;
}
//.............................................................................
void AGlScene::ToDataItem(TDataItem &di) const {
  FParent->LightModel.ToDataItem(di.AddItem("LightModel"));
  TDataItem &fonts = di.AddItem("Fonts");
  for( size_t i=0; i < FontCount(); i++ )
    fonts.AddItem(_GetFont(i).GetName(), _GetFont(i).GetIdString());
}
//.............................................................................
void AGlScene::FromDataItem(const TDataItem &di)  {
  FParent->LightModel.FromDataItem(di.GetItemByName("LightModel"));
  TDataItem &fonts = di.GetItemByName("Fonts");
  for (size_t i = 0; i < fonts.ItemCount(); i++) {
    CreateFont(fonts.GetItemByIndex(i).GetName(),
      fonts.GetItemByIndex(i).GetValue());
  }
}
//.............................................................................
const_strlist AGlScene::ToPov() const {
  TStrList out;
  out.Add("global_settings {");
  TGlOption cl_amb = FParent->LightModel.GetAmbientColor();
  cl_amb *= 10;
  out.Add(" ambient_light ") << pov::to_str(cl_amb);
  out.Add("}");

  TGlOption cl_clear = FParent->LightModel.GetClearColor();
  out.Add("background { color ") << pov::to_str(cl_clear) << " }";
  out.Add("camera {");
  out.Add(" location <0,0,") << 3./FParent->GetBasis().GetZoom() << '>';
  out.Add(" angle 25");
  out.Add(" up 1");
  out.Add(" right -4/3");
  out.Add(" look_at <0,0,0>");
  out.Add("}");
  for( size_t i=0; i < 8; i++ )  {
    const TGlLight &l = FParent->LightModel.GetLight(i);
    if( !l.IsEnabled() )  continue;
    out.Add("light_source {");
    TGlOption lp = l.GetPosition();
    out.Add(" ") << pov::to_str(lp, false);
    lp = l.GetDiffuse();
    if( lp.IsEmpty() )
      lp = l.GetAmbient();
    if( lp.IsEmpty() )
      lp = l.GetSpecular();
    lp *= 1.5;
    out.Add(" color ") << pov::to_str(lp);
    out.Add("}");
  }
  return out;
}
//.............................................................................
//.............................................................................
//.............................................................................
olxstr AGlScene::MetaFont::BuildOlexFontId(const olxstr& fileName, short size,
  bool fixed, bool bold, bool italic)
{
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
  suffix.RightPadding(4, '_');
  return prefix << suffix << size;
}
//.............................................................................
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
//.............................................................................
olxstr AGlScene::MetaFont::GetIdString() const {
  if( IsOlexFont(OriginalId) )
    return BuildOlexFontId(FileName, Size, Fixed, Bold, Italic);
  if( IsVectorFont(OriginalId) )
    return olxstr('@') << Size;
  return EmptyString();
}
//.............................................................................
olxstr AGlScene::MetaFont::GetFileIdString() const {
  if( IsOlexFont(OriginalId) )
    return BuildOlexFontId(EmptyString(), Size, Fixed, Bold, Italic);
  throw TInvalidArgumentException(__OlxSourceInfo, "Olex2 font is expected");
}
//.............................................................................
//.............................................................................
//.............................................................................
void AGlScene::LibMakeCurrent(TStrObjList& Cmds, const TParamList& Options,
  TMacroError& E)
{
  MakeCurrent();
}
//.............................................................................
TLibrary* AGlScene::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("scene") : name);
  lib->Register(
    new TMacro<AGlScene>(this,  &AGlScene::LibMakeCurrent, "MakeCurrent",
      EmptyString(), fpNone,
      "Make scene for rendering/updates")
  );
  return lib;
}
