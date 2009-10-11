#ifndef _SVDHEADER_
#define _SVDHEADER_
#include "ap.h"

bool svddecomposition(ap::real_2d_array a,
     int m,
     int n,
     int uneeded,
     int vtneeded,
     int additionalmemory,
     ap::real_1d_array& w,
     ap::real_2d_array& u,
     ap::real_2d_array& vt); 
#endif

