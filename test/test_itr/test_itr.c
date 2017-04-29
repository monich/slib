/*
 * $Id: test_itr.c,v 1.2 2016/10/02 23:08:54 slava Exp $
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
TestStatus
test_itr_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static IElementC a1[] = { (IElementC)1 };
    static IElementC a2[] = { (IElementC)2 };
    Iterator* it1 = ITR_Array(a1, COUNT(a1));
    Iterator* it2 = ITR_Array(a2, COUNT(a2));
    Iterator* its[2];

    /* Test NULL resistance */
    ITR_Delete(NULL);
    if (ITR_HasNext(NULL) ||
        ITR_Filter(NULL, NULL, NULL, NULL)) {
        ret = TEST_ERR;
    }

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    if (ITR_Singleton(NULL)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (ITR_Array(a1, COUNT(a1))) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (ITR_Filter(it1, NULL, NULL, NULL)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (ITR_Filter(it1, NULL, NULL, NULL)) {
        ret = TEST_ERR;
    }

    its[0] = it1;
    its[1] = it2;
    testMem.failAt = testMem.allocCount;
    if (ITR_Combine(its, COUNT(its))) {
        ret = TEST_ERR;
    }

    testMem.failAt = -1;
    ITR_Delete(it1);
    ITR_Delete(it2);
    return ret;
}

static
TestStatus
test_itr_itra(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static IElementC a[] = { (IElementC)1,  (IElementC)2 };
    Iterator* it = ITR_Array(a, COUNT(a));
    int i;

    for (i=0; i<COUNT(a); i++) {
        if (!ITR_HasNext(it) ||
            ITR_Next(it) != a[i] ||
            ITR_Remove(it)) {
            ret = TEST_ERR;
        }
    }

    if (ITR_HasNext(it)) {
        ret = TEST_ERR;
    }

    ITR_Delete(it);
    it = ITR_Array(NULL, 0);
    if (ITR_HasNext(it)) {
        ret = TEST_ERR;
    }

    ITR_Delete(it);
    return ret;
}

static
TestStatus
test_itr_itrc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static IElementC a[] = { (IElementC)1,  (IElementC)2 };
    Vector* v = VECTOR_Create(0, NULL, NULL);
    Iterator* its[4];
    Iterator* it;
    int i;

    /* Combination of empty iterators is an empty iterator */
    its[0] = ITR_Empty();
    its[1] = ITR_Empty();
    it = ITR_Combine(its, 2);
    if (ITR_HasNext(it)) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);

    /* One empty and two non-empty (on read-only and one not) */ 
    VECTOR_Add(v, (VElement)3);
    VECTOR_Add(v, (VElement)4);
    its[0] = ITR_Array(a, COUNT(a));
    its[1] = ITR_Empty();
    its[2] = NULL;
    its[3] = VECTOR_Iterator(v);
    it = ITR_Combine(its, 4);
    for (i=0; i<4; i++) {
        if (!ITR_HasNext(it) ||
            ITR_Next(it) != ((IElementC)(PtrWord)(i+1))) {
            ret = TEST_ERR;
        }
        if (i < 2) {
            if (ITR_Remove(it)) {
                ret = TEST_ERR;
            }
        } else {
            if (!ITR_Remove(it)) {
                ret = TEST_ERR;
            }
        }
    }
    if (ITR_HasNext(it) ||
        !VECTOR_IsEmpty(v)) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);

    /* Pretty much the same as the above, only we don't iterate to the end */ 
    VECTOR_Add(v, (VElement)3);
    VECTOR_Add(v, (VElement)4);
    its[0] = ITR_Array(a, COUNT(a));
    its[1] = ITR_Empty();
    its[2] = VECTOR_Iterator(v);
    it = ITR_Combine(its, 3);
    for (i=0; i<2; i++) {
        if (!ITR_HasNext(it) ||
            ITR_Next(it) != ((IElementC)(PtrWord)(i+1))) {
            ret = TEST_ERR;
        }
    }
    if (!ITR_HasNext(it) ||
        VECTOR_Size(v) != 2) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);

    /* One empty and one non-empty (and read-only) */
    its[0] = ITR_Empty();
    its[1] = NULL;
    its[2] = ITR_Array(a, COUNT(a));
    it = ITR_Combine(its, 3);
    for (i=0; i<2; i++) {
        if (!ITR_HasNext(it) ||
            ITR_Next(it) != ((IElementC)(PtrWord)(i+1)) ||
            ITR_Remove(it)) {
            ret = TEST_ERR;
        }
    }
    if (ITR_HasNext(it)) {
        ret = TEST_ERR;
    }

    VECTOR_Delete(v);
    ITR_Delete(it);
    return ret;
}

static
IElement
test_itr_minus1(
    Iterator * target,
    void * ctx)
{
    IElement* e = ITR_Next(target);
    return (IElement)(((PtrWord)e) - 1);
}

static
Bool
test_itr_noremove(
    Iterator* target,
    void* ctx)
{
    return False;
}

static
TestStatus
test_itr_itrf(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static IElementC a[] = { (IElementC)1,  (IElementC)2, (IElementC)3 };
    Vector* v = VECTOR_Create(0, NULL, NULL);
    Iterator* it;
    int i;

    /* Nothing to filter if the target is empty */
    it = ITR_Filter(ITR_Empty(), NULL, NULL, NULL);
    if (ITR_HasNext(it)) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);

    /* Unfiltered proxy for a read-only iterator */
    it = ITR_Filter(ITR_Array(a, COUNT(a)), NULL, NULL, NULL);
    for (i=0; i<COUNT(a); i++) {
        if (!ITR_HasNext(it) ||
            ITR_Next(it) != a[i] ||
            ITR_Remove(it)) {
            ret = TEST_ERR;
        }
    }
    ITR_Delete(it);

    /* Unfiltered proxy to a read-write iterator */
    VECTOR_Add(v, (VElement)1);
    VECTOR_Add(v, (VElement)2);
    it = ITR_Filter(VECTOR_Iterator(v), NULL, NULL, NULL);
    for (i=0; i<2; i++) {
        if (!ITR_HasNext(it) ||
            ITR_Next(it) != ((IElementC)(PtrWord)(i+1)) ||
            !ITR_Remove(it)) {
            ret = TEST_ERR;
        }
    }
    if (ITR_HasNext(it) ||
        !VECTOR_IsEmpty(v)) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);

    /* Filter that doesn't allow remove */
    VECTOR_Add(v, (VElement)1);
    VECTOR_Add(v, (VElement)2);
    it = ITR_Filter(VECTOR_Iterator(v), NULL, test_itr_noremove, NULL);
    for (i=0; i<2; i++) {
        if (!ITR_HasNext(it) ||
            ITR_Next(it) != ((IElementC)(PtrWord)(i+1)) ||
            ITR_Remove(it)) {
            ret = TEST_ERR;
        }
    }
    if (ITR_HasNext(it) ||
        VECTOR_Size(v) != 2) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);

    /* Filter that changes the values (subtracts 1) */
    it = ITR_Filter(VECTOR_Iterator(v), test_itr_minus1, NULL, NULL);
    for (i=0; i<2; i++) {
        if (!ITR_HasNext(it) ||
            ITR_Next(it) != ((IElementC)(PtrWord)i) ||
            !ITR_Remove(it)) {
            ret = TEST_ERR;
        }
    }
    if (ITR_HasNext(it) ||
        !VECTOR_IsEmpty(v)) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);
 
    VECTOR_Delete(v);
    return ret;
}

static
TestStatus
test_itr_itrs(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Iterator* it = ITR_Singleton((IElementC)42);
    if (!ITR_HasNext(it) ||
        ITR_Next(it) != (IElementC)42 ||
        ITR_Remove(it) ||
        ITR_HasNext(it)) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);
    return ret;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_itr_alloc},
        {"Array", test_itr_itra},
        {"Combine", test_itr_itrc},
        {"Filter", test_itr_itrf},
        {"Singleton", test_itr_itrs}
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
