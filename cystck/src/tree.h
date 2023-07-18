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
#ifndef tree_h
#define tree_h



#include <limits.h>
#define WRONGTOKEN 257
#define NIL 258
#define IF 259
#define THEN 260
#define ELSE 261
#define ELSEIF 262
#define WHILE 263
#define DO 264
#define REPEAT 265
#define UNTIL 266
#define END 267
#define RETURN 268
#define LOCAL 269
#define FUNCTION 270
#define DOTS 271
#define NUMBER 272
#define STRING 273
#define NAME 274
#define AND 275
#define OR 276
#define EQ 277
#define NE 278
#define LE 279
#define GE 280
#define CONC 281
#define UNARY 282
#define NOT 283

#ifndef Real
#define Real float
#endif

typedef unsigned char  Byte;  /* unsigned 8 bits */

typedef unsigned short Word;  /* unsigned 16 bits */

#define MAX_WORD  (USHRT_MAX-2)  /* maximum value of a word (-2 for safety) */
#define MAX_INT   (INT_MAX-2)  /* maximum value of a int (-2 for safety) */

typedef signed long  Long;  /* 32 bits */

// #define NOT_USED  0xFFFE


typedef struct TaggedString
{
  int tag;  /* if != Cystck_STRING, this is a userdata */
  struct TaggedString *next;
  union {
    struct {
      Word varindex;  /* != NOT_USED  if this is a symbol */
      Word constindex;  /* != NOT_USED  if this is a constant */
    } s;
    void *v;  /* if this is a userdata, here is its value */
  } u;
  unsigned long hash;  /* 0 if not initialized */
  int marked;   /* for garbage collection; never collect (nor change) if > 1 */
  char str[1];   /* \0 byte already reserved */
} TaggedString;
 

TaggedString *createstring (char *str);

#endif
