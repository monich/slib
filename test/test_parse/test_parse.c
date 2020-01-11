/*
 * $Id: test_parse.c,v 1.3 2020/01/11 18:21:35 slava Exp $
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

static
TestStatus
test_parse_bool(
    const TestDesc* test)
{
    static const struct _test_parse_bool {
        Str str;
        int flags;
#define TEST_BOOL_RESULT_TRUE  (0x01) 
#define TEST_BOOL_RESULT_FALSE (0x02) 
#define TEST_BOOL_RESULT_ANY   (TEST_BOOL_RESULT_TRUE|TEST_BOOL_RESULT_FALSE)
    } tests[] = {
        { NULL, 0 },
        { T_(""), 0 },
        { T_(" "), 0 },
        { T_(" maybe "), 0 },
        { T_("true x"), 0 },
        { T_("false y"), 0 },
        { T_("true"), TEST_BOOL_RESULT_TRUE },
        { T_(" True "), TEST_BOOL_RESULT_TRUE },
        { T_("false"), TEST_BOOL_RESULT_FALSE },
        { T_(" False "), TEST_BOOL_RESULT_FALSE },
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        Bool b;
        Str str = tests[i].str;
        const int flags = tests[i].flags;
        if (PARSE_Bool(str, &b)) {
            TEST_ASSERT(PARSE_Bool(str, NULL));
            if (b) {
                TEST_ASSERT(flags & TEST_BOOL_RESULT_TRUE);
            } else {
                TEST_ASSERT(flags & TEST_BOOL_RESULT_FALSE);
            }
        } else {
            TEST_ASSERT(!(flags & TEST_BOOL_RESULT_ANY));
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_i8(
    const TestDesc* test)
{
    static const struct _test_parse_byte {
        Str str;
        int base;
        Bool ok;
        I8s result;
    } tests[] = {
        { NULL, 10, False },
        { T_(""), 10, False },
        { T_(" "), 10, False },
        { T_(" foo "), 10, False },
        { T_("128"), 10, False },
        { T_("ff"), 16, False },
        { T_("-129"), 10, False },
        { T_("127"), 10, True, 127 },
        { T_("-128"), 10, True, -128 },
        { T_(" -12 "), 10, True, -12 },
        { T_("0x1f"), 16, True, 0x1f },
        { T_("0x1f"), 10, False }
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        I8s x;
        const int base = tests[i].base;
        Str str = tests[i].str;
        if (PARSE_I8(str, &x, base)) {
            TEST_ASSERT(PARSE_I8(str, NULL, base));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_u8(
    const TestDesc* test)
{
    static const struct _test_parse_ubyte {
        Str str;
        int base;
        Bool ok;
        I8u result;
    } tests[] = {
        { NULL, 10, False },
        { T_(""), 10, False },
        { T_(" "), 10, False },
        { T_(" foo "), 10, False },
        { T_("128"), 10, True, 128 },
        { T_("ff"), 16, True, 255 },
        { T_("256"), 10, False },
        { T_("-129"), 10, False },
        { T_("127"), 10, True, 127 },
        { T_("-128"), 10, False },
        { T_(" -12 "), 10, False },
        { T_("0x1f"), 16, True, 0x1f },
        { T_("0x1f"), 10, False }
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        I8u x;
        const int base = tests[i].base;
        Str str = tests[i].str;
        if (PARSE_U8(str, &x, base)) {
            TEST_ASSERT(PARSE_U8(str, NULL, base));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_i16(
    const TestDesc* test)
{
    static const struct _test_parse_short {
        Str str;
        int base;
        Bool ok;
        I16s result;
    } tests[] = {
        { NULL, 10, False },
        { T_(""), 10, False },
        { T_(" "), 10, False },
        { T_(" foo "), 10, False },
        { T_("128"), 10, True, 128 },
        { T_("ff"), 16, True, 0xff },
        { T_("0xffff"), 16, False },
        { T_("65536"), 10, False },
        { T_("32767"), 10, True, 32767 },
        { T_("-32768"), 10, True, -32768 }
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        I16s x;
        const int base = tests[i].base;
        Str str = tests[i].str;
        if (PARSE_I16(str, &x, base)) {
            TEST_ASSERT(PARSE_I16(str, NULL, base));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_u16(
    const TestDesc* test)
{
    static const struct _test_parse_ushort {
        Str str;
        int base;
        Bool ok;
        I16u result;
    } tests[] = {
        { NULL, 10, False },
        { T_(""), 10, False },
        { T_(" "), 10, False },
        { T_(" foo "), 10, False },
        { T_("128"), 10, True, 128 },
        { T_("ff"), 16, True, 0xff },
        { T_("0xffff"), 16, True, 0xffff },
        { T_("65536"), 10, False },
        { T_("32767"), 10, True, 32767 },
        { T_("-32768"), 10, False }
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        I16u x;
        const int base = tests[i].base;
        Str str = tests[i].str;
        if (PARSE_U16(str, &x, base)) {
            TEST_ASSERT(PARSE_U16(str, NULL, base));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_i32(
    const TestDesc* test)
{
    static const struct _test_parse_short {
        Str str;
        int base;
        Bool ok;
        I32s result;
    } tests[] = {
        { NULL, 10, False },
        { T_(""), 10, False },
        { T_(" "), 10, False },
        { T_(" foo "), 10, False },
        { T_("0xff"), 16, True, 0xff },
        { T_("ffff"), 16, True, 0xffff },
        { T_(" ffff "), 16, True, 0xffff },
        { T_(" ffff x"), 16, False },
        { T_(" ffffx"), 16, False },
        { T_("7fffffff"), 16, True, 0x7fffffff },
        { T_("ffffffff"), 16, False },
        { T_("-2147483648"), 10, True, -2147483647-1 },
        { T_("-1"), 10, True, 0xffffffff },
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        I32s x;
        const int base = tests[i].base;
        Str str = tests[i].str;
        if (PARSE_I32(str, &x, base)) {
            TEST_ASSERT(PARSE_I32(str, NULL, base));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_u32(
    const TestDesc* test)
{
    static const struct _test_parse_short {
        Str str;
        int base;
        Bool ok;
        I32u result;
    } tests[] = {
        { NULL, 10, False },
        { T_(""), 10, False },
        { T_(" "), 10, False },
        { T_(" foo "), 10, False },
        { T_("0xff"), 16, True, 0xff },
        { T_("ffff"), 16, True, 0xffff },
        { T_(" ffff "), 16, True, 0xffff },
        { T_(" ffff x"), 16, False },
        { T_(" ffffx"), 16, False },
        { T_("7fffffff"), 16, True, 0x7fffffff },
        { T_("ffffffff"), 16, True, 0xffffffff },
        { T_("4294967295"), 10, True, 4294967295 },
        { T_("ffffffffff"), 16, False },
        { T_("-1"), 10, False },
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        I32u x;
        const int base = tests[i].base;
        Str str = tests[i].str;
        if (PARSE_U32(str, &x, base)) {
            TEST_ASSERT(PARSE_U32(str, NULL, base));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_i64(
    const TestDesc* test)
{
    static const struct _test_parse_short {
        Str str;
        int base;
        Bool ok;
        I64s result;
    } tests[] = {
        { NULL, 10, False },
        { T_(""), 10, False },
        { T_(" "), 10, False },
        { T_(" foo "), 10, False },
        { T_("0xff"), 16, True, __INT64_C(0xff) },
        { T_("ffff"), 16, True, __INT64_C(0xffff) },
        { T_("7fffffff"), 16, True, __INT64_C(0x7fffffff) },
        { T_(" 7fffffff "), 16, True, __INT64_C(0x7fffffff) },
        { T_(" 7fffffffx "), 16, False},
        { T_(" 7fffffff x "), 16, False},
        { T_("7fffffffffffffff"), 16, True, __INT64_C(0x7fffffffffffffff) },
        { T_("ffffffffffffffffffffffffffffffff"), 16, False },
        { T_("-1"), 10, True, __INT64_C(-1) },
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        I64s x;
        const int base = tests[i].base;
        Str str = tests[i].str;
        if (PARSE_I64(str, &x, base)) {
            TEST_ASSERT(PARSE_I64(str, NULL, base));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_u64(
    const TestDesc* test)
{
    static const struct _test_parse_short {
        Str str;
        int base;
        Bool ok;
        I64u result;
    } tests[] = {
        { NULL, 10, False },
        { T_(""), 10, False },
        { T_(" "), 10, False },
        { T_(" foo "), 10, False },
        { T_("0xff"), 16, True, __UINT64_C(0xff) },
        { T_("ffff"), 16, True, __UINT64_C(0xffff) },
        { T_("7fffffff"), 16, True, __UINT64_C(0x7fffffff) },
        { T_(" 7fffffff "), 16, True, __UINT64_C(0x7fffffff) },
        { T_(" 7fffffffx "), 16, False },
        { T_(" 7fffffff x "), 16, False },
        { T_("7fffffffffffffff"), 16, True, __UINT64_C(0x7fffffffffffffff) },
        { T_("ffffffffffffffff"), 16, True, __UINT64_C(0xffffffffffffffff) },
        { T_("fffffffffffffffff"), 16, False },
        { T_("-1"), 10, False },
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        I64u x;
        const int base = tests[i].base;
        Str str = tests[i].str;
        if (PARSE_U64(str, &x, base)) {
            TEST_ASSERT(PARSE_U64(str, NULL, base));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_long(
    const TestDesc* test)
{
    static const struct _test_parse_short {
        Str str;
        int base;
        Bool ok;
        long result;
    } tests[] = {
        { NULL, 10, False },
        { T_(""), 10, False },
        { T_(" "), 10, False },
        { T_(" foo "), 10, False },
        { T_("0xff"), 16, True, 0xff },
        { T_("ffff"), 16, True, 0xffff },
        { T_(" ffff "), 16, True, 0xffff },
        { T_(" ffff x"), 16, False },
        { T_("ffffffffffffffffffff"), 16, False },
        { T_(" ffffx"), 16, False }
    };

    /* Long can be 32 or 64 bits */
    int i;
    for (i=0; i<COUNT(tests); i++) {
        long x;
        const int base = tests[i].base;
        Str str = tests[i].str;
        if (PARSE_Long(str, &x, base)) {
            TEST_ASSERT(PARSE_Long(str, NULL, base));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_ulong(
    const TestDesc* test)
{
    static const struct _test_parse_short {
        Str str;
        int base;
        Bool ok;
        unsigned long result;
    } tests[] = {
        { NULL, 10, False },
        { T_(""), 10, False },
        { T_(" "), 10, False },
        { T_(" foo "), 10, False },
        { T_("0xff"), 16, True, 0xff },
        { T_("ffff"), 16, True, 0xffff },
        { T_(" ffff "), 16, True, 0xffff },
        { T_(" ffff x"), 16, False },
        { T_(" ffffx"), 16, False },
        { T_("ffffffffffffffffffff"), 16, False },
        { T_(" -1 "), 16, False }
    };

    /* Long can be 32 or 64 bits */
    int i;
    for (i=0; i<COUNT(tests); i++) {
        unsigned long x;
        const int base = tests[i].base;
        Str str = tests[i].str;
        if (PARSE_ULong(str, &x, base)) {
            TEST_ASSERT(PARSE_ULong(str, NULL, base));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_float(
    const TestDesc* test)
{
    static const struct _test_parse_short {
        Str str;
        Bool ok;
        float result;
    } tests[] = {
        { NULL, False },
        { T_(""), False },
        { T_(" "), False },
        { T_(" x "), False },
        { T_(" 0x "), False },
        { T_(" 0 x "), False },
        { T_(" 0.. "), False },
        { T_("1"), True, 1 },
        { T_("1.5"), True, 1.5 },
        { T_("0"), True, 0 },
        { T_("-0"), True, 0 },
        { T_("1e8"), True, 1e8 },
        { T_("1e88"), False },
        { T_("1e888"), False },
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        float x;
        Str str = tests[i].str;
        if (PARSE_Float(str, &x)) {
            TEST_ASSERT(PARSE_Float(str, NULL));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

static
TestStatus
test_parse_double(
    const TestDesc* test)
{
    static const struct _test_parse_short {
        Str str;
        Bool ok;
        double result;
    } tests[] = {
        { NULL, False },
        { T_(""), False },
        { T_(" "), False },
        { T_(" x "), False },
        { T_(" 0x "), False },
        { T_(" 0 x "), False },
        { T_(" 0.. "), False },
        { T_("1"), True, 1 },
        { T_("1.5"), True, 1.5 },
        { T_("0"), True, 0 },
        { T_("-0"), True, 0 },
        { T_("1e8"), True, 1e8 },
        { T_("1e888"), False },
    };

    int i;
    for (i=0; i<COUNT(tests); i++) {
        double x;
        Str str = tests[i].str;
        if (PARSE_Double(str, &x)) {
            TEST_ASSERT(PARSE_Double(str, NULL));
            TEST_ASSERT(tests[i].result == x);
            TEST_ASSERT(tests[i].ok);
        } else {
            TEST_ASSERT(!tests[i].ok);
        }
    }
    return TEST_OK;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Bool", test_parse_bool},
        {"I8", test_parse_i8},
        {"U8", test_parse_u8},
        {"I16", test_parse_i16},
        {"U16", test_parse_u16},
        {"I32", test_parse_i32},
        {"U32", test_parse_u32},
        {"I64", test_parse_i64},
        {"U64", test_parse_u64},
        {"Long", test_parse_long},
        {"ULong", test_parse_ulong},
        {"Float", test_parse_float},
        {"Double", test_parse_double}
    };

    return TEST_MAIN(argc, argv, tests);
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
