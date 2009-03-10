//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "exception.h"
#include "estrlist.h"
//---------------------------------------------------------------------------
USEFORM("..\..\..\lcells\about.cpp", dlgAbout);
USEFORM("..\..\..\lcells\iinfo.cpp", dlgIndexInfo);
USEFORM("..\..\..\lcells\Main.cpp", dlgMain);
USEFORM("..\..\..\lcells\moldraw.cpp", dlgMolDraw);
USEFORM("..\..\..\lcells\msearch.cpp", dlgSearch);
USEFORM("..\..\..\lcells\preferences.cpp", dlgPref);
USEFORM("..\..\..\lcells\progress.cpp", dlgProgress);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  try
  {
     Application->Initialize();
     Application->Title = "LCells";
     Application->CreateForm(__classid(TdlgMain), &dlgMain);
     Application->CreateForm(__classid(TdlgMolDraw), &dlgMolDraw);
     Application->Run();
  }
  catch (Exception &exception)
  {
     Application->ShowException(&exception);
  }
  catch (const TExceptionBase &e) {
    TStrList out;
    e.GetException()->GetStackTrace(out);
    Application->MessageBox(out.Text('\n').c_str(), "Exception", MB_OK);
  }
  catch (...)
  {
     try
     {
       throw Exception("");
     }
     catch (Exception &exception)
     {
       Application->ShowException(&exception);
     }
  }
  return 0;
}
//---------------------------------------------------------------------------






