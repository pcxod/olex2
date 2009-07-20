//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("..\..\..\installer\licence.cpp", dlgLicence);
USEFORM("..\..\..\installer\main.cpp", fMain);
USEFORM("frame1.cpp", frMain); /* TFrame: File Type */
USEFORM("..\..\..\installer\uninstall.cpp", dlgUninstall);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  try
  {
     Application->Initialize();
     Application->Title = "Olex2 Installer";
     Application->CreateForm(__classid(TfMain), &fMain);
     Application->CreateForm(__classid(TdlgLicence), &dlgLicence);
     Application->Run();
  }
  catch (Exception &exception)
  {
     Application->ShowException(&exception);
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
