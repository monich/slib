/*
 * $Id: test_utf8.c,v 1.1 2016/09/26 16:09:23 slava Exp $
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
test_utf8_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const wchar_t wtest[] = {0x0442, 0x0435, 0x0441, 0x0442, 0};
    Buffer buf;
    wchar_t wc;

    /* Test NULL resistance */
    if (UTF8_Size(NULL) ||
        UTF8_Size2(NULL, 0) ||
        UTF8_Length(NULL) ||
        UTF8_Encode(NULL) ||
        UTF8_Encode2(NULL, 0, wtest) ||
        UTF8_Decode(NULL) ||
        UTF8_Decode2(NULL, 0, NULL)) {
        ret = TEST_ERR;
    }

    if (UTF8_CharSize(NULL, 0) >= 0 ||
        UTF8_DecodeChar(NULL, NULL, NULL) >= 0 ||
        UTF8_DecodeChar(NULL, NULL, &wc) >= 0 || wc) {
        ret = TEST_ERR;
    }

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    if (UTF8_Encode(wtest)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (UTF8_Decode("\xD1\x82\xD0\xB5\xD1\x81\xD1\x82")) {
        ret = TEST_ERR;
    }

    BUFFER_Init(&buf);
    testMem.failAt = testMem.allocCount;
    if (UTF8_EncodeBuf(&buf, wtest)) {
        ret = TEST_ERR;
    }

    testMem.failAt = -1;
    BUFFER_Destroy(&buf);
    return ret;
}

static
TestStatus
test_utf8_size(
    const TestDesc* test)
{
    static const struct _test_utf8_size {
        const char* utf8;
        int result;
    } tests [] = {
        { "", 0 },
        { "\x01", 1 },
        { "\x81", -1 },
        { "\xFF", -1 },
        { "\xD1\x82", 2 },
        { "\xD1\xD1", -1 },
        { "\xE3\x82\x81", 3 },
        { "\xE3\x82\xC1", -1 },
        { "\xE3\xC2", -1 },
        { "\xF4\x83\x82\x81", 4 },
        { "\xF4\x83\x82\xC1", -1 },
        { "\xF4\x83\xC2", -1 },
        { "\xF4\xC3", -1 },
        { "\xF9\x84\x83\x82\x81", 5 },
        { "\xF9\x84\x83\x82\xC1", -1 },
        { "\xF9\x84\x83\xC2", -1 },
        { "\xF9\x84\xC3", -1 },
        { "\xF9\xC4", -1 },
        { "\xFD\x85\x84\x83\x82\x81", 6 },
        { "\xFD\x85\x84\x83\x82\xC1", -1 },
        { "\xFD\x85\x84\x83\xC2", -1 },
        { "\xFD\x85\x84\xC3", -1 },
        { "\xFD\x85\xC4", -1 },
        { "\xFD\xC5", -1 }
    };
    TestStatus ret = TEST_OK;
    int i;
    for (i=0; i<COUNT(tests); i++) {
        const char* utf8 = tests[i].utf8;
        size_t size = strlen(utf8);
        if (UTF8_CharSize(utf8, size) != tests[i].result) {
            ret = TEST_ERR;
        }
        if (tests[i].result > 0 && size > 0 &&
            UTF8_CharSize(utf8, size-1)) {
            ret = TEST_ERR;
        }
    }
    return ret;
}

static
TestStatus
test_utf8_size2(
    const TestDesc* test)
{
    static const struct _test_utf8_size {
        const char* utf8;
        wchar_t wc;
    } tests [] = {
        { "\x32", 0x32 },
        { "\xD1\x82", 0x0442 },
        { "\xE3\x82\x81", 0x3081 },
#if WCHAR_SIZE > 2
        { "\xF4\x83\x82\x81", 0x103081 },
        { "\xF9\x84\x83\x82\x81", 0x1103081 },
        { "\xFD\x85\x84\x83\x82\x81", 0x45103081 },
#endif
     };
    TestStatus ret = TEST_OK;
    int i;
    for (i=0; i<COUNT(tests); i++) {
        const char* utf8 = tests[i].utf8;
        const size_t size = strlen(utf8);
        size_t n = size;
        char buf[8];
        wchar_t wc[2];
        wc[1] = 0;
        if (UTF8_DecodeChar(utf8, &n, wc) != (int)size ||
            wc[0] != tests[i].wc ||
            UTF8_Size2(wc, 1) != size ||
            UTF8_EncodeChar(buf, size-1, wc[0]) ||
            UTF8_EncodeChar(buf, sizeof(buf), wc[0]) != size ||
            memcmp(utf8, buf, size)) {
            ret = TEST_ERR;
        }
    }
    return ret;
}

static
TestStatus
test_utf8_basic(
    const TestDesc* test)
{
    static const wchar_t ws1[] = {'t', 'e', 's', 't', 0};
    static const wchar_t ws2[] = {0x0442, 0x0435, 0x0441, 0x0442, 0};
    static const wchar_t ws3[] = {0x6e2c, 0x8a66, 0};
    static const struct _test_utf8_size {
        const char* s;
        Bool valid;
        const wchar_t* ws;
    } tests [] = {
        { "test", True, ws1 },
        { "\xD1\x82\xD0\xB5\xD1\x81\xD1\x82", True, ws2 },
        { "\xE6\xB8\xAC\xE8\xA9\xA6", True, ws3 },
#if WCHAR_SIZE > 2
#endif
        { "\xD1\xD1", False},
        { "\xD1", False},
        { "\xE3\x82\xC1", False},
        { "\xE3\xC2", False},
        { "\xF4\x83\x82\xC1", False},
        { "\xF4\x83\xC2", False},
        { "\xF4\xC3", False},
        { "\xF9\x84\x83\x82\xC1", False},
        { "\xF9\x84\x83\xC2", False},
        { "\xF9\x84\xC3", False},
        { "\xF9\xC4", False},
        { "\xFD\x85\x84\x83\x82\xC1", False},
        { "\xFD\x85\x84\x83\xC2", False},
        { "\xFD\x85\x84\xC3", False},
        { "\xFD\x85\xC4", False},
        { "\xFD\xC5", False},
    };

    TestStatus ret = TEST_OK;
    Buffer buf;
    int i;

    BUFFER_Init(&buf);
    for (i=0; i<COUNT(tests); i++) {
        const char* s = tests[i].s;
        if (tests[i].valid) {
            const wchar_t* ws = tests[i].ws;
            const size_t size = UTF8_Size(ws);
            const size_t len = wcslen(ws);
            char* s1 = UTF8_Encode(ws);
            wchar_t* ws1 = UTF8_Decode(s);
            if (size != strlen(s) ||
                UTF8_Length(s) != len) {
                ret = TEST_ERR;
            }
            BUFFER_Clear(&buf);
            if (!UTF8_EncodeBuf(&buf, ws) ||
                BUFFER_Size(&buf) != size ||
                memcmp(BUFFER_Access(&buf), s, size)) {
                ret = TEST_ERR;
            }
            if (wcscmp(ws, ws1) ||
                strcmp(s, s1)) {
                ret = TEST_ERR;
            }
            if (UTF8_Decode2(ws1, len, s) != len) {
                ret = TEST_ERR;
            }
            MEM_Free(s1);
            MEM_Free(ws1);
        } else {
            if (UTF8_Length(s) != UTF8_ERROR ||
                UTF8_Decode(s)) {
                ret = TEST_ERR;
            }
        }
    }
    
    BUFFER_Destroy(&buf);
    return ret;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_utf8_alloc},
        {"Size", test_utf8_size},
        {"Size2", test_utf8_size2},
        {"Basic", test_utf8_basic},
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
