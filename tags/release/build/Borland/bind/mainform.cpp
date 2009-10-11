//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "mainform.h"

#include "pbase.h"
#include "evector.h"

#include "cparser.h"

#include "bapp.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMF *MF;
//---------------------------------------------------------------------------
__fastcall TMF::TMF(TComponent* Owner)
  : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void Test()  {
  BObjects objects;
  objects.NewObject<void>();

  objects.NewObject<bool>();
  objects.NewObject<bool[]>();
  objects.NewObject<bool*>();
  objects.NewObject<const bool*>();
  objects.NewObject<bool&>();
  objects.NewObject<const bool&>();
  objects.NewObject<const bool*&>();
  objects.NewObject<const bool *const &>();

  objects.NewObject<char>();
  objects.NewObject<char[]>();
  objects.NewObject<char*>();
  objects.NewObject<const char*>();
  objects.NewObject<char&>();
  objects.NewObject<const char&>();
  objects.NewObject<const char*&>();
  objects.NewObject<const char *const &>();
  objects.NewObject<unsigned char>();

  objects.NewObject<int>();
  objects.NewObject<int[]>();
  objects.NewObject<int*>();
  objects.NewObject<const int*>();
  objects.NewObject<int&>();
  objects.NewObject<const int&>();
  objects.NewObject<const int*&>();
  objects.NewObject<const int *const &>();
  objects.NewObject<unsigned int>();

  objects.NewObject<short int>();
  objects.NewObject<short int[]>();
  objects.NewObject<short int*>();
  objects.NewObject<const short int*>();
  objects.NewObject<short int&>();
  objects.NewObject<const short int&>();
  objects.NewObject<const short int*&>();
  objects.NewObject<const short int *const &>();
  objects.NewObject<unsigned short int>();

  objects.NewObject<long int>();
  objects.NewObject<long int[]>();
  objects.NewObject<long int*>();
  objects.NewObject<const long int*>();
  objects.NewObject<long int&>();
  objects.NewObject<const long int&>();
  objects.NewObject<const long int*&>();
  objects.NewObject<const long int *const &>();
  objects.NewObject<unsigned long int>();

  objects.NewObject<float>();
  objects.NewObject<float[]>();
  objects.NewObject<float*>();
  objects.NewObject<const float*>();
  objects.NewObject<float&>();
  objects.NewObject<const float&>();
  objects.NewObject<const float*&>();
  objects.NewObject<const float *const &>();

  objects.NewObject<double>();
  objects.NewObject<double[]>();
  objects.NewObject<double*>();
  objects.NewObject<const double*>();
  objects.NewObject<double&>();
  objects.NewObject<const double&>();
  objects.NewObject<const double*&>();
  objects.NewObject<const double *const &>();

  objects.NewObject<long double>();
  objects.NewObject<long double[]>();
  objects.NewObject<long double*>();
  objects.NewObject<const long double*>();
  objects.NewObject<long double&>();
  objects.NewObject<const long double&>();
  objects.NewObject<const long double*&>();
  objects.NewObject<const long double *const &>();

  objects.NewObject<evecd>();
  BObject* strObj = objects.NewObject<olxstr>();

  BFunction* bf = new BFunction(objects.FindObject<int>(), "test");
  bf->AddArg( new BArg(objects.FindObject<double>(), "a1") );
  objects.AddFunction(bf);

  strObj->AddFunction(bf);
  TStrList out;
  for( int i=0; i < objects.ObjectCount(); i++ )
    MF->reEdit->Lines->Add( objects.Object(i).ToCString(EmptyString).c_str() );
  for( int i=0; i < objects.FunctionCount(); i++ )
    MF->reEdit->Lines->Add( objects.Function(i).ToCHString().c_str() );

  objects.PyBind(strObj, out );
  for( int i=0; i < out.Count(); i++ )
    MF->reEdit->Lines->Add( out[i].c_str() );
}
void __fastcall TMF::Test1Click(TObject *Sender)
{
  Test();
}
//---------------------------------------------------------------------------

void __fastcall TMF::CPP1Click(TObject *Sender)  {
  if( dlgOpen->Execute() )  {
    TBasicApp bapp( ParamStr(0).c_str() );
    CParser cp;
    cp.AddPath( "c:/Program Files (x86)/Borland/CBuilder6/Include/" );
    cp.AddPath( "c:/Program Files (x86)/Borland/CBuilder6/Include/Stlport/" );
    cp.AddPath( "c:/Program Files/Borland/CBuilder6/Include/" );
    cp.AddPath( "c:/Program Files/Borland/CBuilder6/Include/Stlport/" );
    cp.AddPath( "E:/SVN/olex2-sf/trunk/sdl/" );
    cp.AddPath( "E:/SVN/olex2-sf/trunk/xlib/" );
    cp.AddPath( "E:/SVN/olex2-sf/trunk/glib/" );
    cp.AddPath( "E:/SVN/olex2-sf/trunk/gxlib/" );
    cp.AddPath( "E:/wxWidgets-2.8.7/include/" );
    try  {
      TStrList& rv = cp.Parse( dlgOpen->FileName.c_str() );
      reEdit->Lines->BeginUpdate();
      for( int i=0; i < rv.Count(); i++ )
        reEdit->Lines->Add( rv[i].c_str() );
      reEdit->Lines->EndUpdate();
    }
    catch( TExceptionBase& exc )  {
      Application->MessageBox( exc.GetException()->GetFullMessage().c_str(), EsdlClassName(exc).c_str(), 0 );
    }
  }

}
//---------------------------------------------------------------------------

