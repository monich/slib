/*
 * $Id: test_hash.c,v 1.3 2017/10/28 22:27:19 slava Exp $
 *
 * Copyright (C) 2016-2017 by Slava Monich
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
test_hash_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    HashTable* ht1;
    HashTable* ht2;
    short primeIndex;

    /* Test NULL resistance */
    HASH_Delete(NULL);
    if (HASH_Init(NULL, 0, NULL, NULL, NULL) ||
        !HASH_Examine(NULL, NULL, NULL)) {
        ret = TEST_ERR;
    }

    /* HASH_Init makes sure HASH_InitModule() is called. Since
     * HASH_InitModule has already been called twice at this point,
     * we call HASH_Shutdown() twice. */
    HASH_Shutdown();
    HASH_Shutdown();
    ht1 = HASH_Create(0, NULL, NULL, NULL);
    ht2 = HASH_Create(0, NULL, NULL, NULL);
    HASH_InitModule();

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    if (HASH_Create(0, NULL, NULL, NULL)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount + 1;
    if (HASH_Create(1, NULL, NULL, NULL)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    testMem.failCount = 2;
    if (HASH_Put(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1))) {
        ret = TEST_ERR;
    }

    testMem.failCount = 1;
    /*
     * On Windows we don't have hash bucket pool for the threads
     * not created with THREAD_Create. Therefore we will never
     * try to allocate HashBuckets and there won't be 2 memory
     * allocation attempts in a row. Just skip this part on Windows.
     */
#ifndef _WIN32
    testMem.failAt = testMem.allocCount + 1;
    if (HASH_Put(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1))) {
        ret = TEST_ERR;
    }
#endif

    testMem.failAt = testMem.allocCount;
    if (HASH_TryPut(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1))) {
        ret = TEST_ERR;
    }

    HASH_Put(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1));
    testMem.failAt = testMem.allocCount;
    if (HASH_Keys(ht1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (HASH_Values(ht1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (HASH_Entries(ht1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (HASH_ConstKeys(ht1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (HASH_ConstValues(ht1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (HASH_ConstEntries(ht1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (HASH_Copy(ht2, ht1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    primeIndex = ht1->primeIndex;
    ht1->loadFactor = 100;
    HASH_Rehash(ht1, 11);
    if (ht1->primeIndex != primeIndex) {
        ret = TEST_ERR;
    }

    testMem.failAt = -1;
    HASH_Delete(ht1);
    HASH_Delete(ht2);
    return ret;
}

static
TestStatus
test_hash_basic(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    HashTable* ht1 = HASH_Create(0, NULL, NULL, NULL);
    HashTable* ht2 = HASH_Create(0, NULL, NULL, NULL);
    char* skey;
    short primeIndex;
    int i;

    if (!HASH_Examine(ht1, NULL, NULL)) {
        ret = TEST_ERR;
    }

    HASH_Reinit(ht1, NULL, NULL, NULL);
    HASH_Destroy(ht1);
    if (!HASH_Init(ht1, 0, hashDefaultCompare, hashDefaultHashProc,
        hashFreeNothingProc)) {
        ret = TEST_ERR;
    }

    HASH_Reinit(ht1, hashDefaultCompare, hashDefaultHashProc,
        hashFreeNothingProc);

    /* There's nothing there yet */
    if (HASH_Get(ht1, HASH_INT_KEY(1)) ||
        HASH_Contains(ht1, HASH_INT_KEY(1)) ||
        HASH_Update(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1))) {
        ret = TEST_ERR;
    }

    /* Put and update */
    if (!HASH_TryPut(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1)) ||
        HASH_Get(ht1, HASH_INT_KEY(1)) != HASH_INT_VALUE(1) ||
        !HASH_Contains(ht1, HASH_INT_KEY(1)) ||
        HASH_Size(ht1) != 1 ||
        !HASH_Put(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(2)) ||
        HASH_Get(ht1, HASH_INT_KEY(1)) != HASH_INT_VALUE(2) ||
        !HASH_Contains(ht1, HASH_INT_KEY(1)) ||
        !HASH_Update(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(3)) ||
        HASH_Get(ht1, HASH_INT_KEY(1)) != HASH_INT_VALUE(3) ||
        !HASH_Contains(ht1, HASH_INT_KEY(1))) {
        ret = TEST_ERR;
    }

    if (HASH_Update(ht1, HASH_INT_KEY(12), HASH_INT_VALUE(2)) ||
        HASH_Contains(ht1, HASH_INT_KEY(12)) ||
        HASH_Get(ht1, HASH_INT_KEY(12))) {
        ret = TEST_ERR;
    }

    if (!HASH_Put(ht1, HASH_INT_KEY(12), HASH_INT_VALUE(2)) ||
        !HASH_Put(ht1, HASH_INT_KEY(12), HASH_INT_VALUE(2)) ||
        !HASH_Contains(ht1, HASH_INT_KEY(12)) ||
        !HASH_Get(ht1, HASH_INT_KEY(12))) {
        ret = TEST_ERR;
    }

    /* Copy */
    if (!HASH_Copy(ht2, ht1) ||
        HASH_Size(ht2) != HASH_Size(ht1) ||
        HASH_Get(ht2, HASH_INT_KEY(1)) != HASH_INT_VALUE(3)) {
        ret = TEST_ERR;
    }

    /* Remove */
    if (!HASH_Remove(ht1, HASH_INT_KEY(1)) ||
        HASH_Remove(ht1, HASH_INT_KEY(1)) ||
        !HASH_Remove(ht1, HASH_INT_KEY(12)) ||
        HASH_Remove(ht1, HASH_INT_KEY(12)) ||
        !HASH_IsEmpty(ht1)) {
        ret = TEST_ERR;
    }

    /* 11 is the first prime */
    primeIndex = ht1->primeIndex;
    for (i=0; i<11; i++) {
        HASH_Put(ht1, HASH_INT_KEY(i), HASH_INT_VALUE(i));
    }
    /* Rehash should have happened automatically */
    if (ht1->primeIndex == primeIndex) {
        ret = TEST_ERR;
    }
    HASH_Clear(ht1);
    if (!HASH_IsEmpty(ht1)) {
        ret = TEST_ERR;
    }

    /* Strings */
    HASH_Reinit(ht1, hashCompareStringKey, stringHashProc,
        hashFreeKeyProc);

    if (!HASH_Put(ht1, skey = STRING_Dup("One"), HASH_INT_VALUE(1)) ||
        !HASH_Contains(ht1, skey) ||
        !HASH_Contains(ht1, "One") ||
        HASH_Contains(ht1, "ONE") ||
        HASH_Contains(ht1, "Two") ||
        !HASH_Put(ht1, skey = STRING_Dup("One"), HASH_INT_VALUE(2)) ||
        !HASH_Contains(ht1, skey) ||
        !HASH_Contains(ht1, "One") ||
        HASH_Contains(ht1, "Two") ||
        HASH_Remove(ht1, "ONE") ||
        !HASH_Remove(ht1, "One") ||
        HASH_Remove(ht1, "One") ||
        HASH_Contains(ht1, "One")) {
        ret = TEST_ERR;
    }

    HASH_Reinit(ht1, hashCaseCompareStringKey, stringCaseHashProc,
        hashFreeKeyValueProc);

    if (!HASH_Put(ht1, skey = STRING_Dup("One"), STRING_Dup("A")) ||
        !HASH_Contains(ht1, skey) ||
        !HASH_Contains(ht1, "One") ||
        !HASH_Contains(ht1, "ONE") ||
        HASH_Contains(ht1, "Two") ||
        !HASH_Put(ht1, skey = STRING_Dup("ONE"), STRING_Dup("B")) ||
        !HASH_Contains(ht1, skey) ||
        !HASH_Contains(ht1, "One") ||
        !HASH_Contains(ht1, "ONE") ||
        HASH_Contains(ht1, "Two") ||
        HASH_Remove(ht1, "Two") ||
        !HASH_Remove(ht1, "One") ||
        HASH_Remove(ht1, "One") ||
        HASH_Remove(ht1, "ONE") ||
        HASH_Contains(ht1, "One")) {
        ret = TEST_ERR;
    }

    HASH_Reinit(ht1, hashDefaultCompare, hashDefaultHashProc,
        hashFreeValueProc);
    HASH_Put(ht1, HASH_INT_KEY(1), STRING_Dup("A"));
    HASH_Put(ht1, HASH_INT_KEY(1), STRING_Dup("B"));
    HASH_Clear(ht1);

    HASH_Delete(ht1);
    HASH_Delete(ht2);
    return ret;
}

static
TestStatus
test_hash_iterator(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    HashTable* ht1 = HASH_Create(0, NULL, NULL, NULL);
    HashTable* ht2 = HASH_Create(0, NULL, NULL, NULL);
    const HashEntry* entry = NULL;
    const int n = 6;
    const int key_off = -2;
    const int value_off = key_off + n;
    Iterator* keys;
    Iterator* values;
    Iterator* entries;
    int i;

    /* Empty iterators */
    keys = HASH_ConstKeys(ht1);
    values = HASH_ConstValues(ht1);
    entries = HASH_ConstEntries(ht1);
  
    if (ITR_HasNext(keys) ||
        ITR_HasNext(values) ||
        ITR_HasNext(entries) ||
        !HASH_IsEmpty(ht2)) {
        ret = TEST_ERR;
    }

    ITR_Delete(keys);
    ITR_Delete(values);
    ITR_Delete(entries);

    /* Fill the table (11 is the first prime index) */
    for (i=0; i<n; i++) {
        HASH_Put(ht1, HASH_INT_KEY(key_off+11*i), HASH_INT_VALUE(value_off+i));
    }

    if (!HASH_Copy(ht2, ht1)) {
        ret = TEST_ERR;
    }

    /* Read-only iterators */
    keys = HASH_ConstKeys(ht1);
    values = HASH_ConstValues(ht1);
    entries = HASH_ConstEntries(ht1);
    for (i=0; i<n; i++) {
        HashKey key = NULL;
        HashValue value = NULL;
        
        if (!ITR_HasNext(keys) ||
            !ITR_HasNext(values) ||
            !ITR_HasNext(entries)) {
            ret = TEST_ERR;
        }
 
        if (!(key = ITR_Next(keys)) ||
            !(value = ITR_Next(values)) ||
            !(entry = ITR_Next(entries))) {
            ret = TEST_ERR;
        }

        if (entry->key != key ||
            entry->value != value) {
            ret = TEST_ERR;
        }

        if (HASH_Get(ht2, key) != value ||
            !HASH_Remove(ht2, key)) {
            ret = TEST_ERR;
        }

        if (ITR_Remove(keys) ||
            ITR_Remove(values) ||
            ITR_Remove(entries)) {
            ret = TEST_ERR;
        }
    }

    if (ITR_HasNext(keys) ||
        ITR_HasNext(values) ||
        ITR_HasNext(entries) ||
        !HASH_IsEmpty(ht2)) {
        ret = TEST_ERR;
    }

    ITR_Delete(keys);
    ITR_Delete(values);
    ITR_Delete(entries);

    /* Iterators with remove */
    if (!HASH_Copy(ht2, ht1)) {
        ret = TEST_ERR;
    }

    keys = HASH_Keys(ht1);
    for (i=0; i<n; i++) {
        HashKey key;
        if (!ITR_HasNext(keys) ||
            !(key = ITR_Next(keys)) ||
            !HASH_Remove(ht2, key) ||
            !ITR_Remove(keys) ||
            ITR_Remove(keys)) {
            ret = TEST_ERR;
        }
    }

    if (ITR_HasNext(keys) ||
        !HASH_IsEmpty(ht1) ||
        !HASH_IsEmpty(ht2)) {
        ret = TEST_ERR;
    }

    ITR_Delete(keys);

    for (i=0; i<n; i++) {
        HASH_Put(ht1, HASH_INT_KEY(key_off+i), HASH_INT_VALUE(value_off+i));
        HASH_Put(ht2, HASH_INT_KEY(value_off+i), HASH_INT_VALUE(key_off+i));
    }

    values = HASH_Values(ht1);
    for (i=0; i<n; i++) {
        HashValue value;
        if (!ITR_HasNext(values) ||
            !(value = ITR_Next(values)) ||
            !HASH_Remove(ht2, value) ||
            !ITR_Remove(values) ||
            ITR_Remove(values)) {
            ret = TEST_ERR;
        }
    }

    if (ITR_HasNext(values) ||
        !HASH_IsEmpty(ht1) ||
        !HASH_IsEmpty(ht2)) {
        ret = TEST_ERR;
    }

    ITR_Delete(values);

    for (i=0; i<n; i++) {
        HASH_Put(ht1, HASH_INT_KEY(key_off+11*i), HASH_INT_VALUE(value_off+i));
    }

    if (!HASH_Copy(ht2, ht1)) {
        ret = TEST_ERR;
    }

    entries = HASH_Entries(ht1);
    for (i=0; i<n; i++) {
        if (!ITR_HasNext(entries) ||
            !(entry = ITR_Next(entries)) ||
            HASH_Get(ht2, entry->key) != entry->value ||
            !HASH_Remove(ht2, entry->key)) {
            ret = TEST_ERR;
        }
        /* Remove every other entry */
        if (!(i%2)) {
            if (!ITR_Remove(entries) ||
                ITR_Remove(entries)) {
                ret = TEST_ERR;
            }
        }
    }

    if (ITR_HasNext(entries) ||
        HASH_Size(ht1) != n/2 ||
        !HASH_IsEmpty(ht2)) {
        ret = TEST_ERR;
    }

    ITR_Delete(entries);
    HASH_Clear(ht1);

    /* Concurrent modification */
    for (i=0; i<n; i++) {
        HASH_Put(ht1, HASH_INT_KEY(11*i), HASH_INT_VALUE(i));
    }
    entries = HASH_Entries(ht1);
    if (!ITR_HasNext(entries) ||
        !(entry = ITR_Next(entries))) {
        ret = TEST_ERR;
    }
    HASH_Clear(ht1);
    /* HASH_ItrRemove should detect concurrent modification and fail */
    if (ITR_Remove(entries) ||
        ITR_HasNext(entries)) {
        ret = TEST_ERR;
    }
    ITR_Delete(entries);

    /* Again, only removing all but one element */
    for (i=0; i<n; i++) {
        HASH_Put(ht1, HASH_INT_KEY(11*i), HASH_INT_VALUE(i));
    }
    entries = HASH_Entries(ht1);
    keys = HASH_Keys(ht1);
    if (!ITR_Next(keys) ||
        !ITR_Next(entries) ||
        !(entry = ITR_Next(entries))) {
        ret = TEST_ERR;
    }
    while (ITR_Next(keys)) {
        if (!ITR_Remove(keys)) {
            ret = TEST_ERR;
        }
    }
    if (ITR_Remove(entries) ||
        ITR_HasNext(entries)) {
        ret = TEST_ERR;
    }
    ITR_Delete(entries);
    ITR_Delete(keys);

    HASH_Delete(ht1);
    HASH_Delete(ht2);
    return ret;
}

static
void
test_hash_pool_thread(
    void* arg)
{
    HashTable* ht = HASH_Create(0, NULL, NULL, NULL);
    int i;
    /* Fill the thread local pool */
    for (i=0; i<150; i++) {
        HASH_Put(ht, HASH_INT_KEY(i), HASH_INT_VALUE(i));
    }
    HASH_Clear(ht);
    for (i=0; i<10; i++) {
        HASH_Put(ht, HASH_INT_KEY(i), HASH_INT_VALUE(i));
    }
    HASH_Delete(ht);
}

static
TestStatus
test_hash_pool(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    ThrID tid = THR_NONE;
    if (!THREAD_Create(&tid, test_hash_pool_thread, &ret) ||
        !THREAD_Join(tid)) {
        ret = TEST_ERR;
    }
    return ret;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_hash_alloc},
        {"Basic", test_hash_basic},
        {"Iterator", test_hash_iterator},
        {"Pool", test_hash_pool}
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
