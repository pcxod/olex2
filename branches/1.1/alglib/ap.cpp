#include "ap.h"
using namespace ap;

bool operator==(const complex& lhs, const complex& rhs)
{ return lhs.x==rhs.x && lhs.y==rhs.y; }

bool operator!=(const complex& lhs, const complex& rhs)
{ return lhs.x!=rhs.x || lhs.y!=rhs.y; }

const complex operator+(const complex& lhs)
{ return lhs; }

const complex operator-(const complex& lhs)
{ return complex(-lhs.x, -lhs.y); }

const complex operator+(const complex& lhs, const complex& rhs)
{ complex r = lhs; r += rhs; return r; }

const complex operator+(const complex& lhs, const double& rhs)
{ complex r = lhs; r += rhs; return r; }

const complex operator+(const double& lhs, const complex& rhs)
{ complex r = rhs; r += lhs; return r; }

const complex operator-(const complex& lhs, const complex& rhs)
{ complex r = lhs; r -= rhs; return r; }

const complex operator-(const complex& lhs, const double& rhs)
{ complex r = lhs; r -= rhs; return r; }

const complex operator-(const double& lhs, const complex& rhs)
{ complex r = lhs; r -= rhs; return r; }

const complex operator*(const complex& lhs, const complex& rhs)
{ return complex(lhs.x*rhs.x - lhs.y*rhs.y,  lhs.x*rhs.y + lhs.y*rhs.x); }

const complex operator*(const complex& lhs, const double& rhs)
{ return complex(lhs.x*rhs,  lhs.y*rhs); }

const complex operator*(const double& lhs, const complex& rhs)
{ return complex(lhs*rhs.x,  lhs*rhs.y); }

const complex operator/(const complex& lhs, const complex& rhs)
{
    complex result;
    double e;
    double f;
    if( fabs(rhs.y)<fabs(rhs.x) )
    {
        e = rhs.y/rhs.x;
        f = rhs.x+rhs.y*e;
        result.x = (lhs.x+lhs.y*e)/f;
        result.y = (lhs.y-lhs.x*e)/f;
    }
    else
    {
        e = rhs.x/rhs.y;
        f = rhs.y+rhs.x*e;
        result.x = (lhs.y+lhs.x*e)/f;
        result.y = (-lhs.x+lhs.y*e)/f;
    }
    return result;
}

const complex operator/(const double& lhs, const complex& rhs)
{
    complex result;
    double e;
    double f;
    if( fabs(rhs.y)<fabs(rhs.x) )
    {
        e = rhs.y/rhs.x;
        f = rhs.x+rhs.y*e;
        result.x = lhs/f;
        result.y = -lhs*e/f;
    }
    else
    {
        e = rhs.x/rhs.y;
        f = rhs.y+rhs.x*e;
        result.x = lhs*e/f;
        result.y = -lhs/f;
    }
    return result;
}

const complex operator/(const complex& lhs, const double& rhs)
{ return complex(lhs.x/rhs, lhs.y/rhs); }

double abscomplex(const complex &z)
{
    double w;
    double xabs;
    double yabs;
    double v;

    xabs = fabs(z.x);
    yabs = fabs(z.y);
    w = xabs>yabs ? xabs : yabs;
    v = xabs<yabs ? xabs : yabs; 
    if( v==0 )
        return w;
    else
    {
        double t = v/w;
        return w*sqrt(1+t*t);
    }
}

const complex conj(const complex &z)
{ return complex(z.x, -z.y); }

const complex csqr(const complex &z)
{ return complex(z.x*z.x-z.y*z.y, 2*z.x*z.y); }
