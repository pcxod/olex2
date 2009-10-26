#include "vcov.h"

void VcoVMatrix::ReadShelxMat(const olxstr& fileName, TAsymmUnit& au)  {
  Clear();
  olxstr lstFN( TEFile::ChangeFileExt(fileName, "lst") );
  if( TEFile::Exists(lstFN) && TEFile::Exists(fileName) )  {
    time_t lst_fa = TEFile::FileAge(lstFN);
    time_t mat_fa = TEFile::FileAge(fileName);
    if( lst_fa > mat_fa && (lst_fa-mat_fa) > 5 )
      TBasicApp::GetLog() << "The mat file is possibly out of date\n";
  }
  TCStrList sl, toks;
  const olxcstr sof("sof");
  sl.LoadFromFile(fileName);
  if( sl.Count() < 10 )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file content");
  toks.Strtok(sl[3], ' ');
  size_t cnt = toks[0].ToSizeT();
  TSizeList indexes;
  TDoubleList diag;
  if( cnt == 0 || sl.Count() < cnt+11 )  
    throw TFunctionFailedException(__OlxSourceInfo, "empty/invalid matrix file");
  for( size_t i=1; i < cnt; i++ )  { // skipp OSF
    toks.Clear();
    toks.Strtok(sl[i+7], ' ');
    if( toks[0].ToInt() != i+1 || toks.Count() != 6 )  {
      if( toks.Count() == 5 )
        continue;
      throw TFunctionFailedException(__OlxSourceInfo, "invalid matrix file");
    }
    if( toks[4].CharAt(0) == 'x' )  {
      diag.Add(toks[2].ToDouble());
      Index.AddNew( toks[5], vcoviX, -1 );
      indexes.Add(i);
    }
    else if( toks[4].CharAt(0) == 'y' )  {
      diag.Add(toks[2].ToDouble());
      Index.AddNew( toks[5], vcoviY, -1 );
      indexes.Add(i);
    }
    else if( toks[4].CharAt(0) == 'z' )  {
      diag.Add(toks[2].ToDouble());
      Index.AddNew( toks[5], vcoviZ, -1 );
      indexes.Add(i);
    }
    else if( toks[4] == sof )  {
      diag.Add(toks[2].ToDouble());
      Index.AddNew( toks[5], vcoviO , -1);
      indexes.Add(i);
    }
  }
  TDoubleList all_vcov( (cnt+1)*cnt/2);
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

