
#ifndef _lu_h
#define _lu_h

#include "ap.h"

/*************************************************************************
LU decomposition of a general matrix of size MxN

The subroutine calculates the LU decomposition of a rectangular general
matrix with partial pivoting (with row permutations).

Input parameters:
    A   -   matrix A whose indexes range within [1..M, 1..N].
    M   -   number of rows in matrix A.
    N   -   number of columns in matrix A.

Output parameters:
    A   -   matrices L and U in compact form (see below).
            Array whose indexes range within [1..M, 1..N].
    Pivots - permutation matrix in compact form (see below).
            Array whose index ranges within [1..Min(M,N)].

Matrix A is represented as A = P * L * U, where P is a permutation matrix,
matrix L - lower triangular (or lower trapezoid, if M>N) matrix,
U - upper triangular (or upper trapezoid, if M<N) matrix.

Let M be equal to 4 and N be equal to 3:

                   (  1          )    ( U11 U12 U13  )
A = P1 * P2 * P3 * ( L21  1      )  * (     U22 U23  )
                   ( L31 L32  1  )    (         U33  )
                   ( L41 L42 L43 )

Matrix L has size MxMin(M,N), matrix U has size Min(M,N)xN, matrix P(i) is
a permutation of the identity matrix of size MxM with numbers I and Pivots[I].

The algorithm returns array Pivots and the following matrix which replaces
matrix A and contains matrices L and U in compact form (the example applies
to M=4, N=3).

 ( U11 U12 U13 )
 ( L21 U22 U23 )
 ( L31 L32 U33 )
 ( L41 L42 L43 )

As we can see, the unit diagonal isn't stored.

  -- LAPACK routine (version 3.0) --
     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
     Courant Institute, Argonne National Lab, and Rice University
     June 30, 1992
*************************************************************************/
void ludecomposition(ap::real_2d_array& a,
     int m,
     int n,
     ap::integer_1d_array& pivots);


/*************************************************************************
LU decomposition of a general matrix of size MxN

It uses LUDecomposition. L and U are not output in compact form, but as
separate general matrices filled up by zero elements in their
corresponding positions.

This subroutine described here only serves the purpose to show
how the result of ComplexLUDecomposition subroutine could be unpacked.

  -- ALGLIB --
     Copyright 2005 by Bochkanov Sergey
*************************************************************************/
void ludecompositionunpacked(ap::real_2d_array a,
     int m,
     int n,
     ap::real_2d_array& l,
     ap::real_2d_array& u,
     ap::integer_1d_array& pivots);


#endif
