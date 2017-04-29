/*
 * $Id: test_str.c,v 1.3 2016/10/03 09:15:05 slava Exp $
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
#include <locale.h>

static TestMem testMem;

static
TestStatus
test_str_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    StrBuf* sb;
    Vector v;
    char* s;
    char buf[2];
    int i;
    static const wchar_t wtest[] = {0x0442, 0x0435, 0x0441, 0x0442, 0};
    const char* longstr =
        "01234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789";
    const char* longstr99 =
        "01234567890123456789012345678901234567890123456789"
        "0123456789012345678901234567890123456789012345678";

    /* Test NULL resistance */
    if (STRING_ToUnicode(NULL) ||
        STRING_ToMultiByte(NULL) ||
        STRING_ToMultiByteN(NULL, 0) ||
        STRING_ToUnicode(NULL) ||
        STRING_ToUnicodeN(NULL, 0)) {
        ret = TEST_ERR;
    }

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    if (STRING_Dup8("test")) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRING_DupU(L"test")) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRING_Format(NULL, 0, "%s", "test")) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    s = STRING_Format(buf, COUNT(buf), "%s", "test");
    if (s != buf || strcmp(s, "t")) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    buf[0] = 'x';
    s = STRING_Format(buf, 0, "%s", "test");
    if (s != buf || buf[0] != 'x') {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount + 1;
    s = STRING_Format(NULL, 0, "%s", longstr);
    if (!s || strcmp(s, longstr99)) {
        ret = TEST_ERR;
    }
    MEM_Free(s);

    testMem.failAt = testMem.allocCount;
    if (STRING_ToUnicode("test")) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRING_ToUnicode("\xD1\x82\xD0\xB5\xD1\x81\xD1\x82")) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRING_ToMultiByte(L"test")) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRING_ToMultiByte(wtest)) {
        ret = TEST_ERR;
    }

    sb = STRBUF_Create();

    testMem.failAt = testMem.allocCount;
    if (STRING_FormatFloat(sb, 0.1f)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRING_FormatDouble(sb, 0.1)) {
        ret = TEST_ERR;
    }

    VECTOR_Init(&v, 0, vectorEqualsString, vectorFreeValueProc);

    /* Element has to be long enough not to fit into the buffer on stack */
    testMem.failAt = testMem.allocCount;
    if (STRING_Split("0123456789012345678901234567890123456789,2",
        NULL, ",", False) >= 0) {
        ret = TEST_ERR;
    }

    for (i=0; i<4; i++) {
        testMem.failAt = testMem.allocCount + i;
        VECTOR_Trim(&v);
        if (STRING_Split(",", &v, ",", True) >= 0 ||
            VECTOR_Size(&v) > 0) {
            ret = TEST_ERR;
        }
    }

    for (i=0; i<4; i++) {
        testMem.failAt = testMem.allocCount + i;
        VECTOR_Trim(&v);
        if (STRING_Split("1,2", &v, ",", False) >= 0 ||
            VECTOR_Size(&v) > 0) {
            ret = TEST_ERR;
        }
    }

    for (i=0; i<7; i++) {
        testMem.failAt = testMem.allocCount + i;
        VECTOR_Trim(&v);
        if (STRING_Split(",1,2,", &v, ",", True) >= 0 ||
            VECTOR_Size(&v) > 0) {
            ret = TEST_ERR;
        }
    }

    testMem.failAt = -1;
    STRBUF_Delete(sb);
    VECTOR_Destroy(&v);
    return ret;
}

static
TestStatus
test_str_indexof(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const struct _test_str_indexof {
        Str s;
        Char c;
        int first;
        int last;
    } tests[] = {
        { TEXT(""), 0, 0, 0 },
        { TEXT("test"), 0, 4, 4 },
        { TEXT("test"), 't', 0, 3 },
        { TEXT("test"), 'T', -1, -1 },
        { TEXT("test"), 'e', 1, 1 }
    };
    int i;
    for (i=0; i<COUNT(tests); i++) {
        if (STRING_IndexOf(tests[i].s, tests[i].c) != tests[i].first ||
            STRING_LastIndexOf(tests[i].s, tests[i].c) != tests[i].last) {
            ret = TEST_ERR;
        }
    }
    return ret;
}

static
TestStatus
test_str_hash(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const struct _test_str_hash {
        Str s;
        int hash;
        int hashNoCase;
    } tests[] = {
        { NULL, 0, 0 },
        { TEXT("test"), 3556498, 2571410 },
        { TEXT("Test"), 2603186, 2571410 }
    };
    int i;
    for (i=0; i<COUNT(tests); i++) {
        if (STRING_HashCode(tests[i].s) != tests[i].hash ||
            STRING_HashCodeNoCase(tests[i].s) != tests[i].hashNoCase) {
            ret = TEST_ERR;
        }
    }
    return ret;
}

static
TestStatus
test_str_dup(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    char* s;
    wchar_t* ws;

    if (STRING_Dup(NULL) ||
        STRING_DupU(NULL)) {
        ret = TEST_ERR;
    }

    s = STRING_Dup8("test");
    if (strcmp(s, "test")) {
        ret = TEST_ERR;
    }

    ws = STRING_DupU(L"test");
    if (wcscmp(ws, L"test")) {
        ret = TEST_ERR;
    }

    MEM_Free(s);
    MEM_Free(ws);
    return ret;
}

static
TestStatus
test_str_startswith(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const struct _test_str_startswith {
        const char* s;
        const wchar_t* ws;
        const char* p;
        const wchar_t* wp;
        Bool starts;
        Bool startsNoCase;
    } tests[] = {
#define S_(s) s, L##s
        { S_(""), S_(""), True, True },
        { S_("test"), S_(""), True, True },
        { S_("test"), S_("test"), True, True },
        { S_("test"), S_("t"), True, True },
        { S_("test"), S_("est"), False, False },
        { S_("test"), S_("testt"), False, False },
        { S_("Test"), S_("testt"), False, False },
        { S_("test"), S_("T"), False, True },
        { S_("Test"), S_("t"), False, True }
    };
    int i;
    for (i=0; i<COUNT(tests); i++) {
        const char* s = tests[i].s;
        const char* p = tests[i].p;
        const wchar_t* ws = tests[i].ws;
        const wchar_t* wp = tests[i].wp;
        if (STRING_StartsWith8(s, p) != tests[i].starts ||
            STRING_StartsWithNoCase8(s, p) != tests[i].startsNoCase ||
            STRING_StartsWithU(ws, wp) != tests[i].starts ||
            STRING_StartsWithNoCaseU(ws, wp) != tests[i].startsNoCase) {
            ret = TEST_ERR;
        }
    }
    return ret;
}

static
TestStatus
test_str_endswith(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const struct _test_str_endswith {
        const char* s;
        const wchar_t* ws;
        const char* suffix;
        const wchar_t* wsuffix;
        Bool ends;
        Bool endsNoCase;
    } tests[] = {
        { S_(""), S_(""), True, True },
        { S_("test"), S_(""), True, True },
        { S_("test"), S_("test"), True, True },
        { S_("test"), S_("t"), True, True },
        { S_("test"), S_("est"), True, True },
        { S_("test"), S_("tes"), False, False },
        { S_("test"), S_("ttest"), False, False },
        { S_("test"), S_("T"), False, True },
        { S_("tesT"), S_("t"), False, True }
    };
    int i;
    for (i=0; i<COUNT(tests); i++) {
        const char* s = tests[i].s;
        const char* suffix = tests[i].suffix;
        const wchar_t* ws = tests[i].ws;
        const wchar_t* wsuffix = tests[i].wsuffix;
        if (STRING_EndsWith8(s, suffix) != tests[i].ends ||
            STRING_EndsWithNoCase8(s, suffix) != tests[i].endsNoCase ||
            STRING_EndsWithU(ws, wsuffix) != tests[i].ends ||
            STRING_EndsWithNoCaseU(ws, wsuffix) != tests[i].endsNoCase) {
            ret = TEST_ERR;
        }
    }
    return ret;
}

static
TestStatus
test_str_unicode(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    static const wchar_t wtest[] = {0x0442, 0x0435, 0x0441, 0x0442, 0};
    wchar_t* ws;
    char* s;

    ws = STRING_ToUnicode("test");
    if (wcscmp(ws, L"test")) {
        ret = TEST_ERR;
    }
    MEM_Free(ws);

    s = STRING_ToMultiByte(L"test");
    if (!s || strcmp(s, "test")) {
        ret = TEST_ERR;
    }
    MEM_Free(s);

    /* The rest may (and often does) fail on Windows */
#ifndef _WIN32
    ws = STRING_ToUnicode("\xD1\x82\xD0\xB5\xD1\x81\xD1\x82");
    if (wcscmp(ws, wtest)) {
        ret = TEST_ERR;
    }
    MEM_Free(ws);

    /* Invalid UTF-8 sequence */
    ws = STRING_ToUnicode("\xD1\x82\xD0\xB5\xD1\x81\xD1");
    if (ws) {
        ret = TEST_ERR;
    }

    s = STRING_ToMultiByte(wtest);
    if (!s || strcmp(s, "\xD1\x82\xD0\xB5\xD1\x81\xD1\x82")) {
        ret = TEST_ERR;
    }
    MEM_Free(s);
#endif

    return ret;
}

static
TestStatus
test_str_format(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    char buf2[2];
    char* s;
    StrBuf* sb;
    const char* longstr =
        "01234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789";

    s = STRING_Format(buf2, COUNT(buf2), "%s", "test");
    if (s == buf2 || !s || strcmp(s, "test")) {
        ret = TEST_ERR;
    }
    MEM_Free(s);

    s = STRING_Format(NULL, 0, "%s", longstr);
    if (!s || strcmp(s, longstr)) {
        ret = TEST_ERR;
    }
    MEM_Free(s);

    sb = STRBUF_Create();
    if (strcmp(STRING_FormatDouble(sb, 0.), "0.0")) {
        ret = TEST_ERR;
    }

    if (strcmp(STRING_FormatDouble(sb, 0.1), "0.1")) {
        ret = TEST_ERR;
    }

    if (strcmp(STRING_FormatDouble(sb, -0.1), "-0.1")) {
        ret = TEST_ERR;
    }

    if (strcmp(STRING_FormatDouble(sb, 3.14159265358979),"3.14159265358979")) {
        ret = TEST_ERR;
    }

    if (strcmp(STRING_FormatFloat(sb, 0.f), "0.0")) {
        ret = TEST_ERR;
    }

    if (strcmp(STRING_FormatFloat(sb, 0.1f), "0.1")) {
        ret = TEST_ERR;
    }

    if (strcmp(STRING_FormatFloat(sb, -0.1f), "-0.1")) {
        ret = TEST_ERR;
    }

    if (strcmp(STRING_FormatFloat(sb, 0.111f), "0.111")) {
        ret = TEST_ERR;
    }

    if (strcmp(STRING_FormatFloat(sb, 0.111f), "0.111")) {
        ret = TEST_ERR;
    }

    if (strcmp(STRING_FormatFloat(sb, 3.141592f), "3.141592")) {
        ret = TEST_ERR;
    }

    STRBUF_Delete(sb);
    return ret;
}

static
TestStatus
test_str_split(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Vector v;
    int i;

    static const struct _test_str_split {
        Str s;
        Str delim;
        int countEmpty;
        int countNonEmpty;
    } tests[] = {
        { "", "", 0, 0 },
        { " ", "", 1, 1 },
        { ",", ",.", 2, 0 },
        { "123", ",.", 1, 1 },
        { "1,2", ",.", 2, 2 },
        { "1,.2", ".,", 3, 2 },
        { "1,2,", ",,.", 3, 2 },
        { ".1,2", ",..", 3, 2 },
        { ",,1,2", ",..", 4, 2},
        { ",1,2,", ",..", 4, 2},
        { ",1,2,,", ",..", 5, 2},
        { ",,1,2,,", ",..", 6, 2},
    };

    VECTOR_Init(&v, 0, vectorEqualsString, vectorFreeValueProc);
    for (i=0; i<COUNT(tests); i++) {
        Str s = tests[i].s, delim = tests[i].delim;
        if (STRING_Split(s, NULL, delim, True) != tests[i].countEmpty ||
            STRING_Split(s, NULL, delim, False) != tests[i].countNonEmpty) {
            ret = TEST_ERR;
        }
        VECTOR_Clear(&v);
        if (STRING_Split(s, &v, delim, True) != tests[i].countEmpty ||
            VECTOR_Size(&v) != tests[i].countEmpty) {
            ret = TEST_ERR;
        }
        VECTOR_Clear(&v);
        if (STRING_Split(s, &v, delim, False) != tests[i].countNonEmpty ||
            VECTOR_Size(&v) != tests[i].countNonEmpty) {
            ret = TEST_ERR;
        }
    }
    VECTOR_Destroy(&v);
    return ret;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_str_alloc},
        {"IndexOf", test_str_indexof},
        {"Hash", test_str_hash},
        {"Dup", test_str_dup},
        {"StartsWith", test_str_startswith},
        {"EndsWith", test_str_endswith},
        {"Unicode", test_str_unicode},
        {"Format", test_str_format},
        {"Split", test_str_split}
    };

    int ret;
    setlocale(LC_ALL, "C");
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
