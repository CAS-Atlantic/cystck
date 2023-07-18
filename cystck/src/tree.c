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
#include <string.h>
#include "memory.h"
#include "../include/Cystck.h"
#include "tree.h"
#include <ctype.h>

#define MINBUFF 250



#define MAX_IFS	5

/* "ifstate" keeps the state of each nested $if the lexical is dealing with. */

static struct
  {
    char *name;
    int token;
  } reserved [] = {
      {"and", AND},
      {"do", DO},
      {"else", ELSE},
      {"elseif", ELSEIF},
      {"end", END},
      {"function", FUNCTION},
      {"if", IF},
      {"local", LOCAL},
      {"nil", NIL},
      {"not", NOT},
      {"or", OR},
      {"repeat", REPEAT},
      {"return", RETURN},
      {"then", THEN},
      {"until", UNTIL},
      {"while", WHILE} };


#define RESERVEDSIZE (sizeof(reserved)/sizeof(reserved[0]))


void I_addReserved (void)
{
  long unsigned int i;
  for (i=0; i<RESERVEDSIZE; i++)
  {
    TaggedString *ts = createstring(reserved[i].name);
    ts->marked = reserved[i].token;  /* reserved word  (always > 255) */
  }
}

#define NUM_HASHS  64

typedef struct {
  int size;
  int nuse;  /* number of elements (including EMPTYs) */
  TaggedString **hash;
} stringtable;

//static int initialized = 0;

static stringtable string_root[NUM_HASHS];

static TaggedString EMPTY = {Cystck_STRING, NULL, {{0xFFFE, 0xFFFE}},
                             0, 2, {0}};


static unsigned long hash (char *s, int tag)
{
  unsigned long h;
  if (tag != Cystck_STRING)
    h = (unsigned long)s;
  else {
    h = 0;
    while (*s)
      h = ((h<<5)-h)^(unsigned char)*(s++);
  }
  return h;
}

static Long dimensions[] = 
 {5L, 11L, 23L, 47L, 97L, 197L, 397L, 797L, 1597L, 3203L, 6421L,
  12853L, 25717L, 51437L, 102811L, 205619L, 411233L, 822433L,
  1644817L, 3289613L, 6579211L, 13158023L, MAX_INT};

int I_redimension (int nhash)
{
 int i;
 for (i=0; dimensions[i]<MAX_INT; i++)
 {
  if (dimensions[i] > nhash)
   return dimensions[i];
 }
 fprintf (stderr, "table overflow");
 return 0;  /* to avoid warnings */
}
static void grow (stringtable *tb)
{
  int newsize = I_redimension(tb->size);
  TaggedString **newhash = newvector(newsize, TaggedString *);
  int i;
  for (i=0; i<newsize; i++)
    newhash[i] = NULL;
  /* rehash */
  tb->nuse = 0;
  for (i=0; i<tb->size; i++)
    if (tb->hash[i] != NULL && tb->hash[i] != &EMPTY) {
      int h = tb->hash[i]->hash%newsize;
      while (newhash[h])
        h = (h+1)%newsize;
      newhash[h] = tb->hash[i];
      tb->nuse++;
    }
  free(tb->hash); 
  tb->size = newsize;
  tb->hash = newhash;
}


static TaggedString *newone(char *buff, int tag, unsigned long h)
{
  TaggedString *ts;
  if (tag == Cystck_STRING) {
    ts = (TaggedString *)I_malloc(sizeof(TaggedString)+strlen(buff));
    strcpy(ts->str, buff);
    ts->u.s.varindex = ts->u.s.constindex = 0xFFFE;
    ts->tag = Cystck_STRING;
  }
  else {
    ts = (TaggedString *)I_malloc(sizeof(TaggedString));
    ts->u.v = buff;
    ts->tag = tag == Cystck_ANYTAG ? 0 : tag;
  }
  ts->marked = 0;
  ts->hash = h;
  return ts;
}

static TaggedString *insert (char *buff, int tag, stringtable *tb)
{
  TaggedString *ts;
  unsigned long h = hash(buff, tag);
  int i;
  int j = -1;
  if ((Long)tb->nuse*3 >= (Long)tb->size*2)
  {
    /*if (!initialized)
      initialize();*/
    grow(tb);
  }
  i = h%tb->size;
  while ((ts = tb->hash[i]) != NULL)
  {
    if (ts == &EMPTY)
      j = i;
    else if ((ts->tag == Cystck_STRING) ?
              (tag == Cystck_STRING && (strcmp(buff, ts->str) == 0)) :
              ((tag == ts->tag || tag == Cystck_ANYTAG) && buff == ts->u.v))
      return ts;
    i = (i+1)%tb->size;
  }
  /* not found */
  if (j != -1)  /* is there an EMPTY space? */
    i = j;
  else
    tb->nuse++;
  ts = tb->hash[i] = newone(buff, tag, h);
  return ts;
}


TaggedString *createstring (char *str)
{
  return insert(str, Cystck_STRING, &string_root[(unsigned)str[0]%NUM_HASHS]);
}
