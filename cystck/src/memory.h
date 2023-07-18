/*******************************************************************************
 * Copyright (c) 1996, 2022 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License.
 *
 * [1] https://www.gnu.org/software/classpath/license.html

 *******************************************************************************/
#ifndef memory_h
#define memory_h

#ifndef NULL
#define NULL 0
#endif


/* memory error messages */
#define codeEM   "code size overflow"
#define symbolEM   "symbol table overflow"
#define constantEM   "constant table overflow"
#define stackEM   "stack size overflow\n"
#define lexEM   "lex buffer overflow"
#define refEM   "reference table overflow"
#define tableEM  "table overflow"
#define memEM "not enough memory\n"


void *I_realloc (void *oldblock, unsigned long size);
void *I_buffer (unsigned long size);
int I_growvector (void **block, unsigned long nelems, int size,
                       char *errormsg, unsigned long limit);

#define I_malloc(s)	I_realloc(NULL, (s))
#define newvector(n,s)  ((s *)I_malloc((n)*sizeof(s)))
#define growvector(old,n,s,e,l) \
          (I_growvector((void**)old,n,sizeof(s),e,l))

#endif 

