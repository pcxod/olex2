/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "libfile.h"
#include "efile.h"

void LibFile::FileExists(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::Exists(Params[0]));
}

void LibFile::FileName(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::ExtractFileName(Params[0]));
}

void LibFile::FilePath(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::ExtractFilePath(Params[0]));
}

void LibFile::FileDrive(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::ExtractFileDrive(Params[0]));
}

void LibFile::FileExt(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::ExtractFileExt(Params[0]));
}

void LibFile::ChangeFileExt(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::ChangeFileExt(Params[0], Params[1]));
}

void LibFile::Copy(const TStrObjList& Params, TMacroData& E)  {
  TEFile::Copy(Params[0], Params[1]);
  E.SetRetVal(Params[1]);
}

void LibFile::Rename(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::Rename(Params[0], Params[1]));
}

void LibFile::Delete(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::DelFile(Params[0]));
}

void LibFile::CurDir(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::CurrentDir());
}

void LibFile::ChDir(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::ChangeDir(Params[0]));
}

void LibFile::MkDir(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::MakeDir(Params[0]));
}

void LibFile::OSPath(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::OSPath(Params[0]));
}

void LibFile::Which(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::Which(Params[0]));
}

void LibFile::Age(const TStrObjList& Params, TMacroData& E)  {
  TEFile::CheckFileExists(__OlxSourceInfo, Params[0]);
  time_t v = TEFile::FileAge(Params[0]);
  if( Params.Count() == 1 )
    E.SetRetVal(TETime::FormatDateTime(v));
  else
    E.SetRetVal(TETime::FormatDateTime(Params[1], v));
}

void LibFile::ListDirForGUI(const TStrObjList& Params, TMacroData& E)  {
  TEFile::CheckFileExists(__OlxSourceInfo, Params[0]);
  olxstr cd( TEFile::CurrentDir() );
  olxstr dn(Params[0]);
  TEFile::AddPathDelimeterI(dn);
  TEFile::ChangeDir( Params[0] );
  short attrib = sefFile;
  if( Params.Count() == 3 )  {
    if( Params[2].Equalsi("fd") )
      attrib |= sefDir;
    else if( Params[2].CharAt(0) == 'd' )
      attrib = sefDir;
  }
  TStrList output;
  olxstr tmp;
  TEFile::ListCurrentDir(output, Params[1], attrib);
  TEFile::ChangeDir(cd);
#ifdef __BORLANDC__
  output.QuickSort< TStringWrapperComparator<TSingleStringWrapper<olxstr>,true> >();
#else  // borland dies here...
  output.QSort(false);
#endif
  for( size_t i=0; i < output.Count(); i++ )  {
   tmp.SetLength(0);
    tmp <<  "<-" << dn << output[i];
    output[i] << tmp;
  }
  E.SetRetVal( output.Text(';') );
}

void LibFile::CreateRelativePath(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::CreateRelativePath(Params[0],
    Params.Count() == 2 ? Params[1] : EmptyString()));
}

void LibFile::ExpandRelativePath(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TEFile::ExpandRelativePath(Params[0],
    Params.Count() == 2 ? Params[1] : EmptyString()));
}

TLibrary*  LibFile::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("file") : name);
  lib->Register(
    new TStaticFunction(&LibFile::FileExists, "Exists", fpOne,
    "Returns true if specified file exists")
  );
  lib->Register(
    new TStaticFunction(&LibFile::FileName, "GetName", fpOne,
    "Returns name part of the full/partial file name")
  );
  lib->Register(
    new TStaticFunction(&LibFile::FilePath, "GetPath", fpOne,
    "Returns path component of the full file name")
  );
  lib->Register(
    new TStaticFunction(&LibFile::FileDrive, "GetDrive", fpOne,
    "Returns drive component of the full file name")
  );
  lib->Register(
    new TStaticFunction(&LibFile::FileExt, "GetExt", fpOne,
    "Returns file extension")
    );
  lib->Register(
    new TStaticFunction(&LibFile::ChangeFileExt, "ChangeExt", fpTwo,
    "Returns file name with changed extension")
  );
  lib->Register(
    new TStaticFunction(&LibFile::Copy, "Copy", fpTwo,
    "Copies file provided as first argument into the file provided as second"
    " argument")
  );
  lib->Register(
    new TStaticFunction(&LibFile::Delete, "Delete", fpOne,
    "Deletes specified file")
  );
  lib->Register(
    new TStaticFunction(&LibFile::Rename, "Rename", fpTwo,
    "Renames specified file")
  );
  lib->Register(
    new TStaticFunction(&LibFile::CurDir, "CurDir", fpNone,
    "Returns current folder")
  );
  lib->Register(
    new TStaticFunction(&LibFile::ChDir, "ChDir", fpOne,
    "Changes current folder to provided folder")
  );
  lib->Register(
    new TStaticFunction(&LibFile::MkDir, "MkDir", fpOne,
    "Creates specified folder")
  );
  lib->Register(
    new TStaticFunction(&LibFile::OSPath, "OSPath", fpOne,
    "Returns OS specific path for provided path")
  );
  lib->Register(
    new TStaticFunction(&LibFile::Which, "Which", fpOne,
    "Tries to find a particular file looking at current folder, PATH and "
    "program folder")
  );
  lib->Register(
    new TStaticFunction(&LibFile::Age, "Age", fpOne|fpTwo,
    "Returns file age for provided file using formatting string (if provided)")
  );
  lib->Register(
    new TStaticFunction(&LibFile::ListDirForGUI, "ListDirForGUI",
    fpTwo|fpThree,
    "Returns a ready to use in GUI list of files, matching provided mask(s) "
    "separated by semicolon. The third, optional argument [f,d,fd] specifies "
    "what should be included into the list")
  );
  lib->Register(
    new TStaticFunction(&LibFile::CreateRelativePath, "RelativePath",
    fpOne|fpTwo,
    "Returns a path to a folder relative to basedir; arguments are "
    "(base=basedir,path)")
  );
  lib->Register(
    new TStaticFunction(&LibFile::ExpandRelativePath, "AbsolutePath",
    fpOne|fpTwo,
    "Returns an absolute path to a folder relative to the basedir; arguments "
    "are (base=basedir,path)")
  );
  return lib;
}
