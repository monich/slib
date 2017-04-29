/*
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

typedef struct test_app {
    const void* tests;
    size_t test_size;
    int test_count;
} TestApp;

#define test_at(app,i) \
    ((const TestDesc*)(((I8u*)(app)->tests) + i * (app)->test_size))

#if DEBUG_TRACE
static
void
test_assert_handler(
    const char* msg,
    const char* file,
    long line)
{
    TRACE3("ASSERT: %s at %s:%lu\n", msg, file, line);
}
#endif /* DEBUG_TRACE */

static
int
test_main_run_one(
    const TestDesc* test)
{
    int ret = test->run(test);
    PRINT_Output("%s: %s\n", (ret == TEST_OK) ? "OK" : "FAILED", test->name);
    return ret;
}

static
int
test_main_run(
    TestApp* app,
    const char* name)
{
    int i, ret;
    if (name) {
        const TestDesc* found = NULL;
        for (i=0, ret = TEST_NOTFOUND; i<app->test_count; i++) {
            const TestDesc* test = test_at(app, i);
            if (!strcmp(test->name, name)) {
                ret = test_main_run_one(test);
                found = test;
                break;
            }
        }
        if (!found) TRACE_Error("No such test: %s\n", name);
    } else {
        for (i=0, ret = TEST_OK; i<app->test_count; i++) {
            int test_status = test_main_run_one(test_at(app, i));
            if (ret == TEST_OK && test_status != TEST_OK) {
                ret = test_status;
            }
        }
    }
    return ret;
}

int
test_main(
    int argc,
    char* argv[],
    const void* tests,
    size_t test_size,
    int test_count)
{
    int ret = TEST_ERR;
    Bool verbose = False;
    CmdLine* opt;
    Vector params;

    SLIB_InitModules();
#if DEBUG_TRACE
    slibDebugAssertHandler = test_assert_handler;
#endif
    opt = CMDLINE_Create(argv[0]);
    VECTOR_Init(&params, 0, NULL, NULL);
    CMDLINE_AddTrueOpt(opt, 'v', "verbose", "Enable verbose output", &verbose);
    if (CMDLINE_Parse(opt, argv+1, argc-1, 0, &params)) {
        TestApp app;
        app.tests = tests;
        app.test_size = test_size;
        app.test_count = test_count;
        PRINT_SetMask(verbose ? PRINT_ALL : PRINT_NORMAL);
        if (VECTOR_IsEmpty(&params)) {
            ret = test_main_run(&app, NULL);
        } else {
            int i;
            for (i=0, ret = TEST_OK; i<VECTOR_Size(&params); i++) {
                int test_status = test_main_run(&app, VECTOR_Get(&params, i));
                if (ret == TEST_OK && test_status != TEST_OK) {
                    ret = test_status;
                }
            }
        }
    } else {
        CMDLINE_Help(opt, "[TESTS]", 0);
        ret = TEST_CMDLINE;
    }
    VECTOR_Destroy(&params);
    CMDLINE_Delete(opt);
    SLIB_Shutdown();
    return ret;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
