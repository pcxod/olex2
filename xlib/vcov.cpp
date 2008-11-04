#include "vcov.h"

void VcoVMatrix::ReadShelxMat(const olxstr& fileName, TAsymmUnit& au)  {
  Clear();
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
        data[i][j] = diag[i];
      else  {  // top diagonal to bottom diagonal
        const int ind = indexes[i] <= indexes[j] ? x_ind[indexes[i]] + indexes[j]-indexes[i] :
          x_ind[indexes[j]]  + indexes[i]-indexes[j];
        data[i][j] = all_vcov[ind];  
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
void VcoVMatrix::FindVcoV(const TCAtom& a1, const TCAtom& a2, ematd& m) const {
  m.Resize(4,4);
  m.Null();
  int ind1 = FindAtomIndex(a1),
      ind2 = FindAtomIndex(a2);
  int indexes1[4] = {-1,-1,-1,-1}, 
      indexes2[4] = {-1,-1,-1,-1};
  if( (ind1|ind2) == -1 )
    throw TFunctionFailedException(__OlxSourceInfo, "unable to located provided atoms");
  for( int i=ind1; i < Index.Count() && Index[i].GetC() == a1.GetLoaderId(); i++ )  {
    if( Index[i].GetB() == vcoviX )
      indexes1[0] = i;
    else if( Index[i].GetB() == vcoviY )
      indexes1[1] = i;
    else if( Index[i].GetB() == vcoviZ )
      indexes1[2] = i;
    else if( Index[i].GetB() == vcoviO )
      indexes1[3] = i;
  }
  for( int i=ind2; i < Index.Count() && Index[i].GetC() == a2.GetLoaderId(); i++ )  {
    if( Index[i].GetB() == vcoviX )
      indexes2[0] = i;
    else if( Index[i].GetB() == vcoviY )
      indexes2[1] = i;
    else if( Index[i].GetB() == vcoviZ )
      indexes1[2] = i;
    else if( Index[i].GetB() == vcoviO )
      indexes2[3] = i;
  }
  for( int i=0; i < 4; i++ )  {
    if( indexes1[i] == -1 ) continue;
    for( int j=i; j < 4; j++ )  {
      if( indexes2[j] == -1 )  continue;
      m[i][j] = Get(indexes1[i], indexes2[j]);
      m[j][i] = m[i][j];
    }
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

