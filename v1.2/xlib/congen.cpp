#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "congen.h"
#include "asymmunit.h"
#include "satom.h"
#include "network.h"
#include "unitcell.h"
#include "lattice.h"

static const double sqrt3 = sqrt(3.0);

void AConstraintGenerator::DoGenerateAtom( TCAtomPList& created, TAsymmUnit& au,
    TVPointDList& Crds, const olxstr& StartingName)  {
  TVPointD v;
  bool IncLabel = (Crds.Count() != 1);
  for( int i=0; i < Crds.Count(); i++ )  {
    v = Crds[i];
    au.CartesianToCell(v);

    TCAtom* CA = &au.NewAtom();
    CA->SetId( au.AtomCount()-1 );
    CA->SetLoaderId(liNewAtom);
    if( IncLabel )  {
      int j = i;
      olxstr lbl;
      lbl = (StartingName + (char)('a' + j));

      while( au.GetAtomsInfo()->IsElement(lbl) )
        lbl = StartingName + (char)('a' + ++j);
      CA->SetLabel( au.CheckLabel(CA, lbl) );
      CA->AtomInfo( &au.GetAtomsInfo()->GetAtomInfo(iHydrogenIndex) );
    }
    else
      CA->SetLabel( au.CheckLabel(CA, StartingName) );
    CA->CCenter() = v;
    created.Add( CA );
  }
}

void AConstraintGenerator::GenerateAtom( TCAtomPList& created, TAtomEnvi& envi,
     const short Group, const TBasicAtomInfo& atomType, TAtomEnvi* pivoting )  {
  TAsymmUnit& au = *envi.GetBase().CAtom().GetParent();
  TMatrixD M(3,3), M1(3,3);
  M.E();
  M1.E();
  // idialised triangle in XY plane
  TVPointD PlaneN, Z(0,0,1), RotVec, Vec1, Vec2;
  double ca;

  TAtomEnvi NEnvi;
  TSAtom* NA;
  TVPointDList crds;
  olxstr tmp;
  tmp = atomType.GetSymbol();
  tmp << envi.GetBase().GetLabel().SubStringFrom( envi.GetBase().GetAtomInfo().GetSymbol().Length() );
  if( tmp.Length() > 3 )
    tmp.SetLength(3);

  switch( Group )  {
    case fgCH3:
      if( envi.Count() == 1 )  {
        NA = envi.GetBase().GetNetwork().GetLattice().FindSAtom( envi.GetLabel(0) );
        envi.GetBase().GetNetwork().GetLattice().GetUnitCell().GetAtomEnviList(*NA, NEnvi);
        NEnvi.Exclude( envi.GetBase().CAtom() );
        // best approximation, though not really required ...
        if( NEnvi.Count() >= 2 )  {
          RotVec = NEnvi.GetBase().Center();
          RotVec -= envi.GetBase().Center();
          RotVec.Normalise();
          CreateRotationMatrix(M, RotVec, -0.5 );

          Vec1 = NEnvi.GetCrd(0);
          Vec1 -= NEnvi.GetBase().Center();
          Vec1 = M * Vec1;

          Vec2 = RotVec.XProdVec(Vec1);
          Vec2.Normalise();
          CreateRotationMatrix(M1, Vec2, cos(M_PI*109.4/180) );

          RotVec = M1 * RotVec;
          RotVec *= 0.980;
          crds.AddNew(RotVec);
          RotVec = M * RotVec;  // 120 degree
          crds.AddNew(RotVec);
          RotVec = M * RotVec;  // 240 degree
          crds.AddNew(RotVec);
          for( int i=0; i < 3; i++ )
            crds[i] += envi.GetBase().Center();
        }
      }
      if( crds.IsEmpty() )  {
        PlaneN = envi.GetCrd(0);
        PlaneN -= envi.GetBase().Center();
        PlaneN.Normalise();
        ca = Z.CAngle(PlaneN);
        RotVec = PlaneN.XProdVec(Z);
        RotVec.Normalise();
        CreateRotationMatrix(M, RotVec, ca);

        PlaneN *= -0.327;
        crds.AddNew(1, 0, 0);
        crds.AddNew(-0.5, sqrt3/2, 0);
        crds.AddNew(-0.5, -sqrt3/2, 0);
        for( int i=0; i < crds.Count(); i++ )  {
          crds[i] = M * crds[i];
          crds[i] *= 0.924;
          crds[i] += envi.GetBase().Center();
          crds[i] += PlaneN;
        }
      }
      break;
    case fgCH2:
      if( envi.Count() == 2 )  {
        RotVec = envi.GetCrd(0);
        RotVec -= envi.GetBase().Center();
        RotVec.Normalise();

        Vec1 = envi.GetCrd(1);
        Vec1 -= envi.GetBase().Center();
        Vec1.Normalise();

        CreateRotationMatrix(M, RotVec, -0.5 );
        Vec1 = M * Vec1;

        crds.AddNew(Vec1);
        crds[0] += envi.GetBase().Center();

        Vec1 = M * Vec1;
        crds.AddNew(Vec1);
        crds[1] += envi.GetBase().Center();
      }
      else if( envi.Count() == 1 )  {
        NA = envi.GetBase().GetNetwork().GetLattice().FindSAtom( envi.GetLabel(0) );
        envi.GetBase().GetNetwork().GetLattice().GetUnitCell().GetAtomEnviList(*NA, NEnvi);
        if( NEnvi.Count() >= 2 )  {  // have to create in plane
          NEnvi.Exclude( envi.GetBase().CAtom() );
          if( NEnvi.Count() < 1 )
            throw TFunctionFailedException(__OlxSourceInfo, "assert");
          Vec1 = NEnvi.GetCrd(0);
          Vec1 -= NEnvi.GetBase().Center();
          Vec2 = envi.GetBase().Center();
          Vec2 -= NEnvi.GetBase().Center();
          PlaneN = Vec1.XProdVec(Vec2);
          PlaneN.Normalise();
          CreateRotationMatrix(M, PlaneN, -0.5 );
          Vec1 = NEnvi.GetBase().Center();
          Vec1 -= envi.GetBase().Center();
          Vec1.Normalise();
          Vec1 = M * Vec1;

          crds.AddNew(Vec1);
          crds[0] += envi.GetBase().Center();

          Vec1 = M * Vec1;
          crds.AddNew(Vec1);
          crds[1] += envi.GetBase().Center();
        }
      }
      break;
    case fgCH1:
      for( int i=0; i < envi.Count(); i++ )  {
        if( envi.GetCrd(i).DistanceTo( envi.GetBase().Center() ) > 1.95 )
          continue;
        Vec1 += envi.GetCrd(i);
        Vec1 -= envi.GetBase().Center();
      }
      Vec1.Normalise();
      Vec1 *= -0.96;
      crds.AddNew(Vec1);
      crds[0] += envi.GetBase().Center();
      break;
    case fgOH3:
      break;
    case fgOH2:
      if( envi.IsEmpty() && pivoting != NULL)  {
        if( pivoting->Count() == 2 )  {
          Vec1 = pivoting->GetCrd(0);
          Vec1 -= pivoting->GetBase().Center();
          Vec1.Normalise();
          Vec1 *= 0.85;
          crds.AddNew( pivoting->GetBase().Center() );
          crds[0] += Vec1;

          Vec1 = pivoting->GetCrd(1);
          Vec1 -= pivoting->GetBase().Center();
          Vec1.Normalise();
          Vec1 *= 0.85;
          crds.AddNew( pivoting->GetBase().Center() );
          crds[1] += Vec1;
        }
        else if( pivoting->Count() == 1 )  {
          PlaneN = pivoting->GetCrd(0);
          PlaneN -= envi.GetBase().Center();
          PlaneN.Normalise();
          ca = Z.CAngle(PlaneN);
          RotVec = PlaneN.XProdVec(Z);
          RotVec.Normalise();
          CreateRotationMatrix(M, RotVec, ca);

          crds.AddNew(0, -sin(M_PI*109.4/180), cos(M_PI*109.4/180));
          crds[0] = M * crds[0];
          crds[0] *= 0.85;
          crds.AddNew( crds[0] );
          crds[0] += envi.GetBase().Center();

          CreateRotationMatrix(M, PlaneN, -0.5 );
          crds[1] = M * crds[1];
          crds[1] += envi.GetBase().Center();
        }
      }
      else if( envi.Count() == 1 )  {
        PlaneN = envi.GetCrd(0);
        PlaneN -= envi.GetBase().Center();
        PlaneN.Normalise();
        ca = Z.CAngle(PlaneN);
        RotVec = PlaneN.XProdVec(Z);
        RotVec.Normalise();
        CreateRotationMatrix(M, RotVec, ca);

        crds.AddNew(0, -sin(M_PI*109.4/180), cos(M_PI*109.4/180));
        crds[0] = M * crds[0];
        crds[0] *= 0.85;
        crds.AddNew( crds[0] );
        crds[0] += envi.GetBase().Center();

        CreateRotationMatrix(M, PlaneN, -0.5 );
        crds[1] = M * crds[1];
        crds[1] += envi.GetBase().Center();
      }
      break;
    case fgOH1:
      if( envi.Count() == 1 )  {
        if( pivoting != NULL && pivoting->Count() >= 1 )  {  // any pssibl H-bonds?

          Vec1 = pivoting->GetCrd( 0 );
          Vec1 -= envi.GetBase().Center();

          Vec2 = envi.GetCrd(0);
          Vec2 -= envi.GetBase().Center();

          if( !pivoting->GetCAtom(0).IsHAttached() )
            RotVec = Vec1.XProdVec( Vec2 );
          else
            RotVec = Vec2.XProdVec( Vec1 );
          RotVec.Normalise();
          CreateRotationMatrix(M, RotVec, cos( M_PI*109.4/180)  );

          crds.AddNew(Vec2);
          crds[0] = M * crds[0];
          crds[0].Normalise();
          crds[0] *= 0.85;
          crds[0] += envi.GetBase().Center();
        }
        else  {
          // Ar-B(OH)2 ?
          if( envi.GetBAI(0).GetIndex() == iBoronIndex )  {
            NA = envi.GetBase().GetNetwork().GetLattice().FindSAtom( envi.GetLabel(0) );
            envi.GetBase().GetNetwork().GetLattice().GetUnitCell().GetAtomEnviList(*NA, NEnvi);
            NEnvi.Exclude( envi.GetBase().CAtom() );
            if( NEnvi.Count() == 2 )  { // ArC-BO
            /* in this case the could be cis or trans, put them cis as in Ar-B-O-H ...*/
              if( NEnvi.GetBAI(0).GetIndex() == iOxygenIndex ||  // make sure deal with the right one
                  NEnvi.GetBAI(1).GetIndex() == iOxygenIndex )  {
                if( NEnvi.GetBAI(0).GetIndex() == iOxygenIndex )
                  Vec1 = NEnvi.GetCrd(1);
                else
                  Vec1 = NEnvi.GetCrd(0);

                Vec1 -= NEnvi.GetBase().Center();
                Vec2 = envi.GetBase().Center();
                Vec2 -= NEnvi.GetBase().Center();


                RotVec = Vec1.XProdVec(Vec2);
                RotVec.Normalise();
                CreateRotationMatrix(M, RotVec, -cos( M_PI*109.4/180)  );
                Vec2.Normalise();
                Vec2 = M * Vec2;
                Vec2 *= 0.85;
                Vec2 += envi.GetBase().Center();
                crds.AddNew( Vec2 );
              }
            }
          }
        }
      }
      if( crds.IsEmpty() )  {  // generic case - random placement ...
        PlaneN = envi.GetCrd(0);
        PlaneN -= envi.GetBase().Center();
        PlaneN.Normalise();
        ca = Z.CAngle(PlaneN);
        RotVec = PlaneN.XProdVec(Z);
        RotVec.Normalise();
        CreateRotationMatrix(M, RotVec, ca);

        crds.AddNew(0, -sin(M_PI*109.4/180), cos(M_PI*109.4/180));
        crds[0] = M * crds[0];
        crds[0] *= 0.85;
        crds[0] += envi.GetBase().Center();
      }
      break;
    case fgNH4:
      break;
    case fgNH3:
      if( envi.Count() == 1 )  {
        PlaneN = envi.GetCrd(0);
        PlaneN -= envi.GetBase().Center();
        PlaneN.Normalise();
        ca = Z.CAngle(PlaneN);
        RotVec = PlaneN.XProdVec(Z);
        RotVec.Normalise();
        CreateRotationMatrix(M, RotVec, ca);

        PlaneN *= -0.327;
        crds.AddNew(1, 0, 0);
        crds.AddNew(-0.5, sqrt3/2, 0);
        crds.AddNew(-0.5, -sqrt3/2, 0);
        for( int i=0; i < crds.Count(); i++ )  {
          crds[i] = M * crds[i];
          crds[i] *= 0.85;
          crds[i] += envi.GetBase().Center();
          crds[i] += PlaneN;
        }
      }
      break;
    case fgNH2:
      if( envi.Count() == 1 )  {
        if( pivoting == NULL )  {
          PlaneN = envi.GetCrd(0);
          PlaneN -= envi.GetBase().Center();
          PlaneN.Normalise();
          ca = Z.CAngle(PlaneN);
          RotVec = PlaneN.XProdVec(Z);
          RotVec.Normalise();
          CreateRotationMatrix(M, RotVec, ca);

          crds.AddNew(0, -sin(M_PI*109.4/180), cos(M_PI*109.4/180));
          crds[0] = M * crds[0];
          crds[0] *= 0.85;
          crds.AddNew( crds[0] );
          crds[0] += envi.GetBase().Center();

          CreateRotationMatrix(M, PlaneN, -0.5 );
          crds[1] = M * crds[1];
          crds[1] += envi.GetBase().Center();
        }
        else  {  // C=NH2
          pivoting->Exclude( envi.GetBase().CAtom() );
          if( pivoting->Count() < 1 )
            throw TFunctionFailedException(__OlxSourceInfo, "assert");
          Vec1 = pivoting->GetCrd(0);
          Vec1 -= pivoting->GetBase().Center();
          Vec2 = envi.GetBase().Center();
          Vec2 -= pivoting->GetBase().Center();
          PlaneN = Vec1.XProdVec(Vec2);
          PlaneN.Normalise();
          CreateRotationMatrix(M, PlaneN, -0.5 );
          Vec1 = pivoting->GetBase().Center();
          Vec1 -= envi.GetBase().Center();
          Vec1.Normalise();
          Vec1 *= 0.85;
          Vec1 = M * Vec1;

          crds.AddNew(Vec1);
          crds[0] += envi.GetBase().Center();

          Vec1 = M * Vec1;
          crds.AddNew(Vec1);
          crds[1] += envi.GetBase().Center();
        }
      }
      else if( envi.Count() == 2 )  {
        RotVec = envi.GetCrd(0);
        RotVec -= envi.GetBase().Center();
        RotVec.Normalise();

        Vec1 = envi.GetCrd(1);
        Vec1 -= envi.GetBase().Center();
        Vec1.Normalise();

        CreateRotationMatrix(M, RotVec, -0.5 );
        Vec1 = M * Vec1;

        crds.AddNew(Vec1);
        crds[0] += envi.GetBase().Center();

        Vec1 = M * Vec1;
        crds.AddNew(Vec1);
        crds[1] += envi.GetBase().Center();
      }
      break;
    case fgNH1:
      if( envi.Count() == 3 )  {
        for( int i=0; i < envi.Count(); i++ )  {
          Vec1 = envi.GetCrd(i);
          Vec1 -= envi.GetBase().Center();
          Vec1.Normalise();
          Vec2 += Vec1;
        }
        Vec2.Normalise();
        Vec2 *= -0.90;
        crds.AddNew(Vec2);
        crds[0] += envi.GetBase().Center();
      }
      else if( envi.Count() == 2 )  {
        for( int i=0; i < envi.Count(); i++ )  {
          Vec1 += envi.GetCrd(i);
          Vec1 -= envi.GetBase().Center();
        }
        Vec1.Normalise();
        Vec1 *= -0.90;
        crds.AddNew(Vec1);
        crds[0] += envi.GetBase().Center();
      }
      break;
    case fgBH1:
      if( envi.Count() == 4 ||  envi.Count() == 5 )  {
        bool create = true;
        for( int i=0; i < envi.Count(); i++ )  {
          Vec1 += envi.GetCrd(i);
          Vec1 -= envi.GetBase().Center();
//          if( !(envi.GetBAI(i).GetIndex() == iCarbonIndex ||
//                envi.GetBAI(i).GetIndex() == iBoronIndex) )  {
//            create = false;
//            break;
//          }
        }
        if( create )  {
          Vec1.Normalise();
          Vec1 *= -1.12;
          crds.AddNew(Vec1);
          crds[0] += envi.GetBase().Center();
        }
      }
      break;
  }
  envi.GetBase().CAtom().SetHAttached( crds.Count() != 0 );
  DoGenerateAtom(created, au, crds, tmp);
  for( int i=0; i < created.Count(); i++ )
    created[i]->SetAfixAtomId( envi.GetBase().CAtom().GetLoaderId() );
}

