/*
 * $Id: test_mpm.c,v 1.1 2016/09/26 16:09:19 slava Exp $
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

static TestMem testMem;

static
Bool
test_equal(
    const BigInt* b,
    Str str)
{
    BigInt* b1 = BIGINT_Create();
    Bool equal = BIGINT_Parse(b1, str, 0) && BIGINT_Equal(b1, b);
    BIGINT_Delete(b1);
    return equal;
}

static
TestStatus
test_mpm_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_Create32(0);
    BigInt* b2 = BIGINT_Create32(1);
    BigInt* b3 = BIGINT_Create();
    StrBuf* sb = STRBUF_Create();
    int i;

    /* BIGINT_Delete should be NULL resistant */
    BIGINT_Delete(NULL);

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Create()) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BIGINT_Create32(0)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount + 1;
    if (BIGINT_Create32(1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BIGINT_Create64(0)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount + 1;
    if (BIGINT_Create64(1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BIGINT_SetValue32(b1, 1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BIGINT_SetValue64(b1, 1) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BIGINT_Add(b1, b2) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    /* Left shift requires allocation too */
    testMem.failAt = testMem.allocCount;
    if (!BIGINT_ShiftLeft(b2, 0) ||
        BIGINT_ShiftLeft(b2, 256) ||
        !BIGINT_IsOne(b2)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (!BIGINT_ShiftRight(b2, 0) ||
        BIGINT_ShiftRight(b2, -256) ||
        !BIGINT_IsOne(b2)) {
        ret = TEST_ERR;
    }

    /* Make add/sub fail (except when we are adding/subtracting zero) */
    testMem.failAt = testMem.allocCount;
    if (!BIGINT_AddInt(b1, 0) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_SubInt(b1, 0) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    BIGINT_Zero(b1);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Sub(b1, b2) ||
        !BIGINT_IsZero(b1)) {
       ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 1);
    BIGINT_ShiftLeft(b2, 256);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Add(b1, b2) ||
        !BIGINT_IsOne(b1)) {
       ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BIGINT_Sub(b1, b2) ||
        !BIGINT_IsOne(b1)) {
       ret = TEST_ERR;
    }

    BIGINT_Neg(b2);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Add(b1, b2) ||
        !BIGINT_IsOne(b1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BIGINT_Sub(b1, b2) ||
        !BIGINT_IsOne(b1)) {
       ret = TEST_ERR;
    }

    BIGINT_SetInt(b2, 1);
    BIGINT_SetInt(b1, 1);
    BIGINT_ShiftLeft(b2, 256);
    BIGINT_Sub(b2, b1);
    BIGINT_Copy(b1, b2);
    BIGINT_Compact(b2);
    BIGINT_Compact(b1);

    testMem.failAt = testMem.allocCount;
    if (BIGINT_Add(b2, b1)) {
        ret = TEST_ERR;
    }

    /* This one takes two allocations */
    BIGINT_SetInt(b1, 1);
    BIGINT_ShiftLeft(b1, 512);
    BIGINT_SubInt(b1, 1);
    BIGINT_SetInt(b2, 3);
    BIGINT_Compact(b2);
    BIGINT_Compact(b1);

    testMem.failAt = testMem.allocCount;
    if (BIGINT_Add(b2, b1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount + 1;
    if (BIGINT_Add(b2, b1)) {
        ret = TEST_ERR;
    }

    /* Clone takes two allocations */
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Clone(b2)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount + 1;
    if (BIGINT_Clone(b2)) {
        ret = TEST_ERR;
    }

    /* Compact leaves things as is if it fails to reallocate the buffer */
    BIGINT_SetInt(b1, 1);
    BIGINT_ShiftLeft(b1, 256);  /* This takes 9 words */
    BIGINT_SubInt(b1, 1);       /* Now only 8 words are required */
    testMem.failAt = testMem.allocCount;
    BIGINT_Compact(b1);
    if (b1->alloc == b1->len) {
        ret = TEST_ERR;
    }

    /* Multiplication failures */
    BIGINT_SetInt(b1, 0x40000000);
    BIGINT_ShiftLeft(b1, 7*32);
    BIGINT_Compact(b1);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_MulInt(b1, 4) ||
        b1->len != 8 ||
        b1->value[7] != 0x40000000) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BIGINT_Square(b1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b2, 1);
    BIGINT_MulInt(b1, 4);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Mul(b2, b1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1,2);
    BIGINT_Copy(b2, b1);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Mul(b1, b2) ||
        BIGINT_LowBit(b1) != 1 ||
        BIGINT_BitLen(b1) != 2) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 1);
    BIGINT_ShiftLeft(b1, 255);
    BIGINT_Copy(b2, b1);
    BIGINT_Compact(b1);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Mul(b1, b2) ||
        BIGINT_LowBit(b1) != 255 ||
        BIGINT_BitLen(b1) != 256) {
        ret = TEST_ERR;
    }

    /* Division failures */
    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 5);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Div(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 2)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 5);
    BIGINT_SetInt(b2, 2);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Div(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 5)) {
        ret = TEST_ERR;
    }

    BIGINT_SetValue64(b1, __INT64_C(-0x100000003));
    BIGINT_SetValue64(b2, __INT64_C(0x100000001));
    for (i=0; i<3; i++) {
        BIGINT_Zero(b3);
        testMem.failAt = testMem.allocCount + i;
        if (BIGINT_Div(b1, b2, b3) ||
            !test_equal(b1, "-0x100000003")) {
            ret = TEST_ERR;
        }
    }

    /* Exponent */
    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 8);
    for (i=0; i<5; i++) {
        testMem.failAt = testMem.allocCount + i;
        if (BIGINT_Exp(b1, b2) ||
            !BIGINT_EqualInt(b1, 2)) {
            ret = TEST_ERR;
        }
    }

    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 4);
    BIGINT_SetInt(b3, 5);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_ExpMod(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 2)) {
        ret = TEST_ERR;
    }

#if 0
    /* This allocation failure is damaging b1 */
    testMem.failAt = testMem.allocCount + 1;
    if (BIGINT_ExpMod(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 2)) {
        ret = TEST_ERR;
    }
#endif

    for (i=2; i<5; i++) {
        testMem.failAt = testMem.allocCount + i;
        if (BIGINT_ExpMod(b1, b2, b3) ||
            !BIGINT_EqualInt(b1, 2)) {
            ret = TEST_ERR;
        }
    }

    /* gcd */
    BIGINT_SetInt(b1, 0);
    BIGINT_SetInt(b2, 888);
    BIGINT_SetInt(b3, 666);
    testMem.failAt = testMem.allocCount;
    if (BIGINT_Gcd(b1, b2, b3) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    /* These damage b1 */
    BIGINT_SetInt(b1, 0);
    testMem.failAt = testMem.allocCount + 1;
    if (BIGINT_Gcd(b1, b2, b3)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 0);
    testMem.failAt = testMem.allocCount + 2;
    if (BIGINT_Gcd(b1, b2, b3)) {
        ret = TEST_ERR;
    }

    /* Mod inverse */
    BIGINT_SetInt(b1, 31);
    BIGINT_SetInt(b2, 37);
    for (i=0; i<10; i++) {
        testMem.failAt = testMem.allocCount + i;
        if (BIGINT_ModInverse(b1, b2) ||
            !BIGINT_EqualInt(b1, 31)) {
            ret = TEST_ERR;
        }
    }

    /* Formatting */
    BIGINT_Parse(b1, "0x101010102020202030303030", 16);
    for (i=0; i<2; i++) {
        STRBUF_Destroy(sb);
        STRBUF_Init(sb);
        testMem.failAt = testMem.allocCount + i;
        if (BIGINT_Format(b1, sb, 0) ||
            STRBUF_Length(sb)) {
            ret = TEST_ERR;
        }
    }

    testMem.failAt = -1;
    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    BIGINT_Delete(b3);
    STRBUF_Delete(sb);
    return ret;
}

static
TestStatus
test_mpm_basic(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 =  BIGINT_Create32(0);
    BigInt* b3 = BIGINT_Create64(__INT64_C(0x100000000));
    BigInt* b2;
    int i;

    static const int value[] = {
        -100, -1, 0, 1, 100
    };

    for (i=0; i<COUNT(value); i++) {
        int k;
        if (BIGINT_EqualInt(b3, value[i])) {
            ret = TEST_ERR;
        }
        BIGINT_SetValue32(b1, value[i]);
        if (!BIGINT_EqualInt(b1, value[i])) {
            ret = TEST_ERR;
        }
        for (k=0; k<COUNT(value); k++) {
            if (k != i && BIGINT_EqualInt(b1, value[k])) {
                ret = TEST_ERR;
            }
        }
    }

    BIGINT_SetValue32(b1, 1);
    BIGINT_Zero(b1);

    /* Zero */
    BIGINT_Compact(b1);
    if (!BIGINT_IsZero(b1) ||
        BIGINT_IsPos(b1) ||
        BIGINT_IsNeg(b1) ||
        BIGINT_IsOne(b1) ||
        BIGINT_IsOdd(b1) ||
        !BIGINT_IsEven(b1)) {
        ret = TEST_ERR;
    }

    b2 = BIGINT_Clone(b1);
    if (!BIGINT_IsZero(b2)) {
        ret = TEST_ERR;
    }
  
    BIGINT_Delete(b2);
    b2 = BIGINT_Create64(0);
    if (!BIGINT_Equal(b1, b2) ||
        BIGINT_Compare(b1, b2)) {
        ret = TEST_ERR;
    }

    BIGINT_SetValue32(b1, 0);
    if (!BIGINT_IsZero(b1) ||
        BIGINT_IsPos(b1) ||
        BIGINT_IsNeg(b1) ||
        BIGINT_IsOne(b1)) {
        ret = TEST_ERR;
    }

    /* One */
    BIGINT_SetValue64(b1, 1);

    BIGINT_Delete(b2);
    b2 = BIGINT_Create32(1);
    if (!BIGINT_Equal(b1, b2) ||
        BIGINT_Compare(b1, b2)) {
        ret = TEST_ERR;
    }
    BIGINT_Delete(b2);
    b2 = BIGINT_Create64(1);
    if (!BIGINT_Equal(b1, b2) ||
        BIGINT_Compare(b1, b2)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_IsOne(b1) ||
        !BIGINT_IsPos(b1) ||
        !BIGINT_IsOdd(b1) ||
        BIGINT_IsNeg(b1) ||
        BIGINT_IsEven(b1)) {
        ret = TEST_ERR;
    }

    /* Minus one */
    BIGINT_SetValue32(b1, -1);

    BIGINT_Delete(b2);
    b2 =  BIGINT_Create32(-1);
    if (!BIGINT_Equal(b1, b2) ||
        BIGINT_Compare(b1, b2)) {
        ret = TEST_ERR;
    }
    BIGINT_Delete(b2);
    b2 =  BIGINT_Create64(-1);
    if (!BIGINT_Equal(b1, b2) ||
        BIGINT_Compare(b1, b2)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_IsOne(b1) ||
        !BIGINT_IsNeg(b1) ||
        !BIGINT_IsOdd(b1) ||
        BIGINT_IsPos(b1) ||
        BIGINT_IsEven(b1)) {
        ret = TEST_ERR;
    }

    /* Two */
    BIGINT_SetValue32(b1, 2);

    if (BIGINT_Equal(b1, b2) ||
        !BIGINT_Compare(b1, b2)) {
        ret = TEST_ERR;
    }
    BIGINT_Neg(b2);
    if (BIGINT_Equal(b1, b2) ||
        !BIGINT_Compare(b1, b2)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_IsPos(b1) ||
        !BIGINT_IsEven(b1) ||
        BIGINT_IsOne(b1) ||
        BIGINT_IsNeg(b1) ||
        BIGINT_IsOdd(b1)) {
        ret = TEST_ERR;
    }

    /* Bit length */
    BIGINT_Zero(b1);
    if (BIGINT_LowBit(b1) != -1 ||
        BIGINT_BitLen(b1) != 0 ||
        BIGINT_IsBitSet(b1, 0)) {
        ret = TEST_ERR;
    }

    for (i=0; i<=256; i++) {
        if (!BIGINT_SetInt(b1, 1) ||
            !BIGINT_ShiftLeft(b1, i) ||
            !BIGINT_MulInt(b1, -1) ||
            BIGINT_BitLen(b1) != i) {
            ret = TEST_ERR;
        }
        if (!BIGINT_SetInt(b1, 1) ||
            !BIGINT_ShiftLeft(b1, i) ||
            BIGINT_BitLen(b1) != i+1 ||
            BIGINT_LowBit(b1) != i ||
            BIGINT_BitLen(b1) != (BIGINT_LowBit(b1)+1) ||
            !BIGINT_IsBitSet(b1, i) ||
            BIGINT_IsBitSet(b1, i+1)) {
            ret = TEST_ERR;
        }
        if (i && BIGINT_IsBitSet(b1, 0)) {
            ret = TEST_ERR;
        }
    }

    for (i = 1; i < 256; i++) {
        if (!BIGINT_SetInt(b1, 1) ||
            !BIGINT_ShiftLeft(b1, i) ||
            !BIGINT_AddInt(b1, 1) ||
            BIGINT_BitLen(b1) != i+1 ||
            BIGINT_LowBit(b1) != 0) {
            ret = TEST_ERR;
        }
        /* The same but negative */
        if (!BIGINT_SetInt(b1, 1) ||
            !BIGINT_ShiftLeft(b1, i) ||
            !BIGINT_AddInt(b1, 1) ||
            !BIGINT_MulInt(b1, -1) ||
            BIGINT_BitLen(b1) != i+1) {
            ret = TEST_ERR;
        }
    }

    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    BIGINT_Delete(b3);
    return ret;
}

static
TestStatus
test_mpm_compare(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_Create32(0);
    BigInt* b2 = BIGINT_Create64(0);

    if (!BIGINT_Equal(b1, b2) ||
        !BIGINT_Equal(b2, b1) ||
        BIGINT_AbsCompare(b1, b2) ||
        BIGINT_AbsCompare(b2, b1) ||
        BIGINT_Compare(b1, b2) ||
        BIGINT_Compare(b2, b1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetValue32(b1, 0);
    BIGINT_SetValue64(b2, 0);
    if (!BIGINT_Equal(b1, b2) ||
        !BIGINT_Equal(b2, b1) ||
        BIGINT_AbsCompare(b1, b2) ||
        BIGINT_AbsCompare(b2, b1) ||
        BIGINT_Compare(b1, b2) ||
        BIGINT_Compare(b2, b1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetValue64(b1, 1);
    BIGINT_SetValue64(b2, -1);

    if (BIGINT_Equal(b1, b2) ||
        BIGINT_Equal(b2, b1) ||
        BIGINT_AbsCompare(b1, b2) ||
        BIGINT_AbsCompare(b2, b1) ||
        BIGINT_Compare(b1, b2) <= 0 ||
        BIGINT_Compare(b2, b1) >= 0) {
        ret = TEST_ERR;
    }

    BIGINT_Neg(b2);
    if (!BIGINT_Equal(b1, b2) ||
        !BIGINT_Equal(b2, b1) ||
        BIGINT_AbsCompare(b1, b2) ||
        BIGINT_AbsCompare(b2, b1) ||
        BIGINT_Compare(b1, b2) ||
        BIGINT_Compare(b2, b1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetValue64(b1, __INT64_C(1) << 8*sizeof(BIElem));
    if (BIGINT_Equal(b1, b2) ||
        BIGINT_Equal(b2, b1) ||
        BIGINT_AbsCompare(b1, b2) <= 0 ||
        BIGINT_AbsCompare(b2, b1) >= 0 ||
        BIGINT_Compare(b1, b2) <= 0 ||
        BIGINT_Compare(b2, b1) >= 0) {
        ret = TEST_ERR;
    }

    BIGINT_Neg(b1);
    if (BIGINT_Equal(b1, b2) ||
        BIGINT_Equal(b2, b1) ||
        BIGINT_AbsCompare(b1, b2) <= 0 ||
        BIGINT_AbsCompare(b2, b1) >= 0 ||
        BIGINT_Compare(b1, b2) >= 0 ||
        BIGINT_Compare(b2, b1) <= 0) {
        ret = TEST_ERR;
    }

    BIGINT_Delete(b2);
    b2 = BIGINT_Create64(__INT64_C(2) << 8*sizeof(BIElem));
    BIGINT_Neg(b2);
    if (BIGINT_Equal(b1, b2) ||
        BIGINT_Equal(b2, b1) ||
        BIGINT_AbsCompare(b1, b2) >= 0 ||
        BIGINT_AbsCompare(b2, b1) <= 0 ||
        BIGINT_Compare(b1, b2) <= 0 ||
        BIGINT_Compare(b2, b1) >= 0) {
        ret = TEST_ERR;
    }

    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    return ret;
}

static
TestStatus
test_mpm_shift(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b = BIGINT_Create();

    /* No matter how we shift zero, it's still zero */
    if (!BIGINT_ShiftLeft(b, 0) ||
        !BIGINT_IsZero(b) ||
        !BIGINT_ShiftLeft(b, 256) ||
        !BIGINT_IsZero(b) ||
        !BIGINT_ShiftLeft(b, -256) ||
        !BIGINT_IsZero(b) ||
        !BIGINT_ShiftRight(b, 0) ||
        !BIGINT_IsZero(b) ||
        !BIGINT_ShiftRight(b, 256) ||
        !BIGINT_IsZero(b) ||
        !BIGINT_ShiftRight(b, -256) ||
        !BIGINT_IsZero(b)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_SetInt(b, 1) ||
        !BIGINT_ShiftLeft(b, 256) ||
        b->len != 9 ||
        b->value[8] != 0x00000001) {
        ret = TEST_ERR;
    }

    if (!BIGINT_ShiftRight(b, 256) ||
        !BIGINT_IsOne(b)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_ShiftRight(b, 1) ||
        !BIGINT_IsZero(b)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_SetInt(b, 0x11) ||
        !BIGINT_ShiftRight(b, 5) ||
        !BIGINT_IsZero(b)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_SetInt(b, 0x11) ||
        !BIGINT_ShiftRight(b, 4) ||
        !BIGINT_IsOne(b)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_SetValue64(b, __INT64_C(0x1100000000)) ||
        !BIGINT_ShiftRight(b, 36) ||
        !BIGINT_IsOne(b) ||
        b->len != 1) {
        ret = TEST_ERR;
    }

    if (!BIGINT_SetValue64(b, __INT64_C(0x1100000000)) ||
        !BIGINT_ShiftRight(b, 5) ||
        !test_equal(b, "0x88000000")) {
        ret = TEST_ERR;
    }

    if (!BIGINT_SetValue64(b, __INT64_C(0x1100000000)) ||
        !BIGINT_ShiftRight(b, 8) ||
        !test_equal(b, "0x11000000")) {
        ret = TEST_ERR;
    }

    BIGINT_Delete(b);
    return ret;
}

static
TestStatus
test_mpm_add(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_Create();
    BigInt* b2 = BIGINT_Create();

    /* 0 + 0 = 0 */
    if (!BIGINT_Add(b1, b2) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    /* 1 + 0 = 1 */
    if (!BIGINT_AddInt(b1, 1) ||
        !BIGINT_IsOne(b1) ||
        !BIGINT_IsPos(b1)) {
        ret = TEST_ERR;
    }
    if (!BIGINT_Add(b1, b2) ||
        !BIGINT_IsOne(b1) ||
        !BIGINT_IsPos(b1)) {
        ret = TEST_ERR;
    }

    /* 0 + 1 = 1 */
    if (!BIGINT_Add(b2, b1) ||
        !BIGINT_IsOne(b2) ||
        !BIGINT_IsPos(b2)) {
        ret = TEST_ERR;
    }

    /* (-1) + 1 = 0 */
    BIGINT_AddInt(b2, -2);
    if (!BIGINT_Add(b2, b1) ||
        !BIGINT_IsZero(b2) ||
        BIGINT_IsPos(b2) ||
        BIGINT_IsNeg(b2)) {
        ret = TEST_ERR;
    }

    /* 1 + 1 = 2 */
    BIGINT_SetInt(b2, 1);
    if (!BIGINT_Add(b2, b1)) {
        ret = TEST_ERR;
    }
    BIGINT_SetInt(b1, 2);
    if (!BIGINT_Equal(b1, b2)) {
        ret = TEST_ERR;
    }

    /* 2 + (-1) = 1 */
    BIGINT_SetInt(b1, 1);
    BIGINT_Neg(b1);
    if (!BIGINT_Add(b2, b1)) {
        ret = TEST_ERR;
    }
    BIGINT_SetInt(b1, 1);
    if (!BIGINT_Equal(b1, b2)) {
        ret = TEST_ERR;
    }

    /* 1 + (-2) = -1 */
    BIGINT_SetInt(b2, 2);
    BIGINT_Neg(b2);
    if (!BIGINT_Add(b1, b2)) {
        ret = TEST_ERR;
    }
    BIGINT_SetInt(b2, 1);
    BIGINT_Neg(b2);
    if (!BIGINT_Equal(b1, b2)) {
        ret = TEST_ERR;
    }

    /* 0x100000000 + (-1) = 0xffffffff */
    BIGINT_SetValue64(b1, __INT64_C(0x100000000));
    if (!BIGINT_Add(b1, b2)) {
        ret = TEST_ERR;
    }
    BIGINT_SetValue64(b2, __INT64_C(0xffffffff));
    if (!BIGINT_Equal(b1, b2)) {
        ret = TEST_ERR;
    }

    /* 0xffffffff + (-0x100000000) = (-1) */
    BIGINT_SetValue64(b2, __INT64_C(0x100000000));
    BIGINT_Neg(b2);
    if (!BIGINT_Add(b1, b2) ||
        !BIGINT_IsOne(b1) ||
        !BIGINT_IsNeg(b1)) {
        ret = TEST_ERR;
    }

    /* 0xffffffff + 0xffffffff = 0x1fffffffe */
    BIGINT_SetValue64(b1, __INT64_C(0xffffffff));
    BIGINT_Copy(b2, b1);
    if (!BIGINT_Add(b1, b2)) {
        ret = TEST_ERR;
    }
    BIGINT_SetValue64(b2, __INT64_C(0x1fffffffe));
    if (!BIGINT_Equal(b1, b2)) {
        ret = TEST_ERR;
    }

    /* 1 + 0x10000..0000 = 0x10000..0001 */
    BIGINT_SetInt(b1, 1);
    BIGINT_SetInt(b2, 1);
    BIGINT_ShiftLeft(b2, 256);
    if (!BIGINT_Add(b1, b2) ||
        b1->len != 9 ||
        b1->value[0] != 1) {
        ret = TEST_ERR;
    }

    /* 0x10000..0000 + 1 = 0x10000..0001 */
    BIGINT_SetInt(b1, 1);
    BIGINT_SetInt(b2, 1);
    BIGINT_ShiftLeft(b1, 256);
    if (!BIGINT_Add(b1, b2) ||
        b1->len != 9 ||
        b1->value[0] != 1) {
        ret = TEST_ERR;
    }

    /* 0xffff..ffff + 0xffff..ffff = 0x1ffff..fffe */
    if (!BIGINT_SetInt(b1, 1) ||
        !BIGINT_SetInt(b2, 1) ||
        !BIGINT_ShiftLeft(b1, 256) ||
        !BIGINT_Sub(b1, b2) ||
        !BIGINT_Copy(b2, b1)) {
        ret = TEST_ERR;
    }
    BIGINT_Compact(b1);
    BIGINT_Compact(b2);
    if (!BIGINT_Add(b1, b2) ||
        b1->len != 9 ||
        b1->value[b1->len-1] != 0x00000001 ||
        b1->value[0] != 0xfffffffe) {
        ret = TEST_ERR;
    }

    /* 3 + 0xffff..ffff = 0x10000..0002 */
    if (!BIGINT_SetInt(b2, 1) ||
        !BIGINT_ShiftLeft(b2, 512) ||
        !BIGINT_SubInt(b2, 1) ||
        !BIGINT_SetInt(b1, 3)) {
        ret = TEST_ERR;
    }
    BIGINT_Compact(b1);
    BIGINT_Compact(b2);
    if (!BIGINT_Add(b1, b2) ||
        b1->len != 17 ||
        b1->value[b1->len-1] != 0x00000001 ||
        b1->value[0] != 0x00000002) {
        ret = TEST_ERR;
    }

    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    return ret;
}

static
TestStatus
test_mpm_sub(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_Create();
    BigInt* b2 = BIGINT_Create();

    /* 0 - 0 = 0 */
    if (!BIGINT_Sub(b1, b2) ||
        !BIGINT_Equal(b1, b2) ||
        !BIGINT_IsZero(b1) ||
        !BIGINT_IsZero(b2)) {
        ret = TEST_ERR;
    }

    /* 1 - 0 = 1 */
    if (!BIGINT_SetInt(b1, 1) ||
        !BIGINT_Sub(b1, b2) ||
        !BIGINT_IsOne(b1) ||
        !BIGINT_IsPos(b1)) {
        ret = TEST_ERR;
    }

    /* 0 - 1 = -1 */
    if (!BIGINT_Sub(b2, b1) ||
        !BIGINT_IsOne(b2) ||
        !BIGINT_IsNeg(b2)) {
        ret = TEST_ERR;
    }

    /* 1 - 1 = 0 */
    if (!BIGINT_SetInt(b2, 1) ||
        !BIGINT_Sub(b2, b1) ||
        !BIGINT_IsZero(b2)) {
        ret = TEST_ERR;
    }

    /* 1 - 2 = -1 */
    if (!BIGINT_SetInt(b1, 1) ||
        !BIGINT_SetInt(b2, 2)) {
        ret = TEST_ERR;
    }
    if (!BIGINT_Sub(b1, b2) ||
        !BIGINT_IsOne(b1) ||
        !BIGINT_IsNeg(b1)) {
        ret = TEST_ERR;
    }

    /* 1 - 0x10000..0001 = -10000..0000 */
    if (!BIGINT_SetInt(b1, 1) ||
        !BIGINT_SetInt(b2, 1) ||
        !BIGINT_ShiftLeft(b2, 256) ||
        !BIGINT_AddInt(b2, 1)) {
        ret = TEST_ERR;
    }
    if (!BIGINT_Sub(b1, b2) ||
        !BIGINT_IsNeg(b1) ||
        BIGINT_BitLen(b1) != 256 ||
        BIGINT_LowBit(b1) != 256) {
        ret = TEST_ERR;
    }

    /* (-0x10000..0000) - (-10000..0000) = 0 */
    if (!BIGINT_Copy(b2, b1) ||
        !BIGINT_Sub(b1, b2) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    /* (-0x10000..0000) - (-10000..0001) = 1 */
    if (!BIGINT_Copy(b1, b2) ||
        !BIGINT_SubInt(b2, 1) ||
        !BIGINT_Sub(b1, b2) ||
        !BIGINT_IsOne(b1) ||
        !BIGINT_IsPos(b1)) {
        ret = TEST_ERR;
    }

    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    return ret;
}

static
TestStatus
test_mpm_mul(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_CreateInt(1);
    BigInt* b2 = BIGINT_CreateInt(2);

    /* 1 * 1  = 1 */
    if (!BIGINT_MulInt(b1, 1) ||
        !BIGINT_IsOne(b1) ||
        !BIGINT_IsPos(b1)) {
        ret = TEST_ERR;
    }

    /* 1 * (-1)  = (-1) */
    if (!BIGINT_MulInt(b1, -1) ||
        !BIGINT_IsOne(b1) ||
        !BIGINT_IsNeg(b1)) {
        ret = TEST_ERR;
    }

    /* (-1) * 0 = 0 */
    if (!BIGINT_MulInt(b1, 0) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    /* 0 * 2 = 0 */
    if (!BIGINT_MulInt(b1, 2) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    /* 2 * 2 = 4 */
    if (!BIGINT_MulInt(b2, 2) ||
        !BIGINT_EqualInt(b2, 4)) {
        ret = TEST_ERR;
    }

    /* 4 * (-2) = (-8) */
    if (!BIGINT_MulInt(b2, -2) ||
        !BIGINT_EqualInt(b2, -8)) {
        ret = TEST_ERR;
    }

    /* 0 * 2 = 0 */
    BIGINT_Zero(b1);
    if (!BIGINT_SetInt(b2, 2) ||
        !BIGINT_Mul(b1, b2) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    /* 2 * 0 = 0 */
    if (!BIGINT_Mul(b2, b1) ||
        !BIGINT_IsZero(b2)) {
        ret = TEST_ERR;
    }

    /* 2 * 1  = 2 */
    if (!BIGINT_SetInt(b1, 2) ||
        !BIGINT_SetInt(b2, 1) ||
        !BIGINT_Mul(b1, b2) ||
        !BIGINT_EqualInt(b1, 2)) {
        ret = TEST_ERR;
    }

    /* 2 * (-1)  = (-2) */
    if (!BIGINT_SetInt(b2, -1) ||
        !BIGINT_Mul(b1, b2) ||
        !BIGINT_EqualInt(b1, -2)) {
        ret = TEST_ERR;
    }

    /* 1 * 2  = 2 */
    if (!BIGINT_SetInt(b1, 1) ||
        !BIGINT_SetInt(b2, 2) ||
        !BIGINT_Mul(b1, b2) ||
        !BIGINT_IsPos(b1) ||
        b1->len != 1 ||
        b1->value[0] != 2) {
        ret = TEST_ERR;
    }

    /* (-1) * 2  = (-2) */
    if (!BIGINT_SetInt(b2, -1) ||
        !BIGINT_Mul(b1, b2) ||
        !BIGINT_EqualInt(b1, -2)) {
        ret = TEST_ERR;
    }

    /* 0x40000000 * 4 = 0x100000000 */
    if (!BIGINT_SetInt(b2, 0x40000000) ||
        !BIGINT_MulInt(b2, 4) ||
        !test_equal(b2, "0x100000000")) {
        ret = TEST_ERR;
    }

    /* 0x100000000 * 2 = 0x200000000 */
    if (!BIGINT_SetInt(b1, 2) ||
        !BIGINT_Mul(b1, b2) ||
        !test_equal(b1, "0x200000000")) {
        ret = TEST_ERR;
    }

    /* 0x200000000 ^ 2 = 0x40000000000000000 */
    if (!BIGINT_Copy(b2, b1) ||
        !BIGINT_Mul(b1, b2) ||
        !BIGINT_Square(b2) ||
        !BIGINT_Equal(b1, b2) ||
        !test_equal(b1, "0x40000000000000000")) {
        ret = TEST_ERR;
    }

    /* 0x8000...0000 ^ 2 = 0x20000000..0000 */
    if (!BIGINT_SetInt(b1, 1) ||
        !BIGINT_ShiftLeft(b1, 255) ||
        !BIGINT_Copy(b2, b1)) {
        ret = TEST_ERR;
    }
    if (!BIGINT_Mul(b1, b2) ||
        BIGINT_LowBit(b1) != 510 ||
        BIGINT_BitLen(b1) != 511) {
        ret = TEST_ERR;
    }

    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    return ret;
}

static
TestStatus
test_mpm_div(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_Create();
    BigInt* b2 = BIGINT_CreateInt(2);
    BigInt* b3 = BIGINT_Create();
    int rem = -1;
    int i;

    static const struct _test_mpm_div {
        const char* b1;
        int radix1;
        const char* b2;
        int radix2;
        const char* out;
        const char* rem;
    } tests [] = {
        {
            "-100000003", 16,
            "100000001", 16,
            "-1", "-2"
        }, {
            "-10000000000000003", 16,
            "ffffffffffffffff", 16,
            "-1", "-4"
        }, {
            "2890762651252467304", 10,
            "1234132412341", 10,
            "2342344", "0"
        },{
            "393857363697266141270894839529661589545792779595532988400", 0,
            "1010101010101010101010101010101010101010", 16,
            "0xffffffff", "0"
        },{
            "010101010202020203030303", 16,
            "0101010102020202", 16,
            "4294967296", "50529027"
        },{
            "101010101010101010101010", 16,
            "10101010202020", 16,
            "1099511627520", "138247417872"
        },{
            "-17FA70B4A51F636A08305CC", 16,
            "100E893FDDC8FC", 16,
            "-102621747863",
            "-2979883906359592"
        },{
            "-17FA70B4A51F636A08305CC", 16,
            "100E893FDDC8FC", 16,
            "-102621747863", "-2979883906359592"
        },{
            "-B07B6A754A043FB4E0CCCF0392CFC25D02E8", 16,
            "-3060AF892D9BBD37", 16,
            "4410169439866585074807834", "-2886188871608377170"
        },{
            "DA51FD99C80D8F8A733285B44ABDEE728DED2F76C382B34E6C00", 16,
            "24017CCEAD4B2AB2", 16,
            "135219975754291382884190424624533457208822633",
            "396603263301567230"
        }
    };

    if (BIGINT_DivInt(b1, 0, NULL) ||
        !BIGINT_DivInt(b1, 1, NULL) ||
        !BIGINT_DivInt(b1, 1, &rem) ||
        !BIGINT_IsZero(b1) ||
        rem != 0) {
        ret = TEST_ERR;
    }

    /* 0/2 = 0(0) */
    BIGINT_Zero(b1);
    BIGINT_SetInt(b2, 2);
    if (!BIGINT_DivInt(b1, 2, &rem) ||
        !BIGINT_IsZero(b1) ||
        !BIGINT_IsZero(b3)) {
        ret = TEST_ERR;
    }

    /* 5/1 = 5(0) */
    BIGINT_SetInt(b1, 5);
    if (!BIGINT_DivInt(b1, 1, &rem) ||
        !BIGINT_EqualInt(b1, 5) ||
        rem != 0) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 5);
    BIGINT_SetInt(b2, 1);
    if (!BIGINT_Div(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 5) ||
        !BIGINT_IsZero(b3)) {
        ret = TEST_ERR;
    }

    /* 5/2 = 2(1) */
    BIGINT_SetInt(b1, 5);
    BIGINT_SetInt(b2, 2);
    if (!BIGINT_DivInt(b1, 2, &rem) ||
        !BIGINT_EqualInt(b1, 2) ||
        rem != 1) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 5);
    BIGINT_SetInt(b2, 2);
    if (!BIGINT_Div(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 2) ||
        !BIGINT_EqualInt(b3, 1)) {
        ret = TEST_ERR;
    }

    /* 5/(-2) = -2(1) */
    BIGINT_SetInt(b1, 5);
    if (!BIGINT_DivInt(b1, -2, &rem) ||
        !BIGINT_EqualInt(b1, -2) ||
        rem != 1) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 5);
    BIGINT_SetInt(b2, -2);
    if (!BIGINT_Div(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, -2) ||
        !BIGINT_EqualInt(b3, 1)) {
        ret = TEST_ERR;
    }

    /* 2/5 = 0(2) */
    BIGINT_SetInt(b1, 2);
    if (!BIGINT_DivInt(b1, 5, &rem) ||
        !BIGINT_IsZero(b1) ||
        rem != 2) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 5);
    if (!BIGINT_Div(b1, b2, b3) ||
        !BIGINT_IsZero(b1) ||
        !BIGINT_EqualInt(b3, 2)) {
        ret = TEST_ERR;
    }

    /* 5/5 = 1(0) */
    BIGINT_SetInt(b1, 5);
    if (!BIGINT_DivInt(b1, 5, &rem) ||
        !BIGINT_EqualInt(b1, 1) ||
        rem != 0) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 5);
    BIGINT_SetInt(b2, 5);
    if (!BIGINT_Div(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 1) ||
        !BIGINT_IsZero(b3)) {
        ret = TEST_ERR;
    }

    /* (-5)/(-5) = 1(0) */
    BIGINT_SetInt(b1, -5);
    if (!BIGINT_DivInt(b1, -5, &rem) ||
        !BIGINT_EqualInt(b1, 1) ||
        rem != 0) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, -5);
    BIGINT_SetInt(b2, -5);
    if (!BIGINT_Div(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 1) ||
        !BIGINT_IsZero(b3)) {
        ret = TEST_ERR;
    }

    /* (-5)/5 = -1(0) */
    BIGINT_SetInt(b1, -5);
    if (!BIGINT_DivInt(b1, 5, &rem) ||
        !BIGINT_EqualInt(b1, -1) ||
        rem != 0) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, -5);
    BIGINT_SetInt(b2, 5);
    if (!BIGINT_Div(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, -1) ||
        !BIGINT_IsZero(b3)) {
        ret = TEST_ERR;
    }

    /* 5/(-5) = -1(0) */
    BIGINT_SetInt(b1, 5);
    if (!BIGINT_DivInt(b1, -5, &rem) ||
        !BIGINT_EqualInt(b1, -1) ||
        rem != 0) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 5);
    BIGINT_SetInt(b2, -5);
    if (!BIGINT_Div(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, -1) ||
        !BIGINT_IsZero(b3)) {
        ret = TEST_ERR;
    }

    BIGINT_SetValue64(b1, __INT64_C(0x100000003));
    BIGINT_SetInt(b2, 0x40000000);
    if (!BIGINT_DivInt(b1, 4, &rem) ||
        !BIGINT_Equal(b1, b2) ||
        rem != 3) {
        ret = TEST_ERR;
    }

    BIGINT_SetValue64(b1, __INT64_C(0x100000003));
    BIGINT_SetInt(b2, 0x40000000);
    if (!BIGINT_Div(b1, b2, b3)) {
        ret = TEST_ERR;
    }

    for (i=0; i<COUNT(tests); i++) {
        if (!BIGINT_Parse(b1, tests[i].b1, tests[i].radix1) ||
            !BIGINT_Parse(b2, tests[i].b2, tests[i].radix2) ||
            !BIGINT_Div(b1, b2, b3) ||
            !test_equal(b1, tests[i].out) ||
            !test_equal(b3, tests[i].rem)) {
            ret = TEST_ERR;
        }
    }

#if 0
    {
        StrBuf* sb1 = STRBUF_Create();
        StrBuf* sb2 = STRBUF_Create();
        for (i=0; True; i++) {
            int k, n = RAND_NextInt(10);
            BIGINT_SetInt(b1, 1);
            for (k=0; k<n; k++) {
                BIGINT_MulInt(b1, RAND_NextI32());
            }
            n = RAND_NextInt(10);
            BIGINT_SetInt(b2, 1);
            for (k=0; k<n; k++) {
                BIGINT_MulInt(b2, RAND_NextI32());
            }
            BIGINT_Format(b1, sb1, 16);
            BIGINT_Format(b2, sb2, 16);
            if (!BIGINT_Div(b1, b2, b3)) {
                ret = TEST_ERR;
            }
        }
        STRBUF_Delete(sb1);
        STRBUF_Delete(sb2);
    }
#endif

    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    BIGINT_Delete(b3);
    return ret;
}

static
TestStatus
test_mpm_gcd(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_Create();
    BigInt* b2 = BIGINT_Create();
    BigInt* gcd = BIGINT_Create();

    BIGINT_SetInt(b1, 1234523);
    BIGINT_SetInt(b2, 3452);
    if (!BIGINT_Gcd(gcd, b1, b2) ||
        !BIGINT_EqualInt(gcd, 1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 1);
    if (!BIGINT_Gcd(gcd, b1, b2) ||
        !BIGINT_EqualInt(gcd, 1)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_Gcd(gcd, b2, b1) ||
        !BIGINT_EqualInt(gcd, 1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 888);
    BIGINT_SetInt(b2, 666);
    if (!BIGINT_Gcd(gcd, b1, b2) ||
        !BIGINT_EqualInt(gcd, 222)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 888);
    BIGINT_SetInt(b2, 8);
    if (!BIGINT_Gcd(gcd, b1, b2) ||
        !BIGINT_EqualInt(gcd, 8)) {
        ret = TEST_ERR;
    }

    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    BIGINT_Delete(gcd);
    return ret;
}

static
TestStatus
test_mpm_exp(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_Create();
    BigInt* b2 = BIGINT_CreateInt(-1);

    /* The exponent must be non-negative */
    if (BIGINT_Exp(b1, b2)) {
        ret = TEST_ERR;
    }

    /* 0^0 = 0 */
    BIGINT_Zero(b2);
    if (!BIGINT_Exp(b1, b2) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    /* 2^0 = 1 */
    BIGINT_SetInt(b1, 2);
    if (!BIGINT_Exp(b1, b2) ||
        !BIGINT_EqualInt(b1, 1)) {
        ret = TEST_ERR;
    }

    /* 2^1 = 2 */
    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 1);
    if (!BIGINT_Exp(b1, b2) ||
        !BIGINT_EqualInt(b1, 2)) {
        ret = TEST_ERR;
    }

    /* 2^8 = 256 */
    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 8);
    if (!BIGINT_Exp(b1, b2) ||
        !BIGINT_EqualInt(b1, 256)) {
        ret = TEST_ERR;
    }

    /* 2^7 = 128 */
    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 7);
    if (!BIGINT_Exp(b1, b2) ||
        !BIGINT_EqualInt(b1, 128)) {
        ret = TEST_ERR;
    }

    /* 1^2 = 1 */
    BIGINT_SetInt(b1, 1);
    BIGINT_SetInt(b2, 2);
    if (!BIGINT_Exp(b1, b2) ||
        !BIGINT_EqualInt(b1, 1)) {
        ret = TEST_ERR;
    }

    /* (-1)^2 = 1 */
    BIGINT_SetInt(b1, -1);
    BIGINT_SetInt(b2, 2);
    if (!BIGINT_Exp(b1, b2) ||
        !BIGINT_EqualInt(b1, 1)) {
        ret = TEST_ERR;
    }

    /* (-1)^3 = -1 */
    BIGINT_SetInt(b1, -1);
    BIGINT_SetInt(b2, 3);
    if (!BIGINT_Exp(b1, b2) ||
        !BIGINT_EqualInt(b1, -1)) {
        ret = TEST_ERR;
    }

    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    return ret;
}

static
TestStatus
test_mpm_expmod(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_Create();
    BigInt* b2 = BIGINT_CreateInt(-1);
    BigInt* b3 = BIGINT_Create();

    /* The exponent must be non-negative */
    if (BIGINT_ExpMod(b1, b2, b3)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 4);
    BIGINT_Zero(b3);
    if (BIGINT_ExpMod(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 2)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 4);
    BIGINT_SetInt(b3, 5);
    if (!BIGINT_ExpMod(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 5);
    BIGINT_SetInt(b3, 7);
    if (!BIGINT_ExpMod(b1, b2, b3) ||
        !BIGINT_EqualInt(b1, 4)) {
        ret = TEST_ERR;
    }

    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    BIGINT_Delete(b3);
    return ret;
}

static
TestStatus
test_mpm_modinverse(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_CreateInt(2);
    BigInt* b2 = BIGINT_CreateInt(-2);

    if (BIGINT_ModInverse(b2, b1)) {
        ret = TEST_ERR;
    }

    if (BIGINT_ModInverse(b1, b2)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b2, 2);
    if (BIGINT_ModInverse(b1, b2)) {
        ret = TEST_ERR;
    }

    /* Not invertible */
    BIGINT_SetInt(b1, 17);
    BIGINT_SetInt(b2, 731);
    if (BIGINT_ModInverse(b1, b2) ||
        !BIGINT_EqualInt(b1, 17)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 11);
    if (!BIGINT_Parse(b2, "786345126743", 10) ||
        BIGINT_ModInverse(b1, b2) ||
        !BIGINT_EqualInt(b1, 11)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 53);
    if (!BIGINT_Parse(b2, "638463524287654651231", 10) ||
        BIGINT_ModInverse(b1, b2) ||
        !BIGINT_EqualInt(b1, 53)) {
        ret = TEST_ERR;
    }

    /* Invertible */
    BIGINT_SetInt(b1, 2);
    BIGINT_SetInt(b2, 1);
    if (!BIGINT_ModInverse(b1, b2) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 1);
    BIGINT_SetInt(b2, 1);
    if (!BIGINT_ModInverse(b1, b2) ||
        !BIGINT_IsZero(b1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 23);
    BIGINT_SetInt(b2, 2);
    if (!BIGINT_ModInverse(b1, b2) ||
        !BIGINT_EqualInt(b1, 1)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 31);
    BIGINT_SetInt(b2, 32);
    if (!BIGINT_ModInverse(b1, b2) ||
        !BIGINT_EqualInt(b1, 31)) {
        ret = TEST_ERR;
    }

    BIGINT_SetInt(b1, 31);
    BIGINT_SetInt(b2, 37);
    if (!BIGINT_ModInverse(b1, b2) ||
        !BIGINT_EqualInt(b1, 6)) {
        ret = TEST_ERR;
    }

    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    return ret;
}

static
TestStatus
test_mpm_format(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    BigInt* b1 = BIGINT_Create();
    BigInt* b2 = BIGINT_Create();
    StrBuf* sb = STRBUF_Create();

    static const struct _test_format_int {
        int val;
        char* str;
        int radix;
    } tests1 [] = {
        { -10, "-10", 0 },
        { -20, "-20", 10 },
        { 0, "0", 0 },
        { 10, "10", 10 },
        { 16, "16", 10 },
        { 16, "10", 16 },
    };

    int i;
    for (i=0; i<COUNT(tests1); i++) {
        if (!BIGINT_SetInt(b1, tests1[i].val) ||
            !BIGINT_Format(b1, sb, tests1[i].radix) ||
            !STRBUF_EqualsTo(sb, tests1[i].str)) {
            ret = TEST_ERR;
        }
        if (!BIGINT_Parse(b1, STRBUF_Text(sb), tests1[i].radix) ||
            !BIGINT_EqualInt(b1, tests1[i].val)) {
            ret = TEST_ERR;
        }

        /* 36 is the maximum radix */
        if (!BIGINT_Format(b1, sb, 36)) {
            ret = TEST_ERR;
        }

        STRBUF_ToUpperCase(sb);
        if (!BIGINT_Parse(b1, STRBUF_Text(sb), 36) ||
            !BIGINT_EqualInt(b1, tests1[i].val)) {
            ret = TEST_ERR;
        }

        STRBUF_ToLowerCase(sb);
        if (!BIGINT_Parse(b1, STRBUF_Text(sb), 36) ||
            !BIGINT_EqualInt(b1, tests1[i].val)) {
            ret = TEST_ERR;
        }
    }

    if (!BIGINT_Parse(b1, "+123", 0) ||
        !BIGINT_EqualInt(b1, 123)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_Parse(b1, "-0x123", 0) ||
        !BIGINT_EqualInt(b1, -0x123)) {
        ret = TEST_ERR;
    }

    if (!BIGINT_Parse(b1, "0x123", 16) ||
        !BIGINT_EqualInt(b1, 0x123)) {
        ret = TEST_ERR;
    }

    /* Leading and trailing spaces should be ignored */
    if (!BIGINT_Parse(b1, " 20 ", 10) ||
        !BIGINT_EqualInt(b1, 20)) {
        ret = TEST_ERR;
    }

    if (BIGINT_Format(NULL, sb, 0) ||
        BIGINT_Format(b1, NULL, 0) ||
        BIGINT_Format(b1, sb, -1) ||
        BIGINT_Format(b1, sb, 37) ||
        BIGINT_Parse(b1, NULL, 0) ||
        BIGINT_Parse(b1, "", 40) ||
        BIGINT_Parse(b1, "", 10) ||
        BIGINT_Parse(b1, "", 0) ||
        BIGINT_Parse(b1, " ", 0) ||
        BIGINT_Parse(b1, "0x", 0) ||
        BIGINT_Parse(b1, "0x10", 10) ||
        BIGINT_Parse(b1, "1", -1) ||
        BIGINT_Parse(b1, "1#", 0) ||
        BIGINT_Parse(b1, "abc", 0) ||
        BIGINT_Parse(b1, "10 20", 0)) {
        ret = TEST_ERR;
    }

    STRBUF_Delete(sb);
    BIGINT_Delete(b1);
    BIGINT_Delete(b2);
    return ret;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_mpm_alloc},
        {"Basic", test_mpm_basic},
        {"Compare", test_mpm_compare},
        {"Shift", test_mpm_shift},
        {"Add", test_mpm_add},
        {"Sub", test_mpm_sub},
        {"Mul", test_mpm_mul},
        {"Div", test_mpm_div},
        {"Gcd", test_mpm_gcd},
        {"Exp", test_mpm_exp},
        {"ExpMod", test_mpm_expmod},
        {"ModInverse", test_mpm_modinverse},
        {"Format", test_mpm_format}
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
