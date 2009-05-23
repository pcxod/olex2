//---------------------------------------------------------------------------

#pragma hdrstop

#include <dir.h>
#include "Main.h"
#include "progress.h"
#include "ememstream.h"
#include "CifFile.h"
#include "Cif.h"
#include "efile.h"
#include "etime.h"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool _fastcall TCell::InRange(const TCell &C, double Dev)  {
  double   min = 1-Dev,
      max = 1+Dev;

  if( (a >= C.a*min) && (a <= C.a*max) &&
    (b >= C.b*min) && (b <= C.b*max) &&
    (c >= C.c*min) && (c <= C.c*max) &&
    (aa >= C.aa*min) && (aa <= C.aa*max) &&
    (ab >= C.ab*min) && (ab <= C.ab*max) &&
    (ac >= C.ac*min) && (ac <= C.ac*max)  )
    return true;
  if( (b >= C.a*min) && (b <= C.a*max) &&
    (c >= C.b*min) && (c <= C.b*max) &&
    (a >= C.c*min) && (a <= C.c*max) &&
    (ab >= C.aa*min) && (aa <= C.aa*max) &&
    (ac >= C.ab*min) && (ac <= C.ab*max) &&
    (aa >= C.ac*min) && (aa <= C.ac*max)  )
    return true;
  if( (c >= C.a*min) && (c <= C.a*max) &&
    (a >= C.b*min) && (a <= C.b*max) &&
    (b >= C.c*min) && (b <= C.c*max) &&
    (ac >= C.aa*min) && (ac <= C.aa*max) &&
    (aa >= C.ab*min) && (aa <= C.ab*max) &&
    (ab >= C.ac*min) && (ab <= C.ac*max)  )
    return true;

  // compare niggli form
  if( na != 0 )
    if( fabs(((float)na  - (float)C.na)/(float)na) > (float)Dev )
      return false;
  if( nb != 0 )
    if( fabs(((float)nb  - (float)C.nb)/(float)nb) > (float)Dev )
      return false;
  if( nc != 0 )
    if( fabs(((float)nc  - (float)C.nc)/(float)nc) > (float)Dev )
      return false;
  if( naa != 0 )
    if( fabs(((float)naa  - (float)C.naa)/(float)naa) > (float)Dev )
      return false;
  if( nab != 0 )
    if( fabs(((float)nab  - (float)C.nab)/(float)nab) > (float)Dev )
      return false;
  if( nac != 0 )
    if( fabs(((float)nac  - (float)C.nac)/(float)nac) > (float)Dev )
      return false;


  if( (na  >= C.na*min)  &&  (na  <= C.na*max) &&
    (nb  >= C.nb*min)  &&  (nb  <= C.nb*max) &&
    (nc  >= C.nc*min)  &&  (nc  <= C.nc*max) &&
    (naa >= C.naa*min) && (naa <= C.naa*max) &&
    (nab >= C.nab*min) && (nab <= C.nab*max) &&
    (nac >= C.nac*min) && (nac <= C.nac*max)  )
    return true;
  return false;
}

bool TCell::operator == (const TCell &C)  {
  return ( (a == C.a) && (b == C.b) && (c == C.c) && (aa == C.aa) && (ab == C.ab) && (ac == C.ac) );
}

void _fastcall TCell::operator >> (IDataOutputStream &S)
{
  S << a;
  S << b;
  S << c;
  S << aa;
  S << ab;
  S << ac;

  S << na;
  S << nb;
  S << nc;
  S << naa;
  S << nab;
  S << nac;
  S << Lattice;
}
void _fastcall TCell::operator << (IDataInputStream &S)
{
  S >> a;
  S >> b;
  S >> c;
  S >> aa;
  S >> ab;
  S >> ac;

  S >> na;
  S >> nb;
  S >> nc;
  S >> naa;
  S >> nab;
  S >> nac;
  S >> Lattice;
}
void _fastcall TCell::operator = (const TCell &C)  {
  a = C.a;
  b = C.b;
  c = C.c;
  aa = C.aa;
  ab = C.ab;
  ac = C.ac;

  na = C.na;
  nb = C.nb;
  nc = C.nc;
  naa = C.naa;
  nab = C.nab;
  nac = C.nac;
  Lattice = C.Lattice;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TCifFile::TCifFile(TZipFile *P)
{
  FParent = P;
}
void _fastcall TCifFile::operator >> (IDataOutputStream &S)  {
  S << FName;
  S << FFileAge;
  Cell >> S;
}
void _fastcall TCifFile::operator << (IDataInputStream &S)  {
  S >> FName;
  S >> FFileAge;
  Cell << S;
}
void _fastcall TCifFile::operator << (const TInsCellReader &S)  {
  Cell.a = S.a;
  Cell.b = S.b;
  Cell.c = S.c;
  Cell.aa = S.aa;
  Cell.ab = S.ab;
  Cell.ac = S.ac;
  Cell.Lattice = S.Lattice;
}

void _fastcall TCifFile::SetName(const olxstr& N)  {
  FName = ((Parent==NULL) ? TEFile::UNCFileName(N) : N);
}
bool _fastcall TCifFile::ReduceCell()  {
  TCell C = FCell;
  try  {
    dlgMain->ReduceCell(C); // even if the niggli cell cannot be evaluated then
                // the reduced cell will be calculated
    FCell = C;
    return true;
  }
  catch(const TExceptionBase& exc)  {
    TStrList sl;
    exc.GetException()->GetStackTrace(sl);
    dlgMain->AddMessage( olxstr("Exception occured while processing '") << this->FName << "':");
    dlgMain->AddMessage( sl );
  }
  return false;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TZipFile::TZipFile(TZipShell *Zp)  {
  FZip = Zp;
  FIndex = new TCifIndex;
  FIndex->Parent = this;
}
TZipFile::~TZipFile()  {
  delete FIndex;
}
void _fastcall TZipFile::operator >> (IDataOutputStream &S)  {
  S << FFileAge;
  S << FFileName;
  *FIndex >> S;
}
void _fastcall TZipFile::operator << (IDataInputStream &S)  {
  S >> FFileAge;
  S >> FFileName;
  *FIndex << S;
  TCifFile *C;
  for( int i=0; i < FIndex->IFiles.Count(); i++ )
    FIndex->IFiles[i]->Parent = this;
}
olxstr TZipFile::GetCif(const olxstr& Name)  {
  olxstr Tmp, FFile, Tmp1;
  FZip->Initialize(FFileName);
  FZip->GetFile(Name);
  return olxstr(dlgMain->TmpDir.c_str()) + Name;
}

bool _fastcall TZipFile::GetFiles()  {
  FZip->Initialize(FFileName);
  FZip->MaskFiles("ins", "cif");
  return true;
}

bool _fastcall TZipFile::LoadFromZip(const olxstr& FN)  {
  FIndex->Clear();
  FFileName = TEFile::UNCFileName(FN);
  FFileAge = TEFile::FileAge(FN);
  olxstr Tmp, Tmp1;
  TCifFile *C;
  TInsCellReader Ins;
  TStrList Files;
  bool res;

  FZip->Initialize(FN);
  FZip->MaskFiles("ins", "cif");
  if( !FZip->Files.Count() )  {
    res = true;  // will keep reference to avoid ancounting in update
    goto exit;
  }
  FZip->Extract();
  Files.Assign(FZip->Files);
  FZip->MaskFiles("ins");
  for( int i=0; i < FZip->Files.Count(); i++ )  {
    Tmp = FZip->Files[i];
    Tmp1 = dlgMain->TmpDir;
    Tmp1 << Tmp;
    C = new TCifFile(this);
    C->Name = Tmp;  // location in the archive
    C->FileAge = ::FileAge(Tmp1.c_str());
    if( !Ins.LoadFromInsFile(Tmp1) )  {
      delete C;
      continue;
    }
    *C << Ins;
    C->ReduceCell();
    FIndex->IFiles.AddACopy(C);
  }
  FZip->Files.Assign(Files);
  FZip->MaskFiles("cif");
  for( int i=0; i < FZip->Files.Count(); i++ )  {
    Tmp = FZip->Files[i];
    Tmp1 = dlgMain->TmpDir;
    Tmp1 << Tmp;
    C = new TCifFile(this);
    C->Name = Tmp;  // location in the archive
    C->FileAge = TEFile::FileAge(Tmp1);
    if( !Ins.LoadFromCifFile(Tmp1) )  {
      delete C;
      continue;
    }
    *C << Ins;
    C->ReduceCell();
    FIndex->IFiles.AddACopy(C);
  }
  res = true;
exit:
  return res;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TCifIndex::TCifIndex()  {
  FUpdated = 0;
  FAppend = false;
  FParent = NULL;
}
TCifIndex::~TCifIndex()  {
  Clear();
}
void _fastcall TCifIndex::Clear()  {
  for( int i=0; i < FInsFiles.Count(); i++ )
    delete FInsFiles[i];
  for( int i=0; i < FZipFiles.Count(); i++ )
    delete FZipFiles[i];
  FInsFiles.Clear();
  FZipFiles.Clear();
}
bool _fastcall TCifIndex::Update(bool Total, const olxstr& Dir, int MaxSize, TForm *Parent)
{
  TStrList ZipFiles,
               InsFiles,
               CifFiles;
  olxstr Tmp, FN, FFN;
  TCifFile *C;
  TZipFile *Zip;
  TInsCellReader *InsFile = new TInsCellReader;
  bool Cancel=false, found, update;
  double Value;
  dlgProgress = new TdlgProgress(NULL);
  dlgProgress->OnCancel = &Cancel;
  dlgProgress->AddForm(Parent);
  dlgMain->ClearMessages();
  dlgProgress->Show();
  dlgProgress->Caption = "Searching Files...";
  if( Total )
  {

    int dc = setdisk(getdisk());
    dlgProgress->pbBar->Max = dc-1;
    for( int i=2; i < 26; i++ )  // list of hard drives
    {
      Tmp = "a:\\";
      Tmp[1] = i+'a';
      if( setdisk(i) )
      {
        if( !ListFiles(dlgProgress, Tmp, ZipFiles, InsFiles, CifFiles, MaxSize) )
          goto exit;
        dlgProgress->pbBar->Position = i-1;
      }
    }
  }
  else
  {
    if( !ListFiles(dlgProgress, Dir, ZipFiles, InsFiles, CifFiles, MaxSize) )
      goto exit;
  }
  dlgProgress->Caption = "Processing Ins files...";
  dlgProgress->pbBar->Position = 0;
  dlgProgress->pbBar->Max = InsFiles.Count();
  for( int i=0; i < InsFiles.Count(); i++ )  {
    FFN = InsFiles[i];
    FN = TEFile::UNCFileName(FFN);
    dlgProgress->SetAction(FFN);
    if( !(i%10) )  {
      dlgProgress->pbBar->Position = i;
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    update = found = 0;
    for( int j=0; j < FInsFiles.Count(); j++ )  {
      C = FInsFiles[j];
      if( !C->Name.Comparei(FN) )  {
        found = true;
        time_t age = TEFile::FileAge(FN);
        if( C->FileAge !=  age && fabs(C->FileAge-age) != 3600 )  {
          update = true;
          Tmp = "The file has been updated: ";
          Tmp << FFN;
          dlgMain->AddMessage(Tmp);
          Tmp = "The old date is: ";
          Tmp <<  TETime::FormatDateTime(C->FileAge);
          dlgMain->AddMessage(Tmp);
          Tmp = "The new date is: ";
          Tmp << TETime::FormatDateTime( age );
          dlgMain->AddMessage(Tmp);
        }
        break;
      }
    }
    if( found )  {
      if( update && InsFile->LoadFromInsFile(FFN) )  {
        *C << *InsFile;
        C->FileAge = TEFile::FileAge(FN);
      }
      continue;
    }
    C = new TCifFile(NULL);
    if( !InsFile->LoadFromInsFile(FFN) )  {
      delete C;
      continue;
    }
    FInsFiles.AddACopy(C);
    C->Name = FFN;
    C->FileAge = TEFile::FileAge(FN);
    *C << *InsFile;
    C->ReduceCell();
  }

  dlgProgress->Caption = "Processing Cif files...";
  dlgProgress->pbBar->Position = 0;
  dlgProgress->pbBar->Max = InsFiles.Count();
  for( int i=0; i < CifFiles.Count(); i++ )  {
    FFN = CifFiles[i];
    FN = TEFile::UNCFileName(FFN);
    dlgProgress->SetAction(FFN);
    if( !(i%10) )  {
      dlgProgress->pbBar->Position = i;
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    update = found = 0;
    for( int j=0; j < FInsFiles.Count(); j++ ) {
      C = FInsFiles[j];
      if( !C->Name.Comparei(FN) )  {
        found = true;
        time_t age = TEFile::FileAge(FN);
        if( C->FileAge !=  age && fabs(C->FileAge-age) != 3600 )  {
          update = true;
          Tmp = "The file has been updated: ";
          Tmp << FFN;
          dlgMain->AddMessage(Tmp);
          Tmp = "The old date is: ";
          Tmp << TETime::FormatDateTime( C->FileAge );
          dlgMain->AddMessage(Tmp);
          Tmp = "The new date is: ";
          Tmp << TETime::FormatDateTime( age );
          dlgMain->AddMessage(Tmp);
        }
        break;
      }
    }
    if( found )  {
      if( update && InsFile->LoadFromCifFile(FFN) )  {
        *C << *InsFile;
        C->FileAge = TEFile::FileAge(FN);
      }
      continue;
    }
    C = new TCifFile(NULL);
    if( !InsFile->LoadFromCifFile(FFN) )  {
      delete C;
      continue;
    }
    FInsFiles.AddACopy(C);
    C->Name = FFN;
    C->FileAge = TEFile::FileAge(FN);
    *C << *InsFile;
    C->ReduceCell();
  }
  dlgProgress->Caption = "Processing Zip files...";
  dlgProgress->pbBar->Position = 0;
  dlgProgress->pbBar->Max = ZipFiles.Count();
  for( int i=0; i < ZipFiles.Count(); i++ )  {
    FFN = ZipFiles[i];
    FN = TEFile::UNCFileName(FFN);
    dlgProgress->SetAction(FFN);
    dlgProgress->pbBar->Position = i;
    Application->ProcessMessages();
    if( Cancel )
      goto exit;
    update = found = false;
    for( int j=0; j < FZipFiles.Count(); j++ )  {
      Zip = FZipFiles[j];
      if( !Zip->FileName.Comparei(FN) )  {
        found = true;
        time_t age = TEFile::FileAge(FN);
        if( Zip->FileAge !=  age && fabs(Zip->FileAge-age) != 3600 )  {
          update = true;
          Tmp = "The file has been updated: ";
          Tmp << FFN;
          dlgMain->AddMessage(Tmp);
          Tmp = "The old date is: \t";
          Tmp << TETime::FormatDateTime( Zip->FileAge )
              << " \tThe new date is: \t"
              << TETime::FormatDateTime( age );
          dlgMain->AddMessage(Tmp);
        }
        break;
      }
    }
    if( found )  {
      if( update )  {
        Zip->LoadFromZip(FFN);
      }
      continue;
    }
    Zip = new TZipFile(dlgMain->Zip);
    try  {
      if( !Zip->LoadFromZip(FFN) )  {
        delete Zip;
        Zip = NULL;
        continue;
      }
    }
    catch(...)  {
      Application->MessageBox("Cannot process zip file!","Error", MB_OK|MB_ICONERROR);
      if( Zip )
        delete Zip;
      continue;
    }
    FZipFiles.AddACopy(Zip);
  }
  FUpdated = TETime::Now();
exit:
  delete dlgProgress;
  return !Cancel;
}
void _fastcall TCifIndex::Search(const TCell &C, double Dev, TTypeList<TCifFile*>& Files, bool Silent)  {
  TCifFile *Cf;
  TZipFile *Zp;
  bool Cancel = false;
  if( !Silent )  {
    dlgProgress = new TdlgProgress(NULL);
    dlgProgress->OnCancel = &Cancel;
    dlgProgress->AddForm(dlgMain);
    dlgProgress->Show();
    dlgProgress->Caption = "Searching in CIF/INS Files...";
    dlgProgress->pbBar->Max = FInsFiles.Count() + FZipFiles.Count();
  }
  for( int i=0; i < FInsFiles.Count(); i++ )  {
    Cf = FInsFiles[i];
    if( Cf->Cell.InRange(C, Dev) )
      Files.AddACopy(Cf);
    if( !Silent )  {
      dlgProgress->SetAction(Cf->Name);
      if( !(i%20) )  {
        dlgProgress->pbBar->Position = i;
        Application->ProcessMessages();
        if( Cancel )
          goto exit;
      }
    }
  }
  dlgProgress->Caption = "Searching in ZIP Files...";
  for( int i=0; i < FZipFiles.Count(); i++ )  {
    Zp = FZipFiles[i];
    Zp->Index->Search(C, Dev, Files, true);
    if( !Silent )   {
      dlgProgress->SetAction(Zp->FileName);
      if( !(i%20) )  {
        dlgProgress->pbBar->Position = FInsFiles.Count() + i;
        Application->ProcessMessages();
        if( Cancel )
          goto exit;
      }
    }
  }
exit:
  if( !Silent )
    delete dlgProgress;
}
void _fastcall TCifIndex::operator >> (IDataOutputStream &S)  {
  S << FUpdated;
  S << FInsFiles.Count();
  for( int i=0; i < FInsFiles.Count(); i++ )
    *FInsFiles[i] >> S;

  S << FZipFiles.Count();
  for( int i=0; i < FZipFiles.Count(); i++ )
    *FZipFiles[i] >> S;
}
void _fastcall TCifIndex::operator << (IDataInputStream &S)  {
  short Ver;
  int count;
  TCifFile *Cf;
  TZipFile *Zp;
  if( !FAppend )
    Clear();
  S >> FUpdated;
  S >> count;
  for( int i=0; i < count; i++ )  {
    Cf = new TCifFile(NULL);
    Cf->Parent = FParent;
    *Cf << S;
    FInsFiles.AddACopy(Cf);
  }
  S >> count;
  for( int i=0; i < count; i++ )  {
    Zp = new TZipFile(dlgMain->Zip);
    *Zp << S;
    FZipFiles.AddACopy(Zp);
  }
}
void _fastcall TCifIndex::SaveToFile(const olxstr& FN)  {
  TEFile f(FN, "wb+");
  *this >> f;
}
void _fastcall TCifIndex::LoadFromFile(const olxstr& FN, bool App)  {
  if( !TEFile::FileExists( FN ) )
    return;
  FAppend = App;
  try  {
    TEFile inf( FN, "rb" );
    if( inf.GetSize() == 0 )  return;
    *this << inf;
  }
  catch(...)  {
    FAppend = false;
  }
}
bool _fastcall TCifIndex::ListFiles(TdlgProgress *P, const olxstr& Dir,
  TStrList& ZipFiles,
  TStrList& InsFiles,
  TStrList& CifFiles, int MaxSize )  {

  olxstr Tmp = Dir, Tmp1;
  TEFile::AddTrailingBackslashI(Tmp);
  if( Tmp == dlgMain->TmpDir )
    return true;
  if( !TEFile::ChangeDir(Dir) )
    return true;
  struct ffblk ffblk;
  int done;
  if( &InsFiles != NULL )  {
    done = findfirst("*.ins",&ffblk, 0);
    while( !done )  {
      Tmp1 = Tmp;
      Tmp1 << ffblk.ff_name;
      if( ffblk.ff_fsize < MaxSize )
        InsFiles.Add(Tmp1);
      done = findnext(&ffblk);
    }
    findclose( &ffblk );
  }
  if( &CifFiles != NULL )  {
    done = findfirst("*.cif",&ffblk, 0);
    while( !done )  {
      Tmp1 = Tmp;
      Tmp1 << ffblk.ff_name;
      if( ffblk.ff_fsize < MaxSize )
        CifFiles.Add(Tmp1);
      done = findnext(&ffblk);
    }
    findclose( &ffblk );
  }
  if( &ZipFiles != NULL )  {
    done = findfirst("*.zip",&ffblk, 0);
    while( !done )  {
      Tmp1 = Tmp;
      Tmp1 << ffblk.ff_name;
      if( ffblk.ff_fsize < MaxSize )
        ZipFiles.Add(Tmp1);
      done = findnext(&ffblk);
    }
    findclose( &ffblk );
  }
  done = findfirst("*.*",&ffblk, FA_DIREC);
  while( !done )  {
    if( ffblk.ff_attrib  & FA_DIREC )  {
      if( !strcmpi(ffblk.ff_name, "." ) )
        goto next_dir;
      if( !strcmpi(ffblk.ff_name, ".." ) )
        goto next_dir;
      Tmp1 = Tmp;
      Tmp1 << ffblk.ff_name;
      if( P != NULL )  {
        P->SetAction(Tmp1);
        Application->ProcessMessages();
        if( *(P->OnCancel) )
          return false;
      }
      if( (ffblk.ff_attrib & FA_HIDDEN) || (ffblk.ff_attrib & FA_SYSTEM) )
        goto next_dir;
      if( !ListFiles(P, Tmp1, ZipFiles, InsFiles, CifFiles, MaxSize) )
        return false;
    }
next_dir:
    done = findnext(&ffblk);
  }
  findclose( &ffblk );
  TEFile::ChangeDir(Dir);
  chdir("..");
  return true;
}
void _fastcall TCifIndex::Clean(TForm *Parent)  {
  TCifFile *C, *C1;
  bool Cancel= false;
  olxstr N1, N2;
  dlgProgress = new TdlgProgress(NULL);
  dlgProgress->OnCancel == &Cancel;
  dlgProgress->Caption = "Cleaning the index...";
  dlgProgress->AddForm(Parent);
  dlgProgress->Show();
  dlgProgress->pbBar->Max = FInsFiles.Count();
  for( int i=0; i < FInsFiles.Count(); i++ )  {
    if( FInsFiles.IsNull(i) )  continue;
    C = FInsFiles[i];
    dlgProgress->SetAction(C->Name);
    if( !(i%10) )  {
      dlgProgress->pbBar->Position = i;
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    for( int j=i+1; j < FInsFiles.Count(); j++ )  {
      if( FInsFiles.IsNull(j) )  continue;
      C1 = FInsFiles[j];
      if( C->Cell == C1->Cell )  {
        FInsFiles.NullItem(j);
        delete C1;
        continue;
      }
    }
  }
exit:
  FInsFiles.Pack();
  delete dlgProgress;
}

int _fastcall TCifIndex::GetCount()  {
  int C = FInsFiles.Count();
  for( int i=0; i < FZipFiles.Count(); i++ )
    C += FZipFiles[i]->Index->GetCount();
  return C;
}
void _fastcall TCifIndex::CleanDead(TForm *Parent)  {
  TCifFile *C;
  TZipFile *Z;
  bool Cancel= false;
  olxstr N1, N2;
  dlgProgress = new TdlgProgress(NULL);
  dlgProgress->OnCancel == &Cancel;
  dlgProgress->Caption = "1. Cleaning dead links in CIFS...";
  dlgProgress->pbBar->Max = FInsFiles.Count();
  dlgProgress->AddForm(Parent);
  dlgProgress->Show();
  for( int i=0; i < FInsFiles.Count(); i++ )  {
    C = FInsFiles[i];
    dlgProgress->SetAction(C->Name);
    if( !(i%10) )  {
      dlgProgress->pbBar->Position = i;
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    if( !TEFile::FileExists(C->Name) )  {
      delete C;
      FInsFiles.NullItem(i);
      continue;
    }
  }
  FInsFiles.Pack();
  dlgProgress->Caption = "2. Cleaning dead links in ZIPS...";
  dlgProgress->pbBar->Max = FZipFiles.Count();
  for( int i=0; i < FZipFiles.Count(); i++ )  {
    Z = FZipFiles[i];
    dlgProgress->SetAction(Z->FileName);
    if( !(i%10) )  {
      dlgProgress->pbBar->Position = i;
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    if( !TEFile::FileExists(Z->FileName) )  {
      delete Z;
      FZipFiles.NullItem(i);
      continue;
    }
  }
exit:
  FZipFiles.Pack();
  delete dlgProgress;
}
void _fastcall TCifIndex::Exclusive()
{
  TCifFile *C, *C1;
  TZipFile *Z, *Z1;
  bool Cancel= false;
  olxstr N1, N2;
  TdlgProgress * dlgProg = new TdlgProgress(NULL);
  dlgProg->OnCancel == &Cancel;
  dlgProg->Caption = "1. Creating Exclusive Index for CIFs";
  dlgProg->Show();
  dlgProg->pbBar->Max = FInsFiles.Count() + FZipFiles.Count();
  for( int i=0; i < FInsFiles.Count(); i++ )  {
    if( FInsFiles.IsNull(i) )  continue;
    C = FInsFiles[i];
    if( !(i%10) )  {
      dlgProg->pbBar->Position = i;
      dlgProg->SetAction(C->Name);
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    for( int j=i+1; j < FInsFiles.Count(); j++ )  {
      if( FInsFiles.IsNull(j) )  continue;
      C1 = FInsFiles[j];
      if( C->Name == C1->Name )  {
        if( C->FileAge > C1->FileAge )  {
          delete C1;
          FInsFiles.NullItem(j);
        }
        else  {
          delete C;
          FInsFiles.NullItem(i);
          break;
        }
      }
    }
  }
  dlgProg->Caption = "2. Creating Exclusive Index for ZIPs";
  for( int i=0; i < FZipFiles.Count(); i++ )  {
    if( FZipFiles.IsNull(i) )  continue;
    Z = FZipFiles[i];
    if( Z == NULL )    continue;
    if( !(i%10) )  {
      dlgProg->pbBar->Position = FInsFiles.Count() + i;
      dlgProg->SetAction(Z->FileName);
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    for( int j=i+1; j < FZipFiles.Count(); j++ )  {
      if( FZipFiles.IsNull(j) )  continue;
      Z1 = FZipFiles[j];
      if( Z1 == NULL )    continue;
      if( Z->FileName == Z1->FileName )  {
        if( Z->FileAge > Z1->FileAge )  {
          FZipFiles.NullItem(j);
          delete Z1;
        }
        else  {
          delete Z;
          FZipFiles.NullItem(i);
          break;
        }
      }
    }
  }
exit:
  FInsFiles.Pack();
  FZipFiles.Pack();
  delete dlgProg;
}
#pragma package(smart_init)

