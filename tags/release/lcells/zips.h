//---------------------------------------------------------------------------

#ifndef zipsH
#define zipsH
#include "estrlist.h"
//---------------------------------------------------------------------------
class TZipShell  {
  olxstr FArchive,
           FTmpPath;
  TStrList FFiles;
  void __fastcall SetTmpPath(const olxstr& newPath);
public:
  TZipShell();
  ~TZipShell();
  __property TStrList Files = {read = FFiles};
  __property olxstr TmpPath = {read = FTmpPath, write = SetTmpPath};
  bool _fastcall Initialize(const olxstr& Arch);
  void _fastcall MaskFiles(const olxstr& Mask, const olxstr& Mask1);
  void _fastcall MaskFiles(const olxstr& Mask);
  bool _fastcall Extract();
  bool _fastcall GetFile(const olxstr& Name);
};
#endif

