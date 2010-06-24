#include "vcov.h"
#include "refmodel.h"

void VcoVMatrix::ReadShelxMat(const olxstr& fileName, TAsymmUnit& au)  {
  Clear();
  olxstr lstFN = TEFile::ChangeFileExt(fileName, "lst");
  if( TEFile::Exists(lstFN) && TEFile::Exists(fileName) )  {
    time_t lst_fa = TEFile::FileAge(lstFN);
    time_t mat_fa = TEFile::FileAge(fileName);
    if( lst_fa > mat_fa && (lst_fa-mat_fa) > 5 )
      TBasicApp::GetLog() << "The mat file is possibly out of date\n";
  }
  TCStrList sl, toks;
  sl.LoadFromFile(fileName);
  if( sl.Count() < 10 )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file content");
  toks.Strtok(sl[3], ' ');
  size_t cnt = toks[0].ToSizeT();
  TSizeList indexes;
  TDoubleList diag;
  if( cnt == 0 || sl.Count() < cnt+11 )  
    throw TFunctionFailedException(__OlxSourceInfo, "empty/invalid matrix file");
  olxstr last_atom_name;
  TCAtom* atom;
  for( size_t i=1; i < cnt; i++ )  { // skipp OSF
    toks.Clear();
    toks.Strtok(sl[i+7], ' ');
    if( toks[0].ToInt() != i+1 || toks.Count() != 6 )  {
      if( toks.Count() == 5 )
        continue;
      throw TFunctionFailedException(__OlxSourceInfo, "invalid matrix file");
    }
    if( toks[4].Equals("BASF") )  continue;
    if( toks[4].StartsFrom("FVAR") )  {
      if( toks.Count() != 6 )  continue;
      size_t var_ind = toks[5].ToSizeT();
      if( au.GetRefMod()->Vars.VarCount() < var_ind ) 
        throw TFunctionFailedException(__OlxSourceInfo, "model does not relate to the matrix file");
      XVar& var = au.GetRefMod()->Vars.GetVar(var_ind-1);
      const double esd = toks[2].ToDouble();
      for( size_t j=0; j < var.RefCount(); j++ )  {
        XVarReference& r = var.GetRef(j);
        if( !EsdlInstanceOf(r.referencer, TCAtom) )  continue;
        TCAtom& ca = (TCAtom&)r.referencer;
        if( r.var_index == catom_var_name_Sof )
          ca.SetOccuEsd(esd);
        else if( r.var_index == catom_var_name_Uiso )
          ca.SetUisoEsd(esd);
      }
      continue;
    }
    if( last_atom_name != toks[5] )  {
      atom = au.FindCAtom(toks[5]);
      last_atom_name = toks[5];
    }
    if( atom == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "mismatching matrix file");
    if( toks[4].CharAt(0) == 'x' )  {
      diag.Add(toks[2].ToDouble());
      atom->ccrdEsd()[0] = diag.Last();
      Index.AddNew(toks[5], vcoviX, -1);
      indexes.Add(i);
    }
    else if( toks[4].CharAt(0) == 'y' )  {
      diag.Add(toks[2].ToDouble());
      atom->ccrdEsd()[1] = diag.Last();
      Index.AddNew(toks[5], vcoviY, -1);
      indexes.Add(i);
    }
    else if( toks[4].CharAt(0) == 'z' )  {
      diag.Add(toks[2].ToDouble());
      atom->ccrdEsd()[2] = diag.Last();
      Index.AddNew(toks[5], vcoviZ, -1);
      indexes.Add(i);
    }
    else if( toks[4] == "sof" )  {
      diag.Add(toks[2].ToDouble());
      atom->SetOccuEsd(diag.Last());
      Index.AddNew(toks[5], vcoviO , -1);
      indexes.Add(i);
    }
    else if( toks[4] == "U11" )  {
      if( atom->GetEllipsoid() != NULL )
        atom->GetEllipsoid()->SetEsd(0, toks[2].ToDouble());
      else
        atom->SetUisoEsd(toks[2].ToDouble());
    }
    else if( toks[4] == "U22" )
      atom->GetEllipsoid()->SetEsd(1, toks[2].ToDouble());
    else if( toks[4] == "U33" )
      atom->GetEllipsoid()->SetEsd(2, toks[2].ToDouble());
    else if( toks[4] == "U23" )
      atom->GetEllipsoid()->SetEsd(3, toks[2].ToDouble());
    else if( toks[4] == "U13" )
      atom->GetEllipsoid()->SetEsd(4, toks[2].ToDouble());
    else if( toks[4] == "U12" )
      atom->GetEllipsoid()->SetEsd(5, toks[2].ToDouble());
  }
  TDoubleList all_vcov((cnt+1)*cnt/2);
  size_t vcov_cnt = 0;
  for( size_t i=0; i < sl.Count(); i++ )  {
    const size_t ind = i+cnt+10;
    if( sl[ind].IsEmpty() )  break;
    const size_t ll = sl[ind].Length();
    size_t s_ind = 0;
    while( s_ind < ll )  {
      all_vcov[vcov_cnt++] = sl[ind].SubString(s_ind, 8).ToDouble();
      s_ind += 8;
    }
  }
  TSizeList x_ind(cnt);
  x_ind[0] = 0;
  for( size_t i=1; i < cnt ; i++ )
    x_ind[i] = cnt + 1 - i + x_ind[i-1];

  Allocate(diag.Count());

  for( size_t i=0; i < indexes.Count(); i++ )  {
    for( size_t j=0; j <= i; j++ )  {
      if( i == j )  
        data[i][j] = diag[i]*diag[i];
      else  {  // top diagonal to bottom diagonal
        const size_t ind = indexes[i] <= indexes[j] ? x_ind[indexes[i]] + indexes[j]-indexes[i] :
          x_ind[indexes[j]]  + indexes[i]-indexes[j];
        data[i][j] = all_vcov[ind]*diag[i]*diag[j];  
      }
    }
  }
  for( size_t i=0; i < Index.Count(); i++ )  {
    TCAtom* ca = au.FindCAtom(Index[i].GetA());
    if( ca == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "matrix is not upto date");
    Index[i].C() = ca->GetId();
    size_t j = i;
    while( ++j < Index.Count() && Index[i].GetA().Equalsi(Index[j].GetA()) )
      Index[j].C() = ca->GetId();
    i = j-1;
  }
}
//..................................................................................
double VcoVMatrix::Find(const olxstr& atom, const short va, const short vb) const {
  for( size_t i=0; i < Index.Count(); i++ )  {
    if( Index[i].GetA() == atom )  {
      size_t i1 = InvalidIndex, i2 = InvalidIndex;
      for( size_t j=i; j < Index.Count() && Index[j].GetA() == atom; j++ )  {
        if( Index[j].GetB() == va )  i1 = j;
        if( Index[j].GetB() == vb )  i2 = j;
      }
      if( i1 == InvalidIndex || i2 == InvalidIndex )
        return 0;
      return (i1 <= i2 ) ? data[i2][i1] : data[i1][i2]; 
    }
  }
  return 0;
}
//..................................................................................
void VcoVMatrix::ReadSmtbxMat(const olxstr& fileName, TAsymmUnit& au)  {
  TStrList in;
  in.LoadFromFile(fileName);
  if( in.Count() != 3 || !in[0].Equals("VCOV") )
    throw TInvalidArgumentException(__OlxSourceInfo, "file format");
  TStrList annotations(in[1], ' '),
    values(in[2], ' ');
  if( ((annotations.Count()*(annotations.Count()+1)))/2 != values.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "inconsistent matrix and annotations");

  olxstr last_atom_name;
  TSizeList indexes;
  TDoubleList diag;
  TCAtom* atom = NULL;
  size_t d_index = 0;
  for( size_t i=0; i < annotations.Count(); i++ )  {
    if( i !=  0 )
      d_index += (annotations.Count()-i);
    const size_t di = annotations[i].IndexOf('.');
    if( di == InvalidIndex )
      throw TInvalidArgumentException(__OlxSourceInfo, "annotation");
    const olxstr atom_name = annotations[i].SubStringTo(di);
    const olxstr param_name = annotations[i].SubStringFrom(di+1);
    if( last_atom_name != atom_name )  {
      atom = au.FindCAtom(atom_name);
      last_atom_name = atom_name;
    }
    if( atom == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "mismatching matrix file");
    if( param_name == 'x' )  {
      diag.Add(values[d_index].ToDouble());
      atom->ccrdEsd()[0] = diag.Last();
      Index.AddNew(atom_name, vcoviX, -1);
      indexes.Add(i);
    }
    else if( param_name == 'y' )  {
      diag.Add(values[d_index].ToDouble());
      atom->ccrdEsd()[1] = diag.Last();
      Index.AddNew(atom_name, vcoviY, -1);
      indexes.Add(i);
    }
    else if( param_name == 'z' )  {
      diag.Add(values[d_index].ToDouble());
      atom->ccrdEsd()[2] = diag.Last();
      Index.AddNew(atom_name, vcoviZ, -1);
      indexes.Add(i);
    }
    else if( param_name == "uiso" )  {
      diag.Add(values[d_index].ToDouble());
      atom->SetOccuEsd(diag.Last());
      Index.AddNew(atom_name, vcoviO , -1);
      indexes.Add(i);
    }
    else if( param_name == "u11" )  {
      if( atom->GetEllipsoid() != NULL )
        atom->GetEllipsoid()->SetEsd(0, values[d_index].ToDouble());
      else
        atom->SetUisoEsd(values[d_index].ToDouble());
    }
    else if( param_name == "u22" )
      atom->GetEllipsoid()->SetEsd(1, values[d_index].ToDouble());
    else if( param_name == "u33" )
      atom->GetEllipsoid()->SetEsd(2, values[d_index].ToDouble());
    else if( param_name == "u23" )
      atom->GetEllipsoid()->SetEsd(3, values[d_index].ToDouble());
    else if( param_name == "u13" )
      atom->GetEllipsoid()->SetEsd(4, values[d_index].ToDouble());
    else if( param_name == "u12" )
      atom->GetEllipsoid()->SetEsd(5, values[d_index].ToDouble());
  }

  TSizeList x_ind(annotations.Count());
  x_ind[0] = 0;
  for( size_t i=1; i < x_ind.Count() ; i++ )
    x_ind[i] = x_ind.Count() + 1 - i + x_ind[i-1];

  Allocate(diag.Count());

  for( size_t i=0; i < indexes.Count(); i++ )  {
    for( size_t j=0; j <= i; j++ )  {
      const size_t ind = indexes[i] <= indexes[j] ? x_ind[indexes[i]] + indexes[j]-indexes[i] :
        x_ind[indexes[j]]  + indexes[i]-indexes[j];
      data[i][j] = values[ind].ToDouble();
    }
  }
  for( size_t i=0; i < Index.Count(); i++ )  {
    TCAtom* ca = au.FindCAtom(Index[i].GetA());
    Index[i].C() = ca->GetId();
    size_t j = i;
    while( ++j < Index.Count() && Index[i].GetA().Equalsi(Index[j].GetA()) )
      Index[j].C() = ca->GetId();
    i = j-1;
  }

}
//..................................................................................
