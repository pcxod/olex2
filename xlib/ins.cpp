//---------------------------------------------------------------------------//
// TXFiles: TIns - basic procedures for the SHELX instruction files
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
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
#include "infotab.h"
#include "catomlist.h"

#undef AddAtom
#undef GetObject
#undef Object

//----------------------------------------------------------------------------//
// TIns function bodies
//----------------------------------------------------------------------------//
TIns::TIns()  {  LoadQPeaks = true;  }
//..............................................................................
TIns::~TIns()  {  Clear();  }
//..............................................................................
void TIns::Clear()  {
  GetRM().Clear(rm_clear_ALL);
  GetAsymmUnit().Clear();
  for( size_t i=0; i < Ins.Count(); i++ )
    delete Ins.GetObject(i);
  Ins.Clear();
  Skipped.Clear();
  Title = EmptyString;
  R1 = -1;
}
//..............................................................................
void TIns::LoadFromFile(const olxstr& fileName)  {
  Lst.Clear();
  // load Lst first, as it may have the error indicator
  olxstr lst_fn = TEFile::ChangeFileExt(fileName, "lst");
  if( TEFile::Exists(lst_fn) )  {
    try  {  Lst.LoadFromFile(lst_fn);  }
    catch(...)  {}
  }
  TBasicCFile::LoadFromFile(fileName);
}
//..............................................................................
void TIns::LoadFromStrings(const TStrList& FileContent)  {
  Clear();
  ParseContext cx(GetRM());
  TStrList Toks, InsFile(FileContent);
  for( size_t i=0; i < InsFile.Count(); i++ )  {
    InsFile[i].Trim(' ').\
      Trim('\0').\
      Replace('\t', ' ').
      DeleteSequencesOf(' ').\
      Trim(' ');
  }
  Preprocess(InsFile);
  cm_Element& elmQPeak = XElementLib::GetByIndex(iQPeakIndex);
  cx.Resi = &GetAsymmUnit().GetResidue(0);
  cx.ins = this;
  for( size_t i=0; i < InsFile.Count(); i++ )  {
    try  {
      if( InsFile[i].IsEmpty() )      continue;
      const size_t exi = InsFile[i].IndexOf('!');
      if( exi != InvalidIndex )
        InsFile[i].SetLength(exi);

      Toks.Clear();
      Toks.Strtok(InsFile[i], ' ');
      if( Toks.IsEmpty() )  continue;

      if( Toks[0].Equalsi("MOLE") )  // these are dodgy
        continue;
      else if( ParseIns(InsFile, Toks, cx, i) )
        continue;
      else if( Toks[0].Equalsi("END") )  {   //reset RESI to default
        cx.End = true;  
        cx.Resi = &GetAsymmUnit().GetResidue(0);
        cx.AfixGroups.Clear();
        cx.Part = 0;
      }
      else if( Toks[0].Equalsi("TITL") )
        SetTitle(Toks.Text(' ', 1));
      else if( Toks.Count() < 6 || Toks.Count() > 12 )  // atom sgould have at least 7 parameters
        Ins.Add(InsFile[i]);
      else {
        bool qpeak = olxstr::o_toupper(Toks[0].CharAt(0)) == 'Q';
        if( qpeak && !cx.End && !LoadQPeaks )  continue;
        if( cx.End && !qpeak )  continue;
        // is a valid atom
        //if( !atomsInfo.IsAtom(Toks[0]))  {  Ins.Add(InsFile[i]);  continue;  }
        if( !Toks[1].IsUInt() )  {
          Ins.Add(InsFile[i]);
          continue;
        }
        uint32_t index = Toks[1].ToUInt();
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
        atom->SetLabel(Toks[0], false);
        if( qpeak ) 
          atom->SetType(elmQPeak);
        else
          atom->SetType(*cx.BasicAtoms.GetObject(Toks[1].ToInt()-1));
        if( atom->GetType().GetMr() > 3.5 )
          cx.LastNonH = atom;
        _ProcessAfix(*atom, cx);
      }
    }
    catch(const TExceptionBase& exc)  {
      throw TFunctionFailedException(__OlxSourceInfo, exc,
        olxstr("at line #") << i+1 << " ('" << InsFile[i] << "')");
    }
  }
  smatd sm;
  for( size_t i=0; i < cx.Symm.Count(); i++ )  {
    if( TSymmParser::SymmToMatrix(cx.Symm[i], sm) )
      GetAsymmUnit().AddMatrix(sm);
  }
  // remove dublicated instructtions, rems ONLY
  for( size_t i = 0; i < Ins.Count(); i++ )  {
    if( Ins[i].IsEmpty() || !Ins[i].StartsFromi("REM"))  continue;
    for( size_t j = i+1; j < Ins.Count(); j++ )  {
      if( Ins[i] == Ins[j] )
        Ins[j] = EmptyString;
    }
  }

  Ins.Pack();
  ParseRestraints(cx.rm, Ins);
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
  for( size_t i=0; i < cx.Same.Count(); i++ )  {
    TCAtom* ca = cx.Same[i].B();
    TStrList& sl = cx.Same[i].A();
    if( ca == NULL ) // should not happen
      continue;
    TSameGroup& sg = sgl.New();  // main, reference, group
    size_t max_atoms = 0;
    for( size_t j=0; j < sl.Count(); j++ )  {
       toks.Clear();
       toks.Strtok(sl[j], ' ');
       size_t resi_ind = toks[0].IndexOf('_');
       olxstr resi( (resi_ind != InvalidIndex) ? toks[0].SubStringFrom(resi_ind+1) : EmptyString );
       double esd1=0.02, esd2=0.04;
       size_t from_ind = 0;
       if( toks.Count() > 0 && toks[0].IsNumber() )  {
         esd1 = toks[0].ToDouble(); 
         from_ind++;
       }
       if( toks.Count() > 1 && toks[1].IsNumber() )  {
         esd2 = toks[1].ToDouble(); 
         from_ind++;
       }
       TAtomReference ar( toks.Text(' ', from_ind) );
       TCAtomGroup ag;
       size_t atomAGroup;
       try  {  ar.Expand( cx.rm, ag, resi, atomAGroup);  }
       catch( const TExceptionBase& ex )  {
         throw TFunctionFailedException(__OlxSourceInfo, olxstr("invalid SAME instruction :") << ex.GetException()->GetError());
       }
       if( ag.IsEmpty() )  {
         TBasicApp::NewLogEntry(logError) << "Invalid SAME atom list, removed: " << toks.Text(' ');
         //throw TFunctionFailedException(__OlxSourceInfo, "empty SAME atoms list");
       }
       else  {
         TSameGroup& sg1 = sgl.NewDependent(sg);
         for( size_t k=0; k < ag.Count(); k++ )
           sg1.Add( *ag[k].GetAtom() );
         sg1.Esd12 = esd1;
         sg1.Esd13 = esd2;
         if( ag.Count() > max_atoms )
           max_atoms = ag.Count();
       }
    }
    if( sg.DependentCount() == 0 )  {
      sgl.Release(sg);
      delete &sg;
    }
    else  {
      // now process the reference group
      for( size_t j=0; j < max_atoms; j++ )  {
        if( ca->GetId() + j >= cx.au.AtomCount() )
          throw TFunctionFailedException(__OlxSourceInfo, "not enough atoms to create the reference group for SAME");
        TCAtom& a = cx.au.GetAtom(ca->GetId() + j);
        if( a.GetType() == iHydrogenZ )  {
          max_atoms++;
          continue;
        }
        sg.Add(a);
      }
    }
#ifdef _DEBUG
    for( size_t j=0; j < sg.Count(); j++ )  {
      olxstr s(sg[j].GetLabel());
      s << ':';
      for( size_t k=0; k < sg.DependentCount(); k++ )  {
        if( sg.GetDependent(k).Count() > j )
          s << sg.GetDependent(k)[j].GetLabel() << ' ';
      }
      TBasicApp::NewLogEntry() << s;
    }
#endif
  }
}
//..............................................................................
void TIns::__ProcessConn(ParseContext& cx)  {
  TStrList toks;
  for( size_t i=0; i < Ins.Count(); i++ )  {
    if( Ins[i].IsEmpty() )  // should not happen, but
      continue;
    toks.Clear();
    toks.Strtok(Ins[i], ' ');
    if( toks[0].Equalsi("CONN") )  {
      TStrList sl(toks.SubListFrom(1));
      cx.rm.Conn.ProcessConn(sl);
      Ins[i] = EmptyString;
    }
    else if( toks[0].Equalsi("FREE") )  {
      cx.rm.Conn.ProcessFree(toks.SubListFrom(1));
      Ins[i] = EmptyString;
    }
    else if( toks[0].Equalsi("BIND") )  {
      cx.rm.Conn.ProcessBind(toks.SubListFrom(1));
      Ins[i] = EmptyString;
    }
  }
  Ins.Pack();
}
//..............................................................................
void TIns::_FinishParsing(ParseContext& cx)  {
  __ProcessConn(cx);
  for( size_t i=0; i < Ins.Count(); i++ )  {
    TStrList toks(Ins[i], ' ');
    if( (toks[0].StartsFromi("HTAB") || toks[0].StartsFromi("RTAB") || toks[0].StartsFromi("MPLA")) && 
      toks.Count() > 2 )
    {
      cx.rm.AddInfoTab(toks);
      Ins.Delete(i--);
    }
    else if( toks[0].StartsFromi("ANIS") )  {
      Ins.Delete(i--);
      try  {
        double Q[6];
        memset(&Q[0], 0, sizeof(Q));
        AtomRefList rl(cx.rm, toks.Text(' ', 1));
        TTypeList<TAtomRefList> atoms;
        rl.Expand(cx.rm, atoms);
        for( size_t j=0; j < atoms.Count(); j++ )  {
          for( size_t k=0; k < atoms[j].Count(); k++ )  {
            TCAtom& ca = atoms[j][k].GetAtom();
            if( ca.GetEllipsoid() == NULL )  {
              Q[0] = Q[1] = Q[2] = ca.GetUiso();
              ca.UpdateEllp(Q);
            }
          }
        }
      }
      catch(const TExceptionBase& e)  {
        TBasicApp::NewLogEntry(logError) << e.GetException()->GetFullMessage();
      }
    }
    else  {
      TInsList* Param = new TInsList(toks);
      Ins.GetObject(i) = Param;
      Ins[i] = Param->GetString(0);
      Param->Delete(0);
      for( size_t j=0; j < Param->Count(); j++ )
        Param->GetObject(j) = GetAsymmUnit().FindCAtom(Param->GetString(j));
    }
  }
  cx.rm.Vars.Validate();
  cx.rm.ProcessFrags();
}
//..............................................................................
void TIns::_ProcessAfix0(ParseContext& cx)  {
  if( !cx.AfixGroups.IsEmpty() )  {
    int old_m = cx.AfixGroups.Current().GetB()->GetM();
    if( cx.AfixGroups.Current().GetA() > 0 )  {
      if( old_m != 0 )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("incomplete AFIX group") <<
        (cx.Last != NULL ? (olxstr(" at ") << cx.Last->GetLabel()) : EmptyString) );
      else
        TBasicApp::NewLogEntry(logWarning) << "Possibly incorrect AFIX " << cx.AfixGroups.Current().GetB()->GetAfix() <<
        (cx.Last != NULL ? (olxstr(" at ") << cx.Last->GetLabel()) : EmptyString);
    }
    if( cx.AfixGroups.Current().GetB()->GetPivot() == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "undefined pivot atom for a fitted group");
    while( !cx.AfixGroups.IsEmpty() && cx.AfixGroups.Current().GetA() <= 0 )  // pop all out  
      cx.AfixGroups.Pop();
  }
}
//..............................................................................
bool TIns::ParseIns(const TStrList& ins, const TStrList& Toks, ParseContext& cx, size_t& i)  {
  if( _ParseIns(cx.rm, Toks) )
    return true;
  else if( !cx.CellFound && Toks[0].Equalsi("CELL") )  {
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
  else if( Toks[0].Equalsi("SYMM") && (Toks.Count() > 1))
    cx.Symm.Add( Toks.Text(EmptyString, 1) );
  else if( Toks[0].Equalsi("FRAG") && (Toks.Count() > 1))  {
   int code = Toks[1].ToInt();
    if( code < 17 )
      throw TInvalidArgumentException(__OlxSourceInfo, "FRAG code must be greater than 16");
    double a=1, b=1, c=1, al=90, be=90, ga=90;
    XLibMacros::ParseOnlyNumbers<double, TStrList>(Toks, 6, 2, &a, &b, &c, &al, &be, &ga);
    Fragment* frag = cx.rm.FindFragByCode(code);
    if( frag == NULL )
      frag = &cx.rm.AddFrag(Toks[1].ToInt(), a, b, c, al, be, ga);
    else
      frag->Reset(a, b, c, al, be, ga);
    TStrList f_toks;
    while( ++i < ins.Count() && !ins[i].StartsFromi("FEND") )  {
      if( ins[i].IsEmpty() )  continue;
      f_toks.Strtok(ins[i], ' ');
      if( f_toks.Count() > 4 )
        frag->Add(f_toks[0], f_toks[2].ToDouble(), f_toks[3].ToDouble(), f_toks[4].ToDouble());
      else
        throw TFunctionFailedException(__OlxSourceInfo, "invalid FRAG atom");
      f_toks.Clear();
    }
  }
  else if( Toks[0].Equalsi("PART") && (Toks.Count() > 1) )  {
    cx.Part = (short)Toks[1].ToInt();
    if( cx.Part == 0 )  cx.PartOccu = 0;
    if( Toks.Count() == 3 )
      cx.PartOccu = Toks[2].ToDouble();
    // TODO: validate if appropriate here...
    //_ProcessAfix0(cx);
  }
  else if( Toks[0].Equalsi("AFIX") && (Toks.Count() > 1) )  {
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
    if( afix == 0 )
      _ProcessAfix0(cx);
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
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>((int)frag->Count()-1, afixg, false) );
          cx.SetNextPivot = true;
        }
        else if( m == 0 && !TAfixGroup::IsDependent(afix) )  {  // generic container then, beside, 5 is dependent atom of rigid group
          cx.AfixGroups.Push( AnAssociation3<int,TAfixGroup*,bool>(-1, afixg, false) );
          cx.SetNextPivot = !TAfixGroup::IsRiding(afix); // if not riding
        }
        if( !cx.SetNextPivot )  {
          if( cx.LastNonH == NULL  )
            throw TFunctionFailedException(__OlxSourceInfo, "undefined pivot atom for a fitted group");
          // have to check if several afixes for one atom (if the last is H)
          afixg->SetPivot(*cx.LastNonH);
        }
      }
    }
  }
  else if( Toks[0].Equalsi("RESI") )  {
    if( Toks.Count() < 3 )
      throw TInvalidArgumentException(__OlxSourceInfo, "wrong number of arguments for a residue");
    if( Toks[1].IsNumber() )
      cx.Resi = &cx.au.NewResidue(EmptyString, Toks[1].ToInt(), (Toks.Count() > 2) ? Toks[2] : EmptyString);
    else
      cx.Resi = &cx.au.NewResidue(Toks[1], Toks[2].ToInt(), (Toks.Count() > 3) ? Toks[3] : EmptyString);
  }
  else if( Toks[0].Equalsi("SFAC") )  {
    bool expandedSfacProcessed = false;
    if( Toks.Count() == 16 )  {  // a special case for expanded sfac
      int NumberCount = 0;
      for( size_t i=2; i < Toks.Count(); i++ )  {
        if( Toks[i].IsNumber() )
          NumberCount++;
      }
      if( NumberCount > 0 && NumberCount < 14 )  {
        TBasicApp::NewLogEntry(logError) << "Possibly not well formed SFAC " << Toks[0];
      }
      else  if( NumberCount == 14 )  {
        /* here we do not check if the Toks.String(1) is atom - itcould be a label ...
        so we keep it as it is to save in the ins file
        */
        cx.rm.AddUserContent(Toks[1]);
        cx.BasicAtoms.Add(Toks[1], XElementLib::FindBySymbol(Toks[1]));
        if( cx.BasicAtoms.GetLast().Object == NULL )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not find suitable scatterer for '") << Toks[1] << '\'' );
        expandedSfacProcessed = true;
        const olxstr lb(Toks[1].CharAt(0) == '$' ? Toks[1].SubStringFrom(1) : Toks[1]);
        XScatterer* sc = new XScatterer(lb);
        sc->SetGaussians(
          cm_Gaussians(
            Toks[2].ToDouble(), Toks[3].ToDouble(), Toks[4].ToDouble(), Toks[5].ToDouble(),
            Toks[6].ToDouble(), Toks[7].ToDouble(), Toks[8].ToDouble(), Toks[9].ToDouble(),
            Toks[10].ToDouble())
          );
        sc->SetFpFdp(compd(Toks[11].ToDouble(), Toks[12].ToDouble()));
        sc->SetMu(Toks[13].ToDouble());
        sc->SetR(Toks[14].ToDouble());
        sc->SetWeight(Toks[15].ToDouble());
        cx.rm.AddSfac(*sc);
      }
    }
    if( !expandedSfacProcessed )  {
      for( size_t j=1; j < Toks.Count(); j++ )  {
        if( XElementLib::IsElement(Toks[j]) )  {
          cx.BasicAtoms.Add(Toks[j], XElementLib::FindBySymbol(Toks[j]));
          if( cx.BasicAtoms.GetLast().Object == NULL )
            throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not find suitable scatterer for '") << Toks[j] << '\'' );
          cx.rm.AddUserContent(Toks[j]);
        }
      }
    }
  }
  else if( Toks[0].Equalsi("DISP") && Toks.Count() >= 4 )  {
    const olxstr lb(Toks[1].CharAt(0) == '$' ? Toks[1].SubStringFrom(1) : Toks[1]);
    XScatterer* sc = new XScatterer(lb);
    sc->SetFpFdp(compd(Toks[2].ToDouble(), Toks[3].ToDouble()));
    if( Toks.Count() >= 5 )
      sc->SetMu(Toks[4].ToDouble());
    cx.rm.AddSfac(*sc);
  }
  else if( Toks[0].Equalsi("REM") )  {  
    if( Toks.Count() > 1 )  {
      if( Toks[1].Equalsi("R1") && Toks.Count() > 4 && Toks[3].IsNumber() )  {
        if( cx.ins != NULL )  cx.ins->R1 = Toks[3].ToDouble();
      }
      else if( Toks[1].Equalsi("olex2.stop_parsing") )  {
        while( i < ins.Count() )  {
          if( cx.ins != NULL )
            cx.ins->Skipped.Add(ins[i]);
          if( ins[i].StartsFromi("REM") && ins[i].IndexOf("olex2.resume_parsing") != InvalidIndex ) 
            break;
          i++;
        }
      } 
      else if( Toks[1].StartsFromi("<HKL>") )  {
        olxstr hklsrc = Toks.Text(' ', 1);
        size_t index = hklsrc.FirstIndexOf('>');
        size_t iv = hklsrc.IndexOf("</HKL>");
        if( iv == InvalidIndex )  {
          while( (i+1) < ins.Count() )  {
            i++;
            if( !ins[i].StartsFromi("rem") )  break;
            hklsrc << ins[i].SubStringFrom(4);
            iv = hklsrc.IndexOf("</HKL>");
            if( iv != InvalidIndex )  break;
          }
        }
        if( iv != InvalidIndex )
          hklsrc = hklsrc.SubString(index+1, iv-index-1).Replace("%20", ' ');
        else
          hklsrc = EmptyString;
        cx.rm.SetHKLSource(hklsrc);
      }
      else if( !cx.End  && !cx.rm.IsHKLFSet() )
        if( cx.ins != NULL )
          cx.ins->Ins.Add(Toks.Text(' ')); 
    }
  }
  else if( Toks[0].Equalsi("SAME") )  {
    if( !cx.Same.IsEmpty() && cx.Same.GetLast().GetB() == NULL )  // no atom so far, add to the list of Same
      cx.Same.GetLast().A().Add(Toks.Text(' ', 1));
    else  {
      cx.Same.Add(new AnAssociation2<TStrList,TCAtom*>);
      cx.Same.GetLast().B() = NULL;
      cx.Same.GetLast().A().Add(Toks.Text(' ', 1));
    }
  }
  else if( Toks[0].Equalsi("ANIS") )  {
    if( Toks.Count() == 2 && Toks[1].IsNumber() )
      cx.ToAnis = olx_abs(Toks[1].ToInt());
    else
      return false;
  }
  else
    return false;
  return true;
}
//..............................................................................
void TIns::UpdateParams()  {
  for( size_t i =0; i < Ins.Count(); i++ )  {
    if( Ins.GetObject(i) == NULL )  continue;  // might happen if load failed
    for( size_t j=0; j < Ins.GetObject(i)->Count(); j++ )  {
      if( Ins.GetObject(i)->GetObject(j) != NULL )
        Ins.GetObject(i)->GetString(j) = Ins.GetObject(i)->GetObject(j)->GetLabel();
    }
  }
}
//..............................................................................
void TIns::DelIns(size_t i)  {
  delete Ins.GetObject(i);
  Ins.Delete(i);
}
//..............................................................................
TInsList* TIns::FindIns(const olxstr &Name)  {
  size_t i = Ins.IndexOfi(Name);
  return i != InvalidIndex ? Ins.GetObject(i) : NULL;
}
//..............................................................................
bool TIns::InsExists(const olxstr &Name)  {
  return FindIns(Name) != NULL;
}
//..............................................................................
bool TIns::AddIns(const TStrList& toks, RefinementModel& rm, bool CheckUniq)  {
  // special instructions
  if( _ParseIns(rm, toks) || ParseRestraint(rm, toks) )  return true;
  // check for uniqueness
  if( CheckUniq )  {
    for( size_t i=0; i < Ins.Count(); i++ )  {
      if( Ins[i].Equalsi(toks[0]) )  {
        TInsList *ps = Ins.GetObject(i);
        if( ps->Count() == (toks.Count()-1) )  {
          bool unique = false;
          for( size_t j=0; j < ps->Count(); j++ )  {
            if( !ps->GetString(j).Equalsi(toks[j+1]) )  {
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
  for( size_t i=1; i < toks.Count(); i++ )  {
    Params[i-1] = toks[i];
    Params.GetObject(i-1) = GetAsymmUnit().FindCAtom(toks[i]);
  }
  // end
  Ins.Add(toks[0], &Params);
  return true;
}
//..............................................................................
void TIns::HyphenateIns(const olxstr &InsName, const olxstr &Ins, TStrList &Res)  {
  olxstr Tmp = Ins, Tmp1;
  if( Tmp.Length() > 80 )  {
    while ( Tmp.Length() > 80 )  {
      size_t spindex = Tmp.LastIndexOf(' ', 80-InsName.Length()-2);
      if( spindex != InvalidIndex && spindex > 0 )  {
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
void TIns::HyphenateIns(const olxstr& Ins, TStrList& Res)  {
  bool MultiLine = false, added = false;
  olxstr Tmp(Ins), Tmp1;
  while( Tmp.Length() > 79 )  {
    MultiLine = true;
    size_t spindex = Tmp.LastIndexOf(' ', 77); // for the right hypernation
    if( spindex != InvalidIndex && spindex > 0 )  {
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
void TIns::SaveForSolution(const olxstr& FileName, const olxstr& sMethod, const olxstr& comments, bool rems)  {
  TStrList SL, mtoks;
  if( sMethod.IsEmpty() )
    mtoks.Add("TREF");
  else  {
    mtoks.Strtok(sMethod, "\\n");
    size_t spi = mtoks[0].IndexOf(' ');
    if( spi != InvalidIndex )
      RefMod.SetSolutionMethod(mtoks[0].SubStringTo(spi));
    else
      RefMod.SetSolutionMethod(mtoks[0]);
  }

  UpdateParams();
  SL.Add("TITL ") << GetTitle();

  if( !comments.IsEmpty() && rems ) 
    SL.Add("REM ") << comments;
// try to estimate Z'
  SL.Add( _CellToString() );
  SL.Add( _ZerrToString() );
  _SaveSymm(SL);
  SL.Add(EmptyString);
  SaveSfacUnit(RefMod, RefMod.GetUserContent(), SL, SL.Count()-1);

  _SaveSizeTemp(SL);
  if( rems )
    _SaveHklInfo(SL, true);
  //_SaveFVar(RefMod, SL);

  SL.AddList(mtoks);
  SL.Add(EmptyString);
  SL.Add("HKLF ") << RefMod.GetHKLFStr();
  SL.Add("END");
#ifdef _UNICODE
  TCStrList(SL).SaveToFile(FileName);
#else
  SL.SaveToFile(FileName);
#endif
}
//..............................................................................
void TIns::SaveSfacUnit(const RefinementModel& rm, const ContentList& content,
                     TStrList& list, size_t pos)
{
  TStrList lines;
  short state = 0;
  for( size_t i=0; i < rm.GetUserContent().Count(); i++ )  {
    XScatterer* sd = rm.FindSfacData(rm.GetUserContent()[i].element.symbol);
    if( sd != NULL && sd->IsSFAC() )  {
      olxstr tmp = sd->ToInsString();
      lines.Clear();
      HyphenateIns(tmp, lines);
      for( size_t j=0; j < lines.Count(); j++ )
        list.Insert(pos++, lines[j]);
      state = 1;
    }
    else  {
      if( state == 1 )  {
        list.Insert(pos++,  olxstr("SFAC ") << rm.GetUserContent()[i].element.symbol);
        state = 2;
      }
      else  {
        if( state == 2 )  // SFAC added and pos incremented
          list[pos-1] << ' ' << rm.GetUserContent()[i].element.symbol;
        else if( state == 0 )  {  // nothing added yet
          list[pos++] << "SFAC " << rm.GetUserContent()[i].element.symbol;
          state = 2;
        }
      }
    }
  }
  for( size_t i=0; i < rm.SfacCount(); i++ )  {
    if( rm.GetSfacData(i).IsDISP() )
      list.Insert(pos++, rm.GetSfacData(i).ToInsString());
  }
  olxstr& unit = list.Insert(pos++, "UNIT");

  if( rm.SfacCount() == 0 )  {
    for( size_t i=0; i < content.Count(); i++ )
      unit << ' ' << content[i].count;
  }
  else  {
    for( size_t i=0; i < rm.GetUserContent().Count(); i++ )
      unit << ' ' << rm.GetUserContent()[i].count;
  }
}
//..............................................................................
void TIns::_SaveAtom(RefinementModel& rm, TCAtom& a, int& part, int& afix, 
  TStrPObjList<olxstr,const cm_Element*>* sfac, TStrList& sl,
  TIndexList* index, bool checkSame)
{
  if( a.IsDeleted() || a.IsSaved() )  return;
  if( checkSame && olx_is_valid_index(a.GetSameId()) )  {  // "
    TSameGroup& sg = rm.rSAME[a.GetSameId()];
    if( sg.IsValidForSave() )  {
      for( size_t i=0; i < sg.DependentCount(); i++ )  {
        if( !sg.GetDependent(i).IsValidForSave() )
          continue;
        olxstr tmp("SAME ");
        tmp << olxstr(sg.GetDependent(i).Esd12).TrimFloat() << ' ' 
            << olxstr(sg.GetDependent(i).Esd13).TrimFloat();
        for( size_t j=0; j < sg.GetDependent(i).Count(); j++ )
          tmp << ' ' << sg.GetDependent(i)[j].GetResiLabel();
        HyphenateIns( tmp, sl );
      }
      for( size_t i=0; i < sg.Count(); i++ )
        _SaveAtom(rm, sg[i], part, afix, sfac, sl, index, false);
      return;
    }
  }
  if( a.GetPart() != part )  {
    if( part != 0 && a.GetPart() != 0 )
      sl.Add("PART 0");
    sl.Add("PART ") << (int)a.GetPart();
  }
  TAfixGroup* ag = a.GetDependentAfixGroup();
  int atom_afix = a.GetAfix();
  if( atom_afix != afix || afix == 1 || afix == 2 )  { 
    if( !TAfixGroup::HasExcplicitPivot(afix) || !TAfixGroup::IsDependent(atom_afix) )  {
      TAfixGroup* _ag = a.GetParentAfixGroup();
      if( _ag != NULL )  {
        olxstr& str = sl.Add("AFIX ") << atom_afix;
        if( _ag->GetD() != 0 )  {
          str << ' ' << _ag->GetD();
        }
        if( _ag->GetSof() != 0 )  {
          str << ' ' << _ag->GetSof();
          if( _ag->GetU() != 0 )
            str << ' ' << _ag->GetU();
        }
      }
      else
        sl.Add("AFIX ") << atom_afix;    
    }
  }
  afix = atom_afix;
  part = a.GetPart();
  index_t spindex;
  if( a.GetType() == iQPeakZ )
    spindex = (sfac == NULL ? -2 : (index_t)sfac->IndexOfi('c')+1);
  else
    spindex = (sfac == NULL ? -2 : (index_t)sfac->IndexOfObject(&a.GetType())+1);
  HyphenateIns(_AtomToString(rm, a, spindex == 0 ? 1 : spindex), sl);
  a.SetSaved(true);
  if( index != NULL )  index->Add(a.GetTag());
  for( size_t i=0; i < a.DependentHfixGroupCount(); i++ )  {
    TAfixGroup& hg = a.GetDependentHfixGroup(i);
    size_t sc = 0;
    for( size_t j=0; j < hg.Count(); j++ )  {
      if( !hg[j].IsDeleted() && !hg[j].IsSaved() )  {
        _SaveAtom(rm, hg[j], part, afix, sfac, sl, index, checkSame);
        sc++;
      }
    }
    if( sc != 0 )  {
      sl.Add("AFIX 0");
      afix = 0;
    }
  }
  if( ag != NULL )  {  // save dependent rigid group
    size_t sc = 0;
    for( size_t i=0; i < ag->Count(); i++ )  {
      if( !(*ag)[i].IsDeleted() && !(*ag)[i].IsSaved() )  {
        _SaveAtom(rm, (*ag)[i], part, afix, sfac, sl, index, checkSame);
        sc++;
      }
    }
    if( sc != 0 )  {
      sl.Add("AFIX 0");
      afix = 0;
    }
  }
}
//..............................................................................
void TIns::SaveToStrings(TStrList& SL)  {
  TStrPObjList<olxstr,const cm_Element*> BasicAtoms;
  for( size_t i=0; i < GetRM().GetUserContent().Count(); i++ )  {
    BasicAtoms.Add(GetRM().GetUserContent()[i].element.symbol, &GetRM().GetUserContent()[i].element);
  }
  for( size_t i=0; i < GetAsymmUnit().ResidueCount(); i++ )  {
    TResidue& residue = GetAsymmUnit().GetResidue(i);
    for( size_t j=0; j < residue.Count(); j++ )  {
      if( residue[j].IsDeleted() )  continue;
      residue[j].SetSaved(false);
      size_t spindex = BasicAtoms.IndexOfObject(&residue[j].GetType());  // fix the SFAC, if wrong
      if( spindex == InvalidIndex )  {
        if( residue[j].GetType() != iQPeakZ )  {
          BasicAtoms.Add(residue[j].GetType().symbol, &residue[j].GetType());
          GetRM().AddUserContent(residue[j].GetType().symbol, 1.0);
        }
      }
      if( residue[j].GetLabel().Length() > 4 ) 
        residue[j].SetLabel(GetAsymmUnit().CheckLabel(&residue[j], residue[j].GetLabel()), false);
      for( size_t k=j+1; k < residue.Count(); k++ )  {
        if( residue[k].IsDeleted() )  continue;
        if( residue[j].GetPart() != residue[k].GetPart() && 
            residue[j].GetPart() != 0 && residue[k].GetPart() != 0 )  continue;
        if( residue[j].GetLabel().Equalsi(residue[k].GetLabel()) ) 
          residue[k].SetLabel(GetAsymmUnit().CheckLabel(&residue[k], residue[k].GetLabel()), false);
      }
    }
  }
  
  ValidateRestraintsAtomNames(GetRM());
  UpdateParams();
  SaveHeader(SL, false);
  SL.Add(EmptyString);
  int afix = 0, part = 0;
  uint32_t fragmentId = ~0;
  for( size_t i=0; i < GetAsymmUnit().ResidueCount(); i++ )  {
    TResidue& residue = GetAsymmUnit().GetResidue(i);
    if( i != 0 && !residue.IsEmpty() )  { 
      SL.Add();
      SL.Add(residue.ToString());
      fragmentId = ~0;
    }
    for( size_t j=0; j < residue.Count(); j++ )  {
      TCAtom& ac = residue[j];
      if( ac.IsDeleted() || ac.IsSaved() )  continue;
      if( ac.GetFragmentId() != fragmentId || !olx_is_valid_index(fragmentId) )  {
        if( olx_is_valid_index(fragmentId) )
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
  SL.Add("END");
  SL.Add(EmptyString);
}
//..............................................................................
bool TIns::Adopt(TXFile& XF)  {
  Clear();
  GetRM().Assign(XF.GetRM(), true);
  try  {
    TSpaceGroup& sg = XF.GetLastLoaderSG();
    Title << " in " << sg.GetFullName();
  }
  catch( ... )  {}
  Title = XF.LastLoader()->GetTitle();
  if( RefMod.GetRefinementMethod().IsEmpty() )
    RefMod.SetRefinementMethod("L.S.");
  return true;
}
//..............................................................................
void TIns::UpdateAtomsFromStrings(RefinementModel& rm, TCAtomPList& CAtoms, const TIndexList& index, TStrList& SL, TStrList& Instructions) {
  if( CAtoms.Count() != index.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "index");
  if( CAtoms.IsEmpty() )  return;
  size_t atomCount = 0;
  ParseContext cx(rm);
  Preprocess(SL);
  //rm.FVAR.Clear();
  for( size_t i=0; i < CAtoms.Count(); i++ )  {
    if( CAtoms[i]->GetParentAfixGroup() != NULL )
      CAtoms[i]->GetParentAfixGroup()->Clear();
    if( CAtoms[i]->GetDependentAfixGroup() != NULL )
      CAtoms[i]->GetDependentAfixGroup()->Clear();
    if( CAtoms[i]->GetExyzGroup() != NULL )
      CAtoms[i]->GetExyzGroup()->Clear();
  }
  for( size_t i=0; i < SL.Count(); i++ )  {
    olxstr Tmp = olxstr::DeleteSequencesOf<char>(SL[i], true);
    if( Tmp.IsEmpty() )  continue;
    const size_t exi = Tmp.IndexOf('!');
    if( exi != InvalidIndex )
      Tmp.SetLength(exi);
    TStrList Toks(Tmp, ' ');
    if( Toks.IsEmpty() )  continue;
    if( Toks[0].Equalsi("REM") )  
      ;
    else if( ParseIns(SL, Toks, cx, i) )  
      ;
    else if( Toks.Count() < 6 )  // should be at least
      Instructions.Add(Tmp);
    else if( !XElementLib::IsElement(Toks[1]) )  // is a valid atom
      Instructions.Add(Tmp);
    else if( (!Toks[2].IsNumber()) || (!Toks[3].IsNumber()) || // should be four numbers
        (!Toks[4].IsNumber()) || (!Toks[5].IsNumber()) )  {
      Instructions.Add(Tmp);
    }
    else  {
      cm_Element* elm = XElementLib::FindBySymbol(Toks[1]);
      if( elm == NULL ) // wrong SFAC
        throw TInvalidArgumentException(__OlxSourceInfo, "unknown element symbol");
      TCAtom* atom = NULL;
      if( (atomCount+1) > CAtoms.Count() )  {
        if( CAtoms.GetLast()->GetParent() != NULL )
          atom = &CAtoms.GetLast()->GetParent()->NewAtom(cx.Resi);
        else
          throw TInvalidArgumentException(__OlxSourceInfo, "uninitialised data provided");
      }
      else  {
        atom = CAtoms[index[atomCount]];
        if( cx.Resi != NULL )  
          cx.Resi->Add(*atom);
      }
      _ParseAtom(Toks, cx, atom);
      atomCount++;
      atom->SetLabel(Toks[0], false);
      atom->SetType(*elm);
      if( atom->GetType().GetMr() > 3.5 )
        cx.LastNonH = atom;
      _ProcessAfix(*atom, cx);
    }
  }
  _ProcessSame(cx);
  ParseRestraints(cx.rm, Instructions);
  Instructions.Pack();
}
//..............................................................................
bool TIns::SaveAtomsToStrings(RefinementModel& rm, const TCAtomPList& CAtoms, TIndexList& index, 
                              TStrList& SL, RefinementModel::ReleasedItems* processed)  {
  if( CAtoms.IsEmpty() )  return false;
  int part = 0,
      afix = 0;
  size_t resi = InvalidIndex;
  SaveRestraints(SL, &CAtoms, processed, rm);
  _SaveFVar(rm, SL);
  for( size_t i=0; i < CAtoms.Count(); i++ )  {
    CAtoms[i]->SetSaved(false);
    CAtoms[i]->SetTag(i);
  }
  for( size_t i=0; i < CAtoms.Count(); i++ )  {
    if( CAtoms[i]->IsSaved() )  continue;
    if( CAtoms[i]->GetResiId() != resi && CAtoms[i]->GetResiId() != 0 )  {
      size_t resi = CAtoms[i]->GetResiId();
      SL.Add(rm.aunit.GetResidue(CAtoms[i]->GetResiId()).ToString());
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
  TPtrList<const cm_Element> BasicAtoms;
  for( size_t i=0; i < GetRM().GetUserContent().Count(); i++ )  {
    BasicAtoms.Add(GetRM().GetUserContent()[i].element);
  }
  TSizeList Sfacs;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    cm_Element* elm = XElementLib::FindBySymbolEx(atoms[i].GetName());
    if( elm == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Unknown element: ") << atoms[i].GetName() );
    size_t index = BasicAtoms.IndexOf(elm);
    if( index == InvalidIndex )  {
      GetRM().AddUserContent(*elm, 1.0);
      BasicAtoms.Add(elm);
      Sfacs.Add(BasicAtoms.Count()-1);
    }
    else
      Sfacs.Add(index);
  }
  TStrList SL;
  SL.Add("TITLE ") << GetTitle();
  if( !comments.IsEmpty() )
    SL.Add("REM ") << comments;

  SL.Add(_CellToString());
  SL.Add(_ZerrToString());
  _SaveSymm(SL);
  SL.Add(EmptyString);
  SaveSfacUnit(GetRM(), GetRM().GetUserContent(), SL, SL.Count()-1);

  _SaveRefMethod(SL);
  _SaveSizeTemp(SL);

  // copy "unknown" instructions except rems
  for( size_t i=0; i < Ins.Count(); i++ )  {
    // skip rems and print them at the end
    if( Ins[i].StartsFrom("REM") )  continue;
    TInsList* L = Ins.GetObject(i);
    HyphenateIns(Ins[i]+' ', L->Text(' '), SL);
  }

  _SaveHklInfo(SL, false);

  olxstr& _wght = SL.Add("WGHT ");
  for( size_t i=0; i < RefMod.used_weight.Count(); i++ )
    _wght << ' ' << RefMod.used_weight[i];
  if( RefMod.used_weight.Count() == 0 )  
    _wght << " 0.1";

  SL.Add("FVAR 1");
  SL.Add(EmptyString);
  for( size_t i=0; i < atoms.Count(); i++ )  {
    olxstr& aline = SL.Add(atoms[i].GetName());
    aline.Format(6, true, ' ');
    aline << (Sfacs[i]+1);
    aline.Format(aline.Length()+4, true, ' ');
    for( size_t j=0; j < 3; j++ )
      aline << olxstr::FormatFloat(-5, atoms[i].GetCrd()[j] ) << ' ';
    double v = atoms[i].GetOccup() + 10;
    aline << olxstr::FormatFloat(-5, v) << ' ';
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
    cx.SetNextPivot = false;
    return;
  }
  //if( cx.AfixGroups.Current().GetA() == 0 )  {
  //  cx.AfixGroups.Pop();
  //}
  //else  {
  if( cx.AfixGroups.Current().GetC() )  {
    if( a.GetType() != iHydrogenZ )  {
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
  if( atom == NULL )
    atom = &cx.au.NewAtom(cx.Resi);
  for( short j=0; j < 3; j ++ )
    cx.rm.Vars.SetParam(*atom, catom_var_name_X+j, Toks[2+j].ToDouble());
  atom->SetPart( cx.Part );
  // update the context
  cx.Last = atom;
  if( !cx.Same.IsEmpty() && cx.Same.GetLast().GetB() == NULL )
    cx.Same.GetLast().B() = atom;

  cx.rm.Vars.SetParam(*atom, catom_var_name_Sof, cx.PartOccu == 0 ? Toks[5].ToDouble() : cx.PartOccu);

  if( Toks.Count() == 12 )  {  // full ellipsoid
    for( short j=0; j < 6; j ++ )
      QE[j] = cx.rm.Vars.SetParam(*atom, catom_var_name_U11+j, Toks[j+6].ToDouble());
    cx.au.UcifToUcart(QE);
    TEllipsoid& elp = cx.au.NewEllp().Initialise(QE);
    atom->AssignEllp(&elp);
    if( atom->GetEllipsoid()->IsNPD() )  {
      TBasicApp::NewLogEntry(logInfo) << "Not positevely defined: " << Toks[0];
      atom->SetUiso(0);
    }
    else
      atom->SetUiso(atom->GetEllipsoid()->GetUiso());
    cx.LastWithU = atom;
  }
  else  {
    if( Toks.Count() > 6 )
      cx.rm.Vars.SetParam(*atom, catom_var_name_Uiso, Toks[6].ToDouble());
    else // incomplete data...
      atom->SetUiso(4*caDefIso*caDefIso);
    if( Toks.Count() >= 8 ) // some other data as Q-peak itensity
      atom->SetQPeak( Toks[7].ToDouble() );
    if( atom->GetUiso() <= -0.5 )  {  // a value fixed to the pivot atom value
      if( cx.LastWithU == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Invalid Uiso proxy for: ") << Toks[0]);
      atom->SetUisoScale(olx_abs(atom->GetUiso()));
      atom->SetUisoOwner(cx.LastWithU);
      //atom->SetUiso( 4*caDefIso*caDefIso );
      atom->SetUiso(cx.LastWithU->GetUiso()*olx_abs(atom->GetUiso()));
    }
    else  {
      atom->SetUisoOwner(NULL);
      cx.LastWithU = atom;
      if( cx.ToAnis > 0 )  {
        cx.ToAnis--;
        size_t qes = sizeof(QE);
        memset(&QE[0], 0, qes);
        QE[0] = QE[1] = QE[2] = atom->GetUiso();
        atom->UpdateEllp(QE);
      }
    }
  }
  return atom;
}
//..............................................................................
olxstr TIns::_AtomToString(RefinementModel& rm, TCAtom& CA, index_t SfacIndex)  {
  double v, Q[6];   // quadratic form of ellipsoid
  olxstr Tmp = CA.GetLabel();
  Tmp.Format(6, true, ' ');
  if( SfacIndex < 0 )
    Tmp << CA.GetType().symbol;
  else
    Tmp << SfacIndex;

  Tmp.Format(Tmp.Length()+4, true, ' ');
  for( short j=0; j < 3; j++ )
    Tmp << olxstr::FormatFloat(-5, rm.Vars.GetParam(CA, catom_var_name_X+j)) << ' ';
  
  // save occupancy
  Tmp << olxstr::FormatFloat(-5, rm.Vars.GetParam(CA, catom_var_name_Sof)) << ' ';
  // save Uiso, Uanis
  if( CA.GetEllipsoid() != NULL )  {
    CA.GetEllipsoid()->GetQuad(Q);
    rm.aunit.UcartToUcif(Q);

    for( short j = 0; j < 6; j++ )
      Tmp << olxstr::FormatFloat(-5, rm.Vars.GetParam(CA, catom_var_name_U11+j, Q[j])) << ' ';
  }
  else  {
    if( CA.GetUisoOwner() )  // riding atom
      v = -CA.GetUisoScale();
    else 
      v = rm.Vars.GetParam(CA, catom_var_name_Uiso);
    Tmp << olxstr::FormatFloat(-5, v) << ' ';
  }
  // Q-Peak
  if( CA.GetType() == iQPeakZ )
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
  HyphenateIns("FVAR ", rm.Vars.GetFVARStr(), SL);
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
    for( size_t i=0; i < GetAsymmUnit().MatrixCount(); i++ )
      SL.Add("SYMM ") << TSymmParser::MatrixToSymm( GetAsymmUnit().GetMatrix(i) );
  }
}
//..............................................................................
void TIns::_SaveRefMethod(TStrList& SL)  {
  if( !RefMod.GetRefinementMethod().IsEmpty() )  {
    if( RefMod.LS.Count() != 0 )  {
      olxstr& rm = SL.Add( RefMod.GetRefinementMethod() );
      for( size_t i=0; i < RefMod.LS.Count(); i++ )
        rm << ' ' << RefMod.LS[i];
    }
    if( RefMod.PLAN.Count() != 0 )  {
      olxstr& pn = SL.Add("PLAN ");
      for( size_t i=0; i < RefMod.PLAN.Count(); i++ )
        pn << ' ' << ((i < 1) ? olx_round(RefMod.PLAN[i]) : RefMod.PLAN[i]);
    }
  }
}
//..............................................................................
void TIns::_SaveHklInfo(TStrList& SL, bool solution)  {
  if( !solution )  {
    if( GetRM().HasMERG() )
      SL.Add("MERG ") << GetRM().GetMERG();
    if( !GetRM().GetBASF().IsEmpty() )
      SL.Add("BASF ") << GetRM().GetBASFStr();
    if( GetRM().HasSHEL() )
      SL.Add("SHEL ") << GetRM().GetSHELStr();
    if( GetRM().HasTWIN() )
      SL.Add("TWIN ") << GetRM().GetTWINStr();
  }
  if( GetRM().HasOMIT() )
    SL.Add("OMIT ") << GetRM().GetOMITStr();
  for( size_t i=0; i < GetRM().OmittedCount(); i++ )  {
    const vec3i& r = GetRM().GetOmitted(i);
    SL.Add("OMIT ") << r[0] << ' ' << r[1] << ' ' << r[2];
  }
  if( !GetRM().GetHKLSource().IsEmpty() )  // update html source string
    HyphenateIns("REM ", olxstr("<HKL>") << olxstr(GetRM().GetHKLSource()).Replace(' ', "%20")<< "</HKL>", SL);
}
//..............................................................................
bool Ins_ProcessRestraint(const TCAtomPList* atoms, TSimpleRestraint& sr)  {
  if( sr.AtomCount() == 0 && !sr.IsAllNonHAtoms() )  return false;
  if( atoms == NULL )  return true;
  for( size_t i=0; i < atoms->Count(); i++ )
    if( sr.ContainsAtom(*atoms->GetItem(i)) )
      return true;
  return false;
}
void StoreUsedSymIndex(TUIntList& il, const smatd* m, RefinementModel& rm)  {
  if( m == NULL )  return;
  unsigned int ind = (unsigned int)rm.UsedSymmIndex(*m);
  if( il.IndexOf(ind) == InvalidIndex )
    il.Add(ind);
}

void TIns::SaveRestraints(TStrList& SL, const TCAtomPList* atoms,
                          RefinementModel::ReleasedItems* processed, RefinementModel& rm)  {
  size_t oindex = SL.Count();

  olxstr Tmp;
  TUIntList usedSymm;
  // fixed distances
  for( size_t i=0; i < rm.rDFIX.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rDFIX[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "DFIX ";
    Tmp << rm.Vars.GetParam(sr, 0) << ' ' << sr.GetEsd();
    for( size_t j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HyphenateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // similar distances
  for( size_t i=0; i < rm.rSADI.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rSADI[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "SADI ";
    Tmp << sr.GetEsd();
    for( size_t j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HyphenateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // fixed "angles"
  for( size_t i=0; i < rm.rDANG.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rDANG[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "DANG ";
    Tmp << olxstr::FormatFloat(3, rm.Vars.GetParam(sr, 0)) << ' ' << sr.GetEsd();
    for( size_t j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HyphenateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // fixed chiral atomic volumes
  for( size_t i=0; i < rm.rCHIV.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rCHIV[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "CHIV ";
    Tmp << rm.Vars.GetParam(sr, 0) << ' ' << sr.GetEsd();
    for( size_t j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HyphenateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // planar groups
  for( size_t i=0; i < rm.rFLAT.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rFLAT[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    if( sr.AtomCount() < 4 )  continue;
    Tmp = "FLAT ";
    Tmp << sr.GetEsd();
    for( size_t j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HyphenateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // rigid bond restraint
  for( size_t i=0; i < rm.rDELU.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rDELU[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "DELU ";
    Tmp << sr.GetEsd() << ' ' << sr.GetEsd1();
    for( size_t j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HyphenateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // similar U restraint
  for( size_t i=0; i < rm.rSIMU.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rSIMU[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "SIMU ";
    Tmp << sr.GetEsd() << ' ' << sr.GetEsd1() << ' ' << sr.GetValue();
    for( size_t j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HyphenateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // Uanis restraint to behave like Uiso
  for( size_t i=0; i < rm.rISOR.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rISOR[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    Tmp = "ISOR ";
    Tmp << sr.GetEsd() << ' ' << sr.GetEsd1();
    for( size_t j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetFullLabel(rm);
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HyphenateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // equivalent EADP constraint
  for( size_t i=0; i < rm.rEADP.Count(); i++ )  {
    TSimpleRestraint& sr = rm.rEADP[i];
    sr.Validate();
    if( !Ins_ProcessRestraint(atoms, sr) )  continue;
    if( sr.AtomCount() < 2 )  continue;
    Tmp = "EADP";
    for( size_t j=0; j < sr.AtomCount(); j++ )  {
      Tmp << ' ' << sr.GetAtom(j).GetAtom()->GetLabel();
      StoreUsedSymIndex(usedSymm, sr.GetAtom(j).GetMatrix(), rm);
    }
    HyphenateIns(Tmp, SL);
    if( processed != NULL )  
      processed->restraints.Add( &sr );
  }
  // equivalent EXYZ constraint
  for( size_t i=0; i < rm.ExyzGroups.Count(); i++ )  {
    TExyzGroup& sr = rm.ExyzGroups[i];
    if( sr.IsEmpty() )  continue;
    Tmp = "EXYZ";
    for( size_t j=0; j < sr.Count(); j++ )  {
      if( sr[j].IsDeleted() )  continue;
      Tmp << ' ' << sr[j].GetLabel();
    }
    HyphenateIns(Tmp, SL);
  }
  // store the rest of eqiv ...
  for( size_t i=0; i < rm.UsedSymmCount(); i++ )
    StoreUsedSymIndex( usedSymm, &rm.GetUsedSymm(i), rm);
  // save
  for( size_t i=0; i < usedSymm.Count(); i++ )  {
    Tmp = "EQIV ";
    Tmp << '$' << (i+1) << ' ' << TSymmParser::MatrixToSymm( rm.GetUsedSymm(usedSymm[i]) );
    SL.Insert(oindex+i, Tmp  );
  }
  for( size_t i=0; i < rm.Vars.EquationCount(); i++ )  {
    if( !rm.Vars.GetEquation(i).Validate() )  continue;
    SL.Add("SUMP ") << rm.Vars.GetSUMPStr(i);
    //if( processed != NULL )  
    //  processed->equations.Add( &rm.Vars.GetEquation(i) );
  }
  SL.Add(EmptyString);
  if( atoms == NULL )  {
    for( size_t i=0; i < rm.FragCount(); i++ )
      rm.GetFrag(i).ToStrings(SL);
  }
  else  {
    SortedPtrList<const Fragment, TPointerPtrComparator> saved;
    for( size_t i=0; i < atoms->Count(); i++ )  {
      const int m = TAfixGroup::GetM( (*atoms)[i]->GetAfix() );
      if( m < 17 )  continue;
      const Fragment* frag = rm.FindFragByCode(m);
      if( frag == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "could not locate the FRAG for fitted group");
      if( saved.IndexOf(frag) != InvalidIndex )  continue;
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
  restraints.Add(&rm.rDFIX); 
  restraints.Add(&rm.rSADI); 
  restraints.Add(&rm.rDANG); 
  restraints.Add(&rm.rCHIV); 
  restraints.Add(&rm.rFLAT); 
  restraints.Add(&rm.rDELU); 
  restraints.Add(&rm.rSIMU); 
  restraints.Add(&rm.rISOR); 
  restraints.Add(&rm.rEADP); 
  for( size_t i=0; i < restraints.Count(); i++ )  {
    TSRestraintList& srl = *restraints[i];
    for( size_t j=0; j < srl.Count(); j++ )  {
      TSimpleRestraint& sr = srl[j];
      for( size_t k=0; k < sr.AtomCount(); k++ )
        sr.GetAtom(k).GetAtom()->SetLabel(rm.aunit.ValidateLabel(sr.GetAtom(k).GetAtom()->GetLabel()), false);
    }
  }
  // equivalent EXYZ constraint
  for( size_t i=0; i < rm.ExyzGroups.Count(); i++ )  {
    TExyzGroup& sr = rm.ExyzGroups[i];
    for( size_t j=0; j < sr.Count(); j++ )
      sr[j].SetLabel(rm.aunit.ValidateLabel(sr[j].GetLabel()), false);
  }
}
//..............................................................................
void TIns::ClearIns()  {
  for( size_t i=0; i < Ins.Count(); i++ )
    delete Ins.GetObject(i);
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
  if( RefMod.expl.IsTemperatureSet() )
    SL.Add("TEMP ") << RefMod.expl.GetTempValue().ToString();
}
//..............................................................................
void TIns::SaveHeader(TStrList& SL, bool ValidateRestraintNames)  {
  SL.Add("TITL ") << GetTitle();
  SL.Add( _CellToString() );
  SL.Add( _ZerrToString() );
  _SaveSymm(SL);
  SL.Add(EmptyString);
  SL.Add(EmptyString);
  SaveSfacUnit(GetRM(), GetRM().GetUserContent(), SL, SL.Count()-1);
  if( ValidateRestraintNames )
    ValidateRestraintsAtomNames(GetRM());
  SaveRestraints(SL, NULL, NULL, GetRM());
  _SaveRefMethod(SL);
  _SaveSizeTemp(SL);
  for( size_t i=0; i < GetRM().InfoTabCount(); i++ )  {
    if( GetRM().GetInfoTab(i).IsValid() )
      SL.Add( GetRM().GetInfoTab(i).InsStr() );
  }
  GetRM().Conn.ToInsList(SL);
  // copy "unknown" instructions except rems
  for( size_t i=0; i < Ins.Count(); i++ )  {
    TInsList* L = Ins.GetObject(i);
    if( L == NULL )  continue;  // if load failed
    // skip rems and print them at the end
    //if( Ins[i].StartsFrom("REM") )  continue;
    olxstr tmp = L->IsEmpty() ? EmptyString : L->Text(' ');
    HyphenateIns(Ins[i]+' ', tmp , SL);
  }
  SL << Skipped;
//  for( size_t i=0; i < Skipepd.Count(); i++ )

  _SaveHklInfo(SL, false);

  olxstr& wght = SL.Add("WGHT ");
  for( size_t i=0; i < RefMod.used_weight.Count(); i++ )  {
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
  for( size_t i=0; i < Ins.Count(); i++ )
    delete Ins.GetObject(i);
  Ins.Clear();
  Skipped.Clear();
  Title = EmptyString;
  GetRM().Clear(rm_clear_DEF);
  GetAsymmUnit().ClearMatrices();
// end clear, start parsing
  TStrList toks, lst(in);
  Preprocess(lst);
  ParseContext cx(GetRM());
  cx.ins = this;
  for( size_t i=0; i < lst.Count(); i++ )  {
    try  {
      olxstr Tmp = olxstr::DeleteSequencesOf<char>(lst[i], ' ');
      if( Tmp.IsEmpty() )      continue;
      for( size_t j=0; j < Tmp.Length(); j++ )  {
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
      else if( toks[0].Equalsi("TITL") )
        SetTitle(toks.Text(' ', 1));
      else
        Ins.Add(lst[i]);
    }
    catch( const TExceptionBase& exc )  {
      throw TFunctionFailedException(__OlxSourceInfo, exc,
        olxstr("at line #") << i+1 << " ('" << lst[i] << "')");
    }
  }
  smatd sm;
  for( size_t i=0; i < cx.Symm.Count(); i++ )  {
    if( TSymmParser::SymmToMatrix(cx.Symm[i], sm) )
      GetAsymmUnit().AddMatrix(sm);
  }
  Ins.Pack();
  ParseRestraints(cx.rm, Ins);
  Ins.Pack();
  _FinishParsing(cx);
}
//..............................................................................
bool TIns::ParseRestraint(RefinementModel& rm, const TStrList& toks)  {
  if( toks[0].Equalsi("EQIV") && toks.Count() >= 3 )  {
    smatd SymM;
    TSymmParser::SymmToMatrix(toks.Text(EmptyString, 2), SymM);
    rm.AddUsedSymm(SymM, toks[1]);
    return true;
  }
  TSRestraintList* srl = NULL;
  short RequiredParams = 1, AcceptsParams = 1;
  bool AcceptsAll = false;
  double Esd1Mult = 0, DefVal = 0, DefEsd = 0, DefEsd1 = 0;
  double *Vals[] = {&DefVal, &DefEsd, &DefEsd1};
  // extract residue
  olxstr resi, ins_name = toks[0];
  size_t resi_ind = toks[0].IndexOf('_');
  if( resi_ind != InvalidIndex )  {
    resi = toks[0].SubStringFrom(resi_ind+1);
    ins_name = toks[0].SubStringTo(resi_ind);
  }

  if( ins_name.Equalsi("EXYZ") )  {
    rm.AddEXYZ(toks.SubListFrom(1));
    return true;
  }
  else if( ins_name.Equalsi("DFIX") )  {
    srl = &rm.rDFIX;
    RequiredParams = 1;  AcceptsParams = 2;
    DefEsd = 0.02;
    Vals[0] = &DefVal;  Vals[1] = &DefEsd;
  }
  else if( ins_name.Equalsi("DANG") )  {
    srl = &rm.rDANG;
    RequiredParams = 1;  AcceptsParams = 2;
    DefEsd = 0.04;
    Vals[0] = &DefVal;  Vals[1] = &DefEsd;
  }
  else if( ins_name.Equalsi("SADI") )  {
    srl = &rm.rSADI;
    RequiredParams = 0;  AcceptsParams = 1;
    DefEsd = 0.02;
    Vals[0] = &DefEsd;
  }
  else if( ins_name.Equalsi("CHIV") )  {
    srl = &rm.rCHIV;
    RequiredParams = 1;  AcceptsParams = 2;
    DefEsd = 0.1;
    Vals[0] = &DefEsd;  Vals[1] = &DefVal;
  }
  else if( ins_name.Equalsi("FLAT") )  {
    srl = &rm.rFLAT;
    DefEsd = 0.1;
    RequiredParams = 0;  AcceptsParams = 1;
    Vals[0] = &DefEsd; ;
  }
  else if( ins_name.Equalsi("DELU") )  {
    srl = &rm.rDELU;
    DefEsd = 0.01;  DefEsd1 = 0.01;
    Esd1Mult = 1;
    RequiredParams = 0;  AcceptsParams = 2;
    Vals[0] = &DefEsd;  Vals[1] = &DefEsd1;
    AcceptsAll = true;
  }
  else if( ins_name.Equalsi("SIMU") )  {
    srl = &rm.rSIMU;
    DefEsd = 0.04;  DefEsd1 = 0.08;
    Esd1Mult = 2;
    DefVal = 1.7;
    RequiredParams = 0;  AcceptsParams = 3;
    Vals[0] = &DefEsd;  Vals[1] = &DefEsd1;  Vals[2] = &DefVal;
    AcceptsAll = true;
  }
  else if( ins_name.Equalsi("ISOR") )  {
    srl = &rm.rISOR;
    DefEsd = 0.1;  DefEsd1 = 0.2;
    Esd1Mult = 2;
    RequiredParams = 0;  AcceptsParams = 2;
    Vals[0] = &DefEsd;  Vals[1] = &DefEsd1;
    AcceptsAll = true;
  }
  else if( ins_name.Equalsi("EADP") )  {
    srl = &rm.rEADP;
    RequiredParams = 0;  AcceptsParams = 0;
  }
  else
    srl = NULL;
  if( srl != NULL )  {
    TSimpleRestraint& sr = srl->AddNew();
    size_t index = 1;
    if( toks.Count() > 1 && toks[1].IsNumber() )  {
      if( toks.Count() > 2 && toks[2].IsNumber() )  {
        if( toks.Count() > 3 && toks[3].IsNumber() )  {  // three numerical params
          if( AcceptsParams < 3 )  
            throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical parameters");
          *Vals[0] = toks[1].ToDouble();
          *Vals[1] = toks[2].ToDouble();
          *Vals[2] = toks[3].ToDouble();
          index = 4; 
        }
        else  {  // two numerical params
          if( AcceptsParams < 2 )  
            throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical parameters");
          *Vals[0] = toks[1].ToDouble();
          *Vals[1] = toks[2].ToDouble();
          index = 3; 
        }
      }
      else  {
        if( AcceptsParams < 1 )  
          throw TInvalidArgumentException(__OlxSourceInfo, "too many numerical parameters");
        *Vals[0] = toks[1].ToDouble();
        index = 2; 
      }
    }
    rm.Vars.SetParam(sr, 0, DefVal);
    sr.SetEsd(DefEsd);
    if( Vals[0] == &DefEsd )
      sr.SetEsd1( (index <= 2) ? DefEsd*Esd1Mult : DefEsd1 );
    else
      sr.SetEsd1(DefEsd1);
    if( AcceptsAll && toks.Count() <= index )  {
      sr.SetAllNonHAtoms(true);
    }
    else  {
      TAtomReference aref(toks.Text(' ', index));
      TCAtomGroup agroup;
      size_t atomAGroup;
      try  {  aref.Expand(rm, agroup, resi, atomAGroup);  }
      catch( const TExceptionBase& ex )  {
        TBasicApp::NewLogEntry(logException) << ex.GetException()->GetError();
        return false;
      }
      if( sr.GetListType() == rltBonds && (agroup.Count() == 0 || (agroup.Count()%2)!=0 ) )  {
        TBasicApp::NewLogEntry(logError) << "Wrong restraint parameters list: " << toks.Text(' ');
        return false;
      }
      if( ins_name.Equalsi("FLAT") )  {  // a special case again...
        TSimpleRestraint* sr1 = &sr;
        for( size_t j=0; j < agroup.Count(); j += atomAGroup )  {
          for( size_t k=0; k < atomAGroup; k++ )
            sr1->AddAtom(*agroup[j+k].GetAtom(), agroup[j+k].GetMatrix());
          if( j != 0 )
            srl->ValidateRestraint(*sr1);
          sr1 = &srl->AddNew();
          sr1->SetEsd( sr.GetEsd() );
          sr1->SetEsd1( sr.GetEsd1() );
          sr1->SetValue( sr.GetValue() );
        }
      }
      else
        sr.AddAtoms(agroup);
    }
    srl->ValidateRestraint(sr);
    return true;
  }
  return false;
}

