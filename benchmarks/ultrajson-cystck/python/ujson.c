/*
Developed by ESN, an Electronic Arts Inc. studio. 
Copyright (c) 2014, Electronic Arts Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of ESN, Electronic Arts Inc. nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS INC. BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Portions of code from MODP_ASCII - Ascii transformations (upper/lower, etc)
http://code.google.com/p/stringencoders/
Copyright (c) 2007  Nick Galbreath -- nickg [at] modp [dot] com. All rights reserved.

Numeric decoder derived from from TCL library
http://www.opensource.apple.com/source/tcl/tcl-14/tcl/license.terms
* Copyright (c) 1988-1993 The Regents of the University of California.
* Copyright (c) 1994 Sun Microsystems, Inc.
*/

#include  "../../include/Cystck.h"
#include "py_defines.h"
#include "version.h"

/* objToJSON */
extern CystckDef objTo_JSON;
extern CystckDef objToJSON_encode;
extern CystckDef objToJSON_File;
void initObjToJSON(Py_State *S);

/* JSONToObj */
extern CystckDef JSONTo_Obj;
extern CystckDef JSONToObj_decode;
extern CystckDef JSONFileTo_Obj;




static CystckDef *module_defines[] = {
    &objToJSON_File,
    &JSONFileTo_Obj,
    &JSONTo_Obj,
    &objTo_JSON,
    &JSONToObj_decode,
    &objToJSON_encode,
    NULL
};

static CyModuleDef moduledef = {
  .m_name = "ujson_Cystck",
  .m_doc = 0,
  .m_size = -1,
  .m_methods =  module_defines,
};

CyMODINIT_FUNC (ujson_cystck)
CyInit_ujson_cystck(Py_State *Py_state)
{
    Cystck_Object module;
    initObjToJSON(Py_state);
    module = CystckModule_Create(Py_state,&moduledef);
    if (Cystck_IsNULL(module))
    {
      return 0;
    }
    Cystck_Object version_string;
    version_string = CystckUnicode_FromString(Py_state, UJSON_VERSION);
    if (Cystck_IsNULL(version_string))
    {
      Cystck_pop(Py_state,module);
      return 0;
    }
    Cystck_SetAttr_s(Py_state, module, "__version__", version_string);
   return module; 
}