/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "htmlswitch.h"
#include "bapp.h"
#include "log.h"
#include "utf8file.h"
#include "htmlext.h"
#include "fsext.h"
#include "wxzipfs.h"
#include "integration.h"

void THtmlSwitch::Clear()  {
  Switches.Clear();
  Strings.Clear();
  //Params.Clear();
}
//..............................................................................
void THtmlSwitch::SetFileIndex(size_t ind)  {
  FileIndex = ind;
  ParentHtml->SetSwitchState(*this, FileIndex);
}
//..............................................................................
void THtmlSwitch::UpdateFileIndex()  {
  Clear();
  if( FileIndex == InvalidIndex || FileIndex >= FileCount() )  return;
  olxstr FN = Files[FileIndex];
  IInputStream *is = TFileHandlerManager::GetInputStream(FN);
  if( is == NULL )  {
    TBasicApp::NewLogEntry(logError) <<
      (olxstr("THtmlSwitch::File does not exist: ").quote() << FN);
    return;
  }
#ifdef _UNICODE
  TUtf8File::ReadLines(*is, Strings, false);
#else
  Strings.LoadFromTextStream(*is);
#endif
  delete is;
  const olxstr ignore_open= "<!-- #ignoreif", ignore_close = "#ignoreif -->";
  for( size_t i=0; i < Strings.Count(); i++ )  {
    if (Strings[i].StartsFrom(ignore_open)) {
      olxstr fn = Strings[i].SubStringFrom(ignore_open.Length()).TrimWhiteChars();
      Strings[i].SetLength(0);
      if (fn.IsEmpty()) {
        TBasicApp::NewLogEntry(logError) <<
          (olxstr("Invalid ignoreif construct: ").quote() << Strings[i]);
      }
      size_t j=i+1;
      for (;j < Strings.Count(); j++) {
        if (Strings[j].StartsFrom(ignore_close))
          break;
      }
      if (j >= Strings.Count()) {
        TBasicApp::NewLogEntry(logError) << "Missing closing ignoreif";
        j = Strings.Count()-1;
      }
      else
        Strings[j].SetLength(0);
      olxstr rv = fn;
      if (olex::IOlexProcessor::GetInstance()->processFunction(rv) ) {
        if( rv.ToBool() )
          Strings.DeleteRange(i, j-i+1);
      }
      else {
        TBasicApp::NewLogEntry(logError) <<
          (olxstr("Invalid function in ingnoreif: ").quote() << fn);
      }
    }
    // replace the parameters with their values
    if( Strings[i].IndexOf('#') != InvalidIndex )  {
      // "key word parameter"
      Strings[i].Replace("#switch_name", Name);
      if( ParentSwitch != NULL )  {
        Strings[i].Replace("#parent_name", ParentSwitch->GetName()).\
                   Replace( "#parent_file", ParentSwitch->GetCurrentFile());
      }
      for( size_t j=0; j < Params.Count(); j++ ) {
        Strings[i].Replace(
          olxstr().Allocate(Params.GetName(j).Length()+2) <<
          '#' << Params.GetName(j),
          Params.GetValue(j));
      }
    } // end of parameter replacement
  }
  ParentHtml->CheckForSwitches(*this, TZipWrapper::IsZipFile(FN));
  for( size_t i=0; i < Switches.Count(); i++ )
    Switches[i].UpdateFileIndex();
}
//..............................................................................
bool THtmlSwitch::ToFile()  {
  if( Switches.IsEmpty() )  return true;
  if( GetCurrentFile().IsEmpty() )  return true;
  for( size_t i=0; i < Strings.Count(); i++ )  {
    if( Strings.GetObject(i) )  {
      THtmlSwitch *HO = Strings.GetObject(i);
      ParentHtml->UpdateSwitchState(*HO, Strings[i]);
      HO->ToFile();
    }
  }
#ifdef _UNICODE
  TUtf8File::WriteLines(GetCurrentFile(), Strings, false);
#else
  Strings.SaveToFile(GetCurrentFile());
#endif
  return true;
}
//..............................................................................
THtmlSwitch& THtmlSwitch::NewSwitch()  {
  return Switches.AddNew(ParentHtml, this);
}
//..............................................................................
size_t THtmlSwitch::FindSimilar(const olxstr& start, const olxstr& end,
  TPtrList<THtmlSwitch>& ret)
{
  size_t cnt = 0;
  for( size_t i=0; i < Switches.Count(); i++ )  {
    if( end.IsEmpty() )  {
      if( Switches[i].GetName().StartsFrom(start) )  {
        ret.Add(Switches[i]);
        cnt++;
      }
    }
    else if( start.IsEmpty() )  {
      if( Switches[i].GetName().EndsWith(end) )  {
        ret.Add(Switches[i]);
        cnt++;
      }
    }
    else {
      if( Switches[i].GetName().StartsFrom(start) &&
          Switches[i].GetName().EndsWith(end) )
      {
        ret.Add(Switches[i]);
        cnt++;
      }
    }
    cnt += Switches[i].FindSimilar(start, end, ret);
  }
  return cnt;
}
//..............................................................................
void THtmlSwitch::Expand(TPtrList<THtmlSwitch>& ret)  {
  for( size_t i=0; i < Switches.Count(); i++ )  {
    ret.Add(Switches[i]);
    Switches[i].Expand(ret);
  }
}
//..............................................................................
THtmlSwitch*  THtmlSwitch::FindSwitch(const olxstr &IName)  {
  for( size_t i=0; i < Switches.Count(); i++ )  {
    if( Switches[i].GetName().Equalsi(IName) )
      return &Switches[i];
    else  {
      THtmlSwitch* Res = Switches[i].FindSwitch(IName);
      if( Res != NULL )
        return Res;
    }
  }
  return NULL;
}
//..............................................................................
void THtmlSwitch::ToStrings(TStrList &List)  {
  if( FileIndex != InvalidIndex && FileIndex < Files.Count() )
    List.Add("<SWITCHINFOS SRC=\"")<< Files[FileIndex] << "\"/>";

  for( size_t i=0; i < Strings.Count(); i++ )  {
    if( Strings.GetObject(i) != NULL )
      Strings.GetObject(i)->ToStrings(List);
    List.Add(Strings[i]);
  }

  if( FileIndex != InvalidIndex && FileIndex < Files.Count() )
    List.Add("<SWITCHINFOE/>");
}
