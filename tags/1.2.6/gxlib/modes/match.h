/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_MATCH_MODE_H
#define __OLX_MATCH_MODE_H
#include "glconsole.h"

class TMatchMode : public AMode  {
  TXAtomPList AtomsToMatch;
protected:
  static void TransformAtoms(TSAtomPList& atoms, const mat3d& rm, const vec3d& origin)  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      atoms[i]->crd() = (rm*atoms[i]->crd()) - origin;
      if( atoms[i]->GetEllipsoid() != NULL )
        atoms[i]->GetEllipsoid()->Mult(rm);
    }
  }
public:
  TMatchMode(size_t id) : AMode(id)  {}
  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    AtomsToMatch.Clear();
    SetUserCursor(0, "<M>");
    return true;
  }
  void Finalise_()  {}
  virtual bool OnObject_(AGDrawObject& obj) {
    if (EsdlInstanceOf(obj, TXAtom) && AtomsToMatch.Count() < 7) {
      if (!AtomsToMatch.AddUnique((TXAtom&)obj)) {
        return false;
      }
      FitAtoms(AtomsToMatch, true);
      SetUserCursor(AtomsToMatch.Count(), "<M>");
      return true;
    }
    return false;
  }
  virtual void OnGraphicsDestroy()  {
    AtomsToMatch.Clear();
    SetUserCursor(0, "<M>");
  }
  virtual bool OnKey_(int keyId, short shiftState)  {
    if( shiftState == 0 && keyId == OLX_KEY_ESCAPE )  {
      if( AtomsToMatch.IsEmpty() )  return false;
      AtomsToMatch.Delete(AtomsToMatch.Count()-1);
      SetUserCursor(AtomsToMatch.Count(), "<M>");
      return true;
    }
    return false;
  }
  static void FitAtoms(TXAtomPList& AtomsToMatch, bool group);
};

void TMatchMode::FitAtoms(TXAtomPList& AtomsToMatch, bool group){
  if( (AtomsToMatch.Count() % 2) != 0 || AtomsToMatch.Count() < 2 )  return;
  TNetwork* netA = NULL, *netB = NULL;
  TSAtomPList atomsA, atomsB;
  if( AtomsToMatch.Count() >= 2 )  {
    netA = &AtomsToMatch[0]->GetNetwork();
    netB = &AtomsToMatch[1]->GetNetwork();
    if( netA->GetLattice() != netB->GetLattice() )  {  //match lattices
      atomsB.SetCapacity(netB->GetLattice().GetObjects().atoms.Count());
      atomsB.AddList(netB->GetLattice().GetObjects().atoms);
      atomsA.SetCapacity(netA->GetLattice().GetObjects().atoms.Count());
      atomsA.AddList(netA->GetLattice().GetObjects().atoms);
    }
    else  {
      atomsB.SetCapacity(netB->NodeCount());
      for( size_t i=0; i < netB->NodeCount(); i++ )
        atomsB.Add(netB->Node(i));
      atomsA.SetCapacity(netA->NodeCount());
      for( size_t i=0; i < netA->NodeCount(); i++ )
        atomsA.Add(netA->Node(i));
    }
  }
  if( netA == netB )  {
    TBasicApp::NewLogEntry(logError) << "Atoms belong to the same fragment";
    AtomsToMatch.Clear();
    return;
  }
  if( &netA->GetLattice() == &netB->GetLattice() )  {
    for( size_t i=2; i < AtomsToMatch.Count(); i+=2 )  {
      if( AtomsToMatch[i]->GetNetwork() != netA ||
        AtomsToMatch[i+1]->GetNetwork() != netB )
      {
        if( AtomsToMatch[i]->GetNetwork() == netB &&
          AtomsToMatch[i+1]->GetNetwork() == netA )
        {
          AtomsToMatch.Swap(i, i+1);
        }
        else  {
          AtomsToMatch.Clear();
          return;
        }
      }
    }
  }
  else  {
    for( size_t i=2; i < AtomsToMatch.Count(); i+=2 )  {
      if( AtomsToMatch[i]->GetNetwork().GetLattice() != netA->GetLattice() ||
        AtomsToMatch[i+1]->GetNetwork().GetLattice() != netB->GetLattice() )
      {
        if( AtomsToMatch[i]->GetNetwork().GetLattice() == netB->GetLattice() &&
          AtomsToMatch[i+1]->GetNetwork().GetLattice() == netA->GetLattice() )
        {
          AtomsToMatch.Swap(i, i+1);
        }
        else  {
          AtomsToMatch.Clear();
          return;
        }
      }
    }
  }

  if( AtomsToMatch.Count() == 2 )  {
    vec3d shift(AtomsToMatch[1]->crd()-AtomsToMatch[0]->crd());
    for( size_t i=0; i < atomsA.Count(); i++ )
      atomsA[i]->crd() += shift;
    TNetPList na;
    TNetPList nb;
    if( netA->GetLattice() != netB->GetLattice() )  {
      for( size_t i=0; i < netA->GetLattice().FragmentCount(); i++ )
        na.Add(netA->GetLattice().GetFragment(i));
      for( size_t i=0; i < netB->GetLattice().FragmentCount(); i++ )
        nb.Add(netB->GetLattice().GetFragment(i));
    }
    else  {
      na.Add(netA);
      nb.Add(netB);
    }
    if( group )  {
      TGXApp &gxapp = TGXApp::GetInstance();
      TGlGroup& ga = gxapp.GroupFragments(na, "FragmentA");
      TGlGroup& gb = gxapp.GroupFragments(nb, "FragmentB");
      gxapp.SelectAll(false);
      if( &ga != NULL && ga.IsDefaultColor() )
        ga.SetGlM(TGlMaterial("85;0.000,1.000,0.000,0.000;4144959;1.000,1.000,1.000,0.500;36"));
      if( &gb != NULL && gb.IsDefaultColor() )
        gb.SetGlM(TGlMaterial("85;1.000,0.000,0.000,0.000;4144959;1.000,1.000,1.000,0.500;36"));
    }
  }
  else if( AtomsToMatch.Count() == 4 )  {
    vec3d orgn = AtomsToMatch[1]->crd();
    vec3d vec1 = AtomsToMatch[3]->crd() - orgn;
    vec3d vec2 = AtomsToMatch[2]->crd() - orgn;
    vec3d rv = vec1.XProdVec(vec2).Normalise();
    mat3d rm;
    olx_create_rotation_matrix(rm, rv, vec1.CAngle(vec2));
    TransformAtoms(atomsA, rm, rm*AtomsToMatch[0]->crd()-orgn);
  }
  else if( AtomsToMatch.Count() == 6 )  {
    vec3d rv((AtomsToMatch[1]->crd() - AtomsToMatch[3]->crd()).Normalise());
    vec3d v1((AtomsToMatch[5]->crd() - AtomsToMatch[3]->crd()));
    vec3d v2((AtomsToMatch[4]->crd() - AtomsToMatch[3]->crd()));
    v1 = rv.Normal(v1);
    v2 = rv.Normal(v2);
    // replacing the rotation vector for the one with correct orientation
    rv = v1.XProdVec(v2).Normalise();
    mat3d rm;
    olx_create_rotation_matrix(rm, rv, v1.CAngle(v2));
    TransformAtoms(atomsA, rm, rm*AtomsToMatch[0]->crd()-AtomsToMatch[1]->crd());
  }
  else  {
    TArrayList<olx_pair_t<TSAtom*,TSAtom*> > atoms(AtomsToMatch.Count()/2);
    for( size_t i=0; i < AtomsToMatch.Count(); i+=2 )  {
      atoms[i/2].a = AtomsToMatch[i];
      atoms[i/2].b = AtomsToMatch[i+1];
    }
    align::out ao = TNetwork::GetAlignmentInfo(atoms, false, TSAtom::weight_z);
    mat3d m;
    QuaternionToMatrix(ao.quaternions[0], m);
    for( size_t i=0; i < atomsA.Count(); i++ )  {
      atomsA[i]->crd() = (atomsA[i]->crd()-ao.center_a)*m + ao.center_b;
      if( atomsA[i]->GetEllipsoid() != NULL )
        atomsA[i]->GetEllipsoid()->Mult(m);
    }
    //TNetwork::DoAlignAtoms(atomsA, ao);
  }
  TGXApp::GetInstance().UpdateBonds();
}

#endif
