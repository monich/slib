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

static
void*
test_mem_alloc(
    MemContext* mem,
    size_t size)
{
    void* ptr;
    if (mem->allocCount >= mem->failAt &&
        mem->allocCount < (mem->failAt + mem->failCount)) {
        TRACE1("MEM: simulating %d bytes allocation failure\n", (int)size);
        ptr = NULL;
    } else {
        ptr = MEM_Alloc2(MEM_NextHook(mem->hook), size);
    }
    mem->allocCount++;
    return ptr;
}

static
void*
test_mem_realloc(
    MemContext* mem,
    void* ptr,
    size_t size)
{
    if (mem->allocCount == mem->failAt) {
        TRACE1("MEM: simulating %d bytes (re)allocation failure\n", (int)size);
        ptr = NULL;
    } else {
        ptr = MEM_Realloc2(MEM_NextHook(mem->hook), ptr, size);
    }
    mem->allocCount++;
    return ptr;
}

void
test_mem_init(
    TestMem* mem)
{
    const static MemProc testMemProc = {
        NULL,               /* memInit */
        test_mem_alloc,     /* memAlloc */
        test_mem_realloc,   /* memRealloc */
        NULL,               /* memFree */
    };
    mem->hook = MEM_Hook(&testMemProc, mem);
    mem->allocCount = 0;
    mem->failAt = -1;
    mem->failCount = 1;
    SLIB_InitModules();
}

void
test_mem_deinit(
    TestMem* mem)
{
    SLIB_Shutdown();
    OS_MemFree((void*)(mem->hook));
    mem->hook = NULL;
}

/*
 * Local Variables:
 * mode: C
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
