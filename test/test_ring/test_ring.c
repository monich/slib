/*
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
TestStatus
test_ring_null(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;

    /* RING_Delete should be NULL resistant */
    RING_Delete(NULL);

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    if (RING_Create()) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (RING_Create2(0, 0)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount + 1;
    if (RING_Create2(1, 1)) {
        ret = TEST_ERR;
    }

    return ret;
}

static
TestStatus
test_ring_1(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Ring* r = RING_Create();
    int e1 = 1, e2 = 2, e3 = 3, e4 = 4;

    /* Make one put fail */
    testMem.failAt = testMem.allocCount;
    if (RING_Put(r, &e1)) {
        ret = TEST_ERR;
    }

    /* Put 3, check size and indices, get 3, nothing should be left */
    if (!RING_Put(r, &e1) ||
        !RING_Put(r, &e2) ||
        !RING_Put(r, &e3) ||
        RING_Size(r) != 3) {
        ret = TEST_ERR;
    }

    if (RING_ElementAt(r, 0) != &e1 || RING_IndexOf(r, &e1) != 0 ||
        RING_ElementAt(r, 1) != &e2 || RING_IndexOf(r, &e2) != 1 ||
        RING_ElementAt(r, 2) != &e3 || RING_IndexOf(r, &e3) != 2 ||
        RING_ElementAt(r, 3) || RING_IndexOf(r, &e4) >= 0) {
        ret = TEST_ERR;
    }

    /* Make one compact fail */
    testMem.failAt = testMem.allocCount;
    RING_Compact(r);
    RING_Compact(r);
    if (RING_Get(r) != &e1 || RING_IndexOf(r, &e1) >= 0) {
        ret = TEST_ERR;
    }

    RING_Compact(r);
    if (RING_Get(r) != &e2 || RING_IndexOf(r, &e2) >= 0 ||
        RING_Get(r) != &e3 || RING_IndexOf(r, &e3) >= 0) {
        ret = TEST_ERR;
    }

    RING_Compact(r);
    if (RING_Get(r) || !RING_IsEmpty(r)) {
        ret = TEST_ERR;
    }

    RING_Delete(r);
    return ret;
}

static
TestStatus
test_ring_2(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Ring* r = RING_Create2(0, 3);
    int e1 = 1, e2 = 2, e3 = 3, e4 = 4;
    /* Put 3, #4 fails, get 1, clear, nothing should be left */
    if (!RING_EnsureCapacity(r, 2) ||
        !RING_Put(r, &e1) ||
        !RING_Put(r, &e2) ||
        !RING_Put(r, &e3)) {
        ret = TEST_ERR;
    }

    RING_Compact(r);
    if (RING_Put(r, &e4) ||
        RING_PutFront(r, &e4)) {
        ret = TEST_ERR;
    }

    if (RING_Get(r) != &e1) {
        ret = TEST_ERR;
    }

    RING_Clear(r);
    if (RING_Get(r) || !RING_IsEmpty(r)) {
        ret = TEST_ERR;
    }

    RING_Delete(r);
    return ret;
}

static
TestStatus
test_ring_3(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Ring* r = RING_Create2(2, -1);
    int e1 = 1, e2 = 2, e3 = 3;

    if (!RING_Put(r, &e1) ||
        !RING_Put(r, &e2) ||
        !RING_PutFront(r, &e3)) {
        ret = TEST_ERR;
    }

    if (RING_Get(r) != &e3 ||
        RING_GetLast(r) != &e2 ||
        RING_Get(r) != &e1) {
        ret = TEST_ERR;
    }

    if (RING_Get(r) ||
        RING_GetLast(r) ||
        !RING_IsEmpty(r)) {
        ret = TEST_ERR;
    }

    RING_Delete(r);
    return ret;
}

static
TestStatus
test_ring_4(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Ring* r = RING_Create2(4,3);
    int e1 = 1, e2 = 2, e3 = 3, e4 = 4;

    if (!RING_PutFront(r, &e1) ||
        !RING_Put(r, &e2) ||
        !RING_PutFront(r, &e3)) {
        ret = TEST_ERR;
    }

    if (RING_EnsureCapacity(r, 4) ||
        RING_Put(r, &e4)) {
        ret = TEST_ERR;
    }

    if (RING_Get(r) != &e3 ||
        RING_GetLast(r) != &e2) {
        ret = TEST_ERR;
    }

    RING_Compact(r);
    if (RING_GetLast(r) != &e1) {
        ret = TEST_ERR;
    }

    if (RING_Get(r) ||
        RING_GetLast(r) ||
        !RING_IsEmpty(r)) {
        ret = TEST_ERR;
    }

    RING_Delete(r);
    return ret;
}

static
TestStatus
test_ring_5(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Ring* r = RING_Create2(3, -1);
    int e1 = 1, e2 = 2, e3 = 3;

    if (!RING_PutFront(r, &e3) ||
        !RING_PutFront(r, &e2) ||
        !RING_PutFront(r, &e1)) {
        ret = TEST_ERR;
    }

    if (RING_GetLast(r) != &e3 ||
        RING_Get(r) != &e1) {
        ret = TEST_ERR;
    }

    if (!RING_PutFront(r, &e1) ||
        RING_Get(r) != &e1 ||
        RING_Get(r) != &e2) {
        ret = TEST_ERR;
    }

    if (RING_Get(r) ||
        RING_GetLast(r) ||
        !RING_IsEmpty(r)) {
        ret = TEST_ERR;
    }

    RING_Delete(r);
    return ret;
}

static
TestStatus
test_ring_6(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Ring* r = RING_Create2(3, -1);
    int e1 = 1, e2 = 2, e3 = 3, e4 = 4;

    if (!RING_Put(r, &e1) ||
        !RING_Put(r, &e2) ||
        !RING_Put(r, &e3) ||
        !RING_Put(r, &e4) ||
        RING_GetLast(r) != &e4) {
        ret = TEST_ERR;
    }

    if (!RING_EnsureCapacity(r, r->alloc+1) ||
        RING_Get(r) != &e1 ||
        RING_Size(r) != 2) {
        ret = TEST_ERR;
    }

    if (!RING_EnsureCapacity(r, r->alloc+1) ||
        RING_GetLast(r) != &e3 ||
        RING_Size(r) != 1) {
        ret = TEST_ERR;
    }

    RING_Delete(r);
    return ret;
}

int main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"NULL", test_ring_null},
        {"Ring1", test_ring_1},
        {"Ring2", test_ring_2},
        {"Ring3", test_ring_3},
        {"Ring4", test_ring_4},
        {"Ring5", test_ring_5},
        {"Ring6", test_ring_6}
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
