/*
 * $Id: test_bitset.c,v 1.2 2018/12/27 18:17:29 slava Exp $
 *
 * Copyright (C) 2016-2018 by Slava Monich
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

static TestMem testMem;

static
TestStatus
test_bitset_alloc(
    const TestDesc* test)
{
    BitSet* bs = BITSET_Create();
    BitSet* bs1;
    int i, alloc;

    /* BITSET_Delete should be NULL resistant */
    BITSET_Delete(NULL);

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!BITSET_Create());

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!BITSET_Set(bs, 255));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!BITSET_Clone(bs));

    /* Clone of an empty bitset requires only one allocation */
    testMem.failAt = testMem.allocCount + 1;
    bs1 = BITSET_Clone(bs);
    TEST_ASSERT(bs1);
    BITSET_Delete(bs1);

    /* No memory is required to store zero bits */
    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(BITSET_Alloc(bs, 0));
    TEST_ASSERT(BITSET_Alloc(bs, 1));

    testMem.failAt = -1;
    BITSET_Set(bs, 255);
    for (i = 0; i < 2; i++) {
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!BITSET_Clone(bs));
    }

    BITSET_Xor(bs, bs);
    BITSET_Set(bs, 100);
    testMem.failAt = testMem.allocCount;
    alloc = bs->alloc;
    BITSET_Trim(bs);
    TEST_ASSERT(bs->alloc == alloc);

    bs1 = BITSET_Create();
    BITSET_Set(bs1, 100);
    BITSET_Xor(bs, bs);
    BITSET_Trim(bs);
    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!BITSET_Or(bs, bs1));
    TEST_ASSERT(!BITSET_Length(bs));
    TEST_ASSERT(!BITSET_Size(bs));
    BITSET_Delete(bs1);

    bs1 = BITSET_Create();
    BITSET_Set(bs1, 100);
    BITSET_Xor(bs, bs);
    BITSET_Trim(bs);
    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!BITSET_Xor(bs, bs1));
    TEST_ASSERT(!BITSET_Length(bs));
    TEST_ASSERT(!BITSET_Size(bs));
    BITSET_Delete(bs1);

    testMem.failAt = -1;
    BITSET_Delete(bs);
    return TEST_OK;
}

static
TestStatus
test_bitset_basic(
    const TestDesc* test)
{
    BitSet* bs = BITSET_Create();
    BitSet* bs1 = BITSET_Create();

    TEST_ASSERT(!BITSET_Alloc(bs, -1));
    TEST_ASSERT(!BITSET_Get(bs, 255));
    TEST_ASSERT(!BITSET_Get(bs, -1));
    TEST_ASSERT(!BITSET_Set(bs, -1));
    TEST_ASSERT(!BITSET_Length(bs));
    TEST_ASSERT(!BITSET_Size(bs));
    TEST_ASSERT(BITSET_Equal(bs, bs1));
    TEST_ASSERT(BITSET_HashCode(bs) == 1234);

    TEST_ASSERT(BITSET_Set(bs, 1));
    TEST_ASSERT(BITSET_Get(bs, 1));
    TEST_ASSERT(BITSET_BitCnt(bs) == 1);
    TEST_ASSERT(BITSET_Length(bs) == 2);
    TEST_ASSERT(BITSET_HashCode(bs) == 1232);

    TEST_ASSERT(BITSET_Set(bs, 255));
    TEST_ASSERT(BITSET_Get(bs, 255));
    TEST_ASSERT(BITSET_BitCnt(bs) == 2);
    TEST_ASSERT(BITSET_Length(bs) == 256);
    TEST_ASSERT(BITSET_Size(bs) == 256);
    TEST_ASSERT(BITSET_HashCode(bs) == 1232);

    BITSET_Delete(bs1);
    bs1 = BITSET_Clone(bs);
    TEST_ASSERT(bs1);
    TEST_ASSERT(BITSET_Equal(bs, bs));
    TEST_ASSERT(BITSET_Equal(bs, bs1));
    TEST_ASSERT(BITSET_Equal(bs1, bs));
    TEST_ASSERT(BITSET_Get(bs1, 255));
    TEST_ASSERT(BITSET_Get(bs1, 1));
    TEST_ASSERT(BITSET_BitCnt(bs1) == BITSET_BitCnt(bs));
    TEST_ASSERT(BITSET_Length(bs1) == BITSET_Length(bs));
    TEST_ASSERT(BITSET_HashCode(bs1) == BITSET_HashCode(bs));

    BITSET_Clear(bs, -1);
    BITSET_Clear(bs, 1024);
    BITSET_Clear(bs, 255);
    TEST_ASSERT(!BITSET_Get(bs, 255));
    TEST_ASSERT(BITSET_Get(bs, 1));
    TEST_ASSERT(BITSET_BitCnt(bs) == 1);
    TEST_ASSERT(BITSET_Length(bs) == 2);

    BITSET_Trim(bs);
    TEST_ASSERT(!BITSET_Equal(bs, bs1));
    TEST_ASSERT(!BITSET_Equal(bs1, bs));
    TEST_ASSERT(BITSET_Get(bs, 1));
    TEST_ASSERT(BITSET_BitCnt(bs) == 1);
    TEST_ASSERT(BITSET_Length(bs) == 2);

    BITSET_ClearAll(bs);
    TEST_ASSERT(!BITSET_Get(bs, 1));
    TEST_ASSERT(!BITSET_Length(bs));
    TEST_ASSERT(!BITSET_Size(bs));

    TEST_ASSERT(BITSET_Set(bs, 1));
    TEST_ASSERT(BITSET_Set(bs, 2));
    BITSET_Clear(bs, 2);
    TEST_ASSERT(BITSET_Get(bs, 1));
    TEST_ASSERT(!BITSET_Get(bs, 2));

    BITSET_ClearAll(bs);
    TEST_ASSERT(BITSET_Set(bs, 100));
    TEST_ASSERT(BITSET_Alloc(bs, 200));
    TEST_ASSERT(BITSET_Copy(bs1, bs));
    TEST_ASSERT(BITSET_Equal(bs1, bs));
    TEST_ASSERT(BITSET_Equal(bs, bs1));

    BITSET_Delete(bs1);
    BITSET_Delete(bs);
    return TEST_OK;
}

static
TestStatus
test_bitset_equal(
    const TestDesc* test)
{
    BitSet* bs1 = BITSET_Create();
    BitSet* bs2 = BITSET_Create();

    TEST_ASSERT(BITSET_Equal(bs1, bs2));
    TEST_ASSERT(BITSET_Equal(bs1, bs2));

    TEST_ASSERT(BITSET_Set(bs1, 1));
    TEST_ASSERT(BITSET_Set(bs2, 2));
    TEST_ASSERT(!BITSET_Equal(bs1, bs2));
    TEST_ASSERT(!BITSET_Equal(bs1, bs2));

    TEST_ASSERT(BITSET_Set(bs2, 1));
    TEST_ASSERT(BITSET_Set(bs1, 2));
    TEST_ASSERT(BITSET_Equal(bs1, bs2));

    BITSET_Delete(bs1);
    BITSET_Delete(bs2);
    return TEST_OK;
}

static
TestStatus
test_bitset_trim(
    const TestDesc* test)
{
    BitSet* bs1 = BITSET_Create();
    BitSet* bs2 = BITSET_Create();

    TEST_ASSERT(BITSET_Set(bs1, 1));
    TEST_ASSERT(BITSET_Set(bs1, 100));
    TEST_ASSERT(BITSET_Copy(bs2, bs1));
    TEST_ASSERT(BITSET_Alloc(bs2, 200));

    TEST_ASSERT(BITSET_Equal(bs1, bs2));

    BITSET_Trim(bs2);
    TEST_ASSERT(BITSET_Equal(bs1, bs2));

    TEST_ASSERT(BITSET_Xor(bs1, bs1));
    BITSET_Trim(bs1);
    TEST_ASSERT(!BITSET_BitCnt(bs1));
    TEST_ASSERT(!BITSET_Length(bs1));

    TEST_ASSERT(BITSET_Alloc(bs2, 1));
    BITSET_Trim(bs1);
    TEST_ASSERT(!BITSET_BitCnt(bs1));
    TEST_ASSERT(!BITSET_Length(bs1));
    TEST_ASSERT(!bs1->alloc);

    BITSET_Delete(bs1);
    BITSET_Delete(bs2);
    return TEST_OK;
}

static
TestStatus
test_bitset_and(
    const TestDesc* test)
{
    BitSet* bs1 = BITSET_Create();
    BitSet* bs2 = BITSET_Create();

    BITSET_And(bs1, bs2);
    TEST_ASSERT(!BITSET_BitCnt(bs1));
    TEST_ASSERT(!BITSET_Length(bs1));

    TEST_ASSERT(BITSET_Set(bs1, 1));
    TEST_ASSERT(BITSET_Set(bs2, 2));
    BITSET_And(bs1, bs1);
    BITSET_And(bs1, bs2);
    TEST_ASSERT(!BITSET_Get(bs1, 1));
    TEST_ASSERT(!BITSET_Get(bs1, 2));
    TEST_ASSERT(!BITSET_BitCnt(bs1));
    TEST_ASSERT(!BITSET_Length(bs1));

    TEST_ASSERT(BITSET_Set(bs1, 100));
    TEST_ASSERT(BITSET_Set(bs1, 2));
    TEST_ASSERT(BITSET_Set(bs2, 2));
    BITSET_And(bs1, bs2);
    TEST_ASSERT(!BITSET_Get(bs1, 100));
    TEST_ASSERT(BITSET_Get(bs1, 2));
    TEST_ASSERT(BITSET_BitCnt(bs1) == 1);
    TEST_ASSERT(BITSET_Length(bs1) == 3);

    BITSET_ClearAll(bs2);
    TEST_ASSERT(BITSET_Set(bs1, 100));
    TEST_ASSERT(BITSET_Set(bs1, 200));
    TEST_ASSERT(BITSET_Set(bs2, 200));
    BITSET_And(bs1, bs2);
    TEST_ASSERT(!BITSET_Get(bs1, 100));
    TEST_ASSERT(BITSET_Get(bs1, 200));
    TEST_ASSERT(BITSET_BitCnt(bs1) == 1);
    TEST_ASSERT(BITSET_Length(bs1) == 201);

    BITSET_Delete(bs1);
    BITSET_Delete(bs2);
    return TEST_OK;
}

static
TestStatus
test_bitset_or(
    const TestDesc* test)
{
    BitSet* bs1 = BITSET_Create();
    BitSet* bs2 = BITSET_Create();

    TEST_ASSERT(BITSET_Set(bs1, 1));
    TEST_ASSERT(BITSET_Set(bs2, 2));
    TEST_ASSERT(BITSET_Or(bs1, bs1));
    TEST_ASSERT(BITSET_Or(bs1, bs2));

    TEST_ASSERT(BITSET_Get(bs1, 1));
    TEST_ASSERT(BITSET_Get(bs1, 2));
    TEST_ASSERT(BITSET_BitCnt(bs1) == 2);
    TEST_ASSERT(BITSET_Length(bs1) == 3);

    TEST_ASSERT(BITSET_Set(bs2, 200));
    TEST_ASSERT(BITSET_Or(bs1, bs2));

    TEST_ASSERT(BITSET_Get(bs1, 1));
    TEST_ASSERT(BITSET_Get(bs1, 2));
    TEST_ASSERT(BITSET_Get(bs1, 200));
    TEST_ASSERT(BITSET_BitCnt(bs1) == 3);
    TEST_ASSERT(BITSET_Length(bs1) == 201);

    BITSET_Delete(bs1);
    BITSET_Delete(bs2);
    return TEST_OK;
}

static
TestStatus
test_bitset_xor(
    const TestDesc* test)
{
    BitSet* bs1 = BITSET_Create();
    BitSet* bs2 = BITSET_Create();

    TEST_ASSERT(BITSET_Set(bs1, 1));
    TEST_ASSERT(BITSET_Set(bs2, 2));
    TEST_ASSERT(BITSET_Xor(bs1, bs2));
    TEST_ASSERT(BITSET_Get(bs1, 1));
    TEST_ASSERT(BITSET_Get(bs1, 2));
    TEST_ASSERT(BITSET_BitCnt(bs1) == 2);
    TEST_ASSERT(BITSET_Length(bs1) == 3);

    TEST_ASSERT(BITSET_Xor(bs1, bs1));
    TEST_ASSERT(!BITSET_Get(bs1, 1));
    TEST_ASSERT(!BITSET_Get(bs1, 2));
    TEST_ASSERT(!BITSET_BitCnt(bs1));
    TEST_ASSERT(!BITSET_Length(bs1));

    TEST_ASSERT(BITSET_Xor(bs2, bs2));
    TEST_ASSERT(BITSET_Set(bs1, 1));
    TEST_ASSERT(BITSET_Set(bs1, 2));
    TEST_ASSERT(BITSET_Set(bs2, 1));
    TEST_ASSERT(BITSET_Set(bs2, 100));
    TEST_ASSERT(BITSET_Xor(bs1, bs2));
    TEST_ASSERT(BITSET_Get(bs1, 100));
    TEST_ASSERT(BITSET_Get(bs1, 2));
    TEST_ASSERT(!BITSET_Get(bs1, 1));
    TEST_ASSERT(BITSET_BitCnt(bs1) == 2);
    TEST_ASSERT(BITSET_Length(bs1) == 101);

    BITSET_Delete(bs1);
    BITSET_Delete(bs2);
    return TEST_OK;
}

static
TestStatus
test_bitset_andnot(
    const TestDesc* test)
{
    BitSet* bs1 = BITSET_Create();
    BitSet* bs2 = BITSET_Create();

    TEST_ASSERT(BITSET_Set(bs1, 100));
    TEST_ASSERT(BITSET_Copy(bs2, bs1));
    TEST_ASSERT(BITSET_Set(bs2, 1));
    BITSET_AndNot(bs2, bs1);
    TEST_ASSERT(BITSET_Get(bs2, 1));
    TEST_ASSERT(!BITSET_Get(bs2, 100));
    TEST_ASSERT(BITSET_BitCnt(bs2) == 1);
    TEST_ASSERT(BITSET_Length(bs2) == 2);

    TEST_ASSERT(BITSET_Xor(bs1, bs1));
    TEST_ASSERT(BITSET_Xor(bs2, bs2));
    BITSET_Trim(bs1);
    BITSET_Trim(bs2);
    TEST_ASSERT(BITSET_Set(bs2, 1));
    TEST_ASSERT(BITSET_Set(bs2, 2));
    TEST_ASSERT(BITSET_Set(bs1, 1));
    BITSET_AndNot(bs2, bs1);
    TEST_ASSERT(BITSET_Get(bs2, 2));
    TEST_ASSERT(!BITSET_Get(bs2, 1));
    TEST_ASSERT(BITSET_BitCnt(bs2) == 1);
    TEST_ASSERT(BITSET_Length(bs2) == 3);

    BITSET_Delete(bs1);
    BITSET_Delete(bs2);
    return TEST_OK;
}

static
TestStatus
test_bitset_nextbit(
    const TestDesc* test)
{
    BitSet* bs = BITSET_Create();
    static const int bits1[] = {1, 2, 3};
    static const int bits2[] = {1, 222, 333};
    static const int bits3[] = {31, 63, 255};
    static const int bits4[] = {111, 222, 333};
    static const struct _test_bitset_nextbit {
        int start;
        const int* bits;
        int nbits;
    } tests [] = {
        { -1, bits1, COUNT(bits1) },
        { 0, bits1, COUNT(bits1) },
        { -1, bits2, COUNT(bits2) },
        { -1, bits3, COUNT(bits3) },
        { 0, bits4, COUNT(bits4) },
    };

    int i, k;
    for (i = 0; i < COUNT(tests); i++) {
        int nextbit = tests[i].start;

        BITSET_ClearAll(bs);
        BITSET_Trim(bs);
        for (k = 0; k < tests[i].nbits; k++) {
            TEST_ASSERT(BITSET_Set(bs, tests[i].bits[k]));
        }
        for (k = 0; k < tests[i].nbits; k++) {
            nextbit = BITSET_NextBit(bs, nextbit);
            TEST_ASSERT(nextbit == tests[i].bits[k]);
        }
    }

    BITSET_ClearAll(bs);
    TEST_ASSERT(BITSET_Set(bs, 2));
    TEST_ASSERT(BITSET_NextBit(bs, 2) < 0);
    TEST_ASSERT(BITSET_NextBit(bs, 222) < 0);

    BITSET_Delete(bs);
    return TEST_OK;
}

int main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_bitset_alloc},
        {"Basic", test_bitset_basic},
        {"Equal", test_bitset_equal},
        {"Trim", test_bitset_trim},
        {"And", test_bitset_and},
        {"Or", test_bitset_or},
        {"Xor", test_bitset_xor},
        {"AndNot", test_bitset_andnot},
        {"NextBit", test_bitset_nextbit}
    };

    int ret;
    test_mem_init(&testMem);
    ret = TEST_MAIN(argc, argv, tests);
    test_mem_deinit(&testMem);
    return ret;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
