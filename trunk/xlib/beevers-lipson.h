/* Beevers-Lipson Fourier summation 
 (c) O Dolomanov, 2008
*/
#ifndef __beevers_lipson_h
#define __beevers_lipson_h

#include "ecomplex.h"
#include "xbase.h"
#include "sfutil.h"
#include "arrays.h"

BeginXlibNamespace()

class BVFourier {
public:
  struct MapInfo  {
    double sigma, minVal, maxVal;
  };
  template <class FloatT> static MapInfo CalcEDM(const TArrayList<StructureFactor>& F, 
      FloatT*** map, int mapX, int mapY, int mapZ, double vol)  {
    vec3i min, max;
    SFUtil::FindMinMax(F, min, max); 
    compd ** S, *T;
    int kLen = max[1]-min[1]+1, 
        hLen = max[0]-min[0]+1, 
        lLen = max[2]-min[2]+1;
    S = new compd*[kLen];
    for( int i=0; i < kLen; i++ )
      S[i] = new compd[lLen];
    T = new compd[lLen];
    const double T_PI = 2*M_PI;
    // precalculations
    int minInd = olx_min(min[0], min[1]);
    if( min[2] < minInd )  minInd = min[2];
    int maxInd = olx_max(max[0], max[1]);
    if( max[2] > maxInd )  maxInd = max[2];
    int iLen = maxInd - minInd + 1;
    int mapMax = olx_max(mapX, mapY);
    if( mapZ > mapMax )  mapMax = mapZ;
    compd** sin_cosX = new compd*[mapX],
      **sin_cosY, **sin_cosZ;
    for( int i=0; i < mapX; i++ )  {
      sin_cosX[i] = new compd[iLen];
      for( int j=minInd; j <= maxInd; j++ )  {
        double rv = (double)(i*j)/mapX, ca, sa;
        rv *= T_PI;
        SinCos(-rv, &sa, &ca);
        sin_cosX[i][j-minInd].SetRe(ca);
        sin_cosX[i][j-minInd].SetIm(sa);
      }
    }
    if( mapX == mapY )  {
      sin_cosY = sin_cosX;
    }
    else  {
      sin_cosY = new compd*[mapY];
      for( int i=0; i < mapY; i++ )  {
        sin_cosY[i] = new compd[iLen];
        for( int j=minInd; j <= maxInd; j++ )  {
          double rv = (double)(i*j)/mapY, ca, sa;
          rv *= T_PI;
          SinCos(-rv, &sa, &ca);
          sin_cosY[i][j-minInd].SetRe(ca);
          sin_cosY[i][j-minInd].SetIm(sa);
        }
      }
    }
    if( mapX == mapZ )  {
      sin_cosZ = sin_cosX;
    }
    else if( mapY == mapZ )  {
      sin_cosZ = sin_cosY;
    }
    else  {
      sin_cosZ = new compd*[mapZ];
      for( int i=0; i < mapZ; i++ )  {
        sin_cosZ[i] = new compd[iLen];
        for( int j=minInd; j <= maxInd; j++ )  {
          double rv = (double)(i*j)/mapZ, ca, sa;
          rv *= T_PI;
          SinCos(-rv, &sa, &ca);
          sin_cosZ[i][j-minInd].SetRe(ca);
          sin_cosZ[i][j-minInd].SetIm(sa);
        }
      }
    }
    compd R;
    /* http://smallcode.weblogs.us/2006/11/27/calculate-standard-deviation-in-one-pass/
    for one pass calculation of the variance
    */
    MapInfo mi = {0, 1000, -1000};
    double sum = 0, sq_sum = 0;
    for( int ix=0; ix < mapX; ix++ )  {
      for( int i=0; i < F.Count(); i++ )  {
        const StructureFactor& sf = F[i];
        S[sf.hkl[1]-min[1]][sf.hkl[2]-min[2]] += sf.val*sin_cosX[ix][sf.hkl[0]-minInd];
      }
      for( int iy=0; iy < mapY; iy++ )  {
        for( int i=min[1]; i <= max[1]; i++ )  {
          for( int j=min[2]; j <= max[2]; j++ )  {
            T[j-min[2]] += S[i-min[1]][j-min[2]]*sin_cosY[iy][i-minInd];
          }
        }
        for( int iz=0; iz < mapZ; iz++ )  {
          R.Null();
          for( int i=min[2]; i <= max[2]; i++ )  {
            R += T[i-min[2]]*sin_cosZ[iz][i-minInd];
          }
          double val = R.Re()/vol;
          sum += ((val < 0) ? -val : val);
          sq_sum += val*val;
          //if( abs_map && val < 0 )  val = -val;
          if( val > mi.maxVal )  mi.maxVal = val;
          if( val < mi.minVal )  mi.minVal = val;
          map[ix][iy][iz] = val;
        }
        for( int i=0; i < lLen; i++ )  
          T[i].Null();
      }
      for( int i=0; i < kLen; i++ )  
        for( int j=0; j < lLen; j++ )  
          S[i][j].Null();
    }
    double map_mean = sum/(mapX*mapY*mapZ);
    mi.sigma = sqrt(sq_sum/(mapX*mapY*mapZ) - map_mean*map_mean);
    // clean up of allocated data
    for( int i=0; i < kLen; i++ )
      delete [] S[i];
    delete [] S;
    delete [] T;
    if( sin_cosY == sin_cosX )  sin_cosY = NULL;
    if( sin_cosZ == sin_cosX || sin_cosZ == sin_cosY )  sin_cosZ = NULL;
    for( int i=0; i < mapX; i++ )
      delete [] sin_cosX[i];
    delete [] sin_cosX;
    if( sin_cosY != NULL )  {
      for( int i=0; i < mapY; i++ )
        delete [] sin_cosY[i];
      delete [] sin_cosY;
    }
    if( sin_cosZ != NULL )  {
      for( int i=0; i < mapZ; i++ )
        delete [] sin_cosZ[i];
      delete [] sin_cosZ;
    }
    return mi;
  }
};

EndXlibNamespace()
#endif
