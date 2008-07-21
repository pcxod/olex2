#ifndef __OLX_MATCH_MODE_H
#define __OLX_MATCH_MODE_H


class TMatchMode : public AMode  {
  TXAtomPList AtomsToMatch;
protected:
  void FitAtoms();
public:
  TMatchMode(int id) : AMode(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor( '0', "<M>");
    return true;
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom &XA = (TXAtom&)obj;
      AtomsToMatch.Add( &XA );
      TGlXApp::GetMainForm()->SetUserCursor( AtomsToMatch.Count(), "<M>");
      FitAtoms();
      return true;
    }
    return false;
  }
  virtual void OnGraphicsDestroy()  {
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor( '0', "<M>");
  }
  virtual bool OnKey(int keyId, short shiftState)  {
    if( shiftState == 0 && keyId == WXK_ESCAPE )  {
      if( AtomsToMatch.IsEmpty() )  return false;
      AtomsToMatch.Delete( AtomsToMatch.Count()-1 );
      TGlXApp::GetMainForm()->SetUserCursor( AtomsToMatch.Count(), "<M>");
      return true;
    }
    return false;
  }
};

void TMatchMode::FitAtoms()  {
  if( (AtomsToMatch.Count() % 2) != 0 ||  AtomsToMatch.Count() < 2 )  return;
  TNetwork* netA = NULL, *netB = NULL;
  TSAtomPList atomsB;
  if( AtomsToMatch.Count() >= 2 )  {
    netA = &AtomsToMatch[0]->Atom().GetNetwork();
    netB = &AtomsToMatch[1]->Atom().GetNetwork();
    if( netA->GetLattice() != netB->GetLattice() )  {  //match lattices
      atomsB.SetCapacity( netB->GetLattice().AtomCount() );
      for( int i=0; i < netB->GetLattice().AtomCount(); i++ )
        atomsB.Add( &netB->GetLattice().GetAtom(i) );
    }
    else  {
      atomsB.SetCapacity( netB->NodeCount() );
      for( int i=0; i < netB->NodeCount(); i++ )
        atomsB.Add( &netB->Node(i) );
    }
  }
  if( netA == netB )  {
    TBasicApp::GetLog().Error("Atoms belong to the same fragment");
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor( AtomsToMatch.Count(), "<M>");
    return;
  }
  if( &netA->GetLattice() == &netB->GetLattice() )  {
    for( int i=2; i < AtomsToMatch.Count(); i+=2 )  {
      if( AtomsToMatch[i]->Atom().GetNetwork() != netA ||
          AtomsToMatch[i+1]->Atom().GetNetwork() != netB )  {

        if( AtomsToMatch[i]->Atom().GetNetwork() == netB &&
            AtomsToMatch[i+1]->Atom().GetNetwork() == netA )  {
          AtomsToMatch.Swap(i, i+1);
        }
        else  {
          AtomsToMatch.Clear();
          TGlXApp::GetMainForm()->SetUserCursor( AtomsToMatch.Count(), "<M>");
          return;
        }
      }
    }
  }
  else  {
    for( int i=2; i < AtomsToMatch.Count(); i+=2 )  {
      if( AtomsToMatch[i]->Atom().GetNetwork().GetLattice() != netA->GetLattice() ||
          AtomsToMatch[i+1]->Atom().GetNetwork().GetLattice() != netB->GetLattice() )  {

        if( AtomsToMatch[i]->Atom().GetNetwork().GetLattice() == netB->GetLattice() &&
            AtomsToMatch[i+1]->Atom().GetNetwork().GetLattice() == netA->GetLattice() )  {
          AtomsToMatch.Swap(i, i+1);
        }
        else  {
          AtomsToMatch.Clear();
          TGlXApp::GetMainForm()->SetUserCursor( AtomsToMatch.Count(), "<M>");
          return;
        }
      }
    }
  }

  if( AtomsToMatch.Count() == 2 )  {
    for( int i=0; i < atomsB.Count(); i++ )  {
      atomsB[i]->crd() -= AtomsToMatch[1]->Atom().crd();
      atomsB[i]->crd() += AtomsToMatch[0]->Atom().crd();
    }
    TNetPList na;
    TNetPList nb;
    if( netA->GetLattice() != netB->GetLattice() )  {
      for(int i=0; i < netA->GetLattice().FragmentCount(); i++ )
        na.Add( &netA->GetLattice().GetFragment(i) );
      for(int i=0; i < netB->GetLattice().FragmentCount(); i++ )
        nb.Add( &netB->GetLattice().GetFragment(i) );
    }
    else  {
      na.Add( netA );
      nb.Add( netB );
    }
    TGlGroup& ga = TGlXApp::GetGXApp()->GroupFragments(na, "FragmentA");
    TGlGroup& gb = TGlXApp::GetGXApp()->GroupFragments(nb, "FragmentB");
    TGlXApp::GetGXApp()->SelectAll(false);
    TGlMaterial nm;
    nm.SetFlags(sglmAmbientF);
    nm.AmbientF = 0x0000ff00;
    if( &ga != NULL && ga.IsDefaultColor() )
      ga.GlM( nm );
    nm.AmbientF = 0x000000ff;
    if( &gb != NULL && gb.IsDefaultColor() )
      gb.GlM( nm );

    TGlXApp::GetGXApp()->CenterView();
  }
  if( AtomsToMatch.Count() == 4 )  {
    vec3d orgn = AtomsToMatch[0]->Atom().crd();
    vec3d vec1 = AtomsToMatch[2]->Atom().crd();
             vec1 -= orgn;
    vec3d vec2 = AtomsToMatch[3]->Atom().crd();
             vec2 -= orgn;
    vec3d rv = vec1.XProdVec( vec2 );
    rv.Normalise();
    double ca = vec1.CAngle( vec2 );
    mat3d rm;
    CreateRotationMatrix(rm, rv, ca);

    for( int i=0; i < atomsB.Count(); i++ )
      atomsB[i]->crd() = rm * atomsB[i]->crd();

    orgn = AtomsToMatch[1]->Atom().crd();
    for( int i=0; i < atomsB.Count(); i++ )  {
      atomsB[i]->crd() -= orgn;
      atomsB[i]->crd() += AtomsToMatch[0]->Atom().crd();
    }
    TGlXApp::GetGXApp()->UpdateBonds();
    TGlXApp::GetGXApp()->CenterView();
    //FXApp->CreateObjects();
  }
  if( AtomsToMatch.Count() == 6 )  {
    vec3d rv = AtomsToMatch[0]->Atom().crd();
             rv -= AtomsToMatch[2]->Atom().crd();
    rv.Normalise();

    vec3d v1 = AtomsToMatch[4]->Atom().crd();
             v1 -= AtomsToMatch[2]->Atom().crd();
    vec3d v2 = AtomsToMatch[5]->Atom().crd();
             v2 -= AtomsToMatch[2]->Atom().crd();
    v1 = rv.XProdVec(v1);
    v2 = rv.XProdVec(v2);

    double ca = v1.CAngle( v2 );
    double sa = 0;
    if( ca < 1 )
      sa = sqrt(1 - ca*ca);

    //if( ca < 0 )  ca = -ca;

    double t = 1-ca;

    mat3d rm;
    rm[0][0] = t*rv[0]*rv[0] + ca;
    rm[0][1] = t*rv[0]*rv[1] + sa*rv[2];
    rm[0][2] = t*rv[0]*rv[2] - sa*rv[1];

    rm[1][0] = t*rv[0]*rv[1] - sa*rv[2];
    rm[1][1] = t*rv[1]*rv[1] + ca;
    rm[1][2] = t*rv[1]*rv[2] + sa*rv[0];

    rm[2][0] = t*rv[0]*rv[2] + sa*rv[1];
    rm[2][1] = t*rv[1]*rv[2] - sa*rv[0];
    rm[2][2] = t*rv[2]*rv[2] + ca;

    for( int i=0; i < atomsB.Count(); i++ ) 
      atomsB[i]->crd() = rm * atomsB[i]->crd();

    vec3d orgn = AtomsToMatch[1]->Atom().crd();
    for( int i=0; i < atomsB.Count(); i++ )  {
      atomsB[i]->crd() -= orgn;
      atomsB[i]->crd() += AtomsToMatch[0]->Atom().crd();
    }
    TGlXApp::GetGXApp()->UpdateBonds();
    TGlXApp::GetGXApp()->CenterView();
    //FXApp->CreateObjects();
  }
}

#endif
