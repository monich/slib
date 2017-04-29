/*
 * $Id: s_mfp.c,v 1.7 2016/08/07 16:55:50 slava Exp $
 *
 * Copyright (C) 2012-2016 by Slava Monich
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1.Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   2.Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer
 *     in the documentation and/or other materials provided with the
 *     distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING
 * IN ANY WAY OUT OF THE USE OR INABILITY TO USE THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 */

#include "s_math.h"

/*==========================================================================*
 *              F I X E D    P O I N T    M A T H
 *==========================================================================*/

#define FIX32_SIGN  (((Fix32)1) << (FIX32_SHIFT-1))
#define FIX64_SIGN  (((Fix64)1) << (FIX64_SHIFT-1))

/**
 * Divides a by b
 */
Fix32 FIX32_Div(Fix32 a, Fix32 b)
{
    ASSERT(b);
    if (b) {
        /* Make sure that unsigned division doesn't produce zero */
        Bool negate = (b < 0);
        Fix32 x = (negate ? (-b) : b);

        /* Select optimal shift strategy to avoid overflowing the numerator
         * and at the same time not to lose too many significant bits in the
         * denominator */
        I16s high = FIX32_ToInt(a);
        int shift = 0;
        if (high > 0) {
            shift = -1;
            while (high > 0) (high <<= 1, shift++);
        } else if (high < 0) {
            shift = -1;
            while (high < 0) (high <<= 1, shift++);
        } else {
            shift = FIX32_SHIFT;
        }

        x >>= (FIX32_SHIFT - shift);
        a <<= shift;
        a /= x;
        if (negate) a = -a;
        return a;
    } else {
        return 0;
    }
}

/**
 * Divides a by b
 */
Fix64 FIX64_Div(Fix64 a, Fix64 b)
{
    ASSERT(b);
    if (b) {
        /* Make sure that unsigned division doesn't produce zero */
        Bool negate = (b < 0);
        Fix64 x = (negate ? (-b) : b);

        /* Select optimal shift strategy to avoid overflowing the numerator
         * and at the same time not to lose too many significant bits in the
         * denominator */
        I32s high = FIX64_ToInt(a);
        int shift = 0;
        if (high > 0) {
            shift = -1;
            while (high > 0) (high <<= 1, shift++);
        } else if (high < 0) {
            shift = -1;
            while (high < 0) (high <<= 1, shift++);
        } else {
            shift = FIX64_SHIFT;
        }

        x >>= (FIX64_SHIFT - shift);
        a <<= shift;
        a = MATH_Div64s(a,x);
        if (negate) a = -a;
        return a;
    } else {
        return 0;
    }
}

/**
 * Multiples a by b
 */
Fix32 FIX32_Mul(Fix32 a, Fix32 b)
{
    int sign = 1, shift, bits1, bits2;

    if (a < 0) {
        sign *= (-1);
        a = -a;
    }

    if (b < 0) {
        sign *= (-1);
        b = -b;
    }

    bits1 = MATH_BitLen(a >> FIX32_SHIFT);
    bits2 = MATH_BitLen(b >> FIX32_SHIFT);
    shift = (bits1 - bits2 + FIX32_SHIFT)/2;
    a >>= shift;
    b >>= (FIX32_SHIFT - shift);
    return a*b*sign;
}

/**
 * Multiples a by b
 */
Fix64 FIX64_Mul(Fix64 a, Fix64 b)
{
    int sign = 1, shift, bits1, bits2;

    if (a < 0) {
        sign *= (-1);
        a = -a;
    }

    if (b < 0) {
        sign *= (-1);
        b = -b;
    }

    bits1 = MATH_BitLen((I32s)(a >> FIX64_SHIFT));
    bits2 = MATH_BitLen((I32s)(b >> FIX64_SHIFT));
    shift = (bits1 - bits2 + FIX64_SHIFT)/2;
    a >>= shift;
    b >>= (FIX64_SHIFT - shift);
    return a*b*sign;
}

/*
 * HISTORY:
 *
 * $Log: s_mfp.c,v $
 * Revision 1.7  2016/08/07 16:55:50  slava
 * o removed some unnecessary code that was never executed
 * o added FIX32_Mul function, similar to FIX64_Mul. Its purpose is to avoid
 *   losing significant bits during multiplication (e.g. when multiplying a
 *   large number by a small number)
 *
 * Revision 1.6  2015/08/16 16:07:32  slava
 * o housekeeping
 *
 * Revision 1.5  2012/12/18 09:12:41  slava
 * o fixed sign handling in FIX64_Mul
 *
 * Revision 1.4  2012/12/17 19:56:21  slava
 * o another bug in FIX64_Mul fixed
 *
 * Revision 1.3  2012/12/17 19:52:22  slava
 * o fixed a few point math bugs, now it seems to be working
 * o added FIX64_Mul function
 *
 * Revision 1.2  2012/12/13 17:57:01  slava
 * o use MATH_Div for 64-bit division
 *
 * Revision 1.1  2012/12/13 17:42:26  slava
 * o fixed point math
 *
 * Local Variables:
 * c-basic-offset: 4
 * compile-command: "make -C .."
 * indent-tabs-mode: nil
 * End:
 */
