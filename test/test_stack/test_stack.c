/*
 * $Id: test_stack.c,v 1.1 2016/09/26 16:09:22 slava Exp $
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
test_stack_alloc(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Stack* s;

    /* Test NULL resistance */
    STACK_Delete(NULL);

    /* Simulate allocation failures */
    testMem.failAt = testMem.allocCount;
    if (STACK_Create(0)) {
        ret = TEST_ERR;
    }

    testMem.failAt = testMem.allocCount + 1;
    if (STACK_Create(1)) {
        ret = TEST_ERR;
    }

    s = STACK_Create(0);
    testMem.failAt = testMem.allocCount;
    if (STACK_Push(s, s)) {
        ret = TEST_ERR;
    }

    testMem.failAt = -1;
    STACK_Delete(s);
    return ret;
}

static
TestStatus
test_stack_basic(
    const TestDesc* test)
{
    TestStatus ret = TEST_OK;
    Stack* s = STACK_Create(1);
    const int n = 5;
    int i;

    /* Peek empty stack */
    if (STACK_Peek(s) ||
        STACK_Pop(s)) {
        ret = TEST_ERR;
    }

    /* Push */
    for (i=1; i<=n; i++) {
        if (!STACK_Push(s, (SElement)(PtrWord)i)) {
            ret = TEST_ERR;
        }
    }

    /* Peek */
    if (STACK_Peek(s) != (SElement)(PtrWord)n) {
        ret = TEST_ERR;
    }

    /* Check */
    for (i=1; i<=n; i++) {
        if (!STACK_Contains(s, (SElement)(PtrWord)i)) {
            ret = TEST_ERR;
        }
    }

    if (STACK_Contains(s, NULL)) {
        ret = TEST_ERR;
    }

    /* Pop */
    for (i=n; i>0; i--) {
        if (STACK_Pop(s) != (SElement)(PtrWord)i) {
            ret = TEST_ERR;
        }
    }

    if (STACK_Peek(s) ||
        STACK_Pop(s)) {
        ret = TEST_ERR;
    }

    STACK_Delete(s);
    return ret;
}

int
main(int argc, char* argv[])
{
    static const TestDesc tests[] = {
        {"Alloc", test_stack_alloc},
        {"Basic", test_stack_basic}
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
