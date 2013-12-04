/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "dataitem.h"
#include "datafile.h"
#include "efile.h"
#include "estrbuffer.h"
#include "utf8file.h"
#include "exparse/exptree.h"
#include "integration.h"

UseEsdlNamespace()
using namespace exparse::parser_util;

TDataFile::TDataFile()  {
  FRoot = new TDataItem(NULL, "Root");
}
//.............................................................................
TDataFile::~TDataFile()  {
  delete FRoot;
}
//.............................................................................
bool TDataFile::LoadFromTextStream(IInputStream& io, TStrList* Log)  {
  olxwstr in;
  FRoot->Clear();
  FileName.SetLength(0);
  try  {  in = TUtf8File::ReadAsString(io, false);  }
  catch( ... )  {  return false;  }
  in.DeleteCharSet("\n\r");
  if( in.IsEmpty() )  return false;
  for( size_t i=0; i < in.Length(); i++ )
    if( in.CharAt(i) == '<' )  {
      FRoot->LoadFromString(i, in, Log);
      break;
    }
  return true;
}
//.............................................................................
bool TDataFile::LoadFromXMLTextStream(IInputStream& io, TStrList* Log)  {
  olxwstr in;
  FRoot->Clear();
  FileName.SetLength(0);
  try  { in = TUtf8File::ReadAsString(io, false); }
  catch (...)  { return false; }
  in.DeleteCharSet("\n\r");
  if (in.IsEmpty())  return false;
  for (size_t i = 0; i < in.Length(); i++)
  if (in.CharAt(i) == '<') {
    FRoot->LoadFromXMLString(i, in, Log);
    break;
  }
  return true;
}
//.............................................................................
bool TDataFile::LoadFromXLFile(const olxstr &DataFile, TStrList* Log)  {
  try  {
    TEFile in(DataFile, "rb");
    bool res = LoadFromTextStream(in, Log);
    FileName = DataFile;
    return res;
  }
  catch(const TExceptionBase& e)  {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
}
//.............................................................................
bool TDataFile::LoadFromXMLFile(const olxstr &DataFile, TStrList* Log) {
  try {
    TEFile in(DataFile, "rb");
    bool res = LoadFromXMLTextStream(in, Log);
    FileName = DataFile;
    return res;
  }
  catch (const TExceptionBase& e) {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
}
//.............................................................................
void TDataFile::Include(TStrList* Log)  {
  TDataItem *Inc = FRoot->FindAnyItem("#include");
  olex2::IOlex2Processor *op = olex2::IOlex2Processor::GetInstance();
  while( Inc != NULL )  {
    olxstr Tmp = Inc->GetValue();
    if (op != NULL)
      op->processFunction(Tmp);
    if (!TEFile::IsAbsolutePath(Tmp))
      Tmp = TEFile::ExtractFilePath(FileName) + Tmp;

    if (!TEFile::Exists(Tmp)) {
      if (Log != NULL)
        Log->Add("Included file does not exist: ").quote() << Tmp;
      FRoot->DeleteItem(Inc);
      Inc = FRoot->FindAnyItem("#include");
      continue;
    }
    TDataFile DF;
    DF.LoadFromXLFile(Tmp, Log);
    DF.Include(Log);
    const olxstr& extend_str = Inc->FindField("extend");
    bool extend = extend_str.IsEmpty() ? false : extend_str.ToBool();
    Inc->GetParent()->AddContent(DF.Root(), extend);
    FRoot->DeleteItem(Inc);
    Inc = FRoot->FindAnyItem("#include");
  }
  FRoot->ResolveFields(Log);
}
//.............................................................................
void TDataFile::SaveToXLFile(const olxstr &DataFile)  {
  FileName = DataFile;
  TEStrBuffer bf(1024*32);
  FRoot->SaveToStrBuffer(bf);
  TUtf8File::Create(DataFile, bf.ToString());
}
//.............................................................................
void TDataFile::SaveToXMLFile(const olxstr &DataFile) {
  FileName = DataFile;
  TEStrBuffer bf(1024 * 32);
  FRoot->SaveToXMLStrBuffer(bf);
  TUtf8File::Create(DataFile, bf.ToString());
}
//.............................................................................
