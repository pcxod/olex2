/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_henke_H
#define __olx_xl_henke_H
#include "xbase.h"
/*
  the source of data for Henke tables and scattering data is cctbx
  neutron scattering data was downloaded from this website:
  http://www.ncnr.nist.gov/resources/n-lengths/index.html
  Note from cctbx:
  The original data can be found at:
    ftp://grace.lbl.gov/pub/sf/

  From ftp://grace.lbl.gov/pub/sf/read.me:

                Low-Energy X-ray Interaction Coefficients:
                Photoabsorption, Scattering, and Reflection
                        E = 30-30,000 eV, Z = 1-92

                B. L. Henke, E. M. Gullikson, and J. C. Davis
                        Center for X-Ray Optics, 2-400
                        Lawrence Berkeley Laboratory
                        Berkeley, California 94720

  Reference: B. L. Henke, E. M. Gullikson, and J. C. Davis,
  Atomic Data and Nuclear Data Tables Vol. 54 No. 2 (July 1993).
*/

BeginXlibNamespace()

struct cm_Anomalous_Henke {
  double energy, fp, fdp;
  static const double Undefined;
};

extern const cm_Anomalous_Henke _cm_henke_H[];
extern const cm_Anomalous_Henke _cm_henke_He[];
extern const cm_Anomalous_Henke _cm_henke_Li[];
extern const cm_Anomalous_Henke _cm_henke_Be[];
extern const cm_Anomalous_Henke _cm_henke_B[];
extern const cm_Anomalous_Henke _cm_henke_C[];
extern const cm_Anomalous_Henke _cm_henke_N[];
extern const cm_Anomalous_Henke _cm_henke_O[];
extern const cm_Anomalous_Henke _cm_henke_F[];
extern const cm_Anomalous_Henke _cm_henke_Ne[];
extern const cm_Anomalous_Henke _cm_henke_Na[];
extern const cm_Anomalous_Henke _cm_henke_Mg[];
extern const cm_Anomalous_Henke _cm_henke_Al[];
extern const cm_Anomalous_Henke _cm_henke_Si[];
extern const cm_Anomalous_Henke _cm_henke_P[];
extern const cm_Anomalous_Henke _cm_henke_S[];
extern const cm_Anomalous_Henke _cm_henke_Cl[];
extern const cm_Anomalous_Henke _cm_henke_Ar[];
extern const cm_Anomalous_Henke _cm_henke_K[];
extern const cm_Anomalous_Henke _cm_henke_Ca[];
extern const cm_Anomalous_Henke _cm_henke_Sc[];
extern const cm_Anomalous_Henke _cm_henke_Ti[];
extern const cm_Anomalous_Henke _cm_henke_V[];
extern const cm_Anomalous_Henke _cm_henke_Cr[];
extern const cm_Anomalous_Henke _cm_henke_Mn[];
extern const cm_Anomalous_Henke _cm_henke_Fe[];
extern const cm_Anomalous_Henke _cm_henke_Co[];
extern const cm_Anomalous_Henke _cm_henke_Ni[];
extern const cm_Anomalous_Henke _cm_henke_Cu[];
extern const cm_Anomalous_Henke _cm_henke_Zn[];
extern const cm_Anomalous_Henke _cm_henke_Ga[];
extern const cm_Anomalous_Henke _cm_henke_Ge[];
extern const cm_Anomalous_Henke _cm_henke_As[];
extern const cm_Anomalous_Henke _cm_henke_Se[];
extern const cm_Anomalous_Henke _cm_henke_Br[];
extern const cm_Anomalous_Henke _cm_henke_Kr[];
extern const cm_Anomalous_Henke _cm_henke_Rb[];
extern const cm_Anomalous_Henke _cm_henke_Sr[];
extern const cm_Anomalous_Henke _cm_henke_Y[];
extern const cm_Anomalous_Henke _cm_henke_Zr[];
extern const cm_Anomalous_Henke _cm_henke_Nb[];
extern const cm_Anomalous_Henke _cm_henke_Mo[];
extern const cm_Anomalous_Henke _cm_henke_Tc[];
extern const cm_Anomalous_Henke _cm_henke_Ru[];
extern const cm_Anomalous_Henke _cm_henke_Rh[];
extern const cm_Anomalous_Henke _cm_henke_Pd[];
extern const cm_Anomalous_Henke _cm_henke_Ag[];
extern const cm_Anomalous_Henke _cm_henke_Cd[];
extern const cm_Anomalous_Henke _cm_henke_In[];
extern const cm_Anomalous_Henke _cm_henke_Sn[];
extern const cm_Anomalous_Henke _cm_henke_Sb[];
extern const cm_Anomalous_Henke _cm_henke_Te[];
extern const cm_Anomalous_Henke _cm_henke_I[];
extern const cm_Anomalous_Henke _cm_henke_Xe[];
extern const cm_Anomalous_Henke _cm_henke_Cs[];
extern const cm_Anomalous_Henke _cm_henke_Ba[];
extern const cm_Anomalous_Henke _cm_henke_La[];
extern const cm_Anomalous_Henke _cm_henke_Ce[];
extern const cm_Anomalous_Henke _cm_henke_Pr[];
extern const cm_Anomalous_Henke _cm_henke_Nd[];
extern const cm_Anomalous_Henke _cm_henke_Pm[];
extern const cm_Anomalous_Henke _cm_henke_Sm[];
extern const cm_Anomalous_Henke _cm_henke_Eu[];
extern const cm_Anomalous_Henke _cm_henke_Gd[];
extern const cm_Anomalous_Henke _cm_henke_Tb[];
extern const cm_Anomalous_Henke _cm_henke_Dy[];
extern const cm_Anomalous_Henke _cm_henke_Ho[];
extern const cm_Anomalous_Henke _cm_henke_Er[];
extern const cm_Anomalous_Henke _cm_henke_Tm[];
extern const cm_Anomalous_Henke _cm_henke_Yb[];
extern const cm_Anomalous_Henke _cm_henke_Lu[];
extern const cm_Anomalous_Henke _cm_henke_Hf[];
extern const cm_Anomalous_Henke _cm_henke_Ta[];
extern const cm_Anomalous_Henke _cm_henke_W[];
extern const cm_Anomalous_Henke _cm_henke_Re[];
extern const cm_Anomalous_Henke _cm_henke_Os[];
extern const cm_Anomalous_Henke _cm_henke_Ir[];
extern const cm_Anomalous_Henke _cm_henke_Pt[];
extern const cm_Anomalous_Henke _cm_henke_Au[];
extern const cm_Anomalous_Henke _cm_henke_Hg[];
extern const cm_Anomalous_Henke _cm_henke_Tl[];
extern const cm_Anomalous_Henke _cm_henke_Pb[];
extern const cm_Anomalous_Henke _cm_henke_Bi[];
extern const cm_Anomalous_Henke _cm_henke_Po[];
extern const cm_Anomalous_Henke _cm_henke_At[];
extern const cm_Anomalous_Henke _cm_henke_Rn[];
extern const cm_Anomalous_Henke _cm_henke_Fr[];
extern const cm_Anomalous_Henke _cm_henke_Ra[];
extern const cm_Anomalous_Henke _cm_henke_Ac[];
extern const cm_Anomalous_Henke _cm_henke_Th[];
extern const cm_Anomalous_Henke _cm_henke_Pa[];
extern const cm_Anomalous_Henke _cm_henke_U[];
extern const cm_Anomalous_Henke _cm_henke_D[];

EndXlibNamespace()
#endif
