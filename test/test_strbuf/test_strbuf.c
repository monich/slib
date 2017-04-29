/*
 * $Id: test_strbuf.c,v 1.2 2016/10/02 22:55:25 slava Exp $
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
test_strbuf_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    StrBuf sb1;
    StrBuf16 sb2;
    int i;
    const char* str200 =
        TEXT("01234567890123456789012345678901234567890123456789")
        TEXT("01234567890123456789012345678901234567890123456789")
        TEXT("01234567890123456789012345678901234567890123456789")
        TEXT("01234567890123456789012345678901234567890123456789");

    STRBUF_Init(&sb1);
    STRBUF_InitBufXXX(&sb2);

    /* Test NULL resistance */
    STRBUF_Destroy(NULL);
    STRBUF_Delete(NULL);
    STRBUF_InitBuf2(NULL, NULL, 0, 0);
    STRBUF_SetLength(NULL, 0);
    STRBUF_Clear(NULL);
    STRBUF_Erase(NULL, 0, 0);
    if (STRBUF_Length(NULL) ||
        STRBUF_Dup(NULL) ||
        STRBUF_CopyBuf(NULL, NULL) ||
        STRBUF_Copy(NULL, NULL) ||
        STRBUF_CopyN(NULL, NULL, 0) ||
        STRBUF_Append(NULL, NULL) ||
        STRBUF_AppendBuf(NULL, NULL) ||
        STRBUF_CharAt(NULL, 0) ||
        STRBUF_FirstChar(NULL) ||
        STRBUF_LastChar(NULL) ||
        StrLen(STRBUF_GetString(NULL)) ||
        STRBUF_GetString(NULL) != STRBUF_GetString(&sb1) ||
        STRBUF_Replace(NULL, 0, 0) ||
        STRBUF_StartsWith(NULL, NULL) ||
        STRBUF_EndsWith(NULL, NULL) ||
        STRBUF_StartsWithNoCase(NULL, NULL) ||
        STRBUF_EndsWithNoCase(NULL, NULL) ||
        STRBUF_IndexOf(NULL, 0) >= 0 ||
        STRBUF_LastIndexOf(NULL, 0) >= 0) {
        ret = TEST_ERR;
    }

    /* These two NULL's are equal */
    if (!STRBUF_Equals(NULL, NULL) ||
        !STRBUF_EqualsNoCase(NULL, NULL)) {
        ret = TEST_ERR;
    }

    /* And these are not */
    if (STRBUF_EqualsTo(NULL, NULL) ||
        STRBUF_EqualsToNoCase(NULL, NULL)) {
        ret = TEST_ERR;
    }
 
    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    if (STRBUF_Create()) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRBUF_Alloc(&sb1, 1)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRBUF_AppendDouble(&sb1, 0.0)) {
        ret = TEST_ERR;
    }

    STRBUF_Copy(&sb2.sb, TEXT("test"));
    STRBUF_Alloc(&sb2.sb, 20);
    testMem.failAt = testMem.allocCount;
    if (!STRBUF_EqualsTo(&sb2.sb, TEXT("test")) ||
        STRBUF_Alloc(&sb2.sb, 40)) {
        ret = TEST_ERR;
    }

    STRBUF_Copy(&sb1, TEXT("a"));
    testMem.failAt = testMem.allocCount;
    if (STRBUF_Alloc(&sb1, 10)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRBUF_Inflate(&sb1, 10, 'b') ||
        !STRBUF_EqualsTo(&sb1, TEXT("a"))) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRBUF_Insert(&sb1, TEXT("bbb"), 2) ||
        STRBUF_Insert(&sb1, TEXT("bbb"), 1) ||
        !STRBUF_EqualsTo(&sb1, TEXT("a"))) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRBUF_CopyBuf(&sb1, &sb2.sb) ||
        !STRBUF_EqualsTo(&sb1, TEXT("a"))) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRBUF_AppendBuf(&sb1, &sb2.sb) ||
        !STRBUF_EqualsTo(&sb1, TEXT("a"))) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRBUF_AppendN(&sb1, TEXT("bbbbbbbb"), 6) ||
        !STRBUF_EqualsTo(&sb1, TEXT("a"))) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount;
    if (STRBUF_AppendInt(&sb1, 12345678) ||
        !STRBUF_EqualsTo(&sb1, TEXT("a"))) {
        ret = TEST_ERR;
    }

    STRBUF_Destroy(&sb1);
    STRBUF_Init(&sb1);
    testMem.failAt = testMem.allocCount;
    if (STRBUF_FormatTime(&sb1, 0)) {
        ret = TEST_ERR;
    }

    for (i=0; i<2; i++) {
        STRBUF_Destroy(&sb1);
        STRBUF_Init(&sb1);
        testMem.failAt = testMem.allocCount + i;
        if (STRBUF_AppendFormat(&sb1, TEXT("%s"), str200) ||
            STRBUF_Length(&sb1) != 0) {
            ret = TEST_ERR;
        }
    }

    testMem.failAt = -1;
    STRBUF_Destroy(&sb1);
    STRBUF_Destroy(&sb2.sb);
    return ret;
}

static
TestStatus
test_strbuf_basic(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    StrBuf* sb1 = STRBUF_Create();
    StrBuf* sb2 = STRBUF_Create();
    Char* s = STRBUF_Dup(sb1);

    if (s[0] ||
        STRBUF_FirstChar(sb1) ||
        STRBUF_LastChar(sb1) ||
        STRBUF_CharAt(sb1, 0) ||
        !STRBUF_Equals(sb1, sb2) ||
        !STRBUF_EqualsTo(sb1, "") ||
        !STRBUF_EqualsToNoCase(sb1, "")) {
        ret = TEST_ERR;
    }

    STRBUF_CopyN(sb1, TEXT("abcd"), 3);
    STRBUF_SetLength(sb1, 4);
    if (STRBUF_FirstChar(sb1) != 'a' ||
        STRBUF_CharAt(sb1, 1) != 'b' ||
        STRBUF_LastChar(sb1) != 'c' ||
        STRBUF_GetString(sb1) != sb1->s) {
        ret = TEST_ERR;
    }

    if (STRBUF_Equals(sb1, NULL) ||
        STRBUF_Equals(sb1, sb2) ||
        STRBUF_EqualsTo(sb1, "") ||
        !STRBUF_Equals(sb1, sb1) ||
        !STRBUF_EqualsTo(sb1, sb1->s)) {
        ret = TEST_ERR;
    }

    if (!STRBUF_Inflate(sb1, 2, 0) ||
        !STRBUF_Inflate(sb1, 2, 'd') ||
        !STRBUF_EqualsTo(sb1, TEXT("abc"))) {
        ret = TEST_ERR;
    }

    if (!STRBUF_Inflate(sb1, 4, 'd') ||
        !STRBUF_EqualsTo(sb1, TEXT("abcd"))) {
        ret = TEST_ERR;
    }

    if (STRBUF_InsertChar(sb1, 0, 0) ||
        !STRBUF_Insert(sb1, TEXT(""), 0) ||
        !STRBUF_InsertChar(sb1, '0', 0) ||
        !STRBUF_EqualsTo(sb1, TEXT("0abcd")) ||
        !STRBUF_InsertChar(sb1, '1', 1) ||
        !STRBUF_EqualsTo(sb1, TEXT("01abcd"))) {
        ret = TEST_ERR;
    }

    STRBUF_Erase(sb1, 0, 2);
    if (!STRBUF_EqualsTo(sb1, TEXT("abcd"))) {
        ret = TEST_ERR;
    }

    /* Invalid ranges */
    STRBUF_Erase(sb1, 5, 6);
    if (!STRBUF_EqualsTo(sb1, TEXT("abcd"))) {
        ret = TEST_ERR;
    }

    STRBUF_Erase(sb1, 3, 5);
    if (!STRBUF_EqualsTo(sb1, TEXT("abc"))) {
        ret = TEST_ERR;
    }

    STRBUF_Copy(sb2, TEXT("aBc"));
    if (!STRBUF_EqualsToNoCase(sb1, TEXT("AbC")) ||
        !STRBUF_EqualsToNoCase(sb1, sb1->s) ||
        !STRBUF_EqualsNoCase(sb2, sb1) ||
        !STRBUF_EqualsNoCase(sb1, sb2)) {
        ret = TEST_ERR;
    }

    if (!STRBUF_CopyBuf(sb2, sb2) ||
        !STRBUF_CopyBuf(sb2, sb1) ||
        !STRBUF_Equals(sb1, sb2)) {
        ret = TEST_ERR;
    }

    STRBUF_SetLength(sb2, 2);
    if (STRBUF_Equals(sb1, sb2)) {
        ret = TEST_ERR;
    }

    if (STRBUF_Trim(sb1)) {
        ret = TEST_ERR;
    }

    STRBUF_Clear(sb1);
    if (STRBUF_Trim(sb1)) {
        ret = TEST_ERR;
    }

    STRBUF_Copy(sb1, TEXT(" abc "));
    if (!STRBUF_Trim(sb1) ||
        !STRBUF_EqualsTo(sb1, TEXT("abc"))) {
        ret = TEST_ERR;
    }

    STRBUF_Copy(sb1, TEXT("  "));
    if (!STRBUF_Trim(sb1) ||
        STRBUF_Length(sb1)) {
        ret = TEST_ERR;
    }

    STRBUF_Copy(sb1, TEXT("abc"));
    STRBUF_Copy(sb2, TEXT("def"));
    if (!STRBUF_AppendN(sb1, TEXT("gh"), 0) ||
        !STRBUF_AppendBuf(sb1, sb2) ||
        !STRBUF_EqualsTo(sb1, TEXT("abcdef"))) {
        ret = TEST_ERR;
    }

    /* This will append 2 characters even though we ask for 3 */
    if (!STRBUF_AppendN(sb1, TEXT("gh"), 3) ||
        !STRBUF_EqualsTo(sb1, TEXT("abcdefgh"))) {
        ret = TEST_ERR;
    }

    STRBUF_Clear(sb1);
    STRBUF_Clear(sb2);
    if (!STRBUF_Equals(sb1, sb2)) {
        ret = TEST_ERR;
    }

    MEM_Free(s);
    STRBUF_Delete(sb1);
    STRBUF_Delete(sb2);
    return ret;
}

static
TestStatus
test_strbuf_indexof(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    StrBuf* sb = STRBUF_Create();

    if (STRBUF_IndexOf(sb, 'a') >= 0 ||
        STRBUF_LastIndexOf(sb, 'a') >= 0) {
        ret = TEST_ERR;
    }

    STRBUF_Copy(sb, "abcabc");
    if (STRBUF_IndexOf(sb, 'a') != 0 ||
        STRBUF_LastIndexOf(sb, 'a') != 3 ||
        STRBUF_IndexOf(sb, 'c') != 2 ||
        STRBUF_LastIndexOf(sb, 'c') != 5) {
        ret = TEST_ERR;
    }

    if (STRBUF_IndexOf(sb, 'd') >= 0 ||
        STRBUF_LastIndexOf(sb, 'd') >= 0 ||
        STRBUF_IndexOf(sb, 0) >= 0 ||
        STRBUF_LastIndexOf(sb, 0) >= 0) {
        ret = TEST_ERR;
    }

    STRBUF_Delete(sb);
    return ret;
}

static
TestStatus
test_strbuf_find(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    StrBuf* sb = STRBUF_Create();

    if (STRBUF_Find(sb, TEXT("")) >= 0 ||
        STRBUF_FindNoCase(sb, TEXT("")) >= 0 ||
        STRBUF_FindLast(sb, TEXT("")) >= 0 ||
        STRBUF_FindLastNoCase(sb, TEXT("")) >= 0) {
        ret = TEST_ERR;
    }

    STRBUF_Copy(sb, TEXT("abcabcabc"));
    if (STRBUF_Find(sb, NULL) >= 0 ||
        STRBUF_Find(sb, TEXT("")) != 0 ||
        STRBUF_Find(sb, TEXT("abcd")) >= 0 ||
        STRBUF_Find(sb, TEXT("abc")) != 0 ||
        STRBUF_FindFrom(sb, 1, TEXT("abc")) != 3) {
        ret = TEST_ERR;
    }

    if (STRBUF_FindLast(sb, NULL) >= 0 ||
        STRBUF_FindLast(sb, TEXT("")) >= 0 ||
        STRBUF_FindLast(sb, TEXT("abcd")) >= 0 ||
        STRBUF_FindLast(sb, TEXT("abcabcabcd")) >= 0 ||
        STRBUF_FindLast(sb, TEXT("cab")) != 5 ||
        STRBUF_FindLast(sb, TEXT("abc")) != 6) {
        ret = TEST_ERR;
    }

    STRBUF_Copy(sb, TEXT("abCaBcAbc"));
    if (STRBUF_Find(sb, TEXT("abc")) >= 0 ||
        STRBUF_FindNoCase(sb, TEXT("abc")) != 0) {
        ret = TEST_ERR;
    }
    if (STRBUF_FindFromNoCase(sb, 10, TEXT("abc")) >= 0 ||
        STRBUF_FindFromNoCase(sb, 1, NULL) >= 0 ||
        STRBUF_FindFromNoCase(sb, 1, TEXT("")) != 0 ||
        STRBUF_FindFromNoCase(sb, 8, TEXT("cab")) >= 0 ||
        STRBUF_FindFromNoCase(sb, 1, TEXT("abcd")) >= 0 ||
        STRBUF_FindFromNoCase(sb, 1, TEXT("abc")) != 3) {
        ret = TEST_ERR;
    }
    if (STRBUF_FindLastNoCase(sb, NULL) >= 0 ||
        STRBUF_FindLastNoCase(sb, TEXT("")) >= 0 ||
        STRBUF_FindLastNoCase(sb, TEXT("abcd")) >= 0 ||
        STRBUF_FindLastNoCase(sb, TEXT("abcabcabcd")) >= 0 ||
        STRBUF_FindLastNoCase(sb, TEXT("abc")) != 6) {
        ret = TEST_ERR;
    }

    STRBUF_Delete(sb);
    return ret;
}

static
TestStatus
test_strbuf_replace(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    StrBuf* sb = STRBUF_Create();

    STRBUF_Copy(sb, TEXT("abcabcabc"));
    if (STRBUF_Replace(sb, 'd', 0) != 0 ||
        STRBUF_Replace(sb, 0, 'd') != 0 ||
        STRBUF_Replace(sb, 'd', 'e') != 0 ||
        STRBUF_Replace(sb, 'a', 'd') != 3 ||
        !STRBUF_EqualsTo(sb, TEXT("dbcdbcdbc"))) {
        ret = TEST_ERR;
    }

    if (STRBUF_ReplaceStr(sb, TEXT("aaa"), TEXT("ab")) != 0 ||
        STRBUF_ReplaceStr(sb, TEXT("dbc"), TEXT("abc")) != 3 ||
        !STRBUF_EqualsTo(sb, TEXT("abcabcabc"))) {
        ret = TEST_ERR;
    }

    if (STRBUF_ReplaceStr(sb, TEXT("abc"), TEXT("abcd")) != 3 || 
        !STRBUF_EqualsTo(sb, TEXT("abcdabcdabcd"))) {
        ret = TEST_ERR;
    }

    if (STRBUF_ReplaceStr(sb, TEXT("cd"), TEXT("")) != 3 || 
        !STRBUF_EqualsTo(sb, TEXT("ababab"))) {
        ret = TEST_ERR;
    }

    if (STRBUF_ReplaceStr(sb, TEXT("bab"), TEXT("c")) != 1 || 
        !STRBUF_EqualsTo(sb, TEXT("acab"))) {
        ret = TEST_ERR;
    }

    if (STRBUF_ReplaceStr(sb, TEXT("a"), TEXT("d")) != 2 || 
        !STRBUF_EqualsTo(sb, TEXT("dcdb"))) {
        ret = TEST_ERR;
    }

    if (STRBUF_ReplaceStr(sb, TEXT(""), TEXT("")) != 0) {
       ret = TEST_ERR;
    }

    STRBUF_Delete(sb);
    return ret;
}

static
TestStatus
test_strbuf_start(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    StrBuf* sb = STRBUF_Create();

    if (!STRBUF_StartsWith(sb, TEXT("")) ||
        !STRBUF_StartsWithNoCase(sb, TEXT(""))) {
        ret = TEST_ERR;
    }

    STRBUF_Copy(sb, TEXT("abcabcab"));
    if (!STRBUF_StartsWith(sb, TEXT("")) ||
        !STRBUF_StartsWith(sb, TEXT("abc")) ||
        !STRBUF_StartsWith(sb, TEXT("abcabcab")) ||
        !STRBUF_StartsWithNoCase(sb, TEXT("")) ||
        !STRBUF_StartsWithNoCase(sb, TEXT("abC")) ||
        !STRBUF_StartsWithNoCase(sb, TEXT("AbC"))) {
        ret = TEST_ERR;
    }

    if (STRBUF_StartsWith(sb, TEXT("bcd")) ||
        STRBUF_StartsWith(sb, TEXT("abd")) ||
        STRBUF_StartsWith(sb, TEXT("abcabcabc")) ||
        STRBUF_StartsWithNoCase(sb, TEXT("bCd")) ||
        STRBUF_StartsWithNoCase(sb, TEXT("AbcAbcAbc"))) {
        ret = TEST_ERR;
    }

    STRBUF_Delete(sb);
    return ret;
}

static
TestStatus
test_strbuf_end(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    StrBuf* sb = STRBUF_Create();

    STRBUF_Copy(sb, TEXT("abcabcab"));
    if (!STRBUF_EndsWith(sb, TEXT("cab")) ||
        !STRBUF_EndsWithNoCase(sb, TEXT("Cab"))) {
        ret = TEST_ERR;
    }

    if (STRBUF_EndsWith(sb, TEXT("abc")) ||
        STRBUF_EndsWith(sb, TEXT("dabcabcab")) ||
        STRBUF_EndsWith(sb, TEXT("def")) ||
        STRBUF_EndsWithNoCase(sb, TEXT("Abc")) ||
        STRBUF_EndsWithNoCase(sb, TEXT("def")) ||
        STRBUF_EndsWithNoCase(sb, TEXT("dAbcAbcAb"))) {
        ret = TEST_ERR;
    }

    STRBUF_Delete(sb);
    return ret;
}

static
TestStatus
test_strbuf_format(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    StrBuf* sb = STRBUF_Create();
    const char* str100 =
        TEXT("01234567890123456789012345678901234567890123456789")
        TEXT("01234567890123456789012345678901234567890123456789");
    const char* str200 =
        TEXT("01234567890123456789012345678901234567890123456789")
        TEXT("01234567890123456789012345678901234567890123456789")
        TEXT("01234567890123456789012345678901234567890123456789")
        TEXT("01234567890123456789012345678901234567890123456789");

    if (!STRBUF_FormatTime(sb, 0) ||
        !(STRBUF_EqualsTo(sb, TEXT("Thu Jan  1 00:00:00 1970 UTC")) ||
          STRBUF_EqualsTo(sb, TEXT("Thu Jan 01 00:00:00 1970 UTC")))) {
        ret = TEST_ERR;
    }

    STRBUF_Clear(sb);
    if (!STRBUF_AppendBool(sb, True) ||
        !STRBUF_EqualsTo(sb, TRUE_STRING)) {
        ret = TEST_ERR;
    }

    STRBUF_Clear(sb);
    if (!STRBUF_AppendBool(sb, False) ||
        !STRBUF_EqualsTo(sb, FALSE_STRING)) {
        ret = TEST_ERR;
    }

    STRBUF_Clear(sb);
    if (!STRBUF_AppendDouble(sb, 0.0) ||
        !STRBUF_EqualsTo(sb, TEXT("0.0"))) {
        ret = TEST_ERR;
    }

    STRBUF_Clear(sb);
    if (!STRBUF_AppendFormat(sb, TEXT("%s"), TEXT("test")) ||
        !STRBUF_EqualsTo(sb, TEXT("test"))) {
        ret = TEST_ERR;
    }

    STRBUF_Clear(sb);
    if (!STRBUF_AppendFormat(sb, TEXT("%s"), str100) ||
        !STRBUF_EqualsTo(sb, str100)) {
        ret = TEST_ERR;
    }

    STRBUF_SetLength(sb, 0);
    if (!STRBUF_AppendFormat(sb, TEXT("%s"), str200) ||
        !STRBUF_EqualsTo(sb, str200)) {
        ret = TEST_ERR;
    }

    STRBUF_Delete(sb);
    return ret;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_strbuf_alloc},
        {"Basic", test_strbuf_basic},
        {"IndexOf", test_strbuf_indexof},
        {"Find", test_strbuf_find},
        {"Replace", test_strbuf_replace},
        {"StartsWith", test_strbuf_start},
        {"EndsWith", test_strbuf_end},
        {"Format", test_strbuf_format}
    };

    int ret;
    setlocale(LC_ALL, "C");
#ifdef _WIN32
    _putenv("TZ=UTC");
#else
    setenv("TZ", "UTC", True);
#endif
    tzset();
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
