// PseudorandomGenerator.cpp
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// VESTA is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Alternatively, you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// VESTA is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License or the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License and a copy of the GNU General Public License along with
// VESTA. If not, see <http://www.gnu.org/licenses/>.

#include "PseudorandomGenerator.h"
#include <iostream>

using namespace vesta;
using namespace std;


int main(void)
{
    PseudorandomGenerator gen;

    cout << "unsigned:\n";
    for (int i = 0; i < 20; ++i)
    {
        cout << gen.randFloat() << endl;
    }

    cout << "signed:\n";
    for (int i = 0; i < 20; ++i)
    {
        cout << gen.randSignedFloat() << endl;
    }

    return 0;
}

