#include "olxffi.h"
#include "bapp.h"

#ifdef _LIBFFI
lib_ffi::lib_ffi() :
  ffi_type_void(0),
  ffi_type_uint8(0), ffi_type_sint8(0),
  ffi_type_uint16(0), ffi_type_sint16(0),
  ffi_type_uint32(0), ffi_type_sint32(0),
  ffi_type_uint64(0), ffi_type_sint64(0),
  ffi_type_float(0), ffi_type_double(0),
  ffi_type_longdouble(0),
  ffi_type_pointer(0),
  ffi_closure_alloc(0), ffi_closure_free(0),
  ffi_call(0), ffi_prep_cif(0), ffi_prep_closure_loc(0)
{
  TStrList paths;
#ifdef __WIN32__
  olxstr ext = "dll", sep = "-";
  paths << TEFile::JoinPath(
    TStrList() << olx_getenv("PYTHONHOME") << "DLLs");
#else
#if defined(__MAC__)
  olxstr ext = "dylib", sep = ".";
#else
  olxstr ext = "so", sep = "-";
#endif
  paths << TEFile::JoinPath(
    TStrList() << TBasicApp::GetBaseDir() << "lib");
#endif
  char v[] = { '6', '7', '8', 0 },
    * vp = &v[0];
  olxstr dll;
  olxstr path = olx_getenv("PATH");
  paths.AddAll(TStrList(path, TEFile::GetEnviPathDelimeter()));
  
  while (*vp != 0) {
    dll = TEFile::Which(olxstr("libffi") << sep << *vp << '.' << ext, paths);
    if (!dll.IsEmpty()) {
      break;
    }
    vp++;
  }
  if (dll.IsEmpty()) {
    throw TFunctionFailedException(__OlxSourceInfo, "LIBFFI is not available");
  }
  TBasicApp::NewLogEntry(logInfo) << "Loading LIBFFI: " << dll;
  module = new OlxDll(dll, true, false);
  ffi_type_void = module->get<ffi_type_pointer_t>("ffi_type_void", true);
  ffi_type_uint8 = module->get<ffi_type_pointer_t>("ffi_type_uint8", true);
  ffi_type_sint8 = module->get<ffi_type_pointer_t>("ffi_type_sint8", true);
  ffi_type_uint16 = module->get<ffi_type_pointer_t>("ffi_type_uint16", true);
  ffi_type_sint16 = module->get<ffi_type_pointer_t>("ffi_type_sint16", true);
  ffi_type_uint32 = module->get<ffi_type_pointer_t>("ffi_type_uint32", true);
  ffi_type_sint32 = module->get<ffi_type_pointer_t>("ffi_type_sint32", true);
  ffi_type_uint64 = module->get<ffi_type_pointer_t>("ffi_type_uint64", true);
  ffi_type_sint64 = module->get<ffi_type_pointer_t>("ffi_type_sint64", true);

  ffi_type_float = module->get<ffi_type_pointer_t>("ffi_type_float", true);
  ffi_type_double = module->get<ffi_type_pointer_t>("ffi_type_double", true);
  // this is in 8 but not in 7
  ffi_type_longdouble = module->get<ffi_type_pointer_t>("ffi_type_longdouble", false);

  ffi_type_pointer = module->get<ffi_type_pointer_t>("ffi_type_pointer", true);

  ffi_closure_alloc = module->get<ffi_closure_alloc_t>("ffi_closure_alloc", true);
  ffi_closure_free = module->get<ffi_closure_free_t>("ffi_closure_free", true);
  ffi_call = module->get<ffi_call_t>("ffi_call", true);
  ffi_prep_cif = module->get<ffi_prep_cif_t>("ffi_prep_cif", true);
  ffi_prep_closure_loc = module->get<ffi_prep_closure_loc_t>("ffi_prep_closure_loc", true);
}
#endif // _LIBFFI