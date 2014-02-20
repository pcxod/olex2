/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include <string.h>
#include "ememstream.h"
#include "exception.h"
#include "efile.h"
UseEsdlNamespace()

TEMemoryStream::TEMemoryStream(IInputStream& is) :
  TDirectionalList<char>(OlxIStream::CheckSizeT(is.GetSize()+1))
{
  Position = 0;
  is >> *(IOutputStream*)this;
  SetPosition(0);
}
//..............................................................................
void TEMemoryStream::operator >> (IOutputStream &os)  {
  size_t pos = Position;
  TDirectionalListEntry<char>* en = GetEntryAtPosition(pos);
  if( en == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "no entry at specified position");
  os.Write( en->GetData(), en->GetSize()-pos );
  while( (en = en->GetNext()) != NULL )
    os.Write( en->GetData(), en->GetSize() );
  Position = GetLength();
}
//..............................................................................
TEMemoryStream& TEMemoryStream::operator << (IInputStream &is)  {
  this->CheckInitialised();
  const size_t _off = GetLength() - Position;
  // we are not at the end of the stream ..
  if( _off != 0 )  {
    const size_t _asize = is.GetAvailableSizeT();
    if( _asize < _off )  {  // have enough room for the is
      char* mem = new char[_asize];
      is.Read(mem, _asize);
      TDirectionalList<char>::Write(mem, Position, _asize);
      delete [] mem;
      UpdateLength();
      Position += _asize;
    }
    else  {  // have to created a new segment
      size_t size = GetLength() - Position;
      char* mem = new char[size];
      is.Read(mem, size);
      TDirectionalList<char>::Write(mem, Position, size);
      delete [] mem;
      size = is.GetAvailableSizeT();
      mem = TTBuffer<char>::Alloc(size);
      TDirectionalListEntry<char>* en = GetTail();
      is.Read(mem, size);
      // this takes the ownership of the allocated memory
      en->AddEntry(mem, size);
      UpdateLength();
      Position += _asize;
    }
  }
  else  {
    TDirectionalListEntry<char>* en = GetTail();
    size_t size = is.GetAvailableSizeT();
    if( (en->GetCapacity() - en->GetSize()) != 0 )  {
      size_t towrite = olx_min(en->GetCapacity() - en->GetSize(), size);
      char* bf = new char[towrite];
      is.Read(bf, towrite);
      en->Write(bf, towrite);
      delete [] bf;
      size -= towrite;
      Position = towrite;
    }
    if( size != 0 )  {
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
  const size_t pos = Position;
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
