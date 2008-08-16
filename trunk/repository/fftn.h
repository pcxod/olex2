#include <math.h>

template <typename ArrayClass, class ComplexClass>
void fftn(ArrayClass& data, unsigned nn[], int ndim, short isign)  {
  unsigned nprev = 1, ntot = 1;
  ComplexClass w, wp;
  const double Mult = isign * 2.0 * M_PI;
  /*      Compute total number of complex values  */
  for( int idim = 0; idim < ndim; ++idim)
    ntot *= nn[idim];

  for( int idim = ndim - 1; idim >= 0; --idim) {
    unsigned n = nn[idim];
    unsigned nrem = ntot / (n * nprev);
    unsigned ip2 = nprev * n;        /*      Unit step for next dimension */
    unsigned i2rev = 0;              /*      Bit reversed i2 */
    /*      This is the bit reversal section of the routine */
    /*      Loop over current dimension     */
    for( unsigned i2 = 0; i2 < ip2; i2 += nprev) {
      if( i2 < i2rev )  {
        for( unsigned i1 = i2; i1 < i2 + nprev; ++i1 )  {  //Loop over lower dimensions
          for( unsigned i3 = i1; i3 < ntot; i3 += ip2)  {  //Loop over higher dimensions
            unsigned i3rev = i3 + i2rev - i2;
            ComplexClass temp( data[i3] );
            data[i3] = data[i3rev];
            data[i3rev] = temp;
          }
        }
      }
      unsigned ibit = ip2;
      do {  // Increment from high end of i2rev to low
        ibit >>= 1;
        i2rev ^= ibit;
      } 
      while( ibit >= nprev && (ibit & i2rev) == 0 );
    }

    /*      Here begins the Danielson-Lanczos section of the routine */
    /*      Loop over step sizes    */
    for( unsigned ifp1 = nprev; ifp1 < ip2; ifp1 <<= 1 ) {
      unsigned ifp2 = ifp1 << 1;
      /*  Initialize for the trig. recurrence */
      double theta = Mult * nprev/ifp2;
      wp.Re() = sin(0.5 * theta);
      wp.Re() *= -2.0 * wp.Re();
      wp.Im() = sin(theta);
      w.Re() = 1.0;
      w.Im() = 0.0;
      for( unsigned i3 = 0; i3 < ifp1; i3 += nprev) {  //Loop by unit step in current dimension
        for( unsigned i1 = i3; i1 < i3 + nprev; ++i1 )  //Loop over lower dimensions
          for (i2 = i1; i2 < ntot; i2 += ifp2) {  //Loop over higher dimensions
            /*      Danielson-Lanczos formula */
            unsigned k2 = i2 + ifp1;
            ComplexClass temp(w * data[k2]);
            data[k2] -= temp;
            data[i2] += temp;
          }
          /*      Trigonometric recurrence        */
          w *= wp;
      }
    }
    nprev *= n;
  }
}


