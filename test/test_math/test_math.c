/*
 * $Id: test_math.c,v 1.1 2016/09/26 16:09:17 slava Exp $
 *
 * Copyright (C) 2016 by Slava Monich
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

static
TestStatus
test_math_isqrt32(
    const TestDesc* test)
{
    static const struct test_isqrt32 {
        I32u in;
        I32u out;
    } sample [] = {
        { 1, 1 },
        { 2, 1 },
        { 4, 2 },
        { 5, 2 },
        { 8, 2 },
        { 9, 3 },
        { 10, 3 },
        { 15, 3 },
        { 16, 4 },
        { 17, 4 },
    };

    TestStatus ret = TEST_OK;
    int i;

    for (i=0; i<COUNT(sample); i++) {
        I32u out = isqrt32(sample[i].in);
        if (out != sample[i].out) {
            PRINT_Error("0x%08x\n", sample[i].in);
            ret = TEST_ERR;
        }
    }

    return ret;
}

static
TestStatus
test_math_isqrt64(
    const TestDesc* test)
{
    static const struct test_isqrt64 {
        I64u in;
        I64u out;
    } sample [] = {
        { __UINT64_C(1), __UINT64_C(1) },
        { __UINT64_C(2), __UINT64_C(1) },
        { __UINT64_C(4), __UINT64_C(2) },
        { __UINT64_C(5), __UINT64_C(2) },
        { __UINT64_C(8), __UINT64_C(2) },
        { __UINT64_C(9), __UINT64_C(3) },
        { __UINT64_C(10), __UINT64_C(3) },
        { __UINT64_C(15), __UINT64_C(3) },
        { __UINT64_C(16), __UINT64_C(4) },
        { __UINT64_C(17), __UINT64_C(4) },
    };

    TestStatus ret = TEST_OK;
    int i;

    for (i=0; i<COUNT(sample); i++) {
        I64u out = isqrt64(sample[i].in);
        if (out != sample[i].out) {
            PRINT_Error(I64U_FORMAT "\n", sample[i].in);
            ret = TEST_ERR;
        }
    }

    return ret;
}

static
TestStatus
test_math_bitlen(
    const TestDesc* test)
{
    static const struct _test_bitlen {
        I32u value;
        int count;
    } sample [] = {
        { 0x00000000, 0 },
        { 0x00000001, 1 },
        { 0x00000002, 2 }, { 0x00000003, 2 },
        { 0x00000004, 3 }, { 0x00000005, 3 },
        { 0x00000008, 4 }, { 0x00000009, 4 }, { 0x0000000f, 4 },
        { 0x00000010, 5 }, { 0x00000011, 5 }, { 0x0000001f, 5 },
        { 0x00000020, 6 }, { 0x00000020, 6 }, { 0x00000021, 6 },
        { 0x00000040, 7 }, { 0x00000040, 7 }, { 0x00000041, 7 },
        { 0x00000080, 8 }, { 0x00000080, 8 }, { 0x00000081, 8 },
        { 0x00000100, 9 }, { 0x00000100, 9 }, { 0x00000101, 9 },
        { 0x00000200, 10 }, { 0x000002f0, 10 }, { 0x00000201, 10 },
        { 0x00000400, 11 }, { 0x000004f0, 11 }, { 0x00000401, 11 },
        { 0x00000800, 12 }, { 0x000008f0, 12 }, { 0x00000801, 12 },
        { 0x00001000, 13 }, { 0x00001f00, 13 }, { 0x00001001, 13 },
        { 0x00002000, 14 }, { 0x00002f00, 14 }, { 0x00002001, 14 },
        { 0x00004000, 15 }, { 0x00004f00, 15 }, { 0x00004001, 15 },
        { 0x00008000, 16 }, { 0x00008f00, 16 }, { 0x00008001, 16 },
        { 0x00010000, 17 }, { 0x0001f000, 17 }, { 0x00010001, 17 },
        { 0x00020000, 18 }, { 0x0002f000, 18 }, { 0x00020001, 18 },
        { 0x00040000, 19 }, { 0x0004f000, 19 }, { 0x00040001, 19 },
        { 0x00080000, 20 }, { 0x0008f000, 20 }, { 0x00080001, 20 },
        { 0x00100000, 21 }, { 0x001f0000, 21 }, { 0x00100001, 21 },
        { 0x00200000, 22 }, { 0x002f0000, 22 }, { 0x00200001, 22 },
        { 0x00400000, 23 }, { 0x004f0000, 23 }, { 0x00400001, 23 },
        { 0x00800000, 24 }, { 0x008f0000, 24 }, { 0x00800001, 24 },
        { 0x01000000, 25 }, { 0x01f00000, 25 }, { 0x01000001, 25 },
        { 0x02000000, 26 }, { 0x02f00000, 26 }, { 0x02000001, 26 },
        { 0x04000000, 27 }, { 0x04f00000, 27 }, { 0x04000001, 27 },
        { 0x08000000, 28 }, { 0x08f00000, 28 }, { 0x08000001, 28 },
        { 0x10000000, 29 }, { 0x1f000000, 29 }, { 0x10000001, 29 },
        { 0x20000000, 30 }, { 0x2f000000, 30 }, { 0x20000001, 30 },
        { 0x40000000, 31 }, { 0x4f000000, 31 }, { 0x40000001, 31 },
        { 0x80000000, 32 }, { 0x80000001, 32 }, { 0xffffffff, 32 }
    };

    TestStatus ret = TEST_OK;
    int i;

    for (i=0; i<COUNT(sample); i++) {
        int count = MATH_BitLen(sample[i].value);
        if (count != sample[i].count) {
            PRINT_Error("0x%08x\n", sample[i].value);
            ret = TEST_ERR;
        }
    }

    return ret;
}

static
TestStatus
test_math_bitcnt(
    const TestDesc* test)
{
    static const struct _test_bitcnt {
        int value;
        int count;
    } sample [] = {
        { 0x00000000, 0 },
        { 0x00000001, 1 },
        { 0x80000000, 1 },
        { 0x80000001, 2 },
        { 0x11111111, 8 },
        { 0x22222222, 8 },
        { 0x44444444, 8 },
        { 0x88888888, 8 },
        { 0xffff0000, 16 },
        { 0x0000ffff, 16 },
        { 0x77777777, 24 },
        { 0x7ffffffe, 30 },
        { 0xfffffffe, 31 },
        { 0xffffffff, 32 }
    };

    TestStatus ret = TEST_OK;
    int i;

    for (i=0; i<COUNT(sample); i++) {
        int count = MATH_BitCnt(sample[i].value);
        if (count != sample[i].count) {
            PRINT_Error("0x%08x\n", sample[i].value);
            ret = TEST_ERR;
        }
    }

    return ret;
}

int main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"isqrt32", test_math_isqrt32},
        {"isqrt64", test_math_isqrt64},
        {"BitLen", test_math_bitlen},
        {"BitCnt", test_math_bitcnt},
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
