/*
 * $Id: s_init.h,v 1.2 2015/08/16 16:07:28 slava Exp $
 *
 * Copyright (C) 2015 by Slava Monich
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

#ifndef _SLAVA_INIT_H_
#define _SLAVA_INIT_H_

#include "s_def.h"

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

/*
 * Functions to initialize and deinitialize slib's global data structures.
 *
 * SLIB_InitModules() and SLIB_Init() do the same thing, the only
 * difference is that Win32 version of SLIB_Init() initializes
 * Winsock, while SLIB_InitModules() does not. On any other platform
 * these two functions are absolutely equivalent.
 */
extern void SLIB_Init P_((void));
extern void SLIB_InitModules P_((void));
extern void SLIB_Shutdown P_((void));

/* These may be more readable than the original functions */
#define SLIB_InitWithNetwork() SLIB_InitModules()
#define SLIB_Deinit() SLIB_Shutdown()

#ifdef __cplusplus
} /* end of extern "C" */
#endif  /* __cplusplus */

#endif /* _SLAVA_INIT_H_ */

/*
 * HISTORY:
 *
 * $Log: s_init.h,v $
 * Revision 1.2  2015/08/16 16:07:28  slava
 * o housekeeping
 *
 * Revision 1.1  2015/07/19 12:33:48  slava
 * o moved SLIB_Init(), SLIB_InitModules() and SLIB_Shutdown() into new
 *   s_init.h header so that it's no longer necessary to include s_lib.h
 *   which pulls in all slib headers just to call one of those three
 *   functions.
 *
 * Local Variables:
 * mode:C
 * c-basic-offset:4
 * indent-tabs-mode: nil
 * End:
 */
