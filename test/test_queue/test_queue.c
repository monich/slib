/*
 * $Id: test_queue.c,v 1.1 2016/09/26 16:09:20 slava Exp $
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
test_queue_remove_entry(
    QEntry* e,
    void* param)
{
    QUEUE_RemoveEntry(e);
    return False;
}

static
Bool
test_queue_remove_entries(
    QEntry* e,
    void* param)
{
    int* count = param;
    QUEUE_RemoveEntry(e);
    (*count)++;
    return True;
}

static
Bool
test_queue_remove_entry2(
    QEntry* e,
    void* param1,
    void* param2)
{
    QUEUE_RemoveEntry(e);
    return False;
}

static
Bool
test_queue_remove_entries2(
    QEntry* e,
    void* param1,
    void* param2)
{
    int* count = param1;
    QUEUE_RemoveEntry(e);
    (*count)++;
    return True;
}

static
TestStatus
test_queue_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Queue q;
    QEntry e;

    /* Test NULL resistance */
    if (QUEUE_Find(NULL, NULL) >= 0 ||
        QUEUE_Index(NULL) >= 0 ||
        QUEUE_Move(NULL, NULL) ||
        QUEUE_Get(NULL, 0)) {
        ret = TEST_ERR;
    }

    /* Simulate allocation failures */
    QUEUE_Init(&q);
    QUEUE_InsertTail(&q, &e);
    testMem.failAt = testMem.allocCount;
    if (QUEUE_ConstIterator(&q)) {
        ret = TEST_ERR;
    }
    testMem.failAt = testMem.allocCount;
    if (QUEUE_Iterator(&q)) {
        ret = TEST_ERR;
    }

    testMem.failAt = -1;
    return ret;
}

static
TestStatus
test_queue_basic(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Queue q, q1;
    QEntry e1, e2, e3, e4;

    QUEUE_Init(&q);
    QUEUE_Init(&q1);

    QUEUE_InsertHead(&q, &e1);
    QUEUE_InsertHead(&q, &e2);
    if (QUEUE_Size(&q) != 2 ||
        QUEUE_RemoveHead(&q) != &e2 ||
        QUEUE_RemoveHead(&q) != &e1 ||
        QUEUE_RemoveHead(&q)) {
        ret = TEST_ERR;
    }

    QUEUE_InsertTail(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    if (QUEUE_Size(&q) != 2 ||
        QUEUE_RemoveHead(&q) != &e1 ||
        QUEUE_RemoveHead(&q) != &e2 ||
        QUEUE_RemoveHead(&q)) {
        ret = TEST_ERR;
    }

    QUEUE_InsertTail(&q, &e1);
    if (QUEUE_InsertBefore(&e1, &e2) != 2 ||
        QUEUE_RemoveTail(&q) != &e1 ||
        QUEUE_RemoveTail(&q) != &e2 ||
        QUEUE_RemoveTail(&q)) {
        ret = TEST_ERR;
    }

    QUEUE_InsertTail(&q, &e1);
    if (QUEUE_InsertAfter(&e1, &e2) != 2 ||
        QUEUE_RemoveHead(&q) != &e1 ||
        QUEUE_RemoveHead(&q) != &e2 ||
        QUEUE_RemoveHead(&q)) {
        ret = TEST_ERR;
    }

    QUEUE_InsertTail(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    if (QUEUE_First(&q) != &e1 ||
        QUEUE_Last(&q) != &e2 ||
        QUEUE_Clear(&q) != 2 ||
        QUEUE_Clear(&q) != 0 ||
        QUEUE_Size(&q) != 0 ||
        !QUEUE_IsEmpty(&q) ||
        QUEUE_First(&q) ||
        QUEUE_Last(&q)) {
        ret = TEST_ERR;
    }

    QUEUE_InsertTail(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    if (!QUEUE_RemoveEntry(&e1) ||
        QUEUE_RemoveEntry(&e1) ||
        !QUEUE_RemoveEntry(&e2) ||
        QUEUE_RemoveEntry(&e2) ||
        QUEUE_Size(&q) != 0 ||
        !QUEUE_IsEmpty(&q)) {
        ret = TEST_ERR;
    }

    QUEUE_InsertHead(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    if (QUEUE_Next(&e1) != &e2 ||
        QUEUE_Prev(&e2) != &e1 ||
        QUEUE_Next(&e2) ||
        QUEUE_Prev(&e1)) {
        ret = TEST_ERR;
    }
    if (!QUEUE_RemoveEntry(&e1) ||
        QUEUE_Prev(&e1) ||
        QUEUE_Next(&e1) ||
        QUEUE_Prev(&e2) ||
        QUEUE_Next(&e2)) {
        ret = TEST_ERR;
    }
    if (!QUEUE_RemoveEntry(&e2) ||
        QUEUE_Prev(&e1) ||
        QUEUE_Next(&e1) ||
        QUEUE_Prev(&e2) ||
        QUEUE_Next(&e2) ||
        QUEUE_Size(&q) != 0 ||
        !QUEUE_IsEmpty(&q)) {
        ret = TEST_ERR;
    }

    QUEUE_InsertHead(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    QUEUE_InsertTail(&q, &e3);
    QUEUE_InsertTail(&q, &e4);
    if (QUEUE_Index(&e1) != 0 ||
        QUEUE_Index(&e2) != 1 ||
        QUEUE_Index(&e3) != 2 ||
        QUEUE_Index(&e4) != 3) {
        ret = TEST_ERR;
    }
    if (QUEUE_Find(&q, &e1) != 0 ||
        QUEUE_Find(&q, &e2) != 1 ||
        QUEUE_Find(&q, &e3) != 2 ||
        QUEUE_Find(&q, &e4) != 3) {
        ret = TEST_ERR;
    }
    if (QUEUE_Get(&q, 0) != &e1 ||
        QUEUE_Get(&q, 1) != &e2 ||
        QUEUE_Get(&q, 2) != &e3 ||
        QUEUE_Get(&q, 3) != &e4) {
        ret = TEST_ERR;
    }
    if (QUEUE_Clear(&q) != 4 ||
        QUEUE_Index(&e1) >= 0 ||
        QUEUE_Index(&e2) >= 0 ||
        QUEUE_Index(&e3) >= 0 ||
        QUEUE_Index(&e4) >= 0) {
        ret = TEST_ERR;
    }
    if (QUEUE_Find(&q, &e1) >= 0 ||
        QUEUE_Find(&q, &e2) >= 0 ||
        QUEUE_Find(&q, &e3) >= 0 ||
        QUEUE_Find(&q, &e4) >= 0) {
        ret = TEST_ERR;
    }

    if (QUEUE_Get(&q, -1) ||
        QUEUE_Get(&q, 0)) {
        ret = TEST_ERR;
    }

    QUEUE_InsertHead(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    if (QUEUE_Move(&q1, &q) != 2 ||
        QUEUE_Move(&q1, &q) != 0 ||
        QUEUE_Size(&q) != 0 ||
        QUEUE_Size(&q1) != 2 ||
        !QUEUE_IsEmpty(&q) ||
        QUEUE_IsEmpty(&q1) ||
        QUEUE_Clear(&q1) != 2) {
        ret = TEST_ERR;
    }

    return ret;
}

static
TestStatus
test_queue_move(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Queue q;
    QEntry e1, e2, e3;

    QUEUE_Init(&q);
    QUEUE_InsertTail(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    QUEUE_InsertTail(&q, &e3);
    if (QUEUE_MoveToTail(&e1, 3) != 2 ||
        QUEUE_MoveToTail(&e1, 1) != 0 ||
        QUEUE_First(&q) != &e2 ||
        QUEUE_Last(&q) != &e1) {
        ret = TEST_ERR;
    }

    if (QUEUE_MoveToHead(&e1, 3) != 2 ||
        QUEUE_First(&q) != &e1 ||
        QUEUE_Last(&q) != &e3) {
        ret = TEST_ERR;
    }

    if (QUEUE_MoveToTail(&e1, 1) != 1 ||
        QUEUE_First(&q) != &e2 ||
        QUEUE_Last(&q) != &e3) {
        ret = TEST_ERR;
    }

    if (QUEUE_MoveToHead(&e1, 1) != 1 ||
        QUEUE_MoveToHead(&e1, 1) != 0 ||
        QUEUE_First(&q) != &e1 ||
        QUEUE_Last(&q) != &e3) {
        ret = TEST_ERR;
    }

    if (QUEUE_Clear(&q) != 3 ||
        QUEUE_MoveToHead(&e1, 1) != 0 ||
        QUEUE_MoveToTail(&e1, 1) != 0) {
        ret = TEST_ERR;
    }

    return ret;
}

static
TestStatus
test_queue_examine(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Queue q;
    QEntry e1, e2, e3;
    int count;

    QUEUE_Init(&q);
    QUEUE_InsertTail(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    QUEUE_InsertTail(&q, &e3);
    if (QUEUE_Examine(&q, test_queue_remove_entry, NULL) ||
        QUEUE_Size(&q) != 2 ||
        QUEUE_RemoveEntry(&e1)) {
        ret = TEST_ERR;
    }
    if (QUEUE_ExamineBack(&q, test_queue_remove_entry, NULL) ||
        QUEUE_Size(&q) != 1 ||
        QUEUE_RemoveEntry(&e3)) {
        ret = TEST_ERR;
    }

    QUEUE_InsertHead(&q, &e1);
    QUEUE_InsertTail(&q, &e3);
    if (QUEUE_Examine2(&q, test_queue_remove_entry2, NULL, NULL) ||
        QUEUE_Size(&q) != 2 ||
        QUEUE_RemoveEntry(&e1)) {
        ret = TEST_ERR;
    }
    if (QUEUE_ExamineBack2(&q, test_queue_remove_entry2, NULL, NULL) ||
        QUEUE_Size(&q) != 1 ||
        QUEUE_RemoveEntry(&e3)) {
        ret = TEST_ERR;
    }

    count = 0;
    QUEUE_InsertHead(&q, &e1);
    QUEUE_InsertTail(&q, &e3);
    if (!QUEUE_Examine(&q, test_queue_remove_entries, &count) ||
        count != 3 ||
        !QUEUE_IsEmpty(&q)) {
        ret = TEST_ERR;
    }

    count = 0;
    QUEUE_InsertTail(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    QUEUE_InsertTail(&q, &e3);
    if (!QUEUE_ExamineBack(&q, test_queue_remove_entries, &count) ||
        count != 3 ||
        !QUEUE_IsEmpty(&q)) {
        ret = TEST_ERR;
    }

    count = 0;
    QUEUE_InsertTail(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    QUEUE_InsertTail(&q, &e3);
    if (!QUEUE_Examine2(&q, test_queue_remove_entries2, &count, NULL) ||
        count != 3 ||
        !QUEUE_IsEmpty(&q)) {
        ret = TEST_ERR;
    }

    count = 0;
    QUEUE_InsertTail(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    QUEUE_InsertTail(&q, &e3);
    if (!QUEUE_ExamineBack2(&q, test_queue_remove_entries2, &count, NULL) ||
        count != 3 ||
        !QUEUE_IsEmpty(&q)) {
        ret = TEST_ERR;
    }

    return ret;
}

static
TestStatus
test_queue_iterator(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Queue q;
    QEntry e1, e2;
    Iterator* it;

    QUEUE_Init(&q);

    it = QUEUE_ConstIterator(&q);
    if (ITR_HasNext(it)) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);

    it = QUEUE_Iterator(&q);
    if (ITR_HasNext(it)) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);

    QUEUE_InsertTail(&q, &e1);
    QUEUE_InsertTail(&q, &e2);
    it = QUEUE_ConstIterator(&q);
    if (ITR_Next(it) != &e1 ||
        ITR_Remove(it) ||
        ITR_Next(it) != &e2 ||
        ITR_Remove(it) ||
        ITR_Next(it) ||
        QUEUE_Size(&q) != 2) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);

    it = QUEUE_Iterator(&q);
    if (ITR_Next(it) != &e1 ||
        !ITR_Remove(it) ||
        QUEUE_RemoveEntry(&e1) ||
        ITR_Next(it) != &e2 ||
        !ITR_Remove(it) ||
        QUEUE_RemoveEntry(&e2) ||
        ITR_Next(it) ||
        !QUEUE_IsEmpty(&q)) {
        ret = TEST_ERR;
    }
    ITR_Delete(it);

    return ret;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_queue_alloc},
        {"Basic", test_queue_basic},
        {"Move", test_queue_move},
        {"Examine", test_queue_examine},
        {"Iterator", test_queue_iterator}
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
