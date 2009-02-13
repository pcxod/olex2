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
#include "xmacro.h"
#include "sortedlist.h"

#undef AddAtom
#undef GetObject
#undef Object

//----------------------------------------------------------------------------//
// TIns function bodies
//----------------------------------------------------------------------------//
TIns::TIns()  {
  LoadQPeaks = true;
}
//..............................................................................
TIns::~TIns()  {
  Clear();
}
//..............................................................................
void TIns::Clear()  {
  GetRM().ClearAll();
  GetAsymmUnit().Clear();
  for( int i=0; i < Ins.Count(); i++ )
    delete Ins.Object(i);
  Ins.Clear();
  Skipped.Clear();
  Title = EmptyString;
  R1 = -1;
  Disp.Clear();
  Sfac = EmptyString;
  Unit = EmptyString;
}
//..............................................................................
void TIns::LoadFromStrings(const TStrList& FileContent)  {
  Clear();
  ParseContext cx(GetRM());
  TAtomsInfo& atomsInfo = TAtomsInfo::GetInstance();
  TStrList Toks, InsFile(FileContent);
  for( int i=0; i < InsFile.Count(); i++ )  // heh found it
    InsFile[i] = InsFile[i].Trim(' ').Trim('\0');
  InsFile.CombineLines('=');
  TBasicAtomInfo& baiQPeak = atomsInfo.GetAtomInfo(iQPeakIndex);
  cx.Resi = &GetAsymmUnit().GetResidue(-1);
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

    if( Toks[0].Comparei("MOLE") == 0 )  // these are dodgy
      continue;
    else if( ParseIns(InsFile, Toks, cx, i) )
      continue;
    else if( Toks[0].Comparei("END") == 0 )  {   //reset RESI to default
      cx.End = true;  
      cx.Resi = &GetAsymmUnit().GetResidue(-1);
      cx.AfixGroups.Clear();
      cx.Part = 0;
    }
    else if( Toks.Count() < 6 )  // atom sgould have at least 7 parameters
      Ins.Add(InsFile[i]);
    else {
      bool qpeak = olxstr::o_toupper(Toks[0].CharAt(0)) == 'Q';
      if( qpeak && !cx.End && !LoadQPeaks )  continue;
      if( cx.End && !qpeak )  continue;
      // is a valid atom
      //if( !atomsInfo.IsAtom(Toks[0]))  {  Ins.Add(InsFile[i]);  continue;  }
      if( !Toks[1].IsNumber() )         {  Ins.Add(InsFile[i]);  continue;  }
      int index  = Toks[1].ToInt();
      if( index < 1 || index > cx.BasicAtoms.Count() )  {  // wrong index in SFAC
        Ins.Add(InsFile[i]);
        continue;
      }
      // should be four numbers
      if( (!Toks[2].IsNumber()) || (!Toks[3].IsNumber()) ||
        (!Toks[4].IsNumber()) || (!Toks[5].IsNumber()) )  {
          Ins.Add(InsFile[i]);
          continue;
      }
      if( !cx.CellFound )  {
        Clear();
        throw TFunctionFailedException(__OlxSourceInfo, "uninitialised cell");
      }
      TCAtom* atom = _ParseAtom(Toks, cx);
      atom->Label() = Toks[0];
      if( qpeak ) 
        atom->SetAtomInfo(&baiQPeak);
      else
        atom->SetAtomInfo( cx.BasicAtoms.Object(Toks[1].ToInt()-1) );
      _ProcessAfix(*atom, cx);
    }
  }
  if( Sfac.CharCount(' ') != Unit.CharCount(' ') )  {
    Clear();
    throw TFunctionFailedException(__OlxSourceInfo, "mismatching SFAC/UNIT");
  }
  smatd sm;
  for( int i=0; i < cx.Symm.Count(); i++ )  {
    if( TSymmParser::SymmToMatrix(cx.Symm[i], sm) )
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
  ParseRestraints(Ins, cx);
  Ins.Pack();
  _ProcessSame(cx);
  _FinishParsing(cx);
  // initialise asu data
  GetAsymmUnit().InitData();
  if( !cx.CellFound )  {  // in case there are no atoms
    Clear();
    throw TInvalidArgumentException(__OlxSourceInfo, "empty CELL");
  }
}
//..............................................................................
void TIns::_ProcessSame(ParseContext& cx)  {
  TSameGroupList& sgl = cx.rm.rSAME;
  TStrList toks;
  for( int i=0; i < cx.Same.Count(); i++ )  {
    TCAtom* ca = cx.Same[i].B();
    TStrList& sl = cx.Same[i].A();
    if( ca == NULL ) // should not happen
      continue;
    TSameGroup& sg = sgl.New();  // main, reference, group
    int max_atoms = 0;
    for( int j=0; j < sl.Count(); j++ )  {
       toks.Clear();
       toks.Strtok(sl[j], ' ');
       int resi_ind = toks[0].IndexOf('_');
       olxstr resi( (resi_ind != -1) ? toks[0].SubStringFrom(resi_ind+1) : EmptyString );
       double esd1=0.02, esd2=0.04;
       int from_ind = 1;
       if( toks.Count() > 1 && toks[1].IsNumber() )  {
         esd1 = toks[1].ToDouble(); 
         from_ind++;
       }
       if( toks.Count() > 2 && toks[2].IsNumber() )  {
         esd2 = toks[2].ToDouble(); 
         from_ind++;
       }
       TAtomReference ar( toks.Text(' ', from_ind) );
       TCAtomGroup ag;
       int atomAGroup;
       try  {  ar.Expand( cx.rm, ag, resi, atomAGroup);  }
       catch( const TExceptionBase& ex )  {
         throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid SAME instruction :") << ex.GetException()->GetError());
       }
       if( ag.IsEmpty() )
         throw TFunctionFailedException(__OlxSourceInfo, "empty SAME atoms list");
       TSameGroup& sg1 = sgl.NewDependent(sg);
       for( int k=0; k < ag.Count(); k++ )
         sg1.Add( *ag[k].GetAtom() );
       sg1.Esd12 = esd1;
       sg1.Esd13 = esd2;
       if( ag.Count() > max_atoms )
         max_atoms = ag.Count();
    }
    // now process the reference group
    for( int j=0; j < max_atoms; j++ )  {
      if( ca->GetId() + j >= cx.au.AtomCount() )
        throw TFunctionFailedException(__OlxSourceInfo, "not enough atoms to create the reference group for SAME");
      TCAtom& a = cx.au.GetAtom(ca->GetId() + j);
      if( a.GetAtomInfo() == iHydrogenIndex || a.GetAtomInfo() == iDeuteriumIndex )  {
        max_atoms++;
        continue;
      }
      sg.Add( a );
    }
#ifdef _DEBUG
    for( int j=0; j < sg.Count(); j++ )  {
      olxstr s(sg[j].GetLabel());
      s << ':';
      for( int k=0; k < sg.DependentCount(); k++ )  {
        if( sg.GetDependent(k).Count() > j )
          s << sg.GetDependent(k)[j].GetLabel() << ' ';
      }
      TBasicApp::GetLog() << s << '\n';
    }
#endif
  }
}
//..............................................................................
void TIns::_FinishParsing(ParseContext& cx)  {
  for( int i =0; i < Ins.Count(); i++ )  {
    TInsList* Param = new TInsList(Ins[i], ' ');
    Ins.Object(i) = Param;
    Ins[i] = Param->String(0);
    Param->Delete(0);
    for( int j=0; j < Param->Count(); j++ )
      Param->Object(j) = GetAsymmUnit().FindCAtom(Param->String(j));
  }
  // TODO: an update of the values from the variables may be required...
  //TAsymmUnit& au = GetAsymmUnit();
  //for( int i=0; i < au.AtomCount(); i++ )  {
  //  TCAtom& ca = au.GetAtom(i);
  //}

  cx.rm.Vars.Validate();
  cx.rm.ProcessFrags();
}
//..............................................................................
bool TIns::ParseIns(const TStrList& ins, const TStrList& Toks, ParseContext& cx, int& i)  {
  TAtomsInfo& atomsInfo = TAtomsInfo::GetInstance();
  if( _ParseIns(cx.rm, Toks) )
    return true;
  else if( !cx.CellFound && Toks[0].Comparei("CELL") == 0 )  {
    if( Toks.Count() == 8 )  {
      cx.rm.expl.SetRadiation( Toks[1].ToDouble() );
      cx.au.Axes()[0] = Toks[2];
      cx.au.Axes()[1] = Toks[3];
      cx.au.Axes()[2] = Toks[4];
      cx.au.Angles()[0] = Toks[5];
      cx.au.Angles()[1] = Toks[6];
      cx.au.Angles()[2] = Toks[7];
      cx.CellFound = true;
      cx.au.InitMatrices();
    }
    else  
      throw TFunctionFailedException(__OlxSourceInfo, "invalid Cell instruction");
  }
  else if( Toks[0].Comparei("SYMM") == 0 && (Toks.Count() > 1))
    cx.Symm.Add( Toks.Text(EmptyString, 1) );
  else if( Toks[0].Comparei("FRAG") == 0 && (Toks.Count() > 1))  {
   int code = Toks[1].ToInt();
    if( code < 17 )
      throw TInvalidArgumentException(__OlxSourceInfo, "FRAG code must be more than 16");
    double a=1, b=1, c=1, al=90, be=90, ga=90;
    XLibMacros::ParseOnlyNumbers<double>(Toks, 6, 2, &a, &b, &c, &al, &be, &ga);
    Fragment* frag = cx.rm.FindFragByCode(code);
    if( frag == NULL )
      frag = &cx.rm.AddFrag(Toks[1].ToInt(), a, b, c, al, be, ga);
    else
      frag->Reset(a, b, c, al, be, ga);
    TStrList f_toks;
    while( ++i < ins.Count() && !ins[i].StartFromi("FEND") )  {
      if( ins[i].IsEmpty() )  continue;
      f_toks.Strtok(ins[i], ' ');
      if( f_toks.Count() > 4 )
        frag->Add(f_toks[0], f_toks[2].ToDouble(), f_toks[3].ToDouble(), f_toks[4].ToDouble());
      else
        throw TFunctionFailedException(__OlxSourceInfo, "invalid FRAG atom");
      f_toks.Clear();
    }
  }
  else if( Toks[0].Comparei("PART") == 0 && (Toks.Count() > 1) )  {
    cx.Part = (short)Toks[1].ToInt();
    if( cx.Part == 0 )  cx.PartOccu = 0;
    if( Toks.Count() == 3 )
      cx.PartOccu = Toks[2].ToDouble();
  }
  else if( Toks[0].Comparei("AFIX") == 0 && (Toks.Count() > 1) )  {
    int afix = Toks[1].ToInt();
    TAfixGroup* afixg = NULL;
    int n = 0, m = 0;
    if( afix != 0 )  {
      double d = 0, u = 0, sof = 0;
      if( Toks.Count() > 2 && Toks[2].IsNumber() )
        d = Toks[2].ToDouble();
      if( Toks.Count() > 3 )
        sof = Toks[3].ToDouble();
      if( Toks.Count() > 4 )
        u = Toks[3].ToDouble();
      n = TAfixGroup::GetN(afix);
      m = TAfixGroup::GetM(afix);
      if( !TAfixGroup::IsDependent(afix) )
        afixg = &cx.rm.AfixGroups.New(NULL, afix, d, sof == 11 ? 0 : sof, u == 10.08 ? 0 : u);
      else {
        if( !cx.AfixGroups.IsEmpty() && !cx.AfixGroups.Current().B()->IsFitted() ) 
          cx.AfixGroups.Pop();
      }
    }
    // Shelx manual: n is always a single digit; m may be two, one or zero digits (the last corresponds to m = 0).
    if( afix == 0 )  {
      if( !cx.AfixGroups.IsEmpty() )  {
        int old_m = cx.AfixGroups.Current().GetB()->GetM();
        if( cx.AfixGroups.Current().GetA() > 0 )  {
          if( old_m != 0 )
            throw TFunctionFailedException(__OlxSourceInfo, olxstr("incomplete AFIX group") <<
              (cx.Last != NULL ? (olxstr(" at ") << cx.Last->GetLabel()) : EmptyString) );
          else
            TBasicApp::GetLog().Warning( olxstr("Possibly incorrect AFIX ") << cx.AfixGroups.Current().GetB()->GetAfix() <<
              (cx.Last != NULL ? (olxstr(" at ") << cx.Last->GetLabel()) : EmptyString) );
        }
        if( cx.AfixGroups.Current().GetB()->GetPivot() == NULL )
          throw TFunctionFailedException(__OlxSourceInfo, "undefined pivot atom for a fitted group");
        while( !cx.AfixGroups.IsEmpty() && cx.AfixGroups.Current().GetA() <= 0 )  // pop all out  
          cx.AfixGroups.Pop();
      }
    }
    else  {
      if( !cx.AfixGroups.IsEmpty() && cx.AfixGroups.Current().GetA() == 0 )  {  // pop m =0 as well
        cx.AfixGroups.Pop();
      }
      if( afixg != NULL )  {
        switch( m )  {
        case 1:
        case 4:
        case 8:
        case 14:
        case 15:
        case 16:
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>(1, afixg, false) );
          break;
        case 2:
        case 9:
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>(2, afixg, false) );
          break;
        case 3:
        case 13:
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>(3, afixg, false) );
          break;
        case 7:
        case 6:
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>(5, afixg, true));
          cx.SetNextPivot = true;
          break;
        case 5:
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>(4, afixg, true));
          cx.SetNextPivot = true;
          break;
        case 11:  //naphtalene
        case 10:  // Cp*
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>(9, afixg, true));
          cx.SetNextPivot = true;
          break;
        case 12:  // disordered CH3
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>(6, afixg, false) );
          break;
        }
        if( m > 16 )  {  // FRAG
          Fragment* frag = cx.rm.FindFragByCode(m);
          if( frag == NULL )
            throw TInvalidArgumentException(__OlxSourceInfo, "fitted group should be preceeded by the FRAG..FEND with the same code");
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>(frag->Count()-1, afixg, false) );
          cx.SetNextPivot = true;
        }
        else if( m == 0 && !TAfixGroup::IsDependent(afix) )  {  // generic container then, beside, 5 is dependent atom of rigid group
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>(-1, afixg, false) );
          cx.SetNextPivot = !TAfixGroup::IsRiding(afix); // if not riding
        }
        if( !cx.SetNextPivot )  {
          if( cx.Last == NULL  )
            throw TFunctionFailedException(__OlxSourceInfo, "undefined pivot atom for a fitted group");
          // have to check if several afixes for one atom (if the last is H)
          TCAtom* pivot = NULL;
          if( cx.Last->GetAtomInfo().GetMr() < 3.5 )  { // locate previous non H
            for( int i=cx.Last->GetId()-1; i >=0; i-- )  {
              TCAtom& ca = cx.au.GetAtom(i);
              if( ca.GetAtomInfo().GetMr() > 3.5 )  {
                pivot = &ca;
                break;
              }
            }
          }
          else
            pivot = cx.Last;
          if( pivot == NULL  )
            throw TFunctionFailedException(__OlxSourceInfo, "undefined pivot atom for a fitted group");
          afixg->SetPivot(*pivot);
        }
      }
    }
  }
  else if( Toks[0].Comparei("RESI") == 0 )  {
    if( Toks.Count() < 3 )
      throw TInvalidArgumentException(__OlxSourceInfo, "wrong number of arguments for a residue");
    if( Toks[1].IsNumber() )
      cx.Resi = &cx.au.NewResidue(EmptyString, Toks[1].ToInt(), (Toks.Count() > 2) ? Toks[2] : EmptyString);
    else
      cx.Resi = &cx.au.NewResidue(Toks[1], Toks[2].ToInt(), (Toks.Count() > 3) ? Toks[3] : EmptyString);
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
        if( !Sfac.IsEmpty() )
          Sfac << ' ';
        Sfac << Toks[1];
        cx.BasicAtoms.Add( Toks[1], atomsInfo.FindAtomInfoBySymbol(Toks[1]) );
        if( cx.BasicAtoms.Last().Object() == NULL )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not find suitable scatterer for '") << Toks[1] << '\'' );
        expandedSfacProcessed = true;
        cx.rm.AddNewSfac( Toks[1],
          Toks[2].ToDouble(),  Toks[3].ToDouble(),  Toks[4].ToDouble(),
          Toks[5].ToDouble(),  Toks[6].ToDouble(),  Toks[7].ToDouble(),
          Toks[8].ToDouble(),  Toks[9].ToDouble(),  Toks[10].ToDouble(),
          Toks[11].ToDouble(), Toks[12].ToDouble(), Toks[13].ToDouble());
      }
    }
    if( !expandedSfacProcessed )  {
      for( int j=1; j < Toks.Count(); j++ )  {
        if( atomsInfo.IsAtom(Toks[j]) )  {
          cx.BasicAtoms.Add(Toks[j], atomsInfo.FindAtomInfoBySymbol(Toks[j]) );
          if( cx.BasicAtoms.Last().Object() == NULL )
            throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not find suitable scatterer for '") << Toks[j] << '\'' );
          if( !Sfac.IsEmpty() )
            Sfac << ' ';
          Sfac << Toks[j];
        }
      }
    }
  }
  else if( Toks[0].Comparei("DISP") == 0 )     {  
    Disp.Add( Toks.Text(' ', 1) );
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
        olxstr hklsrc = Toks.Text(' ', 1);
        int index = hklsrc.FirstIndexOf('>');
        int iv = hklsrc.IndexOf("</HKL>");
        if( iv == -1 )  {
          while( (i+1) < ins.Count() )  {
            i++;
            if( !ins[i].StartsFromi("rem") )  break;
            hklsrc << ins[i].SubStringFrom(4);
            iv = hklsrc.IndexOf("</HKL>");
            if( iv != -1 )  break;
          }
        }
        if( iv != -1 )  {
          hklsrc = hklsrc.SubString(index+1, iv-index-1);
          hklsrc.Replace("%20", ' ');
        }
        else
          hklsrc = EmptyString;
        cx.rm.SetHKLSource(hklsrc);
      }
      else if( !cx.End  && !cx.rm.IsHKLFSet() )
        Ins.Add( Toks.Text(' ') ); 
    }
  }
  else if( Toks[0].Comparei("SAME") == 0 )  {
    if( !cx.Same.IsEmpty() && cx.Same.Last().GetB() == NULL )  // no atom so far, add to the list of Same
      cx.Same.Last().A().Add( Toks.Text(' ', 1) );
    else  {
      cx.Same.Add( new AnAssociation2<TStrList,TCAtom*> );
      cx.Same.Last().B() = NULL;
      cx.Same.Last().A().Add( Toks.Text(' ') );
    }
  }
  else
    return false;
  return true;
}
//..............................................................................
void TIns::UpdateParams()  {
  for( int i =0; i < Ins.Count(); i++ )  {
    if( Ins.Object(i) == NULL )  continue;  // might happen if load failed
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
bool TIns::AddIns(const TStrList& toks, RefinementModel& rm, bool CheckUniq)  {
  // special instructions
  if( _ParseIns(rm, toks) )  return true;
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
void TIns::SaveToRefine(const olxstr& FileName, const olxstr& sMethod, const olxstr& comments)  {
  TStrList SL, mtoks;
  if( sMethod.IsEmpty() )
    mtoks.Add("TREF");
  else  {
    mtoks.Strtok(sMethod, "\\n");
    int spi = mtoks[0].IndexOf(' ');
    if( spi >= 0)
      RefMod.SetSolutionMethod(mtoks[0].SubStringTo(spi));
    else
      RefMod.SetSolutionMethod(mtoks[0]);
  }

  int UnitIndex, SfacIndex;
  olxstr Tmp, Tmp1;

  UpdateParams();
  SL.Add("TITL ") << GetTitle();

  if( !comments.IsEmpty() ) 
    SL.Add("REM ") << comments;
// try to estimate Z'
  TTypeList< AnAssociation2<int,TBasicAtomInfo*> > sl;
  TStrList sfac(Sfac, ' ');
  TStrList unit(Unit, ' ');
  TAtomsInfo& atomsInfo = TAtomsInfo::GetInstance();
  if( sfac.Count() != unit.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "SFAC does not match UNIT");
  int ac = 0;
  for( int i=0; i < sfac.Count(); i++ )  {
    int cnt = unit[i].ToInt();
    TBasicAtomInfo* bai = atomsInfo.FindAtomInfoBySymbol(sfac[i]);
    if( *bai == iHydrogenIndex )  continue;
    sl.AddNew( cnt, bai );
    ac += cnt;
  }
  int newZ = Round(AsymmUnit.EstimateZ(ac/AsymmUnit.GetZ()));
  Unit = EmptyString;
  for( int i=0; i < sfac.Count(); i++ )  {
    int cnt = unit[i].ToInt();
    Unit << (double)cnt*newZ/AsymmUnit.GetZ();
    if( (i+1) < sfac.Count() )
      Unit << ' ';
  }
  AsymmUnit.SetZ( newZ );
//

  SL.Add( _CellToString() );
  SL.Add( _ZerrToString() );
  _SaveSymm(SL);
  SfacIndex = SL.Count();  SL.Add(EmptyString);
  UnitIndex = SL.Count();  SL.Add(EmptyString);

  _SaveSizeTemp(SL);
  _SaveHklInfo(SL);
  _SaveFVar(RefMod, SL);

  SL.AddList(mtoks);
  SL.Add(EmptyString);
  SL.Add("HKLF ") << RefMod.GetHKLFStr();
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
  if( GetRM().SfacCount() == 0 )
    list[pos] = olxstr("SFAC ") << Sfac;
  else  {
    TStrList toks(Sfac, ' '), lines;
    olxstr tmp, LeftOut;
    for( int i=0; i < toks.Count(); i++ )  {
      XScatterer* sd = GetRM().FindSfacData( toks[i] );
      if( sd != NULL )  {
        tmp = "SFAC ";
        tmp << sd->ToInsString();
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
    if( !LeftOut.IsEmpty() != 0 )  {
      list.Insert(pos, olxstr("SFAC") << LeftOut );
    }
  }
  for( int i=0; i < Disp.Count(); i++ )
    list.Insert(++pos, olxstr("DISP ") << Disp[i]);
}
//..............................................................................
void TIns::_SaveAtom(RefinementModel& rm, TCAtom& a, int& part, int& afix, 
                     TStrPObjList<olxstr,TBasicAtomInfo*>* sfac, TStrList& sl, TIntList* index, bool checkSame)  {
  if( a.IsDeleted() || a.IsSaved() )  return;
  if( checkSame && a.GetSameId() != -1 )  {  // "
    TSameGroup& sg = rm.rSAME[a.GetSameId()];
    for( int i=0; i < sg.DependentCount(); i++ )  {
      olxstr tmp( "SAME ");
      tmp << olxstr(sg.Esd12).TrimFloat() << ' ' << olxstr(sg.Esd13).TrimFloat();
      for( int j=0; j < sg.GetDependent(i).Count(); j++ )
        tmp << ' ' << sg.GetDependent(i)[j].GetLabel();
      HypernateIns( tmp, sl );
    }
    for( int i=0; i < sg.Count(); i++ )
      _SaveAtom(rm, sg[i], part, afix, sfac, sl, index, false);
    return;
  }
  if( a.GetPart() != part )  sl.Add("PART ") << a.GetPart();
  TAfixGroup* ag = a.GetDependentAfixGroup();
  int atom_afix = a.GetAfix();
  if( atom_afix != afix || afix == 1 || afix == 2 )  { 
    if( !TAfixGroup::HasExcplicitPivot(afix) || !TAfixGroup::IsDependent(atom_afix) )  {
      if( ag != NULL )  {
        olxstr& str = sl.Add("AFIX ") << atom_afix;
        if( ag->GetD() != 0 )  {
          str << ' ' << ag->GetD();
        }
        if( ag->GetSof() != 0 )  {
          str << ' ' << ag->GetSof();
          if( ag->GetU() != 0 )
            str << ' ' << ag->GetU();
        }
      }
      else
        sl.Add("AFIX ") << atom_afix;
    }
  }
  afix = atom_afix;
  part = a.GetPart();
  int spindex;
  if( a.GetAtomInfo() == iQPeakIndex )
    spindex = (sfac == NULL ? -2 : sfac->CIIndexOf('c') );
  else
    spindex = (sfac == NULL ? -2 : sfac->IndexOfObject( &a.GetAtomInfo() ));
  HypernateIns( _AtomToString(rm, a, spindex+1), sl );
  a.SetSaved(true);
  if( index != NULL )  index->Add(a.GetTag());
  for( int i=0; i < a.DependentHfixGroupCount(); i++ )  {
    TAfixGroup& hg = a.GetDependentHfixGroup(i);
    for( int j=0; j < hg.Count(); j++ )
      _SaveAtom(rm, hg[j], part, afix, sfac, sl, index, checkSame);
  }
  if( ag != NULL )  {  // save dependent rigid group
    for( int i=0; i < ag->Count(); i++ )
      _SaveAtom(rm, (*ag)[i], part, afix, sfac, sl, index, checkSame);
  }
}
//..............................................................................
void TIns::SaveToStrings(TStrList& SL)  {
  TAtomsInfo& atomsInfo = TAtomsInfo::GetInstance();
  int UnitIndex, SfacIndex;
  evecd QE;  // quadratic form of s thermal ellipsoid
  olxstr Tmp;
  TStrPObjList<olxstr,TBasicAtomInfo*> BasicAtoms(Sfac, ' ');
  for( int i=0; i < BasicAtoms.Count(); i++ )  {
    TBasicAtomInfo* BAI = atomsInfo.FindAtomInfoBySymbol( BasicAtoms.String(i) );
    if( BAI != NULL )  BasicAtoms.Object(i) = BAI;
    else
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Unknown element: ") << BasicAtoms.String(i));
  }
  int carbonBAIIndex = BasicAtoms.CIIndexOf('c');  // for Q-peaks
  for( int i=-1; i < GetAsymmUnit().ResidueCount(); i++ )  {
    TAsymmUnit::TResidue& residue = GetAsymmUnit().GetResidue(i);
    for( int j=0; j < residue.Count(); j++ )  {
      if( residue[j].IsDeleted() )  continue;
      residue[j].SetSaved(false);
      int spindex = BasicAtoms.IndexOfObject( &residue[j].GetAtomInfo() );  // fix the SFAC, if wrong
      if( spindex == -1 )  {
        if( residue[j].GetAtomInfo() == iQPeakIndex )  {
          if( carbonBAIIndex == -1 )  {
            BasicAtoms.Add( "C", &atomsInfo.GetAtomInfo(iCarbonIndex) );
            carbonBAIIndex = BasicAtoms.Count() -1;
            Unit << ' ' << '1';
            Sfac << ' ' << 'C';
          }
        }
        else  {
          BasicAtoms.Add( residue[j].GetAtomInfo().GetSymbol(), &residue[j].GetAtomInfo() );
          Unit << ' ' << '1';
          Sfac << ' ' << residue[j].GetAtomInfo().GetSymbol();
          if( residue[j].GetAtomInfo() == iCarbonIndex )
            carbonBAIIndex = BasicAtoms.Count() - 1;
        }
      }
      if( residue[j].GetLabel().Length() > 4 ) 
        residue[j].Label() = GetAsymmUnit().CheckLabel(&residue[j], residue[j].GetLabel() );
      for( int k=j+1; k < residue.Count(); k++ )  {
        if( residue[k].IsDeleted() )  continue;
        if( residue[j].GetPart() != residue[k].GetPart() && 
            residue[j].GetPart() != 0 && residue[k].GetPart() != 0 )  continue;
        if( residue[j].GetLabel().Comparei(residue[k].GetLabel()) == 0 ) 
          residue[k].Label() = GetAsymmUnit().CheckLabel(&residue[k], residue[k].GetLabel() );
      }
    }
  }
  
  ValidateRestraintsAtomNames( GetRM() );
  UpdateParams();
  SaveHeader(SL, false, &SfacIndex, &UnitIndex);
  SL.Add(EmptyString);
  int afix = 0, part = 0, fragmentId = 0;
  for( int i=-1; i < GetAsymmUnit().ResidueCount(); i++ )  {
    TAsymmUnit::TResidue& residue = GetAsymmUnit().GetResidue(i);
    if( i != -1 && !residue.IsEmpty() ) SL.Add( residue.ToString() );
    for( int j=0; j < residue.Count(); j++ )  {
      TCAtom& ac = residue[j];
      if( ac.IsDeleted() || ac.IsSaved() )  continue;
      if( ac.GetFragmentId() != fragmentId )  {
        SL.Add(EmptyString);
        fragmentId = ac.GetFragmentId();
      }
      if( ac.GetParentAfixGroup() != NULL && 
         !ac.GetParentAfixGroup()->GetPivot().IsDeleted() )  continue;
      _SaveAtom(GetRM(), ac, part, afix, &BasicAtoms, SL);
    }
  }
  if( afix != 0 )  SL.Add("AFIX 0");
  SL.Add("HKLF ") << RefMod.GetHKLFStr();
  SL.String(UnitIndex) = (olxstr("UNIT ") << Unit);
//  SL.String(SfacIndex) = (olxstr("SFAC ") += FSfac);
  _SaveSfac(SL, SfacIndex);
  SL.Add("END");
  SL.Add(EmptyString);
}
//..............................................................................

void TIns::SetSfacUnit(const olxstr& su) {
  TTypeList<AnAssociation2<olxstr, int> > list;
  TAtomsInfo::GetInstance().ParseElementString(su, list);
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
  try  {
    TSpaceGroup& sg = XF->GetLastLoaderSG();
    Title << "in" << sg.GetFullName();
  }
  catch( ... )  {}
  if( XF->HasLastLoader() )  {
    Title = XF->LastLoader()->GetTitle();
    GetRM().SetHKLSource( XF->LastLoader()->GetRM().GetHKLSource() );
    
    if( EsdlInstanceOf(*XF->LastLoader(), TP4PFile) )  {
      TP4PFile& p4p = XF->GetLastLoader<TP4PFile>();
      RefMod.expl.SetRadiation( p4p.GetRadiation() );
      TStrList lst;
      olxstr tmp = p4p.GetSize();  
      if( tmp.IsEmpty() )  {
        tmp.Replace('?', '0');
        lst.Add("SIZE");
        lst.Add( tmp );  
        AddIns(lst, GetRM()); 
        lst.Clear();
      }
      tmp = p4p.GetTemp();
      if( !tmp.IsEmpty() )  {
        tmp.Replace('?', '0');
        lst.Add("TEMP");
        lst.Add( tmp );  
        AddIns(lst, GetRM()); 
        lst.Clear();
      }
      if( p4p.GetChem() != "?" )  {
        try  {  SetSfacUnit( p4p.GetChem() );  }
        catch( TExceptionBase& )  {  }
      }
    }
    else if( EsdlInstanceOf(*XF->LastLoader(), TCRSFile) )  {
      TCRSFile& crs = XF->GetLastLoader<TCRSFile>();
      Sfac = crs.GetSfac();
      Unit = crs.GetUnit();
      RefMod.expl.SetRadiation( crs.GetRadiation() );
    }
    else if( EsdlInstanceOf(*XF->LastLoader(), TCif) )  {
      TCif& cif = XF->GetLastLoader<TCif>();
      olxstr chem = olxstr::DeleteChars( cif.GetSParam("_chemical_formula_sum"), ' ');
      olxstr strSg = cif.GetSParam("_symmetry_space_group_name_H-M");
      
      SetSfacUnit( chem );
      TSpaceGroup* sg = TSymmLib::GetInstance()->FindGroup( strSg );
      if( sg != NULL )
        GetAsymmUnit().ChangeSpaceGroup(*sg);
    }
  }
  if( RefMod.GetRefinementMethod().IsEmpty() )
    RefMod.SetRefinementMethod( "L.S." );
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
void TIns::UpdateAtomsFromStrings(RefinementModel& rm, TCAtomPList& CAtoms, const TIntList& index, TStrList& SL, TStrList& Instructions) {
  if( CAtoms.Count() != index.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "index");
  if( CAtoms.IsEmpty() )  return;
  TAtomsInfo& atomsInfo = TAtomsInfo::GetInstance();
  TStrList Toks;
  olxstr Tmp, Tmp1;
  TCAtom *atom;
  int atomCount = 0;
  ParseContext cx(rm);
  SL.CombineLines('=');
  //rm.FVAR.Clear();
  for( int i=0; i < CAtoms.Count(); i++ )  {
    if( CAtoms[i]->GetParentAfixGroup() != NULL )
      CAtoms[i]->GetParentAfixGroup()->Clear();
    if( CAtoms[i]->GetDependentAfixGroup() != NULL )
      CAtoms[i]->GetDependentAfixGroup()->Clear();
    if( CAtoms[i]->GetExyzGroup() != NULL )
      CAtoms[i]->GetExyzGroup()->Clear();
  }
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
    Tmp1 = Toks[0];
    if( Tmp1 == "REM" )  
      ;
    else if( ParseIns(SL, Toks, cx, i) )  
      ;
    else if( Toks.Count() < 6 )  // should be at least
      Instructions.Add(Tmp);
    else if( !atomsInfo.IsAtom(Toks[1]) )  // is a valid atom
      Instructions.Add(Tmp);
    else if( (!Toks[2].IsNumber()) || (!Toks[3].IsNumber()) || // should be four numbers
        (!Toks[4].IsNumber()) || (!Toks[5].IsNumber()) )  {
      Instructions.Add(Tmp);
    }
    else  {
      TBasicAtomInfo* bai = atomsInfo.FindAtomInfoBySymbol(Toks[1]);
      if( bai == NULL ) // wrong SFAC
        throw TInvalidArgumentException(__OlxSourceInfo, "unknown element symbol");

      if( (atomCount+1) > CAtoms.Count() )  {
        if( atom && atom->GetParent() )  {
          atom = &atom->GetParent()->NewAtom(cx.Resi);
          atom->SetLoaderId( liNewAtom );
        }
      }
      else  {
        atom = CAtoms[index[atomCount]];
        if( cx.Resi != NULL )  cx.Resi->AddAtom(atom);
      }
      // clear fixed fixed values as they reread
      //atom->FixedValues().Null();

      _ParseAtom( Toks, cx, atom );
      atomCount++;
      atom->Label() = Tmp1;
      atom->SetAtomInfo(bai);
      _ProcessAfix(*atom, cx);
    }
  }
  _ProcessSame(cx);
  ParseRestraints(Instructions, cx);
  Instructions.Pack();
}
//..............................................................................
bool TIns::SaveAtomsToStrings(RefinementModel& rm, const TCAtomPList& CAtoms, TIntList& index, 
                              TStrList& SL, RefinementModel::ReleasedItems* processed)  {
  if( CAtoms.IsEmpty() )  return false;
  int part = 0,
      afix = 0,
      resi = -2;
  SaveRestraints(SL, &CAtoms, processed, rm);
  _SaveFVar(rm, SL);
  for(int i=0; i < CAtoms.Count(); i++ )  {
    CAtoms[i]->SetSaved(false);
    CAtoms[i]->SetTag(i);
  }
  for(int i=0; i < CAtoms.Count(); i++ )  {
    if( CAtoms[i]->IsSaved() )  continue;
    if( CAtoms[i]->GetResiId() != resi && CAtoms[i]->GetResiId() != -1 )  {
      resi = CAtoms[i]->GetResiId();
      SL.Add( GetAsymmUnit().GetResidue(CAtoms[i]->GetResiId()).ToString());
    }
    TCAtom& ac = *CAtoms[i];
    if( ac.GetParentAfixGroup() != NULL && 
      !ac.GetParentAfixGroup()->GetPivot().IsDeleted() )  continue;
    _SaveAtom(rm, ac, part, afix, NULL, SL, &index);
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
  TAtomsInfo& atomsInfo = TAtomsInfo::GetInstance();
  for( int i=0; i < atoms.Count(); i++ )  {
    BAI = atomsInfo.FindAtomInfoEx( atoms[i].GetName() );
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
  _SaveSizeTemp(SL);

  // copy "unknown" instructions except rems
  for( int i=0; i < Ins.Count(); i++ )  {
    L = Ins.Object(i);
    // skip rems and print them at the end
    if( Ins[i].StartsFrom("REM") )  continue;
    Tmp = EmptyString;
    if( L->Count() )  Tmp << L->Text(' ');
    HypernateIns(Ins[i]+' ', Tmp, SL);
  }

  _SaveHklInfo(SL);

  Tmp = "WGHT ";
  for( int i=0; i < RefMod.used_weight.Count(); i++ )  {
    Tmp << RefMod.used_weight[i];
    Tmp.Format(Tmp.Length()+1, true, ' ');
  }
  if( RefMod.used_weight.Count() == 0 )  
    Tmp << "0.1";
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
  SL.Add("HKLF ") << GetRM().GetHKLFStr();
  SL.Add(EmptyString);
#ifdef _UNICODE
  TCStrList(SL).SaveToFile(FileName);
#else
  SL.SaveToFile(FileName);
#endif
}
//..............................................................................
void TIns::_ProcessAfix(TCAtom& a, ParseContext& cx)  {
  if( cx.AfixGroups.IsEmpty() )  return;
  if( cx.SetNextPivot )  {
    cx.AfixGroups.Current().B()->SetPivot(a);
    cx.SetNextPivot = 0;
    return;
  }
  //if( cx.AfixGroups.Current().GetA() == 0 )  {
  //  cx.AfixGroups.Pop();
  //}
  //else  {
  if( cx.AfixGroups.Current().GetC() )  {
    if( a.GetAtomInfo() != iHydrogenIndex && a.GetAtomInfo() != iDeuteriumIndex )  {
      cx.AfixGroups.Current().A()--;
      cx.AfixGroups.Current().B()->AddDependent(a);
    }
  }
  else  {
    cx.AfixGroups.Current().A()--;
    cx.AfixGroups.Current().B()->AddDependent(a);
  }
//    }
}
//..............................................................................
TCAtom* TIns::_ParseAtom(TStrList& Toks, ParseContext& cx, TCAtom* atom)  {
  double QE[6];
  if( atom == NULL )  {
    atom = &cx.au.NewAtom(cx.Resi);
    atom->SetLoaderId(cx.au.AtomCount()-1);
  }
  for( int j=0; j < 3; j ++ )
    cx.rm.Vars.SetAtomParam(*atom, var_name_X+j, Toks[2+j].ToDouble());
  atom->SetPart( cx.Part );
  // update the context
  cx.Last = atom;
  if( !cx.Same.IsEmpty() && cx.Same.Last().GetB() == NULL )
    cx.Same.Last().B() = atom;

  cx.rm.Vars.SetAtomParam(*atom, var_name_Sof, cx.PartOccu == 0 ? Toks[5].ToDouble() : cx.PartOccu );

  if( Toks.Count() == 12 )  {  // full ellipsoid
    for( int j=0; j < 6; j ++ )
      QE[j] = cx.rm.Vars.SetAtomParam(*atom, var_name_U11+j, Toks[j+6].ToDouble() );
    cx.au.UcifToUcart(QE);
    TEllipsoid& elp = cx.au.NewEllp().Initialise(QE);
    atom->AssignEllp( &elp );
    if( atom->GetEllipsoid()->IsNPD() )  {
      TBasicApp::GetLog().Info(olxstr("Not positevely defined: ") << Toks[0]);
      atom->SetUiso( 0 );
    }
    else
      atom->SetUiso( atom->GetEllipsoid()->GetUiso() );
    cx.LastWithU = atom;
  }
  else  {
    if( Toks.Count() > 6 )
      cx.rm.Vars.SetAtomParam(*atom, var_name_Uiso, Toks[6].ToDouble());
    else // incomplete data...
      atom->SetUiso( 4*caDefIso*caDefIso );
    if( Toks.Count() >= 8 ) // some other data as Q-peak itensity
      atom->SetQPeak( Toks[7].ToDouble() );
    if( atom->GetUiso() <= -0.5 )  {  // a value fixed to a bound atom value
      if( cx.LastWithU == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Invalid Uiso proxy for: ") << Toks[0]);
      atom->SetUisoScale( olx_abs(atom->GetUiso()) );
      atom->SetUisoOwner( cx.LastWithU );
      //atom->SetUiso( 4*caDefIso*caDefIso );
      atom->SetUiso( cx.LastWithU->GetUiso()*olx_abs(atom->GetUiso()) );
    }
    else
      cx.LastWithU = atom;
  }
  return atom;
}
//..............................................................................
olxstr TIns::_AtomToString(RefinementModel& rm, TCAtom& CA, int SfacIndex)  {
  double v, Q[6];   // quadratic form of ellipsoid
  olxstr Tmp( CA.Label() );
  Tmp.Format(6, true, ' ');
  if( SfacIndex < 0 )
    Tmp << CA.GetAtomInfo().GetSymbol();
  else
    Tmp << SfacIndex;

  Tmp.Format(Tmp.Length()+4, true, ' ');
  for( int j=0; j < 3; j++ )
    Tmp << olxstr::FormatFloat(-5, rm.Vars.GetAtomParam(CA, var_name_X+j)) << ' ';
  
  // save occupancy
  Tmp << olxstr::FormatFloat(-5, rm.Vars.GetAtomParam(CA, var_name_Sof)) << ' ';
  // save Uiso, Uanis
  if( CA.GetEllipsoid() != NULL )  {
    CA.GetEllipsoid()->GetQuad(Q);
    GetAsymmUnit().UcartToUcif(Q);

    for( int j = 0; j < 6; j++ )
      Tmp << olxstr::FormatFloat(-5, rm.Vars.GetAtomParam(CA, var_name_U11+j, Q)) << ' ';
  }
  else  {
    if( CA.GetUisoOwner() )  // riding atom
      v = -CA.GetUisoScale();
    else 
      v = rm.Vars.GetAtomParam(CA, var_name_Uiso);
    Tmp << olxstr::FormatFloat(-5, v) << ' ';
  }
  // Q-Peak
  if( CA.GetAtomInfo() == iQPeakIndex )
    Tmp << olxstr::FormatFloat(-3, CA.GetQPeak());
  return Tmp;
}
//..............................................................................
olxstr TIns::_CellToString()  {
  olxstr Tmp("CELL ");
  Tmp << RefMod.expl.GetRadiation();
  Tmp << ' ' << GetAsymmUnit().Axes()[0].GetV() <<
         ' ' << GetAsymmUnit().Axes()[1].GetV() <<
         ' ' << GetAsymmUnit().Axes()[2].GetV() <<
         ' ' << GetAsymmUnit().Angles()[0].GetV() <<
         ' ' << GetAsymmUnit().Angles()[1].GetV() <<
         ' ' << GetAsymmUnit().Angles()[2].GetV();
  return Tmp;
}
//..............................................................................
void TIns::_SaveFVar(RefinementModel& rm, TStrList& SL)  {
  olxstr Tmp; // = "FVAR ";
  rm.Vars.Validate();
  HypernateIns("FVAR ", rm.Vars.GetFVARStr(), SL);
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
void TIns::_SaveSymm(TStrList& SL)  {
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
void TIns::_SaveRefMethod(TStrList& SL)  {
  if( !RefMod.GetRefinementMethod().IsEmpty() )  {
    if( RefMod.LS.Count() != 0 )  {
      olxstr& rm = SL.Add( RefMod.GetRefinementMethod() );
      for( int i=0; i < RefMod.LS.Count(); i++ )
        rm << ' ' << RefMod.LS[i];
    }
    if( RefMod.PLAN.Count() != 0 )  {
      olxstr& pn = SL.Add("PLAN ");
      for( int i=0; i < RefMod.PLAN.Count(); i++ )
        pn << ' ' << ((i < 1) ? Round(RefMod.PLAN[i]) : RefMod.PLAN[i]);
    }
  }
}
//..............................................................................
void TIns::_SaveHklInfo(TStrList& SL)  {
  if( GetRM().HasMERG() )
    SL.Add("MERG ") << GetRM().GetMERG();
  if( !GetRM().GetBASF().IsEmpty() )
    SL.Add("BASF ") << GetRM().GetBASFStr();
  if( GetRM().HasOMIT() )
    SL.Add("OMIT ") << GetRM().GetOMITStr();
  if( GetRM().HasSHEL() )
    SL.Add("SHEL ") << GetRM().GetSHELStr();
  if( GetRM().HasTWIN() )
    SL.Add("TWIN ") << GetRM().GetTWINStr();
  for( int i=0; i < GetRM().OmittedCount(); i++ )  {
    const vec3i& r = GetRM().GetOmitted(i);
    SL.Add("OMIT ") << r[0] << ' ' << r[1] << ' ' << r[2];
  }
  if( !GetRM().GetHKLSource().IsEmpty() )  {  // update html source string
    olxstr Tmp( GetRM().GetHKLSource() );
    Tmp.Replace(' ', "%20");
    HypernateIns("REM ", olxstr("<HKL>") << Tmp << "</HKL>", SL);
  }
}
//..............................................................................
bool Ins_ProcessRestraint(const TCAtomPList* atoms, TSimpleRestraint& sr)  {
  if( sr.AtomCount() == 0 && !sr.IsAllNonHAtoms() )  return false;
  if( atoms == NULL )  return true;
  for(int i=0; i < atoms->Count(); i++ )
    if( sr.ContainsAtom( atoms->Item(i) ) )  return true;
  return false;
}
void StoreUsedSymIndex(TIntList& il, const smatd* m, RefinementModel& rm)  {
  if( m == NULL )  return;
  int ind = rm.UsedSymmIndex( *m );
  if( il.IndexOf(ind) == -1 )
    il.Add(ind);
}

void TIns::SaveRestraints(TStrList& SL, const TCAtomPList* atoms,
                          RefinementModel::ReleasedItems* processed, RefinementModel& rm)  {
  int oindex = SL.Count();

  olxstr Tmp;
  TIntList usedSymm;
  // fixed distances
  for( int i=0; i < rm.rDFIX.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rDFIX[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "DFIX ";
    Tmp << sr.GetValue() << ' ' << sr.GetEsd();
    for( int j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HypernateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // similar distances
  for( int i=0; i < rm.rSADI.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rSADI[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "SADI ";
    Tmp << sr.GetEsd();
    for( int j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HypernateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // fixed "angles"
  for( int i=0; i < rm.rDANG.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rDANG[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "DANG ";
    Tmp << olxstr::FormatFloat(3, sr.GetValue()) << ' ' << sr.GetEsd();
    for( int j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HypernateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // fixed chiral atomic volumes
  for( int i=0; i < rm.rCHIV.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rCHIV[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "CHIV ";
    Tmp << sr.GetValue() << ' ' << sr.GetEsd();
    for( int j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HypernateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // planar groups
  for( int i=0; i < rm.rFLAT.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rFLAT[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    if( sr.AtomCount() < 4 )  continue;
    Tmp = "FLAT ";
    Tmp << sr.GetEsd();
    for( int j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HypernateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // rigid bond restraint
  for( int i=0; i < rm.rDELU.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rDELU[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "DELU ";
    Tmp << sr.GetEsd() << ' ' << sr.GetEsd1();
    for( int j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HypernateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // similar U restraint
  for( int i=0; i < rm.rSIMU.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rSIMU[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "SIMU ";
    Tmp << sr.GetEsd() << ' ' << sr.GetEsd1() << ' ' << sr.GetValue();
    for( int j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HypernateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // Uanis restraint to behave like Uiso
  for( int i=0; i < rm.rISOR.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rISOR[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "ISOR ";
    Tmp << sr.GetEsd() << ' ' << sr.GetEsd1();
    for( int j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HypernateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // equivalent EADP constraint
  for( int i=0; i < rm.rEADP.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rEADP[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    if( sr.AtomCount() < 2 )  continue;
    Tmp = "EADP";
    for( int j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetAtom()->GetLabel();
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HypernateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // equivalent EXYZ constraint
  for( int i=0; i < rm.ExyzGroups.Count(); i++ )  {
    TExyzGroup& sr = rm.ExyzGroups[i];
    if( sr.IsEmpty() )  continue;
    Tmp = "EXYZ";
    for( int j=0; j < sr.Count(); j++ )  {
      if( sr[j].IsDeleted() )  continue;
      Tmp << ' ' << sr[j].GetLabel();
    }
    HypernateIns(Tmp, SL);
  }
  // store the rest of eqiv ...
  for( int i=0; i < rm.UsedSymmCount(); i++ )
    StoreUsedSymIndex( usedSymm, &rm.GetUsedSymm(i), rm);
  // save
  for( int i=0; i < usedSymm.Count(); i++ )  {
    Tmp = "EQIV ";
    Tmp << '$' << (i+1) << ' ' << TSymmParser::MatrixToSymm( rm.GetUsedSymm(usedSymm[i]) );
    SL.Insert(oindex+i, Tmp  );
  }
  for( int i=0; i < rm.Vars.EquationCount(); i++ )  {
    if( !rm.Vars.GetEquation(i).Validate() )  continue;
    SL.Add("SUMP ") << rm.Vars.GetSUMPStr(i);
    //if( processed != NULL )  
    //  processed->equations.Add( &rm.Vars.GetEquation(i) );
  }
  SL.Add(EmptyString);
  if( atoms == NULL )  {
    for( int i=0; i < rm.FragCount(); i++ )
      rm.GetFrag(i).ToStrings(SL);
  }
  else  {
    SortedPtrList<Fragment, TPointerComparator> saved; 
    for( int i=0; i < atoms->Count(); i++ )  {
      const int m = TAfixGroup::GetM( (*atoms)[i]->GetAfix() );
      if( m < 17 )  continue;
      Fragment* frag = rm.FindFragByCode(m);
      if( frag == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "could not locate the FRAG for fitted group");
      if( saved.IndexOf(frag) != -1 )  continue;
      saved.Add(frag);
      frag->ToStrings(SL);
    }
  }
  SL.Add(EmptyString);
}
//..............................................................................
void TIns::ValidateRestraintsAtomNames(RefinementModel& rm)  {
  // fixed distances
  TPtrList<TSRestraintList> restraints;
  restraints.Add( &rm.rDFIX ); 
  restraints.Add( &rm.rSADI ); 
  restraints.Add( &rm.rDANG ); 
  restraints.Add( &rm.rCHIV ); 
  restraints.Add( &rm.rFLAT ); 
  restraints.Add( &rm.rDELU ); 
  restraints.Add( &rm.rSIMU ); 
  restraints.Add( &rm.rISOR ); 
  restraints.Add( &rm.rEADP ); 
  for( int i=0; i < restraints.Count(); i++ )  {
    TSRestraintList& srl = *restraints[i];
    for( int j=0; j < srl.Count(); j++ )  {
      TSimpleRestraint& sr = srl[j];
      for( int k=0; k < sr.AtomCount(); k++ )
        sr.GetAtom(k).GetAtom()->Label() = rm.aunit.ValidateLabel(sr.GetAtom(k).GetAtom()->GetLabel());   
    }
  }
  // equivalent EXYZ constraint
  for( int i=0; i < rm.ExyzGroups.Count(); i++ )  {
    TExyzGroup& sr = rm.ExyzGroups[i];
    for( int j=0; j < sr.Count(); j++ )
      sr[j].Label() = rm.aunit.ValidateLabel(sr[j].GetLabel());   
  }
}
//..............................................................................
void TIns::ClearIns()  {
  for( int i=0; i < Ins.Count(); i++ )
    delete Ins.Object(i);
  Ins.Clear();
}
//..............................................................................
bool TIns::AddIns(const olxstr& Params, RefinementModel& rm)  {
  TStrList toks(Params, ' ');
  return AddIns(toks, rm);
}
//..............................................................................
void TIns::_SaveSizeTemp(TStrList& SL)  {
  vec3d size( RefMod.expl.GetCrystalSize() );
  if( !size.IsNull() )
    SL.Add("SIZE ") << size[0] << ' ' << size[1] << ' ' << size[2];
  if( RefMod.expl.GetTemperature() != 0 )
    SL.Add("TEMP ") << RefMod.expl.GetTemperature();
}
//..............................................................................
void TIns::SaveHeader(TStrList& SL, bool ValidateRestraintNames, int* SfacIndex, int* UnitIndex)  {
  SL.Add("TITL ") << GetTitle();
  SL.Add( _CellToString() );
  SL.Add( _ZerrToString() );
  _SaveSymm(SL);
  if( SfacIndex != NULL )  *SfacIndex = SL.Count();  
  SL.Add("SFAC ") << Sfac;
  if( SfacIndex == NULL )  {
    for( int i=0; i < Disp.Count(); i++ )
      SL.Add("DISP ") << Disp[i];
  }
  if( UnitIndex != NULL )  *UnitIndex = SL.Count();  
  SL.Add("UNIT ") << Unit;
  if( ValidateRestraintNames )
    ValidateRestraintsAtomNames( GetRM() );
  SaveRestraints(SL, NULL, NULL, GetRM());
  _SaveRefMethod(SL);
  _SaveSizeTemp(SL);
  // copy "unknown" instructions except rems
  for( int i=0; i < Ins.Count(); i++ )  {
    TInsList* L = Ins.Object(i);
    if( L == NULL )  continue;  // if load failed
    // skip rems and print them at the end
    //if( Ins[i].StartsFrom("REM") )  continue;
    olxstr tmp = L->IsEmpty() ? EmptyString : L->Text(' ');
    HypernateIns(Ins[i]+' ', tmp , SL);
  }
  SL << Skipped;
//  for( int i=0; i < Skipepd.Count(); i++ )

  _SaveHklInfo(SL);

  olxstr& wght = SL.Add("WGHT ");
  for( int i=0; i < RefMod.used_weight.Count(); i++ )  {
    wght << RefMod.used_weight[i];
    if( i+1 < RefMod.used_weight.Count() )
      wght << ' ';
  }
  if( RefMod.used_weight.Count() == 0 )  
    wght << "0.1";
  _SaveFVar(RefMod, SL);
}
//..............................................................................
void TIns::ParseHeader(const TStrList& in)  {
  // clear all but the atoms
  for( int i=0; i < Ins.Count(); i++ )
    delete Ins.Object(i);
  Ins.Clear();
  Sfac = EmptyString;
  Unit = EmptyString;
  Skipped.Clear();
  Title = EmptyString;
  Disp.Clear();
  GetRM().Clear();
  GetAsymmUnit().ClearMatrices();
// end clear, start parsing
  olxstr Tmp;
  TStrList toks, lst(in);
  lst.CombineLines("=");
  ParseContext cx(GetRM());
  for( int i=0; i < lst.Count(); i++ )  {
    Tmp = olxstr::DeleteSequencesOf<char>(lst[i], ' ');
    if( Tmp.IsEmpty() )      continue;
    for( int j=0; j < Tmp.Length(); j++ )  {
      if( Tmp[j] == '!' )  {  // comment sign
        Tmp.SetLength(j-1);
        break;
      }
    }
    toks.Clear();
    toks.Strtok(Tmp, ' ');
    if( toks.IsEmpty() )  continue;

    if( ParseIns(lst, toks, cx, i) )
      continue;
    else
      Ins.Add(lst[i]);
  }
  smatd sm;
  for( int i=0; i < cx.Symm.Count(); i++ )  {
    if( TSymmParser::SymmToMatrix(cx.Symm[i], sm) )
      GetAsymmUnit().AddMatrix(sm);
  }
  Ins.Pack();
  ParseRestraints(Ins, cx);
  Ins.Pack();
  _FinishParsing(cx);
}
//..............................................................................

