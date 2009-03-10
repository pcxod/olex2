//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#include <windows.h>
#include <winbase.h>
#endif

#include "fsext.h"
#include "ememstream.h"
#include "etime.h"
#include "egc.h"
#include "eutf8.h"

#ifndef _NO_PYTHON
#include "pyext.h"
#endif

#include "wxzipfs.h"
//#include "wxzipfsext.h"

  const int16_t TFileHandlerManager::FVersion = 0x0001;
  const char TFileHandlerManager::FSignature[]="ODF_"; // olex data file?
  const int TFileHandlerManager_FSignatureLength = 4;

TFileHandlerManager *TFileHandlerManager::FHandler = NULL;
TStrList TFileHandlerManager::BaseDirs;

TMemoryBlock *TFileHandlerManager::GetMemoryBlock( const olxstr &FN )  {
  olxstr fileName = TEFile::UnixPath(FN);
  TMemoryBlock *mb = FMemoryBlocks[fileName];
  if( mb == NULL )  {
    if( !TEFile::FileExists(fileName) )  return NULL;
    TEFile file(fileName, "rb");
    long fl = file.Length();
    if( fl <= 0 ) return NULL;
    mb = new TMemoryBlock;
    mb->Buffer = new char [ fl + 1];
    mb->Length = file.Length();
    mb->DateTime = TEFile::FileAge( fileName );
    file.Read( mb->Buffer, mb->Length );
    FMemoryBlocks.Add( fileName, mb );
  }
  else  {
    if( mb->DateTime != 0 && TEFile::FileExists(fileName) )  {
      if( TEFile::FileAge( fileName ) != mb->DateTime )  {
        int ind = FMemoryBlocks.IndexOf( fileName );
        FMemoryBlocks.Delete( ind );
        delete [] mb->Buffer;
        delete mb;
        return GetMemoryBlock( fileName );
      }
    }
  }
  return mb;
}
//..............................................................................
TFileHandlerManager::TFileHandlerManager()  {  
#ifndef _NO_PYTHON
  PythonExt::GetInstance()->Register( &TFileHandlerManager::PyInit );
#endif
}
//..............................................................................
TFileHandlerManager::~TFileHandlerManager()  {  _Clear();  }
//..............................................................................
void TFileHandlerManager::_Clear()  {
  for( int i=0; i < FMemoryBlocks.Count(); i++ )  {
    delete [] FMemoryBlocks.Object(i)->Buffer;
    delete FMemoryBlocks.Object(i);
  }
  FMemoryBlocks.Clear();
#ifdef __WXWIDGETS__
  for( int i=0; i < FZipFiles.Count(); i++ )
    delete FZipFiles.Object(i);
  FZipFiles.Clear();
#endif
}
//..............................................................................
IDataInputStream *TFileHandlerManager::_GetInputStream(const olxstr &FN)  {
#ifdef __WXWIDGETS__
  if( TZipWrapper::IsZipFile(FN) )  {
    TZipEntry ze;
    TZipWrapper::SplitZipUrl(FN, ze);
    TZipWrapper *zw = FZipFiles[ze.ZipName];
    if( zw == NULL )  {
      zw = new TZipWrapper( ze.ZipName, true );
      FZipFiles.Add( ze.ZipName, zw );
    }
    return zw->OpenEntry( ze.EntryName );
  }
  else  {
#endif
    TMemoryBlock *mb = GetMemoryBlock( FN );
    if( mb == NULL )  return NULL;
    TEMemoryStream *ms = new TEMemoryStream;
    ms->Write( mb->Buffer, mb->Length );
    ms->SetPosition( 0 );
    return ms;
#ifdef __WXWIDGETS__
  }
#endif
}
//..............................................................................
#ifdef __WXWIDGETS__
wxFSFile *TFileHandlerManager::_GetFSFileHandler( const olxstr &FN )  {
  static wxString st(wxT("OCTET")), es;
  if( TZipWrapper::IsZipFile(FN) )  {
    TZipEntry ze;
    TZipWrapper::SplitZipUrl(FN, ze);
    TZipWrapper *zw = FZipFiles[ze.ZipName];
    if( zw == NULL )  {
      zw = new TZipWrapper( ze.ZipName, true );
      FZipFiles.Add( ze.ZipName, zw );
    }
    wxInputStream *wxIS = zw->OpenWxEntry( ze.EntryName );
    return wxIS == NULL ? NULL : new wxFSFile( wxIS, ze.EntryName.u_str(), st, es, wxDateTime((time_t)0));
  }
  else  {
    TMemoryBlock *mb = GetMemoryBlock( FN );
    return mb == NULL ? NULL : new wxFSFile( new wxMemoryInputStream(mb->Buffer, mb->Length), es, st, es, wxDateTime((time_t)0));
  }
}
#endif
//..............................................................................
void TFileHandlerManager::_SaveToStream(IDataOutputStream& os, short persistenceMask)  {
  os.Write( FSignature, TFileHandlerManager_FSignatureLength );
  os << FVersion;

  uint32_t ic = 0, strl;
  for( int i=0; i < FMemoryBlocks.Count(); i++ )  {
    if( (FMemoryBlocks.Object(i)->PersistenceId & persistenceMask) != 0  )
      ic++;
  }
  os << ic;
  CString utfstr;
  for( int i=0; i < FMemoryBlocks.Count(); i++ )  {
    TMemoryBlock *mb = FMemoryBlocks.Object(i);
    if( (mb->PersistenceId & persistenceMask) != 0 )  {
      utfstr = TUtf8::Encode(FMemoryBlocks.String(i));
      strl = (uint32_t)utfstr.Length();
      os << strl;
      os.Write( (void*)utfstr.raw_str(), strl);
      os << mb->Length;
      os << mb->DateTime;
      if( mb->Length != 0 )
        os.Write( mb->Buffer, mb->Length);
    }
  }
}
//..............................................................................
void TFileHandlerManager::_LoadFromStream(IDataInputStream& is, short persistenceId)  {
  // validation of the stream
  char fSignature[TFileHandlerManager_FSignatureLength+1];
  is.Read( fSignature, TFileHandlerManager_FSignatureLength );
  fSignature[TFileHandlerManager_FSignatureLength] = '\0';
  if( olxstr(fSignature) != FSignature )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file signature");
  int16_t fVersion;
  is >> fVersion;
  if( fVersion > FVersion )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file version");

  size_t length = is.GetSize();

  uint32_t ic, strl;
  TMemoryBlock* mb;
  olxstr in;
  CString utfstr;

  is.Read(&ic, sizeof(int) );
  for(uint32_t i=0; i < ic; i++ )  {
    is >> strl;
    if( strl > (uint32_t)(length - is.GetPosition()) )  {
      _Clear();
      throw TFunctionFailedException(__OlxSourceInfo, "invalid file content");
    }
    utfstr = CEmptyString;
    in = TUtf8::Decode(utfstr.AppendFromStream(is, strl));
    mb = FMemoryBlocks[in];
    if( mb == NULL )  {
      mb = new TMemoryBlock;
      mb->PersistenceId = persistenceId;
      FMemoryBlocks.Add(in, mb);
    }
    else
      delete [] mb->Buffer;
    is >> mb->Length;
    if( fVersion == 0x0001 )  is >> mb->DateTime;
    // validate ...
    if( mb->Length > (length - is.GetPosition()) )  {
      mb->Buffer = new char[4];  // recover the potential errors
      _Clear();
      throw TFunctionFailedException(__OlxSourceInfo, "invalid file content");
    }

    mb->Buffer = new char [mb->Length + 1];
    if( mb->Length != 0 )
      is.Read( mb->Buffer, mb->Length);
    mb->PersistenceId = persistenceId;
  }
}
//..............................................................................
const TMemoryBlock* TFileHandlerManager::FindMemoryBlock(const olxstr& bn) {
  if( !FHandler )  FHandler = new TFileHandlerManager;
  if( bn.IsEmpty() )  return NULL;
  return FHandler->FMemoryBlocks[TEFile::UnixPath(bn)];
}
//..............................................................................
IDataInputStream *TFileHandlerManager::GetInputStream(const olxstr &FN)  {
  if( !FHandler )  FHandler = new TFileHandlerManager;
  if( FN.IsEmpty() )  return NULL;
  return FHandler->_GetInputStream( LocateFile(FN) );
}
//..............................................................................
#ifdef __WXWIDGETS__
wxFSFile *TFileHandlerManager::GetFSFileHandler( const olxstr &FN )  {
  if( FHandler == NULL )  FHandler = new TFileHandlerManager;
  return FHandler->_GetFSFileHandler( LocateFile(FN) );
}
#endif
//..............................................................................
void TFileHandlerManager::Clear(short persistenceMask)  {
  if( FHandler != NULL )  {
    if( persistenceMask == ~0 )  {
      delete FHandler;
      FHandler = NULL;
    }
    else  {
      for( int i=0; i < FHandler->FMemoryBlocks.Count(); i++ )  {
        if( (FHandler->FMemoryBlocks.Object(i)->PersistenceId & persistenceMask) != 0 )  {
          delete [] FHandler->FMemoryBlocks.Object(i)->Buffer;
          delete FHandler->FMemoryBlocks.Object(i);
          FHandler->FMemoryBlocks.Delete(i);
          i--;
        }
      }
    }
  }
}
//..............................................................................
olxstr TFileHandlerManager::LocateFile( const olxstr& fn )  {
  if( FHandler->IsMemoryBlock(fn) )  return fn;
  if( !TEFile::IsAbsolutePath(fn) )  {
    olxstr f = TEFile::AddTrailingBackslash( TEFile::CurrentDir() );
    if( TEFile::FileExists( f + fn ) )  return f + fn;
    for( int i=0; i < BaseDirs.Count(); i++ )  {
      if( TEFile::FileExists( BaseDirs.String(i) + fn ) )
        return BaseDirs.String(i) + fn;
    }
  }
  return fn;
}
//..............................................................................
void TFileHandlerManager::AddBaseDir(const olxstr& bd)  {
  BaseDirs.Add( TEFile::AddTrailingBackslash( bd )  );
}
//..............................................................................
void TFileHandlerManager::_AddMemoryBlock(const olxstr& name, char *bf,
                                                int length, short persistenceId)  {
  olxstr fileName = TEFile::UnixPath(name);
  TMemoryBlock *mb = FMemoryBlocks[fileName];
  if( mb == NULL )  {
    mb = new TMemoryBlock;
    mb->PersistenceId = persistenceId;
    FMemoryBlocks.Add( fileName, mb );
  }
  else  {
    delete [] mb->Buffer;
  }

  mb->Buffer = new char [ length + 1];
  mb->Length = length;
  mb->DateTime = TETime::Now();
  mb->PersistenceId = persistenceId;
  memcpy( mb->Buffer, bf, length );
}
//..............................................................................
void TFileHandlerManager::AddMemoryBlock(const olxstr& name, char *bf,
                                               int length, short persistenceId)  {
  if( length <= 0 )  return;

  if( FHandler == NULL )  FHandler = &TEGC::NewG<TFileHandlerManager>();
  return FHandler->_AddMemoryBlock(name, bf, length, persistenceId );
}
//..............................................................................
int TFileHandlerManager::Count()  {
  if( FHandler == NULL )  FHandler = &TEGC::NewG<TFileHandlerManager>();
  return FHandler->FMemoryBlocks.Count();
}
//..............................................................................
const olxstr& TFileHandlerManager::GetBlockName(int i)  {
  if( FHandler == NULL )  FHandler = &TEGC::NewG<TFileHandlerManager>();
  return FHandler->FMemoryBlocks.String(i);
}
//..............................................................................
long TFileHandlerManager::GetBlockSize(int i)  {
  if( FHandler == NULL )  FHandler = &TEGC::NewG<TFileHandlerManager>();
  return FHandler->FMemoryBlocks.Object(i)->Length;
}
//..............................................................................
const olxstr& TFileHandlerManager::GetBlockDateTime(int i)  {
  if( FHandler == NULL )  FHandler = &TEGC::NewG<TFileHandlerManager>();
  return TEGC::New<olxstr>( TETime::FormatDateTime( FHandler->FMemoryBlocks.Object(i)->DateTime ) );
}
//..............................................................................
short TFileHandlerManager::GetPersistenceId(int i)  {
  if( FHandler == NULL )  FHandler = &TEGC::NewG<TFileHandlerManager>();
  return FHandler->FMemoryBlocks.Object(i)->PersistenceId;
}
//..............................................................................
void TFileHandlerManager::SaveToStream(IDataOutputStream& os, short persistenceMask)  {
  if( FHandler == NULL )  FHandler = &TEGC::NewG<TFileHandlerManager>();
  FHandler->_SaveToStream(os, persistenceMask);
}
//..............................................................................
void TFileHandlerManager::LoadFromStream(IDataInputStream& is, short persistenceId) {
  if( FHandler == NULL )  FHandler = &TEGC::NewG<TFileHandlerManager>();
  FHandler->_LoadFromStream(is, persistenceId);
}
//..............................................................................
bool TFileHandlerManager::Exists(const olxstr& fn)  {
  if( FHandler == NULL )  FHandler = &TEGC::NewG<TFileHandlerManager>();
  return FHandler->IsMemoryBlock(fn);
}
//..............................................................................
//..............................................................................
//..............................................................................
void TFileHandlerManager::LibExists(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal<bool>( IsMemoryBlock(Params[0]) );
}
TLibrary* TFileHandlerManager::ExportLibrary(const olxstr& name)  {
  if( FHandler == NULL )  FHandler = &TEGC::NewG<TFileHandlerManager>();
  TLibrary* lib = new TLibrary( name.IsEmpty() ? olxstr("fs") : name );
  lib->RegisterFunction<TFileHandlerManager>(new TFunction<TFileHandlerManager>(FHandler,  
    &TFileHandlerManager::LibExists, "Exists", fpOne,
"returns true if the specified file exists on the virtual file system") );
  return lib;
}

//..............................................................................
//..............................................................................
//..............................................................................
#ifndef _NO_PYTHON
PyObject* pyExists(PyObject* self, PyObject* args)  {
  olxstr fn;
  PythonExt::ParseTuple(args, "w", &fn);
  return Py_BuildValue("b", TFileHandlerManager::Exists(fn) );
}
PyObject* pyTimestamp(PyObject* self, PyObject* args)  {
  olxstr fn;
  PythonExt::ParseTuple(args, "w", &fn);
  const TMemoryBlock* mb = TFileHandlerManager::FindMemoryBlock(fn);
  if( mb == NULL )  {
    Py_IncRef(Py_None);
    return Py_None;
  }
  return Py_BuildValue("l", mb->DateTime );
}

PyObject* pyNewFile(PyObject* self, PyObject* args)  {
  char *data = NULL;
  olxstr name;
  int persistenceId = 0;
  int length = 0;
  if( !PythonExt::ParseTuple(args, "ws#|i", &name, &data, &length, &persistenceId) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  if( data != NULL && !name.IsEmpty() && length > 0 )  {
    TFileHandlerManager::AddMemoryBlock(name, data, length, persistenceId);
    return Py_BuildValue("b", true);
  }
  return Py_BuildValue("b", false);
}
//..............................................................................
PyObject* pyReadFile(PyObject* self, PyObject* args)  {
  olxstr name;
  if( !PythonExt::ParseTuple(args, "w", &name) )  {
    Py_INCREF(Py_None);
    return Py_None;
  }
  IInputStream* io = TFileHandlerManager::GetInputStream(name);
  if( !name.IsEmpty() && (io = TFileHandlerManager::GetInputStream(name)) != NULL )  {
    char * bf = new char [io->GetSize() + 1];
    io->Read(bf, io->GetSize());
    PyObject* po = Py_BuildValue("s#", bf, io->GetSize() );
    delete [] bf;
    delete io;
    return po;
  }
  PyErr_SetObject(PyExc_TypeError, PythonExt::BuildString("file does not exist") );
  Py_INCREF(Py_None);
  return Py_None;
}


static PyMethodDef OLEXFS_Methods[] = {
  {"Exists", pyExists, METH_VARARGS, "returns true if specified file exists"},
  {"Timestamp", pyTimestamp, METH_VARARGS, "returns timestamp (epoch time) of given file, if file does not exist, returns None"},
  {"NewFile", pyNewFile, METH_VARARGS, "creates a new file (file_name, data,[persistence]), returns true if operation succeeded "},
  {"ReadFile", pyReadFile, METH_VARARGS, "reads previously created file and reurns the content of the file or None, if error has occured"},
  {NULL, NULL, 0, NULL}
   };

void TFileHandlerManager::PyInit()  {
  Py_InitModule( "olex_fs", OLEXFS_Methods );
}
#endif //_NO_PYTHON
