/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "hkl.h"
#include "lst.h"
#include "ins.h"
#include "cif.h"
#include "emath.h"
#include "efile.h"
#include "estrlist.h"
#include "exception.h"
#include "ematrix.h"
#include "symmlib.h"
#include "math/composite.h"

//..............................................................................
THklFile::THklFile()  {
  Basis.I();
  Init();
}
//..............................................................................
THklFile::THklFile(const mat3d& hkl_transformation)
  : Basis(hkl_transformation)
{
  Init();
}
//..............................................................................
void THklFile::Init() {
  Cell.Resize(0);
  CellEsd.Resize(0);
  Radiation = 0;
  Hkl3D = NULL;
}
//..............................................................................
void THklFile::Clear()  {
  Refs.DeleteItems().Clear();
  Clear3D();
}
//..............................................................................
void THklFile::UpdateMinMax(const TReflection& r)  {
  if( Refs.IsEmpty() )  {
    // set starting values
    MinHkl[0] = MaxHkl[0] = r.GetH();
    MinHkl[1] = MaxHkl[1] = r.GetK();
    MinHkl[2] = MaxHkl[2] = r.GetL();
    MinI = MaxI = r.GetI();
    MinIS = MaxIS = r.GetS();
  }
  else  {
    vec3i::UpdateMinMax(r.GetHkl(), MinHkl, MaxHkl);
    if( r.GetI() < MinI )  {  MinI = r.GetI();  MinIS = r.GetS();  }
    if( r.GetI() > MaxI )  {  MaxI = r.GetI();  MaxIS = r.GetS();  }
  }
}
//..............................................................................
void THklFile::Clear3D()  {
  if( Hkl3D == NULL )  return;
  for( int i=MinHkl[0]; i <= MaxHkl[0]; i++ ) {
    for( int j=MinHkl[1]; j <= MaxHkl[1]; j++ )
      for( int k=MinHkl[2]; k <= MaxHkl[2]; k++ )
        if( Hkl3D->Value(i,j,k) != NULL )
          delete Hkl3D->Value(i,j,k);
  }
  delete Hkl3D;
  Hkl3D = NULL;
}

bool THklFile::LoadFromFile(const olxstr& FN, TIns* ins,
  bool* ins_initialised)
{
  try  {
    Clear();
    TEFile::CheckFileExists(__OlxSourceInfo, FN);
    TCStrList SL;
    bool ZeroRead = false, 
      HklFinished = false,
      HasBatch = false,
      FormatInitialised = false;
    SL.LoadFromFile(FN);
    if( SL.IsEmpty() )
      throw TEmptyFileException(__OlxSrcInfo, FN);
    {  // validate if 'real' HKL, not fcf
      if( !IsHKLFileLine(SL[0]) )  {
        TCif cif;
        try  {  cif.LoadFromStrings(SL);  }
        catch(TExceptionBase& e) {
          throw TFunctionFailedException(__OlxSrcInfo, e,
            "unsupported file format");
        }
        // find firt data block with reflections...
        cif_dp::cetTable* hklLoop = NULL;
        for( size_t i = 0; i < cif.BlockCount(); i++ )  {
          hklLoop = cif.GetBlock(i).table_map.Find("_refln", NULL);
          if( hklLoop != NULL )  {
            cif.SetCurrentBlock(i);
            break;
          }
        }
        if( hklLoop == NULL )
          throw TInvalidArgumentException(__OlxSourceInfo, "no hkl loop found");
        const size_t hInd = hklLoop->ColIndex("_refln_index_h");
        const size_t kInd = hklLoop->ColIndex("_refln_index_k");
        const size_t lInd = hklLoop->ColIndex("_refln_index_l");
        size_t mInd = hklLoop->ColIndex("_refln_F_squared_meas");
        if (mInd == InvalidIndex)
          mInd = hklLoop->ColIndex("_refln_F_meas");
        size_t sInd = hklLoop->ColIndex("_refln_F_squared_sigma");
        if (sInd == InvalidIndex)
          sInd = hklLoop->ColIndex("_refln_F_sigma");
        if( (hInd|kInd|lInd|mInd|sInd) == InvalidIndex )  {
          throw TInvalidArgumentException(__OlxSourceInfo,
            "could not locate <h k l meas sigma> data");
        }
        for( size_t i=0; i < hklLoop->RowCount(); i++ )  {
          const cif_dp::CifRow& r = (*hklLoop)[i];
          Refs.Add(
            new TReflection(
            r[hInd]->GetStringValue().ToInt(),
            r[kInd]->GetStringValue().ToInt(),
            r[lInd]->GetStringValue().ToInt(),
            r[mInd]->GetStringValue().ToDouble(),
            r[sInd]->GetStringValue().ToDouble()
            ));
          UpdateMinMax(*Refs.GetLast());
        }
        if( ins != NULL && cif.FindEntry("_cell_length_a") != NULL )  {
          ins->GetRM().Assign(cif.GetRM(), true);
          if( ins_initialised != NULL )
            *ins_initialised = true;
        }
        return true;
      }
    }
    const bool apply_basis = !Basis.IsI();
    size_t removed_cnt = 0;
    const size_t line_cnt = SL.Count();
    Refs.SetCapacity(line_cnt);
    for( size_t i=0; i < line_cnt; i++ )  {
      const olxcstr& line = SL[i];
      if( !ZeroRead && line.Length() < 28 )  continue;
      if( !FormatInitialised )  {
        if( line.Length() >= 32 )
          HasBatch = true;
        FormatInitialised = true;
      }
      if( ZeroRead && !HklFinished )  {
        if( !line.SubString(0,4).IsNumber() ||
          !line.SubString(4,4).IsNumber() ||
          !line.SubString(8,4).IsNumber() ||
          !line.SubString(12,8).IsNumber() ||
          !line.SubString(20,8).IsNumber() )
        {
          HklFinished = true; 
          i--;  // reset to the non-hkl line
        }
      }
      if( !HklFinished )  {
        const int h = line.SubString(0,4).ToInt(),
          k = line.SubString(4,4).ToInt(),
          l = line.SubString(8,4).ToInt();
        // end of the reflections included into calculations
        if( h == 0 && k == 0 && l == 0 )  {
          ZeroRead = true;
          continue;
        }
        try  {
          TReflection* ref = HasBatch ?
            new TReflection(h, k, l, line.SubString(12,8).ToDouble(),
              line.SubString(20,8).ToDouble(),
              line.SubString(28,4).IsNumber() ? line.SubString(28,4).ToInt()
              : 1)
            :
            new TReflection(h, k, l, line.SubString(12,8).ToDouble(),
              line.SubString(20,8).ToDouble());
          ref->SetOmitted(ZeroRead);
          if( apply_basis ) {
            vec3d nh = Basis*vec3d(ref->GetHkl());
            vec3i nih = nh.Round<int>();
            if (!nh.Equals(nih, 0.004) || nih.IsNull()) {
              delete ref;
              removed_cnt++;
              continue;
            }
            ref->SetHkl(nih);
          }
          UpdateMinMax(*ref);
          Refs.Add(ref);
          ref->SetTag(Refs.Count());
        }
        catch(const TExceptionBase& e)  {
          throw TFunctionFailedException(__OlxSourceInfo, e,
            olxstr("HKL file line ") << (i+1));
        }
      }
      else  {
        if( ins == NULL )  {
          for( size_t j=i; j < SL.Count(); j++ )  {
            olxstr line = SL[j].Trim(' ');
            if( line.StartsFromi("CELL") )  {
              TStrList Toks(line, ' ');
              if( Toks.Count() == 8 ) {
                try {
                  Radiation = Toks[1].ToDouble();
                  evecd cell(6); // use tmp in the case CELL is invalid
                  for (size_t k=2; k < Toks.Count(); k++)
                    cell[k-2] = Toks[k].ToDouble();
                  Cell = cell;
                }
                catch (const TExceptionBase &e) {
                  throw TFunctionFailedException(__OlxSourceInfo, e, "reading CELL");
                }
              }
            }
            else if( line.StartsFromi("ZERR") )  {
              TStrList Toks(line, ' ');
              if( Toks.Count() == 8 ) {
                try {
                  evecd cell_esd(6); // use tmp in the case CELL is invalid
                  for (size_t k=2; k < Toks.Count(); k++)
                    cell_esd[k-2] = Toks[k].ToDouble();
                  CellEsd = cell_esd;
                  if (!Cell.IsEmpty())
                    break;
                }
                catch (const TExceptionBase &e) {
                  throw TFunctionFailedException(__OlxSourceInfo, e, "reading ZERR");
                }
              }
            }
          }
          break;
        }
        ins->Clear();
        ins->SetTitle(
          TEFile::ChangeFileExt(TEFile::ExtractFileName(FN),
            EmptyString()) << " imported from HKL file");
        bool cell_found = false, sfac_found = false;
        for( size_t j=i; j < SL.Count(); j++ )  {
          olxstr line = SL[j].Trim(' ');
          if( line.IsEmpty() )  continue;
          if( line.StartsFromi("CELL") )  {
            TStrList Toks(line, ' ');
            if( Toks.Count() != 8 ) {
              throw TFunctionFailedException(__OlxSourceInfo,
                "invalid CELL format");
            }
            ins->GetRM().expl.SetRadiation(Radiation = Toks[1].ToDouble());
            ins->GetAsymmUnit().GetAxes() = vec3d(Toks[2].ToDouble(),
              Toks[3].ToDouble(), Toks[4].ToDouble());
            ins->GetAsymmUnit().GetAngles() = vec3d(Toks[5].ToDouble(),
              Toks[6].ToDouble(), Toks[7].ToDouble());
            Cell.Assign(CompositeVector::Make(
              vec3d_list() << ins->GetAsymmUnit().GetAxes() <<
                ins->GetAsymmUnit().GetAngles()),
              6);
            cell_found = true;
          }
          if( line.StartsFromi("ZERR") )  {
            TStrList Toks(line, ' ');
            if( Toks.Count() != 8 ) {
              throw TFunctionFailedException(__OlxSourceInfo,
                "invalid ZERR format");
            }
            ins->GetAsymmUnit().SetZ((short)olx_round(Toks[1].ToDouble()));
            ins->GetAsymmUnit().GetAxisEsds() = vec3d(Toks[2].ToDouble(),
              Toks[3].ToDouble(), Toks[4].ToDouble());
            ins->GetAsymmUnit().GetAngleEsds() = vec3d(Toks[5].ToDouble(),
              Toks[6].ToDouble(), Toks[7].ToDouble());
            CellEsd.Assign(CompositeVector::Make(
              vec3d_list() << ins->GetAsymmUnit().GetAxisEsds() <<
                ins->GetAsymmUnit().GetAngleEsds()),
              6);
            cell_found = true;
          }
          else if( line.StartsFromi("SFAC") )  {
            TStrList Toks(line, ' ');  // do the validation
            TStrList unit;
            for( size_t k=1; k < Toks.Count(); k++ )  {
              if( !XElementLib::IsAtom(Toks[k]) ) {
                throw TFunctionFailedException(__OlxSourceInfo,
                  olxstr("invalid element ") << Toks[k]);
              }
              unit.Add('1');
            }
            ins->GetRM().SetUserContentType(Toks.SubListFrom(1));
            ins->GetRM().SetUserContentSize(unit);
            sfac_found = true;
          }
          else if( line.StartsFromi("TEMP") ||
                   line.StartsFromi("SIZE") ||
                   line.StartsFromi("REM"))
          {
            ins->AddIns(line, ins->GetRM());
          }
          else if( line.StartsFromi("UNIT") ) {
            ins->GetRM().SetUserContentSize(
              TStrList(line.SubStringFrom(5), ' '));
          }
        }
        if( !cell_found || !sfac_found ) {
          throw TFunctionFailedException(__OlxSourceInfo,
            "could no locate valid CELL/SFAC instructions");
        }
        if( ins_initialised != NULL )
          *ins_initialised = true;
        break;
      }
    }
    if (removed_cnt != 0) {
      TBasicApp::NewLogEntry(logError) <<
        "HKL transformation leads to non-integral/invalid Miller indices";
      TBasicApp::NewLogEntry(logError) << "Removed: " << removed_cnt <<
        " invalid reflections";
    }
  }
  catch(const TExceptionBase& e)  {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
  if( Refs.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "no reflections found");
  return true;
}
//..............................................................................
bool THklFile::SaveToFile(const olxstr& FN)  {
  return THklFile::SaveToFile(FN, Refs, false);
}
//..............................................................................
void THklFile::UpdateRef(const TReflection& R)  {
  size_t ind = olx_abs(R.GetTag())-1;
  if( ind >= Refs.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "reflection tag");
  Refs[ind]->SetOmitted(R.IsOmitted());
}
//..............................................................................
int THklFile::HklCmp(const TReflection &R1, const TReflection &R2)  {
  int r = R1.CompareTo(R2);
  if( r == 0 )  {  // for unmerged data ...
    if( R1.GetI() < R2.GetI() )  return -1;
    if( R1.GetI() > R2.GetI() )  return 1;
  }
  return r;
}
//..............................................................................
void THklFile::InitHkl3D() {
  if (Hkl3D != NULL)  return;
  volatile TStopWatch sw(__FUNC__);
  TArray3D<TRefPList*> &hkl3D = *(new TArray3D<TRefPList*>(MinHkl, MaxHkl));
  for (size_t i=0; i < Refs.Count(); i++) {
    TReflection *r1 = Refs[i];
    TRefPList *&rl = hkl3D(r1->GetHkl());
    if (rl == NULL)
      rl = new TRefPList();
    rl->Add(r1);
  }
  Hkl3D = &hkl3D;
}
//..............................................................................
ConstPtrList<TReflection> THklFile::AllRefs(const vec3i& idx,
  const smatd_list& ml)
{
  TRefPList rv;
  SortedObjectList<vec3i, TComparableComparator> ri;
  for (size_t i=0; i < ml.Count(); i++) {
    ri.AddUnique(TReflection::MulHkl(idx, ml[i]));
  }
  InitHkl3D();
  for (size_t j=0; j < ri.Count(); j++) {
    if (!Hkl3D->IsInRange(ri[j])) continue;
    TRefPList* r = Hkl3D->Value(ri[j]);
    if (r != NULL)
      rv.AddList(*r);
  }
  return rv;
}
//..............................................................................
void THklFile::Append(TReflection& hkl)  {
  UpdateMinMax(hkl);
  Refs.Add(hkl)->SetTag(Refs.Count());
}
//..............................................................................
void THklFile::EndAppend()  {
  //Refs.QuickSorter.SortSF(Refs, HklCmp);
}
//..............................................................................
void THklFile::Append(const TRefPList& hkls)  {
  if( hkls.IsEmpty() )  return;
  for( size_t i=0; i < hkls.Count(); i++ )  {
    UpdateMinMax(*hkls[i]);
    Refs.Add(new TReflection(*hkls[i]))->SetTag(Refs.Count());
  }
  EndAppend();
}
//..............................................................................
void THklFile::Append(const THklFile& hkls)  {
  if( !hkls.RefCount() )  return;
  Append(hkls.Refs);
}
//..............................................................................
bool THklFile::SaveToFile(const olxstr& FN, const TRefPList& refs,
  bool Append)
{
  if( refs.IsEmpty() )  return true;
  if( Append && TEFile::Exists(FN) )  {
    THklFile F;
    F.LoadFromFile(FN);
    F.Append(refs);
    F.SaveToFile(FN);
  }
  else  {
    double scale = 0;
    for( size_t i=0; i < refs.Count(); i++ )  {
      if( refs[i]->GetI() > scale )
        scale = refs[i]->GetI();
    }
    scale = (scale > 99999 ? 99999/scale : 1);
    TEFile out(FN, "w+b");
    TReflection NullRef(0, 0, 0, 0, 0);
    if( refs[0]->GetBatch() != TReflection::NoBatchSet )
      NullRef.SetBatch(0);
    const size_t ref_str_len = NullRef.ToString().Length();
    const size_t bf_sz = ref_str_len+1; 
    olx_array_ptr<char> ref_bf(new char[bf_sz]);
    for( size_t i=0; i < refs.Count(); i++ )  {
      if( !refs[i]->IsOmitted() )
        out.Writecln(refs[i]->ToCBuffer(ref_bf, bf_sz, scale), ref_str_len);
    }
    out.Writecln(NullRef.ToCBuffer(ref_bf, bf_sz, 1), ref_str_len);
    for( size_t i=0; i < refs.Count(); i++ )  {
      if( refs[i]->IsOmitted() )
        out.Writecln(refs[i]->ToCBuffer(ref_bf, bf_sz, scale), ref_str_len);
    }
  }
  return true;
}
//..............................................................................
bool THklFile::SaveToFile(const olxstr& FN, const TRefList& refs)  {
  if( refs.IsEmpty() )  return true;
  double scale = 0;
  for( size_t i=0; i < refs.Count(); i++ )  {
    if( refs[i].GetI() > scale )
      scale = refs[i].GetI();
  }
  scale = (scale > 99999 ? 99999/scale : 1);
  TEFile out(FN, "w+b");
  TReflection NullRef(0, 0, 0, 0, 0);
  if( refs[0].GetBatch() != TReflection::NoBatchSet )
    NullRef.SetBatch(0);
  const size_t ref_str_len = NullRef.ToString().Length();
  const size_t bf_sz = ref_str_len+1; 
  olx_array_ptr<char> ref_bf(new char[bf_sz]);
  for( size_t i=0; i < refs.Count(); i++ )  {
    if( !refs[i].IsOmitted() )
      out.Writecln(refs[i].ToCBuffer(ref_bf, bf_sz, scale), ref_str_len);
  }
  out.Writecln(NullRef.ToCBuffer(ref_bf, bf_sz, 1), ref_str_len);
  for( size_t i=0; i < refs.Count(); i++ )  {
    if( refs[i].IsOmitted() )
      out.Writecln(refs[i].ToCBuffer(ref_bf, bf_sz, scale), ref_str_len);
  }
  return true;
}
//..............................................................................
