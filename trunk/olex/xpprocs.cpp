//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004-2009
//----------------------------------------------------------------------------//
#ifdef _SVN_REVISION_AVAILABLE
#  include "../svn_revision.h"
#endif
#include "wx/protocol/http.h"

#include "mainform.h"
#include "xglcanv.h"
#include "xglapp.h"

#include "wx/clipbrd.h"
#include "wx/filesys.h"
#include "wx/cursor.h"
#include "wx/colordlg.h"
#include "wx/fontdlg.h"
#include "wx/progdlg.h"

#include "wx/image.h"
#include "wx/dcps.h"
#include "evaln.h"
#include "imagep.h"

#include "dgrad.h"
#include "edit.h"
#include "updateoptions.h"
#include "ptable.h"

#include "symmlib.h"
#include "bitarray.h"
#include "glbackground.h"
#include "glgroup.h"
#include "gpcollection.h"

#include "xatom.h"
#include "xbond.h"
#include "xplane.h"
#include "xline.h"
#include "xreflection.h"
#include "gllabel.h"
#include "gllabels.h"
#include "dunitcell.h"

#include "symmparser.h"

#include "efile.h"

#include "ins.h"
#include "cif.h"
#include "hkl.h"
#include "p4p.h"
#include "mol.h"
#include "crs.h"
#include "xyz.h"

#include "fsext.h"
#include "html/htmlext.h"

#include <iostream>

#include "pyext.h"

#include "ellipsoid.h"

#include "obase.h"
#include "glbitmap.h"
#include "bitarray.h"
#include "arrays.h"
#include "sgtest.h"
#include "network.h"

#include "filesystem.h"
#include "httpfs.h"
#include "wxzipfs.h"

#include "ecast.h"
#include "sls.h"

#include "cmdline.h"

#include "xlcongen.h"

#include "tls.h"
#include "ecast.h"

#include "arrays.h"
#include "estrbuffer.h"
#include "unitcell.h"
#include "xgrid.h"
#include "symmtest.h"
#include "xlattice.h"

#include "matprop.h"
#include "utf8file.h"

#include "msgbox.h"

#include "shellutil.h"

#ifndef __BORLANDC__
  #include "ebtree.h"
#endif

#include "dusero.h"

#ifdef __GNUC__
  #ifdef Bool
    #undef Bool
  #endif
  #ifdef Success
    #undef Success
  #endif
#endif

#include "olxmps.h"
#include "beevers-lipson.h"
#include "maputil.h"

#include "olxvar.h"

#include "ecast.h"
#include "atomref.h"
#include "wxglscene.h"
#include "equeue.h"
#include "xmacro.h"
#include "vcov.h"

#include "sfutil.h"
#include "ortdraw.h"
#include "ortdrawtex.h"
#include "testsuit.h"
#include "catomlist.h"
#include "updateapi.h"
#include "planesort.h"
// FOR DEBUG only
#include "egraph.h"
#include "olxth.h"
#include "md5.h"
#include "sha.h"
#include "exparse/expbuilder.h"
#include "encodings.h"
#include "cifdp.h"
#include "glutil.h"
//#include "base_2d.h"
//#include "gl2ps/gl2ps.c"

static const olxstr StartMatchCBName("startmatch");
static const olxstr OnMatchCBName("onmatch");
static const olxstr OnModeChangeCBName("modechange");
static const olxstr NoneString("none");

int CalcL( int v )  {
  int r = 0;
  while( (v/=2) > 2 )  r++;
  return r+2;
}

//olex::IBasicMacroProcessor *olex::OlexPort::MacroProcessor;

//..............................................................................
void TMainForm::funFileLast(const TStrObjList& Params, TMacroError &E)  {
  size_t index = 0;
  if( FXApp->XFile().HasLastLoader() )  index = 1;
  if( FRecentFiles.Count() <= index )  {
    E.ProcessingError(__OlxSrcInfo, "no last file");
    return;
  }
  if( !TEFile::Exists(FRecentFiles[index]) )  {
    E.ProcessingError(__OlxSrcInfo, "file does not exists anymore");
    return;
  }
  E.SetRetVal( FRecentFiles[index] );
}
//..............................................................................
void TMainForm::funCell(const TStrObjList& Params, TMacroError &E)  {
  if( Params[0].IsNumber() && false )  {  // mutliplies cartesian cell... tests...
    const double k = Params[0].ToDouble();
    TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
    au.Axes()[0] *= k;  au.Axes()[1] *= k;  au.Axes()[2] *= k;
    au.InitMatrices();
    for( size_t i=0; i < au.AtomCount(); i++ )
      au.CartesianToCell(au.GetAtom(i).ccrd());
    Macros.ProcessMacro("fuse", E);
  }
  else  {
    if( Params[0].Equalsi('a') )
      E.SetRetVal( FXApp->XFile().GetAsymmUnit().Axes()[0].GetV() );
    else if( Params[0].Equalsi('b') )
      E.SetRetVal( FXApp->XFile().GetAsymmUnit().Axes()[1].GetV() );
    else if( Params[0].Equalsi('c') )
      E.SetRetVal( FXApp->XFile().GetAsymmUnit().Axes()[2].GetV() );
    else if( Params[0].Equalsi("alpha") )
      E.SetRetVal( FXApp->XFile().GetAsymmUnit().Angles()[0].GetV() );
    else if( Params[0].Equalsi("beta") )
      E.SetRetVal( FXApp->XFile().GetAsymmUnit().Angles()[1].GetV() );
    else if( Params[0].Equalsi("gamma") )
      E.SetRetVal( FXApp->XFile().GetAsymmUnit().Angles()[2].GetV() );
    else if( Params[0].Equalsi("volume") )
      E.SetRetVal( olxstr::FormatFloat(2, FXApp->XFile().GetUnitCell().CalcVolume()) );
    else
      E.ProcessingError(__OlxSrcInfo, "invalid argument: ") << Params[0];
  }
}
//..............................................................................
void TMainForm::funCif(const TStrObjList& Params, TMacroError &E)  {
  TCif& cf = FXApp->XFile().GetLastLoader<TCif>();
  if( cf.ParamExists(Params[0]) )  {
    TCif::CifData* cd = cf.FindParam(Params[0]);
    E.SetRetVal(cd->data.Text(EmptyString));
  }
  else
    E.SetRetVal(XLibMacros::NAString);
}
//..............................................................................
void TMainForm::funP4p(const TStrObjList& Params, TMacroError &E)  {
}
//..............................................................................
void TMainForm::funHasGUI(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal( true );
}
//..............................................................................
void TMainForm::funExtraZoom(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( FXApp->GetExtraZoom() );
  else
    FXApp->SetExtraZoom( Params[0].ToDouble() );
}
//..............................................................................
void TMainForm::funIsOS(const TStrObjList& Params, TMacroError &E)  {
#if defined(__WXMSW__)
  E.SetRetVal( Params[0].Equalsi("win") );
#elif defined(__WXMAC__) || defined(__MAC__)
  E.SetRetVal( Params[0].Equalsi("mac") );
#elif defined(__WXGTK__)
  E.SetRetVal(Params[0].Equalsi("linux") );
#else
  E.SetRetVal(false);
#endif
}
//..............................................................................
void TMainForm::funCrs(const TStrObjList& Params, TMacroError &E)  {
  TCRSFile& cf = FXApp->XFile().GetLastLoader<TCRSFile>();
  if( Params[0].Equalsi("sg") )
    E.SetRetVal(cf.GetSG() != NULL ? cf.GetSG()->GetName() : EmptyString );
  else
    E.SetRetVal(XLibMacros::NAString);
}
//..............................................................................
void TMainForm::funDataDir(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(DataDir.SubStringFrom(0, 1));
}
//..............................................................................
void TMainForm::funStrcat(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(Params[0] + Params[1]);
}
//..............................................................................
void TMainForm::funStrcmp(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(Params[0] == Params[1]);
}
//..............................................................................
void TMainForm::funGetEnv(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )  {
#if defined(__WIN32__) && defined(_UNICODE) && defined(_MSC_VER)
    if( _wenviron != NULL )  {
      for( size_t i=0; _wenviron[i] != NULL; i++ )
        TBasicApp::GetLog() << _wenviron[i] << '\n';
    }
#else
    extern char **environ;
    if( environ != NULL )  {
      for( size_t i=0; environ[i] != NULL; i++ )
        TBasicApp::GetLog() << environ[i] << '\n';
    }
#endif
  }
  else
    E.SetRetVal(olx_getenv(Params[0]));
}
//..............................................................................
void TMainForm::funFileSave(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(PickFile(Params[0], Params[1], Params[2], false));
}
//..............................................................................
void TMainForm::funFileOpen(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(PickFile(Params[0], Params[1], Params[2], true));
}
//..............................................................................
void TMainForm::funUnsetVar(const TStrObjList& Params, TMacroError &E)  {
  TOlxVars::UnsetVar(Params[0]);
}
//..............................................................................
void TMainForm::funSetVar(const TStrObjList& Params, TMacroError &E)  {
  TOlxVars::SetVar(Params[0], Params[1]);
}
//..............................................................................
void TMainForm::funGetVar(const TStrObjList& Params, TMacroError &E)  {
  const size_t ind = TOlxVars::VarIndex(Params[0]);
  if( ind == InvalidIndex )  {
    if( Params.Count() == 2 )
      E.SetRetVal( Params[1] );
    else  
      E.ProcessingError(__OlxSrcInfo, "Could not locate specified attribute: '") << Params[0] << '\'';
  }
  else
    E.SetRetVal( TOlxVars::GetVarStr(ind) );
}
//..............................................................................
void TMainForm::funIsVar(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(TOlxVars::IsVar(Params[0]));
}
//..............................................................................
void TMainForm::funVVol(const TStrObjList& Params, TMacroError &E)  {
  ElementRadii radii;
  if( Params.Count() == 1 && TEFile::Exists(Params[0]) )
    radii = TXApp::ReadVdWRadii(Params[0]);
  TXApp::PrintVdWRadii(radii, FXApp->XFile().GetAsymmUnit().GetContentList());

  const TXApp::CalcVolumeInfo vi = FXApp->CalcVolume(&radii);
  olxstr report;
  E.SetRetVal(olxstr::FormatFloat(2, vi.total-vi.overlapping));
  TBasicApp::GetLog().Warning("Please note that this is a highly approximate procedure."
  " Volume of current fragment is calculated using a maximum two overlaping spheres," 
  " to calculate packing indexes, use calcvoid instead");
  
  TBasicApp::GetLog() << "Molecular volume (A): " << olxstr::FormatFloat(2, vi.total-vi.overlapping) << '\n';
  TBasicApp::GetLog() << "Overlapping volume (A): " << olxstr::FormatFloat(2, vi.overlapping) << '\n';
}
//..............................................................................
void TMainForm::funSel(const TStrObjList& Params, TMacroError &E)  {
  TSAtomPList atoms;
  TGlGroup& sel = FXApp->GetSelection();
  for( size_t i=0; i < sel.Count(); i++ )  {
    AGDrawObject& gdo = sel[i];
    if( EsdlInstanceOf(gdo, TXAtom) )
      atoms.Add(((TXAtom&)gdo).Atom());
    else if( EsdlInstanceOf(gdo, TXBond) )  {
      atoms.Add(((TXBond&)gdo).Bond().A());
      atoms.Add(((TXBond&)gdo).Bond().B());
    }
    else if( EsdlInstanceOf(gdo, TXPlane) )  {
      TSPlane& sp = ((TXPlane&)gdo).Plane();
      for( size_t j=0; j < sp.Count(); j++ )
        atoms.Add(sp.GetAtom(j));
    }
  }
  olxstr tmp;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    tmp << atoms[i]->GetLabel();
    if( atoms[i]->CAtom().GetResiId() != 0 )  {
      tmp << '_' <<
        atoms[i]->CAtom().GetParent()->GetResidue(atoms[i]->CAtom().GetResiId()).GetNumber();
    }
    if( (i+1) < atoms.Count() )
      tmp << ' ';
  }
  E.SetRetVal( tmp );
}
//..............................................................................
void TMainForm::funAtoms(const TStrObjList& Params, TMacroError &E)
{
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::funEnv(const TStrObjList& Params, TMacroError &E)  {
  TXAtom *XA = FXApp->GetXAtom(Params[0], false);
  if( XA == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom: ") << Params[0];
    return;
  }
  olxstr tmp;
  for( size_t i=0; i < XA->Atom().NodeCount(); i++ )  {
    tmp << XA->Atom().Node(i).GetLabel() << ' ';
  }
  E.SetRetVal( tmp );
}
//..............................................................................
void TMainForm::funFPS(const TStrObjList& Params, TMacroError &E) {
  TimePerFrame = 0;
  for( int i=0; i < 10; i++ )
    TimePerFrame += FXApp->Draw();
  if( TimePerFrame != 0 )
    E.SetRetVal( 10*(1000./TimePerFrame) );
}
//..............................................................................
void TMainForm::funCursor(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )  {
    wxCursor cr(wxCURSOR_ARROW);
    SetCursor( cr );
    FGlCanvas->SetCursor( cr );
    SetStatusText(wxT(""));
  }
  else  {
    if( Params[0].Equalsi("busy") )  {
      wxCursor cr(wxCURSOR_WAIT);
      SetCursor( cr );
      FGlCanvas->SetCursor( cr );
      if( Params.Count() == 2 )
        SetStatusText(Params[1].u_str());
    }
    else if( Params[0].Equalsi("brush") )  {
      wxCursor cr(wxCURSOR_PAINT_BRUSH);
      SetCursor( cr );
      FGlCanvas->SetCursor( cr );
    }
    else if( Params[0].Equalsi("hand") )  {
      wxCursor cr(wxCURSOR_HAND);
      SetCursor( cr );
      FGlCanvas->SetCursor( cr );
    }
    else  {
      if( TEFile::Exists(Params[0]) )  {
        wxImage img;
        img.LoadFile(Params[0].u_str());
        img.SetMaskColour(254, 254, 254);
        img.SetMask(true);
        wxCursor cr(img);
        SetCursor( cr );
        FGlCanvas->SetCursor( cr );
      }
    }
  }
}
//..............................................................................
void TMainForm::funRGB(const TStrObjList& Params, TMacroError &E)  {
  if( Params.Count() == 3 )  {
    E.SetRetVal( (int)RGB(Params[0].ToInt(), Params[1].ToInt(), Params[2].ToInt()) );
    return;
  }
  if( Params.Count() == 4 )  {
    E.SetRetVal( (int)RGBA( Params[0].ToInt(), Params[1].ToInt(), Params[2].ToInt(), Params[3].ToInt()) );
    return;
  }
}
//..............................................................................
void TMainForm::funHtmlPanelWidth(const TStrObjList &Cmds, TMacroError &E)  {
  if( FHtml == NULL || FHtmlMinimized )
    E.SetRetVal( olxstr("-1") );
  else
    E.SetRetVal(GetHtml()->WI.GetWidth());
}
void TMainForm::funColor(const TStrObjList& Params, TMacroError &E)  {
  wxColourDialog CD(this);
  wxColor wc;
  if( Params.Count() == 2 )  {
     wxColor wxdef(Params[1].u_str());
     wc = wxdef;
  }
  CD.GetColourData().SetColour(wc);
  if( CD.ShowModal() == wxID_OK )  {
    wc = CD.GetColourData().GetColour();
    if( Params.IsEmpty() )  {
      E.SetRetVal( (int)RGB(wc.Red(), wc.Green(), wc.Blue()) );
    }
    else if( Params[0].Equalsi("hex") )  {
      char* bf = new char [35];
      sprintf( bf, "#%.2x%.2x%.2x", wc.Red(), wc.Green(), wc.Blue());
      E.SetRetVal(olxcstr::FromExternal(bf, olxstr::o_strlen(bf)));
    }
  }
  else
    E.ProcessingError(__OlxSrcInfo, EmptyString);
}
//..............................................................................
void TMainForm::funZoom(const TStrObjList &Cmds, TMacroError &E)  {
  if( Cmds.IsEmpty() )
    E.SetRetVal(FXApp->GetRender().GetZoom());
  else  {
    double zoom = FXApp->GetRender().GetZoom() + Cmds[0].ToDouble();
    if( zoom < 0.001 )  zoom = 0.001;
    FXApp->GetRender().SetZoom(zoom);
  }
}
#ifdef __WIN32__
//..............................................................................
void TMainForm::funLoadDll(const TStrObjList &Cmds, TMacroError &E)  {
  if( !TEFile::Exists( Cmds[0] ) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate specified file" );
    return;
  }
  HINSTANCE lib_inst = LoadLibrary(Cmds[0].u_str());
  if( !lib_inst )  {
    E.ProcessingError(__OlxSrcInfo, "could not load the library" );
    return;
  }
  FARPROC initPoint = GetProcAddress(lib_inst, "@olex@OlexPort@GetOlexRunnable$qv");
  if( !initPoint )
    initPoint = GetProcAddress(lib_inst, "?GetOlexRunnable@OlexPort@olex@@SGPAVIOlexRunnable@2@XZ");
  if( !initPoint )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate initialisation point" );
    FreeLibrary( lib_inst );
    return;
  }
  typedef olex::IOlexRunnable* (*GetOlexRunnable)();
  olex::IOlexRunnable* runnable = ((GetOlexRunnable)initPoint)();

  if( !runnable )  {
    E.ProcessingError(__OlxSrcInfo, "NULL runnable" );
    FreeLibrary( lib_inst );
    return;
  }
  runnable->Run( *this );
  //FreeLibrary( lib_inst );
}
#endif // __WIN32__
//..............................................................................
void TMainForm::macBasis(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
// the events are handled in void TMainForm::CellVChange()
  FXApp->SetBasisVisible(Cmds.IsEmpty() ? !FXApp->IsBasisVisible() : Cmds[0].ToBool());
}
//..............................................................................
void TMainForm::macLines(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FGlConsole->SetLinesToShow(Cmds[0].ToInt());
}
//..............................................................................
void TMainForm::macPict(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
#ifdef __WIN32__
  bool Emboss = Options.Contains("bw"), 
    EmbossColour = Options.Contains("c"), 
    PictureQuality = Options.Contains("pq");
  if( EmbossColour )  Emboss = true;

  if( PictureQuality )  FXApp->Quality(qaPict);

  short bits = 24, extraBytes;
  // keep old size values
  const int vpLeft = FXApp->GetRender().GetLeft(),
      vpTop = FXApp->GetRender().GetTop(),
      vpWidth = FXApp->GetRender().GetActualWidth(),
      vpHeight = FXApp->GetRender().GetHeight();

  double res = 2;
  if( Cmds.Count() == 2 && Cmds[1].IsNumber() )  
    res = Cmds[1].ToDouble();
  if( res >= 100 )  // width provided
    res /= vpWidth;
  if( res > 10 )
    res = 10;
  if( res <= 0 )  
    res = 1;

  int BmpHeight = vpHeight*res, BmpWidth = vpWidth*res;

  extraBytes = (4-(BmpWidth*3)%4)%4;

  HDC hDC = wglGetCurrentDC();
  HGLRC glc = wglGetCurrentContext();
  HDC dDC = CreateCompatibleDC(NULL);

  BITMAPFILEHEADER BmpFHdr;
  // intialise bitmap header
  BITMAPINFO BmpInfo;
  BmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
  BmpInfo.bmiHeader.biWidth = BmpWidth;
  BmpInfo.bmiHeader.biHeight = BmpHeight;
  BmpInfo.bmiHeader.biPlanes = 1 ;
  BmpInfo.bmiHeader.biBitCount = (WORD) bits;
  BmpInfo.bmiHeader.biCompression = BI_RGB ;
  BmpInfo.bmiHeader.biSizeImage = (BmpWidth*3+extraBytes+1)*BmpHeight; //TODO: +1 check!!!
  BmpInfo.bmiHeader.biXPelsPerMeter = 0 ;
  BmpInfo.bmiHeader.biYPelsPerMeter = 0 ;
  BmpInfo.bmiHeader.biClrUsed = 0;
  BmpInfo.bmiHeader.biClrImportant = 0 ;

  BmpFHdr.bfType = 0x4d42;
  BmpFHdr.bfSize = sizeof(BmpFHdr)  + sizeof(BITMAPINFOHEADER) + (BmpWidth*3+extraBytes)*BmpHeight;
  BmpFHdr.bfReserved1 = BmpFHdr.bfReserved2 = 0;
  BmpFHdr.bfOffBits = sizeof(BmpFHdr) + sizeof(BmpInfo.bmiHeader) ;

  char *DIBits;  DIBits = NULL;
  HBITMAP DIBmp = CreateDIBSection(dDC, &BmpInfo, DIB_RGB_COLORS, (void **)&DIBits, NULL, 0);

  SelectObject(dDC, DIBmp);

  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
    PFD_TYPE_RGBA,
    bits, //bits
    0,0,0,0,0,0,
    0,0,
    0,0,0,0,0,
    32,
    0,
    0,
    PFD_MAIN_PLANE,
    0,
    0,0,
    };
    int PixelFormat = ChoosePixelFormat(dDC, &pfd);
    SetPixelFormat(dDC, PixelFormat, &pfd);
  HGLRC dglc = wglCreateContext(dDC);
  FXApp->GetRender().Resize(0, 0, BmpWidth, BmpHeight, res);
  wglMakeCurrent(dDC, dglc);
  FBitmapDraw = true;
  FGlConsole->Visible(false);
  FXApp->BeginDrawBitmap(res);
  FXApp->GetRender().EnableFog( FXApp->GetRender().IsFogEnabled() );

  FXApp->Draw();
  GdiFlush();
  FBitmapDraw = false;


  wglDeleteContext(dglc);
  DeleteDC(dDC);
  wglMakeCurrent(hDC, glc);
  FXApp->GetRender().Resize(vpLeft, vpTop, vpWidth, vpHeight, 1);
  FGlConsole->Visible(true);
  FXApp->FinishDrawBitmap();
  if( PictureQuality )  FXApp->Quality(qaMedium);
  FXApp->GetRender().EnableFog( FXApp->GetRender().IsFogEnabled() );

  if( Emboss )  {
    if( EmbossColour )  {
      TProcessImage::EmbossC((unsigned char *)DIBits, BmpWidth, BmpHeight, 3,
                       FXApp->GetRender().LightModel.GetClearColor().GetRGB());
    }
    else  {
      TProcessImage::EmbossBW((unsigned char *)DIBits, BmpWidth, BmpHeight, 3,
                       FXApp->GetRender().LightModel.GetClearColor().GetRGB());
    }
  }

  olxstr bmpFN, outFN;

  if( FXApp->XFile().HasLastLoader() && !TEFile::IsAbsolutePath(Cmds[0]) )
    outFN = TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) << TEFile::ExtractFileName( Cmds[0] );
  else
    outFN = Cmds[0];
  // correct a common typo
  if( TEFile::ExtractFileExt(outFN).Equalsi("jpeg") )
    outFN = TEFile::ChangeFileExt(outFN, "jpg");

  if( TEFile::ExtractFileExt(outFN).Equalsi("bmp") )
    bmpFN = TEFile::ChangeFileExt(outFN, "bmp");
  else
    bmpFN = TEFile::ChangeFileExt(outFN, "bmp.tmp");
  TEFile* BmpFile = new TEFile(bmpFN, "w+b");
  BmpFile->Write(&(BmpFHdr), sizeof(BITMAPFILEHEADER));
  BmpFile->Write(&(BmpInfo), sizeof(BITMAPINFOHEADER));
  char *PP = DIBits;
  BmpFile->Write(PP, (BmpWidth*3+extraBytes)*BmpHeight);
  DeleteObject(DIBmp);
  delete BmpFile;
  //check if the image is bmp
  if( TEFile::ExtractFileExt(outFN).Equalsi("bmp") )
    return;
  wxImage image;
  image.LoadFile(bmpFN.u_str(), wxBITMAP_TYPE_BMP);
  if( !image.Ok() )  {
    Error.ProcessingError(__OlxSrcInfo, "could not process image conversion" );
    return;
  }
  image.SetOption(wxT("quality"), 85);
  image.SaveFile(outFN.u_str() );
  image.Destroy();
  TEFile::DelFile(bmpFN);
#else
  macPicta(Cmds, Options, Error);
#endif // __WIN32__
}
//..............................................................................
void TMainForm::macPicta(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  //wxProgressDialog progress(wxT("Rendering..."), wxT("Pass 1 out of 4"), 5, this, wxPD_AUTO_HIDE); 
  int orgHeight = FXApp->GetRender().GetHeight(),
      orgWidth  = FXApp->GetRender().GetWidth();
  double res = 1;
  if( Cmds.Count() == 2 && Cmds[1].IsNumber() )  
    res = Cmds[1].ToDouble();
  if( res >= 100 )  // width provided
    res /= orgWidth;
  if( res > 10 )
    res = 10;
  if( res <= 0 )  
    res = 1;
  if( res > 1 && res < 100 )
    res = olx_round(res);

  int SrcHeight = (int)(((double)orgHeight/(res*2)-1.0)*res*2),
      SrcWidth  = (int)(((double)orgWidth/(res*2)-1.0)*res*2);
  int BmpHeight = (int)(SrcHeight*res), BmpWidth = (int)(SrcWidth*res);
  if( BmpHeight < SrcHeight )
    SrcHeight = BmpHeight;
  if( BmpWidth < SrcWidth )
    SrcWidth = BmpWidth;
  FXApp->GetRender().Resize(0, 0, SrcWidth, SrcHeight, res); 

  const int bmpSize = BmpHeight*BmpWidth*3;
  char* bmpData = (char*)malloc(bmpSize);
  FGlConsole->Visible(false);
  FXApp->GetRender().OnDraw->SetEnabled( false );
  if( res != 1 )    {
    FXApp->GetRender().GetScene().ScaleFonts(res);
    if( res >= 3 )
      FXApp->Quality(qaPict);
  }
  for( int i=0; i < res; i++ )  {
    for( int j=0; j < res; j++ )  {
      FXApp->GetRender().LookAt(j, i, (int)(res < 1 ? 1 : res));
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels(false, 1);
      int mj = j*SrcWidth;
      int mi = i*SrcHeight;
      for( int k=0; k < SrcWidth; k++ )  {
        for( int l=0; l < SrcHeight; l++ )  {
          int indexA = (l*SrcWidth + k)*3;
          int indexB = bmpSize - (BmpWidth*(mi + l + 1) - mj - k)*3;
          bmpData[indexB] = PP[indexA];
          bmpData[indexB+1] = PP[indexA+1];
          bmpData[indexB+2] = PP[indexA+2];
        }
      }
      delete [] PP;
    }
  }
  if( res != 1 ) {
    FXApp->GetRender().GetScene().RestoreFontScale();
    if( res >= 3 ) 
      FXApp->Quality(qaMedium);
  }

  FXApp->GetRender().OnDraw->SetEnabled( true );
  FGlConsole->Visible(true);
  // end drawing etc
  FXApp->GetRender().Resize(orgWidth, orgHeight); 
  FXApp->GetRender().LookAt(0,0,1);
  FXApp->GetRender().SetView(false, 1);
  FXApp->Draw();
  olxstr bmpFN;
  if( FXApp->XFile().HasLastLoader() && !TEFile::IsAbsolutePath(Cmds[0]) )
    bmpFN = TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) << TEFile::ExtractFileName( Cmds[0] );
  else
    bmpFN = Cmds[0];
  wxImage image;
  image.SetData((unsigned char*)bmpData, BmpWidth, BmpHeight ); 
  // correct a common typo
  if( TEFile::ExtractFileExt(bmpFN).Equalsi("jpeg") )
    bmpFN = TEFile::ChangeFileExt(bmpFN, "jpg");
  image.SaveFile(bmpFN.u_str());
}
//..............................................................................
void TMainForm::macPictPS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  OrtDraw od;
  uint16_t color_mode = 0;
  if( Options.Contains("color_fill") )
    color_mode |= ortep_color_fill;
  if( Options.Contains("color_line") )
    color_mode |= ortep_color_lines;
  if( Options.Contains("color_bond") )
    color_mode |= ortep_color_bond;
  od.SetColorMode(color_mode);
  od.SetHBondScale(Options.FindValue("scale_hb", "0.5").ToDouble());
  od.SetPieDiv(Options.FindValue("div_pie", "4").ToInt());
  od.SetFontLineWidth(Options.FindValue("lw_font", "1").ToDouble());
  od.SetPieLineWidth(Options.FindValue("lw_pie", "0.5").ToDouble());
  od.SetElpLineWidth(Options.FindValue("lw_ellipse", "1").ToDouble());
  od.SetQuadLineWidth(Options.FindValue("lw_octant", "0.5").ToDouble());
  od.SetBondOutlineColor(Options.FindValue("bond_outline_color", "0xFFFFFF").SafeInt<uint32_t>());
  od.SetBondOutlineSize(Options.FindValue("bond_outline_oversize", "10").ToFloat<float>()/100.0f);
  od.SetAtomOutlineColor(Options.FindValue("atom_outline_color", "0xFFFFFF").SafeInt<uint32_t>());
  od.SetAtomOutlineSize(Options.FindValue("atom_outline_oversize", "5").ToFloat<float>()/100.0f);
  if( Options.Contains('p') )
    od.SetPerspective(true);
  olxstr octants = Options.FindValue("octants", "-$C");
  // store the atom draw styles
  TIntList ds(FXApp->AtomCount());
  for( size_t i=0; i < FXApp->AtomCount(); i++ )  {
    ds[i] = FXApp->GetAtom(i).DrawStyle();
    if( ds[i] == adsEllipsoid )
      FXApp->GetAtom(i).DrawStyle(adsOrtep);
  }
  TStrList toks(octants, ',');
  for( size_t i=0; i < toks.Count(); i++ )  {
    if( toks[i].Length() < 2 || !(toks[i].CharAt(0) == '-' || toks[i].CharAt(0) == '+') )  continue;
    if( toks[i].CharAt(0) == '-' )  {  // exclude
      if( toks[i].Length() == 2 && toks[i].CharAt(1) == '*' )  { // special case...
        for( size_t j=0; j < FXApp->AtomCount(); j++ )  {
          if( FXApp->GetAtom(j).DrawStyle() == adsOrtep )
            FXApp->GetAtom(j).DrawStyle(adsEllipsoid);
        }
      }
      else if( toks[i].CharAt(1) == '$' )  {  // atom type
        cm_Element* elm = XElementLib::FindBySymbol(toks[i].SubStringFrom(2));
        if( elm == NULL )  continue;
        for( size_t j=0; j < FXApp->AtomCount(); j++ )  {
          if( FXApp->GetAtom(j).Atom().GetType() == *elm && FXApp->GetAtom(j).DrawStyle() == adsOrtep )
            FXApp->GetAtom(j).DrawStyle(adsEllipsoid);
        }
      }
      else  {  // atom name
        olxstr aname = toks[i].SubStringFrom(1);
        for( size_t j=0; j < FXApp->AtomCount(); j++ )  {
          if( FXApp->GetAtom(j).DrawStyle() == adsOrtep && FXApp->GetAtom(j).Atom().GetLabel().Equalsi(aname) )
            FXApp->GetAtom(j).DrawStyle(adsEllipsoid);
        }
      }
    }
    else  {  // include
      if( toks[i].Length() == 2 && toks[i].CharAt(1) == '*' )  { // special case...
        for( size_t j=0; j < FXApp->AtomCount(); j++ )  {
          if( FXApp->GetAtom(j).DrawStyle() == adsEllipsoid )
            FXApp->GetAtom(j).DrawStyle(adsOrtep);
        }
      }
      else if( toks[i].CharAt(1) == '$' )  {  // atom type
        cm_Element* elm = XElementLib::FindBySymbol(toks[i].SubStringFrom(2));
        if( elm == NULL )  continue;
        for( size_t j=0; j < FXApp->AtomCount(); j++ )  {
          if( FXApp->GetAtom(j).Atom().GetType() == *elm && FXApp->GetAtom(j).DrawStyle() == adsEllipsoid )
            FXApp->GetAtom(j).DrawStyle(adsOrtep);
        }
      }
      else  {  // atom name
        olxstr aname = toks[i].SubStringFrom(1);
        for( size_t j=0; j < FXApp->AtomCount(); j++ )  {
          if( FXApp->GetAtom(j).DrawStyle() == adsEllipsoid && FXApp->GetAtom(j).Atom().GetLabel().Equalsi(aname) )
            FXApp->GetAtom(j).DrawStyle(adsOrtep);
        }
      }
    }
  }

  od.Render(Cmds[0]);
  // restore atom draw styles
  for( size_t i=0; i < FXApp->AtomCount(); i++ )
    FXApp->GetAtom(i).DrawStyle(ds[i]);
}
//..............................................................................
void TMainForm::macPictTEX(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  OrtDrawTex od;
  short color_mode = 0;
  if( Options.Contains("color_fill") )
    color_mode = ortep_color_fill;
  else if( Options.Contains("color_line") )
    color_mode = ortep_color_lines;
  od.SetColorMode(color_mode);
  od.Render(Cmds[0]);
}
//..............................................................................
void TMainForm::macBang(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TTTable<TStrList> Table;
  olxstr Tmp = Cmds.Text(' '), clipbrd;
  TXAtomPList Atoms;
  if( !FindXAtoms(Cmds, Atoms, true, true) ) {
    Error.ProcessingError(__OlxSrcInfo, "could not find any atoms" );
    return;
  }
  TStrList Output;
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    FXApp->BangTable(Atoms[i], Table);
    Output.Clear();
    Table.CreateTXTList(Output, EmptyString, true, true, ' ');
    FGlConsole->PrintText( Output, NULL, false );
    clipbrd << Output.Text(NewLineSequence);
  }
  if( wxTheClipboard->Open() )  {
    if (wxTheClipboard->IsSupported(wxDF_TEXT) )
      wxTheClipboard->SetData( new wxTextDataObject(clipbrd.u_str()) );
    wxTheClipboard->Close();
  }
  TBasicApp::GetLog() << "The environment list was placed to theclipboard\n";
}
//..............................................................................
void TMainForm::macGrow(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
  bool GrowShells = Options.Contains('s'),
       GrowContent = Options.Contains('w');
  TCAtomPList TemplAtoms;
  if( Options.Contains('t') )
    FXApp->FindCAtoms(olxstr(Options['t']).Replace(',', ' '), TemplAtoms);
  if( Cmds.IsEmpty() )  {  // grow fragments
    if( GrowContent ) 
      FXApp->GrowWhole(TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
    else  {
      FXApp->GrowFragments(GrowShells, TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
      const TLattice& latt = FXApp->XFile().GetLattice();
      smatd_list gm;
      /* check if next grow will not introduce simple translations */
      bool grow_next = true;
      while( grow_next )  {
        gm.Clear();
        latt.GetGrowMatrices(gm);
        if( gm.IsEmpty() )  break;
        for( size_t i=0; i < latt.MatrixCount(); i++ )  {
          for( size_t j=0; j < gm.Count(); j++ )  {
            if( latt.GetMatrix(i).r == gm[j].r )  {
              vec3d df = latt.GetMatrix(i).t - gm[j].t;
              for( int k=0; k < 3; k++ )
                df[k] -= olx_round(df[k]);
              if( df.QLength() < 1e-6 )  {
                grow_next = false;
                break;
              }
            }
          }
          if( !grow_next )  break;
        }
        if( grow_next )
          FXApp->GrowFragments(GrowShells, TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
      }
    }
  }
  else  {  // grow atoms
    if( GrowContent )
      FXApp->GrowWhole(TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
    else
      FXApp->GrowAtoms(Cmds.Text(' '), GrowShells, TemplAtoms.IsEmpty() ? NULL : &TemplAtoms);
  }
}
//..............................................................................
void TMainForm::macUniq(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, true);
  if( Atoms.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  TNetPList L, L1;
  for( size_t i=0; i < Atoms.Count(); i++ )
    L.Add(Atoms[i]->Atom().GetNetwork());
  FXApp->InvertFragmentsList(L, L1);
  FXApp->FragmentsVisible(L1, false);
  FXApp->CenterView(true);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::macGroup(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr name = Options.FindValue('n');
  if( FXApp->GetSelection().IsEmpty() )
    FXApp->SelectAll(true);
  if( name.IsEmpty() )  {
    name = "group";
    name << (FXApp->GetRender().GroupCount()+1);
  }
  FXApp->GroupSelection(name);
  FXApp->SelectAll(false);
}
//..............................................................................
void TMainForm::macFmol(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->AllVisible(true);
  FXApp->CenterView();
  FXApp->GetRender().GetBasis().SetZoom(FXApp->GetRender().CalcZoom()*FXApp->GetExtraZoom());
}
//..............................................................................
void TMainForm::macClear(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
  FGlConsole->ClearBuffer();  
}
//..............................................................................
void TMainForm::macCell(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
// the events are handled in void TMainForm::CellVChange()
  if( Cmds.IsEmpty() )
    FXApp->SetCellVisible( !FXApp->IsCellVisible() );
  else
    FXApp->SetCellVisible( Cmds[0].ToBool() );
  FXApp->CenterView();
}
//..............................................................................
void TMainForm::macRota(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 2 )  {  // rota x 90 syntax
    double angle = Cmds[1].ToDouble();
    if( Cmds[0] == "1" || Cmds[0] == "x"  || Cmds[0] == "a" )
      FXApp->GetRender().GetBasis().RotateX(FXApp->GetRender().GetBasis().GetRX()+angle);
    else if( Cmds[0] == "2" || Cmds[0] == "y"  || Cmds[0] == "b" )
      FXApp->GetRender().GetBasis().RotateY(FXApp->GetRender().GetBasis().GetRY()+angle);
    else if( Cmds[0] == "3" || Cmds[0] == "z"  || Cmds[0] == "c" )
      FXApp->GetRender().GetBasis().RotateZ(FXApp->GetRender().GetBasis().GetRZ()+angle);
  }
  else if( Cmds.Count() == 5 )  {  // rota x y z 90 1 syntax - rotation around (x,y,z) 90 degrees with 1 degree inc
    FRotationVector[0] = Cmds[0].ToDouble();
    FRotationVector[1] = Cmds[1].ToDouble();
    FRotationVector[2] = Cmds[2].ToDouble();
    FRotationAngle = Cmds[3].ToDouble();
    FRotationIncrement = Cmds[4].ToDouble();
    FMode = FMode | mRota;
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, "wrong parameters" );
  }
}
//..............................................................................
void TMainForm::macListen(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() != 0 )  {
    FMode |= mListen;
    FListenFile = TEFile::OSPath(Cmds.Text(' '));
    TBasicApp::GetLog() << (olxstr("Listening for: ") << FListenFile << '\n');
  }
  else  {
    if( FMode & mListen )
      TBasicApp::GetLog() << (olxstr("Listening for: ") << FListenFile << '\n');
    else
      TBasicApp::GetLog() << ("Not in a listening mode\n");
  }
}
//..............................................................................
#ifdef __WIN32__
void TMainForm::macWindowCmd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  for( size_t i=1; i < Cmds.Count(); i++ )  {
    if( Cmds[i].Equalsi("nl") )    
      TWinWinCmd::SendWindowCmd(Cmds[0], '\r');
    else if( Cmds[i].Equalsi("sp") )
      TWinWinCmd::SendWindowCmd(Cmds[0], ' ');
    else
      TWinWinCmd::SendWindowCmd(Cmds[0], Cmds[i]+' ');
  }
}
#endif
//..............................................................................
void TMainForm::macProcessCmd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( _ProcessManager->GetRedirected() == NULL )  {
    Error.ProcessingError(__OlxSrcInfo, "process does not exist" );
    return;
  }
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].Equalsi("nl") )
      _ProcessManager->GetRedirected()->Writenl();
    else if( Cmds[i].Equalsi("sp") )
      _ProcessManager->GetRedirected()->Write(' ');
    else
      _ProcessManager->GetRedirected()->Write(Cmds[i]+' ');
  }
}
//..............................................................................
void TMainForm::macWait(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olx_sleep(Cmds[0].ToInt());
}
//..............................................................................
void TMainForm::macSwapBg(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->GetRender().Background()->SetVisible(false);  // hide the gradient background
  if( FXApp->GetRender().LightModel.GetClearColor().GetRGB() == 0xffffffff )
    FXApp->GetRender().LightModel.SetClearColor(FBgColor);
  else  {
    FBgColor = FXApp->GetRender().LightModel.GetClearColor();  // update if changed externally...
    FXApp->GetRender().LightModel.SetClearColor(0xffffffff);
  }
  FXApp->GetRender().InitLights();
}
//..............................................................................
void TMainForm::macSilent(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    if( FMode & mSilent )  TBasicApp::GetLog() << ("Silent mode is on\n");
    else                   TBasicApp::GetLog() << ("Silent mode is off\n");
  }
  else if( Cmds[0] == "on" )  {
    FMode |= mSilent;
    TBasicApp::GetLog().Info("Silent mode is on");
  }
  else if( Cmds[0] == "off" )  {
    FMode &= ~mSilent;
    TBasicApp::GetLog().Info("Silent mode is off");
  }
}
//..............................................................................
void TMainForm::macStop(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds[0] == "listen" )  {
    if( FMode & mListen )  {
      FMode ^= mListen;
      TBasicApp::GetLog().Info( "Listen mode is off" );
    }
  }
  else if( Cmds[0] == "logging" )  {
    if( ActiveLogFile != NULL )  {
      delete ActiveLogFile;
      ActiveLogFile = NULL;
    }
  }
}
//..............................................................................
void TMainForm::macEcho(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr m = Options.FindValue('m');
  if( !m.IsEmpty() )  {
    if( m.Equalsi("info") )
      FGlConsole->SetPrintMaterial(&InfoFontColor);
    else if( m.Equalsi("warning") )
      FGlConsole->SetPrintMaterial(&WarningFontColor);
    else if( m.Equalsi("error") )
      FGlConsole->SetPrintMaterial(&ErrorFontColor);
    else if( m.Equalsi("exception") )
      FGlConsole->SetPrintMaterial(&ExceptionFontColor);
  }
  TBasicApp::GetLog() << Cmds.Text(' ') << '\n';
  //FXApp->Draw();
}
//..............................................................................
void TMainForm::macPost(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( FXApp == NULL || FGlConsole == NULL )  return;
  TBasicApp::GetLog() << Cmds.Text(' ');
  FXApp->Draw();
  wxTheApp->Dispatch();
}
//..............................................................................
void TMainForm::macExit(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  Close(true);
}
//..............................................................................
void TMainForm::macPack(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  const bool ClearCont = !Options.Contains("c");
  bool cell = false;
  if( Cmds.Count() > 0 && Cmds[0].Equalsi("cell") )  {
    cell = true;
    Cmds.Delete(0);
  }

  int64_t st = TETime::msNow();
  if( Cmds.IsEmpty() && cell )
    FXApp->XFile().GetLattice().GenerateCell();
  else  {
    vec3d From( -1.0, -1.0, -1.0);
    vec3d To( 1.5, 1.5, 1.5);

    int number_count = 0;
    for( size_t i=0; i < Cmds.Count(); i++ )  {
      if( Cmds[i].IsNumber() )  {
        if( !(number_count%2) )
          From[number_count/2] = Cmds[i].ToDouble();
        else
          To[number_count/2]= Cmds[i].ToDouble();
        number_count++;
        Cmds.Delete(i);
        i--;
      }
    }

    if( number_count != 0 && !(number_count == 6 || number_count == 1 || number_count == 2) )  {
      Error.ProcessingError(__OlxSrcInfo, "please provide 6, 2 or 1 number" );
      return;
    }

    TCAtomPList TemplAtoms;
    if( !Cmds.IsEmpty() )
      FXApp->FindCAtoms(Cmds.Text(' '), TemplAtoms);

    if( number_count == 6 || number_count == 0 || number_count == 2 )  {
      if( number_count == 2 )  {
        From[1] = From[2] = From[0];
        To[1] = To[2] = To[0];
      }
      FXApp->Generate(From, To, TemplAtoms.IsEmpty() ? NULL : &TemplAtoms, ClearCont);
    }
    else  {
      TXAtomPList xatoms;
      FindXAtoms(Cmds, xatoms, true, true);
      vec3d cent;
      double wght = 0;
      for( size_t i=0; i < xatoms.Count(); i++ )  {
        cent += xatoms[i]->Atom().crd()*xatoms[i]->Atom().CAtom().GetOccu();
        wght += xatoms[i]->Atom().CAtom().GetOccu();
      }
      if( wght != 0 )
        cent /= wght;
      FXApp->Generate(cent, From[0], TemplAtoms.IsEmpty() ? NULL : &TemplAtoms, ClearCont);
    }
  }
  if( TBasicApp::GetInstance().IsProfiling() )  {
    TBasicApp::GetLog().Info( olxstr(FXApp->XFile().GetLattice().AtomCount()) << " atoms and " <<
     FXApp->XFile().GetLattice().BondCount() << " bonds generated in " <<
     FXApp->XFile().GetLattice().FragmentCount() << " fragments (" << (TETime::msNow()-st) << "ms)");
  }
  // optimise drawing ...
  //FXApp->GetRender().Compile(true);
}
//..............................................................................
void TMainForm::macName(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool checkLabels = Options.Contains("c");
  bool changeSuffix = Options.Contains("s");
  if( changeSuffix )  {
    TXAtomPList xatoms;
    FXApp->FindXAtoms( Cmds.Text(' '), xatoms, !Options.Contains("cs") );
    if( xatoms.Count() != 0 )
      FUndoStack->Push( FXApp->ChangeSuffix(xatoms, Options.FindValue("s"), checkLabels) );
  }
  else  {
    bool processed = false;
    if( Cmds.Count() == 1 )  { // bug #49
      const size_t spi = Cmds[0].IndexOf(' ');
      if( spi != InvalidIndex )  {
        FUndoStack->Push( FXApp->Name(Cmds[0].SubStringTo(spi), Cmds[0].SubStringFrom(spi+1), checkLabels, !Options.Contains("cs")) );
        processed = true;
      }
    }
    else if( Cmds.Count() == 2 )  {
      FUndoStack->Push( FXApp->Name(Cmds[0], Cmds[1], checkLabels, !Options.Contains("cs")) );
      processed = true;
    }
    if( !processed )  {
      Error.ProcessingError(__OlxSrcInfo, olxstr("invalid syntax: ") << Cmds.Text(' ') );
    }
  }
}
//..............................................................................
void TMainForm::macTelpV(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->CalcProbFactor(Cmds[0].ToDouble());
}
//..............................................................................
void TMainForm::macLabels(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  short lmode = 0;
  if( Options.Contains('p') )   lmode |= lmPart;
  if( Options.Contains('l') )   lmode |= lmLabels;
  if( Options.Contains('v') )   lmode |= lmOVar;
  if( Options.Contains('o') )   lmode |= lmOccp;
  if( Options.Contains("ao") )  lmode |= lmAOcc;
  if( Options.Contains('u') )   lmode |= lmUiso;
  if( Options.Contains('r') )   lmode |= lmUisR;
  if( Options.Contains('a') )   lmode |= lmAfix;
  if( Options.Contains('h') )   lmode |= lmHydr;
  if( Options.Contains('f') )   lmode |= lmFixed;
  if( Options.Contains("qi") )   lmode |= lmQPeakI;
  if( Options.Contains('i') )   lmode |= lmIdentity;
  if( Options.Contains("co") )   lmode |= lmCOccu;
  if( lmode == 0 )  {
    lmode |= lmLabels;
    lmode |= lmQPeak;
    FXApp->SetLabelsMode(lmode);
    FXApp->SetLabelsVisible(!FXApp->AreLabelsVisible());
  }
  else  {
    FXApp->SetLabelsMode(lmode |= lmQPeak );
    FXApp->SetLabelsVisible(true);
  }
  TStateChange sc(prsLabels, FXApp->AreLabelsVisible());
  OnStateChange.Execute((AOlxCtrl*)this, &sc);
}
//..............................................................................
void TMainForm::macCapitalise(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList xatoms;
  const olxstr format = Cmds[0];
  Cmds.Delete(0);
  if( !FindXAtoms(Cmds, xatoms, true, true) )  return;
  for( size_t i=0; i < xatoms.Count(); i++ )
    xatoms[i]->Atom().CAtom().SetTag(0);
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    if( xatoms[i]->Atom().CAtom().GetTag() != 0 )  continue;
    const olxstr& label = xatoms[i]->Atom().CAtom().GetLabel();
    const size_t len = olx_min(Cmds[0].Length(), label.Length());
    olxstr new_label(label);
    for( size_t j=0; j < len; j++ )   {
      if( format[j] >= 'a' && format[j] <= 'z' )
        new_label[j] = olxstr::o_tolower(label.CharAt(j));
      else if( format[j] >= 'A' && format[j] <= 'Z' )
        new_label[j] = olxstr::o_toupper(label.CharAt(j));
    }
    xatoms[i]->Atom().CAtom().SetLabel(new_label, false);
    xatoms[i]->Atom().CAtom().SetTag(1);
  }
}
//..............................................................................
void TMainForm::macSetEnv(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !olx_setenv(Cmds[0], Cmds[1]) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not set the variable" );
    return;
  }
}
//..............................................................................
void TMainForm::macActivate(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXPlane *XP = FXApp->XPlane(Cmds[0]);
  if( XP != NULL )  {
    FXApp->GetRender().GetBasis().OrientNormal(XP->Plane().GetNormal());
    FXApp->SetGridDepth(XP->Plane().GetCenter());
    FXApp->Draw();
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, "could not find specified plane: ") << Cmds[0];
    return;
  }
}
//..............................................................................
void TMainForm::macInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TStrList Output;
  FXApp->InfoList(Cmds.IsEmpty() ? EmptyString : Cmds.Text(' '), Output, Options.Contains("s"));
  TBasicApp::GetLog() << Output;
}
//..............................................................................
void TMainForm::macHelp(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( FHelpItem == NULL )  {  // just print out built in functions if any
    if( Cmds.IsEmpty() )
      return;
    PostCmdHelp(Cmds[0], true);
    return;
  }
  if( Cmds.IsEmpty() )  {
    if( !Options.Count() )  {
      size_t period=6;
      olxstr Tmp;
      for( size_t i=0; i <= FHelpItem->ItemCount(); i+=period )  {
        Tmp = EmptyString;
        for( size_t j=0; j < period; j++ )  {
          if( (i+j) >= FHelpItem->ItemCount() )
            break;
          Tmp << FHelpItem->GetItem(i+j).GetName();
          Tmp.Format((j+1)*10, true, ' ');
        }
        FGlConsole->PrintText(Tmp);
      }
      return;
    }
    else  {
      if( Options.GetName(0)[0] ==  'c' )  {  // show categories
        TStrList Cats;
        TDataItem *Cat;
        for( size_t i=0; i < FHelpItem->ItemCount(); i++ )  {
          Cat = FHelpItem->GetItem(i).FindItemi("category");
          if( Cat == NULL )  continue;
          for( size_t j=0; j < Cat->ItemCount(); j++ )  {
            if( Cats.IndexOf(Cat->GetItem(j).GetName()) == InvalidIndex )
              Cats.Add(Cat->GetItem(j).GetName());
          }
        }
        if( Cats.Count() )
          FGlConsole->PrintText("Macro categories: ");
        else
          FGlConsole->PrintText("No macro categories was found...");
        Cats.QSort(true);
        for( size_t i=0; i < Cats.Count(); i++ )
          FGlConsole->PrintText(Cats[i]);
      }
    }
  }
  else  {
    if( !Options.Count() )
      PostCmdHelp(Cmds[0], true);
    else  {
      if( Options.GetName(0)[0] ==  'c' )  {  // show categories
        FGlConsole->PrintText(olxstr("Macroses for category: ") << Cmds[0]);
        TDataItem *Cat;
        for( size_t i=0; i < FHelpItem->ItemCount(); i++ )  {
          Cat = FHelpItem->GetItem(i).FindItemi("category");
          if( Cat == NULL )  continue;
          for( size_t j=0; j < Cat->ItemCount(); j++ )  {
            if( Cat->GetItem(j).GetName().Equalsi(Cmds[0]) )  {
              FGlConsole->PrintText(FHelpItem->GetItem(i).GetName());
              break;
            }
          }
        }
      }
    }
  }
}
//..............................................................................
void TMainForm::macMatr(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() )  {
    const mat3d& Matr = FXApp->GetRender().GetBasis().GetMatrix();
    for( size_t i=0; i < 3; i++ )  {
      olxstr Tmp;
      Tmp << olxstr::FormatFloat(3, Matr[0][i]);  Tmp.Format(7, true, ' ');
      Tmp << olxstr::FormatFloat(3, Matr[1][i]);  Tmp.Format(14, true, ' ');
      Tmp << olxstr::FormatFloat(3, Matr[2][i]);  Tmp.Format(21, true, ' ');
      TBasicApp::GetLog() << (Tmp << '\n');
    }
  }
  else  {
    if( Cmds.Count() == 1 )  {
      const mat3d& M = FXApp->IsHklVisible() ? FXApp->XFile().GetAsymmUnit().GetHklToCartesian() :
        FXApp->XFile().GetAsymmUnit().GetCellToCartesian();
      olxstr arg;
      if( Cmds[0] == '1' )  arg = "100";
      else if( Cmds[0] == '2' )  arg = "010";
      else if( Cmds[0] == '3' )  arg = "001";
      else
        arg = Cmds[0];
      if( (arg.Length()%3) != 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "invalid argument, an arguments like 010, 001000, +0-1+1 etc is expected");
        return;
      }
      vec3d n;
      const size_t s = arg.Length()/3;
      for( int i=0; i < 3; i++ )
        n += M[i]*arg.SubString(s*i, s).ToInt();
      if( n.QLength() < 1e-3 )  {
        Error.ProcessingError(__OlxSrcInfo, "non zero expression is expected");
        return;
      }
      FXApp->GetRender().GetBasis().OrientNormal(n);
    }
    else if( Cmds.Count() == 2 )  {  // from to view
      if( (Cmds[0].Length()%3) != 0 || (Cmds[1].Length()%3) != 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "invalid arguments, a klm, two arguments like 010, 001000, +0-1+1 etc are expected");
        return;
      }
      const mat3d& M = FXApp->XFile().GetAsymmUnit().GetCellToCartesian();
      vec3d from, to;
      const size_t fs = Cmds[0].Length()/3, ts = Cmds[1].Length()/3;
      for( int i=0; i < 3; i++ )  {
        from += M[i]*Cmds[0].SubString(fs*i, fs).ToInt();
        to += M[i]*Cmds[1].SubString(ts*i, ts).ToInt();
      }
      vec3d n = from-to;
      if( n.QLength() < 1e-3 )  {
        Error.ProcessingError(__OlxSrcInfo, "from and to arguments must be different");
        return;
      }
      FXApp->GetRender().GetBasis().OrientNormal(n);
    }
    else if( Cmds.Count() == 9 )  {
      mat3d M(Cmds[0].ToDouble(), Cmds[1].ToDouble(), Cmds[2].ToDouble(),
      Cmds[3].ToDouble(), Cmds[4].ToDouble(), Cmds[5].ToDouble(),
      Cmds[6].ToDouble(), Cmds[7].ToDouble(), Cmds[8].ToDouble());
      M.Transpose();
      M[0].Normalise();
      M[1].Normalise();
      M[2].Normalise();
      FXApp->GetRender().GetBasis().SetMatrix(M);
    }
    FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macQual(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Options.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong number of arguments" );
    return;
  }
  else  {
     if( Options.GetName(0)[0] == 'h' ) { FXApp->Quality(qaHigh);  return; }
     if( Options.GetName(0)[0] == 'm' ) { FXApp->Quality(qaMedium);  return; }
     if( Options.GetName(0)[0] == 'l' ) { FXApp->Quality(qaLow);  return; }

    Error.ProcessingError(__OlxSrcInfo, "wrong argument" );
    return;
  }
}
//..............................................................................
void TMainForm::macLine(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, true, true);
  olxstr name;
  vec3d from, to;
  if( Atoms.Count() > 2 )  {
    TSAtomPList satoms;
    TListCaster::POP(Atoms, satoms);
    mat3d params;
    vec3d rms, center;
    TSPlane::CalcPlanes(satoms, params, rms, center);
    double maxl = -1000, minl = 1000;
    for( size_t i=0; i < satoms.Count(); i++ )  {
      vec3d v = satoms[i]->crd() - center;
      if( v.QLength() < 0.0001 )
        continue;
      const double ca = params[2].CAngle(v);
      const double l = v.Length()*ca;
      if( l > maxl )
        maxl = l;
      if( l < minl )
        minl = l;
    }
    from = center+params[2]*minl;
    to = center+params[2]*maxl;
  }
  else if( Atoms.Count() == 2 )  {
    from = Atoms[0]->Atom().crd();
    to = Atoms[1]->Atom().crd();
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, "at least two atoms are expected");
    return;
  }
  FXApp->GetRender().GetBasis().OrientNormal(to-from);
  FXApp->AddLine(name, from, to);
  FXApp->Draw();
}
//..............................................................................
void TMainForm::macMpln(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TSPlane* plane = NULL;
  bool orientOnly = Options.Contains("n"),
    rectangular = Options.Contains("r");
  int weightExtent = Options.FindValue("we", "0").ToInt();
  olxstr planeName;
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, true, true);
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    planeName << Atoms[i]->Atom().GetLabel();
    if( i+1 < Atoms.Count() )
      planeName << ' ';
  }

  if( Atoms.Count() < 3 )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong atom count" );
    return;
  }
  if( orientOnly )  {
    plane = FXApp->TmpPlane(&Atoms, weightExtent);
    if( plane != NULL )  {
      FXApp->GetRender().GetBasis().OrientNormal(plane->GetNormal());
      FXApp->SetGridDepth(plane->GetCenter());
      delete plane;
      plane = NULL;
    }
  }
  else  {
    TXPlane* xp = FXApp->AddPlane(Atoms, rectangular, weightExtent);
    if( xp != NULL )
      plane = &xp->Plane();
  }
  if( plane != NULL )  {
    size_t colCount = 3;
    TTTable<TStrList> tab( Atoms.Count()/colCount + (((Atoms.Count()%colCount)==0)?0:1), colCount*2);
    for( size_t i=0; i < colCount; i++ )  {
      tab.ColName(i*2) = "Label";
      tab.ColName(i*2+1) = "D/A";
    }
    for( size_t i=0; i < Atoms.Count(); i+=colCount )  {
      for( size_t j=0; j < colCount; j++ )  {
        if( i + j >= Atoms.Count() )
          break;
        tab[i/colCount][j*2] = Atoms[i+j]->Atom().GetLabel();
        double v = plane->DistanceTo(Atoms[i+j]->Atom());
        tab[i/colCount][j*2+1] = olxstr::FormatFloat(3, v);
      }
    }
    TStrList Output;
    tab.CreateTXTList(Output, olxstr("Atom-to-plane distances for ") << planeName, true, false, "  | ");
    TBasicApp::GetLog() << ( Output );
    TBasicApp::GetLog() << ( olxstr("Plane normal: ") << plane->GetNormal().ToString() << '\n');
  }
}
//..............................................................................
void TMainForm::macCent(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, true, true);
  FXApp->AddCentroid(Atoms);
}
//..............................................................................
void TMainForm::macMask(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error){
  if( Cmds[0].Equalsi("atoms") && Cmds.Count() > 1 )  {
    int Mask = Cmds[1].ToInt();
    short ADS=0, AtomsStart=2;
    olxstr Tmp;
    TXAtomPList Atoms;
    FindXAtoms(Cmds.SubListFrom(AtomsStart), Atoms, false, false);
    FXApp->UpdateAtomPrimitives(Mask, Atoms.IsEmpty() ? NULL : &Atoms);
  }
  else if( (Cmds[0].Equalsi("bonds") || Cmds[0].Equalsi("hbonds") ) && Cmds.Count() > 1 )  {
    int Mask = Cmds[1].ToInt();
    TXBondPList Bonds;
    FXApp->GetBonds(Cmds.Text(' ', 2), Bonds);
    FXApp->UpdateBondPrimitives(Mask, 
      (Bonds.IsEmpty() && FXApp->GetSelection().Count() == 0) ? NULL : &Bonds, 
      Cmds[0].Equalsi("hbonds"));
  }
  else  {
    int Mask = Cmds.Last().String.ToInt();
    Cmds.Delete( Cmds.Count() - 1 );
    TGPCollection *GPC = FXApp->GetRender().FindCollection(Cmds.Text(' '));
    if( GPC != NULL )  {
      if( GPC->ObjectCount() != 0 )
        GPC->GetObject(0).UpdatePrimitives(Mask);
      //TimePerFrame = FXApp->Draw();
    }
    else  {
      Error.ProcessingError(__OlxSrcInfo, olxstr("Indefined graphics :") << Cmds.Text(' ') );
      return;
    }
  }
}
//..............................................................................
void TMainForm::macARad(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr arad(Cmds[0]);
  Cmds.Delete(0);
  TXAtomPList xatoms;
  FindXAtoms(Cmds, xatoms, false, false);
  FXApp->AtomRad(arad, xatoms.IsEmpty() ? NULL : &xatoms);
}
//..............................................................................
void TMainForm::macADS(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList Atoms;
  short ads = -1;
  if( Cmds[0].Equalsi("elp") )
    ads = adsEllipsoid;
  else if( Cmds[0].Equalsi("sph") )
    ads = adsSphere;
  else if( Cmds[0].Equalsi("ort") )
    ads = adsOrtep;
  else if( Cmds[0].Equalsi("std") )
    ads = adsStandalone;
  if( ads == -1 )  {
    Error.ProcessingError(__OlxSrcInfo, "unknown atom type (elp/sph/ort/std) supported only" );
    return;
  }
  Cmds.Delete(0);
  FindXAtoms(Cmds, Atoms, false, false);
  FXApp->SetAtomDrawingStyle(ads, Atoms.IsEmpty() ? NULL : &Atoms);
}
//..............................................................................
void TMainForm::macAZoom(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !Cmds[0].IsNumber() )  {
    Error.ProcessingError(__OlxSrcInfo, "a number is expected as first argument" );
    return;
  }
  double zoom = Cmds[0].ToDouble();
  Cmds.Delete(0);
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, false);
  FXApp->AtomZoom(zoom, Atoms.IsEmpty() ? NULL : &Atoms);
}
//..............................................................................
void TMainForm::macBRad(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXBondPList bonds;
  if( Cmds.Count() == 2 && Cmds[1].Equalsi("hbonds") )  {
    for( size_t i=0; i < FXApp->BondCount(); i++ )  {
      if( FXApp->GetBond(i).Bond().GetType() == sotHBond )
        bonds.Add( &FXApp->GetBond(i) );
    }
    if( !bonds.IsEmpty() )
      FXApp->BondRad(Cmds[0].ToDouble(), &bonds);
  }
  else  {
    FXApp->GetBonds(Cmds.Text(' ', 1), bonds);
    FXApp->BondRad(Cmds[0].ToDouble(), bonds.IsEmpty() ? NULL : &bonds);
  }
}
//..............................................................................
void TMainForm::macKill(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Modes->GetCurrent() != NULL )  {
    Error.ProcessingError(__OlxSrcInfo, "Kill inaccessible from within a mode");
    return;
  }
  if( Cmds.Count() == 1 && Cmds[0].Equalsi("sel") )  {
    TPtrList<AGDrawObject> Objects;
    TGlGroup& sel = FXApp->GetSelection();
    olxstr out;
    for( size_t i=0; i < sel.Count(); i++ )  {
      Objects.Add( sel[i] );
      if( EsdlInstanceOf(sel[i], TXAtom) )
        out << ((TXAtom&)sel[i]).Atom().GetLabel();
      else
        out << sel[i].GetCollectionName();
      out << ' ';
    }
    if( !out.IsEmpty()  )  {
      FXApp->GetLog() << "Deleting " << out << '\n';
      FUndoStack->Push(FXApp->DeleteXObjects(Objects));
      sel.RemoveDeleted();
    }
  }
  else  {
    TXAtomPList Atoms, Selected;
    FXApp->FindXAtoms(Cmds.Text(' '), Atoms, true, Options.Contains('h'));
    if( Atoms.IsEmpty() )  return;
    for( size_t i=0; i < Atoms.Count(); i++ )
      if( Atoms[i]->IsSelected() )
        Selected.Add(Atoms[i]);
    TXAtomPList& todel = Selected.IsEmpty() ? Atoms : Selected;
    if( todel.IsEmpty() )
      return;
    FXApp->GetLog() << "Deleting ";
    for( size_t i=0; i < todel.Count(); i++ )
      FXApp->GetLog() << todel[i]->Atom().GetLabel() << ' ';
    FXApp->GetLog() << '\n';
    FUndoStack->Push(FXApp->DeleteXAtoms(todel));
  }
}
//..............................................................................
void TMainForm::macHide(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 0 || Cmds[0].Equalsi("sel") )  {
    TPtrList<AGDrawObject> Objects;
    TGlGroup& sel = FXApp->GetSelection();
    for( size_t i=0; i < sel.Count(); i++ )  
      Objects.Add( sel[i] );
    FUndoStack->Push( FXApp->SetGraphicsVisible( Objects, false ) );
    sel.Clear();
  }
  else  {
    TXAtomPList Atoms;
    FXApp->FindXAtoms(Cmds.Text(' '), Atoms, true, Options.Contains('h'));
    if( Atoms.IsEmpty() )  return;
    TPtrList<AGDrawObject> go;
    TListCaster::TT(Atoms, go);
    FUndoStack->Push( FXApp->SetGraphicsVisible(go, false) );
  }
}
//..............................................................................
void TMainForm::macExec(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool Asyn = !Options.Contains('s'), // synchroniusly
    Cout = !Options.Contains('o'),    // catch output
    quite = Options.Contains('q');

  olxstr dubFile(Options.FindValue('s',EmptyString));

  olxstr Tmp;
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    bool Space =  (Cmds[i].FirstIndexOf(' ') != InvalidIndex);
    if( Space )  Tmp << '\"';
    Tmp << Cmds[i];
    if( Space ) Tmp << '\"';
    Tmp << ' ';
  }
  TBasicApp::GetLog().Info( olxstr("EXEC: ") << Tmp);
  short flags = 0;
  if( (Cout && Asyn) || Asyn )  {  // the only combination
    if( !Cout )
      flags = quite ? spfQuite : 0;
    else
      flags = quite ? spfRedirected|spfQuite : spfRedirected;
  }
  else
    flags = spfSynchronised;

#ifdef __WIN32__
  TWinProcess* Process  = new TWinProcess(Tmp, flags);
#elif defined(__WXWIDGETS__)
  TWxProcess* Process = new TWxProcess(Tmp, flags);
#endif

  Process->SetOnTerminateCmds(FOnTerminateMacroCmds);
  FOnTerminateMacroCmds.Clear();
  if( (Cout && Asyn) || Asyn )  {  // the only combination
    if( !Cout )  {
      _ProcessManager->OnCreate(*Process);
      if( !Process->Execute() )  {
        _ProcessManager->OnTerminate(*Process);
        Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
        return;
      }
    }
    else  {
      _ProcessManager->OnCreate(*Process);
      if( !dubFile.IsEmpty() )  {
        TEFile* df = new TEFile(dubFile, "wb+");
        Process->SetDubStream(df);
      }
      if( !Process->Execute() )  {
        _ProcessManager->OnTerminate(*Process);
        Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
        return;
      }
    }
  }
  else if( !Process->Execute() )  {
    Error.ProcessingError(__OlxSrcInfo, "failed to launch a new process" );
    delete Process;
  }
}
//..............................................................................
void TMainForm::macShell(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 0 )
    wxShell();
  else  {
#ifdef __WIN32__
    ShellExecute((HWND)this->GetHWND(), wxT("open"), Cmds[0].u_str(), NULL, TEFile::CurrentDir().u_str(), SW_SHOWNORMAL);
#else
    if( Cmds[0].StartsFrom("http") || Cmds[0].StartsFrom("https") || Cmds[0].EndsWith(".htm") || 
        Cmds[0].EndsWith(".html") || Cmds[0].EndsWith(".php") || Cmds[0].EndsWith(".asp") )
    {
      Macros.ProcessMacro( olxstr("exec -o getvar(defbrowser) '") << Cmds[0] << '\'', Error);
    }
# ifdef __linux__
    else if( Cmds[0].EndsWith(".pdf") )  {
      wxString dskpAttr;
      wxGetEnv(wxT("DESKTOP_SESSION"), &dskpAttr);
      if (dskpAttr.Contains(wxT("gnome")))
        Macros.ProcessMacro( olxstr("exec -o gnome-open '") << Cmds[0] << '\'', Error);
      else if (dskpAttr.Contains(wxT("kde")))
        Macros.ProcessMacro( olxstr("exec -o konqueror '") << Cmds[0] << '\'', Error);
      else if (dskpAttr.Contains(wxT("xfce")))
        Macros.ProcessMacro( olxstr("exec -o thunar '") << Cmds[0] << '\'', Error);
      else
        Macros.ProcessMacro( olxstr("exec -o getvar(defbrowser) '") << Cmds[0] << '\'', Error);
    }
# endif
    else
      Macros.ProcessMacro( olxstr("exec -o getvar(defexplorer) '") << Cmds[0] << '\'', Error);
#endif
  }
}
//..............................................................................
void TMainForm::macSave(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr Tmp = Cmds[0];
  Cmds.Delete(0);
  olxstr FN = Cmds.Text(' ');
  if( Tmp == "style" )  {
    if( !FN.Length() )
    {  FN = PickFile("Save drawing style", "Drawing styles|*.glds", StylesDir, false);  }
    if( FN.IsEmpty() )  {
      Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
      return;
    }
    Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( !(StylesDir.LowerCase() == Tmp.LowerCase()) )  {
        TBasicApp::GetLog().Info(olxstr("Styles folder is changed to: ") << Tmp);
        StylesDir = Tmp;
      }
    }
    else  {
      if( !StylesDir.IsEmpty() )  Tmp = StylesDir;
      else                        Tmp = FXApp->GetBaseDir();
      Tmp << FN;  FN = Tmp;
    }
    FN = TEFile::ChangeFileExt(FN, "glds");
    TDataFile F;
    FXApp->GetRender().GetStyles().ToDataItem(F.Root().AddItem("style"));
    try  {  F.SaveToXLFile(FN); }
    catch( TExceptionBase& )  {
      Error.ProcessingError(__OlxSrcInfo, "failed to save file: " ) << FN;
    }
    return;
  }
  if( Tmp == "scene" )  {
    if( FN.IsEmpty() )
      FN = PickFile("Save scene parameters", "Scene parameters|*.glsp", ScenesDir, false);
    if( FN.IsEmpty() )  {
      Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
      return;
    }
    Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( !(ScenesDir.LowerCase() == Tmp.LowerCase()) )  {
        TBasicApp::GetLog().Info(olxstr("Scene parameters folder is changed to: ") << Tmp);
        ScenesDir = Tmp;
      }
    }
    else  {
      Tmp = (ScenesDir.IsEmpty() ? FXApp->GetBaseDir() : ScenesDir);
      Tmp << FN;  FN = Tmp;
    }
    FN = TEFile::ChangeFileExt(FN, "glsp");
    TDataFile F;
    SaveScene(F.Root(), FXApp->GetRender().LightModel);
    try  {  F.SaveToXLFile(FN);  }
    catch( TExceptionBase& )  {
      Error.ProcessingError(__OlxSrcInfo, "failed to save file: ") << FN ;
    }
    return;
  }
  if( Tmp == "view" )  {
    if( FXApp->XFile().HasLastLoader() )  {
      Tmp = (Cmds.Count() == 1) ? TEFile::ChangeFileExt(Cmds[0], "xlv") :
                                  TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "xlv");
      TDataFile DF;
      FXApp->GetRender().GetBasis().ToDataItem(DF.Root().AddItem("basis"));
      DF.SaveToXLFile(Tmp);
    }
  }
  else if( Tmp == "model" )  {
    if( FXApp->XFile().HasLastLoader() )  {
      Tmp = (Cmds.Count() == 1) ? TEFile::ChangeFileExt(Cmds[0], "oxm") :
                                  TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "oxm");
      //TDataFile DF;
      //TDataItem& mi = DF.Root().AddItem("olex_model");
      FXApp->SaveModel(Tmp);
      //DF.SaveToXLFile(Tmp);
    }
  }
}
//..............................................................................
void TMainForm::macLoad(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error) {
  if( Cmds[0].Equalsi("style") )  {
    olxstr FN = Cmds.Text(' ', 1);
    if( FN.IsEmpty() )
      FN = PickFile("Load drawing style", "Drawing styles|*.glds", StylesDir, true);
    if( FN.IsEmpty() )
      return;
    olxstr Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( !StylesDir.Equalsi(Tmp) )  {
        TBasicApp::GetLog().Info(olxstr("Styles folder is changed to: ") + Tmp);
        StylesDir = Tmp;
      }
    }
    else  {
      if( !StylesDir.IsEmpty() )
        Tmp = StylesDir;
      else
        Tmp = FXApp->GetBaseDir();
      Tmp << FN;
      FN = Tmp;
    }
    FN = TEFile::ChangeFileExt(FN, "glds");
    TEFile::CheckFileExists(__OlxSourceInfo, FN);
    TDataFile F;
    F.LoadFromXLFile(FN, NULL);
    FXApp->GetRender().ClearSelection();
    // this forces the object creation, so if there is anything wrong...
    try  {  FXApp->GetRender().GetStyles().FromDataItem(*F.Root().FindItem("style"), false);  }
    catch(...)  {
      FXApp->GetRender().GetStyles().Clear();
      TBasicApp::GetLog().Error("Failed to load given style");
    }
    FXApp->CreateObjects(true, true);
    FXApp->CenterView(true);
    FN = FXApp->GetRender().GetStyles().GetLinkFile();
    if( !FN.IsEmpty() )  {
      if( TEFile::Exists(FN) )  {
        F.LoadFromXLFile(FN, NULL);
        LoadScene(F.Root(), FXApp->GetRender().LightModel);
      }
      else
        TBasicApp::GetLog().Error(olxstr("Load::style: link file does not exist: ") << FN );
    }
  }
  else if( Cmds[0].Equalsi("scene") )  {
    olxstr FN = Cmds.Text(' ', 1);
    if( FN.IsEmpty() )
      FN = PickFile("Load scene parameters", "Scene parameters|*.glsp", ScenesDir, true);
    if( FN.IsEmpty() )
      return;
    olxstr Tmp = TEFile::ExtractFilePath(FN);
    if( !Tmp.IsEmpty() )  {
      if( !ScenesDir.Equalsi(Tmp) )  {
        TBasicApp::GetLog().Info(olxstr("Scene parameters folder is changed to: ") << Tmp);
        ScenesDir = Tmp;
      }
    }
    else  {
      if( !ScenesDir.IsEmpty() )
        Tmp = ScenesDir;
      else
        Tmp = FXApp->GetBaseDir();
      Tmp << FN;
      FN = Tmp;
    }
    FN = TEFile::ChangeFileExt(FN, "glsp");
    TEFile::CheckFileExists(__OlxSourceInfo, FN);
    TDataFile DF;
    DF.LoadFromXLFile(FN, NULL);
    LoadScene(DF.Root(), FXApp->GetRender().LightModel);
  }
  else if( Cmds[0].Equalsi("view") )  {
    olxstr FN = Cmds.Text(' ', 1);
    if( FXApp->XFile().HasLastLoader() )  {
      if( !FN.IsEmpty() )
        FN = TEFile::ChangeFileExt(FN, "xlv");
      else
        FN = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "xlv");
    }
    if( TEFile::Exists(FN) )  {
      TDataFile DF;
      DF.LoadFromXLFile(FN);
      FXApp->GetRender().GetBasis().FromDataItem(DF.Root().FindRequiredItem("basis"));
    }
  }
  else if( Cmds[0].Equalsi("model") )  {
    olxstr FN = Cmds.Text(' ', 1);
    if( FXApp->XFile().HasLastLoader() )  {
      FN = (!FN.IsEmpty() ? TEFile::ChangeFileExt(FN, "oxm") :
                            TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "oxm") );
    }
    if( !FN.IsEmpty() && TEFile::Exists(FN) )  {
      if( !TEFile::IsAbsolutePath(FN) )
        FN = TEFile::AddPathDelimeter(TEFile::CurrentDir()) << FN;        
      FXApp->XFile().LoadFromFile(FN);
    }
  }
  else if ( Cmds[0].Equalsi("radii") )  {
    if( Cmds.Count() > 1  )  {
      olxstr fn = Cmds.Text(' ', 2);
      if( fn.IsEmpty() )
        fn = PickFile("Load atomic radii", "Text files|*.txt", EmptyString, true);
      if( TEFile::Exists(fn) )  {
        olxdict<olxstr,double,olxstrComparator<false> > radii;
        TStrList sl, toks;
        sl.LoadFromFile(fn);
        // parse the file and fill the dictionary
        TBasicApp::GetLog() << "Using user defined radii for: \n";
        for( size_t i=0; i < sl.Count(); i++ )  {
          toks.Clear();
          toks.Strtok(sl[i], ' ');
          if( toks.Count() == 2 )  {
            TBasicApp::GetLog() << ' ' << toks[0] << '\t' << toks[1] << '\n';
            radii(toks[0], toks[1].ToDouble());
          }
        }
        // end the file parsing
        if( Cmds[1].Equalsi("sfil") )  {
          for( size_t i=0; i < radii.Count(); i++ )  {
            cm_Element* cme = XElementLib::FindBySymbol(radii.GetKey(i));
            if( cme != NULL )
              cme->r_sfil = radii.GetValue(i);
          }
        }
        if( Cmds[1].Equalsi("vdw") )  {
          for( size_t i=0; i < radii.Count(); i++ )  {
            cm_Element* cme = XElementLib::FindBySymbol(radii.GetKey(i));
            if( cme != NULL )
              cme->r_vdw = radii.GetValue(i);
          }
        }
        else
          Error.ProcessingError(__OlxSrcInfo, "undefined radii name" );
      }
    }
  }
  else
    Error.ProcessingError(__OlxSrcInfo, "undefined parameter" );
}
//..............................................................................
void TMainForm::macLink(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  olxstr FN, Tmp;
  if( Cmds.IsEmpty() )
    FN = PickFile("Load scene parameters", "Scene parameters|*.glsp", ScenesDir, false);
  else
    FN = Cmds.Text(' ');

  if( FN.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "no file name is given" );
    return;
  }
  if( FN == "none" )  {
    FXApp->GetRender().GetStyles().SetLinkFile(EmptyString);
    TBasicApp::GetLog().Info("The link has been removed...");
    return;
  }
  Tmp = TEFile::ExtractFilePath(FN);
  if( Tmp.IsEmpty() )  {
    if( !ScenesDir.IsEmpty() )
      Tmp = ScenesDir;
    else
      Tmp = FXApp->GetBaseDir();
    FN = (Tmp << FN );
  }
  FN = TEFile::ChangeFileExt(FN, "glsp");
  if( TEFile::Exists(FN) )  
    FXApp->GetRender().GetStyles().SetLinkFile(FN);
  else  {
    Error.ProcessingError(__OlxSrcInfo, "file does not exists : ") << FN;
    return;
  }
}
//..............................................................................
void TMainForm::macStyle(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty() && Options.IsEmpty() )  {
    olxstr tmp = "Default style: ";
    if( !DefStyle.IsEmpty() )
      tmp << DefStyle;
    else
      tmp << "none";
    TBasicApp::GetLog() << (tmp << '\n');
    return;
  }
  else  {
    if( !Cmds.IsEmpty() )  {
      if( Cmds[0] == "none" )  {
        DefStyle = EmptyString;
        TBasicApp::GetLog() << "Default style is reset to none\n";
        return;
      }
      olxstr FN = Cmds.Text(' ');
      olxstr Tmp = TEFile::ExtractFilePath(FN);
      if( Tmp.IsEmpty() )  {
        if( !StylesDir.IsEmpty() )
          Tmp = StylesDir;
        else
          Tmp = FXApp->GetBaseDir();
        FN = (Tmp << FN);
      }
      if( TEFile::Exists(FN) )
        DefStyle = FN;
      else  {
        Error.ProcessingError(__OlxSrcInfo, "specified file does not exists" );
        return;
      }
      return;
    }
    else  {
      olxstr FN = PickFile("Load drawing style", "Drawing styles|*.glds", StylesDir, false);
      if( TEFile::Exists(FN) )
        DefStyle = FN;
    }
  }
}
//..............................................................................
void TMainForm::macScene(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.IsEmpty()  && Options.IsEmpty() )  {
    olxstr tmp = "Default scene: ";
    if( DefSceneP.IsEmpty() )
      tmp << "none";
    else
      tmp << DefSceneP;
    TBasicApp::GetLog() << (tmp << '\n');
    return;
  }
  if( !Cmds.IsEmpty() )  {
    if( Cmds[0] == "none" )  {
      TBasicApp::GetLog() << "Default scene is reset to none\n";
      DefSceneP = EmptyString;
      return;
    }
    olxstr FN = Cmds.Text(' ');
    olxstr Tmp = TEFile::ExtractFilePath(FN);
    if( Tmp.IsEmpty() )  {
      if( !ScenesDir.IsEmpty() )
        Tmp = ScenesDir;
      else
        Tmp = FXApp->GetBaseDir();
      FN = (Tmp << FN);
    }
    if( TEFile::Exists(FN) )
      DefSceneP = FN;
    else  {
      Error.ProcessingError(__OlxSrcInfo, "specified file does not exists" );
      return;
    }
  }
  else  {
    olxstr FN = PickFile("Load scene parameters", "Scene parameters|*.glsp", ScenesDir, false);
    if( TEFile::Exists(FN) )
      DefSceneP = FN;
  }
}
//..............................................................................
void TMainForm::macSyncBC(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FXApp->XAtomDS2XBondDS("Sphere");  
}
//..............................................................................
void TMainForm::macCeiling(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 1 )  {
    if( Cmds[0].Equalsi("on") )
      FXApp->GetRender().Ceiling()->SetVisible(true);
    else if( Cmds[0].Equalsi("off") )
      FXApp->GetRender().Ceiling()->SetVisible(false);
    else
      Error.ProcessingError(__OlxSrcInfo, "wrong arguments" );
    FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macFade(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  FFadeVector[0] = Cmds[0].ToDouble();
  FFadeVector[1] = Cmds[1].ToDouble();
  FFadeVector[2] = Cmds[2].ToDouble();
  if( FFadeVector[1] < 0 || FFadeVector[1] > 1 )  {
    Error.ProcessingError(__OlxSrcInfo, "wrong arguments" );
    return;
  }
  TGlBackground *C = FXApp->GetRender().Ceiling();
  TGlBackground *G = FXApp->GetRender().Background();
  if( !C->IsVisible() )  {
    if( !G->IsVisible() )  {
      C->LT(FXApp->GetRender().LightModel.GetClearColor());
      C->RT(FXApp->GetRender().LightModel.GetClearColor());
      C->LB(FXApp->GetRender().LightModel.GetClearColor());
      C->RB(FXApp->GetRender().LightModel.GetClearColor());
    }
    else  {
      C->LT(G->LT());
      C->RT(G->RT());
      C->LB(G->LB());
      C->RB(G->RB());
    }

    C->SetVisible(true);
  }
  FMode = FMode | mFade;
  return;
}
//..............................................................................
void TMainForm::macWaitFor(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  // we need to call the timer in case it is disabled ...
  if( Cmds[0].Equalsi("fade") )  {
    if( !IsVisible() )  return;
    while( FMode & mFade )  {
      FParent->Dispatch();
      Dispatch(ID_TIMER, -1, (AActionHandler*)this, NULL);
      olx_sleep(50);
    }
  }
  if( Cmds[0].Equalsi("xfader") )  {
    if( !IsVisible() )  return;
    while( FXApp->GetFader().GetPosition() < 1 && FXApp->GetFader().IsVisible() )  {
      FParent->Dispatch();
      Dispatch(ID_TIMER, -1, (AActionHandler*)this, NULL);
      olx_sleep(50);
    }
  }
  else if( Cmds[0].Equalsi("rota") )  {
    while( FMode & mRota )  {
      FParent->Dispatch();
      Dispatch(ID_TIMER, -1, (AActionHandler*)this, NULL);
      olx_sleep(50);
    }
  }
  else if( Cmds[0].Equalsi("process") )  {
    _ProcessManager->WaitForLast();
  }
}
//..............................................................................
void TMainForm::macHtmlPanelSwap(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    FHtmlOnLeft = !FHtmlOnLeft;
    OnResize();
  }
  else  {
    bool changed = false;
    if( Cmds[0].Equalsi("left") )  {
      if( !FHtmlOnLeft )  {
        changed = FHtmlOnLeft = true;
      }
    }
    else if( Cmds[0].Equalsi("right") )  {
      if( FHtmlOnLeft )  {
        changed = true;
        FHtmlOnLeft = false;
      }
    }
    else  {
      E.ProcessingError( __OlxSrcInfo, olxstr("uncknown option '") << Cmds[0] << '\'');
      return;
    }
    if( changed )  OnResize();
  }
}
//..............................................................................
void TMainForm::macHtmlPanelVisible(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    FHtmlMinimized = !FHtmlMinimized;
    OnResize();
    miHtmlPanel->Check(!FHtmlMinimized);
  }
  else if( Cmds.Count() == 1 )  {
    FHtmlMinimized = !Cmds[0].ToBool();
    OnResize();
    miHtmlPanel->Check(!FHtmlMinimized);
  }
  else if( Cmds.Count() == 2 ) {
    bool show = Cmds[0].ToBool();
    TPopupData *pd = GetPopup( Cmds[1] );
    if( pd != NULL )  {
      if( show && !pd->Dialog->IsShown() )  pd->Dialog->Show();
      if( !show && pd->Dialog->IsShown() )  pd->Dialog->Hide();
    }
    else  {
      E.ProcessingError(__OlxSrcInfo, "undefined popup name" );
    }
  }
}
//..............................................................................
void TMainForm::macHtmlPanelWidth(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {
    TBasicApp::GetLog().Info( olxstr("Current HTML panel width: ") << FHtmlPanelWidth);
    return;
  }
  if( Cmds.Count() == 1 )  {
    FHtmlWidthFixed = !Cmds[0].EndsWith('%');
    float width = (FHtmlWidthFixed ? Cmds[0].ToDouble() : Cmds[0].SubStringTo(Cmds[0].Length()-1).ToDouble());
    if( !FHtmlWidthFixed )  {
      width /= 100;
      if( width < 0.01 )
        FHtmlMinimized = true;
      else  {
        if( width > 0.75 )  width = 0.75;
        FHtmlMinimized = false;
        FHtmlPanelWidth = width;
      }
    }
    else  {
      if( width < 10 )
        FHtmlMinimized = true;
      else  {
        int w, h;
        GetClientSize(&w, &h);
        //if( width > w*0.75 )  width = w*0.75;
        FHtmlMinimized = false;
        FHtmlPanelWidth = width;
      }
    }
    OnResize();
    return;
  }
  E.ProcessingError(__OlxSrcInfo, "wrong number of arguments" );
  return;
}
//..............................................................................
void TMainForm::macQPeakScale(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )
    TBasicApp::GetLog() << (olxstr("Current Q-Peak scale: ") << FXApp->GetQPeakScale() << '\n');
  else  {
    float scale = Cmds[0].ToFloat<float>();
    FXApp->SetQPeakScale(scale);
    return;
  }
}
//..............................................................................
void TMainForm::macQPeakSizeScale(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )
    TBasicApp::GetLog() << (olxstr("Current Q-Peak size scale: ") << FXApp->GetQPeakSizeScale() << '\n');
  else  {
    float scale = Cmds[0].ToFloat<float>();
    FXApp->SetQPeakSizeScale(scale);
    return;
  }
}
//..............................................................................
void TMainForm::macLabel(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList atoms;
  FindXAtoms(Cmds, atoms, true, true);
  short lt = 0;
  olxstr str_lt = Options.FindValue("type");
  if( str_lt.Equalsi("brackets") )
    lt = 1;
  else if( str_lt.Equalsi("subscript") )
    lt = 2;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    // 4 - Picture_labels, TODO - avoid naked index reference...
    TXGlLabel* gxl = FXApp->CreateLabel(atoms[i], 4);
    if( lt != 0 && atoms[i]->Atom().GetLabel().Length() > atoms[i]->Atom().GetType().symbol.Length() )  {
      olxstr bcc = atoms[i]->Atom().GetLabel().SubStringFrom(atoms[i]->Atom().GetType().symbol.Length());
      olxstr lb = atoms[i]->Atom().GetType().symbol;
      if( lt == 1 )
        lb << '(' << bcc << ')';
      else if( lt == 2 )
        lb << "\\-" << bcc;
      gxl->SetLabel(lb);
    }
    gxl->SetVisible(true);
  }
}
//..............................................................................
void TMainForm::macFocus(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  this->Raise();
  FGlCanvas->SetFocus();
}
//..............................................................................
void TMainForm::macRefresh(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  Dispatch(ID_TIMER, msiEnter, (AEventsDispatcher*)this, NULL);
  FHtml->Update();
  FXApp->Draw();
  wxTheApp->Dispatch();
}
//..............................................................................
void TMainForm::macMove(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Cmds.IsEmpty() )  {
    FXApp->MoveToCenter();
    return;
  }
  bool clearSelection = !Options.Contains("cs");
  TXAtom *A = FXApp->GetXAtom(Cmds[0], clearSelection);
  TXAtom *B = FXApp->GetXAtom(Cmds[1], clearSelection);
  if( A == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom - ") << Cmds[0];
    return;
  }
  if( !B )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom - " ) << Cmds[1];
    return;
  }
  bool copy = Options.Contains('c');
  FXApp->MoveFragment(A, B, copy);
}
//..............................................................................
void TMainForm::macShowH(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Cmds.Count() == 2 )  {
    bool v = Cmds[1].ToBool();
    if( Cmds[0] == "a" )  {
      if( v && !FXApp->AreHydrogensVisible() )  {
        TStateChange sc(prsHVis, true);
        FXApp->SetHydrogensVisible(true);
        OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      }
      else if( !v && FXApp->AreHydrogensVisible() )  {
        TStateChange sc(prsHVis, false);
        FXApp->SetHydrogensVisible(false);
        OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      }
    }
    else if( Cmds[0] == "b" )  {
      if( v && !FXApp->AreHBondsVisible() )  {
        TStateChange sc(prsHBVis, true);
        FXApp->SetHBondsVisible(true);
        OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      }
      else if( !v && FXApp->AreHBondsVisible() )  {
        TStateChange sc(prsHBVis, false);
        FXApp->SetHBondsVisible(false);
        OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      }
    }
  }
  else  {
    if( FXApp->AreHydrogensVisible() && !FXApp->AreHBondsVisible() )  {
      TStateChange sc(prsHBVis, true);
      FXApp->SetHBondsVisible(true);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
    }
    else if( FXApp->AreHydrogensVisible() && FXApp->AreHBondsVisible() )  {
      TStateChange sc(prsHBVis|prsHVis, false);
      FXApp->SetHBondsVisible(false);
      FXApp->SetHydrogensVisible(false);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
    }
    else if( !FXApp->AreHydrogensVisible() && !FXApp->AreHBondsVisible() )  {
      TStateChange sc(prsHVis, true);
      FXApp->SetHydrogensVisible(true);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
    }
  }
}
//..............................................................................
void TMainForm::macFvar(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  RefinementModel& rm = FXApp->XFile().GetRM();
  TXAtomPList xatoms;
  for( size_t i=0; i < FXApp->GetSelection().Count(); i++ )  {
    if( EsdlInstanceOf(FXApp->GetSelection()[i], TXAtom) )
      xatoms.Add( (TXAtom&)FXApp->GetSelection()[i] );
  }
  if( Cmds.IsEmpty() && xatoms.IsEmpty() )  {
    olxstr tmp = "Free variables: ";
    rm.Vars.Validate();
    TBasicApp::GetLog() << (rm.Vars.GetFVARStr() << '\n');
    return;
  }
  double fvar = -1101;
  XLibMacros::ParseNumbers<double>(Cmds, 1, &fvar);
  if( xatoms.IsEmpty() )
    FindXAtoms(Cmds, xatoms, true, !Options.Contains("cs"));
  if( fvar == 0 )  {
    for( size_t i=0; i < xatoms.Count(); i++ )
      rm.Vars.FreeParam(xatoms[i]->Atom().CAtom(), catom_var_name_Sof);
  }
  else if( xatoms.Count() == 2 && fvar == -1101 )  {
    XVar& xv = rm.Vars.NewVar();
    rm.Vars.AddVarRef(xv, xatoms[0]->Atom().CAtom(), catom_var_name_Sof, relation_AsVar, 1.0);
    rm.Vars.AddVarRef(xv, xatoms[1]->Atom().CAtom(), catom_var_name_Sof, relation_AsOneMinusVar, 1.0);
  }
  else  {
    for( size_t i=0; i < xatoms.Count(); i++ )
      rm.Vars.SetParam(xatoms[i]->Atom().CAtom(), catom_var_name_Sof, fvar);
  }
}
//..............................................................................
void TMainForm::macSump(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  RefinementModel& rm = FXApp->XFile().GetRM();
  TCAtomPList CAtoms;
  FXApp->FindCAtoms(Cmds.Text(' '), CAtoms);
  if( CAtoms.Count() < 2 )  {
    E.ProcessingError(__OlxSrcInfo, "at least two atoms are expected" );
    return;
  }
  double val = 1, esd = 0.01;
  XLibMacros::ParseNumbers<double>(Cmds, 2, &val, &esd);
  XLEQ& xeq = rm.Vars.NewEquation(val, esd);
  for( size_t i=0; i < CAtoms.Count(); i++ )  {
    if( CAtoms[i]->GetVarRef(catom_var_name_Sof) == NULL || 
      CAtoms[i]->GetVarRef(catom_var_name_Sof)->relation_type == relation_None )  
    {
      XVar& xv = rm.Vars.NewVar(1./CAtoms.Count());
      rm.Vars.AddVarRef(xv, *CAtoms[i], catom_var_name_Sof, relation_AsVar, 1.0);
    }
    xeq.AddMember( CAtoms[i]->GetVarRef(catom_var_name_Sof)->Parent );
  }
}
//..............................................................................
void TMainForm::macPart(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  RefinementModel& rm = FXApp->XFile().GetRM();
  int part = DefNoPart;
  size_t partCount = Options.FindValue("p", "1").ToSizeT();
  XLibMacros::ParseNumbers<int>(Cmds, 1, &part);
  bool linkOccu = Options.Contains("lo");

  TXAtomPList Atoms;
  FindXAtoms(Cmds,Atoms, true, !Options.Contains("cs") );
  if( partCount == 0 || (Atoms.Count()%partCount) != 0 )  {
    E.ProcessingError(__OlxSrcInfo, "wrong number of parts" );
    return;
  }
  XVar* xv = NULL;
  XLEQ* leq = NULL;
  if( linkOccu )  {
    // -21 -> 21
    if( partCount == 2 )
      xv = &rm.Vars.NewVar(0.5);
    // SUMP
    if( partCount > 2 )
      leq = &rm.Vars.NewEquation(1.0, 0.01);
  }

  if( part == DefNoPart ) 
    part = FXApp->XFile().GetLattice().GetAsymmUnit().GetNextPart();

  for( size_t i=0; i < partCount; i++ )  {
    for( size_t j=(Atoms.Count()/partCount)*i; j < (Atoms.Count()/partCount)*(i+1); j++ )  {
      Atoms[j]->Atom().CAtom().SetPart(part);
      if( Atoms[j]->Atom().GetType() == iHydrogenZ ) continue;
      for( size_t k=0; k <  Atoms[j]->Atom().NodeCount(); k++ )  {
        TSAtom& SA = Atoms[j]->Atom().Node(k);
        if( SA.GetType() == iHydrogenZ )
          SA.CAtom().SetPart(part);
      }
      if( linkOccu )  {
        if( partCount == 2 )  {
          if( i )  
            rm.Vars.AddVarRef(*xv, Atoms[j]->Atom().CAtom(), catom_var_name_Sof, relation_AsVar, 1.0);
          else     
            rm.Vars.AddVarRef(*xv, Atoms[j]->Atom().CAtom(), catom_var_name_Sof, relation_AsOneMinusVar, 1.0);
        }
        if( partCount > 2 )  {
          if( Atoms[j]->Atom().CAtom().GetVarRef(catom_var_name_Sof) == NULL )  {
            XVar& nv = rm.Vars.NewVar(1./Atoms.Count());
            rm.Vars.AddVarRef(nv, Atoms[j]->Atom().CAtom(), catom_var_name_Sof, relation_AsVar, 1.0);
          }
          leq->AddMember(Atoms[j]->Atom().CAtom().GetVarRef(catom_var_name_Sof)->Parent);
        }
      }
    }
    part++;
  }
  FXApp->XFile().GetLattice().UpdateConnectivity();
}
void TMainForm::macAfix(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  int afix = -1;
  TXAtomPList Atoms;
  XLibMacros::ParseNumbers<int>(Cmds, 1, &afix);
  if( afix == -1 )  {
    E.ProcessingError(__OlxSrcInfo, "afix should be specified" );
    return;
  }
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  RefinementModel& rm = FXApp->XFile().GetRM();
  FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs"));
  int m = TAfixGroup::GetM(afix), n = TAfixGroup::GetN(afix);
  if( TAfixGroup::IsFitted(afix) && ( n == 6 || n == 9) )  {  // special case
    if( Atoms.IsEmpty() )
      FXApp->AutoAfixRings(afix, NULL, Options.Contains('n'));
    else if( Atoms.Count() == 1 )
      FXApp->AutoAfixRings(afix, &Atoms[0]->Atom(), Options.Contains('n'));
    else  {
      if( (afix == 56 || afix == 59) &&  Atoms.Count() != 5 )  {
        E.ProcessingError(__OlxSrcInfo, "please provide 5 atoms exactly" );
        return;
      }
      else if( (afix == 66 || afix == 69 || afix == 76 || afix == 79) &&  Atoms.Count() != 6 )  {
        E.ProcessingError(__OlxSrcInfo, "please provide 6 atoms exactly" );
        return;
      }
      else if( (afix == 106 || afix == 109 || afix == 116 || afix == 119) &&  Atoms.Count() != 10 )  {
        E.ProcessingError(__OlxSrcInfo, "please provide 10 atoms exactly" );
        return;
      }
      if( Atoms[0]->Atom().CAtom().GetDependentAfixGroup() != NULL )
        Atoms[0]->Atom().CAtom().GetDependentAfixGroup()->Clear();
      TAfixGroup& ag = rm.AfixGroups.New(&Atoms[0]->Atom().CAtom(), afix);
      for( size_t i=1; i < Atoms.Count(); i++ )  {
        TCAtom& ca = Atoms[i]->Atom().CAtom();
        if( ca.GetDependentAfixGroup() != NULL && ca.GetDependentAfixGroup()->GetAfix() == afix )  // if used in case to change order
          ca.GetDependentAfixGroup()->Clear();
      }
      for( size_t i=1; i < Atoms.Count(); i++ )
        ag.AddDependent(Atoms[i]->Atom().CAtom());
    }
  }
  else  {
    if( afix == 0 )  {
      for( size_t i=0; i < Atoms.Count(); i++ )  {
        TCAtom& ca = Atoms[i]->Atom().CAtom();
        if( ca.GetAfix() == 1 || ca.GetAfix() == 2 )  {
          if( ca.GetDependentAfixGroup() != NULL )
            ca.GetDependentAfixGroup()->Clear();
          continue;
        }
        if( ca.GetDependentAfixGroup() != NULL )
          ca.GetDependentAfixGroup()->Clear();
        else if( ca.DependentHfixGroupCount() != 0 )  {
          for( size_t j=0; j < ca.DependentHfixGroupCount(); j++ )
            ca.GetDependentHfixGroup(j).Clear();
        }
        else if( ca.GetParentAfixGroup() != NULL )
          ca.GetParentAfixGroup()->Clear();
      }
    }
    else if( afix == 1 || afix == 2 ) {
      for( size_t i=0; i < Atoms.Count(); i++ )  {
        TCAtom& ca = Atoms[i]->Atom().CAtom();
        if( ca.GetAfix() == 0 )
          rm.AfixGroups.New(&ca, afix);
      }
    }
    else if( !Atoms.IsEmpty() )  {
      TAfixGroup& ag = rm.AfixGroups.New(&Atoms[0]->Atom().CAtom(), afix);
      for( size_t i=1; i < Atoms.Count(); i++ )  
        ag.AddDependent(Atoms[i]->Atom().CAtom());
    }
  }
}
//..............................................................................
void TMainForm::macRRings(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TTypeList< TSAtomPList > rings;
  try  {  FXApp->FindRings(Cmds[0], rings);  }
  catch( const TExceptionBase& exc )  {  throw TFunctionFailedException(__OlxSourceInfo, exc);  }

  double l = 1.39, e = 0.001;
  XLibMacros::ParseNumbers<double>(Cmds, 2, &l, &e);

  for( size_t i=0; i < rings.Count(); i++ )  {
    TSimpleRestraint& dfix = FXApp->XFile().GetRM().rDFIX.AddNew();
    dfix.SetValue( l );
    dfix.SetEsd( e );
    TSimpleRestraint& flat = FXApp->XFile().GetRM().rFLAT.AddNew();
    flat.SetEsd( 0.1 );
    for( size_t j=0; j < rings[i].Count(); j++ )  {
      flat.AddAtom( rings[i][j]->CAtom(), &rings[i][j]->GetMatrix(0) );
      if( (j+1) < rings[i].Count() )
        dfix.AddAtomPair( rings[i][j]->CAtom(), &rings[i][j]->GetMatrix(0),
                          rings[i][j+1]->CAtom(), &rings[i][j+1]->GetMatrix(0));
      else
        dfix.AddAtomPair( rings[i][j]->CAtom(), &rings[i][j]->GetMatrix(0),
                          rings[i][0]->CAtom(), &rings[i][0]->GetMatrix(0));
    }
  }
}
//..............................................................................
void ParseResParam(TStrObjList &Cmds, double& esd, double* len = NULL, double* len1 = NULL, double* ang = NULL)  {
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    if( Cmds[i].IsNumber() )  {
      double v = Cmds[i].ToDouble();
      if( olx_abs(v) > 0.25 )  {
        if( olx_abs(v) < 5 )  {
          if( len == NULL )
            throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical arguments, length");
          else  {
            if( *len == 0 )  {
              *len = v;
              if( len1 != NULL )  *len1 = v;
            }
            else if( len1 != NULL )
              *len1 = v;
            else
              throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical arguments, length");
          }
        }
        else if( olx_abs(v) >= 15 && olx_abs(v) <= 180 )  {  // looks line an angle?
          if( ang != NULL )  *ang = v;
          else
            throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical arguments, angle");
        }
        else
          throw TInvalidArgumentException(__OlxSourceInfo, Cmds[i]);
      }
      else // v < 0.25
        esd = v;
      Cmds.Delete(i);
      i--;
    }
  }
}
//..............................................................................
void TMainForm::macDfix(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double fixLen = 0, esd = 0.02;  // length and esd for dfix
  ParseResParam(Cmds, esd, &fixLen);
  if( fixLen == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "please specify the distance to restrain to" );
    return;
  }

  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, false);
  if( Atoms.IsEmpty() )  {
    TGlGroup& sel = FXApp->GetSelection();
    if( sel.Count() > 0 ) {
      TSimpleRestraint *sr = &FXApp->XFile().GetRM().rDFIX.AddNew();
      sr->SetEsd( esd );
      sr->SetValue( fixLen );
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( !EsdlInstanceOf(sel[i], TXBond) )  continue;
        const TSBond& sb = ((TXBond&)sel[i]).Bond();
        sr->AddAtomPair(sb.A().CAtom(), &sb.A().GetMatrix(0),
          sb.B().CAtom(), &sb.B().GetMatrix(0));
      }
      FXApp->XFile().GetRM().rDFIX.ValidateRestraint( *sr );
    }
    else
      E.ProcessingError(__OlxSrcInfo, "no atoms or bonds provided" );
    if( !Options.Contains("cs") )  FXApp->SelectAll(false);
    return;
  }
  if( !Options.Contains("cs") )  FXApp->SelectAll(false);

  TSimpleRestraint* dfix = &FXApp->XFile().GetRM().rDFIX.AddNew();
  dfix->SetValue( fixLen );
  dfix->SetEsd( esd );
  if( Atoms.Count() == 1 )  {  // special case
    TXAtom* XA = Atoms[0];
    for( size_t i=0; i < XA->Atom().NodeCount(); i++ )  {
      TSAtom* SA = &XA->Atom().Node(i);
      if( SA->IsDeleted() )  continue;
      if( SA->GetType() == iQPeakZ )  continue;
      dfix->AddAtomPair(XA->Atom().CAtom(), &XA->Atom().GetMatrix(0), SA->CAtom(), &SA->GetMatrix(0));
    }
  }
  else if( Atoms.Count() == 3 )  {  // special case
    dfix->AddAtomPair(Atoms[0]->Atom().CAtom(), &Atoms[0]->Atom().GetMatrix(0), 
                      Atoms[1]->Atom().CAtom(), &Atoms[1]->Atom().GetMatrix(0));
    dfix->AddAtomPair(Atoms[1]->Atom().CAtom(), &Atoms[1]->Atom().GetMatrix(0), 
                      Atoms[2]->Atom().CAtom(), &Atoms[2]->Atom().GetMatrix(0));
  }
  else  {
    if( (Atoms.Count()%2) != 0 )  {
      E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected" );
      return;
    }
    for( size_t i=0; i < Atoms.Count(); i += 2 )  {
      TXAtom* XA = Atoms[i];
      TXAtom* XA1 = Atoms[i+1];
      dfix->AddAtomPair( XA->Atom().CAtom(), &XA->Atom().GetMatrix(0),
        XA1->Atom().CAtom(), &XA1->Atom().GetMatrix(0));
      if( dfix->AtomCount() >= 12 )  {
        FXApp->XFile().GetRM().rDFIX.ValidateRestraint(*dfix);
        dfix = &FXApp->XFile().GetRM().rDFIX.AddNew();
        dfix->SetValue( fixLen );
        dfix->SetEsd( esd );
      }
    }
  }
  FXApp->XFile().GetRM().rDFIX.ValidateRestraint(*dfix);
}
//..............................................................................
void TMainForm::macDang(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double fixLen = 0, esd = 0.04;  // length and esd for dang
  ParseResParam(Cmds, esd, &fixLen);
  if( fixLen == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "please specify the distance to restrain to" );
    return;
  }

  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs"));
  if( Atoms.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms or bonds provided" );
    return;
  }
  TSimpleRestraint* dang = &FXApp->XFile().GetRM().rDANG.AddNew();
  dang->SetValue( fixLen );
  dang->SetEsd( esd );
  if( (Atoms.Count()%2) != 0 )  {
    E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected" );
    return;
  }
  for( size_t i=0; i < Atoms.Count(); i += 2 )  {
    TXAtom* XA = Atoms[i];
    TXAtom* XA1 = Atoms[i+1];
    dang->AddAtomPair( XA->Atom().CAtom(), &XA->Atom().GetMatrix(0),
      XA1->Atom().CAtom(), &XA1->Atom().GetMatrix(0));
    if( dang->AtomCount() >= 12 )  {
      FXApp->XFile().GetRM().rDANG.ValidateRestraint(*dang);
      dang = &FXApp->XFile().GetRM().rDANG.AddNew();
      dang->SetValue( fixLen );
      dang->SetEsd( esd );
    }
  }
  FXApp->XFile().GetRM().rDANG.ValidateRestraint(*dang);
}
//..............................................................................
void TMainForm::macTria(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double dfixLenA = 0, dfixLenB = 0,
        esd = 0.02,  // esd for dfix, for dang will be doubled
        angle = 0;
  ParseResParam(Cmds, esd, &dfixLenA, &dfixLenB, &angle);
  if( angle == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "please provide the angle to restrain to" );
    return;
  }
  TXAtomPList xatoms;
  TSAtomPList satoms;
  FindXAtoms(Cmds, xatoms, false, false);
  if( xatoms.IsEmpty() )  {
    TGlGroup& sel = FXApp->GetSelection();
    if( sel.Count() > 1 ) {
      for( size_t i=0; i < sel.Count(); i += 2 )  {
        if( !EsdlInstanceOf(sel[i], TXBond) || !EsdlInstanceOf(sel[i+1], TXBond) ) {
          E.ProcessingError(__OlxSrcInfo, "bonds only expected" );
          return;
        }
        const TSBond& sba = ((TXBond&)sel[i]).Bond();
        const TSBond& sbb = ((TXBond&)sel[i+1]).Bond();
        TSAtom* sa = (&sba.A() == &sbb.A() || &sba.B() == &sbb.A() ) ? &sbb.A() : 
                    ((&sba.A() == &sbb.B() || &sba.B() == &sbb.B()) ? &sbb.B() : 
                      NULL );
        if( sa == NULL )  {
          E.ProcessingError(__OlxSrcInfo, "some bonds do not share atom" );
          return;
        }
        satoms.Add( &sba.Another(*sa) );
        satoms.Add( sa );
        satoms.Add( &sbb.Another(*sa) );
      }
    }
    else
      E.ProcessingError(__OlxSrcInfo, "no atoms or bonds provided" );
  }
  else
    TListCaster::POP(xatoms, satoms);

  if( !Options.Contains("cs") )  FXApp->SelectAll(false);

  for( size_t i=0; i < satoms.Count(); i+=3 )  {
    if( dfixLenA == 0 )  {
      dfixLenA = FXApp->XFile().GetRM().FindRestrainedDistance(satoms[i]->CAtom(), satoms[i+1]->CAtom());
      dfixLenB = FXApp->XFile().GetRM().FindRestrainedDistance(satoms[i+1]->CAtom(), satoms[i+2]->CAtom());
      if( dfixLenA == -1 || dfixLenB == -1 )  {
        TBasicApp::GetLog().Error(olxstr("Tria: please fix or provided distance(s)") );
        dfixLenA = dfixLenB = 0;
        continue;
      }
    }
    TSimpleRestraint* dfix = &FXApp->XFile().GetRM().rDFIX.AddNew();
    dfix->SetValue( dfixLenA );
    dfix->SetEsd( esd );
    dfix->AddAtomPair(satoms[i]->CAtom(), &satoms[i]->GetMatrix(0),
                      satoms[i+1]->CAtom(), &satoms[i+1]->GetMatrix(0) );
    if( dfixLenB != dfixLenA )  {
      FXApp->XFile().GetRM().rDFIX.ValidateRestraint(*dfix);
      dfix = &FXApp->XFile().GetRM().rDFIX.AddNew();
    }
    dfix->AddAtomPair(satoms[i+1]->CAtom(), &satoms[i+1]->GetMatrix(0),
                      satoms[i+2]->CAtom(), &satoms[i+2]->GetMatrix(0) );
    FXApp->XFile().GetRM().rDFIX.ValidateRestraint(*dfix);
    

    TSimpleRestraint* dang = &FXApp->XFile().GetRM().rDANG.AddNew();
    dang->SetValue( sqrt(olx_sqr(dfixLenA) + olx_sqr(dfixLenB) - 2*dfixLenA*dfixLenB*cos(angle*M_PI/180)) );
    dang->SetEsd(esd*2);
    dang->AddAtom(satoms[i]->CAtom(), &satoms[i]->GetMatrix(0) );
    //dang->AddAtom(Atoms[i+1]->Atom().CAtom(), &Atoms[i+1]->Atom().GetMatrix(0) );
    dang->AddAtom(satoms[i+2]->CAtom(), &satoms[i+2]->GetMatrix(0) );
    FXApp->XFile().GetRM().rDANG.ValidateRestraint(*dang);
  }
}
//..............................................................................
void TMainForm::macSadi(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double esd = 0.02;  // esd for sadi
  ParseResParam(Cmds, esd);
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, false);
  if( Atoms.IsEmpty() )  {
    TGlGroup& sel = FXApp->GetSelection();
    if( sel.Count() > 1 ) {
      TSimpleRestraint *sr = &FXApp->XFile().GetRM().rSADI.AddNew();
      sr->SetEsd(esd);
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( !EsdlInstanceOf(sel[i], TXBond) )  continue;
        const TSBond& sb = ((TXBond&)sel[i]).Bond();
        sr->AddAtomPair(sb.A().CAtom(), &sb.A().GetMatrix(0),
          sb.B().CAtom(), &sb.B().GetMatrix(0));
      }
      FXApp->XFile().GetRM().rSADI.ValidateRestraint(*sr);
    }
    else
      E.ProcessingError(__OlxSrcInfo, "no atoms or bonds provided" );
    if( !Options.Contains("cs") )  FXApp->SelectAll(false);
    return;
  }
  if( !Options.Contains("cs") )  FXApp->SelectAll(false);

  TSimpleRestraint *sr = &FXApp->XFile().GetRM().rSADI.AddNew();
  sr->SetEsd(esd);

  if( Atoms.Count() == 1 )  {  // special case
    TSimpleRestraint *sr1 = &FXApp->XFile().GetRM().rSADI.AddNew();
    sr1->SetEsd(esd);
    sr->SetEsd(esd*2);
    TXAtom* XA = Atoms[0];
    double td = 0;
    for( size_t i=0; i < XA->Atom().NodeCount(); i++ )  {
      TSAtom& SA = XA->Atom().Node(i);
      if( SA.IsDeleted() )  continue;
      if( SA.GetType() == iQPeakZ )  continue;
      sr1->AddAtomPair(XA->Atom().CAtom(), &XA->Atom().GetMatrix(0), SA.CAtom(), &SA.GetMatrix(0));
      if( td == 0 )  // need this one to remove opposite atoms from restraint
        td = XA->Atom().crd().DistanceTo( SA.crd() ) * 2;
      for( size_t j=i+1; j < XA->Atom().NodeCount(); j++ )  {
        TSAtom& SA1 = XA->Atom().Node(j);
        if( SA1.IsDeleted() )  continue;
        if( SA1.GetType() == iQPeakZ )  continue;
        double d = SA.crd().DistanceTo(SA1.crd()) ;
        if( d/td > 0.85 )  continue;
        sr->AddAtomPair(SA.CAtom(), &SA.GetMatrix(0), SA1.CAtom(), &SA1.GetMatrix(0));
        if( sr->AtomCount() >= 12 )  {
          FXApp->XFile().GetRM().rSADI.ValidateRestraint(*sr);
          sr = &FXApp->XFile().GetRM().rSADI.AddNew();
          sr->SetEsd( esd*2 );
        }
      }
    }
  }
  else if( Atoms.Count() == 3 )  {  // special case
    sr->AddAtomPair( Atoms[0]->Atom().CAtom(), &Atoms[0]->Atom().GetMatrix(0), 
                       Atoms[1]->Atom().CAtom(), &Atoms[1]->Atom().GetMatrix(0));
    sr->AddAtomPair( Atoms[1]->Atom().CAtom(), &Atoms[1]->Atom().GetMatrix(0), 
                       Atoms[2]->Atom().CAtom(), &Atoms[2]->Atom().GetMatrix(0));
  }
  else  {
    if( (Atoms.Count()%2) != 0 )  {
      E.ProcessingError(__OlxSrcInfo, "even number of atoms is expected" );
      return;
    }
    for( size_t i=0; i < Atoms.Count(); i += 2 )  {
      sr->AddAtomPair(Atoms[i]->Atom().CAtom(), &Atoms[i]->Atom().GetMatrix(0),
        Atoms[i+1]->Atom().CAtom(), &Atoms[i+1]->Atom().GetMatrix(0));
    }
  }
  FXApp->XFile().GetRM().rSADI.ValidateRestraint(*sr);
}
//..............................................................................
void TMainForm::macFlat(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double esd = 0.1;  // esd for flat
  ParseResParam(Cmds, esd);
  TXAtomPList Atoms;
  if( !FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs")) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }

  TSimpleRestraint& sr = FXApp->XFile().GetRM().rFLAT.AddNew();
  sr.SetEsd(esd);

  for( size_t i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), &Atoms[i]->Atom().GetMatrix(0));
  FXApp->XFile().GetRM().rFLAT.ValidateRestraint(sr);
}
//..............................................................................
void TMainForm::macSIMU(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  short setCnt = 0;
  double esd1 = 0.04, esd2=0.08, val = 1.7;  // esd
  if(XLibMacros::ParseNumbers<double>(Cmds, 3, &esd1, &esd2, &val) == 1 )
    esd2 = esd1 * 2;
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs"));
  // validate that atoms of the same type
  TSimpleRestraint& sr = FXApp->XFile().GetRM().rSIMU.AddNew();
  sr.SetAllNonHAtoms( Atoms.IsEmpty() );
  sr.SetEsd(esd1);
  sr.SetEsd1(esd2);
  sr.SetValue(val);
  for( size_t i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), NULL);
  FXApp->XFile().GetRM().rSIMU.ValidateRestraint(sr);
}
//..............................................................................
void TMainForm::macDELU(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  short setCnt = 0;
  double esd1 = 0.01, esd2=0.01;  // esd
  if( XLibMacros::ParseNumbers<double>(Cmds, 2, &esd1, &esd2) == 1 )
    esd2 = esd1;
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs"));
  // validate that atoms of the same type
  TSimpleRestraint& sr = FXApp->XFile().GetRM().rDELU.AddNew();
  sr.SetEsd(esd1);
  sr.SetEsd1(esd2);
  sr.SetAllNonHAtoms( Atoms.IsEmpty() );
  for( size_t i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), NULL);
  FXApp->XFile().GetRM().rDELU.ValidateRestraint(sr);
}
//..............................................................................
void TMainForm::macISOR(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  short setCnt = 0;
  double esd1 = 0.1, esd2=0.2;  // esd
  if( XLibMacros::ParseNumbers<double>(Cmds, 2, &esd1, &esd2) == 1 )
    esd2 = 2*esd1;
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs"));
  // validate that atoms of the same type
  TSimpleRestraint& sr = FXApp->XFile().GetRM().rISOR.AddNew();
  sr.SetEsd(esd1);
  sr.SetEsd1(esd2);
  sr.SetAllNonHAtoms( Atoms.IsEmpty() );
  for( size_t i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), NULL);
  FXApp->XFile().GetRM().rISOR.ValidateRestraint(sr);
}
//..............................................................................
void TMainForm::macChiv(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  short setCnt = 0;
  double esd = 0.1, val=0;  // esd
  XLibMacros::ParseNumbers<double>(Cmds, 2, &esd, &val);

  TXAtomPList Atoms;
  if( !FindXAtoms(Cmds, Atoms, false, !Options.Contains("cs")) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }

  TSimpleRestraint& sr = FXApp->XFile().GetRM().rCHIV.AddNew();
  sr.SetValue(val);
  sr.SetEsd(esd);
  for( size_t i=0; i < Atoms.Count(); i++ )
    sr.AddAtom(Atoms[i]->Atom().CAtom(), &Atoms[i]->Atom().GetMatrix(0));
  FXApp->XFile().GetRM().rCHIV.ValidateRestraint(sr);
}
//..............................................................................
int TMainForm_macShowQ_QPeakSortA(const TCAtom* a, const TCAtom* b)  {
  double v = a->GetQPeak() - b->GetQPeak();
  if( v == 0 && a->GetLabel().Length() > 1 && b->GetLabel().Length() > 1 )  {
    if( a->GetLabel().SubStringFrom(1).IsNumber() && b->GetLabel().SubStringFrom(1).IsNumber() )
      v = b->GetLabel().SubStringFrom(1).ToInt() - a->GetLabel().SubStringFrom(1).ToInt();
  }
  return v < 0 ? 1 : (v > 0 ? -1 : 0);
}
int TMainForm_macShowQ_QPeakSortD(const TCAtom* a, const TCAtom* b)  {
  double v = b->GetQPeak() - a->GetQPeak();
  if( v == 0 && a->GetLabel().Length() > 1 && b->GetLabel().Length() > 1 )  {
    if( a->GetLabel().SubStringFrom(1).IsNumber() && b->GetLabel().SubStringFrom(1).IsNumber() )
      v = a->GetLabel().SubStringFrom(1).ToInt() - b->GetLabel().SubStringFrom(1).ToInt();
  }
  return v < 0 ? 1 : (v > 0 ? -1 : 0);
}
void TMainForm::macShowQ(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double wheel = Options.FindValue("wheel", '0').ToDouble();
  if( wheel != 0 )  {
    //if( !FXApp->QPeaksVisible() )  return;
    TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
    TCAtomPList qpeaks;
    for( size_t i=0; i < au.AtomCount(); i++ )
      if( au.GetAtom(i).GetType() == iQPeakZ )
        qpeaks.Add(au.GetAtom(i));
    qpeaks.QuickSorter.SortSF(qpeaks, TMainForm_macShowQ_QPeakSortA);
    index_t d_cnt = 0;
    for( size_t i=0; i < qpeaks.Count(); i++ )
      if( !qpeaks[i]->IsDetached() )
        d_cnt++;
    if( d_cnt == 0 && wheel < 0 )  return;
    if( d_cnt == qpeaks.Count() && wheel > 0 )  return;
    d_cnt += (int)(wheel);
    if( d_cnt < 0 )  d_cnt = 0;
    if( d_cnt > (int)qpeaks.Count() )
      d_cnt = qpeaks.Count();
    for( size_t i=0; i < qpeaks.Count(); i++ )  
      qpeaks[i]->SetDetached(i >= (size_t)d_cnt);
    FXApp->GetSelection().Clear();
    FXApp->XFile().GetLattice().UpdateConnectivity();
    TimePerFrame = FXApp->Draw();
  }
  else if( Cmds.Count() == 2 )  {
    bool v = Cmds[1].ToBool();
    if( Cmds[0] == "a" )  {
      if( v && !FXApp->AreQPeaksVisible() )  {
        TStateChange sc(prsQVis, true);
        FXApp->SetQPeaksVisible(true);
        OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      }
      else if( !v && FXApp->AreQPeaksVisible() )  {
        TStateChange sc(prsQVis, false);
        FXApp->SetQPeaksVisible(false);
        OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      }
    }
    else if( Cmds[0] == "b" )  {
      if( v && !FXApp->AreQPeakBondsVisible() )  {
        TStateChange sc(prsQBVis, true);
        FXApp->SetQPeakBondsVisible( true );
        OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      }
      else if( !v && FXApp->AreQPeakBondsVisible() )  {
        TStateChange sc(prsQBVis, false);
        FXApp->SetQPeakBondsVisible( false );
        OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      }
    }
  }
  else if( Cmds.Count() == 1 && Cmds[0].IsNumber() )  {
    index_t num = Cmds[0].ToInt();
    const bool negative = num < 0;
    if( num < 0 )  num = olx_abs(num);
    TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
    TCAtomPList qpeaks;
    for( size_t i=0; i < au.AtomCount(); i++ )
      if( au.GetAtom(i).GetType() == iQPeakZ )
        qpeaks.Add(au.GetAtom(i));
    qpeaks.QuickSorter.SortSF(qpeaks, negative ? TMainForm_macShowQ_QPeakSortD : TMainForm_macShowQ_QPeakSortA);
    num = olx_min(qpeaks.Count()*num/100, qpeaks.Count());
    for( size_t i=0; i < qpeaks.Count(); i++ )  
      qpeaks[i]->SetDetached( i >= (size_t)num );
    FXApp->GetSelection().Clear();
    FXApp->XFile().GetLattice().UpdateConnectivity();
    TimePerFrame = FXApp->Draw();
  }
  else  {
    if( (!FXApp->AreQPeaksVisible() && !FXApp->AreQPeakBondsVisible()) )  {
      TStateChange sc(prsQVis, true);
      FXApp->SetQPeaksVisible(true);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      return;
    }
    if( FXApp->AreQPeaksVisible() && !FXApp->AreQPeakBondsVisible())  {
      TStateChange sc(prsQBVis, true);
      FXApp->SetQPeakBondsVisible(true);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      return;
    }
    if( FXApp->AreQPeaksVisible() && FXApp->AreQPeakBondsVisible() )  {
      TStateChange sc(prsQBVis|prsQVis, false);
      FXApp->SetQPeaksVisible(false);
      FXApp->SetQPeakBondsVisible(false);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      return;
    }
  }
}
//..............................................................................
void TMainForm::macMode(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  // this variable is set when the mode is changed from within this function
  static bool ChangingMode = false;
  if( ChangingMode )  return;
  AMode* md = Modes->SetMode( Cmds[0] );
  olxstr tmp = EmptyString;
  if( md != NULL )  {
    tmp << Cmds[0];
    Cmds.Delete(0);
    try  {  md->Init(Cmds, Options);  }
    catch(const TExceptionBase& e)  {  
      throw TFunctionFailedException(__OlxSrcInfo, e);  
    }
  }
  if( md != NULL || Cmds[0].Equalsi("off") )  {
    tmp << Cmds.Text(' ');
    for( size_t i=0; i < Options.Count(); i++ )
      tmp << " -" << Options.GetName(i) << '=' << Options.GetValue(i);
    CallbackFunc(OnModeChangeCBName, tmp);
  }
  ChangingMode = false;
}
//..............................................................................
void TMainForm::macText(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr FN = DataDir + "output.txt";
  TUtf8File::WriteLines(FN, FGlConsole->Buffer());
  Macros.ProcessMacro(olxstr("exec getvar('defeditor') -o \"") << FN << '\"' , E);
}
//..............................................................................
void TMainForm::macShowStr(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.IsEmpty() )  {  //S+C -> C -> S -> S+C
    if( FXApp->IsStructureVisible() )  {
      if( FGlConsole->ShowBuffer() )
        FXApp->SetStructureVisible(false);
      else  {
        if( FGlConsole->GetLinesToShow() != InvalidSize )
          FGlConsole->SetLinesToShow(InvalidSize);
        FGlConsole->ShowBuffer(true);
      }
    }
    else {
      if( FGlConsole->ShowBuffer() )  {
        FXApp->SetStructureVisible(true);
        FGlConsole->ShowBuffer(false);
      }
      else
        FGlConsole->ShowBuffer(false);
    }
  }
  else
    FXApp->SetStructureVisible(Cmds[0].ToBool());
  FXApp->CenterView();
  TStateChange sc(prsStrVis, FXApp->IsStructureVisible());
  OnStateChange.Execute((AEventsDispatcher*)this, &sc);
}
//..............................................................................
void TMainForm::macBind(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( Cmds[0].Equalsi("wheel") )  {
    size_t ind = Bindings.IndexOf("wheel");
    if( ind == InvalidIndex )
      Bindings.Add("wheel", Cmds[1]);
    else
      Bindings.GetObject(ind) = Cmds[1];
  }
  else
    E.ProcessingError(__OlxSrcInfo, olxstr("unknown binding parameter: ") << Cmds[0]);
}
//..............................................................................
void TMainForm::macGrad(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  bool invert = Options.Contains('i');
  if( invert )  {
    TStateChange sc(prsGradBG, !FXApp->GetRender().Background()->IsVisible() );
    OnStateChange.Execute( (AEventsDispatcher*)this, &sc );
    FXApp->GetRender().Background()->SetVisible( !FXApp->GetRender().Background()->IsVisible() );
  }
  else if( Cmds.Count() == 1 )  {
    TStateChange sc(prsGradBG, Cmds[0].ToBool() );
    OnStateChange.Execute( (AEventsDispatcher*)this, &sc );
    FXApp->GetRender().Background()->SetVisible( Cmds[0].ToBool() );
  }
  else if( Cmds.IsEmpty() && !Options.Contains('p'))  {
    TdlgGradient *G = new TdlgGradient(this);
    G->ShowModal();
    G->Destroy();
  }
  else if( Cmds.Count() == 4 )  {
    TGlOption v;
    v = Cmds[0].ToInt();  FXApp->GetRender().Background()->RB(v);
    v = Cmds[1].ToInt();  FXApp->GetRender().Background()->LB(v);
    v = Cmds[2].ToInt();  FXApp->GetRender().Background()->RT(v);
    v = Cmds[3].ToInt();  FXApp->GetRender().Background()->LT(v);
  }
  GradientPicture = Options.FindValue("p", GradientPicture);
  if( GradientPicture.IsEmpty() )  {
    TGlTexture* glt = FXApp->GetRender().Background()->GetTexture();
    if( glt != NULL  )
      glt->SetEnabled(false);
  }
  else if( TEFile::Exists(GradientPicture) )  {
    wxFSFile* inf = TFileHandlerManager::GetFSFileHandler( GradientPicture );
    if( inf == NULL )  {
      E.ProcessingError(__OlxSrcInfo, "Image file does not exist: ") << GradientPicture;
      return;
    }
    wxImage img( *inf->GetStream() );
    delete inf;
    if( !img.Ok() )  {
      E.ProcessingError(__OlxSrcInfo, "Invalid image file: ") << GradientPicture;
      return;
    }
    int owidth = img.GetWidth(), oheight = img.GetHeight();
    int l = CalcL( img.GetWidth() );
    int swidth = (int)pow((double)2, (double)l);
    l = CalcL( img.GetHeight() );
    int sheight = (int)pow((double)2, (double)l);

    if( swidth != owidth || sheight != oheight )
      img.Rescale( swidth, sheight );

    int cl = 3, bmpType = GL_RGB;
    if( img.HasAlpha() )  {
      cl ++;
      bmpType = GL_RGBA;
    }

    unsigned char* RGBData = new unsigned char[ swidth * sheight * cl];
    for( int i=0; i < sheight; i++ )  {
      for( int j=0; j < swidth; j++ )  {
        int indexa = (i*swidth + (swidth-j-1)) * cl;
        RGBData[indexa] = img.GetRed(j, i);
        RGBData[indexa+1] = img.GetGreen(j, i);
        RGBData[indexa+2] = img.GetBlue(j, i);
        if( cl == 4 )
          RGBData[indexa+3] = img.GetAlpha(j, i);
      }
    }
    TGlTexture* glt = FXApp->GetRender().Background()->GetTexture();
    if( glt != NULL  )
      FXApp->GetRender().GetTextureManager().Replace2DTexture(*glt, 0, swidth, sheight, 0, bmpType, RGBData);
    else  {
      int glti = FXApp->GetRender().GetTextureManager().Add2DTexture("grad", 0, swidth, sheight, 0, bmpType, RGBData);
      FXApp->GetRender().Background()->SetTexture( FXApp->GetRender().GetTextureManager().FindTexture(glti) );
      glt = FXApp->GetRender().Background()->GetTexture();
      glt->SetEnvMode( tpeDecal );
      glt->SetSCrdWrapping( tpCrdClamp );
      glt->SetTCrdWrapping( tpCrdClamp );

      glt->SetMagFilter( tpFilterNearest );
      glt->SetMinFilter( tpFilterLinear );
      glt->SetEnabled( true );
    }
    delete [] RGBData;
  }
}
//..............................................................................
void TMainForm::macSplit(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  olxstr cr( Options.FindValue("r", EmptyString).ToLowerCase() );
  TCAtomPList Atoms;
  olxstr tmp = Cmds.IsEmpty() ? olxstr("sel") : Cmds.Text(' ');
  FXApp->FindCAtoms(tmp, Atoms, true);
  if( Atoms.IsEmpty() )  return;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  RefinementModel& rm = FXApp->XFile().GetRM();
  TCAtomPList ProcessedAtoms;
  XVar& var = rm.Vars.NewVar();
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    TCAtom* CA = Atoms[i];
    if( CA->GetEllipsoid() == NULL )  continue;
    vec3d direction;
    double Length = 0;
    olxstr lbl = CA->GetLabel();
    if( CA->GetEllipsoid()->GetSX() > CA->GetEllipsoid()->GetSY() )  {
      if( CA->GetEllipsoid()->GetSX() > CA->GetEllipsoid()->GetSZ() )  {
        Length = CA->GetEllipsoid()->GetSX();
        direction = CA->GetEllipsoid()->GetMatrix()[0];
      }
      else  {
        Length = CA->GetEllipsoid()->GetSZ();
        direction = CA->GetEllipsoid()->GetMatrix()[2];
      }
    }
    else  {
      if( CA->GetEllipsoid()->GetSY() > CA->GetEllipsoid()->GetSZ() )  {
        Length = CA->GetEllipsoid()->GetSY();
        direction = CA->GetEllipsoid()->GetMatrix()[1];
      }
      else  {
        Length = CA->GetEllipsoid()->GetSZ();
        direction = CA->GetEllipsoid()->GetMatrix()[2];
      }
    }
    direction *= Length;
    direction /= 2;
    FXApp->XFile().GetAsymmUnit().CartesianToCell( direction );

    TCAtom& CA1 = FXApp->XFile().GetAsymmUnit().NewAtom();
    CA1.Assign(*CA);
    CA1.SetPart(1);
    CA1.ccrd() += direction;
    CA1.SetLabel(FXApp->XFile().GetAsymmUnit().CheckLabel(&CA1, lbl+'a'), false);
    // link occupancies
    rm.Vars.AddVarRef(var, CA1, catom_var_name_Sof, relation_AsVar, 1.0); 
    ProcessedAtoms.Add(CA1);
    TCAtom& CA2 = *CA;
    CA2.SetPart(2);
    CA2.ccrd() -= direction;
    CA2.SetLabel(FXApp->XFile().GetAsymmUnit().CheckLabel(&CA2, lbl+'b'), false);
    // link occupancies
    rm.Vars.AddVarRef(var, CA2, catom_var_name_Sof, relation_AsOneMinusVar, 1.0); 
    ProcessedAtoms.Add(CA2);
    TSimpleRestraint* sr = NULL;
    if( cr.IsEmpty() );
    else if( cr == "eadp" )
      sr = &rm.rEADP.AddNew();
    else if( cr == "isor" )
      sr = &rm.rISOR.AddNew();
    else if( cr == "simu" )
      sr = &rm.rSIMU.AddNew();
    if( sr != NULL )
      sr->AddAtomPair(CA1, NULL, CA2, NULL);
  }
  FXApp->XFile().GetLattice().SetAnis(ProcessedAtoms, false);
}
//..............................................................................
void TMainForm::macShowP(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TIntList parts;
  if( !Cmds.IsEmpty() )  {
    for( size_t i=0; i < Cmds.Count(); i++ )
      parts.Add(Cmds[i].ToInt());
  }
  FXApp->ShowPart(parts, true);
  if( !Options.Contains('m') )
    FXApp->CenterView();
}
//..............................................................................
void TMainForm::macEditAtom(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Modes->GetCurrent() != NULL )  {
    E.ProcessingError(__OlxSourceInfo, "Please exit all modes before running this command");
    return;
  }
  TCAtomPList CAtoms;
  TXAtomPList Atoms;
  TIns* Ins = NULL;
  try {  Ins = &FXApp->XFile().GetLastLoader<TIns>();  }
  catch(...)  {}  // must be native then...
  
  if( !FindXAtoms(Cmds, Atoms, true, !Options.Contains("cs")) )  {
    E.ProcessingError(__OlxSrcInfo, "wrong atom names" );
    return;
  }
  // synchronise atom names etc
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  RefinementModel& rm = FXApp->XFile().GetRM();
  FXApp->XFile().UpdateAsymmUnit();
  if( Ins != NULL )
    Ins->UpdateParams();
  // get CAtoms and EXYZ equivalents
  for( size_t i=0; i < Atoms.Count(); i++ )  {
    TCAtom* ca = &Atoms[i]->Atom().CAtom();
    CAtoms.Add( ca );
    TExyzGroup* eg = ca->GetExyzGroup();
    if( eg != NULL )  {
      for( size_t j=0; j < eg->Count(); j++ )
        if( !(*eg)[j].IsDeleted() )
          CAtoms.Add( &(*eg)[j] );
    }
  }
  TXApp::UnifyPAtomList(CAtoms);
  RefinementModel::ReleasedItems released;
  size_t ac = CAtoms.Count();  // to avoid recursion
  for( size_t i=0; i < ac; i++ )  {
    if( olx_is_valid_index(CAtoms[i]->GetSameId()) )  {
      TSameGroup& sg = FXApp->XFile().GetRM().rSAME[CAtoms[i]->GetSameId()];
      if( released.sameList.IndexOf(sg) != InvalidIndex )  continue;
      released.sameList.Add( &sg );
      for( size_t j=0; j < sg.Count(); j++ )
        CAtoms.Add(sg[j]);
    }
  }
  // proces dependent SAME's
  for( size_t i=0; i < released.sameList.Count(); i++ )  {
    for( size_t j=0; j < released.sameList[i]->DependentCount(); j++ )  {
      TSameGroup& sg = released.sameList[i]->GetDependent(j);
      if( released.sameList.IndexOf(sg) != InvalidIndex )  continue;
      released.sameList.Add( &sg );
      for( size_t k=0; k < sg.Count(); k++ )
        CAtoms.Add( &sg[k] );
    }
  }

  for( size_t i=0; i < rm.AfixGroups.Count(); i++ )
    rm.AfixGroups[i].SetTag(0);
  for( size_t i=0; i < CAtoms.Count(); i++ )  {  // add afixed mates and afix parents
    TCAtom& ca = *CAtoms[i];
    for( size_t j=0; j < ca.DependentHfixGroupCount(); j++ )  {
      TAfixGroup& hg = ca.GetDependentHfixGroup(j);
      if( hg.GetTag() != 0 )  continue;
      for( size_t k=0; k < hg.Count(); k++ ) 
        CAtoms.Add( &hg[k] );
      hg.SetTag(1);
    }
    if( ca.GetDependentAfixGroup() != NULL && ca.GetDependentAfixGroup()->GetTag() == 0)  {
      for( size_t j=0; j < ca.GetDependentAfixGroup()->Count(); j++ ) 
        CAtoms.Add( &(*ca.GetDependentAfixGroup())[j] );
      ca.GetDependentAfixGroup()->SetTag(1);
    }
    if( ca.GetParentAfixGroup() != NULL && ca.GetParentAfixGroup()->GetTag() == 0 )  {
      CAtoms.Add( &ca.GetParentAfixGroup()->GetPivot() );
      for( size_t j=0; j < ca.GetParentAfixGroup()->Count(); j++ ) 
        CAtoms.Add( &(*ca.GetParentAfixGroup())[j] );
      ca.GetParentAfixGroup()->SetTag(1);
    }
  }
  // make sure that the list is unique
  TXApp::UnifyPAtomList(CAtoms);
  FXApp->XFile().GetAsymmUnit().Sort( &CAtoms );
  TStrList SL;
  TStrPObjList<olxstr, TStrList* > RemovedIns;
  SL.Add("REM please do not modify atom names inside the instructions - they will be updated ");
  SL.Add("REM by Olex2 automatically, though you can change any parameters");
  SL.Add("REM Also do not change the atoms order");
  // go through instructions
  if( Ins != NULL )  {
    for( size_t i=0; i < Ins->InsCount(); i++ )  {
      // do not process remarks
      if( !Ins->InsName(i).Equalsi("rem") )  {
        const TInsList* InsParams = &Ins->InsParams(i);
        bool found = false;
        for( size_t j=0; j < InsParams->Count(); j++ )  {
          TCAtom* CA1 = InsParams->GetObject(j);
          if( CA1 == NULL )  continue;
          for( size_t k=0; k < CAtoms.Count(); k++ )  {
            TCAtom* CA = CAtoms[k];
            if( CA->GetLabel().Equalsi(CA1->GetLabel()) )  {
              found = true;  break;
            }
          }
          if( found )  break;
        }
        if( found )  {
          SL.Add(Ins->InsName(i)) << ' ' << InsParams->Text(' ');
          TStrList* InsParamsCopy = new TStrList(*InsParams);
          //InsParamsCopy->Assign(*InsParams);
          RemovedIns.Add(Ins->InsName(i), InsParamsCopy);
          Ins->DelIns(i);
          i--;
        }
      }
    }
    SL.Add(EmptyString);
  }
  TIndexList atomIndex;
  TIns::SaveAtomsToStrings(FXApp->XFile().GetRM(), CAtoms, atomIndex, SL, &released);
  for( size_t i=0; i < released.restraints.Count(); i++ )
    released.restraints[i]->GetParent().Release(*released.restraints[i]);
  for( size_t i=0; i < released.sameList.Count(); i++ )
    released.sameList[i]->GetParent().Release(*released.sameList[i]);
  TdlgEdit *dlg = new TdlgEdit(this, true);
  dlg->SetText( SL.Text('\n') );
  try  {
    if( dlg->ShowModal() == wxID_OK )  {
      SL.Clear();
      FXApp->XFile().GetRM().Vars.Clear();
      SL.Strtok(dlg->GetText(), '\n');
      TStrList NewIns;
      TIns::UpdateAtomsFromStrings(FXApp->XFile().GetRM(), CAtoms, atomIndex, SL, NewIns);
      // add new instructions
      if( Ins != NULL )  {
        for( size_t i=0; i < NewIns.Count(); i++ )  {
          NewIns[i] = NewIns[i].Trim(' ');
          if( NewIns[i].IsEmpty() )  continue;
          Ins->AddIns(NewIns[i], FXApp->XFile().GetRM());
        }
      }
      // emulate loading a new file
      FXApp->XFile().EndUpdate();
    }
    else  {
      if( Ins != NULL )  {
        for( size_t i=0; i < RemovedIns.Count(); i++ )
          Ins->AddIns(RemovedIns[i], *RemovedIns.GetObject(i), FXApp->XFile().GetRM(), false);
      }
      for( size_t i=0; i < released.restraints.Count(); i++ )
        released.restraints[i]->GetParent().Restore(*released.restraints[i]);
      for( size_t i=0; i < released.sameList.Count(); i++ )
        released.sameList[i]->GetParent().Restore(*released.sameList[i]);
      released.restraints.Clear();
      released.sameList.Clear();
    }
  }
  catch(const TExceptionBase& exc )  {
    TBasicApp::GetLog().Exception(exc.GetException()->GetError());
    if( Ins != NULL )  {
      for( size_t i=0; i < RemovedIns.Count(); i++ )
        Ins->AddIns(RemovedIns[i], *RemovedIns.GetObject(i), FXApp->XFile().GetRM(), false);
    }
    for( size_t i=0; i < released.restraints.Count(); i++ )
      released.restraints[i]->GetParent().Restore(*released.restraints[i]);
    for( size_t i=0; i < released.sameList.Count(); i++ )
      released.sameList[i]->GetParent().Restore(*released.sameList[i]);
    released.restraints.Clear();
    released.sameList.Clear();
  }
  for( size_t i=0; i < RemovedIns.Count(); i++ )
    delete (TStrList*)RemovedIns.GetObject(i);
  for( size_t i=0; i < released.restraints.Count(); i++ )  {
    if( released.restraints[i]->GetVarRef(0) != NULL )
      delete released.restraints[i]->GetVarRef(0);
    delete released.restraints[i];
  }
  for( size_t i=0; i < released.sameList.Count(); i++ )  {
    released.sameList[i]->ReleasedClear();
    delete released.sameList[i];
  }
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macEditIns(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TIns& Ins = FXApp->XFile().GetLastLoader<TIns>();
  TStrList SL;
  FXApp->XFile().UpdateAsymmUnit();  // synchronise au's
  Ins.SaveHeader(SL, true);
  SL.Add("HKLF ") << Ins.GetRM().GetHKLFStr();

  TdlgEdit *dlg = new TdlgEdit(this, true);
  dlg->SetText( SL.Text('\n') );
  try  {
    if( dlg->ShowModal() == wxID_OK )  {
      // clear rems, as they are recreated
      for( size_t i=0; i < Ins.InsCount(); i++ )  {
        if( Ins.InsName(i).Equalsi("REM") )  {
          Ins.DelIns(i--);  continue;
        }
      }
      SL.Clear();
      SL.Strtok( dlg->GetText(), '\n' );
      Ins.ParseHeader(SL);
      FXApp->XFile().LastLoaderChanged();
      BadReflectionsTable(false);
    }
    else  {
    }
  }
  catch(const TExceptionBase& exc )  {
    TStrList output;
    exc.GetException()->GetStackTrace(output);
    TBasicApp::GetLog().Exception(output.Text('\n'));
  }
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macMergeHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TRefList refs;
  RefinementModel::HklStat ms =
    FXApp->XFile().GetRM().GetRefinementRefList<TUnitCell::SymSpace,RefMerger::StandardMerger>(
    FXApp->XFile().GetUnitCell().GetSymSpace(), refs);
  TTTable<TStrList> tab(6, 2);
  tab[0][0] << "Total reflections";             tab[0][1] << ms.GetReadReflections();
  tab[1][0] << "Unique reflections";            tab[1][1] << ms.UniqueReflections;
  tab[2][0] << "Inconsistent equaivalents";     tab[2][1] << ms.InconsistentEquivalents;
  tab[3][0] << "Systematic absences removed";   tab[3][1] << ms.SystematicAbsentcesRemoved;
  tab[4][0] << "Rint";                          tab[4][1] << ms.Rint;
  tab[5][0] << "Rsigma";                        tab[5][1] << ms.Rsigma;
  TStrList Output;
  tab.CreateTXTList(Output, olxstr("Merging statistics "), true, false, "  ");
  TBasicApp::GetLog() << Output << '\n';
  olxstr hklFileName = FXApp->XFile().GetRM().GetHKLSource();
  THklFile::SaveToFile( Cmds.IsEmpty() ? hklFileName : Cmds[0], refs);
}
//..............................................................................
void TMainForm::macEditHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr HklFN( FXApp->LocateHklFile() );
  if( !TEFile::Exists(HklFN) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate the HKL file" );
    return;
  }
  smatd_list matrices;
  FXApp->GetSymm(matrices);
  TSpaceGroup& sg = FXApp->XFile().GetLastLoaderSG();
  THklFile Hkl;
  Hkl.LoadFromFile(HklFN);
  TRefPList Hkls;
  TStrList SL;
  SL.Add("REM Please put \'-\' char in the front of reflections you wish to omit");
  SL.Add("REM and remove '-' char if you want the reflection to be used in the refinement");
  SL.Add(EmptyString);
  if( Cmds.Count() != 3 && FXApp->CheckFileType<TIns>() )  {
    const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
    if( Lst.IsLoaded() )  {
      for( size_t i=0; i < Lst.DRefCount(); i++ )  {
        olxstr Tmp = "REM    ";
        Tmp << Lst.DRef(i).H << ' ';
        Tmp << Lst.DRef(i).K << ' ';
        Tmp << Lst.DRef(i).L << ' ';
        Tmp << "Delta(F^2)/esd=" << Lst.DRef(i).DF;
        Tmp << " Resolution=" << Lst.DRef(i).Res;
        SL.Add(Tmp);
        Hkls.Clear();
        TReflection ref(Lst.DRef(i).H, Lst.DRef(i).K, Lst.DRef(i).L);
        Hkl.AllRefs(ref, matrices, Hkls);

        for( size_t j=0; j < Hkls.Count(); j++ )
          SL.Add( Hkls[j]->ToNString());

        SL.Add(EmptyString);
      }
    }
  }
  else  {
    TReflection Refl(Cmds[0].ToInt(), Cmds[1].ToInt(), Cmds[2].ToInt());
    Hkls.Clear();
    Hkl.AllRefs(Refl, matrices, Hkls);
    for( size_t i=0; i < Hkls.Count(); i++ )
      SL.Add( Hkls[i]->ToNString());
  }
  TdlgEdit *dlg = new TdlgEdit(this, true);
  dlg->SetText(SL.Text('\n'));
  if( dlg->ShowModal() == wxID_OK )  {
    olxstr Tmp = dlg->GetText();
    SL.Clear();
    SL.Strtok(Tmp, '\n');
    TReflection R(0, 0, 0);
    for( size_t i=0; i < SL.Count(); i++ )  {
      if( SL[i].ToUpperCase().StartsFrom("REM") )  continue;
      R.FromNString(SL[i]);
      Hkl.UpdateRef(R);
    }
    Hkl.SaveToFile(HklFN);
  }
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macViewHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !Cmds.Count() )
    FXApp->SetHklVisible(!FXApp->IsHklVisible());
  else
    FXApp->SetHklVisible(Cmds[0].ToBool());
}
//..............................................................................
struct Main_3DIndex  {
  short x,y,z;
  Main_3DIndex(short a, short b, short c) : x(a), y(b), z(c)  {}
  Main_3DIndex()  { }
};
typedef TTypeList< Main_3DIndex > T3DIndexList;
bool InvestigateVoid(short x, short y, short z, TArray3D<short>& map, T3DIndexList& points)  {
const index_t mapX = map.Length1(),
              mapY = map.Length2(),
              mapZ = map.Length3();
  short*** D = map.Data;
  const short refVal = D[x][y][z]-1;
  // skip the surface points
  if( refVal < 0 )  return false;
  D[x][y][z] = -D[x][y][z];
  for( short ii = -1; ii <= 1; ii++)  {
    for( short jj = -1; jj <= 1; jj++)  {
      for( short kk = -1; kk <= 1; kk++)  {
        if( (ii|jj|kk) == 0 )  continue;
        int iind = x+ii,
            jind = y+jj,
            kind = z+kk;
        if( iind < 0 )  iind += mapX;
        if( jind < 0 )  jind += mapY;
        if( kind < 0 )  kind += mapZ;
        if( iind >= mapX )  iind -= mapX;
        if( jind >= mapY )  jind -= mapY;
        if( kind >= mapZ )  kind -= mapZ;
        if( D[iind][jind][kind] <= 0 )  {
          continue;
        }
        else if( D[iind][jind][kind] >= refVal )  {
          points.Add( new Main_3DIndex(iind, jind, kind) );
        }
      }
    }
  }
  return true;
}
//
void TMainForm::macCalcVoid(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  ElementRadii radii;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  if( Cmds.Count() == 1 && TEFile::Exists(Cmds[0]) )
    radii = TXApp::ReadVdWRadii(Cmds[0]);
  TXApp::PrintVdWRadii(radii, au.GetContentList());
  TCAtomPList catoms;
  // consider the selection if any
  TGlGroup& sel = FXApp->GetSelection();
  for( size_t i=0; i < sel.Count(); i++ )  {
    if( EsdlInstanceOf(sel[i], TXAtom) ) 
      catoms.Add( ((TXAtom&)sel[i]).Atom().CAtom() )->SetTag(catoms.Count());
  }
  for( size_t i=0; i < catoms.Count(); i++ )
    if( catoms[i]->GetTag() != i )
      catoms[i] = NULL;
  catoms.Pack();
  //
  if( catoms.IsEmpty() )
    TBasicApp::GetLog() << "Calculating for all atoms of the asymmetric unit\n";
  else  {
    olxstr atoms_str("Calculating for");
    for( size_t i=0; i < catoms.Count(); i++ )
      atoms_str << ' ' << catoms[i]->GetLabel();
    FGlConsole->PrintText( atoms_str << " only" );
  }

  double surfdis = Options.FindValue("d", "0").ToDouble();
  TBasicApp::GetLog() << "Extra distance from the surface: " << surfdis << '\n';
  
  double resolution = Options.FindValue("r", "0.2").ToDouble();
  if( resolution < 0.01 )  
    resolution = 0.02;
  resolution = 1./resolution;
  const vec3i dim(
    (int)(au.Axes()[0].GetV()*resolution),
		(int)(au.Axes()[1].GetV()*resolution),
		(int)(au.Axes()[2].GetV()*resolution));
  const double mapVol = dim.Prod();
  const double vol = FXApp->XFile().GetLattice().GetUnitCell().CalcVolume();
  const int minLevel = olx_round(pow(6*mapVol*3/(4*M_PI*vol), 1./3));
  TArray3D<short> map(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  vec3d voidCenter;
  size_t structureGridPoints = 0;
  FXApp->XGrid().Clear();  // release the occupied memory
  map.FastInitWith(10000);
  if( Options.Contains('p') )
    FXApp->XFile().GetUnitCell().BuildDistanceMap_Direct(map, surfdis, -1,
      radii.IsEmpty() ? NULL : &radii, catoms.IsEmpty() ? NULL : &catoms);
  else  {
    FXApp->XFile().GetUnitCell().BuildDistanceMap_Masks(map, surfdis, -1,
      radii.IsEmpty() ? NULL : &radii, catoms.IsEmpty() ? NULL : &catoms);
  }
  TIntList levels;
  short*** amap = map.Data;
  short MaxLevel = 0;
  for( int i=0; i < dim[0]; i++ )  {
    for( int j=0; j < dim[1]; j++ )  {
      for( int k=0; k < dim[2]; k++ )  {
        if( amap[i][j][k] > MaxLevel )
          MaxLevel = amap[i][j][k];
        if( amap[i][j][k] < 0 )
          structureGridPoints++;
        else  {
          while( levels.Count() <= (size_t)amap[i][j][k] )
            levels.Add(0);
          levels[amap[i][j][k]]++;
        }
      }
    }
  }

  const vec3i MaxXCh = MapUtil::AnalyseChannels1(map.Data, dim, MaxLevel);
  for( int i=0; i < 3; i++ )  {
    if( MaxXCh[i] != 0 )
      TBasicApp::GetLog() << (olxstr((olxch)('a'+i)) << " direction can be penetrated by a sphere of " <<
      olxstr::FormatFloat(2, MaxXCh[i]/resolution) << "A radius\n" );
  }
  //short*** map_copy = MapUtil::ReplicateMap(map.Data, mapX, mapY, mapZ);
  //short*** amap = map_copy;
  ////short*** amap = map.Data;
  //for( int i=0; i < mapX; i++ )  {
  //  for( int j=0; j < mapY; j++ )  {
  //    for( int k=0; k < mapZ; k++ )  {
  //      if( amap[i][j][k] < minLevel )
  //        amap[i][j][k] = 0;
  //      else
  //        amap[i][j][k] = -101;
  //    }
  //  }
  //}
  //MapUtil::AnalyseVoidsX<short>(amap, mapX, mapY, mapZ, minLevel+1);
  //size_t _pc = 0;
  //for( int i=0; i < mapX; i++ )  {
  //  for( int j=0; j < mapY; j++ )  {
  //    for( int k=0; k < mapZ; k++ )  {
  //      if( amap[i][j][k] > minLevel )
  //        _pc++;
  //    }
  //  }
  //}
  //MapUtil::DeleteMap(map_copy, mapX, mapY, mapZ);
  TBasicApp::GetLog() << ( olxstr("Cell volume (A^3) ") << olxstr::FormatFloat(3, vol) << '\n');
  //TBasicApp::GetLog() << ( olxstr("Voids volume (A^3) ") << olxstr::FormatFloat(3, (mapVol-_pc)*vol/mapVol) << '\n');
  //TBasicApp::GetLog() << ( olxstr("Max level reached ") << MaxLevel << '\n');
  //TBasicApp::GetLog() << ( olxstr("  at (") << olxstr::FormatFloat(2, voidCenter[0]) << ", "  <<
  //  olxstr::FormatFloat(2, voidCenter[1]) << ", "  <<
  //  olxstr::FormatFloat(2, voidCenter[2]) << ")\n");
  TBasicApp::GetLog() << ( olxstr("Radius of the largest spherical void is (A) ") <<
    olxstr::FormatFloat(2, (double)MaxLevel/resolution) << '\n');
  TBasicApp::GetLog() << ( olxstr(catoms.IsEmpty() ? "Structure occupies" : "Selected atoms occupy") << " (A^3) "
    << olxstr::FormatFloat(2, structureGridPoints*vol/mapVol) 
    << " (" << olxstr::FormatFloat(2, structureGridPoints*100/mapVol) << "%)\n");

  double totalVol = 0;
  for( size_t i=levels.Count()-1; olx_is_valid_index(i); i-- )  {
    totalVol += levels[i];
    TBasicApp::GetLog() << ( olxstr("Level ") << i << " is " <<
      olxstr::FormatFloat(1, (double)i/resolution) << "A away from the surface " <<
      olxstr::FormatFloat(3, totalVol*vol/mapVol) << "(A^3)\n" );
  }
  //// set map to view voids
  FXApp->XGrid().InitGrid(dim[0], dim[1], dim[2]);
  FXApp->XGrid().SetMinVal(0);
  FXApp->XGrid().SetMaxVal((float)MaxLevel/resolution);
  for( int i=0; i < dim[0]; i++ )  {
    for( int j=0; j < dim[1]; j++ )  {
      for( int k=0; k < dim[2]; k++ )
        FXApp->XGrid().SetValue(i, j, k, map.Data[i][j][k]*10/resolution);
    }
  }
  FXApp->XGrid().AdjustMap();
  //FXApp->XGrid().InitIso();
  FXApp->ShowGrid(true, EmptyString);
  TBasicApp::GetLog() << '\n';
}
//..............................................................................
void TMainForm::macViewGrid(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr gname;
  if( FXApp->XFile().HasLastLoader() && Cmds.IsEmpty() )  {
    gname = TEFile::ExtractFilePath(FXApp->XFile().GetFileName());
    TEFile::AddPathDelimeterI(gname) << "map.txt";
  }
  else if( !Cmds.IsEmpty() )
    gname = Cmds[0];
  FXApp->ShowGrid(true, gname);
}
//..............................................................................
void TMainForm::macExtractHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  
  throw TNotImplementedException(__OlxSourceInfo);
  
  TGlGroup& sel = FXApp->GetSelection();
  if( sel.Count() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "please select some reflections" );
    return;
  }
  TRefPList Refs;
  for( size_t i=0; i < sel.Count(); i++ )  {
    AGDrawObject& obj = sel[i];
    if( EsdlInstanceOf(obj, TXReflection) )
      ;//Refs.Add( ((TXReflection*)obj)->Reflection() );
  }
  if( Refs.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "please select some reflections" );
    return;
  }
  THklFile::SaveToFile(Cmds[0], Refs, true);
}
//..............................................................................
void TMainForm::macAppendHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TIntList h, k, l;
  bool combine = Options.FindValue("c", TrueString).ToBool();
  TStrList toks( Options.FindValue('h', EmptyString), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    h.Add( toks[i].ToInt() );
  toks.Clear();
  toks.Strtok( Options.FindValue('k', EmptyString), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    k.Add( toks[i].ToInt() );
  toks.Clear();
  toks.Strtok( Options.FindValue('l', EmptyString), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    l.Add( toks[i].ToInt() );

  olxstr hklSrc( FXApp->LocateHklFile() );
  if( !TEFile::Exists( hklSrc ) )  {
    E.ProcessingError(__OlxSrcInfo, "could not find hkl file: ") << hklSrc;
    return;
  }
  THklFile Hkl;
  int c = 0;
  Hkl.LoadFromFile( hklSrc );
  if( Options.IsEmpty() )  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].GetTag() < 0 )  {
        Hkl[i].SetTag( -Hkl[i].GetTag() );
        c++;
      }
    }
  }
  else if( combine )  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].GetTag() < 0 )  {
        if( !h.IsEmpty() && h.IndexOf(Hkl[i].GetH()) == InvalidIndex ) continue;
        if( !k.IsEmpty() && k.IndexOf(Hkl[i].GetK()) == InvalidIndex ) continue;
        if( !l.IsEmpty() && l.IndexOf(Hkl[i].GetL()) == InvalidIndex ) continue;
        Hkl[i].SetTag( -Hkl[i].GetTag() );
        c++;
      }
    }
  }
  else  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].GetTag() < 0 )  {
        if( h.IndexOf(Hkl[i].GetH()) != InvalidIndex ||
            k.IndexOf(Hkl[i].GetK()) != InvalidIndex ||
            l.IndexOf(Hkl[i].GetL()) != InvalidIndex )  {
          Hkl[i].SetTag( -Hkl[i].GetTag() );
          c++;
        }
      }
    }
  }
  Hkl.SaveToFile( hklSrc );
  FXApp->GetLog() << c << " reflections appended\n";
}
//..............................................................................
void TMainForm::macExcludeHkl(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TIntList h, k, l;
  bool combine = Options.FindValue("c", TrueString).ToBool();
  TStrList toks( Options.FindValue('h', EmptyString), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    h.Add( toks[i].ToInt() );
  toks.Clear();
  toks.Strtok( Options.FindValue('k', EmptyString), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    k.Add( toks[i].ToInt() );
  toks.Clear();
  toks.Strtok( Options.FindValue('l', EmptyString), ';');
  for( size_t i=0; i < toks.Count(); i++ )
    l.Add( toks[i].ToInt() );

  olxstr hklSrc( FXApp->LocateHklFile() );
  if( !TEFile::Exists( hklSrc ) )  {
    E.ProcessingError(__OlxSrcInfo, "could not find hkl file: ") << hklSrc;
    return;
  }
  if( h.IsEmpty() && k.IsEmpty() && l.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "please provide a condition");
    return;
  }

  THklFile Hkl;
  int c = 0;
  Hkl.LoadFromFile( hklSrc );
  if( combine )  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].GetTag() > 0 )  {
        if( !h.IsEmpty() && h.IndexOf(Hkl[i].GetH()) == InvalidIndex) continue;
        if( !k.IsEmpty() && k.IndexOf(Hkl[i].GetK()) == InvalidIndex) continue;
        if( !l.IsEmpty() && l.IndexOf(Hkl[i].GetL()) == InvalidIndex) continue;
        Hkl[i].SetTag( -Hkl[i].GetTag() );
        c++;
      }
    }
  }
  else  {
    for( size_t i=0; i < Hkl.RefCount(); i++ )  {
      if( Hkl[i].GetTag() > 0 )  {
        if( (!h.IsEmpty() && h.IndexOf(Hkl[i].GetH()) != InvalidIndex) ||
            (!k.IsEmpty() && k.IndexOf(Hkl[i].GetK()) != InvalidIndex) ||
            (!l.IsEmpty() && l.IndexOf(Hkl[i].GetL()) != InvalidIndex) )
        {
          Hkl[i].SetTag( -Hkl[i].GetTag() );
          c++;
        }
      }
    }
  }
  Hkl.SaveToFile( hklSrc );
  FXApp->GetLog() << c << " reflections excluded\n";
}
//..............................................................................
void TMainForm::macDirection(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  vec3d Z;
  mat3d Basis( FXApp->GetRender().GetBasis().GetMatrix() );
  olxstr Tmp;
  if( FXApp->XFile().HasLastLoader() )  {
    TAsymmUnit &au = FXApp->XFile().GetAsymmUnit();
    mat3d cellM = FXApp->IsHklVisible() ? au.GetHklToCartesian() : au.GetCellToCartesian();
    cellM *= Basis;
    Z = mat3d::CramerSolve(mat3d::Transpose(cellM), vec3d(0, 0, 1)).Normalise();
    if( FXApp->IsHklVisible() )  {
      Tmp =  "Direction: (";
      Tmp << olxstr::FormatFloat(3, Z[0]) << "*H, " <<
             olxstr::FormatFloat(3, Z[1]) << "*K, " <<
             olxstr::FormatFloat(3, Z[2]) << "*L)";
      TBasicApp::GetLog() << (Tmp << '\n');
      double H = olx_abs(Z[0]);  H *= H;
      double K = olx_abs(Z[1]);  K *= K;
      double L = olx_abs(Z[2]);  L *= L;
      if( H > 0.01 )  H = 1./H;
      if( K > 0.01 )  K = 1./K;
      if( L > 0.01 )  L = 1./L;
      int iH = olx_round(H), iK = olx_round(K), iL = olx_round(L);
      double diff = olx_abs(H + K + L - iH - iK - iL)/3;
      if( diff < 0.15 )  {
        Tmp = "View along (";
        TBasicApp::GetLog() << ( Tmp << iH << ", " << iK << ", " << iL << ')' << '\n');
      }
    }
    else  {
      Tmp =  "Direction: (";
      Tmp << olxstr::FormatFloat(3, Z[0]) << "*A, " <<
             olxstr::FormatFloat(3, Z[1]) << "*B, " <<
             olxstr::FormatFloat(3, Z[2]) << "*C)";
      TBasicApp::GetLog() << (Tmp << '\n');
      const char *Dir[] = {"000", "100", "010", "001", "110", "101", "011", "111"};
      TTypeList<vec3d> Points;
      Points.AddCCopy(Z.Null());
      Points.AddCCopy(cellM[0]);
      Points.AddCCopy(cellM[1]);
      Points.AddCCopy(cellM[2]);
      Points.AddCCopy(cellM[0] + cellM[1]);
      Points.AddCCopy(cellM[0] + cellM[2]);
      Points.AddCCopy(cellM[1] + cellM[2]);
      Points.AddCCopy(cellM[0] + cellM[1] + cellM[2]);
      for( size_t i=0; i < Points.Count(); i++ )  {
        for( size_t j = i+1; j < Points.Count(); j++ )  {
          Z = (Points[j]-Points[i]).Normalise();
          if( olx_abs( olx_abs(Z[2])-1 ) < 0.02 )  {  // 98 % coincidence
            Tmp = "View along ";
            Tmp << Dir[i] <<  '-' <<  Dir[j] << ' ' <<
                   '(' << (int)(100.00-olx_abs(olx_abs(Z[2])-1)*100) <<  '%' << ')';
            TBasicApp::GetLog() << (Tmp << '\n');
          }
        }
      }
    }
    if( !FXApp->XGrid().IsEmpty() && FXApp->XGrid().IsVisible() && 
      (FXApp->XGrid().GetRenderMode()&(planeRenderModeContour|planeRenderModePlane)) != 0 )
    {
      const mat3f bm(FXApp->GetRender().GetBasis().GetMatrix());
      const mat3f c2c(FXApp->XFile().GetAsymmUnit().GetCartesianToCell());
      const vec3f center(FXApp->GetRender().GetBasis().GetCenter());
      vec3f p(0, 0, FXApp->XGrid().GetDepth());
      p = bm*p;
      p -= center;
      p *= c2c;
      Tmp =  "Grid center: (";
      Tmp << olxstr::FormatFloat(3, p[0]) << "*A, " <<
             olxstr::FormatFloat(3, p[1]) << "*B, " <<
             olxstr::FormatFloat(3, p[2]) << "*C)";
      TBasicApp::GetLog() << (Tmp << '\n');
    }
  }
  else  {
    Z = Basis[2];
    Tmp =  "Normal: (";
    Tmp << olxstr::FormatFloat(3, Z[0]) << ", " <<
           olxstr::FormatFloat(3, Z[1]) << ", " <<
           olxstr::FormatFloat(3, Z[2]) << ')';
    TBasicApp::GetLog() << (Tmp << '\n');
  }
}
//..............................................................................
void TMainForm::macUndo(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !FUndoStack->isEmpty() )  {
    TUndoData* data = FUndoStack->Pop();
    data->Undo();
    delete data;
  }
}
//..............................................................................
void TMainForm::macIndividualise(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, false);
  for( size_t i=0; i < Atoms.Count(); i++ )
    FXApp->Individualise(*Atoms[i]);
}
//..............................................................................
void TMainForm::macCollectivise(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList Atoms;
  FindXAtoms(Cmds, Atoms, false, false);
  for( size_t i=0; i < Atoms.Count(); i++ )
    FXApp->Collectivise(*Atoms[i]);
}
//..............................................................................
void TMainForm::macSel(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds.Count() == 1 && Cmds[0].Equalsi("res") )  {
    FXApp->GetRender().ClearSelection();
    TStrList out;
    TCAtomPList a_res;
    TPtrList<TSimpleRestraint> b_res;
    RefinementModel& rm = FXApp->XFile().GetRM();
    rm.Describe(out, &a_res, &b_res);
    for( size_t i=0 ; i < FXApp->AtomCount(); i++ )
      FXApp->GetAtom(i).Atom().CAtom().SetTag(0);
    for( size_t i=0; i < a_res.Count(); i++ )
      a_res[i]->SetTag(1);
    for( size_t i=0; i < FXApp->AtomCount(); i++ )  {
      TXAtom& xa = FXApp->GetAtom(i);
      if( xa.Atom().CAtom().GetTag() != 1 )  continue;
      if( xa.IsSelected() )  continue;
      FXApp->GetRender().Select( xa );
    }
    for( size_t i=0; i < b_res.Count(); i++ )  {
      const TSimpleRestraint& res = *b_res[i];
      for( size_t j=0; j < res.AtomCount(); j+=2 )  {
        if( res.GetAtom(j).GetMatrix() != NULL || res.GetAtom(j+1).GetMatrix() != NULL )  continue;
        const size_t id1 = res.GetAtom(j).GetAtom()->GetId();
        const size_t id2 = res.GetAtom(j+1).GetAtom()->GetId();
        for( size_t k=0; k < FXApp->BondCount(); k++ )  {
          TXBond& xb = FXApp->GetBond(k);
          if( xb.IsSelected() )  continue;
          const TCAtom& ca1 = xb.Bond().A().CAtom();
          const TCAtom& ca2 = xb.Bond().B().CAtom();
          if( (ca1.GetId() == id1 && ca2.GetId() == id2) ||
              (ca1.GetId() == id2 && ca2.GetId() == id1) )  
          {
            FXApp->GetRender().Select(xb);
            break;
          }
        }
      }
    }
    FGlConsole->PrintText(out);
    FXApp->GetLog() << '\n';
    return;
  }
  if( Cmds.Count() == 1 && TSymmParser::IsRelSymm(Cmds[0]) )  {
    bool invert = Options.Contains('i'), unselect=false;
    if( !invert )
      unselect = Options.Contains('u');
    const smatd matr = TSymmParser::SymmCodeToMatrix(FXApp->XFile().GetUnitCell(), Cmds[0]);
    for( size_t i=0; i < FXApp->AtomCount(); i++ )  {
      TXAtom& a = FXApp->GetAtom(i);
      if( a.IsDeleted() || !a.IsVisible() )  continue;
      if( a.Atom().ContainsMatrix(matr.GetId()) )  {
        if( invert )
          FXApp->GetRender().Select(a, !a.IsSelected());
        else if( unselect )
          FXApp->GetRender().Select(a, false);
        else
          FXApp->GetRender().Select(a, true);
      }
    }
    for( size_t i=0; i < FXApp->BondCount(); i++ )  {
      TXBond& b = FXApp->GetBond(i);
      if( b.IsDeleted() || !b.IsVisible() )  continue;
      if( b.Bond().A().ContainsMatrix(matr.GetId()) && b.Bond().B().ContainsMatrix(matr.GetId()) )  {
        if( invert )
          FXApp->GetRender().Select(b, !b.IsSelected());
        else if( unselect )
          FXApp->GetRender().Select(b, false);
        else
          FXApp->GetRender().Select(b, true);
      }
    }
    return;
  }
  if( Cmds.Count() > 1 && Cmds[0].Equalsi("part") )  {
    Cmds.Delete(0);
    TIntList parts;
    for( size_t i=0; Cmds.Count(); i++ )  {
      if( Cmds[i].IsNumber() )  {
        parts.Add( Cmds[i].ToInt() );
        Cmds.Delete(i--);
      }
      else
        break;
    }
    if( !parts.IsEmpty() )  {
      olxstr cond = "xatom.part==";
      cond << parts[0];
      for( size_t i=1; i < parts.Count(); i++ )
        cond << "||xatom.part==" << parts[i];
      FXApp->SelectAtomsWhere(cond);
    }
  }
  if( Options.IsEmpty() )  {  // print labels of selected atoms
    olxstr Tmp("sel");
    size_t period=5;
    TGlGroup& Sel = FXApp->GetSelection();
    TXAtomPList Atoms;
    FXApp->FindXAtoms(Tmp, Atoms, false);
    for( size_t i=0; i <= Atoms.Count(); i+=period )  {
      Tmp = EmptyString;
      for( size_t j=0; j < period; j++ )  {
        if( (j+i) >= Atoms.Count() )  break;
        Tmp << Atoms[i+j]->Atom().GetGuiLabel();
        Tmp.Format((j+1)*14, true, ' ');
      }
      if( !Tmp.IsEmpty() )
        TBasicApp::GetLog() << (Tmp << '\n');
    }
    if( !Cmds.IsEmpty() )  {
      size_t whereIndex = Cmds.IndexOf(olxstr("where"));
      if( whereIndex >= 1 && whereIndex != InvalidIndex)  {
        Tmp = Cmds[whereIndex-1];
        while( olx_is_valid_index(whereIndex) )  {  Cmds.Delete(whereIndex--);  }
        if( Tmp.Equalsi("atoms") )
          FXApp->SelectAtomsWhere(Cmds.Text(' '));
        else if( Tmp.Equalsi("bonds") )
          FXApp->SelectBondsWhere(Cmds.Text(' '));
        else
          Error.ProcessingError(__OlxSrcInfo, "undefined keyword: " ) << Tmp;
        return;
      }
      else  {
        size_t ringsIndex = Cmds.IndexOf(olxstr("rings"));
        if( ringsIndex != InvalidIndex )  {
          Cmds.Delete( ringsIndex );
          FXApp->SelectRings(Cmds.Text(' '));
        }
        else
          FXApp->SelectAtoms(Cmds.Text(' '));
        return;
      }
    }
    olxstr seli = FXApp->GetSelectionInfo();
    if( !seli.IsEmpty() )
      FXApp->GetLog() << (seli << '\n');
  }
  else  {
    for( size_t i=0; i < Options.Count(); i++ )  {
      if( Options.GetName(i)[0] == 'a' )
        FXApp->SelectAll(true);
      else if( Options.GetName(i)[0] == 'u' )
        FXApp->SelectAll(false);
      else if( Options.GetName(i)[0] == 'i' )  {
        if( Cmds.Count() == 0 )
          FXApp->GetRender().InvertSelection();
        else
          FXApp->SelectAtoms(Cmds.Text(' '), true);
      }
      else
        Error.ProcessingError(__OlxSrcInfo, "wrong options" );
    }
  }
}
//..............................................................................
void TMainForm::macReap(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  SetSGList(EmptyString);
  olxstr FN, Tmp;
  bool Blind = Options.Contains('b'); // a switch showing if the last file is remembered
  bool ReadStyle = !Options.Contains('r');
  bool OverlayXFile = Options.Contains('*');
  if( Cmds.Count() >= 1 && !Cmds[0].IsEmpty() )  {  // merge the file name if a long one...
    FN = TEFile::ExpandRelativePath(Cmds.Text(' '));
    if( TEFile::ExtractFileExt(FN).IsEmpty() )  {
      olxstr res_fn = TEFile::ChangeFileExt(FN, "res"),
             ins_fn = TEFile::ChangeFileExt(FN, "ins");
      if( TEFile::Exists(res_fn) )  {
        if( TEFile::Exists(ins_fn) )
          FN = TEFile::FileAge(ins_fn) < TEFile::FileAge(res_fn) ? res_fn : ins_fn;
        else
          FN = res_fn;
      }
      else
        FN = ins_fn;
    }
#ifdef __WIN32__ // tackle short path names problem
    WIN32_FIND_DATA wfd;
    ZeroMemory(&wfd, sizeof(wfd));
    HANDLE fsh = FindFirstFile(FN.u_str(), &wfd);
    if( fsh != INVALID_HANDLE_VALUE )  {
      FN = TEFile::ExtractFilePath(FN);
      if( !FN.IsEmpty() )
        TEFile::AddPathDelimeterI(FN);
      FN << wfd.cFileName;
      FindClose(fsh);
    }
#endif // win32
    Tmp = TEFile::ExtractFilePath(FN);
    if( Tmp.IsEmpty() )  { FN = XLibMacros::CurrentDir + FN; }
  }
  else  {
    FN = PickFile("Open File",
        olxstr("All supported files|*.ins;*.cif;*.res;*.mol;*.xyz;*.p4p;*.mas;*.crs;*pdb;*.fco;*.fcf;*.hkl;*.oxm;*.mol2")  <<
          "|INS files (*.ins)|*.ins"  <<
          "|Olex2 model files (*.oxm)|*.oxm"  <<
          "|CIF files (*.cif)|*.cif" <<
          "|MDL MOL files (*.mol)|*.mol" <<
          "|Tripos MOL2 files (*.mol2)|*.mol2" <<
          "|XYZ files (*.xyz)|*.xyz" <<
          "|P4P files (*.p4p)|*.p4p" <<
          "|PDB files (*.pdb)|*.pdb" <<
          "|XD Master files (*.mas)|*.mas" <<
          "|CRS files (*.crs)|*.crs" <<
          "|FCO files (*.fco)|*.fco" <<
          "|FCF files (*.fcf)|*.fcf" <<
          "|HKL files (*.hkl)|*.hkl",
          XLibMacros::CurrentDir, true);
  }
  
  if( !FN.IsEmpty() )  {  // the dialog has been successfully executed
    /* with some compilations Borland would bring program into an incorrect state
     if the NonExistenFile exception is thrown from XFile ... (MSVC is fine thought)
    */
    // FN might be a dir on windows when a file does not exist - the code above will get the folder name isntead...
    if( !TEFile::Exists(FN) || TEFile::IsDir(FN) )  {
      Error.ProcessingError(__OlxSrcInfo, "Could not locate specified file");
      return;
    }
    if( OverlayXFile )  {
      TXFile& xf = FXApp->NewOverlayedXFile();
      xf.LoadFromFile(FN);
      FXApp->CreateObjects(true, false);
      FXApp->CenterView(true);
      FXApp->AlignOverlayedXFiles();
      return;
    }
    if( Modes->GetCurrent() != NULL )
      Macros.ProcessMacro("mode off", Error);
    Tmp = TEFile::ChangeFileExt(FN, "xlds");
    if( TEFile::Exists(Tmp) )  {
      Macros.ProcessMacro(olxstr("load view '") << TEFile::ChangeFileExt(FN, EmptyString) << '\'', Error);
    }
    else  {
      if( TEFile::Exists(DefStyle) && ReadStyle )
        FXApp->GetRender().GetStyles().LoadFromFile(DefStyle, false);
    }
    // delete the Space groups infor mation file
    if( !(TEFile::ChangeFileExt(FN, EmptyString) == TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), EmptyString)) )
      TEFile::DelFile(DataDir+"spacegroups.htm");
    // special treatment of the kl files
    if( TEFile::ExtractFileExt(FN).Equalsi("hkl") )  {
      if( !TEFile::Exists( TEFile::ChangeFileExt(FN, "ins") ) )  {
        THklFile hkl;
        bool ins_initialised = false;
        TIns* ins = (TIns*)FXApp->XFile().FindFormat("ins");
        //ins->Clear();
        hkl.LoadFromFile(FN, ins, &ins_initialised);
        if( !ins_initialised )  {
          olxstr src_fn = TEFile::ChangeFileExt(FN, "p4p");
          if( !TEFile::Exists(src_fn) )
            src_fn = TEFile::ChangeFileExt(FN, "crs");
          if( !TEFile::Exists(src_fn) )  {
            Error.ProcessingError(__OlxSrcInfo, "could not initialise CELL/SFAC from the hkl file");
            return;
          }
          else
            FN = src_fn;
        }
        else  {
          FXApp->XFile().SetLastLoader(ins);
          ins->Clear();
          FXApp->XFile().GetRM().SetHKLSource(FN);  // make sure tha SGE finds the related HKL
          TMacroError er;
          Macros.ProcessMacro(olxstr("SGE '") << TEFile::ChangeFileExt(FN, "ins") << '\'', er);
          if( !er.HasRetVal() || !er.GetRetObj< TEPType<bool> >()->GetValue()  )  {
            olxstr s_inp("getuserinput(1, \'Please, enter the spacegroup\', \'')"), s_sg(s_inp);
            TSpaceGroup* sg = NULL;
            while( sg == NULL )  {
              ProcessFunction(s_sg);
              sg = TSymmLib::GetInstance().FindGroup(s_sg);
              if( sg != NULL ) break;
              s_sg = s_inp;
            }
            ins->GetAsymmUnit().ChangeSpaceGroup(*sg);
            if( ins->GetRM().GetUserContent().IsEmpty() )  {
              s_inp = "getuserinput(1, \'Please, enter cell content\', \'C1')";
              ProcessFunction(s_inp);
              ins->GetRM().SetUserFormula(s_inp);
            }
            else  {
              size_t sfac_count = ins->GetRM().GetUserContent().Count();
              TStrList unit;
              for( size_t i=0; i < sfac_count; i++ )  
                unit.Add((sg->MatrixCount()+1)*(sg->GetLattice().VectorCount()+1));
              ins->GetRM().SetUserContentSize(unit);
              ins->GetAsymmUnit().SetZ((sg->MatrixCount()+1)*(sg->GetLattice().VectorCount()+1));
            }
            ins->SaveForSolution(TEFile::ChangeFileExt(FN, "ins"), EmptyString, EmptyString, false);
            Macros.ProcessMacro( olxstr("reap '") << TEFile::ChangeFileExt(FN, "ins") << '\'', Error);
            Macros.ProcessMacro("solve", Error);
          }  // sge, if succeseded will run reap and solve
          return;
        }
      }
      else
        FN = TEFile::ChangeFileExt(FN, "ins");
    }
    try  {
      SaveVFS(plStructure); // save virtual fs file
      int64_t st = TETime::msNow();
      FXApp->LoadXFile(FN);
      st = TETime::msNow() - st;
      TBasicApp::GetLog().Info( olxstr("Structure loaded in ") << st << " ms\n");
      BadReflectionsTable(false, false);
      RefineDataTable(false, false);
      LoadVFS(plStructure);  // load virtual fs file
    }
    catch(TEmptyFileException)  {  
      Error.ProcessingError(__OlxSrcInfo, "empty file");
      return;
    }
    catch(const TExceptionBase& exc)  { 
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    if( FXApp->XFile().HasLastLoader() )  {
      FInfoBox->Clear();
      if( FXApp->CheckFileType<TP4PFile>() || FXApp->CheckFileType<TCRSFile>() )  {
        TMacroError er;
        if( TEFile::Exists( TEFile::ChangeFileExt(FN, "ins") ) )
          Macros.ProcessMacro("SG", er);
        else
          Macros.ProcessMacro("SGE", er);
      }
    // automatic export for kappa cif
      if( FXApp->CheckFileType<TCif>() )  {
        TBasicApp::GetLog() << ("Start importing cif ...\n");
        FXApp->Draw();
        olxstr hklFileName = TEFile::ChangeFileExt(FN, "hkl");
        olxstr insFileName = TEFile::ChangeFileExt(FN, "ins");
        TMacroError er;
        if( !TEFile::Exists(hklFileName)  )  {
          TCif& C = FXApp->XFile().GetLastLoader<TCif>();
          if( C.FindLoop("_refln") != NULL )  {
            er.SetRetVal(&C);
            Macros.ProcessMacro( olxstr("export ") << TEFile::ExtractFileName(hklFileName), er);
            if( !er.IsProcessingError() )  {
              if( !TEFile::Exists(insFileName) )  {
                TIns ins;
                ins.Adopt(FXApp->XFile());
                ins.GetRM().SetHKLSource(hklFileName);
                ins.SaveToFile( insFileName );
                Macros.ProcessMacro( olxstr("@reap \'") << insFileName << '\'', er);
                if( !er.IsProcessingError() )  {
                  TBasicApp::GetLog() << ("End importing cif ...\n");
                  Macros.ProcessMacro("reset", er);
                }
                FXApp->Draw();
                return;
              }
            }
            else
              AnalyseError( er );
          }
        }
      }
      FInfoBox->PostText(FN);
      FInfoBox->PostText(FXApp->XFile().LastLoader()->GetTitle());
      // check if the associated HKL file has the same name and location
      olxstr hkl_fn = TEFile::OSPath(FXApp->XFile().GetRM().GetHKLSource()).DeleteSequencesOf(TEFile::GetPathDelimeter()); 
      olxstr src_fn = TEFile::OSPath(FXApp->XFile().LastLoader()->GetFileName()).DeleteSequencesOf(TEFile::GetPathDelimeter()); 
      if( TEFile::ChangeFileExt(hkl_fn, EmptyString) != TEFile::ChangeFileExt(src_fn, EmptyString) )  {
        TBasicApp::GetLog() << "Note that the associated HKL file differs from the loaded file name:\n";
        TBasicApp::GetLog() << (olxstr("SRC: ") << src_fn << '\n');
        TBasicApp::GetLog() << (olxstr("HKL: ") << hkl_fn << '\n');
      }
      // changes the loaded position of the box to left
      OnResize();

      Tmp = TEFile::ExtractFilePath(FN);
      if( !Tmp.IsEmpty() && !(Tmp == XLibMacros::CurrentDir) )  {
        if( !TEFile::ChangeDir(Tmp) )
          TBasicApp::GetLog() << (olxstr("Cannot change current folder '") << TEFile::CurrentDir() << "'  to '" << Tmp << "'\n");
        else  {
          if( !Blind )  
            XLibMacros::CurrentDir = Tmp;
        }
      }
      if( !Blind )  UpdateRecentFile(FN);
      //::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
      QPeakTable(false, true);
      UpdateRecentFilesTable(false);
      if( FXApp->CheckFileType<TIns>() )  {
        BadReflectionsTable(false, true);
        RefineDataTable(false, true);
        const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
        if( Lst.SplitAtomCount() )  {
          TBasicApp::GetLog() << ("The following atom(s) may be split: \n");
          for( size_t i=0; i < Lst.SplitAtomCount(); i++ )  {
            const TLstSplitAtom& SpA = Lst.SplitAtom(i);
            Tmp = SpA.AtomName;  Tmp.Format(5, true, ' ');
            Tmp << olxstr::FormatFloat(3, SpA.PositionA[0]);  Tmp.Format(12, true, ' ');
            Tmp << olxstr::FormatFloat(3, SpA.PositionA[1]);  Tmp.Format(19, true, ' ');
            Tmp << olxstr::FormatFloat(3, SpA.PositionA[2]);  Tmp.Format(26, true, ' ');
            Tmp << "& ";
            Tmp << olxstr::FormatFloat(3, SpA.PositionB[0]);  Tmp.Format(35, true, ' ');
            Tmp << olxstr::FormatFloat(3, SpA.PositionB[1]);  Tmp.Format(42, true, ' ');
            Tmp << olxstr::FormatFloat(3, SpA.PositionB[2]);
            TBasicApp::GetLog() << (Tmp << '\n');
          }
        }
        if( Lst.TrefTryCount() )  {
          TBasicApp::GetLog() << ("TREF tries:\n");
          olxstr Tmp1;
          Tmp = "CFOM";  Tmp.Format(6, true, ' ');
          Tmp1 = Tmp;
          Tmp = "NQual";  Tmp1 << Tmp.Format(10, true, ' ');
          Tmp = "Try#";   Tmp1 << Tmp.Format(10, true, ' ');
          Tmp1 << "Semivariants";
          TBasicApp::GetLog() << (Tmp1 << '\n');
          int tcount = 0;
          for( size_t i=0; i < Lst.TrefTryCount(); i++ )  {
            if( i > 0 )  {
              if( Lst.TrefTry(i-1).CFOM == Lst.TrefTry(i).CFOM &&
                Lst.TrefTry(i-1).Semivariants == Lst.TrefTry(i).Semivariants &&
                Lst.TrefTry(i-1).NQual == Lst.TrefTry(i).NQual )
                continue;
            }
            Tmp = Lst.TrefTry(i).CFOM;  Tmp.Format(6, true, ' ');
            Tmp1 = Tmp;
            Tmp = Lst.TrefTry(i).NQual;  Tmp1 << Tmp.Format(10, true, ' ');
            Tmp = Lst.TrefTry(i).Try;    Tmp1 << Tmp.Format(10, true, ' ');
            Tmp1 << Lst.TrefTry(i).Semivariants.FormatString(31);
            //Tmp1 += Lst.TrefTry(i).Semivariants.FormatString( Lst.TrefTry(i).Semivariants.Count() );
            TBasicApp::GetLog() << (Tmp1 << '\n');
            tcount ++;
            if( tcount > 5 && ( (i+1) < Lst.TrefTryCount()) )  {
              TBasicApp::GetLog() << ( olxstr("There are ") << Lst.TrefTryCount() - i << " more tries\n");
              break;
            }
          }
        }
        if( Lst.PattSolutionCount() > 1 )  {
          TBasicApp::GetLog() << ( olxstr("There are ") << Lst.PattSolutionCount()
            << " possible patterson solutions in the listing file\n");
          TBasicApp::GetLog() << ( "To browse possible solutions press Ctrl+Up and Ctrl+Down buttons. Press Enter to choose a particular solution\n" );
          FMode |= mSolve;
          CurrentSolution = -1;
        }
      }
    }
//::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
//      FXApp->Draw();  // to update the scene just in case...
    FGlConsole->SetCommand( FGlConsole->GetCommand() );  // force th eupdate
    return;
  }
  else  {
    Error.ProcessingError(__OlxSrcInfo, EmptyString );
    return;
  }
}
//..............................................................................
void TMainForm::macPopup(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  int width = Options.FindValue("w", "100").ToInt(), 
    height = Options.FindValue("h", "200").ToInt(), 
    x = Options.FindValue("x", "0").ToInt(), 
    y = Options.FindValue("y", "0").ToInt();
  olxstr border = Options.FindValue("b"), 
    title = Options.FindValue("t"), 
    onDblClick = Options.FindValue("ondblclick"),
    onSize = Options.FindValue("onsize");
  int iBorder = 0;
  for( size_t i=0; i < border.Length(); i++ )  {
    if( border.CharAt(i) == 't' )  iBorder |= wxCAPTION;
    else if( border.CharAt(i) == 'r' )  iBorder |= wxRESIZE_BORDER;
    else if( border.CharAt(i) == 's' )  iBorder |= wxSYSTEM_MENU;
    else if( border.CharAt(i) == 'c' )  iBorder |= (wxCLOSE_BOX|wxSYSTEM_MENU);
    else if( border.CharAt(i) == 'a' )  iBorder |= (wxMAXIMIZE_BOX|wxSYSTEM_MENU);
    else if( border.CharAt(i) == 'i' )  iBorder |= (wxMINIMIZE_BOX|wxSYSTEM_MENU);
    else if( border.CharAt(i) == 'p' )  iBorder |= wxSTAY_ON_TOP;
  }
  if( iBorder == 0 )
    iBorder = wxNO_BORDER;
  // check if the popup already exists
  TPopupData *pd = GetPopup(Cmds[0]);
  if( pd != NULL )  {
    //pd->Dialog->SetWindowStyle( iBorder );
    //pd->Dialog->SetSize(x, y, width, height, wxSIZE_USE_EXISTING);
    //pd->Dialog->SetTitle( title );
    THtml* ph = FHtml;
    FHtml = pd->Html;
    try  {  pd->Html->LoadPage(Cmds[1].u_str());  }
    catch( ... )  {}
    FHtml = ph;
    pd->Html->SetHomePage(TutorialDir + Cmds[1]);
    if( Options.Contains('w') && Options.Contains('h') )  {
#ifdef __WXGTK__  // any aother way to forse move ???
      pd->Dialog->SetSize(5000, 5000, 0, 0);
#endif
      pd->Dialog->SetSize(x, y, width, height);
      pd->Dialog->GetClientSize(&width, &height);
      pd->Html->SetSize(width, height);
    }
    if( !pd->Dialog->IsShown() && !Options.Contains('s'))
      pd->Dialog->Show(true);
    return;
  }

  TDialog *dlg = new TDialog(this, title.u_str(), wxT("htmlPopupWindow"), wxPoint(x,y),
    wxSize(width, height), iBorder);
  THtml *html1 = new THtml(dlg, FXApp);
  dlg->OnResize.Add(html1, html_parent_resize, msiExecute);
//  html1->WI.AddWindowStyle(wxTAB_TRAVERSAL);
  html1->SetWebFolder(TutorialDir);
  html1->SetHomePage(TutorialDir + Cmds[1]);
  html1->SetMovable(false);
  html1->SetOnSizeData(onSize.Replace("\\(", '('));
  html1->SetOnDblClickData(onDblClick.Replace("\\(", '('));
  dlg->GetClientSize(&width, &height);
  html1->SetSize(width, height);
  pd = new TPopupData;
  pd->Dialog = dlg;
  pd->Html = html1;

  FPopups.Add(Cmds[0], pd);
  THtml* ph = FHtml;
  FHtml = html1;
  try  {  html1->LoadPage(Cmds[1].u_str());  }
  catch( ... )  {}
  FHtml = ph;
  
  html1->OnLink.Add(this, ID_ONLINK);
  html1->OnKey.Add(this, ID_HTMLKEY);
  html1->OnDblClick.Add(this, ID_ONLINK);
  html1->OnSize.Add(this, ID_ONLINK);
  if( !Options.Contains('s') )
    dlg->Show();
}
//..............................................................................
void TMainForm::macDelta(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 1 )  {
    double delta = Cmds[0].ToDouble();
    if( delta < 0.1 || delta > 0.9 )
      delta = 0.5;
    FXApp->XFile().GetLattice().SetDelta(delta );
    Macros.ProcessMacro("fuse", E);
  }
  else
    TBasicApp::GetLog() << ( olxstr("Current delta (covalent bonds) is: ") << FXApp->XFile().GetLattice().GetDelta() << '\n' );
}
//..............................................................................
void TMainForm::macDeltaI(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 1 )  {
    double deltai = Cmds[0].ToDouble();
    if( deltai < 0.9 )
      deltai = 0.9;
    else if( deltai > 3 )  
      deltai = 3;
    FXApp->XFile().GetLattice().SetDeltaI(deltai);
  }
  else
    TBasicApp::GetLog() << ( olxstr("Current delta (short interactions bonds) is: ") << FXApp->XFile().GetLattice().GetDeltaI() << '\n' );
}
//..............................................................................
void TMainForm::macPython(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Options.Contains('i') || Options.Contains('l') )  {
    TdlgEdit *dlg = new TdlgEdit(TGlXApp::GetMainForm(), true);
    dlg->SetTitle( wxT("Python script editor") );
    if( Options.Contains('l') )  {
      olxstr FN = PickFile("Open File",
        olxstr("Python scripts (*.py)|*.py")  <<
        "|Text files (*.txt)|*.txt"  <<
        "|All files (*.*)|*.*",
        TBasicApp::GetBaseDir(), true);
      if( !FN.IsEmpty() && TEFile::Exists(FN) )  {
        TStrList sl;
        sl.LoadFromFile(FN);
        dlg->SetText( sl.Text('\n') );
      }
    }
    else
      dlg->SetText(wxT("import olex\n"));
    if( dlg->ShowModal() == wxID_OK )
      PythonExt::GetInstance()->RunPython(dlg->GetText());
    dlg->Destroy();
  }
  olxstr tmp = Cmds.Text(' ');
  tmp.Replace("\\n", "\n");
  if( !tmp.EndsWith('\n') )  tmp << '\n';
  PythonExt::GetInstance()->RunPython(tmp);
}
//..............................................................................
void TMainForm::funEval(const TStrObjList& Params, TMacroError &E)  {
  TStrList Vars;
  TStrPObjList<olxstr,TSOperation*> Funcs;
  TSOperation S(NULL, &Funcs, &Vars, NULL);
  if( S.LoadFromExpression(Params[0]) == 0 )
    E.SetRetVal(S.Evaluate());
}
//..............................................................................
void TMainForm::macCreateMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  for( size_t i=0; i < Cmds.Count(); i++ )
    Cmds[i].Replace("\\t", '\t');
  size_t ind = Cmds[0].LastIndexOf(';');
  if( ind == InvalidIndex )  
    throw TInvalidArgumentException(__OlxSourceInfo, "menu name");
  olxstr menuName = Cmds[0].SubStringTo(ind);

  short itemType = mtNormalItem;
  if( Options.Contains("r") )  itemType = mtRadioItem;
  if( Options.Contains("c") )  itemType = mtCheckItem;
  if( Options.Contains("#") )  itemType = mtSeparator;
  olxstr stateDependent = Options.FindValue("s");
  olxstr modeDependent = Options.FindValue("m");
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  {
    TStrList toks;
    toks.Strtok( Cmds[0], ';');
    size_t mi=0;
    while( (ind = menuName.LastIndexOf(';')) != InvalidIndex && ! menu )  {
      menuName = menuName.SubStringTo(ind);
      menu = Menus[menuName];
      mi++;
      
      if( menu )  break;
    }
    mi = (menu == NULL ? 0 : toks.Count() - 1 - mi);
    for( size_t i=mi; i < toks.Count(); i++ )  {
      if( (i+1) == toks.Count() )  {
        int accell = AccMenus.GetLastId();
        if( accell == 0 )
          accell = 1000;
        else
          accell++;
        if( Cmds.Count() == 3 )  {
          size_t insindex = menu->FindItem( Cmds[2].u_str() );
          if( insindex == InvalidIndex )  insindex = 0;
          if( itemType == mtSeparator )
            menu->InsertSeparator(insindex);
          else {
            TMenuItem* item = new TMenuItem(itemType, accell, menu, toks[i]);
            if( !modeDependent.IsEmpty() )  item->SetActionQueue(OnModeChange, modeDependent, TMenuItem::ModeDependent);
            if( !stateDependent.IsEmpty() )  item->SetActionQueue(OnStateChange, stateDependent, TMenuItem::StateDependent);
            if( Cmds.Count() > 1 )  item->SetCommand( Cmds[1] );
            menu->Insert(insindex, item );
            AccMenus.AddAccell(accell, item );
          }
        }
        else  {
          if( itemType == mtSeparator )  menu->AppendSeparator();
          else {
            TMenuItem* item = new TMenuItem(itemType, accell, menu, toks[i]);
            if( !modeDependent.IsEmpty() )  item->SetActionQueue(OnModeChange, modeDependent, TMenuItem::ModeDependent);
            if( !stateDependent.IsEmpty() )  item->SetActionQueue(OnStateChange, stateDependent, TMenuItem::StateDependent);
            if( Cmds.Count() > 1 )  item->SetCommand(Cmds[1]);
            menu->Append( item );
            AccMenus.AddAccell(accell, item );
          }
        }
      }
      else  {
        TMenu* smenu = new TMenu();
        if( !menu )  {
          if( Cmds.Count() == 3 )  {
            size_t insindex = MenuBar->FindMenu(Cmds[2].u_str());
            if( insindex == InvalidIndex )  insindex = 0;
            MenuBar->Insert(insindex, smenu, toks[i].u_str());
          }
          else
            MenuBar->Append(smenu, toks[i].u_str());
        }
        else
          menu->Append(-1, toks[i].u_str(), (wxMenu*)smenu);
        olxstr addedMenuName;
        for( size_t j=0; j <= i; j++ )  {
          addedMenuName << toks[j];
          if( j < i )  addedMenuName << ';';
        }
        Menus.Add(addedMenuName, smenu);
        menu = smenu;
      }
    }
  }
  else  {
    int accell = AccMenus.GetLastId();
    if( accell == 0 )
      accell = 1000;
    else
      accell++;
    menuName = Cmds[0].SubStringFrom(ind+1);
    if( menuName == '#' )  menu->AppendSeparator();
    else  {
      size_t insindex;
      if( (insindex=menu->FindItem( menuName.u_str() )) != InvalidIndex )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("duplicated name: ") << menuName);
      if( Cmds.Count() == 3 )  {
        if( insindex == InvalidIndex )  insindex = 0;
        if( itemType == mtSeparator )  menu->InsertSeparator(insindex);
        else  {
          TMenuItem* item = new TMenuItem(itemType, accell, menu, menuName);
          if( Cmds.Count() > 1 )  item->SetCommand( Cmds[1] );
          if( !modeDependent.IsEmpty() )  item->SetActionQueue(OnModeChange, modeDependent, TMenuItem::ModeDependent);
          if( !stateDependent.IsEmpty() )  item->SetActionQueue(OnStateChange, stateDependent, TMenuItem::StateDependent);
          menu->Insert(insindex, item);
          AccMenus.AddAccell(accell, item);
        }
      }
      else  {
        if( itemType == mtSeparator )  menu->AppendSeparator();
        else  {
          TMenuItem* item = new TMenuItem(itemType, accell, menu, menuName);
          if( !modeDependent.IsEmpty() )  item->SetActionQueue(OnModeChange, modeDependent, TMenuItem::ModeDependent);
          if( !stateDependent.IsEmpty() )  item->SetActionQueue(OnStateChange, stateDependent, TMenuItem::StateDependent);
          if( Cmds.Count() > 1 )  item->SetCommand(Cmds[1]);
          menu->Append(item);
          AccMenus.AddAccell(accell, item);
        }
      }
    }
  }
}
//..............................................................................
void TMainForm::macDeleteMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TMenu* menu = Menus[Cmds[0]];
  if( menu == NULL )  {
    size_t ind = Cmds[0].LastIndexOf(';');
    if( ind == InvalidIndex )  return;
    olxstr menuName = Cmds[0].SubStringTo(ind);
    olxstr itemName =  Cmds[0].SubStringFrom(ind+1);
    menu = Menus[menuName];
    if( menu == NULL )  return;
    ind = menu->FindItem(itemName.u_str());
    if( ind == InvalidIndex )  return;
    menu->Destroy((int)ind);
  }
  else
  {   /*
    size_t ind = Cmds[0].LastIndexOf(';');
    if( ind == InvalidIndex )  {
      ind = MenuBar->FindMenu( Cmds[0].c_str() );
      if( ind != InvalidIndex )  MenuBar
    }
    olxstr menuName = Cmds[0].SubStringTo( ind );
    olxstr itemName =  Cmds[0].SubStringFrom( ind+1 );
    menu = (TMenu*)Menus.ObjectByName( menuName );
    if( !menu )  return;
    ind = menu->FindItem( itemName.c_str() );
    if( ind == InvalidIndex )  return;
    menu->Destroy( ind );  */
  }
}
//..............................................................................
void TMainForm::macEnableMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  size_t ind = Cmds[0].LastIndexOf(';');
  if( ind == InvalidIndex )  return;
  olxstr menuName = Cmds[0].SubStringTo(ind);
  olxstr itemName =  Cmds[0].SubStringFrom(ind+1);
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  return;
  ind = menu->FindItem(itemName.u_str());
  if( ind == InvalidIndex )  return;
  menu->Enable((int)ind, true);
}
//..............................................................................
void TMainForm::macDisableMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  size_t ind = Cmds[0].LastIndexOf(';');
  if( ind == InvalidIndex )  return;
  olxstr menuName = Cmds[0].SubStringTo(ind);
  olxstr itemName =  Cmds[0].SubStringFrom(ind+1);
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  return;
  ind = menu->FindItem(itemName.u_str());
  if( ind == InvalidIndex )  return;
  menu->Enable((int)ind, false);
}
//..............................................................................
void TMainForm::macCheckMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  size_t ind = Cmds[0].LastIndexOf(';');
  if( ind == InvalidIndex )  return;
  olxstr menuName = Cmds[0].SubStringTo(ind);
  olxstr itemName =  Cmds[0].SubStringFrom(ind+1);
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  return;
  ind = menu->FindItem(itemName.u_str());
  if( ind == InvalidIndex )  return;
  menu->Check((int)ind, true);
}
//..............................................................................
void TMainForm::macUncheckMenu(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  size_t ind = Cmds[0].LastIndexOf(';');
  if( ind == InvalidIndex )  return;
  olxstr menuName = Cmds[0].SubStringTo(ind);
  olxstr itemName =  Cmds[0].SubStringFrom(ind+1);
  TMenu* menu = Menus[menuName];
  if( menu == NULL )  return;
  ind = menu->FindItem(itemName.u_str());
  if( ind == InvalidIndex )  return;
  menu->Check((int)ind, false);
}
//..............................................................................
void TMainForm::macCreateShortcut(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  AccShortcuts.AddAccell( TranslateShortcut( Cmds[0]), Cmds[1] );
}
//..............................................................................
void TMainForm::macSetCmd(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  FGlConsole->SetCommand( Cmds.Text(' ') );
}
//..............................................................................
void TMainForm::funCmdList(const TStrObjList &Cmds, TMacroError &E) {
  if( FGlConsole->GetCommandCount() == 0 ) return;
  size_t cc = FGlConsole->GetCommandIndex() + Cmds[0].ToInt();
  if( FGlConsole->GetCommandCount() == 0 )  {
    E.SetRetVal(EmptyString);
    return;
  }
  if( cc >= FGlConsole->GetCommandCount() )
    cc = 0;
  E.SetRetVal( FGlConsole->GetCommandByIndex(cc) );
}
//..............................................................................
void TMainForm::macUpdateOptions(TStrObjList &Cmds, const TParamList &Options, TMacroError &E) {
  if( !FXApp->IsBaseDirWriteable() )  {
    E.ProcessingError(__OlxSrcInfo, "This feature is not accessible in read-only installation");
    return;
  }
  TdlgUpdateOptions* dlg = new TdlgUpdateOptions(this);
  dlg->ShowModal();
  dlg->Destroy();
}
//..............................................................................
void TMainForm::macReload(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds[0].Equalsi("macro") )  {
    if( TEFile::Exists(FXApp->GetBaseDir() + "macro.xld") )  {
      TStrList SL;
      FMacroFile.LoadFromXLFile(FXApp->GetBaseDir() + "macro.xld", &SL);
      TDataItem* root = FMacroFile.Root().FindItem("xl_macro");
      FMacroFile.Include(&SL);
      TBasicApp::GetLog() << (SL);
      Macros.Load(*root);
    }
  }
  else if( Cmds[0].Equalsi("help") )  {
    if( TEFile::Exists(FXApp->GetBaseDir() + "help.xld") )  {
      TStrList SL;
      FHelpFile.LoadFromXLFile(FXApp->GetBaseDir() + "help.xld", &SL);
      FHelpItem = FHelpFile.Root().FindItem("xl_help");
      TBasicApp::GetLog() << (SL);
    }
  }
  else if( Cmds[0].Equalsi("dictionary") )  {
    if( TEFile::Exists( DictionaryFile ) )
      Dictionary.SetCurrentLanguage(DictionaryFile, Dictionary.GetCurrentLanguage() );
  }
}
//..............................................................................
void TMainForm::macSelBack(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FXApp->RestoreSelection();
}
//..............................................................................
void TMainForm::macStoreParam(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  size_t ind = StoredParams.IndexOfComparable(Cmds[0]);
  if( ind == InvalidIndex )
    StoredParams.Add( Cmds[0], Cmds[1] );
  else
    StoredParams.GetObject(ind) = Cmds[1];
  if( Cmds.Count() == 3 && Cmds[2].ToBool() )
    SaveSettings(DataDir + FLastSettingsFile);
}
//..............................................................................
void TMainForm::macCreateBitmap(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  wxFSFile* inf = TFileHandlerManager::GetFSFileHandler( Cmds[1] );
  if( inf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Image file does not exist: ") << Cmds[1];
    return;
  }
  wxImage img( *inf->GetStream() );
  delete inf;
  if( !img.Ok() )  {
    E.ProcessingError(__OlxSrcInfo, "Invalid image file: ") << Cmds[1];
    return;
  }
  bool resize = !Options.Contains('r');

  int owidth = img.GetWidth(), oheight = img.GetHeight();
  int l = CalcL( img.GetWidth() );
  int swidth = (int)pow((double)2, (double)l);
  l = CalcL( img.GetHeight() );
  int sheight = (int)pow((double)2, (double)l);

  if( swidth != owidth || sheight != oheight )
    img.Rescale( swidth, sheight );

  int cl = 3, bmpType = GL_RGB;
  if( img.HasAlpha() )  {
    cl ++;
    bmpType = GL_RGBA;
  }

  unsigned char* RGBData = new unsigned char[ swidth * sheight * cl];
  for( int i=0; i < sheight; i++ )  {
    for( int j=0; j < swidth; j++ )  {
      int indexa = (i*swidth + (swidth-j-1)) * cl;
      RGBData[indexa] = img.GetRed(j, i);
      RGBData[indexa+1] = img.GetGreen(j, i);
      RGBData[indexa+2] = img.GetBlue(j, i);
      if( cl == 4 )
        RGBData[indexa+3] = img.GetAlpha(j, i);
    }
  }
  bool Created = (FXApp->FindGlBitmap(Cmds[0]) == NULL);

  TGlBitmap* glB = FXApp->CreateGlBitmap( Cmds[0], 0, 0, swidth, sheight, RGBData, bmpType);
  delete [] RGBData;

  int Top = FInfoBox->IsVisible() ? (FInfoBox->GetTop() + FInfoBox->GetHeight()) : 0;
  if( Created )  {
    for( size_t i=0; i < FXApp->GlBitmapCount(); i++ )  {
      TGlBitmap& b = FXApp->GlBitmap(i);
      if( &b == glB )  continue;
      Top += (b.GetHeight() + 2);
    }
  }

  if( Created ) {
    glB->SetWidth( owidth );
    glB->SetHeight( oheight );
  }
  glB->SetTop( Top );
  if( resize && Created ) {
    double r = ((double)FXApp->GetRender().GetWidth()/(double)owidth)/10.0;
    glB->Basis.Reset();
    glB->Basis.SetZoom(r);
  }
  glB->SetLeft( FXApp->GetRender().GetWidth() - glB->GetWidth() );
  FXApp->Draw();
}
//..............................................................................
void TMainForm::macDeleteBitmap(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  FXApp->DeleteGlBitmap( Cmds[0] );
}
//..............................................................................
void TMainForm::ChangeSolution(int sol)  {
  if( !FXApp->CheckFileType<TIns>() )  return;
  const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
  if( Lst.PattSolutionCount() == 0 )  {
    if( Solutions.IsEmpty() )  return;
    if( sol < 0 )  
      sol = Solutions.Count()-1;
    else if( (size_t)sol >= Solutions.Count() )  
      sol = 0;
    olxstr cf = olxstr(SolutionFolder) << Solutions[sol] << ".res";
    TEFile::Copy( cf, TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res"), true);
    TEFile::Copy( TEFile::ChangeFileExt(cf, "lst"),
                  TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "lst"), true);
    FXApp->LoadXFile( TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res") );
    CurrentSolution = sol;
    TBasicApp::GetLog() << (olxstr("Current solution with try #") << Solutions[sol] << '\n');
    FXApp->Draw();
  }
  else  {
    if( sol < 0 )  
      sol = Lst.PattSolutionCount()-1;
    else if( (size_t)sol >= Lst.PattSolutionCount() )  
      sol = 0;

    TIns& Ins = FXApp->XFile().GetLastLoader<TIns>();
    Ins.SavePattSolution( TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res"),
      Lst.PattSolution(sol), olxstr("Solution #") << (sol+1) );

    FXApp->LoadXFile(TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res"));
    CurrentSolution = sol;
    TBasicApp::GetLog() << ( olxstr("Current patt solution #") << (sol+1) << '\n' );
    FXApp->Draw();
  }
}
//..............................................................................
void TMainForm::macTref(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
  if( !Lst.TrefTryCount() )  {
    E.ProcessingError(__OlxSrcInfo, "lst file does not contain tries");
    return;
  }
  Solutions.Clear();
  CurrentSolution = -1;
  FMode |= mSolve;
  SolutionFolder = EmptyString;

  FXApp->XFile().UpdateAsymmUnit();  // update the last loader RM

  int reps = Cmds[0].ToInt();
  for( size_t i=0; i < Lst.TrefTryCount(); i++ )  {
    if( i > 0 )  {
      if( Lst.TrefTry(i-1).CFOM == Lst.TrefTry(i).CFOM &&
        Lst.TrefTry(i-1).Semivariants == Lst.TrefTry(i).Semivariants &&
        Lst.TrefTry(i-1).NQual == Lst.TrefTry(i).NQual )
      continue;
    }
    Solutions.AddACopy(Lst.TrefTry(i).Try);
    reps --;
    if( reps <=0 )  break;
  }
  SolutionFolder = TEFile::ExtractFilePath(FXApp->XFile().GetFileName() );
  TEFile::AddPathDelimeterI( SolutionFolder ) << "olex_sol\\";
  if( !TEFile::Exists( SolutionFolder ) )
    TEFile::MakeDir( SolutionFolder );
  olxstr cinsFN = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "ins");
  olxstr cresFN = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "res");
  olxstr clstFN = TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "lst");
  TOlxVars::SetVar("internal_tref", TrueString);
  try  {
    for( size_t i=0; i < Solutions.Count(); i++ )  {
      TIns& Ins = FXApp->XFile().GetLastLoader<TIns>();
      Ins.SaveForSolution(cinsFN, olxstr("TREF -") << Solutions[i], EmptyString);
      FXApp->LoadXFile(cinsFN);
      Macros.ProcessMacro("solve", E);
      Macros.ProcessMacro("waitfor process", E);
      TEFile::Copy(cresFN, olxstr(SolutionFolder) <<  Solutions[i] << ".res");
      TEFile::Copy(clstFN, olxstr(SolutionFolder) <<  Solutions[i] << ".lst");
    }
    ChangeSolution(0);
  }
  catch(...)  {  }
  TOlxVars::UnsetVar("internal_tref");
}
//..............................................................................
void TMainForm::macPatt(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  const TLst& Lst = FXApp->XFile().GetLastLoader<TIns>().GetLst();
  if( Lst.PattSolutionCount() == 0 )  {
    E.ProcessingError(__OlxSrcInfo, "lst file does not contain Patterson solutions");
    return;
  }
  Solutions.Clear();
  CurrentSolution = -1;
  FMode |= mSolve;
  SolutionFolder = EmptyString;

  FXApp->XFile().UpdateAsymmUnit();  // update the last loader RM
}
//..............................................................................
void TMainForm::macExport(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr exName;
  if( !Cmds.IsEmpty() )
    exName = Cmds[0];
  else
    exName = TEFile::ChangeFileExt( FXApp->XFile().GetFileName(), "hkl" );

  if( TEFile::Exists(exName) )  {
    E.ProcessingError(__OlxSrcInfo, "the hkl file already exists");
    return;
  }
  TCif* C = NULL;
  if( E.RetObj() != NULL && EsdlInstanceOf(*E.RetObj(), TCif) )
    C = E.GetRetObj< TCif >();
  else
    C = &FXApp->XFile().GetLastLoader<TCif>();
  TCifLoop* hklLoop = C->FindLoop("_refln");
  if( hklLoop == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "no hkl loop found");
    return;
  }
  size_t hInd = hklLoop->GetTable().ColIndex("_refln_index_h");
  size_t kInd = hklLoop->GetTable().ColIndex("_refln_index_k");
  size_t lInd = hklLoop->GetTable().ColIndex("_refln_index_l");
  size_t mInd = hklLoop->GetTable().ColIndex("_refln_F_squared_meas");
  size_t sInd = hklLoop->GetTable().ColIndex("_refln_F_squared_sigma");

  if( (hInd|kInd|lInd|mInd|sInd) == InvalidIndex ) {
    E.ProcessingError(__OlxSrcInfo, "could not locate <h k l meas sigma> data");
    return;
  }

  THklFile file;
  for( size_t i=0; i < hklLoop->GetTable().RowCount(); i++ )  {
    TReflection* r = new TReflection( hklLoop->GetTable()[i][hInd].ToInt(),
                                      hklLoop->GetTable()[i][kInd].ToInt(),
                                      hklLoop->GetTable()[i][lInd].ToInt(),
                                      hklLoop->GetTable()[i][mInd].ToDouble(),
                                      hklLoop->GetTable()[i][sInd].ToDouble() );
    file.Append( *r );
  }
  file.SaveToFile( exName );
}
//..............................................................................
void TMainForm::funAlert(const TStrObjList& Params, TMacroError &E) {
  olxstr msg( Params[1] );
  msg.Replace("\\n", "\n");
  if( Params.Count() == 2 )  {
    E.SetRetVal( TdlgMsgBox::Execute(this, msg, Params[0]) );
  }
  else if( Params.Count() == 3 || Params.Count() == 4 )  {
    int flags = 0;
    bool showCheckBox=false;
    for( size_t i=0; i < Params[2].Length(); i++ )  {
      if( Params[2].CharAt(i) == 'Y' )  flags |= wxYES;
      else if( Params[2].CharAt(i) == 'N' )  flags |= wxNO;
      else if( Params[2].CharAt(i) == 'C' )  flags |= wxCANCEL;
      else if( Params[2].CharAt(i) == 'O' )  flags |= wxOK;
      else if( Params[2].CharAt(i) == 'X' )  flags |= wxICON_EXCLAMATION;
      else if( Params[2].CharAt(i) == 'H' )  flags |= wxICON_HAND;
      else if( Params[2].CharAt(i) == 'E' )  flags |= wxICON_ERROR;
      else if( Params[2].CharAt(i) == 'I' )  flags |= wxICON_INFORMATION;
      else if( Params[2].CharAt(i) == 'Q' )  flags |= wxICON_QUESTION;
      else if( Params[2].CharAt(i) == 'R' )  showCheckBox = true;
    }
    olxstr tickBoxMsg(EmptyString);
    if( Params.Count() == 4 )
      tickBoxMsg = Params[3];
    E.SetRetVal( TdlgMsgBox::Execute(this, msg, Params[0], tickBoxMsg, flags, showCheckBox) );
  }
  FGlCanvas->SetFocus();
}
//..............................................................................
void TMainForm::macAddLabel(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  double x = 0, 
         y = 0, 
         z = 0;
  olxstr name, label;
  if( Cmds.Count() == 3 )  {
    name = Cmds[0];
    label = Cmds[2];
    TStrList toks;
    toks.Strtok( Cmds[1], ' ');
    if( toks.Count() == 3 )  {
      x = toks[0].ToDouble();
      y = toks[1].ToDouble();
      z = toks[2].ToDouble();
    }
  }
  else  if( Cmds.Count() == 5 ) {
    name = Cmds[0];
    x = Cmds[1].ToDouble();
    y = Cmds[2].ToDouble();
    z = Cmds[3].ToDouble();
    label = Cmds[4];
  }
  FXApp->AddLabel( name, 
                   vec3d(x, y, z),
                   label);
}
//..............................................................................
//
  class TOnSync : public AActionHandler  {
    TGXApp* xa;
    olxstr BaseDir;
    public:
      TOnSync(TGXApp& xapp, const olxstr baseDir)  {
        xa = & xapp;
        BaseDir = baseDir;
      }
      bool Execute(const IEObject *Sender, const IEObject *Data)  {
        if( !EsdlInstanceOf(*Data, olxstr) )  return false;
        olxstr cpath = olxstr::CommonString(BaseDir, *(const olxstr*)Data);
        TBasicApp::GetLog() << ( olxstr("\rInstalling /~/") << ((olxstr*)Data)->SubStringFrom(cpath.Length()) );
        xa->Draw();
        wxTheApp->Dispatch();
        return true;
      }
  };
class TDownloadProgress: public AActionHandler  {
    TGXApp* xa;
public:
  TDownloadProgress(TGXApp& xapp) : xa(&xapp) {  }
  bool Enter(const IEObject *Sender, const IEObject *Data)  {
    if( Data != NULL && EsdlInstanceOf(*Data, TOnProgress) )
      TBasicApp::GetLog() <<  ((TOnProgress*)Data)->GetAction() << '\n';
    return true;
  }
  bool Exit(const IEObject *Sender, const IEObject *Data)  {
    TBasicApp::GetLog() <<  "\nDone\n";
    return true;
  }
  bool Execute(const IEObject *Sender, const IEObject *Data)  {
    if( !EsdlInstanceOf(*Data, TOnProgress) )
      return false;
    IEObject* p_d = const_cast<IEObject*>(Data);
    TOnProgress *A = dynamic_cast<TOnProgress*>(p_d);
    if( A->GetPos() <= 0 )  return false;
    if( A->GetMax() <= 0 )
      TBasicApp::GetLog() <<  (olxstr("\r") << A->GetPos()/1024 << "Kb");
    else
      TBasicApp::GetLog() <<  (olxstr("\r") << A->GetPos()*100/A->GetMax() << '%');
    xa->Draw();
    wxTheApp->Dispatch();
    return true;
  }
};
//
void TMainForm::macInstallPlugin(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( !FPluginItem->ItemExists(Cmds[0]) )  {
    TStateChange sc(prsPluginInstalled, true, Cmds[0]);
    olxstr local_file = Options['l'];
    if( !local_file.IsEmpty() )  {
      if( !TEFile::Exists(local_file) )  {
        E.ProcessingError(__OlxSrcInfo, "cannot find plugin archive");
        return;
      }
      TwxZipFileSystem zipFS( local_file, false );
      TOSFileSystem osFS( TBasicApp::GetBaseDir() );
      TFSIndex fsIndex( zipFS );
      TStrList properties;
      properties.Add(Cmds[0]);
      TOnSync* progressListener = new TOnSync(*FXApp, TBasicApp::GetBaseDir() );
      osFS.OnAdoptFile.Add(progressListener);

      IEObject* Cause = NULL;
      try  {  fsIndex.Synchronise(osFS, properties);  }
      catch( const TExceptionBase& exc )  {
        Cause = exc.Replicate();
      }
      osFS.OnAdoptFile.Remove(progressListener);
      delete progressListener;
      if( Cause )
        throw TFunctionFailedException(__OlxSourceInfo, Cause);

      FPluginItem->AddItem( Cmds[0] );
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);

      FPluginFile.SaveToXLFile( PluginFile );
      TBasicApp::GetLog() << ("Installation complete\n");
      FXApp->Draw();
    }
    else  {
      olxstr SettingsFile( TBasicApp::GetBaseDir() + "usettings.dat" );
      if( TEFile::Exists(SettingsFile) )  {
        updater::UpdateAPI api;
        short res = api.InstallPlugin(new TDownloadProgress(*FXApp), 
          new TOnSync(*FXApp, TBasicApp::GetBaseDir() ),
          Cmds[0]
        );
        if( res == updater::uapi_OK )  {
          FPluginItem->AddItem( Cmds[0] );
          OnStateChange.Execute((AEventsDispatcher*)this, &sc);

          FPluginFile.SaveToXLFile( PluginFile );
          TBasicApp::GetLog() << ("\rInstallation complete\n");
        }
        else  {
          TBasicApp::GetLog() << (olxstr("Plugin installation failed with error code: ") << res);
          TBasicApp::GetLog() << api.GetLog();
        }
        FXApp->Draw();
      }
      else  {
        TBasicApp::GetLog() << ("Could not locate usettings.dat file\n");
      }
    }
  }
  else  {
    TDataItem* di = FPluginItem->FindItem( Cmds[0] );
    if( di != NULL )  {
      TStateChange sc(prsPluginInstalled, false, Cmds[0]);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      Macros.ProcessMacro( olxstr("uninstallplugin ") << Cmds[0], E );
    }
    else  {
      TBasicApp::GetLog() << ( olxstr("Specified plugin does not exist: ") << Cmds[0] << '\n' );
    }
  }
}
//..............................................................................
void TMainForm::macSignPlugin(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TDataItem* di = FPluginItem->FindItem( Cmds[0] );
  if( di )  di->AddField("signature", Cmds[1] );
  FPluginFile.SaveToXLFile( PluginFile );
}
//..............................................................................
// local linkage (for classes within functions) is not supported by Borland, though MSVC is fine
  class TFSTraverser {
    olxstr Property, BaseDir;
    TGXApp* xa;
    TPtrList<TFSItem> ToDelete;
    public:
      TFSTraverser(TGXApp& xapp, const olxstr& baseDir, const olxstr& prop)  {
        Property = prop;
        BaseDir = baseDir;
        xa = &xapp;
      }
      ~TFSTraverser()  {
        TPtrList<TFSItem> FoldersToDelete;
        for( size_t i=0; i < ToDelete.Count(); i++ )
          if( !ToDelete[i]->IsFolder() && ToDelete[i]->GetParent() != NULL )  {
            if( FoldersToDelete.IndexOf( ToDelete[i]->GetParent() ) == InvalidIndex )
              FoldersToDelete.Add( ToDelete[i]->GetParent() );
            ToDelete[i]->GetParent()->Remove( *ToDelete[i] );
            ToDelete.Delete(i);
            i--;
          }
        while( true )  {
          bool deleted = false;
          for( size_t i=0; i < FoldersToDelete.Count(); i++ )  {
            if( FoldersToDelete[i]->IsEmpty() )  {
              olxstr path = FoldersToDelete[i]->GetIndexFS().GetBase() + FoldersToDelete[i]->GetFullName(),
              cpath = path.CommonString(path, BaseDir);
              TBasicApp::GetLog() << ( olxstr("\rDeleting folder /~/") << path.SubStringFrom(cpath.Length()) );
              xa->Draw();
              wxTheApp->Dispatch();
              TEFile::RmDir( path );
              deleted = true;
              FoldersToDelete[i]->GetParent()->Remove( *FoldersToDelete[i] );
              FoldersToDelete.Delete(i);
              i--;
            }
          }
          if( !deleted )  break;
        }
      }
      bool OnItem(TFSItem& it) {
        if( it.HasProperty(Property) )  {
          olxstr path = it.GetIndexFS().GetBase() + it.GetFullName(),
                   cpath = path.CommonString(path, BaseDir);
          TBasicApp::GetLog() << ( olxstr("\rDeleting /~/") << path.SubStringFrom(cpath.Length()) );
          xa->Draw();
          wxTheApp->Dispatch();
          TEFile::DelFile( path );
          ToDelete.Add( &it );
        }
        return true;
      }
  };
void TMainForm::macUninstallPlugin(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds[0].StartsFrom("olex") )  {
    E.ProcessingError(__OlxSrcInfo, "cannot uninstall core components");
    return;
  }
  TDataItem* di = FPluginItem->FindItem( Cmds[0] );
  if( di != NULL )  {
    TStateChange sc(prsPluginInstalled, false);
    FPluginItem->DeleteItem( di );
    OnStateChange.Execute((AEventsDispatcher*)this, &sc);
    olxstr indexFile = TBasicApp::GetBaseDir() + "index.ind";
    if( TEFile::Exists(indexFile) )  {
      TOSFileSystem osFS( TBasicApp::GetBaseDir() );
      TFSIndex fsIndex( osFS );

      fsIndex.LoadIndex( indexFile );
      TFSTraverser* trav = new TFSTraverser(*FXApp, TBasicApp::GetBaseDir(), olxstr("plugin-") << Cmds[0]);
      TFSItem::Traverser.Traverse<TFSTraverser>(fsIndex.GetRoot(), *trav );
      delete trav;
      fsIndex.SaveIndex( indexFile );
      TBasicApp::GetLog() << "\rUninstallation complete\n";
      FXApp->Draw();
    }
  }
  FPluginFile.SaveToXLFile( PluginFile );
}
//..............................................................................
void TMainForm::funIsPluginInstalled(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( FPluginItem->ItemExists(Params[0]) );
}
//..............................................................................
void TMainForm::funValidatePlugin(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( true );
}
//..............................................................................
void TMainForm::macUpdateFile(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
#ifndef __WIN32__  // skip updating launch or other win files not on win
  olxstr ext(TEFile::ExtractFileExt(Cmds[0]));
  if( ext.Equalsi("exe") || ext.Equalsi("dll") || ext.Equalsi("pyd") )  {
    TBasicApp::GetLog() << "Skipping windows files update...\n";
    return;
  }
#endif
  olxstr SettingsFile( TBasicApp::GetBaseDir() + "usettings.dat" );
  TEFile::CheckFileExists( __OlxSourceInfo, SettingsFile );
  const TSettingsFile settings(SettingsFile);
  olxstr Proxy, Repository;
  bool Force = Options.Contains('f');

  Proxy = settings["proxy"];
  Repository = settings["repository"];
  if( settings.GetParam("update").Equalsi("never") )  {
    TBasicApp::GetLog() << (olxstr("User settings prevented updating file: ") << Cmds[0]);
    return;
  }
  if( !Repository.IsEmpty() && !Repository.EndsWith('/') )  Repository << '/';

  TUrl url(Repository);
  if( !Proxy.IsEmpty() ) 
    url.SetProxy( Proxy );

  THttpFileSystem httpFS( url );
  TOSFileSystem osFS( TBasicApp::GetBaseDir() );
  TFSIndex fsIndex( httpFS );

  IEObject* Cause = NULL;
  try  {
    if( fsIndex.UpdateFile(osFS, Cmds[0], Force) )
      TBasicApp::GetLog() << ( olxstr("Updated '") << Cmds[0] << '\'' << '\n');
    else
      TBasicApp::GetLog() << ( olxstr("Up-to-date '") << Cmds[0] << '\'' << '\n');
  }
  catch( const TExceptionBase& exc )  {  Cause = exc.Replicate();  }
  if( Cause != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, Cause);
}
//..............................................................................
void TMainForm::macNextSolution(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( ((FMode&mSolve) == mSolve) )  {
    ChangeSolution( CurrentSolution - 1 );
    return;
  }
}
//..............................................................................
//..............................................................................
double MatchAtomPairsQT(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                        smatdd& res, bool TryInversion, double (*weight_calculator)(const TSAtom&))  {
  if( atoms.Count() < 3 )  return -1;
  double rms = TNetwork::FindAlignmentMatrix(atoms, res, TryInversion, weight_calculator);
  TBasicApp::GetLog() << ( olxstr("RMS is ") << olxstr::FormatFloat(3, rms) << " A\n");
  return rms;
}
//..............................................................................
bool MatchConsiderNet(const TNetwork& net)  {
  return !(net.NodeCount() < 3 || net.Node(0).CAtom().IsDetached());
}
//..............................................................................
double MatchAtomPairsQTEsd(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                        smatdd& res, bool TryInversion, double (*weight_calculator)(const TSAtom&))
{
  if( atoms.Count() < 3 )  return -1;
  VcoVContainer vcovc;
  TXApp& xapp = TXApp::GetInstance();
  vcovc.ReadShelxMat( TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "mat"), 
    xapp.XFile().GetAsymmUnit() );

  TSAtomPList atoms_out;
  vec3d_alist crds_out;
  TDoubleList wghts_out;
  TNetwork::PrepareESDCalc(atoms, TryInversion, atoms_out, crds_out ,wghts_out, weight_calculator);
  TEValue<double> rv = vcovc.CalcAlignmentRMSD(atoms_out, crds_out, wghts_out);
  TBasicApp::GetLog() << ( olxstr("RMS is ") << rv.ToString() << " A\n");
  double rms = TNetwork::FindAlignmentMatrix(atoms, res, TryInversion, weight_calculator);
  return rms;
}
//..............................................................................
void TMainForm::CallMatchCallbacks(TNetwork& netA, TNetwork& netB, double RMS)  {
  olxstr arg;
  TStrObjList callBackArg;
  for( size_t i=0; i < netA.NodeCount(); i++ )  {
    arg << netA.Node(i).GetLabel();
    if( (i+1) < netA.NodeCount() )  arg << ',';
  }
  callBackArg.Add(RMS);
  callBackArg.Add(arg);
  arg = EmptyString;
  for( size_t i=0; i < netB.NodeCount(); i++ )  {
    arg << netB.Node(i).GetLabel();
    if( (i+1) < netB.NodeCount() )  arg << ',';
  }
  callBackArg.Add(arg);
  CallbackFunc(OnMatchCBName, callBackArg);
}
//..............................................................................
void TMainForm::macMatch(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TActionQueue* q_draw = FXApp->ActionQueue(olxappevent_GL_DRAW);
  if( q_draw != NULL )  q_draw->SetEnabled(false);
  // restore if already applied
  TLattice& latt = FXApp->XFile().GetLattice();
  const TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  latt.RestoreADPs();
  FXApp->UpdateBonds();
  FXApp->CenterView();
  if( Options.Contains("u") )  {
    if( q_draw != NULL )  q_draw->SetEnabled(true);
    return;
  }
  CallbackFunc(StartMatchCBName, EmptyString);
  const bool TryInvert = Options.Contains("i");
  TXAtomPList atoms;
  double (*weight_calculator)(const TSAtom&) = &TNetwork::weight_occu;
  if( Options.FindValue('c', "geom") == "mass" )
    weight_calculator = &TNetwork::weight_occu_aw;
  const bool subgraph = Options.Contains("s");
  olxstr suffix = Options.FindValue("n");
  const bool name = Options.Contains("n");
  const bool align = Options.Contains("a");
  FindXAtoms(Cmds, atoms, false, true);
  if( !atoms.IsEmpty() )  {
    if( atoms.Count() == 2 && (&atoms[0]->Atom().GetNetwork() != &atoms[1]->Atom().GetNetwork()) )  {
      TTypeList<AnAssociation2<size_t, size_t> > res;
      TSizeList sk;
      TNetwork &netA = atoms[0]->Atom().GetNetwork(),
               &netB = atoms[1]->Atom().GetNetwork();
      bool match = subgraph ? netA.IsSubgraphOf( netB, res, sk ) :
                              netA.DoMatch( netB, res, TryInvert, weight_calculator);
      TBasicApp::GetLog() << ( olxstr("Graphs match: ") << match << '\n' );
      if( match )  {
        // restore the other unit cell, if any...
        if( &latt != &netA.GetLattice() || &latt != &netB.GetLattice() )  {
          TLattice& latt1 = (&latt == &netA.GetLattice()) ? netB.GetLattice() : netA.GetLattice();
          latt1.RestoreADPs();
        }
        TTypeList< AnAssociation2<TSAtom*,TSAtom*> > satomp;
        TSAtomPList atomsToTransform;
        TBasicApp::GetLog() << ("Matching pairs:\n");
        olxstr tmp('=');
        for( size_t i=0; i < res.Count(); i++ )  {
          tmp << '{' << netA.Node( res[i].GetA()).GetLabel() <<
                 ',' << netB.Node( res[i].GetB()).GetLabel() << '}';

          if( atomsToTransform.IndexOf( &netB.Node(res[i].GetB()) ) == InvalidIndex )  {
            atomsToTransform.Add( &netB.Node( res[i].GetB()) );
            satomp.AddNew<TSAtom*,TSAtom*>(&netA.Node( res[i].GetA()), &netB.Node( res[i].GetB()));
          }
        }
        if( name )  {
          if( suffix.Length() > 1 && suffix.CharAt(0) == '$' )  {  // change CXXX to CSuffix+whatever left of XXX
            olxstr new_l;
            const olxstr l_val = suffix.SubStringFrom(1);
            for( size_t i=0; i < res.Count(); i++ )  {
              const olxstr& old_l = netA.Node(res[i].GetA()).GetLabel();
              const cm_Element& elm = netA.Node(res[i].GetA()).GetType();
              const index_t l_d = old_l.Length() - elm.symbol.Length();
              if( l_d <= (index_t)l_val.Length() ) 
                new_l = elm.symbol + l_val;
              else if( l_d > (index_t)l_val.Length() )
                new_l = olxstr(elm.symbol) << l_val << old_l.SubStringFrom(elm.symbol.Length()+l_val.Length());
              netB.Node(res[i].GetB()).CAtom().SetLabel(new_l, false);
            }
          }
          else if( suffix.Length() > 1 && suffix.CharAt(0) == '-' )  {  // change the ending
            olxstr new_l;
            const olxstr l_val = suffix.SubStringFrom(1);
            for( size_t i=0; i < res.Count(); i++ )  {
              const olxstr& old_l = netA.Node(res[i].GetA()).GetLabel();
              const cm_Element& elm = netA.Node(res[i].GetA()).GetType();
              const index_t l_d = old_l.Length() - elm.symbol.Length();
              if( l_d <= (index_t)l_val.Length() ) 
                new_l = elm.symbol + l_val;
              else if( l_d > (index_t)l_val.Length() )
                new_l = old_l.SubStringTo(old_l.Length()-l_val.Length()) << l_val;
              netB.Node(res[i].GetB()).CAtom().SetLabel(new_l, false);
            }
          }
          else  {
            for( size_t i=0; i < res.Count(); i++ )
              netB.Node(res[i].GetB()).CAtom().SetLabel(netA.Node(res[i].GetA()).GetLabel() + suffix, false);
          }
        }
        smatdd S;
        double rms = -1;
        if( Options.Contains("esd") )
          rms = MatchAtomPairsQTEsd(satomp, S, TryInvert, weight_calculator);
        else
          rms = MatchAtomPairsQT( satomp, S, TryInvert, weight_calculator);
        TBasicApp::GetLog() << ("Transformation matrix B to A):\n");
        for( int i=0; i < 3; i++ )
          TBasicApp::GetLog() << S.r[i].ToString() << ' ' << S.t[i] << '\n' ;
        // execute callback function
        CallMatchCallbacks(netA, netB, rms);
        // ends execute callback
        if( align && rms >= 0 )  {
          TNetwork::DoAlignAtoms(satomp, atomsToTransform, S, TryInvert, weight_calculator);
          FXApp->UpdateBonds();
          FXApp->CenterView();
        }

        TBasicApp::GetLog() << (tmp << '\n');
        if( subgraph )  {
          sk.Add( res[0].GetB() );
          res.Clear();
          while( atoms[0]->Atom().GetNetwork().IsSubgraphOf( atoms[1]->Atom().GetNetwork(), res, sk ) )  {
            sk.Add( res[0].GetB() );
            tmp = '=';
            for( size_t i=0; i < res.Count(); i++ )  {
              tmp << '{' << atoms[0]->Atom().GetNetwork().Node( res[i].GetA()).GetLabel() <<
                     ',' << atoms[1]->Atom().GetNetwork().Node( res[i].GetB()).GetLabel() << '}';
            }
            TBasicApp::GetLog() << (tmp << '\n');
            res.Clear();
          }
        }
      }
    }
    else if( atoms.Count() >= 6 && (atoms.Count()%2) == 0 )  {  // a full basis provided
      TTypeList< AnAssociation2<TSAtom*,TSAtom*> > satomp;
      TSAtomPList atomsToTransform;
      for( size_t i=0; i < atoms.Count()/2; i++ )
        satomp.AddNew<TSAtom*,TSAtom*>(&atoms[i]->Atom(), &atoms[i+atoms.Count()/2]->Atom());
      TNetwork &netA = satomp[0].GetA()->GetNetwork(),
               &netB = satomp[0].GetB()->GetNetwork();
      bool valid = true;
      for( size_t i=1; i < satomp.Count(); i++ )  {
        if( satomp[i].GetA()->GetNetwork() != netA || satomp[i].GetB()->GetNetwork() != netB )  {
          valid = false;
          E.ProcessingError(__OlxSrcInfo, "atoms should belong to two distinct fragments or the same fragment");
          break;
        }
      }
      if( valid )  {
        // restore the other unit cell, if any...
        if( &latt != &netA.GetLattice() || &latt != &netB.GetLattice() )  {
          TLattice& latt1 = (&latt == &netA.GetLattice()) ? netB.GetLattice() : netA.GetLattice();
          latt1.RestoreADPs();
        }
        if( netA != netB )  {  // collect all atoms
          for( size_t i=0; i < netB.NodeCount(); i++ )
            atomsToTransform.Add(netB.Node(i));
        }
        else  {
          for( size_t i=atoms.Count()/2; i < atoms.Count(); i++ )
            atomsToTransform.Add(atoms[i]->Atom());
        }
        smatdd S;
        double rms = MatchAtomPairsQT( satomp, S, TryInvert, weight_calculator);
        TNetwork::DoAlignAtoms(satomp, atomsToTransform, S, TryInvert, weight_calculator);
        FXApp->UpdateBonds();
        FXApp->CenterView();
        CallMatchCallbacks(netA, netB, rms);
      }
    }
  }
  else  {
    TNetPList nets;
    FXApp->GetNetworks(nets);
    TTypeList<AnAssociation2<size_t, size_t> > res;
    TTypeList<AnAssociation2<TSAtom*,TSAtom*> > satomp;
    TSAtomPList atomsToTransform;
    // restore the other unit cell, if any...
    for( size_t i=0; i < nets.Count(); i++ )  {
      if( &latt != &nets[i]->GetLattice() )
        nets[i]->GetLattice().RestoreADPs();
    }
    for( size_t i=0; i < nets.Count(); i++ )  {
      if( !MatchConsiderNet(*nets[i]) )  continue;
      for( size_t j=i+1; j < nets.Count(); j++ )  {
        if( !MatchConsiderNet(*nets[j]) )  continue;
        res.Clear();
        if( nets[i]->DoMatch( *nets[j], res, TryInvert, weight_calculator) )  {
          satomp.Clear();
          atomsToTransform.Clear();
          for( size_t k=0; k < res.Count(); k++ )  {
            if( atomsToTransform.IndexOf( &nets[j]->Node(res[k].GetB()) ) == InvalidIndex )  {
              atomsToTransform.Add( &nets[j]->Node( res[k].GetB()) );
              satomp.AddNew<TSAtom*,TSAtom*>(&nets[i]->Node( res[k].GetA()),
                                             &nets[j]->Node( res[k].GetB()));
            }
          }
          smatdd S;
          double rms = MatchAtomPairsQT(satomp, S, TryInvert, weight_calculator);
          CallMatchCallbacks(*nets[i], *nets[j], rms);
          if( rms >= 0 ) 
            TNetwork::DoAlignAtoms(satomp, atomsToTransform, S, TryInvert, weight_calculator);
        }
      }
    }
    FXApp->UpdateBonds();
    FXApp->CenterView();
    // do match all possible fragments with similar number of atoms
  }
  if( q_draw != NULL )  q_draw->SetEnabled(true);
}
//..............................................................................
void TMainForm::macShowWindow(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 2 )  {
    if( Cmds[0].Equalsi("help") )  {
      HelpWindowVisible = Cmds[1].ToBool();
      FHelpWindow->SetVisible( HelpWindowVisible );
      FGlConsole->ShowBuffer( !HelpWindowVisible );  // sync states
      TStateChange sc(prsHelpVis, HelpWindowVisible);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
    } 
    else  if( Cmds[0].Equalsi("info") )  {
      InfoWindowVisible = Cmds[1].ToBool();
      FInfoBox->SetVisible( InfoWindowVisible );
      TStateChange sc(prsInfoVis, InfoWindowVisible);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      OnResize();
      FXApp->Draw();
    } 
    else if( Cmds[0].Equalsi("cmdline") )  {
      CmdLineVisible = Cmds[1].ToBool();
      FCmdLine->Show( CmdLineVisible );
      if( CmdLineVisible )  FCmdLine->SetFocus();
      FGlConsole->SetPromptVisible( !CmdLineVisible );
      TStateChange sc(prsCmdlVis, CmdLineVisible);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      OnResize();
      FXApp->Draw();
    }
  }
  else  {
    if( Cmds[0].Equalsi("help") )  {
      HelpWindowVisible = !HelpWindowVisible;
      FHelpWindow->SetVisible( HelpWindowVisible );
      FGlConsole->ShowBuffer( !HelpWindowVisible );  // sync states
      TStateChange sc(prsHelpVis, HelpWindowVisible);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
    } 
    else if( Cmds[0].Equalsi("info") )  {
      InfoWindowVisible = !InfoWindowVisible;
      FInfoBox->SetVisible( InfoWindowVisible );
      TStateChange sc(prsInfoVis, InfoWindowVisible);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      OnResize();
      FXApp->Draw();
    } 
    else if( Cmds[0].Equalsi("cmdline") )  {
      CmdLineVisible = !CmdLineVisible;
      FCmdLine->Show( CmdLineVisible );
      if( CmdLineVisible )  FCmdLine->SetFocus();
      FGlConsole->SetPromptVisible( !CmdLineVisible );
      TStateChange sc(prsCmdlVis, CmdLineVisible);
      OnStateChange.Execute((AEventsDispatcher*)this, &sc);
      OnResize();
      FXApp->Draw();
    }
  }
}
//..............................................................................
void TMainForm::funGetUserInput(const TStrObjList& Params, TMacroError &E) {
  bool MultiLine = Params[0].ToInt() != 1;
  TdlgEdit *dlg = new TdlgEdit(this, MultiLine);
  dlg->SetTitle(Params[1].u_str());
  dlg->SetText(Params[2]);
  if( dlg->ShowModal() == wxID_OK )
    E.SetRetVal(dlg->GetText());
  else
    E.SetRetVal(EmptyString);
  dlg->Destroy();
}
//..............................................................................
void TMainForm::funGetCompilationInfo(const TStrObjList& Params, TMacroError &E) {
  olxstr timestamp(olxstr(__DATE__) << ' ' << __TIME__), revision;
#ifdef _SVN_REVISION_AVAILABLE
  timestamp = compile_timestamp;
  revision = svn_revision_number;
#endif
  if( Params.IsEmpty() )  {
    if( revision.IsEmpty() )
      E.SetRetVal(timestamp);
    else
      E.SetRetVal(timestamp << " svn.r" << revision);
  }
  else  {
    if( Params.Count() == 1 && Params[0].Equalsi("full") )  {
      olxstr rv = timestamp;
      if( !revision.IsEmpty() )  rv << " svn.r" << revision;
#ifdef _MSC_FULL_VER
      rv << " MSC:" << _MSC_FULL_VER;
#elif __INTEL_COMPILER
      rv << " Intel:" << __INTEL_COMPILER;
#elif __GNUC__
      rv << " GCC:" << __GNUC__ << '.' << __GNUC_MINOR__ << '.' << __GNUC_PATCHLEVEL__;
#endif
#ifdef _WIN64
      rv << " on WIN64";
#elif _WIN32
      rv << " on WIN32";
#  if _M_IX86_FP == 0
     rv << " without SSE";
#  elif _M_IX86_FP == 1
     rv << " with SSE";
#  elif _M_IX86_FP == 2
     rv << " with SSE2";
#  endif

#elif __MAC__
      rv << " on MAC";
#elif __linux__
      rv << " on Linux";
#  ifdef __LP64__
      rv << "64";
#  else
      rv << "32";
#  endif
#else
      rv << " on other";
#endif
      E.SetRetVal(rv);
    }
    else  {
      try {  
        time_t date = TETime::ParseDate(__DATE__);
        time_t time = TETime::ParseTime(__TIME__);
        E.SetRetVal<olxstr>(TETime::FormatDateTime(Params[0], date+time));
      }
      catch( TExceptionBase& ) {
        E.SetRetVal(timestamp);
      }
    }
  }
}
//..............................................................................
void TMainForm::macDelOFile(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( FXApp->OverlayedXFileCount() == 0 )  {
    Macros.ProcessMacro("fuse", Error);
    return;
  }
  int ind = Cmds[0].ToInt();
  if( ind <= -1 )  {
    if( FXApp->OverlayedXFileCount() > 0 )
      FXApp->DeleteOverlayedXFile(FXApp->OverlayedXFileCount()-1);
  }
  else  {
    if( (size_t)ind < FXApp->OverlayedXFileCount() )
      FXApp->DeleteOverlayedXFile(ind);
    else
      Error.ProcessingError(__OlxSrcInfo, "no overlayed files at given position");
  }
}
//..............................................................................

class helper_Tetrahedron  {
  vec3d_list Points;
  olxstr Name;
  double Volume;
protected:
  double CalcVolume()  {
    return TetrahedronVolume(Points[0], Points[1], Points[2], Points[3]);
  }
public:
  helper_Tetrahedron(const olxstr& name) : Name(name)  {
    Name = name;
    Volume = -1;
  }
  void AddPoint(const vec3d& p)  {
    Points.AddNew(p);
    if( Points.Count() == 4 )
      Volume = CalcVolume();
  }
  const olxstr& GetName() const {  return Name;  }
  const vec3d& operator [] (size_t i) const {  return Points[i];  }
  double GetVolume() const {  return Volume;  }
  int Compare(const helper_Tetrahedron& th) const {
    const double v = GetVolume() - th.GetVolume();
    return v < 0 ? -1 : (v > 0 ? 1 : 0);
  }
};


void TMainForm::macCalcVol(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList xatoms;
  if( !FindXAtoms(Cmds, xatoms, false, !Options.Contains("cs")) )  {
    Error.ProcessingError(__OlxSrcInfo, "no atom(s) given");
    return;
  }
  bool normalise = Options.Contains('n');
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    TSAtomPList atoms;
    for( size_t j=0; j < xatoms[i]->Atom().NodeCount(); j++ ) {
      TSAtom& A = xatoms[i]->Atom().Node(j);
      if( A.IsDeleted() || (A.GetType() == iQPeakZ) )
        continue;
      atoms.Add(A);
    }
    if( atoms.Count() < 3 ) continue;
    if( normalise )
      TBasicApp::GetLog() << (olxstr("Current atom: ") << xatoms[i]->Atom().GetLabel() << 
      " (volumes for normalised bonds)\n");
    else
      TBasicApp::GetLog() << (olxstr("Current atom: ") << xatoms[i]->Atom().GetLabel() << '\n');
    if( atoms.Count() == 3 )  {
      double sa = Angle(atoms[0]->crd(), xatoms[i]->Atom().crd(), atoms[1]->crd());
      sa += Angle(atoms[0]->crd(), xatoms[i]->Atom().crd(), atoms[2]->crd());
      sa += Angle(atoms[1]->crd(), xatoms[i]->Atom().crd(), atoms[2]->crd());
      TBasicApp::GetLog() << (olxstr("Sum of angles is ") << olxstr::FormatFloat(3,sa) << '\n' );
      double v;
      if( normalise )  {
        v = TetrahedronVolume(
        xatoms[i]->Atom().crd(), 
        xatoms[i]->Atom().crd() + (atoms[0]->crd()-xatoms[i]->Atom().crd()).Normalise(), 
        xatoms[i]->Atom().crd() + (atoms[1]->crd()-xatoms[i]->Atom().crd()).Normalise(), 
        xatoms[i]->Atom().crd() + (atoms[2]->crd()-xatoms[i]->Atom().crd()).Normalise());
      }
      else
        v = TetrahedronVolume(xatoms[i]->Atom().crd(), atoms[0]->crd(), atoms[1]->crd(), atoms[2]->crd());
      TBasicApp::GetLog() << (olxstr("The tetrahedron volume is ") << olxstr::FormatFloat(3,v) << '\n' );
    }
    else if( atoms.Count() == 4 )  {
      double v;
      if( normalise )  {
        v = TetrahedronVolume(
        xatoms[i]->Atom().crd() + (atoms[0]->crd()-xatoms[i]->Atom().crd()).Normalise(), 
        xatoms[i]->Atom().crd() + (atoms[1]->crd()-xatoms[i]->Atom().crd()).Normalise(), 
        xatoms[i]->Atom().crd() + (atoms[2]->crd()-xatoms[i]->Atom().crd()).Normalise(), 
        xatoms[i]->Atom().crd() + (atoms[3]->crd()-xatoms[i]->Atom().crd()).Normalise());
      }
      else
        v = TetrahedronVolume(atoms[0]->crd(), atoms[1]->crd(), atoms[2]->crd(), atoms[3]->crd());
      TBasicApp::GetLog() << (olxstr("The tetrahedron volume is ") << olxstr::FormatFloat(3,v) << '\n' );
    }
    else  {
      TTypeList<helper_Tetrahedron> tetrahedra;
      for( size_t i1=0; i1 < atoms.Count(); i1++ ) {
        for( size_t i2=i1+1; i2 < atoms.Count(); i2++ ) {
          for( size_t i3=i2+1; i3 < atoms.Count(); i3++ ) {
            helper_Tetrahedron& th = tetrahedra.AddNew(olxstr(xatoms[i]->Atom().GetLabel()) << '-'
              << atoms[i1]->GetLabel() << '-' << atoms[i2]->GetLabel() << '-' << atoms[i3]->GetLabel());
            if( normalise )  {
              th.AddPoint(xatoms[i]->Atom().crd());
              th.AddPoint(xatoms[i]->Atom().crd() + (atoms[i1]->crd()-xatoms[i]->Atom().crd()).Normalise());
              th.AddPoint(xatoms[i]->Atom().crd() + (atoms[i2]->crd()-xatoms[i]->Atom().crd()).Normalise());
              th.AddPoint(xatoms[i]->Atom().crd() + (atoms[i3]->crd()-xatoms[i]->Atom().crd()).Normalise());
            }
            else  {
              th.AddPoint(xatoms[i]->Atom().crd());
              th.AddPoint(atoms[i1]->crd());
              th.AddPoint(atoms[i2]->crd());
              th.AddPoint(atoms[i3]->crd());
            }
          }
        }
      }
      const size_t thc = (atoms.Count()-2)*2;
      tetrahedra.QuickSorter.Sort<TComparableComparator>(tetrahedra);
      for( size_t j=0; j < tetrahedra.Count(); j++ )  {
        TBasicApp::GetLog() << (olxstr("Tetrahedron ") << j+1 <<  ' ' << tetrahedra[j].GetName() 
          << " V = " << tetrahedra[j].GetVolume() << '\n');
      }
      while(  tetrahedra.Count() > thc )
        tetrahedra.Delete(0);
      double v = 0;
      for( size_t j=0; j < tetrahedra.Count(); j++ )
        v += tetrahedra[j].GetVolume();
      TBasicApp::GetLog() << ( olxstr("The sum of volumes of ") << thc << " largest tetrahedra is " << olxstr::FormatFloat(3,v) << '\n' );
    }
  }
}
//..............................................................................
void TMainForm::funTranslatePhrase(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal(TranslatePhrase(Params[0]));
}
//..............................................................................
void TMainForm::funCurrentLanguageEncoding(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( Dictionary.GetCurrentLanguageEncodingStr() );
}
//..............................................................................
void TMainForm::funIsCurrentLanguage(const TStrObjList& Params, TMacroError &E) {
  TStrList toks;
  toks.Strtok(Params[0], ';');
  for( size_t i=0; i < toks.Count(); i++ )  {
    if( toks[i] == Dictionary.GetCurrentLanguage() )  {
      E.SetRetVal( true );
      return;
    }
  }
  E.SetRetVal( false );
}
//..............................................................................
void TMainForm::macSchedule(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !Cmds[0].IsNumber() )  {
    Error.ProcessingError(__OlxSrcInfo, "invalid syntax: <interval 'task'> are expected ");
    return;
  }
  bool repeatable = false;
  for( size_t i=0; i < Options.Count(); i++ )
    if( Options.GetName(i)[0] == 'r' )  {  repeatable = true;  break;  }

  TScheduledTask& task = Tasks.AddNew();
  task.Repeatable= repeatable;
  task.Interval = Cmds[0].ToInt();
  task.Task = Cmds[1];
  task.LastCalled = TETime::Now();
}
//..............................................................................
void TMainForm::funSGList(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( GetSGList() );
}
//..............................................................................
void TMainForm::macTls(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  double cellParameters[12];
  for( int i = 0; i < 3; i++ )  {
    cellParameters[i]= FXApp->XFile().GetAsymmUnit().Axes()[i].GetV();
    cellParameters[i+3]= FXApp->XFile().GetAsymmUnit().Angles()[i].GetV();
    cellParameters[i+6]= FXApp->XFile().GetAsymmUnit().Axes()[i].GetE();
    cellParameters[i+9]= FXApp->XFile().GetAsymmUnit().Angles()[i].GetE();
  }
  TSAtomPList satoms;
  TXAtomPList xatoms;
  FXApp->FindXAtoms(Cmds.Text(' '), xatoms);
  TListCaster::POP(xatoms, satoms);
  if( !satoms.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "atom count");
  xlib::TLS tls(satoms, cellParameters);

  TTTable<TStrList> tab(12, 3);
  olxstr ttitle("TLS analysis for: ");
  for( size_t i=0; i < satoms.Count(); i++ )  {
    ttitle << satoms[i]->GetLabel();
    if( (i+1) < satoms.Count() )
      ttitle << ", ";
  }
  tab[0][0] = "T";
  for( size_t i=0; i < 3; i++ )  {
    tab[1][i] = olxstr::FormatFloat( -3, tls.GetT()[i][0] );
    tab[2][i] = olxstr::FormatFloat( -3, tls.GetT()[i][1] );
    tab[3][i] = olxstr::FormatFloat( -3, tls.GetT()[i][2] );
  }
  tab[4][0] = "L";
  for( size_t i=0; i < 3; i++ )  {
    tab[5][i] = olxstr::FormatFloat( -3, tls.GetL()[i][0] );
    tab[6][i] = olxstr::FormatFloat( -3, tls.GetL()[i][1] );
    tab[7][i] = olxstr::FormatFloat( -3, tls.GetL()[i][2] );
  }
  tab[8][0] = "S";
  for( size_t i=0; i < 3; i++ )  {
    tab[9][i] = olxstr::FormatFloat( -3, tls.GetS()[i][0] );
    tab[10][i] = olxstr::FormatFloat( -3, tls.GetS()[i][1] );
    tab[11][i] = olxstr::FormatFloat( -3, tls.GetS()[i][2] );
  }
  TStrList output;
  tab.CreateTXTList(output, ttitle, false, false, ' ');
  TBasicApp::GetLog() << ( output );
}
//..............................................................................
void TMainForm::funChooseElement(const TStrObjList& Params, TMacroError &E) {
  TPTableDlg *Dlg = new TPTableDlg(this);
  if( Dlg->ShowModal() == wxID_OK )
    E.SetRetVal(Dlg->GetSelected()->symbol);
  else
    E.SetRetVal(EmptyString );
  Dlg->Destroy();
}
//..............................................................................
void TMainForm::funChooseDir(const TStrObjList& Params, TMacroError &E) {
  olxstr title = "Choose directory",
           defPath = TEFile::CurrentDir();
  if( Params.Count() > 0 )  title = Params[0];
  if( Params.Count() > 1 )  defPath = Params[1];
  wxDirDialog dlg(this, title.u_str(), defPath.u_str());
  if( dlg.ShowModal() == wxID_OK )
    E.SetRetVal<olxstr>(dlg.GetPath().c_str());
  else
    E.ProcessingError(__OlxSrcInfo, EmptyString);
}
//..............................................................................
void TMainForm::funStrDir(const TStrObjList& Params, TMacroError &E) {
  E.SetRetVal( GetStructureOlexFolder().SubStringFrom(0,1) );
}
//..............................................................................

#ifndef __BORLANDC__
class Test_BTreeTraverser {
  int Y, Z;
  bool ZSet, YSet;
public:
  Test_BTreeTraverser()  {  Z = Y = -1;  ZSet = YSet = false;  }
  void SetZ(int v)  {  Z = v;  ZSet = true;  }
  void SetY(int v)  {  Y = v;  YSet = true;  }
  bool OnItem(const BTree<int,int>::Entry* en)  {
    if( ZSet && YSet )  {
      TBasicApp::GetLog() << (olxstr("pair ") << '{' << en->key << ',' << Y << ','
        << Z << '}' << '=' << en->val << '\n');
    }
    else if( YSet )  {
      TBasicApp::GetLog() << (olxstr("pair ") << '{' << en->key << ',' << Y << '}'
        << '=' << en->val << '\n');
    }
    else
      TBasicApp::GetLog() << (olxstr("pair ") << '{' << en->key << '}'
        << '=' << en->val << '\n');
    ZSet = YSet = false;
    return true;
  }
};
#endif

struct SpAtom  {
  vec3f center;
  float r;
};

void TMainForm::macTest(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TTypeList<vec3f> verts;
  TTypeList<GlTriangle> triags;
  GlSphereEx().Generate(1.0, 5, verts, triags);
  TXAtomPList atoms;
  if( !FindXAtoms(Cmds, atoms, true, true) )
    return;

  double volume_p = 0, volume_a = 0, area_p = 0;
  TArrayList<SpAtom> satoms(atoms.Count());
  TArrayList<int8_t> t_map(atoms.Count()*triags.Count());
  for( size_t i=0; i < atoms.Count(); i++ )  {
    const double r = atoms[i]->Atom().GetType().r_vdw;
    satoms[i].center = atoms[i]->Atom().crd();
    satoms[i].r = r;
    volume_p += SphereVol(r);
    area_p += 4*M_PI*r*r;
    const size_t off = i*triags.Count();
    for( size_t j=0; j < triags.Count(); j++ )  {
      t_map[j+off] = 3;
      volume_a += olx_abs((verts[triags[j].verts[0]]*r).DotProd(
        (verts[triags[j].verts[1]]*r).XProdVec(
        (verts[triags[j].verts[2]]*r))));
    }
  }
  volume_a /= 6;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    const SpAtom& a1 = satoms[i];
    const float r_sq = a1.r*a1.r;
    for( size_t j=0; j < atoms.Count(); j++ )  {
      if( i == j )  continue;
      const SpAtom& a2 = satoms[j];
      const float dist = a1.center.DistanceTo(a2.center);
      if( dist >= (a1.r+a2.r) )  continue;
      const size_t off = triags.Count()*j;
      for( size_t k=0; k < triags.Count(); k++ )  {
        if( t_map[k+off] == 0 )  continue;
        const float d[] = {
          (verts[triags[k].verts[0]]*a2.r+a2.center).QDistanceTo(a1.center),
          (verts[triags[k].verts[1]]*a2.r+a2.center).QDistanceTo(a1.center),
          (verts[triags[k].verts[2]]*a2.r+a2.center).QDistanceTo(a1.center)
        };
        if( d[0] < r_sq && d[1] < r_sq && d[2] < r_sq )
          t_map[k+off] = 0;
        else if( (d[0] < r_sq && (d[1] < r_sq || d[2] < r_sq)) || (d[1] < r_sq && d[2] < r_sq) )
          t_map[k+off] = 1;
        else if( d[0] < r_sq || d[1] < r_sq || d[2] < r_sq )
          t_map[k+off] = 2;
        
      }
    }
  }
  double mol_vol_x = 0, mol_vol_y = 0, mol_vol_z = 0, mol_area = 0;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    const size_t off = triags.Count()*i;
    for( size_t j=0; j < triags.Count(); j++ )  {
      if( t_map[off+j] == 0 )  continue;
      const vec3f v1 = verts[triags[j].verts[0]]*satoms[i].r,
                  v2 = verts[triags[j].verts[1]]*satoms[i].r,
                  v3 = verts[triags[j].verts[2]]*satoms[i].r;
      vec3f dp = (v2-v1).XProdVec(v3-v1);
      const float area = 0.5*dp.Length();
      mol_area += area;
      const float dx21 = v2[0]-v1[0],
        dx31 = v3[0]-v1[0],
        dy21 = v2[1]-v1[1],
        dy31 = v3[1]-v1[1],
        dz21 = v2[2]-v1[2],
        dz31 = v3[2]-v1[2];
      const float m = 1.0f/2*(float)t_map[off+j]/3.0f;
      mol_vol_z += m*(1./3*(v1[2]+v2[2]+v3[2])+satoms[i].center[2])*(dx21*dy31-dy21*dx31);
      mol_vol_y += m*(1./3*(v1[1]+v2[1]+v3[1])+satoms[i].center[1])*(dz21*dx31-dx21*dz31);
      mol_vol_x += m*(1./3*(v1[0]+v2[0]+v3[0])+satoms[i].center[0])*(dy21*dz31-dz21*dy31);
    }
  }
  TBasicApp::GetLog() << (olxstr("Molecular volume: ") << olxstr::FormatFloat(2, (mol_vol_x+mol_vol_y+mol_vol_z)/3) << '\n');
  return;
  //cif_dp::TCifDP cdp;
  //TStrList _sl;
  //_sl.LoadFromFile("cif_core.cif");
  //cdp.LoadFromStrings(_sl);
  //_sl.Clear();
  //cdp.SaveToStrings(_sl);
  //TCStrList(_sl).SaveToFile("test_cif");
  //return;

  //uint64_t test_a = 1021;
  //uint64_t test_b = test_a%10, test_c = test_a/10;
  //uint64_t test_d = test_a/10, test_e = test_a-test_d*10;
  FXApp->SetActiveXFile(0);
  //TAsymmUnit& _au = FXApp->XFile().GetAsymmUnit();
  //TUnitCell& uc = FXApp->XFile().GetUnitCell();
  //size_t ac = (size_t)olx_round((uc.CalcVolume()/18.6)/uc.MatrixCount());
  //TPSTypeList<double, TCAtom*> atoms;
  //for( size_t i=0; i < _au.AtomCount(); i++ )
  //  atoms.Add(_au.GetAtom(i).GetUiso(), &_au.GetAtom(i));
  //if( ac < _au.AtomCount() )  {
  //  size_t df = _au.AtomCount() - ac;
  //  for( size_t i=0; i < df; i++ )  {
  //    atoms.GetObject(atoms.Count()-i-1)->SetDeleted(true);
  //  }
  //  FXApp->XFile().EndUpdate();
  //}
  //return;
  
  //wxImage img;
  //img.LoadFile(wxT("c:/tmp/tex2d.jpg"));
  //int tex_id = FXApp->GetRender().GetTextureManager().Add2DTexture("shared_site", 1, img.GetWidth(), img.GetHeight(), 0, GL_RGB, img.GetData());
  //if( tex_id != -1 )  {
  //  TXAtomPList xatoms;
  //  FindXAtoms(Cmds, xatoms, true, true);
  //  for( size_t i=0; i < xatoms.Count(); i++ )  {
  //    xatoms[i]->Primitives()->Primitive(0)->Texture(tex_id); 
  //  }
  //}
  //return;
  olxstr hklfn = FXApp->LocateHklFile();
  if( TEFile::Exists(hklfn) )  {
    const TRefPList& fpp = FXApp->XFile().GetRM().GetFriedelPairs();
    if( fpp.IsEmpty() )  return;

    TRefList refs, nrefs, fp;
    fp.SetCapacity(fpp.Count());
    for( size_t i=0; i < fpp.Count(); i++ )
      fp.AddNew( *fpp[i] );
    const vec3i_list empty_omits;
    smatd_list ml;
    FXApp->XFile().GetLastLoaderSG().GetMatrices(ml, mattAll^mattIdentity);
    MergeStats stat = RefMerger::Merge<smatd_list,RefMerger::ShelxMerger>(ml, fp, refs, empty_omits, false);
    nrefs.SetCapacity(refs.Count());
    for( size_t i=0; i < refs.Count(); i++ ) 
      nrefs.AddNew(refs[i]).GetHkl() *= -1;
    TArrayList<compd> FP(refs.Count()), FN(refs.Count());
    SFUtil::CalcSF(FXApp->XFile(), refs, FP, true);
    SFUtil::CalcSF(FXApp->XFile(), nrefs, FN, true);
    double pscale = SFUtil::CalcFScale(FP, refs);
    double nscale = SFUtil::CalcFScale(FN, nrefs);
    double sx = 0, sy = 0, sxs = 0, sxy = 0;
    const size_t f_cnt = FP.Count();
    for( size_t i=0; i < f_cnt; i++ )  {
      const double I = pscale*pscale*refs[i].GetI();
      const double pqm = FP[i].qmod();
      const double nqm = FN[i].qmod();
      sx += (nqm - pqm);
      sy += (I - pqm);
      sxy += (nqm - pqm)*(I - pqm);
      sxs += (nqm - pqm)*(nqm - pqm);
    }
    double k = (sxy - sx*sy/f_cnt)/(sxs - sx*sx/f_cnt);
    double a = (sy - k*sx)/f_cnt;  
    double sdiff = 0;
    for( size_t i=0; i < f_cnt; i++ )  {
      const double I = pscale*pscale*refs[i].GetI();
      const double pqm = FP[i].qmod();
      const double nqm = FN[i].qmod();
      sdiff += olx_sqr((I - pqm) - k*(nqm - pqm) - a);
    }
    TBasicApp::GetLog() << "K: " << TEValue<double>(k, sqrt(sdiff/(f_cnt*(f_cnt-1)))).ToString() << '\n';
  }
  return;
  TSymmLib& sl = TSymmLib::GetInstance();
  smatd_list ml;
  static const size_t dim = 29;
  bool** cell[dim];
  for( size_t i=0; i < dim; i++ )  {
    cell[i] = new bool*[dim];
    for( size_t j=0; j < dim; j++ )  {
      cell[i][j] = new bool[dim]; 
    }
  }
  vec3d p1;
  vec3i ip;
  for( size_t i=0; i < sl.SGCount(); i++ )  {
    ml.Clear();
    sl.GetGroup(i).GetMatrices(ml, mattAll);
    for( size_t i1=0; i1 < dim; i1++ )
      for( size_t i2=0; i2 < dim; i2++ )
        for( size_t i3=0; i3 < dim; i3++ )
          cell[i1][i2][i3] = false;
    int sets = 0, mi1 = 0, mi2 = 0, mi3 = 0;
    for( size_t i1=0; i1 < dim; i1++ )  {
      const double d1 = (double)i1/dim;
      for( size_t i2=0; i2 < dim; i2++ )  {
        const double d2 = (double)i2/dim;
        for( size_t i3=0; i3 < dim; i3++ )  {
          const double d3 = (double)i3/dim;
          if( cell[i1][i2][i3] )  continue;
          int vc = 0;
          vec3i minv;
          for( size_t l=1; l < ml.Count(); l++)  {  // skip I
            p1 = ml[l] * vec3d(d1,d2,d3);
            p1 *= dim;
            for( size_t k=0; k < 3; k++ )  {
              ip[k] = olx_round(p1[k]);
              while( ip[k] < 0  )   ip[k] += dim;
              while( ip[k] >= dim ) ip[k] -= dim;
            }
            if( cell[ip[0]][ip[1]][ip[2]] )  continue;
            if( vc++ == 0 )
              minv = ip;
            else if( ip.QLength() < minv.QLength() )
              minv = ip;
            cell[ip[0]][ip[1]][ip[2]] = true;
            sets++;
          }
          if( minv[0] > mi1 )  mi1 = minv[0];
          if( minv[1] > mi2 )  mi2 = minv[1];
          if( minv[2] > mi3 )  mi3 = minv[2];
        }
      }
    }
    TBasicApp::GetLog() << sl.GetGroup(i).GetName() << "; mc = " << 
      ml.Count() << " - " << (double)sets*100/(dim*dim*dim) << "% {" << mi1 << ',' << mi2 << ',' << mi3 << "}\n";
  }
  for( size_t i=0; i < dim; i++ )  {
    for( size_t j=0; j < dim; j++ )
      delete [] cell[i][j];
    delete [] cell[i];
  }
    return;
  TEBitArray ba;
  olxstr rr = ba.FormatString(31);
  if( FXApp->XFile().HasLastLoader() )  {
    mat3d h2c = mat3d::Transpose(FXApp->XFile().GetAsymmUnit().GetHklToCartesian());
    TBasicApp::GetLog() << 1./h2c[0][0] << ',' << 1./h2c[1][1] << ',' << 1./h2c[2][2] << '\n';
    mat3d r, I;
    r[0][0] = h2c[0].DotProd(h2c[0]);  r[0][1] = h2c[0].DotProd(h2c[1]);  r[0][2] = h2c[0].DotProd(h2c[2]);
    r[0][1] = r[1][0];                 r[1][1] = h2c[1].DotProd(h2c[1]);  r[1][2] = h2c[1].DotProd(h2c[2]);
    r[2][0] = r[0][2];                 r[2][1] = r[1][2];                 r[2][2] = h2c[2].DotProd(h2c[2]);
    
    mat3d::EigenValues(r, I.I() );
    TBasicApp::GetLog() << r[0][0] << ',' << r[1][1] << ',' << r[2][2] << '\n';
    TBasicApp::GetLog() << 1./r[0][0] << ',' << 1./r[1][1] << ',' << 1./r[2][2] << '\n';
  }
  
  olxstr fn( FXApp->XFile().GetFileName() );
  for( size_t i=0; i < 250; i++ )  {
    Macros.ProcessMacro(olxstr("@reap '") << fn << '\'', Error);
    Dispatch(ID_TIMER, msiEnter, (AEventsDispatcher*)this, NULL);
    FHtml->Update();
    FXApp->Draw();
    wxTheApp->Dispatch();
  }
  return;
  if( !Cmds.IsEmpty() )  {
    TAtomReference ar(Cmds.Text(' '));
    TCAtomGroup ag;
    size_t atomAGroup;
    olxstr unp(ar.Expand(FXApp->XFile().GetRM(), ag, EmptyString, atomAGroup));
    TBasicApp::GetLog() << "Expanding " << ar.GetExpression() << " to atoms \n";
    for( size_t i=0; i < ag.Count(); i++ )
      TBasicApp::GetLog() << ag[i].GetFullLabel(FXApp->XFile().GetRM()) << ' ';
    TBasicApp::GetLog() << "\nUnprocessed instructions " << (unp.IsEmpty() ? olxstr("none") : unp) << '\n';
    return;
  }
#ifndef __BORLANDC__
  BTree<int, int> tree;
  tree.Add(0,0);
  tree.Add(-1,-10);
  tree.Add(1,10);
  tree.Add(-2,-20);
  tree.Add(-2,-21);
  tree.Add(-2,-22);
  tree.Add(-2,-23);
  tree.Add(2,20);
  tree.Add(-3,-30);
  tree.Add(3,30);
  tree.Add(3,31);
  tree.Add(3,32);
  tree.Add(3,33);
  tree.Add(4,40);
  int* tv = NULL;
  tv = tree.Find(0);
  tv = tree.Find(1);
  tv = tree.Find(2);
  tv = tree.Find(3);
  tv = tree.Find(4);
  tv = tree.Find(-1);
  tv = tree.Find(-2);
  tv = tree.Find(-3);
  Test_BTreeTraverser tt;
  tree.Traverser.Traverse(tree, tt);

  BTree2<int,int> tree2;
  tree2.Add(0,0,0);
  tree2.Add(0,1,1);
  tv = tree2.Find(0,1);
  tree2.Traverser.FullTraverse(tree2, tt);

  BTree3<int, int> tree3;
  tree3.Add(0, 0, 0, 0);
  tree3.Add(0, 1, 0, 1);
  tree3.Add(0, 0, 1, 2);
  tree3.Add(1, 0, 1, 3);
  tv = tree3.Find(0, 0, 1);
  tv = tree3.Find(0, 0, 0);
  tv = tree3.Find(0, 1, 0);
  tree3.Traverser.FullTraverse(tree3, tt);
#endif  
  if( Cmds.IsEmpty() )  return;
  const olxcstr atom_s("ATOM");
  TCStrList lst, toks;
  char bf[256];
  char format[] = "%s";
  TIntList AtomF;
  AtomF.Add(6);  //"ATOM  "
  AtomF.Add(5);  //serial number
  AtomF.Add(5);  //name
  AtomF.Add(4);  //residue name
  AtomF.Add(2);  //chain ID      #21
  AtomF.Add(4);  //  residue sequence number #26
  AtomF.Add(12);  // x                        #38
  AtomF.Add(8);  // y
  AtomF.Add(8);  // z
  AtomF.Add(6);  // occupancy
  AtomF.Add(6);  // temperature factor  #66
  AtomF.Add(12);  // element             #78
  AtomF.Add(2);  // charge

  lst.LoadFromFile(Cmds[0]);
  for( size_t i=0; i < lst.Count(); i++ )  {
    if( lst[i].StartsFrom(atom_s) )  {
      toks.Clear();
      toks.Strtok(lst[i], ' ');
      memset(bf, 32, 255);
      toks.Delete( toks.Count()-2 );
      int offset = 0;
      for( size_t j=0; j < olx_min(AtomF.Count(),toks.Count()); j++ )  {
        if( j > 0 )  {
          offset += AtomF[j];
          memcpy(&bf[offset-toks[j].Length()], toks[j].c_str(), toks[j].Length() );
        }
        else if( j == 0 )  {
          memcpy(&bf[offset], toks[j].c_str(), toks[j].Length() );
          offset += AtomF[j];
        }
      }
      bf[offset] = '\0';
      lst[i] = bf;
    }
  }
  lst.Pack();
  lst.SaveToFile( Cmds[0] + ".out" );
  return;
/*
  TStrList lst, toks;
  TEFile sgFile( FXApp->BaseDir() + "sg.txt", "rb" );
  TDataFile df;
  df.LoadFromXLFile( FXApp->BaseDir() + "symmlib.xld" );
  lst.LoadFromStream( sgFile );
  for( size_t i=2; i < lst.Count(); i++ )  {
    toks.Clear();
    olxstr::Strtok( olxstr::RemoveMultiSpaces( lst[i], true ), ' ', toks);
    if( toks.Count() < 7 )  continue;
    olxstr axis, num;
    int si = toks[0].IndexOf(':');
    if( si != -1 )  {
      num = toks[0].SubStringTo(si);
      axis = toks[0].SubStringFrom(si+1);
    }
    else
      num = toks[0];

    for( size_t j=0; j < df.Root().ItemCount(); j++ )  {
      if( df.Root().Item(j).GetFieldValue( "#" ) == num &&
          df.Root().Item(j).GetFieldValue("AXIS") == axis )  {
        TBasicApp::GetLog() << ( olxstr("Found ") << df.Root().Item(j).GetName() );
        olxstr tmp = toks[1];
        tmp << ' ' << toks[2] << ' ' << toks[3] << ' ' << toks[4];
        df.Root().Item(j).SetFieldValue( "FULL", tmp );
        break;
      }
    }
  }
  // validating
  for( size_t i=0; i < df.Root().ItemCount(); i++ )  {
    olxstr tmp = df.Root().Item(i).GetFieldValue("FULL");
    if( tmp.IsEmpty() )  {
      if( df.Root().Item(i).GetName().Length() == 4 )  {
        tmp << df.Root().Item(i).GetName()[0] << ' ' <<
               df.Root().Item(i).GetName()[1] << ' ' <<
               df.Root().Item(i).GetName()[2] << ' ' <<
               df.Root().Item(i).GetName()[3];
        TBasicApp::GetLog() << ( olxstr("Empty, but patched for ") << df.Root().Item(i).GetName() );
      }
      else
        TBasicApp::GetLog() << ( olxstr("Empty val for ") << df.Root().Item(i).GetName() );
      continue;
    }
    toks.Clear();
    olxstr::Strtok( tmp, ' ', toks );
    if( toks.Count() != 4 )  {
      TBasicApp::GetLog() << ( olxstr("Wrong toks count for ") << df.Root().Item(i).GetName() );
      continue;
    }
  }
  // saving
  df.SaveToXLFile( FXApp->BaseDir() + "sg.xld" );
*/
/*
  if( !FXApp->CheckFileType<TIns>() )  return;

  TIns *Ins = (TIns*)FXApp->XFile().GetLastLoader();
  olxstr HklFN = Ins->GetHKLSource();
  if( !TEFile::Exists(HklFN) )  {
    Error.ProcessingError(__OlxSrcInfo, "could not locate HKL file" );
    return;
  }
  TScattererItLib scatlib;

  THklFile Hkl;
  Hkl.LoadFromFile(HklFN);
  TVectorD vec(3);
  TMatrixD M( Ins->GetAsymmUnit().GetHklToCartesian() );

  double minV = 1000, maxV = 0;
  TEFile out( TEFile::ChangeFileExt(Ins->GetFileName(), "out"), "wb+" );
  olxstr tmp;
  Hkl.AnalyseReflections(*TSymmLib::GetInstance().FindSG(Ins->GetAsymmUnit()));
//  Hkl.AnalyseReflections( *TSymmLib::GetInstance().FindGroup("P-1") );
  for( size_t i=0; i < Hkl.RefCount(); i++ )  {
    TReflection* ref = Hkl.Ref(i);
    tmp = ref->ToString();
    tmp << ' ' << ref->IsCentric() << ' ' << ref->GetDegeneracy();
    out.Writenl(tmp);
  }
  double cm = 0, am = 0, integ = 0, meanFs = 0;
  int cc = 0, ac = 0, arc = 0;
  double cd = 0, ad = 0;

  long minH = (long)Hkl.GetMinValues()[0];
  long maxH = (long)Hkl.GetMaxValues()[0];
  long minK = (long)Hkl.GetMinValues()[1];
  long maxK = (long)Hkl.GetMaxValues()[1];
  long minL = (long)Hkl.GetMinValues()[2];
  long maxL = (long)Hkl.GetMaxValues()[2];


  typedef AnAssociation4<double, double, int, double> refc;
  TArray3D<refc*> Hkl3D(minH, maxH, minK, maxK, minL, maxL);

  TTypeList<refc*> allRefs;


  for( size_t i=0; i < Hkl.RefCount(); i++ )  {
    TReflection* ref = Hkl.Ref(i);

    if( olx_abs(ref->Data()[0])/ref->Data()[1] > 3 )
      continue;

    if( ref->Data()[0] < 0 )
      continue;

    refc* ref1 = Hkl3D[ref->H()][ref->K()][ref->L()];
    if( ref1 != NULL )  {
      ref1->A() += ref->Data()[0];
      ref1->B() += QRT(ref->Data()[1]);
      ref1->C() ++;
      ref1->D() += QRT(ref->Data()[0]);
    }
    else
      Hkl3D[ref->H()][ref->K()][ref->L()] =
        new refc(ref->Data()[0], QRT(ref->Data()[1]), 1, QRT(ref->Data()[0]) );
  }

  allRefs.SetCapacity( Hkl.RefCount() );

  for( int i=minH; i < maxH; i++ )  {
    for( int j=minK; j < maxK; j++ )  {
      for( int k=minL; k < maxL; k++ )  {
        refc* ref1 = Hkl3D[i][j][k];
        if( ref1 != NULL )  {
          ref1->A() /= ref1->C();
          ref1->B() /= sqrt(ref1->GetB());
          ref1->D() /= ref1->C();
          ref1->D() -= QRT( ref1->GetA() );
          ref1->D() = sqrt( ref1->GetD() );
          allRefs.AddACopy( ref1 );
        }
      }
    }
  }

  integ = 0;
  meanFs = 0;
  for( size_t i=0; i < allRefs.Count(); i++ )  {
//    meanFs += allRefs[i]->GetA();
    meanFs += allRefs[i]->GetA()/olx_max(allRefs[i]->GetB(), allRefs[i]->GetD());
    //}
    delete allRefs[i];
  }
  if( allRefs.Count() != 0 )
    meanFs /= allRefs.Count();

  for( size_t i=0; i < allRefs.Count(); i++ )
    integ += olx_abs( (allRefs[i]->GetA()/olx_max(allRefs[i]->GetB(), allRefs[i]->GetD()))/meanFs -1 );
//    integ += olx_abs(allRefs[i]->GetA()/meanFs -1 );

  if( allRefs.Count() != 0 )
    integ /= allRefs.Count();
  TBasicApp::GetLog() << (olxstr("Calculated value: ") << integ );
*/
  // qpeak anlysis
  TPSTypeList<double, TCAtom*> SortedQPeaks;
  TDoubleList vals;
  int cnt = 0;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).GetType() == iQPeakZ )  {
      SortedQPeaks.Add(au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
    }
  }
  vals.Add(0);
  for( size_t i=SortedQPeaks.Count()-1; i != InvalidIndex; i-- )  {
    if( (SortedQPeaks.GetComparable(i) - SortedQPeaks.GetComparable(i-1))/SortedQPeaks.GetComparable(i) > 0.1 )  {
      TBasicApp::GetLog() << ( olxstr("Threshold here: ") << SortedQPeaks.GetObject(i)->GetLabel() << '\n' );
      vals.Last() += SortedQPeaks.GetComparable(i);
      cnt++;
      vals.Last() /= cnt;
      cnt = 0;
      vals.Add(0);
      continue;
    }
    vals.Last() += SortedQPeaks.GetComparable(i);
    cnt ++;
  }
  if( cnt != 0 )
    vals.Last() /= cnt;
  for( size_t i=0; i < vals.Count(); i++ )
    TBasicApp::GetLog() << vals[i] << '\n';
}
//..............................................................................
void TMainForm::macLstRes(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TStrList output;
  olxstr Tmp, Tmp1;
  RefinementModel& rm = FXApp->XFile().GetRM();
  // fixed distances
  if( rm.rDFIX.Count() != 0 )  {
    output.Add( olxstr("Restrained distances: ") << rm.rDFIX.Count() );
    for( size_t i=0; i < rm.rDFIX.Count(); i++ )  {
      TSimpleRestraint& sr = rm.rDFIX[i];
      sr.Validate();
      if( sr.AtomCount() == 0 )  continue;
      Tmp = olxstr::FormatFloat(3, sr.GetValue());
      Tmp << ' ' << olxstr::FormatFloat(3, sr.GetEsd()) << ' ';
      for( size_t j=0; j < sr.AtomCount(); j+=2 )  {
        Tmp1 = EmptyString;
        Tmp1 << '[' << sr.GetAtom(j).GetFullLabel(rm) << ',' << sr.GetAtom(j+1).GetFullLabel(rm) << ']';
        Tmp << Tmp1.Format(11, true, ' ');
      }
      output.Add( Tmp );
    }
  }
  // similar distances
  if( rm.rSADI.Count() != 0 )  {
    output.Add( olxstr("Similar distances: ") << rm.rSADI.Count() );
    for( size_t i=0; i < rm.rSADI.Count(); i++ )  {
      TSimpleRestraint& sr = rm.rSADI[i];
      sr.Validate();
      if( sr.AtomCount() == 0 )  continue;
      Tmp = olxstr::FormatFloat(3, sr.GetEsd());
      Tmp << ' ';
      for( size_t j=0; j < sr.AtomCount(); j+=2 )  {
        Tmp1 = EmptyString;
        Tmp1 << '[' << sr.GetAtom(j).GetFullLabel(rm) << ',' << sr.GetAtom(j+1).GetFullLabel(rm) << ']';
        Tmp << Tmp1.Format(11, true, ' ');
      }
      output.Add( Tmp );
    }
  }
  // fixed "angles"
  if( rm.rDANG.Count() != 0 )  {
    output.Add( olxstr("Restrained angles: ") << rm.rDANG.Count() );
    for( size_t i=0; i < rm.rDANG.Count(); i++ )  {
      TSimpleRestraint& sr = rm.rDANG[i];
      sr.Validate();
      if( sr.AtomCount() == 0 )  continue;
      Tmp = olxstr::FormatFloat(3, sr.GetValue());
      Tmp << ' ' << olxstr::FormatFloat(3, sr.GetEsd()) << ' ';
      for( size_t j=0; j < sr.AtomCount(); j+=2 )  {
        Tmp1 = EmptyString;
        Tmp1 << '[' << sr.GetAtom(j).GetFullLabel(rm) << ',' << sr.GetAtom(j+1).GetFullLabel(rm) << ']';
        Tmp << Tmp1.Format(11, true, ' ');
      }
      output.Add( Tmp );
    }
  }
  // fixed chiral atomic volumes
  if( rm.rCHIV.Count() != 0 )  {
    output.Add( olxstr("Restrained 'chiral' volumes: ") << rm.rCHIV.Count() );
    for( size_t i=0; i < rm.rCHIV.Count(); i++ )  {
      TSimpleRestraint& sr = rm.rCHIV[i];
      sr.Validate();
      if( sr.AtomCount() == 0 )  continue;
      Tmp = olxstr::FormatFloat(3, sr.GetValue());
      Tmp << ' ' << olxstr::FormatFloat(3, sr.GetEsd()) << ' ';
      for( size_t j=0; j < sr.AtomCount(); j++ )  {
        Tmp << sr.GetAtom(j).GetFullLabel(rm);
        if( (j+1) < sr.AtomCount() )  Tmp << ", ";
      }
      output.Add( Tmp );
    }
  }
  // planar groups
  if( rm.rFLAT.Count() != 0 )  {
    output.Add( olxstr("Planar groups: ") << rm.rFLAT.Count() );
    for( size_t i=0; i < rm.rFLAT.Count(); i++ )  {
      TSimpleRestraint& sr = rm.rFLAT[i];
      sr.Validate();
      if( sr.AtomCount() < 4 )  continue;
      Tmp = olxstr::FormatFloat(3, sr.GetEsd());
      Tmp << " [";
      for( size_t j=0; j < sr.AtomCount(); j++ )  {
        Tmp << sr.GetAtom(j).GetFullLabel(rm);
        if( (j+1) < sr.AtomCount() )  Tmp << ", ";
      }
      output.Add( Tmp << ']' );
    }
  }
  if( rm.rDELU.Count() != 0 )  {
    output.Add( olxstr("Rigind bond restrains: ") << rm.rDELU.Count() );
  }
  if( rm.rSIMU.Count() != 0 )  {
    output.Add( olxstr("Similar Uij restrains: ") << rm.rSIMU.Count() );
  }
  if( rm.rISOR.Count() != 0 )  {
    output.Add( olxstr("Uij restrains to Uiso: ") << rm.rISOR.Count() );
  }
  if( rm.rEADP.Count() != 0 )  {
    output.Add( olxstr("Equivalent Uij constraints: ") << rm.rEADP.Count() );
  }
  
  TBasicApp::GetLog() << (output);
}
//..............................................................................
void TMainForm::macLstSymm(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TUnitCell& uc = FXApp->XFile().GetLattice().GetUnitCell();

  TTTable<TStrList> tab(uc.MatrixCount(), 2);
  tab.ColName(0) = "Code";
  tab.ColName(1) = "Symm";
  for( size_t i=0; i < uc.MatrixCount(); i++ )  {
    tab[i][0] = TSymmParser::MatrixToSymmCode(uc.GetSymSpace(), uc.GetMatrix(i));
    tab[i][1] = TSymmParser::MatrixToSymm(uc.GetMatrix(i));
  }
  TStrList output;
  tab.CreateTXTList(output, "Unit cell symmetry list", true, true, ' ');
  TBasicApp::GetLog() << (output);
  TimePerFrame = FXApp->Draw();
}
//..............................................................................
void TMainForm::macSgen(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList Atoms;
  smatd_list symm;
  smatd matr;
  for( size_t i=0; i < Cmds.Count(); i++ )  {
    bool validSymm = false;
    if( TSymmParser::IsRelSymm(Cmds[i]) )  {
      try  {
        matr = TSymmParser::SymmCodeToMatrix(FXApp->XFile().GetLattice().GetUnitCell(), Cmds[i]);
        validSymm = true;
      }
      catch( TExceptionBase& )  {}
    }
    if( !validSymm )  {
      try  {
        TSymmParser::SymmToMatrix(Cmds[i], matr);
        validSymm = true;
      }
      catch( TExceptionBase& )  {}
    }
    if( validSymm )  {
      Cmds.Delete(i--);
      symm.AddCCopy(matr);
    }
  }
  if( symm.IsEmpty() )  {
    Error.ProcessingError(__OlxSrcInfo, "no symm code(s) provided");
    return;
  }
  if( !FindXAtoms(Cmds, Atoms, true, true) )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }
  FXApp->Grow(Atoms, symm);
}
//..............................................................................
void TMainForm::macIT(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList xatoms;
  if( !FindXAtoms(Cmds, xatoms, true, true) )  {
    Error.ProcessingError(__OlxSrcInfo, "no atoms provided" );
    return;
  }

  bool orient = Options.Contains("o");

  mat3d I, Iv;
  Iv.I();
  vec3d cent, c;
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    TSAtom& a = xatoms[i]->Atom();
    if( a.GetType() == iQPeakZ )  continue;
    cent += a.crd();
  }
  cent /= xatoms.Count();
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    TSAtom& a = xatoms[i]->Atom();
    if( a.GetType() == iQPeakZ )  continue;
    c = a.crd();
    c -= cent;
    double w = a.GetType().GetMr()*a.CAtom().GetOccu();
    I[0][0] += w*( olx_sqr(c[1]) + olx_sqr(c[2]));
    I[0][1] -= w*c[0]*c[1];
    I[0][2] -= w*c[0]*c[2];
    I[1][0] = I[0][1];
    I[1][1] += w*( olx_sqr(c[0]) + olx_sqr(c[2]));
    I[1][2] -= w*c[1]*c[2];
    I[2][0] = I[0][2];
    I[2][1] = I[1][2];
    I[2][2] += w*( olx_sqr(c[0]) + olx_sqr(c[1]));
  }
  TBasicApp::GetLog() << ("Inertion tensor:\n");
  for( size_t i=0; i < 3; i++ )
    TBasicApp::GetLog() << I[i].ToString() << '\n';
  mat3d::EigenValues(I, Iv);
  TBasicApp::GetLog() << ( olxstr("Ixx =  ") << olxstr::FormatFloat(3, I[0][0]) <<
                                 "  Iyy = "  << olxstr::FormatFloat(3, I[1][1]) <<
                                 "  Izz = "  << olxstr::FormatFloat(3, I[2][2]) << '\n'  );
  TBasicApp::GetLog() << ("Eigen vectors:\n");
  for( size_t i=0; i < 3; i++ )
    TBasicApp::GetLog() << Iv[i].ToString() << '\n';

  if( orient )  {
    vec3d t =  FXApp->GetRender().GetBasis().GetCenter();
     FXApp->GetRender().GetBasis().Orient( Iv, false );
     FXApp->GetRender().GetBasis().SetCenter(t);
  }

}
//..............................................................................
void TMainForm::macStartLogging(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  bool clear = Options.Contains("c");
  if( clear )  {
    olxstr fn = ActiveLogFile->GetName();
    if( ActiveLogFile != NULL )  {
      ActiveLogFile->Delete();
      delete ActiveLogFile;
      ActiveLogFile = NULL;
    }
    else  {
      if( TEFile::Exists(Cmds[0]) )  {
        TEFile::DelFile( Cmds[0] );
      }
    }
  }
  if( ActiveLogFile != NULL )
    return;
  if( TEFile::Exists(Cmds[0]) )
    ActiveLogFile = new TEFile(Cmds[0], "a+b");
  else
    ActiveLogFile = new TEFile(Cmds[0], "w+b");
  ActiveLogFile->Writenl( EmptyString );
  ActiveLogFile->Writenl( olxstr("Olex2 log started on ") << TETime::FormatDateTime( TETime::Now()) );
}
//..............................................................................
void TMainForm::macViewLattice(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !TEFile::Exists( Cmds[0] )  )  {
    Error.ProcessingError(__OlxSrcInfo, "file does not exist" );
    return;
  }
  TBasicCFile* bcf = FXApp->XFile().FindFormat( TEFile::ExtractFileExt(Cmds[0]) );
  if( bcf == NULL )  {
    Error.ProcessingError(__OlxSrcInfo, "unknown file format" );
    return;
  }
  bcf = (TBasicCFile*)bcf->Replicate();
  try  {  bcf->LoadFromFile(Cmds[0]);  }
  catch(const TExceptionBase& exc)  {
    delete bcf;
    throw TFunctionFailedException(__OlxSourceInfo, exc);
  }
  TXLattice& xl = FXApp->AddLattice("xlatt", bcf->GetAsymmUnit().GetCellToCartesian() );
  delete bcf;
  return;
}
//..............................................................................
void main_GenerateCrd(const vec3d_list& p, const smatd_list& sm, vec3d_list& res) {
  vec3d v, t;
  for( size_t i=0; i < sm.Count(); i++ )  {
    v = sm[i] * p[0];
    t.Null();
    for( size_t j=0; j < 3; j++ )  {
      while( v[j] > 1.01 )  {  v[j] -= 1;  t[j] -= 1;  }
      while( v[j] < -0.01 ) {  v[j] += 1;  t[j] += 1;  }
    }
    bool found = false;
    for( size_t j=0; j < res.Count(); j += p.Count() )  { 
      if( v.QDistanceTo( res[j] ) < 0.0001 )  {
        found = true;
        break;
      }
    }
    if( !found )  {
      res.AddCCopy(v);
      for( size_t j=1; j < p.Count(); j++ )  {
        v = sm[i] * p[j];
        v += t;
        res.AddCCopy(v);
      }
    }
  }
  // expand translation
  size_t rc = res.Count();
  for( int x=-1; x <= 1; x++ )  {
    for( int y=-1; y <= 1; y++ )  {
      for( int z=-1; z <= 1; z++ )  {
        if( x == 0 && y == 0 && z == 0 )  continue;
        for( size_t i=0; i < rc; i+= p.Count() )  {
          v = res[i];
          v[0] += x;  v[1] += y;  v[2] += z;
          t.Null();
          for( size_t j=0; j < 3; j++ )  {
            if( v[j] > 1.01 )        {  v[j] -= 1;  t[j] -= 1;  }
            else if( v[j] < -0.01 )  {  v[j] += 1;  t[j] += 1;  }
          }
          bool found = false;
          for( size_t j=0; j < res.Count(); j+=p.Count() )  {  
            if( v.QDistanceTo( res[j] ) < 0.0001 )  {
              found = true;
              break;
            }
          }
          if( !found )  {
            res.AddCCopy(v);
            for( size_t j=1; j < p.Count(); j++ )  {
              v = res[i+j];
              v[0] += x;  v[1] += y;  v[2] += z;
              v += t;
              res.AddCCopy(v);
            }
          }
        }
      }
    }
  }
}

void TMainForm::macAddObject(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( Cmds[0].Equalsi("cell") && Cmds.Count() == 2 )  {
    if( !TEFile::Exists( Cmds[1] )  )  {
      Error.ProcessingError(__OlxSrcInfo, "file does not exist" );
      return;
    }
    TBasicCFile* bcf = FXApp->XFile().FindFormat( TEFile::ExtractFileExt(Cmds[1]) );
    if( bcf == NULL )  {
      Error.ProcessingError(__OlxSrcInfo, "unknown file format" );
      return;
    }
    bcf = (TBasicCFile*)bcf->Replicate();
    try  {  bcf->LoadFromFile(Cmds[1]);  }
    catch(const TExceptionBase& exc)  {
      delete bcf;
      throw TFunctionFailedException(__OlxSourceInfo, exc);
    }
    TDUnitCell* duc = new TDUnitCell(FXApp->GetRender(), olxstr("cell") << (UserCells.Count()+1) );
    double cell[6];
    cell[0] = bcf->GetAsymmUnit().Axes()[0].GetV();
    cell[1] = bcf->GetAsymmUnit().Axes()[1].GetV();
    cell[2] = bcf->GetAsymmUnit().Axes()[2].GetV();
    cell[3] = bcf->GetAsymmUnit().Angles()[0].GetV();
    cell[4] = bcf->GetAsymmUnit().Angles()[1].GetV();
    cell[5] = bcf->GetAsymmUnit().Angles()[2].GetV();
    duc->Init( cell );
    FXApp->AddObjectToCreate( duc );
    TSpaceGroup* sg = TSymmLib::GetInstance().FindSG(bcf->GetAsymmUnit());
    UserCells.Add(AnAssociation2<TDUnitCell*, TSpaceGroup*>(duc, sg));
    duc->Create();
    delete bcf;
  }
  else {
    TDUnitCell* uc = NULL;
    TSpaceGroup* sg = NULL;
    if( !Cmds[2].IsNumber() )  {
      Error.ProcessingError(__OlxSrcInfo, "invalid unit cell reference" );
      return;
    }
    int cr = Cmds[2].ToInt()-1;
    if( cr == -1 )  {
      uc = &FXApp->DUnitCell();
      sg = TSymmLib::GetInstance().FindSG(FXApp->XFile().GetAsymmUnit());
    }
    else if( cr >=0 && (size_t)cr < UserCells.Count() )  {
      uc = UserCells[cr].A();
      sg = UserCells[cr].B();
    }
    else {
      Error.ProcessingError(__OlxSrcInfo, "invalid unit cell reference" );
      return;
    }
    smatd_list ml;
    sg->GetMatrices( ml, mattAll );
    vec3d_list p, allPoints;

    if( Cmds[0].Equalsi("sphere") )  {
      if( (Cmds.Count()-3)%3 != 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "invalid number of arguments" );
        return;
      }
      for( size_t i=3; i < Cmds.Count(); i+= 3 ) 
        p.AddNew(Cmds[i].ToDouble(), Cmds[i+1].ToDouble(), Cmds[i+2].ToDouble());
      main_GenerateCrd(p, ml, allPoints);
      TArrayList<vec3f>& data = *(new TArrayList<vec3f>(allPoints.Count()));
      for( size_t i=0; i < allPoints.Count(); i++ )
        data[i] = allPoints[i] * uc->GetCellToCartesian();
      TDUserObj* uo = new TDUserObj(FXApp->GetRender(), sgloSphere, Cmds[1]);
      uo->SetVertices(&data);
      FXApp->AddObjectToCreate( uo );
      uo->Create();
    }
    else if( Cmds[0].Equalsi("line") )  {
      if( (Cmds.Count()-3)%6 != 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "invalid number of arguments" );
        return;
      }
      for( size_t i=3; i < Cmds.Count(); i+= 6 )  {
        p.AddNew(Cmds[i].ToDouble(), Cmds[i+1].ToDouble(), Cmds[i+2].ToDouble());
        p.AddNew(Cmds[i+3].ToDouble(), Cmds[i+4].ToDouble(), Cmds[i+5].ToDouble());
      }
      main_GenerateCrd(p, ml, allPoints);
      TArrayList<vec3f>& data = *(new TArrayList<vec3f>(allPoints.Count() ));
      for( size_t i=0; i < allPoints.Count(); i++ )
        data[i] = allPoints[i] * uc->GetCellToCartesian();
      TDUserObj* uo = new TDUserObj(FXApp->GetRender(), sgloLines, Cmds[1]);
      uo->SetVertices(&data);
      FXApp->AddObjectToCreate( uo );
      uo->Create();
    }
    else if( Cmds[0].Equalsi("plane") )  {
      if( (Cmds.Count()-3)%12 != 0 )  {
        Error.ProcessingError(__OlxSrcInfo, "invalid number of arguments" );
        return;
      }
      for( size_t i=3; i < Cmds.Count(); i+= 12 )  {
        p.AddNew(Cmds[i].ToDouble(), Cmds[i+1].ToDouble(), Cmds[i+2].ToDouble());
        p.AddNew(Cmds[i+3].ToDouble(), Cmds[i+4].ToDouble(), Cmds[i+5].ToDouble());
        p.AddNew(Cmds[i+6].ToDouble(), Cmds[i+7].ToDouble(), Cmds[i+8].ToDouble());
        p.AddNew(Cmds[i+9].ToDouble(), Cmds[i+10].ToDouble(), Cmds[i+11].ToDouble());
      }
      main_GenerateCrd(p, ml, allPoints);
      TArrayList<vec3f>& data = *(new TArrayList<vec3f>(allPoints.Count() ));
      for( size_t i=0; i < allPoints.Count(); i++ )
        data[i] = allPoints[i] * uc->GetCellToCartesian();
      TDUserObj* uo = new TDUserObj(FXApp->GetRender(), sgloQuads, "user_plane");
      uo->SetVertices(&data);
      FXApp->AddObjectToCreate( uo );
      uo->Create();
    }
    else if( Cmds[0].Equalsi("poly") )  {
      ;
    }
    else
      Error.ProcessingError(__OlxSrcInfo, "unknown object type");
  }
}
//..............................................................................
void TMainForm::macDelObject(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::macTextm(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  if( !TEFile::Exists( Cmds[0] )  )  {
    Error.ProcessingError(__OlxSrcInfo, "file does not exist" );
    return;
  }
  TStrList sl;
  sl.LoadFromFile( Cmds[0] );
  for( size_t i=0; i < sl.Count(); i++ )  {
    Macros.ProcessMacro(sl[i], Error);
    if( !Error.IsSuccessful() )  break;
  }
}
//..............................................................................
void TMainForm::macOnRefine(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
//..............................................................................
class MTTestTh : public AOlxThread  {
  olxcstr msg;
  compd cd_res;
  compf cf_res;
public:
  MTTestTh() {  Detached = false;  }
	int Run()  {
#ifdef _DEBUG
    for( size_t i=0; i < 10000; i++ )  {
#else
    for( size_t i=0; i < 250000; i++ )  {
#endif
      msg = SHA256::Digest(MD5::Digest(msg));
      for( size_t j=0; j < 1000; j++ )  {
        cd_res = (cd_res + compd(1.2, 1.4))*compd(1.00000001, 0.9999999);
        cd_res /= compd(1.001, 0.999);
        cd_res -= 1;
        cf_res = (cf_res + compf(1.2, 1.4))*compf(1.00000001, 0.9999999);
        cf_res /= compf(1.001, 0.999);
        cf_res -= 1;
      }
    }
	  return 0;
	}
	DefPropC(olxcstr, msg);
};

void TMainForm::macTestMT(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  uint64_t times[8], min_t;
	MTTestTh threads[8];
	size_t max_th = 1;
	memset(times, 0, sizeof(uint64_t)*8);
  TBasicApp::GetLog() << ("Testing multithreading compatibility...\n");
  for( size_t i=1; i <= 8; i++ )  {
	  uint64_t st = TETime::msNow();
		for( size_t j=0; j < i; j++ )
		  threads[j].Start();
	  for( size_t j=0; j < i; j++ )
		  threads[j].Join();
		times[i-1] = TETime::msNow() - st;
    if( i == 1 )
      min_t = times[0];
    else if( times[i-1] < min_t )
      min_t = times[i-1];
    TBasicApp::GetLog() << ( olxstr(i) << " threads " << times[i-1] << " ms\n");
		TBasicApp::GetInstance().Update();
		if( i > 1 && ((double)times[i-1]/min_t) > 1.4 )  {
		  max_th = i-1;
			break;
		}
	}
  TBasicApp::GetLog() << ( olxstr("Maximum number of threads is set to ") << max_th << '\n' );
  FXApp->SetMaxThreadCount(max_th);
}
//..............................................................................
void TMainForm::macSetFont(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TwxGlScene& scene = dynamic_cast<TwxGlScene&>(FXApp->GetRender().GetScene());
  TGlFont* glf = scene.FindFont(Cmds[0]);
  if( glf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined font ") << Cmds[0]);
    return;
  }
  TwxGlScene::MetaFont mf(Cmds[1]);
  olxstr ps(Options.FindValue("ps"));
  if( !ps.IsEmpty() )  {
    if( ps.CharAt(0) == '+' || ps.CharAt(0) == '-' )
      mf.SetSize( mf.GetSize() + ps.ToInt() );
    else 
      mf.SetSize(ps.ToInt());
  }
  if( Options.Contains('i') )  mf.SetItalic(true);
  if( Options.Contains('b') )  mf.SetBold(true);
  scene.CreateFont(glf->GetName(), mf.GetIdString());
  if( Cmds[0] == "Picture_labels" )
    FXApp->UpdateLabels();
}

//..............................................................................
void TMainForm::funChooseFont(const TStrObjList &Params, TMacroError &E)  {
  olxstr fntId(EmptyString);
  if( !Params.IsEmpty() && (Params[0].Comparei("olex2") == 0) )
    fntId = TwxGlScene::MetaFont::BuildOlexFontId("olex2.fnt", 12, true, false, false);
  olxstr rv(FXApp->GetRender().GetScene().ShowFontDialog(NULL, fntId));
  E.SetRetVal(rv);
}
//..............................................................................
void TMainForm::funGetFont(const TStrObjList &Params, TMacroError &E)  {
  TGlFont* glf = FXApp->GetRender().GetScene().FindFont(Params[0]);
  if( glf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined font ") << Params[0]);
    return;
  }
  E.SetRetVal(glf->GetIdString());
}
//..............................................................................
void TMainForm::macEditMaterial(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TGlMaterial* mat = NULL, *smat = NULL;
  TGPCollection* gpc = NULL;
  if( Cmds[0] == "helpcmd" )
    mat = &HelpFontColorCmd;
  else if( Cmds[0] == "helptxt" )
    mat = &HelpFontColorTxt;
  else if( Cmds[0] == "execout" )
    mat = &ExecFontColor;
  else if( Cmds[0] == "error" )
    mat = &ErrorFontColor;
  else if( Cmds[0] == "exception" )
    mat = &ExceptionFontColor;
  else  {
    size_t di = Cmds[0].IndexOf('.');
    if( di != InvalidIndex )  {
      TGPCollection* _gpc = FXApp->GetRender().FindCollection(Cmds[0].SubStringTo(di));
      if( _gpc != NULL )  {
        TGlPrimitive* glp = _gpc->FindPrimitiveByName(Cmds[0].SubStringFrom(di+1));
        if( glp != NULL )  {
          mat = &glp->GetProperties();
          smat = &_gpc->GetStyle().GetMaterial(Cmds[0].SubStringFrom(di+1), *mat);
        }
      }
      else  {  // modify the style if exists
        TGraphicsStyle* gs = FXApp->GetRender().GetStyles().FindStyle(Cmds[0].SubStringTo(di));
        if( gs != NULL )
          mat = gs->FindMaterial(Cmds[0].SubStringFrom(di+1));
      }
    }
    else
      gpc = FXApp->GetRender().FindCollection(Cmds[0]);
  }
  if( mat == NULL && gpc == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined material/control ") << Cmds[0]);
    return;
  }
  TdlgMatProp* MatProp = new TdlgMatProp(this, gpc, FXApp);
  if( mat != NULL )
    MatProp->SetCurrent(*mat);

  if( MatProp->ShowModal() == wxID_OK )  {
    if( mat != NULL )
      *mat = MatProp->GetCurrent();
    if( smat != NULL )
      *smat = MatProp->GetCurrent();
  }
  MatProp->Destroy();
}
//..............................................................................
void TMainForm::macSetMaterial(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TGlMaterial* mat = NULL;
  TGlMaterial glm(Cmds[1]);
  if( Cmds[0] == "helpcmd" )
    mat = &HelpFontColorCmd;
  else if( Cmds[0] == "helptxt" )
    mat = &HelpFontColorTxt;
  else if( Cmds[0] == "execout" )
    mat = &ExecFontColor;
  else if( Cmds[0] == "error" )
    mat = &ErrorFontColor;
  else if( Cmds[0] == "exception" )
    mat = &ExceptionFontColor;
  else {
    size_t di = Cmds[0].IndexOf('.');
    bool found = false;
    if( di != InvalidIndex )  {
      TGPCollection* gpc = FXApp->GetRender().FindCollection(Cmds[0].SubStringTo(di));
      if( gpc != NULL )  {
        TGlPrimitive* glp = gpc->FindPrimitiveByName(Cmds[0].SubStringFrom(di+1));
        if( glp != NULL )  {
          glp->SetProperties(glm);
          gpc->GetStyle().SetMaterial(Cmds[0].SubStringFrom(di+1), glm);
          found = true;
        }
      }
      else  {  // modify the style if exists
        TGraphicsStyle* gs = FXApp->GetRender().GetStyles().FindStyle(Cmds[0].SubStringTo(di));
        if( gs != NULL )  {
          mat = gs->FindMaterial(Cmds[0].SubStringFrom(di+1));
          if( mat != NULL )  {
            *mat = glm;
            found = true;
          }
        }
      }
    }
    if( !found )
      E.ProcessingError(__OlxSrcInfo, olxstr("Undefined object ") << Cmds[0]);
    return;
  }
  if( mat == NULL )
    E.ProcessingError(__OlxSrcInfo, olxstr("Undefined material ") << Cmds[0]);
  else
    *mat = glm;
}
//..............................................................................
void TMainForm::funChooseMaterial(const TStrObjList &Params, TMacroError &E)  {
  TGlMaterial glm;
  if( Params.Count() == 1 )
    glm.FromString( Params[0] );
  TdlgMatProp* MatProp = new TdlgMatProp(this, NULL, FXApp);
  MatProp->SetCurrent( glm );

  if( MatProp->ShowModal() == wxID_OK )
    E.SetRetVal( MatProp->GetCurrent().ToString() );
  else
    E.ProcessingError(__OlxSrcInfo, EmptyString);
  MatProp->Destroy();
}
//..............................................................................
void TMainForm::funGetMaterial(const TStrObjList &Params, TMacroError &E)  {
  const TGlMaterial* mat = NULL;
  if( Params[0] == "helpcmd" )
    mat = &HelpFontColorCmd;
  else if( Params[0] == "helptxt" )
    mat = &HelpFontColorTxt;
  else if( Params[0] == "execout" )
    mat = &ExecFontColor;
  else if( Params[0] == "error" )
    mat = &ErrorFontColor;
  else if( Params[0] == "exception" )
    mat = &ExceptionFontColor;
  else  {
    size_t di = Params[0].IndexOf('.');
    if( di != InvalidIndex )  {
      TGPCollection* gpc = FXApp->GetRender().FindCollection(Params[0].SubStringTo(di));
      if( gpc != NULL )  {
        TGlPrimitive* glp = gpc->FindPrimitiveByName(Params[0].SubStringFrom(di+1));
        if( glp != NULL )
          mat = &glp->GetProperties();
      }
      else  {  // check if the style exists
        TGraphicsStyle* gs = FXApp->GetRender().GetStyles().FindStyle(Params[0].SubStringTo(di));
        if( gs != NULL )
          mat = gs->FindMaterial(Params[0].SubStringFrom(di+1));
      }
    }
    else  {
      TGPCollection* gpc = FXApp->GetRender().FindCollection(Params[0]);
      if( gpc != NULL && EsdlInstanceOf(*gpc, TGlGroup) )  {
        mat = &((TGlGroup*)gpc)->GetGlM();
      }
      else  {  // check if the style exists
        TGraphicsStyle* gs = FXApp->GetRender().GetStyles().FindStyle(Params[0]);
        mat = gs->FindMaterial("mat");
      }
    }
  }
  if( mat == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined material ") << Params[0]);
    return;
  }
  else
    E.SetRetVal(mat->ToString());
}
//..............................................................................
void TMainForm::macLstGO(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TStrList output;
  output.SetCapacity( FXApp->GetRender().CollectionCount() );
  for( size_t i=0; i < FXApp->GetRender().CollectionCount(); i++ )  {
    TGPCollection& gpc = FXApp->GetRender().GetCollection(i);
    output.Add( gpc.GetName() ) << '[';
    for( size_t j=0; j < gpc.PrimitiveCount(); j++ )  {
      output.Last().String << gpc.GetPrimitive(j).GetName();
      if( (j+1) < gpc.PrimitiveCount() )
        output.Last().String << ';';
    }
    output.Last().String << "]->" << gpc.ObjectCount();
  }
  TBasicApp::GetLog() << ( output );
}
//..............................................................................
void TMainForm::macCalcPatt(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  // space group matrix list
  TSpaceGroup* sg = NULL;
  try  { sg = &FXApp->XFile().GetLastLoaderSG();  }
  catch(...)  {
    E.ProcessingError(__OlxSrcInfo, "could not locate space group");
    return;
  }
  TUnitCell::SymSpace sp = FXApp->XFile().GetUnitCell().GetSymSpace();
  olxstr hklFileName = FXApp->LocateHklFile();
  if( !TEFile::Exists(hklFileName) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate hkl file");
    return;
  }
  TRefList refs;
  RefinementModel::HklStat stats =
    FXApp->XFile().GetRM().GetFourierRefList<TUnitCell::SymSpace,RefMerger::StandardMerger>(
    sp, refs);
  const double vol = FXApp->XFile().GetLattice().GetUnitCell().CalcVolume();
  TArrayList<SFUtil::StructureFactor> P1SF(refs.Count()*sp.Count());
  size_t index = 0;
  for( size_t i=0; i < refs.Count(); i++ )  {
    const TReflection& ref = refs[i];
    for( size_t j=0; j < sp.Count(); j++, index++ )  {
      P1SF[index].hkl = ref * sp[j];
      P1SF[index].ps = sp[j].t.DotProd(ref.GetHkl());
      P1SF[index].val = sqrt(refs[i].GetI());
      P1SF[index].val *= compd::polar(1, 2*M_PI*P1SF[index].ps);
    }
  }
  const double resolution = 5;
  const vec3i dim(
    (int)(au.Axes()[0].GetV()*resolution),
		(int)(au.Axes()[1].GetV()*resolution),
		(int)(au.Axes()[2].GetV()*resolution));
  FXApp->XGrid().InitGrid(dim);
  BVFourier::MapInfo mi = BVFourier::CalcPatt(P1SF, FXApp->XGrid().Data()->Data, dim, vol);
  FXApp->XGrid().AdjustMap();
  FXApp->XGrid().SetMinVal(mi.minVal);
  FXApp->XGrid().SetMaxVal(mi.maxVal);
  FXApp->XGrid().SetMaxHole( mi.sigma*1.4);
  FXApp->XGrid().SetMinHole(-mi.sigma*1.4);
  FXApp->XGrid().SetScale( -(mi.maxVal - mi.minVal)/2.5 );
  FXApp->XGrid().InitIso();
  FXApp->ShowGrid(true, EmptyString);
}
//..............................................................................
//..............................................................................
void TMainForm::funGetMouseX(const TStrObjList &Params, TMacroError &E)  {
  E.SetRetVal( ::wxGetMousePosition().x );
}
//..............................................................................
void TMainForm::funGetMouseY(const TStrObjList &Params, TMacroError &E)  {
  E.SetRetVal( ::wxGetMousePosition().y );
}
//..............................................................................
void TMainForm::funGetWindowSize(const TStrObjList &Params, TMacroError &E)  {
  if( Params.IsEmpty() || Params[0].Equalsi("main") )  {
    wxRect sz = GetRect();
    E.SetRetVal( olxstr(sz.x) << ',' << sz.y << ',' << sz.width << ',' << sz.height);
  }
  else if( Params[0].Equalsi("gl") ) {
    wxRect sz = FGlCanvas->GetRect();
    E.SetRetVal( olxstr(sz.x) << ',' << sz.y << ',' << sz.width << ',' << sz.height);
  }
  else if( Params[0].Equalsi("html") ) {
    wxRect sz = FHtml->GetRect();
    E.SetRetVal( olxstr(sz.x) << ',' << sz.y << ',' << sz.width << ',' << sz.height);
  }
  else if( Params[0].Equalsi("main-cs") ) {
    if( Params.Count() == 1 )  {
      int w=0, h=0;
      GetClientSize(&w, &h);
      E.SetRetVal( olxstr('0') << ',' << '0' << ',' << w << ',' << h);
    }
    else if( Params.Count() == 3 )  {
      int w=Params[1].ToInt(), h=Params[2].ToInt();
      ClientToScreen(&w, &h);
      E.SetRetVal( olxstr(w) << ',' << h);
    }
  }
  else
    E.ProcessingError(__OlxSrcInfo, "undefined window");
}
//..............................................................................
void TMainForm::macCalcFourier(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
// scale type
  static const short stSimple     = 0x0001,
                     stRegression = 0x0002;
  double resolution = Options.FindValue("r", "0.25").ToDouble(), 
    maskInc = 1.0;
  if( resolution < 0.1 )  resolution = 0.1;
  resolution = 1./resolution;
  short mapType = SFUtil::mapTypeCalc;
  if( Options.Contains("tomc") )
    mapType = SFUtil::mapType2OmC;
  else if( Options.Contains("obs") )
    mapType = SFUtil::mapTypeObs;
  else if( Options.Contains("diff") )
    mapType = SFUtil::mapTypeDiff;
  olxstr strMaskInc = Options.FindValue("m");
  if( !strMaskInc.IsEmpty() )
    maskInc = strMaskInc.ToDouble();
  TRefList refs;
  TArrayList<compd> F;
  olxstr err( SFUtil::GetSF(refs, F, mapType, 
    Options.Contains("fcf") ? SFUtil::sfOriginFcf : SFUtil::sfOriginOlex2, 
    (Options.FindValue("scale", "r").ToLowerCase().CharAt(0) == 'r') ? SFUtil::scaleRegression : SFUtil::scaleSimple) );
  if( !err.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, err);
    return;
  }
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  TUnitCell& uc = FXApp->XFile().GetUnitCell();
  TArrayList<SFUtil::StructureFactor> P1SF;
  SFUtil::ExpandToP1(refs, F, uc.GetMatrixList(), P1SF);
  const double vol = FXApp->XFile().GetLattice().GetUnitCell().CalcVolume();
  BVFourier::MapInfo mi;
// init map
  const vec3i dim(
    (int)(au.Axes()[0].GetV()*resolution),
		(int)(au.Axes()[1].GetV()*resolution),
		(int)(au.Axes()[2].GetV()*resolution));
  TArray3D<float> map(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  mi = BVFourier::CalcEDM(P1SF, map.Data, dim, vol);
//////////////////////////////////////////////////////////////////////////////////////////
  FXApp->XGrid().InitGrid(dim);

  FXApp->XGrid().SetMaxHole(mi.sigma*1.4);
  FXApp->XGrid().SetMinHole(-mi.sigma*1.4);
  //FXApp->XGrid().SetScale( -mi.sigma*4 );
  FXApp->XGrid().SetScale( -(mi.maxVal - mi.minVal)/2.5 );
  FXApp->XGrid().SetMinVal(mi.minVal);
  FXApp->XGrid().SetMaxVal(mi.maxVal);
  // copy map
  MapUtil::CopyMap(FXApp->XGrid().Data()->Data, map.Data, dim);
  FXApp->XGrid().AdjustMap();

  TBasicApp::GetLog() << (olxstr("Map max val ") << olxstr::FormatFloat(3, mi.maxVal) << 
    " min val " << olxstr::FormatFloat(3, mi.minVal) << '\n' << 
    "Map sigma " << olxstr::FormatFloat(3, mi.sigma) << '\n');
  // map integration
  if( Options.Contains('i') )  {
    TArrayList<MapUtil::peak> Peaks;
    TTypeList<MapUtil::peak> MergedPeaks;
    vec3d norm(1./dim[0], 1./dim[1], 1./dim[2]);
    MapUtil::Integrate<float>(map.Data, dim, (mi.maxVal - mi.minVal)/2.5, Peaks);
    //MapUtil::Integrate<float>(map.Data, mapX, mapY, mapZ, mi.sigma*5, Peaks);
    MapUtil::MergePeaks(uc.GetSymSpace(), norm, Peaks, MergedPeaks);
    MergedPeaks.QuickSorter.SortSF(MergedPeaks, MapUtil::PeakSortBySum);
    const int PointCount = dim.Prod();
    for( size_t i=0; i < MergedPeaks.Count(); i++ )  {
      const MapUtil::peak& peak = MergedPeaks[i];
      if( peak.count == 0 )  continue;
      vec3d cnt((double)peak.center[0]/dim[0], (double)peak.center[1]/dim[1], (double)peak.center[2]/dim[2]); 
      const double ed = (double)((long)((peak.summ*1000)/peak.count))/1000;
      TCAtom& ca = au.NewAtom();
      ca.SetLabel(olxstr("Q") << olxstr((100+i)));
      ca.ccrd() = cnt;
      ca.SetQPeak(ed);
    }
    au.InitData();
    TActionQueue* q_draw = FXApp->ActionQueue(olxappevent_GL_DRAW);
    bool q_draw_changed = false;
    if( q_draw != NULL )  {
      q_draw->SetEnabled(false);
      q_draw_changed = true;
    }
    FXApp->XFile().EndUpdate();
    if( q_draw != NULL && q_draw_changed )
      q_draw->SetEnabled(true);
    Macros.ProcessMacro("compaq -q", E);
  }  // integration
  if( Options.Contains("m") )  {
    FractMask* fm = new FractMask;
    FXApp->BuildSceneMask(*fm, maskInc);
    FXApp->XGrid().SetMask(*fm);
  }
  FXApp->XGrid().InitIso();
  FXApp->ShowGrid(true, EmptyString);
}
//..............................................................................
void TMainForm::macShowSymm(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
void TMainForm::macProjSph(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList xatoms;
  FindXAtoms(Cmds, xatoms, false, true);
  if( xatoms.Count() != 1 )  {
    E.ProcessingError(__OlxSourceInfo, "one atom is expected");
    return;
  }
  double R = Options.FindValue("r", "5").ToDouble();
  vec3d shift = xatoms[0]->Atom().crd();
  TNetwork& net = xatoms[0]->Atom().GetNetwork();
  for( size_t i=0; i < net.NodeCount(); i++ )  {
    TSAtom& sa = net.Node(i);
    sa.crd() -= shift;
    if( &sa != &xatoms[0]->Atom() )
      sa.crd().NormaliseTo(R);
  }
  FXApp->GetRender().GetBasis().NullCenter();
  for( size_t i=0; i < FXApp->BondCount(); i++ )
    FXApp->GetBond(i).BondUpdated();
}
//..............................................................................
void TMainForm::macTestBinding(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr empty = EmptyString;
  OlxTests tests;
  tests.Add(&TSymmParser::Tests);
  tests.Add(&smatd::Tests);
  tests.run();
  AtomRefList arl(FXApp->XFile().GetRM(), Cmds.Text(' '), "suc");
  TTypeList<TAtomRefList> res;
  TResidue& main_resi = FXApp->XFile().GetAsymmUnit().GetResidue(0);
  arl.Expand(FXApp->XFile().GetRM(), res);
  for( size_t i=0; i < res.Count(); i++ )  {
    TBasicApp::GetLog() << '\n';
    for( size_t j=0; j < res[i].Count(); j++ )
      TBasicApp::GetLog() << res[i][j].GetFullLabel(FXApp->XFile().GetRM(), main_resi);
  }
  TBasicApp::GetLog() << '\n' << arl.GetExpression() << '\n';
  if( Cmds.Count() == 1 && TEFile::Exists(Cmds[0]) )  {
    TEFile f(Cmds[0], "rb");
    uint64_t st = TETime::msNow();
    TBasicApp::GetLog() << "MD5: " << MD5::Digest(f) << '\n';
    TBasicApp::GetLog() << olxstr::FormatFloat(3, ((double)f.Length()/(((TETime::msNow() - st) + 1)*1.024*1024))) << " Mb/s\n";
    f.SetPosition(0);
    st = TETime::msNow();
    TBasicApp::GetLog() << "SHA1: " << SHA1::Digest(f) << '\n';
    TBasicApp::GetLog() << olxstr::FormatFloat(3, ((double)f.Length()/(((TETime::msNow() - st) + 1)*1.024*1024))) << " Mb/s\n";
    f.SetPosition(0);
    st = TETime::msNow();
    TBasicApp::GetLog() << "SHA224: " << SHA224::Digest(f) << '\n';
    TBasicApp::GetLog() << olxstr::FormatFloat(3, ((double)f.Length()/(((TETime::msNow() - st) + 1)*1.024*1024))) << " Mb/s\n";
    f.SetPosition(0);
    st = TETime::msNow();
    TBasicApp::GetLog() << "SHA256: " << SHA256::Digest(f) << '\n';
    TBasicApp::GetLog() << olxstr::FormatFloat(3, ((double)f.Length()/(((TETime::msNow() - st) + 1)*1.024*1024))) << " Mb/s\n";
  }
  using namespace esdl::exparse;
  EvaluableFactory evf;
  context cx;
  context::init_global(cx);
  evf.types.Add(&typeid(olxstr), new StringValue(""));
  evf.classes.Add(&typeid(olxstr), &StringValue::info);
  StringValue::init_library();

  exp_builder _exp(evf, cx);
  IEvaluable* iv = _exp.build("a = 'ab c, de\\';()'");
  iv = _exp.build("b = 'ab c'");
  //_exp.scope.add_var("a", new StringValue("abcdef"));
  iv = _exp.build("a.sub(0,4).sub(1,3).len()");
  if( !iv->is_final() )  {
    IEvaluable* iv1 = iv->_evaluate();
    delete iv1;
  }
  if( iv->ref_cnt == 0 )  delete iv;
  //iv = _exp.build("x = a.sub (0,4).len() + b.len()");
  //iv = _exp.build("c = a.sub(0,3) == b.sub(0,3)", false);
  //iv = _exp.build("c = a.sub(0,3) != b.sub(0,3)");
  //iv = _exp.build("c = !(a.sub(0,3) == b.sub(0,3))");
  //iv = _exp.build("c = !(a.sub(0,4) == b.sub(0,3))");
  //iv = _exp.build("c = b.sub(0,3) + 'dfg'");
  //iv = _exp.build("c = c + 100");
  //iv = _exp.build("c = 1.2 + 1.1 - .05");
  //iv = _exp.build("a.len() + 1.2 + 1.1 - abs(-.05)*cos(PI/2)");
  iv = _exp.build("a='AaBc'[1].toUpper()");
  iv = _exp.build("a='100'.atoi()");
  
  //iv = _exp.build("if(a){ a = a.sub(0,3); }else{ a = a.sub(0,4); }");
  if( !iv->is_final() && false )  {
    IEvaluable* iv1 = iv->_evaluate();
    delete iv1;
    iv1 = _exp.build("a = 'cos(a)'");
    iv1 = iv->_evaluate();
    delete iv1;
    iv1 = _exp.build("a = cos(c)");
    iv1 = iv->_evaluate();
    delete iv1;
  }
  if( iv->ref_cnt == 0 )  delete iv;
}
//..............................................................................
double Main_FindClosestDistance(const smatd_list& ml, vec3d& o_from, const TCAtom& a_to) {
  vec3d V1, V2, from(o_from), to(a_to.ccrd());
  V2 = from-to;
  a_to.GetParent()->CellToCartesian(V2);
  double minD = V2.QLength();
  for( size_t i=0; i < ml.Count(); i++ )  {
    V1 = ml[i] * from;
    V2 = V1;
    V2 -=to;
    int iv = olx_round(V2[0]);
    V2[0] -= iv;  V1[0] -= iv;  // find closest distance
    iv = olx_round(V2[1]);
    V2[1] -= iv;  V1[1] -= iv;
    iv = olx_round(V2[2]);
    V2[2] -= iv;  V1[2] -= iv;
    a_to.GetParent()->CellToCartesian(V2);
    double D = V2.QLength();
    if( D < minD )  {
      minD = D;
      o_from = V1;
    }
  }
  return sqrt(minD);
}
class TestDistanceAnalysisIteration {
  TCif cif;
  const TStrList& files;
  TAsymmUnit& au;
  smatd_list ml;
public:
  TPSTypeList<int, int> XYZ, XY, XZ, YZ, XX, YY, ZZ;  // length, occurence 

  TestDistanceAnalysisIteration( const TStrList& f_list) : 
      files(f_list), au(cif.GetAsymmUnit())  
  {  
    //TBasicApp::GetInstance().SetMaxThreadCount(1);  // reset for xfile
  }
  ~TestDistanceAnalysisIteration()  {   }
  int Run(long i)  {
    TBasicApp::GetLog() << files[i] << '\n';  
    try { cif.LoadFromFile(files[i]);  }
    catch( ... )  {
      TBasicApp::GetLog().Exception(olxstr("Failed on ") << files[i]);
      return 0;
    }
    TSpaceGroup* sg = TSymmLib::GetInstance().FindSG(au);
    if( sg == NULL )  {
      TBasicApp::GetLog().Exception(olxstr("Unknown sg for ") << files[i]);    
      return 0;
    }
    int cc = 0;
    ml.Clear();
    sg->GetMatrices(ml, mattAll);
    vec3d diff;
    for( size_t j=0; j < au.AtomCount(); j++ )  {
      TCAtom& a1 = au.GetAtom(j);
      if( a1.GetTag() == -1 )  continue;
      if( a1.GetType() == iHydrogenZ )  continue;
      for( size_t k=j+1; k < au.AtomCount(); k++ )  {
        TCAtom& a2 = au.GetAtom(k);
        if( a2.GetTag() == -1 )  continue;
        cc ++;
        if( a2.GetType() == iHydrogenZ )  continue;
        vec3d from(a1.ccrd()), to(a2.ccrd());
        int d = olx_round(Main_FindClosestDistance(ml, from, a2)*100);
        if( d < 1 )  {  // symm eq
          a2.SetTag(-1);
          continue;
        }
        // XYZ
        size_t ind = XYZ.IndexOfComparable(d);
        if( ind == InvalidIndex )  XYZ.Add(d, 1);
        else  XYZ.GetObject(ind)++;
        // XY
        diff[0] = from[0]-to[0];  diff[1] = from[1]-to[1]; diff[2] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = XY.IndexOfComparable(d);
        if( ind == InvalidIndex )  XY.Add(d, 1);
        else  XY.GetObject(ind)++;
        // XZ
        diff[0] = from[0]-to[0];  diff[2] = from[2]-to[2]; diff[1] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = XZ.IndexOfComparable(d);
        if( ind == InvalidIndex )  XZ.Add(d, 1);
        else  XZ.GetObject(ind)++;
        // YZ
        diff[1] = from[1]-to[1];  diff[2] = from[2]-to[2]; diff[0] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = YZ.IndexOfComparable(d);
        if( ind == InvalidIndex )  YZ.Add(d, 1);
        else  YZ.GetObject(ind)++;
        // XX
        diff[0] = from[0]-to[0];  diff[1] = 0; diff[2] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = XX.IndexOfComparable(d);
        if( ind == InvalidIndex )  XX.Add(d, 1);
        else  XX.GetObject(ind)++;
        // YY
        diff[1] = from[1]-to[1];  diff[0] = 0; diff[2] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = YY.IndexOfComparable(d);
        if( ind == InvalidIndex )  YY.Add(d, 1);
        else  YY.GetObject(ind)++;
        // ZZ
        diff[2] = from[2]-to[2];  diff[0] = 0; diff[1] = 0;
        au.CellToCartesian(diff);
        d = olx_round(diff.Length()*100);  // keep two numbers
        ind = ZZ.IndexOfComparable(d);
        if( ind == InvalidIndex )  ZZ.Add(d, 1);
        else  ZZ.GetObject(ind)++;
      }
    }
    return cc;
  }
  inline TestDistanceAnalysisIteration* Replicate() const {  return new TestDistanceAnalysisIteration(files);  }
};
void TMainForm::macTestStat(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TStrList files;
  TEFile::ListCurrentDir(files, "*.cif", sefFile);
  TCif cif;
  TAsymmUnit& au = cif.GetAsymmUnit();

  TPSTypeList<const cm_Element*, double*> atomTypes;
  vec3d v1;
  double tmp_data[601];
  smatd_list ml;
  for( size_t i=0; i < files.Count(); i++ )  {
    TBasicApp::GetLog() << files[i] << '\n';  
    try { cif.LoadFromFile(files[i]);  }
    catch( ... )  {
      TBasicApp::GetLog().Exception(olxstr("Failed on ") << files[i]);
      continue;
    }
    TSpaceGroup* sg = TSymmLib::GetInstance().FindSG(au);
    if( sg == NULL )  {
      TBasicApp::GetLog().Exception(olxstr("Unknown sg for ") << files[i]);    
      continue;
    }
    ml.Clear();
    sg->GetMatrices(ml, mattAll);
    for( size_t j=0; j < au.AtomCount(); j++ )  {
      TCAtom& a1 = au.GetAtom(j);
      if( a1.GetTag() == -1 )  continue;
      if( a1.GetType() == iHydrogenZ )  continue;
      size_t ai = atomTypes.IndexOfComparable(&a1.GetType());
      double* data = ((ai == InvalidIndex) ? new double[601] : atomTypes.GetObject(ai));
      if( ai == InvalidIndex )  {// new array, initialise
        memset(data, 0, sizeof(double)*600);
        atomTypes.Add(&a1.GetType(), data);
      }
      vec3d from(a1.ccrd());
      for( size_t k=j+1; k < au.AtomCount(); k++ )  {
        TCAtom& a2 = au.GetAtom(k);
        if( a2.GetTag() == -1 )  continue;
        if( a2.GetType() == iHydrogenZ )  continue;
        vec3d to(a2.ccrd());
        memset(tmp_data, 0, sizeof(double)*600);
        for( size_t l=0; l < ml.Count(); l++ )  {
          v1 = ml[l] * to - from;
          v1[0] -= olx_round(v1[0]);  v1[1] -= olx_round(v1[1]);  v1[2] -= olx_round(v1[2]);
          au.CellToCartesian(v1);
          double d = v1.Length();
          if( d <= 0.01 )  {
            a2.SetTag(-1);
            break;
          }
          if( d < 6 )
            tmp_data[olx_round(d*100)] ++;
        }
        if( a2.GetTag() != -1 )  {
          for( size_t l=0; l < 600; l++ )
            data[l] += tmp_data[l];
        }
      }
    }
    FXApp->Draw();
    wxTheApp->Dispatch();
  }
  memset(tmp_data, 0, sizeof(double)*600);
  TEFile out("c:\\tmp\\bin_r5.db", "w+b");
  for( size_t i=0; i < atomTypes.Count(); i++ )  {
    double* data = atomTypes.GetObject(i);
    TCStrList sl;
    sl.SetCapacity( 600 );
    double sq = 0;
    for( size_t j=0; j < 600; j++ )  {
      if( j < 598 )
        sq += ((data[j+1]+data[j])*0.01/2.0);
    }
    for( size_t j=0; j < 600; j++ )  {
      data[j] /= sq;
      tmp_data[j] += data[j]*100.0;
      sl.Add( (double)j/100 ) << '\t' << data[j]*1000.0;  // normalised by 1000 square
    }
    sl.SaveToFile( (Cmds[0]+'_') << atomTypes.GetComparable(i)->symbol << ".xlt");
    out << (int16_t)atomTypes.GetComparable(i)->index;
    out.Write(data, 600*sizeof(double));
    delete [] data;
  }
  TCStrList sl;
  sl.SetCapacity( 600 );
  for( size_t i=0; i < 600; i++ )
    sl.Add( (double)i/100 ) << '\t' << tmp_data[i];
  sl.SaveToFile( (Cmds[0]+"_all") << ".xlt");
  return;
// old procedure
  TestDistanceAnalysisIteration testdai(files);
//  FXApp->SetMaxThreadCount(4);
//  TListIteratorManager<TestDistanceAnalysisIteration>* Test = new TListIteratorManager<TestDistanceAnalysisIteration>(testdai, files.Count(), tLinearTask, 0);
//  delete Test;
  if( files.Count() == 1 && TEFile::Exists(Cmds[0]) )  {
    TStrList sl;
    sl.LoadFromFile( Cmds[0] );
    TPSTypeList<int, int> ref_data, &data = testdai.XYZ;
    for( size_t i=0; i < sl.Count(); i++ )  {
      const size_t ind = sl[i].IndexOf('\t');
      if( ind == InvalidIndex )  {
        TBasicApp::GetLog() << (olxstr("could not parse '") << sl[i] << "\'\n");
        continue;
      }
      ref_data.Add( olx_round(sl[i].SubStringTo(ind).ToDouble()*100), sl[i].SubStringFrom(ind+1).ToInt() );
    }
    double R = 0;
    for( size_t i=0; i < data.Count(); i++ )  {
      size_t ind = ref_data.IndexOfComparable(data.GetComparable(i));
      if( ind == InvalidIndex )  {
        TBasicApp::GetLog() << (olxstr("undefined distance ") << data.GetComparable(i) << '\n');
        continue;
      }
      R += sqrt( (double)ref_data.GetObject(ind)*data.GetObject(i) );
    }
//    if( cc != 0 )
//      TBasicApp::GetLog() << (olxstr("Rc=") << R/cc);
  }
  else  {  // just save the result
    TStrPObjList<olxstr, TPSTypeList<int, int>* > data;
    data.Add("xyz", &testdai.XYZ);
    data.Add("xx", &testdai.XX);
    data.Add("xy", &testdai.XY);
    data.Add("xz", &testdai.XZ);
    data.Add("yy", &testdai.YY);
    data.Add("yz", &testdai.YZ);
    data.Add("zz", &testdai.ZZ);
    for( size_t i=0; i < data.Count(); i++ )  {
      TCStrList sl;
      TPSTypeList<int, int>& d = *data.GetObject(i);
      sl.SetCapacity( d.Count() );
      for( size_t j=0; j < d.Count(); j++ )  {
        sl.Add( (double)d.GetComparable(j)/100 ) << '\t' << d.GetObject(j);
      }
      sl.SaveToFile( (Cmds[0]+data[i]) << ".xlt");
    }
  }
}
//..............................................................................
void TMainForm::macExportFont(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TwxGlScene& wxs = dynamic_cast<TwxGlScene&>(FXApp->GetRender().GetScene());
  if( &wxs == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "invalid scene object");
    return;
  }
  wxs.ExportFont(Cmds[0], Cmds[1]);
}
//..............................................................................
void TMainForm::macImportFont(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TwxGlScene& wxs = dynamic_cast<TwxGlScene&>(FXApp->GetRender().GetScene());
  TGlFont* glf = wxs.FindFont( Cmds[0] );
  if( glf == NULL )  {
    E.ProcessingError(__OlxSrcInfo, olxstr("undefined font ") << Cmds[0]);
    return;
  }
  wxs.ImportFont(Cmds[0], Cmds[1]);
}
//..............................................................................
class Esd_Tetrahedron  {
  TSAtomPList atoms;
  olxstr Name;
  TEValue<double> Volume;
  VcoVContainer& vcov;
protected:
  void CalcVolume()  {
    Volume = vcov.CalcTetrahedronVolume( *atoms[0], *atoms[1], *atoms[2], *atoms[3] );
  }
public:
  Esd_Tetrahedron(const olxstr& name, const VcoVContainer& _vcov): Volume(-1,0), vcov(const_cast<VcoVContainer&>(_vcov))  {
    Name = name;
  }
  void Add( TSAtom* a )  {
    atoms.Add( a );
    if( atoms.Count() == 4 )
      CalcVolume();
  }
  const olxstr& GetName() const  {  return Name;  }
  double GetVolume()  const  {  return Volume.GetV();  }
  double GetEsd()  const  {  return Volume.GetE();  }
};
int Esd_ThSort( const Esd_Tetrahedron& th1, const Esd_Tetrahedron& th2 )  {
  double v = th1.GetVolume() - th2.GetVolume();
  if( v < 0 )  return -1;
  if( v > 0 )  return 1;
  return 0;
}
void TMainForm::macEsd(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  VcoVContainer vcovc;
  vcovc.ReadShelxMat( TEFile::ChangeFileExt(FXApp->XFile().GetFileName(), "mat"), FXApp->XFile().GetAsymmUnit() );
  TGlGroup& sel = FXApp->GetSelection();
  if( sel.Count() != 0 )  {
    if( sel.Count() == 1 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) )  {
        TSAtomPList atoms;
        TXAtom& xa = (TXAtom&)sel[0];
        for( size_t i=0; i < xa.Atom().NodeCount(); i++ ) {
          TSAtom& A = xa.Atom().Node(i);
          if( A.IsDeleted() || (A.GetType() == iQPeakZ) )
            continue;
          atoms.Add(A);
        }
        if( atoms.Count() == 3 )
          atoms.Add( &xa.Atom() );
        if( atoms.Count() < 4 )  {
          Error.ProcessingError(__OlxSrcInfo, "An atom with at least four bonds is expected");
          return;
        }
        TTypeList<Esd_Tetrahedron> tetrahedra;
        // special case for 4 nodes
        if( atoms.Count() == 4 )  {
          Esd_Tetrahedron& th = tetrahedra.AddNew( olxstr(atoms[0]->GetLabel() ) << '-'
            << atoms[1]->GetLabel() << '-'
            << atoms[2]->GetLabel() << '-'
            << atoms[3]->GetLabel(), vcovc);
          th.Add( atoms[0] );
          th.Add( atoms[1] );
          th.Add( atoms[2] );
          th.Add( atoms[3] );
        }
        else  {
          for( size_t i=0; i < atoms.Count(); i++ ) {
            for( size_t j=i+1; j < atoms.Count(); j++ ) {
              for( size_t k=j+1; k < atoms.Count(); k++ ) {
                Esd_Tetrahedron& th = tetrahedra.AddNew( olxstr(xa.Atom().GetLabel() ) << '-'
                  << atoms[i]->GetLabel() << '-'
                  << atoms[j]->GetLabel() << '-'
                  << atoms[k]->GetLabel(), vcovc);
                th.Add( &xa.Atom() );
                th.Add( atoms[i] );
                th.Add( atoms[j] );
                th.Add( atoms[k] );
              }
            }
          }
        }
        const size_t thc = (atoms.Count()-2)*2;

        TTypeList<Esd_Tetrahedron>::QuickSorter.SortSF( tetrahedra, &Esd_ThSort );
        bool removed = false;
        while(  tetrahedra.Count() > thc )  {
          TBasicApp::GetLog() << ( olxstr("Removing tetrahedron ") <<  tetrahedra[0].GetName() << " with volume " << tetrahedra[0].GetVolume() << '\n' );
          tetrahedra.Delete(0);
          removed = true;
        }
        double v = 0, esd = 0;
        for( size_t i=0; i < tetrahedra.Count(); i++ )  {
          v += tetrahedra[i].GetVolume();
          esd += tetrahedra[i].GetEsd()*tetrahedra[i].GetEsd();
        }
        TEValue<double> ev(v, sqrt(esd));
        if( removed )
          TBasicApp::GetLog() << ( olxstr("The volume for remaining tetrahedra is ") << ev.ToString() << '\n' );
        else
          TBasicApp::GetLog() << ( olxstr("The tetrahedra volume is ") << ev.ToString() << " A^3\n" );
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) )  {
        TSAtomPList atoms;
        TXPlane& xp = (TXPlane&)sel[0];
        olxstr pld;
        for( size_t i=0; i < xp.Plane().Count(); i++ )  {
          atoms.Add(xp.Plane().GetAtom(i));
          pld << atoms.Last()->GetLabel() << ' ';
        }
        TBasicApp::GetLog() << (olxstr("Plane ") << pld << "RMS: " << vcovc.CalcPlane(atoms).ToString() << '\n');
        TEVPoint<double> c_cent( vcovc.CalcCentroid(atoms) );
        TBasicApp::GetLog() << (olxstr("Plane ") << pld << "cartesian centroid : {" << c_cent[0].ToString() <<
          ", " << c_cent[1].ToString() << ", " << c_cent[2].ToString() << "}\n");
        TEVPoint<double> f_cent( vcovc.CalcCentroidF(atoms) );
        TBasicApp::GetLog() << (olxstr("Plane ") << pld << "fractional centroid : {" << f_cent[0].ToString() <<
          ", " << f_cent[1].ToString() << ", " << f_cent[2].ToString() << "}\n");
      }
      else if( EsdlInstanceOf(sel[0], TXBond) )  {
        TXBond& xb = (TXBond&)sel[0];
        TBasicApp::GetLog() << (olxstr(xb.Bond().A().GetLabel()) << " to " <<
          xb.Bond().B().GetLabel() << " distance: " <<
          vcovc.CalcDistance(xb.Bond().A(), xb.Bond().B()).ToString() << '\n');
      }
    }
    else if( sel.Count() == 2 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[1], TXAtom) )  {
        TBasicApp::GetLog() << (olxstr(((TXAtom&)sel[0]).Atom().GetLabel()) << " to " <<
          ((TXAtom&)sel[1]).Atom().GetLabel() << " distance: " <<
          vcovc.CalcDistance(((TXAtom&)sel[0]).Atom(), ((TXAtom&)sel[1]).Atom()).ToString() << '\n');
      }
      else if( EsdlInstanceOf(sel[0], TXBond) && EsdlInstanceOf(sel[1], TXBond) )  {
        TSBond& b1 = ((TXBond&)sel[0]).Bond();
        TSBond& b2 = ((TXBond&)sel[1]).Bond();
        TEValue<double> v(vcovc.CalcB2BAngle(b1.A(), b1.B(), b2.A(), b2.B())),
          v1(180-v.GetV(), v.GetE());
        TBasicApp::GetLog() << (olxstr(b1.A().GetLabel()) << '-' << b1.B().GetLabel() << " to " <<
          b2.A().GetLabel() << '-' << b2.B().GetLabel() << " angle: " <<
          v.ToString() << '(' << v1.ToString() << ")\n");
      }
      else if( (EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[1], TXPlane)) ||  
               (EsdlInstanceOf(sel[1], TXAtom) && EsdlInstanceOf(sel[0], TXPlane)))  {
        TSAtomPList atoms;
        TXPlane& xp = (TXPlane&)sel[ EsdlInstanceOf(sel[0], TXPlane) ? 0 : 1];
        olxstr pld;
        for( size_t i=0; i < xp.Plane().Count(); i++ )  {
          atoms.Add(xp.Plane().GetAtom(i));
          pld << atoms.Last()->GetLabel() << ' ';
        }
        TSAtom& sa = ((TXAtom&)sel[EsdlInstanceOf(sel[0],TXAtom) ? 0 : 1]).Atom();
        TBasicApp::GetLog() << (olxstr(sa.GetLabel()) << " to plane " << pld << "distance: " <<
          vcovc.CalcP2ADistance(atoms, sa).ToString() << '\n');
        TBasicApp::GetLog() << (olxstr(sa.GetLabel()) << " to plane " << pld << "centroid distance: " <<
          vcovc.CalcPC2ADistance(atoms, sa).ToString() << '\n' );
        // test show complete equivalence to the numerical diff above
//        TBasicApp::GetLog() << (olxstr(sa.GetLabel()) << " to plane " << pld << "centroid distance (precise): " <<
//          vcovc.CalcPC2ADistanceP(atoms, sa).ToString() << '\n' );
      }
      else if( (EsdlInstanceOf(sel[0], TXBond) && EsdlInstanceOf(sel[1], TXPlane)) ||  
               (EsdlInstanceOf(sel[1], TXBond) && EsdlInstanceOf(sel[0], TXPlane)))  {
        TSAtomPList atoms;
        TXPlane& xp = (TXPlane&)sel[ EsdlInstanceOf(sel[0], TXPlane) ? 0 : 1];
        olxstr pld;
        for( size_t i=0; i < xp.Plane().Count(); i++ )  {
          atoms.Add(xp.Plane().GetAtom(i));
          pld << atoms.Last()->GetLabel() << ' ';
        }
        TSBond& sb = ((TXBond&)sel[ EsdlInstanceOf(sel[0], TXBond) ? 0 : 1]).Bond();
        TEValue<double> v(vcovc.CalcP2VAngle(atoms, sb.A(), sb.B())),
          v1(180-v.GetV(), v.GetE());
        TBasicApp::GetLog() << (olxstr(sb.A().GetLabel()) << '-' << sb.B().GetLabel() << " to plane " << pld << "angle: " <<
          v.ToString() << '(' << v1.ToString() << ")\n");
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) && EsdlInstanceOf(sel[1], TXPlane) )  {
        TSAtomPList p1, p2;
        TXPlane& xp1 = (TXPlane&)sel[0];
        TXPlane& xp2 = (TXPlane&)sel[1];
        olxstr pld1, pld2;
        for( size_t i=0; i < xp1.Plane().Count(); i++ )  {
          p1.Add(xp1.Plane().GetAtom(i));
          pld1 << p1.Last()->GetLabel() << ' ';
        }
        for( size_t i=0; i < xp2.Plane().Count(); i++ )  {
          p2.Add(xp2.Plane().GetAtom(i));
          pld2 << p2.Last()->GetLabel() << ' ';
        }
        const TEValue<double> angle = vcovc.CalcP2PAngle(p1, p2);
        TBasicApp::GetLog() << (olxstr("Plane ") << pld1 << "to plane angle: " <<
          angle.ToString() << '\n' );
        TBasicApp::GetLog() << (olxstr("Plane centroid to plane centroid distance: ") <<
          vcovc.CalcPC2PCDistance(p1, p2).ToString() << '\n' );
        TBasicApp::GetLog() << (olxstr("Plane [") << pld1 << "] to plane centroid distance: " <<
          vcovc.CalcP2PCDistance(p1, p2).ToString() << '\n' );
        TBasicApp::GetLog() << (olxstr("Plane [") << pld1 << "] to plane shift: " <<
          vcovc.CalcP2PShiftDistance(p1, p2).ToString() << '\n' );
        if( olx_abs(angle.GetV()) > 1e-6 )  {
          TBasicApp::GetLog() << (olxstr("Plane [") << pld2 << "] to plane centroid distance: " <<
            vcovc.CalcP2PCDistance(p2, p1).ToString() << '\n' );
          TBasicApp::GetLog() << (olxstr("Plane [") << pld2 << "] to plane shift distance: " <<
            vcovc.CalcP2PShiftDistance(p2, p1).ToString() << '\n' );
        }
      }
    }
    else if( sel.Count() == 3 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[1], TXAtom) && EsdlInstanceOf(sel[2], TXAtom) )  {
        TSAtom& a1 = ((TXAtom&)sel[0]).Atom();
        TSAtom& a2 = ((TXAtom&)sel[1]).Atom();
        TSAtom& a3 = ((TXAtom&)sel[2]).Atom();
        TBasicApp::GetLog() << (olxstr(a1.GetLabel()) << '-' << a2.GetLabel() << '-' << a3.GetLabel() << " angle: " <<
          vcovc.CalcAngle(a1, a2, a3).ToString() << '\n');
      }
      else if( (EsdlInstanceOf(sel[0], TXPlane) && EsdlInstanceOf(sel[1], TXAtom) && EsdlInstanceOf(sel[2], TXAtom)) || 
               (EsdlInstanceOf(sel[1], TXPlane) && EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[2], TXAtom)) ||
               (EsdlInstanceOf(sel[2], TXPlane) && EsdlInstanceOf(sel[1], TXAtom) && EsdlInstanceOf(sel[0], TXAtom)))  {
        TSAtom* a1 = NULL, *a2 = NULL;
        TXPlane* xp = NULL;
        TSAtomPList atoms;
        for( size_t  i=0; i < 3; i++ )  {
          if( EsdlInstanceOf(sel[i], TXPlane) )
            xp = &(TXPlane&)sel[i];
          else  {
            if( a1 == NULL )
              a1 = &((TXAtom&)sel[i]).Atom();
            else
              a2 = &((TXAtom&)sel[i]).Atom();
          }
        }
        olxstr pld;
        for( size_t i=0; i < xp->Plane().Count(); i++ )  {
          atoms.Add(xp->Plane().GetAtom(i));
          pld << atoms.Last()->GetLabel() << ' ';
        }
        TBasicApp::GetLog() << (olxstr(a1->GetLabel()) << '-' << a2->GetLabel() << " to plane " << pld << "angle: " <<
          vcovc.CalcP2VAngle(atoms, *a1, *a2).ToString() << '\n' );
      }
      else if( EsdlInstanceOf(sel[0], TXPlane) && EsdlInstanceOf(sel[1], TXPlane) && EsdlInstanceOf(sel[2], TXPlane) )  {
        TSPlane& p1 = ((TXPlane&)sel[0]).Plane();
        TSPlane& p2 = ((TXPlane&)sel[1]).Plane();
        TSPlane& p3 = ((TXPlane&)sel[2]).Plane();
        TSAtomPList a1, a2, a3;
        for( size_t i=0; i < p1.Count(); i++ )  a1.Add(p1.GetAtom(i));
        for( size_t i=0; i < p2.Count(); i++ )  a2.Add(p2.GetAtom(i));
        for( size_t i=0; i < p3.Count(); i++ )  a3.Add(p3.GetAtom(i));
        TBasicApp::GetLog() << "Angle between plane centroids: " <<
          vcovc.Calc3PCAngle(a1, a2, a3).ToString() << '\n';
      }
    }
    else if( sel.Count() == 4 )  {
      if( EsdlInstanceOf(sel[0], TXAtom) && EsdlInstanceOf(sel[1], TXAtom) && 
          EsdlInstanceOf(sel[2], TXAtom) && EsdlInstanceOf(sel[3], TXAtom) )  {
        TSAtom& a1 = ((TXAtom&)sel[0]).Atom();
        TSAtom& a2 = ((TXAtom&)sel[1]).Atom();
        TSAtom& a3 = ((TXAtom&)sel[2]).Atom();
        TSAtom& a4 = ((TXAtom&)sel[3]).Atom();
        TBasicApp::GetLog() << (olxstr(a1.GetLabel()) << '-' << a2.GetLabel() << '-' << a3.GetLabel() << '-' << a4.GetLabel()
          << " torsion angle: " <<
          vcovc.CalcTAngle(a1, a2, a3, a4).ToString() << '\n');
        TBasicApp::GetLog() << (olxstr(a1.GetLabel()) << '-' << a2.GetLabel() << '-' << a3.GetLabel() << '-' << a4.GetLabel()
          << " tetrahedron volume: " <<
          vcovc.CalcTetrahedronVolume(a1, a2, a3, a4).ToString() << '\n');
      }
    }
    else if( sel.Count() == 7 )  {
      TSAtomPList atoms, sorted_atoms, face_atoms;
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( EsdlInstanceOf(sel[i], TXAtom) )
          atoms.Add( ((TXAtom&)sel[i]).Atom() );
      }
      if( atoms.Count() != 7 )
        return;
      TBasicApp::GetLog() << "Octahedral distortion is (using best line, for the selection): " << 
        vcovc.CalcOHDistortionBL(atoms).ToString() << '\n';
      TBasicApp::GetLog() << "Octahedral distortion is (using best plane, for the selection): " << 
        vcovc.CalcOHDistortionBP(atoms).ToString() << '\n';
      TSAtom* central_atom = atoms[0];
      atoms.Delete(0);
      olxdict<index_t, vec3d, TPrimitiveComparator> transforms;
      int face_cnt = 0;
      double total_val_bp = 0, total_esd_bp = 0;//, total_val_bl = 0, total_esd_bl=0;
      for( size_t i=0; i < 6; i++ )  {
        for( size_t j=i+1; j < 6; j++ )  {
          for( size_t k=j+1; k < 6; k++ )  {
            const double thv = TetrahedronVolume( 
              central_atom->crd(),
              (atoms[i]->crd()-central_atom->crd()).Normalise() + central_atom->crd(),
              (atoms[j]->crd()-central_atom->crd()).Normalise() + central_atom->crd(),
              (atoms[k]->crd()-central_atom->crd()).Normalise() + central_atom->crd());
            if( thv < 0.1 )  continue;
            sorted_atoms.Clear();
            transforms.Clear();
            for( size_t l=0; l < atoms.Count(); l++ )
              atoms[l]->SetTag(0);
            atoms[i]->SetTag(1);  atoms[j]->SetTag(1);  atoms[k]->SetTag(1);
            const vec3d face_center = (atoms[i]->crd()+atoms[j]->crd()+atoms[k]->crd())/3;
            const vec3d normal = (face_center - central_atom->crd()).Normalise();
            transforms.Add(1, central_atom->crd() - face_center);
            vec3d face1_center;
            for( size_t l=0; l < atoms.Count(); l++ )
              if( atoms[l]->GetTag() == 0 )
                face1_center += atoms[l]->crd();
            face1_center /= 3;
            transforms.Add(0, central_atom->crd() - face1_center);
            PlaneSort::Sorter::DoSort(atoms, transforms, central_atom->crd(), normal, sorted_atoms);
            TBasicApp::GetLog() << (olxstr("Face ") << ++face_cnt << ": ") ;
            for( size_t l=0; l < sorted_atoms.Count(); l++ )
              TBasicApp::GetLog() << sorted_atoms[l]->GetLabel() << ' ';
            sorted_atoms.Insert(0, central_atom);
            TEValue<double> rv = vcovc.CalcOHDistortionBP(sorted_atoms);
            total_val_bp += rv.GetV()*3;
            total_esd_bp += olx_sqr(rv.GetE()); 
            TBasicApp::GetLog() << rv.ToString() << '\n';
            //TBasicApp::GetLog() << "BP: " << rv.ToString();
            //rv = vcovc.CalcOHDistortionBL(sorted_atoms);
            //total_val_bl += olx_abs(180.0 - rv.GetV() * 3);
            //total_esd_bl += olx_sqr(rv.GetE()); 
            //TBasicApp::GetLog() << " BL: " << rv.ToString() << '\n';
          }
        }
      }
      if( face_cnt == 8 )  {
        TBasicApp::GetLog() << (olxstr("Combined distortion (best plane): ") << TEValue<double>(total_val_bp, 3*sqrt(total_esd_bp)).ToString() << '\n');
        //TBasicApp::GetLog() << (olxstr("Combined distortion (best line): ") << TEValue<double>(total_val_bl, 3*sqrt(total_esd_bl)).ToString() << '\n');
      }
      else  {
        TBasicApp::GetLog() << "Could not locate required 8 octahedron faces\n";
      }
    }
  }
}
//..............................................................................
void TMainForm::macImportFrag(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr FN = PickFile("Load Fragment",
    "XYZ files (*.xyz)|*.xyz",
    XLibMacros::CurrentDir, true);
  if( FN.IsEmpty() ) 
    return;
  TXyz xyz;
  xyz.LoadFromFile(FN);
  TXAtomPList xatoms;
  FXApp->AdoptAtoms(xyz.GetAsymmUnit(), xatoms);
  int part = Options.FindValue("p", "-100").ToInt();
  if( part != -100 )  {
    for( size_t i=0; i < xatoms.Count(); i++ )
      xatoms[i]->Atom().CAtom().SetPart(part);
  }
  Macros.ProcessMacro("mode fit", E);
  AMode *md = Modes->GetCurrent();
  if( md != NULL  )
    md->AddAtoms(xatoms);
}
//..............................................................................
void TMainForm::macExportFrag(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  olxstr FN = PickFile("Save Fragment as...",
    "XYZ files (*.xyz)|*.xyz",
    XLibMacros::CurrentDir, false);
  if( FN.IsEmpty() )  return;
  TXAtomPList xatoms;
  TGlGroup& glg = FXApp->GetSelection();
  for( size_t i=0; i < glg.Count(); i++ )  {
    if( EsdlInstanceOf(glg[i], TXAtom) )
      xatoms.Add( (TXAtom&)glg[i] );
  }
  TNetPList nets;
  for( size_t i=0; i < xatoms.Count(); i++ )  {
    TNetwork* net = &xatoms[i]->Atom().GetNetwork();
    if( nets.IndexOf(net) == InvalidIndex )
      nets.Add(net);
  }
  if( nets.Count() != 1 )  {
    E.ProcessingError(__OlxSrcInfo, "please select one fragment or one atom only");
    return;
  }
  TXyz xyz;
  
  for( size_t i=0; i < nets[0]->NodeCount(); i++ )  {
    if( nets[0]->Node(i).IsDeleted() || nets[0]->Node(i).GetType() == iQPeakZ )
      continue;
    TCAtom& ca = xyz.GetAsymmUnit().NewAtom();
    ca.ccrd() = nets[0]->Node(i).crd();
    ca.SetType(nets[0]->Node(i).GetType());
  }
  xyz.SaveToFile(FN);
}
//..............................................................................
void TMainForm::macConn(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  try  {  
    TStrList lst(Cmds);
    if( (Cmds.Count() == 1 && Cmds[0].IsNumber()) || 
        (Cmds.Count() == 2 && Cmds[0].IsNumber() && Cmds[1].IsNumber()) )  
    {
      TCAtomPList atoms;
      FXApp->FindCAtoms("sel", atoms, false);
      if( atoms.IsEmpty() )  return;
      for( size_t i=0; i < atoms.Count(); i++ )
        lst.Add("#c") << atoms[i]->GetId();
    }
    FXApp->XFile().GetRM().Conn.ProcessConn(lst);
    FXApp->XFile().GetAsymmUnit()._UpdateConnInfo();
    FXApp->GetRender().SelectAll(false);
    FXApp->XFile().GetLattice().UpdateConnectivity();
  }
  catch( const TExceptionBase& exc )  {
    E.ProcessingError(__OlxSrcInfo, exc.GetException()->GetError());
  }
}
//..............................................................................
void TMainForm::macAddBond(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList atoms;
  if( !FindXAtoms(Cmds, atoms, false, true) )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  if( (atoms.Count() % 2) != 0 )  {
    E.ProcessingError(__OlxSrcInfo, "even number if atoms is expected");
    return;
  }
  for( size_t i=0; i < atoms.Count(); i += 2 )  {
    TSAtom* a1 = NULL, *a2 = NULL;
    if( atoms[i]->Atom().GetMatrix(0).IsFirst() )
      a1 = &atoms[i]->Atom();
    else if( atoms[i+1]->Atom().GetMatrix(0).IsFirst() )
      a1 = &atoms[i+1]->Atom();
    else  {
      FXApp->GetLog() << (olxstr("At maximum one symmetry equivalent atom is allowed, skipping: ") <<
        atoms[i]->Atom().GetGuiLabel() << '-' << atoms[i+1]->Atom().GetGuiLabel() << '\n');
      continue;
    }
    a2 = (a1 == &atoms[i]->Atom()) ? &atoms[i+1]->Atom() : &atoms[i]->Atom();
    const smatd& eqiv = FXApp->XFile().GetRM().AddUsedSymm(a2->GetMatrix(0));
    FXApp->XFile().GetRM().Conn.AddBond(a1->CAtom(), a2->CAtom(), NULL, &eqiv, true);
  }
  FXApp->XFile().GetAsymmUnit()._UpdateConnInfo();
  FXApp->XFile().GetLattice().UpdateConnectivity();
}
//..............................................................................
void TMainForm::macDelBond(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TSAtomPList pairs;
  if( Cmds.IsEmpty() )  {
    TGlGroup& glg = FXApp->GetSelection();
    for( size_t i=0; i < glg.Count(); i++ )  {
      if( EsdlInstanceOf(glg[i], TXBond) )  {
        TSBond& sb = ((TXBond&)glg[i]).Bond();
        pairs.Add(&sb.A());
        pairs.Add(&sb.B());
      }
    }
  }
  else  {
    TXAtomPList atoms;
    FindXAtoms(Cmds, atoms, false, false);
    if( (atoms.Count()%2) == 0 )
      TListCaster::POP(atoms, pairs);
  }
  if( !pairs.IsEmpty()  )  {
    for( size_t i=0; i < pairs.Count(); i+=2 )  {
      TSAtom* a1 = NULL, *a2 = NULL;
      if( pairs[i]->GetMatrix(0).IsFirst() )
        a1 = pairs[i];
      else if( pairs[i+1]->GetMatrix(0).IsFirst() )
        a1 = pairs[i+1];
      else  {
        FXApp->GetLog() << (olxstr("At maximum one symmetry equivalent atom is allowed, skipping: ") <<
          pairs[i]->GetGuiLabel() << '-' << pairs[i+1]->GetGuiLabel() << '\n');
        continue;
      }
      a2 = (a1 == pairs[i]) ? pairs[i+1] : pairs[i];
      const smatd& eqiv = FXApp->XFile().GetRM().AddUsedSymm(a2->GetMatrix(0));
      FXApp->XFile().GetRM().Conn.RemBond(a1->CAtom(), a2->CAtom(), NULL, &eqiv, true);
    }
    FXApp->GetRender().SelectAll(false);
    FXApp->XFile().GetAsymmUnit()._UpdateConnInfo();
    FXApp->XFile().GetLattice().UpdateConnectivity();
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "please select some bonds or provide atom pairs");
    return;
  }
}
//..............................................................................
void TMainForm::macUpdateQPeakTable(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  QPeakTable(false);
}
//..............................................................................
void TMainForm::funCheckState(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(CheckState(TStateChange::DecodeState(Params[0]), Params.Count() == 2 ? Params[1] : EmptyString));
}
//..............................................................................
void TMainForm::funGlTooltip(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )
    E.SetRetVal( _UseGlTooltip );
  else
    UseGlTooltip( Params[0].ToBool() );
}
//..............................................................................
void TMainForm::funCurrentLanguage(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(Dictionary.GetCurrentLanguage());
  else
    Dictionary.SetCurrentLanguage(DictionaryFile, Params[0] );
}
//..............................................................................
void TMainForm::macSAME(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList atoms;
  bool invert = Options.Contains("i");
  size_t groups_count = InvalidSize;
  if( !Cmds.IsEmpty() && Cmds[0].IsNumber() )  {
    groups_count = Cmds[0].ToSizeT();
    Cmds.Delete(0);
  }
  FindXAtoms(Cmds, atoms, false, true);
  if( atoms.Count() == 2 )  {
    TTypeList< AnAssociation2<size_t, size_t> > res;
    TNetwork &netA = atoms[0]->Atom().GetNetwork(),
      &netB = atoms[1]->Atom().GetNetwork();
    if( &netA == &netB )  {
      E.ProcessingError(__OlxSrcInfo, "Please select different fragments");
      return;
    }
    if( !netA.DoMatch(netB, res, invert, &TNetwork::weight_occu) )  {
      E.ProcessingError(__OlxSrcInfo, "Graphs do not match");
      return;
    }
    //got the pairs now...
    TSameGroup& sg = FXApp->XFile().GetRM().rSAME.New();
    for( size_t i=0; i < netA.NodeCount(); i++ )  {
      netA.Node(i).SetTag(-1);
      netB.Node(i).SetTag(-1);
    }
    for( size_t i=0; i < res.Count(); i++ )  {
      if( netA.Node(res[i].GetA()).GetType().GetMr() < 3.5 || netA.Node(res[i].GetA()).GetTag() == 0 )
        continue;
      sg.Add( netA.Node(res[i].GetA()).CAtom() );
      netA.Node(res[i].GetA()).SetTag(0);
      TBasicApp::GetLog() << netA.Node(res[i].GetA()).GetLabel() << ' ';
    }
    TBasicApp::GetLog() << '\n';
    TSameGroup& d_sg = FXApp->XFile().GetRM().rSAME.NewDependent(sg);
    for( size_t i=0; i < res.Count(); i++ )  {
      if( netB.Node(res[i].GetB()).GetType().GetMr() < 3.5 || netB.Node(res[i].GetB()).GetTag() == 0 )
        continue;
      d_sg.Add( netB.Node(res[i].GetB()).CAtom() );
      netB.Node(res[i].GetB()).SetTag(0);
      TBasicApp::GetLog() << netB.Node(res[i].GetB()).GetLabel() << ' ';
    }
    TBasicApp::GetLog() << '\n';
  }
  else if( groups_count != InvalidSize && (atoms.Count()%groups_count) == 0 )  {
    TPtrList<TSameGroup> deps;
    TSameGroup& sg = FXApp->XFile().GetRM().rSAME.New();
    for( size_t i=0; i < groups_count-1; i++ )
      deps.Add( FXApp->XFile().GetRM().rSAME.NewDependent(sg) );
    const size_t cnt = atoms.Count()/groups_count;
    for( size_t i=0; i < cnt; i++ )  {
      sg.Add(atoms[i]->Atom().CAtom());
      for( size_t j=1; j < groups_count; j++ )
        deps[j-1]->Add(atoms[cnt*j+i]->Atom().CAtom());
    }
    TBasicApp::GetLog() << "SAME instruction is added\n";
  }
  else  {
    E.ProcessingError(__OlxSrcInfo, "inlvaid input arguments");
  }
}
//..............................................................................
void TMainForm::macRESI(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXAtomPList atoms;
  olxstr resi_class = Cmds[0];
  Cmds.Delete(0);
  int resi_number = 0;
  if( Cmds.Count() > 0  && Cmds[0].IsNumber() )  {
    resi_number = olx_abs(Cmds[0].ToInt());
    Cmds.Delete(0);
  }
  FindXAtoms(Cmds, atoms, false, true);
  if( atoms.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "no atoms provided");
    return;
  }
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  for( size_t i=0; i < atoms.Count(); i++ )
    atoms[i]->Atom().CAtom().SetTag(i);
  if( resi_class.Equalsi("none") )  {
    TResidue& main_resi = au.GetResidue(0);
    for( size_t i=0; i < atoms.Count(); i++ )  {
      TCAtom& ca = atoms[i]->Atom().CAtom();
      if( ca.GetTag() == i && olx_is_valid_index(ca.GetResiId()) )  {
        main_resi.Add(ca);
      }
    }
  }
  else  {
    TResidue& resi = au.NewResidue(resi_class, resi_number, Options.FindValue('a'));
    for( size_t i=0; i < atoms.Count(); i++ )
      if( atoms[i]->Atom().CAtom().GetTag() == i )
        resi.Add( atoms[i]->Atom().CAtom() );
  }
}
//..............................................................................
void TMainForm::funGetMAC(const TStrObjList& Params, TMacroError &E)  {
  bool full = (Params.Count() == 1 && Params[0].Equalsi("full") );
  olxstr rv(EmptyString, 256);
  char bf[16];
  TShellUtil::MACInfo MACsInfo;
  TShellUtil::ListMACAddresses(MACsInfo);
  for( size_t i=0; i < MACsInfo.Count(); i++ )  {
    if( full )
      rv << MACsInfo[i] << '=';
    for( size_t j=0; j < MACsInfo.GetObject(i).Count(); j++ )  {
      sprintf(bf, "%02X", MACsInfo.GetObject(i)[j] );
      rv << bf;
      if( j < 5 )  rv << '-';
    }
    if( (i+1) < MACsInfo.Count() )  
      rv << ';';
  }
  E.SetRetVal(rv.IsEmpty() ? XLibMacros::NAString : rv);
}
//..............................................................................
void main_CreateWBox(TGXApp& app, const TSAtomPList& atoms, const TTypeList< AnAssociation2<vec3d, double> >& crds, 
  const TDoubleList& all_radii, bool print_info)  
{
  static int obj_cnt = 0;
  mat3d normals;
  vec3d rms, center, Ds;
  TSPlane::CalcPlanes(crds, normals, rms, center);  
  vec3d mind, maxd, mind1, maxd1;
  for( int i=0; i < 3; i++ )  {
    Ds[i] = normals[i].DotProd(center)/normals[i].Length();
    normals[i].Normalise();
    for( size_t j=0; j < crds.Count(); j++ )  {
      const double d = crds[j].GetA().DotProd(normals[i]) - Ds[i];
      if( d < 0 )  {
        const double d1 = d - all_radii[j];
        if( d1 < mind[i] )
          mind[i] = d1;
      }
      else  {
        const double d1 = d + all_radii[j];
        if( d1 > maxd[i] )
          maxd[i] = d1;
      }
      if( d < 0 )  {
        const double d1 = d - atoms[j]->GetType().r_sfil;
        if( d1 < mind1[i] )
          mind1[i] = d1;
      }
      else  {
        const double d1 = d + atoms[j]->GetType().r_sfil;
        if( d1 > maxd1[i] )
          maxd1[i] = d1;
      }
    }
  }
	if( print_info )  {
    app.GetLog() << (olxstr("Wrapping box dimension: ") << 
      olxstr::FormatFloat(3, maxd[0]-mind[0]) << " x "  <<
      olxstr::FormatFloat(3, maxd[1]-mind[1]) << " x "  <<
      olxstr::FormatFloat(3, maxd[2]-mind[2]) << " A\n");
    app.GetLog() << (olxstr("Wrapping box volume: ") << 
      olxstr::FormatFloat(3, (maxd[0]-mind[0])*(maxd[1]-mind[1])*(maxd[2]-mind[2])) << " A^3\n");
	}
  vec3d nx = normals[0]*mind1[0];
  vec3d px = normals[0]*maxd1[0];
  vec3d ny = normals[1]*mind1[1];
  vec3d py = normals[1]*maxd1[1];
  vec3d nz = normals[2]*mind1[2];
  vec3d pz = normals[2]*maxd1[2];

  vec3d faces[] = {
    px + py + pz, px + ny + pz, px + ny + nz, px + py + nz, // normals[0]
    nx + py + pz, nx + py + nz, nx + ny + nz, nx + ny + pz, // -normals[0]
    px + py + pz, px + py + nz, nx + py + nz, nx + py + pz, // normals[1]
    px + ny + nz, px + ny + pz, nx + ny + pz, nx + ny + nz, // -normals[1]
    nx + py + pz, nx + ny + pz, px + ny + pz, px + py + pz, // normals[2]
    nx + py + nz, px + py + nz, px + ny + nz, nx + ny + nz // -normals[2]
  };

  TArrayList<vec3f>& poly_d = *(new TArrayList<vec3f>(24));
  TArrayList<vec3f>& poly_n = *(new TArrayList<vec3f>(6));
  for( int i=0; i < 6; i++ )  {
    const vec3d cnt((faces[i*4]+faces[i*4+1]+faces[i*4+2]+faces[i*4+3])/4);
    const vec3f norm = (faces[i*4+1]-faces[i*4]).XProdVec(faces[i*4+3]-faces[i*4]);
    if( norm.DotProd(cnt) < 0 )  {  // does normal look inside?
      poly_n[i] = -norm;
      olx_swap(faces[i*4+1], faces[i*4+3]);
    }
    else
      poly_n[i] = norm;
  }
  for( int i=0; i < 24; i++ )
		poly_d[i] = center + faces[i];
  TDUserObj* uo = new TDUserObj(app.GetRender(), sgloQuads, olxstr("wbox") << obj_cnt++);
  uo->SetVertices(&poly_d);
  uo->SetNormals(&poly_n);
  app.AddObjectToCreate( uo );
  uo->SetMaterial("1029;2566914048;2574743415");
  uo->Create();
  if( print_info )
    app.GetLog() << "Please note that displayed and used atomic radii might be DIFFERENT\n";
}
void TMainForm::macWBox(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  ElementRadii radii;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  if( Cmds.Count() == 1 && TEFile::Exists(Cmds[0]) )
    radii = TXApp::ReadVdWRadii(Cmds[0]);
  TXApp::PrintVdWRadii(radii, au.GetContentList());
	TSAtomPList satoms;
	const bool use_aw = Options.Contains('w');
  if( Options.Contains('s') )  {
  	TLattice& latt = FXApp->XFile().GetLattice();
		for( size_t i=0; i < latt.FragmentCount(); i++ )  {
		  satoms.Clear();
			TNetwork& f = latt.GetFragment(i);
			for( size_t j=0; j < f.NodeCount(); j++ )  {
			  if( f.Node(j).IsDeleted() )  continue;
				satoms.Add(f.Node(j));
			}
			if( satoms.Count() < 3 )  continue;
  
			TTypeList< AnAssociation2<vec3d, double> > crds;
      TArrayList<double> all_radii(satoms.Count());
      for( size_t j=0; j < satoms.Count(); j++ )  {
	      if( use_aw )
          crds.AddNew(satoms[j]->crd(), satoms[j]->GetType().GetMr());
		    else
          crds.AddNew( satoms[j]->crd(), 1.0 );
        const size_t ri = radii.IndexOf(&satoms[j]->GetType());
        if( ri == InvalidIndex )
          all_radii[j] = satoms[j]->GetType().r_vdw;
        else
          all_radii[j] = radii.GetValue(ri);
      }
			main_CreateWBox(*FXApp, satoms, crds, all_radii, false);
		}
	}
	else  {
    TXAtomPList xatoms;
    if( !FindXAtoms(Cmds, xatoms, true, true) || xatoms.Count() < 3 )  {
      E.ProcessingError(__OlxSrcInfo, "no enough atoms provided");
      return;
    }
    TTypeList< AnAssociation2<vec3d, double> > crds;
    TArrayList<double> all_radii(xatoms.Count());
    for( size_t i=0; i < xatoms.Count(); i++ )  {
	    if( use_aw )
        crds.AddNew(xatoms[i]->Atom().crd(), xatoms[i]->Atom().GetType().GetMr());
		  else
        crds.AddNew( xatoms[i]->Atom().crd(), 1.0 );
      const size_t ri = radii.IndexOf(&xatoms[i]->Atom().GetType());
      if( ri == InvalidIndex )
        all_radii[i] = xatoms[i]->Atom().GetType().r_vdw;
      else
        all_radii[i] = radii.GetValue(ri);
    }
    TListCaster::POP(xatoms, satoms);
	  main_CreateWBox(*FXApp, satoms, crds, all_radii, true);
	}
}
//..............................................................................
void TMainForm::macCenter(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  if( Cmds.Count() == 3 && Cmds[0].IsNumber() && Cmds[1].IsNumber() && Cmds[2].IsNumber() )
    FXApp->GetRender().GetBasis().SetCenter(vec3d(-Cmds[0].ToDouble(), -Cmds[1].ToDouble(), -Cmds[2].ToDouble()));
  else  {
    if( Options.Contains('z') )
      FXApp->GetRender().GetBasis().SetZoom(FXApp->GetRender().CalcZoom());
    else  {
      TXAtomPList atoms;
      FindXAtoms(Cmds, atoms, true, true);
      vec3d center;
      double sum = 0;
      for( size_t i=0; i < atoms.Count(); i++ )  {
        center += atoms[i]->Atom().crd()*atoms[i]->Atom().CAtom().GetOccu();
        sum += atoms[i]->Atom().CAtom().GetOccu();;
      }
      if( sum != 0 )  {
        center /= sum;
        FXApp->GetRender().GetBasis().SetCenter(-center);
      }
    }
  }
}
//..............................................................................
void TMainForm::funProfiling(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )  E.SetRetVal(FXApp->IsProfiling());
  else
    FXApp->SetProfiling(Params[0].ToBool());
}
//..............................................................................
void TMainForm::funThreadCount(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )  E.SetRetVal(FXApp->GetMaxThreadCount());
  else  {
    int pthc = Params[0].ToInt();
    int rthc = wxThread::GetCPUCount();
    if( rthc != -1 && pthc > rthc )  {
      E.ProcessingError(__OlxSrcInfo, "Number of proposed threads is larger than number of phisical ones");
      return;
    }
    if( pthc > 0 )
      FXApp->SetMaxThreadCount(pthc);
    else if( pthc == -1 )  {
      if( rthc == -1 )  {
        E.ProcessingError(__OlxSrcInfo, "Could not determine the number of CPU");
        return;
      }
      FXApp->SetMaxThreadCount(rthc);
    }
  }
}
//..............................................................................
void TMainForm::macPictS(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  int orgHeight = FXApp->GetRender().GetHeight(),
      orgWidth  = FXApp->GetRender().GetWidth();
  double res = 1,
    half_ang = Options.FindValue('a', "6").ToDouble()/2,
    sep_width = Options.FindValue('s', "10").ToDouble();
  if( Cmds.Count() == 2 && Cmds[1].IsNumber() )  
    res = Cmds[1].ToDouble();
  if( res >= 100 )  // width provided
    res /= orgWidth;
  if( res > 10 )
    res = 10;
  if( res <= 0 )  
    res = 1;
  if( res > 1 && res < 100 )
    res = olx_round(res);

  int SrcHeight = (int)(((double)orgHeight/(res*2)-1.0)*res*2),
      SrcWidth  = (int)(((double)orgWidth/(res*2)-1.0)*res*2);
  SrcHeight = Options.FindValue('h', SrcHeight).ToInt();
  int BmpHeight = (int)(SrcHeight*res);
  if( (BmpHeight%2) != 0 )  BmpHeight++;
  int BmpWidth = (int)(SrcWidth*res),
      ImgWidth = olx_round((double)BmpWidth*(2 + sep_width/100));  // sep_width% in between pictures
  const int imgOff = ImgWidth - BmpWidth;
  if( BmpHeight < SrcHeight )
    SrcHeight = BmpHeight;
  if( BmpWidth < SrcWidth )
    SrcWidth = BmpWidth;
  FXApp->GetRender().Resize(0, 0, SrcWidth, SrcHeight, res); 

  const int bmpSize = BmpHeight*ImgWidth*3;
  char* bmpData = (char*)malloc(bmpSize);
  memset(bmpData, ~0, bmpSize);
  FGlConsole->Visible(false);
  FXApp->GetRender().OnDraw->SetEnabled(false);
  if( res != 1 )    {
    FXApp->GetRender().GetScene().ScaleFonts(res);
    if( res >= 3 )
      FXApp->Quality(qaPict);
  }
  const double RY = FXApp->GetRender().GetBasis().GetRY();
  FXApp->GetRender().GetBasis().RotateY(RY-half_ang);
  for( int i=0; i < res; i++ )  {
    for( int j=0; j < res; j++ )  {
      FXApp->GetRender().LookAt(j, i, (int)(res < 1 ? 1 : res));
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels(false, 1);
      const int mj = j*SrcWidth;
      const int mi = i*SrcHeight;
      for( int k=0; k < SrcWidth; k++ )  {
        for( int l=0; l < SrcHeight; l++ )  {
          const int indexA = (l*SrcWidth + k)*3;
          const int indexB = bmpSize - (ImgWidth*(mi + l + 1) - mj - k)*3;
          bmpData[indexB] = PP[indexA];
          bmpData[indexB+1] = PP[indexA+1];
          bmpData[indexB+2] = PP[indexA+2];
        }
      }
      delete [] PP;
    }
  }
  // second half
  FXApp->GetRender().GetBasis().RotateY(RY+half_ang);
  for( int i=0; i < res; i++ )  {
    for( int j=0; j < res; j++ )  {
      FXApp->GetRender().LookAt(j, i, (int)(res < 1 ? 1 : res));
      FXApp->GetRender().Draw();
      char *PP = FXApp->GetRender().GetPixels(false, 1);
      const int mj = j*SrcWidth;
      const int mi = i*SrcHeight;
      for( int k=0; k < SrcWidth; k++ )  {
        for( int l=0; l < SrcHeight; l++ )  {
          const int indexA = (l*SrcWidth + k)*3;
          const int indexB = bmpSize - (ImgWidth*(mi + l + 1) - mj - k - imgOff)*3;
          bmpData[indexB] = PP[indexA];
          bmpData[indexB+1] = PP[indexA+1];
          bmpData[indexB+2] = PP[indexA+2];
        }
      }
      delete [] PP;
    }
  }
  FXApp->GetRender().GetBasis().RotateY(RY);
  if( res != 1 ) {
    FXApp->GetRender().GetScene().RestoreFontScale();
    if( res >= 3 ) 
      FXApp->Quality(qaMedium);
  }

  FXApp->GetRender().OnDraw->SetEnabled( true );
  FGlConsole->Visible(true);
  // end drawing etc
  FXApp->GetRender().Resize(orgWidth, orgHeight); 
  FXApp->GetRender().LookAt(0,0,1);
  FXApp->GetRender().SetView(false, 1);
  FXApp->Draw();
  olxstr bmpFN;
  if( FXApp->XFile().HasLastLoader() && !TEFile::IsAbsolutePath(Cmds[0]) )
    bmpFN = TEFile::ExtractFilePath(FXApp->XFile().GetFileName()) << TEFile::ExtractFileName(Cmds[0]);
  else
    bmpFN = Cmds[0];
  wxImage image;
  image.SetData((unsigned char*)bmpData, ImgWidth, BmpHeight); 
  // correct a common typo
  if( TEFile::ExtractFileExt(bmpFN).Equalsi("jpeg") )
    bmpFN = TEFile::ChangeFileExt(bmpFN, "jpg");
  image.SaveFile(bmpFN.u_str());
}
//..............................................................................
void TMainForm::funFullScreen(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )  E.SetRetVal(IsFullScreen());
  else  {
    if( Params[0].Equalsi("swap") )
      ShowFullScreen(!IsFullScreen());
    else
      ShowFullScreen(Params[0].ToBool());
  }
}
