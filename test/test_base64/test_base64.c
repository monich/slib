/*
 * $Id: test_base64.c,v 1.2 2018/12/27 18:16:33 slava Exp $
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
test_base64_alloc(
    const TestDesc* test)
{
    Str test_base64 = TEXT("dGVzdA==");
    static const Char test_data[] = {'t', 'e', 's', 't'};
    Buffer* out = BUFFER_Create();
    StrBuf* sb = STRBUF_Create();
    int i;

    /* Test NULL resistance */
    TEST_ASSERT(BASE64_Decode(NULL, NULL));
    TEST_ASSERT(BASE64_StdDecode(NULL, NULL));
    TEST_ASSERT(BASE64_SafeDecode(NULL, NULL));

    /* Simulate allocation failures */
    for (i = 0; i < 4; i++) {
        BUFFER_Clear(out);
        BUFFER_Trim(out);
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!BASE64_Decode(test_base64, out));

        BUFFER_Clear(out);
        BUFFER_Trim(out);
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!BASE64_StdDecode(test_base64, out));

        BUFFER_Clear(out);
        BUFFER_Trim(out);
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!BASE64_SafeDecode(test_base64, out));
    }

    for (i = 0; i < 3; i++) {
        File* in;
        const size_t len = StrLen(test_base64);

        BUFFER_Clear(out);
        BUFFER_Trim(out);
        in = FILE_MemIn(test_base64, len);
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!BASE64_DecodeFile(in, out));
        FILE_Close(in);

        BUFFER_Clear(out);
        BUFFER_Trim(out);
        in = FILE_MemIn(test_base64, len);
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!BASE64_StdDecodeFile(in, out));
        FILE_Close(in);

        BUFFER_Clear(out);
        BUFFER_Trim(out);
        in = FILE_MemIn(test_base64, len);
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!BASE64_SafeDecodeFile(in, out));
        FILE_Close(in);
    }

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!BASE64_Encode(test_data, sizeof(test_data), 0));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!BASE64_EncodeStr(test_data, sizeof(test_data), sb, 0));

    testMem.failAt = -1;
    BUFFER_Delete(out);
    STRBUF_Delete(sb);
    return TEST_OK;
}

static
TestStatus
test_base64_encode(
    const TestDesc* test)
{
    StrBuf* sb = STRBUF_Create();
    int i;

    static const struct _test_base64_encode {
        const char* in;
        Str out;
        int flags;
    } tests[] = {
        { "abc", TEXT("YWJj"), 0 },
        { "abc", TEXT("YWJj"), BASE64_PAD },
        { "abcd", TEXT("YWJjZA"), 0 },
        { "abcd", TEXT("YWJjZA=="), BASE64_PAD },
        { "abcde", TEXT("YWJjZGU"), 0 },
        { "abcde", TEXT("YWJjZGU="), BASE64_PAD },
        { "c\xef\xe3", TEXT("Y+/j"), 0 },
        { "c\xef\xe3", TEXT("Y-_j"), BASE64_URLSAFE }
    };

    for (i = 0; i < COUNT(tests); i++) {
        const char* in = tests[i].in;
        const int len = (int)strlen(in);
        Str out = tests[i].out;
        int flags = tests[i].flags;
        Char* str = BASE64_Encode(in, len, flags);

        TEST_ASSERT(str);
        TEST_ASSERT(!StrCmp(str, out));
        MEM_Free(str);

        STRBUF_Clear(sb);
        TEST_ASSERT(BASE64_EncodeStr(in, len, sb, flags));
        TEST_ASSERT(STRBUF_EqualsTo(sb, out));
    }

    STRBUF_Delete(sb);
    return TEST_OK;
}

static
TestStatus
test_base64_decode_ok(
    const TestDesc* test)
{
    Buffer* out = BUFFER_Create();
    int i;

    static const struct _test_base64_decode_ok {
        Str in;
        const char* out;
        Bool (*decode)(Str base64, Buffer * out);
    } ok [] = {
        { TEXT(""), "",  BASE64_Decode },
        { TEXT("YWJj"), "abc", BASE64_Decode },
        { TEXT(" YW Jj "), "abc", BASE64_Decode },
        { TEXT("YWJjZA"), "abcd", BASE64_Decode },
        { TEXT("YWJjZA=="), "abcd", BASE64_Decode },
        { TEXT("YWJjZA==  "), "abcd", BASE64_Decode },
        { TEXT("YWJjZGU"), "abcde", BASE64_Decode },
        { TEXT("YWJjZGU="), "abcde", BASE64_Decode },
        { TEXT("Y+/j"),  "c\xef\xe3", BASE64_Decode },
        { TEXT("Y+/j"),  "c\xef\xe3", BASE64_StdDecode },
        { TEXT("Y-_j"), "c\xef\xe3", BASE64_Decode },
        { TEXT("Y-_j"), "c\xef\xe3", BASE64_SafeDecode }
    };

    for (i = 0; i < COUNT(ok); i++) {
        const char* result = ok[i].out;

        BUFFER_Clear(out);
        TEST_ASSERT(ok[i].decode(ok[i].in, NULL));
        TEST_ASSERT(ok[i].decode(ok[i].in, out));
        TEST_ASSERT(BUFFER_Size(out) == strlen(result));
        TEST_ASSERT(!memcmp(BUFFER_Access(out), result, BUFFER_Size(out)));
    }

    BUFFER_Delete(out);
    return TEST_OK;
}

static
TestStatus
test_base64_decode_err(
    const TestDesc* test)
{
    Buffer* out = BUFFER_Create();
    int i;

    static const struct _test_base64_decode_err {
        Str in;
        Bool (*decode)(Str base64, Buffer * out);
    } err [] = {
        { TEXT("YWJjZA==="), BASE64_Decode },
        { TEXT("YWJjjj== junk"), BASE64_Decode },
        { TEXT("YWJjZA=a"), BASE64_Decode },
        { TEXT("YWJj=ZA"), BASE64_Decode },
        { TEXT("\xfe\xed"), BASE64_Decode },
        { TEXT("YWJjj"), BASE64_Decode },
        { TEXT("Y:)j"), BASE64_Decode },
        { TEXT("Y+/j"), BASE64_SafeDecode },
        { TEXT("Y-_j"), BASE64_StdDecode }
    };

    for (i = 0; i < COUNT(err); i++) {
        TEST_ASSERT(!err[i].decode(err[i].in, NULL));
        TEST_ASSERT(!err[i].decode(err[i].in, out));
        TEST_ASSERT(!BUFFER_Size(out));
    }

    BUFFER_Delete(out);
    return TEST_OK;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_base64_alloc},
        {"Encode", test_base64_encode},
        {"DecodeOK", test_base64_decode_ok},
        {"DecodeErr", test_base64_decode_err}
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
