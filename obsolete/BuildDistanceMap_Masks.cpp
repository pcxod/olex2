/* this procedure is one of many in an attempt to optimise the void calculations.
It creates a different mask for every distinct radius; every mask consists of 
zero level - the core mask for the atom and the following shells calculated with 
the map resolution. Then incrementally masks are expanded for all atoms of the unit
cell until all map points are initialised. It was shown that the procedure is fast,
but not as accurate is direct calculation. The main cause of the errors is the
'anti-aliasing' problem which arises then mask is snapped on the closest to the
atom centre grid point; although the maximum remote distance and channel sizes are 
often very close to the direct calculation, the packing index might be miles off;
therefore it cannon be used for getting reliable (packing) figures but can serve 
as an estimate of the structure porosity. However, the masks occupy
a lot of memory, and a simpler procedure - with single mask reused for all atoms
is left in the implementation.
*/
void TUnitCell::BuildDistanceMap_Masks(TArray3D<short>& map, double delta, short val, 
                                  ElementRadii* _radii, const TCAtomPList* _template)
{
  TTypeList<AnAssociation3<vec3d,TCAtom*,short> > allAtoms;
  GenereteAtomCoordinates(allAtoms, true, _template);
  const vec3i dim(map.Length1(), map.Length2(), map.Length3());
  // angstrem per pixel scale
  double capp = Lattice->GetAsymmUnit().Axes()[2].GetV()/dim[2],
         bapp = Lattice->GetAsymmUnit().Axes()[1].GetV()/dim[1],
         aapp = Lattice->GetAsymmUnit().Axes()[0].GetV()/dim[0];
  // precalculate the sphere/ellipsoid etc coordinates for all distinct scatterers
  const TAsymmUnit& au = GetLattice().GetAsymmUnit();
  const double sin_a = sin(au.GetAngles()[0].GetV()*M_PI/180);
  const double sin_b = sin(au.GetAngles()[1].GetV()*M_PI/180);
  const double sin_g = sin(au.GetAngles()[2].GetV()*M_PI/180);
  const double max_r = 15;
  const int shell_res = olx_round(1./aapp);
  // type : R
  olxdict<short, double, TPrimitiveComparator> radii;
  // R : shells
  olxdict<double, TArrayList<vec3i_list>, TPrimitiveComparator> atom_shells;
  olxdict<double, vec3i_list, TPrimitiveComparator> zero_shells;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    size_t ind = radii.IndexOf(au.GetAtom(i).GetType().index);
    if( ind != InvalidIndex )  continue;
    double r = au.GetAtom(i).GetType().r_sfil + delta;
    if( _radii != NULL )  {
      size_t b_i = _radii->IndexOf(&au.GetAtom(i).GetType());
      if( b_i != InvalidIndex )
        r = _radii->GetValue(b_i) + delta;
    }
    radii.Add(au.GetAtom(i).GetType().index, r);
    if( atom_shells.IndexOf(r) == InvalidIndex )  {
      atom_shells.Add(r).SetCount(olx_round((max_r-r)*shell_res)+1);
      zero_shells.Add(r);//.SetIncrement(512);
    }
  }

  const int ad = olx_round(olx_max(max_r/sin_b, max_r/sin_g)/aapp);
  const int bd = olx_round(olx_max(max_r/sin_a, max_r/sin_g)/aapp);
  const int cd = olx_round(olx_max(max_r/sin_a, max_r/sin_b)/aapp);
  for( int x=-ad; x <= ad; x++ )  {
    for( int y=-bd; y <= bd; y++ )  {
      for( int z=-cd; z <= cd; z++ )  {
        vec3d v((double)x/dim[0], (double)y/dim[1], (double)z/dim[2]);
        au.CellToCartesian(v);
        const double r = v.Length();
        for( size_t i=0; i < atom_shells.Count(); i++ )  {
          if( r <= atom_shells.GetKey(i) )
            zero_shells.GetValue(i).AddNew(x,y,z);
          else if( r < max_r )  {
            short shell_index = (short)((r-atom_shells.GetKey(i))*shell_res);
            atom_shells.GetValue(i)[shell_index].AddNew(x,y,z);
          }
        }
      }
    }
  }
  // this builds the structure map
  for( size_t i=0; i < allAtoms.Count(); i++ )  {
    allAtoms[i].A() *= dim;
    size_t atom_ind = zero_shells.IndexOf(radii[allAtoms[i].GetB()->GetType().index]);
    allAtoms[i].C() = atom_ind;
    const vec3i_list& zero_shell = zero_shells.GetValue(atom_ind);
    for( size_t j=0; j < zero_shell.Count(); j++ )  {
      vec3i crd = allAtoms[i].A() + zero_shell[j];
      for( size_t k=0; k < 3; k++ )  {
        if( crd[k] < 0 )
          crd[k] += dim[k];
        else if( crd[k] >= dim[k] )
          crd[k] -= dim[k];
      }
      map.Data[crd[0]][crd[1]][crd[2]] = val;
    }
  }
  short shell_expansion=0;
  bool changes = true;
  while( changes )  {
    changes = false;
    const short map_val = shell_expansion+1;//*shell_res;
    size_t unused_atoms=0;
    for( size_t i=0; i < allAtoms.Count(); i++ )  {
      const vec3i_list& shell = atom_shells.GetValue(allAtoms[i].GetC())[shell_expansion];
      short atom_used = false;
      for( size_t j=0; j < shell.Count(); j++ )  {
        vec3i crd = allAtoms[i].GetA() + shell[j];
        for( size_t k=0; k < 3; k++ )  {
          if( crd[k] < 0 )
            crd[k] += dim[k];
          else if( crd[k] >= dim[k] )
            crd[k] -= dim[k];
        }
        short& cv = map.Data[crd[0]][crd[1]][crd[2]];
        if( cv > map_val )  {
          cv = map_val;
          atom_used = changes = true;
        }
      }
      if( !atom_used )  {
        allAtoms.NullItem(i);
        unused_atoms++;
      }
    }
    if( unused_atoms > 0 )
      allAtoms.Pack();
    shell_expansion++;
  }
}
