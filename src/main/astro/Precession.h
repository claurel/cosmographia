// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// Cosmographia is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

#ifndef _ASTRO_PRECESSION_H_
#define _ASTRO_PRECESSION_H_

#include <Eigen/Geometry>

void
PrecessionAngles_IAU1976(double tFrom, double tTo,
                         double* zeta, double* z, double* theta, double *dzeta, double* dz, double* dtheta);

Eigen::Quaterniond
Precession_IAU1976(double tFrom, double tTo);

#endif // _ASTRO_PRECESSION_H_
