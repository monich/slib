/*
 * $Id: test_buf.c,v 1.2 2016/10/02 22:55:20 slava Exp $
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
test_buf_null(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Buffer* buf = BUFFER_Create();

    /* BUFFER_Delete should be NULL resistant */
    BUFFER_Delete(NULL);

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    if (BUFFER_Create()) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BUFFER_Create2(NULL, 0)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BUFFER_CreateRead(NULL, 0)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BUFFER_CreateWrite(NULL, 0, False)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BUFFER_Reserve0(buf, 1)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_byteswap(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    const I16u x16 = 0x0123;
    const I16u y16 = 0x2301;
    const I32u x32 = 0x01234567;
    const I32u y32 = 0x67452301;
    const I64u x64 = __INT64_C(0x0123456789abcdef);
    const I64u y64 = __INT64_C(0xefcdab8967452301);

    if (DATA_Swap16(x16) != y16 ||
        DATA_Swap32(x32) != y32 ||
        DATA_Swap64(x64) != y64) {
        ret = TEST_ERR;
    }

    if (DATA_Conv16(x16, BIG_ENDIAN, LITTLE_ENDIAN) != y16 ||
        DATA_Conv32(x32, BIG_ENDIAN, LITTLE_ENDIAN) != y32 ||
        DATA_Conv64(x64, BIG_ENDIAN, LITTLE_ENDIAN) != y64) {
        ret = TEST_ERR;
    }

    if (DATA_Conv16(x16, LITTLE_ENDIAN, BIG_ENDIAN) != y16 ||
        DATA_Conv32(x32, LITTLE_ENDIAN, BIG_ENDIAN) != y32 ||
        DATA_Conv64(x64, LITTLE_ENDIAN, BIG_ENDIAN) != y64) {
        ret = TEST_ERR;
    }

    if (DATA_Conv16(x16, BIG_ENDIAN, BIG_ENDIAN) != x16 ||
        DATA_Conv32(x32, BIG_ENDIAN, BIG_ENDIAN) != x32 ||
        DATA_Conv64(x64, BIG_ENDIAN, BIG_ENDIAN) != x64) {
        ret = TEST_ERR;
    }

    if (DATA_Conv16(x16, LITTLE_ENDIAN, LITTLE_ENDIAN) != x16 ||
        DATA_Conv32(x32, LITTLE_ENDIAN, LITTLE_ENDIAN) != x32 ||
        DATA_Conv64(x64, LITTLE_ENDIAN, LITTLE_ENDIAN) != x64) {
        ret = TEST_ERR;
    }

    return ret;
}

static
TestStatus
test_buf_basic1(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Buffer* buf = BUFFER_Create();
    const I8s in8 = (I8s)0xab;
    const I16s in16 = 0x0123;
    const I32s in32 = 0x01234567;
    const I64s in64 = __INT64_C(0x0123456789abcdef);
    I8s out8 = 0;
    I16s out16 = 0;
    I32s out32 = 0;
    I64s out64 = 0;
    size_t size;

    /* There should be no buffer initially */
    if (BUFFER_Access(buf)) {
        ret = TEST_ERR;
    }

    /* And nothing to skip */
    if (BUFFER_Skip(buf, 0) ||
        BUFFER_Skip(buf, 1)) {
        ret = TEST_ERR;
    }

    /* Nothing happens if we put in nothing */
    if (BUFFER_Put(buf, NULL, 0, True)) {
        ret = TEST_ERR;
    }

    /* Now put in something */
    if (!BUFFER_PutI8(buf, in8) ||
        !BUFFER_PutI16(buf, in16) ||
        !BUFFER_PutI32(buf, in32) ||
        !BUFFER_PutI64(buf, in64)) {
        ret = TEST_ERR;
    }

    if (BUFFER_Unput(buf, sizeof(in64)) != sizeof(in64) ||
        !BUFFER_PutI64(buf, in64)) {
        ret = TEST_ERR;
    }

    if (!BUFFER_Access(buf)) {
        ret = TEST_ERR;
    }

    if (!BUFFER_GetI8(buf, &out8) || out8 != in8 ||
        !BUFFER_PushBack(buf, NULL, 1) || !BUFFER_GetI8(buf, NULL) ||
        !BUFFER_GetI16(buf, &out16) || out16 != in16 ||
        !BUFFER_GetI32(buf, &out32) || out32 != in32 ||
        !BUFFER_GetI64(buf, &out64) || out64 != in64) {
        ret = TEST_ERR;
    }

    /* There should be nothing left */
    if (BUFFER_GetI8(buf, &out8) ||
        BUFFER_GetI16(buf, &out16) ||
        BUFFER_GetI32(buf, &out32) ||
        BUFFER_GetI64(buf, &out64)) {
        ret = TEST_ERR;
    }

    /* Clearing the buffer doesn't deallocate the storage */
    BUFFER_Clear(buf);
    if (BUFFER_Size(buf) ||
        !BUFFER_Access(buf)) {
        ret = TEST_ERR;
    }

    /* Trimming does */
    BUFFER_Trim(buf);
    if (BUFFER_Size(buf) ||
        BUFFER_Access(buf) ||
        BUFFER_Steal(buf, NULL) ||
        BUFFER_Steal(buf, &size) || size) {
        ret = TEST_ERR;
    }

    /* Practice stealing the buffer */
    if (!BUFFER_PutI8(buf, 0)) {
        ret = TEST_ERR;
    }
    MEM_Free(BUFFER_Steal(buf, &size));

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_basic2(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    size_t size = 4;
    void* data = MEM_Alloc(size);
    Buffer* buf = BUFFER_Create2(data, size);

    if (BUFFER_Size(buf) != size) {
        ret = TEST_ERR;
    }

    BUFFER_Clear(buf);
    if (!BUFFER_PutI8(buf, 0x01) ||
        !BUFFER_PutI8(buf, 0x02) ||
        BUFFER_Size(buf) != 2) {
        ret = TEST_ERR;
    }

    if (!BUFFER_PutI8(buf, 0x03) ||
        !BUFFER_PutI8(buf, 0x04) ||
        BUFFER_Size(buf) != 4) {
        ret = TEST_ERR;
    }

    if (BUFFER_Skip(buf, 2) != 2 ||
        !BUFFER_PutI8(buf, 0x05) ||
        !BUFFER_PutI8(buf, 0x06) ||
        BUFFER_Size(buf) != 4) {
        ret = TEST_ERR;
    }

    /* This one should fail */
    testMem.failAt = testMem.allocCount;
    if (BUFFER_Steal(buf, NULL)) {
        ret = TEST_ERR;
    }

    MEM_Free(BUFFER_Steal(buf, NULL));
    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_trim1(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    size_t size = 6;
    void* data = MEM_Alloc(size);
    Buffer* buf = BUFFER_Create2(data, size);
    I32s x32 = 0;

    buf->maxsiz = size;
    buf->order = BIG_ENDIAN;
    buf->flags &= ~BUFFER_OWN_DATA;
    BUFFER_Clear(buf);
    if (!BUFFER_PutI32(buf, 0x01abcdef)) {
        ret = TEST_ERR;
    }

    /* This trim shouldn't free our buffer */
    BUFFER_Trim(buf);

    /* Simulate allocation failure */
    testMem.failAt = testMem.allocCount;
    if (BUFFER_EnsureCapacity(buf, BUFFER_Size(buf)+1, False)) {
        ret = TEST_ERR;
    }

    /* Will fail to allocate more than maxsiz */
    if (BUFFER_EnsureCapacity(buf, size+1, False) ||
        !BUFFER_EnsureCapacity(buf, size + 1, True)) {
        ret = TEST_ERR;
    }

    if (!BUFFER_PutI8(buf, 0x01) ||
        !BUFFER_PutI8(buf, 0x02) ||
        BUFFER_PutI8(buf, 0x03) ||
        BUFFER_Size(buf) != size) {
        ret = TEST_ERR;
    }

    if (BUFFER_Skip(buf, 4) != 4 ||
        !BUFFER_PutI8(buf, 0x03) ||
        !BUFFER_PutI8(buf, 0x04) ||
        BUFFER_Size(buf) != 4) {
        ret = TEST_ERR;
    }

    /* Make one trim fail */
    testMem.failAt = testMem.allocCount;
    BUFFER_Trim(buf);
    BUFFER_Trim(buf);

    if (!BUFFER_GetI32(buf, &x32) || x32 != 0x01020304 ||
        BUFFER_Size(buf) != 0) {
        ret = TEST_ERR;
    }

    MEM_Free(data);
    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_trim2(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Buffer* buf = BUFFER_Create2(NULL, 0);
    I32s x32 = 0;

    buf->maxsiz = 4;
    buf->order = BIG_ENDIAN;
    if (!BUFFER_PutI8(buf, 0x01) ||
        !BUFFER_PutI8(buf, 0x02) ||
        !BUFFER_PutI8(buf, 0x03) ||
        !BUFFER_PutI8(buf, 0x04) ||
        BUFFER_PutI8(buf, 0x05) ||
        BUFFER_Size(buf) != 4) {
        ret = TEST_ERR;
    }

    if (BUFFER_Skip(buf, 2) != 2 ||
        BUFFER_Size(buf) != 2) {
        ret = TEST_ERR;
    }

    BUFFER_Trim(buf);
    if (!BUFFER_PutI8(buf, 0x05) ||
        !BUFFER_PutI8(buf, 0x06) ||
        !BUFFER_GetI32(buf, &x32) || x32 != 0x03040506) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_unput(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Buffer* buf = BUFFER_Create();
    I32s x32 = 0;

    buf->maxsiz = 6;
    buf->order = BIG_ENDIAN;

    /* Unput at the beginning of the buffer */
    if (!BUFFER_PutI32(buf, 0) ||
        BUFFER_Unput(buf, buf->maxsiz) != 4 ||
        BUFFER_Size(buf)) {
        ret = TEST_ERR;
    }

    /* "Wrapped" unput */
    if (!BUFFER_PutI32(buf, 0) ||
        !BUFFER_PutI8(buf, 0x01) ||
        !BUFFER_PutI8(buf, 0x02) ||
        BUFFER_Skip(buf, 4) != 4) {
        ret = TEST_ERR;
    }

    if (!BUFFER_PutI8(buf, 0x03) ||
        BUFFER_Unput(buf, 2) != 2) {
        ret = TEST_ERR;
    }

    /* Unput exactly the "wrapped" part */
    if (!BUFFER_PutI8(buf, 0x04) ||
        !BUFFER_PutI8(buf, 0x05) ||
        BUFFER_Unput(buf, 1) != 1) {
    }

    /* Similar case except when the buffer is full */
    if (!BUFFER_PutI32(buf, 0) ||
        BUFFER_Unput(buf, 2) != 2 || BUFFER_Size(buf) != 4 ||
        BUFFER_Unput(buf, 2) != 2 || BUFFER_Size(buf) != 2) {
        ret = TEST_ERR;
    }

    /* Make sure the buffer contains what we expect */
    if (!BUFFER_PutI8(buf, 0x06) ||
        !BUFFER_PutI8(buf, 0x07) ||
        !BUFFER_GetI32(buf, &x32) || x32 != 0x01040607 ||
        BUFFER_Size(buf)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_access1(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d1 [] = {0x01, 0x02, 0x03, 0x04};
    Buffer* buf = BUFFER_Create();
    const void* data;

    /* With enough memory available we should move the data in place */
    buf->maxsiz = 8;
    if (!BUFFER_PutI32(buf, 0) ||
        !BUFFER_PutI16(buf, 0) ||
        BUFFER_Skip(buf, 4) != 4 ||
        !BUFFER_PutI8(buf, 0x01) ||
        !BUFFER_PutI8(buf, 0x02) ||
        !BUFFER_PutI8(buf, 0x03) ||
        !BUFFER_PutI8(buf, 0x04) ||
        BUFFER_Skip(buf, 2) != 2) {
        ret = TEST_ERR;
    }

    data = BUFFER_Access(buf);
    if (!data ||
        BUFFER_Size(buf) != sizeof(d1) ||
        memcmp(data, d1, sizeof(d1))) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_access2(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d1 [] = {0x01, 0x02, 0x03, 0x04};
    size_t size = 4;
    void* mybuf = MEM_Alloc(size);
    Buffer* buf = BUFFER_CreateWrite(mybuf, size, False);
    const void* data;

    /* Unwrap full buffer */
    buf->maxsiz = size;
    if (!BUFFER_PutI16(buf, 0) ||
        !BUFFER_PutI8(buf, 0x01) ||
        !BUFFER_PutI8(buf, 0x02) ||
        BUFFER_Skip(buf, 2) != 2 ||
        !BUFFER_PutI8(buf, 0x03) ||
        !BUFFER_PutI8(buf, 0x04)) {
        ret = TEST_ERR;
    }

    /* This one should fail */
    testMem.failAt = testMem.allocCount;
    if (BUFFER_Access(buf)) {
        ret = TEST_ERR;
    }

    /* And this one should succeed */
    data = BUFFER_Access(buf);
    if (!data ||
        BUFFER_Size(buf) != sizeof(d1) ||
        memcmp(data, d1, sizeof(d1))) {
        ret = TEST_ERR;
    }

    MEM_Free(mybuf);
    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_access3(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d1 [] = {0x01, 0x02, 0x03, 0x04};
    size_t size = 6;
    void* mybuf = MEM_Alloc(size);
    Buffer* buf = BUFFER_CreateWrite(mybuf, size, True);
    const void* data;

    /* 1 byte head and 3 bytes tail */
    buf->maxsiz = size;
    if (!BUFFER_PutI32(buf, 0) ||
        !BUFFER_PutI8(buf, 0) ||
        BUFFER_Skip(buf, 4) != 4 ||
        !BUFFER_Put(buf, d1, sizeof(d1), False) ||
        BUFFER_Skip(buf, 1) != 1) {
        ret = TEST_ERR;
    }

    data = BUFFER_Access(buf);
    if (!data ||
        BUFFER_Size(buf) != sizeof(d1) ||
        memcmp(data, d1, sizeof(d1))) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_access4(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d1 [] = {0x01, 0x02, 0x03, 0x04};
    size_t size = 4;
    void* mybuf = MEM_Alloc(size);
    Buffer* buf = BUFFER_CreateWrite(mybuf, size, False);
    const void* data;

    /* 3 bytes head and 1 byte tail */
    buf->maxsiz = size;
    if (!BUFFER_PutI8(buf, 0) ||
        !BUFFER_Put(buf, d1, sizeof(d1)-1, False) ||
        BUFFER_Skip(buf, 1) != 1 ||
        !BUFFER_Put(buf, d1 + sizeof(d1) - 1, 1, False)) {
        ret = TEST_ERR;
    }

    /* This one should fail */
    testMem.failAt = testMem.allocCount;
    if (BUFFER_Access(buf)) {
        ret = TEST_ERR;
    }

    /* And this one should succeed */
    data = BUFFER_Access(buf);
    if (!data ||
        BUFFER_Size(buf) != sizeof(d1) ||
        memcmp(data, d1, sizeof(d1))) {
        ret = TEST_ERR;
    }

    MEM_Free(mybuf);
    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_reserve(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d [] = {0x01, 0x02, 0x03, 0x04};
    static const I8u d2 [] = {0x05, 0x06};
    Buffer* buf = BUFFER_Create();
    I8u* data = BUFFER_Reserve0(buf, sizeof(d));
    size_t i;

    for (i=0; i<sizeof(d); i++) {
        if (data[i]) {
            ret = TEST_ERR;
        }
    }

    /* This won't do anything */
    if (!BUFFER_Reserve(buf, 0)) {
        ret = TEST_ERR;
    }

    memcpy(data, d, sizeof(d));
    data = BUFFER_Access(buf);
    if (!data ||
        BUFFER_Size(buf) != sizeof(d) ||
        memcmp(data, d, sizeof(d))) {
        ret = TEST_ERR;
    }

    /* Double the capacity */
    BUFFER_EnsureCapacity(buf, 2*sizeof(d), False);
    memcpy(BUFFER_Reserve(buf, 2), d2, sizeof(d2));

    /* Force reserve to move the data */
    BUFFER_Skip(buf, 2);
    data = BUFFER_Reserve0(buf, sizeof(d));
    for (i=0; i<sizeof(d); i++) {
        if (data[i]) {
            ret = TEST_ERR;
        }
    }
    memcpy(data, d, sizeof(d));

    /* Make sure that the contents was moved correctly */
    if (buf->data[0] != d[2] ||
        buf->data[1] != d[3] ||
        buf->data[2] != d2[0] ||
        buf->data[3] != d2[1]) {
        ret = TEST_ERR;
    }

    BUFFER_Skip(buf, 4);
    data = BUFFER_Access(buf);
    if (!data ||
        BUFFER_Size(buf) != sizeof(d) ||
        memcmp(data, d, sizeof(d))) {
        ret = TEST_ERR;
    }

    BUFFER_Skip(buf, 4);
    if (BUFFER_Size(buf)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_move1(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d [] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Buffer* src = BUFFER_Create();
    Buffer* dest = BUFFER_Create();
    const void* data;
    const size_t size = sizeof(d);

    /* There's nothing there yet */
    if (BUFFER_Move(src, dest, size) != 0) {
        ret = TEST_ERR;
    }

    /* Put something in there */
    if (BUFFER_Put(src, d, size, False) != size) {
        ret = TEST_ERR;
    }

    /* This move will fail */
    testMem.failAt = testMem.allocCount;
    if (BUFFER_Move(src, dest, size) ||
        BUFFER_Size(src) != size) {
        ret = TEST_ERR;
    }

    /* And this one should succeed */
    if (BUFFER_Move(src, dest, size) != size ||
        BUFFER_Size(src)) {
        ret = TEST_ERR;
    }

    data = BUFFER_Access(dest);
    if (BUFFER_Size(dest) != size ||
        memcmp(data, d, size)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(src);
    BUFFER_Delete(dest);
    return ret;
}

static
TestStatus
test_buf_move2(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d [] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Buffer* src = BUFFER_Create();
    Buffer* dest = BUFFER_Create();
    const void* data;
    const size_t size = sizeof(d);

    /* "Wrap" the source buffer */
    src->maxsiz = sizeof(d)+2;
    if (!BUFFER_EnsureCapacity(src, src->maxsiz, False) ||
        !BUFFER_PutI32(src, 0) ||
        BUFFER_Skip(src, 2) != 2 ||
        BUFFER_Put(src, d, size, False) != size ||
        BUFFER_Skip(src, 2) != 2 ||
        BUFFER_Move(src, dest, size+1) != size ||
        BUFFER_Size(src)) {
        ret = TEST_ERR;
    }

    data = BUFFER_Access(dest);
    if (BUFFER_Size(dest) != size ||
        memcmp(data, d, size)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(src);
    BUFFER_Delete(dest);
    return ret;
}

static
TestStatus
test_buf_push1(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d [] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Buffer* buf = BUFFER_Create();
    const void* data;
    const size_t size = sizeof(d);

    buf->maxsiz = size;
    if (BUFFER_Put(buf, d, size, False) != size ||
        BUFFER_Skip(buf, 2) != 2 ||
        !BUFFER_PushBack(buf, NULL, 2) ||
        BUFFER_PushBack(buf, NULL, 2)) {
        ret = TEST_ERR;
    }

    data = BUFFER_Access(buf);
    if (BUFFER_Size(buf) != size ||
        memcmp(data, d, size)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_push2(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d [] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Buffer* buf = BUFFER_Create();
    const void* data;
    const size_t size = sizeof(d);

    /* "Wrap" the buffer */
    buf->maxsiz = sizeof(d);
    if (!BUFFER_EnsureCapacity(buf, buf->maxsiz, False) ||
        !BUFFER_PutI32(buf, 0) ||
        BUFFER_Skip(buf, 2) != 2 ||
        BUFFER_Put(buf, d, size - 2, False) != (size - 2) ||
        BUFFER_Skip(buf, 2) != 2 ||
        BUFFER_Put(buf, d + (size - 2), 2, False) != 2) {
        ret = TEST_ERR;
    }

    if (BUFFER_Skip(buf, 2) != 2 ||
        !BUFFER_PushBack(buf, NULL, 2) ||
        BUFFER_PushBack(buf, NULL, 2)) {
        ret = TEST_ERR;
    }

    data = BUFFER_Access(buf);
    if (BUFFER_Size(buf) != size ||
        memcmp(data, d, size)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_push3(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d [] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    Buffer* buf = BUFFER_Create();
    const void* data;
    const size_t size = sizeof(d);

    /* "Wrap" the buffer */
    buf->maxsiz = sizeof(d);
    if (!BUFFER_EnsureCapacity(buf, buf->maxsiz, False) ||
        !BUFFER_PutI32(buf, 0) ||
        !BUFFER_PutI16(buf, 0) ||
        BUFFER_Skip(buf, 4) != 4 ||
        BUFFER_Put(buf, d, size - 2, False) != (size - 2) ||
        BUFFER_Skip(buf, 2) != 2 ||
        BUFFER_Put(buf, d + (size - 2), 2, False) != 2) {
        ret = TEST_ERR;
    }

    if (BUFFER_Skip(buf, 4) != 4 ||
        !BUFFER_PushBack(buf, NULL, 4) ||
        BUFFER_PushBack(buf, NULL, 1)) {
        ret = TEST_ERR;
    }

    data = BUFFER_Access(buf);
    if (BUFFER_Size(buf) != size ||
        memcmp(data, d, size)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_push4(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const I8u d [] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    static const I8u d1 [] = {0x0a, 0x0b, 0x0c, 0x0d, 0x05, 0x06, 0x07, 0x08};
    Buffer* buf = BUFFER_Create();
    const void* data;
    const size_t size = sizeof(d);

    /* "Wrap" the buffer */
    buf->maxsiz = sizeof(d);
    if (!BUFFER_EnsureCapacity(buf, buf->maxsiz, False) ||
        !BUFFER_PutI32(buf, 0) ||
        !BUFFER_PutI16(buf, 0) ||
        BUFFER_Skip(buf, 5) != 5 ||
        BUFFER_Put(buf, d, size - 1, False) != (size - 1) ||
        BUFFER_Skip(buf, 1) != 1 ||
        BUFFER_Put(buf, d + (size - 1), 1, False) != 1) {
        ret = TEST_ERR;
    }

    if (BUFFER_Skip(buf, 4) != 4 ||
        !BUFFER_PushBack(buf, d1, 4) ||
        BUFFER_PushBack(buf, NULL, 1)) {
        ret = TEST_ERR;
    }

    data = BUFFER_Access(buf);
    if (BUFFER_Size(buf) != size ||
        memcmp(data, d1, size)) {
        ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_read(
    const TestDesc* test)
{
    static const I8u data[] = {
        0x01, 0x23,
        0x01, 0x23, 0x45, 0x67,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
    };
    const I16s be16 = 0x0123;
    const I16s le16 = 0x2301;
    const I32s be32 = 0x01234567;
    const I32s le32 = 0x67452301;
    const I64s be64 = __INT64_C(0x0123456789abcdef);
    const I64s le64 = __INT64_C(0xefcdab8967452301);
    Buffer* buf = BUFFER_CreateRead(data, sizeof(data));
    TestStatus ret = TEST_OK;
    I16s out16 = 0;
    I32s out32 = 0;
    I64s out64 = 0;

    /* Can't put anything in there */
    if (!BUFFER_IsReadOnly(buf) ||
        BUFFER_PutI16(buf, be16) ||
        BUFFER_EnsureCapacity(buf, buf->alloc + 1, False) ||
        BUFFER_EnsureCapacity(buf, buf->alloc + 1, True)) {
        ret = TEST_ERR;
    }

    /* Skip the data, no conversion */
    if (!BUFFER_GetI16(buf, NULL) ||
        !BUFFER_GetI32(buf, NULL) ||
        !BUFFER_GetI64(buf, NULL) ||
        BUFFER_GetI8(buf, NULL)) {
        ret = TEST_ERR;
    }

    /* Can't push anything back */
    if (BUFFER_PushBack(buf, NULL, 1)) {
        ret = TEST_ERR;
    }

    /* Read little endian */
    BUFFER_Delete(buf);
    buf = BUFFER_CreateRead(data, sizeof(data));
    buf->order = LITTLE_ENDIAN;
    if (!BUFFER_GetI16(buf, &out16) || out16 != le16 ||
        !BUFFER_GetI32(buf, &out32) || out32 != le32 ||
        !BUFFER_GetI64(buf, &out64) || out64 != le64 ||
        BUFFER_GetI8(buf, NULL)) {
       ret = TEST_ERR;
    }

    /* Read big endian */
    BUFFER_Delete(buf);
    buf = BUFFER_CreateRead(data, sizeof(data));
    buf->order = BIG_ENDIAN;
    if (!BUFFER_GetI16(buf, &out16) || out16 != be16 ||
        !BUFFER_GetI32(buf, &out32) || out32 != be32 ||
        !BUFFER_GetI64(buf, &out64) || out64 != be64 ||
        BUFFER_GetI8(buf, NULL)) {
       ret = TEST_ERR;
    }

    /* Trim doesn't do anything we buffer doesn't own the data */
    BUFFER_Trim(buf);
    BUFFER_Delete(buf);
    return ret;
}

static
TestStatus
test_buf_write(
    const TestDesc* test)
{
    static const I8u be[] = {
        0x01, 0x23,
        0x01, 0x23, 0x45, 0x67,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
    };
    static const I8u le[] = {
        0x23, 0x01,
        0x67, 0x45, 0x23, 0x01,
        0xef, 0xcd, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01
    };
    const I16u x16 = 0x0123;
    const I32u x32 = 0x01234567;
    const I64u x64 = __INT64_C(0x0123456789abcdef);
    TestStatus ret = TEST_OK;
    const void* data;
    Buffer* buf = BUFFER_CreateWrite(NULL, 0, False);

    if (BUFFER_IsReadOnly(buf)) {
        ret = TEST_ERR;
    }

    /* Write little endian */
    buf->order = LITTLE_ENDIAN;
    if (!BUFFER_PutI16(buf, x16) ||
        !BUFFER_PutI32(buf, x32) ||
        !BUFFER_PutI64(buf, x64)) {
        ret = TEST_ERR;
    }

    data = BUFFER_Access(buf);
    if (BUFFER_Size(buf) != sizeof(le) || !data ||
        memcmp(data, le, sizeof(le))) {
        ret = TEST_ERR;
    }

    /* Write big endian */
    buf->order = BIG_ENDIAN;
    if (!BUFFER_PutI16(buf, x16) ||
        !BUFFER_PutI32(buf, x32) ||
        !BUFFER_PutI64(buf, x64) ||
        BUFFER_Skip(buf, sizeof(le)) != sizeof(le)) {
        ret = TEST_ERR;
    }

    data = BUFFER_Access(buf);
    if (BUFFER_Size(buf) != sizeof(be) || !data ||
        memcmp(data, be, sizeof(be))) {
       ret = TEST_ERR;
    }

    BUFFER_Delete(buf);
    return ret;
}

int main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"NULL", test_buf_null},
        {"ByteSwap", test_buf_byteswap},
        {"Basic1", test_buf_basic1},
        {"Basic2", test_buf_basic2},
        {"Trim1", test_buf_trim1},
        {"Trim2", test_buf_trim2},
        {"Unput", test_buf_unput},
        {"Access1", test_buf_access1},
        {"Access2", test_buf_access2},
        {"Access3", test_buf_access3},
        {"Access4", test_buf_access4},
        {"Reserve", test_buf_reserve},
        {"Move1", test_buf_move1},
        {"Move2", test_buf_move2},
        {"Push1", test_buf_push1},
        {"Push2", test_buf_push2},
        {"Push3", test_buf_push3},
        {"Push4", test_buf_push4},
        {"Read", test_buf_read},
        {"Write", test_buf_write},
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
