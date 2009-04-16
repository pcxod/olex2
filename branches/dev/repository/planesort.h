#ifndef _olx_plane_sort_H
#define _olx_plane_sort_H
#include "splane.h"

namespace PlaneSort {
  struct Sorter {
    TPSTypeList<double, const vec3d*> sortedPlane;
    Sorter(const TSPlane& sp)  {
      sortedPlane.Add( 0, &sp.GetAtom(0).crd() );
      vec3d org(sp.GetAtom(0).crd()-sp.GetCenter());
      for( int i=1; i < sp.CrdCount(); i++ )  {
        vec3d vec = sp.GetAtom(i).crd() - sp.GetCenter();
        double ca = org.CAngle(vec);
        vec = org.XProdVec(vec);
        // negative - vec is on the right, positive - on the left
        double vo = vec.CAngle(sp.GetNormal());
        if( ca >= 0 )  { // -90 to 90
          if( vo < 0 )  // -90 to 0 3->4
            sortedPlane.Add( 3.0 + ca, &sp.GetAtom(i).crd() );
          else  // 0 to 90 0->1
            sortedPlane.Add( 1.0 - ca, &sp.GetAtom(i).crd() );
        }
        else if( ca > -1 ) {  // 90-270
          if( vo < 0 )  // 180 to 270 2->3
            sortedPlane.Add( 3.0 + ca, &sp.GetAtom(i).crd() );
          else  // 90 to 180 1->2
            sortedPlane.Add( 1.0 - ca, &sp.GetAtom(i).crd() );
        }
        else  {  //-1, special case
          sortedPlane.Add( 2, &sp.GetAtom(i).crd() );
        }
      }
    }
  };

};
#endif
