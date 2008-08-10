//---------------------------------------------------------------------------//
// namespace TXFiles: XShelxIns - basic procedures for the SHELX instruction files
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdlib.h>

#include "xins.h"

#include "bapp.h"
#include "log.h"

#include "unitcell.h"
#include "symmparser.h"

#include "efile.h"
#include "symmlib.h"
#include "typelist.h"
#include "egc.h"

#undef AddAtom
#undef GetObject
#undef Object

//----------------------------------------------------------------------------//
// XShelxIns function bodies
//----------------------------------------------------------------------------//
XShelxIns::XShelxIns(XModel& _xm) : xm(_xm) {
  HKLF = 4;
  LoadQPeaks = true;
}
//..............................................................................
XShelxIns::~XShelxIns()  {
  Clear();
}
//..............................................................................
void XShelxIns::Clear()  {
  Skipped.Clear();
  Title = EmptyString;
  FWght1.Resize(0);
  HklSrc = EmptyString;
  FLS.Resize(0);
  FPLAN.Resize(0);
  Sfac = EmptyString;
  Unit = EmptyString;
  Error = 0;
  R1 = -1;
}
//..............................................................................
void XShelxIns::LoadFromStrings(const TStrList& FileContent)  {
  Clear();
  TAtomsInfo* AtomsInfo = TAtomsInfo::GetInstance();
  ParseContext cx;
  TStrList Toks, InsFile(FileContent), ins;
  InsFile.CombineLines('=');
  bool   End = false;// true if END instruction reached
  short   SameAtomsLeft=0, SameId=-1;    // atom's PART
  double partOccu = 0;
  cx.Resi = &xm.Residues[0];
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
      ins.Add(InsFile[i]);
    else if( ParseIns(InsFile, Toks, cx, i) )
      continue;
    else if( Toks[0].StartsFromi("SAME") )  {
      cx.SameIns = Toks.Text(' ', 1);
    }
    else if( Toks[0].Comparei("END") == 0 )     {   //reset RESI to default
      End = true;  
      cx.Resi = &xm.Residues[0];
    }
    else if( Toks.Count() < 6 )  // atom sgould have at least 7 parameters
      ins.Add(InsFile[i]);
    else {
      if( olxstr::o_toupper(Toks[0].CharAt(0)) == 'Q' && !End )  {
        if( !LoadQPeaks  )  continue;
      }
      if( End && (olxstr::o_toupper(Toks[0].CharAt(0)) != 'Q') )  continue;
    // is a valid atom
      if( !AtomsInfo->IsAtom(Toks[0]))  {  ins.Add(InsFile[i]);  continue;  }
      if( !Toks[1].IsNumber() )         {  ins.Add(InsFile[i]);  continue;  }
      int index  = Toks[1].ToInt();
      if( index < 1 || index > cx.BasicAtoms.Count() )  {  // wrong index in SFAC
        ins.Add(InsFile[i]);
        continue;
      }
      // should be four numbers
      if( (!Toks[2].IsNumber()) || (!Toks[3].IsNumber()) ||
        (!Toks[4].IsNumber()) || (!Toks[5].IsNumber()) )  {
          ins.Add(InsFile[i]);
          continue;
      }
      if( !cx.CellFound )  {
        Clear();
        throw TFunctionFailedException(__OlxSourceInfo, "uninitialised cell");
      }
      XScatterer* sc = _ParseAtom(Toks, cx );
      if( !cx.SameIns.IsEmpty() )  {
        cx.Same.AddNew(sc, cx.SameIns);
        cx.SameIns = EmptyString;
      }
    }
  }
  if( GetSfac().CharCount(' ') != GetUnit().CharCount(' ') )  {
    Clear();
    throw TFunctionFailedException(__OlxSourceInfo, "mismatching SFAC/UNIT");
  }
  smatd sm;
  for( int i=0; i < cx.Symm.Count(); i++ )  {
    if( TSymmParser::SymmToMatrix(cx.Symm[i], sm) )
      xm.Cell.symm.AddCCopy(sm);
  }
  // remove dublicated instructtions, rems etc
  for( int i = 0; i < ins.Count(); i++ )  {
    if( ins[i].IsEmpty() )  continue;
    for( int j = i+1; j < ins.Count(); j++ )  {
      if( ins[i] == ins[j] )
        ins[j] = EmptyString;
    }
  }

  ins.Pack();
  ParseRestraints(ins, cx);
  ins.Pack();
  _FinishParsing();
  // initialise asu data
  if( !cx.CellFound )  {  // in case there are no atoms
    Clear();
    throw TInvalidArgumentException(__OlxSourceInfo, "empty CELL");
  }
  if( SameAtomsLeft != 0 ) 
    throw TFunctionFailedException(__OlxSourceInfo, "incomplete SAME instruction");
}
//..............................................................................
void XShelxIns::_FinishParsing()  {
  //for( int i =0; i < Ins.Count(); i++ )  {
  //  XShelxInsList* Param = new XShelxInsList(Ins[i], ' ');
  //  Ins.Object(i) = Param;
  //  Ins[i] = Param->String(0);
  //  // special treatment of HFIX instructions
  //  if( !Param->String(0).Comparei("HFIX") && Param->Count() > 2 ) {
  //    int iv = Param->String(1).ToInt();
  //    if( iv > 0 )  {
  //      for( int j=2; j<Param->Count(); j++ )  {
  //        TCAtom* atom = GetAsymmUnit().FindCAtom(Param->String(j));
  //        if( atom != NULL )
  //          atom->SetHfix(iv);
  //      }
  //    }
  //    continue;
  //  }
  //  //end
  //  Param->Delete(0);
  //  for( int j=0; j < Param->Count(); j++ )
  //    Param->Object(j) = GetAsymmUnit().FindCAtom(Param->String(j));
  //}
}
//..............................................................................
bool XShelxIns::ParseIns(const TStrList& ins, const TStrList& Toks, ParseContext& cx, int& i)  {
  if( _ParseIns(Toks) )
    return true;
  else if( !cx.CellFound && Toks[0].Comparei("CELL") == 0 )  {
    if( Toks.Count() == 8 )  {
      xm.Cell.Init(Toks[2].ToDouble(), Toks[3].ToDouble(), Toks[4].ToDouble(),
                      Toks[5].ToDouble(),Toks[6].ToDouble(),Toks[7].ToDouble());
      xm.WaveLength = Toks[1].ToDouble();
      cx.CellFound = true;
    }
    else  
      throw TFunctionFailedException(__OlxSourceInfo, "invalid Cell instruction");
  }
  else if( Toks[0].Comparei("SYMM") == 0 && (Toks.Count() > 1))
    cx.Symm.Add( Toks.Text(EmptyString, 1) );
  else if( Toks[0].Comparei("PART") == 0 && (Toks.Count() > 1) )  {
    cx.Part = (short)Toks[1].ToInt();
    if( cx.Part == 0 )  cx.PartOccu = 0;
    if( Toks.Count() == 3 )
      cx.PartOccu = Toks[2].ToDouble();
  }
  else if( Toks[0].Comparei("AFIX") == 0 && (Toks.Count() > 1) )  {
    cx.Afix = (short)Toks[1].ToInt();
  }
  else if( Toks[0].Comparei("RESI") == 0 )  {
    if( Toks.Count() < 3 )
      throw TInvalidArgumentException(__OlxSourceInfo, "wrong number of arguments for a residue");
    if( Toks[1].IsNumber() )
      cx.Resi = &xm.NewResidue(EmptyString, Toks[1].ToInt(), (Toks.Count() > 2) ? Toks[2] : EmptyString);
    else
      cx.Resi = &xm.NewResidue(Toks[1], Toks[2].ToInt(), (Toks.Count() > 3) ? Toks[3] : EmptyString);
  }
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
        cx.BasicAtoms.Add( Toks[1], XElementLib::FindBySymbol(Toks[1]) );
        if( cx.BasicAtoms.Last().Object() == NULL )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not find suitable scatterer for '") << Toks[1] << '\'' );
        expandedSfacProcessed = true;
        xm.NewScattererData( Toks[0],
          Toks[2].ToDouble(), Toks[3].ToDouble(), Toks[4].ToDouble(),
          Toks[5].ToDouble(), Toks[6].ToDouble(), Toks[7].ToDouble(),
          Toks[8].ToDouble(), Toks[9].ToDouble(), Toks[10].ToDouble(), 
          Toks[11].ToDouble(), Toks[12].ToDouble(), Toks[13].ToDouble(),
          Toks[14].ToDouble(), Toks[15].ToDouble());
      }
    }
    if( !expandedSfacProcessed )  {
      for( int j=1; j < Toks.Count(); j++ )  {
        if( XElementLib::IsElement(Toks[j]) )  {
          cx.BasicAtoms.Add(Toks[j], XElementLib::FindBySymbolEx(Toks[j]) );
          if( cx.BasicAtoms.Last().Object() == NULL )
            throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not find suitable scatterer for '") << Toks[j] << '\'' );
          Sfac << Toks[j] << ' ';
        }
      }
    }
    Sfac = Sfac.Trim(' ');
  }
  else if( Toks[0].Comparei("REM") == 0 )     {  
    if( Toks.Count() > 1 )  {
      if( Toks[1].Comparei("R1") == 0 && Toks.Count() > 4 && Toks[3].IsNumber() )  {
        R1 = Toks[3].ToDouble();
      }
      else if( Toks[1].Comparei("olex2.stop_parsing") == 0 )  {
        while( i < ins.Count() )  {
          Skipped.Add( ins[i] );
          if( ins[i].StartsFromi("REM") && ins[i].IndexOf("olex2.resume_parsing") != -1 ) 
            break;
          i++;
        }
      } 
      else if( Toks[1].StartsFromi("<HKL>") )  {
        HklSrc = Toks.Text(' ', 1);
        int index = HklSrc.FirstIndexOf('>');
        int iv = HklSrc.IndexOf("</HKL>");
        if( iv == -1 )  {
          while( (i+1) < ins.Count() )  {
            i++;
            if( !ins[i].StartsFromi("rem") )  break;
            HklSrc << ins[i].SubStringFrom(4);
            iv = HklSrc.IndexOf("</HKL>");
            if( iv != -1 )  break;
          }
        }
        if( iv != -1 )  {
          HklSrc = HklSrc.SubString(index+1, iv-index-1);
          HklSrc.Replace("%20", ' ');
        }
        else
          HklSrc = EmptyString;
      }
    }
  }
  else
    return false;
  return true;
}
//..............................................................................
void XShelxIns::UpdateParams()  {
  //for( int i =0; i < ins.Count(); i++ )  {
  //  for( int j=0; j < ins.Object(i)->Count(); j++ )  {
  //    if( ins.Object(i)->Object(j) != NULL )
  //      Ins.Object(i)->String(j) = Ins.Object(i)->Object(j)->GetLabel();
  //  }
  //}
}
//..............................................................................
//void XShelxIns::DelIns(int i)  {
//  delete ins.Object(i);
//  ins.Delete(i);
//}
//..............................................................................
//XShelxInsList* XShelxIns::FindIns(const olxstr &Name)  {
//  int i = Ins.CIIndexOf(Name);
//  return i >= 0 ? Ins.Object(i) : NULL;
//}
////..............................................................................
//bool XShelxIns::InsExists(const olxstr &Name)  {
//  return FindIns(Name) != NULL;
//}
////..............................................................................
//void XShelxIns::AddVar(float val)  {
//  FVars.Resize(FVars.Count() + 1);
//  FVars[FVars.Count()-1] = val;
//}
//..............................................................................
bool XShelxIns::AddIns(const TStrList& toks)  {
  // special instructions
  return _ParseIns(toks);
}
//..............................................................................
void XShelxIns::HypernateIns(const olxstr &InsName, const olxstr &Ins, TStrList &Res)  {
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
void XShelxIns::HypernateIns(const olxstr& Ins, TStrList& Res)  {
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
//void XShelxIns::FixUnit()  {
//  TStrPObjList<olxstr,TBasicAtomInfo*> BasicAtoms;
//  Unit = EmptyString;
//  Sfac = EmptyString;
//  GetAsymmUnit().SummFormula(BasicAtoms, Sfac, Unit);
//}
//..............................................................................
void XShelxIns::SaveForSolve(const olxstr& FileName, const olxstr& sMethod, const olxstr& comments)  {
  TStrList SL, mtoks;
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
  SL.Add("TITL ") << Title;

  if( !comments.IsEmpty() ) 
    SL.Add("REM ") << comments;
// try to estimate Z'
  TTypeList< AnAssociation2<int,cm_Element*> > sl;
  TStrList sfac(GetSfac(), ' ');
  TStrList unit(GetUnit(), ' ');
  if( sfac.Count() != unit.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "SFAC does not match UNIT");
  int ac = 0;
  for( int i=0; i < sfac.Count(); i++ )  {
    int cnt = unit[i].ToInt();
    cm_Element* elm = XElementLib::FindBySymbol(sfac[i]);
    if( *elm == iHydrogenZ )  continue;
    sl.AddNew( cnt, elm );
    ac += cnt;
  }
  
  //FAsymmUnit->SetZ( Round(FAsymmUnit->EstimateZ(ac/FAsymmUnit->GetZ())) );

  SL.Add( _CellToString() );
  SL.Add( _ZerrToString() );
  _SaveSymm(SL);
  SfacIndex = SL.Count();  SL.Add(EmptyString);
  UnitIndex = SL.Count();  SL.Add(EmptyString);
  SL.Add("TEMP ") << xm.Temperature;
  SL.Add("SIZE ") << xm.Size[0] << ' ' << xm.Size[1] << ' ' << xm.Size[2];
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
void XShelxIns::_SaveSfac(TStrList& list, int pos)  {
  if( xm.Sfac.Count() == 0 )
    list[pos] = olxstr("SFAC ") << Sfac;
  else  {
    TStrList toks(Sfac, ' '), lines;
    olxstr tmp, LeftOut;
    for( int i=0; i < toks.Count(); i++ )  {
      XScattererData* sd = xm.FindScattererData( toks[i] );
      if( sd->source == NULL )  {
        tmp = "SFAC ";
        tmp << toks[i];
        for( int j=0; j < 4; j++ )
          tmp << ' ' << sd->gaussians[j] << ' ' << sd->gaussians[j+4];
        tmp << ' ' << sd->gaussians[8] << ' ' << sd->fp << ' ' << sd->fdp  <<
          ' '  << sd->mu << ' ' << sd->r << ' ' << sd->wt; 
        lines.Clear();
        HypernateIns(tmp, lines);
        for( int j=0; j < lines.Count(); j++ )  {
          list.Insert(pos, lines[j] );
          pos++;
        }
      }
      else  {
        LeftOut << ' ' << toks[i];
      }
    }
    if( !LeftOut.IsEmpty() )
      list.Insert(pos, olxstr("SFAC") << LeftOut );
  }
}
//..............................................................................
void XShelxIns::SaveToStrings(TStrList& SL)  {
  int UnitIndex, SfacIndex;
  evecd QE;  // quadratic form of s thermal ellipsoid
  olxstr Tmp;
  TBasicAtomInfo *BAI;
  for( int i=0; i < xm.Residues.Count(); i++ )  {
    XResidue& residue = xm.Residues[i];
    for( int j=0; j < residue.Count(); j++ )  {
      if( residue[j].Deleted )  continue;
      for( int k=j+1; k < residue.Count(); k++ )  {
        if( residue[k].Deleted )  continue;
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
    XShelxInsList* L = Ins.Object(i);
    if( !Ins[i].StartsFrom("REM") )  continue;
    olxstr tmp = L->IsEmpty() ? EmptyString : L->Text(' ');
    HypernateIns(Ins.String(i)+' ', tmp, SL);
  }
}
//..............................................................................

void XShelxIns::SetSfacUnit(const olxstr& su) {
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
bool XShelxIns::Adopt(TXFile *XF)  {
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
      TSpaceGroup* sg = TSymmLib::GeXShelxInstance()->FindGroup( strSg );
      if( sg != NULL )
        GetAsymmUnit().ChangeSpaceGroup(*sg);
    }
  }
  if( RefinementMethod.IsEmpty() )
    RefinementMethod = "L.S.";
  return true;
}
//..............................................................................
void XShelxIns::DeleteAtom(TCAtom *CA)  {
  for( int i =0; i < Ins.Count(); i++ )  {
    for( int j=0; j < Ins.Object(i)->Count(); j++ ) 
      if( Ins.Object(i)->Object(j) == CA )  Ins.Object(i)->Object(j) = NULL;
  }
}
//..............................................................................
void XShelxIns::UpdateAtomsFromStrings(TCAtomPList& CAtoms, TStrList& SL, TStrList& Instructions) {
  TStrList Toks;
  olxstr Tmp, Tmp1;
  TCAtom *atom;
  int iv, atomCount = 0;
  ParseContext cx;
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
    if( Tmp1 == "REM" )  ;
    else if( ParseIns(SL, Toks, cx, i) )  
      ;
    else if( Toks.Count() < 6 )  // should be at least
      Instructions.Add(Tmp);
    else if( !AtomsInfo->IsAtom(Tmp1) )  // is a valid atom
      Instructions.Add(Tmp);
    else if( !Toks[1].IsNumber() )  // should be a number
      Instructions.Add(Tmp);
    else if( (!Toks[2].IsNumber()) || (!Toks[3].IsNumber()) || // should be four numbers
        (!Toks[4].IsNumber()) || (!Toks[5].IsNumber()) )  {
      Instructions.Add(Tmp);
    }
    else  {
      iv  = Toks[1].ToInt();
      if( iv != -1 ) // wrong index in SFAC, only -1 is supported
        throw TInvalidArgumentException(__OlxSourceInfo, "wrong SFAC index, only -1 is supported");

      if( (atomCount+1) > CAtoms.Count() )  {
        if( atom && atom->GetParent() )  {
          atom = &atom->GetParent()->NewAtom(cx.Resi);
          atom->SetLoaderId( liNewAtom );
        }
      }
      else  {
        atom = CAtoms[atomCount];
        if( cx.Resi != NULL )  cx.Resi->AddAtom(atom);
      }
      // clear fixed fixed values as they reread
      atom->FixedValues().Null();

      _ParseAtom( Toks, cx, atom );
      atomCount++;
      atom->SetPart(cx.Part);
      atom->SetAfix(cx.Afix);
      atom->SetLabel( Tmp1 );
    }
  }
  ParseRestraints(Instructions, CAtoms[0]->GetParent());
  Instructions.Pack();
}
//..............................................................................
bool XShelxIns::SaveAtomsToStrings(const TCAtomPList& CAtoms, TStrList& SL, TSimpleRestraintPList* processed)  {
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
void XShelxIns::SavePattSolution(const olxstr& FileName, const TTypeList<TPattAtom>& atoms, const olxstr& comments )  {
  TStrPObjList<olxstr,TBasicAtomInfo*> BasicAtoms;
  TStrList SL;
  XShelxInsList* L;
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
  
  _SaveSymm(SL);
  
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
TCAtom* XShelxIns::_ParseAtom(TStrList& Toks, ParseContext& cx, TCAtom* atom)  {
  evecd QE(6);
  if( atom == NULL )  {
    atom = &GetAsymmUnit().NewAtom(cx.Resi);
    atom->SetLoaderId(GetAsymmUnit().AtomCount()-1);
  }
  atom->ccrd()[0] = Toks[2].ToDouble();
  atom->ccrd()[1] = Toks[3].ToDouble();
  atom->ccrd()[2] = Toks[4].ToDouble();
  for( int j=0; j < 3; j ++ )  {
    if( fabs(atom->ccrd()[j]) >= 5 )  {
      atom->ccrd()[j] -= 10;
      atom->FixedValues()[ TCAtom::CrdFixedValuesOffset + j ] = 10;
    }
  }
  // initialise uncertanties using average cell error
  atom->ccrdEsd()[0] = fabs(atom->ccrd()[0]*Error);
  atom->ccrdEsd()[1] = fabs(atom->ccrd()[1]*Error);
  atom->ccrdEsd()[2] = fabs(atom->ccrd()[2]*Error);
  atom->SetAfix( cx.Afix );
  atom->SetPart( cx.Part );
  if( cx.PartOccu != 0 )
    atom->SetOccp( cx.PartOccu );
  else
    atom->SetOccp(  Toks[5].ToDouble() );

  if( fabs(atom->GetOccp()) > 10 )  {  // a variable or fixed param
    int iv = (int)(atom->GetOccp()/10); iv *= 10; // extract variable index
    atom->SetOccpVar( iv );
    atom->SetOccp( fabs(atom->GetOccp() - iv) );
    iv = (int)(abs(iv)/10);            // do not need to store the sign anymore
    if( (iv <= FVars.Count()) && iv > 1 )  {  // check if it is a free variable and not just equal to "1"
      if( atom->GetOccpVar() < 0 )  atom->SetOccp( 1 - FVars[iv-1] );
      else                          atom->SetOccp(FVars[iv-1]);
      // do not process - causes too many problems!!!
      if( cx.PartOccu != 0 )
        atom->SetOccpVar( cx.PartOccu );
      else
        atom->SetOccpVar(  Toks[5].ToDouble() );
    }
    if( iv != 1 )  // keep the value
      atom->SetOccpVar(  Toks[5].ToDouble() );
  }
  if( Toks.Count() == 12 )  {  // full ellipsoid
    atom->EllpE().Resize(6);
    QE[0] = Toks[6].ToDouble();
    QE[1] = Toks[7].ToDouble();
    QE[2] = Toks[8].ToDouble();
    QE[3] = Toks[9].ToDouble();
    QE[4] = Toks[10].ToDouble();
    QE[5] = Toks[11].ToDouble();

    for( int j=0; j < 6; j ++ )  {
      if( fabs(QE[j]) > 10 )  {
        int iv = (int)QE[j]/10;
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
      atom->SetUiso( Toks[6].ToDouble() );
      if( fabs(atom->GetUiso()) > 10 )  {
        atom->SetUisoVar( atom->GetUiso() );
        int iv = (int)atom->GetUiso()/10;
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
      atom->SetQPeak( Toks[7].ToDouble() );
    if( atom->GetUiso() < 0 )  {  // a value fixed to a bound atom value
      atom->SetUisoVar(atom->GetUiso());
      atom->SetUiso( 4*caDefIso*caDefIso );
    }
  }
  return atom;
}
//..............................................................................
olxstr XShelxIns::_AtomToString(TCAtom* CA, int SfacIndex)  {
  double v;
  evecd QE(6);   // quadratic form of ellipsoid
  olxstr Tmp = CA->Label();
  Tmp.Format(6, true, ' ');
  Tmp << SfacIndex;
  Tmp.Format(Tmp.Length()+4, true, ' ');
  for( int j=0; j < 3; j++ )  {
    v = CA->ccrd()[j];
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
olxstr XShelxIns::_CellToString()  {
  olxstr Tmp("CELL ");
  Tmp << rm.WaveLength;
  Tmp << ' ' << GetAsymmUnit().Axes()[0].GetV() <<
         ' ' << GetAsymmUnit().Axes()[1].GetV() <<
         ' ' << GetAsymmUnit().Axes()[2].GetV() <<
         ' ' << GetAsymmUnit().Angles()[0].GetV() <<
         ' ' << GetAsymmUnit().Angles()[1].GetV() <<
         ' ' << GetAsymmUnit().Angles()[2].GetV();
  return Tmp;
}
//..............................................................................
void XShelxIns::_SaveFVar(TStrList& SL)  {
  olxstr Tmp; // = "FVAR ";
  for( int i=0; i < FVars.Count(); i++ )  {
    Tmp << FVars[i];
    Tmp.Format(Tmp.Length()+1, true, ' ');
  }
  if( FVars.Count() == 0 ) Tmp << 1;
  HypernateIns("FVAR ", Tmp, SL);
}
//..............................................................................
olxstr XShelxIns::_ZerrToString()  {
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
void XShelxIns::_SaveSymm(TStrList& SL)  {
  SL.Add("LATT ") << GetAsymmUnit().GetLatt();
  if( GetAsymmUnit().MatrixCount() == 1 )  {
    if( !GetAsymmUnit().GetMatrix(0).r.IsI() )
      SL.Add("SYMM ") << TSymmParser::MatrixToSymm( GetAsymmUnit().GetMatrix(0) );
  }
  else  {
    for( int i=0; i < GetAsymmUnit().MatrixCount(); i++ )
      SL.Add("SYMM ") << TSymmParser::MatrixToSymm( GetAsymmUnit().GetMatrix(i) );
  }
}
//..............................................................................
void XShelxIns::_SaveRefMethod(TStrList& SL)  {
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
void XShelxIns::_SaveHklSrc(TStrList& SL)  {
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

void StoreUsedSymIndex(TIntList& il, const smatd* m, TAsymmUnit* au)  {
  if( m == NULL )  return;
  int ind = au->UsedSymmIndex( *m );
  if( il.IndexOf(ind) == -1 )
    il.Add(ind);
}

void XShelxIns::SaveRestraints(TStrList& SL, const TCAtomPList* atoms,
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
void XShelxIns::ClearIns()  {
  for( int i=0; i < Ins.Count(); i++ )
    delete Ins.Object(i);
  Ins.Clear();
}
//..............................................................................
bool XShelxIns::AddIns(const olxstr& Params)  {
  TStrList toks(Params, ' ');
  return AddIns(toks);
}
//..............................................................................
void XShelxIns::SaveHeader(TStrList& SL, int* SfacIndex, int* UnitIndex)  {
  SL.Add("TITL ") << GetTitle();
  SL.Add( _CellToString() );
  SL.Add( _ZerrToString() );
  _SaveSymm(SL);
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
    XShelxInsList* L = Ins.Object(i);
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
void XShelxIns::ParseHeader(const TStrList& in)  {
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
  ParseContext cx;
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

    if( ParseIns(in, toks, cx, i) )
      continue;
    else
      Ins.Add(in[i]);
  }
  smatd sm;
  for( int i=0; i < cx.Symm.Count(); i++ )  {
    if( TSymmParser::SymmToMatrix(cx.Symm[i], sm) )
      GetAsymmUnit().AddMatrix(sm);
  }
  Ins.Pack();
  ParseRestraints(Ins, NULL);
  Ins.Pack();
  _FinishParsing();
}
//..............................................................................

