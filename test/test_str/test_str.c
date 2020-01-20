/*
 * $Id: test_str.c,v 1.6 2020/01/20 22:22:05 slava Exp $
 *
 * Copyright (C) 2016-2020 by Slava Monich
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
    StrBuf* sb;
    Vector v;
    char* s;
    wchar_t* ws;
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
    TEST_ASSERT(!STRING_ToUnicode(NULL));
    TEST_ASSERT(!STRING_ToMultiByte(NULL));
    TEST_ASSERT(!STRING_ToMultiByteN(NULL, 0));
    TEST_ASSERT(!STRING_ToUnicode(NULL));
    TEST_ASSERT(!STRING_ToUnicodeN(NULL, 0));

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!STRING_Dup8("test"));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!STRING_DupU(L"test"));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!STRING_Format(NULL, 0, "%s", "test"));

    testMem.failAt = testMem.allocCount;
    s = STRING_Format(buf, COUNT(buf), "%s", "test");
    TEST_ASSERT(s == buf && !strcmp(s, "t"));

    testMem.failAt = testMem.allocCount;
    buf[0] = 'x';
    s = STRING_Format(buf, 0, "%s", "test");
    TEST_ASSERT(s == buf && buf[0] == 'x');

    testMem.failAt = testMem.allocCount + 1;
    s = STRING_Format(NULL, 0, "%s", longstr);
    TEST_ASSERT(s && !strcmp(s, longstr99));
    MEM_Free(s);

    testMem.failAt = testMem.allocCount;
    ws = STRING_ToUnicode("test"); /* The first failure is ignored */
    TEST_ASSERT(!STRING_CompareU(ws, L"test"));
    MEM_Free(ws);

    testMem.failAt = testMem.allocCount + 1; /* The second failure isn't */
    TEST_ASSERT(!STRING_ToUnicode("test"));

    testMem.failAt = testMem.allocCount + 1;
    TEST_ASSERT(!STRING_ToUnicode("\xD1\x82\xD0\xB5\xD1\x81\xD1\x82"));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!STRING_ToMultiByte(L"test"));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!STRING_ToMultiByte(wtest));

    sb = STRBUF_Create();

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!STRING_FormatFloat(sb, 0.1f));

    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(!STRING_FormatDouble(sb, 0.1));

    VECTOR_Init(&v, 0, vectorEqualsString, vectorFreeValueProc);

    /* Element has to be long enough not to fit into the buffer on stack */
    testMem.failAt = testMem.allocCount;
    TEST_ASSERT(STRING_Split("0123456789012345678901234567890123456789,2",
        NULL, ",", False) <= 0);

    for (i = 0; i < 4; i++) {
        testMem.failAt = testMem.allocCount + i;
        VECTOR_Trim(&v);
        TEST_ASSERT(STRING_Split(",", &v, ",", True) < 0);
        TEST_ASSERT(!VECTOR_Size(&v));
    }

    for (i = 0; i < 4; i++) {
        testMem.failAt = testMem.allocCount + i;
        VECTOR_Trim(&v);
        TEST_ASSERT(STRING_Split("1,2", &v, ",", False) < 0);
        TEST_ASSERT(!VECTOR_Size(&v));
    }

    for (i = 0; i < 7; i++) {
        testMem.failAt = testMem.allocCount + i;
        VECTOR_Trim(&v);
        TEST_ASSERT(STRING_Split(",1,2,", &v, ",", True) < 0);
        TEST_ASSERT(!VECTOR_Size(&v));
    }

    testMem.failAt = -1;
    STRBUF_Delete(sb);
    VECTOR_Destroy(&v);
    return TEST_OK;
}

static
TestStatus
test_str_indexof(
    const TestDesc* test)
{
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

    for (i = 0; i < COUNT(tests); i++) {
        Str s = tests[i].s;
        Char c = tests[i].c;

        TEST_ASSERT(STRING_IndexOf(s, c) == tests[i].first);
        TEST_ASSERT(STRING_LastIndexOf(s, c) == tests[i].last);
    }
    return TEST_OK;
}

static
TestStatus
test_str_hash(
    const TestDesc* test)
{
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

    for (i = 0; i < COUNT(tests); i++) {
        TEST_ASSERT(STRING_HashCode(tests[i].s) == tests[i].hash);
        TEST_ASSERT(STRING_HashCodeNoCase(tests[i].s) == tests[i].hashNoCase);
    }
    return TEST_OK;
}

static
TestStatus
test_str_compare(
    const TestDesc* test)
{
    TEST_ASSERT(STRING_Equal(NULL, NULL));

    TEST_ASSERT(STRING_Compare8("", "") == 0);
    TEST_ASSERT(STRING_Compare8("a", "a") == 0);
    TEST_ASSERT(STRING_Compare8("a", "b") < 0);
    TEST_ASSERT(STRING_Compare8("b", "a") > 0);
    TEST_ASSERT(STRING_Compare8("", NULL) > 0);
    TEST_ASSERT(STRING_Compare8(NULL, "") < 0);

    TEST_ASSERT(STRING_CompareU(L"", L"") == 0);
    TEST_ASSERT(STRING_CompareU(L"a", L"a") == 0);
    TEST_ASSERT(STRING_CompareU(L"a", L"b") < 0);
    TEST_ASSERT(STRING_CompareU(L"b", L"a") > 0);
    TEST_ASSERT(STRING_CompareU(L"", NULL) > 0);
    TEST_ASSERT(STRING_CompareU(NULL, L"") < 0);

    return TEST_OK;
}

static
TestStatus
test_str_dup(
    const TestDesc* test)
{
    char* s;
    wchar_t* ws;

    TEST_ASSERT(!STRING_Dup(NULL));
    TEST_ASSERT(!STRING_DupU(NULL));

    s = STRING_Dup8("test");
    TEST_ASSERT(s);
    TEST_ASSERT(!strcmp(s, "test"));

    ws = STRING_DupU(L"test");
    TEST_ASSERT(ws);
    TEST_ASSERT(!wcscmp(ws, L"test"));

    MEM_Free(s);
    MEM_Free(ws);
    return TEST_OK;
}

static
TestStatus
test_str_startswith(
    const TestDesc* test)
{
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

    for (i = 0; i < COUNT(tests); i++) {
        const char* s = tests[i].s;
        const char* p = tests[i].p;
        const wchar_t* ws = tests[i].ws;
        const wchar_t* wp = tests[i].wp;

        TEST_ASSERT(STRING_StartsWith8(s, p) == tests[i].starts);
        TEST_ASSERT(STRING_StartsWithNoCase8(s, p) == tests[i].startsNoCase);
        TEST_ASSERT(STRING_StartsWithU(ws, wp) == tests[i].starts);
        TEST_ASSERT(STRING_StartsWithNoCaseU(ws, wp) == tests[i].startsNoCase);
    }
    return TEST_OK;
}

static
TestStatus
test_str_endswith(
    const TestDesc* test)
{
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
    for (i = 0; i < COUNT(tests); i++) {
        const char* s = tests[i].s;
        const char* suffix = tests[i].suffix;
        const wchar_t* ws = tests[i].ws;
        const wchar_t* wsuffix = tests[i].wsuffix;

        TEST_ASSERT(STRING_EndsWith8(s, suffix) == tests[i].ends);
        TEST_ASSERT(STRING_EndsWithNoCase8(s, suffix) == tests[i].endsNoCase);
        TEST_ASSERT(STRING_EndsWithU(ws, wsuffix) == tests[i].ends);
        TEST_ASSERT(STRING_EndsWithNoCaseU(ws, wsuffix) == tests[i].endsNoCase);
    }
    return TEST_OK;
}

static
TestStatus
test_str_unicode(
    const TestDesc* test)
{
    static const wchar_t wtest[] = {0x0442, 0x0435, 0x0441, 0x0442, 0};
    wchar_t* ws;
    char* s;

    ws = STRING_ToUnicode("");
    TEST_ASSERT(ws);
    TEST_ASSERT(!ws[0]);
    MEM_Free(ws);

    ws = STRING_ToUnicode("test");
    TEST_ASSERT(ws);
    TEST_ASSERT(!wcscmp(ws, L"test"));
    MEM_Free(ws);

    s = STRING_ToMultiByte(L"test");
    TEST_ASSERT(s);
    TEST_ASSERT(!strcmp(s, "test"));
    MEM_Free(s);

    /* The rest may (and often does) fail on Windows */
#ifndef _WIN32
    ws = STRING_ToUnicode("\xD1\x82\xD0\xB5\xD1\x81\xD1\x82");
    TEST_ASSERT(ws);
    TEST_ASSERT(!wcscmp(ws, wtest));
    MEM_Free(ws);

    /* Invalid UTF-8 sequence */
    ws = STRING_ToUnicode("\xD1\x82\xD0\xB5\xD1\x81\xD1");
    TEST_ASSERT(!ws);

    s = STRING_ToMultiByte(wtest);
    TEST_ASSERT(s);
    TEST_ASSERT(!strcmp(s, "\xD1\x82\xD0\xB5\xD1\x81\xD1\x82"));
    MEM_Free(s);
#endif
    return TEST_OK;
}

static
TestStatus
test_str_format(
    const TestDesc* test)
{
    char buf2[2];
    char* s;
    StrBuf* sb;
    const char* longstr =
        "01234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789"
        "01234567890123456789012345678901234567890123456789";

    s = STRING_Format(buf2, COUNT(buf2), "%s", "test");
    TEST_ASSERT(s != buf2);
    TEST_ASSERT(s);
    TEST_ASSERT(!strcmp(s, "test"));
    MEM_Free(s);

    s = STRING_Format(NULL, 0, "%s", longstr);
    TEST_ASSERT(s);
    TEST_ASSERT(!strcmp(s, longstr));
    MEM_Free(s);

    sb = STRBUF_Create();
    TEST_ASSERT(!strcmp(STRING_FormatDouble(sb, 0.), "0.0"));
    TEST_ASSERT(!strcmp(STRING_FormatDouble(sb, 0.1), "0.1"));
    TEST_ASSERT(!strcmp(STRING_FormatDouble(sb, -0.1), "-0.1"));
    TEST_ASSERT(!strcmp(STRING_FormatDouble(sb, 3.14159265358979),
        "3.14159265358979"));
    TEST_ASSERT(!strcmp(STRING_FormatFloat(sb, 0.f), "0.0"));
    TEST_ASSERT(!strcmp(STRING_FormatFloat(sb, 0.1f), "0.1"));
    TEST_ASSERT(!strcmp(STRING_FormatFloat(sb, -0.1f), "-0.1"));
    TEST_ASSERT(!strcmp(STRING_FormatFloat(sb, 0.111f), "0.111"));
    TEST_ASSERT(!strcmp(STRING_FormatFloat(sb, 3.141592f), "3.141592"));

    STRBUF_Delete(sb);
    return TEST_OK;
}

static
TestStatus
test_str_split(
    const TestDesc* test)
{
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
    for (i = 0; i < COUNT(tests); i++) {
        Str s = tests[i].s, d = tests[i].delim;

        TEST_ASSERT(STRING_Split(s, NULL, d, True) == tests[i].countEmpty);
        TEST_ASSERT(STRING_Split(s, NULL, d, False) == tests[i].countNonEmpty);

        VECTOR_Clear(&v);
        TEST_ASSERT(STRING_Split(s, &v, d, True) == tests[i].countEmpty);
        TEST_ASSERT(VECTOR_Size(&v) == tests[i].countEmpty);

        VECTOR_Clear(&v);
        TEST_ASSERT(STRING_Split(s, &v, d, False) == tests[i].countNonEmpty);
        TEST_ASSERT(VECTOR_Size(&v) == tests[i].countNonEmpty);
    }
    VECTOR_Destroy(&v);
    return TEST_OK;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_str_alloc},
        {"IndexOf", test_str_indexof},
        {"Hash", test_str_hash},
        {"Compare", test_str_compare},
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
