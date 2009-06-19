#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "settingsfile.h"
#include "efile.h"
#include "exception.h"

//..............................................................................
void TSettingsFile::LoadSettings(const olxstr& fileName)  {
  Clear();
  TEFile::CheckFileExists(__OlxSourceInfo, fileName);
  Lines.LoadFromFile(fileName);

  for( int i=0; i < Lines.Count(); i++ )  {
    olxstr ln = Lines[i].Trim(' ');
    int ind = ln.FirstIndexOf('=');
    if( ind == -1 || ln.StartsFrom('#') || ln.IsEmpty() )  {
      Lines.GetObject(i) = false;
      continue;
    }
    else  {
      olxstr pn = ln.SubStringTo(ind).Trim(' ');
      olxstr pv = ln.SubStringFrom(ind+1).Trim(' ');
      ind = Params.IndexOf(pn);
      // in case of duplicate params - keep the latest value
      if( ind == -1 )  {
        Params.Add(pn, pv);
        Lines.GetObject(i) = true;
        Lines[i] = pn;
      }
      else  {
        Lines[i] = EmptyString;
        Lines.GetObject(i) = false;
        Params.GetValue(ind) = pv;
      }
    }
  }
}
//..............................................................................
void TSettingsFile::SaveSettings(const olxstr& fileName)  {
  TEFile f( fileName, "w+b" );
  for( int i=0; i < Lines.Count(); i++ )  {
    if( !Lines.GetObject(i) ) 
      f.Writenl( Lines[i].c_str(), Lines[i].Length() );
    else  {
      CString ln = Lines[i];
      ln << '=' << Params[Lines[i]];
      f.Writenl( ln );
    }
  }
}
//..............................................................................

