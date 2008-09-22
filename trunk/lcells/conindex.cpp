//---------------------------------------------------------------------------


#pragma hdrstop

#include "main.h"
#include "progress.h"
#include "draw_mol.h"
#include "conindex.h"
#include "ciffile.h"
#include "search.h"

#include "satom.h"
#include "sbond.h"
#include "symmlib.h"
#include "ins.h"
#include "ememstream.h"

const int MaxPaths = 400;
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
_fastcall TConInfo::TConInfo()  {
  GraphRadius = 0;
  Rings = 0;
  Center = NULL;
}
_fastcall TConInfo::~TConInfo()  {
  Clear();
}
void _fastcall TConInfo::Clear()  {
  for( int i=0; i < Paths.Count(); i++ )
    delete Paths[i];
  Paths.Clear();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int NodesSort(TConNode * const &I, TConNode * const &I1)  {
  return I->Nodes.Count() - I1->Nodes.Count();
}
//---------------------------------------------------------------------------
int _fastcall NodesSortMr(void *I, void *I1)
{
  return ((TConNode*)I)->AtomType - ((TConNode*)I1)->AtomType;
}
//---------------------------------------------------------------------------
void __fastcall TConNode::Analyse(TNet *Parent, TConInfo *CI)  {
  TConNode *N, *N1;
  TTypeList<TConNode*> L;
  CI->GraphRadius = 0;
  CI->Rings = 0;
  for( int i=0; i < Parent->Nodes.Count(); i++ )  {
    N = Parent->Nodes[i];
    N->Id = -1;
    N->Used = false;   //
    N->Used1 = false;
  }
  this->Id = 0;
  for( int i=0; i < Nodes.Count(); i++ )  {
    if( i == 0 )  CI->GraphRadius = 1;
    Nodes[i]->Id = 1;
    L.AddACopy( Nodes[i] );
  }
  for( int i=0; i < L.Count(); i++ )  {
    N = L[i];
    for( int j=0; j < N->Nodes.Count(); j++ )  {
      N1 = N->Nodes[j];
      if( N1->Id == -1 )  {
        L.AddACopy(N1);
        N1->Id = N->Id + 1;
        CI->GraphRadius = N1->Id;
//        CI->Rings--;
      }
//      else
//        CI->Rings++;
    }
  }
}
void _fastcall TConNode::SetUsed1True()  {
  if( Used1 )  return;
  if( !Id )  return;
  Used1 = true;
  for( int i=0; i < Nodes.Count(); i++ )  {
    if( Nodes[i]->Used1 )  continue;
    if( (Nodes[i]->Id < Id) && Nodes[i]->Id != 0  )
      Nodes[i]->SetUsed1True();
  }
}
void _fastcall TConNode::FindPaths(TConNode *Parent, TConInfo *CI, TTypeList<TConNode*> *Path)
{
// USED should be preset to false
// ID should be initialised using ANLALYSE function
  if( Id > CI->GraphRadius )    return;
  if( CI->Paths.Count() > MaxPaths )  return;
//  if( Used )              return;
  if( Parent )  {
    if( Id <= Parent->Id )
      return;
  }
  TConNode *N, *N1;
  TTypeList<TConNode*>* L;
  int added = 0;
//  this->Used = true;
  if( !Path )  {
    Path = new TTypeList<TConNode*>();
    CI->Paths.AddACopy(Path);
  }
  Path->AddACopy(this);
  for( int i=0; i < Nodes.Count(); i++ )  {
    N = Nodes[i];
    if( N->Id > CI->GraphRadius )
      continue;
    if( Parent != NULL )  {
      if( N->Id <= Parent->Id )
        continue;
    }
//     if( N->Used )
//      continue;
    if( !added )  {
      N->FindPaths(this, CI, Path);
      added++;
    }
    else  {
      L = new TTypeList<TConNode*>();
      L->SetCapacity(Id+1);
      for( int j=0; j < Id+1; j++ )  // have to copy at least one node
        L->AddACopy( Path->Item(j) );
      CI->Paths.AddACopy(L);
      N->FindPaths(this, CI, L);
    }
  }
}
void _fastcall TConNode::FillList(int lastindex, TTypeList<TConNode*>& L)  {
  if( this->Used )  return;
  if( Id <= lastindex )  {
    L.AddACopy(this);
    this->Used = true;
    for( int i=0; i < Nodes.Count(); i++ )  {
      if( Nodes[i]->Used )  continue;
      if( Nodes[i]->Id <= lastindex )
        Nodes[i]->FillList(lastindex, L);
    }
  }
}

bool _fastcall TConNode::IsSimilar(TConNode *N)
{
  if( (Id == -1) || (N->Id == -1) )
    return false;
  if( N->AtomType != AtomType )    return false;
  if( N->Id != Id )          return false;
  if( Nodes.Count() > N->Nodes.Count() )return false;

  TConNode *Nd, *Nd1;
  bool found;
  for( int i=0; i < N->Nodes.Count(); i++ )
    N->Nodes[i]->Used1 = false;

  for( int i=0; i < Nodes.Count(); i++ )  {
    Nd = Nodes[i];
    found = false;
    for( int j=0; j < N->Nodes.Count(); j++ )  {
      Nd1 = N->Nodes[j];
      if( Nd1->Used1 )
        continue;
      if( Nd->AtomType != Nd1->AtomType )
        continue;
      if( Nd->Id == Nd1->Id )  {
        found = true;
        Nd1->Used1 = true;
        break;
      }
    }
    if( !found )
      return false;
  }
  return true;
}

//---------------------------------------------------------------------------
_fastcall TNet::TNet()  {
}
void _fastcall TNet::Clear()  {
  for( int i=0; i < FNodes.Count(); i++ )
    delete FNodes[i];
  FNodes.Clear();
}
_fastcall TNet::~TNet()  {
  for( int i=0; i < FNodes.Count(); i++ )
    delete FNodes[i];
}

void _fastcall TNet::Analyse()
{
/*  if( FNodes.Count() )
  {
    ((TConNode*)FNodes->Items[0])->Analyse(this, FCollisions, FMouths);
  }

  TConNode *N;
  for( int i = 0; i < FNodes.Count(); i++ )
  {
    N = (TConNode*)FNodes[i];
    ConS += N->Nodes.Count();
    Mr += N->AtomType;
  }*/
  return;
}

olxstr TNet::GetDebugInfo()  {
  TConNode *N;
  olxstr T;
//  for( int i = 0; i < FNodes.Count(); i++ )
//  {
//    N = (TConNode*)FNodes[i];
//    N->Id = i;
//  }
  for( int i = 0; i < FNodes.Count(); i++ )  {
    N = FNodes[i];
    T << N->Id << '(';
    for( int j=0; j < N->Nodes.Count(); j++ )
      T << N->Nodes[j]->Id << ';';
    T << ')';
  }
  return T;
}
void _fastcall TNet::Assign(TLattice& latt)  {
  Clear();
  for( int i=0; i < latt.AtomCount(); i++ )  {
    if( latt.GetAtom(i).GetAtomInfo() == iQPeakIndex )  continue;
    latt.GetAtom(i).SetTag(FNodes.Count());
    TConNode *N = new TConNode;
    FNodes.AddACopy(N);
    N->AtomType = latt.GetAtom(i).GetAtomInfo().GetIndex();
  }
  int ni=0;
  for( int i=0; i < latt.AtomCount(); i++ )  {
    TSAtom& sa = latt.GetAtom(i);
    if( sa.GetAtomInfo() == iQPeakIndex )  continue;
    for( int j=0; j < sa.BondCount(); j++ )    {
      if( sa.Bond(j).Another(sa).GetAtomInfo() == iQPeakIndex )
        continue;
      FNodes[ni]->Nodes.AddACopy( FNodes[sa.Bond(j).Another(sa).GetTag()] );
    }
    ni++;
  }
  TTypeList<TConNode*>::QuickSorter.SortSF(FNodes, NodesSort);
  Analyse();
}

bool _fastcall TNet::IsSubstructureA(TNet *N)
{
/*  if( (N->Nodes.Count() < FNodes.Count()) )
    return false;
  if( !FNodes.Count() )
    return false;
  TConNode *Nd, *Nd1, *Nd2, *Nd3;
  bool found;
  int NodesFound;
  for( int bn = FNodes.Count()-1; bn >= 0; bn-- )
  {
    Nd = (TConNode*)FNodes->Items[bn];
    Nd->Analyse(this);
    // nodes supposed to be sorted in ascending order
    for( int i = N->Nodes.Count()-1; i >= 0 ; i-- )
    {
      Nd1 = (TConNode*)N->Nodes[i];
      if( Nd1->AtomType != Nd->AtomType )
        continue;
      if( Nd1->Nodes.Count() < Nd->Nodes.Count() )
        return false;
      Nd1->Analyse(N);
      for( int j = N->Nodes.Count()-1; j >= 0 ; j-- )
      {
        Nd3 = (TConNode*)N->Nodes->Items[j];
        Nd3->Used1 = false;
      }
      NodesFound = 0;
      for( int j = FNodes.Count()-1; j >=0 ; j-- )
      {
        Nd2 = (TConNode*)FNodes->Items[j];
        found = false;
        for( int k = 0; k < N->Nodes.Count(); k++ )
        {
          Nd3 = (TConNode*)N->Nodes->Items[k];
          if( Nd3->Used1 )  continue;
          if( Nd2->IsSimilar(Nd3) )
          {
            found = true;
            NodesFound++;
            Nd3->Used1 = true;
            break;
          }
        }
        if( !found )
          break;
      }
      if( NodesFound == FNodes.Count() )  // all nodes are found
        return true;
    }
  }
  return false;
  */
  return false;
}
bool _fastcall TNet::IsSubstructure(TConInfo *CI, TNet *N)
{
// CI should be initialised before the function call
  TConNode *Nd1, *Nd2;
  bool found_path, found_node, res = false;
  int PathsFound;
  if( !FNodes.Count() )            return false;
  if( FNodes.Count() > N->Nodes.Count() )    return false;
  TConInfo *CI1 = new TConInfo;
  TTypeList<TConNode*> *L, *L1;
  for( int i = N->Nodes.Count()-1; i >= 0 ; i-- )  {
    Nd1 = N->Nodes[i];
    if( Nd1->Nodes.Count() < CI->Center->Nodes.Count() )    // nodes supposed to be sorted in ascending order
      break;
    if( CI->Center->AtomType != Nd1->AtomType )
      continue;
    Nd1->Analyse(N, CI1);
    if( !CI->Center->IsSimilar(Nd1) )    continue;
    CI1->Center = Nd1;
    if( CI->GraphRadius > CI1->GraphRadius )  continue;  // !shorter! graph
    if( CI->Rings > CI1->Rings )        continue;  // !shorter! graph

    CI1->GraphRadius = CI->GraphRadius;
    Nd1->FindPaths(NULL, CI1, NULL);
    if( CI1->Paths.Count() >= MaxPaths )  {  // is it a mistake?
      res = false;
      goto exit;
    }

    if( CI->Paths.Count() > CI1->Paths.Count() )  continue;  // !shorter! graph
    PathsFound = 0;
    for( int j = 0; j < CI->Paths.Count(); j++ )  {
      L = CI->Paths[j];
      for( int k = 0; k < CI1->Paths.Count(); k++ )  {
        L1 = CI1->Paths[k];
        if( L1->Count() < L->Count() )  continue;
        found_path = true;
        for( int l=0; l < L->Count(); l++ )  {
/*          if( Nd1->AtomType != Nd2->AtomType )
          {
            found_path = false;
            break;
          }*/
          if( !L->Item(l)->IsSimilar( L1->Item(l) ) )  {
            found_path = false;
            break;
          }
        }
        if( found_path )
          break;
      }
      if( !found_path )    break;
      else                 PathsFound++;
    }
    if( PathsFound == CI->Paths.Count() )  {
      res = true;
      goto exit;
    }
  }
exit:
    delete CI1;
  return res;
}

void _fastcall TNet::operator >> (IDataOutputStream& S)  {
  TConNode *N;
  for( int i = 0; i < FNodes.Count(); i++ )
    FNodes[i]->Id = i;
  S << (int32_t)FNodes.Count();
  for( int i = 0; i < FNodes.Count(); i++ )  {
    S << FNodes[i]->Id;
    S.Write(&FNodes[i]->AtomType, 1);
  }
  for( int i = 0; i < FNodes.Count(); i++ )  {
    N = FNodes[i];
    S << (int32_t)N->Nodes.Count();
    for( int j=0; j < N->Nodes.Count(); j++ )  {
      S << N->Nodes.Item(j)->Id;
    }
  }
}

void _fastcall TNet::operator << (IDataInputStream& S)  {
  TConNode *N, *N1;
  int32_t count;
  short Id;
  Clear();
  S >> count;
  FNodes.SetCapacity( count );
  for( int i = 0; i < count; i++ )  {
    N = new TConNode;
    S >> N->Id;
    S.Read(&N->AtomType, 1);
    FNodes.AddACopy( N );
//    Mr += N->AtomType;
  }
  for( int i = 0; i < FNodes.Count(); i++ )  {
    N = FNodes[i];
    S >> count;
//    ConS += count;
    N->Nodes.SetCapacity( count );
    for( int j=0; j < count; j++ )  {
      S >> Id;
      N->Nodes.AddACopy( FNodes[Id] );
    }
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
_fastcall TConFile::TConFile()  {
  FFileAge = 0;
  FNet = new TNet;
  FParent = NULL;
}
_fastcall TConFile::~TConFile()  {
  delete FNet;
}
void _fastcall TConFile::operator >> (IDataOutputStream& S)  {
  S << FFileAge;
  S << FTitle;
  S << FFileName;
  S << FSpaceGroup;
  S << FInstructions;
  *FNet >> S;
}
void _fastcall TConFile::operator << (IDataInputStream& S)  {
  S >> FFileAge;
  S >> FTitle;
  S >> FFileName;
  S >> FSpaceGroup;
  S >> FInstructions;
  *FNet << S;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
_fastcall TConZip::TConZip()  {
  FIndex = new TConIndex;
  FIndex->Parent = this;
  FFileAge = 0;
}
_fastcall TConZip::~TConZip()  {
  delete FIndex;
}
void _fastcall TConZip::operator >> (IDataOutputStream& S)  {
  S << FFileName;
  S << FFileAge;
  *FIndex >> S;
}
void _fastcall TConZip::operator << (IDataInputStream& S)  {
  S >> FFileName;
  S >> FFileAge;
  *FIndex << S;
  for( int i=0; i < FIndex->ConFiles.Count(); i++ )
    FIndex->ConFiles[i]->Parent = this;
}
void _fastcall TConZip::Clear()  {
  FIndex->Clear();
  FFileName = "";
  FFileAge = 0;
}
void _fastcall TConZip::Assign(TZipFile *ZF, TXFile& xf)  {
  olxstr FN;
  bool res;
  TConFile *CF;
  if( !ZF->Index->IFiles.Count() )  {  // no necessaty to update!
    Clear();
    FFileAge = ZF->FileAge;
    FFileName = ZF->FileName;
    return;
  }
  try  {
    ZF->GetFiles();
  }
  catch(...)  {
    FN = "Cannot process ZIP: ";
    FN << ZF->FileName;
    dlgMain->AddMessage(FN);
    FFileAge = ZF->FileAge;
    FFileName = ZF->FileName;
    return;
  }
  Clear();
  FFileAge = ZF->FileAge;
  FFileName = ZF->FileName;
  for( int i=0; i < ZF->Zip->Files.Count(); i++ )  {
    FN = ZF->Zip->Files.String(i);
    if( !TEFile::FileExists(FN) )
      continue;
    try  {  xf.LoadFromFile(FN);  }
    catch(const TExceptionBase& exc)  {
      TStrList sl;
      exc.GetException()->GetStackTrace(sl);
      dlgMain->AddMessage( olxstr("Exception occured while processing '") << FN << "':");
      dlgMain->AddMessage( sl );
      continue;
    }
    CF = new TConFile;
    CF->Parent = this;
    CF->Net->Assign( xf.GetLattice() );
    CF->Title = xf.GetLastLoader()->GetTitle();
    CF->FileName = FN;
    CF->FileAge = 0; // CifF->FileAge;
    TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( xf.GetAsymmUnit() );
    if( sg != NULL )
      CF->SpaceGroup = sg->GetName();
    else
      CF->SpaceGroup = "U";
    if(  EsdlInstanceOf( *xf.GetLastLoader(), TIns ) )  {
      TIns* ins = (TIns*)xf.GetLastLoader();
      for( int j=0; j < ins->InsCount(); j++ )  {

        if( !ins->InsName(j).Length() ||
            !( (ins->InsName(j)[0] < 'z' && ins->InsName(j)[0] >= 'a') ||
               (ins->InsName(j)[0] < 'Z' && ins->InsName(j)[0] >= 'A') ) )
          continue;

        CF->Instructions << ins->InsName(j);
        if( ins->InsParams(j).Count() != 0 )
          CF->Instructions << ' ' << ins->InsParams(j).Text(' ');
        CF->Instructions << ';';
      }
    }
    FIndex->ConFiles.AddACopy(CF);
  }
}
bool _fastcall TConZip::Update(TZipFile *ZF, TXFile& xf)  {
  Assign(ZF, xf);
  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TConIndex::TConIndex()  {
}
__fastcall TConIndex::~TConIndex() {
  Clear();
}
void _fastcall TConIndex::Clear()
{
  for( int i=0; i < FConFiles.Count(); i++ )
    delete FConFiles[i];
  for( int i=0; i < FZipFiles.Count(); i++ )
    delete FZipFiles[i];
  FConFiles.Clear();
  FZipFiles.Clear();
}

void _fastcall TConIndex::Update(const TStrList& IndexFiles, TXFile& xf)  {
  TCifIndex *Index = new TCifIndex();
  TConFile *ConF;
  TCifFile *CifF;
  TZipFile *ZipF;
  TConZip   *ZipC;
  bool Uniq, Cancel = false, res;

  TdlgProgress *dlgProg = new TdlgProgress(NULL);
  dlgProg->OnCancel = &Cancel;
//  dlgProg->AddForm(dlgSearch);  //the form is not defined
  dlgProg->Show();
  dlgProg->Caption = "Updating information CIF/INS Files...";
  for( int i=0; i < IndexFiles.Count(); i++ )  {
    Index->LoadFromFile(IndexFiles.String(i), true);
  }
  Index->Exclusive();  // removing files with the same name - avoid problems with update
  dlgProg->pbBar->Max = Index->IFiles.Count() + Index->ZFiles.Count();
  for( int i=0; i < Index->IFiles.Count(); i++ )  {
    CifF = Index->IFiles[i];
    if( i != 0 )  {
      dlgProg->pbBar->Position = i;
      dlgProg->SetAction(CifF->Name);
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    Uniq = true;
    for( int j=0; j < FConFiles.Count(); j++ )  {
      if( FConFiles[j]->FileName == CifF->Name )  {
        Uniq = false;
        ConF = FConFiles[j]; // used later!!
        break;
      }
    }
    if( Uniq )  {  // crete a new record
      if( !TEFile::FileExists(CifF->Name) )
        continue;
      dlgMain->AddMessage(olxstr("Processing '") << CifF->Name << '\'' );
      if( TEFile::ExtractFileExt(CifF->Name).IndexOf(';') != -1 )  continue;
      try  {  xf.LoadFromFile(CifF->Name);  }
      catch(const TExceptionBase& exc)  {
        TStrList sl;
        exc.GetException()->GetStackTrace( sl );
        dlgMain->AddMessage( sl );
        continue;
      }
//      dlgMain->AddMessage("Done");
      ConF = new TConFile;
      ConF->Net->Assign( xf.GetLattice() );
      ConF->Title = xf.GetLastLoader()->GetTitle();
      ConF->FileName = CifF->Name;
      ConF->FileAge = CifF->FileAge;

      TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( xf.GetAsymmUnit() );
      if( sg != NULL )
        ConF->SpaceGroup = sg->GetName();
      else
        ConF->SpaceGroup = "U";

      if(  EsdlInstanceOf( *xf.GetLastLoader(), TIns ) )  {
        TIns* ins = (TIns*)xf.GetLastLoader();
        for( int j=0; j < ins->InsCount(); j++ )  {

          if( !ins->InsName(j).Length() ||
              !( (ins->InsName(j)[0] < 'z' && ins->InsName(j)[0] >= 'a' ) ||
                 (ins->InsName(j)[0] < 'Z' && ins->InsName(j)[0] >= 'A' ) ) )
            continue;

          ConF->Instructions << ins->InsName(j);
          if( ins->InsParams(j).Count() != 0 )
            ConF->Instructions << ' ' << ins->InsParams(j).Text(' ');
          ConF->Instructions << ';';
        }
      }

      FConFiles.AddACopy(ConF);
    }
    else  {
      if( CifF->FileAge != ConF->FileAge )  {  // update
        dlgMain->AddMessage(olxstr("Processing '") << CifF->Name << '\'' );
        try  {
          xf.LoadFromFile(CifF->Name);
          ConF->Net->Assign( xf.GetLattice() );
          ConF->FileAge = CifF->FileAge;
        }
        catch(const TExceptionBase& exc)  {
          TStrList sl;
          exc.GetException()->GetStackTrace( sl );
          dlgMain->AddMessage( sl );
          continue;
        }
      }
    }
  }
  dlgProg->Caption = "Updating information ZIP Files...";
  for( int i=0; i < Index->ZFiles.Count(); i++ )  {
    ZipF = Index->ZFiles[i];
    if( !(i%2) )  {
      dlgProg->pbBar->Position = Index->IFiles.Count() + i;
      dlgProg->SetAction(ZipF->FileName);
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    Uniq = true;
    for( int j=0; j < FZipFiles.Count(); j++ )  {
      ZipC = FZipFiles[j];
      if( ZipC->FileName == ZipF->FileName )  {
        Uniq = false;
        break;
      }
    }
    if( Uniq )  {  // cretae new record
      ZipC = new TConZip;
      try  {
        ZipC->Assign(ZipF, xf);
      }
      catch(...)  {
        dlgMain->AddMessage(olxstr("Cannot process ZIP: ") << ZipF->FileName );
//        delete ZipC;  // will keep a reference
//        continue;
      }
      FZipFiles.AddACopy(ZipC);
    }
    else  {
      if( ZipC->FileAge != ZipF->FileAge )  {  // update
        dlgMain->AddMessage(olxstr("Processing '") << ZipF->FileName << '\'' );
        try  {  ZipC->Update(ZipF, xf);  }
        catch( const TExceptionBase& exc )  {
          TStrList sl;
          exc.GetException()->GetStackTrace( sl );
          dlgMain->AddMessage( sl );
          continue;
        }
      }
    }
  } 
exit:
  delete Index;
  delete dlgProg;
}
void _fastcall TConIndex::SaveToFile(const olxstr& FN)  {
  TEFile f(FN, "wb+");
  //S->Size = S->Size + GetCount()*(sizeof(TConFile)+800);
  *this >> f;
}
void _fastcall TConIndex::LoadFromFile(const olxstr& FN)  {
  TEFile inf( FN, "rb");
  if( inf.GetSize() == 0 )  return;
  *this << inf;
}
void _fastcall TConIndex::Search(TConInfo *CI, TNet *N, TTypeList<TConFile*>& Results, bool Silent)
{
  TConFile *CF;
  TConZip *CZ;
  bool Cancel = false;
  TdlgProgress *dlgProg;
  if( !N->Nodes.Count() )  return;
  bool DelCI = false;
  if( CI == NULL )  {
    CI = new TConInfo;
    CI->Center = (TConNode*)N->Nodes[N->Nodes.Count()-1];
    CI->Center->Analyse(N, CI);
    CI->Center->FindPaths(NULL, CI, NULL);
    DelCI = true;
  }
//  TStringList *Debug = new TStringList;
//  Debug->Add(N->GetDebugInfo());
  if( !Silent )  {
    dlgProg = new TdlgProgress(NULL);
    dlgProg->OnCancel = &Cancel;
    dlgProg->Show();
    dlgProg->Caption = "Seacrhing CIF/INS Files...";
    dlgProg->pbBar->Max = FConFiles.Count() + FZipFiles.Count();
  }
  for( int i=0; i < FConFiles.Count(); i++ )  {
    CF = FConFiles[i];
    if( !(i%2) )  {
      if( !Silent )  {
        dlgProg->pbBar->Position = i;
        dlgProg->SetAction(CF->FileName);
      }
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    if( N->IsSubstructure(CI, CF->Net) )
      Results.AddACopy(CF);
  }
  if( !Silent )
    dlgProg->Caption = "Seacrhing ZIP Files...";

  for( int i=0; i < FZipFiles.Count(); i++ )  {
    CZ = FZipFiles[i];
    if( !(i%2) )  {
      if( !Silent )  {
        dlgProg->pbBar->Position = FConFiles.Count() + i;
        dlgProg->SetAction(CZ->FileName);
      }
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    CZ->Index->Search(CI, N, Results, true);
  }
exit:
  if( !Silent )  delete dlgProg;
  if( DelCI )    delete CI;
//  Debug->SaveToFile("d:\\debug.dat");
//  delete Debug;
}
TConFile * _fastcall TConIndex::GetRecord(const olxstr& FileName)  {
  TConFile *CF;
  TConZip *CZ;
  for( int i=0; i < FConFiles.Count(); i++ )
    if( !FConFiles[i]->FileName.Comparei(FileName) )
      return FConFiles[i];

  for( int i=0; i < FZipFiles.Count(); i++ )  {
    CF = FZipFiles[i]->Index->GetRecord(FileName);
    if( CF != NULL )
      return CF;
  }
  return NULL;
}
void _fastcall TConIndex::Search(const short What, const olxstr& Text, TTypeList<TConFile*>& Results, bool Silent)  {
  TConFile *CF;
  TConZip *CZ;
  bool Cancel = false;
  TdlgProgress *dlgProg;
  if( !Silent )  {
    dlgProg = new TdlgProgress(NULL);
    dlgProg->OnCancel = &Cancel;
    dlgProg->Show();
    dlgProg->Caption = "Seacrhing CIF/INS Files...";
    dlgProg->pbBar->Max = FConFiles.Count() + FZipFiles.Count();
  }
  for( int i=0; i < FConFiles.Count(); i++ )  {
    CF = FConFiles[i];
    if( !(i%20) )  {
      if( !Silent )  {
        dlgProg->pbBar->Position = i;
        dlgProg->SetAction(CF->FileName);
      }
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    if( (What & ssTitle) != 0 &&  CF->Title.FirstIndexOfi(Text, 0) != -1 )
        Results.AddACopy(CF);
    if( (What & ssSG) != 0 && !CF->SpaceGroup.Comparei(Text) )
        Results.AddACopy(CF);
    if( (What & ssIns) != 0 && CF->Instructions.FirstIndexOfi(Text, 0) != -1 )
        Results.AddACopy(CF);
  }
  if( !Silent )  dlgProg->Caption = "Seacrhing ZIP Files...";
  for( int i=0; i < FZipFiles.Count(); i++ )  {
    CZ = FZipFiles[i];
    if( !(i%2) )  {
      if( !Silent )  {
        dlgProg->pbBar->Position = FConFiles.Count() + i;
        dlgProg->SetAction(CZ->FileName);
      }
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    CZ->Index->Search(What, Text, Results, true);
  }
exit:
  if( !Silent )  delete dlgProg;
}
void _fastcall TConIndex::operator >> (IDataOutputStream& S)  {
  S << FConFiles.Count();
  for( int i=0; i < FConFiles.Count(); i++)
    *FConFiles[i] >> S;

  S << FZipFiles.Count();
  for( int i=0; i < FZipFiles.Count(); i++)
    *FZipFiles[i] >> S;
}
void _fastcall TConIndex::operator << (IDataInputStream& S)  {
  int count;
  Clear();
  TConFile *CF;
  TConZip *CZ;
  S >> count;
  FConFiles.SetCapacity( count );
  for( int i=0; i < count; i++ )  {
    CF = new TConFile;
    *CF << S;
    FConFiles.AddACopy( CF );
  }
  S >> count;
  FZipFiles.SetCapacity( count );
  for( int i=0; i < count; i++ )  {
    CZ = new TConZip;
    *CZ << S;
    FZipFiles.AddACopy( CZ );
  }
}
int TConIndex::GetCount()  {
  int c = FConFiles.Count();
  for( int i=0; i < FZipFiles.Count(); i++ )
    c += FZipFiles[i]->Index->GetCount();
  return c;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#pragma package(smart_init)

