/* -*- mode: c++; c-basic-offset: 4 -*- */

/*
  _ttconv.c

  Python wrapper for TrueType conversion library in ../ttconv.
 */
#define PY_SSIZE_T_CLEAN
#include "mplutils.h"

#include <cstring>
#include "ttconv/pprdrv.h"
#include "py_exceptions.h"
#include <vector>
#include <cassert>
#include "../../include/Cystck.h"

/**
 * An implementation of TTStreamWriter that writes to a Python
 * file-like object.
 */
class PythonFileWriter : public TTStreamWriter
{
    Cystck_Object _write_method;
    Py_State *_S;

  public:
    PythonFileWriter()
    {
        _write_method = 0;
        _S = NULL;

    }

    ~PythonFileWriter()
    {
        if (_S){
            Cystck_DECREF(_S, _write_method);
            _S = NULL;
        }
    }

    void set ( Py_State *S, Cystck_Object write_method)
    {
        if (_S) {
            Cystck_DECREF(_S, _write_method);
            _S = NULL;
        }
        _write_method = Cystck_Dup(S, write_method);
        _S = S;
    }

    virtual void write(const char *a)
    {
        Cystck_Object result = 0;
        if (!Cystck_IsNULL(_write_method)) {
            Cystck_Object decoded = CystckUnicode_DecodeLatin1(_S, a, strlen(a), "");
            if ( Cystck_IsNULL(decoded) ) {
                throw py::exception();
            }
            Cystck_pushobject(_S, decoded);
            Cystck_Object tuple[] ={decoded};
            Cystck_Object argtuple = CystckTuple_FromArray(_S, tuple,1);
            Cystck_pushobject(_S, argtuple);
            result = Cystck_CallTupleDict(_S, _write_method, argtuple, 0);
            Cystck_pop(_S, argtuple);
            Cystck_pop(_S, decoded);
            if (Cystck_IsNULL(result) ) {
                throw py::exception();
            }
            Cystck_DECREF(_S, result);
        }
    }
};

int fileobject_to_PythonFileWriter(Py_State *S, Cystck_Object object, void *address)
{
    PythonFileWriter *file_writer = (PythonFileWriter *)address;

    Cystck_Object write_method = Cystck_GetAttr_s(S, object, "write");
    if ( Cystck_IsNULL(write_method) || !Cystck_Callable_Check(S, write_method)) {
        CystckErr_SetString(S, S->Cystck_TypeError, "Expected a file-like object with a write method.");
        return 0;
    }
    Cystck_pushobject(S, write_method);
    file_writer->set(S, write_method);
    Cystck_pop(S, write_method);

    return 1;
}

int pyiterable_to_vector_int( Py_State *S, Cystck_Object object, void *address)
{
    std::vector<int> *result = (std::vector<int> *)address;

    Cystck_ssize_t nentries = Cystck_Length(S, object);
    Cystck_Object item;
    for (Cystck_ssize_t i = 0; i < nentries; ++i) {
        item = Cystck_GetItem_i(S, object, i);
        Cystck_pushobject(S, item);
        long value = CystckLong_AsLong(S, item);
        Cystck_pop(S, item);
        if (value == -1 && Cystck_Err_Occurred(S)) {
            return 0;
        }
        result->push_back((int)value);
    }

    return 1;
}

static Cystck_Object convert_ttf_to_ps(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    const char *filename;
    PythonFileWriter output;
    int fonttype;
    std::vector<int> glyph_ids;
    Cystck_Object h_filename = 0;
    Cystck_Object h_output = 0;
    Cystck_Object h_glyph_ids = 0;

    static const char *kwlist[] = { "filename", "output", "fonttype", "glyph_ids", NULL };
    if (!CystckArg_parseTupleAndKeywords(S, args,
                                     kwds,
                                     "OOi|O&:convert_ttf_to_ps",
                                     (const char **)kwlist,
                                     &h_filename,
                                     &h_output,
                                     &fonttype,
                                     &h_glyph_ids)) {
        return -1;
    }

    if (!CystckBytes_Check(S, h_filename)) {
        CystckErr_SetString(S, S->Cystck_TypeError, "convert_ttf_to_ps");
        return -1;
    }
    filename = CystckBytes_AsString(S, h_filename);
    if (!fileobject_to_PythonFileWriter(S, h_output, &output) || 
            (!Cystck_IsNULL(h_glyph_ids) && !pyiterable_to_vector_int(S, h_glyph_ids, &glyph_ids))) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "convert_ttf_to_ps"); 
        return -1;
    }

    if (fonttype != 3 && fonttype != 42) {
        CystckErr_SetString(S, S->Cystck_ValueError,
                        "fonttype must be either 3 (raw Postscript) or 42 "
                        "(embedded Truetype)");
        return -1;
    }

    try
    {
        insert_ttfont(filename, output, (font_type_enum)fonttype, glyph_ids);
    }
    catch (TTException &e)
    {
        CystckErr_SetString(S, S->Cystck_RuntimeError, e.getMessage());
        return -1;
    }
    catch (const py::exception &)
    {
        return -1;
    }
    catch (...)
    {
        CystckErr_SetString(S, S->Cystck_RuntimeError, "Unknown C++ exception");
        return -1;
    }

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;
}

CystckDef_METH(convert_ttf_to_ps_def, "convert_ttf_to_ps", convert_ttf_to_ps, Cystck_METH_KEYWORDS,
"convert_ttf_to_ps(filename, output, fonttype, glyph_ids)\n"
"\n"
"Converts the Truetype font into a Type 3 or Type 42 Postscript font, "
"optionally subsetting the font to only the desired set of characters.\n"
"\n"
"filename is the path to a TTF font file.\n"
"output is a Python file-like object with a write method that the Postscript "
"font data will be written to.\n"
"fonttype may be either 3 or 42.  Type 3 is a \"raw Postscript\" font. "
"Type 42 is an embedded Truetype font.  Glyph subsetting is not supported "
"for Type 42 fonts.\n"
"glyph_ids (optional) is a list of glyph ids (integers) to keep when "
"subsetting to a Type 3 font.  If glyph_ids is not provided or is None, "
"then all glyphs will be included.  If any of the glyphs specified are "
"composite glyphs, then the component glyphs will also be included.")

static CystckDef *module_defines[] = {
    &convert_ttf_to_ps_def,
    NULL
};
static const char *module_docstring =
    "Module to handle converting and subsetting TrueType "
    "fonts to Postscript Type 3, Postscript Type 42 and "
    "Pdf Type 3 fonts.";

static CyModuleDef moduledef = {
  .m_name = "_ttconv",
  .m_doc = module_docstring,
  .m_size = -1,
  .m_methods = module_defines,
};


#ifdef __cplusplus
extern "C" {
#endif

#pragma GCC visibility push(default)
CyMODINIT_FUNC(_ttconv)
CyInit__ttconv(Py_State *S)
{
    return CystckModule_Create(S, &moduledef);
}

#pragma GCC visibility pop
#ifdef __cplusplus
}
#endif
