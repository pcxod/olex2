/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "absorpc.h"
#include "math/spline.h"

cm_Absorption_Coefficient_Reg::cm_Absorption_Coefficient_Reg()  {
  entries.Add("H", _cm_absorpc_H);
  entries.Add("He", _cm_absorpc_He);
  entries.Add("Li", _cm_absorpc_Li);
  entries.Add("Be", _cm_absorpc_Be);
  entries.Add("B", _cm_absorpc_B);
  entries.Add("C", _cm_absorpc_C);
  entries.Add("N", _cm_absorpc_N);
  entries.Add("O", _cm_absorpc_O);
  entries.Add("F", _cm_absorpc_F);
  entries.Add("Ne", _cm_absorpc_Ne);
  entries.Add("Na", _cm_absorpc_Na);
  entries.Add("Mg", _cm_absorpc_Mg);
  entries.Add("Al", _cm_absorpc_Al);
  entries.Add("Si", _cm_absorpc_Si);
  entries.Add("P", _cm_absorpc_P);
  entries.Add("S", _cm_absorpc_S);
  entries.Add("Cl", _cm_absorpc_Cl);
  entries.Add("Ar", _cm_absorpc_Ar);
  entries.Add("K", _cm_absorpc_K);
  entries.Add("Ca", _cm_absorpc_Ca);
  entries.Add("Sc", _cm_absorpc_Sc);
  entries.Add("Ti", _cm_absorpc_Ti);
  entries.Add("V", _cm_absorpc_V);
  entries.Add("Cr", _cm_absorpc_Cr);
  entries.Add("Mn", _cm_absorpc_Mn);
  entries.Add("Fe", _cm_absorpc_Fe);
  entries.Add("Co", _cm_absorpc_Co);
  entries.Add("Ni", _cm_absorpc_Ni);
  entries.Add("Cu", _cm_absorpc_Cu);
  entries.Add("Zn", _cm_absorpc_Zn);
  entries.Add("Ga", _cm_absorpc_Ga);
  entries.Add("Ge", _cm_absorpc_Ge);
  entries.Add("As", _cm_absorpc_As);
  entries.Add("Se", _cm_absorpc_Se);
  entries.Add("Br", _cm_absorpc_Br);
  entries.Add("Kr", _cm_absorpc_Kr);
  entries.Add("Rb", _cm_absorpc_Rb);
  entries.Add("Sr", _cm_absorpc_Sr);
  entries.Add("Y", _cm_absorpc_Y);
  entries.Add("Zr", _cm_absorpc_Zr);
  entries.Add("Nb", _cm_absorpc_Nb);
  entries.Add("Mo", _cm_absorpc_Mo);
  entries.Add("Tc", _cm_absorpc_Tc);
  entries.Add("Ru", _cm_absorpc_Ru);
  entries.Add("Rh", _cm_absorpc_Rh);
  entries.Add("Pd", _cm_absorpc_Pd);
  entries.Add("Ag", _cm_absorpc_Ag);
  entries.Add("Cd", _cm_absorpc_Cd);
  entries.Add("In", _cm_absorpc_In);
  entries.Add("Sn", _cm_absorpc_Sn);
  entries.Add("Sb", _cm_absorpc_Sb);
  entries.Add("Te", _cm_absorpc_Te);
  entries.Add("I", _cm_absorpc_I);
  entries.Add("Xe", _cm_absorpc_Xe);
  entries.Add("Cs", _cm_absorpc_Cs);
  entries.Add("Ba", _cm_absorpc_Ba);
  entries.Add("La", _cm_absorpc_La);
  entries.Add("Ce", _cm_absorpc_Ce);
  entries.Add("Pr", _cm_absorpc_Pr);
  entries.Add("Nd", _cm_absorpc_Nd);
  entries.Add("Pm", _cm_absorpc_Pm);
  entries.Add("Sm", _cm_absorpc_Sm);
  entries.Add("Eu", _cm_absorpc_Eu);
  entries.Add("Gd", _cm_absorpc_Gd);
  entries.Add("Tb", _cm_absorpc_Tb);
  entries.Add("Dy", _cm_absorpc_Dy);
  entries.Add("Ho", _cm_absorpc_Ho);
  entries.Add("Er", _cm_absorpc_Er);
  entries.Add("Tm", _cm_absorpc_Tm);
  entries.Add("Yb", _cm_absorpc_Yb);
  entries.Add("Lu", _cm_absorpc_Lu);
  entries.Add("Hf", _cm_absorpc_Hf);
  entries.Add("Ta", _cm_absorpc_Ta);
  entries.Add("W", _cm_absorpc_W);
  entries.Add("Re", _cm_absorpc_Re);
  entries.Add("Os", _cm_absorpc_Os);
  entries.Add("Ir", _cm_absorpc_Ir);
  entries.Add("Pt", _cm_absorpc_Pt);
  entries.Add("Au", _cm_absorpc_Au);
  entries.Add("Hg", _cm_absorpc_Hg);
  entries.Add("Tl", _cm_absorpc_Tl);
  entries.Add("Pb", _cm_absorpc_Pb);
  entries.Add("Bi", _cm_absorpc_Bi);
  entries.Add("Po", _cm_absorpc_Po);
  entries.Add("At", _cm_absorpc_At);
  entries.Add("Rn", _cm_absorpc_Rn);
  entries.Add("Fr", _cm_absorpc_Fr);
  entries.Add("Ra", _cm_absorpc_Ra);
  entries.Add("Ac", _cm_absorpc_Ac);
  entries.Add("Th", _cm_absorpc_Th);
  entries.Add("Pa", _cm_absorpc_Pa);
  entries.Add("U", _cm_absorpc_U);
  entries.Add("Np", _cm_absorpc_Np);
  entries.Add("Pu", _cm_absorpc_Pu);
  entries.Add("Am", _cm_absorpc_Am);
  entries.Add("Cm", _cm_absorpc_Cm);
  entries.Add("Bk", _cm_absorpc_Bk);
  entries.Add("Cf", _cm_absorpc_Cf);
}

double cm_Absorption_Coefficient_Reg::_CalcForE(double eV,
  cm_Absorption_Entry& ac, double (cm_Absorption_Coefficient::*f)() const) const
{
  eV /= 1e6;
  double t_ev = eV;
  long k = 1000; // 'cut' energy to 3 significant digits
  while( int(t_ev) == 0 )  {
    t_ev *= 10;
    k *= 10;
  }
  while( int (t_ev) > 10 )  {
    t_ev /= 10;
    k /= 10;
  }
  if( k != 0 )
    eV = double((long)(eV*k))/k;
  if( ac.size == 0 )  {  // update the size
    const cm_Absorption_Coefficient* _ac = ac.data;
    while( (++_ac)->energy != 0 )
      ac.size++;
  }
  if (eV < ac.data[0].energy || eV > ac.data[ac.size].energy) {
    throw TInvalidArgumentException(__OlxSourceInfo, "energy is out of range");
  }
  if( eV == ac.data[0].energy )
    return (ac.data[0].*f)();
  size_t i_start = 0, i_end = ac.size-1;
  while( i_start != i_end-1 )  {
    const size_t mid = (i_end+i_start)/2;
    if( ac.data[mid].energy > eV )
      i_end = mid;
    else
      i_start = mid;
  }
  // matches the entry?
  if( olx_abs(ac.data[i_start].energy-eV) < 1e-3/k )
    return (ac.data[i_start].*f)();
  TTypeList<olx_pair_t<double,double> > left, right;
  // go left
  for( size_t i=i_start-1; i != InvalidIndex; i-- )  {
    const double v = (ac.data[i].*f)();
    if( v == 0 )  continue;  // no value
    // absorption edge
    if( !left.IsEmpty() && left.GetLast().GetA() == ac.data[i].energy )
      break;
    left.AddNew(ac.data[i].energy, v);
    if( left.Count() > 3 )
      break;
  }
  // go right
  for( size_t i=i_start+1; i < ac.size; i++ )  {
    const double v = (ac.data[i].*f)();
    if( v == 0 )  continue;  // no value
    // absorption edge
    if( !right.IsEmpty() && right.GetLast().GetA() == ac.data[i].energy )
      break;
    right.AddNew(ac.data[i].energy, v);
    if( right.Count() > 3 )
      break;
  }
  if( (left.Count()+right.Count()) >= 1 )  {  // use spline interpolation
    math::spline::Spline3<double> s;
    s.x.Resize(left.Count()+right.Count()+1);
    s.y.Resize(s.x.Count());
    size_t index = 0;
    for( size_t i=0; i < left.Count(); i++, index++ )  {
      s.x[index] = left[i].GetA();
      s.y[index] = left[i].GetB();
    }
    s.x[index] = ac.data[i_start].energy;
    s.y[index++] = (ac.data[i_start].*f)();
    for( size_t i=0; i < right.Count(); i++, index++ )  {
      s.x[index] = right[i].GetA();
      s.y[index] = right[i].GetB();
    }
    if( s.x.Count() >= 5 )  // Akima
      return math::spline::Builder<double>::akima(s).interpolate(eV);
    else if( s.x.Count() >= 3 ) // Ctmull-Rom
      return math::spline::Builder<double>::catmull_rom(
        s, math::spline::boundary_type_parabolic, 0.5).interpolate(eV);
    else  // linear
      return math::spline::Builder<double>::linear(s).interpolate(eV);
  }
  // most probably on the absorption edge... other option - throw an exception...
  return (ac.data[i_start].*f)();
}
