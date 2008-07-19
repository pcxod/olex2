//---------------------------------------------------------------------------//
// namespace TXFiles: TIns - basic procedures for the SHELX instruction files
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdlib.h>

#include "ins.h"

#include "bapp.h"
#include "log.h"

#include "catom.h"
#include "ellipsoid.h"
#include "unitcell.h"
#include "symmparser.h"

#include "efile.h"
#include "lst.h"
#include "p4p.h"
#include "crs.h"
#include "cif.h"
#include "symmlib.h"
#include "typelist.h"
#include "egc.h"

#undef AddAtom
#undef GetObject
#undef Object

//----------------------------------------------------------------------------//
// TIns function bodies
//----------------------------------------------------------------------------//
TIns::TIns(TAtomsInfo *S) : TBasicCFile(S)  {
  Radiation = 0.71073f;
  HKLF = 4;
  LoadQPeaks = true;
}
//..............................................................................
TIns::~TIns()  {
  Clear();
}
//..............................................................................
void TIns::Clear()  {
  GetAsymmUnit().Clear();
  for( int i=0; i < Ins.Count(); i++ )
    delete Ins.Object(i);
  Ins.Clear();
  Skipped.Clear();
  FTitle = EmptyString;
  FVars.Resize(0);
  FWght.Resize(0);
  FWght1.Resize(0);
  FHKLSource = EmptyString;
  FLS.Resize(0);
  FPLAN.Resize(0);
  Sfac = EmptyString;
  Unit = EmptyString;
  Error = 0;
}
//..............................................................................
void TIns::LoadFromStrings(const TStrList& FileContent)  {
  Clear();
  ParseResults pr;
  TStrList Toks, InsFile(FileContent);
  InsFile.CombineLines('=');
  bool   End = false;// true if END instruction reached
  short   Part=0, Afix=0, SameAtomsLeft=0, SameId=-1;    // atom's PART
  double partOccu = 0;
  TAsymmUnit::TResidue* CurrResi = &GetAsymmUnit().GetResidue(-1);
  for( int i=0; i < InsFile.Count(); i++ )
    InsFile[i] = olxstr::DeleteSequencesOf<char>(InsFile[i], ' ');
  for( int i=0; i < InsFile.Count(); i++ )  {
    if( InsFile[i].IsEmpty() )      continue;

    for( int j=0; j < InsFile[i].Length(); j++ )  {
      if( InsFile[i].CharAt(j) == '!' )  {  // comment sign
        InsFile[i].SetLength(j-1);
        break;
      }
    }
    Toks.Clear();
    Toks.Strtok(InsFile[i], ' ');
    if( Toks.IsEmpty() )  continue;

    if( Toks[0].Comparei("SUMP") == 0 )  // can look like an atom !
      Ins.Add(InsFile[i]);
    else if( ParseIns(InsFile, Toks, pr, i) )
      continue;
    else if( Toks[0].Comparei("PART") == 0 && (Toks.Count() > 1) )  {
      Part = (short)Toks[1].ToInt();
      if( Part == 0 )  partOccu = 0;
      if( Toks.Count() == 3 )
        partOccu = Toks[2].ToDouble();
    }
    else if( Toks[0].Comparei("AFIX") == 0 && (Toks.Count() > 1) )  {
      Afix = (short)Toks[1].ToInt();
    }
    else if( Toks[0].Comparei("RESI") == 0 )  {
      if( Toks.Count() < 3 )
        throw TInvalidArgumentException(__OlxSourceInfo, "wrong number of arguments for a residue");
      if( Toks[1].IsNumber() )
        CurrResi = &GetAsymmUnit().NewResidue(EmptyString, Toks[1].ToInt(), (Toks.Count() > 2) ? Toks[2] : EmptyString);
      else
        CurrResi = &GetAsymmUnit().NewResidue(Toks[1], Toks[2].ToInt(), (Toks.Count() > 3) ? Toks[3] : EmptyString);
    }
    else if( Toks[0].StartsFromi("SAME") )  {
      if( SameAtomsLeft != 0 )
        throw TFunctionFailedException(__OlxSourceInfo, "previous SAME is incomplete");
      int resi_ind = Toks[0].IndexOf('_');
      olxstr resi( (resi_ind != -1) ? Toks[0].SubStringFrom(resi_ind+1) : EmptyString );
      TAtomReference ar( Toks.Text(' ', 1) );
      TCAtomGroup ag;
      int atomAGroup;
      try  {  ar.Expand( GetAsymmUnit(), ag, resi, atomAGroup);  }
      catch( const TExceptionBase& ex )  {
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid SAME instruction :") << ex.GetException()->GetError());
      }
      if( ag.IsEmpty() )
        throw TFunctionFailedException(__OlxSourceInfo, "forward referencing is not enabled for SAME");
      SameAtomsLeft = ag.Count();
      for( int j=0; j < ag.Count(); j++ )
        ag[j].GetAtom()->SetSortable(false);
      TSimpleRestraint& sr = GetAsymmUnit().SimilarFragments().AddNew();
      SameId = GetAsymmUnit().SimilarFragments().Count() - 1;
      sr.AddAtoms(ag);
      sr.SetEsd(0.02);
      sr.SetEsd1(0.02);
    }
    else if( Toks[0].Comparei("END") == 0 )     {   //reset RESI to default
      End = true;  
      CurrResi = &GetAsymmUnit().GetResidue(-1);
    }
    else if( Toks.Count() < 6 )  // atom sgould have at least 7 parameters
      Ins.Add(InsFile[i]);
    else {
      if( olxstr::o_toupper(Toks[0].CharAt(0)) == 'Q' && !End )  {
        if( !LoadQPeaks  )  continue;
      }
      if( End && (olxstr::o_toupper(Toks[0].CharAt(0)) != 'Q') )  continue;
    // is a valid atom
      if( !AtomsInfo->IsAtom(Toks[0]))  {  Ins.Add(InsFile[i]);  continue;  }
      if( !Toks[1].IsNumber() )         {  Ins.Add(InsFile[i]);  continue;  }
      int index  = Toks[1].ToInt();
      if( index < 1 || index > pr.BasicAtoms.Count() )  {  // wrong index in SFAC
        Ins.Add(InsFile[i]);
        continue;
      }
      // should be four numbers
      if( (!Toks[2].IsNumber()) || (!Toks[3].IsNumber()) ||
        (!Toks[4].IsNumber()) || (!Toks[5].IsNumber()) )  {
          Ins.Add(InsFile[i]);
          continue;
      }
      if( !pr.CellFound )  {
        Clear();
        throw TFunctionFailedException(__OlxSourceInfo, "uninitialised cell");
      }
      TCAtom* atom = _ParseAtom(Toks, partOccu, CurrResi );
      atom->SetPart(Part);
      atom->SetAfix(Afix);
      int afix_group = Afix/10;
      if( afix_group == 5 || afix_group == 6 || afix_group == 7 || 
        afix_group == 10 || afix_group == 11 || afix_group > 160 )
        atom->SetSortable(false);
      if( SameAtomsLeft != 0 )  {
        SameAtomsLeft--;
        atom->SetSortable(false);
      }
      if( SameId != -1 )  {
        atom->SetSameId(SameId);
        SameId = -1;
      }
      atom->SetLabel( Toks[0] );
      if( atom->GetAtomInfo() != iQPeakIndex )  // the use sfac
        atom->AtomInfo( pr.BasicAtoms.Object(Toks[1].ToInt()-1) );
    }
  }
  if( GetSfac().CharCount(' ') != GetUnit().CharCount(' ') )  {
    Clear();
    throw TFunctionFailedException(__OlxSourceInfo, "mismatching SFAC/UNIT");
  }
  TMatrixD sm(3,4);
  for( int i=0; i < pr.Symm.Count(); i++ )  {
    if( TSymmParser::SymmToMatrix(pr.Symm[i], sm) )
      GetAsymmUnit().AddMatrix(sm);
  }
  // remove dublicated instructtions, rems etc
  for( int i = 0; i < Ins.Count(); i++ )  {
    if( Ins.String(i).IsEmpty() )  continue;
    for( int j = i+1; j < Ins.Count(); j++ )  {
      if( Ins[i] == Ins[j] )
        Ins[j] = EmptyString;
    }
  }

  Ins.Pack();
  ParseRestraints(Ins, NULL);
  Ins.Pack();
  _FinishParsing();
  // initialise asu data
  GetAsymmUnit().InitData();
  // this bit was modified, so it now takes both - one line an multiline
  // parameter; the closing tag must exist - otherwise all rems after <hkl>
  // will be removed
  for( int i=0; i < InsCount(); i++ )  {
    if( !InsName(i).Comparei("exyz") && (InsParams(i).Count() > 1) ) {
      TCAtomPList atoms;
      bool error = false;
      for( int j=0; j < InsParams(i).Count(); j++ )  {
        TCAtom* atom = GetAsymmUnit().FindCAtom(InsParams(i).String(j));
        if( atom == NULL )  {
          TBasicApp::GetLog().Error(olxstr("TIns::LoadFromStrings: unknow atom in EXYZ instruction - ") << InsParams(i).String(j));
          TBasicApp::GetLog().Error("Please fix the problem and reload the file");
          error = true;
          break;
        }
        atoms.Add(atom);
      }
      if( !error )  GetAsymmUnit().AddExyz(atoms);
    }
  }
  if( !pr.CellFound )  {  // in case there are no atoms
    Clear();
    throw TInvalidArgumentException(__OlxSourceInfo, "empty CELL");
  }
}
//..............................................................................
void TIns::_FinishParsing()  {
  for( int i =0; i < Ins.Count(); i++ )  {
    TInsList* Param = new TInsList(Ins[i], ' ');
    Ins.Object(i) = Param;
    Ins[i] = Param->String(0);
    // special treatment of HFIX instructions
    if( !Param->String(0).Comparei("HFIX") && Param->Count() > 2 ) {
      int iv = Param->String(1).ToInt();
      if( iv > 0 )  {
        for( int j=2; j<Param->Count(); j++ )  {
          TCAtom* atom = GetAsymmUnit().FindCAtom(Param->String(j));
          if( atom != NULL )
            atom->SetHfix(iv);
        }
      }
      continue;
    }
    //end
    Param->Delete(0);
    for( int j=0; j < Param->Count(); j++ )
      Param->Object(j) = GetAsymmUnit().FindCAtom(Param->String(j));
  }
}
//..............................................................................
bool TIns::ParseIns(const TStrList& ins, const TStrList& Toks, ParseResults& res, int& i)  {
  if( _ParseIns(Toks) )
    return true;
  else if( !res.CellFound && Toks[0].Comparei("CELL") == 0 )  {
    if( Toks.Count() == 8 )  {
      SetRadiation( Toks[1].ToDouble() );
      GetAsymmUnit().Axes().Value(0) = Toks[2];
      GetAsymmUnit().Axes().Value(1) = Toks[3];
      GetAsymmUnit().Axes().Value(2) = Toks[4];
      GetAsymmUnit().Angles().Value(0) = Toks[5];
      GetAsymmUnit().Angles().Value(1) = Toks[6];
      GetAsymmUnit().Angles().Value(2) = Toks[7];
      if( &res != NULL )  res.CellFound = true;
      GetAsymmUnit().InitMatrices();
    }
    else  
      throw TFunctionFailedException(__OlxSourceInfo, "invalid Cell instruction");
  }
  else if( Toks[0].Comparei("SYMM") == 0 && (Toks.Count() > 1))
    res.Symm.Add( Toks.Text(EmptyString, 1) );
  else if( Toks[0].Comparei("SFAC") == 0 )  {
    bool expandedSfacProcessed = false;
    if( Toks.Count() == 16 )  {  // a special case for expanded sfac
      int NumberCount = 0;
      for( int i=2; i < Toks.Count(); i++ )  {
        if( Toks[i].IsNumber() )
          NumberCount++;
      }
      if( NumberCount > 0 && NumberCount < 14 )  {
        TBasicApp::GetLog().Error( olxstr("Possibly not well formed SFAC ") << Toks[0]);
      }
      else  if( NumberCount == 14 )  {
        /* here we do not check if the Toks.String(1) is atom - itcould be a label ...
        so we keep it as it is to save in the ins file
        */
        Sfac << Toks[1] << ' ';
        res.BasicAtoms.Add( Toks[1], AtomsInfo->FindAtomInfoBySymbol(Toks[1]) );
        if( res.BasicAtoms.Last().Object() == NULL )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not find suitable scatterer for '") << Toks[1] << '\'' );
        expandedSfacProcessed = true;
        GetAsymmUnit().AddNewSfac( Toks.String(1),
          Toks[2].ToDouble(), Toks[3].ToDouble(), Toks[4].ToDouble(),
          Toks[5].ToDouble(), Toks[6].ToDouble(), Toks[7].ToDouble(),
          Toks[8].ToDouble(), Toks[9].ToDouble(), Toks[10].ToDouble() );
      }
    }
    if( !expandedSfacProcessed )  {
      for( int j=1; j < Toks.Count(); j++ )  {
        if( AtomsInfo->IsAtom(Toks[j]) )  {
          res.BasicAtoms.Add(Toks[j], AtomsInfo->FindAtomInfoBySymbol(Toks[j]) );
          if( res.BasicAtoms.Last().Object() == NULL )
            throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not find suitable scatterer for '") << Toks[j] << '\'' );
          Sfac << Toks[j] << ' ';
        }
      }
    }
    Sfac = Sfac.Trim(' ');
  }
  else if( Toks[0].Comparei("REM") == 0 )     {  
    if( Toks.Count() > 1 )  {
      if( Toks[1].Comparei("olex2.stop_parsing") == 0 )  {
        while( i < ins.Count() )  {
          Skipped.Add( ins[i] );
          if( ins[i].StartsFromi("REM") && ins[i].IndexOf("olex2.resume_parsing") != -1 ) 
            break;
          i++;
        }
      } 
      else if( Toks[1].StartsFromi("<HKL>") )  {
        FHKLSource = Toks.Text(' ', 1);
        int index = FHKLSource.FirstIndexOf('>');
        int iv = FHKLSource.IndexOf("</HKL>");
        if( iv == -1 )  {
          while( (i+1) < ins.Count() )  {
            i++;
            if( !ins[i].StartsFromi("rem") )  break;
            FHKLSource << ins[i].SubStringFrom(4);
            iv = FHKLSource.IndexOf("</HKL>");
            if( iv != -1 )  break;
          }
        }
        if( iv != -1 )  {
          FHKLSource = FHKLSource.SubString(index+1, iv-index-1);
          FHKLSource.Replace("%20", ' ');
        }
        else
          FHKLSource = EmptyString;
      }
    }
  }
  else
    return false;
  return true;
}
//..............................................................................
void TIns::UpdateParams()  {
  for( int i =0; i < Ins.Count(); i++ )  {
    for( int j=0; j < Ins.Object(i)->Count(); j++ )  {
      if( Ins.Object(i)->Object(j) != NULL )
        Ins.Object(i)->String(j) = Ins.Object(i)->Object(j)->GetLabel();
    }
  }
}
//..............................................................................
void TIns::DelIns(int i)  {
  delete Ins.Object(i);
  Ins.Delete(i);
}
//..............................................................................
TInsList* TIns::FindIns(const olxstr &Name)  {
  int i = Ins.CIIndexOf(Name);
  return i >= 0 ? Ins.Object(i) : NULL;
}
//..............................................................................
bool TIns::InsExists(const olxstr &Name)  {
  return FindIns(Name) != NULL;
}
//..............................................................................
void TIns::AddVar(float val)  {
  FVars.Resize(FVars.Count() + 1);
  FVars[FVars.Count()-1] = val;
}
//..............................................................................
bool TIns::AddIns(const TStrList& toks, bool CheckUniq)  {
  // special instructions
  if( _ParseIns(toks) )  return true;
  // check for uniqueness
  if( CheckUniq )  {
    for( int i=0; i < Ins.Count(); i++ )  {
      if( !Ins[i].Comparei(toks[0]) )  {
        TInsList *ps = Ins.Object(i);
        if( ps->Count() == (toks.Count()-1) )  {
          bool unique = false;
          for( int j=0; j < ps->Count(); j++ )  {
            if( ps->String(j).Comparei(toks[j+1]) != 0 )  {
              unique = true;
              break;
            }
          }
          if( !unique )  
            return false;
        }
      }
    }
  }
  //special treatment of the addins HFIX num atoms instruction
  if( toks[0].Comparei("HFIX") == 0 && toks.Count() > 2 )  {
    int hfix = toks[1].ToInt();
    for( int i=2; i < toks.Count(); i++ )  {
      TCAtom* ca = GetAsymmUnit().FindCAtom(toks[i]);
      if( ca != NULL )  ca->SetHfix(hfix);
    }
    return true;
  }
  TInsList& Params = *(new TInsList(toks.Count()-1));
  for( int i=1; i < toks.Count(); i++ )  {
    Params[i-1] = toks[i];
    Params.Object(i-1) = GetAsymmUnit().FindCAtom(toks[i]);
  }
  // end
  Ins.Add(toks[0], &Params);
  return true;
}
//..............................................................................
void TIns::HypernateIns(const olxstr &InsName, const olxstr &Ins, TStrList &Res)  {
  olxstr Tmp = Ins, Tmp1;
  int spindex;
  if( Tmp.Length() > 80 )  {
    while ( Tmp.Length() > 80 )  {
      spindex = Tmp.LastIndexOf(' ', 80-InsName.Length()-2);
      if( spindex > 0 )  {
        Tmp1 = Tmp.SubStringTo(spindex);
        Tmp1.Insert(InsName, 0);
        Res.Add(Tmp1);
        Tmp.Delete(0, spindex+1); // remove the space
      }
      else  {
        Tmp1 = Tmp.SubStringTo(80-InsName.Length()-2);
        Tmp1.Insert(InsName, 0);
        Res.Add(Tmp1);
        Tmp.Delete(0, 80-InsName.Length()-2);
      }
    }
    if( Tmp.Length() != 0 ) {
      Tmp.Insert(InsName, 0);
      Res.Add(Tmp);
    }
  }
  else  {
    Tmp.Insert(InsName, 0);
    Res.Add(Tmp);
  }
}
//..............................................................................
void TIns::HypernateIns(const olxstr& Ins, TStrList& Res)  {
  bool MultiLine = false, added = false;
  olxstr Tmp(Ins), Tmp1;
  while( Tmp.Length() > 79 )  {
    MultiLine = true;
    int spindex = Tmp.LastIndexOf(' ', 77); // for the right hypernation
    if( spindex > 0 )  {
      if( added )  Tmp1 = ' ';
      Tmp1 << Tmp.SubStringTo(spindex);
      if( Tmp1.Length() && Tmp1[Tmp1.Length()-1] != ' ')
        Tmp1 << ' ';
      Tmp1 << '=';
      Res.Add(Tmp1);
      Tmp.Delete(0, spindex+1); // remove the space
      added = true;
    }
    else  {
      Tmp1 = ' ';  // a space before each line
      Tmp1 << Tmp.SubStringTo(79);
      Res.Add(Tmp1);
      Tmp.Delete(0, 79);
    }
  }
  if( !Tmp.IsEmpty() )  {  // add the last bit
    if( MultiLine )
      Tmp.Insert(' ', 0);
    Res.Add(Tmp);
  }
}
//..............................................................................
void TIns::FixUnit()  {
  TStrPObjList<olxstr,TBasicAtomInfo*> BasicAtoms;
  Unit = EmptyString;
  Sfac = EmptyString;
  GetAsymmUnit().SummFormula(BasicAtoms, Sfac, Unit);
}
//..............................................................................
void TIns::SaveToRefine(const olxstr& FileName, const olxstr& sMethod, const olxstr& comments)  {
  TStrList SL, mtoks;
  TInsList* L;
  if( sMethod.IsEmpty() )
    mtoks.Add("TREF");
  else  {
    mtoks.Strtok(sMethod, "\\n");
    int spi = mtoks[0].IndexOf(' ');
    if( spi >= 0)
      SetSolutionMethod(mtoks[0].SubStringTo(spi));
    else
      SetSolutionMethod(mtoks[0]);
  }

  int UnitIndex, SfacIndex;
  olxstr Tmp, Tmp1;

  UpdateParams();
  SL.Add("TITL ") << GetTitle();

  if( !comments.IsEmpty() ) 
    SL.Add("REM ") << comments;
// try to estimate Z'
  TTypeList< AnAssociation2<int,TBasicAtomInfo*> > sl;
  TStrList sfac(GetSfac(), ' ');
  TStrList unit(GetUnit(), ' ');
  if( sfac.Count() != unit.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "SFAC does not match UNIT");
  int ac = 0;
  for( int i=0; i < sfac.Count(); i++ )  {
    int cnt = unit[i].ToInt();
    TBasicAtomInfo* bai = GetAtomsInfo().FindAtomInfoBySymbol(sfac[i]);
    if( *bai == iHydrogenIndex )  continue;
    sl.AddNew( cnt, bai );
    ac += cnt;
  }
  
  FAsymmUnit->SetZ( Round(FAsymmUnit->EstimateZ(ac/FAsymmUnit->GetZ())) );
//

  SL.Add( _CellToString() );
  SL.Add( _ZerrToString() );

  SL.Add("LATT ") << GetAsymmUnit().GetLatt();
  if( GetAsymmUnit().MatrixCount() == 1 )  {
    if( !GetAsymmUnit().GetMatrix(0).IsE() ) 
      SL.Add("SYMM ") << TSymmParser::MatrixToSymm( GetAsymmUnit().GetMatrix(0) );
  }
  else  {
    for( int i=0; i < GetAsymmUnit().MatrixCount(); i++ ) 
      SL.Add("SYMM ") << TSymmParser::MatrixToSymm( GetAsymmUnit().GetMatrix(i) );
  }
  SfacIndex = SL.Count();  SL.Add(EmptyString);
  UnitIndex = SL.Count();  SL.Add(EmptyString);

  for( int i=0; i < Ins.Count(); i++ )  {  // copy "unknown" instructions
    L = Ins.Object(i);
    if( Ins.String(i).Comparei("SIZE") == 0 || Ins.String(i).Comparei("TEMP") == 0 )  {
      Tmp = EmptyString;
      if( L->Count() != 0 )  Tmp << L->Text(' ');
      HypernateIns(Ins.String(i)+' ', Tmp, SL);
    }
  }

  _SaveHklSrc(SL);
  _SaveFVar(SL);

  SL.AddList(mtoks);
  SL.Add(EmptyString);
  SL.Add("HKLF ") << HKLF;
  SL.String(UnitIndex) = olxstr("UNIT ") << Unit;
  _SaveSfac( SL, SfacIndex );
  SL.Add("END");
#ifdef _UNICODE
  TCStrList(SL).SaveToFile(FileName);
#else
  SL.SaveToFile(FileName);
#endif
}
//..............................................................................
void TIns::_SaveSfac(TStrList& list, int pos)  {
  if( GetAsymmUnit().SfacCount() == 0 )
    list.String(pos) = olxstr("SFAC ") << Sfac;
  else  {
    TStrList toks(Sfac, ' '), lines;
    olxstr tmp, LeftOut;
    for( int i=0; i < toks.Count(); i++ )  {
      TLibScatterer* sd = GetAsymmUnit().FindSfacData( toks.String(i) );
      if( sd != NULL )  {
        tmp = "SFAC ";
        tmp << toks.String(i);
        for( int j=0; j < sd->Size(); j++ )
          tmp << ' ' << sd->GetData()[j];

        lines.Clear();
        HypernateIns(tmp, lines);
        for( int j=0; j < lines.Count(); j++ )  {
          list.Insert(pos, lines.String(j) );
          pos++;
        }
      }
      else  {
        LeftOut << ' ' << toks.String(i);
      }
    }
    if( LeftOut.Length() != 0 )  {
      list.Insert(pos, olxstr("SFAC") << LeftOut );
    }
  }
}
//..............................................................................
void TIns::SaveToStrings(TStrList& SL)  {
  int UnitIndex, SfacIndex;
  TVectorD QE;  // quadratic form of s thermal ellipsoid
  olxstr Tmp;
  TBasicAtomInfo *BAI;
  for( int i=-1; i < GetAsymmUnit().ResidueCount(); i++ )  {
    TAsymmUnit::TResidue& residue = GetAsymmUnit().GetResidue(i);
    for( int j=0; j < residue.Count(); j++ )  {
      if( residue[j].IsDeleted() )  continue;
      for( int k=j+1; k < residue.Count(); k++ )  {
        if( residue[k].IsDeleted() )  continue;
        if( residue[j].GetPart() != residue[k].GetPart() )  continue;
        if( residue[j].GetLabel().Comparei(residue[k].GetLabel()) == 0 ) 
          residue[k].Label() = GetAsymmUnit().CheckLabel(&residue[k], residue[k].GetLabel() );
      }
    }
  }

  TStrPObjList<olxstr,TBasicAtomInfo*> BasicAtoms(Sfac, ' ');
  for( int i=0; i < BasicAtoms.Count(); i++ )  {
    BAI = AtomsInfo->FindAtomInfoBySymbol( BasicAtoms.String(i) );
    if( BAI != NULL )  BasicAtoms.Object(i) = BAI;
    else
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Unknown element: ") << BasicAtoms.String(i));
  }

  UpdateParams();
  SaveHeader(SL, &SfacIndex, &UnitIndex);
  SL.Add(EmptyString);
  int afix = 0, part = 0, spindex, fragmentId = 0;
  int carbonBAIIndex = BasicAtoms.CIIndexOf('c');
  for( int i=-1; i < GetAsymmUnit().ResidueCount(); i++ )  {
    TAsymmUnit::TResidue& residue = GetAsymmUnit().GetResidue(i);
    if( i != -1 && !residue.IsEmpty() ) SL.Add( residue.ToString() );
    for( int j=0; j < residue.Count(); j++ )  {
      TCAtom& ac = residue[j];
      if( ac.IsDeleted() )  continue;
      if( ac.GetFragmentId() != fragmentId )  {
        SL.Add(EmptyString);
        fragmentId = ac.GetFragmentId();
      }
      if( ac.GetSameId() != -1 )  {
        TSimpleRestraint& sr = GetAsymmUnit().SimilarFragments()[ac.GetSameId()];
        Tmp = "SAME ";  Tmp << sr.GetEsd() << ' ' << sr.GetEsd1();
        for( int j=0; j < sr.AtomCount(); j++ )
          Tmp << ' ' << sr.GetAtom(j).GetFullLabel();
        HypernateIns( Tmp, SL );
      }
      if( ac.GetPart() != part )  SL.Add("PART ") << ac.GetPart();
      if( ac.GetAfix() != afix )  SL.Add("AFIX ") << ac.GetAfix();
      spindex = BasicAtoms.IndexOfObject( &ac.GetAtomInfo() );
      if( spindex == -1 )  {
        if( ac.GetAtomInfo() == iQPeakIndex )  {
          if( carbonBAIIndex == -1 )  {
            BasicAtoms.Add( "C", &AtomsInfo->GetAtomInfo(iCarbonIndex) );
            spindex = carbonBAIIndex = BasicAtoms.Count() -1;
            Unit << ' ' << '1';
          }
          spindex = carbonBAIIndex;
        }
        else  {
          BasicAtoms.Add( ac.GetAtomInfo().GetSymbol(), &ac.GetAtomInfo() );
          Unit << ' ' << '1';
          Sfac << ' ' << ac.GetAtomInfo().GetSymbol();
          spindex = BasicAtoms.Count() - 1;
        }
      }
      HypernateIns( _AtomToString(&ac, spindex+1), SL );
      afix = ac.GetAfix();
      part = ac.GetPart();
    }
  }
  SL.Add("HKLF ") << HKLF;
  SL.String(UnitIndex) = (olxstr("UNIT ") << Unit);
//  SL.String(SfacIndex) = (olxstr("SFAC ") += FSfac);
  _SaveSfac(SL, SfacIndex);
  SL.Add("END");
  SL.Add(EmptyString);
  // put all REMS
  for( int i=0; i < Ins.Count(); i++ )  {
    TInsList* L = Ins.Object(i);
    if( !Ins[i].StartsFrom("REM") )  continue;
    olxstr tmp = L->IsEmpty() ? EmptyString : L->Text(' ');
    HypernateIns(Ins.String(i)+' ', tmp, SL);
  }
}
//..............................................................................

void TIns::SetSfacUnit(const olxstr& su) {
  TTypeList<AnAssociation2<olxstr, int> > list;
  AtomsInfo->ParseElementString(su, list);
  Sfac = EmptyString;
  Unit = EmptyString;
  for( int i=0; i < list.Count(); i++ )  {
   Sfac << list[i].GetA();
   Unit << list[i].GetB()*GetAsymmUnit().GetZ();
   if( (i+1) < list.Count() )  {
     Sfac << ' ';
     Unit << ' ';
   }
  }
}
//..............................................................................
bool TIns::Adopt(TXFile *XF)  {
  Clear();
  GetAsymmUnit().Assign( XF->GetAsymmUnit() );
  GetAsymmUnit().SetZ( (short)XF->GetLattice().GetUnitCell().MatrixCount() );

  if( (XF->GetLastLoader() != NULL) )  {
    FTitle = XF->GetLastLoader()->GetTitle();
    SetHKLSource( XF->GetLastLoader()->GetHKLSource() );
    
    if( EsdlInstanceOf(*XF->GetLastLoader(), TP4PFile) )  {
      TP4PFile* p4p = (TP4PFile*)XF->GetLastLoader();
      Radiation = p4p->GetRadiation();
      TStrList lst;
      olxstr tmp = p4p->GetSize();  
      if( tmp.IsEmpty() )  {
        tmp.Replace('?', '0');
        lst.Add("SIZE");
        lst.Add( tmp );  AddIns(lst); lst.Clear();
      }
      tmp = p4p->GetTemp();
      if( !tmp.IsEmpty() )  {
        tmp.Replace('?', '0');
        lst.Add("TEMP");
        lst.Add( tmp );  AddIns(lst); lst.Clear();
      }
      if( p4p->GetChem() != "?" )  {
        try  {  SetSfacUnit( p4p->GetChem() );  }
        catch( TExceptionBase& )  {  }
      }
    }
    else if( EsdlInstanceOf(*XF->GetLastLoader(), TCRSFile) )  {
      TCRSFile* crs = (TCRSFile*)XF->GetLastLoader();
      Sfac = crs->GetSfac();
      Unit = crs->GetUnit();
    }
    else if( EsdlInstanceOf(*XF->GetLastLoader(), TCif) )  {
      TCif* cif = (TCif*)XF->GetLastLoader();
      olxstr chem = olxstr::DeleteChars( cif->GetSParam("_chemical_formula_sum"), ' ');
      olxstr strSg = cif->GetSParam("_symmetry_space_group_name_H-M");
      
      SetSfacUnit( chem );
      TSpaceGroup* sg = TSymmLib::GetInstance()->FindGroup( strSg );
      if( sg != NULL )
        GetAsymmUnit().ChangeSpaceGroup(*sg);
    }
  }
  if( RefinementMethod.IsEmpty() )
    RefinementMethod = "L.S.";
  return true;
}
//..............................................................................
void TIns::DeleteAtom(TCAtom *CA)  {
  for( int i =0; i < Ins.Count(); i++ )  {
    for( int j=0; j < Ins.Object(i)->Count(); j++ ) 
      if( Ins.Object(i)->Object(j) == CA )  Ins.Object(i)->Object(j) = NULL;
  }
}
//..............................................................................
void TIns::UpdateAtomsFromStrings(TCAtomPList& CAtoms, TStrList& SL, TStrList& Instructions) {
  TStrList Toks;
  olxstr Tmp, Tmp1;
  TCAtom *atom;
  int iv, Part=0, Afix=0, atomCount = 0;
  double partOccu = 0;
  TVectorD QE(6);  // quadratic form of ellipsoid
  TAsymmUnit::TResidue* resi = NULL;
  SL.CombineLines('=');
  FVars.Resize(0);
  for( int i=0; i < SL.Count(); i++ )  {
    Tmp = olxstr::DeleteSequencesOf<char>(SL[i].UpperCase(), true);
    if( Tmp.IsEmpty() )  continue;

    for( int j=0; j < Tmp.Length(); j++ )  {
      if( Tmp[j] == '!' )  {  // comment sign
        Tmp.SetLength(j-1);  break;
      }
    }
    Toks.Clear();
    Toks.Strtok(Tmp, ' ');
    if( Toks.IsEmpty() )  continue;
    Tmp1 = Toks.String(0);
    if( Tmp1 == "REM" )  continue;
    if( Tmp1 == "RESI" )  {
      if( Toks.Count() < 3 )
        throw TInvalidArgumentException(__OlxSourceInfo, "invalid number of arguments for a residue");
      resi = &CAtoms[0]->GetParent()->NewResidue(Toks[1], Toks[2].ToInt(), (Toks.Count() > 3) ? Toks[3] : EmptyString);
      continue;
    }
    if( Tmp1 == "PART" && (Toks.Count() > 1) )  {
      Part = (short)Toks[1].ToInt();
      if( Part == 0 )
        partOccu = 0;
      if( Toks.Count() == 3 )
        partOccu = Toks[2].ToDouble();
      continue;
    }
    if( Tmp1 == "AFIX"  && (Toks.Count() > 1) )  {
      Afix = (short)Toks[1].ToInt();  continue;
    }
    if( Toks.Count() < 6 )  {  // should be at least
      Instructions.Add(Tmp);  continue;
    }
    if( !AtomsInfo->IsAtom(Tmp1) )  {  // is a valid atom
      Instructions.Add(Tmp);  continue;
    }
    if( !Toks.String(1).IsNumber() )  {  // should be a number
      Instructions.Add(Tmp);  continue;
    }
    // should be four numbers
    if( (!Toks.String(2).IsNumber()) || (!Toks.String(3).IsNumber()) ||
        (!Toks.String(4).IsNumber()) || (!Toks.String(5).IsNumber()) )  {
      Instructions.Add(Tmp);  continue;
    }
    iv  = Toks[1].ToInt();
    if( iv != -1 ) // wrong index in SFAC, only -1 is supported
      throw TInvalidArgumentException(__OlxSourceInfo, "wrong SFAC index, only -1 is supported");

    if( (atomCount+1) > CAtoms.Count() )  {
      if( atom && atom->GetParent() )  {
        atom = &atom->GetParent()->NewAtom(resi);
        atom->SetLoaderId( liNewAtom );
      }
    }
    else  {
      atom = CAtoms[atomCount];
      if( resi != NULL )  resi->AddAtom(atom);
    }
    // clear fixed fixed values as they reread
    atom->FixedValues().Null();
    
    _ParseAtom( Toks, partOccu, resi, atom );
    atomCount++;
    atom->SetPart(Part);
    atom->SetAfix(Afix);
    atom->SetLabel( Tmp1 );
  }
  ParseRestraints(Instructions, CAtoms[0]->GetParent());
  Instructions.Pack();
}
//..............................................................................
bool TIns::SaveAtomsToStrings(const TCAtomPList& CAtoms, TStrList& SL, TSimpleRestraintPList* processed)  {
  if( CAtoms.IsEmpty() )  return false;
  short part = 0,
        afix = 0,
        resi = -2;
  SaveRestraints(SL, &CAtoms, processed, CAtoms[0]->GetParent());
  _SaveFVar(SL);
  for(int i=0; i < CAtoms.Count(); i++ )  {
    if( CAtoms[i]->GetResiId() != resi && CAtoms[i]->GetResiId() != -1 )  {
      resi = CAtoms[i]->GetResiId();
      SL.Add( GetAsymmUnit().GetResidue(CAtoms[i]->GetResiId()).ToString());
    }
    if( CAtoms[i]->GetSameId() != -1 )  {
      TSimpleRestraint& sr = CAtoms[i]->GetParent()->SimilarFragments()[CAtoms[i]->GetSameId()];
      olxstr& tmp = (SL.Add( "SAME ") << sr.GetEsd() << ' ' << sr.GetEsd1());
      for( int j=0; j < sr.AtomCount(); j++ )
        tmp << ' ' << sr.GetAtom(j).GetFullLabel();
    }
    if( CAtoms[i]->GetPart() != part )
      SL.Add( olxstr("PART ") << CAtoms[i]->GetPart());
    if( CAtoms[i]->GetAfix() != afix )
      SL.Add( olxstr("AFIX ") << CAtoms[i]->GetAfix());

    HypernateIns( _AtomToString( CAtoms[i], -1 ), SL );

    afix = CAtoms[i]->GetAfix();
    part = CAtoms[i]->GetPart();
  }
  return true;
}
//..............................................................................
void TIns::SavePattSolution(const olxstr& FileName, const TTypeList<TPattAtom>& atoms, const olxstr& comments )  {
  TStrPObjList<olxstr,TBasicAtomInfo*> BasicAtoms;
  TStrList SL;
  TInsList* L;
  TTypeList<int> Sfacs, Unit;
  double V;
  olxstr Tmp, Tmp1;
  TBasicAtomInfo *BAI;

  for( int i=0; i < atoms.Count(); i++ )  {
    BAI = AtomsInfo->FindAtomInfoEx( atoms[i].GetName() );
    if( BAI == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Unknown element: ") << atoms[i].GetName() );
    int index = BasicAtoms.IndexOf( BAI->GetSymbol() );
    if(  index == -1 )  {
      BasicAtoms.Add( BAI->GetSymbol(), BAI );
      Sfacs.AddACopy( BasicAtoms.Count() - 1 );
      Unit.AddACopy(1);
    }
    else  {
      Unit[index]++;
      Sfacs.AddACopy( index );
    }
  }
  Tmp = "TITL ";  Tmp << GetTitle();  SL.Add(Tmp);
  if( !comments.IsEmpty() )  {
    Tmp = "REM ";  Tmp << comments;  SL.Add(Tmp);
  }

  SL.Add( _CellToString() );
  SL.Add( _ZerrToString() );

  Tmp = "LATT "; Tmp << GetAsymmUnit().GetLatt(); SL.Add(Tmp);
  if( GetAsymmUnit().MatrixCount() == 1 )  {
    if( !GetAsymmUnit().GetMatrix(0).IsE() )  {
      Tmp = "SYMM ";
      Tmp << TSymmParser::MatrixToSymm( GetAsymmUnit().GetMatrix(0) );
      SL.Add(Tmp);
    }
  }
  else  {
    for( int i=0; i < GetAsymmUnit().MatrixCount(); i++ )  {
      Tmp = "SYMM ";
      Tmp << TSymmParser::MatrixToSymm( GetAsymmUnit().GetMatrix(i) );
      SL.Add(Tmp);
    }
  }
  Tmp = EmptyString;
  Tmp1 = "UNIT ";
  for( int i=0; i < BasicAtoms.Count(); i++ )  {
    Tmp  << BasicAtoms.String(i) << ' ';
    Tmp1 << Unit[i] << ' ';
  }
  olxstr lastSfac = Sfac;
  Sfac = Tmp;
  SL.Add(EmptyString);
  _SaveSfac(SL, SL.Count()-1);
  Sfac = lastSfac;

  SL.Add(Tmp1);
  
  _SaveRefMethod(SL);

  // copy "unknown" instructions except rems
  for( int i=0; i < Ins.Count(); i++ )  {
    L = Ins.Object(i);
    // skip rems and print them at the end
    if( Ins[i].StartsFrom("REM") )  continue;
    Tmp = EmptyString;
    if( L->Count() )  Tmp << L->Text(' ');
    HypernateIns(Ins[i]+' ', Tmp, SL);
  }

  _SaveHklSrc(SL);

  Tmp = "WGHT ";
  for( int i=0; i < FWght.Count(); i++ )  {
    Tmp << FWght[i];
    Tmp.Format(Tmp.Length()+1, true, ' ');
  }
  if( FWght.Count() == 0 )  Tmp << 1;
  SL.Add(Tmp);

  Tmp = "FVAR 1";

  SL.Add( EmptyString );
  for( int i=0; i < atoms.Count(); i++ )  {
    Tmp = atoms[i].GetName();
    Tmp.Format(6, true, ' ');
    Tmp << (Sfacs[i]+1);
    Tmp.Format(Tmp.Length()+4, true, ' ');
    for( int j=0; j < 3; j++ )
      Tmp << olxstr::FormatFloat(-5, atoms[i].GetCrd()[j] ) << ' ';
    V = atoms[i].GetOccup() + 10;
    Tmp << olxstr::FormatFloat(-5, V) << ' ';
    SL.Add(Tmp);
  }
  SL.Add(olxstr("HKLF ") << HKLF);
  SL.Add(EmptyString);
#ifdef _UNICODE
  TCStrList(SL).SaveToFile(FileName);
#else
  SL.SaveToFile(FileName);
#endif
}
//..............................................................................
TCAtom* TIns::_ParseAtom(TStrList& Toks, double partOccu, TAsymmUnit::TResidue* resi, TCAtom* atom)  {
  int iv;
  TVectorD QE(6);
  if( atom == NULL )  {
    atom = &GetAsymmUnit().NewAtom(resi);
    atom->SetLoaderId(GetAsymmUnit().AtomCount()-1);
  }
  atom->CCenter().Value(0) = Toks.String(2);
  atom->CCenter().Value(1) = Toks.String(3);
  atom->CCenter().Value(2) = Toks.String(4);
  for( int j=0; j < 3; j ++ )  {
    if( fabs(atom->CCenter().Value(j).GetV()) >= 5 )  {
      atom->CCenter().Value(j).V() -= 10;
      atom->FixedValues()[ TCAtom::CrdFixedValuesOffset + j ] = 10;
    }
  }
  // initialise uncertanties using average cell error
  atom->CCenter().Value(0).E() = (float)fabs(atom->CCenter()[0].GetV()*Error);
  atom->CCenter().Value(1).E() = (float)fabs(atom->CCenter()[1].GetV()*Error);
  atom->CCenter().Value(2).E() = (float)fabs(atom->CCenter()[2].GetV()*Error);
  if( partOccu != 0 )
    atom->SetOccp( partOccu );
  else
    atom->SetOccp(  Toks.String(5).ToDouble() );

  if( fabs(atom->GetOccp()) > 10 )  {  // a variable or fixed param
    iv = (int)(atom->GetOccp()/10); iv *= 10; // extract variable index
    atom->SetOccpVar( iv );
    atom->SetOccp( fabs(atom->GetOccp() - iv) );
    iv = (int)(abs(iv)/10);            // do not need to store the sign anymore
    if( (iv <= FVars.Count()) && iv > 1 )  {  // check if it is a free variable and not just equal to "1"
      if( atom->GetOccpVar() < 0 )  atom->SetOccp( 1 - FVars[iv-1] );
      else                          atom->SetOccp(FVars[iv-1]);
      // do not process - causes too many problems!!!
      if( partOccu != 0 )
        atom->SetOccpVar( partOccu );
      else
        atom->SetOccpVar(  Toks.String(5).ToDouble() );
    }
    if( iv != 1 )  // keep the value
      atom->SetOccpVar(  Toks.String(5).ToDouble() );
  }
  if( Toks.Count() == 12 )  {  // full ellipsoid
    atom->EllpE().Resize(6);
    QE[0] = Toks.String(6).ToDouble();
    QE[1] = Toks.String(7).ToDouble();
    QE[2] = Toks.String(8).ToDouble();
    QE[3] = Toks.String(9).ToDouble();
    QE[4] = Toks.String(10).ToDouble();
    QE[5] = Toks.String(11).ToDouble();

    for( int j=0; j < 6; j ++ )  {
      if( fabs(QE[j]) > 10 )  {
        iv = (int)QE[j]/10;
        iv *= 10;
        QE[j] -= iv;
        atom->FixedValues()[TCAtom::UisoFixedValuesOffset+j] = fabs((double)iv);
      }
    }
    GetAsymmUnit().UcifToUcart(QE);
    atom->UpdateEllp(QE);
    if( atom->GetEllipsoid()->IsNPD() )  {
      TBasicApp::GetLog().Info(olxstr("Not positevely defined: ") << atom->Label());
      atom->SetUiso( 0 );
    }
    else
      atom->SetUiso( (QE[0] +  QE[1] + QE[2])/3);
  }
  else  {
    if( Toks.Count() > 6 )  {
      atom->SetUiso( Toks.String(6).ToDouble() );
      if( fabs(atom->GetUiso()) > 10 )  {
        atom->SetUisoVar( atom->GetUiso() );
        iv = (int)atom->GetUiso()/10;
        if( (iv <= FVars.Count()) && iv > 1 )  {  // check if it is a free variable and not just equal to "1"
          if( atom->GetUiso() < 0 )
            atom->SetUiso( 1 - FVars[iv-1] );
          else
            atom->SetUiso( FVars[iv-1] );
        }
        else  {  // simply fixed value
          iv *= 10;
          atom->SetUiso( atom->GetUiso() - iv );
          atom->SetUisoVar( (double)iv );
        }
      }
    }
    else
      atom->SetUiso( 4*caDefIso*caDefIso );
    if( Toks.Count() >= 8 ) // some other data as Q-peak itensity
      atom->SetQPeak( Toks.String(7).ToDouble() );
    if( atom->GetUiso() < 0 )  {  // a value fixed to a bound atom value
      atom->SetUisoVar(atom->GetUiso());
      atom->SetUiso( 4*caDefIso*caDefIso );
    }
  }
  return atom;
}
//..............................................................................
olxstr TIns::_AtomToString(TCAtom* CA, int SfacIndex)  {
  double v;
  TVectorD QE(6);   // quadratic form of ellipsoid
  olxstr Tmp = CA->Label();
  Tmp.Format(6, true, ' ');
  Tmp << SfacIndex;
  Tmp.Format(Tmp.Length()+4, true, ' ');
  for( int j=0; j < 3; j++ )  {
    v = CA->CCenter()[j].GetV();
    v += CA->FixedValues()[TCAtom::CrdFixedValuesOffset + j];
    Tmp << olxstr::FormatFloat(-5, v ) << ' ';
  }
  // save occupancy
  if( CA->GetOccpVar() != 0 && CA->GetOccpVar() != 10 )  v = CA->GetOccpVar();
  else                                                   v = CA->GetOccpVar() + CA->GetOccp();
  Tmp << olxstr::FormatFloat(-5, v) << ' ';
  // save Uiso, Uanis
  if( CA->GetEllipsoid() != NULL )  {
    CA->GetEllipsoid()->GetQuad(QE);
    GetAsymmUnit().UcartToUcif(QE);

    for( int j = 0; j < 6; j++ )  {
      v = QE[j];
      v += (CA->FixedValues()[TCAtom::UisoFixedValuesOffset+j]*Sign(v));
      Tmp << olxstr::FormatFloat(-5, v ) << ' ';
    }
  }
  else  {
    if( CA->GetUisoVar() )  // riding atom
      v = CA->GetUisoVar();
    else  {
      v = CA->GetUiso();
      //v += CA->GetUisoVar() * Sign(v);
    }
    Tmp << olxstr::FormatFloat(-5, v) << ' ';
  }
  // Q-Peak
  if( CA->GetAtomInfo() == iQPeakIndex )
    Tmp << olxstr::FormatFloat(-3, CA->GetQPeak());
  return Tmp;
}
//..............................................................................
olxstr TIns::_CellToString()  {
  olxstr Tmp("CELL ");
  Tmp << GetRadiation();
  Tmp << ' ' << GetAsymmUnit().Axes()[0].GetV() <<
         ' ' << GetAsymmUnit().Axes()[1].GetV() <<
         ' ' << GetAsymmUnit().Axes()[2].GetV() <<
         ' ' << GetAsymmUnit().Angles()[0].GetV() <<
         ' ' << GetAsymmUnit().Angles()[1].GetV() <<
         ' ' << GetAsymmUnit().Angles()[2].GetV();
  return Tmp;
}
//..............................................................................
void TIns::_SaveFVar(TStrList& SL)  {
  olxstr Tmp; // = "FVAR ";
  for( int i=0; i < FVars.Count(); i++ )  {
    Tmp << FVars[i];
    Tmp.Format(Tmp.Length()+1, true, ' ');
  }
  if( FVars.Count() == 0 ) Tmp << 1;
  HypernateIns("FVAR ", Tmp, SL);
}
//..............................................................................
olxstr TIns::_ZerrToString()  {
  olxstr Tmp("ZERR ");
  Tmp << GetAsymmUnit().GetZ();
  Tmp << ' ' << GetAsymmUnit().Axes()[0].GetE() <<
         ' ' << GetAsymmUnit().Axes()[1].GetE() <<
         ' ' << GetAsymmUnit().Axes()[2].GetE() <<
         ' ' << GetAsymmUnit().Angles()[0].GetE() <<
         ' ' << GetAsymmUnit().Angles()[1].GetE() <<
         ' ' << GetAsymmUnit().Angles()[2].GetE();
  return Tmp;
}
//..............................................................................
void TIns::_SaveRefMethod(TStrList& SL)  {
  if( !GetRefinementMethod().IsEmpty() )  {
    if( FLS.Count() != 0 )  {
      olxstr& rm = SL.Add( GetRefinementMethod() );
      for( int i=0; i < FLS.Count(); i++ )
        rm << ' ' << FLS[i];
    }
    if( FPLAN.Count() != 0 )  {
      olxstr& pn = SL.Add("PLAN ");
      for( int i=0; i < FPLAN.Count(); i++ )
        pn << ' ' << ((i < 1) ? Round(FPLAN[i]) : FPLAN[i]);
    }
  }
}
//..............................................................................
void TIns::_SaveHklSrc(TStrList& SL)  {
  if( FHKLSource.Length() != 0 )  {  // update html source string
    olxstr Tmp( FHKLSource );
    Tmp.Replace(' ', "%20");
    HypernateIns("REM ", olxstr("<HKL>") << Tmp << "</HKL>", SL);
  }
}
//..............................................................................
bool ProcessRestraint(const TCAtomPList* atoms, TSimpleRestraint& sr)  {
  if( sr.AtomCount() == 0 && !sr.IsAllNonHAtoms() )  return false;
  if( atoms == NULL )  return true;
  for(int i=0; i < atoms->Count(); i++ )
    if( sr.ContainsAtom( atoms->Item(i) ) )  return true;
  return false;
}

void StoreUsedSymIndex(TIntList& il, const TMatrixD* m, TAsymmUnit* au)  {
  if( m == NULL )  return;

  int ind = au->UsedSymmIndex( *m );
  if( il.IndexOf(ind) == -1 )
    il.Add(ind);
}

void TIns::SaveRestraints(TStrList& SL, const TCAtomPList* atoms,
                          TSimpleRestraintPList* processed, TAsymmUnit* au)  {
  if( au == NULL )
    au = &GetAsymmUnit();

  int oindex = SL.Count();

  olxstr Tmp;
  TIntList usedSymm;

  // fixed distances
  for( int i=0; i < au->RestrainedDistances().Count(); i++ )  {
    TSimpleRestraint& sr = au->RestrainedDistances()[i];
    sr.Validate();
    if( !ProcessRestraint(atoms, sr) )  continue;
    Tmp = "DFIX ";
    Tmp << sr.GetValue() << ' ' << sr.GetEsd();
    for( int j=0; j < sr.AtomCount(); j++ )  {
     Tmp << ' ' << sr.GetAtom(j).GetFullLabel();
     StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), au);
    }
    HypernateIns(Tmp, SL);
    if( processed != 0 )  processed->Add( &sr );
  }
  // similar distances
  for( int i=0; i < au->SimilarDistances().Count(); i++ )  {
    TSimpleRestraint& sr = au->SimilarDistances()[i];
    sr.Validate();
    if( !ProcessRestraint(atoms, sr) )  continue;
    Tmp = "SADI ";
    Tmp << sr.GetEsd();
    for( int j=0; j < sr.AtomCount(); j++ )  {
     Tmp << ' ' << sr.GetAtom(j).GetFullLabel();
     StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), au);
    }
    HypernateIns(Tmp, SL);
    if( processed != 0 )  processed->Add( &sr );
  }
  // fixed "angles"
  for( int i=0; i < au->RestrainedAngles().Count(); i++ )  {
    TSimpleRestraint& sr = au->RestrainedAngles()[i];
    sr.Validate();
    if( !ProcessRestraint(atoms, sr) )  continue;
    Tmp = "DANG ";
    Tmp << olxstr::FormatFloat(3, sr.GetValue()) << ' ' << sr.GetEsd();
    for( int j=0; j < sr.AtomCount(); j++ )  {
     Tmp << ' ' << sr.GetAtom(j).GetFullLabel();
     StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), au);
    }
    HypernateIns(Tmp, SL);
    if( processed != 0 )  processed->Add( &sr );
  }
  // fixed chiral atomic volumes
  for( int i=0; i < au->RestrainedVolumes().Count(); i++ )  {
    TSimpleRestraint& sr = au->RestrainedVolumes()[i];
    sr.Validate();
    if( !ProcessRestraint(atoms, sr) )  continue;
    Tmp = "CHIV ";
    Tmp << sr.GetValue() << ' ' << sr.GetEsd();
    for( int j=0; j < sr.AtomCount(); j++ )  {
     Tmp << ' ' << sr.GetAtom(j).GetFullLabel();
     StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), au);
    }
    HypernateIns(Tmp, SL);
    if( processed != 0 )  processed->Add( &sr );
  }
  // planar groups
  for( int i=0; i < au->RestrainedPlanarity().Count(); i++ )  {
    TSimpleRestraint& sr = au->RestrainedPlanarity()[i];
    sr.Validate();
    if( !ProcessRestraint(atoms, sr) )  continue;
    if( sr.AtomCount() < 4 )  continue;
    Tmp = "FLAT ";
    Tmp << sr.GetEsd();
    for( int j=0; j < sr.AtomCount(); j++ )  {
     Tmp << ' ' << sr.GetAtom(j).GetFullLabel();
     StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), au);
    }
    HypernateIns(Tmp, SL);
    if( processed != 0 )  processed->Add( &sr );
  }
  // rigid bond restraint
  for( int i=0; i < au->RigidBonds().Count(); i++ )  {
    TSimpleRestraint& sr = au->RigidBonds()[i];
    sr.Validate();
    if( !ProcessRestraint(atoms, sr) )  continue;
    Tmp = "DELU ";
    Tmp << sr.GetEsd() << ' ' << sr.GetEsd1();
    for( int j=0; j < sr.AtomCount(); j++ )  {
     Tmp << ' ' << sr.GetAtom(j).GetFullLabel();
     StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), au);
    }
    HypernateIns(Tmp, SL);
    if( processed != 0 )  processed->Add( &sr );
  }
  // similar U restraint
  for( int i=0; i < au->SimilarU().Count(); i++ )  {
    TSimpleRestraint& sr = au->SimilarU()[i];
    sr.Validate();
    if( !ProcessRestraint(atoms, sr) )  continue;
    Tmp = "SIMU ";
    Tmp << sr.GetEsd() << ' ' << sr.GetEsd1() << ' ' << sr.GetValue();
    for( int j=0; j < sr.AtomCount(); j++ )  {
     Tmp << ' ' << sr.GetAtom(j).GetFullLabel();
     StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), au);
    }
    HypernateIns(Tmp, SL);
    if( processed != 0 )  processed->Add( &sr );
  }
  // Uanis restraint to behave like Uiso
  for( int i=0; i < au->RestranedUaAsUi().Count(); i++ )  {
    TSimpleRestraint& sr = au->RestranedUaAsUi()[i];
    sr.Validate();
    if( !ProcessRestraint(atoms, sr) )  continue;
    Tmp = "ISOR ";
    Tmp << sr.GetEsd() << ' ' << sr.GetEsd1();
    for( int j=0; j < sr.AtomCount(); j++ )  {
     Tmp << ' ' << sr.GetAtom(j).GetFullLabel();
     StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), au);
    }
    HypernateIns(Tmp, SL);
    if( processed != 0 )  processed->Add( &sr );
  }
  // equivalent EADP constraint
  for( int i=0; i < au->EquivalentU().Count(); i++ )  {
    TSimpleRestraint& sr = au->EquivalentU()[i];
    sr.Validate();
    if( !ProcessRestraint(atoms, sr) )  continue;
    if( sr.AtomCount() < 2 )  continue;
    Tmp = "EADP";
    for( int j=0; j < sr.AtomCount(); j++ )  {
     Tmp << ' ' << sr.GetAtom(j).GetFullLabel();
     StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), au);
    }
    HypernateIns(Tmp, SL);
    if( processed != 0 )  processed->Add( &sr );
  }
  // store the rest of eqiv ...
  for( int i=0; i < au->UsedSymmCount(); i++ )
    StoreUsedSymIndex( usedSymm, &au->GetUsedSymm(i), au);
  // save
  for( int i=0; i < usedSymm.Count(); i++ )  {
    Tmp = "EQIV ";
    Tmp << '$' << (i+1) << ' ' << TSymmParser::MatrixToSymm( au->GetUsedSymm(usedSymm[i]) );
    SL.Insert(oindex+i, Tmp  );
  }
}
//..............................................................................
void TIns::ClearIns()  {
  for( int i=0; i < Ins.Count(); i++ )
    delete Ins.Object(i);
  Ins.Clear();
}
//..............................................................................
bool TIns::AddIns(const olxstr& Params)  {
  TStrList toks(Params, ' ');
  return AddIns(toks);
}
//..............................................................................
void TIns::SaveHeader(TStrList& SL, int* SfacIndex, int* UnitIndex)  {
  SL.Add("TITL ") << GetTitle();

  SL.Add( _CellToString() );
  SL.Add( _ZerrToString() );

  SL.Add("LATT ") << GetAsymmUnit().GetLatt();
  if( GetAsymmUnit().MatrixCount() == 1 )  {
    if( !GetAsymmUnit().GetMatrix(0).IsE() )
      SL.Add("SYMM ") << TSymmParser::MatrixToSymm( GetAsymmUnit().GetMatrix(0) );
  }
  else  {
    for( int i=0; i < GetAsymmUnit().MatrixCount(); i++ )
      SL.Add("SYMM ") << TSymmParser::MatrixToSymm( GetAsymmUnit().GetMatrix(i) );
  }
  if( SfacIndex != NULL )  *SfacIndex = SL.Count();  
  SL.Add("SFAC ") << Sfac;
  if( UnitIndex != NULL )  *UnitIndex = SL.Count();  
  SL.Add("UNIT ") << Unit;
  for( int i=0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& ca = GetAsymmUnit().GetAtom(i);
    if( ca.IsDeleted() )  continue;
    if( ca.GetHfix() > 0 )
      SL.Add("HFIX ") << ca.GetHfix() << ' ' << ca.GetLabel();
  }
  SaveRestraints(SL, NULL, NULL, NULL);
  _SaveRefMethod(SL);

  // copy "unknown" instructions except rems
  for( int i=0; i < Ins.Count(); i++ )  {
    TInsList* L = Ins.Object(i);
    // skip rems and print them at the end
    if( Ins[i].StartsFrom("REM") )  continue;
    olxstr tmp = L->IsEmpty() ? EmptyString : L->Text(' ');
    HypernateIns(Ins[i]+' ', tmp , SL);
  }
  SL << Skipped;
//  for( int i=0; i < Skipepd.Count(); i++ )

  _SaveHklSrc(SL);

  olxstr& wght = SL.Add("WGHT ");
  for( int i=0; i < FWght.Count(); i++ )  {
    wght << FWght[i];
    if( i+1 < FWght.Count() )
      wght << ' ';
  }
  if( FWght.Count() == 0 )  wght << "0.1";
  _SaveFVar(SL);
}
//..............................................................................
void TIns::ParseHeader(const TStrList& in)  {
  // clear all but the atoms
  for( int i=0; i < Ins.Count(); i++ )
    delete Ins.Object(i);
  Ins.Clear();
  Skipped.Clear();
  FTitle = EmptyString;
  FVars.Resize(0);
  FWght.Resize(0);
  FWght1.Resize(0);
  FHKLSource = EmptyString;
  FLS.Resize(0);
  FPLAN.Resize(0);
  Sfac = EmptyString;
  Unit = EmptyString;
  Error = 0;
  GetAsymmUnit().ClearRestraints();
  GetAsymmUnit().ClearMatrices();
// end clear, start parsing
  olxstr Tmp;
  TStrList toks;
  ParseResults pr;
  for( int i=0; i < in.Count(); i++ )  {
    Tmp = olxstr::DeleteSequencesOf<char>(in[i], ' ');
    if( Tmp.IsEmpty() )      continue;

    while( Tmp[Tmp.Length()-1] == '=' )   {  // hypernation sign
      Tmp.SetLength(Tmp.Length()-1);
      // this will remove empty lines (in case of linux/dos mixup
      while( i++ && (i+1) < in.Count() && in[i].IsEmpty() )
        ;
      Tmp << in[i];
    }
    for( int j=0; j < Tmp.Length(); j++ )  {
      if( Tmp[j] == '!' )  {  // comment sign
        Tmp.SetLength(j-1);
        break;
      }
    }
    toks.Clear();
    toks.Strtok(Tmp, ' ');
    if( toks.IsEmpty() )  continue;

    if( ParseIns(in, toks, pr, i) )
      continue;
    else
      Ins.Add(in[i]);
  }
  TMatrixD sm(3,4);
  for( int i=0; i < pr.Symm.Count(); i++ )  {
    if( TSymmParser::SymmToMatrix(pr.Symm[i], sm) )
      GetAsymmUnit().AddMatrix(sm);
  }
  Ins.Pack();
  ParseRestraints(Ins, NULL);
  Ins.Pack();
  _FinishParsing();
}
//..............................................................................

