//---------------------------------------------------------------------------

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "bapp.h"

// for InterlockedIncrement in
#if defined __WIN32__ && __BORLANDC__
  #include <windows.h>
  #include <winbase.h>
#endif

#include "wxglscene.h"
#include "exception.h"
#include "glfont.h"
#include "glrender.h"
#include "efile.h"
#include "bitarray.h"

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
const int TwxGlScene_MaxFontSize = 36;
const int TwxGlScene_MinFontSize = 10;

class dlgEditOlexFont: public wxDialog  {
protected:
  wxCheckBox* cbFixed, *cbBold, *cbItalic;
  wxComboBox* cbSize;
public:
  dlgEditOlexFont(const olxstr& fntId) : wxDialog(NULL, -1, wxT("Olex2 Font") )  {
    olxstr fontId(fntId.SubStringFrom(TwxGlScene::OlexFontId.Length()) );
    wxBoxSizer *ASizer = new wxBoxSizer( wxHORIZONTAL );
    cbSize = new wxComboBox(this, -1, fontId.SubStringFrom(4).u_str());
      ASizer->Add( cbSize, 0, wxALL, 5);
    cbFixed = new wxCheckBox(this, -1, wxT("Fixed width"));
    cbFixed->SetValue( fontId.CharAt(0) == 'f' );
      ASizer->Add( cbFixed, 0, wxALL, 5);

    wxBoxSizer *BSizer = new wxBoxSizer( wxHORIZONTAL );
    cbBold = new wxCheckBox(this, -1, wxT("Bold"));
    cbBold->SetValue( fontId.IndexOf('b') != -1 );
      BSizer->Add( cbBold, 0, wxALL, 5);
    cbItalic = new wxCheckBox(this, -1, wxT("Italic"));
    cbItalic->SetValue( fontId.IndexOf('i') != -1 );
      BSizer->Add( cbItalic, 0, wxALL, 5);

    wxBoxSizer *ButtonsSizer = new wxBoxSizer( wxHORIZONTAL );
    ButtonsSizer->Add( new wxButton( this, wxID_OK, wxT("OK") ), 0, wxALL, 1);
    ButtonsSizer->Add( new wxButton( this, wxID_CANCEL, wxT("Cancel") ), 0, wxALL, 1);
    ButtonsSizer->Add( new wxButton( this, wxID_HELP, wxT("Help") ),     0, wxALL, 1);

    wxBoxSizer *TopSizer = new wxBoxSizer( wxVERTICAL );
    TopSizer->Add( ASizer,     0, wxALL, 1);
    TopSizer->Add( BSizer,     0, wxALL, 1);
    TopSizer->Add( ButtonsSizer,     0, wxALL, 1);

    TopSizer->SetSizeHints( this );   // set size hints to honour minimum size
    SetSizer(TopSizer);
    Center();
  }
  olxstr GetIdString() const {
    olxstr prefix( TwxGlScene::OlexFontId );
    prefix << (cbFixed->IsChecked() ? "f" : "n" );
    if( cbItalic->IsChecked() )  {
      prefix << "i";
      if( cbBold->IsChecked() )
        prefix << "b";
    }
    else if( cbBold->IsChecked() )
      prefix << "rb";
    else
      prefix << "r";
    prefix.Format(4, true, '_');
    return prefix << cbSize->GetValue().c_str();
  }

};


const olxstr TwxGlScene::OlexFontId("#olex2.fnt:");

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
  if( fntDesc.StartsFrom(OlexFontId) )  {
    return ImportFont(name, fntDesc.SubStringFrom(OlexFontId.Length()), Flags);
  }
  TGlFont *Fnt = FindFont(name);
  wxFont Font( fntDesc.u_str() );
    
  if( Fnt != NULL ) 
    Fnt->ClearData();
  else
    Fnt = new TGlFont(name);

  // LINUZ port - ... native font string is system dependent...
  if( Font.GetPointSize() <= 1 )
    Font.SetPointSize(6);
    
  Fnt->IdString(Font.GetNativeFontInfoDesc().c_str());
  Fnt->SetPointSize( Font.GetPointSize() );
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
  Fnt->CreateGlyphs(Font.IsFixedWidth(), ImageW, ImageW);
  for( int i=0; i < 256; i++ )  // to find maximum height and width
    delete Images[i];
  
  if( FindFont(name) == NULL )
    Fonts.Add(Fnt);
  return Fnt;
}
//..............................................................................
void TwxGlScene::StartDraw()  {  AGlScene::StartDraw();  }
//..............................................................................
void TwxGlScene::EndDraw()  {  AGlScene::EndDraw();  }
//..............................................................................
void TwxGlScene::StartSelect(int x, int y, GLuint *Bf)  {  AGlScene::StartSelect(x, y, Bf);  }
//..............................................................................
void TwxGlScene::EndSelect()  {  AGlScene::EndSelect();  }
//..............................................................................
void TwxGlScene::Destroy()  {  AGlScene::Destroy();  }
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
  ba.SetSize( 256*glf.MaxWidth()*glf.MaxHeight() );
  int bit_cnt=0;
  for( int i=0; i < 256; i++ )  {
    TFontCharSize* cs = glf.CharSize(i);
    if( cs->Data == NULL )  {
      bit_cnt += glf.MaxWidth()*glf.MaxHeight();
      continue;
    }
    for( int j=0; j < glf.MaxWidth(); j++ )  {
      for( int k=0; k < glf.MaxHeight(); k++ )  {
        int ind = (k*ImageW+j)*3;
        if( (cs->Data[ind] | cs->Data[ind+1] | cs->Data[ind+2]) != cs->Background ) // is black?
          ba.SetTrue(bit_cnt);
        bit_cnt++;
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
    uint16_t s = glf.MaxWidth(); 
    zos.Write(&s, sizeof(uint16_t));
    s = glf.MaxHeight(); 
    zos.Write(&s, sizeof(uint16_t));
    zos.Write(ba.GetData(), ba.CharCount());
    zos.CloseEntry();
  }
  fnt.SetPointSize(TwxGlScene_LastFontSize);
  TwxGlScene_RipFont(fnt, glf, ba);
  zos.PutNextEntry((olxstr(prefix) << TwxGlScene_LastFontSize).u_str());
  uint16_t s = glf.MaxWidth(); 
  zos.Write(&s, sizeof(uint16_t));
  s = glf.MaxHeight(); 
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
  for( int i=0; i < toks.Count(); i++ )  {
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
  TGlFont* Fnt = FindFont(Name);
  olxstr fileName( FontsFolder + "olex2.fnt" );
  wxFileInputStream fis( fileName.u_str() );
  char sig[4];
  int fntSize = fntDesc.SubStringFrom(4).ToInt(), originalFntSize = -1;
  olxstr fntPrefix = fntDesc.SubStringTo(4);
  if( fntSize == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "invalid font size");
  fis.Read(sig, 4);
  if( olxstr::o_memcmp(sig, "ofnt", 4) != 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file signature");

  wxZipInputStream zin(fis);
  uint16_t maxw, maxh;
  wxZipEntry* zen = NULL;
  olxstr entryName(fntDesc);
  bool Scale = false;
  if( fntSize < TwxGlScene_MinFontSize )  {
    entryName = fntPrefix;
    entryName << TwxGlScene_MinFontSize;
    originalFntSize = TwxGlScene_MinFontSize;
    Scale = true;
  }
  else if( fntSize > TwxGlScene_MaxFontSize )  {
    entryName = fntPrefix;
    entryName << TwxGlScene_LastFontSize;
    originalFntSize = TwxGlScene_LastFontSize;
    Scale = true;
  }

  while( (zen = zin.GetNextEntry()) != NULL )  {
    if( entryName == zen->GetName().c_str() )
      break;
    delete zen;
  }
  if( zen == NULL || (entryName != zen->GetName().c_str()) )  {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid font description");
  }
  zin.Read(&maxw, sizeof(uint16_t));
  zin.Read(&maxh, sizeof(uint16_t));
  int contentLen = zin.GetLength() - 2*sizeof(uint16_t);
  char * bf1 = new char[ contentLen + 1];
  zin.Read(bf1, contentLen);
  TEBitArray ba(bf1, contentLen );
  delete [] bf1;

  if( Fnt != NULL ) 
    Fnt->ClearData();
  else
    Fnt = new TGlFont(Name);

  Fnt->SetPointSize( fntSize );
  Fnt->IdString( olxstr(OlexFontId) << fntDesc );
  TPtrList<wxImage> Images;
  int bit_cnt = 0;
  uint16_t imgw = Scale ? (uint16_t)(Round((double)maxw*fntSize/originalFntSize)) : maxw, 
    imgh = Scale ? (uint16_t)(Round((double)maxh*fntSize/originalFntSize)) : maxh;
  for( int i=0; i < 256; i++ )  {
    wxImage* img = new wxImage(maxw, maxh);
    unsigned char* idata = img->GetData();
    memset(idata, 0, maxw*maxh);
    for( int j=0; j < maxw; j++ )  {
      for( int k=0; k < maxh; k++ )  {
        if( ba.Get(bit_cnt) )
          idata[(k*maxw+j)*3] = 255;
        bit_cnt++;
      }
    }
    if( Scale )
      img->Rescale(imgw, imgh, wxIMAGE_QUALITY_HIGH);
    Fnt->CharFromRGBArray(i, img->GetData(), imgw, imgh);
    Images.Add(img);
  }
  Fnt->CreateGlyphs(Name.CharAt(0) == 'f', imgw, imgh);
  for( int i=0; i < 256; i++ )  // to find maximum height and width
    delete Images[i];
  if( FindFont(Name) == NULL )
    Fonts.Add(Fnt);
  delete zen;
  return Fnt;
}
//..............................................................................
void TwxGlScene::ScaleFonts(double scale)  {
  if( !FontSizes.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "call RestoreFontScale");
  FontSizes.SetCount( Fonts.Count() );
  for( int i=0; i < Fonts.Count(); i++ )  {
    FontSizes[i] = Fonts[i]->GetPointSize();
    olxstr fontId;
    if( Fonts[i]->IdString().StartsFrom(OlexFontId) )  {
      fontId = Fonts[i]->IdString().SubStringTo(OlexFontId.Length()+4);
      fontId << Round( Fonts[i]->GetPointSize()*scale );
    }
    else  {
      wxFont font;
      ((wxFontBase&)font).SetNativeFontInfo( Fonts[i]->IdString().u_str() );
      font.SetPointSize(font.GetPointSize()*scale);
      fontId = font.GetNativeFontInfoDesc().c_str();
    }
    CreateFont(Fonts[i]->GetName(), fontId);
  }
}
//..............................................................................
void TwxGlScene::RestoreFontScale()  {
  if( FontSizes.IsEmpty() || FontSizes.Count() != Fonts.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "call ScaleFont first");
  for( int i=0; i < Fonts.Count(); i++ )  {
    olxstr fontId;
    if( Fonts[i]->IdString().StartsFrom(OlexFontId) )  {
      fontId = Fonts[i]->IdString().SubStringTo(OlexFontId.Length()+4);
      fontId << FontSizes[i];
    }
    else  {
      wxFont font;
      ((wxFontBase&)font).SetNativeFontInfo( Fonts[i]->IdString().u_str() );
      font.SetPointSize(FontSizes[i]);
      fontId = font.GetNativeFontInfoDesc().c_str();
    }
    CreateFont(Fonts[i]->GetName(), fontId);
  }
  FontSizes.Clear();
}
//..............................................................................
bool TwxGlScene::ShowFontDialog(TGlFont& glf)  {
  bool res = false;
  if( glf.IdString().StartsFrom(OlexFontId) )  {
    dlgEditOlexFont* dlg = new dlgEditOlexFont( glf.IdString() );
    if( dlg->ShowModal() == wxID_OK )  {
      CreateFont( glf.GetName(), dlg->GetIdString() );
      res = true;
    }
    dlg->Destroy();
  }
  else  {
    wxFontData fnt_data;
    wxFont Fnt;
    Fnt.SetNativeFontInfo( glf.IdString().u_str() );
    fnt_data.SetInitialFont(Fnt);
    wxFontDialog fD(NULL, fnt_data);
    if( fD.ShowModal() == wxID_OK )  {
      Fnt = fD.GetFontData().GetChosenFont();
      CreateFont(glf.GetName(), Fnt.GetNativeFontInfoDesc().c_str());
      res = true;
    }
  }
  return res;
}
#endif // __WXWIDGETS__

