/*
 * $Revision$ $Date$
 *
 * Copyright by Astos Solutions GmbH, Germany
 *
 * This file is published under the Astos Solutions Free Public License
 * For details on copyright and terms of use see 
 * http://www.astos.de/Astos_Solutions_Free_Public_License.html
 */

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
        return static_cast<v_uint32>((m_seed & 0x0000ffffffff0000ULL) >> 16);
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
