/*
 * $Id: test_md.c,v 1.1 2016/09/26 16:09:18 slava Exp $
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

typedef struct test_digest {
    TestDesc desc;
    Digest* (*create_digest)(void);
    void (*one_shot_digest)(const void* in, size_t n, void* out);
    const char* in;
    int repeat;
    const I8u* out;
    size_t outsize;
} TestDigest;

/* MD5 examples from http://www.ietf.org/rfc/rfc1321 */

#define MD5_TEST1 ""
#define MD5_TEST2 "a"
#define MD5_TEST3 "abc"
#define MD5_TEST4 "message digest"
#define MD5_TEST5 "abcdefghijklmnopqrstuvwxyz"
#define MD5_TEST6 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef" \
                  "ghijklmnopqrstuvwxyz0123456789"
#define MD5_TEST7 "12345678901234567890123456789012" \
                  "34567890123456789012345678901234" \
                  "5678901234567890"
static const I8u test_md5_data1[] = {
    0xd4,0x1d,0x8c,0xd9,0x8f,0x00,0xb2,0x04,
    0xe9,0x80,0x09,0x98,0xec,0xf8,0x42,0x7e
};
static const I8u test_md5_data2[] = {
    0x0c,0xc1,0x75,0xb9,0xc0,0xf1,0xb6,0xa8,
    0x31,0xc3,0x99,0xe2,0x69,0x77,0x26,0x61
};
static const I8u test_md5_data3[] = {
    0x90,0x01,0x50,0x98,0x3c,0xd2,0x4f,0xb0,
    0xd6,0x96,0x3f,0x7d,0x28,0xe1,0x7f,0x72
};
static const I8u test_md5_data4[] = {
    0xf9,0x6b,0x69,0x7d,0x7c,0xb7,0x93,0x8d,
    0x52,0x5a,0x2f,0x31,0xaa,0xf1,0x61,0xd0
};
static const I8u test_md5_data5[] = {
    0xc3,0xfc,0xd3,0xd7,0x61,0x92,0xe4,0x00,
    0x7d,0xfb,0x49,0x6c,0xca,0x67,0xe1,0x3b
};
static const I8u test_md5_data6[] = {
    0xd1,0x74,0xab,0x98,0xd2,0x77,0xd9,0xf5,
    0xa5,0x61,0x1c,0x2c,0x9f,0x41,0x9d,0x9f
};
static const I8u test_md5_data7[] = {
    0x57,0xed,0xf4,0xa2,0x2b,0xe3,0xc9,0x55,
    0xac,0x49,0xda,0x2e,0x21,0x07,0xb6,0x7a
};

/* SHA1 examples from http://www.ietf.org/rfc/rfc3174 */

#define SHA1_TEST1   "abc"
#define SHA1_TEST2a  "abcdbcdecdefdefgefghfghighijhi"
#define SHA1_TEST2b  "jkijkljklmklmnlmnomnopnopq"
#define SHA1_TEST2   SHA1_TEST2a SHA1_TEST2b
#define SHA1_TEST3   "a"
#define SHA1_TEST4a  "01234567012345670123456701234567"
#define SHA1_TEST4b  "01234567012345670123456701234567"
#define SHA1_TEST4   SHA1_TEST4a SHA1_TEST4b

/* And this (longer) one is mine */
#define SHA1_TEST5   SHA1_TEST1 SHA1_TEST2 SHA1_TEST3 SHA1_TEST4

static const I8u test_sha1_data1[] = {
    0xA9,0x99,0x3E,0x36,0x47,0x06,0x81,0x6A,
    0xBA,0x3E,0x25,0x71,0x78,0x50,0xC2,0x6C,
    0x9C,0xD0,0xD8,0x9D
};
static const I8u test_sha1_data2[] = {
    0x84,0x98,0x3E,0x44,0x1C,0x3B,0xD2,0x6E,
    0xBA,0xAE,0x4A,0xA1,0xF9,0x51,0x29,0xE5,
    0xE5,0x46,0x70,0xF1
};
static const I8u test_sha1_data3[] = {
    0x34,0xAA,0x97,0x3C,0xD4,0xC4,0xDA,0xA4,
    0xF6,0x1E,0xEB,0x2B,0xDB,0xAD,0x27,0x31,
    0x65,0x34,0x01,0x6F
};
static const I8u test_sha1_data4[] = {
    0xDE,0xA3,0x56,0xA2,0xCD,0xDD,0x90,0xC7,
    0xA7,0xEC,0xED,0xC5,0xEB,0xB5,0x63,0x93,
    0x4F,0x46,0x04,0x52
};
static const I8u test_sha1_data5[] = {
    0xF3,0xEA,0x92,0xC3,0x89,0xA0,0x30,0xF0,
    0x40,0x65,0xFF,0x6A,0x9D,0x47,0x28,0x61,
    0x1A,0x8F,0x5B,0xD4
};

static
TestStatus
test_md(
    const TestDesc* desc)
{
    TestStatus ret = TEST_OK;
    int i;
    const TestDigest* test = CAST(desc,TestDigest,desc);
    const size_t input_len = strlen(test->in);
    const char* name;
    Digest* digest;
    size_t size;
    void* out;

    /* Simulate allocation failure */
    testMem.failAt = testMem.allocCount;
    if (test->create_digest()) {
         ret = TEST_ERR;
    }

    /* This one will succeed */
    digest = test->create_digest();
    DIGEST_Init(digest);
    size = DIGEST_Size(digest);
    name = DIGEST_Name(digest);
    out = MEM_Alloc(size);

    for (i=0; i<test->repeat; i++) {
        if (input_len > 1) {
            DIGEST_Update(digest, test->in, 1);
            DIGEST_Update(digest, test->in+1, input_len-1);
        } else {
            DIGEST_Update(digest, test->in, input_len);
        }
    }
    DIGEST_Finish(digest, out);
    if (size != test->outsize) {
        PRINT_Error("%s output size mismatch\n", name);
        ret = TEST_ERR;
    } else if (memcmp(out, test->out, size)) {
        PRINT_Error("%s digest mismatch\n", name);
        ret = TEST_ERR;
    } else if (test->repeat == 1) {
        test->one_shot_digest(test->in, input_len, out);
        if (memcmp(out, test->out, size)) {
            PRINT_Error("%s single shot digest mismatch\n", name);
            ret = TEST_ERR;
        }
    }

    DIGEST_Delete(digest);
    MEM_Free(out);
    return ret;
}

#define TEST_DATA(d) d, sizeof(d)
#define TEST_(ALG,alg,i,n) \
    {{#ALG "_TEST" #i, test_md}, ALG##_Create, ALG##_Digest, \
      ALG##_TEST##i, n, TEST_DATA(test_##alg##_data##i) }
#define TEST_MD5(i,n) TEST_(MD5,md5,i,n)
#define TEST_SHA1(i,n) TEST_(SHA1,sha1,i,n)
#define TEST_SHA256(i,n) TEST_(SHA256,sha256,i,n)

int main(int argc, char* argv[])
{
    static const TestDigest tests[] = {
        /* MD5 */
        TEST_MD5(1,1),
        TEST_MD5(2,1),
        TEST_MD5(3,1),
        TEST_MD5(4,1),
        TEST_MD5(5,1),
        TEST_MD5(6,1),
        TEST_MD5(7,1),
        /* SHA1 */
        TEST_SHA1(1,1),
        TEST_SHA1(2,1),
        TEST_SHA1(3,1000000),
        TEST_SHA1(4,10),
        TEST_SHA1(5,1)
    };

    int ret;
    test_mem_init(&testMem);
    DIGEST_Delete(NULL); /* This isn't worth a testcase :) */
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
