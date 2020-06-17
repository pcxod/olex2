/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "settingsfile.h"
#include "utf8file.h"
#include "exception.h"

void TSettingsFile::LoadSettings(const olxstr& fileName)  {
  Clear();
  TEFile::CheckFileExists(__OlxSourceInfo, fileName);
  TEFile::ReadLines(fileName, Lines);

  for( size_t i=0; i < Lines.Count(); i++ )  {
    olxstr ln = Lines[i].Trim(' ');
    size_t ind = ln.FirstIndexOf('=');
    if( ind == InvalidIndex || ln.StartsFrom('#') || ln.IsEmpty() )  {
      Lines.GetObject(i) = false;
      continue;
    }
    else  {
      olxstr pn = ln.SubStringTo(ind).Trim(' ');
      olxstr pv = ln.SubStringFrom(ind+1).Trim(' ');
      ind = Params.IndexOf(pn);
      // in case of duplicate params - keep the latest value
      if( ind == InvalidIndex )  {
        Params.Add(pn, pv);
        Lines.GetObject(i) = true;
        Lines[i] = pn;
      }
      else  {
        Lines[i].SetLength(0);
        Lines.GetObject(i) = false;
        Params.GetValue(ind) = pv;
      }
    }
  }
}
//..............................................................................
void TSettingsFile::SaveSettings(const olxstr& fileName)  {
  olx_object_ptr<TUtf8File> f(TUtf8File::Create(fileName,false));
  for( size_t i=0; i < Lines.Count(); i++ )  {
    if (!Lines.GetObject(i)) {
      f->Writeln(Lines[i]);
    }
    else {
      f->Writeln(olxstr(Lines[i]) << '=' << Params.Get(Lines[i]));
    }
  }
}
//..............................................................................
