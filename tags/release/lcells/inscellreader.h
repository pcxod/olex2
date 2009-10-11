//---------------------------------------------------------------------------

#ifndef inscellreaderH
#define inscellreaderH

#include "ebase.h"
//---------------------------------------------------------------------------
class TInsCellReader
{
	double Fa, Fb, Fc, Faa, Fab, Fac;
	short FLattice;
public:
	TInsCellReader();
	bool _fastcall LoadFromInsFile(const olxstr& FN);
	bool _fastcall LoadFromCifFile(const olxstr& FN);
	__property short Lattice = {read = FLattice};
	__property double a	= {read = Fa};
	__property double b	= {read = Fb};
	__property double c	= {read = Fc};
	__property double aa	= {read = Faa};
	__property double ab	= {read = Fab};
	__property double ac	= {read = Fac};
};
#endif
