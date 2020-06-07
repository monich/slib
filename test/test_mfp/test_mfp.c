/*
 * $Id: test_mfp.c,v 1.3 2020/06/07 01:38:46 slava Exp $
 *
 * Copyright (C) 2016-2020 by Slava Monich
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

#include "test_common.h"

static const Fix32 f32_minus1 = 0xffff0000;
static const Fix32 f32_1      = 0x00010000;
static const Fix32 f32_2      = 0x00020000;
static const Fix32 f32_2by3   = 0x0000aaaa;
static const Fix32 f32_1by4   = 0x00004000;
static const Fix64 f64_minus1 = __INT64_C(0xffffffff00000000);
static const Fix64 f64_1      = __INT64_C(0x0000000100000000);
static const Fix64 f64_2      = __INT64_C(0x0000000200000000);
static const Fix64 f64_2by3   = __INT64_C(0x00000000aaaaaaaa);
static const Fix64 f64_1by4   = __INT64_C(0x0000000040000000);

static
TestStatus
test_mfp_basic(
    const TestDesc* test)
{
    TEST_ASSERT(!FIX32_Int(0));
    TEST_ASSERT(!FIX64_Int(0));
    TEST_ASSERT(FIX32_Int(1) == f32_1);
    TEST_ASSERT(FIX64_Int(1) == f64_1);
    TEST_ASSERT(FIX32_Int(-1) == f32_minus1);
    TEST_ASSERT(FIX64_Int(-1) == f64_minus1);
    TEST_ASSERT(FIX32_Int(2) == f32_2);
    TEST_ASSERT(FIX64_Int(2) == f64_2);
    TEST_ASSERT(FIX32_Int(0xffff) == f32_minus1);
    TEST_ASSERT(FIX64_Int(0xffffffff) == f64_minus1);
    return TEST_OK;
}

static
TestStatus
test_mfp_div(
    const TestDesc* test)
{
    /* FIX32_Div and FIX64_Div should survive devision by zero */
    TEST_ASSERT(!FIX32_Div(FIX32_Int(1),0));
    TEST_ASSERT(!FIX64_Div(FIX64_Int(1),0));
    TEST_ASSERT(!FIX32_Div(0, f32_1));
    TEST_ASSERT(!FIX64_Div(0, f64_1));
    TEST_ASSERT(!FIX32_Div(0, f32_minus1));
    TEST_ASSERT(!FIX64_Div(0, f64_minus1));
    TEST_ASSERT(FIX32_Ratio(1,4) == f32_1by4);
    TEST_ASSERT(FIX64_Ratio(1,4) == f64_1by4);
    TEST_ASSERT(FIX32_Ratio(2,3) == f32_2by3);
    TEST_ASSERT(FIX64_Ratio(2,3) == f64_2by3);
    TEST_ASSERT(FIX32_Div(FIX32_Int(2), FIX32_Int(3)) == f32_2by3);
    TEST_ASSERT(FIX64_Div(FIX64_Int(2), FIX64_Int(3)) == f64_2by3);
    TEST_ASSERT(FIX32_Div(FIX32_Int(2), FIX32_Int(-3)) == -f32_2by3);
    TEST_ASSERT(FIX64_Div(FIX64_Int(2), FIX64_Int(-3)) == -f64_2by3);
    TEST_ASSERT(FIX32_Div(FIX32_Int(-2), FIX32_Int(3)) == -f32_2by3);
    TEST_ASSERT(FIX64_Div(FIX64_Int(-2), FIX64_Int(3)) == -f64_2by3);
    TEST_ASSERT(FIX32_Div(FIX32_Int(2), f32_1by4) == FIX32_Int(8));
    TEST_ASSERT(FIX64_Div(FIX64_Int(2), f64_1by4) == FIX64_Int(8));
    TEST_ASSERT(FIX32_Div(FIX32_Int(SHRT_MIN),f32_1) == FIX32_Int(SHRT_MIN));
    TEST_ASSERT(FIX64_Div(FIX64_Int(INT_MIN),f64_1) == FIX64_Int(INT_MIN));
    TEST_ASSERT(FIX32_Div(FIX32_Int(SHRT_MIN),f32_2) == FIX32_Int(SHRT_MIN/2));
    TEST_ASSERT(FIX64_Div(FIX64_Int(INT_MIN),f64_2) == FIX64_Int(INT_MIN/2));
    return TEST_OK;
}

static
TestStatus
test_mfp_mul(
    const TestDesc* test)
{
    TEST_ASSERT(FIX32_Mul(f32_2, f32_2) == FIX32_Int(4));
    TEST_ASSERT(FIX64_Mul(f64_2, f64_2) == FIX64_Int(4));
    TEST_ASSERT(FIX32_Mul(f32_minus1, f32_minus1) == f32_1);
    TEST_ASSERT(FIX64_Mul(f64_minus1, f64_minus1) == f64_1);
    TEST_ASSERT(FIX32_Mul(FIX32_Int(-1024),FIX32_Ratio(1,1024)) == f32_minus1);
    TEST_ASSERT(FIX64_Mul(FIX64_Int(-1024),FIX64_Ratio(1,1024)) == f64_minus1);
    TEST_ASSERT(FIX32_Mul(FIX32_Int(1024), FIX32_Ratio(1,1024)) == f32_1);
    TEST_ASSERT(FIX64_Mul(FIX64_Int(1024), FIX64_Ratio(1,1024)) == f64_1);
    return TEST_OK;
}

int main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Basic", test_mfp_basic},
        {"Div", test_mfp_div},
        {"Mul", test_mfp_mul},
    };

    return TEST_MAIN(argc, argv, tests);
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
