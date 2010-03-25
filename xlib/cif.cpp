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

//----------------------------------------------------------------------------//
// TCif function bodies
//----------------------------------------------------------------------------//
TCif::TCif() : FDataNameUpperCase(true)  {  }
//..............................................................................
TCif::~TCif()  {  Clear();  }
//..............................................................................
void TCif::Clear()  {
  for( size_t i=0; i < Lines.Count(); i++ )  {
    if( Lines.GetObject(i) != NULL )  {
      delete Lines.GetObject(i)->Data;
      delete Lines.GetObject(i);
    }
  }
  Lines.Clear();
  Parameters.Clear();
  FDataName = EmptyString;
  FWeightA = EmptyString;
  FWeightB = EmptyString;
  for( size_t i=0; i < Loops.Count(); i++ )
    delete Loops.GetObject(i);
  Loops.Clear();
  GetRM().ClearAll();
  GetAsymmUnit().Clear();
  DataManager.Clear();
  Matrices.Clear();
  MatrixMap.Clear();
}
//..............................................................................
void TCif::Format()  {
  TCifData *D;
  olxstr Tmp, Tmp1;
  TStrList toks;
  TStrList *NewData;
  bool AddSpace;
  for( size_t i=0; i < Lines.Count()-1; i++ )  {
    if( Lines[i].Length() == 0  && Lines[i+1].Length() == 0 )  {
      if( Lines.GetObject(i) != NULL )
        delete Lines.GetObject(i);
    }
  }
  Lines.Pack();
  for( size_t i=0; i < Lines.Count(); i++ )  {
    if( Lines.GetObject(i) != NULL )  {
      size_t li = 0;
      D = Lines.GetObject(i);
      for( size_t j=0; j < D->Data->Count(); j++ )  {
        size_t DL = D->Data->GetString(j).Length();
        for( size_t k=0; k < DL; k++ )  {
          const olxch Char = D->Data->GetString(j).CharAt(k);
          if( Char == '\'' || Char == '"' || Char == ';')  {
            D->Data->GetString(j)[k] = ' ';
            D->Quoted = true;
          }
        }
        D->Data->GetString(j).DeleteSequencesOf(' ').Trim(' ');
        if( !D->Data->GetString(j).IsEmpty() )  
          li++;
      }
      // check if a space should be added at the beginning of line
      AddSpace = false;
      if( li > 1 )  AddSpace = true;
      else  {
        if( D->Data->Count() == 1 )  {
          if( D->Data->GetString(0).Length() > 80 )
            AddSpace = true;
        }
      }

      NewData = new TStrList;
      for( size_t j=0; j < D->Data->Count(); j++ )  {
        size_t DL = D->Data->GetString(j).Length();
        if( DL != 0 )  {
          toks.Clear();
          toks.Hyphenate(D->Data->GetString(j), 78);
          for( size_t k=0; k < toks.Count(); k++ )  {
            if( AddSpace )  Tmp = ' '; // if a space should be added at the beginning of line
            else            Tmp = EmptyString;
            Tmp << toks[k];
            NewData->Add(Tmp);
          }
        }
      }
      delete D->Data;
      D->Data = NewData;
      if( NewData->Count() > 1 )
        D->Quoted = true;
    }
  }
}
//..............................................................................
void TCif::LoadFromStrings(const TStrList& Strings)  {
  TStrList LoopData;
  Clear();
  Lines.Assign(Strings);
  for( size_t i=0; i < Lines.Count(); i++ )  {
    Lines[i].DeleteSequencesOf<char>(' ').Trim(' ');
    if( Lines[i].IsEmpty() )  continue;
    size_t spindex = Lines[i].FirstIndexOf('#');  // a comment char
    if( spindex != InvalidIndex )  {
      if( spindex != 0 )  {
        olxstr Tmp = Lines[i];
        Lines[i] = Tmp.SubStringTo(spindex-1);  // remove the '#' character
        Lines.Insert(i+1, Tmp.SubStringFrom(spindex));
        i++;
      }
    }
  }
  olxch Char;
  for( size_t i=0; i < Lines.Count(); i++ )  {
    olxstr Tmp = Lines[i];
    if( Tmp.IsEmpty() )  continue;
    if( Tmp.CharAt(0) == '#')  continue;
next_loop:
    if( Tmp.Equalsi("loop_") )  {  // parse a loop
      TCifLoop *Loop = new TCifLoop();
      LoopData.Clear();
      Loops.Add(EmptyString, Loop);
      Char = '_';
      while( Char == '_' )  {  // skip loop definition
        i++;
        if( i >= Lines.Count() )  {  // // end of file?
          Loop->Format(LoopData); // parse the loop
          Loops[Loops.Count()-1] = Loop->GetLoopName();
          goto exit;
        }
        Tmp = Lines[i];
        if( Tmp.IsEmpty() )  continue;
        else  
          Char = Tmp.CharAt(0);
        if( Char == '_' )  {
          if( Loop->GetTable().ColCount() )  {
            // check that the itm is actualli belongs to the loop
            // happens in the case of empty loops
            if( olxstr::CommonString(Tmp, Loop->GetTable().ColName(0)).Length() == 1 )  {
              goto finalize_loop;
            }
          }
          Loop->GetTable().AddCol(Tmp);
          Lines[i] = EmptyString;
        }
      }
      Char = 'a';
      while( Char != '_' )  {  // skip loop data
        Lines[i] = EmptyString;
        LoopData.Add(Tmp);
        i++;

        if( i >= Lines.Count() )  {
          Loop->Format(LoopData);
          Loops.Last().String = Loop->GetLoopName();
          goto exit;
        }
        Tmp = Lines[i];
        if( Tmp == "loop_" || Tmp.StartsFrom("data_") )
          goto finalize_loop;   // a new loop or dataset started
        if( Tmp.IsEmpty() )   continue;
        else  
          Char = Tmp.CharAt(0);
      }
finalize_loop:
      Loop->Format(LoopData);
      Loops.Last().String = Loop->GetLoopName();
      if( Tmp == "loop_" )
        goto next_loop;
    }
    if( Tmp.CharAt(0) == '_' )  {  // parameter
      bool String = false;
      olxstr Val(EmptyString), Param(EmptyString);
      size_t spindex = Tmp.FirstIndexOf(' ');
      if( spindex != InvalidIndex )  {
        Param = Tmp.SubStringTo(spindex);
        Val = Tmp.SubStringFrom(spindex+1); // to remove the space
      }
      else  {
        Param = Tmp;
        Val = EmptyString;
      }
      Lines[i] = Param;
      TCifData *D = new TCifData;
      D->Data = new TStrList;
      D->Quoted = false;
      Lines.GetObject(i) = D;
      Parameters.Add(Param, D);
      if( !Val.IsEmpty() )
        D->Data->Add(Val);
      size_t j = i + 1;
      if( j < Lines.Count() )  {  // initialise 'Char'
        if( !Lines[j].IsEmpty() )
          Char = Lines[j].CharAt(0);
        else    // will initiate the while loop below
          Char = 'a';
      }
      while( Char != '_' && (j < Lines.Count()) )  {
        olxstr Tmp1 = Lines[j];
        if( !Lines[j].IsEmpty() )  {
          Char = Lines[j].CharAt(0);
          if( Char == '#' )  {  j++;  continue;  }
          if( Lines[j].Length() > 4 )  {  // check for special data items
            olxstr tmp = Lines[j].SubString(0,4);
            if( tmp.Equalsi("data") || tmp.Equalsi("loop") )  break;
          }
          if( Char != '_' )  {
            D->Data->Add(Lines[j]);
            Lines.Delete(j);
            j--;
          }
          else
            break;
        }
        j++;
      }
      i = j-1;
      D->Quoted = String;
    }
    else  {
      if( Tmp.StartsFrom("data_") )  {
        if( FDataNameUpperCase )
          FDataName = Tmp.SubStringFrom(5).UpperCase();
        else
          FDataName = Tmp.SubStringFrom(5);
        FDataName.DeleteSequencesOf(' ');
        Lines[i] = "data_";
        Lines[i] << FDataName;
      }
    }
  }
exit:
  Format();
  /******************************************************************/
  /*search fo the weigting sceme*************************************/
  TCifData *D = FindParam( "_refine_ls_weighting_details");
  if( D != NULL)  {
    if( D->Data->Count() == 1 )  {
      const olxstr& tmp = D->Data->GetString(0);
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
  }
  /******************************************************************/
  for( size_t i=0; i < Lines.Count()-1; i++ )  {
    if( (Lines.GetString(i).Length()|Lines.GetString(i+1).Length()) == 0 )  {
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
void GroupSection(TStrPObjList<olxstr,TCifData*>& lines, size_t index,
       const olxstr& sectionName, AnAssociation2<size_t,size_t>& indexes)  {
  olxstr tmp;
  for( size_t i=index; i < lines.Count(); i++ )  {
    tmp = lines[i].Trim(' ');
    if( tmp.IsEmpty() || tmp == "loop_" )  continue;
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
    if( tmp.IsEmpty() || tmp == "loop_" )  continue;
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
    olxstr Tmp1 = olxstr::DeleteSequencesOf<char>(Tmp.ToLowerCase(), ' ');
    if( Tmp1.LowerCase() == "loop_" )  {
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
      TCifData* D = Lines.GetObject(i);
      if( D->Data->Count() > 1 )  {
        Strings.Add(Tmp);
        Strings.Add(";");
        for( size_t j=0; j < D->Data->Count(); j++ )
          Strings.Add( D->Data->GetString(j) );
        Strings.Add(";");
      }
      else  {
        if( D->Data->Count() == 1 )  {
          if( (D->Data->GetString(0).Length() + 34) >= 80 )  {
            Strings.Add(Tmp);
            Strings.Add(";");
            Strings.Add(D->Data->GetString(0));
            Strings.Add(";");
          }
          else  {
            if( D->Quoted )  {
              Tmp << '\'' << D->Data->GetString(0) << '\'';
            }
            else
              Tmp << D->Data->GetString(0);
            Strings.Add(Tmp);

          }
        }
        else  {  // empty parameter
          if( D->Quoted )
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
bool TCif::ParamExists(const olxstr &Param) {
  return (Lines.IndexOf(Param) != InvalidIndex);
}
//..............................................................................
const olxstr& TCif::GetSParam(const olxstr &Param) const {
  if( Param[0] != '_' )
    return EmptyString;
  size_t i = Lines.IndexOf(Param);
  if( i != InvalidIndex )  {
    if( Lines.GetObject(i)->Data->Count() >= 1 )
      return Lines.GetObject(i)->Data->GetString(0);
    return EmptyString;
  }
  return EmptyString;
}
//..............................................................................
TCifData *TCif::FindParam(const olxstr &Param) const {
  if( Param[0] != '_' )  return NULL;
  size_t i = Lines.IndexOf(Param);
  return (i == InvalidIndex) ? NULL : Lines.GetObject(i);
}
//..............................................................................
bool TCif::SetParam(const olxstr &Param, TCifData *Params)  {
  size_t i = Lines.IndexOf(Param);
  if( i != InvalidIndex )  {
    Lines.GetObject(i)->Data->Assign(*(Params->Data));
    Lines.GetObject(i)->Quoted = Params->Quoted;
    return true;
  }
  return false;
}
//..............................................................................
bool TCif::AddParam(const olxstr &Param, TCifData *Params)  {
  size_t i = Lines.IndexOf(Param);
  if( i != InvalidIndex )  return false;
  TCifData *Data = new TCifData;
  Data->Data = new TStrList;
  Data->Data->Assign(*(Params->Data));
  Data->Quoted = Params->Quoted;
  Lines.Add(Param, Data);
  Parameters.Add(Param, Data);
  return true;
}
//..............................................................................
bool TCif::AddParam(const olxstr &Param, const olxstr &Params, bool Quoted)  {
  size_t i = Lines.IndexOf(Param);
  if( i != InvalidIndex )  return false;
  TCifData *Data = new TCifData;
  Data->Data = new TStrList;
  Data->Data->Add(Params);
  Data->Quoted = Quoted;
  Lines.Add(Param, Data);
  Parameters.Add(Param, Data);
  return true;
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
  else 
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  
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
    TCifData *Data = new TCifData;
    Data->Data = new TStrList;
    Data->Data->Add('?');
    Data->Quoted = true;
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
  AddParam("_cell_length_a", GetAsymmUnit().Axes()[0].ToString(), false);
  AddParam("_cell_length_b", GetAsymmUnit().Axes()[1].ToString(), false);
  AddParam("_cell_length_c", GetAsymmUnit().Axes()[2].ToString(), false);

  AddParam("_cell_angle_alpha", GetAsymmUnit().Angles()[0].ToString(), false);
  AddParam("_cell_angle_beta",  GetAsymmUnit().Angles()[1].ToString(), false);
  AddParam("_cell_angle_gamma", GetAsymmUnit().Angles()[2].ToString(), false);

  AddParam("_chemical_formula_sum", GetAsymmUnit().SummFormula(' ', false), true);
  AddParam("_chemical_formula_weight", olxstr(GetAsymmUnit().MolWeight()), false);

  TSpaceGroup& sg = XF.GetLastLoaderSG();
  AddParam("_cell_formula_units_Z", XF.GetAsymmUnit().GetZ(), false);
  AddParam("_symmetry_cell_setting", sg.GetBravaisLattice().GetName(), true);
  AddParam("_symmetry_space_group_name_H-M", sg.GetName(), true);
  AddParam("_symmetry_space_group_name_Hall", sg.GetHallSymbol(), true);
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
    AddParam("_refine_diff_density_max", XF.GetAsymmUnit().GetMaxQPeak(), false);
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

  TCifData *Params;

  olxstr Tmp, Val, SVal;
  size_t index, start, end;
  double theta;

  for( size_t i=0; i < String.Length(); i++ )  {
    if( String.CharAt(i) == Quote )  {
      if( (i+1) < String.Length() && String.CharAt(i+1) == Quote )  {
        String.Delete(i, 1);
        continue;
      }
      if( (i+1) < String.Length() && 
        (String.CharAt(i+1) == '$' || String.CharAt(i+1) == '_' || 
          (String.CharAt(i+1) <= '9' && String.CharAt(i+1) >= '0')) ) {
        Val = EmptyString;
        start = i;
        while( (i+1) < String.Length() )  {
          i++;
          if( String.CharAt(i) == Quote )  {
            if( (i+1) < String.Length() && String.CharAt(i+1) == Quote )  {
              String.Delete(i, 1);
              Val << Quote;
              continue;
            }
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
              Tmp = ResolveExternal( Val );
              ResolveParamsFromDictionary(Dic, Tmp, Quote, ResolveExternal);
              String.Insert(Tmp, start);
              i = start + Tmp.Length() - 1;
            }
          }
          else if( Val.CharAt(0) == '_' )  {
            Params = FindParam(Val);
            if( Params == NULL || Params->Data->IsEmpty() )  
              Tmp = 'N';
            else
              Tmp = Params->Data->GetString(0);
            String.Delete(start, end-start+1);
            String.Insert(Tmp, start);
            i = start + Tmp.Length() - 1;
          }
          else
            TBasicApp::GetLog() << olxstr("A number or function starting from '$' or '_' is expected");
          continue;
        }
        index = Val.ToInt();
        Val = EmptyString;
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
          SVal = Dic[index-1];
          Tmp = EmptyString;
          if( SVal.Length() != 0 )  {
            if( SVal.Equalsi("date") )  {
              Tmp = TETime::FormatDateTime( TETime::Now() );
              String.Insert(Tmp, start);
            }
            else if( SVal.Equalsi("sg_number") )  {
              TSpaceGroup* sg = TSymmLib::GetInstance().FindSG(GetAsymmUnit());
              if( sg != NULL )
                Tmp = sg->GetNumber();
              else
                Tmp = "unknown";
            }
            else if( SVal.Equalsi("data_name") )
              Tmp = GetDataName();
            else if( SVal.Equalsi("weighta") )
              Tmp = GetWeightA();
            else if( SVal.Equalsi("weightb") )
              Tmp = GetWeightB();
            else {
              Params = FindParam(SVal);
              if( !Params )  {
                TBasicApp::GetLog().Info(olxstr("The parameter \'") << SVal << "' is not found");
                Tmp = "N";
              }
              else if( !Params->Data->Count() )  {
                TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not found");
                  Tmp = "none";
              }
              else if( Params->Data->Count() == 1 )  {
                if( Params->Data->GetString(0).IsEmpty() )  {
                  TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not found");
                  Tmp = "none";
                }
                else if( Params->Data->GetString(0).CharAt(0) == '?' )  {
                  TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not defined");
                  Tmp = "?";
                }
                else
                  Tmp = Params->Data->GetString(0);
              }
              else if( index == 13 || index == 14 || index == 30 )  {
                if( DoubleTheta )  {
                  theta = Params->Data->Text(EmptyString).ToDouble();
                  theta *= 2;
                  Tmp = theta;
                }
                else
                  Tmp = Params->Data->Text(' ');
              }
              else
                Tmp = Params->Data->Text(' ');
            }

            String.Insert(Tmp, start);
            i = start + Tmp.Length() - 1;
          }
        }
      }
    }
  }
  return true;
}
//..............................................................................
void TCif::MultValue(olxstr &Val, const olxstr &N)  {
  olxstr E, V;
  const size_t c = Val.Length();
  double dV, dM = N.ToDouble();
  size_t i = 0;
  while( (i < c) && (Val[i] !='(') )  {
    V << Val[i];
    i++;
  }
  i++;
  while( (i < c) && (Val[i] !=')') )  {
    E << Val[i];
    i++;
  }
  dV = V.ToDouble();
  dV *= dM;
  Val = dV;
  if( !E.IsEmpty() )
    Val << '(' << E << ')';
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


