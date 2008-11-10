#include "vcov.h"

void VcoVMatrix::ReadShelxMat(const olxstr& fileName, TAsymmUnit& au)  {
  Clear();
  olxstr lstFN( TEFile::ChangeFileExt(fileName, "lst") );
  if( TEFile::FileExists(lstFN) && TEFile::FileExists(fileName) )  {
    time_t lst_fa = TEFile::FileAge(lstFN);
    time_t mat_fa = TEFile::FileAge(fileName);
    if( lst_fa > mat_fa && (lst_fa-mat_fa) > 5 )
      TBasicApp::GetLog() << "The mat file is possibly out of date\n";
  }
  TCStrList sl, toks;
  const CString sof("sof");
  sl.LoadFromFile(fileName);
  if( sl.Count() < 10 )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file content");
  toks.Strtok(sl[3], ' ');
  int cnt = toks[0].ToInt();
  TIntList indexes;
  TDoubleList diag;
  if( cnt <= 0 || sl.Count() < cnt+11 )  
    throw TFunctionFailedException(__OlxSourceInfo, "empty/invalid matrix file");
  for( int i=1; i < cnt; i++ )  { // skipp OSF
    toks.Clear();
    toks.Strtok(sl[i+7], ' ');
    if( toks[0].ToInt() != i+1 || toks.Count() != 6 )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid matrix file");
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
  int vcov_cnt = 0;
  for( int i=0; i < sl.Count(); i++ )  {
    const int ind = i+cnt+10;
    if( sl[ind].IsEmpty() )  break;
    const int ll = sl[ind].Length();
    int s_ind = 0;
    while( s_ind < ll )  {
      all_vcov[vcov_cnt++] = sl[ind].SubString(s_ind, 8).ToDouble();
      s_ind += 8;
    }
  }
  TIntList x_ind(cnt);
  x_ind[0] = 0;
  for( int i=1; i < cnt ; i++ )
    x_ind[i] = cnt + 1 - i + x_ind[i-1];

  Allocate(diag.Count());

  for( int i=0; i < indexes.Count(); i++ )  {
    for( int j=0; j <= i; j++ )  {
      if( i == j )  
        data[i][j] = diag[i]*diag[i];
      else  {  // top diagonal to bottom diagonal
        const int ind = indexes[i] <= indexes[j] ? x_ind[indexes[i]] + indexes[j]-indexes[i] :
          x_ind[indexes[j]]  + indexes[i]-indexes[j];
        data[i][j] = all_vcov[ind]*diag[i]*diag[j];  
      }
    }
  }
  for( int i=0; i < Index.Count(); i++ )  {
    TCAtom* ca = au.FindCAtom(Index[i].GetA());
    if( ca == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "matrix is not upto date");
    Index[i].C() = ca->GetLoaderId();
    int j = i;
    while( ++j < Index.Count() && Index[i].GetA().Comparei(Index[j].GetA()) == 0 )
      Index[j].C() = ca->GetLoaderId();
    i = j-1;
  }
}

double VcoVMatrix::Find(const olxstr& atom, const short va, const short vb) const {
  for( int i=0; i < Index.Count(); i++ )  {
    if( Index[i].GetA() == atom )  {
      int i1 = -1, i2 = -1;
      for( int j=i; j < Index.Count() && Index[j].GetA() == atom; j++ )  {
        if( Index[j].GetB() == va )  i1 = j;
        if( Index[j].GetB() == vb )  i2 = j;
      }
      if( i1 == -1 || i2 == -1 )  return 0;
      return (i1 <= i2 ) ? data[i2][i1] : data[i1][i2]; 
    }
  }
  return 0;
}

