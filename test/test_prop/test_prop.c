/*
 * $Id: test_prop.c,v 1.3 2020/01/23 00:25:04 slava Exp $
 *
 * Copyright (C) 2020 by Slava Monich
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
test_prop_alloc(
    const TestDesc* test)
{
    Prop* p;
    int i = 0;
    const Str k = T_("foo");
    const Str v = T_("bar");

    /* NULL resistance */
    PROP_Delete(NULL);

    /* Simulate allocation failures */
    for (i = 0; i < 2; i++) {
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!PROP_Create());
    }

    /* Assert that all possible allocation failures in PROP_Create()
     * have been exhausted. */
    testMem.failAt = testMem.allocCount + i;
    TEST_ASSERT((p = PROP_Create()) != NULL);

    for (i = 0; i < 4; i++) {
        testMem.failAt = testMem.allocCount + i;
        TEST_ASSERT(!PROP_Set(p, k, v));
    }

    /* Assert that all possible allocation failures in PROP_Set()
     * have been exhausted. */
    testMem.failAt = testMem.allocCount + i;
    TEST_ASSERT(PROP_Set(p, k, v));

    testMem.failAt = -1;
    PROP_Delete(p);
    return TEST_OK;
}

static
TestStatus
test_prop_basic(
    const TestDesc* test)
{
    Prop* p = PROP_Create();
    const Str skey = T_("foo");
    const Str sval = T_("bar");
    const Str ikey = T_("int");
    const int ival = INT_MAX;
    int i = 0;
    const Str uikey = T_("uint");
    const unsigned int uival = UINT_MAX;
    unsigned int ui = 0;
    const Str lkey = T_("long");
    const long lval = LONG_MAX;
    long l = 0;
    const Str ulkey = T_("ulong");
    const unsigned long ulval = ULONG_MAX;
    unsigned long ul = 0;
    const Str bkey = T_("bool");
    const Bool bval = True;
    Bool b = False;
    const Str dkey = T_("double");
    const double dval = 1.0;
    double d = 0.0;
    const Str nosuchkey = T_("nosuchkey");
    const Str empty = T_("");
    const Str comment = T_("comment");

    TEST_ASSERT(!PROP_Size(p));
    TEST_ASSERT(!PROP_Get(p, nosuchkey));             /* Missing key */
    TEST_ASSERT(!PROP_GetComment(p, nosuchkey));

    /* Setting non-empty comment creates empty value */
    TEST_ASSERT(PROP_SetComment(p, skey, NULL));
    TEST_ASSERT(!PROP_Get(p, skey));
    TEST_ASSERT(PROP_SetComment(p, skey, comment));
    TEST_ASSERT(STRING_Equal(PROP_Get(p, skey), empty));
    TEST_ASSERT(STRING_Equal(PROP_GetComment(p, skey), comment));
    TEST_ASSERT(PROP_Size(p) == 1);

    TEST_ASSERT(PROP_Set(p, skey, empty));            /* Same thing again */
    TEST_ASSERT(STRING_Equal(PROP_Get(p, skey), empty));
    TEST_ASSERT(PROP_Set(p, skey, sval));             /* Replace the value */
    TEST_ASSERT(STRING_Equal(PROP_Get(p, skey), sval));

    TEST_ASSERT(!PROP_GetInt(p, nosuchkey, NULL));    /* Missing key */
    TEST_ASSERT(PROP_SetInt(p, ikey, ival));
    TEST_ASSERT(PROP_GetInt(p, ikey, NULL));
    TEST_ASSERT(PROP_GetInt(p, ikey, &i));
    TEST_ASSERT(i == ival);

    TEST_ASSERT(!PROP_GetUInt(p, nosuchkey, NULL));   /* Missing key */
    TEST_ASSERT(PROP_SetUInt(p, uikey, uival));
    TEST_ASSERT(PROP_GetUInt(p, uikey, NULL));
    TEST_ASSERT(PROP_GetUInt(p, uikey, &ui));
    TEST_ASSERT(ui == uival);

    TEST_ASSERT(!PROP_GetLong(p, nosuchkey, NULL));   /* Missing key */
    TEST_ASSERT(PROP_SetLong(p, lkey, lval));
    TEST_ASSERT(PROP_GetLong(p, lkey, NULL));
    TEST_ASSERT(PROP_GetLong(p, lkey, &l));
    TEST_ASSERT(l == lval);

    TEST_ASSERT(!PROP_GetULong(p, nosuchkey, NULL));  /* Missing key */
    TEST_ASSERT(PROP_SetULong(p, ulkey, ulval));
    TEST_ASSERT(PROP_GetULong(p, ulkey, NULL));
    TEST_ASSERT(PROP_GetULong(p, ulkey, &ul));
    TEST_ASSERT(ul == ulval);

    TEST_ASSERT(!PROP_GetBool(p, nosuchkey, NULL));   /* Missing key */
    TEST_ASSERT(!PROP_GetBool(p, skey, NULL));        /* Invalid value */
    TEST_ASSERT(!PROP_GetIntBool(p, skey, NULL));     /* Invalid value */
    TEST_ASSERT(PROP_SetBool(p, bkey, bval));
    TEST_ASSERT(PROP_GetBool(p, bkey, NULL));
    TEST_ASSERT(PROP_GetBool(p, bkey, &b));
    TEST_ASSERT(PROP_GetIntBool(p, bkey, NULL));
    TEST_ASSERT(PROP_GetIntBool(p, bkey, &i));
    TEST_ASSERT(b == bval);
    TEST_ASSERT(i == bval);

    TEST_ASSERT(!PROP_GetDouble(p, nosuchkey, NULL)); /* Missing key */
    TEST_ASSERT(PROP_SetDouble(p, dkey, dval));
    TEST_ASSERT(PROP_GetDouble(p, dkey, NULL));
    TEST_ASSERT(PROP_GetDouble(p, dkey, &d));
    TEST_ASSERT(d == dval);

    /* Setting comment for an existing record */
    TEST_ASSERT(!PROP_GetComment(p, dkey));
    TEST_ASSERT(PROP_SetComment(p, dkey, comment));
    TEST_ASSERT(STRING_Equal(PROP_GetComment(p, dkey), comment));

    /* Comment for the entire file */
    TEST_ASSERT(PROP_Comment(p, comment));

    /* Remove some entries */
    TEST_ASSERT(PROP_Remove(p, ikey));
    TEST_ASSERT(PROP_Remove(p, skey));
    TEST_ASSERT(!PROP_Remove(p, nosuchkey)); /* Missing key */
    TEST_ASSERT(!PROP_Remove(p, ikey));
    TEST_ASSERT(!PROP_Remove(p, skey));
    TEST_ASSERT(!PROP_Get(p, ikey));
    TEST_ASSERT(!PROP_Get(p, skey));

    PROP_Delete(p);
    return TEST_OK;
}

static
TestStatus
test_prop_copy(
    const TestDesc* test)
{
    Prop* p1 = PROP_Create();
    Prop* p2 = PROP_Create();
    const Str k1 = T_("k1");
    const Str k2 = T_("k2");
    const Str v1 = T_("v1");
    const Str v2 = T_("v2");
    const Str comment = T_("comment");

    TEST_ASSERT(PROP_Comment(p1, comment));
    TEST_ASSERT(PROP_Set(p1, k1, v1));
    TEST_ASSERT(PROP_Set(p1, k2, v2));
    TEST_ASSERT(PROP_SetComment(p1, k2, comment));
    TEST_ASSERT(PROP_Get(p1, k1));
    TEST_ASSERT(PROP_Get(p1, k2));
    TEST_ASSERT(!PROP_GetComment(p1, k1));
    TEST_ASSERT(PROP_GetComment(p1, k2));

    TEST_ASSERT(PROP_Copy(p2, p1));
    TEST_ASSERT(!PROP_GetComment(p2, k1));
    TEST_ASSERT(!PROP_GetComment(p2, k2)); /* PROP_Copy ignores comments */
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, k1), v1));
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, k2), v2));

    /* PROP_CopyAll copies comments as well */
    TEST_ASSERT(PROP_CopyAll(p2, p1));
    TEST_ASSERT(!PROP_GetComment(p2, k1));
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, k1), v1));
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, k2), v2));
    TEST_ASSERT(STRING_Equal(PROP_GetComment(p2, k2), comment));

    /* p1 remains intact */
    TEST_ASSERT(!PROP_GetComment(p1, k1));
    TEST_ASSERT(STRING_Equal(PROP_Get(p1, k1), v1));
    TEST_ASSERT(STRING_Equal(PROP_Get(p1, k2), v2));
    TEST_ASSERT(STRING_Equal(PROP_GetComment(p1, k2), comment));

    PROP_Delete(p1);
    PROP_Delete(p2);
    return TEST_OK;
}

static
TestStatus
test_prop_move(
    const TestDesc* test)
{
    Prop* p1 = PROP_Create();
    Prop* p2 = PROP_Create();
    const Str key = T_("foo");
    const Str val = T_("bar");

    TEST_ASSERT(PROP_Set(p1, key, val));
    TEST_ASSERT(PROP_Get(p1, key));
    PROP_Move(p2, p1);
    TEST_ASSERT(!PROP_Get(p1, key));
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, key), val));

    PROP_Delete(p1);
    PROP_Delete(p2);
    return TEST_OK;
}

static
TestStatus
test_prop_merge(
    const TestDesc* test)
{
    Prop* p1 = PROP_Create();
    Prop* p2 = PROP_Create();
    const Str k1 = T_("k1");
    const Str v1 = T_("v1");
    const Str k2 = T_("k2");
    const Str v2 = T_("v2");

    TEST_ASSERT(PROP_Set(p2, k2, v2));
    TEST_ASSERT(PROP_Size(p2) == 1);
    PROP_Merge(p2, NULL, p1); /* This has no effect */
    TEST_ASSERT(PROP_Size(p2) == 1);

    TEST_ASSERT(PROP_Set(p1, k1, v1));

    PROP_Merge(p2, NULL, p1);
    TEST_ASSERT(PROP_Size(p2) == 2); /* Now there are two entries there */
    TEST_ASSERT(STRING_Equal(PROP_Get(p1, k1), v1));
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, k1), v1));
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, k2), v2));

    /* Merge with a prefix */
    PROP_Merge(p2, T_("k"), p1);
    TEST_ASSERT(PROP_Size(p2) == 3);
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, k1), v1));
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, k2), v2));
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, T_("kk1")), v1));

    PROP_Delete(p1);
    PROP_Delete(p2);
    return TEST_OK;
}

static
TestStatus
test_prop_extract(
    const TestDesc* test)
{
    Prop* p1 = PROP_Create();
    Prop* p2 = PROP_Create();
    const Str v1 = T_("v1");
    const Str v2 = T_("v2");

    TEST_ASSERT(PROP_Set(p1, T_("x.k1"), v1));
    TEST_ASSERT(PROP_Set(p1, T_("y.k2"), v2));

    /* Extract and keep the prefix */
    TEST_ASSERT(PROP_Extract(p2, T_("x."), T_("x."), p1));
    TEST_ASSERT(PROP_Size(p2) == 1);
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, T_("x.k1")), v1));

    /* Extract and replace the prefix */
    PROP_Clear(p2);
    TEST_ASSERT(PROP_Extract(p2, T_("x."), T_("z."), p1));
    TEST_ASSERT(PROP_Size(p2) == 1);
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, T_("z.k1")), v1));

    /* Extract and remove the prefix */
    PROP_Clear(p2);
    TEST_ASSERT(PROP_Extract(p2, T_("y."), NULL, p1));
    TEST_ASSERT(PROP_Size(p2) == 1);
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, T_("k2")), v2));

    /* Copy and add the prefix */
    PROP_Clear(p2);
    TEST_ASSERT(PROP_Extract(p2, NULL, T_("z."), p1));
    TEST_ASSERT(PROP_Size(p2) == 2);
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, T_("z.x.k1")), v1));
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, T_("z.y.k2")), v2));

    /* Copy everything */
    PROP_Clear(p2);
    TEST_ASSERT(PROP_Extract(p2, NULL, NULL, p1));
    TEST_ASSERT(PROP_Size(p2) == 2);
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, T_("x.k1")), v1));
    TEST_ASSERT(STRING_Equal(PROP_Get(p2, T_("y.k2")), v2));

    PROP_Delete(p1);
    PROP_Delete(p2);
    return TEST_OK;
}

static
TestStatus
test_prop_iterator(
    const TestDesc* test)
{
    Prop* p = PROP_Create();
    const Str k1 = T_("k1");
    const Str k2 = T_("k2");
    const Str v1 = T_("v1");
    const Str v2 = T_("v2");
    const Str comment = T_("comment");
    const PropData* d;
    Iterator* it;

    /* Empty iterator */
    it = PROP_Keys(p);
    TEST_ASSERT(!ITR_HasNext(it));
    ITR_Delete(it);

    /* Add some properties */
    TEST_ASSERT(PROP_Comment(p, comment));
    TEST_ASSERT(PROP_Set(p, k1, v1));
    TEST_ASSERT(PROP_Set(p, k2, v2));
    TEST_ASSERT(PROP_SetComment(p, k2, comment));
    TEST_ASSERT(PROP_Size(p) == 2);

    /* Keys */
    it = PROP_Keys(p);
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT(STRING_Equal(ITR_Next(it), k1));
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT(STRING_Equal(ITR_Next(it), k2));
    TEST_ASSERT(!ITR_HasNext(it));
    ITR_Delete(it);

    it = PROP_ConstKeys(p);
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT(STRING_Equal(ITR_Next(it), k1));
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT(STRING_Equal(ITR_Next(it), k2));
    TEST_ASSERT(!ITR_Remove(it)); /* Const iterators don't remove anything */
    TEST_ASSERT(!ITR_HasNext(it));
    ITR_Delete(it);

    /* Values */
    it = PROP_Values(p);
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT(STRING_Equal(ITR_Next(it), v1));
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT(STRING_Equal(ITR_Next(it), v2));
    TEST_ASSERT(!ITR_HasNext(it));
    ITR_Delete(it);

    it = PROP_ConstValues(p);
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT(STRING_Equal(ITR_Next(it), v1));
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT(STRING_Equal(ITR_Next(it), v2));
    TEST_ASSERT(!ITR_Remove(it)); /* Const iterators don't remove anything */
    TEST_ASSERT(!ITR_HasNext(it));
    ITR_Delete(it);

    /* Data */
    it = PROP_Data(p);
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT((d = ITR_Next(it)) != NULL);
    TEST_ASSERT(STRING_Equal(d->key, k1));
    TEST_ASSERT(STRING_Equal(d->value, v1));
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT((d = ITR_Next(it)) != NULL);
    TEST_ASSERT(STRING_Equal(d->key, k2));
    TEST_ASSERT(STRING_Equal(d->value, v2));
    TEST_ASSERT(!ITR_HasNext(it));
    ITR_Delete(it);

    it = PROP_ConstData(p);
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT((d = ITR_Next(it)) != NULL);
    TEST_ASSERT(STRING_Equal(d->key, k1));
    TEST_ASSERT(STRING_Equal(d->value, v1));
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT((d = ITR_Next(it)) != NULL);
    TEST_ASSERT(STRING_Equal(d->key, k2));
    TEST_ASSERT(STRING_Equal(d->value, v2));
    TEST_ASSERT(!ITR_Remove(it)); /* Const iterators don't remove anything */
    TEST_ASSERT(!ITR_HasNext(it));
    ITR_Delete(it);

    /* Remove entries using the iterator */
    it = PROP_Keys(p);
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT(STRING_Equal(ITR_Next(it), k1));
    TEST_ASSERT(ITR_Remove(it));
    TEST_ASSERT(PROP_Size(p) == 1);
    TEST_ASSERT(!PROP_Get(p, k1));  /* k1 isn't there anymore */
    TEST_ASSERT(PROP_Get(p, k2));   /* k2 must be still there */
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT(STRING_Equal(ITR_Next(it), k2));
    TEST_ASSERT(ITR_Remove(it));
    TEST_ASSERT(PROP_Size(p) == 0);
    TEST_ASSERT(!ITR_HasNext(it));
    ITR_Delete(it);

    PROP_Delete(p);
    return TEST_OK;
}

static
TestStatus
test_prop_write(
    const TestDesc* test)
{
    Prop* p = PROP_Create();
    File* out;
    Buffer buf;
    const Str key = T_("foo");
    const char* str;
    int i;
    static const int failPoints[] = {
        /* foo = bar # comment  */
        /*   ..    .....      . */
        2, 3, 8, 9, 10, 11, 12, 19
    };

    BUFFER_Init(&buf);
    out = FILE_MemBuf(&buf, False);
    TEST_ASSERT(PROP_Set(p, key, T_("bar")));
    TEST_ASSERT(PROP_Write(p, out));

    BUFFER_PutChar(&buf, '\0'); /* Zero terminate the buffer */
    str = BUFFER_Access(&buf);
    Verbose("%s", str);
    /* Hmm.. Do we need this extra space at the end? */
    TEST_ASSERT(!strcmp("foo = bar \n", str));
    FILE_Close(out);

    /* Save a comment */
    TEST_ASSERT(PROP_SetComment(p, key, T_("comment")));

    BUFFER_Clear(&buf);
    out = FILE_MemBuf(&buf, False);
    TEST_ASSERT(PROP_Write(p, out));

    BUFFER_PutChar(&buf, '\0'); /* Zero terminate the buffer */
    str = BUFFER_Access(&buf);
    Verbose("%s", str);
    TEST_ASSERT(!strcmp("foo = bar # comment\n", str));
    FILE_Close(out);

    /* Simulate write failures */
    for (i = 0; i < COUNT(failPoints); i++) {
        BUFFER_Clear(&buf);
        BUFFER_Trim(&buf);
        buf.maxsiz = failPoints[i];
        Verbose("Failing at %d", failPoints[i]);
        out = FILE_MemBuf(&buf, False);
        TEST_ASSERT(!PROP_Write(p, out));
        FILE_Close(out);
    }

    /* Backslash and non-ASCII chars */
    PROP_Clear(p);
    TEST_ASSERT(PROP_Set(p, T_("!"), T_("\x0f")));

    BUFFER_Clear(&buf);
    out = FILE_MemBuf(&buf, False);
    TEST_ASSERT(PROP_Write(p, out));

    BUFFER_PutChar(&buf, '\0'); /* Zero terminate the buffer */
    str = BUFFER_Access(&buf);
    Verbose("%s", str);
    /* Hmm.. Do we need this extra space at the end? */
    TEST_ASSERT(!strcmp("\\! = \\u000F \n", str));
    FILE_Close(out);

    BUFFER_Destroy(&buf);
    PROP_Delete(p);
    return TEST_OK;
}

static
TestStatus
test_prop_write_xml(
    const TestDesc* test)
{
    Prop* p = PROP_Create();
    File* out;
    Buffer buf;
    const Str key = T_("foo");
    const char* str;
    const char* xml1 =
        "<props>\n"
        "<p k=\"foo\" v=\"bar\"/>\n"
        "</props>\n";
    const char* xml2 =
        "<props>\n"
        "<p k=\"foo\" v=\"bar\" c=\"comment\"/>\n"
        "</props>\n";

    BUFFER_Init(&buf);
    out = FILE_MemBuf(&buf, False);
    TEST_ASSERT(PROP_Set(p, key, T_("bar")));
    TEST_ASSERT(PROP_WriteXML(p, NULL, out));

    BUFFER_PutChar(&buf, '\0'); /* Zero terminate the buffer */
    str = BUFFER_Access(&buf);
    Verbose("%s", str);
    TEST_ASSERT(!strcmp(str, xml1));
    FILE_Close(out);

    /* Save a comment */
    TEST_ASSERT(PROP_SetComment(p, key, T_("comment")));

    BUFFER_Clear(&buf);
    out = FILE_MemBuf(&buf, False);
    TEST_ASSERT(PROP_WriteXML(p, "", out));

    BUFFER_PutChar(&buf, '\0'); /* Zero terminate the buffer */
    str = BUFFER_Access(&buf);
    Verbose("%s", str);
    TEST_ASSERT(!strcmp(str, xml2));
    FILE_Close(out);

    BUFFER_Destroy(&buf);
    PROP_Delete(p);
    return TEST_OK;
}

static
TestStatus
test_prop_read(
    const TestDesc* test)
{
    Prop* p = PROP_Create();
    File* in;
    Iterator* it;
    const PropData* d;
    const Str key = T_("foo");
    const Str val = T_("bar");
    const Str comment = T_("comment");
    static const char keyVal[] = "foo = bar\n\n";
    static const char keyValComment[] = "\n#xxx\nfoo = bar#comment";
    static const char escapeChars[] = "\\! = \\u000F ";

    in = FILE_MemIn(keyVal, sizeof(keyVal) - 1);
    TEST_ASSERT(PROP_Read(p, in));
    FILE_Close(in);

    TEST_ASSERT(PROP_Size(p) == 1);
    TEST_ASSERT(STRING_Equal(PROP_Get(p, key), val));
    TEST_ASSERT(!PROP_GetComment(p, key));

    /* Same thing with a comment */
    in = FILE_MemIn(keyValComment, sizeof(keyValComment) - 1);
    TEST_ASSERT(PROP_Read(p, in));
    FILE_Close(in);

    TEST_ASSERT(PROP_Size(p) == 1);
    TEST_ASSERT(STRING_Equal(PROP_Get(p, key), val));
    TEST_ASSERT(STRING_Equal(PROP_GetComment(p, key), comment));
    it = PROP_ConstData(p);
    TEST_ASSERT(ITR_HasNext(it));
    TEST_ASSERT((d = ITR_Next(it)) != NULL);
    TEST_ASSERT(STRING_Equal(d->key, key));
    TEST_ASSERT(STRING_Equal(d->value, val));
    TEST_ASSERT(STRING_Equal(d->c, comment));
    ITR_Delete(it);

    /* Test escape sequences */
    in = FILE_MemIn(escapeChars, sizeof(escapeChars) - 1);
    TEST_ASSERT(PROP_Read(p, in));
    FILE_Close(in);

    TEST_ASSERT(PROP_Size(p) == 1);
    TEST_ASSERT(STRING_Equal(PROP_Get(p, T_("!")), T_("\x0f")));

    PROP_Delete(p);
    return TEST_OK;
}

static
TestStatus
test_prop_read_xml(
    const TestDesc* test)
{
#ifndef _WIN32
    Prop* p = PROP_Create();
    File* in;
    XMLTag* root;
    const Str key = T_("foo");
    const Str val = T_("bar");
    const Str comment = T_("comment");
    static const char keyVal[] = 
        "<props>\n"
        "  <p k=\"foo\" v=\"bar\"/>\n"
        "</props>\n";
    static const char keyValComment[] =
        "<props>\n"
        "  <p c=\"xxx\"/>\n"
        "  <p k=\"foo\" v=\"bar\" c=\"comment\"/>\n"
        "</props>\n";

    in = FILE_MemIn(keyVal, sizeof(keyVal) - 1);
    root = XML_Handle(in, PropRootTag, PROP_RootCB, p);
    TEST_ASSERT(root);
    TEST_ASSERT(root->endTag(root, NULL));
    FILE_Close(in);

    TEST_ASSERT(PROP_Size(p) == 1);
    TEST_ASSERT(STRING_Equal(PROP_Get(p, key), val));
    TEST_ASSERT(!PROP_GetComment(p, key));

    /* Same thing with a comment (Prop is allocated by PROP_RootCB) */
    PROP_Delete(p);
    in = FILE_MemIn(keyValComment, sizeof(keyValComment) - 1);
    root = XML_Handle(in, PropRootTag, PROP_RootCB, NULL);
    TEST_ASSERT(root);
    p = PROP_FromTag(root);
    FILE_Close(in);

    TEST_ASSERT(PROP_Size(p) == 1);
    TEST_ASSERT(STRING_Equal(PROP_Get(p, key), val));
    TEST_ASSERT(STRING_Equal(PROP_GetComment(p, key), comment));

    PROP_Delete(p);
#endif /* _WIN32 */
    return TEST_OK;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_prop_alloc},
        {"Basic", test_prop_basic},
        {"Copy", test_prop_copy},
        {"Move", test_prop_move},
        {"Merge", test_prop_merge},
        {"Extract", test_prop_extract},
        {"Iterator", test_prop_iterator},
        {"Write", test_prop_write},
        {"WriteXML", test_prop_write_xml},
        {"Read", test_prop_read},
        {"ReadXML", test_prop_read_xml}
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
