#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "settingsfile.h"
#include "efile.h"
#include "exception.h"

//..............................................................................
TSettingsFile::TSettingsFile( const olxstr& fileName )  {
  LoadSettings( fileName );
}
//..............................................................................
TSettingsFile::~TSettingsFile()  {
  Clear();
}
//..............................................................................
void TSettingsFile::LoadSettings(const olxstr& fileName)  {
  Clear();
  TEFile::CheckFileExists(__OlxSourceInfo, fileName);
  TStrList strs;
  strs.LoadFromFile(fileName);

  for( int i=0; i < strs.Count(); i++ )  {
    if( strs[i].IsEmpty() )  continue;
    
    int ind = strs[i].FirstIndexOf('=');
    if( ind < 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("no assignement char at line ") << (i+1));
    Params.Add( strs[i].SubStringTo(ind), strs[i].SubStringFrom(ind+1) );
  }
}
//..............................................................................
void TSettingsFile::SaveSettings(const olxstr& fileName)  {
  TEFile f( fileName, "w+b" );
  olxstr line;
  for( int i=0; i < Params.Count(); i++ )  {
    line = Params.GetComparable(i);
    line << '=' << Params.GetObject(i);
    f.Writenl( line.c_str(), line.Length() );
  }
}
//..............................................................................
void TSettingsFile::Clear()  {
  Params.Clear();
}
//..............................................................................
void TSettingsFile::UpdateParam(const olxstr& paramName, const olxstr& val)  {
  int index = Params.IndexOfComparable( paramName );
  if( index != -1 )
    Params.GetObject(index) = val;
  else
    Params.Add(paramName, val);
}
//..............................................................................

