//----------------------------------------------------------------------------//
// crystallographic symmetry library
// TXBond
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdlib.h>

#include "symmlib.h"
#include "dataitem.h"
#include "symmparser.h"

  TSymmLib* TSymmLib::Instance = NULL;

TCLattice::TCLattice(int latt)  {
  this->Latt = latt;
  switch( abs(Latt) )  {
   case 1:
    Name = "Primitive";
    Symbol = "P";
    break;
   case 2:      // Body Centered (I)
    Name = "Body Centered";
    Symbol = "I";
    Vectors.AddNew<double,double,double>(0.5, 0.5, 0.5);
    break;
   case 3:      // R Centered
    Name = "R Centered";
    Symbol = "R";
    Vectors.AddNew<double,double,double>(2./3., 1./3., 1./3.);
    Vectors.AddNew<double,double,double>(1./3., 2./3., 2./3.);
    break;
   case 4:      // Face Centered (F)
    Name = "Face Centered";
    Symbol = "F";
    Vectors.AddNew<double,double,double>(0, 0.5, 0.5);
    Vectors.AddNew<double,double,double>(0.5, 0, 0.5);
    Vectors.AddNew<double,double,double>(0.5, 0.5, 0);
    break;
   case 5:      // A Centered (A)
    Name = "A Centered";
    Symbol = "A";
    Vectors.AddNew<double,double,double>(0, 0.5, 0.5);
    break;
   case 6:      // B Centered (B)
    Name = "B Centered";
    Symbol = "B";
    Vectors.AddNew<double,double,double>(0.5, 0, 0.5);
    break;
   case 7:      // C Centered (C);
    Name = "C Centered";
    Symbol = "C";
    Vectors.AddNew<double,double,double>(0.5, 0.5, 0);
    break;
   default:
    throw new TIncorrectLattExc(__OlxSourceInfo, Latt);
  }
}
//..............................................................................
//..............................................................................
TSpaceGroup::TSpaceGroup(const olxstr& Name, const olxstr& FullName, const olxstr HallSymbol, 
                         const olxstr& Axis, int Number, TCLattice& Latt, bool Centrosymmetric)  {
  this->Name = Name;
  this->FullName = FullName;
  this->HallSymbol = HallSymbol;
  this->Axis = Axis;
  this->Number = Number;
  this->Latt = &Latt;
  this->CentroSymmetric = Centrosymmetric;
  BravaisLattice = NULL;
  LaueClass = NULL;
  PointGroup = NULL;
  Translations = false;
}
//..............................................................................
void TSpaceGroup::AddMatrix(const TMatrixD& m)   {
  Matrices.AddCCopy(m);
  if( !Translations )  {
    if( m[0][3] != 0 || m[1][3] != 0 || m[1][3] != 0 )
      Translations = true;
  }
}
//..............................................................................
bool TSpaceGroup::ContainsElement(TSymmElement* symme)  {
  if( MatrixCount() != symme->MatrixCount() )  return false;
  for( int i=0; i  < MatrixCount(); i++ )  Matrices[i].SetTag(0);

  for( int i=0; i  < symme->MatrixCount(); i++ )  {
    bool found = false;
    TMatrixD& m = symme->GetMatrix(i);
    for( int j=0; j  < MatrixCount(); j++ )
    {
      TMatrixD& m1 = Matrices[j];
      if( m1.GetTag() )  continue;
      bool equal = true;
      for( int k=0; k < 3; k++ )  {
        for( int l=0; l < 3; l++ )  {
          if( m[k][l] != m1[k][l] )  {
            equal = false;
            break;
          }
        }
        if( !equal )  break;
      }
      if( !equal )  continue;
      m1.SetTag(1);
      found = true;
      break;
    }
    if( !found )  return false;
  }
  return true;
}
//..............................................................................
bool TSpaceGroup::ContainsGroup(TSpaceGroup* symme)  {
  if( MatrixCount() != symme->MatrixCount() )  return false;
  if( IsCentrosymmetric() != symme->IsCentrosymmetric() )  return false;
  for( int i=0; i  < MatrixCount(); i++ )  Matrices[i].SetTag(0);

  for( int i=0; i  < symme->MatrixCount(); i++ )  {
    bool found = false;
    TMatrixD& m = symme->GetMatrix(i);
    for( int j=0; j  < MatrixCount(); j++ )
    {
      TMatrixD& m1 = Matrices[j];
      if( m1.GetTag() )  continue;
      bool equal = true;
      for( int k=0; k < 3; k++ )  {
        for( int l=0; l < 3; l++ )  {
          if( m[k][l] != m1[k][l] )  {
            equal = false;
            break;
          }
        }
        if( !equal )  break;
      }
      if( !equal )  continue;
      m1.SetTag(1);
      found = true;
      break;
    }
    if( !found )  return false;
  }
  return true;
}
//..............................................................................
bool TSpaceGroup::ContainsElement( const TMatrixDList& matrices, TSymmElement* symme) {
  if( matrices.Count() < symme->MatrixCount() )  return false;
  for( int i=0; i  < matrices.Count(); i++ )  matrices[i].SetTag(0);

  for( int i=0; i  < symme->MatrixCount(); i++ )  {
    bool found = false;
    TMatrixD& m = symme->GetMatrix(i);
    for( int j=0; j  < matrices.Count(); j++ )
    {
      TMatrixD& m1 = matrices[j];
      if( m1.GetTag() )  continue;
      bool equal = true;
//      for( int k=0; k < 3; k++ )  {
//        for( int l=0; l < 3; l++ )  {
//          if( m[k][l] != m1[k][l] )  {
//            equal = false;
//            break;
//          }
//        }
      // have to consider sign change only for centrisymmetric groups
      if( !equal )  continue;
      for( int k=0; k < 3; k++ )  {
        // check only if the necessary translations do exist
        if( m[k][3] == 0 )  continue;
        double diff = m[k][3] - m1[k][3];
        double summ = m[k][3] + m1[k][3];
        int iv = (int)diff;  diff -= iv;
        iv = (int)summ;      summ -= iv;
        if( fabs(diff) < 0.01 || fabs(diff) > 0.99 )  diff = 0;
        if( fabs(summ) < 0.01 || fabs(summ) > 0.99 )  summ = 0;
        if( diff < 0 )  diff += 1;
        if( summ < 0 )  summ += 1;
        if( diff > 0.01 && summ > 0.01 )  {
          equal = false;
          break;
        }
      }
      if( equal )  {
        found = true;
        m1.SetTag( 1 );
      }
    }
    if( !found )  return false;
  }
  return true;
}
//..............................................................................
bool TSpaceGroup::IsSubElement( TSpaceGroup* symme )  const  {
  if( MatrixCount() < symme->MatrixCount() )  return false;
  for( int i=0; i  < MatrixCount(); i++ )  Matrices[i].SetTag(0);

  for( int i=0; i  < symme->MatrixCount(); i++ )  {
    bool found = false;
    TMatrixD& m = symme->GetMatrix(i);
    for( int j=0; j  < MatrixCount(); j++ )
    {
      TMatrixD& m1 = Matrices[j];
      if( m1.GetTag() )  continue;
      bool equal = true;
      int matrixElements = 0;
      int signChanges = 0;
      for( int k=0; k < 3; k++ )  {
        for( int l=0; l < 3; l++ )  {
          if( m1[k][l] != 0 )  matrixElements++;
          if( m[k][l] != m1[k][l] )  {
            if( fabs(m[k][l]) != fabs(m1[k][l]) )  {
              equal = false;
              break;
            }
            else  signChanges++;
          }
        }
        if( !equal )  break;
      }
      // have to consider sign change only for centrisymmetric groups
      if( IsCentrosymmetric() )  {
        if( !equal || ( (signChanges != matrixElements) && (signChanges != 0)) )  continue;
      }
      else  {
        if( !equal || (signChanges != 0) )  continue;
      }
      equal = true;
      for( int k=0; k < 3; k++ )  {
        // check only if the necessary translations do exist
        if( m[k][3] == 0 )  continue;
        double diff = m[k][3] - m1[k][3];
        int iv = (int)diff;  diff -= iv;
        if( fabs(diff) < 0.01 || fabs(diff) > 0.99 )  diff = 0;
        if( diff < 0 )  diff += 1;
        if( diff > 0.01 )  {
          equal = false;
          break;
        }
      }
      if( equal )  {
        found = true;
        m1.SetTag( 1 );
      }
    }
    if( !found )  return false;
  }
  return true;
}
//..............................................................................
bool TSpaceGroup::EqualsWithoutTranslation (const TSpaceGroup& sg) const  {
  if( MatrixCount() != sg.MatrixCount() )  return false;
  int mc = MatrixCount();
  bool found;
  for( int i=0; i  < mc; i++ )  Matrices[i].SetTag(0);

  for( int i=0; i  < mc; i++ )  {
    found = false;
    TMatrixD& m = sg.GetMatrix(i);
    for( int j=0; j  < mc; j++ )  {
      TMatrixD& m1 = Matrices[j];
      if( m1.GetTag() )  continue;
      bool equal = true;
      int signChanges = 0;
      int matrixElements = 0;
      for( int k=0; k < 3; k++ )  {
        for( int l=0; l < 3; l++ )  {
          if( m1[k][l] != 0 )  matrixElements++;
          if( m[k][l] != m1[k][l] )  {
            if( fabs(m[k][l]) != fabs(m1[k][l]) )  {
              equal = false;
              break;
            }
            else  signChanges++;
          }
        }
        if( !equal )  break;
      }

      // have to consider sign change only for centrisymmetric groups
      if( !equal || ( (signChanges != matrixElements) && (signChanges != 0)) )  continue;
      found = true;
      m1.SetTag(1);
    }
    if( !found )  return false;
  }
  return true;
}
//..............................................................................
bool TSpaceGroup::operator == (const TAsymmUnit& AU) const
{
  if( Latt->GetLatt() != abs(AU.GetLatt()) )  return false;
  if( CentroSymmetric && (AU.GetLatt() < 0) )  return false;
  if( (!CentroSymmetric) && (AU.GetLatt() > 0) )  return false;
  if( MatrixCount() != AU.MatrixCount() )  return false;

  int mc = MatrixCount();
  int iv;
  bool found;
  TVectorD translation(3);
  for( int i=0; i  < mc; i++ )  Matrices[i].SetTag(0);

  for( int i=0; i  < mc; i++ )  {
    found = false;
    const TMatrixD* m = &AU.GetMatrix(i);
    for( int j=0; j  < mc; j++ )  {
      TMatrixD& m1 = Matrices[j];
      if( m1.GetTag() )  continue;
      bool equal = true;
      int matrixElements = 0;
      int signChanges = 0;
      for( int k=0; k < 3; k++ )  {
        for( int l=0; l < 3; l++ )  {
          if( m1[k][l] != 0 )  matrixElements++;
          if( m->Data(k)[l] != m1[k][l] )  {
            if( fabs(m->Data(k)[l]) != fabs(m1[k][l]) )  {
              equal = false;
              break;
            }
            else  signChanges++;
          }
        }
        if( !equal )  break;
      }

      // have to consider sign change only for centrisymmetric groups
      if( IsCentrosymmetric() )  {
        if( !equal || ( (signChanges != matrixElements) && (signChanges != 0)) )  continue;
      }
      else  {
        if( !equal || (signChanges != 0) )  continue;
      }
      for( int k=0; k < 3; k++ )  {
        translation[k] = m->Data(k)[3] - m1[k][3];
        iv = (int)translation[k];  translation[k] -= iv;
        if( fabs(translation[k]) < 0.01 || fabs(translation[k]) >= 0.99 )
          translation[k] = 0;
        if( translation[k] < 0 )
          translation[k] += 1;
      }
      if( translation.Length() < 0.02 )  {  found = true;  break;  }
      for( int k=0; k < Latt->VectorCount(); k++ )  {
        if( translation.DistanceTo(Latt->GetVector(k)) < 0.02 )
        {  found = true;  break;  }
      }
      if( found )  {  m1.SetTag(1);  break;  }
      if( CentroSymmetric )  {
        equal = true;
        for( int k=0; k < 3; k++ )  {
          for( int l=0; l < 3; l++ )
            if( m->Data(k)[l] != -m1[k][l] )  {  equal = false;  break;  }
          if( !equal )  break;
        }
        if( !equal )  continue;
        for( int k=0; k < 3; k++ )  {
          translation[k] = m->Data(k)[3] - m1[k][3];
          iv = (int)translation[k];  translation[k] -= iv;
          if( fabs(translation[k]) < 0.01 || fabs(translation[k]) >= 0.99 )
            translation[k] = 0;
          if( translation[k] < 0 )
            translation[k] += 1;
        }
        if( translation.Length() < 0.02 )  {  found = true;  break;  }
        for( int k=0; k < Latt->VectorCount(); k++ )  {
          if( translation.DistanceTo(Latt->GetVector(k)) < 0.02 )
          {  found = true;  break;  }
        }
        if( found )  {  m1.SetTag(1);  break;  }
      }
    }
    if( !found )  return false;
  }
  return true;
}
//..............................................................................
bool TSpaceGroup::EqualsExpandedSG(const TAsymmUnit& AU) const  {
  if( MatrixCount() > AU.MatrixCount() )  return false;
  
  TMatrixDList allMatrices;
  GetMatrices(allMatrices, mattAll );

  if( allMatrices.Count() != AU.MatrixCount() )  return false;

  bool found;
  TVectorD translation(3);
  for( int i=0; i  < allMatrices.Count(); i++ )  allMatrices[i].SetTag(0);

  for( int i=0; i  < allMatrices.Count(); i++ )  {
    found = false;
    const TMatrixD* m = &AU.GetMatrix(i);
    for( int j=0; j  < allMatrices.Count(); j++ )  {
      TMatrixD& m1 = allMatrices[j];
      if( m1.GetTag() )  continue;
      bool equal = true;
      for( int k=0; k < 3; k++ )  {
        for( int l=0; l < 3; l++ )  {
          if( m->Data(k)[l] != m1[k][l] )  {
            equal = false;
            break;
          }
        }
        if( !equal )  break;
      }
      if( !equal )  continue;

      for( int k=0; k < 3; k++ )  {
        translation[k] = m->Data(k)[3] - m1[k][3];
        int iv = (int)translation[k];  translation[k] -= iv;
        if( fabs(translation[k]) < 0.01 || fabs(translation[k]) >= 0.99 )
          translation[k] = 0;
        if( translation[k] < 0 )
          translation[k] += 1;
      }
      if( translation.Length() < 0.01 )  found = true;
      if( found )  {  m1.SetTag(1);  break;  }
    }
    if( !found )  return false;
  }
  return true;
}
//..............................................................................
int TSpaceGroup::GetUniqMatrices(TMatrixDList& matrices, short Flags) const  {
  TMatrixDList allm;
  TMatrixD matr(3,3);
  int c = 0;
  GetMatrices( allm, Flags );
  for( int i=0; i < allm.Count(); i++ )  {
    TMatrixD& m = allm[i];
    matr[0][0] = m [0][0];  matr[0][1] = m [0][1];  matr[0][2] = m [0][2];
    matr[1][0] = m [1][0];  matr[1][1] = m [1][1];  matr[1][2] = m [1][2];
    matr[2][0] = m [2][0];  matr[2][1] = m [2][1];  matr[2][2] = m [2][2];

    if( matrices.IndexOf( matr ) == -1 )  {
      matrices.AddCCopy( matr );
      c++;
    }
  }
  return c;
}
//..............................................................................
void TSpaceGroup::GetMatrices(TMatrixDList& matrices, short Flags) const {
  TMatrixD *m, *m1;
  for( int i=-1; i < MatrixCount(); i++ )  {
    m = NULL;
    if( i < 0 && ((Flags & mattIdentity) == mattIdentity) )  {
      m = new TMatrixD(3, 4);
      m->E();
    }
    else  {
      if( i == -1 )  continue;
      if( (Flags & mattTranslation) == mattTranslation && (Flags & mattCentering) == 0 )  {
        TMatrixD& mt = Matrices[i];
        if( mt[0][3] != 0 || mt[1][3] != 0 || mt[2][3] != 0 )  continue;
          m = new TMatrixD( mt );
      }
      else
        m = new TMatrixD( Matrices[i] );
    }

    if( !m )  continue;

    matrices.Add(*m);
    if( (Flags & mattCentering) == mattCentering )  {
      for( int j=0; j < Latt->VectorCount(); j++ )  {
        TVectorD& v = Latt->GetVector(j);
        if( (Flags & mattTranslation) == 0 )  {
          TMatrixD& mt = Matrices[i];
          double dv = mt[0][3] - v[0];
          int    iv = (int)dv;  dv -= iv;
          if( fabs(dv) < 0.01 || fabs(dv) > 0.99 )  dv = 0;
          if( dv != 0 )  continue;
          dv = mt[1][3] - v[1];
          iv = (int)dv;    dv -= iv;
          if( fabs(dv) < 0.01 || fabs(dv) > 0.99 )  dv = 0;
          if( dv != 0 )  continue;
          dv = mt[2][3] - v[2];
          iv = (int)dv;     dv -= iv;
          if( fabs(dv) < 0.01 || fabs(dv) > 0.99 )  dv = 0;
          if( dv != 0 )  continue;
        }
        m1 = new TMatrixD(*m);

        m1->Data(0)[3] += v[0];
        int iv = (int)m1->Data(0)[3];
        m1->Data(0)[3] -= iv;
        if( m1->Data(0)[3] < 0 )  m1->Data(0)[3] += 1;

        m1->Data(1)[3] += v[1];
        iv = (int)m1->Data(1)[3];
        m1->Data(1)[3] -= iv;
        if( m1->Data(1)[3] < 0 )  m1->Data(1)[3] += 1;

        m1->Data(2)[3] += v[2];
        iv = (int)m1->Data(2)[3];
        m1->Data(2)[3] -= iv;
        if( m1->Data(2)[3] < 0 )  m1->Data(2)[3] += 1;
        matrices.Add(*m1);
      }
    }
  }
  if( CentroSymmetric && ((Flags & mattInversion) == mattInversion) )  {
    for( int i=0; i < matrices.Count(); i++ )  {
      m = new TMatrixD( matrices[i] );
      for( int j=0; j < 3; j++ )
        for( int k=0; k < 3; k++ )
          m->Data(j)[k]*=-1;
      matrices.Insert(i+1, *m);
      i++;
    }
    if( (Flags & mattIdentity) != mattIdentity )  {
      m = new TMatrixD(3,4);
      m->E();
      *m *= -1;
      matrices.Insert(0, *m);
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
int TBravaisLattice::FindSpaceGroups(TPtrList<TSpaceGroup>& SpaceGroups) const  {
  int rc = 0;
  for( int i=0; i < TSymmLib::GetInstance()->SGCount(); i++ )  {
    if( &TSymmLib::GetInstance()->GetGroup(i).GetBravaisLattice() == this )  {
      rc++;
      SpaceGroups.Add( &TSymmLib::GetInstance()->GetGroup(i) );
    }
  }
  return rc;
}
//..............................................................................
//..............................................................................
//..............................................................................
TSymmElement::TSymmElement(const olxstr& name, TSpaceGroup* sg)  {
  sg->GetMatrices(Matrices, mattAll^mattIdentity);
  Name = name;
}
//..............................................................................
//..............................................................................
//..............................................................................
TSymmLib::TSymmLib(const olxstr& FN)  {
  if( Instance != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "An instance of the library is already created");

  for( int i=1; i < 8; i++ )  {
    TCLattice* CL = new TCLattice(i);
    Lattices.Add(CL->GetSymbol(), CL);
  }
  TDataFile DF;
  DF.LoadFromXLFile(FN, NULL);
  TSpaceGroup *SG;
  TMatrixD M(3, 4);
  olxstr Tmp;

  int Latt;
  for( int i=0; i < DF.Root().ItemCount(); i++ )  {
    TDataItem & DI = DF.Root().Item(i);
    Latt = DI.GetFieldValue("LATT").ToInt();
    SG = new TSpaceGroup(DI.GetName(), DI.GetFieldValue("FULL"), DI.GetFieldValue("HS"),
                         DI.GetFieldValue("AXIS"), DI.GetFieldValue("#").ToInt(),
                         GetLattice(abs(Latt)-1), (Latt > 0));
    SpaceGroups.Add(DI.GetName(), SG);

    for( int j=0; j < DI.ItemCount(); j++ )  {
      M.Null();
      TSymmParser::SymmToMatrix(DI.Item(j).GetValue(), M);
      SG->AddMatrix(M);
    }
  }
  TBravaisLattice* BL = new TBravaisLattice("Triclinic");
  BL->AddSymmetry( this->FindGroup("P-1") );
  BL->AddLattice( FindLattice("P") );
  BravaisLattices.Add( BL->GetName(), BL );

  BL = new TBravaisLattice("Monoclinic");
  BL->AddSymmetry( this->FindGroup("P2/m") );
  BL->AddLattice( FindLattice("P") );
  BL->AddLattice( FindLattice("A") );
  BL->AddLattice( FindLattice("B") );
  BL->AddLattice( FindLattice("C") );
  BravaisLattices.Add( BL->GetName(), BL );

  BL = new TBravaisLattice("Orthorhombic");
  BL->AddSymmetry( this->FindGroup("Pmmm") );
  BL->AddLattice( FindLattice("P") );
  BL->AddLattice( FindLattice("A") );
  BL->AddLattice( FindLattice("B") );
  BL->AddLattice( FindLattice("C") );
  BL->AddLattice( FindLattice("I") );
  BL->AddLattice( FindLattice("F") );
  BravaisLattices.Add( BL->GetName(), BL );

  BL = new TBravaisLattice("Tetragonal");
  BL->AddSymmetry( this->FindGroup("P4/m") );
  BL->AddSymmetry( this->FindGroup("P4/mmm") );
  BL->AddLattice( FindLattice("P") );
  BL->AddLattice( FindLattice("I") );
  BravaisLattices.Add( BL->GetName(), BL );

  BL = new TBravaisLattice("Trigonal");
  BL->AddSymmetry( this->FindGroup("P-3") );
  BL->AddSymmetry( this->FindGroup("P-3m1") );
  BL->AddSymmetry( this->FindGroup("P-31m") );
  BL->AddLattice( FindLattice("P") );
  BL->AddLattice( FindLattice("R") );
  BravaisLattices.Add( BL->GetName(), BL );

  BL = new TBravaisLattice("Hexagonal");
  BL->AddSymmetry( this->FindGroup("P6/m") );
  BL->AddSymmetry( this->FindGroup("P6/mmm") );
  BL->AddLattice( FindLattice("P") );
  BravaisLattices.Add( BL->GetName(), BL );

  BL = new TBravaisLattice("Cubic");
  BL->AddSymmetry( this->FindGroup("Pm-3") );
  BL->AddSymmetry( this->FindGroup("Pm-3m") );
  BL->AddLattice( FindLattice("P") );
  BL->AddLattice( FindLattice("I") );
  BL->AddLattice( FindLattice("F") );
  BravaisLattices.Add( BL->GetName(), BL );

//  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("2--", FindGroup("P211") );
//  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("-2-", FindGroup("P2") );
//  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("--2", FindGroup("P112") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("21--", FindGroup("P2111") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("-21-", FindGroup("P21") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("--21", FindGroup("P1121") );
//  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("3", FindGroup("P3") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("31", FindGroup("P31") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("32", FindGroup("P32") );
//  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("4", FindGroup("P4") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("41", FindGroup("P41") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("42", FindGroup("P42") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("43", FindGroup("P43") );
//  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("6", FindGroup("P6") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("61", FindGroup("P61") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("62", FindGroup("P62") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("63", FindGroup("P63") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("64", FindGroup("P64") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("65", FindGroup("P65") );

  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("n--", FindGroup("Pn11") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("-n-", FindGroup("Pn") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("--n", FindGroup("P11n") );

  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("-a-", FindGroup("Pa") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("--a", FindGroup("P11a") );

  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("b--", FindGroup("Pb11") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("--b", FindGroup("P11b") );

  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("c--", FindGroup("Pc11") );
  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("-c-", FindGroup("Pc") );

//  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("m--", FindGroup("Pm11") );
//  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("-m-", FindGroup("Pm") );
//  SymmetryElements.AddNew<olxstr, TSpaceGroup*>("--m", FindGroup("P11m") );

  // xonstructing glide d planes
  TMatrixD dMatt(3,4);
  TSymmElement& d1 = SymmetryElements.AddNew<olxstr>("d--");
  dMatt[0][0] = -1;  dMatt[1][1] = 1;  dMatt[2][2] = 1;
  dMatt[0][3] = 0;  dMatt[1][3] = dMatt[2][3] = 1./4.;
  d1.AddMatrix(dMatt);
  TSymmElement& d2 = SymmetryElements.AddNew<olxstr>("-d-");
  dMatt[0][0] = 1;  dMatt[1][1] = -1;  dMatt[2][2] = 1;
  dMatt[1][3] = 0;  dMatt[0][3] = dMatt[2][3] = 1./4.;
  d2.AddMatrix(dMatt);
  TSymmElement& d3 = SymmetryElements.AddNew<olxstr>("--d");
  dMatt[0][0] = 1;  dMatt[1][1] = 1;  dMatt[2][2] = -1;
  dMatt[2][3] = 0;  dMatt[0][3] = dMatt[1][3] = 1./4.;
  d3.AddMatrix(dMatt);

  PointGroups.Add( FindGroup("P1") );
  PointGroups.Add( FindGroup("P-1") );

  PointGroups.Add( FindGroup("P2") );
  PointGroups.Add( FindGroup("Pm") );
  PointGroups.Add( FindGroup("P2/m") );

  PointGroups.Add( FindGroup("P222") );
  PointGroups.Add( FindGroup("P2mm") );
  PointGroups.Add( FindGroup("Pm2m") );
  PointGroups.Add( FindGroup("Pmm2") );
  PointGroups.Add( FindGroup("Pmmm") );

  PointGroups.Add( FindGroup("P4") );
  PointGroups.Add( FindGroup("P-4") );
  PointGroups.Add( FindGroup("P4/m") );
  PointGroups.Add( FindGroup("P422") );
  PointGroups.Add( FindGroup("P4mm") );
  PointGroups.Add( FindGroup("P-42m") );
  PointGroups.Add( FindGroup("P-4m2") );
  PointGroups.Add( FindGroup("P4/mmm") );

  PointGroups.Add( FindGroup("P3") );
  PointGroups.Add( FindGroup("P-3") );
  PointGroups.Add( FindGroup("P321") );
  PointGroups.Add( FindGroup("P3m1") );
  PointGroups.Add( FindGroup("P-3m1") );
  PointGroups.Add( FindGroup("P312") );
  PointGroups.Add( FindGroup("P31m") );
  PointGroups.Add( FindGroup("P-31m") );

  PointGroups.Add( FindGroup("P6") );
  PointGroups.Add( FindGroup("P-6") );
  PointGroups.Add( FindGroup("P6/m") );
  PointGroups.Add( FindGroup("P622") );
  PointGroups.Add( FindGroup("P6mm") );
  PointGroups.Add( FindGroup("P-62m") );
  PointGroups.Add( FindGroup("P-6m2") );
  PointGroups.Add( FindGroup("P6/mmm") );

  PointGroups.Add( FindGroup("P23") );
  PointGroups.Add( FindGroup("Pm-3") );
  PointGroups.Add( FindGroup("P432") );
  PointGroups.Add( FindGroup("P-43m") );
  PointGroups.Add( FindGroup("Pm-3m") );

  InitRelations();

  Instance = this;
}
//..............................................................................
TSymmLib::~TSymmLib()
{
  for(int i=0; i < SGCount(); i++ )  delete &(GetGroup(i));
  for(int i=0; i < LatticeCount(); i++ )  delete &(GetLattice(i));
  for(int i=0; i < BravaisLatticeCount(); i++ )  delete &(GetBravaisLattice(i));
  Instance = NULL;
}
//..............................................................................
void TSymmLib::GetGroupByNumber(int N, TPtrList<TSpaceGroup>& res) const  {
  for(int i=0; i < SGCount(); i++ )
    if( GetGroup(i).GetNumber() == N )  res.Add( &GetGroup(i) );
}
//..............................................................................
TSpaceGroup* TSymmLib::FindSG(const TAsymmUnit& AU)  const {
  for(int i=0; i < SGCount(); i++ )
    if( GetGroup(i) == AU )  return &(GetGroup(i));
  return NULL;
}
TSpaceGroup* TSymmLib::FindExpandedSG(const TAsymmUnit& AU) const  {
  for(int i=0; i < SGCount(); i++ )
    if( GetGroup(i).EqualsExpandedSG(AU) )  return &(GetGroup(i));
  return NULL;
}
//..............................................................................
int TSymmLib::FindBravaisLattices(TAsymmUnit& AU, TTypeList<TBravaisLatticeRef>& res)  const {
  double Alpha = AU.Angles()[0].GetV(), Beta = AU.Angles()[1].GetV(), Gamma = AU.Angles()[2].GetV();
  double A = AU.Axes()[0].GetV(),       B = AU.Axes()[1].GetV(),      C = AU.Axes()[2].GetV();
  // alpha = beta = gamma
  if( Alpha == Beta && Alpha == Gamma )  {
    if( Alpha == 90 )  {
      if( A == B && A == C )  {
        res.AddNew<TBravaisLattice*,int>( this->FindBravaisLattice("Cubic"), 0 );
        res.AddNew<TBravaisLattice*,int>( this->FindBravaisLattice("Tetragonal"), -1 );
        res.AddNew<TBravaisLattice*,int>( this->FindBravaisLattice("Orthorhombic"), -1 );
        res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Monoclinic"), -1 );
        res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Triclinic"), -1 );
      }
      else if( A == B )  {
        res.AddNew<TBravaisLattice*,int>( this->FindBravaisLattice("Tetragonal"), 0 );
        res.AddNew<TBravaisLattice*,int>( this->FindBravaisLattice("Orthorhombic"), -1 );
        res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Monoclinic"), -1 );
        res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Triclinic"), -1 );
      }
      else  {
        res.AddNew<TBravaisLattice*,int>( this->FindBravaisLattice("Orthorhombic"), 0 );
        res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Monoclinic"), -1 );
        res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Triclinic"), -1 );
      }
    }
    else  {
      res.AddNew<TBravaisLattice*,int>( this->FindBravaisLattice("Trigonal"), 0 );
      res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Triclinic"), -1 );
    }
  }
  else if( Alpha == Beta )  {
    if( Alpha == 90 )  {
      if( Gamma == 120 )  {
        res.AddNew<TBravaisLattice*,int>( this->FindBravaisLattice("Trigonal"), 0 );
        res.AddNew<TBravaisLattice*,int>( this->FindBravaisLattice("Hexagonal"), 0 );
        res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Monoclinic"), -1 );
        res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Triclinic"), -1 );
      }
      else  {
        res.AddNew<TBravaisLattice*,int>( this->FindBravaisLattice("Monoclinic"), 0 );
        res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Triclinic"), -1 );
      }
    }
  }
  else if( Alpha == Gamma && Alpha == 90 )  {
    res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Monoclinic"), 0 );
    res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Triclinic"), -1 );
  }
  else
    res.AddNew<TBravaisLattice*,int>( FindBravaisLattice("Triclinic"), 0 );
  return 0;
}
//..............................................................................
int TSymmLib::FindLaueClassGroups(const TSpaceGroup& LaueClass, TPtrList<TSpaceGroup>& res)  const {
  int rc = 0;
  for( int i=0; i < SGCount(); i++ )  {
    if( &GetGroup(i).GetLaueClass() == &LaueClass )  {
      res.Add( &GetGroup(i) );
      rc++;
    }
  }
  return rc;
}
//..............................................................................
int TSymmLib::FindPointGroupGroups(const TSpaceGroup& PointGroup, TPtrList<TSpaceGroup>& res) const {
  int rc = 0;
  for( int i=0; i < SGCount(); i++ )  {
    if( &GetGroup(i).GetPointGroup() == &PointGroup )  {
      res.Add( &GetGroup(i) );
      rc++;
    }
  }
  return rc;
}
//..............................................................................
void TSymmLib::InitRelations()  {
  TPtrList<TSpaceGroup> allSG;
  for( int i=0; i < BravaisLatticeCount(); i++ )  {
    TBravaisLattice& bl = GetBravaisLattice(i);
    for( int j=0; j < bl.LatticeCount(); j++ )  {
      bl.GetLattice(j).AddBravaiseLattice( &bl );
    }
    for( int j=0; j < SGCount(); j++ )  {
      if( &(GetGroup(j).GetBravaisLattice()) != NULL )  continue;
      bool found = false;
      for( int k=0; k < bl.SymmetryCount(); k++ )  {
        if( bl.GetSymmetry(k).EqualsWithoutTranslation( GetGroup(j) ) )  {
          allSG.Clear();
          GetGroupByNumber( GetGroup(j).GetNumber(), allSG );
          for( int l=0; l < allSG.Count(); l++ )  {
            if(  &(allSG[l]->GetBravaisLattice()) != NULL )
              throw TFunctionFailedException(__OlxSourceInfo, "assert");
            allSG[l]->SetBravaisLattice( bl );
            allSG[l]->SetLaueClass( bl.GetSymmetry(k) );
          }
          found = true;
          break;
        }
        if( found )  break;
      }
    }
  }
  for( int i=0; i < SGCount(); i++ )  {
    if( &GetGroup(i).GetPointGroup() != NULL )  continue;
    TSpaceGroup* pg = NULL;
    for( int j=0; j < PointGroups.Count(); j++ )  {
      if( GetGroup(i).ContainsGroup( PointGroups[j] ) )  {
        pg = PointGroups[j];
        break;
      }
    }
    if( pg != NULL )  {
      allSG.Clear();
      GetGroupByNumber( GetGroup(i).GetNumber(), allSG );
      for( int j=0; j < allSG.Count(); j++ )  {
        if( &allSG[j]->GetPointGroup() != NULL )
          throw TFunctionFailedException(__OlxSourceInfo, "assert");;
        allSG[j]->SetPointGroup( *pg );
      }
    }
  }
  // test
  for( int i=0; i < SGCount(); i++ )  {
    if( &GetGroup(i).GetPointGroup() == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "assert");;
  }
}

//..............................................................................
TSymmElement*  TSymmLib::FindSymmElement(const olxstr& name)  const  {
  for( int i=0; i < SymmetryElements.Count(); i++ )
    if( SymmetryElements[i].GetName() == name )  return &SymmetryElements[i];
  return NULL;
}



