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
#include <stdlib.h>

#include "memory.h"
#include "../include/Cystck.h"


#define DEBUG 0


void *I_realloc (void *oldblock, unsigned long size)
{
  void *block;
  size_t s = (size_t)size;
  if (s != size)
    PyErr_SetString(PyExc_ValueError,"Allocation Error: Block too big");
  block = oldblock ? realloc(oldblock, s) : malloc(s);
  if (block == NULL)
  {  fprintf (stderr, memEM);
    return PyErr_NoMemory(); 
  }
  return block;
}


int I_growvector (void **block, unsigned long nelems, int size,
                       char *errormsg, unsigned long limit)
{
  if (nelems >= limit)
  {  PyErr_SetString(PyExc_ValueError,errormsg);
    exit(1);
  }
  nelems = (nelems == 0) ? 20 : nelems*2;
  if (nelems > limit)
    nelems = limit;
  *block = I_realloc(*block, nelems*size);
  return (int)nelems;
}


void* I_buffer (unsigned long size)
{
  static unsigned long buffsize = 0;
  static char* buffer = NULL;
  if (size > buffsize)
    buffer = I_realloc(buffer, buffsize=size);
  return buffer;
}

