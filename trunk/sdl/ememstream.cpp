//---------------------------------------------------------------------------//
// see TIniStream implementation for details
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <string.h>
#include "ememstream.h"
#include "exception.h"
#include "efile.h"


UseEsdlNamespace()
//----------------------------------------------------------------------------//
//TStream function bodies
//----------------------------------------------------------------------------//
TEMemoryStream::TEMemoryStream(IInputStream& is) : TDirectionalList<char>(is.GetSize()+1)  {
  Position = 0;
  is >> *(IOutputStream*)this;
  SetPosition(0);
}
//..............................................................................
void TEMemoryStream::operator >> (IOutputStream &os)  {
  size_t pos = Position;
  TDirectionalListEntry<char>* en = GetEntryAtPosition( pos );
  if( en == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "no entry at specified position");
  os.Write( en->GetData(), en->GetSize()-pos );
  while( (en = en->GetNext()) != NULL )
    os.Write( en->GetData(), en->GetSize() );
  Position = GetLength();
}
//..............................................................................
IOutputStream& TEMemoryStream::operator << (IInputStream &is)  {
  this->CheckInitialised();

  // we are not at the end of the stream ..
  if( (GetLength() - Position) != 0 )  {
    long size = GetLength() - Position;
    char* mem = new char[ size ];
    is.Read( mem, size );
    // debug - remove the assignement after
    TDirectionalList<char>::Write( mem, Position, size );
    delete [] mem;
    size = is.GetSize() - size;
    mem = TTBuffer<char>::Alloc(size);
    TDirectionalListEntry<char>* en = GetTail();
    is.Read( mem, size );
    // this takes the ownership of the allocated memory
    en->AddEntry( mem, size );
    UpdateLength();
    Position += is.GetSize();
  }
  else  {
    TDirectionalListEntry<char>* en = GetTail();
    long size = is.GetSize();
    if( en->GetCapacity() - en->GetSize() )  {
      int towrite = olx_min(en->GetCapacity() - en->GetSize(), size);
      char* bf = new char[towrite];
      is.Read(bf, towrite);
      en->Write(bf, towrite);
      delete [] bf;
      size -= towrite;
      Position = towrite;
    }
    if( size )  {
      char* mem = TTBuffer<char>::Alloc(size);
      is.Read( mem, size );
      en->AddEntry( mem, size );
      UpdateLength();
      Position = GetLength();
    }
  }
  return *this;
}
//..............................................................................
void TEMemoryStream::SaveToFile(const olxstr& FN)  {
  TEFile file(FN, "w+b");
  long pos = GetPosition();
  SetPosition(0);
  file << *this;
  SetPosition(pos);
}
//..............................................................................
void TEMemoryStream::LoadFromFile(const olxstr& FN)  {
  TEFile file(FN, "rb");
  file >> *this;
  SetPosition(0);
}

