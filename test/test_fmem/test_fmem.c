/*
 * $Id: test_fmem.c,v 1.2 2016/10/02 22:55:21 slava Exp $
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
test_fmem_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u data[] = {1, 2, 3};
    I8u databuf[4];
    Buffer* buf = BUFFER_Create();
    File* f = FILE_Mem();
    int i;

    /* Test NULL resistance */
    FILE_MemClear(NULL);
    if (FILE_IsMem(NULL) ||
        FILE_MemSize(NULL) ||
        FILE_MemData(NULL)) {
        ret = TEST_ERR;
    }

    /* Simulate allocation failures */
    for (i=0; i<3; i++) {
        testMem.failAt = testMem.allocCount + i;
        if (FILE_Mem()) {
            ret = TEST_ERR;
        }
    }

    for (i=0; i<3; i++) {
        testMem.failAt = testMem.allocCount + i;
        if (FILE_MemIn(data, sizeof(data))) {
            ret = TEST_ERR;
        }
    }

    for (i=0; i<3; i++) {
        testMem.failAt = testMem.allocCount + i;
        if (FILE_MemOut(1)) {
            ret = TEST_ERR;
        }
    }

    for (i=0; i<3; i++) {
        testMem.failAt = testMem.allocCount + i;
        if (FILE_MemOut2(databuf, sizeof(databuf))) {
            ret = TEST_ERR;
        }
    }

    for (i=0; i<2; i++) {
        testMem.failAt = testMem.allocCount + i;
        if (FILE_MemBuf(buf, False)) {
            ret = TEST_ERR;
        }
    }

    /* Once one write fails, the second one will fail too? */
    testMem.failAt = testMem.allocCount;
    if (FILE_Write(f, data, sizeof(data)) >= 0 ||
        FILE_Write(f, data, sizeof(data)) >= 0) {
        ret = TEST_ERR;
    }

    /* But after FILE_MemClear() it should succeed */
    FILE_MemClear(f);
    if (FILE_Write(f, data, sizeof(data)) != sizeof(data)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    FILE_Close(f);
    return ret;
}

static
TestStatus
test_fmem_read(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u data[] = {1, 2, 3};
    int skip = 1;
    File* f = FILE_MemIn(data, sizeof(data));
    Buffer* buf = BUFFER_Create();

    if (!FILE_IsMem(f) ||
        FILE_IsFileIO(f) ||
        FILE_Fd(f) >= 1 ||
        FILE_TargetFd(f) >= 1 ||
        FILE_CanBlock(f) ||
        FILE_Eof(f) ||
        FILE_Flush(f) ||
        FILE_MemSize(f) != sizeof(data)) {
        ret = TEST_ERR;
    }

    if (FILE_ReadData(f, buf, -1) != sizeof(data) ||
        !FILE_Eof(f) ||
        FILE_BytesRead(f) != sizeof(data) ||
        BUFFER_Size(buf) != sizeof(data) ||
        memcmp(BUFFER_Access(buf), data, sizeof(data))) {
        ret = TEST_ERR;
    }

    BUFFER_Clear(buf);
    FILE_Close(f);

    f = FILE_MemIn(data, sizeof(data));
    if (FILE_Skip(f, skip) != (size_t)skip ||
        FILE_ReadData(f, buf, -1) != (int)(sizeof(data)-skip) ||
        BUFFER_Size(buf) != sizeof(data)-skip ||
        memcmp(BUFFER_Access(buf), data + skip, sizeof(data)-skip)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    FILE_Close(f);
    return ret;
}

static
TestStatus
test_fmem_write(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u data[] = {1, 2, 3};
    File* f = FILE_Mem();
    Buffer* buf = BUFFER_Create();

    if (!FILE_WriteAll(f, data, sizeof(data)) ||
        FILE_MemSize(f) != sizeof(data) ||
        memcmp(FILE_MemData(f), data, sizeof(data))) {
        ret = TEST_ERR;
    }

    if (FILE_ReadData(f, buf, -1) != sizeof(data) ||
        !FILE_Eof(f) ||
        FILE_BytesWritten(f) != sizeof(data) ||
        FILE_BytesRead(f) != sizeof(data) ||
        BUFFER_Size(buf) != sizeof(data) ||
        memcmp(BUFFER_Access(buf), data, sizeof(data))) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    FILE_Close(f);
    return ret;
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
