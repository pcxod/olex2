/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_ed_analysis_H
#define __olx_xlib_wd_analysis_H
#include "analysis.h"
#include "estopwatch.h"
#include "../maputil.h"
#include "../sfutil.h"
#include "../unitcell.h"
#include "../beevers-lipson.h"
BeginXlibNamespace()

namespace analysis {

  class EDMapAnalysis {
    double TryPoint(const TUnitCell& uc, const vec3i& p,
      const TArray3D<bool> &mask) const
    {
      TRefList refs;
      TArrayList<compd> F;
      TArrayList<SFUtil::StructureFactor> P1SF;
      const TUnitCell::SymSpace sym_space = uc.GetSymSpace();
      SFUtil::GetSF(refs, F, SFUtil::mapTypeDiff,
        SFUtil::sfOriginOlex2, SFUtil::scaleRegression);
      SFUtil::ExpandToP1(refs, F, sym_space, P1SF);
      BVFourier::MapInfo mi = BVFourier::CalcEDM(
        P1SF, Map->Data, Map->GetSize(), uc.CalcVolume());
      return MapUtil::IntegrateMask(Map->Data, Map->GetSize(), p, mask);
    }

    double Resolution, Threshold, R_fraction,
      used_map_res;
    TArray3D<float> *Map;
    void Init(const vec3d &cell_dim)  {
      if( Map == NULL || used_map_res != Resolution )  {
        used_map_res = Resolution;
        if( Map != NULL )
          delete Map;
        const vec3i dim(cell_dim/Resolution);
        Map = new TArray3D<float>(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
      }
    }
    static int sort_types(const AnAssociation2<const cm_Element*, double> *i,
      const AnAssociation2<const cm_Element*, double> *j)
    {
      return olx_cmp(olx_abs(i->GetB()), olx_abs(j->GetB()));
    }
  public:
    EDMapAnalysis()
      : Resolution(0.2),
        Threshold(3),
        R_fraction(0.5),
        Map(NULL)
    {
    
    }
    ~EDMapAnalysis()  {
      if( Map != NULL )
        delete Map;
    }
    DefPropP(double, Resolution)
    DefPropP(double, Threshold)
    DefPropP(double, R_fraction)

    ConstTypeList<AnAssociation2<const cm_Element*, double> >
    TryAtomType(const TLattice &latt, TCAtom &a, const ElementPList &elements)
    {
      const TAsymmUnit& au = latt.GetAsymmUnit();
      const TUnitCell& uc = latt.GetUnitCell();
      Init(au.GetAxes());
      olx_object_ptr<TArray3D<bool> > mask_ptr =
        uc.BuildAtomMask(Map->GetSize(), a.GetType().r_vdw*R_fraction);
      TArray3D<bool> &mask = mask_ptr();
      size_t mask_size = 0;
      for( size_t ix=0; ix < mask.Length1(); ix++ )  {
        for( size_t iy=0; iy < mask.Length2(); iy++ )
          for( size_t iz=0; iz < mask.Length3(); iz++ )
            if( mask.Data[ix][iy][iz] )
              mask_size++;
      }
      const cm_Element &original_type = a.GetType();
      TTypeList<AnAssociation2<const cm_Element*, double> > res(elements.Count());
      for( size_t i=0; i < elements.Count(); i++ )  {
        a.SetType(*elements[i]);
        res.Set(i,
          new AnAssociation2<const cm_Element*, double>(elements[i],
            TryPoint(uc,
            (a.ccrd()*Map->GetSize()).Round<int>(),
            mask)/mask_size));
      }
      a.SetType(original_type);
      res.QuickSorter.SortSF(res, &EDMapAnalysis::sort_types);
      return res;
    }

    ConstTypeList<Result> Analyse(TLattice &latt) {
      TTypeList<Result> res;
      TStopWatch sw(__FUNC__);
      TRefList refs;
      TArrayList<compd> F;
      olxstr err(SFUtil::GetSF(refs, F, SFUtil::mapTypeDiff, SFUtil::sfOriginOlex2, SFUtil::scaleRegression));
      if( !err.IsEmpty() )
        throw TFunctionFailedException(__OlxSourceInfo, err);
      TAsymmUnit& au = latt.GetAsymmUnit();
      TUnitCell& uc = latt.GetUnitCell();
      TArrayList<SFUtil::StructureFactor> P1SF;
      const TUnitCell::SymSpace sym_space = uc.GetSymSpace();
      sw.start("Expanding structure factors to P1 (fast symm)");
      SFUtil::ExpandToP1(refs, F, sym_space, P1SF);
      sw.stop();
      const double vol = uc.CalcVolume();
      Init(au.GetAxes());
      TArrayList<AnAssociation3<TCAtom*,double, size_t> > atoms(au.AtomCount());
      for( size_t i=0; i < au.AtomCount(); i++ )  {
        atoms[i].A() = &au.GetAtom(i);
        atoms[i].B() = 0;
        atoms[i].C() = 0;
        atoms[i].A()->SetTag(i);
      }
      size_t found_cnt = 0;
      sw.start("Calculating electron density map in P1 (Beevers-Lipson)");
      BVFourier::MapInfo mi = BVFourier::CalcEDM(P1SF, Map->Data, Map->GetSize(), vol);
      sw.stop();
      TArrayList<MapUtil::peak> _Peaks;
      TTypeList<MapUtil::peak> Peaks;
      sw.start("Integrating P1 map: ");
      ElementRadii radii;
      for( size_t i=0; i < au.AtomCount(); i++ )  {
        if( radii.IndexOf(&au.GetAtom(i).GetType()) == InvalidIndex )
          radii.Add(&au.GetAtom(i).GetType(), au.GetAtom(i).GetType().r_vdw*R_fraction);
      }
      olxdict<short, TArray3D<bool>*, TPrimitiveComparator> atom_masks =
        uc.BuildAtomMasks(Map->GetSize(), &radii, 0);
      TSizeList mask_sizes(atom_masks.Count());
      for( size_t i=0; i < atom_masks.Count(); i++ )  {
        TArray3D<bool> &mask = *atom_masks.GetValue(i);
        size_t cnt = 0;
        for( size_t ix=0; ix < mask.Length1(); ix++ )  {
          for( size_t iy=0; iy < mask.Length2(); iy++ )
            for( size_t iz=0; iz < mask.Length3(); iz++ )
              if( mask.Data[ix][iy][iz] )
                cnt++;
        }
        mask_sizes[i] = cnt;
      }
      for( size_t i=0; i < atoms.Count(); i++ )  {
        if( atoms[i].GetA()->IsDeleted() || atoms[i].GetA()->GetType() == iQPeakZ )
          continue;
        vec3i p = (atoms[i].A()->ccrd()*Map->GetSize()).Round<int>();
        size_t ti = atom_masks.IndexOf(atoms[i].GetA()->GetType().index);
        atoms[i].B() = MapUtil::IntegrateMask(Map->Data, Map->GetSize(), p,
          *atom_masks.GetValue(ti));
        atoms[i].C() = mask_sizes[ti];
      }
      const double minEd = mi.sigma*Threshold;
      for( size_t i=0; i < atoms.Count(); i++ )  {
        if( atoms[i].GetC() == 0 )  continue;
        const double ed = atoms[i].GetB()/atoms[i].GetC();  
        if( olx_abs(ed) < minEd )  continue;
        const cm_Element& original_type = atoms[i].GetA()->GetType();
        const size_t ti = atom_masks.IndexOf(original_type.index);
        TArray3D<bool> &mask = *atom_masks.GetValue(ti);
        TTypeList<AnAssociation2<const cm_Element*, double> > tries;
        tries.AddNew(&original_type, ed);
        if( ed < 0 )  {
          double p_ed = ed;
          const cm_Element* p_e = &original_type;
          while( p_ed < 0 )  {
            p_e = XElementLib::PrevZ(*p_e);
            if( p_e == NULL )  break;
            atoms[i].GetA()->SetType(*p_e);
            sw.start("Trying previous element");
            p_ed = TryPoint(uc,
              (atoms[i].GetA()->ccrd()*Map->GetSize()).Round<int>(),
              mask)/mask_sizes[ti];
            sw.stop();
            tries.AddNew(p_e, p_ed);
          }
        }
        else  {
          double n_ed = ed;
          const cm_Element* n_e = &original_type;
          while( n_ed > 0 )  {
            n_e = XElementLib::NextZ(*n_e);
            if( n_e == NULL )  break;
            atoms[i].GetA()->SetType(*n_e);
            sw.start("Trying next element");
            n_ed = TryPoint(uc,
              (atoms[i].GetA()->ccrd()*Map->GetSize()).Round<int>(),
              mask)/mask_sizes[ti];
            sw.stop();
            tries.AddNew(n_e, n_ed);
          }
        }
        atoms[i].GetA()->SetType(original_type);
        if( tries.Count() > 1 )  {
          size_t li = tries.Count()-1, pli = li-1;
          const cm_Element *proposed =
            (olx_abs(tries[li].GetB()) < olx_abs(tries[pli].GetB()))
              ? tries[li].GetA() : tries[pli].GetA();
          if( proposed != &original_type )
            res.Add(new Result(*atoms[i].A(), *proposed));
        }
      }
      for( size_t i=0; i < atom_masks.Count(); i++ )
        delete atom_masks.GetValue(i);
      sw.print(TBasicApp::NewLogEntry(logInfo));
      return res;
    }
  };
};
EndXlibNamespace()
#endif
