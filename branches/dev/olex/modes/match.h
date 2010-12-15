#ifndef __OLX_MATCH_MODE_H
#define __OLX_MATCH_MODE_H


class TMatchMode : public AMode  {
  TXAtomPList AtomsToMatch;
protected:
  static void TransformAtoms(TSAtomPList& atoms, const mat3d& rm, const vec3d& origin)  {
    for( size_t i=0; i < atoms.Count(); i++ )  {
      atoms[i]->crd() = (rm*atoms[i]->crd()) - origin;
      if( atoms[i]->GetEllipsoid() != NULL )
        atoms[i]->GetEllipsoid()->MultMatrix(rm);
    }
  }
public:
  TMatchMode(size_t id) : AMode(id)  {}
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor('0', "<M>");
    return true;
  }
  void Finalise()  {}
  virtual bool OnObject(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXAtom) && AtomsToMatch.Count() < 7 )  {
      AtomsToMatch.Add((TXAtom&)obj);
      TGlXApp::GetMainForm()->SetUserCursor(AtomsToMatch.Count(), "<M>");
      FitAtoms(AtomsToMatch, "<M>", true);
      return true;
    }
    return false;
  }
  virtual void OnGraphicsDestroy()  {
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor('0', "<M>");
  }
  virtual bool OnKey(int keyId, short shiftState)  {
    if( shiftState == 0 && keyId == WXK_ESCAPE )  {
      if( AtomsToMatch.IsEmpty() )  return false;
      AtomsToMatch.Delete(AtomsToMatch.Count()-1);
      TGlXApp::GetMainForm()->SetUserCursor(AtomsToMatch.Count(), "<M>");
      return true;
    }
    return false;
  }
  static void FitAtoms(TXAtomPList& AtomsToMatch, const olxstr& cursor_name, bool group);
};

void TMatchMode::FitAtoms(TXAtomPList& AtomsToMatch, const olxstr& cursor_name, bool group)  {
  if( (AtomsToMatch.Count() % 2) != 0 || AtomsToMatch.Count() < 2 )  return;
  TNetwork* netA = NULL, *netB = NULL;
  TSAtomPList atomsA, atomsB;
  if( AtomsToMatch.Count() >= 2 )  {
    netA = &AtomsToMatch[0]->Atom().GetNetwork();
    netB = &AtomsToMatch[1]->Atom().GetNetwork();
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
    TGlXApp::GetMainForm()->SetUserCursor(AtomsToMatch.Count(), cursor_name);
    return;
  }
  if( &netA->GetLattice() == &netB->GetLattice() )  {
    for( size_t i=2; i < AtomsToMatch.Count(); i+=2 )  {
      if( AtomsToMatch[i]->Atom().GetNetwork() != netA ||
          AtomsToMatch[i+1]->Atom().GetNetwork() != netB )
      {
        if( AtomsToMatch[i]->Atom().GetNetwork() == netB &&
            AtomsToMatch[i+1]->Atom().GetNetwork() == netA )
        {
          AtomsToMatch.Swap(i, i+1);
        }
        else  {
          AtomsToMatch.Clear();
          TGlXApp::GetMainForm()->SetUserCursor(AtomsToMatch.Count(), cursor_name);
          return;
        }
      }
    }
  }
  else  {
    for( size_t i=2; i < AtomsToMatch.Count(); i+=2 )  {
      if( AtomsToMatch[i]->Atom().GetNetwork().GetLattice() != netA->GetLattice() ||
          AtomsToMatch[i+1]->Atom().GetNetwork().GetLattice() != netB->GetLattice() )
      {
        if( AtomsToMatch[i]->Atom().GetNetwork().GetLattice() == netB->GetLattice() &&
            AtomsToMatch[i+1]->Atom().GetNetwork().GetLattice() == netA->GetLattice() )
        {
          AtomsToMatch.Swap(i, i+1);
        }
        else  {
          AtomsToMatch.Clear();
          TGlXApp::GetMainForm()->SetUserCursor(AtomsToMatch.Count(), cursor_name);
          return;
        }
      }
    }
  }

  if( AtomsToMatch.Count() == 2 )  {
    vec3d shift(AtomsToMatch[1]->Atom().crd()-AtomsToMatch[0]->Atom().crd());
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
      TGlGroup& ga = TGlXApp::GetGXApp()->GroupFragments(na, "FragmentA");
      TGlGroup& gb = TGlXApp::GetGXApp()->GroupFragments(nb, "FragmentB");
      TGlXApp::GetGXApp()->SelectAll(false);
      if( &ga != NULL && ga.IsDefaultColor() )
        ga.SetGlM(TGlMaterial("85;0.000,1.000,0.000,0.000;4144959;1.000,1.000,1.000,0.500;36"));
      if( &gb != NULL && gb.IsDefaultColor() )
        gb.SetGlM(TGlMaterial("85;1.000,0.000,0.000,0.000;4144959;1.000,1.000,1.000,0.500;36"));
    }
  }
  else if( AtomsToMatch.Count() == 4 )  {
    vec3d orgn = AtomsToMatch[1]->Atom().crd();
    vec3d vec1 = AtomsToMatch[3]->Atom().crd() - orgn;
    vec3d vec2 = AtomsToMatch[2]->Atom().crd() - orgn;
    vec3d rv = vec1.XProdVec(vec2).Normalise();
    mat3d rm;
    olx_create_rotation_matrix(rm, rv, vec1.CAngle(vec2));
    TransformAtoms(atomsA, rm, rm*AtomsToMatch[0]->Atom().crd()-orgn);
  }
  else if( AtomsToMatch.Count() == 6 )  {
    vec3d rv((AtomsToMatch[1]->Atom().crd() - AtomsToMatch[3]->Atom().crd()).Normalise());
    vec3d v1((AtomsToMatch[5]->Atom().crd() - AtomsToMatch[3]->Atom().crd()));
    vec3d v2((AtomsToMatch[4]->Atom().crd() - AtomsToMatch[3]->Atom().crd()));
    v1 = rv.Normal(v1);
    v2 = rv.Normal(v2);
    rv = v1.XProdVec(v2).Normalise();  // replacing the rotation vector for the one with correct orientation
    mat3d rm;
    olx_create_rotation_matrix(rm, rv, v1.CAngle(v2));
    TransformAtoms(atomsA, rm, rm*AtomsToMatch[0]->Atom().crd()-AtomsToMatch[1]->Atom().crd());
  }
  else  {
    TArrayList<AnAssociation2<TSAtom*,TSAtom*> > atoms(AtomsToMatch.Count()/2);
    for( size_t i=0; i < AtomsToMatch.Count(); i+=2 )  {
      atoms[i/2].A() = &AtomsToMatch[i]->Atom();
      atoms[i/2].B() = &AtomsToMatch[i+1]->Atom();
    }
    align::out ao = TNetwork::GetAlignmentInfo(atoms, false, TSAtom::weight_z);
    mat3d m;
    QuaternionToMatrix(ao.quaternions[0], m);
    for( size_t i=0; i < atomsA.Count(); i++ )  {
      atomsA[i]->crd() = (atomsA[i]->crd()-ao.center_a)*m + ao.center_b;
      if( atomsA[i]->GetEllipsoid() != NULL )
        atomsA[i]->GetEllipsoid()->MultMatrix(m);
    }
    //TNetwork::DoAlignAtoms(atomsA, ao);
  }
  TGlXApp::GetGXApp()->UpdateBonds();
}

#endif
