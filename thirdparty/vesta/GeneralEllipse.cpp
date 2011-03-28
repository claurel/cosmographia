/*
 * $Revision: 565 $ $Date: 2011-02-15 16:00:43 -0800 (Tue, 15 Feb 2011) $
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * this file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

#include "GeneralEllipse.h"
#include <Eigen/QR>
#include <cmath>

using namespace vesta;
using namespace Eigen;
using namespace std;


/** Compute the principal semi-axes of the ellipsoid. The axes
  * are the rows of the returned matrix. Note that there is no
  * ordering of semi-major or semi-minor axes.
  */
Matrix<double, 3, 2>
GeneralEllipse::principalSemiAxes() const
{
    Matrix2d S;
    double s00 = m_generatingVectors.col(0).dot(m_generatingVectors.col(0));
    double s01 = m_generatingVectors.col(0).dot(m_generatingVectors.col(1));
    double s11 = m_generatingVectors.col(1).dot(m_generatingVectors.col(1));
    double s10 = s01;
    S << s00, s01, s10, s11;

    SelfAdjointEigenSolver<Matrix2d> solver(S, true);
    Vector2d e = solver.eigenvalues();
    Matrix2d ev = solver.eigenvectors();

    Matrix<double, 3, 2> result;
    result.col(0) = ev(0, 0) * m_generatingVectors.col(0) + ev(1, 0) * m_generatingVectors.col(1);
    result.col(1) = ev(0, 1) * m_generatingVectors.col(0) + ev(1, 1) * m_generatingVectors.col(1);

    return result;
}
