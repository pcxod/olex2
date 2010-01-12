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

//---------------------------------------------------------------------------//
// TCifLoop function bodies
//---------------------------------------------------------------------------//
TCifLoop::TCifLoop()  {  }
//..............................................................................
TCifLoop::~TCifLoop()  {  Clear();  }
//..............................................................................
void  TCifLoop::Clear()  {
  for( size_t i=0; i < FTable.RowCount(); i++ )  {
    for( size_t j=0; j < FTable.ColCount(); j++ )
      delete FTable[i].GetObject(j);
  }
  FTable.Clear();
  FComments = EmptyString;
}
//..............................................................................
void TCifLoop::DeleteAtom(TCAtom *A)  {
  TCifLoopData *CD;
  for( size_t i=0; i < FTable.RowCount(); i++ )  {
    for( size_t j=0; j < FTable.ColCount(); j++ )  {
      CD = FTable[i].GetObject(j);
      if( CD->CA == A )  {
        for( size_t k=0; k < FTable.ColCount(); k++ )
          delete FTable[i].GetObject(k);
        FTable.DelRow(i);
        i--;
        break;
      }
    }
  }
}
//..............................................................................
void TCifLoop::Format(TStrList& Data)  {
  if( FTable.ColCount() == 0 )  return;
  olxstr Param;
  FComments = EmptyString;
  const size_t ColCount = FTable.ColCount();
  for( size_t i=0; i < Data.Count(); i++ )  {
    if( Data[i].IsEmpty() )  continue;
    if( Data[i].CharAt(0) == '#' )  {
      FComments << Data[i] << '\n';
      Data[i] = EmptyString;
    }
  }
  olxstr D = Data.Text(" \\n ").Replace('\t', ' ').DeleteSequencesOf(' ');
  TStrPObjList<olxstr,TCifLoopData*> Params;
  TCifLoopData *CData=NULL;
  const size_t DL = D.Length();
  for( size_t i=0; i < DL; i++ )  {
    Param = EmptyString;
    olxch Char = D.CharAt(i);
    if( Char == ' ' ) continue;
    if( Char == '\'' || Char == ';' || Char == '"')  {  // string param
      olxch SepChar = Char;
      Char = 'a';
      while( Char != SepChar )  {
	      Param << D.CharAt(i);
        i++;
        if( i >= DL )  {
          Param.Delete(0, 1);
          Params.Add(Param, new TCifLoopData(true));
          goto end_cyc;
        }
        Char = D.CharAt(i);
      }
      Param.Delete(0, 1);
      Params.Add(Param, new TCifLoopData(true));
      continue;
    }
    // normal parameter
    while( (Char != ' ') )  {
      Param << D.CharAt(i);
      i++;
      if( i >= DL )  {
        if( !Param.IsEmpty() && Param != "\\n" )
          Params.Add(Param, new TCifLoopData(false));
        goto end_cyc;
      }
      Char = D.CharAt(i);
    }
    if( !Param.IsEmpty() && Param != "\\n" )
      Params.Add(Param, new TCifLoopData(false));
end_cyc:;
  }
  if( (Params.Count() % ColCount) != 0 )  {
    // clean up the memory
    for( size_t i=0; i < Params.Count(); i++ )
      if( Params.GetObject(i) != NULL )
        delete Params.GetObject(i);
    olxstr msg("wrong loop parameters number. ");
    msg << "Failed in loop: " << GetLoopName() << ". Failed on: \'" << Param << '\'';
    throw TFunctionFailedException(__OlxSourceInfo, msg );
  }
  const size_t RowCount = Params.Count()/ColCount;
  FTable.SetRowCount(RowCount);
  for( size_t i=0; i < RowCount; i++ )  {
    for( size_t j=0; j < ColCount; j++ )  {
      FTable[i].GetObject(j) = Params.GetObject(i*ColCount+j);
      FTable[i][j] = Params[i*ColCount+j];
    }
  }
}
//..............................................................................
void TCifLoop::SaveToStrings(TStrList& Strings) const {
  TStrList toks, htoks;
  for( size_t i=0; i < FTable.ColCount(); i++ )  // loop header
    Strings.Add("  ") << FTable.ColName(i);

  for( size_t i=0; i < FTable.RowCount(); i++ ) {  // loop content
    olxstr Tmp;
    for( size_t j=0; j < FTable.ColCount(); j++ )  {
      TCifLoopData* CLD = FTable[i].GetObject(j);
      if( CLD->CA != NULL )  {
        if( CLD->CA->IsDeleted() )  {  // skip deleted atom
          Tmp = EmptyString;
          break;
        }
      }
      // according to the cif rules, the strings cannot be hyphenated ...
      olxstr str = FTable[i][j];
      if( str.EndsWith("\\n") )
        str.SetLength(str.Length()-2);
      if( CLD->String )  {
        if( str.IndexOf("\\n") != InvalidIndex )  {
          if( !Tmp.IsEmpty() )  {
            Strings.Add(Tmp);
            Tmp = EmptyString;
          }
          toks.Clear();
          toks.Strtok(str, "\\n");
          Strings.Add(';');
          for( size_t j=0; j < toks.Count(); j++ )  {
            htoks.Clear();
            htoks.Hyphenate(toks[j], 78);
            for( size_t k=0; k < htoks.Count(); k++ )  {
              if( htoks[k].Length() > 1 )  // not just a space char
                Strings.Add(htoks[k]);
            }
          }
          Strings.Add(';');
        }
        else  {
          if( Tmp.Length() + 3 + str.Length() > 80 )  {
            Strings.Add(Tmp);
            Tmp = EmptyString;
          }
          Tmp << ' ' << '\'' << str << '\'';
        }
      }
      else  {
        if( Tmp.Length() + str.Length() > 80 )  {  // \\n at the end
          Strings.Add( Tmp );
          Tmp = EmptyString;
        }
        Tmp << ' ' << str;
      }
    }
    if( Tmp.Length() > 80 )  {
      toks.Clear();
      if( Tmp.StartsFrom(" '") )  // remove quotations ...
        Tmp = Tmp.SubString(2, Tmp.Length()-3);
      toks.Strtok(Tmp, "\\n");
      Strings.Add(';');
      for( size_t j=0; j < toks.Count(); j++ )  {
        htoks.Clear();
        htoks.Hyphenate(toks[j], 78);
        for( size_t k=0; k < htoks.Count(); k++ )  {
          if( htoks[k].Length() > 1 )  // not just a space char
            Strings.Add(htoks[k]);
        }
      }
      Strings.Add(';');
    }
    else  {
      if( !Tmp.IsEmpty() )
        Strings.Add(Tmp);
    }
  }
  if( !FComments.IsEmpty() )
    Strings.Add(FComments);
}
//..............................................................................
olxstr TCifLoop::GetLoopName() const {
  if( FTable.ColCount() == 0 )  return EmptyString;
  if( FTable.ColCount() == 1 )  return FTable.ColName(0);
  
  olxstr C = olxstr::CommonString(FTable.ColName(0), FTable.ColName(1));
  for( size_t i=2; i < FTable.ColCount(); i++ )
    C = olxstr::CommonString(FTable.ColName(i), C);

  if( C.Last() == '_' )
    C.SetLength(C.Length()-1);
  return C;
}
//..............................................................................
olxstr TCifLoop::ShortColumnName(size_t in) const {
  return FTable.ColName(in).SubStringFrom(GetLoopName().Length());
}
//..............................................................................
int TCifLoop::CifLoopSorter::Compare(const TCifLoopTable::TableSort& r1,
                                     const TCifLoopTable::TableSort& r2)
{
  uint64_t id1 = 0, id2 = 0;
  int ac = 0;
  for( size_t i=r1.data.Count(); i > 0; i-- )  {
    bool atom = false;
    if( r1.data.GetObject(i-1)->CA != 0 )  {
      id1 |= ((uint64_t)(r1.data.GetObject(i-1)->CA->GetId()) << ac*16);
      atom = true;
    }
    if( r2.data.GetObject(i-1)->CA != 0 )  {
      id2 |= ((uint64_t)(r2.data.GetObject(i-1)->CA->GetId()) << ac*16);
      atom = true;
    }
    if( atom )  ac++;
  }
  return (id1 < id2 ? -1 : (id1 > id2 ? 1 : 0)); 
}
void TCifLoop::UpdateTable()  {
  for( size_t i=0; i < FTable.RowCount(); i++ )  {
    for( size_t j=0; j < FTable.ColCount(); j++ )  {
      if( FTable[i].GetObject(j)->CA != NULL )
        FTable[i][j] = FTable[i].GetObject(j)->CA->GetLabel();                                      
    }
  }
  FTable.SortRows<CifLoopSorter>();
  size_t pi = FTable.ColIndex("_atom_site_disorder_group");
  if( pi != InvalidIndex )  {
    for( size_t i=0; i < FTable.RowCount(); i++ )
      if( FTable[i].GetObject(0)->CA != NULL && FTable[i].GetObject(0)->CA->GetPart() != 0 )
        FTable[i][pi] = (int)FTable[i].GetObject(0)->CA->GetPart();
  }
}
//----------------------------------------------------------------------------//
// TCifValue function bodies
//----------------------------------------------------------------------------//
olxstr TCifValue::FormatTranslation(const vec3d& v)  {
  olxstr retVal = (5.0-v[0]);
  retVal << (5.0-v[1]);
  retVal << (5.0-v[2]);
  return retVal;
}
//..............................................................................
olxstr TCifValue::Format() const  {
  switch( Atoms.Count() )  {
    case 2:
    case 3:
    case 4:
       break;
  }
  return "nnnn";
}
//----------------------------------------------------------------------------//
// TCifDataManager function bodies
//----------------------------------------------------------------------------//
TCifValue* TCifDataManager::Match( const TSAtomPList& Atoms )  {
  for( size_t i=0; i < Items.Count(); i++ )  {
    if( Items[i].Count() !=  Atoms.Count() )  continue;
    for( size_t j=0; j < Atoms.Count(); j++ )  {
      bool found = false;
      for( size_t k=0; k < Atoms.Count(); k++ )  {
        if( Atoms[k]->CAtom().GetId() == Items[i].GetAtom(k).GetId() )  {
          found = true;
          break;
        }
      }
      if( !found )  return NULL;
      return &Items[i];
    }
  }
  return NULL;
}
//..............................................................................
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
            D->String = true;
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
      if( NewData->Count() > 1 )  D->String = true;
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
      D->String = false;
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
      D->String = String;
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
        Loops.GetObject(loopc)->UpdateTable();
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
            if( D->String )  {
              Tmp << '\'' << D->Data->GetString(0) << '\'';
            }
            else
              Tmp << D->Data->GetString(0);
            Strings.Add(Tmp);

          }
        }
        else  {  // empty parameter
          if( D->String )
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
TCifData *TCif::FindParam(const olxstr &Param)  {
  if( Param[0] != '_' )
    return NULL;
  size_t i = Lines.IndexOf(Param);
  return (i == InvalidIndex) ? NULL : Lines.GetObject(i);
}
//..............................................................................
bool TCif::SetParam(const olxstr &Param, TCifData *Params)  {
  size_t i = Lines.IndexOf(Param);
  if( i != InvalidIndex )  {
    Lines.GetObject(i)->Data->Assign(*(Params->Data));
    Lines.GetObject(i)->String = Params->String;
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
  Data->String = Params->String;
  Lines.Add(Param, Data);
  Parameters.Add(Param, Data);
  return true;
}
//..............................................................................
bool TCif::AddParam(const olxstr &Param, const olxstr &Params, bool String)  {
  size_t i = Lines.IndexOf(Param);
  if( i != InvalidIndex )  return false;
  TCifData *Data = new TCifData;
  Data->Data = new TStrList;
  Data->Data->Add(Params);
  Data->String = String;
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
  double fv =  GetAsymmUnit().Axes()[0].GetV() * GetAsymmUnit().Axes()[1].GetV() * GetAsymmUnit().Axes()[2].GetV() *
        GetAsymmUnit().Angles()[0].GetV() * GetAsymmUnit().Angles()[1].GetV() * GetAsymmUnit().Angles()[2].GetV();
  if( fv == 0 )  {    return;  }

  GetAsymmUnit().InitMatrices();

  Loop = FindLoop("_symmetry_equiv_pos");
  if( Loop == NULL )  Loop = FindLoop("_symmetry_equiv_pos_as_xyz");
  if( Loop!= NULL  )  {
    size_t sindex = Loop->GetTable().ColIndex("_symmetry_equiv_pos_as_xyz");
    if( sindex != InvalidIndex )  {
      for( size_t i=0; i < Loop->GetTable().RowCount(); i++ )  {
        smatd Matrix;
        if( TSymmParser::SymmToMatrix(Loop->GetTable()[i][sindex], Matrix) )  {
          GetAsymmUnit().AddMatrix(Matrix);
        }
        else
          throw TFunctionFailedException(__OlxSourceInfo, "could not process symmetry matrix");
      }
    }
    TSpaceGroup* sg = TSymmLib::GetInstance().FindExpandedSG(GetAsymmUnit());
    if( sg != NULL )
      GetAsymmUnit().ChangeSpaceGroup(*sg);
    else 
      throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  }
  try  {  GetRM().SetUserFormula(olxstr::DeleteChars(GetSParam("_chemical_formula_sum"), ' '));  }
  catch(...)  {  }
  
  this->Title = FDataName.UpperCase();
  this->Title << " OLEX2: imported from CIF";

  ALoop = FindLoop("_atom_site");
  if( ALoop == NULL )  return;

  size_t ALabel =  ALoop->GetTable().ColIndex("_atom_site_label");
  size_t ACx =     ALoop->GetTable().ColIndex("_atom_site_fract_x");
  size_t ACy =     ALoop->GetTable().ColIndex("_atom_site_fract_y");
  size_t ACz =     ALoop->GetTable().ColIndex("_atom_site_fract_z");
  size_t ACUiso =  ALoop->GetTable().ColIndex("_atom_site_U_iso_or_equiv");
  size_t ASymbol = ALoop->GetTable().ColIndex("_atom_site_type_symbol");
  size_t APart   = ALoop->GetTable().ColIndex("_atom_site_disorder_group");
  if( (ALabel|ACx|ACy|ACz|ASymbol) == InvalidIndex )  {
    TBasicApp::GetLog().Error("Failed to locate required fields in atoms loop");
    return;
  }
  for( size_t i=0; i < ALoop->GetTable().RowCount(); i++ )  {
    TCAtom& A = GetAsymmUnit().NewAtom();
    A.SetLabel(ALoop->GetTable()[i][ALabel], false);
    A.SetType(*XElementLib::FindBySymbol(ALoop->GetTable()[i][ASymbol]));
    EValue = ALoop->GetTable()[i][ACx];
    A.ccrd()[0] = EValue.GetV();  A.ccrdEsd()[0] = EValue.GetE();
    EValue = ALoop->GetTable()[i][ACy];
    A.ccrd()[1] = EValue.GetV();  A.ccrdEsd()[1] = EValue.GetE();
    EValue = ALoop->GetTable()[i][ACz];
    A.ccrd()[2] = EValue.GetV();  A.ccrdEsd()[2] = EValue.GetE();
    if( ACUiso != InvalidIndex )    {
      EValue = ALoop->GetTable()[i][ACUiso];
      A.SetUisoEsd(EValue.GetE());
      A.SetUiso(EValue.GetV());
    }
    if( APart != InvalidIndex && ALoop->GetTable()[i][APart].IsNumber() )
      A.SetPart(ALoop->GetTable()[i][APart].ToInt());

//    if( !A->Info ) ;
    ALoop->GetTable()[i].GetObject(ALabel)->CA = &A;
  }
  for( size_t i=0; i < Loops.Count(); i++ )  {
    if( Loops.GetObject(i) == ALoop )  continue;
    for( size_t j=0; j < Loops.GetObject(i)->GetTable().ColCount(); j++ )  {
      size_t ALabel1 = Loops.GetObject(i)->GetTable().ColName(j).IndexOf("atom_site" );
      size_t ALabel2 = Loops.GetObject(i)->GetTable().ColName(j).IndexOf("label" );
      if(  ALabel1 != InvalidIndex && ALabel2 != InvalidIndex )  {
        for( size_t k=0; k < Loops.GetObject(i)->GetTable().RowCount(); k++ )  {
          ALabel1 = ALabel;
          Loops.GetObject(i)->GetTable()[k].GetObject(j)->CA =
               GetAsymmUnit().FindCAtom( Loops.GetObject(i)->GetTable()[k][j] );
        }
      }
    }
  }

  ALoop = FindLoop("_atom_site_aniso");
  if( ALoop == NULL )  return;
  ALabel =  ALoop->GetTable().ColIndex("_atom_site_aniso_label");
  size_t U11 =     ALoop->GetTable().ColIndex("_atom_site_aniso_U_11");
  size_t U22 =     ALoop->GetTable().ColIndex("_atom_site_aniso_U_22");
  size_t U33 =     ALoop->GetTable().ColIndex("_atom_site_aniso_U_33");
  size_t U23 =     ALoop->GetTable().ColIndex("_atom_site_aniso_U_23");
  size_t U13 =     ALoop->GetTable().ColIndex("_atom_site_aniso_U_13");
  size_t U12 =     ALoop->GetTable().ColIndex("_atom_site_aniso_U_12");
  if( (ALabel|U11|U22|U33|U23|U13|U12) == InvalidIndex )  return;
  for( size_t i=0; i < ALoop->GetTable().RowCount(); i++ )  {
    TCAtom* A = GetAsymmUnit().FindCAtom( ALoop->GetTable()[i][ALabel] );
    if( A == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("wrong atom in the aniso loop ") << ALabel);
    EValue = ALoop->GetTable()[i][U11];  Q[0] = EValue.GetV();  E[0] = EValue.GetE();
    EValue = ALoop->GetTable()[i][U22];  Q[1] = EValue.GetV();  E[1] = EValue.GetE();
    EValue = ALoop->GetTable()[i][U33];  Q[2] = EValue.GetV();  E[2] = EValue.GetE();
    EValue = ALoop->GetTable()[i][U23];  Q[3] = EValue.GetV();  E[3] = EValue.GetE();
    EValue = ALoop->GetTable()[i][U13];  Q[4] = EValue.GetV();  E[4] = EValue.GetE();
    EValue = ALoop->GetTable()[i][U12];  Q[5] = EValue.GetV();  E[5] = EValue.GetE();
    GetAsymmUnit().UcifToUcart(Q);
    A->AssignEllp(&GetAsymmUnit().NewEllp().Initialise(Q, E));
  }
  // bond lengths
  /*
  ALoop = this->Loop("_geom_bond");
  if( !ALoop )  return;
  ATable = ALoop->Table();
  ALabel =  ATable->ColIndex("_geom_bond_atom_site_label_1");
  ALabel1 =  ATable->ColIndex("_geom_bond_atom_site_label_2");
  int BD =     ATable->ColIndex("_geom_bond_distance");
  int SymmA = ATable->ColIndex("_geom_bond_site_symmetry_2");
  if( ALabel < 0 || ALabel1 < 0 || BD < 0 || SymmA )  return;

  for( i=0; i < ATable->RowCount(); i++ )
  {
    Row = ATable->Row(i);
    TCifValue* cv = DataManager.NewValue();

    A = GetAsymmUnit().Atom( Row.String(ALabel) );
    if( A ) cv->AddAtom()
    A->LoaderId(GetAsymmUnit().AtomCount()-1);
    A->Label( Row->String(ALabel));
    A->ccrd().Value(0) = Row->String(ACx);
    A->ccrd().Value(1) = Row->String(ACy);
    A->ccrd().Value(2) = Row->String(ACz);
    if( ACUiso >= 0 )
    {
      EValue = Row->String(ACUiso);
      A->EllpsE().Resize(1);
      A->Uiso( EValue.V );  A->EllpsE()[0] = EValue.E;
    }

//    if( !A->Info ) ;
    LoopData = (TCifLoopData*)Row->Object(ALabel);
    LoopData->CA = A;
  }
  */
}
//..............................................................................
TCifLoop& TCif::Loop(size_t i) {
  return *Loops.GetObject(i);
};
//..............................................................................
TCifLoop *TCif::FindLoop(const olxstr &L)  {
  size_t i = Loops.IndexOf(L);
  return (i == InvalidIndex) ? NULL : Loops.GetObject(i);
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
    Data->String = true;
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
  olxstr Param;
  TCifLoop *Loop;
  TCifLoopTable *Table, *ATable;
  TCAtom *A;
  TEValueD EValue;
  double Q[6], E[6];  // quadratic form of s thermal ellipsoid
  bool AddUTable=false;

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

  AddParam("_chemical_formula_sum", GetAsymmUnit().SummFormula(' '), true);
  Param = GetAsymmUnit().MolWeight();
  AddParam("_chemical_formula_weight", Param, false);

  TSpaceGroup& sg = XF.GetLastLoaderSG();
  AddParam("_cell_formula_units_Z", XF.GetAsymmUnit().GetZ(), true);
  AddParam("_symmetry_cell_setting", sg.GetBravaisLattice().GetName(), true);
  AddParam("_symmetry_space_group_name_H-M", sg.GetName(), true);
  AddParam("_symmetry_space_group_name_Hall", sg.GetHallSymbol(), true);

  Loop = &AddLoop("_symmetry_equiv_pos_as_xyz");
  Table = &Loop->GetTable();
  Table->AddCol("_symmetry_equiv_pos_as_xyz");

  smatd_list matrices;
  sg.GetMatrices(matrices, mattAll);

  for( size_t i=0; i < matrices.Count(); i++ )  {
    TStrPObjList<olxstr,TCifLoopData*>& Row = Table->AddRow(EmptyString);
    Row[0] = TSymmParser::MatrixToSymm( matrices[i] );
    Row.GetObject(0) = new TCifLoopData(true);
  }

  Loop = &AddLoop("_atom_site");
  Table = &Loop->GetTable();
  Table->AddCol("_atom_site_label");
  Table->AddCol("_atom_site_type_symbol");
  Table->AddCol("_atom_site_fract_x");
  Table->AddCol("_atom_site_fract_y");
  Table->AddCol("_atom_site_fract_z");
  Table->AddCol("_atom_site_U_iso_or_equiv");
  Table->AddCol("_atom_site_disorder_group");

  for( size_t i = 0; i < GetAsymmUnit().AtomCount(); i++ )  {
    A = &GetAsymmUnit().GetAtom(i);
    if( A->GetEllipsoid() != NULL )  {
      AddUTable = true;  
      break;
    }
  }
  if( AddUTable )  {
    Loop = &AddLoop("_atom_site_aniso");
    ATable = &Loop->GetTable();
    ATable->AddCol("_atom_site_aniso_label");
    ATable->AddCol("_atom_site_aniso_U_11");
    ATable->AddCol("_atom_site_aniso_U_22");
    ATable->AddCol("_atom_site_aniso_U_33");
    ATable->AddCol("_atom_site_aniso_U_23");
    ATable->AddCol("_atom_site_aniso_U_13");
    ATable->AddCol("_atom_site_aniso_U_12");
  }

  for( size_t i = 0; i < GetAsymmUnit().AtomCount(); i++ )  {
    A = &GetAsymmUnit().GetAtom(i);
    TStrPObjList<olxstr,TCifLoopData*>& Row = Table->AddRow(EmptyString);
    Row[0] = A->GetLabel();  Row.GetObject(0) = new TCifLoopData(A);
    Row[1] = A->GetType().symbol;  Row.GetObject(1) = new TCifLoopData;
    for( int j=0; j < 3; j++ )  {
      EValue.V() = A->ccrd()[j];  EValue.E() = A->ccrdEsd()[j];
      Row[j+2] = EValue.ToCStr();
      Row.GetObject(j+2) = new TCifLoopData;
    }
    EValue.V() = A->GetUiso();  EValue.E() = A->GetUisoEsd();
    Row[5] = EValue.ToString();
    Row.GetObject(5) = new TCifLoopData;
    // process part as well
    if( A->GetPart() != 0 )
      Row[6] = (int)A->GetPart();
    else
      Row[6] = '.';
    Row.GetObject(6) = new TCifLoopData;

    if( A->GetEllipsoid() != NULL )  {
      A->GetEllipsoid()->GetQuad(Q, E);
      GetAsymmUnit().UcartToUcif(Q);

      TStrPObjList<olxstr,TCifLoopData*>& Row1 = ATable->AddRow(EmptyString);
      Row1[0] = A->GetLabel();  Row1.GetObject(0) = new TCifLoopData(A);
      for( int j=0; j < 6; j++ )  {
        EValue.V() = Q[j];  EValue.E() = E[j];
        Row1[j+1] = EValue.ToCStr();
        Row1.GetObject(j+1) = new TCifLoopData;
      }
    }
  }
  if( XF.GetAsymmUnit().IsQPeakMinMaxInitialised() )
    AddParam("_refine_diff_density_max", XF.GetAsymmUnit().GetMaxQPeak(), false);
  return true;
}
//----------------------------------------------------------------------------//
// TLLTBondSort function bodies - bond sorting procedure in TLinkedLoopTable
//----------------------------------------------------------------------------//
int TLLTBondSort::Compare(const TLBond *I, const TLBond *I1)  {
  double v;
  if( SortType & slltLength )  {  // length, Mr, Label
    v = I->Value.ToDouble() - I1->Value.ToDouble();
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    v = I->Another(Atom).CA.GetType().GetMr() - I1->Another(Atom).CA.GetType().GetMr();
    if( v > 0 ) return 1;
    if( v < 0 ) return -1;
    v = I->Another(Atom).Label.Compare(I1->Another(Atom).Label);
    if( v == 0 )
      return olx_cmp_size_t(Symmetry.IndexOf(I->S2), Symmetry.IndexOf(I1->S2));
  }
  if( SortType & slltName )  {  // Name, length
    v = I->Another(Atom).Label.Compare(I1->Another(Atom).Label);
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    if( v == 0 )  {
      v = (int)(Symmetry.IndexOf(I->S2) - Symmetry.IndexOf(I1->S2));
      if( v < 0 ) return -1;
      if( v > 0 ) return 1;
    }
    v = I->Value.ToDouble() - I1->Value.ToDouble();
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    return 0;
  }
  if( SortType & slltMw )  {  // Mr, Length, Label
    v = I->Another(Atom).CA.GetType().GetMr() - I1->Another(Atom).CA.GetType().GetMr();
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    v = I->Value.ToDouble() - I1->Value.ToDouble();
    if( v > 0 ) return 1;
    if( v < 0 ) return -1;
    if( v == 0 )
      return olx_cmp_size_t(Symmetry.IndexOf(I->S2), Symmetry.IndexOf(I1->S2));
  }
  return 0;
}
//----------------------------------------------------------------------------//
// TLBond function bodies - bond objsect for TLinkedLoopTable
//----------------------------------------------------------------------------//
const TLAtom& TLBond::Another(TLAtom& A) const {
  if(&A == &A1)  return A2; 
  if(&A == &A2)  return A1; 
  throw TInvalidArgumentException(__OlxSourceInfo, "atom");
}
//..............................................................................
bool TLBond::operator == (const TLBond &B) const {
  if( A1 == B.A1 && A2 == B.A2 && S2 == B.S2 )
    return true;
  return false;
}
//----------------------------------------------------------------------------//
// TLAngle function bodies - angle objsect for TLinkedLoopTable
//----------------------------------------------------------------------------//
bool TLAngle::Contains(const TLAtom& A) const {
  if( A1==A || A2==A || A3 == A ) return true; 
  return false;
}
//..............................................................................
bool TLAngle::FormedBy(const TLBond& B, const TLBond& B1) const {
  TLAtom *a1=NULL, *a3=NULL;
  olxstr s1, s3;
  if( B.A1 == A2 )  {  a1 = &B.A2;  s1 = B.S2;  }
  if( B.A2 == A2 )  {  a1 = &B.A1;  s1 = ".";  }
  if( B1.A1 == A2 )  {  a3 = &B1.A2;  s3 = B1.S2;  }
  if( B1.A2 == A2 )  {  a3 = &B1.A1;  s3 = ".";  }
  if( a1 == &A1 && a3 == &A3 )  {
//    if( LA.S1 == S1 && LA.S3 == S3 )
      return true;
  }
  if( a1 == &A3 && a3 == &A1 )  {
//    if( LA.S1 == S3 && LA.S3 == S1 )
      return true;
  }
  return false;
}
//----------------------------------------------------------------------------//
// TLinkedLoopTable function bodies
//----------------------------------------------------------------------------//
TLinkedLoopTable::TLinkedLoopTable(TCif *C)  {
  FCif = C;
  for( size_t j=0; j < C->GetAsymmUnit().AtomCount(); j++ )  {
    TCAtom &CA = C->GetAsymmUnit().GetAtom(j);
    TLAtom* LA = new TLAtom(CA);
    FAtoms.Add(LA);
    LA->Label = CA.GetLabel();
  }
  TCifLoop *CL = C->FindLoop("_geom_bond");
  if( CL == NULL ) return;
  TCifLoopTable *Table = &CL->GetTable();
  size_t index =  Table->ColIndex("_geom_bond_atom_site_label_1");
  size_t index1 = Table->ColIndex("_geom_bond_atom_site_label_2");
  size_t index2 = Table->ColIndex("_geom_bond_distance");
  size_t index3 = Table->ColIndex("_geom_bond_site_symmetry_2");
  if( (index|index1|index2|index3) == InvalidIndex )  return;  // will not work then ...
  for( size_t j=0; j < Table->RowCount(); j++ )  {
    TStrPObjList<olxstr,TCifLoopData*>& L = (*Table)[j];
    TLBond *LB = new TLBond(AtomByName(L[index]), AtomByName(L[index1]));
    LB->Value = L[index2];
    LB->S2 = L[index3];
    bool uniq = true;
    for( size_t k=0; k < FBonds.Count(); k++ )  {
      TLBond *LB1 = FBonds[k];
      if( ((LB->A1 == LB1->A2) && (LB->A2 == LB1->A1)) )  {  // only then atoms are inverted !!
        if( LB->Value == LB1->Value )  {
          uniq = false;
          break;
        }
      }
    }
    uniq = true;
    if( uniq )  {
      FBonds.Add(LB);
      LB->A1.Bonds.Add(LB);
      if( LB->S2 == "." )
        LB->A2.Bonds.Add(LB);
    }
    else
      delete LB;
  }

  CL = C->FindLoop("_geom_angle");
  if( CL == NULL )  return;
  Table = &CL->GetTable();
  index =  Table->ColIndex("_geom_angle_atom_site_label_1");
  index1 = Table->ColIndex("_geom_angle_atom_site_label_2");
  index2 = Table->ColIndex("_geom_angle_atom_site_label_3");
  index3 = Table->ColIndex("_geom_angle");
  size_t index4 = Table->ColIndex("_geom_angle_site_symmetry_1");
  size_t index5 = Table->ColIndex("_geom_angle_site_symmetry_3");
  if( (index|index1|index2|index3|index4|index5) == InvalidIndex )  
    return;  // will not work then ...
  for( size_t j=0; j < Table->RowCount(); j++ )  {
    TStrPObjList<olxstr,TCifLoopData*>& L = (*Table)[j];
    TLAngle* LAn = new TLAngle(AtomByName(L[index]), AtomByName(L[index1]), AtomByName(L[index2]));
    FAngles.Add(LAn);
    LAn->Value = L[index3];
    LAn->S1 = L[index4];
    LAn->S3 = L[index5];
  }
}
//..............................................................................
TLinkedLoopTable::~TLinkedLoopTable()  {
  for( size_t i=0; i < FAtoms.Count(); i++ )
    delete FAtoms[i];
  for( size_t i=0; i < FBonds.Count(); i++ )
    delete FBonds[i];
  for( size_t i=0; i < FAngles.Count(); i++ )
    delete FAngles[i];
}
//..............................................................................
TLAtom& TLinkedLoopTable::AtomByName(const olxstr &Name)  {
  for( size_t i=0; i < FAtoms.Count(); i++ )  {
    if( FAtoms[i]->Label == Name )
      return *FAtoms[i];
  }
  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("atom_name=") << Name);
}
//..............................................................................
TTTable<TStrList>* TLinkedLoopTable::MakeTable(const olxstr &Atom)  {
  TLAtom& A = AtomByName(Atom);
  if( A.Bonds.IsEmpty() )
    return NULL;
  size_t bc = A.Bonds.Count();
  TStrList Symm;
  Symm.Add("."); // to give proper numbering of symm operations
  // search for symm operations
  for( size_t i=0; i < bc; i++ )  {
    TLBond* LB = A.Bonds[i];
    size_t sind = Symm.IndexOf(LB->S2);
    if( sind == InvalidIndex )
      Symm.Add(LB->S2);
  }
  // sort bonds according to the requirements
  TLLTBondSort BondSort(A, Symm, slltLength);
  A.Bonds.QuickSorter.SortMF(A.Bonds, BondSort, &TLLTBondSort::Compare);
  olxstr Tmp;
  Table.Resize(bc+1, bc);
  Table.EmptyContent(true);
  Table.ColName(0) = A.Label;
  for( size_t i=0; i < bc-1; i++ )  {
    TLBond* LB = A.Bonds[i];
    const TLAtom& AAtom = LB->Another(A);
    size_t sind = Symm.IndexOf(LB->S2);
    if( sind != 0 )
      Tmp << "<sup>" << sind << "</sup>";
    Table.ColName(i+1) = AAtom.Label;
  }
  for( size_t i=0; i < bc; i++ )  {
    TLBond* LB = A.Bonds[i];
    const TLAtom& AAtom = LB->Another(A);
    size_t sind = Symm.IndexOf(LB->S2);
    if( sind != 0 )
      Tmp << "<sup>" << sind << "</sup>";

    Table.RowName(i) = AAtom.Label;
    Table[i][0] = LB->Value;
    for( size_t j=0; j < bc-1; j++ )  {
      TLBond* LB1 = A.Bonds[j];
      if( i==j )  {
        Table[i][j+1] = "-";
        continue;
      }
      if( i < j )  {
        Table[i][j+1] = EmptyString;
        continue;
      }
      bool found = false;
      for( size_t k=0; k < A.Angles.Count(); k++ )  {
        TLAngle* LA = A.Angles[k];
        if( LA->FormedBy(*LB, *LB1) )  {
          found = true;
          Table[i][j+1] = LA->Value;
          break;
        }
      }
      if( !found )
        Table[i][j+1] = '?';
    }
  }
  Tmp = EmptyString;
  for( size_t i=0; i < Symm.Count(); i++ )  {
    if( Symm[i].Length() > 0 &&  (Symm[i] != ".") )  {
      olxstr Tmp1 = SymmCodeToSymm(FCif, Symm[i]);
      if( !Tmp1.IsEmpty() )  {
        Tmp << "<sup>" << i << "</sup>" << ": " << Tmp1;
        if( i < (Symm.Count()-1) )
          Tmp << "<br>";
      }
    }
  }
  Table[A.Bonds.Count()][0] = Tmp;
  return &Table;
}
//..............................................................................
olxstr TLinkedLoopTable::SymmCodeToSymm(TCif *Cif, const olxstr &Code)  {
  olxstr Symm, Tmp;
  TCifLoop *SL = Cif->FindLoop("_symmetry_equiv_pos_as_xyz");
  if( SL == NULL )  return Symm;
  TStrList Toks(Code, '_');
  smatd mSymm;
  TCifLoopTable& LT = SL->GetTable();
  if( Toks.Count() == 1 )  {
    size_t isymm = Toks[0].ToSizeT()-1;
    if( isymm >= LT.RowCount() )  return Symm;
    Symm = LT[isymm][0];
    return Symm;
  }
  if( Toks.Count() != 2 )  return Symm;
  size_t isymm = Toks[0].ToSizeT()-1;
  if( isymm >= LT.RowCount() )  return Symm;
  if( Toks[1].Length() != 3 )  return Symm;
  TSymmParser::SymmToMatrix(LT[isymm][0], mSymm);
  mSymm.t[0] += (int)(Toks[1].CharAt(0)-'5');
  mSymm.t[1] += (int)(Toks[1].CharAt(1)-'5');
  mSymm.t[2] += (int)(Toks[1].CharAt(2)-'5');
  Symm = TSymmParser::MatrixToSymm(mSymm);
  return Symm;
}
//..............................................................................
bool TCif::ResolveParamsFromDictionary(TStrList &Dic, olxstr &String,
 olxch Quote,
 olxstr (*ResolveExternal)(const olxstr& valueName),
 bool DoubleTheta)  {

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
bool TCif::CreateTable(TDataItem *TD, TTTable<TStrList> &Table, smatd_list& SymmList)  {
  TCifLoop *Loop;
  int defcnt, RowDeleted=0, ColDeleted=0;
  olxstr Tmp, Val;
  bool AddRow;
  TStrList Toks;
  smatd_list AllSymmList;
  smatd SymmMatr;

  size_t sindex = InvalidIndex;
  TCifLoop* SymmLoop = FindLoop("_symmetry_equiv_pos");
  if( SymmLoop == NULL)
    SymmLoop = FindLoop("_symmetry_equiv_pos_as_xyz");
  if( SymmLoop != NULL )  {
    sindex = SymmLoop->GetTable().ColIndex("_symmetry_equiv_pos_as_xyz");
    TCifLoopTable& Table = SymmLoop->GetTable();
    for( size_t i=0; i < Table.RowCount(); i++ )  {
      smatd& Matrix = AllSymmList.AddNew();
        if( !TSymmParser::SymmToMatrix(Table[i][sindex], Matrix) )
          throw TFunctionFailedException(__OlxSourceInfo, "could not process symmetry matrix");
    }
  }

  SymmList.Clear();

  TCifLoopTable* LT = NULL;
  for( size_t i=0; i < Loops.Count(); i++ )  {
    Loop = Loops.GetObject(i);
    LT = &Loop->GetTable();
    if( LT->ColCount() < TD->ItemCount() )  continue;
    defcnt = 0;
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
    AddRow = true;
    for( size_t j=0; j < LT->ColCount(); j++ )  {
      TDataItem *DI = TD->FindItemi( LT->ColName(j) );

      if( sindex != InvalidIndex && LT->ColName(j).StartsFrom("_geom_") && LT->ColName(j).IndexOf("site_symmetry") != InvalidIndex)  {
        // 1_555
        if( (*LT)[i][j] != '.' )  {
          olxstr tmp = LT->ColName(j).SubStringFrom( LT->ColName(j).LastIndexOf('_')+1 );
          //if( !tmp.IsNumber() ) continue;
          Tmp = "label_";
          Tmp << tmp;
          SymmMatr = TSymmParser::SymmCodeToMatrix(AllSymmList, (*LT)[i][j] );
          size_t matIndex = SymmList.IndexOf( SymmMatr );
          if( matIndex == InvalidIndex )  {
            SymmList.AddCCopy(SymmMatr);
            matIndex = SymmList.Count()-1;
          }
          for( size_t k=0; k < LT->ColCount(); k++ )  {
            if( LT->ColName(k).EndsWith( Tmp ) )  {
              Table[i][k] << "<sup>" << (matIndex+1) << "</sup>";
              break;
            }
          }
        }
      }
      if( DI == NULL )  continue;
      Val = (*LT)[i][j];


      Tmp = DI->GetFieldValue("mustequal", EmptyString);
      Toks.Clear();
      Toks.Strtok(Tmp, ';');
      if( !Tmp.IsEmpty() && (Toks.IndexOfi(Val) == InvalidIndex) ) // equal to
      {  AddRow = false;  break;  }

      Tmp = DI->GetFieldValue("atypeequal", EmptyString);
      if( !Tmp.IsEmpty() )  {  // check for atom type equals to
        TCifLoopData* CD = (*LT)[i].GetObject(j);
        if( CD != NULL && CD->CA != NULL )
          if( !CD->CA->GetType().symbol.Equalsi(Tmp) )  {
            AddRow = false;
            break;
          }
      }
      Tmp = DI->GetFieldValue("atypenotequal", EmptyString);
      if( !Tmp.IsEmpty() )  {  // check for atom type equals to
        TCifLoopData* CD = (*LT)[i].GetObject(j);
        if( CD != NULL && CD->CA != NULL )
          if( CD->CA->GetType().symbol.Equalsi(Tmp) )  {
            AddRow = false;
            break;
          }
      }

      Tmp = DI->GetFieldValue("mustnotequal", EmptyString);
      Toks.Clear();
      Toks.Strtok(Tmp, ';');
      if( !Tmp.IsEmpty() && ( Toks.IndexOfi(Val) != InvalidIndex) ) // not equal to
      {  AddRow = false;  break;  }

      Tmp = DI->GetFieldValue("multiplier", EmptyString);
      if( Tmp.Length() )  {  // Multiply
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
    TDataItem *DI = TD->FindItemi( LT->ColName(i) );
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


