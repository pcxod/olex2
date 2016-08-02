/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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
#include "label_corrector.h"
#include "estopwatch.h"
#include "absorpc.h"

#undef AddAtom
#undef GetObject
#undef Object

TIns::TIns()  {  LoadQPeaks = true;  }
//..............................................................................
TIns::~TIns()  {  Clear();  }
//..............................................................................
void TIns::Clear() {
  GetRM().Clear(rm_clear_ALL);
  GetAsymmUnit().Clear();
  for (size_t i=0; i < Ins.Count(); i++)
    delete Ins.GetObject(i);
  Ins.Clear();
  Skipped.Clear();
  Title.SetLength(0);
  R1 = -1;
  RefinementInfo.Clear();
}
//..............................................................................
void TIns::LoadFromFile(const olxstr& fileName)  {
  TStopWatch sw(__FUNC__);
  Lst.Clear();
  // load Lst first, as it may have the error indicator
  olxstr lst_fn = TEFile::ChangeFileExt(fileName, "lst");
  if( TEFile::Exists(lst_fn) )  {
    try  {
      sw.start("Loading LST file");
      Lst.LoadFromFile(lst_fn);
    }
    catch(...)  {}
  }
  sw.start("Loading the file");
  TBasicCFile::LoadFromFile(fileName);
  if (Lst.IsLoaded()) {
    try {
      if (GetRM().HasEXTI()) {
        olxstr str_exti = Lst.params.Find("exti", EmptyString());
        if (!str_exti.IsEmpty()) {
          TEValueD exti = str_exti;
          GetRM().SetEXTI(exti.GetV(), exti.GetE());
        }
      }
      olxstr val_n = "basf_", val;
      size_t cnt = 1;
      while (!(val = Lst.params.Find(olxstr(val_n) << cnt, EmptyString()))
        .IsEmpty())
      {
        if (GetRM().GetBASF().Count() >= cnt) {
          TEValueD dv = val;
          if (olx_abs(GetRM().GetBASF()[cnt - 1].GetV() - dv.GetV()) < dv.GetE())
            GetRM().GetBASF()[cnt - 1] = dv;
          else
            break;
          cnt++;
        }
        else
          break;
      }
      val_n = "fvar_";
      cnt = 2;
      while (!(val = Lst.params.Find(olxstr(val_n) << cnt, EmptyString()))
        .IsEmpty())
      {
        if (GetRM().Vars.VarCount() >= cnt) {
          TEValueD dv = val;
          if (olx_abs(GetRM().Vars.GetVar(cnt-1).GetValue() - dv.GetV()) < dv.GetE())
            GetRM().Vars.GetVar(cnt - 1).SetEsd(dv.GetE());
          else
            break;
          cnt++;
        }
        else
          break;
      }
      if (RefinementInfo.IsEmpty()) {
        RefinementInfo("R1_all",
          Lst.params.Find("R1all", XLibMacros::NAString()));
        RefinementInfo("R1_gt",
          Lst.params.Find("R1", XLibMacros::NAString()));
        RefinementInfo("wR_ref",
          Lst.params.Find("wR2", XLibMacros::NAString()));
        RefinementInfo("GOOF",
          Lst.params.Find("S", XLibMacros::NAString()));
        RefinementInfo("Shift_max",
          Lst.params.Find("max_shift/esd", XLibMacros::NAString()));
        RefinementInfo("Shift_mean",
          Lst.params.Find("mean_shift/esd", XLibMacros::NAString()));
        RefinementInfo("Hole",
          Lst.params.Find("hole", XLibMacros::NAString()));
        RefinementInfo("Peak",
          Lst.params.Find("peak", XLibMacros::NAString()));

        RefinementInfo("Reflections_all",
          Lst.params.Find("ref_unique", XLibMacros::NAString()));
        RefinementInfo("Reflections_gt",
          Lst.params.Find("ref_4sig", XLibMacros::NAString()));
        RefinementInfo("Parameters",
          Lst.params.Find("param_n", XLibMacros::NAString()));
        RefinementInfo("Flack",
          Lst.params.Find("flack", XLibMacros::NAString()));
      }
    }
    catch (...)  {}
  }
}
//..............................................................................
void TIns::LoadFromStrings(const TStrList& FileContent)  {
  Clear();
  ParseContext cx(GetRM());
  cm_Element& elmQPeak = XElementLib::GetByIndex(iQPeakIndex);
  cx.Resi = &GetAsymmUnit().GetResidue(0);
  cx.ins = this;
  TStrList Toks, InsFile(FileContent);
  // read the extras before preprocessing
  _ReadExtras(InsFile, cx);
  for (size_t i = 0; i < InsFile.Count(); i++) {
    InsFile[i].Replace('\t', ' ')
      .TrimR(' ')
      .TrimR('\0')
      .TrimR('\r')
      .TrimR(' ');
  }
  Preprocess(InsFile);
  for (size_t i = 0; i < InsFile.Count(); i++) {
    try {
      if (InsFile[i].IsEmpty() || InsFile[i].StartsFrom(' '))
        continue;
      const size_t exi = InsFile[i].IndexOf('!');
      if (exi != InvalidIndex) {
        InsFile[i].SetLength(exi);
      }
      Toks.Clear();
      Toks.Strtok(InsFile[i], ' ');
      if (Toks.IsEmpty()) {
        continue;
      }
      if (Toks[0].Equalsi("MOLE")) { // these are dodgy
        continue;
      }
      else if (ParseIns(InsFile, Toks, cx, i)) {
        continue;
      }
      else if (Toks[0].Equalsi("END")) {   //reset RESI to default
        // this will help with recognising ins after end which to be ignored
        Ins.Add(Toks[0]);
        cx.End = true;
        cx.Resi = &GetAsymmUnit().GetResidue(0);
        cx.AfixGroups.Clear();
        cx.Part = 0;
      }
      else if (Toks[0].Equalsi("TITL")) {
        SetTitle(Toks.Text(' ', 1));
      }
      // atom should have at least 7 parameters
      else if (Toks.Count() < 6 || Toks.Count() > 12) {
        Ins.Add(InsFile[i]);
      }
      else {
        bool qpeak = olxstr::o_toupper(Toks[0].CharAt(0)) == 'Q';
        if (qpeak && !cx.End && !LoadQPeaks) {
          continue;
        }
        if (cx.End && !qpeak) {
          continue;
        }
        // is a valid atom
        //if( !atomsInfo.IsAtom(Toks[0]))  {  Ins.Add(InsFile[i]);  continue;  }
        if (!Toks[1].IsUInt())  {
          Ins.Add(InsFile[i]);
          continue;
        }
        const uint32_t sindex = Toks[1].ToUInt()-1;
        if (sindex >= cx.BasicAtoms.Count()) {  // wrong index in SFAC
          Ins.Add(InsFile[i]);
          continue;
        }
        // should be four numbers
        if ((!Toks[2].IsNumber()) || (!Toks[3].IsNumber()) ||
          (!Toks[4].IsNumber()) || (!Toks[5].IsNumber()))
        {
          Ins.Add(InsFile[i]);
          continue;
        }
        if (!cx.CellFound) {
          Clear();
          throw TFunctionFailedException(__OlxSourceInfo, "uninitialised cell");
        }
        TCAtom* atom = _ParseAtom(Toks, cx);
        atom->SetLabel(Toks[0], false);
        if (qpeak)
          atom->SetType(elmQPeak);
        else {
          atom->SetType(*cx.BasicAtoms.GetObject(sindex));
          atom->SetCharge(XScatterer::ChargeFromLabel(
            cx.BasicAtoms[sindex], cx.BasicAtoms.GetObject(sindex)));
        }
        if (atom->GetType().z > 1 &&
          (cx.AfixGroups.IsEmpty() || cx.AfixGroups.Top().GetB()->GetAfix() != 3))
        {
          cx.LastRideable = atom;
        }
        _ProcessAfix(*atom, cx);
      }
    }
    catch (const TExceptionBase& exc)  {
      throw TFunctionFailedException(__OlxSourceInfo, exc,
        olxstr("at line #") << i + 1 << " ('" << InsFile[i] << "')");
    }
  }
  for (size_t i = 0; i < cx.Symm.Count(); i++) {
    GetAsymmUnit().AddMatrix(TSymmParser::SymmToMatrix(cx.Symm[i]));
  }

  ParseRestraints(cx.rm, Ins);
  Ins.Pack();
  _ProcessSame(cx);
  _FinishParsing(cx);
  // remove dublicated instructions, rems ONLY
  for (size_t i = 0; i < Ins.Count(); i++) {
    if (!Ins[i].StartsFromi("REM")) continue;
    olxstr a = olxstr(' ').Join(*Ins.GetObject(i));
    for (size_t j = i + 1; j < Ins.Count(); j++) {
      if (Ins[i] == Ins[j] && a == olxstr(' ').Join(*Ins.GetObject(j))) {
        delete Ins.GetObject(j);
        Ins[j].SetLength(0);
      }
    }
  }
  Ins.Pack();
  // initialise asu data
  GetAsymmUnit().InitData();
  if( !cx.CellFound )  {  // in case there are no atoms
    Clear();
    throw TInvalidArgumentException(__OlxSourceInfo, "empty CELL");
  }
  // update the refinement model data
  if (Lst.IsLoaded()) {
    TTypeList<RefinementModel::BadReflection> bad_refs;
    for (size_t i=0; i < Lst.DRefCount(); i++) {
      const TLstRef &lr = Lst.DRef(i);
      bad_refs.Add(
        new RefinementModel::BadReflection(
        vec3i(lr.H, lr.K, lr.L),
        lr.Fo,
        lr.Fc,
        olx_abs(lr.Fo-lr.Fc)/lr.DF,
        lr.DF*olx_sign(lr.Fc-lr.Fo))
        );
    }
    GetRM().SetBadReflectionList(bad_refs);
  }
}
//..............................................................................
void TIns::_ProcessSame(ParseContext& cx)  {
  TSameGroupList& sgl = cx.rm.rSAME;
  for (size_t i=0; i < cx.Same.Count(); i++) {
    TCAtom* ca = cx.Same[i].b;
    TStrList& sl = cx.Same[i].a;
    TPtrList<TSameGroup> all_groups;
    size_t max_atoms = 0;
    for (size_t j=0; j < sl.Count(); j++) {
       TStrList toks(sl[j], ' ');
       size_t resi_ind = toks[0].IndexOf('_');
       olxstr resi( (resi_ind != InvalidIndex)
         ? toks[0].SubStringFrom(resi_ind+1) : EmptyString());
       double esd1=0.02, esd2=0.04;
       size_t from_ind = 1;
       if (toks.Count() > 1 && toks[1].IsNumber()) {
         esd1 = toks[1].ToDouble();
         from_ind++;
       }
       if (toks.Count() > 2 && toks[2].IsNumber()) {
         esd2 = toks[2].ToDouble();
         from_ind++;
       }
      TSameGroup& sg1 = *all_groups.Add(sgl.New());
      sg1.GetAtoms().Build(toks.Text(' ', from_ind), resi);
      sg1.Esd12 = esd1;
      sg1.Esd13 = esd2;
      if (sg1.GetAtoms().IsExplicit()) {
        TTypeList<ExplicitCAtomRef> atoms = sg1.GetAtoms().ExpandList(cx.rm);
        if (atoms.Count() > max_atoms)
          max_atoms = atoms.Count();
      }
    }
    // now process the reference group
    if (max_atoms != 0 && ca != 0) {
      bool valid = true;
      TSameGroup& sg = sgl.New();  // main, reference, group
      for (size_t j = 0; j < max_atoms; j++) {
        if (ca->GetId() + j >= cx.au.AtomCount()) {
          TBasicApp::NewLogEntry(logError) <<
            "Not enough atoms to create the reference group for SAME";
          valid = false;
          break;
        }
        TCAtom& a = cx.au.GetAtom(ca->GetId() + j);
        if (a.GetType() == iHydrogenZ) {
          max_atoms++; // do not count the H atoms!
          continue;
        }
        sg.Add(a);
      }
      if (valid) {
        for (size_t j = 0; j < all_groups.Count(); j++) {
          sg.AddDependent(*all_groups[j]);
        }
      }
      else {
        sgl.Delete(all_groups << sg);
      }
    }
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
      Ins[i].SetLength(0);
    }
    else if( toks[0].Equalsi("FREE") )  {
      cx.rm.Conn.ProcessFree(toks.SubListFrom(1));
      Ins[i].SetLength(0);
    }
    else if( toks[0].Equalsi("BIND") )  {
      cx.rm.Conn.ProcessBind(toks.SubListFrom(1));
      Ins[i].SetLength(0);
    }
  }
  Ins.Pack();
}
//..............................................................................
void TIns::_ReadExtras(TStrList &l, ParseContext &cx) {
  cx.Extras.Clear();
  for (size_t i = 0; i < l.Count(); i++) {
    if (l[i].Equals("REM <olex2.extras>")) {
      size_t j = i;
      while (++j != l.Count() && !l[j].Equals("REM </olex2.extras>"))
        ;
      if (j == l.Count()) {
        TBasicApp::NewLogEntry(logError) << "Failed to read Olex2 extra "
          "information";
        return;
      }
      l.SubList(i + 1, j - i - 1, cx.Extras);
      l.DeleteRange(i, j-i+1);
      i = j;
    }
    else if (l[i].Contains("Refinement Information")) {
      size_t j = i;
      while (++j < l.Count() && l[j].StartsFrom("REM ")) {
        size_t eq_idx = l[j].FirstIndexOf('=', 4);
        if (eq_idx == InvalidIndex) {
          break;
        }
        RefinementInfo(l[j].SubString(4, eq_idx - 4).TrimWhiteChars(),
          l[j].SubStringFrom(eq_idx + 1).TrimWhiteChars());
      }
      if (j == l.Count()) {
        j--;
      }
      l.DeleteRange(i, j - i + 1);
      break;
    }
  }
  for (size_t i = 0; i < cx.Extras.Count(); i++) {
    if (cx.Extras[i].Length() > 4)
      cx.Extras[i] = cx.Extras[i].SubStringFrom(4);
    else
      cx.Extras[i].SetLength(0);
  }
}
//..............................................................................
void TIns::_FinishParsing(ParseContext& cx) {
  __ProcessConn(cx);
  for (size_t i = 0; i < Ins.Count(); i++)  {
    TStrList toks(Ins[i], ' ');
    if (toks.Count() == 1 && toks[0].Equalsi("END")) {
      Ins.DeleteRange(i, Ins.Count()-i);
      break;
    }
    else if ((toks[0].StartsFromi("HTAB") || toks[0].StartsFromi("RTAB") ||
      toks[0].StartsFromi("MPLA") || toks[0].StartsFromi("CONF")) &&
      toks.Count() > 2)
    {
      cx.rm.AddInfoTab(toks);
      Ins.Delete(i--);
    }
    else if (toks[0].Equalsi("OMIT")) {
      cx.rm.AddOMIT(toks.SubListFrom(1));
      Ins.Delete(i--);
    }
    else if (toks[0].StartsFromi("ANIS")) {
      Ins.Delete(i--);
      try {
        double Q[6];
        memset(&Q[0], 0, sizeof(Q));
        AtomRefList rl(cx.rm, toks.Text(' ', 1));
        TTypeList<TAtomRefList> atoms;
        rl.Expand(cx.rm, atoms);
        for (size_t j = 0; j < atoms.Count(); j++) {
          for (size_t k = 0; k < atoms[j].Count(); k++) {
            TCAtom& ca = atoms[j][k].GetAtom();
            if (ca.GetEllipsoid() == NULL) {
              Q[0] = Q[1] = Q[2] = ca.GetUiso();
              ca.UpdateEllp(Q);
            }
          }
        }
      }
      catch (const TExceptionBase& e) {
        TBasicApp::NewLogEntry(logError) << e.GetException()->GetFullMessage();
      }
    }
    else {
      TInsList* Param = new TInsList(toks);
      Ins.GetObject(i) = Param;
      Ins[i] = Param->GetString(0);
      Param->Delete(0);
      for (size_t j = 0; j < Param->Count(); j++)
        Param->GetObject(j) = GetAsymmUnit().FindCAtom(Param->GetString(j));
    }
  }
  // automatically generate the 'extras'
  olxdict<TCAtom*, TCAtomPList, TPointerComparator> extras;
  for (size_t i=0; i < GetAsymmUnit().AtomCount(); i++) {
    TCAtom &a = GetAsymmUnit().GetAtom(i);
    if (a.GetUisoOwner() != NULL && a.GetAfix() == 0) {
      extras.Add(a.GetUisoOwner()).Add(a);
    }
  }
  for (size_t i=0; i < extras.Count(); i++) {
    TCAtom * owner = const_cast<TCAtom *>(extras.GetKey(i));
    TAfixGroup &ag = GetRM().AfixGroups.New(owner, -1);
    for (size_t j = 0; j < extras.GetValue(i).Count(); j++) {
      ag.AddDependent(*extras.GetValue(i)[j]);
    }
  }
  cx.rm.ReadInsExtras(cx.Extras);
  cx.rm.Vars.Validate();
  cx.rm.ProcessFrags();
}
//..............................................................................
void TIns::_ProcessAfix0(ParseContext& cx)  {
  if (!cx.AfixGroups.IsEmpty()) {
    int old_m = cx.AfixGroups.Top().GetB()->GetM();
    bool valid = true;
    if (cx.AfixGroups.Top().GetA() > 0) {
      if (old_m != 0) {
        TBasicApp::NewLogEntry(logError) << olxstr("Incomplete AFIX group") <<
        (cx.Last != NULL ? (olxstr(" at ") << cx.Last->GetLabel())
          : EmptyString());
        valid = false;
      }
      else {
        TBasicApp::NewLogEntry(logError) << "Possibly incorrect AFIX " <<
          cx.AfixGroups.Top().GetB()->GetAfix() <<
          (cx.Last != NULL ? (olxstr(" at ") << cx.Last->GetLabel())
            : EmptyString());
      }
    }
    if (cx.AfixGroups.Top().GetB()->GetPivot() == NULL) {
      TBasicApp::NewLogEntry(logError) << "Undefined pivot atom for a fitted "
        "group" << (cx.Last != NULL ? (olxstr(" at ") << cx.Last->GetLabel())
          : EmptyString());
      valid = false;
    }
    if (!valid) {
      cx.AfixGroups.Top().GetB()->Clear();
    }
    // pop all complete
    size_t po = 0;
    while (!cx.AfixGroups.IsEmpty() && cx.AfixGroups.Top().GetA() == 0) {
      cx.AfixGroups.Pop();
      po++;
    }
    if (po == 0) { //pop last then
      cx.AfixGroups.Pop();
    }
    if (!cx.AfixGroups.IsEmpty() && cx.AfixGroups.Top().GetA() < 0) {
      int afix = cx.AfixGroups.Top().b->GetAfix();
      /* flag that if the next atom is not in AFIX 5- terminate the rigid
      group*/
      if (afix == 6 || afix == 9) {
        cx.AfixGroups.Top().SetA(0);
      }
    }
  }
}
//..............................................................................
bool TIns::ParseIns(const TStrList& ins, const TStrList& Toks,
  ParseContext& cx, size_t& i)
{
  if (_ParseIns(cx.rm, Toks)) {
    return true;
  }
  else if( !cx.CellFound && Toks[0].Equalsi("CELL") )  {
    if( Toks.Count() == 8 )  {
      cx.rm.expl.SetRadiation( Toks[1].ToDouble() );
      cx.au.GetAxes() =
        vec3d(Toks[2].ToDouble(), Toks[3].ToDouble(), Toks[4].ToDouble());
      cx.au.GetAngles() =
        vec3d(Toks[5].ToDouble(), Toks[6].ToDouble(), Toks[7].ToDouble());
      cx.CellFound = true;
      cx.au.InitMatrices();
    }
    else  {
      throw TFunctionFailedException(__OlxSourceInfo,
        "invalid Cell instruction");
    }
  }
  else if( Toks[0].Equalsi("SYMM") && (Toks.Count() > 1))
    cx.Symm.Add( Toks.Text(EmptyString(), 1) );
  else if( Toks[0].Equalsi("FRAG") && (Toks.Count() > 1))  {
   int code = Toks[1].ToInt();
    if( code < 17 )  {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "FRAG code must be greater than 16");
    }
    double a=1, b=1, c=1, al=90, be=90, ga=90;
    XLibMacros::ParseOnly(
      Toks.SubListFrom(2), "dddddd", &a, &b, &c, &al, &be, &ga);
    Fragment* frag = cx.rm.FindFragByCode(code);
    if( frag == NULL )
      frag = &cx.rm.AddFrag(Toks[1].ToInt(), a, b, c, al, be, ga);
    else
      frag->Reset(a, b, c, al, be, ga);
    TStrList f_toks;
    while( ++i < ins.Count() && !ins[i].StartsFromi("FEND") )  {
      if( ins[i].IsEmpty() )  continue;
      f_toks.Strtok(ins[i], ' ');
      if( f_toks.Count() > 4 )  {
        frag->Add(f_toks[0], f_toks[2].ToDouble(), f_toks[3].ToDouble(),
          f_toks[4].ToDouble());
      }
      else
        throw TFunctionFailedException(__OlxSourceInfo, "invalid FRAG atom");
      f_toks.Clear();
    }
  }
  else if (Toks[0].Equalsi("PART") && (Toks.Count() > 1)) {
    cx.Part = (short)Toks[1].ToInt();
    cx.PartOccu = 0;
    if (Toks.Count() == 3)
      cx.PartOccu = Toks[2].ToDouble();
    // TODO: validate if appropriate here...
    //_ProcessAfix0(cx);
  }
  else if (Toks[0].Equalsi("SPEC")) {
    cx.SPEC = Toks.Count() > 1 ? Toks[1].ToDouble() : 0.2;
  }
  else if (Toks[0].Equalsi("AFIX") && (Toks.Count() > 1)) {
    const int afix = Toks[1].ToInt();
    TAfixGroup* afixg = NULL;
    int n = 0, m = 0;
    if (afix != 0) {
      double d = 0, u = 0, sof = 0;
      if (Toks.Count() > 2 && Toks[2].IsNumber()) {
        d = Toks[2].ToDouble();
      }
      if (Toks.Count() > 3) {
        sof = Toks[3].ToDouble();
      }
      if (Toks.Count() > 4) {
        u = Toks[3].ToDouble();
      }
      n = TAfixGroup::GetN(afix);
      m = TAfixGroup::GetM(afix);
      if (!TAfixGroup::IsDependent(afix)) {
        /*shelxl produces 'broken' res files (by removing termitating AFIX 0
        for fitted groups) limiting the construction of rigid groups... so to
        read them correctly we have to pop the last rigid group if encounter
        a new one
        */
        if (!cx.AfixGroups.IsEmpty() &&
            !cx.AfixGroups.Top().b->IsFixedGroup() &&
            !TAfixGroup::IsFixedGroup(afix))
        {
          cx.AfixGroups.Pop();
        }
        afixg = &cx.rm.AfixGroups.New(NULL, afix, d, sof == 11 ? 0 : sof,
          u == 10.08 ? 0 : u);
      }
      else {
        if( !cx.AfixGroups.IsEmpty() &&
            cx.AfixGroups.Top().b->IsFixedGroup() &&
            cx.AfixGroups.Top().GetA() == 0 )
        {
          cx.AfixGroups.Pop();
        }
      }
    }
    /* Shelx manual: n is always a single digit; m may be two, one or zero
    digits (the last corresponds to m = 0).
    */
    if (afix == 0) {
      _ProcessAfix0(cx);
    }
    else {
      if (!cx.AfixGroups.IsEmpty()) {
        if (cx.AfixGroups.Top().GetA() == 0) {
          // reset the group popping out if needed
          int ax = cx.AfixGroups.Top().GetB()->GetAfix();
          if ((ax == 6 || ax == 9) && afix == 5) {
            cx.AfixGroups.Top().SetA(-1);
          }
          // pop m = 0 as well
          else {
            cx.AfixGroups.Pop();
          }
        }
      }
      /* terminate afix 1, 2, 3 when any other afix encountered
      or any other when the same or other, independent and without implicit
      pivot is encountered
      */
      if (!cx.AfixGroups.IsEmpty() &&
        ((cx.AfixGroups.Top().GetA() < 0 &&
        !TAfixGroup::IsDependent(afix) &&
        !TAfixGroup::HasImplicitPivot(afix)) ||
        (cx.AfixGroups.Top().GetB()->GetAfix() < 4)))
      {
        cx.AfixGroups.Pop();
      }
      if (afixg != NULL) {
        switch( m )  {
        case 1:
        case 4:
        case 8:
        case 14:
        case 15:
        case 16:
          cx.AfixGroups.Push(AnAssociation3<int,TAfixGroup*,bool>(
            1, afixg, false));
          break;
        case 2:
        case 9:
          cx.AfixGroups.Push(AnAssociation3<int,TAfixGroup*,bool>(
            2, afixg, false));
          break;
        case 3:
        case 13:
          cx.AfixGroups.Push(AnAssociation3<int,TAfixGroup*,bool>(
            3, afixg, false));
          break;
        case 7:
        case 6:
          cx.AfixGroups.Push(AnAssociation3<int,TAfixGroup*,bool>(
            5, afixg, true));
          cx.SetNextPivot = true;
          break;
        case 5:
          cx.AfixGroups.Push(AnAssociation3<int,TAfixGroup*,bool>(
            4, afixg, true));
          cx.SetNextPivot = true;
          break;
        case 11:  //naphtalene
        case 10:  // Cp*
          cx.AfixGroups.Push(AnAssociation3<int,TAfixGroup*,bool>(
            9, afixg, true));
          cx.SetNextPivot = true;
          break;
        case 12:  // disordered CH3
          cx.AfixGroups.Push(AnAssociation3<int,TAfixGroup*,bool>(
            6, afixg, false));
          break;
        }
        if( m > 16 )  {  // FRAG
          Fragment* frag = cx.rm.FindFragByCode(m);
          if( frag == NULL )  {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "fitted group should be preceeded by the FRAG..FEND with the same code");
          }
          cx.AfixGroups.Push(AnAssociation3<int,TAfixGroup*,bool>(
            (int)frag->Count()-1, afixg, false));
          cx.SetNextPivot = true;
        }
        else if (m == 0) {
          if (!TAfixGroup::IsDependent(afix)) {
            // generic container then, besides, 5 is dependent atom of rigid group
            cx.AfixGroups.Push(AnAssociation3<int, TAfixGroup*, bool>(
              -1, afixg, false));
            cx.SetNextPivot = TAfixGroup::HasExcplicitPivot(afix) ||
              !TAfixGroup::HasPivot(n);  // if not riding
          }
        }
        if (!cx.SetNextPivot) {
          if (cx.LastRideable == NULL) {
            throw TFunctionFailedException(__OlxSourceInfo,
              "undefined pivot atom for a fitted group");
          }
          // have to check if several afixes for one atom (if the last is H)
          afixg->SetPivot(*cx.LastRideable);
        }
      }
    }
  }
  else if (Toks[0].Equalsi("RESI")) {
    _ProcessAfix0(cx);
    if (Toks.Count() < 2) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "number of arguments for RESI");
    }
    if (Toks[1].IsNumber()) {
      int n = Toks[1].ToInt();
      if (Toks.Count() > 2) {
        if (Toks[2].IsNumber()) {
          cx.Resi = &cx.au.NewResidue(EmptyString(), n,
            Toks[2].ToInt());
        }
        else {
          cx.Resi = &cx.au.NewResidue(Toks[2], n, n);
        }
      }
      else {
        cx.Resi = &cx.au.NewResidue(EmptyString(), n, n);
      }
    }
    else {
      if (Toks.Count() < 3 || !Toks[2].IsNumber()) {
        throw TInvalidArgumentException(__OlxSourceInfo, "number for RESI");
      }
      int n = Toks[2].ToInt();
      cx.Resi = &cx.au.NewResidue(Toks[1], n,
        (Toks.Count() > 3) ? Toks[3].ToInt() : n);
    }
  }
  else if (Toks[0].Equalsi("SFAC")) {
    bool expandedSfacProcessed = false;
    if (Toks.Count() == 16) {  // a special case for expanded sfac
      int NumberCount = 0;
      for (size_t i=2; i < Toks.Count(); i++) {
        if (Toks[i].IsNumber())
          NumberCount++;
      }
      if (NumberCount > 0 && NumberCount < 14) {
        TBasicApp::NewLogEntry(logError) << "Possibly not well formed SFAC "
          << Toks[0];
      }
      else  if (NumberCount == 14) {
        olxstr lb(Toks[1].CharAt(0) == '$'
          ? Toks[1].SubStringFrom(1) : Toks[1]);
        cm_Element *elm = XElementLib::FindBySymbolEx(lb);
        if (elm == 0) {
          throw TFunctionFailedException(__OlxSourceInfo,
            olxstr("Could not find suitable scatterer for ").quote()
            << Toks[1]);
        }
        lb = XScatterer::NormaliseCharge(lb, elm);
        cx.rm.AddUserContent(*elm, 0, XScatterer::ChargeFromLabel(lb));
        cx.BasicAtoms.Add(lb, elm);
        expandedSfacProcessed = true;
        XScatterer* sc = new XScatterer(lb);
        sc->SetGaussians(
          cm_Gaussians(
            Toks[2].ToDouble(), Toks[4].ToDouble(), Toks[6].ToDouble(),
            Toks[8].ToDouble(), Toks[3].ToDouble(), Toks[5].ToDouble(),
            Toks[7].ToDouble(), Toks[9].ToDouble(), Toks[10].ToDouble())
          );
        sc->SetFpFdp(compd(Toks[11].ToDouble(), Toks[12].ToDouble()));
        sc->SetMu(Toks[13].ToDouble());
        sc->SetR(Toks[14].ToDouble());
        sc->SetWeight(Toks[15].ToDouble());
        cx.rm.AddSfac(*sc);
      }
    }
    if (!expandedSfacProcessed) {
      for (size_t j = 1; j < Toks.Count(); j++) {
        if (XElementLib::IsElement(Toks[j])) {
          cx.BasicAtoms.Add(Toks[j], XElementLib::FindBySymbol(Toks[j]));
          if (cx.BasicAtoms.GetLast().Object == NULL) {
            throw TFunctionFailedException(__OlxSourceInfo,
              olxstr("Could not find suitable scatterer for ").quote()
              << Toks[j]);
          }
          cx.rm.AddUserContent(Toks[j]);
        }
      }
    }
  }
  else if (Toks[0].Equalsi("DISP") && Toks.Count() >= 4) {
    const olxstr lb(Toks[1].CharAt(0) == '$'
      ? Toks[1].SubStringFrom(1) : Toks[1]);
    XScatterer* sc = new XScatterer(lb);
    sc->SetFpFdp(compd(Toks[2].ToDouble(), Toks[3].ToDouble()));
    if (Toks.Count() >= 5)
      sc->SetMu(Toks[4].ToDouble());
    cx.rm.AddSfac(*sc);
  }
  else if (Toks[0].Equalsi("REM")) {
    if (Toks.Count() > 1) {
      if (Toks[1].Equalsi("R1") && Toks.Count() > 4 && Toks[3].IsNumber()) {
        if (cx.ins != NULL) {
          cx.ins->R1 = Toks[3].ToDouble();
        }
      }
      else if (Toks[1].Equalsi("olex2.stop_parsing")) {
        while (i < ins.Count()) {
          if (cx.ins != NULL) {
            cx.ins->Skipped.Add(ins[i]);
          }
          if (ins[i].StartsFromi("REM") &&
              ins[i].IndexOf("olex2.resume_parsing") != InvalidIndex)
          {
            break;
          }
          i++;
        }
      }
      else if (Toks[1].StartsFromi("<HKL>")) {
        olxstr hklsrc = Toks.Text(' ', 1);
        size_t index = hklsrc.FirstIndexOf('>');
        size_t iv = hklsrc.IndexOf("</HKL>");
        if (iv == InvalidIndex) {
          while ((i+1) < ins.Count()) {
            i++;
            if (!ins[i].StartsFromi("rem")) {
              break;
            }
            hklsrc << ins[i].SubStringFrom(4);
            if ((iv = hklsrc.IndexOf("</HKL>")) != InvalidIndex) {
              break;
            }
          }
        }
        if (iv != InvalidIndex) {
          hklsrc = hklsrc.SubString(index+1, iv-index-1).Replace("%20", ' ');
        }
        else {
          hklsrc.SetLength(0);
        }
        cx.rm.SetHKLSource(hklsrc);
      }
      else if (!cx.End  && !cx.rm.IsHKLFSet()) {
        if (cx.ins != NULL) {
          cx.ins->Ins.Add(Toks.Text(' '));
        }
      }
    }
  }
  else if (Toks[0].StartsFromi("SAME")) {
    // no atom so far, add to the list of Same
    if (!cx.Same.IsEmpty() && cx.Same.GetLast().GetB() == NULL) {
      cx.Same.GetLast().a.Add(Toks.Text(' '));
    }
    else {
      cx.Same.Add(new olx_pair_t<TStrList,TCAtom*>);
      cx.Same.GetLast().b = NULL;
      cx.Same.GetLast().a.Add(Toks.Text(' '));
    }
  }
  else if (Toks[0].Equalsi("ANIS")) {
    if (Toks.Count() == 2 && Toks[1].IsNumber()) {
      cx.ToAnis = olx_abs(Toks[1].ToInt());
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
  return true;
}
//..............................................................................
void TIns::UpdateParams()  {
  for( size_t i =0; i < Ins.Count(); i++ )  {
    if( Ins.GetObject(i) == NULL )  continue;  // might happen if load failed
    for( size_t j=0; j < Ins.GetObject(i)->Count(); j++ )  {
      if( Ins.GetObject(i)->GetObject(j) != NULL )  {
        Ins.GetObject(i)->GetString(j) =
          Ins.GetObject(i)->GetObject(j)->GetLabel();
      }
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
  if (_ParseIns(rm, toks) || ParseRestraint(rm, toks)) return true;
  // check for uniqueness
  if (CheckUniq) {
    bool unique = false;
    for (size_t i = 0; i < Ins.Count(); i++) {
      if (Ins[i].Equalsi(toks[0])) {
        TInsList *ps = Ins.GetObject(i);
        if (ps->Count() == (toks.Count()-1)) {
          for (size_t j=0; j < ps->Count(); j++) {
            if (!ps->GetString(j).Equalsi(toks[j+1])) {
              unique = true;
              break;
            }
          }
          if (!unique)
            return false;
        }
      }
    }
  }
  TInsList& Params = *(new TInsList(toks.Count()-1));
  for (size_t i=1; i < toks.Count(); i++) {
    Params[i-1] = toks[i];
    Params.GetObject(i-1) = GetAsymmUnit().FindCAtom(toks[i]);
  }
  // end
  Ins.Add(toks[0], &Params);
  return true;
}
//..............................................................................
void TIns::HyphenateIns(const olxstr &InsName, const olxstr &Ins,
  TStrList &Res, int sz)
{
  olxstr Tmp = Ins;
  if (Tmp.Length() > 80 - InsName.Length()) {
    while (Tmp.Length() > 80 - InsName.Length())  {
      size_t spindex = Tmp.LastIndexOf(' ', sz - InsName.Length());
      if (spindex != InvalidIndex && spindex > 0)  {
        Res.Add(InsName + Tmp.SubStringTo(spindex));
        Tmp = olxstr(' ') << Tmp.SubStringFrom(spindex + 1);
      }
      else {
        Res.Add(InsName + Tmp.SubStringTo(80 - InsName.Length() - 2));
        Tmp = Tmp.SubStringFrom(sz - InsName.Length() - 2);
      }
    }
    if (!Tmp.IsEmpty())
      Res.Add(InsName + Tmp);
  }
  else
    Res.Add(InsName + Tmp);
}
//..............................................................................
void TIns::HyphenateIns(const olxstr& Ins, TStrList& Res, int sz)  {
  bool MultiLine = false, added = false;
  olxstr Tmp(Ins), Tmp1;
  while( Tmp.Length() >= (size_t)sz )  {
    MultiLine = true;
    size_t spindex = Tmp.LastIndexOf(' ', sz-3); // for the right hyphenation
    if( spindex != InvalidIndex && spindex > 0 )  {
      if( added )  Tmp1 = ' ';
      Tmp1 << Tmp.SubStringTo(spindex);
      if( !Tmp1.IsEmpty() && Tmp1.GetLast() != ' ')
        Tmp1 << ' ';
      Tmp1 << '=';
      Res.Add(Tmp1);
      Tmp.Delete(0, spindex+1); // remove the space
      added = true;
    }
    else  {
      Tmp1 = ' ';  // a space before each line
      Tmp1 << Tmp.SubStringTo(sz-1);
      Res.Add(Tmp1);
      Tmp.Delete(0, sz-1);
    }
  }
  if( !Tmp.IsEmpty() )  {  // add the last bit
    if( MultiLine )
      Tmp.Insert(' ', 0);
    Res.Add(Tmp);
  }
}
//..............................................................................
void TIns::FixTypeListAndLabels() {
  sorted::PointerPointer<const cm_Element> elms;
  for (size_t i=0; i < GetRM().GetUserContent().Count(); i++) {
    elms.Add(&GetRM().GetUserContent()[i].element);
  }
  for (size_t i=0; i < GetAsymmUnit().ResidueCount(); i++ ) {
    TResidue& residue = GetAsymmUnit().GetResidue(i);
    LabelCorrector lc(TXApp::GetMaxLabelLength());
    for (size_t j=0; j < residue.Count(); j++) {
      if (residue[j].IsDeleted()) {
        continue;
      }
      residue[j].SetSaved(false);
      // fix the SFAC, if wrong
      if (!elms.Contains(&residue[j].GetType())) {
        if (residue[j].GetType() != iQPeakZ) {
          elms.Add(&residue[j].GetType());
          GetRM().AddUserContent(residue[j].GetType(), 1.0);
        }
      }
      lc.Correct(residue[j]);
    }
  }
}
//..............................................................................
void TIns::SaveForSolution(const olxstr& FileName, const olxstr& sMethod,
  const olxstr& comments, bool rems, bool save_atoms)
{
  TStrList SL, mtoks;
  if (sMethod.IsEmpty())
    mtoks.Add("TREF");
  else {
    mtoks.Strtok(sMethod, "\\n");
    size_t spi = mtoks[0].IndexOf(' ');
    if (spi != InvalidIndex)
      RefMod.SetSolutionMethod(mtoks[0].SubStringTo(spi));
    else
      RefMod.SetSolutionMethod(mtoks[0]);
  }

  UpdateParams();
  SL.Add("TITL ") << GetTitle();

  if (!comments.IsEmpty() && rems)
    SL.Add("REM ") << comments;
// try to estimate Z'
  SL.Add(_CellToString());
  SL.Add(_ZerrToString());
  _SaveSymm(SL);
  if (this->FindIns("NEUT") != 0) {
    SL.Add("NEUT");
  }
  SL.Add(EmptyString());
  TStrList sfac = SaveSfacUnit(RefMod, SL, SL.Count()-1, false);

  _SaveSizeTemp(SL);
  if( rems )
    _SaveHklInfo(SL, true);
  //_SaveFVar(RefMod, SL);

  SL.AddList(mtoks);
  SL.Add(EmptyString());
  if (save_atoms) {
    const TAsymmUnit &au = GetAsymmUnit();
    for (size_t i=0; i < au.AtomCount(); i++)  {
      TCAtom &a = au.GetAtom(i);
      if( a.IsDeleted() || a.GetType().z < 2) continue;
      olxstr& aline = SL.Add(a.GetLabel());
      aline.RightPadding(6, ' ', true);
      aline << (sfac.IndexOf(a.GetType().symbol)+1);
      aline.RightPadding(aline.Length()+4, ' ', true);
      for( size_t j=0; j < 3; j++ )
        aline << olxstr::FormatFloat(-5, a.ccrd()[j] ) << ' ';
      double v = a.GetOccu() + 10;
      aline << olxstr::FormatFloat(-5, v) << ' ';
    }
  }
  SL.Add("HKLF ") << RefMod.GetHKLFStr();
  SL.Add("END");
#ifdef _UNICODE
  TEFile::WriteLines(FileName, TCStrList(SL));
#else
  TEFile::WriteLines(FileName, SL);
#endif
}
//..............................................................................
TStrList::const_list_type TIns::SaveSfacUnit(const RefinementModel& rm,
  TStrList& list, size_t pos)
{
  TStrList sfac;
  sorted::ObjectComparable<olxstr> elms;
  short state = 0;
  for (size_t i = 0; i < rm.GetUserContent().Count(); i++) {
    olxstr es = rm.GetUserContent()[i].element.symbol;
    if (rm.GetUserContent()[i].charge != 0) {
      es << olx_sign_char(rm.GetUserContent()[i].charge);
      if (olx_abs(rm.GetUserContent()[i].charge) > 1) {
        es << olx_abs(rm.GetUserContent()[i].charge);
      }
    }
    if (!elms.AddUnique(es).b) {
      continue;
    }
    XScatterer* sd = rm.FindSfacData(es);
    if (sd != NULL && sd->IsSFAC()) {
      TStrList lines;
      HyphenateIns(sd->ToInsString(), lines);
      list.Insert(pos, lines);
      pos += lines.Count();
      state = 1;
    }
    else {
      if (state == 1) {
        list.Insert(pos++, "SFAC ") << es;
        state = 2;
      }
      else  {
        if (state == 2) { // SFAC added and pos incremented
          list[pos - 1] << ' ' << es;
        }
        else if (state == 0)  {  // nothing added yet
          list[pos++] << "SFAC " << es;
          state = 2;
        }
      }
    }
    sfac << es;
  }
  for (size_t i = 0; i < rm.aunit.AtomCount(); i++) {
    TCAtom &a = rm.aunit.GetAtom(i);
    if (a.IsDeleted()) continue;
    int ch = a.GetCharge();
    if (ch == 0) {
      continue;
    }
    int chp = olx_abs(ch);
    olxstr l = olxstr(a.GetType().symbol) << olx_sign_char(ch);
    if (chp > 1) {
      l << olx_abs(ch);
    }
    XScatterer* sd = rm.FindSfacData(l);
    if (!elms.Contains(l)) {
      olx_object_ptr<XScatterer> scp;
      if (sd == 0) {
        cm_Absorption_Coefficient_Reg ac;
        double en = rm.expl.GetRadiationEnergy();
        scp = (sd = new XScatterer(a.GetType(), en));
        sd->SetLabel(l);
        sd->SetFpFdp(a.GetType().CalcFpFdp(en) - a.GetType().z);
        try {
          double absorpc =
            ac.CalcMuOverRhoForE(en, *ac.locate(a.GetType().symbol));
          sd->SetMu(absorpc*a.GetType().GetMr() / 0.6022142);
        }
        catch (...) {
          TBasicApp::NewLogEntry() << "Could not locate absorption data for: " <<
            l;
          sd->SetMu(0);
        }
      }
      TStrList lines;
      HyphenateIns(sd->ToInsString(), lines);
      list.Insert(pos, lines);
      pos += lines.Count();
      sfac << l;
      elms.Add(l);
    }
  }
  for (size_t i = 0; i < rm.SfacCount(); i++)  {
    if (rm.GetSfacData(i).IsDISP() &&
      elms.Contains(rm.GetSfacData(i).GetLabel()))
    {
      list.Insert(pos++, rm.GetSfacData(i).ToInsString());
    }
  }

  olxstr& unit = list.Insert(pos++, "UNIT");
  for (size_t i = 0; i < rm.GetUserContent().Count(); i++) {
    unit << ' ' << rm.GetUserContent()[i].count;
  }
  for (size_t i = rm.GetUserContent().Count(); i < sfac.Count(); i++) {
    unit << " 0";
  }
  return sfac;
}
//..............................................................................
TStrList::const_list_type TIns::SaveSfacUnit(const RefinementModel& rm,
  TStrList& list, size_t pos, bool save_disp)
{
  TStrList sfac;
  short state = 0;
  SortedObjectList<olxstr, olxstrComparator<false> > elms;
  for (size_t i=0; i < rm.GetUserContent().Count(); i++) {
    elms.AddUnique(rm.GetUserContent()[i].element.symbol);
    XScatterer* sd = rm.FindSfacData(rm.GetUserContent()[i].element.symbol);
    if (sd != NULL && sd->IsSFAC()) {
      TStrList lines;
      HyphenateIns(sd->ToInsString(), lines);
      list.Insert(pos, lines);
      pos += lines.Count();
      state = 1;
    }
    else {
      if( state == 1 )  {
        list.Insert(pos++,  "SFAC ") << rm.GetUserContent()[i].element.symbol;
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
    sfac << rm.GetUserContent()[i].element.symbol;
  }
  if (save_disp) {
    for( size_t i=0; i < rm.SfacCount(); i++ )  {
      if( rm.GetSfacData(i).IsDISP() &&
        elms.Contains(rm.GetSfacData(i).GetLabel()))
      {
        list.Insert(pos++, rm.GetSfacData(i).ToInsString());
      }
    }
  }
  olxstr& unit = list.Insert(pos++, "UNIT");
  for (size_t i=0; i < rm.GetUserContent().Count(); i++) {
    unit << ' ' << rm.GetUserContent()[i].count;
  }
  return sfac;
}
//..............................................................................
void TIns::_SaveAtom(RefinementModel& rm, TCAtom& a, int& part, int& afix,
  double &spec, TStrList* sfac, TStrList &sl, TIndexList* index,
  bool checkSame, bool checkResi)
{
  if( a.IsDeleted() || a.IsSaved() )  return;
  if( checkResi && a.GetResiId() != 0 )  {
    const TResidue& resi = rm.aunit.GetResidue(a.GetResiId());
    sl.Add(resi.ToString());
    for( size_t i=0; i < resi.Count(); i++ )
      _SaveAtom(rm, resi[i], part, afix, spec, sfac, sl, index, true, false);
    return;
  }
  if (checkSame && olx_is_valid_index(a.GetSameId())) {  // "
    TSameGroup& sg = rm.rSAME[a.GetSameId()];
    if (sg.IsValidForSave()) {
      for (size_t i = 0; i < sg.DependentCount(); i++) {
        if (!sg.GetDependent(i).IsValidForSave())
          continue;
        olxstr tmp("SAME ");
        tmp << olxstr(sg.GetDependent(i).Esd12).TrimFloat() << ' '
          << olxstr(sg.GetDependent(i).Esd13).TrimFloat() << ' '
          << sg.GetDependent(i).GetAtoms().GetExpression();
        HyphenateIns(tmp, sl);
      }
      if (sg.GetAtoms().IsExplicit()) {
        TAtomRefList atoms = sg.GetAtoms().ExpandList(rm);
        for (size_t i = 0; i < atoms.Count(); i++) {
          _SaveAtom(rm, atoms[i].GetAtom(), part, afix, spec, sfac, sl, index,
            false, checkResi);
        }
      }
      return;
    }
  }
  if (a.GetUisoOwner() != NULL && !a.GetUisoOwner()->IsSaved()) {
    _SaveAtom(rm, *a.GetUisoOwner(), part, afix, spec, sfac, sl, index,
      checkSame, checkResi);
  }
  if (a.GetSpecialPositionDeviation() != spec) {
    sl.Add("SPEC ") << a.GetSpecialPositionDeviation();
  }
  if (a.GetPart() != part) {
    if (part != 0 && a.GetPart() != 0)
      sl.Add("PART 0");
    sl.Add("PART ") << (int)a.GetPart();
  }
  TAfixGroup* ag = a.GetDependentAfixGroup();
  int atom_afix = a.GetAfix();
  if ((atom_afix != afix || afix == 1 || afix == 2) && atom_afix > 0) {
    if (!TAfixGroup::HasExcplicitPivot(afix) ||
      !TAfixGroup::IsDependent(atom_afix))
    {
      TAfixGroup* _ag = a.GetParentAfixGroup();
      if (_ag != NULL) {
        olxstr& str = sl.Add("AFIX ") << atom_afix;
        if (_ag->GetD() != 0) {
          str << ' ' << _ag->GetD();
        }
        if (_ag->GetSof() != 0)  {
          str << ' ' << _ag->GetSof();
          if (_ag->GetU() != 0) {
            str << ' ' << _ag->GetU();
          }
        }
      }
      else {
        olxstr& str = sl.Add("AFIX ") << atom_afix;
        if (ag != NULL) {
          if (ag->GetD() != 0) {
            str << ' ' << ag->GetD();
          }
          if (ag->GetSof() != 0) {
            str << ' ' << ag->GetSof();
            if (ag->GetU() != 0) {
              str << ' ' << ag->GetU();
            }
          }
        }
      }
    }
  }
  afix = atom_afix;
  part = a.GetPart();
  spec = a.GetSpecialPositionDeviation();
  index_t spindex;
  if (a.GetType() == iQPeakZ) {
    spindex = (sfac == NULL ? -2 : (index_t)sfac->IndexOf('C') + 1);
  }
  else {
    int ch = a.GetCharge();
    olxstr l = a.GetType().symbol;
    if (ch != 0) {
      l << (ch > 0 ? '+' : '-');
      if (olx_abs(ch) > 1) {
        l << olx_abs(ch);
      }
    }
    spindex = (sfac == NULL ? -2 : (index_t)sfac->IndexOf(l) + 1);
  }
  HyphenateIns(AtomToString(rm, a, spindex == 0 ? 1 : spindex), sl);
  a.SetSaved(true);
  if (index != NULL)  index->Add(a.GetId());
  for (size_t i=0; i < a.DependentHfixGroupCount(); i++) {
    TAfixGroup& hg = a.GetDependentHfixGroup(i);
    size_t sc = 0;
    for (size_t j=0; j < hg.Count(); j++) {
      if (!hg[j].IsDeleted() && !hg[j].IsSaved()) {
        _SaveAtom(rm, hg[j], part, afix, spec, sfac, sl, index, checkSame,
          checkResi);
        sc++;
      }
    }
    if (sc != 0 && afix > 0) {
      sl.Add("AFIX 0");
      afix = 0;
    }
  }
  if (ag != NULL) {  // save dependent rigid group
    size_t sc = 0;
    for (size_t i=0; i < ag->Count(); i++) {
      if (!(*ag)[i].IsDeleted() && !(*ag)[i].IsSaved()) {
        _SaveAtom(rm, (*ag)[i], part, afix, spec, sfac, sl, index, checkSame,
          checkResi);
        sc++;
      }
    }
    if (afix > 0) {
      sl.Add("AFIX 0");
      afix = 0;
    }
  }
}
//..............................................................................
void TIns::SaveToStrings(TStrList& SL) {
  FixTypeListAndLabels();
  for (size_t i=0; i < GetAsymmUnit().AtomCount(); i++) {
    TCAtom &ca = GetAsymmUnit().GetAtom(i);
    if (ca.IsDeleted()) continue;
    olxstr lb = ca.GetLabel();
    lb.Replace('\t' ,' ').Replace('_', ' ');
    if (lb.Contains(' ')) {
      TBasicApp::NewLogEntry(logError) << "Changing invalid atom labels for" <<
        (olxstr(' ').quote() << ca.GetLabel());
      ca.SetLabel(lb.DeleteChars(' '), false);
    }
  }
  ValidateRestraintsAtomNames(GetRM());
  UpdateParams();
  TStrList sfac = SaveHeader(SL, false);
  SaveExtras(SL, NULL, NULL, GetRM());
  SL.Add(EmptyString());
  int afix = 0, part = 0;
  double spec = 0;
  uint32_t fragmentId = ~0;
  TCAtomPList peaks;
  for (size_t i=0; i < GetAsymmUnit().ResidueCount(); i++) {
    TResidue& residue = GetAsymmUnit().GetResidue(i);
    if (i != 0 && !residue.IsEmpty()) {
      SL.Add();
      SL.Add(residue.ToString());
      fragmentId = ~0;
    }
    for (size_t j=0; j < residue.Count(); j++) {
      TCAtom& ac = residue[j];
      if (ac.IsDeleted() || ac.IsSaved()) {
        continue;
      }
      if (ac.GetType() == iQPeakZ) {
        peaks << ac;
        continue;
      }
      if (ac.GetFragmentId() != fragmentId || !olx_is_valid_index(fragmentId)) {
        if (olx_is_valid_index(fragmentId))
          SL.Add(EmptyString());
        fragmentId = ac.GetFragmentId();
      }
      if (ac.GetParentAfixGroup() != NULL &&
        !ac.GetParentAfixGroup()->GetPivot().IsDeleted())
      {
        continue;
      }
      _SaveAtom(GetRM(), ac, part, afix, spec, &sfac, SL, NULL, true, false);
    }
  }
  if (afix != 0) {
    SL.Add("AFIX 0");
  }
  SL.Add("HKLF ") << RefMod.GetHKLFStr();
  SL.Add(EmptyString());
  SL.Add("END");
  for (size_t i = 0; i < peaks.Count(); i++) {
    TCAtom &p = *peaks[i];
    SL.Add(p.GetLabel()).stream(' ') << "1" <<
      p.ccrd()[0] << p.ccrd()[1] << p.ccrd()[2] << "11" << "0.05" << p.GetQPeak();
  }
}
//..............................................................................
void TIns::_DrySaveAtom(TCAtom& a, TSizeList &indices,
  bool checkSame, bool checkResi)
{
  if (a.IsDeleted() || a.IsSaved())  return;
  if (checkResi && a.GetResiId() != 0)  {
    const TResidue& resi = a.GetParent()->GetResidue(a.GetResiId());
    for (size_t i = 0; i < resi.Count(); i++) {
      _DrySaveAtom(resi[i], indices, true, false);
    }
    return;
  }
  if (checkSame && olx_is_valid_index(a.GetSameId())) {
    TSameGroup& sg = a.GetParent()->GetRefMod()->rSAME[a.GetSameId()];
    if (sg.IsValidForSave())  {
      if (sg.GetAtoms().IsExplicit()) {
        TAtomRefList atoms = sg.GetAtoms().ExpandList(*a.GetParent()->GetRefMod());
        for (size_t i = 0; i < atoms.Count(); i++)
          _DrySaveAtom(atoms[i].GetAtom(), indices, false, checkResi);
      }
      return;
    }
  }
  if (a.GetUisoOwner() != NULL && !a.GetUisoOwner()->IsSaved()) {
    _DrySaveAtom(*a.GetUisoOwner(), indices, checkSame, checkResi);
  }
  TAfixGroup* ag = a.GetDependentAfixGroup();
  indices.Add(a.GetTag());
  a.SetSaved(true);
  for (size_t i = 0; i < a.DependentHfixGroupCount(); i++) {
    TAfixGroup& hg = a.GetDependentHfixGroup(i);
    for (size_t j = 0; j < hg.Count(); j++)  {
      if (!hg[j].IsDeleted() && !hg[j].IsSaved()) {
        _DrySaveAtom(hg[j], indices, checkSame, checkResi);
      }
    }
  }
  if (ag != NULL) {  // save dependent rigid group
    for (size_t i = 0; i < ag->Count(); i++)  {
      if (!(*ag)[i].IsDeleted() && !(*ag)[i].IsSaved()) {
        _DrySaveAtom((*ag)[i], indices, checkSame, checkResi);
      }
    }
  }
}
//..............................................................................
TSizeList::const_list_type TIns::DrySave(const TCAtomPList& atoms) {
  TSizeList rv;
  for (size_t i = 0; i < atoms.Count(); i++) {
    atoms[i]->SetSaved(false);
    atoms[i]->SetTag((index_t)i);
  }
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i]->IsDeleted() || atoms[i]->IsSaved())
      continue;
    if (atoms[i]->GetParentAfixGroup() != NULL &&
      !atoms[i]->GetParentAfixGroup()->GetPivot().IsDeleted())
    {
      continue;
    }
    _DrySaveAtom(*atoms[i], rv, true, false);
  }
  // move all not saved atoms to the end
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (!atoms[i]->IsSaved()) {
      rv.Add(atoms[i]->GetTag());
    }
  }
  return rv;
}
//..............................................................................
bool TIns::Adopt(TXFile &XF, int) {
  Clear();
  GetRM().Assign(XF.GetRM(), true);
  try {
    TSpaceGroup& sg = XF.GetLastLoaderSG();
    Title << " in " << sg.GetFullName();
  }
  catch (...)  {}
  Title = XF.LastLoader()->GetTitle();
  if (RefMod.GetRefinementMethod().IsEmpty())
    RefMod.SetRefinementMethod("L.S.");
  return true;
}
//..............................................................................
void TIns::UpdateAtomsFromStrings(RefinementModel& rm,
  const TIndexList& index, TStrList& SL, TStrList& Instructions)
{
  if (index.IsEmpty()) return;
  size_t atomCount = 0;
  ParseContext cx(rm);
  Preprocess(SL);
  for (size_t i=0; i < index.Count(); i++) {
    if ((size_t)index[i] >= rm.aunit.AtomCount())
      throw TInvalidArgumentException(__OlxSourceInfo, "atom index");
    TCAtom &ca = rm.aunit.GetAtom(index[i]);
    if (ca.GetParentAfixGroup() != NULL) {
      ca.GetParentAfixGroup()->Clear();
    }
    if (ca.GetDependentAfixGroup() != NULL) {
      ca.GetDependentAfixGroup()->Clear();
    }
    if (ca.GetExyzGroup() != NULL) {
      ca.GetExyzGroup()->Clear();
    }
    ca.SetFixedType(false);
  }
  TTypeList<olx_pair_t<TCAtom *, olxstr> > atom_labels;
  for( size_t i=0; i < SL.Count(); i++ )  {
    olxstr Tmp = olxstr::DeleteSequencesOf<char>(SL[i], true);
    if (Tmp.IsEmpty()) {
      continue;
    }
    const size_t exi = Tmp.IndexOf('!');
    if( exi != InvalidIndex )
      Tmp.SetLength(exi);
    TStrList Toks(Tmp, ' ');
    if (Toks.IsEmpty()) {
      continue;
    }
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
      if( elm == NULL )  {// wrong SFAC
        throw TInvalidArgumentException(__OlxSourceInfo,
          "unknown element symbol");
      }
      TCAtom* atom = NULL;
      if (atomCount >= index.Count()) {
        atom = &rm.aunit.NewAtom(cx.Resi);
      }
      else {
        atom = &rm.aunit.GetAtom(index[atomCount]);
        if( cx.Resi != NULL )
          cx.Resi->Add(*atom);
      }
      _ParseAtom(Toks, cx, atom);
      atomCount++;
      atom_labels.AddNew(atom, Toks[0]);
      atom->SetType(*elm);
      if (atom->GetType().z > 1 &&
        (cx.AfixGroups.IsEmpty() || cx.AfixGroups.Top().GetB()->GetAfix() != 3))
      {
        cx.LastRideable = atom;
      }
      _ProcessAfix(*atom, cx);
    }
  }
  _ProcessSame(cx);
  ParseRestraints(cx.rm, Instructions);
  Instructions.Pack();
  for (size_t i=0; i < atom_labels.Count(); i++)
    atom_labels[i].a->SetLabel(atom_labels[i].GetB(), false);
}
//..............................................................................
bool TIns::SaveAtomsToStrings(RefinementModel& rm, const TCAtomPList& CAtoms,
  TIndexList& index, TStrList& SL, RefinementModel::ReleasedItems* processed)
{
  if (CAtoms.IsEmpty()) return false;
  int part = 0, afix = 0;
  double spec = 0;
  SaveRestraints(SL, &CAtoms, processed, rm);
  _SaveFVar(rm, SL);
  for (size_t i = 0; i < CAtoms.Count(); i++) {
    CAtoms[i]->SetSaved(false);
  }
  for (size_t i=0; i < CAtoms.Count(); i++) {
    if( CAtoms[i]->IsSaved() )  continue;
    TCAtom& ac = *CAtoms[i];
    if (ac.GetParentAfixGroup() != NULL &&
      !ac.GetParentAfixGroup()->GetPivot().IsDeleted())
    {
      continue;
    }
    _SaveAtom(rm, ac, part, afix, spec, NULL, SL, &index);
  }
  SaveExtras(SL, &CAtoms, processed, rm);
  return true;
}
//..............................................................................
void TIns::SavePattSolution(const olxstr& FileName,
  const TTypeList<TPattAtom>& atoms, const olxstr& comments )
{
  TPtrList<const cm_Element> BasicAtoms;
  for( size_t i=0; i < GetRM().GetUserContent().Count(); i++ )
    BasicAtoms.Add(GetRM().GetUserContent()[i].element);
  TSizeList Sfacs;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    cm_Element* elm = XElementLib::FindBySymbolEx(atoms[i].GetName());
    if( elm == NULL )  {
      throw TFunctionFailedException(__OlxSourceInfo,
       olxstr("Unknown element: ") << atoms[i].GetName());
    }
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
  SL.Add(EmptyString());
  SaveSfacUnit(GetRM(), SL, SL.Count()-1, true);

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
  SL.Add(EmptyString());
  for( size_t i=0; i < atoms.Count(); i++ )  {
    olxstr& aline = SL.Add(atoms[i].GetName());
    aline.RightPadding(6, ' ', true);
    aline << (Sfacs[i]+1);
    aline.RightPadding(aline.Length()+4, ' ', true);
    for( size_t j=0; j < 3; j++ )
      aline << olxstr::FormatFloat(-5, atoms[i].GetCrd()[j] ) << ' ';
    double v = atoms[i].GetOccup() + 10;
    aline << olxstr::FormatFloat(-5, v) << ' ';
  }
  SL.Add("HKLF ") << GetRM().GetHKLFStr();
  SL.Add(EmptyString());
#ifdef _UNICODE
  TEFile::WriteLines(FileName, TCStrList(SL));
#else
  TEFIle::WriteLines(FileName, SL);
#endif
}
//..............................................................................
void TIns::_ProcessAfix(TCAtom& a, ParseContext& cx) {
  if (cx.AfixGroups.IsEmpty()) {
    return;
  }
  if (cx.SetNextPivot) {
    cx.AfixGroups.Top().b->SetPivot(a);
    cx.SetNextPivot = false;
    return;
  }
  if (cx.AfixGroups.Top().GetA() == 0) {
    cx.AfixGroups.Pop();
  }
  else {
    if (cx.AfixGroups.Top().GetC()) {
      if (a.GetType() != iHydrogenZ) {
        cx.AfixGroups.Top().a--;
        cx.AfixGroups.Top().b->AddDependent(a);
      }
    }
    else {
      cx.AfixGroups.Top().a--;
      cx.AfixGroups.Top().b->AddDependent(a);
    }
  }
}
//..............................................................................
TCAtom* TIns::_ParseAtom(TStrList& Toks, ParseContext& cx, TCAtom* atom)  {
  double QE[6];
  if( atom == NULL )
    atom = &cx.au.NewAtom(cx.Resi);
  for( short j=0; j < 3; j ++ )
    cx.rm.Vars.SetParam(*atom, catom_var_name_X+j, Toks[2+j].ToDouble());
  atom->SetPart(cx.Part);
  atom->SetSpecialPositionDeviation(cx.SPEC);
  // update the context
  cx.Last = atom;
  if( !cx.Same.IsEmpty() && cx.Same.GetLast().GetB() == NULL )
    cx.Same.GetLast().b = atom;

  cx.rm.Vars.SetParam(*atom, catom_var_name_Sof,
    cx.PartOccu == 0 ? Toks[5].ToDouble() : cx.PartOccu);

  if( Toks.Count() == 12 )  {  // full ellipsoid
    for( short j=0; j < 6; j ++ )  {
      QE[j] = cx.rm.Vars.SetParam(
        *atom, catom_var_name_U11+j, Toks[j+6].ToDouble());
    }
    cx.au.UcifToUcart(QE);
    TEllipsoid& elp = cx.au.NewEllp().Initialise(QE);
    atom->AssignEllp(&elp);
    if( atom->GetEllipsoid()->IsNPD() )  {
      TBasicApp::NewLogEntry(logInfo) << "Not positevely defined: " << Toks[0];
      atom->SetUiso(0);
    }
    else
      atom->SetUiso(atom->GetEllipsoid()->GetUeq());
    cx.LastWithU = atom;
  }
  else  {
    if( Toks.Count() > 6 )
      cx.rm.Vars.SetParam(*atom, catom_var_name_Uiso, Toks[6].ToDouble());
    else // incomplete data...
      atom->SetUiso(4*caDefIso*caDefIso);
    if( Toks.Count() >= 8 ) // some other data as Q-peak itensity
      atom->SetQPeak(Toks[7].ToDouble());
    if( atom->GetUiso() <= -0.5 )  {  // a value fixed to the pivot atom value
      if( cx.LastWithU == NULL )  {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("Invalid Uiso proxy for: ") << Toks[0]);
      }
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
        memset(&QE[0], 0, sizeof(QE));
        QE[0] = QE[1] = QE[2] = atom->GetUiso();
        atom->UpdateEllp(QE);
      }
    }
  }
  return atom;
}
//..............................................................................
olxstr TIns::AtomToString(RefinementModel& rm, TCAtom& CA, index_t SfacIndex)  {
  evecd Q(6);
  olxstr Tmp = CA.GetLabel();
  Tmp.RightPadding(6, ' ', true);
  if( SfacIndex < 0 )
    Tmp << CA.GetType().symbol;
  else
    Tmp << SfacIndex;

  Tmp.RightPadding(Tmp.Length()+4, ' ', true);
  for( short j=0; j < 3; j++ )  {
    Tmp << olxstr::FormatFloat(-5,
      rm.Vars.GetParam(CA, catom_var_name_X+j)) << ' ';
  }

  // save occupancy
  Tmp << olxstr::FormatFloat(-5,
    rm.Vars.GetParam(CA, catom_var_name_Sof)) << ' ';
  // save Uiso, Uanis
  if( CA.GetEllipsoid() != NULL )  {
    CA.GetEllipsoid()->GetShelxQuad(Q);
    rm.aunit.UcartToUcif(Q);

    for( short j = 0; j < 6; j++ )  {
      Tmp << olxstr::FormatFloat(-5,
        rm.Vars.GetParam(CA, catom_var_name_U11+j, Q[j])) << ' ';
    }
  }
  else  {
    double v = (CA.GetUisoOwner() == NULL
      ? rm.Vars.GetParam(CA, catom_var_name_Uiso) : -CA.GetUisoScale());
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
  Tmp.stream(' ') <<
    GetAsymmUnit().GetAxes()[0] <<
    GetAsymmUnit().GetAxes()[1] <<
    GetAsymmUnit().GetAxes()[2] <<
    GetAsymmUnit().GetAngles()[0] <<
    GetAsymmUnit().GetAngles()[1] <<
    GetAsymmUnit().GetAngles()[2];
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
  Tmp.stream(' ') <<
    GetAsymmUnit().GetAxisEsds()[0] <<
    GetAsymmUnit().GetAxisEsds()[1] <<
    GetAsymmUnit().GetAxisEsds()[2] <<
    GetAsymmUnit().GetAngleEsds()[0] <<
    GetAsymmUnit().GetAngleEsds()[1] <<
    GetAsymmUnit().GetAngleEsds()[2];
  return Tmp;
}
//..............................................................................
void TIns::_SaveSymm(TStrList& SL)  {
  SL.Add("LATT ") << GetAsymmUnit().GetLatt();
  if( GetAsymmUnit().MatrixCount() == 1 )  {
    if( !GetAsymmUnit().GetMatrix(0).r.IsI() )
      SL.Add("SYMM ") << TSymmParser::MatrixToSymm(GetAsymmUnit().GetMatrix(0));
  }
  else  {
    for( size_t i=0; i < GetAsymmUnit().MatrixCount(); i++ )
      SL.Add("SYMM ") << TSymmParser::MatrixToSymm(GetAsymmUnit().GetMatrix(i));
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
void TIns::_SaveHklInfo(TStrList& SL, bool solution) {
  if (!solution) {
    if (GetRM().HasMERG()) {
      SL.Add("MERG ") << GetRM().GetMERG();
    }
    if (!GetRM().GetBASF().IsEmpty()) {
      HyphenateIns("BASF ", GetRM().GetBASFStr(), SL);
    }
    if (GetRM().HasSHEL()) {
      SL.Add("SHEL ") << GetRM().GetSHELStr();
    }
    if (GetRM().HasTWIN()) {
      SL.Add("TWIN ") << GetRM().GetTWINStr();
    }
    if (GetRM().HasOMIT()) {
      SL.Add("OMIT ") << GetRM().GetOMITStr();
    }
    for (size_t i = 0; i < GetRM().OmittedCount(); i++) {
      const vec3i& r = GetRM().GetOmitted(i);
      SL.Add("OMIT ") << r[0] << ' ' << r[1] << ' ' << r[2];
    }
  }
}
//..............................................................................
bool Ins_ProcessRestraint(const TCAtomPList* atoms,
  const TSimpleRestraint& sr, const RefinementModel &rm)
{
  if (sr.IsEmpty() && !sr.IsAllNonHAtoms()) return false;
  if (atoms == NULL)  return true;
  if (sr.IsAllNonHAtoms()) return true;
  TTypeList<ExplicitCAtomRef> ra = sr.GetAtoms().ExpandList(rm);
  for (size_t i=0; i < ra.Count(); i++)
    if (atoms->Contains(ra[i].GetAtom()))
      return true;
  return false;
}
//..............................................................................
olxstr TIns::RestraintToString(const TSimpleRestraint &sr,
  const TIns::RCInfo &ri, const TCAtomPList *atoms)
{
  const RefinementModel &rm = sr.GetParent().GetRM();
  if (!Ins_ProcessRestraint(atoms, sr, rm))
    return EmptyString();
  if (sr.GetAtoms().IsExplicit()) {
    // has lower atom count limit?
    if ((int)sr.GetAtoms().ExpandList(rm).Count() < ri.atom_limit)
      return EmptyString();
  }
  bool def = rm.IsDEFSSet() ? rm.IsDefaultRestraint(sr) :
    (rm.DoShowRestraintDefaults() ? false : rm.IsDefaultRestraint(sr));
  olxstr line = sr.GetIdName();
  if (!sr.GetAtoms().GetResi().IsEmpty())
    line << '_' << sr.GetAtoms().GetResi();
  if( ri.has_value > 0 )  // has value and is first?
    line << ' ' << rm.Vars.GetParam(sr, 0);
  if( ri.esd_cnt > 0 && !def )  // uses Esd?
    line << ' ' << sr.GetEsd();
  if( ri.esd_cnt > 1 && !def )  // has extra Esd?
    line << ' ' << sr.GetEsd1();
  if( ri.has_value < 0 && !def )  // has value and is last?
    line << ' ' << rm.Vars.GetParam(sr, 0);
  line << ' ' << sr.GetAtoms().GetExpression();
  return line;
}
//..............................................................................
void TIns::SaveRestraints(TStrList& SL, const TCAtomPList* atoms,
  RefinementModel::ReleasedItems* processed, RefinementModel& rm)
{
  size_t oindex = SL.Count();
  typedef olx_pair_t<TSRestraintList*,RCInfo> ResInfo;
  TStringToList<olxstr, ResInfo> restraints;
  // fixed distances, has value, one esd, no atom limit
  restraints.Add("DFIX", ResInfo(&rm.rDFIX, RCInfo(1, 1, 2)));
  // similar distances, no value, one esd, no atom limit
  restraints.Add("SADI", ResInfo(&rm.rSADI, RCInfo(0, 1, 4)));
  // fixed "angles", has value, one esd, no atom limit
  restraints.Add("DANG", ResInfo(&rm.rDANG, RCInfo(1, 1, 2)));
  /* fixed chiral atomic volumes, has value, one esd, no atom limit - volume
  restrained for each atom */
  restraints.Add("CHIV", ResInfo(&rm.rCHIV, RCInfo(1, 1, 1)));
  // planar groups
  restraints.Add("FLAT", ResInfo(&rm.rFLAT, RCInfo(0, 1, 4)));
  // rigid bond restraint
  restraints.Add("DELU", ResInfo(&rm.rDELU, RCInfo(0, 2, -1)));
  // similar U restraint
  restraints.Add("SIMU", ResInfo(&rm.rSIMU, RCInfo(-1, 2, -1)));
  // rigid body restraint
  restraints.Add("RIGU", ResInfo(&rm.rRIGU, RCInfo(0, 2, -1)));
  // Uanis restraint to behave like Uiso
  restraints.Add("ISOR", ResInfo(&rm.rISOR, RCInfo(0, 2, -1)));
  // equivalent EADP constraint
  restraints.Add("EADP", ResInfo(&rm.rEADP, RCInfo(0, 0, 2, false)));

  if (rm.IsDEFSSet()) {
    SL.Add("DEFS ") << rm.GetDEFSStr();
  }

  if (TBasicApp::GetInstance().GetOptions().GetBoolOption("group_restraints")) {
    typedef AnAssociation3<size_t, size_t, size_t> triple_t;
    TTypeList<triple_t> sorted_res;

    for (size_t i = 0; i < restraints.Count(); i++) {
      ResInfo& r = restraints.GetObject(i);
      for (size_t j = 0; j < r.GetA()->Count(); j++) {
        TSimpleRestraint& sr = (*r.a)[j];
        sr.UpdateResi();
        const RCInfo& ri = r.GetB();
        TTypeList<ExplicitCAtomRef> ra = sr.Validate().GetAtoms().ExpandList(rm);
        size_t min_id = InvalidIndex;
        for (size_t k = 0; k < ra.Count(); k++) {
          if (ra[k].GetAtom().GetId() < min_id) {
            min_id = ra[k].GetAtom().GetId();
          }
        }
        sorted_res.Add(Association::New(min_id, i, j));
      }
    }
    QuickSorter::Sort(sorted_res,
      ComplexComparator::Make(FunctionAccessor::MakeConst(&triple_t::GetA),
      TPrimitiveComparator()));

    for (size_t i = 0; i < sorted_res.Count(); i++) {
      const triple_t &t = sorted_res[i];
      ResInfo& r = restraints.GetObject(t.GetB());
      const RCInfo& ri = r.GetB();
      TSimpleRestraint& sr = (*r.a)[t.GetC()];
      olxstr line = RestraintToString(sr, ri, atoms);
      if (line.IsEmpty())  continue;
      HyphenateIns(line, SL);
      if (processed != NULL)
        processed->restraints.Add(sr);
    }
  }
  else {
    for (size_t i = 0; i < restraints.Count(); i++) {
      ResInfo& r = restraints.GetObject(i);
      for (size_t j = 0; j < r.GetA()->Count(); j++) {
        TSimpleRestraint& sr = (*r.a)[j];
        sr.UpdateResi();
        const RCInfo& ri = r.GetB();
        sr.Validate();
        olxstr line = RestraintToString(sr, ri, atoms);
        if (line.IsEmpty()) {
          continue;
        }
        HyphenateIns(line, SL);
        if (processed != NULL)
          processed->restraints.Add(sr);
      }
    }
  }

  // equivalent EXYZ constraint
  for (size_t i = 0; i < rm.ExyzGroups.Count(); i++) {
    TExyzGroup& sr = rm.ExyzGroups[i];
    if (sr.IsEmpty())  continue;
    olxstr Tmp = "EXYZ";
    for (size_t j = 0; j < sr.Count(); j++)  {
      if (!sr[j].IsDeleted()) {
        Tmp << ' ' << sr[j].GetResiLabel();
      }
    }
    HyphenateIns(Tmp, SL);
  }
  // store the eqiv ...
  for (size_t i = 0; i < rm.UsedSymmCount(); i++) {
    olxstr Tmp = "EQIV ";
    Tmp << '$' << (i + 1) << ' ' << TSymmParser::MatrixToSymm(rm.GetUsedSymm(i));
    SL.Insert(oindex + i, Tmp);
  }
  for (size_t i = 0; i < rm.Vars.EquationCount(); i++) {
    if (!rm.Vars.GetEquation(i).Validate())  continue;
    SL.Add("SUMP ") << rm.Vars.GetSUMPStr(i);
    //if( processed != NULL )
    //  processed->equations.Add( &rm.Vars.GetEquation(i) );
  }
  for (size_t i = 0; i < rm.rSAME.Count(); i++) {
    if (!rm.rSAME[i].GetAtoms().IsExplicit()) {
      rm.rSAME[i].GetAtoms().UpdateResi();
      olxstr &l = SL.Add("SAME");
      if (!rm.rSAME[i].GetAtoms().GetResi().IsEmpty()) {
        l << '_' << rm.rSAME[i].GetAtoms().GetResi();
      }
      if (rm.DoShowRestraintDefaults() || !rm.IsDefaultRestraint(rm.rSAME[i])) {
        l << ' ' << rm.rSAME[i].Esd12 << ' ' << rm.rSAME[i].Esd13;
      }
      l << ' ' << rm.rSAME[i].GetAtoms().GetExpression();
      if (processed != NULL) {
        processed->sameList.Add(rm.rSAME[i]);
      }
    }
  }
  SL.Add(EmptyString());
  if (atoms == NULL) {
    for (size_t i = 0; i < rm.FragCount(); i++)
      rm.GetFrag(i).ToStrings(SL);
  }
  else {
    sorted::PointerPointer<const Fragment> saved;
    for (size_t i = 0; i < atoms->Count(); i++) {
      const int m = TAfixGroup::GetM((*atoms)[i]->GetAfix());
      if (m < 17)  continue;
      const Fragment* frag = rm.FindFragByCode(m);
      if (frag == NULL)  {
        throw TFunctionFailedException(__OlxSourceInfo,
          "could not locate the FRAG for fitted group");
      }
      if (saved.Contains(frag)) {
        continue;
      }
      saved.Add(frag);
      frag->ToStrings(SL);
    }
  }
}
//..............................................................................
void TIns::SaveExtras(TStrList& SL, const TCAtomPList* atoms,
  RefinementModel::ReleasedItems* processed, RefinementModel& rm)
{
  TStrList extras(rm.WriteInsExtras(atoms, false), NewLineSequence());
  if (!extras.IsEmpty()) {
    SL.Add("REM <olex2.extras>");
    for (size_t i = 0; i < extras.Count(); i++)
      HyphenateIns("REM ", extras[i], SL);
    SL.Add("REM </olex2.extras>");
  }
}
//..............................................................................
void TIns::ValidateRestraintsAtomNames(RefinementModel& rm, bool report)  {
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
  restraints.Add(&rm.rAngle);
  restraints.Add(&rm.rDihedralAngle);
  restraints.Add(&rm.rFixedUeq);
  restraints.Add(&rm.rSimilarUeq);
  LabelCorrector lc(rm.aunit, TXApp::GetMaxLabelLength());
  olxstr err_names;
  for (size_t i=0; i < restraints.Count(); i++) {
    TSRestraintList& srl = *restraints[i];
    for (size_t j=0; j < srl.Count(); j++) {
      if (!srl[j].GetAtoms().GetResi().IsEmpty()) continue;
      TTypeList<ExplicitCAtomRef> atoms = srl[j].GetAtoms().ExpandList(rm);
      for (size_t k=0; k < atoms.Count(); k++) {
        if (!lc.IsGlobal(atoms[k].GetAtom()))
          err_names << ' ' << atoms[k].GetAtom().GetLabel();
      }
    }
  }
  // equivalent EXYZ constraint
  for( size_t i=0; i < rm.ExyzGroups.Count(); i++ )  {
    TExyzGroup& sr = rm.ExyzGroups[i];
    for (size_t j = 0; j < sr.Count(); j++) {
      lc.CorrectGlobal(sr[j]);
    }
  }
  if (report && !err_names.IsEmpty()) {
    TBasicApp::NewLogEntry(logError) << "The following atom names are used in"
      " restraints but cannot be globally resolved: " << err_names;
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
TStrList::const_list_type TIns::SaveHeader(TStrList& SL,
  bool ValidateRestraintNames)
{
  SL.Add("TITL ") << GetTitle();
  for (size_t i = 0; i < Ins.Count(); i++) {
    TInsList* L = Ins.GetObject(i);
    if (L == NULL)  continue;  // if load failed
    if (Ins[i].Equalsi("REM")) {
      HyphenateIns(Ins[i] + ' ', L->Text(' '), SL);
    }
  }
  SL.Add(_CellToString());
  SL.Add(_ZerrToString());
  _SaveSymm(SL);
  if (FindIns("NEUT") != 0) {
    SL.Add("NEUT");
  }
  SL.Add(EmptyString());
  TStrList::const_list_type rv = SaveSfacUnit(GetRM(), SL, SL.Count() - 1);
  if (ValidateRestraintNames)
    ValidateRestraintsAtomNames(GetRM());
  SaveRestraints(SL, NULL, NULL, GetRM());
  _SaveRefMethod(SL);
  _SaveSizeTemp(SL);
  for (size_t i=0; i < GetRM().InfoTabCount(); i++) {
    if (GetRM().GetInfoTab(i).IsValid()) {
      GetRM().GetInfoTab(i).UpdateResi();
      SL.Add(GetRM().GetInfoTab(i).InsStr());
    }
  }
  GetRM().Conn.ToInsList(SL);
  // copy "unknown" instructions except rems

  for (size_t i=0; i < Ins.Count(); i++) {
    TInsList* L = Ins.GetObject(i);
    if (L == NULL) {  // if load failed
      continue;
    }
    if (!Ins[i].Equalsi("REM") && !Ins[i].Equalsi("NEUT")) {
      HyphenateIns(Ins[i] + ' ', L->Text(' '), SL);
    }
  }
  SL << Skipped;
  if (!GetRM().OmittedAtoms().IsEmpty())
    SL.Add("OMIT ") << GetRM().OmittedAtoms().GetExpression();

  if (GetRM().HasEXTI()) {
    SL.Add("EXTI ") << GetRM().GetEXTI().GetV();
  }

  _SaveHklInfo(SL, false);

  olxstr& wght = SL.Add("WGHT ");
  for (size_t i = 0; i < RefMod.used_weight.Count(); i++) {
    wght << RefMod.used_weight[i];
    if (i + 1 < RefMod.used_weight.Count())
      wght << ' ';
  }
  if (RefMod.used_weight.IsEmpty())
    wght << "0.1";
  _SaveFVar(RefMod, SL);
  return rv;
}
//..............................................................................
void TIns::ParseHeader(const TStrList& in)  {
  // clear all but the atoms
  for (size_t i=0; i < AsymmUnit.AtomCount(); i++)
    AsymmUnit.GetAtom(i).SetFixedType(false);
  for( size_t i=0; i < Ins.Count(); i++ )
    delete Ins.GetObject(i);
  Ins.Clear();
  Skipped.Clear();
  Title.SetLength(0);
  GetRM().Clear(rm_clear_DEF);
  GetAsymmUnit().ClearMatrices();
// end clear, start parsing
  TStrList toks, lst(in);
  Preprocess(lst);
  ParseContext cx(GetRM());
  cx.ins = this;
  _ReadExtras(lst, cx);
  for (size_t i = 0; i < lst.Count(); i++)  {
    try  {
      olxstr Tmp = olxstr::DeleteSequencesOf<char>(lst[i], ' ');
      if( Tmp.IsEmpty() )  continue;
      size_t ci = Tmp.IndexOf('!');
      if (ci != InvalidIndex) {
        if (ci == 0) {
          lst[i] = olxstr("REM ") << lst[i].SubStringFrom(1).TrimWhiteChars();
          Tmp = olxstr("REM ") << Tmp.SubStringFrom(1).TrimWhiteChars();
        }
        else
          Tmp.SetLength(ci);
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
  for (size_t i=0; i < cx.Symm.Count(); i++) {
    GetAsymmUnit().AddMatrix(TSymmParser::SymmToMatrix(cx.Symm[i]));
  }
  Ins.Pack();
  ParseRestraints(cx.rm, Ins);
  Ins.Pack();
  _ProcessSame(cx);
  _FinishParsing(cx);
}
//..............................................................................
bool TIns::ParseRestraint(RefinementModel& rm, const TStrList& _toks) {
  if (_toks.IsEmpty())  return false;
  TStrList toks(_toks);
  if (toks[0].Equalsi("EQIV") && toks.Count() >= 3) {
    try {
      rm.AddUsedSymm(TSymmParser::SymmToMatrix(toks.Text(EmptyString(), 2)), toks[1]);
    }
    catch (const TExceptionBase &e) {
      throw TFunctionFailedException(__OlxSourceInfo, e, "to parse EQIV");
    }
    return true;
  }
  TSRestraintList* srl = NULL;
  short RequiredParams = 1, AcceptsParams = 1;
  bool AcceptsAll = false;
  double Esd1Mult = 0, DefVal = 0, esd = 0, esd1 = 0;
  double *Vals[] = { &DefVal, &esd, &esd1 };
  bool use_var_manager = true, check_resi = true;
  if (toks[0].StartsFromi("olex2.")) {
    check_resi = use_var_manager = false;
  }
  // extract residue
  olxstr resi, ins_name = toks[0];
  const size_t resi_ind = toks[0].IndexOf('_');
  if (check_resi && resi_ind != InvalidIndex) {
    resi = toks[0].SubStringFrom(resi_ind + 1);
    ins_name = toks[0].SubStringTo(resi_ind);
  }

  if (ins_name.Equalsi("EXYZ")) {
    rm.AddEXYZ(toks.SubListFrom(1));
    return true;
  }
  else if (ins_name.Equalsi("DFIX")) {
    srl = &rm.rDFIX;
    RequiredParams = 1;  AcceptsParams = 2;
    Vals[0] = &DefVal;  Vals[1] = &esd;
  }
  else if (ins_name.Equalsi("DANG")) {
    srl = &rm.rDANG;
    RequiredParams = 1;  AcceptsParams = 2;
    Vals[0] = &DefVal;  Vals[1] = &esd;
  }
  else if (ins_name.Equalsi("SADI")) {
    srl = &rm.rSADI;
    RequiredParams = 0;  AcceptsParams = 1;
    Vals[0] = &esd;
  }
  else if (ins_name.Equalsi("CHIV")) {
    srl = &rm.rCHIV;
    RequiredParams = 1;  AcceptsParams = 2;
    Vals[0] = &DefVal;  Vals[1] = &esd;
  }
  else if (ins_name.Equalsi("FLAT")) {
    srl = &rm.rFLAT;
    RequiredParams = 0;  AcceptsParams = 1;
    Vals[0] = &esd;
  }
  else if (ins_name.Equalsi("DELU")) {
    srl = &rm.rDELU;
    Esd1Mult = 1;
    RequiredParams = 0;  AcceptsParams = 2;
    Vals[0] = &esd;  Vals[1] = &esd1;
    AcceptsAll = true;
  }
  else if (ins_name.Equalsi("RIGU")) {
    srl = &rm.rRIGU;
    Esd1Mult = 1;
    RequiredParams = 0;  AcceptsParams = 2;
    Vals[0] = &esd;  Vals[1] = &esd1;
    AcceptsAll = true;
  }
  else if (ins_name.Equalsi("SIMU")) {
    srl = &rm.rSIMU;
    Esd1Mult = 2;
    DefVal = 2;
    RequiredParams = 0;  AcceptsParams = 3;
    Vals[0] = &esd;  Vals[1] = &esd1;  Vals[2] = &DefVal;
    AcceptsAll = true;
    use_var_manager = false;
  }
  else if (ins_name.Equalsi("ISOR")) {
    srl = &rm.rISOR;
    Esd1Mult = 2;
    RequiredParams = 0;  AcceptsParams = 2;
    Vals[0] = &esd;  Vals[1] = &esd1;
    AcceptsAll = true;
  }
  else if (ins_name.Equalsi("EADP")) {
    srl = &rm.rEADP;
    RequiredParams = 0;  AcceptsParams = 0;
  }
  else if (ins_name.Equalsi(rm.rAngle.GetIdName()) ||
    ins_name.Equalsi("Angle"))
  {
    srl = &rm.rAngle;
    RequiredParams = 1;  AcceptsParams = 2;
    Vals[0] = &DefVal;  Vals[1] = &esd;
  }
  else if (ins_name.Equalsi(rm.rDihedralAngle.GetIdName()) ||
    ins_name.Equalsi("DAngle"))
  {
    srl = &rm.rDihedralAngle;
    RequiredParams = 1;  AcceptsParams = 2;
    Vals[0] = &DefVal;  Vals[1] = &esd;
  }
  else if (ins_name.Equalsi(rm.rFixedUeq.GetIdName()) ||
    ins_name.Equalsi("FIXU"))
  {
    srl = &rm.rFixedUeq;
    RequiredParams = 1;  AcceptsParams = 2;
    Vals[0] = &DefVal;  Vals[1] = &esd;
  }
  else if (ins_name.Equalsi(rm.rSimilarUeq.GetIdName()) ||
    ins_name.Equalsi("SIMQ"))
  {
    srl = &rm.rSimilarUeq;
    RequiredParams = 0;  AcceptsParams = 1;
    Vals[0] = &esd;
  }
  else if (ins_name.Equalsi(rm.rSimilarAdpVolume.GetIdName()) ||
    ins_name.Equalsi("SIMV"))
  {
    srl = &rm.rSimilarAdpVolume;
    RequiredParams = 0;  AcceptsParams = 1;
    Vals[0] = &esd;
  }
  else {
    srl = NULL;
  }
  if (srl != NULL) {
    TSimpleRestraint& sr = srl->AddNew();
    esd = sr.GetEsd();
    esd1 = sr.GetEsd1();
    size_t index = 1;
    if (toks.Count() > 1 && toks[1].IsNumber()) {
      if (toks.Count() > 2 && toks[2].IsNumber()) {
        if (toks.Count() > 3 && toks[3].IsNumber()) {  // three numerical params
          if (AcceptsParams < 3) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "too many numerical parameters");
          }
          *Vals[0] = toks[1].ToDouble();
          *Vals[1] = toks[2].ToDouble();
          *Vals[2] = toks[3].ToDouble();
          index = 4;
        }
        else {  // two numerical params
          if (AcceptsParams < 2) {
            throw TInvalidArgumentException(__OlxSourceInfo,
              "too many numerical parameters");
          }
          *Vals[0] = toks[1].ToDouble();
          *Vals[1] = toks[2].ToDouble();
          index = 3;
        }
      }
      else {
        if (AcceptsParams < 1) {
          throw TInvalidArgumentException(__OlxSourceInfo,
            "too many numerical parameters");
        }
        *Vals[0] = toks[1].ToDouble();
        index = 2;
      }
    }
    if (use_var_manager) {
      rm.Vars.SetParam(sr, 0, DefVal);
    }
    else {
      sr.SetValue(DefVal);
    }
    sr.SetEsd(esd);
    if (Vals[0] == &esd) {
      sr.SetEsd1((index <= 2) ? esd*Esd1Mult : esd1);
    }
    else {
      sr.SetEsd1(esd1);
    }
    if (AcceptsAll && toks.Count() <= index) {
      sr.SetAllNonHAtoms(true);
    }
    else {
      sr.AtomsFromExpression(toks.Text(' ', index), resi);
    }
    srl->ValidateRestraint(sr);
    if (!Ins_ProcessRestraint(NULL, sr, rm) && DoPreserveInvalid()) {
      TBasicApp::NewLogEntry() <<
        (olxstr("Preserving invalid instruction: ").quote() << toks.Text(' '));
      return false;
    }
    return true;
  }
  return false;
}
