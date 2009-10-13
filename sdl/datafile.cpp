//---------------------------------------------------------------------------//
// TDataFile
// (c) Oleg V. Dolomanov, 2004-2009
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "dataitem.h"
#include "datafile.h"
#include "efile.h"
#include "estrbuffer.h"
#include "utf8file.h"
#include "exparse/exptree.h"

UseEsdlNamespace()
using namespace exparse::parser_util;
//..............................................................................
//..............................................................................

TDataFile::TDataFile()  {
  FRoot = new TDataItem(NULL, "Root");
}
//..............................................................................
TDataFile::~TDataFile()  {
  delete FRoot;
}
//..............................................................................
//..............................................................................
bool TDataFile::LoadFromTextStream(IInputStream& io, TStrList* Log)  {
  WString in;
  FRoot->Clear();
  FileName = EmptyString;
  try  {  in = TUtf8File::ReadAsString(io, false);  }
  catch( ... )  {  return false;  }
  if( in.IsEmpty() )  return false;
  in = in.DeleteChars( olxstr("\n\r") ).Replace('\t', ' ').DeleteSequencesOf(' ');
  for( int i=0; i < in.Length(); i++ )  
    if( in.CharAt(i) == '<' )  {
      FRoot->LoadFromString(i, in, Log);  
      break;
    }
  return true;
}
//..............................................................................
bool TDataFile::LoadFromXLFile(const olxstr &DataFile, TStrList* Log)  {
  TEFile in(DataFile, "rb");
  return LoadFromTextStream(in, Log);
}
//..............................................................................
void TDataFile::Include(TStrList* Log)  {
  TDataItem *Inc;
  TDataFile DF;
  olxstr Tmp;
  Inc = FRoot->GetAnyItem("#include");
  while( Inc != NULL )  {
    Tmp = TEFile::ExtractFilePath( Inc->GetValue() );
    if( Tmp.IsEmpty() )
      Tmp = TEFile::ExtractFilePath(FileName);
    if( Tmp.IsEmpty() )
      Tmp = Inc->GetValue();
    else
      Tmp << TEFile::ExtractFileName(Inc->GetValue());

    if( !TEFile::Exists(Tmp) )  {
      if( Log != NULL )
        Log->Add(olxstr("Included file does not exist: ") << Tmp );
      FRoot->DeleteItem(Inc);
      Inc = FRoot->GetAnyItem("#include");
      continue;
    }
    DF.LoadFromXLFile(Tmp, Log);
    DF.Include(Log);
    Inc->GetParent()->AddContent( DF.Root() );

    FRoot->DeleteItem(Inc);
    Inc = FRoot->GetAnyItem("#include");
  }
  FRoot->ResolveFields(Log);
}
//..............................................................................
void TDataFile::SaveToXLFile(const olxstr &DataFile)  {
  FileName = DataFile;
  TEStrBuffer bf(1024*32);
  FRoot->SaveToStrBuffer(bf);
  TUtf8File::Create( DataFile, bf.ToString() );
}
//..............................................................................


