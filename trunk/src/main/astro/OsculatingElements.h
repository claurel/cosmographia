// This file is part of Cosmographia.
//
// Copyright (C) 2012 Chris Laurel <claurel@gmail.com>
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

#ifndef _ASTRO_OSCULATING_ELEMENTS_H_
#define _ASTRO_OSCULATING_ELEMENTS_H_

#include <vesta/OrbitalElements.h>
#include <vesta/StateVector.h>

vesta::OrbitalElements
CalculateOsculatingElements(const vesta::StateVector& state, double gm, double epoch);

#endif // _ASTRO_OSCULATING_ELEMENTS_H_
