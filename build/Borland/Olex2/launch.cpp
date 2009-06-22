//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("..\..\..\launch\main.cpp", dlgMain);
USEFORM("..\..\..\launch\splash.cpp", dlgSplash);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  try
  {
     Application->Initialize();
     Application->Title = "Olex2 Launcher";
     Application->CreateForm(__classid(TdlgMain), &dlgMain);
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

