
#include "inv.h"

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
     int n)
{
    bool result;
    ap::real_1d_array work;
    int i;
    int j;
    int jp;
    int jp1;
    double v;
    int i_;

    result = true;
    
    //
    // Quick return if possible
    //
    if( n==0 )
    {
        return result;
    }
    work.setbounds(1, n);
    
    //
    // Form inv(U)
    //
    if( !invtriangular(a, n, true, false) )
    {
        result = false;
        return result;
    }
    
    //
    // Solve the equation inv(A)*L = inv(U) for inv(A).
    //
    for(j = n; j >= 1; j--)
    {
        
        //
        // Copy current column of L to WORK and replace with zeros.
        //
        for(i = j+1; i <= n; i++)
        {
            work(i) = a(i,j);
            a(i,j) = 0;
        }
        
        //
        // Compute current column of inv(A).
        //
        if( j<n )
        {
            jp1 = j+1;
            for(i = 1; i <= n; i++)
            {
                v = 0.0;
                for(i_=jp1; i_<=n;i_++)
                {
                    v += a(i,i_)*work(i_);
                }
                a(i,j) = a(i,j)-v;
            }
        }
    }
    
    //
    // Apply column interchanges.
    //
    for(j = n-1; j >= 1; j--)
    {
        jp = pivots(j);
        if( jp!=j )
        {
            for(i_=1; i_<=n;i_++)
            {
                work(i_) = a(i_,j);
            }
            for(i_=1; i_<=n;i_++)
            {
                a(i_,j) = a(i_,jp);
            }
            for(i_=1; i_<=n;i_++)
            {
                a(i_,jp) = work(i_);
            }
        }
    }
    return result;
}


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
bool inverse(ap::real_2d_array& a, int n)
{
    bool result;
    ap::integer_1d_array pivots;

    ludecomposition(a, n, n, pivots);
    result = inverselu(a, pivots, n);
    return result;
}



