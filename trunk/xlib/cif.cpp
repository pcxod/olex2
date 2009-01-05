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
  for(int i=0; i < FTable.RowCount(); i++ )  {
    for(int j=0; j < FTable.ColCount(); j++ )
      delete FTable[i].Object(j);
  }
  FTable.Clear();
  FComments = EmptyString;
}
//..............................................................................
void TCifLoop::DeleteAtom( TCAtom *A )  {
  TCifLoopData *CD;
  for(int i=0; i < FTable.RowCount(); i++ )  {
    for(int j=0; j < FTable.ColCount(); j++ )  {
      CD = FTable[i].Object(j);
      if( CD->CA == A )  {
        for(int k=0; k < FTable.ColCount(); k++ )
          delete FTable[i].Object(k);
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
  olxstr D, Param;
  FComments = EmptyString;
  int DL, RowCount, ColCount = FTable.ColCount();
  for( int i=0; i < Data.Count(); i++ )  {
    if( Data[i].IsEmpty() )  continue;
    if( Data[i].CharAt(0) == '#' )  {
      FComments << Data[i] << '\n';
      Data.String(i) = EmptyString;
    }
  }
  D = Data.Text(" \\n ");
  TStrPObjList<olxstr,TCifLoopData*> Params;
  TCifLoopData *CData=NULL;
  D.DeleteSequencesOf(' ');
  olxch Char, SepChar;
  DL = D.Length();
  for( int i=0; i < DL; i++ )  {
    Param = EmptyString;
    Char = D.CharAt(i);
    if( Char == ' ' ) continue;
    if( Char == '\'' || Char == ';' || Char == '"')  {  // string param
      SepChar = Char;
      Char = 'a';
      while( Char != SepChar )  {
	      Param << D.CharAt(i);
        i++;
        if( i >= DL )  {
          Param.Delete(0, 1);
          Params.Add(Param, new TCifLoopData(true) );
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
    olxstr msg("wrong loop parameters number. ");
    msg << "Failed in loop: " << GetLoopName() << ". Failed on: \'" << Param << '\'';
    throw TFunctionFailedException(__OlxSourceInfo, msg );
  }
  RowCount = Params.Count() / ColCount;
  FTable.SetRowCount(RowCount);
  for( int i=0; i < RowCount; i++ )  {
    for( int j=0; j < ColCount; j++ )  {
      FTable[i].Object(j) = Params.Object(i*ColCount+j);
      FTable[i].String(j) = Params.String(i*ColCount+j);
    }
  }
}
//..............................................................................
void TCifLoop::SaveToStrings( TStrList& Strings )  {
  olxstr Tmp, str;
  TStrList toks, htoks;
  TCifLoopData *CLD;
  for( int i=0; i < FTable.ColCount(); i++ )  // loop header
    Strings.Add( olxstr("  ") << olxstr(FTable.ColName(i)) );

  for( int i=0; i < FTable.RowCount(); i++ ) {  // loop content
    Tmp = EmptyString;
    for( int j=0; j < FTable.ColCount(); j++ )  {
      CLD = FTable[i].Object(j);
      if( CLD->CA )  {
        if( CLD->CA->IsDeleted() )  {  // skip deleted atom
          Tmp = EmptyString;  break;
        }
      }
      // according to the cif rules, the strings cannot be hypernated ...
      str = FTable[i].String(j);
      if( str.EndsWith("\\n") )  str.SetLength( str.Length()-2 );
      if( CLD->String )  {
        if( str.IndexOf("\\n") != -1 )  {
          if( !Tmp.IsEmpty() )  {
            Strings.Add( Tmp );
            Tmp = EmptyString;
          }
          toks.Clear();
          toks.Strtok(str, "\\n");
          Strings.Add(';');
          for( int j=0; j < toks.Count(); j++ )  {
            htoks.Clear();
            htoks.Hypernate(toks[j], 78);
            for( int k=0; k < htoks.Count(); k++ )  {
              if( htoks[k].Length() > 1 )  // not just a space char
                Strings.Add(htoks[k]);
            }
          }
          Strings.Add(';');
        }
        else  {
          if( Tmp.Length() + 3 + str.Length() > 80 )  {
            Strings.Add( Tmp );
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
      if( Tmp.StartsFrom(" '") )  // remove quatations ...
        Tmp = Tmp.SubString(2, Tmp.Length()-3);
      toks.Strtok(Tmp, "\\n");
      Strings.Add(';');
      for( int j=0; j < toks.Count(); j++ )  {
        htoks.Clear();
        htoks.Hypernate(toks[j], 78);
        for( int k=0; k < htoks.Count(); k++ )  {
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
olxstr TCifLoop::GetLoopName()  {
  olxstr C;
  if( FTable.ColCount() == 0 )  return C;
  if( FTable.ColCount() == 1 )  return FTable.ColName(0);
  C = olxstr::CommonString(FTable.ColName(0), FTable.ColName(1));

  for( int i=2; i < FTable.ColCount(); i++ )
    C = olxstr::CommonString(FTable.ColName(i), C);

  if( C[C.Length()-1] == '_' )
    C.SetLength(C.Length()-1);
  return C;
}
//..............................................................................
olxstr TCifLoop::ShortColumnName(int in)  {
  return FTable.ColName(in).SubStringFrom( GetLoopName().Length() );
}
//..............................................................................
void TCifLoop::UpdateTable()  {
  for(int i=0; i < FTable.RowCount(); i++ )  {
    for(int j=0; j < FTable.ColCount(); j++ )  {
      if( FTable[i].Object(j)->CA )
        FTable[i].String(j) = FTable[i].Object(j)->CA->Label();                                      
    }
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
  for( int i=0; i < Items.Count(); i++ )  {
    if( Items[i].Count() !=  Atoms.Count() )  continue;
    for( int j=0; j < Atoms.Count(); j++ )  {
      bool found = false;
      for( int k=0; k < Atoms.Count(); k++ )  {
        if( Atoms[k]->CAtom().GetLoaderId() == Items[i].GetAtom(k).GetLoaderId() )  {
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
void TCifDataManager::Clear()  {  Items.Clear();  }
//----------------------------------------------------------------------------//
// TCif function bodies
//----------------------------------------------------------------------------//
TCif::TCif() : FDataNameUpperCase(true)  {  }
//..............................................................................
TCif::~TCif()  {  Clear();  }
//..............................................................................
void TCif::Clear()  {
  for( int i=0; i < Lines.Count(); i++ )  {
    if( Lines.Object(i) != NULL )  {
      delete Lines.Object(i)->Data;
      delete Lines.Object(i);
    }
  }
  Lines.Clear();
  Parameters.Clear();
  FDataName = EmptyString;
  FWeightA = EmptyString;
  FWeightB = EmptyString;
  for( int i=0; i < Loops.Count(); i++ )
    delete Loops.Object(i);
  Loops.Clear();
  GetAsymmUnit().Clear();
  GetRM().ClearAll();
}
//..............................................................................
void TCif::Format()  {
  TCifData *D;
  olxstr Tmp, Tmp1;
  TStrList toks;
  TStrList *NewData;
  int DL, li;
  olxch Char;
  bool AddSpace;
  for( int i=0; i < Lines.Count()-1; i++ )  {
    if( Lines[i].Length() == 0  && Lines[i+1].Length() == 0 )  {
      if( Lines.Object(i) != NULL )
        delete Lines.Object(i);
    }
  }
  Lines.Pack();
  for( int i=0; i < Lines.Count(); i++ )  {
    if( Lines.Object(i) != NULL )  {
      li = 0;
      D = Lines.Object(i);
      for( int j=0; j < D->Data->Count(); j++ )  {
        DL = D->Data->String(j).Length();
        for( int k=0; k < DL; k++ )  {
          Char = D->Data->String(j)[k];
          if( Char == '\'' || Char == '"' || Char == ';')  {
            D->Data->String(j)[k] = ' ';
            D->String = true;
          }
        }
        D->Data->String(j) = olxstr::DeleteSequencesOf(D->Data->String(j), ' ').Trim(' ');
        if( D->Data->String(j).Length() )  li++;
      }
      // check if a space should be added at the beginning of line
      AddSpace = false;
      if( li > 1 )  AddSpace = true;
      else  {
        if( D->Data->Count() == 1 )  {
          if( D->Data->String(0).Length() > 80 )
            AddSpace = true;
        }
      }

      NewData = new TStrList;
      for( int j=0; j < D->Data->Count(); j++ )  {
        DL = D->Data->String(j).Length();
        if( DL != 0 )  {
          toks.Clear();
          toks.Hypernate(D->Data->String(j), 78);
          for( int k=0; k < toks.Count(); k++ )  {
            if( AddSpace )  Tmp = ' '; // if a space should be added at the beginning of line
            else            Tmp = EmptyString;
            Tmp << toks.String(k);
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
  olxstr Tmp, Val, Param, Tmp1, Tmp2;
  olxch Char;
  bool String;
  TCifLoop *Loop;
  int spindex;
  TCifData *D;
  TStrList LoopData;

  Clear();
  Lines.Assign(Strings);
  for( int i=0; i < Lines.Count(); i++ )  {
    Lines[i] = Lines[i].Trim(' ');
    if( Lines[i].IsEmpty() )  continue;
    spindex = Tmp.FirstIndexOf('#');  // a comment char
    if( spindex != -1 )  {
      if( spindex != 0 )  {
        Tmp = Lines[i];
        Lines[i] = Tmp.SubStringTo(spindex-1);  // remove the '#' character
        Lines.Insert(i+1, Tmp.SubStringFrom(spindex));
        i++;
      }
    }
  }
  for( int i=0; i < Lines.Count(); i++ )  {
    Tmp = olxstr::DeleteSequencesOf<char>(Lines[i], ' ');
    if( Tmp.IsEmpty() )  {
      Lines[i] = EmptyString;
      continue;
    }
    if( Tmp.CharAt(0) == '#')  continue;
next_loop:
    if( Tmp.Comparei("loop_") == 0  )  {  // parse a loop
      Loop = new TCifLoop();
      LoopData.Clear();
      Loops.Add(EmptyString, Loop);
      Char = '_';
      while( Char == '_' )  {  // skip loop definition
        i++;
        if( i >= Lines.Count() )  {  // // end of file?
          Loop->Format(LoopData); // parse the loop
          Loops.String(Loops.Count()-1) = Loop->GetLoopName();
          goto exit;
        }
        Tmp = olxstr::DeleteSequencesOf<char>( Lines[i], ' ' );
        if( Tmp.IsEmpty() )  continue;
        else                 Char = Tmp.CharAt(0);
        if( Char == '_' )  {
          if( Loop->Table().ColCount() )  {
            // chceck that the itm is actualli belongs to the loop
            // happens in the case of empty loops
            if( olxstr::CommonString(Tmp, Loop->Table().ColName(0)).Length() == 1 )  {
              goto finalize_loop;
            }
          }
          Loop->Table().AddCol(Tmp);
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
          Loops.Last().String() = Loop->GetLoopName();
          goto exit;
        }
        Tmp = olxstr::DeleteSequencesOf<char>(Lines[i], ' ');
        if( Tmp == "loop_" )  goto finalize_loop;   // a new loop started
        if( Tmp.IsEmpty() )   continue;
        else                  Char = Tmp.CharAt(0);
      }
finalize_loop:
      Loop->Format(LoopData);
      Loops.Last().String() = Loop->GetLoopName();
      if( Tmp == "loop_" )
        goto next_loop;
    }
    if( Tmp.CharAt(0) == '_' )  {  // parameter
      String = false;
      spindex = Tmp.FirstIndexOf(' ');
      if( spindex >=0 )  {
        Param = Tmp.SubStringTo(spindex);
        Val = Tmp.SubStringFrom(spindex+1); // to remove the space
      }
      else  {
        Param = Tmp;
        Val = EmptyString;
      }
      Lines[i] = Param;
      D = new TCifData;
      D->Data = new TStrList;
      D->String = false;
      Lines.Object(i) = D;
      Parameters.Add(Param, D);
      if( !Val.IsEmpty() )
        D->Data->Add(Val);
      int j = i + 1;
      if( j < Lines.Count() )  {  // initialise 'Char'
        Tmp1 = olxstr::DeleteSequencesOf<char>(Lines[j], ' ');
        if( !Tmp1.IsEmpty() )  Char = Tmp1.CharAt(0);
        else Char = 'a';  // whil initialte the while cycle bolow
      }
      while( Char != '_' && (j < Lines.Count()) )  {
        Tmp1 = olxstr::DeleteSequencesOf<char>(Lines[j], ' ');
        if( Tmp1.Length() )  {
          Char = Tmp1[0];
          if( Char == '#' )  {  j++;  continue;  }
          if( Tmp1.Length() > 4 )  {  // check for special data items
            Tmp2 = Tmp1.SubString(0,4);
            if( (Tmp2.Comparei("data") == 0)  || (Tmp2.Comparei("loop") == 0)  )  break;
          }
          if( Char != '_' )  {
            D->Data->Add(Tmp1);
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
      if( !FDataName.Length() )  {
        Tmp1 = Tmp.SubStringTo(4);
        if( Tmp1 == "data" )  {
          if( FDataNameUpperCase )
            FDataName = Tmp.SubStringFrom(5).UpperCase();
          else
            FDataName = Tmp.SubStringFrom(5);
          FDataName.DeleteSequencesOf(' ');
          Tmp1 << '_';
          Tmp1 << FDataName;
          Lines[i] = Tmp1;
        }
      }
    }
  }
exit:
  Format();
  /******************************************************************/
  /*search fo the weigting sceme*************************************/
  D = FindParam( "_refine_ls_weighting_details");
  if( D != NULL)  {
    if( D->Data->Count() == 1 )  {
      const olxstr& tmp = D->Data->String(0);
      for( int k=0; k < tmp.Length(); k++ )  {
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
  for( int i=0; i < Lines.Count()-1; i++ )  {
    if( !Lines.String(i).Length() && !Lines.String(i+1).Length() )  {
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
  for( int i=0; i < Lines.Count(); i++ )  {
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
void GroupSection(TStrPObjList<olxstr,TCifData*>& lines, int index,
       const olxstr& sectionName, AnAssociation2<int,int>& indexes)  {
  olxstr tmp;
  for( int i=index; i < lines.Count(); i++ )  {
    tmp = lines[i].Trim(' ');
    if( tmp.IsEmpty() || tmp == "loop_" )  continue;
    int ind = tmp.FirstIndexOf('_', 1);
    if( ind <= 0 ) // a _loop ?
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
  TCSTypeList<olxstr, AnAssociation2<int,int> > sections;
  olxstr tmp;
  for( int i=0; i < Lines.Count(); i++ )  {
    tmp = Lines[i].Trim(' ');
    if( tmp.IsEmpty() || tmp == "loop_" )  continue;
    int ind = tmp.FirstIndexOf('_', 1);
    if( ind <= 0 ) // a _loop ?
      continue;
    tmp = tmp.SubStringTo(ind);
    ind = sections.IndexOfComparable(tmp);
    if( ind == -1 )  {
      sections.Add( tmp, AnAssociation2<int,int>(i,i) );
      AnAssociation2<int,int>& indexes = sections[tmp];
      GroupSection(Lines, i+1, tmp, indexes);
    }
  }
  // sorting the groups internally ...
  for( int i=0; i < sections.Count(); i++ )  {
    int ss = sections.GetObject(i).GetA(),
        se = sections.GetObject(i).GetB();
    bool changes = true;
    while( changes )  {
      changes = false;
      for( int j=ss; j < se; j++ )  {
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
  olxstr Tmp, Tmp1;
  int loopc=0;
  //Lines.Sort();
  for( int i=0; i < Lines.Count(); i++ )  {
    Tmp = Lines[i];
    Tmp1 = olxstr::DeleteSequencesOf<char>(Tmp.ToLowerCase(), ' ');
    if( Tmp1.LowerCase() == "loop_" )  {
      if( loopc < Loops.Count() )  {
        Loops.Object(loopc)->UpdateTable();
        // skip empty loops, as they break the format
        if( Loops.Object(loopc)->Table().RowCount() != 0 )  {
          Strings.Add("loop_");
          Loops.Object(loopc)->SaveToStrings(Strings);
        }
      }
      loopc++;
      if( (i+1) < Lines.Count() && !Lines[i+1].IsEmpty() )  // add a
        Strings.Add(EmptyString);
      continue;
    }
    if( Lines.Object(i) != NULL )  {
      Tmp.Format(34, true, ' ');
      TCifData* D = Lines.Object(i);
      if( D->Data->Count() > 1 )  {
        Strings.Add(Tmp);
        Strings.Add(";");
        for( int j=0; j < D->Data->Count(); j++ )
          Strings.Add( D->Data->String(j) );
        Strings.Add(";");
      }
      else  {
        if( D->Data->Count() == 1 )  {
          if( (D->Data->String(0).Length() + 34) >= 80 )  {
            Strings.Add(Tmp);
            Strings.Add(";");
            Strings.Add(D->Data->String(0));
            Strings.Add(";");
          }
          else  {
            if( D->String )  {
              Tmp << '\'' << D->Data->String(0) << '\'';
            }
            else
              Tmp << D->Data->String(0);
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
  return (Lines.IndexOf(Param) == -1) ? false : true;
}
//..............................................................................
const olxstr& TCif::GetSParam(const olxstr &Param) const {
  if( Param[0] != '_' )
    return EmptyString;
  int i = Lines.IndexOf(Param);
  if( i >= 0 )  {
    if( Lines.Object(i)->Data->Count() >= 1 )
      return Lines.Object(i)->Data->String(0);
    return EmptyString;
  }
  return EmptyString;
}
//..............................................................................
TCifData *TCif::FindParam(const olxstr &Param)  {
  if( Param[0] != '_' )
    return NULL;
  int i = Lines.IndexOf(Param);
  return (i == -1 ) ? NULL : Lines.Object(i);
}
//..............................................................................
bool TCif::SetParam(const olxstr &Param, TCifData *Params)  {
  int i = Lines.IndexOf(Param);
  if( i >= 0 )  {
    Lines.Object(i)->Data->Assign(*(Params->Data));
    Lines.Object(i)->String = Params->String;
    return true;
  }
  return false;
}
//..............................................................................
bool TCif::AddParam(const olxstr &Param, TCifData *Params)  {
  int i = Lines.IndexOf(Param);
  if( i >= 0 )  return false;
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
  int i = Lines.IndexOf(Param);
  if( i >= 0 )  return false;
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
  TCAtom *A;
  double Q[6], E[6]; // quadratic form of ellipsoid
  TEValueD EValue;
  GetAsymmUnit().Axes()[0] = GetSParam("_cell_length_a");
  GetAsymmUnit().Axes()[1] = GetSParam("_cell_length_b");
  GetAsymmUnit().Axes()[2] = GetSParam("_cell_length_c");

  GetAsymmUnit().Angles()[0] = GetSParam("_cell_angle_alpha");
  GetAsymmUnit().Angles()[1] = GetSParam("_cell_angle_beta");
  GetAsymmUnit().Angles()[2] = GetSParam("_cell_angle_gamma");

  // check if the ci file contains valid parameters
  double fv =  GetAsymmUnit().Axes()[0].GetV() * GetAsymmUnit().Axes()[1].GetV() * GetAsymmUnit().Axes()[2].GetV() *
        GetAsymmUnit().Angles()[0].GetV() * GetAsymmUnit().Angles()[1].GetV() * GetAsymmUnit().Angles()[2].GetV();
  if( fv == 0 )  {    return;  }

  GetAsymmUnit().InitMatrices();

  Loop = FindLoop("_symmetry_equiv_pos");
  if( Loop == NULL )  Loop = FindLoop("_symmetry_equiv_pos_as_xyz");
  if( Loop!= NULL  )  {
    int sindex = Loop->Table().ColIndex("_symmetry_equiv_pos_as_xyz");
    if( sindex >= 0 )  {
      for( int i=0; i < Loop->Table().RowCount(); i++ )  {
        smatd Matrix;
        if( TSymmParser::SymmToMatrix(Loop->Table()[i].String(sindex), Matrix) )  {
          GetAsymmUnit().AddMatrix(Matrix);
        }
        else
          throw TFunctionFailedException(__OlxSourceInfo, "could not process symmetry matrix");
      }
    }
    TSpaceGroup* sg = TSymmLib::GetInstance()->FindExpandedSG( GetAsymmUnit() );
    if( sg != NULL )  {
      GetAsymmUnit().ChangeSpaceGroup( *sg );
    }
    else  {
      throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
    }
  }
  this->Title = FDataName.UpperCase();
  this->Title << " OLEX: imported from CIF";

  ALoop = FindLoop("_atom_site");
  if( !ALoop )  return;

  int ALabel =  ALoop->Table().ColIndex("_atom_site_label"), ALabel1, ALabel2;
  int ACx =     ALoop->Table().ColIndex("_atom_site_fract_x");
  int ACy =     ALoop->Table().ColIndex("_atom_site_fract_y");
  int ACz =     ALoop->Table().ColIndex("_atom_site_fract_z");
  int ACUiso =  ALoop->Table().ColIndex("_atom_site_U_iso_or_equiv");
  int ASymbol = ALoop->Table().ColIndex("_atom_site_type_symbol");
  if( ALabel < 0 || ACx < 0 || ACy < 0 || ACz < 0 || ASymbol < 0)  return;
  TAtomsInfo& atoms_info = TAtomsInfo::GetInstance();
  for( int i=0; i < ALoop->Table().RowCount(); i++ )  {
    A = &GetAsymmUnit().NewAtom();
    A->SetLoaderId(GetAsymmUnit().AtomCount()-1);
    A->SetLabel( ALoop->Table()[i][ALabel] );
    A->SetAtomInfo( atoms_info.FindAtomInfoBySymbol(ALoop->Table()[i][ASymbol]) );
    EValue = ALoop->Table()[i][ACx];
    A->ccrd()[0] = EValue.GetV();  A->ccrdEsd()[0] = EValue.GetE();
    EValue = ALoop->Table()[i].String(ACy);
    A->ccrd()[1] = EValue.GetV();  A->ccrdEsd()[1] = EValue.GetE();
    EValue = ALoop->Table()[i].String(ACz);
    A->ccrd()[2] = EValue.GetV();  A->ccrdEsd()[2] = EValue.GetE();
    if( ACUiso >= 0 )    {
      EValue = ALoop->Table()[i].String(ACUiso);
      A->SetUisoEsd( EValue.GetE() );
      A->SetUiso( EValue.GetV() );
    }

//    if( !A->Info ) ;
    ALoop->Table()[i].Object(ALabel)->CA = A;
  }
  for( int i=0; i < Loops.Count(); i++ )  {
    if( Loops.Object(i) == ALoop )  continue;
    for( int j=0; j < Loops.Object(i)->Table().ColCount(); j++ )  {
      ALabel1 = Loops.Object(i)->Table().ColName(j).IndexOf("atom_site" );
      ALabel2 = Loops.Object(i)->Table().ColName(j).IndexOf("label" );
      if(  ALabel1 >= 0 && ALabel2 >= 0 )  {
        for( int k=0; k < Loops.Object(i)->Table().RowCount(); k++ )  {
          ALabel1 = ALabel;
          Loops.Object(i)->Table()[k].Object(j)->CA =
               GetAsymmUnit().FindCAtom( Loops.Object(i)->Table()[k].String(j) );
        }
      }
    }
  }

  ALoop = FindLoop("_atom_site_aniso");
  if( !ALoop )  return;
  int U11, U22, U33, U12, U13, U23;
  ALabel =  ALoop->Table().ColIndex("_atom_site_aniso_label");
  U11 =     ALoop->Table().ColIndex("_atom_site_aniso_U_11");
  U22 =     ALoop->Table().ColIndex("_atom_site_aniso_U_22");
  U33 =     ALoop->Table().ColIndex("_atom_site_aniso_U_33");
  U23 =     ALoop->Table().ColIndex("_atom_site_aniso_U_23");
  U13 =     ALoop->Table().ColIndex("_atom_site_aniso_U_13");
  U12 =     ALoop->Table().ColIndex("_atom_site_aniso_U_12");
  if( ALabel < 0 || U11 < 0 || U22 < 0 || U33 < 0 || U23 < 0 || U13 < 0 || U12 < 0 )  return;
  for( int i=0; i < ALoop->Table().RowCount(); i++ )  {
    A = GetAsymmUnit().FindCAtom( ALoop->Table()[i].String(ALabel) );
    if( A == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, olxstr("wrong atom in the aniso loop ") << ALabel);
    EValue = ALoop->Table()[i][U11];  Q[0] = EValue.GetV();  E[0] = EValue.GetE();
    EValue = ALoop->Table()[i][U22];  Q[1] = EValue.GetV();  E[1] = EValue.GetE();
    EValue = ALoop->Table()[i][U33];  Q[2] = EValue.GetV();  E[2] = EValue.GetE();
    EValue = ALoop->Table()[i][U23];  Q[3] = EValue.GetV();  E[3] = EValue.GetE();
    EValue = ALoop->Table()[i][U13];  Q[4] = EValue.GetV();  E[4] = EValue.GetE();
    EValue = ALoop->Table()[i][U12];  Q[5] = EValue.GetV();  E[5] = EValue.GetE();
    GetAsymmUnit().UcifToUcart(Q);
    A->AssignEllp( &GetAsymmUnit().NewEllp().Initialise(Q, E));
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
TCifLoop& TCif::Loop(int i) {
  return *Loops.Object(i);
};
//..............................................................................
TCifLoop *TCif::FindLoop(const olxstr &L)  {
  int i = Loops.IndexOf(L);
  return (i == -1) ? NULL : Loops.Object(i);
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
TCifLoop& TCif::PublicationInfoLoop()  {
  const static olxstr publ_ln( "_publ_author" ), publ_jn("_publ_requested_journal");
  TCifLoop *CF = FindLoop( publ_ln );
  if( CF != NULL )  return *CF;
  int index = -1;
  for( int i=0; i < Lines.Count(); i++ )  {
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
  CF->Table().AddCol("_publ_author_name");
  CF->Table().AddCol("_publ_author_email");
  CF->Table().AddCol("_publ_author_address");
  return *CF;
}
//..............................................................................
bool TCif::Adopt(TXFile *XF)  {
  Clear();
  olxstr Param;
  TCifLoop *Loop;
  TCifLoopTable *Table, *ATable;
  TCAtom *A;
  TEValueD EValue;
  double Q[6], E[6];  // quadratic form of s thermal ellipsoid
  bool AddUTable=false;

  GetAsymmUnit().Assign( XF->GetAsymmUnit() );
  GetAsymmUnit().SetZ( (short)XF->GetLattice().GetUnitCell().MatrixCount());
  Title = "CIFEXP";

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

  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( XF->GetAsymmUnit() );
  if( sg == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");

  AddParam("_cell_formula_units_Z", XF->GetAsymmUnit().GetZ(), true);
  AddParam("_symmetry_cell_setting", sg->GetBravaisLattice().GetName(), true);
  AddParam("_symmetry_space_group_name_H-M", sg->GetName(), true);

  Loop = &AddLoop("_symmetry_equiv_pos_as_xyz");
  Table = &Loop->Table();
  Table->AddCol("_symmetry_equiv_pos_as_xyz");

  smatd_list matrices;
  sg->GetMatrices(matrices, mattAll);

  for( int i=0; i < matrices.Count(); i++ )  {
    TStrPObjList<olxstr,TCifLoopData*>& Row = Table->AddRow(EmptyString);
    Row[0] = TSymmParser::MatrixToSymm( matrices[i] );
    Row.Object(0) = new TCifLoopData(true);
  }

  Loop = &AddLoop("_atom_site");
  Table = &Loop->Table();
  Table->AddCol("_atom_site_label");
  Table->AddCol("_atom_site_type_symbol");
  Table->AddCol("_atom_site_fract_x");
  Table->AddCol("_atom_site_fract_y");
  Table->AddCol("_atom_site_fract_z");

  for( int i = 0; i < GetAsymmUnit().AtomCount(); i++ )  {
    A = &GetAsymmUnit().GetAtom(i);
    if( A->GetEllipsoid() != NULL )  {
      AddUTable = true;  break;
    }
  }
  if( AddUTable )  {
    Loop = &AddLoop("_atom_site_aniso");
    ATable = &Loop->Table();
    ATable->AddCol("_atom_site_aniso_label");
    ATable->AddCol("_atom_site_aniso_U_11");
    ATable->AddCol("_atom_site_aniso_U_22");
    ATable->AddCol("_atom_site_aniso_U_33");
    ATable->AddCol("_atom_site_aniso_U_23");
    ATable->AddCol("_atom_site_aniso_U_13");
    ATable->AddCol("_atom_site_aniso_U_12");
  }

  for( int i = 0; i < GetAsymmUnit().AtomCount(); i++ )  {
    A = &GetAsymmUnit().GetAtom(i);
    TStrPObjList<olxstr,TCifLoopData*>& Row = Table->AddRow(EmptyString);
    Row[0] = A->Label();  Row.Object(0) = new TCifLoopData(A);
    Row[1] = A->GetAtomInfo().GetSymbol();  Row.Object(1) = new TCifLoopData;
    for( int j=0; j < 3; j++ )  {
      EValue.V() = A->ccrd()[j];  EValue.E() = A->ccrdEsd()[j];
      Row[j+2] = EValue.ToCStr();
      Row.Object(j+2) = new TCifLoopData;
    }      
    if( A->GetEllipsoid() != NULL )  {
      A->GetEllipsoid()->GetQuad(Q, E);
      GetAsymmUnit().UcartToUcif(Q);

      TStrPObjList<olxstr,TCifLoopData*>& Row1 = ATable->AddRow(EmptyString);
      Row1[0] = A->Label();  Row1.Object(0) = new TCifLoopData(A);
      for( int j=0; j < 3; j++ )  {
        EValue.V() = Q[j];  EValue.E() = E[j];
        Row1[j+1] = EValue.ToCStr();
        Row1.Object(j+1) = new TCifLoopData;
      }
    }
  }
  if( XF->HasLastLoader() )
    GetRM().SetHKLSource( XF->LastLoader()->GetRM().GetHKLSource() );

  return true;
}
//----------------------------------------------------------------------------//
// TLLTBondSort function bodies - bond sorting procedure in TLinkedLoopTable
//----------------------------------------------------------------------------//
int TLLTBondSort::Compare(void *I, void *I1)
{
  double v;
  if( SortType & slltLength ) // length, Mr, Label
  {
    v = ((TLBond*)I)->Value.ToDouble() - ((TLBond*)I1)->Value.ToDouble();
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    v = ((TLBond*)I)->Another(Atom)->CA->GetAtomInfo().GetMr() - ((TLBond*)I1)->Another(Atom)->CA->GetAtomInfo().GetMr();
    if( v > 0 ) return 1;
    if( v < 0 ) return -1;
    v = ((TLBond*)I)->Another(Atom)->Label.Compare(((TLBond*)I1)->Another(Atom)->Label);
    if( !v )
      return Symmetry->IndexOf(((TLBond*)I)->S2) - Symmetry->IndexOf(((TLBond*)I1)->S2);
  }
  if( SortType & slltName )  // Name, length
  {
    v = ((TLBond*)I)->Another(Atom)->Label.Compare(((TLBond*)I1)->Another(Atom)->Label);
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    if( !v )
    {
      v = Symmetry->IndexOf(((TLBond*)I)->S2) - Symmetry->IndexOf(((TLBond*)I1)->S2);
      if( v < 0 ) return -1;
      if( v > 0 ) return 1;
    }
    v = ((TLBond*)I)->Value.ToDouble() - ((TLBond*)I1)->Value.ToDouble();
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    return 0;
  }
  if( SortType & slltMw )  // Mr, Length, Label
  {
    v = ((TLBond*)I)->Another(Atom)->CA->GetAtomInfo().GetMr() - ((TLBond*)I1)->Another(Atom)->CA->GetAtomInfo().GetMr();
    if( v < 0 ) return -1;
    if( v > 0 ) return 1;
    v = ((TLBond*)I)->Value.ToDouble() - ((TLBond*)I1)->Value.ToDouble();
    if( v > 0 ) return 1;
    if( v < 0 ) return -1;
    if( !v )
      return Symmetry->IndexOf(((TLBond*)I)->S2) - Symmetry->IndexOf(((TLBond*)I1)->S2);
  }
  return 0;
}
//----------------------------------------------------------------------------//
// TLBond function bodies - bond objsect for TLinkedLoopTable
//----------------------------------------------------------------------------//
TLAtom *TLBond::Another(TLAtom *A)
{
  if(A == A1)     return A2; 
  if(A == A2)     return A1; 
  return NULL;
}
//..............................................................................
bool TLBond::operator == (TLBond * B)
{
  if( A1 == B->A1 && A2 == B->A2 && S2 == B->S2 )
    return true;
  return false;
}
//----------------------------------------------------------------------------//
// TLAngle function bodies - angle objsect for TLinkedLoopTable
//----------------------------------------------------------------------------//
bool TLAngle::Contains(TLAtom *A)
{
  if( (A1==A) || (A2==A) || (A3 == A) ) return true; 
  return false;
}
//..............................................................................
bool TLAngle::FormedBy(TLBond *B, TLBond *B1 )  {
  TLAngle LA;
  if( B->A1 == A2 )  {
    LA.A1 = B->A2;
    LA.S1 = B->S2;
  }
  if( B->A2 == A2 )  {
    LA.A1 = B->A1;
    LA.S1 = ".";
  }
  if( B1->A1 == A2 )  {
    LA.A3 = B1->A2;
    LA.S3 = B1->S2;
  }
  if( B1->A2 == A2 )  {
    LA.A3 = B1->A1;
    LA.S3 = ".";
  }
  if( LA.A1 == A1 && LA.A3 == A3 )  {
//    if( LA.S1 == S1 && LA.S3 == S3 )
      return true;
  }
  if( LA.A1 == A3 && LA.A3 == A1 )  {
//    if( LA.S1 == S3 && LA.S3 == S1 )
      return true;
  }
  return false;
}
//----------------------------------------------------------------------------//
// TLinkedLoopTable function bodies
//----------------------------------------------------------------------------//
TLinkedLoopTable::TLinkedLoopTable(TCif *C)  {
  FAtoms = new TEList;
  FBonds = new TEList;
  FAngles = new TEList;
  FCif = C;
  BondSort.SortType = slltLength;
  olxstr Tmp;
  TCifLoop *CL;
  TCifLoopTable *Table;
  TLAtom *LA;
  TLBond *LB, *LB1;
  TLAngle *LAn;
  TCAtom *CA;
  int index, index1, index2, index3, index4, index5;
  bool uniq;

  for( int j=0; j < C->GetAsymmUnit().AtomCount(); j++ )  {
    CA = &C->GetAsymmUnit().GetAtom(j);
    LA = new TLAtom;
    LA->Bonds = new TEList;
    LA->Angles = new TEList;
    FAtoms->Add(LA);
    LA->Label = CA->Label();
    LA->CA = CA;
  }
  CL = C->FindLoop("_geom_bond");
  if( !CL ) return;

  Table = &CL->Table();
  index =  Table->ColIndex("_geom_bond_atom_site_label_1");
  index1 = Table->ColIndex("_geom_bond_atom_site_label_2");
  index2 = Table->ColIndex("_geom_bond_distance");
  index3 = Table->ColIndex("_geom_bond_site_symmetry_2");
  if( (index == -1) || (index1 == -1) || (index2 == -1) || (index3 == -1))  return;  // will not work then ...
  for( int j=0; j < Table->RowCount(); j++ )  {
    TStrPObjList<olxstr,TCifLoopData*>& L = (*Table)[j];
    LB = new TLBond;
    Tmp = L[index];
    LB->A1 = AtomByName(Tmp);
    Tmp = L[index1];
    LB->A2 = AtomByName(Tmp);
    LB->Value = L[index2];
    LB->S2 = L[index3];
    uniq = true;
    for( int k=0; k < FBonds->Count(); k++ )  {
      LB1 = (TLBond*)FBonds->Item(k);
      if( ((LB->A1 == LB1->A2) && (LB->A2 == LB1->A1)) )  {  // only then atoms are inverted !!
        if( LB->Value == LB1->Value )  {
          uniq = false;
          break;
        }
      }
    }
    uniq = true;
    if( uniq )  {
      FBonds->Add(LB);
      LB->A1->Bonds->Add(LB);
      if( LB->S2 == "." )
        LB->A2->Bonds->Add(LB);
    }
    else
    delete LB;
  }

  CL = C->FindLoop("_geom_angle");
  if( !CL ) return;

  Table = &CL->Table();

  index =  Table->ColIndex("_geom_angle_atom_site_label_1");
  index1 = Table->ColIndex("_geom_angle_atom_site_label_2");
  index2 = Table->ColIndex("_geom_angle_atom_site_label_3");
  index3 = Table->ColIndex("_geom_angle");
  index4 = Table->ColIndex("_geom_angle_site_symmetry_1");
  index5 = Table->ColIndex("_geom_angle_site_symmetry_3");
  if( (index == -1) || (index1 == -1) || (index2 == -1) || (index3 == -1) || (index4 == -1) || (index5 == -1))  
    return;  // will not work then ...
  for( int j=0; j < Table->RowCount(); j++ )  {
    TStrPObjList<olxstr,TCifLoopData*>& L = (*Table)[j];
    LAn = new TLAngle;
    FAngles->Add(LAn);
    Tmp = L[index];
    LAn->A1 = AtomByName(Tmp);
    Tmp = L[index1];
    LAn->A2 = AtomByName(Tmp);
    Tmp = L[index2];
    LAn->A3 = AtomByName(Tmp);
    LAn->A2->Angles->Add(LAn);
    LAn->Value = L[index3];
    LAn->S1 = L[index4];
    LAn->S3 = L[index5];
  }
}
//..............................................................................
TLinkedLoopTable::~TLinkedLoopTable()  {
  TLAtom *A;
  for( int i=0; i < FAtoms->Count(); i++ )  {
    A = (TLAtom*)FAtoms->Item(i);
    delete A->Bonds;
    delete A->Angles;
    delete A;
  }
  delete FAtoms;
  for( int i=0; i < FBonds->Count(); i++ )
    delete (TLBond*)FBonds->Item(i);
  delete FBonds;
  for( int i=0; i < FAngles->Count(); i++ )
    delete (TLAngle*)FAngles->Item(i);
  delete FAngles;
}
//..............................................................................
TLAtom *TLinkedLoopTable::AtomByName(const olxstr &Name)  {
  TLAtom *A;
  for( int i=0; i < FAtoms->Count(); i++ )  {
    A = (TLAtom*)FAtoms->Item(i);
    if( A->Label == Name )
      return A;
  }
  return NULL;
}
//..............................................................................
TTTable<TStrList>* TLinkedLoopTable::MakeTable(const olxstr &Atom)  {
  TLBond *LB, *LB1;
  TLAngle *LA;

  TLAtom *A = AtomByName(Atom);

  if( A == NULL || A->Bonds->IsEmpty() )
    return NULL;
  bool found;
  int bc = A->Bonds->Count(), sind;
  TLAtom *AAtom;
  olxstr Tmp, Tmp1;
  TStrList Symm;
  Symm.Add("."); // to give proper numbering of symm operations
  // search for symm operations
  for( int i=0; i < bc; i++ )  {
    LB = (TLBond*)A->Bonds->Item(i);
    sind = Symm.IndexOf(LB->S2);
    if(  sind == -1 )
      Symm.Add(LB->S2);
  }
  // sort bonds according to the requirements
  BondSort.Atom = A;
  BondSort.Symmetry = &Symm;
  A->Bonds->Sort(&BondSort);

  Table.Resize(bc+1, bc);
  Table.EmptyContent(true);
  Table.ColName(0) = A->Label;
  for( int i=0; i < bc-1; i++ )  {
    LB = (TLBond*)A->Bonds->Item(i);
    AAtom = LB->Another(A);
    Tmp = AAtom->Label;
    sind = Symm.IndexOf(LB->S2);
    if( sind != 0 )
      Tmp << "<sup>" << sind << "</sup>";
    Table.ColName(i+1) = Tmp;
  }
  for( int i=0; i < bc; i++ )  {
    LB = (TLBond*)A->Bonds->Item(i);
    AAtom = LB->Another(A);
    Tmp = AAtom->Label;
    sind = Symm.IndexOf(LB->S2);
    if( sind != 0 )
      Tmp << "<sup>" << sind << "</sup>";

    Table.RowName(i) = Tmp;

    Table[i].String(0) = LB->Value;
    for( int j=0; j < bc-1; j++ )  {
      LB1 = (TLBond*)A->Bonds->Item(j);
      if( i==j )  {
        Table[i].String(j+1) = "-";
        continue;
      }
      if( i < j )  {
        Table[i].String(j+1) = EmptyString;
        continue;
      }
      found = false;
      for( int k=0; k < A->Angles->Count(); k++ )  {
        LA = (TLAngle*)A->Angles->Item(k);
        if( LA->FormedBy(LB, LB1) )  {
          found = true;
          Table[i].String(j+1) = LA->Value;
          break;
        }
      }
      if( !found )
        Table[i].String(j+1) = '?';
    }
  }
  Tmp = EmptyString;
  for( int i=0; i < Symm.Count(); i++ )  {
    if( Symm.String(i).Length() > 0 &&  (Symm.String(i) != ".") )  {
      Tmp1 = SymmCodeToSymm(FCif, Symm.String(i));
      if( Tmp1.Length() )  {
        Tmp << "<sup>" << i << "</sup>" << ": " << Tmp1;
        if( i < (Symm.Count()-1) )
          Tmp << "<br>";
      }
    }
  }
  Table[A->Bonds->Count()].String(0) = Tmp;
  return &Table;
}
//..............................................................................
olxstr TLinkedLoopTable::SymmCodeToSymm(TCif *Cif, const olxstr &Code)  {
  olxstr Symm, Tmp;
  TCifLoop *SL = Cif->FindLoop("_symmetry_equiv_pos_as_xyz");
  if( SL == NULL )  return Symm;
  TStrList Toks(Code, '_');
  TCifLoopTable* LT;
  int isymm;
  smatd mSymm;
  LT = &SL->Table();
  if( Toks.Count() == 1 )  {
    isymm = Toks[0].ToInt()-1;
    if( isymm < 0 || isymm >= LT->RowCount() )  return Symm;
    Symm = (*LT)[isymm][0];
    return Symm;
  }
  if( Toks.Count() != 2 )  return Symm;
  isymm = Toks[0].ToInt()-1;
  if( isymm < 0 || isymm >= LT->RowCount() )  return Symm;
  if( Toks.String(1).Length() != 3 )  return Symm;
  TSymmParser::SymmToMatrix((*LT)[isymm][0], mSymm);
  mSymm.t[0] += (int)(Toks.String(1)[0]-'5');
  mSymm.t[1] += (int)(Toks.String(1)[1]-'5');
  mSymm.t[2] += (int)(Toks.String(1)[2]-'5');
  Symm = TSymmParser::MatrixToSymm(mSymm);
  return Symm;
}
//..............................................................................
void TCif::DeleteAtom(TCAtom *A)  {
  for( int i=0; i < LoopCount(); i++ )
    Loop(i).DeleteAtom(A);
}
//..............................................................................
bool TCif::ResolveParamsFromDictionary(TStrList &Dic, olxstr &String,
 olxch Quote,
 olxstr (*ResolveExternal)(const olxstr& valueName),
 bool DoubleTheta)  {

  TCifData *Params;

  olxstr Tmp, Val, SVal;
  int index, start, end;
  double theta;

  for( int i=0; i < String.Length(); i++ )  {
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
              Tmp = Params->Data->String(0);
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
          SVal = Dic.String(index-1);
          Tmp = EmptyString;
          if( SVal.Length() != 0 )  {
            if( SVal.Comparei("date") == 0 )  {
              Tmp = TETime::FormatDateTime( TETime::Now() );
              String.Insert(Tmp, start);
            }
            else if( !SVal.Comparei("sg_number") )  {
              TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG( GetAsymmUnit() );
              if( sg != NULL )
                Tmp = sg->GetNumber();
              else
                Tmp = "unknown";
            }
            else if( !SVal.Comparei("data_name") )
              Tmp = GetDataName();
            else if( !SVal.Comparei("weighta") )
              Tmp = GetWeightA();
            else if( !SVal.Comparei("weightb") )
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
                if( Params->Data->String(0).IsEmpty() )  {
                  TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not found");
                  Tmp = "none";
                }
                else if( Params->Data->String(0)[0] == '?' )  {
                  TBasicApp::GetLog().Info(olxstr("Value of parameter \'") << SVal << "' is not defined");
                  Tmp = "?";
                }
                else
                  Tmp = Params->Data->String(0);
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
  int c = Val.Length();
  double   dV, dM = N.ToDouble();
  int i = 0;
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
  TCifLoopTable* LT;
  TDataItem *DI;
  bool AddRow;
  TStrList Toks;
  smatd_list AllSymmList;
  smatd SymmMatr;

  int sindex = -1;
  TCifLoop* SymmLoop = FindLoop("_symmetry_equiv_pos");
  if( SymmLoop == NULL)
    SymmLoop = FindLoop("_symmetry_equiv_pos_as_xyz");
  if( SymmLoop != NULL )  {
    sindex = SymmLoop->Table().ColIndex("_symmetry_equiv_pos_as_xyz");
    TCifLoopTable* Table = &SymmLoop->Table();
    for( int i=0; i < Table->RowCount(); i++ )  {
      smatd& Matrix = AllSymmList.AddNew();
        if( !TSymmParser::SymmToMatrix((*Table)[i][sindex], Matrix) )
          throw TFunctionFailedException(__OlxSourceInfo, "could not process symmetry matrix");
    }
  }

  SymmList.Clear();

  for( int i=0; i < Loops.Count(); i++ )  {
    Loop = Loops.Object(i);
    LT = &Loop->Table();
    if( LT->ColCount() < TD->ItemCount() )  continue;
    defcnt = 0;
    for( int j=0; j < LT->ColCount(); j++ )  {
      if( TD->FindItemCI(LT->ColName(j)) != NULL )
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
  for( int i=0; i < LT->RowCount(); i++ )  {
    AddRow = true;
    for( int j=0; j < LT->ColCount(); j++ )  {
      DI = TD->FindItemCI( LT->ColName(j) );

      if( sindex >=0 && LT->ColName(j).IndexOf("site_symmetry") != -1 )  {
        // 1_555
        if( (*LT)[i][j] != '.' )  {
          olxstr tmp = LT->ColName(j).SubStringFrom( LT->ColName(j).LastIndexOf('_')+1 );
          if( !tmp.IsNumber() )
            continue;
          Tmp = "label_";
          Tmp << tmp;
          SymmMatr = TSymmParser::SymmCodeToMatrix(AllSymmList, (*LT)[i][j] );
          int matIndex = SymmList.IndexOf( SymmMatr );
          if( matIndex == -1 )  {
            SymmList.AddCCopy( SymmMatr );
            matIndex = SymmList.Count()-1;
          }
          for( int k=0; k < LT->ColCount(); k++ )  {
            if( LT->ColName(k).EndsWith( Tmp ) )  {
              Table[i].String(k) << "<sup>" << (matIndex+1) << "</sup>";
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
      if( !Tmp.IsEmpty() && (Toks.CIIndexOf(Val)==-1) ) // equal to
      {  AddRow = false;  break;  }

      Tmp = DI->GetFieldValue("atypeequal", EmptyString);
      if( !Tmp.IsEmpty() )  {  // check for atom type equals to
        TCifLoopData* CD = (TCifLoopData*)(*LT)[i].Object(j);
        if( CD != NULL && CD->CA != NULL )
          if( CD->CA->GetAtomInfo().GetSymbol().Comparei( Tmp ) )  {
            AddRow = false;
            break;
          }
      }
      Tmp = DI->GetFieldValue("atypenotequal", EmptyString);
      if( !Tmp.IsEmpty() )  {  // check for atom type equals to
        TCifLoopData* CD = (TCifLoopData*)(*LT)[i].Object(j);
        if( CD != NULL && CD->CA != NULL )
          if( !CD->CA->GetAtomInfo().GetSymbol().Comparei( Tmp ) )  {
            AddRow = false;
            break;
          }
      }

      Tmp = DI->GetFieldValue("mustnotequal", EmptyString);
      Toks.Clear();
      Toks.Strtok(Tmp, ';');
      if( Tmp.Length() && ( Toks.CIIndexOf(Val)!=-1) ) // not equal to
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
  for( int i=0; i < LT->ColCount(); i++ )  {
    DI = TD->FindItemCI( LT->ColName(i) );
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


