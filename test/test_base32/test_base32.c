/*
 * $Id: test_base32.c,v 1.1 2016/09/26 16:09:13 slava Exp $
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
test_base32_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Str test_base32 = TEXT("MFRGGZDF");
    static const Char test_data[] = {'t', 'e', 's', 't'};
    Buffer* out = BUFFER_Create();
    StrBuf* sb = STRBUF_Create();
    int i;

    /* Test NULL resistance */
    if (!BASE32_Decode(NULL, NULL) ||
        !BASE32_StrictDecode(NULL, NULL)) {
        ret = TEST_ERR;
    }

    /* Simulate allocation failures */
    for (i=0; i<4; i++) {
        BUFFER_Clear(out);
        BUFFER_Trim(out);
        testMem.failAt = testMem.allocCount + i;
        if (BASE32_Decode(test_base32, out)) {
            ret = TEST_ERR;
        }

        BUFFER_Clear(out);
        BUFFER_Trim(out);
        testMem.failAt = testMem.allocCount + i;
        if (BASE32_StrictDecode(test_base32, out)) {
            ret = TEST_ERR;
        }
    }

    for (i=0; i<4; i++) {
        File* in;
        const size_t len = StrLen(test_base32);

        BUFFER_Clear(out);
        BUFFER_Trim(out);
        in = FILE_MemIn(test_base32, len);
        testMem.failAt = testMem.allocCount + i;
        if (BASE32_DecodeFile(in, out)) {
            ret = TEST_ERR;
        }
        FILE_Close(in);

        BUFFER_Clear(out);
        BUFFER_Trim(out);
        in = FILE_MemIn(test_base32, len);
        testMem.failAt = testMem.allocCount + i;
        if (BASE32_StrictDecodeFile(in, out)) {
            ret = TEST_ERR;
        }
        FILE_Close(in);
    }

    testMem.failAt = testMem.allocCount;
    if (BASE32_Encode(test_data, sizeof(test_data), 0)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (BASE32_EncodeStr(test_data, sizeof(test_data), sb, 0)) {
        ret = TEST_ERR;
    }

    testMem.failAt = -1;
    BUFFER_Delete(out);
    STRBUF_Delete(sb);
    return ret;
}

static
TestStatus
test_base32_encode(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    StrBuf* sb = STRBUF_Create();
    int i;

    static const struct _test_base32_encode {
        const char* in;
        Str out;
        int flags;
    } tests[] = {
        { "a", TEXT("ME"), 0 },
        { "a", TEXT("ME======"), BASE32_PAD },
        { "ab", TEXT("MFRA"), 0 },
        { "ab", TEXT("MFRA===="), BASE32_PAD },
        { "abc", TEXT("MFRGG"), 0 },
        { "abc", TEXT("MFRGG==="), BASE32_PAD },
        { "abcd", TEXT("MFRGGZA"), 0 },
        { "abcd", TEXT("MFRGGZA="), BASE32_PAD },
        { "abcde", TEXT("MFRGGZDF"), 0 },
        { "abcde", TEXT("MFRGGZDF"), BASE32_PAD },
        { "abcdef", TEXT("MFRGGZDFMY"), 0 },
        { "abcdef", TEXT("MFRGGZDFMY======"), BASE32_PAD },
        { "c\xef\xe3", TEXT("MPX6G"), 0 },
        { "c\xef\xe3", TEXT("mpx6g"), BASE32_LOWERCASE }
    };

    for (i=0; i<COUNT(tests); i++) {
        const char* in = tests[i].in;
        const int len = (int)strlen(in);
        Str out = tests[i].out;
        int flags = tests[i].flags;
        Char* str;
        
        str = BASE32_Encode(in, len, flags);
        if (!str || StrCmp(str, out)) {
            Error("'%s' -> '%s'\n", in, str);
            ret = TEST_ERR;
        }
        MEM_Free(str);

        STRBUF_Clear(sb);
        if (!BASE32_EncodeStr(in, len, sb, flags) ||
            !STRBUF_EqualsTo(sb, out)) {
            ret = TEST_ERR;
        }
    }

    STRBUF_Delete(sb);
    return ret;
}

static
TestStatus
test_base32_decode_ok(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Buffer* out = BUFFER_Create();
    int i;

    static const struct _test_base32_decode_ok {
        Str in;
        const char* out;
        Bool (*decode)(Str base32, Buffer * out);
    } ok [] = {
        { TEXT(""), "",  BASE32_Decode },
        { TEXT("ME"), "a",  BASE32_Decode },
        { TEXT("ME===="), "a",  BASE32_Decode },
        { TEXT("MFRA"), "ab",  BASE32_Decode },
        { TEXT("MFRA===="), "ab",  BASE32_Decode },
        { TEXT("MFRGG"), "abc",  BASE32_Decode },
        { TEXT("MFRGG==="), "abc",  BASE32_Decode },
        { TEXT("M FR GG "), "abc", BASE32_Decode },
        { TEXT("MFRGGZA"), "abcd", BASE32_Decode },
        { TEXT("MFRGGZA="), "abcd", BASE32_Decode },
        { TEXT("MFRGGZA=  "), "abcd", BASE32_Decode },
        { TEXT("MFRGGZDF"), "abcde", BASE32_Decode },
        { TEXT("mFRggZDf"), "abcde", BASE32_Decode },
        { TEXT("mpx6g"),  "c\xef\xe3", BASE32_Decode },
        { TEXT("MPX6G"), "c\xef\xe3", BASE32_StrictDecode }
    };

    for (i=0; i<COUNT(ok); i++) {
        const char* result = ok[i].out;

        BUFFER_Clear(out);
        if (!ok[i].decode(ok[i].in, NULL) ||
            !ok[i].decode(ok[i].in, out) ||
            BUFFER_Size(out) != strlen(result) ||
            memcmp(BUFFER_Access(out), result, BUFFER_Size(out))) {
            Error("'%s'\n", ok[i].in);
            ret = TEST_ERR;
        }
    }

    BUFFER_Delete(out);
    return ret;
}

static
TestStatus
test_base32_decode_err(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Buffer* out = BUFFER_Create();
    int i;

    static const struct _test_base32_decode_err {
        Str in;
        Bool (*decode)(Str base32, Buffer * out);
    } err [] = {
        { TEXT("MFRGG===="), BASE32_Decode },
        //{ TEXT("MFRGG=== junk"), BASE32_Decode },
        { TEXT("M"), BASE32_Decode },
        { TEXT("MFR"), BASE32_Decode },
        { TEXT("MFRGGZ"), BASE32_Decode },
        { TEXT("mfrg:)"), BASE32_Decode },
        { TEXT("MFR=GG"), BASE32_Decode },
        { TEXT("\xfe\xed"), BASE32_Decode },
        { TEXT("MFRGg"), BASE32_StrictDecode }
    };

    for (i=0; i<COUNT(err); i++) {
        if (err[i].decode(err[i].in, NULL) ||
            err[i].decode(err[i].in, out) ||
            BUFFER_Size(out)) {
            Error("'%s'\n", err[i].in);
            ret = TEST_ERR;
        }
    }

    BUFFER_Delete(out);
    return ret;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_base32_alloc},
        {"Encode", test_base32_encode},
        {"DecodeOK", test_base32_decode_ok},
        {"DecodeErr", test_base32_decode_err}
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
