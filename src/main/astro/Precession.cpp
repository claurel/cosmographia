#include "Precession.h"
#include "Rotation.h"
#include <vesta/Units.h>

using namespace vesta;
using namespace Eigen;


static Matrix3d
wmatrix(const Vector3d& w)
{
    Matrix3d wmat;
    wmat <<  0.0, -w.z(),  w.y(),
             w.z(),    0.0, -w.x(),
            -w.y(),  w.x(),    0.0;

    return wmat;
}


static const double DaysPerJulianCentury = 36525.0;

/** Get angles and their derivatives for the IAU1976 Earth precession model.
  *
  * The rotation of the Earth due to precession is given by:
  *   Rz(-z) * Ry(theta) * Rz(-theta)
  * where Rz and Ry are rotations about the z- and y- axes.
  */
void
PrecessionAngles_IAU1976(double jdFrom, double jdTo,
                         double* zeta, double* z, double* theta, double *dzeta, double* dz, double* dtheta)
{
    const double asecToRad = arcsecToRadians(1.0);

    double T = (jdFrom - 2451545.0) / DaysPerJulianCentury;
    double t = (jdTo - jdFrom) / DaysPerJulianCentury;

    double T2 = T * T;
    double w = 2306.2181 + 1.39656 * T - 0.000139 * T2;

#if USE_TRUNCATED_PRECESSION
    *zeta = t * (t * (t * .017998 + .30188) + 2306.2181) * ArcsecToRadians;
    *z = t * (t * (t * .018203 + 1.09468) + 2306.2181) * ArcsecToRadians;
    *theta = t * (t * (t * -.041833 - .42665) + 2004.3109) * ArcsecToRadians;
#else
    *zeta  = (w + ((0.30188 - 0.000344 * T) + 0.017998 * t) * t) * t * asecToRad;
    *z     = (w + ((1.09468 + 0.000066 * T) + 0.018203 * t) * t) * t * asecToRad;
    *theta = ((2004.3109 + (-0.85330 - 0.000217 * T) * T)
           + ((-0.42665 - 0.000217 * T) - 0.041833 * t) * t) * t * asecToRad;
#endif

    // Note that the derivatives used here are from the truncated precession
    // polynomials. SPICE uses the same approximation.
    double ts = 1.0 / (DaysPerJulianCentury * 86400.0);
    *dzeta  = ts * (t * (t * 3 *  0.017998 + 2 * 0.30188) + 2306.2181) * asecToRad;
    *dz     = ts * (t * (t * 3 *  0.018203 + 2 * 1.09468) + 2306.2181) * asecToRad;
    *dtheta = ts * (t * (t * 3 * -0.041833 - 2 * 0.42665) + 2004.3109) * asecToRad;

    return;
}


/** Compute the rotation due to precession from Julian date jdFrom to
 *  date jdTo using the IAU 1976 precession model.
 *
 *  TODO: optionally return the derivative of the rotation
 *
 * \param tFrom date (as time in secs since J2000.0 TDB) to precess coordinates from
 * \param tTo date (as time in secs since J2000.0 TDB) to precess coordinates to
 *
 * \returns a unit quaternion giving the rotation due to precession.
 *
 *  The IAU 1976 precession model is accurate for dates around J2000, but
 *  loses accuracy in the distant past or future. The IAU's SOFA library
 *  lists the following errors:
 *   - below 0.1 arcsec from 1960AD to 2040AD,
 *   - below 1 arcsec from 1640AD to 2360AD,
 *   - below 3 arcsec from 500BC to 3000AD.
 *   - over 10 arcsec outside range 1200BC to 3900AD
 *   - over 100 arcsec outside 4200BC to 5600AD
 *   - over 1000 arcsec outside 6800BC to 8200AD.
 */
Quaterniond
Precession_IAU1976(double tFrom, double tTo)
{
    double zeta = 0.0;
    double z = 0.0;
    double theta = 0.0;
    double dzeta = 0.0;
    double dz = 0.0;
    double dtheta = 0.0;

    PrecessionAngles_IAU1976(tFrom, tTo, &zeta, &z, &theta, &dzeta, &dz, &dtheta);

    Quaterniond q1 = Quaterniond(AngleAxisd(-z,    Vector3d::UnitZ()));
    Quaterniond q2 = Quaterniond(AngleAxisd(theta, Vector3d::UnitY()));
    Quaterniond q3 = Quaterniond(AngleAxisd(-zeta, Vector3d::UnitZ()));
    Quaterniond q = q3 * q2 * q1;

    Quaterniond dq1 = qDerivative(Vector3d::UnitZ(),    -z,    -dz);
    Quaterniond dq2 = qDerivative(Vector3d::UnitY(), theta, dtheta);
    Quaterniond dq3 = qDerivative(Vector3d::UnitZ(), -zeta, -dzeta);
    Quaterniond dq;

    // Compute the instantaneous angular momentum vector w. There's
    // still an extra factor of 1/2 in w, since dq is 1/2 * w(t) * q(t).
    Quaterniond wq = dq * q.conjugate();
    Vector3d w = 2.0 * wq.vec();

    // Unused: compute the 3x3 matrix derivative for rotation due to precession
    //Matrix3d dqdt = wmatrix(2.0 * wq.vec()) * q.toRotationMatrix();

    return q;
}

