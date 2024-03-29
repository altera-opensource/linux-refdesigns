/*------------------------------------------------------------------
 * safe_mem_constraint.h
 *
 * Copyright (c) 2008 Bo Berry
 * Copyright (c) 2008-2009 Cisco Systems
 * Copyright (c) 2022 Intel Corp
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *------------------------------------------------------------------
 */

#ifndef __SAFE_MEM_CONSTRAINT_H__
#define __SAFE_MEM_CONSTRAINT_H__

#include "safeclib_private.h"

/*
 * Function used by the libraries to invoke the registered
 * runtime-constraint handler. Always needed.
 */
extern void invoke_safe_mem_constraint_handler(
                           const char *msg,
                           void *ptr,
                           errno_t error);

#endif /* __SAFE_MEM_CONSTRAINT_H__ */
