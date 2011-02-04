// PseudorandomGenerator.h
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

#ifndef _VESTA_PSEUDORANDOM_GENERATOR_H_
#define _VESTA_PSEUDORANDOM_GENERATOR_H_

#include "../IntegerTypes.h"


namespace vesta
{

class PseudorandomGenerator
{
private:
    enum
    {
        IEEE754_Significand_Mask = 0x007fffff
    };

public:
    PseudorandomGenerator()
    {
    }

    PseudorandomGenerator(v_uint64 seed) :
        m_seed(seed)
    {
    }

private:
    // This is used to generate random floats quickly via some tricky bit
    // manipulations of IEEE754 single-precision values
    union FloatAsInt
    {
        v_uint32 i;
        float f;
    };

public:
    /**
     * Return a pseudorandom 32-bit unsigned integer.
     */
    v_uint32 randUint()
    {
        // Same values used by Java's rand() function
        m_seed = 25214903917ULL * m_seed + 11;

        // Use only the middle 32-bits
        return (m_seed & 0x0000ffffffff0000ULL) >> 16;
    }

    /** Return a random floating point value in [ 0, 1 )
     */
    float randFloat()
    {
        // Construct an IEEE754 floating point value in [ 1, 2 )
        FloatAsInt fi;
        fi.i = randUint();
        fi.i = (fi.i & IEEE754_Significand_Mask) | 0x3f800000;

        // Map it to [ 0, 1 )
        return fi.f - 1.0f;
    }


    /** Return a random floating point value in [ -1, +1 )
     */
    float randSignedFloat()
    {
        // Construct an IEEE754 floating point value in [ 2, 4 )
        FloatAsInt fi;
        fi.i = randUint();
        fi.i = (fi.i & IEEE754_Significand_Mask) | 0x40000000;

        return fi.f - 3.0f;
    }

private:
    v_uint64 m_seed;
};

}

#endif // _VESTA_PSEUDORANDOM_GENERATOR_H_
