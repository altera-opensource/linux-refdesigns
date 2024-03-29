/*------------------------------------------------------------------
 * strlastsame_s.c
 *
 * Copyright (c) 2008 Bo Berry
 * Copyright (c) 2008-2011 Cisco Systems
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

#include "safeclib_private.h"
#include "safe_str_constraint.h"
#include "safe_str_lib.h"


/**
 * NAME
 *    strlastsame_s
 *
 * SYNOPSIS
 *    #include "safe_str_lib.h"
 *    errno_t
 *    strlastsame_s(const char *dest, rsize_t dmax,
 *                  const char *src, rsize_t *index)
 *
 * DESCRIPTION
 *    Returns the index of the last character that is the
 *    same between dest and src. The scanning stops at the
 *    first nul in dest or src, or after dmax characters.
 *
 * EXTENSION TO
 *    ISO/IEC TR 24731, Programming languages, environments
 *    and system software interfaces, Extensions to the C Library,
 *    Part I: Bounds-checking interfaces
 *
 * INPUT PARAMETERS
 *    dest      pointer to string to compare against
 *
 *    dmax    restricted maximum length of string dest
 *
 *    src      pointer to the string to be compared to dest
 *
 *    index   pointer to returned index
 *
 * OUTPUT PARAMETERS
 *    index   updated index
 *
 * RUNTIME CONSTRAINTS
 *    Neither dest nor src shall not be a null pointer.
 *    indicator shall not be a null pointer.
 *    dmax shall not be 0
 *    dmax shall not be greater than RSIZE_MAX_STR
 *
 * RETURN VALUE
 *    index to last same char, when the return code is OK
 *
 *    EOK         index to last same char is returned
 *    ESNULLP     NULL pointer
 *    ESZEROL     zero length
 *    ESLEMAX     length exceeds max limit
 *    ESNOTFND    not found
 *    ESUNTERM    string unterminated
 *
 * ALSO SEE
 *    strfirstchar_s(), strfirstdiff_s(), strfirstsame_s(),
 *    strlastchar_s(), strlastdiff_s()
 *
 */
errno_t
strlastsame_s (const char *dest, rsize_t dmax,
               const char *src, rsize_t *index)
{
    const char *rp;
    bool similarity;

    if (index == NULL) {
        invoke_safe_str_constraint_handler("strlastsame_s: index is null",
                   NULL, ESNULLP);
        return (ESNULLP);
    }
    *index = 0;

    if (dest == NULL) {
        invoke_safe_str_constraint_handler("strlastsame_s: dest is null",
                   NULL, ESNULLP);
        return (ESNULLP);
    }

    if (src == NULL) {
        invoke_safe_str_constraint_handler("strlastsame_s: src is null",
                   NULL, ESNULLP);
        return (ESNULLP);
    }

    if (dmax == 0 ) {
        invoke_safe_str_constraint_handler("strlastsame_s: dmax is 0",
                   NULL, ESZEROL);
        return (ESZEROL);
    }

    if (dmax > RSIZE_MAX_STR) {
        invoke_safe_str_constraint_handler("strlastsame_s: dmax exceeds max",
                   NULL, ESLEMAX);
        return (ESLEMAX);
    }

    /* hold reference point */
    rp = dest;

    /*
     * find the last offset
     */
    similarity = false;
    while (*dest && *src && dmax) {

        if (*dest == *src) {
            similarity = true;
            *index = (uint32_t)(dest - rp);
        }

        dest++;
        src++;
        dmax--;
    }

    if (similarity) {
        return (EOK);
    } else {
        return (ESNOTFND);
    }
}
EXPORT_SYMBOL(strlastsame_s)
