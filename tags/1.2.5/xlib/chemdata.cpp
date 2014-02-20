/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "chemdata.h"

const double cm_Anomalous_Henke::Undefined = -999.0;

static const cm_Neutron_Scattering _cm_neutron_H(-3.739, 0, 0.3326);
static const cm_Neutron_Scattering _cm_neutron_H1(-3.7406, 0, 0.3326);
static const cm_Neutron_Scattering _cm_neutron_H2(6.671, 0, 0.000519);
static const cm_Neutron_Scattering _cm_neutron_He(3.26, 0, 0.00747);
static const cm_Neutron_Scattering _cm_neutron_He3(5.74, -1.483, 5333);
static const cm_Neutron_Scattering _cm_neutron_He4(3.26, 0, 0);
static const cm_Neutron_Scattering _cm_neutron_Li(-1.9, 0, 70.5);
static const cm_Neutron_Scattering _cm_neutron_Li6(2, -0.261, 940);
static const cm_Neutron_Scattering _cm_neutron_Li7(-2.22, 0, 0.0454);
static const cm_Neutron_Scattering _cm_neutron_Be(7.79, 0, 0.0076);
static const cm_Neutron_Scattering _cm_neutron_B(5.3, -0.213, 767);
static const cm_Neutron_Scattering _cm_neutron_B10(-0.1, -1.066, 3835);
static const cm_Neutron_Scattering _cm_neutron_B11(6.65, 0, 0.0055);
static const cm_Neutron_Scattering _cm_neutron_C(6.646, 0, 0.0035);
static const cm_Neutron_Scattering _cm_neutron_C12(6.6511, 0, 0.00353);
static const cm_Neutron_Scattering _cm_neutron_C13(6.19, 0, 0.00137);
static const cm_Neutron_Scattering _cm_neutron_N(9.36, 0, 1.9);
static const cm_Neutron_Scattering _cm_neutron_N14(9.37, 0, 1.91);
static const cm_Neutron_Scattering _cm_neutron_N15(6.44, 0, 0.000024);
static const cm_Neutron_Scattering _cm_neutron_O(5.803, 0, 0.00019);
static const cm_Neutron_Scattering _cm_neutron_O16(5.803, 0, 0.0001);
static const cm_Neutron_Scattering _cm_neutron_O17(5.78, 0, 0.236);
static const cm_Neutron_Scattering _cm_neutron_O18(5.84, 0, 0.00016);
static const cm_Neutron_Scattering _cm_neutron_F(5.654, 0, 0.0096);
static const cm_Neutron_Scattering _cm_neutron_Ne(4.566, 0, 0.039);
static const cm_Neutron_Scattering _cm_neutron_Ne20(4.631, 0, 0.036);
static const cm_Neutron_Scattering _cm_neutron_Ne21(6.66, 0, 0.67);
static const cm_Neutron_Scattering _cm_neutron_Ne22(3.87, 0, 0.046);
static const cm_Neutron_Scattering _cm_neutron_Na(3.63, 0, 0.53);
static const cm_Neutron_Scattering _cm_neutron_Mg(5.375, 0, 0.063);
static const cm_Neutron_Scattering _cm_neutron_Mg24(5.66, 0, 0.05);
static const cm_Neutron_Scattering _cm_neutron_Mg25(3.62, 0, 0.19);
static const cm_Neutron_Scattering _cm_neutron_Mg26(4.89, 0, 0.0382);
static const cm_Neutron_Scattering _cm_neutron_Al(3.449, 0, 0.231);
static const cm_Neutron_Scattering _cm_neutron_Si(4.1491, 0, 0.171);
static const cm_Neutron_Scattering _cm_neutron_Si28(4.107, 0, 0.177);
static const cm_Neutron_Scattering _cm_neutron_Si29(4.7, 0, 0.101);
static const cm_Neutron_Scattering _cm_neutron_Si30(4.58, 0, 0.107);
static const cm_Neutron_Scattering _cm_neutron_P(5.13, 0, 0.172);
static const cm_Neutron_Scattering _cm_neutron_S(2.847, 0, 0.53);
static const cm_Neutron_Scattering _cm_neutron_S32(2.804, 0, 0.54);
static const cm_Neutron_Scattering _cm_neutron_S33(4.74, 0, 0.54);
static const cm_Neutron_Scattering _cm_neutron_S34(3.48, 0, 0.227);
static const cm_Neutron_Scattering _cm_neutron_S36(3, 0, 0.15);
static const cm_Neutron_Scattering _cm_neutron_Cl(9.577, 0, 33.5);
static const cm_Neutron_Scattering _cm_neutron_Cl35(11.65, 0, 44.1);
static const cm_Neutron_Scattering _cm_neutron_Cl37(3.08, 0, 0.433);
static const cm_Neutron_Scattering _cm_neutron_Ar(1.909, 0, 0.675);
static const cm_Neutron_Scattering _cm_neutron_Ar36(24.9, 0, 5.2);
static const cm_Neutron_Scattering _cm_neutron_Ar38(3.5, 0, 0.8);
static const cm_Neutron_Scattering _cm_neutron_Ar40(1.83, 0, 0.66);
static const cm_Neutron_Scattering _cm_neutron_K(3.67, 0, 2.1);
static const cm_Neutron_Scattering _cm_neutron_K39(3.74, 0, 2.1);
static const cm_Neutron_Scattering _cm_neutron_K40(3, 0, 35);
static const cm_Neutron_Scattering _cm_neutron_K41(2.69, 0, 1.46);
static const cm_Neutron_Scattering _cm_neutron_Ca(4.7, 0, 0.43);
static const cm_Neutron_Scattering _cm_neutron_Ca40(4.8, 0, 0.41);
static const cm_Neutron_Scattering _cm_neutron_Ca42(3.36, 0, 0.68);
static const cm_Neutron_Scattering _cm_neutron_Ca43(-1.56, 0, 6.2);
static const cm_Neutron_Scattering _cm_neutron_Ca44(1.42, 0, 0.88);
static const cm_Neutron_Scattering _cm_neutron_Ca46(3.6, 0, 0.74);
static const cm_Neutron_Scattering _cm_neutron_Ca48(0.39, 0, 1.09);
static const cm_Neutron_Scattering _cm_neutron_Sc(12.29, 0, 27.5);
static const cm_Neutron_Scattering _cm_neutron_Ti(-3.438, 0, 6.09);
static const cm_Neutron_Scattering _cm_neutron_Ti46(4.93, 0, 0.59);
static const cm_Neutron_Scattering _cm_neutron_Ti47(3.63, 0, 1.7);
static const cm_Neutron_Scattering _cm_neutron_Ti48(-6.08, 0, 7.84);
static const cm_Neutron_Scattering _cm_neutron_Ti49(1.04, 0, 2.2);
static const cm_Neutron_Scattering _cm_neutron_Ti50(6.18, 0, 0.179);
static const cm_Neutron_Scattering _cm_neutron_V(-0.3824, 0, 5.08);
static const cm_Neutron_Scattering _cm_neutron_V50(7.6, 0, 60);
static const cm_Neutron_Scattering _cm_neutron_V51(-0.402, 0, 4.9);
static const cm_Neutron_Scattering _cm_neutron_Cr(3.635, 0, 3.05);
static const cm_Neutron_Scattering _cm_neutron_Cr50(-4.5, 0, 15.8);
static const cm_Neutron_Scattering _cm_neutron_Cr52(4.92, 0, 0.76);
static const cm_Neutron_Scattering _cm_neutron_Cr53(-4.2, 0, 18.1);
static const cm_Neutron_Scattering _cm_neutron_Cr54(4.55, 0, 0.36);
static const cm_Neutron_Scattering _cm_neutron_Mn(-3.73, 0, 13.3);
static const cm_Neutron_Scattering _cm_neutron_Fe(9.45, 0, 2.56);
static const cm_Neutron_Scattering _cm_neutron_Fe54(4.2, 0, 2.25);
static const cm_Neutron_Scattering _cm_neutron_Fe56(9.94, 0, 2.59);
static const cm_Neutron_Scattering _cm_neutron_Fe57(2.3, 0, 2.48);
static const cm_Neutron_Scattering _cm_neutron_Fe58(15, 0, 1.28);
static const cm_Neutron_Scattering _cm_neutron_Co(2.49, 0, 37.18);
static const cm_Neutron_Scattering _cm_neutron_Ni(10.3, 0, 4.49);
static const cm_Neutron_Scattering _cm_neutron_Ni58(14.4, 0, 4.6);
static const cm_Neutron_Scattering _cm_neutron_Ni60(2.8, 0, 2.9);
static const cm_Neutron_Scattering _cm_neutron_Ni61(7.6, 0, 2.5);
static const cm_Neutron_Scattering _cm_neutron_Ni62(-8.7, 0, 14.5);
static const cm_Neutron_Scattering _cm_neutron_Ni64(-0.37, 0, 1.52);
static const cm_Neutron_Scattering _cm_neutron_Cu(7.718, 0, 3.78);
static const cm_Neutron_Scattering _cm_neutron_Cu63(6.43, 0, 4.5);
static const cm_Neutron_Scattering _cm_neutron_Cu65(10.61, 0, 2.17);
static const cm_Neutron_Scattering _cm_neutron_Zn(5.68, 0, 1.11);
static const cm_Neutron_Scattering _cm_neutron_Zn64(5.22, 0, 0.93);
static const cm_Neutron_Scattering _cm_neutron_Zn66(5.97, 0, 0.62);
static const cm_Neutron_Scattering _cm_neutron_Zn67(7.56, 0, 6.8);
static const cm_Neutron_Scattering _cm_neutron_Zn68(6.03, 0, 1.1);
static const cm_Neutron_Scattering _cm_neutron_Zn70(6, 0, 0.092);
static const cm_Neutron_Scattering _cm_neutron_Ga(7.288, 0, 2.75);
static const cm_Neutron_Scattering _cm_neutron_Ga69(7.88, 0, 2.18);
static const cm_Neutron_Scattering _cm_neutron_Ga71(6.4, 0, 3.61);
static const cm_Neutron_Scattering _cm_neutron_Ge(8.185, 0, 2.2);
static const cm_Neutron_Scattering _cm_neutron_Ge70(10, 0, 3);
static const cm_Neutron_Scattering _cm_neutron_Ge72(8.51, 0, 0.8);
static const cm_Neutron_Scattering _cm_neutron_Ge73(5.02, 0, 15.1);
static const cm_Neutron_Scattering _cm_neutron_Ge74(7.58, 0, 0.4);
static const cm_Neutron_Scattering _cm_neutron_Ge76(8.2, 0, 0.16);
static const cm_Neutron_Scattering _cm_neutron_As(6.58, 0, 4.5);
static const cm_Neutron_Scattering _cm_neutron_Se(7.97, 0, 11.7);
static const cm_Neutron_Scattering _cm_neutron_Se74(0.8, 0, 51.8);
static const cm_Neutron_Scattering _cm_neutron_Se76(12.2, 0, 85);
static const cm_Neutron_Scattering _cm_neutron_Se77(8.25, 0, 42);
static const cm_Neutron_Scattering _cm_neutron_Se78(8.24, 0, 0.43);
static const cm_Neutron_Scattering _cm_neutron_Se80(7.48, 0, 0.61);
static const cm_Neutron_Scattering _cm_neutron_Se82(6.34, 0, 0.044);
static const cm_Neutron_Scattering _cm_neutron_Br(6.795, 0, 6.9);
static const cm_Neutron_Scattering _cm_neutron_Br79(6.8, 0, 11);
static const cm_Neutron_Scattering _cm_neutron_Br81(6.79, 0, 2.7);
static const cm_Neutron_Scattering _cm_neutron_Kr(7.81, 0, 25);
static const cm_Neutron_Scattering _cm_neutron_Kr86(8.1, 0, 0.003);
static const cm_Neutron_Scattering _cm_neutron_Rb(7.09, 0, 0.38);
static const cm_Neutron_Scattering _cm_neutron_Rb85(7.03, 0, 0.48);
static const cm_Neutron_Scattering _cm_neutron_Rb87(7.23, 0, 0.12);
static const cm_Neutron_Scattering _cm_neutron_Sr(7.02, 0, 1.28);
static const cm_Neutron_Scattering _cm_neutron_Sr84(7, 0, 0.87);
static const cm_Neutron_Scattering _cm_neutron_Sr86(5.67, 0, 1.04);
static const cm_Neutron_Scattering _cm_neutron_Sr87(7.4, 0, 16);
static const cm_Neutron_Scattering _cm_neutron_Sr88(7.15, 0, 0.058);
static const cm_Neutron_Scattering _cm_neutron_Y(7.75, 0, 1.28);
static const cm_Neutron_Scattering _cm_neutron_Zr(7.16, 0, 0.185);
static const cm_Neutron_Scattering _cm_neutron_Zr90(6.4, 0, 0.011);
static const cm_Neutron_Scattering _cm_neutron_Zr91(8.7, 0, 1.17);
static const cm_Neutron_Scattering _cm_neutron_Zr92(7.4, 0, 0.22);
static const cm_Neutron_Scattering _cm_neutron_Zr94(8.2, 0, 0.0499);
static const cm_Neutron_Scattering _cm_neutron_Zr96(5.5, 0, 0.0229);
static const cm_Neutron_Scattering _cm_neutron_Nb(7.054, 0, 1.15);
static const cm_Neutron_Scattering _cm_neutron_Mo(6.715, 0, 2.48);
static const cm_Neutron_Scattering _cm_neutron_Mo92(6.91, 0, 0.019);
static const cm_Neutron_Scattering _cm_neutron_Mo94(6.8, 0, 0.015);
static const cm_Neutron_Scattering _cm_neutron_Mo95(6.91, 0, 13.1);
static const cm_Neutron_Scattering _cm_neutron_Mo96(6.2, 0, 0.5);
static const cm_Neutron_Scattering _cm_neutron_Mo97(7.24, 0, 2.5);
static const cm_Neutron_Scattering _cm_neutron_Mo98(6.58, 0, 0.127);
static const cm_Neutron_Scattering _cm_neutron_Mo100(6.73, 0, 0.4);
static const cm_Neutron_Scattering _cm_neutron_Tc(6.8, 0, 20);
static const cm_Neutron_Scattering _cm_neutron_Ru(7.03, 0, 2.56);
static const cm_Neutron_Scattering _cm_neutron_Rh(5.88, 0, 144.8);
static const cm_Neutron_Scattering _cm_neutron_Pd(5.91, 0, 6.9);
static const cm_Neutron_Scattering _cm_neutron_Pd102(7.7, 0, 3.4);
static const cm_Neutron_Scattering _cm_neutron_Pd104(7.7, 0, 0.6);
static const cm_Neutron_Scattering _cm_neutron_Pd105(5.5, 0, 20);
static const cm_Neutron_Scattering _cm_neutron_Pd106(6.4, 0, 0.304);
static const cm_Neutron_Scattering _cm_neutron_Pd108(4.1, 0, 8.55);
static const cm_Neutron_Scattering _cm_neutron_Pd110(7.7, 0, 0.226);
static const cm_Neutron_Scattering _cm_neutron_Ag(5.922, 0, 63.3);
static const cm_Neutron_Scattering _cm_neutron_Ag107(7.555, 0, 37.6);
static const cm_Neutron_Scattering _cm_neutron_Ag109(4.165, 0, 91);
static const cm_Neutron_Scattering _cm_neutron_Cd(4.87, -0.7, 2520);
static const cm_Neutron_Scattering _cm_neutron_Cd106(5, 0, 1);
static const cm_Neutron_Scattering _cm_neutron_Cd108(5.4, 0, 1.1);
static const cm_Neutron_Scattering _cm_neutron_Cd110(5.9, 0, 11);
static const cm_Neutron_Scattering _cm_neutron_Cd111(6.5, 0, 24);
static const cm_Neutron_Scattering _cm_neutron_Cd112(6.4, 0, 2.2);
static const cm_Neutron_Scattering _cm_neutron_Cd113(-8, -5.73, 20600);
static const cm_Neutron_Scattering _cm_neutron_Cd114(7.5, 0, 0.34);
static const cm_Neutron_Scattering _cm_neutron_Cd116(6.3, 0, 0.075);
static const cm_Neutron_Scattering _cm_neutron_In(4.065, -0.0539, 193.8);
static const cm_Neutron_Scattering _cm_neutron_In113(5.39, 0, 12);
static const cm_Neutron_Scattering _cm_neutron_In115(4.01, -0.0562, 202);
static const cm_Neutron_Scattering _cm_neutron_Sn(6.225, 0, 0.626);
static const cm_Neutron_Scattering _cm_neutron_Sn112(6, 0, 1);
static const cm_Neutron_Scattering _cm_neutron_Sn114(6.2, 0, 0.114);
static const cm_Neutron_Scattering _cm_neutron_Sn115(6, 0, 30);
static const cm_Neutron_Scattering _cm_neutron_Sn116(5.93, 0, 0.14);
static const cm_Neutron_Scattering _cm_neutron_Sn117(6.48, 0, 2.3);
static const cm_Neutron_Scattering _cm_neutron_Sn118(6.07, 0, 0.22);
static const cm_Neutron_Scattering _cm_neutron_Sn119(6.12, 0, 2.2);
static const cm_Neutron_Scattering _cm_neutron_Sn120(6.49, 0, 0.14);
static const cm_Neutron_Scattering _cm_neutron_Sn122(5.74, 0, 0.18);
static const cm_Neutron_Scattering _cm_neutron_Sn124(5.97, 0, 0.133);
static const cm_Neutron_Scattering _cm_neutron_Sb(5.57, 0, 4.91);
static const cm_Neutron_Scattering _cm_neutron_Sb121(5.71, 0, 5.75);
static const cm_Neutron_Scattering _cm_neutron_Sb123(5.38, 0, 3.8);
static const cm_Neutron_Scattering _cm_neutron_Te(5.8, 0, 4.7);
static const cm_Neutron_Scattering _cm_neutron_Te120(5.3, 0, 2.3);
static const cm_Neutron_Scattering _cm_neutron_Te122(3.8, 0, 3.4);
static const cm_Neutron_Scattering _cm_neutron_Te123(-0.05, -0.116, 418);
static const cm_Neutron_Scattering _cm_neutron_Te124(7.96, 0, 6.8);
static const cm_Neutron_Scattering _cm_neutron_Te125(5.02, 0, 1.55);
static const cm_Neutron_Scattering _cm_neutron_Te126(5.56, 0, 1.04);
static const cm_Neutron_Scattering _cm_neutron_Te128(5.89, 0, 0.215);
static const cm_Neutron_Scattering _cm_neutron_Te130(6.02, 0, 0.29);
static const cm_Neutron_Scattering _cm_neutron_I(5.28, 0, 6.15);
static const cm_Neutron_Scattering _cm_neutron_Xe(4.92, 0, 23.9);
static const cm_Neutron_Scattering _cm_neutron_Cs(5.42, 0, 29);
static const cm_Neutron_Scattering _cm_neutron_Ba(5.07, 0, 1.1);
static const cm_Neutron_Scattering _cm_neutron_Ba130(-3.6, 0, 30);
static const cm_Neutron_Scattering _cm_neutron_Ba132(7.8, 0, 7);
static const cm_Neutron_Scattering _cm_neutron_Ba134(5.7, 0, 2);
static const cm_Neutron_Scattering _cm_neutron_Ba135(4.67, 0, 5.8);
static const cm_Neutron_Scattering _cm_neutron_Ba136(4.91, 0, 0.68);
static const cm_Neutron_Scattering _cm_neutron_Ba137(6.83, 0, 3.6);
static const cm_Neutron_Scattering _cm_neutron_Ba138(4.84, 0, 0.27);
static const cm_Neutron_Scattering _cm_neutron_La(8.24, 0, 8.97);
static const cm_Neutron_Scattering _cm_neutron_La138(8, 0, 57);
static const cm_Neutron_Scattering _cm_neutron_La139(8.24, 0, 8.93);
static const cm_Neutron_Scattering _cm_neutron_Ce(4.84, 0, 0.63);
static const cm_Neutron_Scattering _cm_neutron_Ce136(5.8, 0, 7.3);
static const cm_Neutron_Scattering _cm_neutron_Ce138(6.7, 0, 1.1);
static const cm_Neutron_Scattering _cm_neutron_Ce140(4.84, 0, 0.57);
static const cm_Neutron_Scattering _cm_neutron_Ce142(4.75, 0, 0.95);
static const cm_Neutron_Scattering _cm_neutron_Pr(4.58, 0, 11.5);
static const cm_Neutron_Scattering _cm_neutron_Nd(7.69, 0, 50.5);
static const cm_Neutron_Scattering _cm_neutron_Nd142(7.7, 0, 18.7);
static const cm_Neutron_Scattering _cm_neutron_Nd143(14, 0, 337);
static const cm_Neutron_Scattering _cm_neutron_Nd144(2.8, 0, 3.6);
static const cm_Neutron_Scattering _cm_neutron_Nd145(14, 0, 42);
static const cm_Neutron_Scattering _cm_neutron_Nd146(8.7, 0, 1.4);
static const cm_Neutron_Scattering _cm_neutron_Nd148(5.7, 0, 2.5);
static const cm_Neutron_Scattering _cm_neutron_Nd150(5.3, 0, 1.2);
static const cm_Neutron_Scattering _cm_neutron_Pm(12.6, 0, 168.4);
static const cm_Neutron_Scattering _cm_neutron_Sm(0.8, -1.65, 5922);
static const cm_Neutron_Scattering _cm_neutron_Sm144(-3, 0, 0.7);
static const cm_Neutron_Scattering _cm_neutron_Sm147(14, 0, 57);
static const cm_Neutron_Scattering _cm_neutron_Sm148(-3, 0, 2.4);
static const cm_Neutron_Scattering _cm_neutron_Sm149(-19.2, -11.7, 42080);
static const cm_Neutron_Scattering _cm_neutron_Sm150(14, 0, 104);
static const cm_Neutron_Scattering _cm_neutron_Sm152(-5, 0, 206);
static const cm_Neutron_Scattering _cm_neutron_Sm154(9.3, 0, 8.4);
static const cm_Neutron_Scattering _cm_neutron_Eu(7.22, -1.26, 4530);
static const cm_Neutron_Scattering _cm_neutron_Eu151(6.13, -2.53, 9100);
static const cm_Neutron_Scattering _cm_neutron_Eu153(8.22, 0, 312);
static const cm_Neutron_Scattering _cm_neutron_Gd(6.5, -13.82, 49700);
static const cm_Neutron_Scattering _cm_neutron_Gd152(10, 0, 735);
static const cm_Neutron_Scattering _cm_neutron_Gd154(10, 0, 85);
static const cm_Neutron_Scattering _cm_neutron_Gd155(6, -17, 61100);
static const cm_Neutron_Scattering _cm_neutron_Gd156(6.3, 0, 1.5);
static const cm_Neutron_Scattering _cm_neutron_Gd157(-1.14, -71.9, 259000);
static const cm_Neutron_Scattering _cm_neutron_Gd158(9, 0, 2.2);
static const cm_Neutron_Scattering _cm_neutron_Gd160(9.15, 0, 0.77);
static const cm_Neutron_Scattering _cm_neutron_Tb(7.38, 0, 23.4);
static const cm_Neutron_Scattering _cm_neutron_Dy(16.9, -0.276, 994);
static const cm_Neutron_Scattering _cm_neutron_Dy156(6.1, 0, 33);
static const cm_Neutron_Scattering _cm_neutron_Dy158(6, 0, 43);
static const cm_Neutron_Scattering _cm_neutron_Dy160(6.7, 0, 56);
static const cm_Neutron_Scattering _cm_neutron_Dy161(10.3, 0, 600);
static const cm_Neutron_Scattering _cm_neutron_Dy162(-1.4, 0, 194);
static const cm_Neutron_Scattering _cm_neutron_Dy163(5, 0, 124);
static const cm_Neutron_Scattering _cm_neutron_Dy164(49.4, -0.79, 2840);
static const cm_Neutron_Scattering _cm_neutron_Ho(8.01, 0, 64.7);
static const cm_Neutron_Scattering _cm_neutron_Er(7.79, 0, 159);
static const cm_Neutron_Scattering _cm_neutron_Er162(8.8, 0, 19);
static const cm_Neutron_Scattering _cm_neutron_Er164(8.2, 0, 13);
static const cm_Neutron_Scattering _cm_neutron_Er166(10.6, 0, 19.6);
static const cm_Neutron_Scattering _cm_neutron_Er167(3, 0, 659);
static const cm_Neutron_Scattering _cm_neutron_Er168(7.4, 0, 2.74);
static const cm_Neutron_Scattering _cm_neutron_Er170(9.6, 0, 5.8);
static const cm_Neutron_Scattering _cm_neutron_Tm(7.07, 0, 100);
static const cm_Neutron_Scattering _cm_neutron_Yb(12.43, 0, 34.8);
static const cm_Neutron_Scattering _cm_neutron_Yb168(-4.07, -0.62, 2230);
static const cm_Neutron_Scattering _cm_neutron_Yb170(6.77, 0, 11.4);
static const cm_Neutron_Scattering _cm_neutron_Yb171(9.66, 0, 48.6);
static const cm_Neutron_Scattering _cm_neutron_Yb172(9.43, 0, 0.8);
static const cm_Neutron_Scattering _cm_neutron_Yb173(9.56, 0, 17.1);
static const cm_Neutron_Scattering _cm_neutron_Yb174(19.3, 0, 69.4);
static const cm_Neutron_Scattering _cm_neutron_Yb176(8.72, 0, 2.85);
static const cm_Neutron_Scattering _cm_neutron_Lu(7.21, 0, 74);
static const cm_Neutron_Scattering _cm_neutron_Lu175(7.24, 0, 21);
static const cm_Neutron_Scattering _cm_neutron_Lu176(6.1, -0.57, 2065);
static const cm_Neutron_Scattering _cm_neutron_Hf(7.7, 0, 104.1);
static const cm_Neutron_Scattering _cm_neutron_Hf174(10.9, 0, 561);
static const cm_Neutron_Scattering _cm_neutron_Hf176(6.61, 0, 23.5);
static const cm_Neutron_Scattering _cm_neutron_Hf177(0.8, 0, 373);
static const cm_Neutron_Scattering _cm_neutron_Hf178(5.9, 0, 84);
static const cm_Neutron_Scattering _cm_neutron_Hf179(7.46, 0, 41);
static const cm_Neutron_Scattering _cm_neutron_Hf180(13.2, 0, 13.04);
static const cm_Neutron_Scattering _cm_neutron_Ta(6.91, 0, 20.6);
static const cm_Neutron_Scattering _cm_neutron_Ta180(7, 0, 563);
static const cm_Neutron_Scattering _cm_neutron_Ta181(6.91, 0, 20.5);
static const cm_Neutron_Scattering _cm_neutron_W(4.86, 0, 18.3);
static const cm_Neutron_Scattering _cm_neutron_W180(5, 0, 30);
static const cm_Neutron_Scattering _cm_neutron_W182(6.97, 0, 20.7);
static const cm_Neutron_Scattering _cm_neutron_W183(6.53, 0, 10.1);
static const cm_Neutron_Scattering _cm_neutron_W184(7.48, 0, 1.7);
static const cm_Neutron_Scattering _cm_neutron_W186(-0.72, 0, 37.9);
static const cm_Neutron_Scattering _cm_neutron_Re(9.2, 0, 89.7);
static const cm_Neutron_Scattering _cm_neutron_Re185(9, 0, 112);
static const cm_Neutron_Scattering _cm_neutron_Re187(9.3, 0, 76.4);
static const cm_Neutron_Scattering _cm_neutron_Os(10.7, 0, 16);
static const cm_Neutron_Scattering _cm_neutron_Os184(10, 0, 3000);
static const cm_Neutron_Scattering _cm_neutron_Os186(11.6, 0, 80);
static const cm_Neutron_Scattering _cm_neutron_Os187(10, 0, 320);
static const cm_Neutron_Scattering _cm_neutron_Os188(7.6, 0, 4.7);
static const cm_Neutron_Scattering _cm_neutron_Os189(10.7, 0, 25);
static const cm_Neutron_Scattering _cm_neutron_Os190(11, 0, 13.1);
static const cm_Neutron_Scattering _cm_neutron_Os192(11.5, 0, 2);
static const cm_Neutron_Scattering _cm_neutron_Ir(10.6, 0, 425);
static const cm_Neutron_Scattering _cm_neutron_Pt(9.6, 0, 10.3);
static const cm_Neutron_Scattering _cm_neutron_Pt190(9, 0, 152);
static const cm_Neutron_Scattering _cm_neutron_Pt192(9.9, 0, 10);
static const cm_Neutron_Scattering _cm_neutron_Pt194(10.55, 0, 1.44);
static const cm_Neutron_Scattering _cm_neutron_Pt195(8.83, 0, 27.5);
static const cm_Neutron_Scattering _cm_neutron_Pt196(9.89, 0, 0.72);
static const cm_Neutron_Scattering _cm_neutron_Pt198(7.8, 0, 3.66);
static const cm_Neutron_Scattering _cm_neutron_Au(7.63, 0, 98.65);
static const cm_Neutron_Scattering _cm_neutron_Hg(12.692, 0, 372.3);
static const cm_Neutron_Scattering _cm_neutron_Hg199(16.9, 0, 2150);
static const cm_Neutron_Scattering _cm_neutron_Tl(8.776, 0, 3.43);
static const cm_Neutron_Scattering _cm_neutron_Tl203(6.99, 0, 11.4);
static const cm_Neutron_Scattering _cm_neutron_Tl205(9.52, 0, 0.104);
static const cm_Neutron_Scattering _cm_neutron_Pb(9.405, 0, 0.171);
static const cm_Neutron_Scattering _cm_neutron_Pb204(9.9, 0, 0.65);
static const cm_Neutron_Scattering _cm_neutron_Pb206(9.22, 0, 0.03);
static const cm_Neutron_Scattering _cm_neutron_Pb207(9.28, 0, 0.699);
static const cm_Neutron_Scattering _cm_neutron_Pb208(9.5, 0, 0.00048);
static const cm_Neutron_Scattering _cm_neutron_Bi(8.532, 0, 0.0338);
static const cm_Neutron_Scattering _cm_neutron_Po(0, 0, 0);
static const cm_Neutron_Scattering _cm_neutron_At(0, 0, 0);
static const cm_Neutron_Scattering _cm_neutron_Rn(0, 0, 0);
static const cm_Neutron_Scattering _cm_neutron_Fr(0, 0, 0);
static const cm_Neutron_Scattering _cm_neutron_Ra(10, 0, 12.8);
static const cm_Neutron_Scattering _cm_neutron_Ac(0, 0, 0);
static const cm_Neutron_Scattering _cm_neutron_Th(10.31, 0, 7.37);
static const cm_Neutron_Scattering _cm_neutron_Pa(9.1, 0, 200.6);
static const cm_Neutron_Scattering _cm_neutron_U(8.417, 0, 7.57);
static const cm_Neutron_Scattering _cm_neutron_U234(12.4, 0, 100.1);
static const cm_Neutron_Scattering _cm_neutron_U235(10.47, 0, 680.9);
static const cm_Neutron_Scattering _cm_neutron_U238(8.402, 0, 2.68);
static const cm_Neutron_Scattering _cm_neutron_Np(10.55, 0, 175.9);
static const cm_Neutron_Scattering _cm_neutron_Am(8.3, 0, 75.3);
static const cm_Isotope _cm_isotope_H[] = {
  {1.007825, 0.99989, &_cm_neutron_H1}, 
  {2.014102, 0.000115, &_cm_neutron_H2}};
static const cm_Isotope _cm_isotope_He[] = {
  {3.016029, 0.000001, &_cm_neutron_He3}, 
  {4.002603, 0.999999, &_cm_neutron_He4}};
static const cm_Isotope _cm_isotope_Li[] = {
  {6.015121, 0.0759, &_cm_neutron_Li6}, 
  {7.016003, 0.9241, &_cm_neutron_Li7}};
static const cm_Isotope _cm_isotope_Be[] = {
  {9.012182, 1, NULL}};
static const cm_Isotope _cm_isotope_B[] = {
  {10.012937, 0.199, &_cm_neutron_B10}, 
  {11.009305, 0.8022, &_cm_neutron_B11}};
static const cm_Isotope _cm_isotope_C[] = {
  {12, 0.9889, &_cm_neutron_C12}, 
  {13.003355, 0.0111, &_cm_neutron_C13}};
static const cm_Isotope _cm_isotope_N[] = {
  {14.003074, 0.99632, &_cm_neutron_N14}, 
  {15.000109, 0.00368, &_cm_neutron_N15}};
static const cm_Isotope _cm_isotope_O[] = {
  {15.994915, 0.99757, &_cm_neutron_O16}, 
  {16.999131, 0.00038, &_cm_neutron_O17}, 
  {17.99916, 0.00205, &_cm_neutron_O18}};
static const cm_Isotope _cm_isotope_F[] = {
  {18.998403, 1, NULL}};
static const cm_Isotope _cm_isotope_Ne[] = {
  {19.992436, 0.9048, &_cm_neutron_Ne20}, 
  {20.993843, 0.0027, &_cm_neutron_Ne21}, 
  {21.991383, 0.0925, &_cm_neutron_Ne22}};
static const cm_Isotope _cm_isotope_Na[] = {
  {22.989768, 1, NULL}};
static const cm_Isotope _cm_isotope_Mg[] = {
  {23.985042, 0.7899, &_cm_neutron_Mg24}, 
  {24.985837, 0.1, &_cm_neutron_Mg25}, 
  {25.982594, 0.1101, &_cm_neutron_Mg26}};
static const cm_Isotope _cm_isotope_Al[] = {
  {26.981539, 1, NULL}};
static const cm_Isotope _cm_isotope_Si[] = {
  {27.976927, 0.922297, &_cm_neutron_Si28}, 
  {28.976495, 0.046832, &_cm_neutron_Si29}, 
  {29.973771, 0.030872, &_cm_neutron_Si30}};
static const cm_Isotope _cm_isotope_P[] = {
  {30.973762, 1, NULL}};
static const cm_Isotope _cm_isotope_S[] = {
  {31.972071, 0.9493, &_cm_neutron_S32}, 
  {32.971458, 0.0076, &_cm_neutron_S33}, 
  {33.967867, 0.0429, &_cm_neutron_S34}, 
  {35.967081, 0.0002, &_cm_neutron_S36}};
static const cm_Isotope _cm_isotope_Cl[] = {
  {34.968853, 0.7578, &_cm_neutron_Cl35}, 
  {36.965903, 0.2422, &_cm_neutron_Cl37}};
static const cm_Isotope _cm_isotope_Ar[] = {
  {35.967546, 0.003365, &_cm_neutron_Ar36}, 
  {37.962733, 0.000632, &_cm_neutron_Ar38}, 
  {39.962384, 0.996003, &_cm_neutron_Ar40}};
static const cm_Isotope _cm_isotope_K[] = {
  {38.963707, 0.932581, &_cm_neutron_K39}, 
  {39.963999, 0.000117, &_cm_neutron_K40}, 
  {40.961825, 0.067302, &_cm_neutron_K41}};
static const cm_Isotope _cm_isotope_Ca[] = {
  {39.962591, 0.96941, &_cm_neutron_Ca40}, 
  {41.958618, 0.00647, &_cm_neutron_Ca42}, 
  {42.958766, 0.00135, &_cm_neutron_Ca43}, 
  {43.955481, 0.02086, &_cm_neutron_Ca44}, 
  {45.953689, 0.00004, &_cm_neutron_Ca46}, 
  {47.952533, 0.00187, &_cm_neutron_Ca48}};
static const cm_Isotope _cm_isotope_Sc[] = {
  {44.95591, 1, NULL}};
static const cm_Isotope _cm_isotope_Ti[] = {
  {45.952629, 0.0825, &_cm_neutron_Ti46}, 
  {46.951764, 0.0744, &_cm_neutron_Ti47}, 
  {47.947947, 0.7372, &_cm_neutron_Ti48}, 
  {48.947871, 0.0541, &_cm_neutron_Ti49}, 
  {49.944792, 0.0518, &_cm_neutron_Ti50}};
static const cm_Isotope _cm_isotope_V[] = {
  {49.947161, 0.0025, &_cm_neutron_V50}, 
  {50.943962, 0.9975, &_cm_neutron_V51}};
static const cm_Isotope _cm_isotope_Cr[] = {
  {49.946046, 0.04345, &_cm_neutron_Cr50}, 
  {51.94051, 0.83789, &_cm_neutron_Cr52}, 
  {52.940651, 0.09501, &_cm_neutron_Cr53}, 
  {53.938882, 0.02365, &_cm_neutron_Cr54}};
static const cm_Isotope _cm_isotope_Mn[] = {
  {54.938047, 1, NULL}};
static const cm_Isotope _cm_isotope_Fe[] = {
  {53.939613, 0.05845, &_cm_neutron_Fe54}, 
  {55.934939, 0.91754, &_cm_neutron_Fe56}, 
  {56.935396, 0.02119, &_cm_neutron_Fe57}, 
  {57.933277, 0.00282, &_cm_neutron_Fe58}};
static const cm_Isotope _cm_isotope_Co[] = {
  {58.933198, 1, NULL}};
static const cm_Isotope _cm_isotope_Ni[] = {
  {57.935346, 0.680769, &_cm_neutron_Ni58}, 
  {59.930788, 0.26223, &_cm_neutron_Ni60}, 
  {60.931058, 0.011399, &_cm_neutron_Ni61}, 
  {61.928346, 0.036345, &_cm_neutron_Ni62}, 
  {63.927968, 0.009256, &_cm_neutron_Ni64}};
static const cm_Isotope _cm_isotope_Cu[] = {
  {62.929599, 0.6917, &_cm_neutron_Cu63}, 
  {64.927793, 0.3083, &_cm_neutron_Cu65}};
static const cm_Isotope _cm_isotope_Zn[] = {
  {63.929145, 0.4863, &_cm_neutron_Zn64}, 
  {65.926035, 0.279, &_cm_neutron_Zn66}, 
  {66.927129, 0.041, &_cm_neutron_Zn67}, 
  {67.924846, 0.1875, &_cm_neutron_Zn68}, 
  {69.925325, 0.0062, &_cm_neutron_Zn70}};
static const cm_Isotope _cm_isotope_Ga[] = {
  {68.92558, 0.60108, &_cm_neutron_Ga69}, 
  {70.924701, 0.39892, &_cm_neutron_Ga71}};
static const cm_Isotope _cm_isotope_Ge[] = {
  {69.92425, 0.2084, &_cm_neutron_Ge70}, 
  {71.922079, 0.2754, &_cm_neutron_Ge72}, 
  {72.923463, 0.0773, &_cm_neutron_Ge73}, 
  {73.921177, 0.3628, &_cm_neutron_Ge74}, 
  {75.921402, 0.0761, &_cm_neutron_Ge76}};
static const cm_Isotope _cm_isotope_As[] = {
  {74.921594, 1, NULL}};
static const cm_Isotope _cm_isotope_Se[] = {
  {73.922475, 0.0089, &_cm_neutron_Se74}, 
  {75.919212, 0.0937, &_cm_neutron_Se76}, 
  {76.919912, 0.0763, &_cm_neutron_Se77}, 
  {77.917308, 0.2377, &_cm_neutron_Se78}, 
  {79.91652, 0.4961, &_cm_neutron_Se80}, 
  {81.916698, 0.0873, &_cm_neutron_Se82}};
static const cm_Isotope _cm_isotope_Br[] = {
  {78.918336, 0.5069, &_cm_neutron_Br79}, 
  {80.916289, 0.4931, &_cm_neutron_Br81}};
static const cm_Isotope _cm_isotope_Kr[] = {
  {77.920396, 0.0035, NULL}, 
  {79.91638, 0.0228, NULL}, 
  {81.913482, 0.1158, NULL}, 
  {82.914135, 0.1149, NULL}, 
  {83.911507, 0.57, NULL}, 
  {85.910616, 0.173, &_cm_neutron_Kr86}};
static const cm_Isotope _cm_isotope_Rb[] = {
  {84.911794, 0.7217, &_cm_neutron_Rb85}, 
  {86.909187, 0.2783, &_cm_neutron_Rb87}};
static const cm_Isotope _cm_isotope_Sr[] = {
  {83.91343, 0.0056, &_cm_neutron_Sr84}, 
  {85.909267, 0.0986, &_cm_neutron_Sr86}, 
  {86.908884, 0.07, &_cm_neutron_Sr87}, 
  {87.905619, 0.8258, &_cm_neutron_Sr88}};
static const cm_Isotope _cm_isotope_Y[] = {
  {89.905849, 1, NULL}};
static const cm_Isotope _cm_isotope_Zr[] = {
  {89.904703, 0.5145, &_cm_neutron_Zr90}, 
  {90.905644, 0.1122, &_cm_neutron_Zr91}, 
  {91.905039, 0.1715, &_cm_neutron_Zr92}, 
  {93.906315, 0.1738, &_cm_neutron_Zr94}, 
  {95.908275, 0.028, &_cm_neutron_Zr96}};
static const cm_Isotope _cm_isotope_Nb[] = {
  {92.906377, 1, NULL}};
static const cm_Isotope _cm_isotope_Mo[] = {
  {91.906809, 0.1484, &_cm_neutron_Mo92}, 
  {93.905085, 0.0925, &_cm_neutron_Mo94}, 
  {94.905841, 0.1592, &_cm_neutron_Mo95}, 
  {95.904679, 0.1668, &_cm_neutron_Mo96}, 
  {96.90602, 0.0955, &_cm_neutron_Mo97}, 
  {97.905407, 0.2413, &_cm_neutron_Mo98}, 
  {99.907477, 0.0963, &_cm_neutron_Mo100}};
static const cm_Isotope _cm_isotope_Ru[] = {
  {95.907599, 0.0554, NULL}, 
  {97.905287, 0.0187, NULL}, 
  {98.905939, 0.1276, NULL}, 
  {99.904219, 0.126, NULL}, 
  {100.905582, 0.1706, NULL}, 
  {101.904349, 0.3155, NULL}, 
  {103.905424, 0.1862, NULL}};
static const cm_Isotope _cm_isotope_Rh[] = {
  {102.9055, 1, NULL}};
static const cm_Isotope _cm_isotope_Pd[] = {
  {101.905634, 0.0102, &_cm_neutron_Pd102}, 
  {103.904029, 0.1114, &_cm_neutron_Pd104}, 
  {104.905079, 0.2233, &_cm_neutron_Pd105}, 
  {105.903478, 0.2733, &_cm_neutron_Pd106}, 
  {107.903895, 0.2646, &_cm_neutron_Pd108}, 
  {109.905167, 0.1172, &_cm_neutron_Pd110}};
static const cm_Isotope _cm_isotope_Ag[] = {
  {106.905092, 0.51839, &_cm_neutron_Ag107}, 
  {108.904756, 0.48161, &_cm_neutron_Ag109}};
static const cm_Isotope _cm_isotope_Cd[] = {
  {105.906461, 0.0125, &_cm_neutron_Cd106}, 
  {107.904176, 0.0089, &_cm_neutron_Cd108}, 
  {109.903005, 0.1249, &_cm_neutron_Cd110}, 
  {110.904182, 0.128, &_cm_neutron_Cd111}, 
  {111.902757, 0.2413, &_cm_neutron_Cd112}, 
  {112.9044, 0.1222, &_cm_neutron_Cd113}, 
  {113.903357, 0.2873, &_cm_neutron_Cd114}, 
  {115.904755, 0.0749, &_cm_neutron_Cd116}};
static const cm_Isotope _cm_isotope_In[] = {
  {112.904061, 0.0429, &_cm_neutron_In113}, 
  {114.903882, 0.9571, &_cm_neutron_In115}};
static const cm_Isotope _cm_isotope_Sn[] = {
  {111.904826, 0.0097, &_cm_neutron_Sn112}, 
  {113.902784, 0.0066, &_cm_neutron_Sn114}, 
  {114.903348, 0.0034, &_cm_neutron_Sn115}, 
  {115.901747, 0.1454, &_cm_neutron_Sn116}, 
  {116.902956, 0.0768, &_cm_neutron_Sn117}, 
  {117.901609, 0.2422, &_cm_neutron_Sn118}, 
  {118.903311, 0.0859, &_cm_neutron_Sn119}, 
  {119.902199, 0.3258, &_cm_neutron_Sn120}, 
  {121.90344, 0.0463, &_cm_neutron_Sn122}, 
  {123.905274, 0.0579, &_cm_neutron_Sn124}};
static const cm_Isotope _cm_isotope_Sb[] = {
  {120.903821, 0.5721, &_cm_neutron_Sb121}, 
  {122.904216, 0.4279, &_cm_neutron_Sb123}};
static const cm_Isotope _cm_isotope_Te[] = {
  {119.904048, 0.0009, &_cm_neutron_Te120}, 
  {121.90305, 0.0255, &_cm_neutron_Te122}, 
  {122.904271, 0.0089, &_cm_neutron_Te123}, 
  {123.902818, 0.0474, &_cm_neutron_Te124}, 
  {124.904428, 0.0707, &_cm_neutron_Te125}, 
  {125.90331, 0.1884, &_cm_neutron_Te126}, 
  {127.904463, 0.3174, &_cm_neutron_Te128}, 
  {129.906229, 0.3408, &_cm_neutron_Te130}};
static const cm_Isotope _cm_isotope_I[] = {
  {126.904473, 1, NULL}};
static const cm_Isotope _cm_isotope_Xe[] = {
  {123.905894, 0.0009, NULL}, 
  {125.904281, 0.0009, NULL}, 
  {127.903531, 0.0192, NULL}, 
  {128.90478, 0.2644, NULL}, 
  {129.903509, 0.0408, NULL}, 
  {130.905072, 0.2118, NULL}, 
  {131.904144, 0.2689, NULL}, 
  {133.905395, 0.1044, NULL}, 
  {135.907214, 0.0887, NULL}};
static const cm_Isotope _cm_isotope_Cs[] = {
  {132.905429, 1, NULL}};
static const cm_Isotope _cm_isotope_Ba[] = {
  {129.906282, 0.00106, &_cm_neutron_Ba130}, 
  {131.905042, 0.00101, &_cm_neutron_Ba132}, 
  {133.904486, 0.02417, &_cm_neutron_Ba134}, 
  {134.905665, 0.06592, &_cm_neutron_Ba135}, 
  {135.904553, 0.07854, &_cm_neutron_Ba136}, 
  {136.905812, 0.11232, &_cm_neutron_Ba137}, 
  {137.905232, 0.71698, &_cm_neutron_Ba138}};
static const cm_Isotope _cm_isotope_La[] = {
  {137.907105, 0.0009, &_cm_neutron_La138}, 
  {138.906347, 0.9991, &_cm_neutron_La139}};
static const cm_Isotope _cm_isotope_Ce[] = {
  {135.90714, 0.00185, &_cm_neutron_Ce136}, 
  {137.905985, 0.00251, &_cm_neutron_Ce138}, 
  {139.905433, 0.8845, &_cm_neutron_Ce140}, 
  {141.909241, 0.11114, &_cm_neutron_Ce142}};
static const cm_Isotope _cm_isotope_Pr[] = {
  {140.907647, 1, NULL}};
static const cm_Isotope _cm_isotope_Nd[] = {
  {141.907719, 0.272, &_cm_neutron_Nd142}, 
  {142.90981, 0.122, &_cm_neutron_Nd143}, 
  {143.910083, 0.238, &_cm_neutron_Nd144}, 
  {144.91257, 0.083, &_cm_neutron_Nd145}, 
  {145.913113, 0.172, &_cm_neutron_Nd146}, 
  {147.916889, 0.057, &_cm_neutron_Nd148}, 
  {149.920887, 0.056, &_cm_neutron_Nd150}};
static const cm_Isotope _cm_isotope_Sm[] = {
  {143.911998, 0.0307, &_cm_neutron_Sm144}, 
  {146.914894, 0.1499, &_cm_neutron_Sm147}, 
  {147.914819, 0.1124, &_cm_neutron_Sm148}, 
  {148.91718, 0.1382, &_cm_neutron_Sm149}, 
  {149.917273, 0.0738, &_cm_neutron_Sm150}, 
  {151.919728, 0.2675, &_cm_neutron_Sm152}, 
  {153.922205, 0.2275, &_cm_neutron_Sm154}};
static const cm_Isotope _cm_isotope_Eu[] = {
  {150.919702, 0.4781, &_cm_neutron_Eu151}, 
  {152.921225, 0.5219, &_cm_neutron_Eu153}};
static const cm_Isotope _cm_isotope_Gd[] = {
  {151.919786, 0.002, &_cm_neutron_Gd152}, 
  {153.920861, 0.0218, &_cm_neutron_Gd154}, 
  {154.922618, 0.148, &_cm_neutron_Gd155}, 
  {155.922118, 0.2047, &_cm_neutron_Gd156}, 
  {156.923956, 0.1565, &_cm_neutron_Gd157}, 
  {157.924019, 0.2484, &_cm_neutron_Gd158}, 
  {159.927049, 0.2186, &_cm_neutron_Gd160}};
static const cm_Isotope _cm_isotope_Tb[] = {
  {158.925342, 1, NULL}};
static const cm_Isotope _cm_isotope_Dy[] = {
  {155.924277, 0.0006, &_cm_neutron_Dy156}, 
  {157.924403, 0.001, &_cm_neutron_Dy158}, 
  {159.925193, 0.0234, &_cm_neutron_Dy160}, 
  {160.92693, 0.1891, &_cm_neutron_Dy161}, 
  {161.926795, 0.2551, &_cm_neutron_Dy162}, 
  {162.928728, 0.249, &_cm_neutron_Dy163}, 
  {163.929171, 0.2818, &_cm_neutron_Dy164}};
static const cm_Isotope _cm_isotope_Ho[] = {
  {164.930319, 1, NULL}};
static const cm_Isotope _cm_isotope_Er[] = {
  {161.928775, 0.0014, &_cm_neutron_Er162}, 
  {163.929198, 0.0161, &_cm_neutron_Er164}, 
  {165.93029, 0.3361, &_cm_neutron_Er166}, 
  {166.932046, 0.2293, &_cm_neutron_Er167}, 
  {167.932368, 0.2678, &_cm_neutron_Er168}, 
  {169.935461, 0.1493, &_cm_neutron_Er170}};
static const cm_Isotope _cm_isotope_Tm[] = {
  {168.934212, 1, NULL}};
static const cm_Isotope _cm_isotope_Yb[] = {
  {167.933894, 0.0013, &_cm_neutron_Yb168}, 
  {169.934759, 0.0304, &_cm_neutron_Yb170}, 
  {170.936323, 0.1428, &_cm_neutron_Yb171}, 
  {171.936378, 0.2183, &_cm_neutron_Yb172}, 
  {172.938208, 0.1613, &_cm_neutron_Yb173}, 
  {173.938859, 0.3183, &_cm_neutron_Yb174}, 
  {175.942564, 0.1276, &_cm_neutron_Yb176}};
static const cm_Isotope _cm_isotope_Lu[] = {
  {174.94077, 0.9741, &_cm_neutron_Lu175}, 
  {175.942679, 0.0259, &_cm_neutron_Lu176}};
static const cm_Isotope _cm_isotope_Hf[] = {
  {173.940044, 0.0016, &_cm_neutron_Hf174}, 
  {175.941406, 0.0526, &_cm_neutron_Hf176}, 
  {176.943217, 0.186, &_cm_neutron_Hf177}, 
  {177.943696, 0.2728, &_cm_neutron_Hf178}, 
  {178.945812, 0.1362, &_cm_neutron_Hf179}, 
  {179.946546, 0.3508, &_cm_neutron_Hf180}};
static const cm_Isotope _cm_isotope_Ta[] = {
  {179.947462, 0.00012, &_cm_neutron_Ta180}, 
  {180.947992, 0.99988, &_cm_neutron_Ta181}};
static const cm_Isotope _cm_isotope_W[] = {
  {179.946701, 0.0012, &_cm_neutron_W180}, 
  {181.948202, 0.265, &_cm_neutron_W182}, 
  {182.95022, 0.1431, &_cm_neutron_W183}, 
  {183.950928, 0.3064, &_cm_neutron_W184}, 
  {185.954357, 0.2843, &_cm_neutron_W186}};
static const cm_Isotope _cm_isotope_Re[] = {
  {184.952951, 0.374, &_cm_neutron_Re185}, 
  {186.955744, 0.626, &_cm_neutron_Re187}};
static const cm_Isotope _cm_isotope_Os[] = {
  {183.952488, 0.0002, &_cm_neutron_Os184}, 
  {185.95383, 0.0159, &_cm_neutron_Os186}, 
  {186.955741, 0.0196, &_cm_neutron_Os187}, 
  {187.95583, 0.1324, &_cm_neutron_Os188}, 
  {188.958137, 0.1615, &_cm_neutron_Os189}, 
  {189.958436, 0.2626, &_cm_neutron_Os190}, 
  {191.961467, 0.4078, &_cm_neutron_Os192}};
static const cm_Isotope _cm_isotope_Ir[] = {
  {190.960584, 0.373, NULL}, 
  {192.962917, 0.627, NULL}};
static const cm_Isotope _cm_isotope_Pt[] = {
  {189.959917, 0.00014, &_cm_neutron_Pt190}, 
  {191.961019, 0.00782, &_cm_neutron_Pt192}, 
  {193.962655, 0.32967, &_cm_neutron_Pt194}, 
  {194.964766, 0.33832, &_cm_neutron_Pt195}, 
  {195.964926, 0.25242, &_cm_neutron_Pt196}, 
  {197.967869, 0.07163, &_cm_neutron_Pt198}};
static const cm_Isotope _cm_isotope_Au[] = {
  {196.966543, 1, NULL}};
static const cm_Isotope _cm_isotope_Hg[] = {
  {196.966543, 0.0015, NULL}, 
  {197.966743, 0.0997, NULL}, 
  {198.968254, 0.1687, &_cm_neutron_Hg199}, 
  {199.9683, 0.231, NULL}, 
  {200.970277, 0.1318, NULL}, 
  {201.970617, 0.2986, NULL}, 
  {203.973467, 0.0687, NULL}};
static const cm_Isotope _cm_isotope_Tl[] = {
  {202.97232, 0.29524, &_cm_neutron_Tl203}, 
  {204.974401, 0.70476, &_cm_neutron_Tl205}};
static const cm_Isotope _cm_isotope_Pb[] = {
  {203.97302, 0.014, &_cm_neutron_Pb204}, 
  {205.97444, 0.241, &_cm_neutron_Pb206}, 
  {206.975872, 0.221, &_cm_neutron_Pb207}, 
  {207.976627, 0.524, &_cm_neutron_Pb208}};
static const cm_Isotope _cm_isotope_Bi[] = {
  {208.980374, 1, NULL}};
static const cm_Isotope _cm_isotope_Th[] = {
  {232.0381, 1, NULL}};
static const cm_Isotope _cm_isotope_Pa[] = {
  {231.0359, 1, NULL}};
static const cm_Isotope _cm_isotope_U[] = {
  {234.040947, 0.000055, &_cm_neutron_U234}, 
  {235.043924, 0.0072, &_cm_neutron_U235}, 
  {238.050785, 0.992745, &_cm_neutron_U238}};
//static const cm_Gaussians _cm_gaussians_H(0.489918, 0.262003, 0.196767, 0.049879, 20.6593, 7.74039, 49.5519, 2.20159, 0.001305);
static const cm_Gaussians _cm_gaussians_H(0.493002, 0.322912, 0.140191, 0.040810, 10.5109, 26.1257, 3.14236, 57.7997, 0.003038);
static const cm_Gaussians _cm_gaussians_He(0.8734, 0.6309, 0.3112, 0.178, 9.1037, 3.3568, 22.9276, 0.9821, 0.0064);
static const cm_Gaussians _cm_gaussians_Li(1.1282, 0.7508, 0.6175, 0.4653, 3.9546, 1.0524, 85.3905, 168.261, 0.0377);
static const cm_Gaussians _cm_gaussians_Be(1.5919, 1.1278, 0.5391, 0.7029, 43.6427, 1.8623, 103.483, 0.542, 0.0385);
static const cm_Gaussians _cm_gaussians_B(2.0545, 1.3326, 1.0979, 0.7068, 23.2185, 1.021, 60.3498, 0.1403, -0.1932);
static const cm_Gaussians _cm_gaussians_C(2.31, 1.02, 1.5886, 0.865, 20.8439, 10.2075, 0.5687, 51.6512, 0.2156);
static const cm_Gaussians _cm_gaussians_N(12.2126, 3.1322, 2.0125, 1.1663, 0.0057, 9.8933, 28.9975, 0.5826, -11.529);
static const cm_Gaussians _cm_gaussians_O(3.0485, 2.2868, 1.5463, 0.867, 13.2771, 5.7011, 0.3239, 32.9089, 0.2508);
static const cm_Gaussians _cm_gaussians_F(3.5392, 2.6412, 1.517, 1.0243, 10.2825, 4.2944, 0.2615, 26.1476, 0.2776);
static const cm_Gaussians _cm_gaussians_Ne(3.9553, 3.1125, 1.4546, 1.1251, 8.4042, 3.4262, 0.2306, 21.7184, 0.3515);
static const cm_Gaussians _cm_gaussians_Na(4.7626, 3.1736, 1.2674, 1.1128, 3.285, 8.8422, 0.3136, 129.424, 0.676);
static const cm_Gaussians _cm_gaussians_Mg(5.4204, 2.1735, 1.2269, 2.3073, 2.8275, 79.2611, 0.3808, 7.1937, 0.8584);
static const cm_Gaussians _cm_gaussians_Al(6.4202, 1.9002, 1.5936, 1.9646, 3.0387, 0.7426, 31.5472, 85.0886, 1.1151);
static const cm_Gaussians _cm_gaussians_Si(6.2915, 3.0353, 1.9891, 1.541, 2.4386, 32.3337, 0.6785, 81.6937, 1.1407);
static const cm_Gaussians _cm_gaussians_P(6.4345, 4.1791, 1.78, 1.4908, 1.9067, 27.157, 0.526, 68.1645, 1.1149);
static const cm_Gaussians _cm_gaussians_S(6.9053, 5.2034, 1.4379, 1.5863, 1.4679, 22.2151, 0.2536, 56.172, 0.8669);
static const cm_Gaussians _cm_gaussians_Cl(11.4604, 7.1964, 6.2556, 1.6455, 0.0104, 1.1662, 18.5194, 47.7784, -9.5574);
static const cm_Gaussians _cm_gaussians_Ar(7.4845, 6.7723, 0.6539, 1.6442, 0.9072, 14.8407, 43.8983, 33.3929, 1.4445);
static const cm_Gaussians _cm_gaussians_K(8.2186, 7.4398, 1.0519, 0.8659, 12.7949, 0.7748, 213.187, 41.6841, 1.4228);
static const cm_Gaussians _cm_gaussians_Ca(8.6266, 7.3873, 1.5899, 1.0211, 10.4421, 0.6599, 85.7484, 178.437, 1.3751);
static const cm_Gaussians _cm_gaussians_Sc(9.189, 7.3679, 1.6409, 1.468, 9.0213, 0.5729, 136.108, 51.3531, 1.3329);
static const cm_Gaussians _cm_gaussians_Ti(9.7595, 7.3558, 1.6991, 1.9021, 7.8508, 0.5, 35.6338, 116.105, 1.2807);
static const cm_Gaussians _cm_gaussians_V(10.2971, 7.3511, 2.0703, 2.0571, 6.8657, 0.4385, 26.8938, 102.478, 1.2199);
static const cm_Gaussians _cm_gaussians_Cr(10.6406, 7.3537, 3.324, 1.4922, 6.1038, 0.392, 20.2626, 98.7399, 1.1832);
static const cm_Gaussians _cm_gaussians_Mn(11.2819, 7.3573, 3.0193, 2.2441, 5.3409, 0.3432, 17.8674, 83.7543, 1.0896);
static const cm_Gaussians _cm_gaussians_Fe(11.7695, 7.3573, 3.5222, 2.3045, 4.7611, 0.3072, 15.3535, 76.8805, 1.0369);
static const cm_Gaussians _cm_gaussians_Co(12.2841, 7.3409, 4.0034, 2.3488, 4.2791, 0.2784, 13.5359, 71.1692, 1.0118);
static const cm_Gaussians _cm_gaussians_Ni(12.8376, 7.292, 4.4438, 2.38, 3.8785, 0.2565, 12.1763, 66.3421, 1.0341);
static const cm_Gaussians _cm_gaussians_Cu(13.338, 7.1676, 5.6158, 1.6735, 3.5828, 0.247, 11.3966, 64.8126, 1.191);
static const cm_Gaussians _cm_gaussians_Zn(14.0743, 7.0318, 5.1652, 2.41, 3.2655, 0.2333, 10.3163, 58.7097, 1.3041);
static const cm_Gaussians _cm_gaussians_Ga(15.2354, 6.7006, 4.3591, 2.9623, 3.0669, 0.2412, 10.7805, 61.4135, 1.7189);
static const cm_Gaussians _cm_gaussians_Ge(16.0816, 6.3747, 3.7068, 3.683, 2.8509, 0.2516, 11.4468, 54.7625, 2.1313);
static const cm_Gaussians _cm_gaussians_As(16.6723, 6.0701, 3.4313, 4.2779, 2.6345, 0.2647, 12.9479, 47.7972, 2.531);
static const cm_Gaussians _cm_gaussians_Se(17.0006, 5.8196, 3.9731, 4.3543, 2.4098, 0.2726, 15.2372, 43.8163, 2.8409);
static const cm_Gaussians _cm_gaussians_Br(17.1789, 5.2358, 5.6377, 3.9851, 2.1723, 16.5796, 0.2609, 41.4328, 2.9557);
static const cm_Gaussians _cm_gaussians_Kr(17.3555, 6.7286, 5.5493, 3.5375, 1.9384, 16.5623, 0.2261, 39.3972, 2.825);
static const cm_Gaussians _cm_gaussians_Rb(17.1784, 9.6435, 5.1399, 1.5292, 1.7888, 17.3151, 0.2748, 164.934, 3.4873);
static const cm_Gaussians _cm_gaussians_Sr(17.5663, 9.8184, 5.422, 2.6694, 1.5564, 14.0988, 0.1664, 132.376, 2.5064);
static const cm_Gaussians _cm_gaussians_Y(17.776, 10.2946, 5.72629, 3.26588, 1.4029, 12.8006, 0.125599, 104.354, 1.91213);
static const cm_Gaussians _cm_gaussians_Zr(17.8765, 10.948, 5.41732, 3.65721, 1.27618, 11.916, 0.117622, 87.6627, 2.06929);
static const cm_Gaussians _cm_gaussians_Nb(17.6142, 12.0144, 4.04183, 3.53346, 1.18865, 11.766, 0.204785, 69.7957, 3.75591);
static const cm_Gaussians _cm_gaussians_Mo(3.7025, 17.2356, 12.8876, 3.7429, 0.2772, 1.0958, 11.004, 61.6584, 4.3875);
static const cm_Gaussians _cm_gaussians_Tc(19.1301, 11.0948, 4.64901, 2.71263, 0.864132, 8.14487, 21.5707, 86.8472, 5.40428);
static const cm_Gaussians _cm_gaussians_Ru(19.2674, 12.9182, 4.86337, 1.56756, 0.80852, 8.43467, 24.7997, 94.2928, 5.37874);
static const cm_Gaussians _cm_gaussians_Rh(19.2957, 14.3501, 4.73425, 1.28918, 0.751536, 8.21758, 25.8749, 98.6062, 5.328);
static const cm_Gaussians _cm_gaussians_Pd(19.3319, 15.5017, 5.29537, 0.605844, 0.698655, 7.98929, 25.2052, 76.8986, 5.26593);
static const cm_Gaussians _cm_gaussians_Ag(19.2808, 16.6885, 4.8045, 1.0463, 0.6446, 7.4726, 24.6605, 99.8156, 5.179);
static const cm_Gaussians _cm_gaussians_Cd(19.2214, 17.6444, 4.461, 1.6029, 0.5946, 6.9089, 24.7008, 87.4825, 5.0694);
static const cm_Gaussians _cm_gaussians_In(19.1624, 18.5596, 4.2948, 2.0396, 0.5476, 6.3776, 25.8499, 92.8029, 4.9391);
static const cm_Gaussians _cm_gaussians_Sn(19.1889, 19.1005, 4.4585, 2.4663, 5.8303, 0.5031, 26.8909, 83.9571, 4.7821);
static const cm_Gaussians _cm_gaussians_Sb(19.6418, 19.0455, 5.0371, 2.6827, 5.3034, 0.4607, 27.9074, 75.2825, 4.5909);
static const cm_Gaussians _cm_gaussians_Te(19.9644, 19.0138, 6.14487, 2.5239, 4.81742, 0.420885, 28.5284, 70.8403, 4.352);
static const cm_Gaussians _cm_gaussians_I(20.1472, 18.9949, 7.5138, 2.2735, 4.347, 0.3814, 27.766, 66.8776, 4.0712);
static const cm_Gaussians _cm_gaussians_Xe(20.2933, 19.0298, 8.9767, 1.99, 3.9282, 0.344, 26.4659, 64.2658, 3.7118);
static const cm_Gaussians _cm_gaussians_Cs(20.3892, 19.1062, 10.662, 1.4953, 3.569, 0.3107, 24.3879, 213.904, 3.3352);
static const cm_Gaussians _cm_gaussians_Ba(20.3361, 19.297, 10.888, 2.6959, 3.216, 0.2756, 20.2073, 167.202, 2.7731);
static const cm_Gaussians _cm_gaussians_La(20.578, 19.599, 11.3727, 3.28719, 2.94817, 0.244475, 18.7726, 133.124, 2.14678);
static const cm_Gaussians _cm_gaussians_Ce(21.1671, 19.7695, 11.8513, 3.33049, 2.81219, 0.226836, 17.6083, 127.113, 1.86264);
static const cm_Gaussians _cm_gaussians_Pr(22.044, 19.6697, 12.3856, 2.82428, 2.77393, 0.222087, 16.7669, 143.644, 2.0583);
static const cm_Gaussians _cm_gaussians_Nd(22.6845, 19.6847, 12.774, 2.85137, 2.66248, 0.210628, 15.885, 137.903, 1.98486);
static const cm_Gaussians _cm_gaussians_Pm(23.3405, 19.6095, 13.1235, 2.87516, 2.5627, 0.202088, 15.1009, 132.721, 2.02876);
static const cm_Gaussians _cm_gaussians_Sm(24.0042, 19.4258, 13.4396, 2.89604, 2.47274, 0.196451, 14.3996, 128.007, 2.20963);
static const cm_Gaussians _cm_gaussians_Eu(24.6274, 19.0886, 13.7603, 2.9227, 2.3879, 0.1942, 13.7546, 123.174, 2.5745);
static const cm_Gaussians _cm_gaussians_Gd(25.0709, 19.0798, 13.8518, 3.54545, 2.25341, 0.181951, 12.9331, 101.398, 2.4196);
static const cm_Gaussians _cm_gaussians_Tb(25.8976, 18.2185, 14.3167, 2.95354, 2.24256, 0.196143, 12.6648, 115.362, 3.58324);
static const cm_Gaussians _cm_gaussians_Dy(26.507, 17.6383, 14.5596, 2.96577, 2.1802, 0.202172, 12.1899, 111.874, 4.29728);
static const cm_Gaussians _cm_gaussians_Ho(26.9049, 17.294, 14.5583, 3.63837, 2.07051, 0.19794, 11.4407, 92.6566, 4.56796);
static const cm_Gaussians _cm_gaussians_Er(27.6563, 16.4285, 14.9779, 2.98233, 2.07356, 0.223545, 11.3604, 105.703, 5.92046);
static const cm_Gaussians _cm_gaussians_Tm(28.1819, 15.8851, 15.1542, 2.98706, 2.02859, 0.238849, 10.9975, 102.961, 6.75621);
static const cm_Gaussians _cm_gaussians_Yb(28.6641, 15.4345, 15.3087, 2.98963, 1.9889, 0.257119, 10.6647, 100.417, 7.56672);
static const cm_Gaussians _cm_gaussians_Lu(28.9476, 15.2208, 15.1, 3.71601, 1.90182, 9.98519, 0.261033, 84.3298, 7.97628);
static const cm_Gaussians _cm_gaussians_Hf(29.144, 15.1726, 14.7586, 4.30013, 1.83262, 9.5999, 0.275116, 72.029, 8.58154);
static const cm_Gaussians _cm_gaussians_Ta(29.2024, 15.2293, 14.5135, 4.76492, 1.77333, 9.37046, 0.295977, 63.3644, 9.24354);
static const cm_Gaussians _cm_gaussians_W(29.0818, 15.43, 14.4327, 5.11982, 1.72029, 9.2259, 0.321703, 57.056, 9.8875);
static const cm_Gaussians _cm_gaussians_Re(28.7621, 15.7189, 14.5564, 5.44174, 1.67191, 9.09227, 0.3505, 52.0861, 10.472);
static const cm_Gaussians _cm_gaussians_Os(28.1894, 16.155, 14.9305, 5.67589, 1.62903, 8.97948, 0.382661, 48.1647, 11.0005);
static const cm_Gaussians _cm_gaussians_Ir(27.3049, 16.7296, 15.6115, 5.83377, 1.59279, 8.86553, 0.417916, 45.0011, 11.4722);
static const cm_Gaussians _cm_gaussians_Pt(27.0059, 17.7639, 15.7131, 5.7837, 1.51293, 8.81174, 0.424593, 38.6103, 11.6883);
static const cm_Gaussians _cm_gaussians_Au(16.8819, 18.5913, 25.5582, 5.86, 0.4611, 8.6216, 1.4826, 36.3956, 12.0658);
static const cm_Gaussians _cm_gaussians_Hg(20.6809, 19.0417, 21.6575, 5.9676, 0.545, 8.4484, 1.5729, 38.3246, 12.6089);
static const cm_Gaussians _cm_gaussians_Tl(27.5446, 19.1584, 15.538, 5.52593, 0.65515, 8.70751, 1.96347, 45.8149, 13.1746);
static const cm_Gaussians _cm_gaussians_Pb(31.0617, 13.0637, 18.442, 5.9696, 0.6902, 2.3576, 8.618, 47.2579, 13.4118);
static const cm_Gaussians _cm_gaussians_Bi(33.3689, 12.951, 16.5877, 6.4692, 0.704, 2.9238, 8.7937, 48.0093, 13.5782);
static const cm_Gaussians _cm_gaussians_Po(34.6726, 15.4733, 13.1138, 7.02588, 0.700999, 3.55078, 9.55642, 47.0045, 13.677);
static const cm_Gaussians _cm_gaussians_At(35.3163, 19.0211, 9.49887, 7.42518, 0.68587, 3.97458, 11.3824, 45.4715, 13.7108);
static const cm_Gaussians _cm_gaussians_Rn(35.5631, 21.2816, 8.0037, 7.4433, 0.6631, 4.0691, 14.0422, 44.2473, 13.6905);
static const cm_Gaussians _cm_gaussians_Fr(35.9299, 23.0547, 12.1439, 2.11253, 0.646453, 4.17619, 23.1052, 150.645, 13.7247);
static const cm_Gaussians _cm_gaussians_Ra(35.763, 22.9064, 12.4739, 3.21097, 0.616341, 3.87135, 19.9887, 142.325, 13.6211);
static const cm_Gaussians _cm_gaussians_Ac(35.6597, 23.1032, 12.5977, 4.08655, 0.589092, 3.65155, 18.599, 117.02, 13.5266);
static const cm_Gaussians _cm_gaussians_Th(35.5645, 23.4219, 12.7473, 4.80703, 0.563359, 3.46204, 17.8309, 99.1722, 13.4314);
static const cm_Gaussians _cm_gaussians_Pa(35.8847, 23.2948, 14.1891, 4.17287, 0.547751, 3.41519, 16.9235, 105.251, 13.4287);
static const cm_Gaussians _cm_gaussians_U(36.0228, 23.4128, 14.9491, 4.188, 0.5293, 3.3253, 16.0927, 100.613, 13.3966);
static const cm_Gaussians _cm_gaussians_Np(36.1874, 23.5964, 15.6402, 4.1855, 0.511929, 3.25396, 15.3622, 97.4908, 13.3573);
static const cm_Gaussians _cm_gaussians_Pu(36.5254, 23.8083, 16.7707, 3.47947, 0.499384, 3.26371, 14.9455, 105.98, 13.3812);
static const cm_Gaussians _cm_gaussians_Am(36.6706, 24.0992, 17.3415, 3.49331, 0.483629, 3.20647, 14.3136, 102.273, 13.3592);
static const cm_Gaussians _cm_gaussians_Cm(36.6488, 24.4096, 17.399, 4.21665, 0.465154, 3.08997, 13.4346, 88.4834, 13.2887);
static const cm_Gaussians _cm_gaussians_Bk(36.7881, 24.7736, 17.8919, 4.23284, 0.451018, 3.04619, 12.8946, 86.003, 13.2754);
static const cm_Gaussians _cm_gaussians_Cf(36.9185, 25.1995, 18.3317, 4.24391, 0.437533, 3.00775, 12.4044, 83.7881, 13.2674);

static cm_Element _cm_element_H(0, "H", "Hydrogen", 0xFFFFFF, 1, 1.09, 0.23, 0.12, 0.32, 0.82, &_cm_gaussians_H, _cm_isotope_H, 2, _cm_henke_H, 501, &_cm_neutron_H);
static cm_Element _cm_element_He(1, "He", "Helium", 0x8000, 2, 1.4, 1.5, 0.3, 1.5, 2, &_cm_gaussians_He, _cm_isotope_He, 2, _cm_henke_He, 501, &_cm_neutron_He);
static cm_Element _cm_element_Li(2, "Li", "Lithium", 0x800080, 3, 1.82, 1.28, 0.16, 1.52, 2.02, &_cm_gaussians_Li, _cm_isotope_Li, 2, _cm_henke_Li, 501, &_cm_neutron_Li);
static cm_Element _cm_element_Be(3, "Be", "Beryllium", 0x8000, 4, 2.00, 0.96, 0.3, 1.11, 1.61, &_cm_gaussians_Be, _cm_isotope_Be, 1, _cm_henke_Be, 502, &_cm_neutron_Be);
static cm_Element _cm_element_B(4, "B", "Boron", 0xFFFF, 5, 2.00, 0.83, 0.2, 0.82, 1.32, &_cm_gaussians_B, _cm_isotope_B, 2, _cm_henke_B, 502, &_cm_neutron_B);
static cm_Element _cm_element_C(5, "C", "Carbon", 0x0, 6, 1.7, 0.68, 0.2, 0.77, 1.27, &_cm_gaussians_C, _cm_isotope_C, 2, _cm_henke_C, 502, &_cm_neutron_C);
static cm_Element _cm_element_N(6, "N", "Nitrogen", 0xFF0000, 7, 1.55, 0.68, 0.2, 0.7, 1.2, &_cm_gaussians_N, _cm_isotope_N, 2, _cm_henke_N, 503, &_cm_neutron_N);
static cm_Element _cm_element_O(7, "O", "Oxygen", 0xFF, 8, 1.52, 0.68, 0.2, 0.66, 1.16, &_cm_gaussians_O, _cm_isotope_O, 3, _cm_henke_O, 502, &_cm_neutron_O);
static cm_Element _cm_element_F(8, "F", "Fluorine", 0x8000, 9, 1.47, 0.64, 0.2, 0.64, 1.14, &_cm_gaussians_F, _cm_isotope_F, 1, _cm_henke_F, 502, &_cm_neutron_F);
static cm_Element _cm_element_Ne(9, "Ne", "Neon", 0x8000, 10, 1.54, 1.5, 0.3, 1.5, 2, &_cm_gaussians_Ne, _cm_isotope_Ne, 3, _cm_henke_Ne, 503, &_cm_neutron_Ne);
static cm_Element _cm_element_Na(10, "Na", "Sodium", 0x808000, 11, 2.27, 1.66, 0.2, 1.86, 2.36, &_cm_gaussians_Na, _cm_isotope_Na, 1, _cm_henke_Na, 504, &_cm_neutron_Na);
static cm_Element _cm_element_Mg(11, "Mg", "Magnesium", 0x8000, 12, 1.73, 1.41, 0.25, 1.6, 2.1, &_cm_gaussians_Mg, _cm_isotope_Mg, 3, _cm_henke_Mg, 504, &_cm_neutron_Mg);
static cm_Element _cm_element_Al(12, "Al", "Aluminium", 0x808000, 13, 2.00, 1.21, 0.25, 1.25, 1.75, &_cm_gaussians_Al, _cm_isotope_Al, 1, _cm_henke_Al, 504, &_cm_neutron_Al);
static cm_Element _cm_element_Si(13, "Si", "Silicon", 0xFF, 14, 2.1, 1.2, 0.3, 1.17, 1.67, &_cm_gaussians_Si, _cm_isotope_Si, 3, _cm_henke_Si, 504, &_cm_neutron_Si);
static cm_Element _cm_element_P(14, "P", "Phosphorus", 0x800080, 15, 1.8, 1.05, 0.3, 1.1, 1.6, &_cm_gaussians_P, _cm_isotope_P, 1, _cm_henke_P, 504, &_cm_neutron_P);
static cm_Element _cm_element_S(15, "S", "Sulfur", 0xFFFF, 16, 1.8, 1.02, 0.3, 1.03, 1.53, &_cm_gaussians_S, _cm_isotope_S, 4, _cm_henke_S, 504, &_cm_neutron_S);
static cm_Element _cm_element_Cl(16, "Cl", "Chlorine", 0x8000, 17, 1.75, 0.99, 0.3, 0.99, 1.49, &_cm_gaussians_Cl, _cm_isotope_Cl, 2, _cm_henke_Cl, 504, &_cm_neutron_Cl);
static cm_Element _cm_element_Ar(17, "Ar", "Argon", 0x8000, 18, 1.88, 1.51, 0.3, 1.5, 2, &_cm_gaussians_Ar, _cm_isotope_Ar, 3, _cm_henke_Ar, 506, &_cm_neutron_Ar);
static cm_Element _cm_element_K(18, "K", "Potassium", 0x808000, 19, 2.75, 2.03, 0.25, 2.27, 2.77, &_cm_gaussians_K, _cm_isotope_K, 3, _cm_henke_K, 503, &_cm_neutron_K);
static cm_Element _cm_element_Ca(19, "Ca", "Calcium", 0x8000, 20, 2.00, 1.76, 0.25, 1.97, 2.47, &_cm_gaussians_Ca, _cm_isotope_Ca, 6, _cm_henke_Ca, 504, &_cm_neutron_Ca);
static cm_Element _cm_element_Sc(20, "Sc", "Scandium", 0x8000, 21, 2.00, 1.7, 0.25, 1.61, 2.11, &_cm_gaussians_Sc, _cm_isotope_Sc, 1, _cm_henke_Sc, 504, &_cm_neutron_Sc);
static cm_Element _cm_element_Ti(21, "Ti", "Titanium", 0x8000, 22, 2.00, 1.6, 0.25, 1.45, 1.95, &_cm_gaussians_Ti, _cm_isotope_Ti, 5, _cm_henke_Ti, 504, &_cm_neutron_Ti);
static cm_Element _cm_element_V(22, "V", "Vanadium", 0x8000, 23, 2.00, 1.53, 0.25, 1.31, 1.81, &_cm_gaussians_V, _cm_isotope_V, 2, _cm_henke_V, 504, &_cm_neutron_V);
static cm_Element _cm_element_Cr(23, "Cr", "Chromium", 0x800080, 24, 2.00, 1.39, 0.25, 1.24, 1.74, &_cm_gaussians_Cr, _cm_isotope_Cr, 4, _cm_henke_Cr, 504, &_cm_neutron_Cr);
static cm_Element _cm_element_Mn(24, "Mn", "Manganese", 0x808000, 25, 2.00, 1.61, 0.25, 1.37, 1.87, &_cm_gaussians_Mn, _cm_isotope_Mn, 1, _cm_henke_Mn, 504, &_cm_neutron_Mn);
static cm_Element _cm_element_Fe(25, "Fe", "Iron", 0xFF, 26, 2.00, 1.52, 0.25, 1.24, 1.74, &_cm_gaussians_Fe, _cm_isotope_Fe, 4, _cm_henke_Fe, 504, &_cm_neutron_Fe);
static cm_Element _cm_element_Co(26, "Co", "Cobalt", 0xFF0000, 27, 2.00, 1.26, 0.25, 1.25, 1.75, &_cm_gaussians_Co, _cm_isotope_Co, 1, _cm_henke_Co, 504, &_cm_neutron_Co);
static cm_Element _cm_element_Ni(27, "Ni", "Nickel", 0x8000, 28, 1.63, 1.24, 0.25, 1.25, 1.75, &_cm_gaussians_Ni, _cm_isotope_Ni, 5, _cm_henke_Ni, 504, &_cm_neutron_Ni);
static cm_Element _cm_element_Cu(28, "Cu", "Copper", 0xFF, 29, 1.4, 1.32, 0.25, 1.28, 1.78, &_cm_gaussians_Cu, _cm_isotope_Cu, 2, _cm_henke_Cu, 504, &_cm_neutron_Cu);
static cm_Element _cm_element_Zn(29, "Zn", "Zinc", 0x8000, 30, 1.39, 1.22, 0.25, 1.33, 1.83, &_cm_gaussians_Zn, _cm_isotope_Zn, 5, _cm_henke_Zn, 504, &_cm_neutron_Zn);
static cm_Element _cm_element_Ga(30, "Ga", "Gallium", 0x8000, 31, 1.87, 1.22, 0.3, 1.26, 1.76, &_cm_gaussians_Ga, _cm_isotope_Ga, 2, _cm_henke_Ga, 506, &_cm_neutron_Ga);
static cm_Element _cm_element_Ge(31, "Ge", "Germanium", 0x8000, 32, 2.00, 1.17, 0.3, 1.22, 1.72, &_cm_gaussians_Ge, _cm_isotope_Ge, 5, _cm_henke_Ge, 506, &_cm_neutron_Ge);
static cm_Element _cm_element_As(32, "As", "Arsenic", 0x8000, 33, 1.85, 1.21, 0.3, 1.21, 1.71, &_cm_gaussians_As, _cm_isotope_As, 1, _cm_henke_As, 506, &_cm_neutron_As);
static cm_Element _cm_element_Se(33, "Se", "Selenium", 0xFFFF, 34, 1.9, 1.22, 0.3, 1.17, 1.67, &_cm_gaussians_Se, _cm_isotope_Se, 6, _cm_henke_Se, 506, &_cm_neutron_Se);
static cm_Element _cm_element_Br(34, "Br", "Bromine", 0xFF, 35, 1.85, 1.21, 0.3, 1.14, 1.64, &_cm_gaussians_Br, _cm_isotope_Br, 2, _cm_henke_Br, 506, &_cm_neutron_Br);
static cm_Element _cm_element_Kr(35, "Kr", "Krypton", 0x8000, 36, 2.02, 1.5, 0.3, 1.5, 2, &_cm_gaussians_Kr, _cm_isotope_Kr, 6, _cm_henke_Kr, 509, &_cm_neutron_Kr);
static cm_Element _cm_element_Rb(36, "Rb", "Rubidium", 0x8000, 37, 2.00, 2.2, 0.3, 2.48, 2.98, &_cm_gaussians_Rb, _cm_isotope_Rb, 2, _cm_henke_Rb, 508, &_cm_neutron_Rb);
static cm_Element _cm_element_Sr(37, "Sr", "Strontium", 0x8000, 38, 2.00, 1.95, 0.3, 1.15, 1.65, &_cm_gaussians_Sr, _cm_isotope_Sr, 4, _cm_henke_Sr, 508, &_cm_neutron_Sr);
static cm_Element _cm_element_Y(38, "Y", "Yttrium", 0x8000, 39, 2.00, 1.9, 0.3, 1.78, 2.28, &_cm_gaussians_Y, _cm_isotope_Y, 1, _cm_henke_Y, 508, &_cm_neutron_Y);
static cm_Element _cm_element_Zr(39, "Zr", "Zirconium", 0x8000, 40, 2.00, 1.75, 0.3, 1.59, 2.09, &_cm_gaussians_Zr, _cm_isotope_Zr, 5, _cm_henke_Zr, 508, &_cm_neutron_Zr);
static cm_Element _cm_element_Nb(40, "Nb", "Niobium", 0x8000, 41, 2.00, 1.64, 0.3, 1.43, 1.93, &_cm_gaussians_Nb, _cm_isotope_Nb, 1, _cm_henke_Nb, 508, &_cm_neutron_Nb);
static cm_Element _cm_element_Mo(41, "Mo", "Molybdenum", 0x8000, 42, 2.00, 1.54, 0.3, 1.36, 1.86, &_cm_gaussians_Mo, _cm_isotope_Mo, 7, _cm_henke_Mo, 508, &_cm_neutron_Mo);
static cm_Element _cm_element_Tc(42, "Tc", "Technetium", 0x8000, 43, 2.00, 1.47, 0.3, 1.35, 1.85, &_cm_gaussians_Tc, NULL, 0, _cm_henke_Tc, 508, &_cm_neutron_Tc);
static cm_Element _cm_element_Ru(43, "Ru", "Ruthenium", 0xFF, 44, 2.00, 1.46, 0.3, 1.33, 1.83, &_cm_gaussians_Ru, _cm_isotope_Ru, 7, _cm_henke_Ru, 508, &_cm_neutron_Ru);
static cm_Element _cm_element_Rh(44, "Rh", "Rhodium", 0x8000, 45, 2.00, 1.45, 0.3, 1.35, 1.85, &_cm_gaussians_Rh, _cm_isotope_Rh, 1, _cm_henke_Rh, 508, &_cm_neutron_Rh);
static cm_Element _cm_element_Pd(45, "Pd", "Palladium", 0x8000, 46, 1.63, 1.39, 0.3, 1.38, 1.88, &_cm_gaussians_Pd, _cm_isotope_Pd, 6, _cm_henke_Pd, 507, &_cm_neutron_Pd);
static cm_Element _cm_element_Ag(46, "Ag", "Silver", 0x800000, 47, 1.72, 1.45, 0.3, 1.44, 1.94, &_cm_gaussians_Ag, _cm_isotope_Ag, 2, _cm_henke_Ag, 508, &_cm_neutron_Ag);
static cm_Element _cm_element_Cd(47, "Cd", "Cadmium", 0x8000, 48, 1.58, 1.44, 0.3, 1.49, 1.99, &_cm_gaussians_Cd, _cm_isotope_Cd, 8, _cm_henke_Cd, 510, &_cm_neutron_Cd);
static cm_Element _cm_element_In(48, "In", "Indium", 0x8000, 49, 1.93, 1.42, 0.3, 1.44, 1.94, &_cm_gaussians_In, _cm_isotope_In, 2, _cm_henke_In, 510, &_cm_neutron_In);
static cm_Element _cm_element_Sn(49, "Sn", "Tin", 0xFFFF, 50, 2.17, 1.39, 0.3, 1.4, 1.9, &_cm_gaussians_Sn, _cm_isotope_Sn, 10, _cm_henke_Sn, 510, &_cm_neutron_Sn);
static cm_Element _cm_element_Sb(50, "Sb", "Antimony", 0xFF, 51, 2.00, 1.39, 0.3, 1.41, 1.91, &_cm_gaussians_Sb, _cm_isotope_Sb, 2, _cm_henke_Sb, 508, &_cm_neutron_Sb);
static cm_Element _cm_element_Te(51, "Te", "Tellurium", 0x8000, 52, 2.06, 1.47, 0.3, 1.37, 1.87, &_cm_gaussians_Te, _cm_isotope_Te, 8, _cm_henke_Te, 508, &_cm_neutron_Te);
static cm_Element _cm_element_I(52, "I", "Iodine", 0x800080, 53, 1.98, 1.4, 0.3, 1.33, 1.83, &_cm_gaussians_I, _cm_isotope_I, 1, _cm_henke_I, 508, &_cm_neutron_I);
static cm_Element _cm_element_Xe(53, "Xe", "Xenon", 0x8000, 54, 2.16, 1.5, 0.3, 1.5, 2, &_cm_gaussians_Xe, _cm_isotope_Xe, 9, _cm_henke_Xe, 509, &_cm_neutron_Xe);
static cm_Element _cm_element_Cs(54, "Cs", "Cesium", 0x8000, 55, 2.00, 2.44, 0.3, 2.65, 3.15, &_cm_gaussians_Cs, _cm_isotope_Cs, 1, _cm_henke_Cs, 508, &_cm_neutron_Cs);
static cm_Element _cm_element_Ba(55, "Ba", "Barium", 0x8000, 56, 2.00, 2.15, 0.3, 2.17, 2.67, &_cm_gaussians_Ba, _cm_isotope_Ba, 7, _cm_henke_Ba, 508, &_cm_neutron_Ba);
static cm_Element _cm_element_La(56, "La", "Lanthanum", 0x8000, 57, 2.00, 2.07, 0.3, 1.87, 2.37, &_cm_gaussians_La, _cm_isotope_La, 2, _cm_henke_La, 508, &_cm_neutron_La);
static cm_Element _cm_element_Ce(57, "Ce", "Cerium", 0x8000, 58, 2.00, 2.04, 0.3, 1.83, 2.33, &_cm_gaussians_Ce, _cm_isotope_Ce, 4, _cm_henke_Ce, 508, &_cm_neutron_Ce);
static cm_Element _cm_element_Pr(58, "Pr", "Praseodymium", 0x8000, 59, 2.00, 2.03, 0.3, 1.82, 2.32, &_cm_gaussians_Pr, _cm_isotope_Pr, 1, _cm_henke_Pr, 508, &_cm_neutron_Pr);
static cm_Element _cm_element_Nd(59, "Nd", "Neodymium", 0x8000, 60, 2.00, 2.01, 0.3, 1.81, 2.31, &_cm_gaussians_Nd, _cm_isotope_Nd, 7, _cm_henke_Nd, 508, &_cm_neutron_Nd);
static cm_Element _cm_element_Pm(60, "Pm", "Promethium", 0x8000, 61, 2.00, 1.99, 0.3, 1.81, 2.31, &_cm_gaussians_Pm, NULL, 0, _cm_henke_Pm, 508, &_cm_neutron_Pm);
static cm_Element _cm_element_Sm(61, "Sm", "Samarium", 0x8000, 62, 2.00, 1.98, 0.3, 1.8, 2.3, &_cm_gaussians_Sm, _cm_isotope_Sm, 7, _cm_henke_Sm, 508, &_cm_neutron_Sm);
static cm_Element _cm_element_Eu(62, "Eu", "Europium", 0x8000, 63, 2.00, 1.98, 0.3, 2, 2.5, &_cm_gaussians_Eu, _cm_isotope_Eu, 2, _cm_henke_Eu, 514, &_cm_neutron_Eu);
static cm_Element _cm_element_Gd(63, "Gd", "Gadolinium", 0x8000, 64, 2.00, 1.96, 0.3, 1.79, 2.29, &_cm_gaussians_Gd, _cm_isotope_Gd, 7, _cm_henke_Gd, 514, &_cm_neutron_Gd);
static cm_Element _cm_element_Tb(64, "Tb", "Terbium", 0x8000, 65, 2.00, 1.94, 0.3, 1.76, 2.26, &_cm_gaussians_Tb, _cm_isotope_Tb, 1, _cm_henke_Tb, 514, &_cm_neutron_Tb);
static cm_Element _cm_element_Dy(65, "Dy", "Dysprosium", 0x8000, 66, 2.00, 1.92, 0.3, 1.75, 2.25, &_cm_gaussians_Dy, _cm_isotope_Dy, 7, _cm_henke_Dy, 514, &_cm_neutron_Dy);
static cm_Element _cm_element_Ho(66, "Ho", "Holmium", 0x8000, 67, 2.00, 1.92, 0.3, 1.74, 2.24, &_cm_gaussians_Ho, _cm_isotope_Ho, 1, _cm_henke_Ho, 514, &_cm_neutron_Ho);
static cm_Element _cm_element_Er(67, "Er", "Erbium", 0x8000, 68, 2.00, 1.89, 0.3, 1.73, 2.23, &_cm_gaussians_Er, _cm_isotope_Er, 6, _cm_henke_Er, 514, &_cm_neutron_Er);
static cm_Element _cm_element_Tm(68, "Tm", "Thulium", 0x8000, 69, 2.00, 1.9, 0.3, 1.72, 2.22, &_cm_gaussians_Tm, _cm_isotope_Tm, 1, _cm_henke_Tm, 514, &_cm_neutron_Tm);
static cm_Element _cm_element_Yb(69, "Yb", "Ytterbium", 0x8000, 70, 2.00, 1.87, 0.3, 1.94, 2.44, &_cm_gaussians_Yb, _cm_isotope_Yb, 7, _cm_henke_Yb, 514, &_cm_neutron_Yb);
static cm_Element _cm_element_Lu(70, "Lu", "Lutetium", 0x8000, 71, 2.00, 1.87, 0.3, 1.72, 2.22, &_cm_gaussians_Lu, _cm_isotope_Lu, 2, _cm_henke_Lu, 514, &_cm_neutron_Lu);
static cm_Element _cm_element_Hf(71, "Hf", "Hafnium", 0x8000, 72, 2.00, 1.75, 0.3, 1.56, 2.06, &_cm_gaussians_Hf, _cm_isotope_Hf, 6, _cm_henke_Hf, 514, &_cm_neutron_Hf);
static cm_Element _cm_element_Ta(72, "Ta", "Tantalum", 0x8000, 73, 2.00, 1.7, 0.3, 1.43, 1.93, &_cm_gaussians_Ta, _cm_isotope_Ta, 2, _cm_henke_Ta, 514, &_cm_neutron_Ta);
static cm_Element _cm_element_W(73, "W", "Tungsten", 0x8000, 74, 2.00, 1.62, 0.3, 1.37, 1.87, &_cm_gaussians_W, _cm_isotope_W, 5, _cm_henke_W, 516, &_cm_neutron_W);
static cm_Element _cm_element_Re(74, "Re", "Rhenium", 0x8000, 75, 2.00, 1.51, 0.3, 1.37, 1.87, &_cm_gaussians_Re, _cm_isotope_Re, 2, _cm_henke_Re, 516, &_cm_neutron_Re);
static cm_Element _cm_element_Os(75, "Os", "Osmium", 0x800000, 76, 2.00, 1.44, 0.3, 1.34, 1.84, &_cm_gaussians_Os, _cm_isotope_Os, 7, _cm_henke_Os, 516, &_cm_neutron_Os);
static cm_Element _cm_element_Ir(76, "Ir", "Iridium", 0x8000, 77, 2.00, 1.41, 0.3, 1.36, 1.86, &_cm_gaussians_Ir, _cm_isotope_Ir, 2, _cm_henke_Ir, 515, &_cm_neutron_Ir);
static cm_Element _cm_element_Pt(77, "Pt", "Platinum", 0x800000, 78, 1.72, 1.36, 0.3, 1.37, 1.87, &_cm_gaussians_Pt, _cm_isotope_Pt, 6, _cm_henke_Pt, 516, &_cm_neutron_Pt);
static cm_Element _cm_element_Au(78, "Au", "Gold", 0xFFFF, 79, 1.66, 1.5, 0.3, 1.44, 1.94, &_cm_gaussians_Au, _cm_isotope_Au, 1, _cm_henke_Au, 506, &_cm_neutron_Au);
static cm_Element _cm_element_Hg(79, "Hg", "Mercury", 0xFFFF, 80, 1.55, 1.32, 0.3, 1.5, 2, &_cm_gaussians_Hg, _cm_isotope_Hg, 7, _cm_henke_Hg, 516, &_cm_neutron_Hg);
static cm_Element _cm_element_Tl(80, "Tl", "Thallium", 0x8000, 81, 1.96, 1.45, 0.3, 1.64, 2.14, &_cm_gaussians_Tl, _cm_isotope_Tl, 2, _cm_henke_Tl, 516, &_cm_neutron_Tl);
static cm_Element _cm_element_Pb(81, "Pb", "Lead", 0x8000, 82, 2.02, 1.46, 0.3, 1.6, 2.1, &_cm_gaussians_Pb, _cm_isotope_Pb, 4, _cm_henke_Pb, 516, &_cm_neutron_Pb);
static cm_Element _cm_element_Bi(82, "Bi", "Bismuth", 0x8000, 83, 2.00, 1.48, 0.3, 1.6, 2.1, &_cm_gaussians_Bi, _cm_isotope_Bi, 1, _cm_henke_Bi, 516, &_cm_neutron_Bi);
static cm_Element _cm_element_Po(83, "Po", "Polonium", 0x8000, 84, 2.00, 1.4, 0.3, 1.6, 2.1, &_cm_gaussians_Po, NULL, 0, _cm_henke_Po, 516, &_cm_neutron_Po);
static cm_Element _cm_element_At(84, "At", "Astatine", 0x8000, 85, 2.00, 1.21, 0.3, 1.6, 2.1, &_cm_gaussians_At, NULL, 0, _cm_henke_At, 516, &_cm_neutron_At);
static cm_Element _cm_element_Rn(85, "Rn", "Radon", 0x8000, 86, 2.00, 1.5, 0.3, 1.8, 2.3, &_cm_gaussians_Rn, NULL, 0, _cm_henke_Rn, 516, &_cm_neutron_Rn);
static cm_Element _cm_element_Fr(86, "Fr", "Francium", 0x8000, 87, 2.00, 2.6, 0.3, 2.8, 3.3, &_cm_gaussians_Fr, NULL, 0, _cm_henke_Fr, 516, &_cm_neutron_Fr);
static cm_Element _cm_element_Ra(87, "Ra", "Radium", 0x8000, 88, 2.00, 2.21, 0.3, 2.2, 2.7, &_cm_gaussians_Ra, NULL, 0, _cm_henke_Ra, 516, &_cm_neutron_Ra);
static cm_Element _cm_element_Ac(88, "Ac", "Actinium", 0x8000, 89, 2.00, 2.15, 0.3, 1.9, 2.4, &_cm_gaussians_Ac, NULL, 0, _cm_henke_Ac, 516, &_cm_neutron_Ac);
static cm_Element _cm_element_Th(89, "Th", "Thorium", 0x8000, 90, 2.00, 2.06, 0.3, 1.85, 2.35, &_cm_gaussians_Th, _cm_isotope_Th, 1, _cm_henke_Th, 516, &_cm_neutron_Th);
static cm_Element _cm_element_Pa(90, "Pa", "Protactinium", 0x8000, 91, 2.00, 2.00, 0.3, 1.8, 2.3, &_cm_gaussians_Pa, _cm_isotope_Pa, 1, _cm_henke_Pa, 516, &_cm_neutron_Pa);
static cm_Element _cm_element_U(91, "U", "Uranium", 0x8000, 92, 1.86, 1.96, 0.3, 1.8, 2.3, &_cm_gaussians_U, _cm_isotope_U, 3, _cm_henke_U, 514, &_cm_neutron_U);
static cm_Element _cm_element_Np(92, "Np", "Neptunium", 0x8000, 93, 2.00, 1.9, 0.3, 1.8, 2.3, &_cm_gaussians_Np, NULL, 0, NULL, 0, &_cm_neutron_Np);
static cm_Element _cm_element_Pu(93, "Pu", "Plutonium", 0x8000, 94, 2.00, 1.87, 0.3, 1.8, 2.3, &_cm_gaussians_Pu, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_Am(94, "Am", "Americium", 0x8000, 95, 2.00, 1.8, 0.3, 1.8, 2.3, &_cm_gaussians_Am, NULL, 0, NULL, 0, &_cm_neutron_Am);
static cm_Element _cm_element_Cm(95, "Cm", "Curium", 0x8000, 96, 2.00, 1.69, 0.3, 1.8, 2.3, &_cm_gaussians_Cm, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_Bk(96, "Bk", "Berkelium", 0x8000, 97, 2.00, 1.54, 0.3, 1.8, 2.3, &_cm_gaussians_Bk, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_Cf(97, "Cf", "Californium", 0x8000, 98, 2.00, 1.83, 0.3, 1.8, 2.3, &_cm_gaussians_Cf, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_Es(98, "Es", "Einsteinium", 0x8000, 99, 2.00, 1.5, 0.3, 1.8, 2.3, NULL, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_Fm(99, "Fm", "Fermium", 0x8000, 100, 2.00, 1.5, 0.3, 1.8, 2.3, NULL, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_Md(100, "Md", "Mendelevium", 0x8000, 101, 2.00, 1.5, 0.3, 1.8, 2.3, NULL, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_No(101, "No", "Nobelium", 0x8000, 102, 2.00, 1.5, 0.3, 1.8, 2.3, NULL, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_Lr(102, "Lr", "Lawrencium", 0x8000, 103, 2.00, 1.5, 0.3, 1.8, 2.3, NULL, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_Ku(103, "Ku", "?", 0x8000, 104, 2.00, 1.5, 0.3, 1.8, 2.3, NULL, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_Q(104, "Q", "Peak", 0x0, -1, 0, 0, 0.2, 0.77, 1.27, NULL, NULL, 0, NULL, 0, NULL);
static cm_Element _cm_element_D(105, "D", "Deuterium", 0xFFFFFF, 1, 1.09, 0.23, 0.12, 0.32, 0.82, &_cm_gaussians_H, _cm_isotope_H, 2, _cm_henke_H, 501, &_cm_neutron_H2);

static const short cm_Element_Count = 106;
cm_Element cm_Elements[cm_Element_Count] = {
  _cm_element_H,
  _cm_element_He,
  _cm_element_Li,
  _cm_element_Be,
  _cm_element_B,
  _cm_element_C,
  _cm_element_N,
  _cm_element_O,
  _cm_element_F,
  _cm_element_Ne,
  _cm_element_Na,
  _cm_element_Mg,
  _cm_element_Al,
  _cm_element_Si,
  _cm_element_P,
  _cm_element_S,
  _cm_element_Cl,
  _cm_element_Ar,
  _cm_element_K,
  _cm_element_Ca,
  _cm_element_Sc,
  _cm_element_Ti,
  _cm_element_V,
  _cm_element_Cr,
  _cm_element_Mn,
  _cm_element_Fe,
  _cm_element_Co,
  _cm_element_Ni,
  _cm_element_Cu,
  _cm_element_Zn,
  _cm_element_Ga,
  _cm_element_Ge,
  _cm_element_As,
  _cm_element_Se,
  _cm_element_Br,
  _cm_element_Kr,
  _cm_element_Rb,
  _cm_element_Sr,
  _cm_element_Y,
  _cm_element_Zr,
  _cm_element_Nb,
  _cm_element_Mo,
  _cm_element_Tc,
  _cm_element_Ru,
  _cm_element_Rh,
  _cm_element_Pd,
  _cm_element_Ag,
  _cm_element_Cd,
  _cm_element_In,
  _cm_element_Sn,
  _cm_element_Sb,
  _cm_element_Te,
  _cm_element_I,
  _cm_element_Xe,
  _cm_element_Cs,
  _cm_element_Ba,
  _cm_element_La,
  _cm_element_Ce,
  _cm_element_Pr,
  _cm_element_Nd,
  _cm_element_Pm,
  _cm_element_Sm,
  _cm_element_Eu,
  _cm_element_Gd,
  _cm_element_Tb,
  _cm_element_Dy,
  _cm_element_Ho,
  _cm_element_Er,
  _cm_element_Tm,
  _cm_element_Yb,
  _cm_element_Lu,
  _cm_element_Hf,
  _cm_element_Ta,
  _cm_element_W,
  _cm_element_Re,
  _cm_element_Os,
  _cm_element_Ir,
  _cm_element_Pt,
  _cm_element_Au,
  _cm_element_Hg,
  _cm_element_Tl,
  _cm_element_Pb,
  _cm_element_Bi,
  _cm_element_Po,
  _cm_element_At,
  _cm_element_Rn,
  _cm_element_Fr,
  _cm_element_Ra,
  _cm_element_Ac,
  _cm_element_Th,
  _cm_element_Pa,
  _cm_element_U,
  _cm_element_Np,
  _cm_element_Pu,
  _cm_element_Am,
  _cm_element_Cm,
  _cm_element_Bk,
  _cm_element_Cf,
  _cm_element_Es,
  _cm_element_Fm,
  _cm_element_Md,
  _cm_element_No,
  _cm_element_Lr,
  _cm_element_Ku,
  _cm_element_Q,
  _cm_element_D
};
//..............................................................................
//..............................................................................
cm_Element* XElementLib::FindBySymbol(const olxstr& symbol) {
  for( short i=0; i < cm_Element_Count; i++ )
    if( cm_Elements[i].symbol.Equalsi(symbol) )
      return &cm_Elements[i];
  return NULL;
}
//..............................................................................
cm_Element& XElementLib::GetByIndex(short ind) {
  return cm_Elements[ind];
}
//..............................................................................
cm_Element* XElementLib::FindByZ(short z) {
  if (z <= 0 || z > iMaxElementIndex )  return NULL;
  int idx = z;
  while (cm_Elements[idx].z < z && ++idx < iMaxElementIndex)
    ;
  if (idx < iMaxElementIndex && cm_Elements[idx].z == z)
    return &cm_Elements[idx];
  while (cm_Elements[idx].z > z && --idx > 0)
    ;
  if (idx >= 0 && cm_Elements[idx].z == z)
    return &cm_Elements[idx];
  return NULL;
}
//..............................................................................
cm_Element* XElementLib::NextZ(const cm_Element& elm)  {
  if( elm.z == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot iterate the Q-peaks");
  short idx = elm.index;
  while (++idx < cm_Element_Count && cm_Elements[idx].z != elm.z+1)
    ;
  return (idx < cm_Element_Count) ? &cm_Elements[idx] : NULL;
}
//..............................................................................
cm_Element* XElementLib::PrevZ(const cm_Element& elm)  {
  if( elm.z == -1 )
    throw TInvalidArgumentException(__OlxSourceInfo, "cannot iterate the Q-peaks");
  short idx = elm.index;
  while (--idx >= 0 && cm_Elements[idx].z != elm.z-1)
    ;
  return (idx >=0 ) ? &cm_Elements[idx] : NULL;
}
//..............................................................................
cm_Element *XElementLib::NextGroup(int group, const cm_Element *e) {
  if (e == NULL) return NULL;
  if (IsGroup(group, *e))
    return &cm_Elements[e->index];
  short idx = e->index;
  while (++idx < cm_Element_Count && !IsGroup(group, cm_Elements[idx]))
    ;
  return (idx < cm_Element_Count) ? &cm_Elements[idx] : NULL;
}
//..............................................................................
cm_Element *XElementLib::PrevGroup(int group, const cm_Element *e) {
  if (e == NULL) return NULL;
  if (IsGroup(group, *e))
    return &cm_Elements[e->index];
  short idx = e->index;
  while (--idx >= 0 && !IsGroup(group, cm_Elements[idx]))
    ;
  return (idx >= 0) ? &cm_Elements[idx] : NULL;
}
//..............................................................................
cm_Element* XElementLib::FindBySymbolEx(const olxstr& label)  {
  if( label.IsEmpty() )  return NULL;
  // check if first char is a letter
  if( !( (label.CharAt(0) >= 'a' && label.CharAt(0) <= 'z') ||
    ( label.CharAt(0) >= 'A' && label.CharAt(0) <= 'Z') ) )
    return NULL;
  olxstr lbl(label.ToUpperCase());
  if( lbl.Length() > 2 )  lbl = lbl.SubStringTo(2);
  // check for 2 char label
  if( lbl.Length() == 2 && (lbl.CharAt(1) >= 'A' && lbl.CharAt(1) <= 'Z') )  {
    for( short i=0; i < cm_Element_Count; i++ )
      if( cm_Elements[i].symbol.Length() == 2 && cm_Elements[i].symbol.Equalsi(lbl) )
        return &cm_Elements[i];
  }
  //check for single char label
  for( short i=0; i < cm_Element_Count; i++ ) {
    if( cm_Elements[i].symbol.Length() == 1 &&
        (cm_Elements[i].symbol.CharAt(0) == lbl.CharAt(0)) )
    {
      return &cm_Elements[i];
    }
  }
  return NULL;
}
//..............................................................................
ContentList& XElementLib::ParseElementString(const olxstr& su,
  ContentList& res)
{
  olxstr elm, cnt;
  bool nowCnt = false;
  TStrList toks;
  for (size_t i=0; i < su.Length(); i++) {
    if (olxstr::o_iswhitechar(su[i]))  continue;
    if (nowCnt) {
      if (olxstr::o_isdigit(su[i]) || su[i] == '.')
        cnt << su[i];
      else {
        if (!elm.IsEmpty() && !cnt.IsEmpty()) {
          toks.Clear();
          ParseSimpleElementStr(elm, toks);
          for (size_t i=0; i < toks.Count()-1; i++)
            ExpandShortcut(toks[i], res);
          ExpandShortcut(toks.GetLastString(), res, cnt.ToDouble());
          cnt.SetLength(0);
        }
        nowCnt = false;
        elm = su[i];
      }
    }
    else {
      if (!( (su[i] >= '0' && su[i] <= '9') || su[i] == '.'))
        elm << su[i];
      else {
        nowCnt = true;
        cnt = su[i];
      }
    }
  }
  if (!elm.IsEmpty()) {
    toks.Clear();
    ParseSimpleElementStr(elm, toks);
    for (size_t i=0; i < toks.Count()-1; i++)
      ExpandShortcut(toks[i], res);
    ExpandShortcut(toks.GetLastString(), res,
      cnt.IsEmpty() ? 1 : cnt.ToDouble());
  }
  return res;
}
//..............................................................................
void XElementLib::ParseSimpleElementStr(const olxstr& str, TStrList& toks)  {
  if (str.Length() == 1) {
    if (IsElement(str) || IsShortcut(str))
      toks.Add(str);
    else {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Unknown element: ").quote() << str);
    }
  }
  else if (str.Length() == 2) {
    // both capital? prioritise two elements
    if (str[0] >= 'A' && str[0] <= 'Z' && str[1] >= 'A' && str[1] <= 'Z') {
      if (IsElement(str[0]) && IsElement(str[1])) {
        toks.Add(str[0]);
        toks.Add(str[1]);
      }
    }
    else {
      if (IsElement(str) || IsShortcut(str))
        toks.Add(str);
      else if (IsElement(str[0]) || IsElement(str[1])) {
        toks.Add(str[0]);
        toks.Add(str[1]);
      }
      else {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Unknown element: ").quote() << str);
      }
    }
  }
  else {
    size_t st=0;
    for (size_t i=1; i < str.Length(); i++)  {
      if (olxstr::o_isalpha(str[i])) {
        if (st != i) {
          if (str[i] >= 'a' && str[i] <= 'z') {
            olxstr tmp = str.SubString(st, i-st+1);
            if (IsElement(tmp) || IsShortcut(tmp)) {
              toks.Add(tmp);
              st = ++i;
              continue;
            }
          }
          olxstr tmp = str.SubString(st, i-st);
          if (!IsElement(tmp) && !IsShortcut(tmp)) {
            throw TFunctionFailedException(__OlxSourceInfo,
              olxstr("Unknown element: ").quote() << tmp);
          }
          toks.Add(tmp);
          st = i;
        }
        else {
          st = i;
        }
      }
    }
    if (st < str.Length())  {
      olxstr tmp = str.SubStringFrom(st);
      if(!IsElement(tmp) && !IsShortcut(tmp)) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Unknown element - ") << tmp);
      }
      toks.Add(tmp);
    }
  }
}
//..............................................................................
void XElementLib::ExpandShortcut(const olxstr& sh, ContentList& res, double cnt)  {
  ContentList shc;
  if( sh.Equalsi("Ph") )  {
    shc.AddNew(XElementLib::GetByIndex(iCarbonIndex), 6);
    shc.AddNew(XElementLib::GetByIndex(iHydrogenIndex), 5);
  }
  else if( sh.Equalsi("Py") )  {
    shc.AddNew(XElementLib::GetByIndex(iCarbonIndex), 5);
    shc.AddNew(XElementLib::GetByIndex(iNitrogenIndex), 1);
    shc.AddNew(XElementLib::GetByIndex(iHydrogenIndex), 4);
  }
  else if( sh.Equalsi("Tf") )  {
    shc.AddNew(XElementLib::GetByIndex(iCarbonIndex), 1);
    shc.AddNew(XElementLib::GetByIndex(iSulphurIndex), 1);
    shc.AddNew(XElementLib::GetByIndex(iOxygenIndex), 2);
    shc.AddNew(XElementLib::GetByIndex(iFluorineIndex), 3);
  }
  else if( sh.Equalsi("Cp") )  {
    shc.AddNew(XElementLib::GetByIndex(iCarbonIndex), 5);
    shc.AddNew(XElementLib::GetByIndex(iHydrogenIndex), 5);
  }
  else if( sh.Equalsi("Me") )  {
    shc.AddNew(XElementLib::GetByIndex(iCarbonIndex), 1);
    shc.AddNew(XElementLib::GetByIndex(iHydrogenIndex), 3);
  }
  else if( sh.Equalsi("Et") )  {
    shc.AddNew(XElementLib::GetByIndex(iCarbonIndex), 2);
    shc.AddNew(XElementLib::GetByIndex(iHydrogenIndex), 5);
  }
  else if( sh.Equalsi("Bu") )  {
    shc.AddNew(XElementLib::GetByIndex(iCarbonIndex), 4);
    shc.AddNew(XElementLib::GetByIndex(iHydrogenIndex), 9);
  }
  else  {  // just add whatever is provided
    cm_Element* elm = XElementLib::FindBySymbol(sh);
    if( elm == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "element/shortcut");
    shc.AddNew(*elm, 1);
  }

  for( size_t i=0; i < shc.Count(); i++ )  {
    shc[i].count *= cnt;
    bool found = false;
    for( size_t j=0; j < res.Count(); j++ )  {
      if( res[j].element == shc[i].element )  {
        res[j] += shc[i];
        found = true;
        break;
      }
    }
    if( !found )
      res.AddCopy(shc[i]);
  }
}
//..............................................................................
ContentList& XElementLib::SortContentList(ContentList& cl)  {
  const cm_Element *c_type = NULL, *h_type = NULL;
  ElementPList elms;
  for( size_t i=0; i < cl.Count(); i++ )  {
    elms.Add(cl[i].element);
    if( *elms.GetLast() == iCarbonZ )
      c_type = elms.GetLast();
    else if( *elms.GetLast() == iHydrogenZ )
      h_type = elms.GetLast();
  }
  QuickSorter::Sort(elms,
    ElementSymbolSorter(),
    SyncSwapListener::Make(cl));
  if( c_type != NULL && elms.Count() > 1 )  {
    size_t ind = elms.IndexOf(c_type);
    cl.Move(ind, 0);
  }
  if( h_type != NULL && elms.Count() > 2 )  {
    size_t ind = elms.IndexOf(h_type);
    cl.Move(ind, 1);
  }
  return cl;
}
//..............................................................................
