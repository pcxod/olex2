#include "conninfo.h"
#include "atomref.h"

void ConnInfo::ProcessFree(RefinementModel& rm, const TStrList& ins)  {
  TAtomReference ar(ins.Text(' '));
  TCAtomGroup ag;
  int aag;
  try  {  ar.Expand(rm, ag, EmptyString, aag);  }
  catch(TExceptionBase& ex)  {
    throw TFunctionFailedException(__OlxSourceInfo, ex, "Failed locate atoms");
  }
  if( ag.Count() != 2 )
    throw TFunctionFailedException(__OlxSourceInfo, "Two atoms are expected for FREE");
  if( ag[0].GetMatrix() != NULL && ag[1].GetMatrix() != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "At maximum one equivalent position is expectd for FREE");
  if( ag[0].GetMatrix() == NULL )
    AddBond( *ag[0].GetAtom(), *ag[1].GetAtom(), ag[1].GetMatrix() );
  else
    AddBond( *ag[1].GetAtom(), *ag[0].GetAtom(), ag[0].GetMatrix() );
}
//........................................................................
void ConnInfo::ProcessBind(RefinementModel& rm, const TStrList& ins)  {
  TAtomReference ar(ins.Text(' '));
  TCAtomGroup ag;
  int aag;
  try  {  ar.Expand(rm, ag, EmptyString, aag);  }
  catch(TExceptionBase& ex)  {
    throw TFunctionFailedException(__OlxSourceInfo, ex, "Failed locate atoms");
  }
  if( ag.Count() != 2 )
    throw TFunctionFailedException(__OlxSourceInfo, "Two atoms are expected for BIND");
  if( ag[0].GetMatrix() != NULL && ag[1].GetMatrix() != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "At maximum one equivalent position is expectd for BIND");
  if( ag[0].GetMatrix() == NULL )
    RemBond( *ag[0].GetAtom(), *ag[1].GetAtom(), ag[1].GetMatrix() );
  else
    RemBond( *ag[1].GetAtom(), *ag[0].GetAtom(), ag[0].GetMatrix() );
}
//........................................................................
