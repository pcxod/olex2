#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "bapp.h"

// for InterlockedIncrement in
#if defined __WIN32__ && __BORLANDC__
  #include <winbase.h>
#endif

#include "wxglscene.h"
#include "exception.h"
#include "glfont.h"
#include "glrender.h"
#include "efile.h"

//#include "ft2build.h"  
//#include FT_FREETYPE_H
//#include FT_GLYPH_H
//---------------------------------------------------------------------------
// AGlScene
//---------------------------------------------------------------------------
#if defined(__WXWIDGETS__)

#include "wx/zipstrm.h"
#include "wx/wfstream.h"

#include "wx/fontdlg.h"
#include "wx/dialog.h"
#include "wx/combobox.h"
#include "wx/checkbox.h"

const int TwxGlScene_LastFontSize = 72;
const int TwxGlScene_MaxFontSize = 28;
const int TwxGlScene_MinFontSize = 8;

class dlgEditOlexFont: public wxDialog  {
protected:
  wxCheckBox* cbFixed, *cbBold, *cbItalic;
  wxComboBox* cbSize, *cbFileName;
public:
  dlgEditOlexFont(const olxstr& fntId, const TStrList& fntFiles) : 
      wxDialog(NULL, -1, wxT("Olex2 Font"), wxDefaultPosition )  {

    TwxGlScene::MetaFont mf(fntId);
    wxBoxSizer *AASizer = new wxBoxSizer( wxHORIZONTAL );
      AASizer->Add( new wxStaticText(this, -1, wxT("File name")), wxALL, 5);
    cbFileName = new wxComboBox(this, -1, mf.GetFileName().u_str());
    for( size_t i=0; i < fntFiles.Count(); i++ )
      cbFileName->Append( fntFiles[i].u_str() ); 
    AASizer->Add( cbFileName, 0, wxALL, 5);

    wxBoxSizer *ASizer = new wxBoxSizer( wxHORIZONTAL );
    cbSize = new wxComboBox(this, -1, olxstr(mf.GetSize()).u_str() );
    cbSize->AppendString(wxT("10"));
    cbSize->AppendString(wxT("12"));
    cbSize->AppendString(wxT("16"));
    cbSize->AppendString(wxT("20"));
    cbSize->AppendString(wxT("24"));
      ASizer->Add( cbSize, 0, wxALL, 5);
    cbFixed = new wxCheckBox(this, -1, wxT("Fixed width"));
    cbFixed->SetValue( mf.IsFixed() );
      ASizer->Add( cbFixed, 0, wxALL, 5);

    wxBoxSizer *BSizer = new wxBoxSizer( wxHORIZONTAL );
    cbBold = new wxCheckBox(this, -1, wxT("Bold"));
    cbBold->SetValue( mf.IsBold() );
      BSizer->Add( cbBold, 0, wxALL, 5);
    cbItalic = new wxCheckBox(this, -1, wxT("Italic"));
    cbItalic->SetValue( mf.IsItalic() );
      BSizer->Add( cbItalic, 0, wxALL, 5);

    wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );
    ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 1);
    ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 1);
    ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxALL, 1);

    wxBoxSizer *TopSizer = new wxBoxSizer( wxVERTICAL );
    TopSizer->Add( AASizer,     0, wxALL, 1);
    TopSizer->Add( ASizer,     0, wxALL, 1);
    TopSizer->Add( BSizer,     0, wxALL, 1);
    TopSizer->Add( ButtonsSizer,     0, wxALL, 1);

    TopSizer->SetSizeHints( this );   // set size hints to honour minimum size
    SetSizer(TopSizer);
    Center();
  }
  olxstr GetIdString() const {
    long fs = 10;
    cbSize->GetValue().ToLong(&fs);
    return TwxGlScene::MetaFont::BuildOlexFontId( cbFileName->GetValue().c_str(), 
      (int)fs, 
      cbFixed->IsChecked(),
      cbBold->IsChecked(),
      cbItalic->IsChecked() );
  }

};


TwxGlScene::TwxGlScene(const olxstr& fontsFolder) : FontsFolder(fontsFolder)  {  }
//..............................................................................
TwxGlScene::~TwxGlScene()  {
  Destroy();
}
//..............................................................................
//TGlFont* TwxGlScene::CreateFont(const olxstr& name, void *Data, TGlFont *ReplaceFnt, bool BmpF, bool FixedW)  {
//  TGlFont *Fnt;
//  wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
//  
//  try  {  Font = *static_cast<wxFont*>(Data);  }
//  catch(...)  {
//    throw TInvalidArgumentException(__OlxSourceInfo, "invalid data type");
//  }
////////////////////////////////////////////////////////////////
//  FT_Library library;
//  FT_Face face;
//  FT_Error error = FT_Init_FreeType( &library );
//  if( error )
//    throw TFunctionFailedException(__OlxSourceInfo, "could not load the FreeType2 librray");
//  error = FT_New_Face(library, "C:/WINDOWS/Fonts/arial.ttf", 0, &face);
//  if( error )
//    throw TFunctionFailedException(__OlxSourceInfo, "could not load font face");
//  const int fs = 18;
//  error = FT_Set_Pixel_Sizes(face, 0, fs);
/////////////////////////////////////////////////////////////////
//  if( ReplaceFnt )  {
//    Fnt = ReplaceFnt;
//    Fnt->ClearData();
//  }
//  else
//    Fnt = new TGlFont(name);
//
//  const int maxHeight = face->size->metrics.height/64;
//  unsigned char* dt[256];
////  FT_BBox bbox;
//  Fnt->IdString(Font.GetNativeFontInfoDesc().c_str());
//  for( int i=0; i < 256; i++ )  {
//    FT_UInt glyph_index = FT_Get_Char_Index( face, i );
//    error = FT_Load_Glyph(face, glyph_index, 0);
//    FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
//    FT_Bitmap& bmp = face->glyph->bitmap;
//    dt[i] = new unsigned char [maxHeight*maxHeight*3];
//    memset(dt[i], 0, maxHeight*maxHeight*3);
//    int dy = (maxHeight - face->glyph->bitmap_top-1);
//    for( int j=0; j < bmp.width; j++ )  {
//      for( int k=0; k < bmp.rows; k++ )  {
//        int ind = (k*bmp.pitch+j/8);
//        int bit = (1 << (7-j%8));
//        if( (bmp.buffer[ind]&bit) != 0 )  {
//          int ind1 = ((k+dy)*maxHeight + j+face->glyph->bitmap_left)*3;
//          if( ind1 >= maxHeight*maxHeight*3 )
//            continue;
//          dt[i][ind1] = 1;
//        }
//      }
//    }
//    Fnt->CharFromRGBArray(i, dt[i], maxHeight, maxHeight);
//  }
//  //Fnt->CreateTextures(ImageW, ImageW);
//  Fnt->CreateGlyphs(false, maxHeight, maxHeight);
//  for( int i=0; i < 256; i++ )
//    delete dt[i];
//  if( ReplaceFnt == NULL )
//    Fonts.Add(Fnt);
//  return Fnt;
//}
//..............................................................................
TGlFont* TwxGlScene::CreateFont(const olxstr& name, const olxstr& fntDesc, short Flags)  {
  if( MetaFont::IsOlexFont(fntDesc) )  {
    return ImportFont(name, fntDesc, Flags);
  }
  TGlFont *Fnt = FindFont(name);
  if( Fnt != NULL ) 
    Fnt->ClearData();
  else
    Fnt = new TGlFont(name);

  if( MetaFont::IsVectorFont(fntDesc) )  {
    olxdict<size_t, olxstr, TPrimitiveComparator> dummy;
    Fnt->CreateHershey(dummy, 120);
    if( FindFont(name) == NULL )
      Fonts.Add(Fnt);
    Fnt->SetIdString(fntDesc);
    MetaFont mf(fntDesc);
    Fnt->SetPointSize(mf.GetSize());
    return Fnt;
  }

  // LINUZ port - ... native font string is system dependent...
  wxFont Font(fntDesc.u_str());
  if( Font.GetPointSize() <= 1 )
    Font.SetPointSize(6);
    
  Fnt->SetIdString(Font.GetNativeFontInfoDesc().c_str());
  Fnt->SetPointSize(Font.GetPointSize());

  TPtrList<wxImage> Images;
  wxImage *Image;
  int ImageW = Font.GetPointSize()*2;
  wxMemoryDC memDC;
  memDC.SetFont(Font);
#if defined(__WXGTK__)
  wxBitmap Bmp(ImageW, ImageW);
#else
  wxBitmap Bmp(ImageW, ImageW, 1);
#endif
  memDC.SelectObject(Bmp);
  memDC.SetPen(*wxBLACK_PEN);
  memDC.SetBackground(*wxWHITE_BRUSH);
  memDC.SetBackgroundMode(wxSOLID);
  //memDC.SetTextBackground(*wxWHITE);
  memDC.SetTextForeground(*wxBLACK);
  for( int i=0; i < 256; i++ )  {
    memDC.SelectObject(Bmp);
    memDC.Clear();
    memDC.DrawText(wxString((olxch)i), 0, 0);
    memDC.SelectObject(wxNullBitmap);
    Image = new wxImage( Bmp.ConvertToImage() );
    Fnt->CharFromRGBArray(i, Image->GetData(), ImageW, ImageW);
    Images.Add(Image);
  }
  //Fnt->CreateTextures(ImageW, ImageW);
  Fnt->CreateGlyphsFromRGBArray(Font.IsFixedWidth(), ImageW, ImageW);
  for( int i=0; i < 256; i++ )  // to find maximum height and width
    delete Images[i];
  
  if( FindFont(name) == NULL )
    Fonts.Add(Fnt);
  return Fnt;
}
//..............................................................................
//..............................................................................
void TwxGlScene_RipFont(wxFont& fnt, TGlFont& glf, TEBitArray& ba)  {
  TPtrList<wxImage> Images;
  glf.ClearData();
  int ImageW = fnt.GetPointSize()*2;
  wxMemoryDC memDC;
  memDC.SetFont(fnt);
  wxBitmap Bmp(ImageW, ImageW);
  memDC.SelectObject(Bmp);
  memDC.SetPen(*wxBLACK_PEN);
  memDC.SetBackground(*wxWHITE_BRUSH);
  memDC.SetBackgroundMode(wxSOLID);
  memDC.SetTextForeground(*wxBLACK);
  for( int i=0; i < 256; i++ )  {
    memDC.SelectObject(Bmp);
    memDC.Clear();
    memDC.DrawText(wxString((olxch)i), 0, 0);
    memDC.SelectObject(wxNullBitmap);
    wxImage* Image = new wxImage( Bmp.ConvertToImage() );
    glf.CharFromRGBArray(i, Image->GetData(), ImageW, ImageW);
    Images.Add(Image);
  }
  ba.Clear();
  uint16_t maxw = glf.GetMaxWidth()+1, maxh = glf.GetMaxHeight()+1;
  ba.SetSize( 256*maxw*maxh );
  for( int i=0; i < 256; i++ )  {
    TFontCharSize* cs = glf.CharSize(i);
    if( cs->Data == NULL )  continue;
    for( uint16_t j=0; j < maxw; j++ )  {
      for( uint16_t k=0; k < maxh; k++ )  {
        int ind = (k*ImageW+j)*3;
        if( (cs->Data[ind] | cs->Data[ind+1] | cs->Data[ind+2]) != cs->Background ) // is black?
          ba.SetTrue( maxw*(i*maxh+k) + j );
      }
    }
    delete Images[i];
  }
}
//..............................................................................
void TwxGlScene_RipFontA(wxFont& fnt, TGlFont& glf, wxZipOutputStream& zos)  {
  TEBitArray ba;
  olxstr prefix( fnt.IsFixedWidth() ? "f" : "n" );
  if( fnt.GetStyle() == wxFONTSTYLE_ITALIC )  {
    prefix << "i";
    if( fnt.GetWeight() == wxFONTWEIGHT_BOLD )
      prefix << "b";
  }
  else if( fnt.GetWeight() == wxFONTWEIGHT_BOLD )
    prefix << "rb";
  else
    prefix << "r";
  prefix.Format(4, true, '_');
  for( int i=TwxGlScene_MinFontSize; i <= TwxGlScene_MaxFontSize; i+=1 )  {
    fnt.SetPointSize(i);
    TwxGlScene_RipFont(fnt, glf, ba);
    zos.PutNextEntry((olxstr(prefix) << i).u_str());
    uint16_t s = glf.GetMaxWidth()+1; 
    zos.Write(&s, sizeof(uint16_t));
    s = glf.GetMaxHeight()+1; 
    zos.Write(&s, sizeof(uint16_t));
    zos.Write(ba.GetData(), ba.CharCount());
    zos.CloseEntry();
  }
  fnt.SetPointSize(TwxGlScene_LastFontSize);
  TwxGlScene_RipFont(fnt, glf, ba);
  zos.PutNextEntry((olxstr(prefix) << TwxGlScene_LastFontSize).u_str());
  uint16_t s = glf.GetMaxWidth()+1; 
  zos.Write(&s, sizeof(uint16_t));
  s = glf.GetMaxHeight()+1; 
  zos.Write(&s, sizeof(uint16_t));
  zos.Write(ba.GetData(), ba.CharCount());
  zos.CloseEntry();
}
void TwxGlScene::ExportFont(const olxstr& name, const olxstr& fileName)  {
  wxFont Font(10, wxMODERN, wxNORMAL, wxNORMAL);//|wxFONTFLAG_ANTIALIASED);
  TStrList toks(name, '&');
  TGlFont Fnt(name);
  wxFileOutputStream fos( fileName.u_str() );
  fos.Write("ofnt", 4);
  wxZipOutputStream zos(fos, 9);
  for( size_t i=0; i < toks.Count(); i++ )  {
    Font.SetNativeFontInfo( toks[i].u_str() );
    // regular
    TwxGlScene_RipFontA(Font, Fnt, zos);
    // bold
    Font.SetWeight(wxFONTWEIGHT_BOLD);
    TwxGlScene_RipFontA(Font, Fnt, zos);
    // bold italic
    Font.SetStyle(wxFONTSTYLE_ITALIC);
    TwxGlScene_RipFontA(Font, Fnt, zos);
    // italic
    Font.SetWeight(wxFONTWEIGHT_NORMAL);
    TwxGlScene_RipFontA(Font, Fnt, zos);
  }
  zos.Close();
  fos.Close();
}
//..............................................................................
TGlFont* TwxGlScene::ImportFont(const olxstr& Name, const olxstr& fntDesc, short Flags)  {
  MetaFont mf(fntDesc);
  if( mf.GetSize() <= 1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "invalid font size");
  TGlFont* Fnt = FindFont(Name);
  olxstr fileName( FontsFolder + mf.GetFileName() );
  if( !TEFile::Exists(fileName) )
    return NULL;
  wxFileInputStream fis( fileName.u_str() );
  char sig[4];
  int originalFntSize = -1;
  olxstr fntPrefix( mf.GetFileIdString().SubStringTo(4) );
  fis.Read(sig, 4);
  if( olxstr::o_memcmp(sig, "ofnt", 4) != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file signature");

  wxZipInputStream* zin = new wxZipInputStream(fis);
  uint16_t maxw, maxh;
  wxZipEntry* zen = NULL;
  olxstr entryName(mf.GetFileIdString());
  bool Scale = false;
  if( mf.GetSize() < TwxGlScene_MinFontSize )  {
    entryName = fntPrefix;
    entryName << TwxGlScene_MinFontSize;
    originalFntSize = TwxGlScene_MinFontSize;
    Scale = true;
  }
  else if( mf.GetSize() > TwxGlScene_MaxFontSize )  {
    entryName = fntPrefix;
    entryName << TwxGlScene_LastFontSize;
    originalFntSize = TwxGlScene_LastFontSize;
    Scale = true;
  }

  while( (zen = zin->GetNextEntry()) != NULL )  {
    if( entryName == zen->GetName().c_str() )
      break;
    delete zen;
  }
  if( zen == NULL || (entryName != zen->GetName().c_str()) )  {
    delete zin;
    throw TFunctionFailedException(__OlxSourceInfo, "invalid font description");
  }
  zin->Read(&maxw, sizeof(uint16_t));
  zin->Read(&maxh, sizeof(uint16_t));
  int contentLen = zin->GetLength() - 2*sizeof(uint16_t);
  unsigned char * bf1 = new unsigned char[ contentLen + 1];
  zin->Read(bf1, contentLen);
  TEBitArray ba(bf1, contentLen, true);  // bf1 will be deleted

  if( Fnt != NULL ) 
    Fnt->ClearData();
  else
    Fnt = new TGlFont(Name);

  Fnt->SetPointSize( mf.GetSize() );
  Fnt->SetIdString( mf.GetIdString() );
  if( Scale )  {
    TPtrList<wxImage> Images;
    int bit_cnt = 0;
    uint16_t imgw = (uint16_t)(olx_round((double)maxw*mf.GetSize()/originalFntSize)), 
      imgh = (uint16_t)(olx_round((double)maxh*mf.GetSize()/originalFntSize));
    for( int i=0; i < 256; i++ )  {
      wxImage* img = new wxImage(maxw, maxh);
      unsigned char* idata = img->GetData();
      memset(idata, 0, maxw*maxh);
      for( uint16_t j=0; j < maxw; j++ )  {
        for( uint16_t k=0; k < maxh; k++ )  {
          if( ba.Get(maxw*(i*maxh + k) + j) )
            idata[(k*maxw+j)*3] = 255;
          bit_cnt++;
        }
      }
      img->Rescale(imgw, imgh, wxIMAGE_QUALITY_HIGH);
      Fnt->CharFromRGBArray(i, img->GetData(), imgw, imgh);
      Images.Add(img);
    }
    Fnt->CreateGlyphsFromRGBArray(mf.IsFixed(), imgw, imgh);
    for( int i=0; i < 256; i++ )  // to find maximum height and width
      delete Images[i];
  }
  else
    Fnt->CreateGlyphs(ba, mf.IsFixed(), maxw, maxh);
  if( FindFont(Name) == NULL )
    Fonts.Add(Fnt);
  zin->CloseEntry();
  delete zen;
  delete zin;
  return Fnt;
}
//..............................................................................
void TwxGlScene::ScaleFonts(double scale)  {
  if( !FontSizes.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "call RestoreFontScale");
  FontSizes.SetCount( Fonts.Count() );
  for( size_t i=0; i < Fonts.Count(); i++ )  // kepp the sizes if scaling fails
    FontSizes[i] = Fonts[i]->GetPointSize();
  for( size_t i=0; i < Fonts.Count(); i++ )  {
    olxstr fontId;
    if( MetaFont::IsOlexFont(Fonts[i]->GetIdString()) )  {
      MetaFont mf(Fonts[i]->GetIdString());
      int sz = (int)(Fonts[i]->GetPointSize()*scale);
      mf.SetSize( sz < 2 ? 2 : sz );
      fontId << mf.GetIdString();
    }
    else if( MetaFont::IsVectorFont(Fonts[i]->GetIdString()) )
      fontId << Fonts[i]->GetIdString();
    else  {
      wxFont font;
      ((wxFontBase&)font).SetNativeFontInfo(Fonts[i]->GetIdString().u_str());
      int sz = (int)(font.GetPointSize()*scale);
      font.SetPointSize( sz < 2 ? 2 : sz );
      fontId = font.GetNativeFontInfoDesc().c_str();
    }
    CreateFont(Fonts[i]->GetName(), fontId);
  }
}
//..............................................................................
void TwxGlScene::RestoreFontScale()  {
  if( FontSizes.IsEmpty() || FontSizes.Count() != Fonts.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "call ScaleFont first");
  for( size_t i=0; i < Fonts.Count(); i++ )  {
    olxstr fontId;
    if( MetaFont::IsOlexFont(Fonts[i]->GetIdString()) )  {
      MetaFont mf(Fonts[i]->GetIdString());
      mf.SetSize(FontSizes[i]);
      fontId = mf.GetIdString();
    }
    else if( MetaFont::IsVectorFont(Fonts[i]->GetIdString()) )
      fontId << Fonts[i]->GetIdString();
    else  {
      wxFont font;
      ((wxFontBase&)font).SetNativeFontInfo(Fonts[i]->GetIdString().u_str());
      font.SetPointSize(FontSizes[i]);
      fontId = font.GetNativeFontInfoDesc().c_str();
    }
    CreateFont(Fonts[i]->GetName(), fontId);
  }
  FontSizes.Clear();
}
//..............................................................................
olxstr TwxGlScene::ShowFontDialog(TGlFont* glf, const olxstr& fntDesc)  {
  olxstr rv(EmptyString);
  olxstr fntId( (glf != NULL) ? glf->GetIdString() : fntDesc );
  if( MetaFont::IsOlexFont(fntId) )  {
    if( fntId.CharAt(fntId.Length()-1) == ':' )  // default font
      fntId << MetaFont::BuildOlexFontId(EmptyString, 12, true, false, false);
    TStrList fntFiles;
    TEFile::ListDir( FontsFolder, fntFiles, "*.fnt", sefFile );
    dlgEditOlexFont* dlg = new dlgEditOlexFont( fntId, fntFiles );
    if( dlg->ShowModal() == wxID_OK )  {
      if( glf != NULL )  // recreate, if provided
        CreateFont( glf->GetName(), dlg->GetIdString() );
      rv = dlg->GetIdString();
    }
    dlg->Destroy();
  }
  else  {
    wxFontData fnt_data;
    wxFont Fnt(12, wxMODERN, wxNORMAL, wxNORMAL);
    if( !fntId.IsEmpty() )  // is not default?
      Fnt.SetNativeFontInfo( fntId.u_str() );
    fnt_data.SetInitialFont(Fnt);
    fnt_data.EnableEffects(false);
    wxFontDialog fD(NULL, fnt_data);
    if( fD.ShowModal() == wxID_OK )  {
      Fnt = fD.GetFontData().GetChosenFont();
      if( glf != NULL )  // recreate if provided
        CreateFont(glf->GetName(), Fnt.GetNativeFontInfoDesc().c_str());
      rv = Fnt.GetNativeFontInfoDesc().c_str();
    }
  }
  return rv;
}
//..............................................................................
//..............................................................................
//..............................................................................
bool TwxGlScene::MetaFont::SetIdString(const olxstr& idstr)  {
  if( AGlScene::MetaFont::SetIdString(idstr) )
    return true;
  wxFont f(idstr.u_str());
  if( !f.IsOk() )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid font ID");
  Fixed = f.IsFixedWidth();
  Italic = (f.GetStyle() == wxFONTSTYLE_ITALIC);
  Bold = (f.GetWeight() == wxFONTWEIGHT_BOLD);
  Size = f.GetPointSize();
  return true;
}
//..............................................................................
olxstr TwxGlScene::MetaFont::GetIdString() const {
  const olxstr rv = AGlScene::MetaFont::GetIdString();
  if( !rv.IsEmpty() )  return rv;
  wxFont f(OriginalId.u_str());
  f.SetStyle(Italic ? wxFONTSTYLE_ITALIC: wxFONTSTYLE_NORMAL);
  f.SetWeight(Bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
  f.SetPointSize(Size);
  return f.GetNativeFontInfoDesc().c_str();
}
//..............................................................................
#endif // __WXWIDGETS__

