
#ifndef _inv_h
#define _inv_h

#include "ap.h"

#include "lu.h"
#include "trinverse.h"


/*************************************************************************
Inversion of a matrix given by its LU decomposition.

Input parameters:
    A       -   LU decomposition of the matrix (output of LUDecomposition subroutine).
    Pivots  -   table of permutations which were made during the LU decomposition
                (the output of LUDecomposition subroutine).
    N       -   size of matrix A.

Output parameters:
    A       -   inverse of matrix A.
                Array whose indexes range within [1..N, 1..N].

Result:
    True, if the matrix is not singular.
    False, if the matrix is singular.

  -- LAPACK routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     February 29, 1992
*************************************************************************/
bool inverselu(ap::real_2d_array& a,
     const ap::integer_1d_array& pivots,
     int n);


/*************************************************************************
Inversion of a general matrix.

Input parameters:
    A   -   matrix. Array whose indexes range within [1..N, 1..N].
    N   -   size of matrix A.

Output parameters:
    A   -   inverse of matrix A.
            Array whose indexes range within [1..N, 1..N].

Result:
    True, if the matrix is not singular.
    False, if the matrix is singular.

  -- ALGLIB --
     Copyright 2005 by Bochkanov Sergey
*************************************************************************/
bool inverse(ap::real_2d_array& a, int n);


#endif
