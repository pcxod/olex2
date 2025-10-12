/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
/* a thin wrapper around a DLL for dynamic loading */
#ifdef _LIBFFI
#include <ffi.h>
#include "olxdll.h"

struct lib_ffi {
  olx_object_ptr<OlxDll> module;
  typedef void* (*ffi_closure_alloc_t)(size_t size, void** code);
  typedef void (*ffi_closure_free_t)(void*);
  typedef void (*ffi_call_t)(ffi_cif* cif,
    void (*fn)(void),
    void* rvalue,
    void** avalue);
  typedef ffi_status(*ffi_prep_cif_t)(ffi_cif* cif,
    ffi_abi abi,
    unsigned int nargs,
    ffi_type* rtype,
    ffi_type** atypes);
  typedef ffi_status
  (*ffi_prep_closure_loc_t)(ffi_closure*,
    ffi_cif*,
    void (*fun)(ffi_cif*, void*, void**, void*),
    void* user_data,
    void* codeloc);

  lib_ffi();
  typedef ffi_type* ffi_type_pointer_t;


  ffi_type_pointer_t ffi_type_void,
    ffi_type_uint8, ffi_type_sint8,
    ffi_type_uint16, ffi_type_sint16,
    ffi_type_uint32, ffi_type_sint32,
    ffi_type_uint64, ffi_type_sint64,
    ffi_type_float, ffi_type_double,
    ffi_type_longdouble,
    ffi_type_pointer;


  ffi_closure_alloc_t ffi_closure_alloc;
  ffi_closure_free_t ffi_closure_free;
  ffi_call_t ffi_call;
  ffi_prep_cif_t ffi_prep_cif;
  ffi_prep_closure_loc_t ffi_prep_closure_loc;

};
#endif // _LIBFFI