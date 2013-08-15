// ChebyshevPolyTrajectory.cpp
//
// Copyright (C) 2010-2013 Chris Laurel <claurel@gmail.com>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met: 
//
//    1. Redistributions of source code must retain the above copyright notice, this
//       list of conditions and the following disclaimer. 
//    2. Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimer in the documentation
//       and/or other materials provided with the distribution. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "ChebyshevPolyTrajectory.h"
#include <vesta/Debug.h>
#include <algorithm>

using namespace vesta;
using namespace Eigen;
using namespace std;


/** Create a new Chebyshev polynomial trajectory.
  * The coefficients array must contain (degree + 1) * granuleCount * 3 values. They should be arranged
  * as an array of triples with low-order coefficients first: x0 y0 z0 x1 y1 z1 ...
  *
  * \param coeffs the array of Chebyshev coefficients for interpolating the position
  * \param degree the degree of the polynomial (there will be degree + 1 coefficients)
  * \param granuleCount the number of granules in the trajectory
  * \param startTimeTdbSec the first instant of the trajectory in seconds since J2000 (TDB time scale)
  * \param granuleLengthSec the time span covered by each granule
  */
ChebyshevPolyTrajectory::ChebyshevPolyTrajectory(const double coeffs[],
                                                 unsigned int degree,
                                                 double granuleCount,
                                                 double startTimeTdbSec,
                                                 double granuleLengthSec) :
    m_coeffs(NULL),
    m_degree(degree),
    m_granuleCount(granuleCount),
    m_startTime(startTimeTdbSec),
    m_granuleLength(granuleLengthSec),
    m_period(0.0),
    m_boundingRadius(0.0)
{
    // assert(degree <= MaxChebyshevDegree);
    unsigned int coeffCount = (degree + 1) * granuleCount * 3;
    m_coeffs = new double[coeffCount];
    copy(coeffs, coeffs + coeffCount, m_coeffs);

    setStartTime(startTimeTdbSec);
    setEndTime(startTimeTdbSec + granuleCount * granuleLengthSec);

    // Calculate a conservative estimate for the bounding radius (i.e. size of a sphere
    // large enough to contain the trajectory.)
    for (unsigned int granule = 0; granule < granuleCount; ++granule)
    {
        const double* granuleCoeffs = m_coeffs + granule * (degree + 1) * 3;
        Vector3d x0(granuleCoeffs[0], granuleCoeffs[1], granuleCoeffs[2]);
        Vector3d ext = Vector3d::Zero();
        for (unsigned int i = 1; i <= degree; ++i)
        {
            ext += Vector3d(granuleCoeffs[i * 3 + 0], granuleCoeffs[i * 3 + 1], granuleCoeffs[i * 3 + 2]).cwise().abs();
        }

        m_boundingRadius = max(m_boundingRadius, (x0 + ext).norm());
    }
}


ChebyshevPolyTrajectory::~ChebyshevPolyTrajectory()
{
    delete[] m_coeffs;
}


StateVector
ChebyshevPolyTrajectory::state(double tdbSec) const
{
    tdbSec = max(startTime(), min(endTime(), tdbSec));

    int granuleIndex = int((tdbSec - m_startTime) / m_granuleLength);
    double granuleStartTime = m_startTime + m_granuleLength * granuleIndex;

    // The interpolation parameter is u, which has a value in [-1, 1]
    double u = 2.0 * (tdbSec - granuleStartTime) / m_granuleLength - 1.0;

    // Clamp times outside the time span covered by the trajectory
    if (granuleIndex < 0)
    {
        u = -1.0;
        granuleIndex = 0;
    }
    else if (granuleIndex >= int(m_granuleCount))
    {
        u = 1.0;
        granuleIndex = m_granuleCount - 1;
    }

    // Position terms
    double x[MaxChebyshevDegree + 1];
    x[0] = 1.0;
    x[1] = u;

    // Velocity terms (derivatives of position)
    double v[MaxChebyshevDegree + 1];
    v[0] = 0.0;
    v[1] = 1.0;

    for (unsigned int i = 2; i <= m_degree; ++i)
    {
        x[i] = 2.0 * u * x[i - 1] - x[i - 2];
        v[i] = 2.0 * u * v[i - 1] - v[i - 2] + 2.0 * x[i - 1];
    }

    // TODO: We can reduce numerical errors by summing high order terms first; should
    // find out if this matters enough to be worth the trouble.
    double* granuleCoeffs = m_coeffs + granuleIndex * (m_degree + 1) * 3;
    Vector3d position = Map<MatrixXd>(granuleCoeffs, m_degree + 1, 3).transpose() * Map<MatrixXd>(x, m_degree + 1, 1);
    Vector3d velocity = Map<MatrixXd>(granuleCoeffs, m_degree + 1, 3).transpose() * Map<MatrixXd>(v, m_degree + 1, 1);

    return StateVector(position, velocity * (2.0 / m_granuleLength));
}


double
ChebyshevPolyTrajectory::boundingSphereRadius() const
{
    return m_boundingRadius;
}


bool
ChebyshevPolyTrajectory::isPeriodic() const
{
    return m_period != 0.0;
}


double
ChebyshevPolyTrajectory::period() const
{
    return m_period;
}


void
ChebyshevPolyTrajectory::setPeriod(double period)
{
    m_period = period;
}
