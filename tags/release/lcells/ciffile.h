//---------------------------------------------------------------------------

#ifndef CifFileH
#define CifFileH
#include "inscellreader.h"
#include "zips.h"
#include "datastream.h"
#include <sys/types.h>
//#include "typelist.h"

//---------------------------------------------------------------------------
class TIniStream;
class TCifIndex;
class TZipFile;
struct TCell  {
  double   a,   b,   c,  // cell params
           aa,  ab,  ac;  // cell angles
  short Lattice;
  double   na,   nb,   nc,  // cell params // Niggli form
           naa,  nab,  nac;  // cell angles
  TCell()  {
    a = b = c = aa = ab = ac = 0;
    na = nb = nc = naa = nab = nac = 0;
  }
  bool _fastcall InRange(const TCell &C, double Dev);
  void _fastcall operator >> (IDataOutputStream &S);
  void _fastcall operator << (IDataInputStream &S);
  bool operator == (const TCell &C);
  void _fastcall operator = (const TCell &C);
};

class TCifFile  {
  olxstr FName;
  time_t FFileAge;
  TCell FCell;
  TZipFile *FParent;
  void _fastcall SetName(const olxstr& N);
public:
  TCifFile(TZipFile *P);

  void _fastcall operator >> (IDataOutputStream &S);
  void _fastcall operator << (IDataInputStream &S);

  void _fastcall operator << (const TInsCellReader &S);
  __property TZipFile *Parent = {read = FParent, write = FParent};
  __property olxstr Name = {read = FName, write = SetName};
  __property time_t FileAge = {read = FFileAge, write = FFileAge};
  __property TCell Cell = {read = FCell, write = FCell};
  bool _fastcall ReduceCell();
};

class TZipFile  {
  TCifIndex *FIndex;
  time_t FFileAge;
  olxstr FFileName;
  TZipShell *FZip;
public:
  TZipFile(TZipShell *Zip);
  ~TZipFile();
  olxstr GetCif(const olxstr& Name);
  bool _fastcall GetFiles();
  bool _fastcall LoadFromZip(const olxstr& FN);
  void _fastcall operator >> (IDataOutputStream &S);
  void _fastcall operator << (IDataInputStream &S);

  __property TCifIndex *Index = {read = FIndex};
  __property time_t FileAge = {read = FFileAge};
  __property olxstr FileName = {read = FFileName};
  __property TZipShell*  Zip = {read = FZip};

};

class TCifIndex  {
  TTypeList<TCifFile*>  FInsFiles;
  TTypeList<TZipFile*> FZipFiles;
  bool _fastcall ListFiles(class TdlgProgress *P, const olxstr& Dir,
     TStrList& ZipFiles,
     TStrList& InsFiles,
     TStrList& CifFiles, int MaxSize);
  time_t FUpdated;
  bool FAppend;
  TZipFile *FParent;
public:
  TCifIndex();
  ~TCifIndex();
  __property time_t Updated   = {read = FUpdated};
  __property TTypeList<TCifFile*> IFiles    = {read = FInsFiles};
  __property TTypeList<TZipFile*> ZFiles    = {read = FZipFiles};
  __property TZipFile *Parent = {read = FParent, write = FParent};
  void _fastcall Clear();
  bool _fastcall Update(bool Total, const olxstr& Dir, int MaxSize, TForm *Parent);
  void _fastcall Clean(TForm *Parent);  // removes records with the same cell
  void _fastcall CleanDead(TForm *Parent);  // removes records without files
  void _fastcall Exclusive();            // if several files are loaded into the index,
                        // then the function leaves uniq filenames only
  void _fastcall Search(const TCell &C, double Dev, TTypeList<TCifFile*>& Files, bool Silent);
  void _fastcall SaveToFile(const olxstr& FN);
  void _fastcall LoadFromFile(const olxstr& FN, bool Append);
  void _fastcall operator >> (IDataOutputStream &S);
  void _fastcall operator << (IDataInputStream &S);
  int _fastcall GetCount();
};
#endif

