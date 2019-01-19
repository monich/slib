/*
 * $Id: test_fnull.c,v 1.1 2019/01/19 11:51:35 slava Exp $
 *
 * Copyright (C) 2019 by Slava Monich
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
test_fnull_alloc(
    const TestDesc* test)
{
    int i;

    /* Test NULL resistance */
    TEST_ASSERT(!FILE_IsNull(NULL));

    /* Simulate allocation failures */
    for (i = 0; i < 2; i++) {
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!FILE_Null());
    }

    return TEST_OK;
}

static
TestStatus
test_fnull_basic(
    const TestDesc* test)
{
    File* f = FILE_Null();
    File* fmem = FILE_Mem();

    TEST_ASSERT(FILE_IsNull(f));
    TEST_ASSERT(!FILE_IsNull(fmem));
    TEST_ASSERT(!FILE_IsFileIO(f));
    TEST_ASSERT(FILE_Fd(f) < 1);
    TEST_ASSERT(FILE_TargetFd(f) < 1);
    TEST_ASSERT(!FILE_CanBlock(f));
    TEST_ASSERT(FILE_Eof(f));
    TEST_ASSERT(FILE_Flush(f));

    TEST_ASSERT(FILE_Reopen(f, "test", ""));
    TEST_ASSERT(FILE_IsNull(f));
    TEST_ASSERT(FILE_Eof(f));

    FILE_Close(fmem);
    FILE_Close(f);
    return TEST_OK;
}

static
TestStatus
test_fnull_read(
    const TestDesc* test)
{
    File* f = FILE_Null();
    int buf;

    TEST_ASSERT(FILE_IsNull(f));
    TEST_ASSERT(FILE_Read(f, &buf, sizeof(buf)) == 0);
    TEST_ASSERT(FILE_Read(f, &buf, sizeof(buf)) < 0);
    TEST_ASSERT(FILE_Eof(f));
    TEST_ASSERT(!FILE_BytesRead(f));

    FILE_Close(f);
    return TEST_OK;
}

static
TestStatus
test_fnull_write(
    const TestDesc* test)
{
    File* f = FILE_Null();
    int data = 0;

    TEST_ASSERT(FILE_Write(f, &data, sizeof(data)) == sizeof(data));
    TEST_ASSERT(FILE_BytesWritten(f) == sizeof(data));
    FILE_Close(f);
    return TEST_OK;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_fnull_alloc},
        {"Basic", test_fnull_basic},
        {"Read", test_fnull_read},
        {"Write", test_fnull_write}
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
