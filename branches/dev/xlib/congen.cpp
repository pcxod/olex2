#include "congen.h"
#include "asymmunit.h"
#include "satom.h"
#include "network.h"
#include "unitcell.h"
#include "lattice.h"

AConstraintGenerator::AConstraintGenerator(RefinementModel& rm) : RefMod(rm) {
  Distances(GenId(fgCH3, 1), 0.96);
  Distances(GenId(fgCH2, 2), 0.97);
  Distances(GenId(fgCH2, 1), 0.86);
  Distances(GenId(fgCH1, 3), 0.98);
  Distances(GenId(fgCH1, 2), 0.93);
  Distances(GenId(fgCH1, 1), 0.93);

  Distances(GenId(fgOH2, 0), 0.85);
  Distances(GenId(fgOH1, 0), 0.82);

  Distances(GenId(fgNH3, 0), 0.89);
  Distances(GenId(fgNH2, 0), 0.86);
  Distances(GenId(fgNH1, 0), 0.86);
  Distances(GenId(fgNH1, 3), 0.91);

  Distances(GenId(fgBH1, 3), 1.1084);
  Distances(GenId(fgBH1, 5), 1.10);

  Distances(GenId(fgSiH1, 3), 1.00);
  Distances(GenId(fgSiH2, 2), 1.43);

  Distances(GenId(fgSH1, 0), 1.2);
  
  if( rm.expl.IsTemperatureSet() )  {
    if( rm.expl.GetTempValue().GetV() < -70 )  {
      for( size_t i=0; i < Distances.Count(); i++ )
        Distances.GetValue(i) += 0.02;
    }
    else if( rm.expl.GetTempValue().GetV() < -20 )  {
      for( size_t i=0; i < Distances.Count(); i++ )
        Distances.GetValue(i) += 0.01;
    }
  }
}

void AConstraintGenerator::DoGenerateAtom(TCAtomPList& created, TAsymmUnit& au,
    vec3d_list& Crds, const olxstr& StartingName)
{
  vec3d v;
  bool IncLabel = (Crds.Count() != 1);
  for( size_t i=0; i < Crds.Count(); i++ )  {
    v = Crds[i];
    au.CartesianToCell(v);
    TCAtom& CA = au.NewAtom();
    if( IncLabel )  {
      size_t j = i;
      olxstr lbl = (StartingName + (char)('a' + j));
      CA.SetLabel(au.CheckLabel(&CA, lbl), false);
    }
    else
      CA.SetLabel(au.CheckLabel(&CA, StartingName), false);
    CA.SetType(XElementLib::GetByIndex(iHydrogenIndex));
    CA.ccrd() = v;
    created.Add(CA);
  }
}

void AConstraintGenerator::GenerateAtom( TCAtomPList& created, TAtomEnvi& envi,
     const short Group, const cm_Element& atomType, TAtomEnvi* pivoting )
{
  TAsymmUnit& au = *envi.GetBase().CAtom().GetParent();
  mat3d M, M1;
  M.I();
  M1.I();
  // idialised triangle in XY plane
  static const vec3d Z(0,0,1), X(1,0,0);
  vec3d PlaneN, RotVec, Vec1, Vec2, Vec3;
  double ca;
  bool AnglesEqual;

  TAtomEnvi NEnvi;
  TSAtom* NA;
  vec3d_list crds;
  olxstr tmp;
  double dis;
  tmp = atomType.symbol;
  tmp << envi.GetBase().GetLabel().SubStringFrom(envi.GetBase().GetType().symbol.Length());
  if( tmp.Length() > 3 )
    tmp.SetLength(3);

  switch( Group )  {
    case fgCH3:
      dis = Distances[GenId(fgCH3,1)];
      if( envi.Count() == 1 )  {
        NA = envi.GetBase().GetNetwork().GetLattice().FindSAtom(envi.GetCAtom(0));
        envi.GetBase().GetNetwork().GetLattice().GetUnitCell().GetAtomEnviList(*NA, NEnvi);
        NEnvi.Exclude( envi.GetBase().CAtom() );
        // best approximation, though not really required (one atom in plane of the subs and two - out)...
        if( NEnvi.Count() >= 2 )  {
          RotVec = (NEnvi.GetBase().crd() - envi.GetBase().crd()).Normalise();
          olx_create_rotation_matrix(M, RotVec, -0.5);

          Vec1 = NEnvi.GetCrd(0)  - NEnvi.GetBase().crd();
          Vec1 = M * Vec1;

          Vec2 = RotVec.XProdVec(Vec1).Normalise();
          olx_create_rotation_matrix(M1, Vec2, cos(M_PI*109.4/180) );

          RotVec = M1 * RotVec;
          RotVec *= dis;
          crds.AddNew(RotVec);
          RotVec = M * RotVec;  // 120 degree
          crds.AddNew(RotVec);
          RotVec = M * RotVec;  // 240 degree
          crds.AddNew(RotVec);
          for( int i=0; i < 3; i++ )
            crds[i] += envi.GetBase().crd();
        }
      }
      if( crds.IsEmpty() )  {
        PlaneN = (envi.GetCrd(0) - envi.GetBase().crd()).Normalise();
        RotVec = PlaneN.XProdVec(Z).Normalise();
        olx_create_rotation_matrix(M, RotVec, cos(M_PI*109.4/180));
        crds.AddNew(M*PlaneN);
        olx_create_rotation_matrix(M, PlaneN, cos(M_PI*120./180));
        crds.AddNew(M*crds[0]);
        crds.AddNew(M*crds[1]);
    
        for( size_t i=0; i < crds.Count(); i++ )  {
          crds[i] *= dis;
          crds[i] += envi.GetBase().crd();
        }
      }
      break;
    case fgCH2:
      if( envi.Count() == 2 )  {
        dis = -Distances[GenId(fgCH2,2)];
        // summ vector
        Vec1 = ((envi.GetCrd(0) - envi.GetBase().crd()).Normalise() + (envi.GetCrd(1) - envi.GetBase().crd()).Normalise()).Normalise();
        RotVec = (envi.GetCrd(0) - envi.GetBase().crd()).XProdVec( envi.GetCrd(1) - envi.GetBase().crd() ).Normalise();
        olx_create_rotation_matrix(M, RotVec, cos(M_PI*109.4/360), sin(M_PI*109.4/360));
        crds.AddNew(M*Vec1);
        olx_create_rotation_matrix(M, RotVec, cos(-M_PI*109.4/360), sin(-M_PI*109.4/360));
        crds.AddNew(M*Vec1);
        // final 90 degree rotation
        olx_create_rotation_matrix(M, Vec1, 0, 1);
        crds[0] = (M*crds[0])*dis + envi.GetBase().crd();
        crds[1] = (M*crds[1])*dis + envi.GetBase().crd();
      }
      else if( envi.Count() == 1 )  {
        dis = Distances[GenId(fgCH2,1)];
        NA = envi.GetBase().GetNetwork().GetLattice().FindSAtom( envi.GetCAtom(0) );
        envi.GetBase().GetNetwork().GetLattice().GetUnitCell().GetAtomEnviList(*NA, NEnvi);
        if( NEnvi.Count() >= 2 )  {  // have to create in plane
          NEnvi.Exclude( envi.GetBase().CAtom() );
          if( NEnvi.Count() < 1 )
            throw TFunctionFailedException(__OlxSourceInfo, "assert");
          Vec1 = NEnvi.GetCrd(0) - NEnvi.GetBase().crd();
          Vec2 = envi.GetBase().crd() - NEnvi.GetBase().crd();
          PlaneN = Vec1.XProdVec(Vec2).Normalise();
          olx_create_rotation_matrix(M, PlaneN, -0.5 );
          Vec1 = (NEnvi.GetBase().crd() - envi.GetBase().crd()).Normalise();

          Vec1 = M * Vec1;
          crds.AddNew(Vec1*dis + envi.GetBase().crd());

          Vec1 = M * Vec1;
          crds.AddNew(Vec1*dis + envi.GetBase().crd());
        }
      }
      break;
    case fgCH1:
      AnglesEqual = (envi.Count() == 3);  // proposed by Luc, see Afix 1 in shelxl
      if( envi.Count() == 3 )  {
        dis = Distances[GenId(fgCH1,3)];
        for( size_t i=0; i < envi.Count(); i++ )  {
          if( envi.GetCrd(i).DistanceTo( envi.GetBase().crd() ) > 1.95 &&
            envi.GetType(i) != iBromineZ )  {  // bromine
            AnglesEqual = false;
            break;
          }
        }
        if( AnglesEqual )  {  
          Vec1 = (envi.GetCrd(0)-envi.GetBase().crd()).Normalise();
          Vec2 = (envi.GetCrd(1)-envi.GetBase().crd()).Normalise();
          Vec3 = (envi.GetCrd(2)-envi.GetBase().crd()).Normalise();
          crds.AddNew( (Vec1+Vec2+Vec3).Normalise() );
          Vec1 -= Vec2;
          Vec3 -= Vec2;
          Vec3 = Vec1.XProdVec(Vec3);
          Vec3.NormaliseTo( crds[0].CAngle(Vec3) < 0 ? dis : -dis);
          crds[0] = (Vec3 += envi.GetBase().crd());
        }
      }
      if( !AnglesEqual )  {
        size_t c = 0;
        for( size_t i=0; i < envi.Count(); i++ )  {
//          if( envi.GetCrd(i).DistanceTo( envi.GetBase().crd() ) > 1.95 &&
//            envi.GetBAI(i) != 34 ) // bromine
//            continue;
          Vec1 += (envi.GetCrd(i) - envi.GetBase().crd()).Normalise();
          c++;
        }
        dis = Distances[GenId(fgCH1,(uint16_t)c)];
        crds.AddNew(Vec1.NormaliseTo(-dis) + envi.GetBase().crd());
      }
      break;
    case fgOH3:
      break;
    case fgOH2:
      dis = Distances[GenId(fgOH2,0)];
      if( envi.IsEmpty() && pivoting != NULL)  {
        if( pivoting->Count() == 2 )  {
          Vec1 = (pivoting->GetCrd(0) - pivoting->GetBase().crd()).NormaliseTo(dis);
          crds.AddNew(pivoting->GetBase().crd() + Vec1);
          Vec1 = (pivoting->GetCrd(1) - pivoting->GetBase().crd()).NormaliseTo(dis);
          crds.AddNew(pivoting->GetBase().crd() + Vec1);
        }
        else if( pivoting->Count() == 1 )  {
          PlaneN = (pivoting->GetCrd(0) - envi.GetBase().crd()).Normalise();
          ca = Z.CAngle(PlaneN);
          RotVec = PlaneN.XProdVec(Z).Normalise();
          olx_create_rotation_matrix(M, RotVec, ca);

          crds.AddNew(0, -sin(M_PI*109.4/180), cos(M_PI*109.4/180));
          crds[0] = M * crds[0];
          crds[0] *= dis;
          if( envi.GetBase().CAtom().GetDegeneracy() != 2 )  {
            crds.AddNew(crds[0]);
            olx_create_rotation_matrix(M, PlaneN, -0.5);
            crds[1] = M * crds[1];
            crds[1] += envi.GetBase().crd();
          }
          crds[0] += envi.GetBase().crd();
        }
      }
      else if( envi.Count() == 1 )  {
        PlaneN = (envi.GetCrd(0) - envi.GetBase().crd()).Normalise();
        ca = Z.CAngle(PlaneN);
        RotVec = PlaneN.XProdVec(Z).Normalise();
        olx_create_rotation_matrix(M, RotVec, ca);

        crds.AddNew(0, -sin(M_PI*109.4/180), cos(M_PI*109.4/180));
        crds[0] = M * crds[0];
        crds[0] *= dis;
        if( envi.GetBase().CAtom().GetDegeneracy() != 2 )  {
          crds.AddNew(crds[0]);
          olx_create_rotation_matrix(M, PlaneN, -0.5);
          crds[1] = M * crds[1];
          crds[1] += envi.GetBase().crd();
        }
        crds[0] += envi.GetBase().crd();
      }
      break;
    case fgOH1:
      dis = Distances[GenId(fgOH1,0)];
      if( envi.Count() == 1 )  {
        if( pivoting != NULL && pivoting->Count() >= 1 )  {  // any pssibl H-bonds?
          Vec1 = pivoting->GetCrd(0) - envi.GetBase().crd();
          Vec2 = envi.GetCrd(0) - envi.GetBase().crd();
          if( !pivoting->GetCAtom(0).IsHAttached() )
            RotVec = Vec1.XProdVec(Vec2).Normalise();
          else
            RotVec = Vec2.XProdVec(Vec1).Normalise();
          olx_create_rotation_matrix(M, RotVec, cos(M_PI*109.4/180));

          crds.AddNew(Vec2);
          crds[0] = M * crds[0];
          crds[0].NormaliseTo(dis);
          crds[0] += envi.GetBase().crd();
        }
        else  {
          // Ar-B(OH)2 ?
          if( envi.GetType(0) == iBoronZ )  {
            NA = envi.GetBase().GetNetwork().GetLattice().FindSAtom(envi.GetCAtom(0));
            envi.GetBase().GetNetwork().GetLattice().GetUnitCell().GetAtomEnviList(*NA, NEnvi);
            NEnvi.Exclude(envi.GetBase().CAtom());
            if( NEnvi.Count() == 2 )  { // ArC-BO
            /* in this case the could be cis or trans, put them cis as in Ar-B-O-H ...*/
              if( NEnvi.GetType(0) == iOxygenZ ||  // make sure deal with the right one
                  NEnvi.GetType(1) == iOxygenZ )  {
                if( NEnvi.GetType(0) == iOxygenZ )
                  Vec1 = NEnvi.GetCrd(1);
                else
                  Vec1 = NEnvi.GetCrd(0);

                Vec1 -= NEnvi.GetBase().crd();
                Vec2 = envi.GetBase().crd() - NEnvi.GetBase().crd();

                RotVec = Vec1.XProdVec(Vec2).Normalise();
                olx_create_rotation_matrix(M, RotVec, -cos(M_PI*109.4/180));
                Vec2.Normalise();
                Vec2 = M * Vec2;
                Vec2 *= dis;
                crds.AddNew(Vec2 + envi.GetBase().crd());
              }
            }
          }
        }
      }
      else if( envi.Count() == 2 )  {
        const double d1 = envi.GetCrd(0).DistanceTo(envi.GetBase().crd());
        const double d2 = envi.GetCrd(1).DistanceTo(envi.GetBase().crd());
        if( (d1 > 1.8 && d2 < 1.8) || (d2 > 1.8 && d1 < 1.8) )  {
          Vec1 = envi.GetCrd(0) - envi.GetBase().crd();
          Vec2 = envi.GetCrd(1) - envi.GetBase().crd();
          if( d1 < 1.8 )  {
            RotVec = Vec1.XProdVec(Vec2).Normalise();
            crds.AddNew(Vec1);
          }
          else  {
            RotVec = Vec2.XProdVec(Vec1).Normalise();
            crds.AddNew(Vec2);
          }
          olx_create_rotation_matrix(M, RotVec, cos(M_PI*109.4/180));
          crds[0] = M * crds[0];
          crds[0].NormaliseTo(dis);
          crds[0] += envi.GetBase().crd();
        }
      }
      if( crds.IsEmpty() )  {  // generic case - random placement ...
        PlaneN = envi.GetCrd(0) - envi.GetBase().crd();
        ca = Z.CAngle(PlaneN);
        RotVec = PlaneN.XProdVec(Z).Normalise();
        olx_create_rotation_matrix(M, RotVec, ca);

        crds.AddNew(0, -sin(M_PI*109.4/180), cos(M_PI*109.4/180));
        crds[0] = M * crds[0];
        crds[0] *= dis;
        crds[0] += envi.GetBase().crd();
      }
      break;
    case fgNH4:
      break;
    case fgNH3:
      if( envi.Count() == 1 )  {
        dis = Distances[GenId(fgNH3,0)];
        PlaneN = (envi.GetCrd(0) - envi.GetBase().crd()).Normalise();
        RotVec = PlaneN.XProdVec(Z).Normalise();
        olx_create_rotation_matrix(M, RotVec, cos(M_PI*109.4/180));
        crds.AddNew(M*PlaneN);
        olx_create_rotation_matrix(M, PlaneN, cos(M_PI*120./180));
        crds.AddNew(M*crds[0]);
        crds.AddNew(M*crds[1]);
    
        for( size_t i=0; i < crds.Count(); i++ )  {
          crds[i] *= dis;
          crds[i] += envi.GetBase().crd();
        }
      }
      break;
    case fgNH2:
      dis = Distances[GenId(fgNH2,0)];
      if( envi.Count() == 1 )  {
        if( pivoting == NULL )  {
          PlaneN = (envi.GetCrd(0) - envi.GetBase().crd()).Normalise();
          ca = Z.CAngle(PlaneN);
          RotVec = PlaneN.XProdVec(Z);
          RotVec.Normalise();
          olx_create_rotation_matrix(M, RotVec, ca);

          crds.AddNew(0, -sin(M_PI*109.4/180), cos(M_PI*109.4/180));
          crds[0] = M * crds[0];
          crds[0] *= dis;
          crds.AddNew(crds[0]);
          crds[0] += envi.GetBase().crd();

          olx_create_rotation_matrix(M, PlaneN, -0.5);
          crds[1] = M * crds[1];
          crds[1] += envi.GetBase().crd();
        }
        else  {  // C=NH2
          pivoting->Exclude(envi.GetBase().CAtom());
          if( pivoting->Count() < 1 )
            throw TFunctionFailedException(__OlxSourceInfo, "assert");
          Vec1 = pivoting->GetCrd(0) - pivoting->GetBase().crd();
          Vec2 = envi.GetBase().crd() - pivoting->GetBase().crd();
          PlaneN = Vec1.XProdVec(Vec2);
          PlaneN.Normalise();
          olx_create_rotation_matrix(M, PlaneN, -0.5);
          Vec1 = (pivoting->GetBase().crd() - envi.GetBase().crd()).NormaliseTo(dis);
          Vec1 = M * Vec1;

          crds.AddNew(Vec1);
          crds[0] += envi.GetBase().crd();

          Vec1 = M * Vec1;
          crds.AddNew(Vec1);
          crds[1] += envi.GetBase().crd();
        }
      }
      else if( envi.Count() == 2 )  {
        dis = -dis;
        Vec1 = ((envi.GetCrd(0) - envi.GetBase().crd()).Normalise() + (envi.GetCrd(1) - envi.GetBase().crd()).Normalise()).Normalise();
        RotVec = (envi.GetCrd(0) - envi.GetBase().crd()).XProdVec( envi.GetCrd(1) - envi.GetBase().crd() ).Normalise();
        olx_create_rotation_matrix(M, RotVec, cos(M_PI/3), sin(M_PI/3) );
        crds.AddNew(M*Vec1);
        olx_create_rotation_matrix(M, RotVec, cos(-M_PI/3), sin(-M_PI/3) );
        crds.AddNew(M*Vec1);
        // final 90 degree rotation
        olx_create_rotation_matrix(M, Vec1, 0, 1);
        crds[0] = (M*crds[0])*dis + envi.GetBase().crd();
        crds[1] = (M*crds[1])*dis + envi.GetBase().crd();
      }
      break;
    case fgNH1:
      if( envi.Count() >= 2 )  {
        if( envi.Count() == 3 )
          dis = Distances[GenId(fgNH1,3)];
        else
          dis = Distances[GenId(fgNH1,0)];  // generic...
        for( size_t i=0; i < envi.Count(); i++ )
          Vec1 += (envi.GetCrd(i)-envi.GetBase().crd()).Normalise();
        crds.AddNew(Vec1.NormaliseTo(-dis) + envi.GetBase().crd());
      }
      break;
    case fgBH1:
      if( envi.Count() == 3 )  {
        dis = Distances[GenId(fgBH1,3)];
        Vec1 = (envi.GetCrd(0)-envi.GetBase().crd()).Normalise();
        Vec2 = (envi.GetCrd(1)-envi.GetBase().crd()).Normalise();
        Vec3 = (envi.GetCrd(2)-envi.GetBase().crd()).Normalise();
        crds.AddNew( (Vec1+Vec2+Vec3).Normalise() );
        Vec1 -= Vec2;
        Vec3 -= Vec2;
        Vec3 = Vec1.XProdVec(Vec3);
        Vec3.NormaliseTo(crds[0].CAngle(Vec3) < 0 ? dis : -dis);
        crds[0] = (Vec3 += envi.GetBase().crd());
      }
      else if( envi.Count() == 4 || envi.Count() == 5 )  {
        dis = Distances[GenId(fgBH1,5)];
        bool create = true;
        for( size_t i=0; i < envi.Count(); i++ )  {
          Vec1 += (envi.GetCrd(i)-envi.GetBase().crd()).Normalise();
//          if( !(envi.GetBAI(i).GetIndex() == iCarbonIndex ||
//                envi.GetBAI(i).GetIndex() == iBoronIndex) )  {
//            create = false;
//            break;
//          }
        }
        if( create ) 
          crds.AddNew(Vec1.NormaliseTo(-dis) + envi.GetBase().crd());
      }
      break;
    case fgSiH1:
      if( envi.Count() == 3 )  {
        dis = Distances[GenId(fgSiH1,3)];
        Vec1 = (envi.GetCrd(0)-envi.GetBase().crd()).Normalise();
        Vec2 = (envi.GetCrd(1)-envi.GetBase().crd()).Normalise();
        Vec3 = (envi.GetCrd(2)-envi.GetBase().crd()).Normalise();
        crds.AddNew( (Vec1+Vec2+Vec3).Normalise() );
        Vec1 -= Vec2;
        Vec3 -= Vec2;
        Vec3 = Vec1.XProdVec(Vec3);
        Vec3.NormaliseTo( crds[0].CAngle(Vec3) < 0 ? dis : -dis);
        crds[0] = (Vec3 += envi.GetBase().crd());
      }
      break;
    case fgSiH2:
      if( envi.Count() == 2 )  {
        dis = -Distances[GenId(fgSiH2,2)];
        // summ vector
        Vec1 = ((envi.GetCrd(0) - envi.GetBase().crd()).Normalise() + (envi.GetCrd(1) - envi.GetBase().crd()).Normalise()).Normalise();
        RotVec = (envi.GetCrd(0) - envi.GetBase().crd()).XProdVec( envi.GetCrd(1) - envi.GetBase().crd() ).Normalise();
        olx_create_rotation_matrix(M, RotVec, cos(M_PI/3), sin(M_PI/3) );
        crds.AddNew(M*Vec1);
        olx_create_rotation_matrix(M, RotVec, cos(-M_PI/3), sin(-M_PI/3) );
        crds.AddNew(M*Vec1);
        // final 90 degree rotation
        olx_create_rotation_matrix(M, Vec1, 0, 1);
        crds[0] = (M*crds[0])*dis + envi.GetBase().crd();
        crds[1] = (M*crds[1])*dis + envi.GetBase().crd();
      }
      break;
    case fgSH1:
      dis = Distances[GenId(fgSH1,0)];
      if( envi.Count() == 1 )  {
        if( pivoting != NULL && pivoting->Count() >= 1 )  {  // any possible H-bonds?
          Vec1 = pivoting->GetCrd(0) - envi.GetBase().crd();
          Vec2 = envi.GetCrd(0) - envi.GetBase().crd();
          if( !pivoting->GetCAtom(0).IsHAttached() )
            RotVec = Vec1.XProdVec(Vec2).Normalise();
          else
            RotVec = Vec2.XProdVec(Vec1).Normalise();
          olx_create_rotation_matrix(M, RotVec, cos(M_PI*109.4/180));

          crds.AddNew(Vec2);
          crds[0] = M * crds[0];
          crds[0].NormaliseTo(dis);
          crds[0] += envi.GetBase().crd();
        }
      }
      if( crds.IsEmpty() )  {  // generic case - random placement ...
        PlaneN = envi.GetCrd(0) - envi.GetBase().crd();
        ca = Z.CAngle(PlaneN);
        RotVec = PlaneN.XProdVec(Z).Normalise();
        olx_create_rotation_matrix(M, RotVec, ca);

        crds.AddNew(0, -sin(M_PI*109.4/180), cos(M_PI*109.4/180));
        crds[0] = M * crds[0];
        crds[0] *= dis;
        crds[0] += envi.GetBase().crd();
      }
      break;
  }
  envi.GetBase().CAtom().SetHAttached(!crds.IsEmpty());
  DoGenerateAtom(created, au, crds, tmp);
}

