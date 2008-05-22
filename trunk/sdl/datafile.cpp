//---------------------------------------------------------------------------//
// TDataFile
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "dataitem.h"
#include "datafile.h"
#include "efile.h"
#include "estrbuffer.h"
#include "utf8file.h"

UseEsdlNamespace()
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
bool TDataFile::LoadFromXLFile(const olxstr &DataFile, TStrList* Log)  {
  TWStrList File;
  olxstr Data;
  FRoot->Clear();
  FFileName = EmptyString;
  TEFile::CheckFileExists(__OlxSourceInfo, DataFile);
  try  {  TUtf8File::ReadLines(DataFile, File, false);  }
  catch( ... )  {  return false;  }

  if( File.IsEmpty() )  return false;
  FFileName = DataFile;
  olxstr OData( File.Text(' ') );
  OData.DeleteSequencesOf(' ');
  OData.Replace('\t', ' ');
  Data.SetCapacity( OData.Length() );
  int dl = OData.Length();
  for( int i=0; i < dl; i++ )  {
    if( OData.CharAt(i) == '<' )  {
      while( (i+1) < dl && OData.CharAt(i+1) == ' ')  // skip spaces
        i++;
    }
    Data << OData.CharAt(i);
  }
  FRoot->LoadFromString(0, Data, Log);  
  return true;
}
//..............................................................................
void TDataFile::Include(TStrList* Log)  {
  TDataItem *Inc;
  TDataFile *DF;
  olxstr Tmp;
  DF = new TDataFile;
  Inc = FRoot->GetAnyItem("#include");
  while( Inc )  {
    Tmp = TEFile::ExtractFilePath( Inc->GetValue() );
    if( Tmp.IsEmpty() )
      Tmp = TEFile::ExtractFilePath(FFileName);
    Tmp << TEFile::ExtractFileName(Inc->GetValue());

    if( !TEFile::FileExists(Tmp) )  {
      if( Log != NULL )
        Log->Add(olxstr("Included filed does not exist: ") << Tmp );
      FRoot->DeleteItem(Inc);
      Inc = FRoot->GetAnyItem("#include");
      continue;
    }
    DF->LoadFromXLFile(Tmp, Log);
    DF->Include(Log);
    Inc->GetParent()->AddContent(DF->Root());

    FRoot->DeleteItem(Inc);
    Inc = FRoot->GetAnyItem("#include");
  }
  FRoot->ResolveFields(Log);
  delete DF;

}
//..............................................................................
void TDataFile::SaveToXLFile(const olxstr &DataFile)  {
  FFileName = DataFile;
  TEStrBuffer bf;
  FRoot->SaveToStrBuffer(bf);
  TUtf8File::Create( DataFile, bf.ToString() );
}
//..............................................................................


