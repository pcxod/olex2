#ifndef _olx_plane_sort_H
#define _olx_plane_sort_H
#include "splane.h"
#include "edict.h"

namespace PlaneSort {
  struct Sorter {
    TPSTypeList<double, const vec3d*> sortedPlane;
    Sorter(const TSPlane& sp)  {  DoSort(sp);  }
    Sorter() { }
    void DoSort(const TSPlane& sp)  {
      sortedPlane.Add( 0, &sp.GetAtom(0).crd() );
      vec3d org(sp.GetAtom(0).crd()-sp.GetCenter());
      for( int i=1; i < sp.CrdCount(); i++ )  {
        vec3d vec = sp.GetAtom(i).crd() - sp.GetCenter();
        double ca = org.CAngle(vec);
        vec = org.XProdVec(vec);
        // negative - vec is on the right, positive - on the left, if ca == 0, vec == (0,0,0)
        double vo = (ca == -1 ? 0 : vec.CAngle(sp.GetNormal()));
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
    
    static void DoSort(const TSAtomPList& atoms, 
      const olxdict<int, vec3d, TPrimitiveComparator>& transforms, // tag dependent translations
      vec3d& center, const vec3d& normal, TSAtomPList& output)  
    {
      if( atoms.IsEmpty() )
        throw TInvalidArgumentException(__OlxSourceInfo, "atom list");
      TPSTypeList<double, TSAtom*> sortedPlane;
      sortedPlane.Add(0, atoms[0]);
      vec3d org(atoms[0]->crd()+transforms[atoms[0]->GetTag()]-center);
      for( int i=1; i < atoms.Count(); i++ )  {
        vec3d vec = atoms[i]->crd() + transforms[atoms[i]->GetTag()] - center;
        double ca = org.CAngle(vec);
        vec = org.XProdVec(vec);
        // negative - vec is on the right, positive - on the left, if ca == 0, vec == (0,0,0)
        double vo = (ca == -1 ? 0 : vec.CAngle(normal));
        if( ca >= 0 )  { // -90 to 90
          if( vo < 0 )  // -90 to 0 3->4
            sortedPlane.Add( 3.0 + ca, atoms[i] );
          else  // 0 to 90 0->1
            sortedPlane.Add( 1.0 - ca, atoms[i] );
        }
        else if( ca > -1 ) {  // 90-270
          if( vo < 0 )  // 180 to 270 2->3
            sortedPlane.Add( 3.0 + ca, atoms[i] );
          else  // 90 to 180 1->2
            sortedPlane.Add( 1.0 - ca, atoms[i] );
        }
        else  {  //-1, special case
          sortedPlane.Add( 2, atoms[i] );
        }
      }
      output.SetCapacity(atoms.Count());
      for( int i=0; i < atoms.Count(); i++ )
        output.Add(sortedPlane.GetObject(i));
    }
  };
};
#endif
