/*
 * $Id: test_hash.c,v 1.4 2019/01/16 23:57:53 slava Exp $
 *
 * Copyright (C) 2016-2019 by Slava Monich
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
    HashTable* ht1;
    HashTable* ht2;
    short primeIndex;

    /* Test NULL resistance */
    HASH_Delete(NULL);
    TEST_ASSERT(!HASH_Init(NULL, 0, NULL, NULL, NULL));
    TEST_ASSERT(HASH_Examine(NULL, NULL, NULL));

    /* HASH_Init makes sure HASH_InitModule() is called. Since
     * HASH_InitModule has already been called twice at this point,
     * we can call HASH_Shutdown() twice. */
    HASH_Shutdown();
    HASH_Shutdown();
    ht1 = HASH_Create(0, NULL, NULL, NULL);
    ht2 = HASH_Create(0, NULL, NULL, NULL);
    HASH_InitModule(); /* Now HASH_InitModule() is called twice again */

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!HASH_Create(0, NULL, NULL, NULL));

    testMem.failAt = testMem.allocCount + 1;
    TEST_ASSERT(!HASH_Create(1, NULL, NULL, NULL));

    testMem.failAt = testMem.allocCount;
    testMem.failCount = 2;
    TEST_ASSERT(!HASH_Put(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1)));

    testMem.failCount = 1;
    /*
     * On Windows we don't have hash bucket pool for the threads
     * not created with THREAD_Create. Therefore we will never
     * try to allocate HashBuckets and there won't be 2 memory
     * allocation attempts in a row. Just skip this part on Windows.
     */
#ifndef _WIN32
    testMem.failAt = testMem.allocCount + 1;
    TEST_ASSERT(!HASH_Put(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1)));
#endif

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!HASH_TryPut(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1)));

    HASH_Put(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1));
    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!HASH_Keys(ht1));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!HASH_Values(ht1));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!HASH_Entries(ht1));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!HASH_ConstKeys(ht1));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!HASH_ConstValues(ht1));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!HASH_ConstEntries(ht1));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!HASH_Copy(ht2, ht1));

    testMem.failAt = testMem.allocCount;
    primeIndex = ht1->primeIndex;
    ht1->loadFactor = 100;
    HASH_Rehash(ht1, 11);
    TEST_ASSERT(ht1->primeIndex == primeIndex);

    testMem.failAt = -1;
    HASH_Delete(ht1);
    HASH_Delete(ht2);
    return TEST_OK;
}

static
TestStatus
test_hash_basic(
    const TestDesc* test)
{
    HashTable* ht1 = HASH_Create(0, NULL, NULL, NULL);
    HashTable* ht2 = HASH_Create(0, NULL, NULL, NULL);
    char* skey;
    short primeIndex;
    int i;

    TEST_ASSERT(HASH_Examine(ht1, NULL, NULL));
    HASH_Reinit(ht1, NULL, NULL, NULL);
    HASH_Destroy(ht1);
    TEST_ASSERT(HASH_Init(ht1, 0, hashDefaultCompare, hashDefaultHashProc,
        hashFreeNothingProc));

    HASH_Reinit(ht1, hashDefaultCompare, hashDefaultHashProc,
        hashFreeNothingProc);

    /* There's nothing there yet */
    TEST_ASSERT(!HASH_Get(ht1, HASH_INT_KEY(1)));
    TEST_ASSERT(!HASH_Contains(ht1, HASH_INT_KEY(1)));
    TEST_ASSERT(!HASH_Update(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1)));

    /* Put and update */
    TEST_ASSERT(HASH_TryPut(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(1)));
    TEST_ASSERT(HASH_Get(ht1, HASH_INT_KEY(1)) == HASH_INT_VALUE(1));
    TEST_ASSERT(HASH_Contains(ht1, HASH_INT_KEY(1)));
    TEST_ASSERT(HASH_Size(ht1) == 1);
    TEST_ASSERT(HASH_Put(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(2)));
    TEST_ASSERT(HASH_Get(ht1, HASH_INT_KEY(1)) == HASH_INT_VALUE(2));
    TEST_ASSERT(HASH_Contains(ht1, HASH_INT_KEY(1)));
    TEST_ASSERT(HASH_Update(ht1, HASH_INT_KEY(1), HASH_INT_VALUE(3)));
    TEST_ASSERT(HASH_Get(ht1, HASH_INT_KEY(1)) == HASH_INT_VALUE(3));
    TEST_ASSERT(HASH_Contains(ht1, HASH_INT_KEY(1)));

    TEST_ASSERT(!HASH_Update(ht1, HASH_INT_KEY(12), HASH_INT_VALUE(2)));
    TEST_ASSERT(!HASH_Contains(ht1, HASH_INT_KEY(12)));
    TEST_ASSERT(!HASH_Get(ht1, HASH_INT_KEY(12)));

    TEST_ASSERT(HASH_Put(ht1, HASH_INT_KEY(12), HASH_INT_VALUE(2)));
    TEST_ASSERT(HASH_Put(ht1, HASH_INT_KEY(12), HASH_INT_VALUE(2)));
    TEST_ASSERT(HASH_Contains(ht1, HASH_INT_KEY(12)));
    TEST_ASSERT(HASH_Get(ht1, HASH_INT_KEY(12)));

    /* Copy */
    TEST_ASSERT(HASH_Copy(ht2, ht1));
    TEST_ASSERT(HASH_Size(ht2) == HASH_Size(ht1));
    TEST_ASSERT(HASH_Get(ht2, HASH_INT_KEY(1)) == HASH_INT_VALUE(3));

    /* Remove */
    TEST_ASSERT(HASH_Remove(ht1, HASH_INT_KEY(1)));
    TEST_ASSERT(!HASH_Remove(ht1, HASH_INT_KEY(1)));
    TEST_ASSERT(HASH_Remove(ht1, HASH_INT_KEY(12)));
    TEST_ASSERT(!HASH_Remove(ht1, HASH_INT_KEY(12)));
    TEST_ASSERT(HASH_IsEmpty(ht1));

    /* 11 is the first prime */
    primeIndex = ht1->primeIndex;
    for (i = 0; i < 11; i++) {
        TEST_ASSERT(HASH_Put(ht1, HASH_INT_KEY(i), HASH_INT_VALUE(i)));
    }
    /* Rehash should have happened automatically */
    TEST_ASSERT(ht1->primeIndex != primeIndex);
    HASH_Clear(ht1);
    TEST_ASSERT(HASH_IsEmpty(ht1));

    /* Strings */
    HASH_Reinit(ht1, hashCompareStringKey, stringHashProc,
        hashFreeKeyProc);

    TEST_ASSERT(HASH_Put(ht1, skey = STRING_Dup("One"), HASH_INT_VALUE(1)));
    TEST_ASSERT(HASH_Contains(ht1, skey));
    TEST_ASSERT(HASH_Contains(ht1, "One"));
    TEST_ASSERT(!HASH_Contains(ht1, "ONE"));
    TEST_ASSERT(!HASH_Contains(ht1, "Two"));
    TEST_ASSERT(HASH_Put(ht1, skey = STRING_Dup("One"), HASH_INT_VALUE(2)));
    TEST_ASSERT(HASH_Contains(ht1, skey));
    TEST_ASSERT(HASH_Contains(ht1, "One"));
    TEST_ASSERT(!HASH_Contains(ht1, "Two"));
    TEST_ASSERT(!HASH_Remove(ht1, "ONE"));
    TEST_ASSERT(HASH_Remove(ht1, "One"));
    TEST_ASSERT(!HASH_Remove(ht1, "One"));
    TEST_ASSERT(!HASH_Contains(ht1, "One"));

    HASH_Reinit(ht1, hashCaseCompareStringKey, stringCaseHashProc,
        hashFreeKeyValueProc);

    TEST_ASSERT(HASH_Put(ht1, skey = STRING_Dup("One"), STRING_Dup("A")));
    TEST_ASSERT(HASH_Contains(ht1, skey));
    TEST_ASSERT(HASH_Contains(ht1, "One"));
    TEST_ASSERT(HASH_Contains(ht1, "ONE"));
    TEST_ASSERT(!HASH_Contains(ht1, "Two"));
    TEST_ASSERT(HASH_Put(ht1, skey = STRING_Dup("ONE"), STRING_Dup("B")));
    TEST_ASSERT(HASH_Contains(ht1, skey));
    TEST_ASSERT(HASH_Contains(ht1, "One"));
    TEST_ASSERT(HASH_Contains(ht1, "ONE"));
    TEST_ASSERT(!HASH_Contains(ht1, "Two"));
    TEST_ASSERT(!HASH_Remove(ht1, "Two"));
    TEST_ASSERT(HASH_Remove(ht1, "One"));
    TEST_ASSERT(!HASH_Remove(ht1, "One"));
    TEST_ASSERT(!HASH_Remove(ht1, "ONE"));
    TEST_ASSERT(!HASH_Contains(ht1, "One"));

    HASH_Reinit(ht1, hashDefaultCompare, hashDefaultHashProc,
        hashFreeValueProc);
    TEST_ASSERT(HASH_Put(ht1, HASH_INT_KEY(1), STRING_Dup("A")));
    TEST_ASSERT(HASH_Put(ht1, HASH_INT_KEY(1), STRING_Dup("B")));
    HASH_Clear(ht1);

    HASH_Delete(ht1);
    HASH_Delete(ht2);
    return TEST_OK;
}

static
TestStatus
test_hash_iterator(
    const TestDesc* test)
{
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
  
    TEST_ASSERT(!ITR_HasNext(keys));
    TEST_ASSERT(!ITR_HasNext(values));
    TEST_ASSERT(!ITR_HasNext(entries));
    TEST_ASSERT(HASH_IsEmpty(ht2));

    ITR_Delete(keys);
    ITR_Delete(values);
    ITR_Delete(entries);

    /* Fill the table (11 is the first prime index) */
    for (i = 0; i < n; i++) {
        HASH_Put(ht1, HASH_INT_KEY(key_off+11*i), HASH_INT_VALUE(value_off+i));
    }

    TEST_ASSERT(HASH_Copy(ht2, ht1));

    /* Read-only iterators */
    keys = HASH_ConstKeys(ht1);
    values = HASH_ConstValues(ht1);
    entries = HASH_ConstEntries(ht1);
    for (i = 0; i < n; i++) {
        HashKey key = NULL;
        HashValue value = NULL;
        
        TEST_ASSERT(ITR_HasNext(keys));
        TEST_ASSERT(ITR_HasNext(values));
        TEST_ASSERT(ITR_HasNext(entries));
 
        TEST_ASSERT((key = ITR_Next(keys)) != NULL);
        TEST_ASSERT((value = ITR_Next(values)) != NULL);
        TEST_ASSERT((entry = ITR_Next(entries)) != NULL);

        TEST_ASSERT(entry->key == key);
        TEST_ASSERT(entry->value == value);

        TEST_ASSERT(HASH_Get(ht2, key) == value);
        TEST_ASSERT(HASH_Remove(ht2, key));

        TEST_ASSERT(!ITR_Remove(keys));
        TEST_ASSERT(!ITR_Remove(values));
        TEST_ASSERT(!ITR_Remove(entries));
    }

    TEST_ASSERT(!ITR_HasNext(keys));
    TEST_ASSERT(!ITR_HasNext(values));
    TEST_ASSERT(!ITR_HasNext(entries));
    TEST_ASSERT(HASH_IsEmpty(ht2));

    ITR_Delete(keys);
    ITR_Delete(values);
    ITR_Delete(entries);

    /* Iterators with remove */
    TEST_ASSERT(HASH_Copy(ht2, ht1));
    keys = HASH_Keys(ht1);
    TEST_ASSERT(keys);
    for (i = 0; i < n; i++) {
        HashKey key;

        TEST_ASSERT(ITR_HasNext(keys));
        key = ITR_Next(keys);
        TEST_ASSERT(key);
        TEST_ASSERT(HASH_Remove(ht2, key));
        TEST_ASSERT(ITR_Remove(keys));
        TEST_ASSERT(!ITR_Remove(keys));
    }

    TEST_ASSERT(!ITR_HasNext(keys));
    TEST_ASSERT(HASH_IsEmpty(ht1));
    TEST_ASSERT(HASH_IsEmpty(ht2));
    ITR_Delete(keys);

    for (i = 0; i < n; i++) {
        HASH_Put(ht1, HASH_INT_KEY(key_off+i), HASH_INT_VALUE(value_off+i));
        HASH_Put(ht2, HASH_INT_KEY(value_off+i), HASH_INT_VALUE(key_off+i));
    }

    values = HASH_Values(ht1);
    TEST_ASSERT(values);
    for (i = 0; i < n; i++) {
        HashValue value;

        TEST_ASSERT(ITR_HasNext(values));
        value = ITR_Next(values);
        TEST_ASSERT(value);
        TEST_ASSERT(HASH_Remove(ht2, value));
        TEST_ASSERT(ITR_Remove(values));
        TEST_ASSERT(!ITR_Remove(values));
    }

    TEST_ASSERT(!ITR_HasNext(values));
    TEST_ASSERT(HASH_IsEmpty(ht1));
    TEST_ASSERT(HASH_IsEmpty(ht2));
    ITR_Delete(values);

    for (i = 0; i < n; i++) {
        HASH_Put(ht1, HASH_INT_KEY(key_off+11*i), HASH_INT_VALUE(value_off+i));
    }

    TEST_ASSERT(HASH_Copy(ht2, ht1));
    entries = HASH_Entries(ht1);
    TEST_ASSERT(entries);
    for (i = 0; i < n; i++) {
        TEST_ASSERT(ITR_HasNext(entries));
        entry = ITR_Next(entries);
        TEST_ASSERT(entry);
        TEST_ASSERT(HASH_Get(ht2, entry->key) == entry->value);
        TEST_ASSERT(HASH_Remove(ht2, entry->key));

        /* Remove every other entry */
        if (!(i % 2)) {
            TEST_ASSERT(ITR_Remove(entries));
            TEST_ASSERT(!ITR_Remove(entries));
        }
    }

    TEST_ASSERT(!ITR_HasNext(entries));
    TEST_ASSERT(HASH_Size(ht1) == n/2);
    TEST_ASSERT(HASH_IsEmpty(ht2));
    ITR_Delete(entries);
    HASH_Clear(ht1);

    /* Concurrent modification */
    for (i = 0; i < n; i++) {
        TEST_ASSERT(HASH_Put(ht1, HASH_INT_KEY(11*i), HASH_INT_VALUE(i)));
    }
    entries = HASH_Entries(ht1);
    TEST_ASSERT(entries);
    TEST_ASSERT(ITR_HasNext(entries));
    TEST_ASSERT(ITR_Next(entries));
    HASH_Clear(ht1);
    /* HASH_ItrRemove should detect concurrent modification and fail */
    TEST_ASSERT(!ITR_Remove(entries));
    TEST_ASSERT(!ITR_HasNext(entries));
    ITR_Delete(entries);

    /* Again, only removing all but one element */
    for (i = 0; i < n; i++) {
        TEST_ASSERT(HASH_Put(ht1, HASH_INT_KEY(11*i), HASH_INT_VALUE(i)));
    }
    entries = HASH_Entries(ht1);
    keys = HASH_Keys(ht1);
    TEST_ASSERT(entries);
    TEST_ASSERT(keys);
    TEST_ASSERT(ITR_Next(keys));
    TEST_ASSERT(ITR_Next(entries));
    TEST_ASSERT(ITR_Next(entries));
    while (ITR_Next(keys)) {
        TEST_ASSERT(ITR_Remove(keys));
    }
    TEST_ASSERT(!ITR_Remove(entries));
    TEST_ASSERT(!ITR_HasNext(entries));
    ITR_Delete(entries);
    ITR_Delete(keys);

    HASH_Delete(ht1);
    HASH_Delete(ht2);
    return TEST_OK;
}

static
void
test_hash_pool_thread(
    void* arg)
{
    HashTable* ht = HASH_Create(0, NULL, NULL, NULL);
    int i;

    /* Fill the thread local pool */
    for (i = 0; i < 150; i++) {
        TEST_ASSERT(HASH_Put(ht, HASH_INT_KEY(i), HASH_INT_VALUE(i)));
    }
    HASH_Clear(ht);
    for (i = 0; i < 10; i++) {
        TEST_ASSERT(HASH_Put(ht, HASH_INT_KEY(i), HASH_INT_VALUE(i)));
    }
    HASH_Delete(ht);
}

static
TestStatus
test_hash_pool(
    const TestDesc* test)
{
    ThrID tid = THR_NONE;

    TEST_ASSERT(THREAD_Create(&tid, test_hash_pool_thread, NULL));
    TEST_ASSERT(tid != THR_NONE);
    TEST_ASSERT(THREAD_Join(tid));
    return TEST_OK;
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
