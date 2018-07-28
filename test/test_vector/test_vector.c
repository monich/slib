/*
 * $Id: test_vector.c,v 1.4 2018/07/28 16:22:20 slava Exp $
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
Bool
test_vector_equals_proc(
    VElementC e1,
    VElementC e2)
{
    return vectorEqualsDefault(e1, e2);
}

static
void
test_vector_free_proc(
    VElement e)
{
    vectorFreeNothingProc(e);
}

static
int
test_vector_strcmp_r(
    VElementC e1,
    VElementC e2)
{
    return vectorCompareString(e2, e1);
}

static
int
test_vector_strcasecmp_r(
    VElementC e1,
    VElementC e2)
{
    return vectorCompareStringNoCase(e2, e1);
}

static
TestStatus
test_vector_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Vector* v1;
    Vector* v2;
    int prevalloc;
    VElement* prevdata;

    /* Test NULL resistance */
    VECTOR_Delete(NULL);

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    if (VECTOR_Create(0, NULL, NULL)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount + 1;
    if (VECTOR_Create(1, NULL, NULL)) {
        ret = TEST_ERR;
    }

    v1 = VECTOR_Create(0, NULL, NULL);
    testMem.failAt = testMem.allocCount;
    if (VECTOR_Add(v1, NULL)) {
        ret = TEST_ERR;
    }

    v1->free = NULL;
    testMem.failAt = testMem.allocCount;
    if (VECTOR_TryAdd(v1, NULL)) {
        ret = TEST_ERR;
    }

    v1->free = vectorFreeNothingProc;
    testMem.failAt = testMem.allocCount;
    if (VECTOR_TryAdd(v1, NULL)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (VECTOR_Insert(v1, 0, NULL) ||
        VECTOR_Insert(v1, -1, NULL) ||
        VECTOR_Insert(v1, 1, NULL)) {
        ret = TEST_ERR;
    }

    v1->free = NULL;
    testMem.failAt = testMem.allocCount;
    if (VECTOR_TryInsert(v1, 0, NULL) ||
        VECTOR_TryInsert(v1, -1, NULL)||
        VECTOR_TryInsert(v1, 2, NULL)) {
        ret = TEST_ERR;
    }

    v1->free = vectorFreeNothingProc;
    testMem.failAt = testMem.allocCount;
    if (VECTOR_TryInsert(v1, 0, NULL)) {
        ret = TEST_ERR;
    }

    if (VECTOR_ToArray(v1)) {
        /* Returns NULL for empty arrays */
        ret = TEST_ERR;
    }

    if (!VECTOR_TryAdd(v1, NULL) ||
        !VECTOR_GetElements(v1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (VECTOR_ToArray(v1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (VECTOR_Iterator(v1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (VECTOR_ConstIterator(v1)) {
        ret = TEST_ERR;
    }

    v2 = VECTOR_Create(0, NULL, NULL);
    testMem.failAt = testMem.allocCount;
    if (VECTOR_CopyAll(v1, v2)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (VECTOR_Copy(v1, 0, 1, v2) ||
        VECTOR_Copy(v2, 0, 1, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Add(v2, NULL);
    VECTOR_Add(v2, NULL);
    VECTOR_Truncate(v2, 1);
    prevalloc = v2->alloc;
    prevdata = v2->data;
    testMem.failAt = testMem.allocCount;
    VECTOR_Trim(v2);
    if (v2->alloc != prevalloc ||
        v2->data != prevdata) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (VECTOR_SubVector(v2, -1, 1) ||
        VECTOR_SubVector(v2, 0, 2) ||
        VECTOR_SubVector(v2, 2, 0) ||
        VECTOR_SubVector(v2, 0, 1)) {
        ret = TEST_ERR;
    }

    VECTOR_Trim(v1);
    testMem.failAt = testMem.allocCount;
    if (!VECTOR_AddAll(v2, v1) ||
        VECTOR_AddAll(v1, v2)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (!VECTOR_InsertAll(v2, 0, v1) ||
        VECTOR_InsertAll(v2, -1, v1) ||
        VECTOR_InsertAll(v2, 2, v1) ||
        VECTOR_InsertAll(v1, 0, v2)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (VECTOR_InsertInOrder(v1, NULL, vectorCompareDefault) >= 0) {
        ret = TEST_ERR;
    }

    testMem.failAt = -1;
    VECTOR_Delete(v1);
    VECTOR_Delete(v2);
    return ret;
}

static
TestStatus
test_vector_basic(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Vector* v1 = VECTOR_Create(0, NULL, NULL);
    Vector* v2 = VECTOR_Create(0, NULL, NULL);
    Vector* v3 = NULL;
    const int n = 4;
    const VElement e1 = (VElement)(PtrWord)1;
    const VElement inval = (VElement)(PtrWord)(n+1);
    VElement* elems;
    int i;

    /* These have no effect on empty vectors */
    VECTOR_Trim(v1);
    VECTOR_Truncate(v2, 0);
    if (VECTOR_Pop(v1)) {
        ret = TEST_ERR;
    }

    /* Invalid indices */
    if (VECTOR_Get(v1, -1) ||
        VECTOR_Get(v1, 0)) {
        ret = TEST_ERR;
    }

    /* Fill the vectors */
    for (i=0; i<n; i++) {
        VElement e = (VElement)(PtrWord)(i+1);
        VECTOR_Add(v1, e);
        if (VECTOR_Get(v1, i) != e) {
            ret = TEST_ERR;
        }
    }

    /* Invalid index */
    if (VECTOR_Get(v1, i)) {
        ret = TEST_ERR;
    }

    /* Copy to v2 */
    VECTOR_CopyAll(v1, v2);
    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    /* Invalid moves */
    if (VECTOR_Move(v1, -1, 1) ||
        VECTOR_Move(v1, 1, -1) ||
        VECTOR_Move(v1, n, 0) ||
        VECTOR_Move(v1, 0, n)) {
        ret = TEST_ERR;
    }

    VECTOR_Swap(v1, -1, 1);
    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }
    
    VECTOR_Swap(v1, 1, -1);
    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }
    
    VECTOR_Swap(v1, n, 0);
    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }
    
    VECTOR_Swap(v1, 0, n);
    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    /* Valid move */
    if (VECTOR_Move(v1, 0, 1) != e1 ||
        VECTOR_Move(v1, 1, 1) != e1 ||
        VECTOR_Move(v1, 1, 0) != e1) {
        ret = TEST_ERR;
    }

    VECTOR_Swap(v1, 0, n-1);
    if (VECTOR_Equals(v1, v2) ||
        VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Swap(v1, 0, n-1);
    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    /* Setting or removing invalid index has no effect */
    VECTOR_Set(v1, -1,  inval);
    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Set(v1, n, inval);
    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    if (VECTOR_Remove(v1, -1) ||
        VECTOR_Remove(v1, n)) {
        ret = TEST_ERR;
    }

    if (VECTOR_RemoveElement(v1, inval)) {
        ret = TEST_ERR;
    }

    /* Truncate */
    VECTOR_Truncate(v2, 5);
    if (VECTOR_Size(v2) != 4) {
        ret = TEST_ERR;
    }

    VECTOR_Truncate(v2, 3);
    if (VECTOR_Size(v2) != 3) {
        ret = TEST_ERR;
    }

    VECTOR_Set(v2, 2, (VElement)(PtrWord)2);
    v2->free = NULL;
    VECTOR_Set(v2, 2, inval);
    if (VECTOR_Pop(v2) != inval ||
        VECTOR_Size(v2) != 2) {
        ret = TEST_ERR;
    }

    v2->free = vectorFreeNothingProc;
    VECTOR_Add(v2, inval);
    if (VECTOR_Pop(v2) != inval ||
        VECTOR_Size(v2) != 2) {
        ret = TEST_ERR;
    }

    VECTOR_Trim(v2);
    if (v2->size != v2->alloc ||
        v2->size != 2) {
        ret = TEST_ERR;
    }

    VECTOR_Truncate(v1, 2);
    if (!VECTOR_Equals(v1, v2)) {
        ret = TEST_ERR;
    }

    v1->free = NULL;
    VECTOR_Truncate(v1, 0);
    if (v1->size) {
        ret = TEST_ERR;
    }

    VECTOR_Trim(v1);
    if (v1->alloc) {
        ret = TEST_ERR;
    }

    if (!VECTOR_RemoveElement(v2, (VElement)(PtrWord)1)) {
        ret = TEST_ERR;
    }

    /* Fill */
    VECTOR_Clear(v1);
    VECTOR_Clear(v2);
    for (i=0; i<n; i++) {
        VECTOR_Add(v1, (VElement)(PtrWord)i);
        VECTOR_Add(v2, inval);
    }

    if (VECTOR_Equals(v1, v2) ||
        VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Fill(v1, inval);

    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    elems = VECTOR_ToArray(v1);
    for (i=0; i<n; i++) {
        if (elems[i] != inval) {
            ret = TEST_ERR;
        }
    }

    MEM_Free(elems);

    /* RemoveAll */
    if (VECTOR_RemoveDups(v1) != n-1 ||
        VECTOR_RemoveDups(v1) != 0) {
        ret = TEST_ERR;
    }

    for (i=0; i<n; i++) {
        if (VECTOR_RemoveAll(v2, v1) != 1) {
            ret = TEST_ERR;
        }
    }

    if (VECTOR_RemoveAll(v2, v1) != 0) {
        ret = TEST_ERR;
    }

    VECTOR_Set(v1, 0,  inval);
    VECTOR_Add(v2, NULL);
    if (VECTOR_RemoveAll(v2, v1) != 0) {
        ret = TEST_ERR;
    }

    VECTOR_Clear(v1);
    VECTOR_Clear(v2);
    for (i=0; i<n; i++) {
        VElement e = (VElement)(PtrWord)i;
        VECTOR_Add(v1, e);
        VECTOR_Add(v1, e);
        VECTOR_Add(v2, e);
    }

    VECTOR_Add(v1, NULL);
    if (VECTOR_RemoveDups(v1) != n+1 ||
        VECTOR_RemoveDups(v1) != 0 ||
        !VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    /* AddAll */
    VECTOR_Clear(v1);
    VECTOR_Clear(v2);
    for (i=0; i<n; i++) {
        VECTOR_Add(v1, (VElement)(PtrWord)i);
    }

    VECTOR_Delete(v3);
    v3 = VECTOR_SubVector(v1, 0, 2);
    if (!VECTOR_AddAll(v2, v3)) {
        ret = TEST_ERR;
    }

    VECTOR_Delete(v3);
    v3 = VECTOR_SubVector(v1, 2, n);
    if (!VECTOR_AddAll(v2, v3)) {
        ret = TEST_ERR;
    }

    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    /* InsertAll */
    VECTOR_Clear(v1);
    VECTOR_Clear(v2);
    for (i=0; i<n; i++) {
        VECTOR_Add(v1, (VElement)(PtrWord)i);
    }

    VECTOR_Delete(v3);
    v3 = VECTOR_SubVector(v1, 2, n);
    if (!VECTOR_InsertAll(v2, 0, v3)) {
        ret = TEST_ERR;
    }

    VECTOR_Delete(v3);
    v3 = VECTOR_SubVector(v1, 0, 2);
    if (!VECTOR_InsertAll(v2, 0, v3)) {
        ret = TEST_ERR;
    }

    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    /* RetainAll */
    VECTOR_Clear(v2);
    VECTOR_RetainAll(v1, v2);
    if (!VECTOR_IsEmpty(v1)) {
        ret = TEST_ERR;
    }
    for (i=0; i<n; i++) {
        VElement e = (VElement)(PtrWord)i;
        VECTOR_Add(v1, e);
        VECTOR_Add(v1, e);
    }

    VECTOR_Add(v2, (VElement)(PtrWord)2);
    VECTOR_RetainAll(v1, v2);
    if (VECTOR_Size(v1) != 2 ||
        VECTOR_Get(v1, 0) != VECTOR_Get(v2, 0) ||
        VECTOR_Get(v1, 1) != VECTOR_Get(v2, 0)) {
        ret = TEST_ERR;
    }

    /* RemoveRange */
    VECTOR_Clear(v1);
    for (i=0; i<n; i++) {
        VECTOR_Add(v1, (VElement)(PtrWord)i);
    }

    v1->free = test_vector_free_proc;
    if (VECTOR_RemoveRange(v1, -1, n+1) != n ||
        VECTOR_RemoveRange(v1, -1, n+1) != 0) {
        ret = TEST_ERR;
    }

    for (i=0; i<n; i++) {
        VECTOR_Add(v1, (VElement)(PtrWord)i);
    }

    v1->free = vectorFreeNothingProc;
    if (VECTOR_RemoveRange(v1, -1, n+1) != n ||
        VECTOR_RemoveRange(v1, -1, n+1) != 0) {
        ret = TEST_ERR;
    }

    VECTOR_Delete(v1);
    VECTOR_Delete(v2);
    VECTOR_Delete(v3);
    return ret;
}

static
TestStatus
test_vector_equals(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Vector* v1 = VECTOR_Create(1, vectorEqualsString, vectorFreeValueProc);
    Vector* v2 = VECTOR_Create(2, vectorEqualsString, vectorFreeValueProc);
    Vector* v3 = VECTOR_Create(3, vectorEqualsStringNoCase,
        vectorFreeValueProc);

    VECTOR_Add(v1, STRING_Dup("B"));
    VECTOR_Add(v1, STRING_Dup("A"));
    VECTOR_Add(v1, STRING_Dup("C"));
    VECTOR_Add(v2, STRING_Dup("A"));
    VECTOR_Add(v2, STRING_Dup("B"));
    VECTOR_Add(v2, STRING_Dup("C"));
    VECTOR_Insert(v3, 0, STRING_Dup("c"));
    VECTOR_Insert(v3, 0, STRING_Dup("a"));
    VECTOR_TryInsert(v3, 1, STRING_Dup("b"));

    if (VECTOR_Equals(v1, v2) ||
        VECTOR_Equals(v2, v1) ||
        VECTOR_Equals(v2, v3) ||
        !VECTOR_Equals(v3, v2) ||
        VECTOR_ContainsAny(v2, v3) ||
        !VECTOR_ContainsAny(v3, v2) ||
        !VECTOR_ContainsAny(v1, v2) ||
        !VECTOR_ContainsAny(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Swap(v1, 0, 0);
    VECTOR_Swap(v1, 0, 1);
    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Add(v1, NULL);
    if (VECTOR_IndexOf(v1, NULL) != 3 ||
        VECTOR_Equals(v1, v2) ||
        VECTOR_Equals(v2, v1) ||
        !VECTOR_ContainsAny(v1, v2) ||
        !VECTOR_ContainsAny(v2, v1) ||
        !VECTOR_ContainsAll(v1, v2) ||
        VECTOR_ContainsAll(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Remove(v1, 3);
    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1) ||
        !VECTOR_ContainsAny(v1, v2) ||
        !VECTOR_ContainsAny(v2, v1) ||
        !VECTOR_ContainsAll(v1, v2) ||
        !VECTOR_ContainsAll(v2, v1)) {
        ret = TEST_ERR;
    }

    if (VECTOR_RemoveAll(v1, v2) != 3 ||
        VECTOR_RemoveAll(v1, v2) != 0 ||
        VECTOR_Contains(v1, NULL) ||
        !VECTOR_IsEmpty(v1) ||
        VECTOR_ContainsAny(v2, v1) ||
        VECTOR_ContainsAny(v1, v2) ||
        VECTOR_Equals(v1, v2) ||
        VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Add(v1, STRING_Dup("D"));
    VECTOR_Add(v1, STRING_Dup("E"));
    VECTOR_Add(v1, STRING_Dup("F"));
    if (VECTOR_ContainsAny(v2, v1) ||
        VECTOR_ContainsAny(v1, v2) ||
        VECTOR_Equals(v1, v2) ||
        VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Delete(v1);
    VECTOR_Delete(v2);
    VECTOR_Delete(v3);
    return ret;
}

static
TestStatus
test_vector_iterator(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Vector* v1 = VECTOR_Create(0, NULL, NULL);
    Vector* v2 = VECTOR_Create(1, NULL, NULL);
    Iterator* it1;
    Iterator* it2;
    VElement e;
    int i;

    v1->free = NULL;
    v1->equals = test_vector_equals_proc;

    it1 = VECTOR_Iterator(v1);
    it2 = VECTOR_ConstIterator(v2);
    if (ITR_HasNext(it1) ||
        ITR_HasNext(it2) ||
        ITR_Next(it1) ||
        ITR_Next(it1) ||
        ITR_Last(it1) ||
        ITR_Last(it1) ||
        ITR_Remove(it1) ||
        ITR_Remove(it1)) {
        ret = TEST_ERR;
    }
    ITR_Delete(it1);
    ITR_Delete(it2);

    for (i=0; i<4; i++) {
        e = (VElement)(PtrWord)i;
        VECTOR_Add(v1, e);
        VECTOR_Add(v2, e);
        if (VECTOR_IndexOf(v1, e) != i ||
            VECTOR_IndexOf(v2, e) != i ||
            VECTOR_LastIndexOf(v1, e) != i ||
            VECTOR_LastIndexOf(v2, e) != i) {
            ret = TEST_ERR;
        }
    }

    e = (VElement)(PtrWord)i;
    if (VECTOR_IndexOf(v1, e) >= 0 ||
        VECTOR_IndexOf(v2, e) >= 0 ||
        VECTOR_LastIndexOf(v1, e) >= 0 ||
        VECTOR_LastIndexOf(v2, e) >= 0) {
        ret = TEST_ERR;
    }

    it1 = VECTOR_Iterator(v1);
    it2 = VECTOR_ConstIterator(v2);
    while (ITR_HasNext(it1)) {
        VElement e1 = ITR_Next(it1);
        VElement e2 = ITR_Next(it2);
        if (ITR_Last(it1) != e1 ||
            ITR_Last(it2) != e2 ||
            e1 != e2 ||
            !ITR_Remove(it1) ||
            ITR_Remove(it2)) {
            ret = TEST_ERR;
        }
    }

    if (ITR_HasNext(it2) ||
        VECTOR_Size(v1) != 0 ||
        VECTOR_Size(v2) != 4) {
        ret = TEST_ERR;
    }

    ITR_Delete(it1);
    ITR_Delete(it2);

    VECTOR_Delete(v1);
    VECTOR_Delete(v2);
    return ret;
}

static
TestStatus
test_vector_sort(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Vector* v1 = VECTOR_Create(0, NULL, NULL);
    Vector* v2 = VECTOR_Create(1, NULL, NULL);
    const VElement e0 = (VElement)(PtrWord)0;
    const VElement e1 = (VElement)(PtrWord)1;
    const VElement e2 = (VElement)(PtrWord)2;
    const VElement e4 = (VElement)(PtrWord)4;
    const VElement e5 = (VElement)(PtrWord)5;
    static const Str strings [] = {"a", "b", "c", "d", "e"};
    const int n = 5;
    int i;

    v1->free = test_vector_free_proc;

    for (i=0; i<n; i++) {
        VECTOR_Add(v1, (VElement)(PtrWord)i);
    }

    if (VECTOR_Copy(v1, -1, v1->size, v2) != n) {
        ret = TEST_ERR;
    }

    RAND_SetSeed(1);
    VECTOR_Shuffle(v1, NULL);
    VECTOR_Shuffle(v2, RANDOM_GetRandom());

    if (VECTOR_Equals(v1, v2) ||
        VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Sort(v1, vectorCompareDefault);
    VECTOR_Sort2(v2, vectorCompareDefault2, NULL);

    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Sort(v1, NULL);
    VECTOR_Sort2(v2, NULL, NULL);

    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    for (i=0; i<n; i++) {
        const VElement e = (VElement)(PtrWord)i;
        if (VECTOR_Search(v1, e, NULL) != i ||
            VECTOR_Search(v1, e, vectorCompareDefault) != i ||
            VECTOR_Search2(v1, e, NULL, NULL) != i ||
            VECTOR_Search2(v1, e, vectorCompareDefault2, NULL) != i) {
            ret = TEST_ERR;
        }
    }

    /* Non-existent element */
    if (VECTOR_Search(v1, e5, NULL) >= 0 ||
        VECTOR_Search(v1, e5, vectorCompareDefault) >= 0 ||
        VECTOR_Search2(v1, e5, NULL, NULL) >= 0 ||
        VECTOR_Search2(v1, e5, vectorCompareDefault2, NULL) >= 0) {
        ret = TEST_ERR;
    }

    /* Insert in order */
    VECTOR_Trim(v1);
    for (i=-1; i<=n; i++) {
        if (VECTOR_UpdateOrder(v1, i, vectorCompareDefault) != i) {
            ret = TEST_ERR;
        }
    }

    VECTOR_Insert(v1, 1, e5);
    if (VECTOR_UpdateOrder(v1, 1, vectorCompareDefault) != n ||
        VECTOR_InsertInOrder(v2, e5, vectorCompareDefault) != n ||
        !VECTOR_Equals(v1, v2)) {
        ret = TEST_ERR;
    }

    VECTOR_Truncate(v1, n);
    VECTOR_Truncate(v2, n);

    VECTOR_Insert(v1, 0, e4);
    if (VECTOR_UpdateOrder(v1, 0, vectorCompareDefault) != n ||
        VECTOR_InsertInOrder(v2, e4, vectorCompareDefault) != n ||
        !VECTOR_Equals(v1, v2)) {
        ret = TEST_ERR;
    }

    VECTOR_Truncate(v1, n);
    VECTOR_Truncate(v2, n);

    VECTOR_Insert(v1, n, e1);
    if (VECTOR_UpdateOrder(v1, n, vectorCompareDefault) != 2 ||
        VECTOR_InsertInOrder(v2, e1, vectorCompareDefault) != 2 ||
        !VECTOR_Equals(v1, v2)) {
        ret = TEST_ERR;
    }

    VECTOR_Remove(v1, 2);
    VECTOR_Insert(v1, 0, e1);
    if (VECTOR_UpdateOrder(v1, 0, vectorCompareDefault) != 2 ||
        !VECTOR_Equals(v1, v2)) {
        ret = TEST_ERR;
    }

    VECTOR_Remove(v1, 2);
    VECTOR_Remove(v2, 2);
    VECTOR_Remove(v1, 0);
    VECTOR_Remove(v2, 0);

    VECTOR_Insert(v1, n-1, e0);
    if (VECTOR_UpdateOrder(v1, n-1, vectorCompareDefault) != 0 ||
        VECTOR_InsertInOrder(v2, e0, vectorCompareDefault) != 0 ||
        !VECTOR_Equals(v1, v2)) {
        ret = TEST_ERR;
    }

    VECTOR_Remove(v1, 0);
    VECTOR_Remove(v2, 0);
    VECTOR_Insert(v1, 2, e2);
    VECTOR_Insert(v1, 2, e2);
    VECTOR_Insert(v1, 2, e2);
    VECTOR_Insert(v2, 2, e2);
    VECTOR_Insert(v2, 2, e2);
    if (VECTOR_InsertInOrder(v2, e2, vectorCompareDefault) != 4 ||
        !VECTOR_Equals(v1, v2)) {
        ret = TEST_ERR;
    }

    /* Strings */
    VECTOR_Delete(v1);
    VECTOR_Delete(v2);

    v1 = VECTOR_Create(0, vectorEqualsString, vectorFreeValueProc);
    v2 = VECTOR_Create(0, vectorEqualsStringNoCase, vectorFreeValueProc);

    VECTOR_Add(v1, NULL);
    VECTOR_Add(v2, NULL);
    for (i=0; i<COUNT(strings); i++) {
        VECTOR_Add(v1, STRING_Dup(strings[i]));
        VECTOR_Add(v2, STRING_Dup(strings[i]));
    }

    VECTOR_Shuffle(v1, NULL);
    VECTOR_Shuffle(v2, NULL);

    if (VECTOR_Equals(v1, v2) ||
        VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Sort(v1, vectorCompareString);
    VECTOR_Sort(v2, vectorCompareStringNoCase);

    if (!VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Swap(v1, 0, VECTOR_Size(v1)-1);
    if (!VECTOR_Sort(v1, test_vector_strcmp_r) ||
        !VECTOR_Sort(v1, vectorCompareString) ||
        !VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Reverse(v1);
    if (!VECTOR_Sort(v2, test_vector_strcasecmp_r) ||
        !VECTOR_Equals(v1, v2) ||
        !VECTOR_Equals(v2, v1)) {
        ret = TEST_ERR;
    }

    VECTOR_Delete(v1);
    VECTOR_Delete(v2);
    return ret;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_vector_alloc},
        {"Basic", test_vector_basic},
        {"Equals", test_vector_equals},
        {"Iterator", test_vector_iterator},
        {"Sort", test_vector_sort}
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
