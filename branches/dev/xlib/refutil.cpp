#include "refutil.h"
#include "xapp.h"
#include "unitcell.h"

DefineFSFactory(IRef_analysis, Ref_analysis)

//...........................................................................................
RefUtil::RefUtil()  {
  analyser = NULL;
  Centrosymmetric = false;
  TXApp& xapp = TXApp::GetInstance();
  TSpaceGroup* sg = NULL;
  try  { sg = &xapp.XFile().GetLastLoaderSG();  }
  catch(...)  {  }
  if( sg != NULL )  {
    Centrosymmetric = sg->IsCentrosymmetric();
    analyser = fs_factory_IRef_analysis(sg->GetName());
  }
  if( analyser == NULL )  {
    const TUnitCell& uc = xapp.XFile().GetUnitCell();
    mat3i mI;
    mI.I() *= -1;
    for( int i=1; i < uc.MatrixCount(); i++ )  {
      matrices.AddCCopy( uc.GetMatrix(i) );
      if( !Centrosymmetric && uc.GetMatrix(i).r == mI )
        Centrosymmetric = true;
    }
  }
}
//...........................................................................................


