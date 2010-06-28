//---------------------------------------------------------------------------//
// namespace TXFiles
// CIF and related data management procedures
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "cif.h"
#include "dataitem.h"

#include "catom.h"
#include "satom.h"
#include "symmparser.h"

#include "unitcell.h"
#include "ellipsoid.h"

#include "bapp.h"
#include "log.h"

#include "symmlib.h"
#include "etime.h"

using namespace exparse::parser_util;

//----------------------------------------------------------------------------//
// TCif function bodies
//----------------------------------------------------------------------------//
TCif::TCif() : FDataNameUpperCase(true)  {  }
//..............................................................................
TCif::~TCif()  {  Clear();  }
//..............................................................................
void TCif::Clear()  {
  for( size_t i=0; i < Lines.Count(); i++ )  {
    if( Lines.GetObject(i) != NULL )
      delete Lines.GetObject(i);
  }
  Lines.Clear();
  Parameters.Clear();
  FDataName = EmptyString;
  FWeightA = EmptyString;
  FWeightB = EmptyString;
  for( size_t i=0; i < Loops.Count(); i++ )
    delete Loops.GetObject(i);
  Loops.Clear();
  GetRM().Clear(rm_clear_ALL);
  GetAsymmUnit().Clear();
  DataManager.Clear();
  Matrices.Clear();
  MatrixMap.Clear();
}
//..............................................................................
void TCif::Format()  {
  for( size_t i=0; i < Lines.Count()-1; i++ )  {
    if( Lines[i].Length() == 0  && Lines[i+1].Length() == 0 )  {
      if( Lines.GetObject(i) != NULL )
        delete Lines.GetObject(i);
    }
  }
  Lines.Pack();
  for( size_t i=0; i < Lines.Count(); i++ )  {
    if( Lines.GetObject(i) == NULL )  continue;
    CifData& D = *Lines.GetObject(i);
    if( D.data.Count() > 1 )
      D.data.TrimWhiteCharStrings();
    for( size_t j=0; j < D.data.Count(); j++ )  {
      if( D.data[j].Length() > 1 )  {
        const olxch ch = D.data[j].CharAt(0);
        if( is_quote(ch) && D.data[j].EndsWith(ch) )  {
          D.data[j] = D.data[j].SubStringFrom(1,1);
          D.quoted = true;
        }
      }
    }
    if( D.data.Count() == 1 )
      D.data[0].TrimWhiteChars();
    else if( D.data.Count() > 1 )
      D.quoted = true;
  }
}
//..............................................................................
bool TCif::ExtractLoop(size_t& start)  {
  if( !Lines[start].StartsFromi("loop_") )  return false;
  TCifLoop& Loop = *(new TCifLoop);
  Loops.Add(EmptyString, &Loop);
  TStrList loop_data;
  TTypeList<AnAssociation2<size_t,bool> > OxfordCols;  // index, delete flag
  bool parse_header = true;
  if( Lines[start].IndexOf(' ') != InvalidIndex )  {
    TStrList toks;
    CIFToks(Lines[start], toks);
    for( size_t i=1; i < toks.Count(); i++ )  {
      if( !parse_header || toks[i].CharAt(0) != '_' )  {
        parse_header = false;
        loop_data.Add(toks[i]);
      }
      else
        Loop.GetTable().AddCol(toks[i]);
    }
  }
  while( parse_header )  {  // skip loop definition
    if( ++start >= Lines.Count() )  {  // // end of file?
      Loops.Last().String = Loop.GetLoopName();
      return true;
    }
    if( Lines[start].IsEmpty() )  continue;
    if( Lines[start].CharAt(0) != '_' )  {  start--;  break;  }
    if( Loop.GetTable().ColCount() != 0 )  {
      /* check that the item actually belongs to the loop, this might happens in the case of empty loops */
      if( olxstr::CommonString(Lines[start], Loop.GetTable().ColName(0)).Length() == 1 )  {
        // special processing of the _oxford loop items
        if( !Lines[start].StartsFrom("_oxford") ||
            olxstr::CommonString(Lines[start].SubStringFrom(7), Loop.GetTable().ColName(0)).Length() <= 1 )
        {
          Loops.Last().String = Loop.GetLoopName();
          start--;  // rewind
          return true;
        }
      }
    }
    bool param_found = false;  // in the case loop header is mixed up with loop data...
    if( Lines[start].IndexOf(' ') == InvalidIndex )  {
      if( Loop.GetTable().ColCount() > 0 )  {  // find and remember oxford loop items...
        if( Lines[start].StartsFrom("_oxford") && olxstr::CommonString(Lines[start], Loop.GetTable().ColName(0)).Length() == 1 )  {
          Lines[start].Replace("_oxford", EmptyString);
          OxfordCols.AddNew(Loop.GetTable().ColCount(), true);
        }
      }
      Loop.GetTable().AddCol(Lines[start]);
    }
    else  {
      TStrList toks;
      CIFToks(Lines[start], toks);
      for( size_t i=0; i < toks.Count(); i++ )  {
        if( param_found || toks[i].CharAt(0) != '_' )  {
          param_found = true;
          loop_data.Add(toks[i]);
        }
        else  {
          if( Loop.GetTable().ColCount() > 0 )  {  // find and remember oxford loop items...
            if( toks[i].StartsFrom("_oxford") && olxstr::CommonString(toks[i], Loop.GetTable().ColName(0)).Length() == 1 )  {
              toks[i].Replace("_oxford", EmptyString);
              OxfordCols.AddNew(Loop.GetTable().ColCount(), true);
            }
          }
          Loop.GetTable().AddCol(toks[i]);
        }
      }
    }
    Lines[start] = EmptyString;
    if( param_found )  break;
  }
  size_t q_cnt = 0;
  while( true )  {  // skip loop data
    while( ++start < Lines.Count() && Lines[start].IsEmpty() )  continue;
    if( start >= Lines.Count() )  break;
    // a new loop or dataset started (not a part of a multi-string value)
    if( (q_cnt%2) == 0 && (Lines[start].StartsFrom('_') || 
      Lines[start].StartsFromi("loop_") || Lines[start].StartsFromi("data_")) )
      break;
    if( Lines[start].CharAt(0) == ';' )  q_cnt++;
    loop_data.Add(Lines[start]);
    Lines[start] = EmptyString;
  }
  Loop.Format(loop_data);
  Loops.Last().String = Loop.GetLoopName();
  start--;
  if( !OxfordCols.IsEmpty() )  {  // re-format the loop to correct the syntax
    TArrayList<size_t> OxfordRows;
    for( size_t i=0; i < Loop.GetTable().RowCount(); i++ )  {
      for( size_t j=0; j < OxfordCols.Count(); j++ )  {
        if( Loop[i][OxfordCols[j].GetA()] != '.' )
          OxfordRows.Add(i);
      }
    }
    if( !OxfordRows.IsEmpty() )  {
      // try to find a reference row, like _atom_site and whatever _label, in reverse order!
      for( size_t i=Loop.GetTable().ColCount(); i > 0; i-- )  {
        const olxstr& col_name = Loop.GetTable().ColName(i-1);
        if( col_name.IndexOf("atom_site") != InvalidIndex && col_name.IndexOf("label") != InvalidIndex )
          OxfordCols.InsertNew(0, i-1, false);
      }
      TCifLoop& ox_loop = AddLoop(olxstr("_oxford") << Loop.GetLoopName());
      for( size_t i=0; i < OxfordCols.Count(); i++ )
        ox_loop.GetTable().AddCol(olxstr("_oxford") << Loop.GetTable().ColName(OxfordCols[i].GetA()));
      for( size_t i=0; i < OxfordRows.Count(); i++ )  {
        TCifRow& row = ox_loop.GetTable().AddRow(EmptyString);
        for( size_t j=0; j < OxfordCols.Count(); j++ )  {
          const olxstr& val = Loop[OxfordRows[i]][OxfordCols[j].GetA()];
          if( val.StartsFrom('\'') || val.StartsFrom('"') )
            row.Set(j, val.SubStringFrom(1,1).TrimWhiteChars(), new StringCifCell(true));
          else
            row.Set(j, val, new StringCifCell(false));
        }
      }
    }
    size_t col_deleted = 0;
    for( size_t i=0; i < OxfordCols.Count(); i++ )  {
      if( !OxfordCols[i].GetB() )  continue;
      size_t col_ind = OxfordCols[i].GetA()-col_deleted;
      for( size_t j=0; j < Loop.GetTable().RowCount(); j++ )
        delete Loop[j].GetObject(col_ind);
      Loop.GetTable().DelCol(col_ind);
    }
  }
  return true;
}
//..............................................................................
void TCif::LoadFromStrings(const TStrList& Strings)  {
  Clear();
  Lines = Strings;
  for( size_t i=0; i < Lines.Count(); i++ )  {
    if( Lines[i].StartsFrom(';') )  {  // skip these things
      while( ++i < Lines.Count() && !Lines[i].StartsFrom(';') )  continue;
      continue;
    }
    Lines[i].DeleteSequencesOf<char>(' ').Trim(' ');
    if( Lines[i].IsEmpty() )  continue;
    size_t spindex = Lines[i].FirstIndexOf('#');  // a comment char
    if( spindex != InvalidIndex )  {
      if( spindex != 0 )  {
        olxstr Tmp = Lines[i];
        Lines[i] = Tmp.SubStringTo(spindex-1);  // remove the '#' character
        Lines.Insert(++i, Tmp.SubStringFrom(spindex));
      }
    }
  }
  for( size_t i=0; i < Lines.Count(); i++ )  {
    const olxstr& line = Lines[i];
    if( line.IsEmpty() )  continue;
    if( line.CharAt(0) == '#')  continue;
    if( ExtractLoop(i) )  continue;
    if( line.CharAt(0) == '_' )  {  // parameter
      olxstr Val, Param;
      size_t spindex = line.FirstIndexOf(' ');
      if( spindex != InvalidIndex )  {
        Param = line.SubStringTo(spindex);
        Val = line.SubStringFrom(spindex+1); // to remove the space
      }
      else
        Param = line;
      CifData *D = Lines.Set(i, Param, Parameters.Add(Param, new CifData(false)).Object).Object;
      if( !Val.IsEmpty() )
        D->data.Add(Val);
      else  {
        olxch Char;
        while( ++i < Lines.Count() && Lines[i].IsEmpty() )  continue;
        if( i >= Lines.Count() )  continue;
        Char = Lines[i].CharAt(0);
        while( Char == '#' && ++i < Lines.Count() )  {
          while( Lines[i].IsEmpty() && ++i < Lines.Count() )  continue;
          if( i >= Lines.Count() )  break;
          Char = Lines[i].CharAt(0);
        }
        if( Char == ';' )  {
          size_t sc_count = 1; 
          if( Lines[i].Length() > 1 )
            D->data.Add(Lines[i].SubStringFrom(1));
          Lines[i] = EmptyString;
          while( ++i < Lines.Count() )  {
            if( !Lines[i].IsEmpty() && Lines[i].CharAt(0) == ';' )  {
              Lines[i] = EmptyString;
              break;
            }
            D->data.Add(Lines[i]);
            Lines[i] = EmptyString;
          }
          D->quoted = true;
        }
        else if( Char = '\'' || Char == '"' )  {
          D->data.Add(Lines[i]);
          Lines[i] = EmptyString;
          continue;
        }
      }
    }
    else if( line.StartsFrom("data_") )  {
      if( FDataNameUpperCase )
        FDataName = line.SubStringFrom(5).UpperCase();
      else
        FDataName = line.SubStringFrom(5);
      FDataName.DeleteSequencesOf(' ');
      Lines[i] = "data_";
      Lines[i] << FDataName;
    }
  }
  Format();
  /******************************************************************/
  /*search for the weigting sceme*************************************/
  CifData* D = FindParam( "_refine_ls_weighting_details");
  if( D != NULL && D->data.Count() == 1 )  {
    const olxstr& tmp = D->data[0];
    for( size_t k=0; k < tmp.Length(); k++ )  {
      if( tmp[k] == '+' )  {
        if( FWeightA.IsEmpty() )  {
          while( tmp[k] != ')' )  {
            k++;
            if( k >= tmp.Length() )  break;
            FWeightA << tmp[k];
          }
          k--;
          continue;
        }
        if( FWeightB.IsEmpty() )  {
          while( tmp[k] != ']' )  {
            k++;
            if( k >= tmp.Length() )  break;
            FWeightB << tmp[k];
          }
          FWeightB.Delete(FWeightB.Length()-1, 1); // remove the [ bracket
        }
      }
    }
  }
  /******************************************************************/
  for( size_t i=0; i < Lines.Count()-1; i++ )  {
    if( (Lines[i].Length()|Lines[i+1].Length()) == 0 )  {
      Lines.Delete(i+1);
      i--;
    }
  }
  Initialize();
}
//..............................................................................
void TCif::SetDataName(const olxstr &S)  {
  olxstr Tmp, Tmp1;
  bool found = false;
  for( size_t i=0; i < Lines.Count(); i++ )  {
    Tmp = olxstr::DeleteSequencesOf<char>(Lines[i], ' ');
    Tmp1 = Tmp.SubString(0,4);
    if( Tmp1 == "data" )  {
      Tmp1 << '_' << S;
      Lines[i] = Tmp1;
      found = true;
    }
  }
  if( !found )  {
      Tmp = "data_";
      Lines.Insert(0, Tmp << S);
  }
  FDataName = S;
}
//..............................................................................
void GroupSection(TStrPObjList<olxstr,TCif::CifData*>& lines, size_t index,
       const olxstr& sectionName, AnAssociation2<size_t,size_t>& indexes)  {
  olxstr tmp;
  for( size_t i=index; i < lines.Count(); i++ )  {
    tmp = lines[i].Trim(' ');
    if( tmp.IsEmpty() || tmp.StartsFromi("loop_") )  continue;
    size_t ind = tmp.FirstIndexOf('_', 1);
    if( ind == InvalidIndex || ind == 0 ) // a _loop ?
      continue;
    tmp = tmp.SubStringTo(ind);
    if( tmp == sectionName )  {
      if( indexes.GetB() != (i+1) )
        lines.Move(i, indexes.GetB()+1);
      indexes.B() ++;
    }
  }
}
void TCif::Group()  {
  TCSTypeList<olxstr, AnAssociation2<size_t,size_t> > sections;
  olxstr tmp;
  for( size_t i=0; i < Lines.Count(); i++ )  {
    tmp = Lines[i].Trim(' ');
    if( tmp.IsEmpty() || tmp.StartsFrom("loop_") )  continue;
    size_t ind = tmp.FirstIndexOf('_', 1);
    if( ind == InvalidIndex || ind == 0 ) // a _loop ?
      continue;
    tmp = tmp.SubStringTo(ind);
    ind = sections.IndexOfComparable(tmp);
    if( ind == InvalidIndex )  {
      sections.Add( tmp, AnAssociation2<size_t,size_t>(i,i) );
      AnAssociation2<size_t,size_t>& indexes = sections[tmp];
      GroupSection(Lines, i+1, tmp, indexes);
    }
  }
  // sorting the groups internally ...
  for( size_t i=0; i < sections.Count(); i++ )  {
    size_t ss = sections.GetObject(i).GetA(),
        se = sections.GetObject(i).GetB();
    bool changes = true;
    while( changes )  {
      changes = false;
      for( size_t j=ss; j < se; j++ )  {
        if( Lines[j].Compare(Lines[j+1]) > 0 )  {
          Lines.Swap(j, j+1);
          changes = true;
        }
      }
    }
  }
}
//..............................................................................
void TCif::SaveToStrings(TStrList& Strings)  {
  GetAsymmUnit().ComplyToResidues();
  size_t loopc=0;
  //Lines.Sort();
  for( size_t i=0; i < Lines.Count(); i++ )  {
    olxstr Tmp = Lines[i];
    if( Lines[i].StartsFromi("loop_") )  {
      if( loopc < Loops.Count() )  {
        Loops.GetObject(loopc)->UpdateTable(*this);
        // skip empty loops, as they break the format
        if( Loops.GetObject(loopc)->GetTable().RowCount() != 0 )  {
          Strings.Add("loop_");
          Loops.GetObject(loopc)->SaveToStrings(Strings);
        }
      }
      loopc++;
      if( (i+1) < Lines.Count() && !Lines[i+1].IsEmpty() )  // add a
        Strings.Add(EmptyString);
      continue;
    }
    if( Lines.GetObject(i) != NULL )  {
      Tmp.Format(34, true, ' ');
      CifData* D = Lines.GetObject(i);
      if( D->data.Count() > 1 )  {
        Strings.Add(Tmp);
        Strings.Add(";");
        for( size_t j=0; j < D->data.Count(); j++ )
          Strings.Add(D->data[j]);
        Strings.Add(";");
      }
      else  {
        if( D->data.Count() == 1 )  {
          if( (D->data[0].Length() + 34) >= 80 )  {
            Strings.Add(Tmp);
            Strings.Add(";");
            Strings.Add(D->data[0]);
            Strings.Add(";");
          }
          else  {
            if( D->quoted )  {
              Tmp << '\'' << D->data[0] << '\'';
            }
            else
              Tmp << D->data[0];
            Strings.Add(Tmp);

          }
        }
        else  {  // empty parameter
          if( D->quoted )
            Tmp << "'?'";
          else
            Tmp << '?';
          Strings.Add(Tmp);
        }
      }
    }
    else
      Strings.Add(Tmp);
  }
}
//..............................................................................
bool TCif::ParamExists(const olxstr& Param) const {
  return (Lines.IndexOf(Param) != InvalidIndex);
}
//..............................................................................
const olxstr& TCif::GetSParam(const olxstr &Param) const {
  if( Param[0] != '_' )
    return EmptyString;
  size_t i = Lines.IndexOf(Param);
  if( i != InvalidIndex )  {
    if( Lines.GetObject(i)->data.Count() >= 1 )
      return Lines.GetObject(i)->data[0];
    return EmptyString;
  }
  return EmptyString;
}
//..............................................................................
TCif::CifData *TCif::FindParam(const olxstr &Param) const {
  if( Param[0] != '_' )  return NULL;
  size_t i = Lines.IndexOf(Param);
  return (i == InvalidIndex) ? NULL : Lines.GetObject(i);
}
//..............................................................................
bool TCif::SetParam(const olxstr& name, const CifData& value)  {
  size_t i = Lines.IndexOf(name);
  if( i == InvalidIndex )  {
    Parameters.Add(name, Lines.Add(name, new CifData(value)).Object);
    return true;
  }
  Lines.GetObject(i)->data = value.data;
  Lines.GetObject(i)->quoted = value.quoted;
  return false;
}
//..............................................................................
bool TCif::ReplaceParam(const olxstr& old_name, const olxstr& new_name, const CifData& value)  {
  size_t i = Lines.IndexOf(old_name);
  if( i == InvalidIndex )  {
    Parameters.Add(new_name, Lines.Add(new_name, new CifData(value)).Object);
    return true;
  }
  else  {
    Lines[i] = new_name;
    Lines.GetObject(i)->data = value.data;
    Lines.GetObject(i)->quoted = value.quoted;
    i = Parameters.IndexOf(old_name);
    Parameters[i] = new_name;
  }
  return false;
}
//..............................................................................
void TCif::Initialize()  {
  olxstr Param;
  TCifLoop *ALoop, *Loop;
  double Q[6], E[6]; // quadratic form of ellipsoid
  TEValueD EValue;
  try  {
    GetAsymmUnit().Axes()[0] = GetSParam("_cell_length_a");
    GetAsymmUnit().Axes()[1] = GetSParam("_cell_length_b");
    GetAsymmUnit().Axes()[2] = GetSParam("_cell_length_c");

    GetAsymmUnit().Angles()[0] = GetSParam("_cell_angle_alpha");
    GetAsymmUnit().Angles()[1] = GetSParam("_cell_angle_beta");
    GetAsymmUnit().Angles()[2] = GetSParam("_cell_angle_gamma");
  }
  catch(...) {  return;  }
  // check if the cif file contains valid parameters
  if( GetAsymmUnit().CalcCellVolume() == 0 )
    return;

  GetAsymmUnit().InitMatrices();

  Loop = FindLoop("_space_group_symop");
  if( Loop == NULL )
    Loop = FindLoop("_space_group_symop_operation_xyz");
  if( Loop != NULL  )  {
    size_t sindex = Loop->GetTable().ColIndex("_space_group_symop_operation_xyz");
    size_t iindex = Loop->GetTable().ColIndex("_space_group_symop_id");
    if( sindex != InvalidIndex )  {
      for( size_t i=0; i < Loop->GetTable().RowCount(); i++ )  {
        if( !TSymmParser::SymmToMatrix(Loop->GetTable()[i][sindex], Matrices.AddNew()) )
          throw TFunctionFailedException(__OlxSourceInfo, "could not process symmetry matrix");
        if( iindex == InvalidIndex )
          MatrixMap.Add(i+1, i);
        else
          MatrixMap.Add(Loop->GetTable()[i][iindex], i);
      }
    }
  }
  else  {
    Loop = FindLoop("_symmetry_equiv_pos");
    if( Loop == NULL )
      Loop = FindLoop("_symmetry_equiv_pos_as_xyz");
    if( Loop != NULL  )  {
      TCifLoop& symop_loop = *(new TCifLoop);
      symop_loop.GetTable().AddCol("_space_group_symop_id");
      symop_loop.GetTable().AddCol("_space_group_symop_operation_xyz");

      size_t sindex = Loop->GetTable().ColIndex("_symmetry_equiv_pos_as_xyz");
      size_t iindex = Loop->GetTable().ColIndex("_symmetry_equiv_pos_site_id");
      if( sindex != InvalidIndex )  {
        for( size_t i=0; i < Loop->GetTable().RowCount(); i++ )  {
          if( !TSymmParser::SymmToMatrix(Loop->GetTable()[i][sindex], Matrices.AddNew()) )
            throw TFunctionFailedException(__OlxSourceInfo, "could not process symmetry matrix");
          TCifRow& row = symop_loop.GetTable().AddRow(EmptyString);
          if( iindex == InvalidIndex )  {
            MatrixMap.Add(i+1, i);
            row[0] = i+1;
          }
          else  {
            MatrixMap.Add(Loop->GetTable()[i][iindex], i);
            row[0] = Loop->GetTable()[i][iindex];
          }
          row.GetObject(0) = new StringCifCell(false);
          row[1] = Loop->GetTable()[i][sindex];
          row.GetObject(1) = new StringCifCell(true);
        }
      }
      // replace obsolete loop
      size_t li = Loops.IndexOfObject(Loop);
      Loops.Delete(li);
      delete Loop;
      Loops.Insert(li, symop_loop.GetLoopName(), &symop_loop);
    }
  }
  TSpaceGroup* sg = TSymmLib::GetInstance().FindSymSpace(Matrices);
  if( sg != NULL )
    GetAsymmUnit().ChangeSpaceGroup(*sg);
  else   {
    GetAsymmUnit().ChangeSpaceGroup(*TSymmLib::GetInstance().FindGroup("P1"));
    //throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  }
  
  try  {  GetRM().SetUserFormula(olxstr::DeleteChars(GetSParam("_chemical_formula_sum"), ' '));  }
  catch(...)  {  }
  
  this->Title = FDataName.UpperCase();
  this->Title << " OLEX2: imported from CIF";

  ALoop = FindLoop("_atom_site");
  if( ALoop == NULL )  return;

  size_t ALabel =  ALoop->GetTable().ColIndex("_atom_site_label");
  size_t ACi[] = {
    ALoop->GetTable().ColIndex("_atom_site_fract_x"),
    ALoop->GetTable().ColIndex("_atom_site_fract_y"),
    ALoop->GetTable().ColIndex("_atom_site_fract_z")
  };
  size_t ACUiso =  ALoop->GetTable().ColIndex("_atom_site_U_iso_or_equiv");
  size_t ASymbol = ALoop->GetTable().ColIndex("_atom_site_type_symbol");
  size_t APart   = ALoop->GetTable().ColIndex("_atom_site_disorder_group");
  size_t SiteOccu = ALoop->GetTable().ColIndex("_atom_site_occupancy");
  size_t Degen = ALoop->GetTable().ColIndex("_atom_site_symmetry_multiplicity");
  if( (ALabel|ACi[0]|ACi[1]|ACi[2]|ASymbol) == InvalidIndex )  {
    TBasicApp::GetLog().Error("Failed to locate required fields in atoms loop");
    return;
  }
  for( size_t i=0; i < ALoop->GetTable().RowCount(); i++ )  {
    TCAtom& A = GetAsymmUnit().NewAtom();
    A.SetLabel(ALoop->GetTable()[i][ALabel], false);
    cm_Element* type = XElementLib::FindBySymbol(ALoop->GetTable()[i][ASymbol]);
    if( type == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("Undefined element: ") << ALoop->GetTable()[i][ASymbol]);
    A.SetType(*type);
    for( int j=0; j < 3; j++ )  {
      EValue = ALoop->GetTable()[i][ACi[j]];
      A.ccrd()[j] = EValue.GetV();  A.ccrdEsd()[j] = EValue.GetE();
      if( EValue.GetE() == 0 )
        GetRM().Vars.FixParam(A, catom_var_name_X+j);
    }
    if( ACUiso != InvalidIndex )    {
      EValue = ALoop->GetTable()[i][ACUiso];
      A.SetUisoEsd(EValue.GetE());
      A.SetUiso(EValue.GetV());
      if( EValue.GetE() == 0 )  GetRM().Vars.FixParam(A, catom_var_name_Uiso);
    }
    if( APart != InvalidIndex && ALoop->GetTable()[i][APart].IsNumber() )
      A.SetPart(ALoop->GetTable()[i][APart].ToInt());
    if( SiteOccu != InvalidIndex )  {
      EValue = ALoop->GetTable()[i][SiteOccu];
      A.SetOccu(EValue.GetV());
      A.SetOccuEsd(EValue.GetE());
      if( EValue.GetE() == 0 )  GetRM().Vars.FixParam(A, catom_var_name_Sof);
    }
    if( Degen != InvalidIndex )
      A.SetOccu(A.GetOccu()/ALoop->GetTable()[i][Degen].ToDouble());
    ALoop->SetData(i, ALabel, new AtomCifCell(&A));
  }
  for( size_t i=0; i < Loops.Count(); i++ )  {
    if( Loops.GetObject(i) == ALoop )  continue;
    TCifLoopTable& tab = Loops.GetObject(i)->GetTable();
    for( size_t j=0; j < tab.ColCount(); j++ )  {
      if(  tab.ColName(j).IndexOf("atom_site") != InvalidIndex &&
        tab.ColName(j).IndexOf("label") != InvalidIndex )
      {
        for( size_t k=0; k < tab.RowCount(); k++ )  {
          TCAtom* ca = GetAsymmUnit().FindCAtom(tab[k][j]);
          Loops.GetObject(i)->SetData(k, j, new AtomCifCell(ca));
        }
      }
    }
  }

  ALoop = FindLoop("_atom_site_aniso");
  if( ALoop == NULL )  return;
  ALabel =  ALoop->GetTable().ColIndex("_atom_site_aniso_label");
  size_t Ui[] = {
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_11"),
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_22"),
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_33"),
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_23"),
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_13"),
    ALoop->GetTable().ColIndex("_atom_site_aniso_U_12")
  };
  if( (ALabel|Ui[0]|Ui[1]|Ui[2]|Ui[3]|Ui[4]|Ui[5]) != InvalidIndex )  {
    for( size_t i=0; i < ALoop->GetTable().RowCount(); i++ )  {
      TCAtom* A = GetAsymmUnit().FindCAtom( ALoop->GetTable()[i][ALabel] );
      if( A == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("wrong atom in the aniso loop ") << ALabel);
      for( int j=0; j < 6; j++ )  {
        EValue = ALoop->GetTable()[i][Ui[j]];  Q[j] = EValue.GetV();  E[j] = EValue.GetE();
        if( EValue.GetE() == 0 )
          GetRM().Vars.FixParam(*A, catom_var_name_U11+j);
      }
      GetAsymmUnit().UcifToUcart(Q);
      A->AssignEllp(&GetAsymmUnit().NewEllp().Initialise(Q, E));
    }
  }
  // geometric parameters
  ALoop = FindLoop("_geom_bond");
  if( ALoop != NULL )  {
    TCifLoopTable& tab = ALoop->GetTable();
    size_t ALabel =  tab.ColIndex("_geom_bond_atom_site_label_1");
    size_t ALabel1 = tab.ColIndex("_geom_bond_atom_site_label_2");
    size_t BD =  tab.ColIndex("_geom_bond_distance");
    size_t SymmA = tab.ColIndex("_geom_bond_site_symmetry_2");
    if( (ALabel|ALabel1|BD|SymmA) != InvalidIndex )  {
      TEValueD ev;
      for( size_t i=0; i < tab.RowCount(); i++ )  {
        TCifRow& Row = tab[i];
        ACifValue* cv = NULL;
        ev = Row[BD];
        if( Row[SymmA] == '.' )  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]),
            ev);
        }
        else  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]),
            SymmCodeToMatrix(Row[SymmA]),
            ev);
        }
        DataManager.AddValue(cv);
      }
    }
  }
  ALoop = FindLoop("_geom_hbond");
  if( ALoop != NULL )  {
    TCifLoopTable& tab = ALoop->GetTable();
    size_t ALabel =  tab.ColIndex("_geom_hbond_atom_site_label_D");
    size_t ALabel1 = tab.ColIndex("_geom_hbond_atom_site_label_A");
    size_t BD =  tab.ColIndex("_geom_hbond_distance_DA");
    size_t SymmA = tab.ColIndex("_geom_hbond_site_symmetry_A");
    if( (ALabel|ALabel1|BD|SymmA) != InvalidIndex )  {
      TEValueD ev;
      for( size_t i=0; i < tab.RowCount(); i++ )  {
        TCifRow& Row = tab[i];
        ACifValue* cv = NULL;
        ev = Row[BD];
        if( Row[SymmA] == '.' )  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]),
            ev);
        }
        else  {
          cv = new CifBond(
            *GetAsymmUnit().FindCAtom(Row[ALabel]),
            *GetAsymmUnit().FindCAtom(Row[ALabel1]),
            SymmCodeToMatrix(Row[SymmA]),
            ev);
        }
        DataManager.AddValue(cv);
      }
    }
  }
  ALoop = FindLoop("_geom_angle");
  if( ALoop != NULL )  {
    TCifLoopTable& tab = ALoop->GetTable();
    const size_t ind_l =  tab.ColIndex("_geom_angle_atom_site_label_1");
    const size_t ind_m =  tab.ColIndex("_geom_angle_atom_site_label_2");
    const size_t ind_r =  tab.ColIndex("_geom_angle_atom_site_label_3");
    const size_t ind_a =  tab.ColIndex("_geom_angle");
    const size_t ind_sl = tab.ColIndex("_geom_angle_site_symmetry_1");
    const size_t ind_sr = tab.ColIndex("_geom_angle_site_symmetry_3");
    if( (ind_l|ind_m|ind_r|ind_a|ind_sl|ind_sr) != InvalidIndex )  {
      TEValueD ev;
      smatd im;
      im.I();
      for( size_t i=0; i < tab.RowCount(); i++ )  {
        TCifRow& Row = tab[i];
        ACifValue* cv = NULL;
        ev = Row[ind_a];
        if( Row[ind_sl] == '.' && Row[ind_sr] == '.' )  {
          cv = new CifAngle(
            *GetAsymmUnit().FindCAtom(Row[ind_l]),
            *GetAsymmUnit().FindCAtom(Row[ind_m]),
            *GetAsymmUnit().FindCAtom(Row[ind_r]),
            ev);
        }
        else  {
          cv = new CifAngle(
            *GetAsymmUnit().FindCAtom(Row[ind_l]),
            *GetAsymmUnit().FindCAtom(Row[ind_m]),
            *GetAsymmUnit().FindCAtom(Row[ind_r]),
            Row[ind_sl] == '.' ? im : SymmCodeToMatrix(Row[ind_sl]),
            Row[ind_sr] == '.' ? im : SymmCodeToMatrix(Row[ind_sr]),
            ev);
        }
        DataManager.AddValue(cv);
      }
    }
  }
}
//..............................................................................
TCifLoop& TCif::AddLoop(const olxstr &Name)  {
  TCifLoop *CF = FindLoop(Name);
  if( CF != NULL )  return *CF;
  Lines.Add("loop_");
  CF = new TCifLoop;
  Loops.Add(Name, CF);
  return *CF;
}
//..............................................................................
TCifLoop& TCif::GetPublicationInfoLoop()  {
  const static olxstr publ_ln( "_publ_author" ), publ_jn("_publ_requested_journal");
  TCifLoop *CF = FindLoop( publ_ln );
  if( CF != NULL )  return *CF;
  size_t index = InvalidIndex;
  for( size_t i=0; i < Lines.Count(); i++ )  {
    if( Lines[i].SubStringTo(4) == "data" )  {
      index = i;
      break;
    }
  }
  Lines.Insert(index+1, "loop_");
  // to make the automatic grouping to work ...
  if( ! ParamExists(publ_jn) )  {
    CifData* Data = new CifData(true);
    Data->data.Add('?');
    Lines.Insert(index+2, publ_jn, Data);
    Lines.Insert(index+3, EmptyString, NULL);
    Parameters.Add(publ_jn, Data);
  }
  CF = new TCifLoop;
  Loops.Insert(0, publ_ln, CF);
  CF->GetTable().AddCol("_publ_author_name");
  CF->GetTable().AddCol("_publ_author_email");
  CF->GetTable().AddCol("_publ_author_address");
  return *CF;
}
//..............................................................................
bool TCif::Adopt(TXFile& XF)  {
  Clear();
  double Q[6], E[6];  // quadratic form of s thermal ellipsoid
  GetRM().Assign(XF.GetRM(), true);
  GetAsymmUnit().SetZ((short)XF.GetLattice().GetUnitCell().MatrixCount());
  Title = "OLEX2_EXP";

  SetDataName(Title);
  SetParam("_cell_length_a", GetAsymmUnit().Axes()[0].ToString(), false);
  SetParam("_cell_length_b", GetAsymmUnit().Axes()[1].ToString(), false);
  SetParam("_cell_length_c", GetAsymmUnit().Axes()[2].ToString(), false);

  SetParam("_cell_angle_alpha", GetAsymmUnit().Angles()[0].ToString(), false);
  SetParam("_cell_angle_beta",  GetAsymmUnit().Angles()[1].ToString(), false);
  SetParam("_cell_angle_gamma", GetAsymmUnit().Angles()[2].ToString(), false);

  SetParam("_chemical_formula_sum", GetAsymmUnit().SummFormula(' ', false), true);
  SetParam("_chemical_formula_weight", olxstr(GetAsymmUnit().MolWeight()), false);

  TSpaceGroup& sg = XF.GetLastLoaderSG();
  SetParam("_cell_formula_units_Z", XF.GetAsymmUnit().GetZ(), false);
  SetParam("_symmetry_cell_setting", sg.GetBravaisLattice().GetName(), true);
  SetParam("_symmetry_space_group_name_H-M", sg.GetName(), true);
  SetParam("_symmetry_space_group_name_Hall", sg.GetHallSymbol(), true);
  {
    TCifLoop& Loop = AddLoop("_space_group_symop");
    TCifLoopTable& Table = Loop.GetTable();
    Table.AddCol("_space_group_symop_id");
    Table.AddCol("_space_group_symop_operation_xyz");
    sg.GetMatrices(Matrices, mattAll);
    for( size_t i=0; i < Matrices.Count(); i++ )  {
      TCifRow& row = Table.AddRow(EmptyString);
      row[0] = (i+1);  row.GetObject(0) = new StringCifCell(false);
      row[1] = TSymmParser::MatrixToSymm(Matrices[i]);
      row.GetObject(1) = new StringCifCell(true);
    }
  }

  TCifLoopTable& atom_table = AddLoop("_atom_site").GetTable();
  atom_table.AddCol("_atom_site_label");
  atom_table.AddCol("_atom_site_type_symbol");
  atom_table.AddCol("_atom_site_fract_x");
  atom_table.AddCol("_atom_site_fract_y");
  atom_table.AddCol("_atom_site_fract_z");
  atom_table.AddCol("_atom_site_U_iso_or_equiv");
  atom_table.AddCol("_atom_site_occupancy");
  atom_table.AddCol("_atom_site_symmetry_multiplicity");
  atom_table.AddCol("_atom_site_disorder_group");

  TCifLoopTable& u_table = AddLoop("_atom_site_aniso").GetTable();
  u_table.AddCol("_atom_site_aniso_label");
  u_table.AddCol("_atom_site_aniso_U_11");
  u_table.AddCol("_atom_site_aniso_U_22");
  u_table.AddCol("_atom_site_aniso_U_33");
  u_table.AddCol("_atom_site_aniso_U_23");
  u_table.AddCol("_atom_site_aniso_U_13");
  u_table.AddCol("_atom_site_aniso_U_12");

  for( size_t i = 0; i < GetAsymmUnit().AtomCount(); i++ )  {
    TCAtom& A = GetAsymmUnit().GetAtom(i);
    TCifRow& Row = atom_table.AddRow(EmptyString);
    Row[0] = A.GetLabel();  Row.GetObject(0) = new AtomCifCell(&A);
    Row[1] = A.GetType().symbol;  Row.GetObject(1) = new StringCifCell(false);
    for( int j=0; j < 3; j++ )
      Row.Set(j+2, TEValueD(A.ccrd()[j], A.ccrdEsd()[j]).ToString(), new StringCifCell(false));
    Row.Set(5, TEValueD(A.GetUiso(), A.GetUisoEsd()).ToString(), new StringCifCell(false));
    Row.Set(6, TEValueD(A.GetOccu()*A.GetDegeneracy(), A.GetOccuEsd()).ToString(), new StringCifCell(false));
    Row.Set(7, A.GetDegeneracy(), new StringCifCell(false));
    // process part as well
    if( A.GetPart() != 0 )
      Row[8] = (int)A.GetPart();
    else
      Row[8] = '.';
    Row.GetObject(8) = new StringCifCell(false);
    if( A.GetEllipsoid() != NULL )  {
      A.GetEllipsoid()->GetQuad(Q, E);
      GetAsymmUnit().UcartToUcif(Q);
      TCifRow& Row1 = u_table.AddRow(EmptyString);
      Row1[0] = A.GetLabel();  Row1.GetObject(0) = new AtomCifCell(&A);
      for( int j=0; j < 6; j++ )  {
        Row1.Set(j+1, TEValueD(Q[j], E[j]).ToString(), new StringCifCell(false));
      }
    }
  }
  if( XF.GetAsymmUnit().IsQPeakMinMaxInitialised() )
    SetParam("_refine_diff_density_max", XF.GetAsymmUnit().GetMaxQPeak(), false);
  return true;
}
//..............................................................................
smatd TCif::SymmCodeToMatrix(const olxstr &Code) const {
  size_t ui = Code.LastIndexOf('_');
  if( ui == InvalidIndex )
    return GetMatrixById(Code);
  smatd mSymm = GetMatrixById(Code.SubStringTo(ui));
  olxstr str_t = Code.SubStringFrom(ui+1);
  if( str_t.Length() != 3 )
    return mSymm;
  mSymm.t[0] += (int)(str_t.CharAt(0)-'5');
  mSymm.t[1] += (int)(str_t.CharAt(1)-'5');
  mSymm.t[2] += (int)(str_t.CharAt(2)-'5');
  return mSymm;
}
//..............................................................................
bool TCif::ResolveParamsFromDictionary(TStrList &Dic, olxstr &String,
 olxch Quote,
 olxstr (*ResolveExternal)(const olxstr& valueName),
 bool DoubleTheta) const
{
  size_t start, end;
  for( size_t i=0; i < String.Length(); i++ )  {
    if( String.CharAt(i) == Quote )  {
      if( (i+1) < String.Length() && String.CharAt(i+1) == Quote )  {
        String.Delete(i, 1);
        continue;
      }
      if( i > 0 && String.CharAt(i-1) == '\\' )  // escaped?
        continue;
      olxstr Val;
      if( (i+1) < String.Length() &&
          (String.CharAt(i+1) == '$' || String.CharAt(i+1) == '_' ||
          olxstr::o_isdigit(String.CharAt(i+1))) )
      {
        start = i;
        while( ++i < String.Length() )  {
          if( String.CharAt(i) == Quote )  {
            if( (i+1) < String.Length() && String.CharAt(i+1) == Quote )  {
              String.Delete(i, 1);
              Val << Quote;
              continue;
            }
            else if( String.CharAt(i-1) == '\\' ) // escaped?
              ;
            else  {
              end = i;  
              break;
            }
          }
          Val << String.CharAt(i);
        }
      }
      if( !Val.IsEmpty() )  {
        if( !Val.IsNumber() )  {
          if( Val.CharAt(0) == '$' )  {
            if( ResolveExternal != NULL )  {
              String.Delete(start, end-start+1);
              Val.Replace("\\%", '%');
              ResolveParamsFromDictionary(Dic, Val, Quote, ResolveExternal);
              olxstr Tmp = ResolveExternal(Val);
              ResolveParamsFromDictionary(Dic, Tmp, Quote, ResolveExternal);
              String.Insert(Tmp, start);
              i = start + Tmp.Length() - 1;
            }
          }
          else if( Val.CharAt(0) == '_' )  {
            CifData* Params = FindParam(Val);
            olxstr Tmp = 'N';
            if( Params != NULL && !Params->data.IsEmpty() )  
              Tmp = Params->data[0];
            String.Delete(start, end-start+1);
            String.Insert(Tmp, start);
            i = start + Tmp.Length() - 1;
          }
          else
            TBasicApp::GetLog() << olxstr("A number or function starting from '$' or '_' is expected");
          continue;
        }
        size_t index = Val.ToSizeT();
        // Not much use if not for personal use :D
        /*
        if( index >= 73 )  {  //direct insert
          // 73 - crystals handling
          // 74 - other programs
          // 75 - solved and refined by...
          // 76 - collected for
          // 77 - anisotropic atoms
          // ....
          if( (index > Dic.Count()) || (index <= 0) )
            TBasicApp::GetLog()->Info( olxstr("Wrong parameter index ") << index);
          else  {
            String.Delete(start, end-start+1);
            String.Insert(Dic.String(index-1), start);
            i = start + Dic.String(index-1).Length() - 1;
          }
          continue;
        }
        */
        if( (index > Dic.Count()) || (index <= 0) )
          TBasicApp::GetLog().Error(olxstr("Wrong parameter index ") << index);
        else  {  // resolve indexes
          String.Delete(start, end-start+1);
          olxstr SVal = Dic[index-1];
          olxstr value;
          if( !SVal.IsEmpty() )  {
            if( SVal.Equalsi("date") )
              value = TETime::FormatDateTime(TETime::Now());
            else if( SVal.Equalsi("sg_number") )  {
              TSpaceGroup* sg = TSymmLib::GetInstance().FindSG(GetAsymmUnit());
              if( sg != NULL )
                value = sg->GetNumber();
              else
                value = "unknown";
            }
            else if( SVal.Equalsi("data_name") )
              value = GetDataName();
            else if( SVal.Equalsi("weighta") )
              value = GetWeightA();
            else if( SVal.Equalsi("weightb") )
              value = GetWeightB();
            else {
              CifData* Params = FindParam(SVal);
              if( Params == NULL )  {
                TBasicApp::GetLog().Info(olxstr("The parameter \'") << SVal << "' is not found");
                value = 'N';
              }
              else if( !Params->data.Count() )  {
                TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not found");
                  value = "none";
              }
              else if( Params->data.Count() == 1 )  {
                if( Params->data[0].IsEmpty() )  {
                  TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not found");
                  value = "none";
                }
                else if( Params->data[0].CharAt(0) == '?' )  {
                  TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not defined");
                  value = '?';
                }
                else
                  value = Params->data[0];
              }
              else if( index == 13 || index == 14 || index == 30 )  {
                if( DoubleTheta )
                  value = (Params->data.Text(EmptyString).ToDouble()*2);
                else
                  value = Params->data.Text(' ');
              }
              else
                value = Params->data.Text(' ');
            }
            String.Insert(value, start);
            i = start + value.Length() - 1;
          }
        }
      }
    }
  }
  return true;
}
//..............................................................................
void TCif::MultValue(olxstr &Val, const olxstr &N)  {
  Val = (TEValue<double>(Val) *= N.ToDouble()).ToString();
}
//..............................................................................
bool TCif::CreateTable(TDataItem *TD, TTTable<TStrList> &Table, smatd_list& SymmList) const {
  int RowDeleted=0, ColDeleted=0;

  SymmList.Clear();

  TCifLoopTable* LT = NULL;
  for( size_t i=0; i < Loops.Count(); i++ )  {
    TCifLoop* Loop = Loops.GetObject(i);
    LT = &Loop->GetTable();
    if( LT->ColCount() < TD->ItemCount() )  continue;
    size_t defcnt = 0;
    for( size_t j=0; j < LT->ColCount(); j++ )  {
      if( TD->FindItemi(LT->ColName(j)) != NULL )
        defcnt ++;
    }
    if( defcnt == TD->ItemCount() )  break;
    else  LT = NULL;
  }
  if( LT == NULL )  {
    TBasicApp::GetLog().Info(olxstr("Could not find loop for table definition: ") << TD->GetName());
    return false;
  }
  Table.Assign(*LT);
  // process rows
  for( size_t i=0; i < LT->RowCount(); i++ )  {
    bool AddRow = true;
    for( size_t j=0; j < LT->ColCount(); j++ )  {
      TDataItem *DI = TD->FindItemi(LT->ColName(j));
      if( LT->ColName(j).StartsFrom("_geom_") && 
        LT->ColName(j).IndexOf("site_symmetry") != InvalidIndex)
      {
        if( (*LT)[i][j] != '.' )  {  // 1_555
          olxstr tmp = LT->ColName(j).SubStringFrom(LT->ColName(j).LastIndexOf('_')+1);
          //if( !tmp.IsNumber() ) continue;
          olxstr Tmp = "label_";
          Tmp << tmp;
          smatd SymmMatr = SymmCodeToMatrix((*LT)[i][j]);
          size_t matIndex = SymmList.IndexOf(SymmMatr);
          if( matIndex == InvalidIndex )  {
            SymmList.AddCCopy(SymmMatr);
            matIndex = SymmList.Count()-1;
          }
          for( size_t k=0; k < Table.ColCount(); k++ )  {
            if( Table.ColName(k).EndsWith(Tmp) )  {
              Table[i-RowDeleted][k] << "<sup>" << (matIndex+1) << "</sup>";
              break;
            }
          }
        }
      }
      if( DI == NULL )  continue;
      olxstr Val = (*LT)[i][j];
      olxstr Tmp = DI->GetFieldValue("mustequal", EmptyString);
      TStrList Toks(Tmp, ';');
      if( !Tmp.IsEmpty() && (Toks.IndexOfi(Val) == InvalidIndex) ) // equal to
      {  AddRow = false;  break;  }

      Tmp = DI->GetFieldValue("atypeequal", EmptyString);
      if( !Tmp.IsEmpty() )  {  // check for atom type equals to
        ICifCell* CD = (*LT)[i].GetObject(j);
        if( CD != NULL && CD->GetAtomRef() != NULL )
          if( !CD->GetAtomRef()->GetType().symbol.Equalsi(Tmp) )  {
            AddRow = false;
            break;
          }
      }
      Tmp = DI->GetFieldValue("atypenotequal", EmptyString);
      if( !Tmp.IsEmpty() )  {  // check for atom type equals to
        ICifCell* CD = (*LT)[i].GetObject(j);
        if( CD != NULL && CD->GetAtomRef() != NULL )
          if( CD->GetAtomRef()->GetType().symbol.Equalsi(Tmp) )  {
            AddRow = false;
            break;
          }
      }
      Tmp = DI->GetFieldValue("mustnotequal", EmptyString);
      Toks.Clear();
      Toks.Strtok(Tmp, ';');
      if( !Tmp.IsEmpty() && (Toks.IndexOfi(Val) != InvalidIndex) ) // not equal to
      {  AddRow = false;  break;  }

      Tmp = DI->GetFieldValue("multiplier", EmptyString);
      if( !Tmp.IsEmpty() )  {  // Multiply
        Val = Table[i-RowDeleted][j];
        MultValue(Val, Tmp);
        Table[i-RowDeleted][j] = Val;
      }
    }
    if( !AddRow )  {
      Table.DelRow(i-RowDeleted);
      RowDeleted++;
    }
  }
  // process columns
  for( size_t i=0; i < LT->ColCount(); i++ )  {
    TDataItem *DI = TD->FindItemi(LT->ColName(i));
    if( DI != NULL )  {
      Table.ColName(i-ColDeleted) = DI->GetFieldValueCI("caption");
      if( !DI->GetFieldValueCI("visible", FalseString).ToBool() )  {
        Table.DelCol(i-ColDeleted);
        ColDeleted++;
      }
    }
    else  {
      Table.DelCol(i-ColDeleted);
      ColDeleted++;
    }
  }
  return true;
}
//..............................................................................
size_t TCif::CIFToks(const olxstr& exp, TStrList& out)  {
  size_t start = 0;
  const size_t toks_c = out.Count();
  for( size_t i=0; i < exp.Length(); i++ )  {
    const olxch ch = exp.CharAt(i);
    if( is_quote(ch) && (i==0 || olxstr::o_iswhitechar(exp[i-1])) )  {
      while( ++i < exp.Length() )  {
        if( exp[i] == ch && ((i+1) >= exp.Length() || olxstr::o_iswhitechar(exp[i+1])) )  {
          break;
        }
      }
    }
    else if( olxstr::o_iswhitechar(ch) )  {
      if( start == i )  { // white chars cannot define empty args
        start = i+1;
        continue;
      }
      out.Add(exp.SubString(start, i-start).TrimWhiteChars());
      start = i+1;
    }
  }
  if( start < exp.Length() )
    out.Add(exp.SubStringFrom(start).TrimWhiteChars());
  return out.Count() - toks_c;
}


