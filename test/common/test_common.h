/*
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

#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "s_lib.h"

typedef enum test_status {
    TEST_OK,
    TEST_ERR,
    TEST_CMDLINE,
    TEST_NOTFOUND
} TestStatus;

typedef struct test_desc TestDesc;
struct test_desc {
    const char* name;
    TestStatus (*run)(const TestDesc* test);
};

typedef struct _MemContext {
    const MemHook* hook;
    int allocCount;
    int failAt;
    int failCount;
} TestMem;

void
test_mem_init(
    TestMem* hook);

void
test_mem_deinit(
    TestMem* hook);

int
test_main(
    int argc,
    char* argv[],
    const void* tests,
    size_t testSize,
    int testCount);

void
test_assert(
    const char* msg,
    const char* file,
    long line);

#define TEST_MAIN(argc,argv,tests) \
    test_main(argc,argv,tests,sizeof(tests[0]),COUNT(tests))
#define TEST_ASSERT(x) \
    ((x) ? NOTHING : test_assert(#x,__FILE__,__LINE__))

#endif /* TEST_COMMON_H */

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
