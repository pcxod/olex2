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
TConInfo::TConInfo()  {
  GraphRadius = 0;
  Rings = 0;
  Center = NULL;
}
TConInfo::~TConInfo()  {
  Clear();
}
void TConInfo::Clear()  {
  Paths.Clear();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int NodesSort(const TConNode& I, const TConNode& I1)  {
  return I.Nodes.Count() - I1.Nodes.Count();
}
//---------------------------------------------------------------------------
//int NodesSortMr(void *I, void *I1)  {
//  return ((TConNode*)I)->AtomType - ((TConNode*)I1)->AtomType;
//}
//---------------------------------------------------------------------------
void TConNode::Analyse(TNet& Parent, TConInfo& CI)  {
  NodePList L;
  NodeList& p_nodes = Parent.Nodes;
  CI.GraphRadius = 0;
  CI.Rings = 0;
  for( size_t i=0; i < p_nodes.Count(); i++ )  {
    TConNode& N = p_nodes[i];
    N.Id = -1;
    N.Used = false;   //
    N.Used1 = false;
  }
  this->Id = 0;
  for( size_t i=0; i < Nodes.Count(); i++ )  {
    if( i == 0 )
      CI.GraphRadius = 1;
    Nodes[i]->Id = 1;
    L.Add( Nodes[i] );
  }
  for( size_t i=0; i < L.Count(); i++ )  {
    TConNode* N = L[i];
    for( size_t j=0; j < N->Nodes.Count(); j++ )  {
      TConNode* N1 = N->Nodes[j];
      if( N1->Id == -1 )  {
        L.Add(N1);
        N1->Id = N->Id + 1;
        CI.GraphRadius = N1->Id;
//        CI->Rings--;
      }
//      else
//        CI->Rings++;
    }
  }
}
void TConNode::SetUsed1True()  {
  if( Used1 )  return;
  if( !Id )  return;
  Used1 = true;
  for( size_t i=0; i < Nodes.Count(); i++ )  {
    if( Nodes[i]->Used1 )  continue;
    if( (Nodes[i]->Id < Id) && Nodes[i]->Id != 0  )
      Nodes[i]->SetUsed1True();
  }
}
void TConNode::FindPaths(TConNode* Parent, TConInfo& CI, NodePList* Path) {
// USED should be preset to false
// ID should be initialised using ANLALYSE function
  if( Id > CI.GraphRadius )    return;
  if( CI.Paths.Count() > MaxPaths )  return;
//  if( Used )              return;
  if( Parent != NULL )  {
    if( Id <= Parent->Id )
      return;
  }
  int added = 0;
//  this->Used = true;
  if( Path == NULL )  {
    Path = new TPtrList<TConNode>();
    CI.Paths.Add(Path);
  }
  Path->Add(this);
  for( size_t i=0; i < Nodes.Count(); i++ )  {
    TConNode* N = Nodes[i];
    if( N->Id > CI.GraphRadius )
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
      TPtrList<TConNode>* L = new TPtrList<TConNode>(Id+1);
      for( int j=0; j < Id+1; j++ )  // have to copy at least one node
        (*L)[j] = Path->Item(j);
      CI.Paths.Add(L);
      N->FindPaths(this, CI, L);
    }
  }
}
void TConNode::FillList(int lastindex, TPtrList<TConNode>& L)  {
  if( this->Used )  return;
  if( Id <= lastindex )  {
    L.Add(this);
    this->Used = true;
    for( size_t i=0; i < Nodes.Count(); i++ )  {
      if( Nodes[i]->Used )  continue;
      if( Nodes[i]->Id <= lastindex )
        Nodes[i]->FillList(lastindex, L);
    }
  }
}

bool TConNode::IsSimilar(const TConNode& N) const {
  if( (Id == -1) || (N.Id == -1) )
    return false;
  if( N.AtomType != AtomType )           return false;
  if( N.Id != Id )                       return false;
  if( Nodes.Count() > N.Nodes.Count() )  return false;

  for( size_t i=0; i < N.Nodes.Count(); i++ )
    N.Nodes[i]->Used1 = false;

  for( size_t i=0; i < Nodes.Count(); i++ )  {
    TConNode* Nd = Nodes[i];
    bool found = false;
    for( size_t j=0; j < N.Nodes.Count(); j++ )  {
      TConNode* Nd1 = N.Nodes[j];
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
void TNet::Analyse()
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
  olxstr T;
//  for( int i = 0; i < FNodes.Count(); i++ )
//  {
//    N = (TConNode*)FNodes[i];
//    N->Id = i;
//  }
  for( size_t i = 0; i < Nodes.Count(); i++ )  {
    TConNode& N = Nodes[i];
    T << N.Id << '(';
    for( size_t j=0; j < N.Nodes.Count(); j++ )
      T << N.Nodes[j]->Id << ';';
    T << ')';
  }
  return T;
}
void TNet::Assign(TLattice& latt)  {
  Clear();
  for( size_t i=0; i < latt.AtomCount(); i++ )  {
    if( latt.GetAtom(i).GetAtomInfo() == iQPeakIndex )  continue;
    latt.GetAtom(i).SetTag(Nodes.Count());
    Nodes.AddNew().AtomType = latt.GetAtom(i).GetAtomInfo().GetIndex();
  }
  int ni=0;
  for( size_t i=0; i < latt.AtomCount(); i++ )  {
    TSAtom& sa = latt.GetAtom(i);
    if( sa.GetAtomInfo() == iQPeakIndex )  continue;
    for( size_t j=0; j < sa.BondCount(); j++ )    {
      if( sa.Bond(j).Another(sa).GetAtomInfo() == iQPeakIndex )
        continue;
      Nodes[ni].Nodes.Add(Nodes[sa.Bond(j).Another(sa).GetTag()]);
    }
    ni++;
  }
  NodeList::QuickSorter.SortSF(Nodes, NodesSort);
  Analyse();
}

bool TNet::IsSubstructureA(TNet& N) {
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
bool TNet::IsSubstructure(TConInfo *CI, TNet& N)  {
// CI should be initialised before the function call
  NodeList& n_nodes = N.Nodes;
  bool found_path, found_node, res = false;
  size_t PathsFound;
  if( Nodes.IsEmpty() )                   return false;
  if( Nodes.Count() > n_nodes.Count() )  return false;
  TConInfo *CI1 = new TConInfo;
  for( int i = n_nodes.Count()-1; i >= 0 ; i-- )  {
    TConNode& Nd1 = n_nodes[i];
    if( Nd1.Nodes.Count() < CI->Center->Nodes.Count() )    // nodes supposed to be sorted in ascending order
      break;
    if( CI->Center->AtomType != Nd1.AtomType )
      continue;
    Nd1.Analyse(N, *CI1);
    if( !CI->Center->IsSimilar(Nd1) )    continue;
    CI1->Center = &Nd1;
    if( CI->GraphRadius > CI1->GraphRadius )  continue;  // !shorter! graph
    if( CI->Rings > CI1->Rings )        continue;  // !shorter! graph

    CI1->GraphRadius = CI->GraphRadius;
    Nd1.FindPaths(NULL, *CI1, NULL);
    if( CI1->Paths.Count() >= MaxPaths )  {  // is it a mistake?
      res = false;
      goto exit;
    }

    if( CI->Paths.Count() > CI1->Paths.Count() )  continue;  // !shorter! graph
    PathsFound = 0;
    for( size_t j = 0; j < CI->Paths.Count(); j++ )  {
      const TPtrList<TConNode>& L = CI->Paths[j];
      for( size_t k = 0; k < CI1->Paths.Count(); k++ )  {
        const TPtrList<TConNode>& L1 = CI1->Paths[k];
        if( L1.Count() < L.Count() )  continue;
        found_path = true;
        for( size_t l=0; l < L.Count(); l++ )  {
/*          if( Nd1->AtomType != Nd2->AtomType )
          {
            found_path = false;
            break;
          }*/
          if( !L[l]->IsSimilar( *L1[l] ) )  {
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

void TNet::operator >> (IDataOutputStream& S)  {
  S << (int32_t)Nodes.Count();
  for( size_t i = 0; i < Nodes.Count(); i++ )  {
    Nodes[i].Id = i;
    S << Nodes[i].Id;
    S.Write(&Nodes[i].AtomType, 1);
  }
  for( size_t i = 0; i < Nodes.Count(); i++ )  {
    TConNode& N = Nodes[i];
    S << (int32_t)N.Nodes.Count();
    for( size_t j=0; j < N.Nodes.Count(); j++ )  {
      S << N.Nodes.Item(j)->Id;
    }
  }
}

void TNet::operator << (IDataInputStream& S)  {
  int32_t count;
  short Id;
  Clear();
  S >> count;
  Nodes.SetCapacity( count );
  for( int i = 0; i < count; i++ )  {
    TConNode& N = Nodes.AddNew();
    S >> N.Id;
    S.Read(&N.AtomType, 1);
  }
  for( size_t i = 0; i < Nodes.Count(); i++ )  {
    TConNode& N = Nodes[i];
    S >> count;
//    ConS += count;
    N.Nodes.SetCapacity( count );
    for( int j=0; j < count; j++ )  {
      S >> Id;
      N.Nodes.Add(Nodes[Id]);
    }
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void TConFile::operator >> (IDataOutputStream& S)  {
  S << FileAge;
  S << Title;
  S << FileName;
  S << SpaceGroup;
  S << Instructions;
  Net >> S;
}
void TConFile::operator << (IDataInputStream& S)  {
  S >> FileAge;
  S >> Title;
  S >> FileName;
  S >> SpaceGroup;
  S >> Instructions;
  Net << S;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TConZip::TConZip()  {
  Index = new TConIndex(this);
  FileAge = 0;
}
TConZip::~TConZip()  {
  delete Index;
}
void TConZip::operator >> (IDataOutputStream& S)  {
  S << FileName;
  S << FileAge;
  *Index >> S;
}
void TConZip::operator << (IDataInputStream& S)  {
  S >> FileName;
  S >> FileAge;
  *Index << S;
  for( size_t i=0; i < Index->ConFiles.Count(); i++ )
    Index->ConFiles[i].Parent = this;
}
void TConZip::Clear()  {
  Index->Clear();
  FileName = EmptyString;
  FileAge = 0;
}
void TConZip::Assign(TZipFile *ZF, TXFile& xf)  {
  olxstr FN;
  bool res;
  TConFile *CF;
  if( !ZF->Index->IFiles.Count() )  {  // no necessaty to update!
    Clear();
    FileAge = ZF->FileAge;
    FileName = ZF->FileName;
    return;
  }
  try  {
    ZF->GetFiles();
  }
  catch(...)  {
    FN = "Cannot process ZIP: ";
    FN << ZF->FileName;
    dlgMain->AddMessage(FN);
    FileAge = ZF->FileAge;
    FileName = ZF->FileName;
    return;
  }
  Clear();
  FileAge = ZF->FileAge;
  FileName = ZF->FileName;
  for( size_t i=0; i < ZF->Zip->Files.Count(); i++ )  {
    FN = ZF->Zip->Files[i];
    if( !TEFile::Exists(FN) )
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
    CF->Net.Assign( xf.GetLattice() );
    CF->Title = xf.LastLoader()->GetTitle();
    CF->FileName = FN;
    CF->FileAge = 0; // CifF->FileAge;
    TSpaceGroup* sg = TSymmLib::GetInstance().FindSG( xf.GetAsymmUnit() );
    if( sg != NULL )
      CF->SpaceGroup = sg->GetName();
    else
      CF->SpaceGroup = "U";
    if(  EsdlInstanceOf( *xf.LastLoader(), TIns ) )  {
      TIns* ins = &xf.GetLastLoader<TIns>();
      for( size_t j=0; j < ins->InsCount(); j++ )  {

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
    Index->ConFiles.Add(CF);
  }
}
bool TConZip::Update(TZipFile *ZF, TXFile& xf)  {
  Assign(ZF, xf);
  return true;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void TConIndex::Clear() {
  ConFiles.Clear();
  ZipFiles.Clear();
}

void TConIndex::Update(const TStrList& IndexFiles, TXFile& xf)  {
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
  for( size_t i=0; i < IndexFiles.Count(); i++ )  {
    Index->LoadFromFile(IndexFiles[i], true);
  }
  Index->Exclusive();  // removing files with the same name - avoid problems with update
  dlgProg->pbBar->Max = Index->IFiles.Count() + Index->ZFiles.Count();
  for( size_t i=0; i < Index->IFiles.Count(); i++ )  {
    CifF = Index->IFiles[i];
    if( i != 0 )  {
      dlgProg->pbBar->Position = i;
      dlgProg->SetAction(CifF->Name);
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    Uniq = true;
    for( size_t j=0; j < ConFiles.Count(); j++ )  {
      if( ConFiles[j].FileName == CifF->Name )  {
        Uniq = false;
        ConF = &ConFiles[j]; // used later!!
        break;
      }
    }
    if( Uniq )  {  // crete a new record
      if( !TEFile::Exists(CifF->Name) )
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
      ConF->Net.Assign( xf.GetLattice() );
      ConF->Title = xf.LastLoader()->GetTitle();
      ConF->FileName = CifF->Name;
      ConF->FileAge = CifF->FileAge;

      TSpaceGroup* sg = TSymmLib::GetInstance().FindSG( xf.GetAsymmUnit() );
      if( sg != NULL )
        ConF->SpaceGroup = sg->GetName();
      else
        ConF->SpaceGroup = "U";

      if(  EsdlInstanceOf( *xf.LastLoader(), TIns ) )  {
        TIns* ins = &xf.GetLastLoader<TIns>();
        for( size_t j=0; j < ins->InsCount(); j++ )  {

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

      ConFiles.Add(ConF);
    }
    else  {
      if( CifF->FileAge != ConF->FileAge )  {  // update
        dlgMain->AddMessage(olxstr("Processing '") << CifF->Name << '\'' );
        try  {
          xf.LoadFromFile(CifF->Name);
          ConF->Net.Assign( xf.GetLattice() );
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
  for( size_t i=0; i < Index->ZFiles.Count(); i++ )  {
    ZipF = Index->ZFiles[i];
    if( !(i%2) )  {
      dlgProg->pbBar->Position = Index->IFiles.Count() + i;
      dlgProg->SetAction(ZipF->FileName);
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    Uniq = true;
    for( size_t j=0; j < ZipFiles.Count(); j++ )  {
      ZipC = &ZipFiles[j];
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
      ZipFiles.Add(ZipC);
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
void TConIndex::SaveToFile(const olxstr& FN)  {
  TEFile f(FN, "wb+");
  //S->Size = S->Size + GetCount()*(sizeof(TConFile)+800);
  *this >> f;
}
void TConIndex::LoadFromFile(const olxstr& FN)  {
  TEFile inf( FN, "rb");
  if( inf.GetSize() == 0 )  return;
  *this << inf;
}
void TConIndex::Search(TConInfo* CI, TNet& N, TPtrList<TConFile>& Results, bool Silent) {
  TConFile *CF;
  TConZip *CZ;
  bool Cancel = false;
  TdlgProgress *dlgProg;
  if( N.Nodes.IsEmpty() )  return;
  bool DelCI = false;
  if( CI == NULL )  {
    CI = new TConInfo;
    CI->Center = &N.Nodes.Last();
    CI->Center->Analyse(N, *CI);
    CI->Center->FindPaths(NULL, *CI, NULL);
    DelCI = true;
  }
//  TStringList *Debug = new TStringList;
//  Debug->Add(N->GetDebugInfo());
  if( !Silent )  {
    dlgProg = new TdlgProgress(NULL);
    dlgProg->OnCancel = &Cancel;
    dlgProg->Show();
    dlgProg->Caption = "Seacrhing CIF/INS Files...";
    dlgProg->pbBar->Max = ConFiles.Count() + ZipFiles.Count();
  }
  for( size_t i=0; i < ConFiles.Count(); i++ )  {
    CF = &ConFiles[i];
    if( !(i%2) )  {
      if( !Silent )  {
        dlgProg->pbBar->Position = i;
        dlgProg->SetAction(CF->FileName);
      }
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    if( N.IsSubstructure(CI, CF->Net) )
      Results.Add(CF);
  }
  if( !Silent )
    dlgProg->Caption = "Seacrhing ZIP Files...";

  for( size_t i=0; i < ZipFiles.Count(); i++ )  {
    CZ = &ZipFiles[i];
    if( !(i%2) )  {
      if( !Silent )  {
        dlgProg->pbBar->Position = ConFiles.Count() + i;
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
TConFile * TConIndex::GetRecord(const olxstr& FileName)  {
  for( size_t i=0; i < ConFiles.Count(); i++ )
    if( !ConFiles[i].FileName.Comparei(FileName) )
      return &ConFiles[i];

  for( size_t i=0; i < ZipFiles.Count(); i++ )  {
    TConFile* CF = ZipFiles[i].Index->GetRecord(FileName);
    if( CF != NULL )
      return CF;
  }
  return NULL;
}
void TConIndex::Search(const short What, const olxstr& Text, TPtrList<TConFile>& Results, bool Silent)  {
  bool Cancel = false;
  TdlgProgress *dlgProg;
  if( !Silent )  {
    dlgProg = new TdlgProgress(NULL);
    dlgProg->OnCancel = &Cancel;
    dlgProg->Show();
    dlgProg->Caption = "Seacrhing CIF/INS Files...";
    dlgProg->pbBar->Max = ConFiles.Count() + ZipFiles.Count();
  }
  for( size_t i=0; i < ConFiles.Count(); i++ )  {
    TConFile& CF = ConFiles[i];
    if( !(i%20) )  {
      if( !Silent )  {
        dlgProg->pbBar->Position = i;
        dlgProg->SetAction(CF.FileName);
      }
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    if( (What & ssTitle) != 0 &&  CF.Title.FirstIndexOfi(Text, 0) != -1 )
        Results.Add(&CF);
    if( (What & ssSG) != 0 && !CF.SpaceGroup.Comparei(Text) )
        Results.Add(&CF);
    if( (What & ssIns) != 0 && CF.Instructions.FirstIndexOfi(Text, 0) != -1 )
        Results.Add(&CF);
  }
  if( !Silent )  dlgProg->Caption = "Seacrhing ZIP Files...";
  for( size_t i=0; i < ZipFiles.Count(); i++ )  {
    TConZip& CZ = ZipFiles[i];
    if( !(i%2) )  {
      if( !Silent )  {
        dlgProg->pbBar->Position = ConFiles.Count() + i;
        dlgProg->SetAction(CZ.FileName);
      }
      Application->ProcessMessages();
      if( Cancel )
        goto exit;
    }
    CZ.Index->Search(What, Text, Results, true);
  }
exit:
  if( !Silent )  delete dlgProg;
}
void TConIndex::operator >> (IDataOutputStream& S)  {
  S << (int32_t)ConFiles.Count();
  for( size_t i=0; i < ConFiles.Count(); i++)
    ConFiles[i] >> S;

  S << (int32_t)ZipFiles.Count();
  for( size_t i=0; i < ZipFiles.Count(); i++)
    ZipFiles[i] >> S;
}
void TConIndex::operator << (IDataInputStream& S)  {
  int32_t count;
  Clear();
  TConZip *CZ;
  S >> count;
  ConFiles.SetCapacity( count );
  for( int i=0; i < count; i++ )  {
    TConFile& CF = ConFiles.AddNew();
    CF << S;
  }
  S >> count;
  ZipFiles.SetCapacity( count );
  for( int i=0; i < count; i++ )  {
    TConZip& CZ = ZipFiles.AddNew();
    CZ << S;
  }
}
int TConIndex::GetCount()  {
  size_t c = ConFiles.Count();
  for( size_t i=0; i < ZipFiles.Count(); i++ )
    c += ZipFiles[i].Index->GetCount();
  return c;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#pragma package(smart_init)


