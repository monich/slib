/*
 * $Id: test_fmem.c,v 1.3 2018/12/27 23:44:41 slava Exp $
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
test_fmem_alloc(
    const TestDesc* test)
{
    static const I8u data[] = {1, 2, 3};
    I8u databuf[4];
    Buffer* buf = BUFFER_Create();
    File* f = FILE_Mem();
    int i;

    /* Test NULL resistance */
    FILE_MemClear(NULL);
    TEST_ASSERT(!FILE_IsMem(NULL));
    TEST_ASSERT(!FILE_MemSize(NULL));
    TEST_ASSERT(!FILE_MemData(NULL));

    /* Simulate allocation failures */
    for (i = 0; i < 3; i++) {
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!FILE_Mem());
    }

    for (i = 0; i < 3; i++) {
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!FILE_MemIn(data, sizeof(data)));
    }

    for (i = 0; i < 3; i++) {
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!FILE_MemOut(1));
    }

    for (i = 0; i < 3; i++) {
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!FILE_MemOut2(databuf, sizeof(databuf)));
    }

    for (i = 0; i < 2; i++) {
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!FILE_MemBuf(buf, False));
    }

    /* Once one write fails, the second one will fail too? */
    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(FILE_Write(f, data, sizeof(data)) < 0);
    TEST_ASSERT(FILE_Write(f, data, sizeof(data)) < 0);

    /* But after FILE_MemClear() it should succeed */
    FILE_MemClear(f);
    TEST_ASSERT(FILE_Write(f, data, sizeof(data)) == sizeof(data));

    BUFFER_Delete(buf);
    FILE_Close(f);
    return TEST_OK;
}

static
TestStatus
test_fmem_read(
    const TestDesc* test)
{
    static const I8u data[] = {1, 2, 3};
    int skip = 1;
    File* f = FILE_MemIn(data, sizeof(data));
    Buffer* buf = BUFFER_Create();

    TEST_ASSERT(FILE_IsMem(f));
    TEST_ASSERT(!FILE_IsFileIO(f));
    TEST_ASSERT(FILE_Fd(f) < 1);
    TEST_ASSERT(FILE_TargetFd(f) < 1);
    TEST_ASSERT(!FILE_CanBlock(f));
    TEST_ASSERT(!FILE_Eof(f));
    TEST_ASSERT(!FILE_Flush(f));
    TEST_ASSERT(FILE_MemSize(f) == sizeof(data));

    TEST_ASSERT(FILE_ReadData(f, buf, -1) == sizeof(data));
    TEST_ASSERT(FILE_Eof(f));
    TEST_ASSERT(FILE_BytesRead(f) == sizeof(data));
    TEST_ASSERT(BUFFER_Size(buf) == sizeof(data));
    TEST_ASSERT(!memcmp(BUFFER_Access(buf), data, sizeof(data)));

    BUFFER_Clear(buf);
    FILE_Close(f);

    f = FILE_MemIn(data, sizeof(data));
    TEST_ASSERT(f);
    TEST_ASSERT(FILE_Skip(f, skip) == (size_t)skip);
    TEST_ASSERT(FILE_ReadData(f, buf, -1) == (int)(sizeof(data)-skip));
    TEST_ASSERT(BUFFER_Size(buf) == sizeof(data)-skip);
    TEST_ASSERT(!memcmp(BUFFER_Access(buf), data + skip, sizeof(data)-skip));

    BUFFER_Delete(buf);
    FILE_Close(f);
    return TEST_OK;
}

static
TestStatus
test_fmem_write(
    const TestDesc* test)
{
    static const I8u data[] = {1, 2, 3};
    File* f = FILE_Mem();
    Buffer* buf = BUFFER_Create();

    TEST_ASSERT(FILE_WriteAll(f, data, sizeof(data)));
    TEST_ASSERT(FILE_MemSize(f) == sizeof(data));
    TEST_ASSERT(!memcmp(FILE_MemData(f), data, sizeof(data)));

    TEST_ASSERT(FILE_ReadData(f, buf, -1) == sizeof(data));
    TEST_ASSERT(FILE_Eof(f));
    TEST_ASSERT(FILE_BytesWritten(f) == sizeof(data));
    TEST_ASSERT(FILE_BytesRead(f) == sizeof(data));
    TEST_ASSERT(BUFFER_Size(buf) == sizeof(data));
    TEST_ASSERT(!memcmp(BUFFER_Access(buf), data, sizeof(data)));

    BUFFER_Delete(buf);
    FILE_Close(f);
    return TEST_OK;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_fmem_alloc},
        {"Read", test_fmem_read},
        {"Write", test_fmem_write}
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
