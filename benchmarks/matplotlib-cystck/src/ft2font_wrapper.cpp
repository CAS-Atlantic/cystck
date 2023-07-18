#include "mplutils.h"
#include "ft2font.h"
#include "py_converters.h"
#include "py_exceptions.h"
#include "numpy_cpp.h"

// From Python
#include <structmember.h>

#include <set>
#include <algorithm>

#define STRINGIFY(s) XSTRINGIFY(s)
#define XSTRINGIFY(s) #s

static Cystck_Object convert_xys_to_array(Py_State *S, std::vector<double> &xys)
{
    npy_intp dims[] = {(npy_intp)xys.size() / 2, 2 };
    if (dims[0] > 0) {
        return Cystck_FromPyObject(S, PyArray_SimpleNewFromData(2, dims, NPY_DOUBLE, &xys[0]));
    } else {
        return Cystck_FromPyObject(S, PyArray_SimpleNew(2, dims, NPY_DOUBLE));
    }
}

/**********************************************************************
 * FT2Image
 * */

typedef struct
{
    Cystck_HEAD
    FT2Image *x;
    Cystck_ssize_t shape[2];
    Cystck_ssize_t strides[2];
    Cystck_ssize_t suboffsets[2];
} PyFT2Image;

CystckType_HELPERS(PyFT2Image)

// static PyTypeObject PyFT2ImageType;

static Cystck_Object PyFT2Image_new(Py_State *S, Cystck_Object type, Cystck_Object args, Cystck_Object kwds)
{
    PyFT2Image *self;
    Cystck_Object h_self = Cystck_New(S, type, &self);
    self->x = NULL;
    Cystck_pushobject(S, h_self);
    return 1;
}

static int PyFT2Image_init(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    PyFT2Image *self = PyFT2Image_AsStruct(S, S->self);
    double width;
    double height;

    if (!CystckArg_parseTuple(S, args, "dd:FT2Image", &width, &height)) {
        return -1;
    }

    CALL_CPP_INIT_CYSTCK(S, "FT2Image", (self->x = new FT2Image(width, height)));

    return 0;
}

static void PyFT2Image_dealloc(Py_State *S)
{
    PyFT2Image *self = PyFT2Image_AsStruct(S, S->self);
    delete self->x;
    //Py_TYPE(self)->tp_free((PyObject *)self);
}

const char *PyFT2Image_draw_rect__doc__ =
    "draw_rect(self, x0, y0, x1, y1)\n"
    "--\n\n"
    "Draw an empty rectangle to the image.\n";

static Cystck_Object  PyFT2Image_draw_rect(Py_State *S, Cystck_Object args)
{
    PyFT2Image *self = PyFT2Image_AsStruct(S, S->self);
    double x0, y0, x1, y1;

    if (!CystckArg_parseTuple(S, args, "dddd:draw_rect", &x0, &y0, &x1, &y1)) {
        return -1;
    }

    CALL_CPP_CYSTCK(S,"draw_rect", (self->x->draw_rect(x0, y0, x1, y1)));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

    return 1;
}

const char *PyFT2Image_draw_rect_filled__doc__ =
    "draw_rect_filled(self, x0, y0, x1, y1)\n"
    "--\n\n"
    "Draw a filled rectangle to the image.\n";

static Cystck_Object PyFT2Image_draw_rect_filled(Py_State *S, Cystck_Object args)
{
    PyFT2Image *self = PyFT2Image_AsStruct(S, S->self);
    double x0, y0, x1, y1;

    if (!CystckArg_parseTuple(S, args, "dddd:draw_rect_filled", &x0, &y0, &x1, &y1)) {
        return -1;
    }

    CALL_CPP_CYSTCK(S,"draw_rect_filled", (self->x->draw_rect_filled(x0, y0, x1, y1)));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

    return 1;
}

static int PyFT2Image_get_buffer(Py_State *S, Cystck_Object c_self, Cystck_buffer *buf, int flags)
{
    PyFT2Image *self = PyFT2Image_AsStruct(S, c_self);
    FT2Image *im = self->x;

    buf->obj = Cystck_Dup(S, c_self);
    buf->buf = im->get_buffer();
    buf->len = im->get_width() * im->get_height();
    buf->readonly = 0;
    buf->format = (char *)"B";
    buf->ndim = 2;
    self->shape[0] = im->get_height();
    self->shape[1] = im->get_width();
    buf->shape = self->shape;
    self->strides[0] = im->get_width();
    self->strides[1] = 1;
    buf->strides = self->strides;
    buf->suboffsets = NULL;
    buf->itemsize = 1;
    buf->internal = NULL;

    return 1;
}

static Cystck_Object h_PyFT2ImageType;

CystckDef_SLOT(PyFT2Image_new_def, PyFT2Image_new, Cystck_tp_new)
CystckDef_SLOT(PyFT2Image_init_def, PyFT2Image_init, Cystck_tp_init)
CystckDef_SLOT(PyFT2Image_get_buffer_def, PyFT2Image_get_buffer, Cystck_bf_getbuffer)
CystckDef_SLOT(PyFT2Image_dealloc_def, PyFT2Image_dealloc, Cystck_tp_dealloc)

CystckDef_METH(PyFT2Image_draw_rect_def, "draw_path", PyFT2Image_draw_rect, Cystck_METH_VARARGS, PyFT2Image_draw_rect__doc__)
CystckDef_METH(PyFT2Image_draw_rect_filled_def, "draw_rect_filled", PyFT2Image_draw_rect_filled, Cystck_METH_VARARGS, PyFT2Image_draw_rect_filled__doc__)

CystckDef *PyFT2Image_defines[] = {
    // slots
    &PyFT2Image_new_def,
    &PyFT2Image_init_def,
    &PyFT2Image_get_buffer_def,
    &PyFT2Image_dealloc_def,

    // methods
    &PyFT2Image_draw_rect_def,
    &PyFT2Image_draw_rect_filled_def,
    NULL
};

Cystck_Type_Spec PyFT2Image_type_spec = {
    .name = "matplotlib.ft2font.FT2Image",
    .basicsize = sizeof(PyFT2Image),
    .flags = Cystck_TPFLAGS_DEFAULT | Cystck_TPFLAGS_BASETYPE,
    .m_methods = PyFT2Image_defines,
};
/**********************************************************************
 * Glyph
 * */

typedef struct
{
    Cystck_HEAD
    size_t glyphInd;
    long width;
    long height;
    long horiBearingX;
    long horiBearingY;
    long horiAdvance;
    long linearHoriAdvance;
    long vertBearingX;
    long vertBearingY;
    long vertAdvance;
    FT_BBox bbox;
} PyGlyph;

CystckType_HELPERS(PyGlyph)

// static PyTypeObject PyGlyphType;

static Cystck_Object PyGlyph_from_FT2Font( Py_State *S, Cystck_Object m, const FT2Font *font)
{
    Cystck_Object h_PyGlyphType = Cystck_GetAttr_s(S, m, "PyGlyph");
    Cystck_pushobject(S, h_PyGlyphType);
    const FT_Face &face = font->get_face();
    const long hinting_factor = font->get_hinting_factor();
    const FT_Glyph &glyph = font->get_last_glyph();

    PyGlyph *self;
    Cystck_Object h_self = Cystck_New(S, h_PyGlyphType, &self);

    self->glyphInd = font->get_last_glyph_index();
    
    FT_Glyph_Get_CBox(glyph, ft_glyph_bbox_subpixels, &self->bbox);

    self->width = face->glyph->metrics.width / hinting_factor;
    self->height = face->glyph->metrics.height;
    self->horiBearingX = face->glyph->metrics.horiBearingX / hinting_factor;
    self->horiBearingY = face->glyph->metrics.horiBearingY;
    self->horiAdvance = face->glyph->metrics.horiAdvance;
    self->linearHoriAdvance = face->glyph->linearHoriAdvance / hinting_factor;
    self->vertBearingX = face->glyph->metrics.vertBearingX;
    self->vertBearingY = face->glyph->metrics.vertBearingY;
    self->vertAdvance = face->glyph->metrics.vertAdvance;

    return h_self;
}


static void PyGlyph_dealloc(Py_State *S)
{
    // Py_TYPE(self)->tp_free((PyObject *)self);
}

static Cystck_Object PyGlyph_get_bbox(Py_State *S, void *closure)
{
    PyGlyph *self = PyGlyph_AsStruct(S, S->self);
    Cystck_pushobject(S, Cystck_BuildValue(S,
        "llll", self->bbox.xMin, self->bbox.yMin, self->bbox.xMax, self->bbox.yMax));
    return 1;
}

CystckDef_SLOT(PyGlyph_dealloc_def, PyGlyph_dealloc, Cystck_tp_dealloc)

CystckDef_GET(PyGlyph_get_bbox_get, "bbox", PyGlyph_get_bbox, NULL, NULL)

CystckDef_MEMBER(width_member, "width", CystckMember_LONG, offsetof(PyGlyph, width), 1, "")
CystckDef_MEMBER(height_member, "height", CystckMember_LONG, offsetof(PyGlyph, height), 1, "")
CystckDef_MEMBER(horiBearingX_member, "horiBearingX", CystckMember_LONG, offsetof(PyGlyph, horiBearingX), 1,  "")
CystckDef_MEMBER(horiBearingY_member, "horiBearingY", CystckMember_LONG, offsetof(PyGlyph, horiBearingY), 1, "")
CystckDef_MEMBER(horiAdvance_member, "horiAdvance", CystckMember_LONG, offsetof(PyGlyph, horiAdvance), 1,  "")
CystckDef_MEMBER(linearHoriAdvance_member, "linearHoriAdvance", CystckMember_LONG, offsetof(PyGlyph, linearHoriAdvance), 1, "")
CystckDef_MEMBER(vertBearingX_member, "vertBearingX", CystckMember_LONG, offsetof(PyGlyph, vertBearingX), 1,  "")
CystckDef_MEMBER(vertBearingY_member, "vertBearingY", CystckMember_LONG, offsetof(PyGlyph, vertBearingY), 1,  "")
CystckDef_MEMBER(vertAdvance_member, "vertAdvance", CystckMember_LONG, offsetof(PyGlyph, vertAdvance), 1, "")

CystckDef *PyGlyph_defines[] = {
    // slots
    &PyGlyph_dealloc_def,
    
    // getsets
    &PyGlyph_get_bbox_get,

    // members
    &width_member,
    &height_member,
    &horiBearingX_member,
    &horiBearingY_member,
    &horiAdvance_member,
    &linearHoriAdvance_member,
    &vertBearingX_member,
    &vertBearingY_member,
    &vertAdvance_member,
    NULL
};

Cystck_Type_Spec PyGlyph_type_spec = {
    .name = "matplotlib.ft2font.Glyph",
    .basicsize = sizeof(PyGlyph),
    .flags = Cystck_TPFLAGS_DEFAULT | Cystck_TPFLAGS_BASETYPE,
    .m_methods = PyGlyph_defines,
};


/**********************************************************************
 * FT2Font
 * */

struct PyFT2Font
{
    Cystck_HEAD
    FT2Font *x;
    PyObject *py_file;
    FT_StreamRec stream;
    Cystck_ssize_t shape[2];
    Cystck_ssize_t strides[2];
    Cystck_ssize_t suboffsets[2];
    std::vector<PyObject *> fallbacks;
};
CystckType_HELPERS(PyFT2Font)

static PyTypeObject PyFT2FontType;

static unsigned long read_from_file_callback(FT_Stream stream,
                                             unsigned long offset,
                                             unsigned char *buffer,
                                             unsigned long count)
{
    PyObject *py_file = ((PyFT2Font *)stream->descriptor.pointer)->py_file;
    PyObject *seek_result = NULL, *read_result = NULL;
    Cystck_ssize_t n_read = 0;
    if (!(seek_result = PyObject_CallMethod(py_file, "seek", "k", offset))
        || !(read_result = PyObject_CallMethod(py_file, "read", "k", count))) {
        goto exit;
    }
    char *tmpbuf;
    if (PyBytes_AsStringAndSize(read_result, &tmpbuf, &n_read) == -1) {
        goto exit;
    }
    memcpy(buffer, tmpbuf, n_read);
exit:
    Py_XDECREF(seek_result);
    Py_XDECREF(read_result);
    if (PyErr_Occurred()) {
        PyErr_WriteUnraisable(py_file);
        if (!count) {
            return 1;  // Non-zero signals error, when count == 0.
        }
    }
    return (unsigned long)n_read;
}

static void close_file_callback(FT_Stream stream)
{
    PyObject *type, *value, *traceback;
    PyErr_Fetch(&type, &value, &traceback);
    PyFT2Font *self = (PyFT2Font *)stream->descriptor.pointer;
    PyObject *close_result = NULL;
    if (!(close_result = PyObject_CallMethod(self->py_file, "close", ""))) {
        goto exit;
    }
exit:
    Py_XDECREF(close_result);
    Py_CLEAR(self->py_file);
    if (PyErr_Occurred()) {
        PyErr_WriteUnraisable((PyObject*)self);
    }
    PyErr_Restore(type, value, traceback);
}

static Cystck_Object PyFT2Font_new(Py_State *S, Cystck_Object type, Cystck_Object args, Cystck_Object kwds)
{
    PyFT2Font *self;
    Cystck_Object h_self = Cystck_New(S, type, &self);
    self->x = NULL;
    self->py_file = 0;
    memset(&self->stream, 0, sizeof(FT_StreamRec));
    Cystck_pushobject(S, h_self);
    return 1;
}

static Cystck_Object CystckCall(Py_State *S, Cystck_Object obj, const char *func_name, Cystck_Object argtuple, Cystck_Object kwds) {
    if (!Cystck_HasAttr_s(S, obj, func_name)) {
        return Cystck_NULL;
    }
    Cystck_Object func = Cystck_GetAttr_s(S, obj, func_name);
    Cystck_pushobject(S, func);
    if (Cystck_IsNULL(func) || !Cystck_Callable_Check(S, func)) {
        Cystck_pop(S, func);
        return 0;
    }
    Cystck_Object result = Cystck_CallTupleDict(S, func, argtuple, kwds);
    if (Cystck_IsNULL(result)) {
        Cystck_pop(S, func);
        return Cystck_NULL;
    }
    Cystck_pop(S, func);
    return result;
}

static Cystck_Object CystckPackLongAndCallMethod(Py_State *S, Cystck_Object obj, const char *func_name, unsigned long val) {
    Cystck_Object h_val = CystckLong_FromUnsignedLong(S, val);
    Cystck_pushobject(S, h_val);
    Cystck_Object tuple[] = {h_val};
    Cystck_Object argtuple = CystckTuple_FromArray(S, tuple, 1);
    Cystck_Object result = CystckCall(S, obj, func_name, argtuple, 0);
    Cystck_pop(S, h_val);
    Cystck_pushobject(S, argtuple);
    if (Cystck_IsNULL(result)) {
        Cystck_pop(S, argtuple);
        return Cystck_NULL;
    }
    Cystck_pop(S, argtuple);
    return result;
}

static Cystck_Object CystckCallOpen(Py_State *S, Cystck_Object open, Cystck_Object filename, const char *flags) {
    Cystck_Object h_flags = CystckUnicode_FromString(S, flags);
    Cystck_pushobject(S, h_flags);
    Cystck_Object tuple[] = {filename, h_flags};
    Cystck_Object argtuple = CystckTuple_FromArray(S, tuple, 2);
    Cystck_pop(S, h_flags);
    Cystck_pushobject(S, argtuple);
    Cystck_Object result = Cystck_CallTupleDict(S, open, argtuple, 0);
    if (Cystck_IsNULL(result)) {
        Cystck_pop(S, argtuple);
        return 0;
    }
    Cystck_pop(S, argtuple);
    return result;
}

static Cystck_Object CystckCallMethodRead(Py_State *S, Cystck_Object filename, const char *func_name, long size) {
    Cystck_Object h_size = CystckLong_FromLong(S, size);
    Cystck_pushobject(S, h_size);
    Cystck_Object tuple[] = {h_size};
    Cystck_Object argtuple = CystckTuple_FromArray(S, tuple, 1);
    Cystck_pop(S, h_size);
    Cystck_pushobject(S, argtuple);
    Cystck_Object result = CystckCall(S, filename, func_name, argtuple, 0);
    if (Cystck_IsNULL(result)) {
        Cystck_pop(S, argtuple);
        return 0;
    }
    Cystck_pop(S, argtuple);
    return result;
}

const char *PyFT2Font_init__doc__ =
    "FT2Font(filename, hinting_factor=8, *, _fallback_list=None, _kerning_factor=0)\n"
    "--\n\n"
    "Create a new FT2Font object.\n"
    "\n"
    "Parameters\n"
    "----------\n"
    "filename : str or file-like\n"
    "    The source of the font data in a format (ttf or ttc) that FreeType can read\n\n"
    "hinting_factor : int, optional\n"
    "    Must be positive. Used to scale the hinting in the x-direction\n"
    "_fallback_list : list of FT2Font, optional\n"
    "    A list of FT2Font objects used to find missing glyphs.\n\n"
    "    .. warning::\n"
    "        This API is both private and provisional: do not use it directly\n\n"
    "_kerning_factor : int, optional\n"
    "    Used to adjust the degree of kerning.\n\n"
    "    .. warning::\n"
    "        This API is private: do not use it directly\n\n"
    "Attributes\n"
    "----------\n"
    "num_faces\n"
    "    Number of faces in file.\n"
    "face_flags, style_flags : int\n"
    "    Face and style flags; see the ft2font constants.\n"
    "num_glyphs\n"
    "    Number of glyphs in the face.\n"
    "family_name, style_name\n"
    "    Face family and style name.\n"
    "num_fixed_sizes\n"
    "    Number of bitmap in the face.\n"
    "scalable\n"
    "    Whether face is scalable; attributes after this one are only\n"
    "    defined for scalable faces.\n"
    "bbox\n"
    "    Face global bounding box (xmin, ymin, xmax, ymax).\n"
    "units_per_EM\n"
    "    Number of font units covered by the EM.\n"
    "ascender, descender\n"
    "    Ascender and descender in 26.6 units.\n"
    "height\n"
    "    Height in 26.6 units; used to compute a default line spacing\n"
    "    (baseline-to-baseline distance).\n"
    "max_advance_width, max_advance_height\n"
    "    Maximum horizontal and vertical cursor advance for all glyphs.\n"
    "underline_position, underline_thickness\n"
    "    Vertical position and thickness of the underline bar.\n"
    "postscript_name\n"
    "    PostScript name of the font.\n";

static int PyFT2Font_init(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    // self->S = S;
    Cystck_Object filename = 0, open = 0, data = 0, fallback_list = 0;
    FT_Open_Args open_args;
    long hinting_factor = 8;
    int kerning_factor = 0;
    const char *names[] = {
        "filename", "hinting_factor", "_fallback_list", "_kerning_factor", NULL
    };
    std::vector<FT2Font *> fallback_fonts;
    if (!CystckArg_parseTupleAndKeywords(S, 
             args, kwds, "O|l$Oi:FT2Font", (const char **)names, &filename,
             &hinting_factor, &fallback_list, &kerning_factor)) {
        return -1;
    }
    if (hinting_factor <= 0) {
      CystckErr_SetString(S, S->Cystck_ValueError,
                      "hinting_factor must be greater than 0");
      goto exit;
    }

    self->stream.base = NULL;
    self->stream.size = 0x7fffffff;  // Unknown size.
    self->stream.pos = 0;
    self->stream.descriptor.pointer = self;
    self->stream.read = &read_from_file_callback;
    memset((void *)&open_args, 0, sizeof(FT_Open_Args));
    open_args.flags = FT_OPEN_STREAM;
    open_args.stream = &self->stream;

    if ( Cystck_IsNULL(fallback_list)) {
        if (!CystckList_Check(S, fallback_list)) {
            CystckErr_SetString(S, S->Cystck_TypeError, "Fallback list must be a list");
            goto exit;
        }
        Cystck_ssize_t size = CystckList_Size(S, fallback_list);

        // go through fallbacks once to make sure the types are right
        for (Cystck_ssize_t i = 0; i < size; ++i) {
            // this returns a borrowed reference
            Cystck_Object item = CystckList_GetItem(S, fallback_list, i);
            if (!Cystck_IsInstance(S, item, Cystck_FromTypeObject(self))) {
                CystckErr_SetString(S, S->Cystck_TypeError, "Fallback fonts must be FT2Font objects.");
                goto exit;
            }
        }
        // go through a second time to add them to our lists
        for (Cystck_ssize_t i = 0; i < size; ++i) {
            // this returns a borrowed reference
            Cystck_Object item = CystckList_GetItem(S, fallback_list, i);
            // Increase the ref count, we will undo this in dealloc this makes
            // sure things do not get gc'd under us!
            //Py_INCREF(item);
            self->fallbacks.push_back(Cystck_AsPyObject(S, Cystck_Dup(S, item)));
            // Also (locally) cache the underlying FT2Font objects. As long as
            // the Python objects are kept alive, these pointer are good.
            FT2Font *fback = reinterpret_cast<PyFT2Font *>(Cystck_AsPyObject(S, Cystck_Dup(S, item)))->x;
            fallback_fonts.push_back(fback);
        }
    }

    if (CystckBytes_Check(S, filename) || CystckUnicode_Check(S, filename)) {
        Cystck_Object builtins = Cystck_Import_ImportModule("builtins");
        Cystck_pushobject(S, builtins);
        open = Cystck_GetAttr_s(S, builtins, "open");
        Cystck_pop(S, builtins);
        if (Cystck_IsNULL(open)) {
            goto exit;
        }
        Cystck_pushobject(S, open);
        self->py_file =  Cystck_AsPyObject(S,CystckCallOpen(S, open, filename, "rb"));
        Cystck_pop(S, open);
        if (Cystck_IsNULL(self->py_file)) {
            goto exit;
        }
        self->stream.close = &close_file_callback;
    } else if (!Cystck_HasAttr_s(S, filename, "read")
               || Cystck_IsNULL(data = CystckCallMethodRead(S, filename, "read", 0))
               || !CystckBytes_Check(S, data)) {
        CystckErr_SetString(S, S->Cystck_TypeError,
                        "First argument must be a path or binary-mode file object");
        Cystck_CLEAR(S, data);
        goto exit;
    } else {
        self->py_file = Cystck_AsPyObject(S, Cystck_Dup(S, filename));
        self->stream.close = NULL;
    }
    Cystck_CLEAR(S, data);

    CALL_CPP_FULL(
        "FT2Font", (self->x = new FT2Font(open_args, hinting_factor, fallback_fonts)),
        Cystck_CLEAR(S, Cystck_FromPyObject(S, self->py_file)), -1);

    CALL_CPP_INIT_CYSTCK(S, "FT2Font->set_kerning_factor", (self->x->set_kerning_factor(kerning_factor)));
exit:
    return Cystck_Err_Occurred(S) ? -1 : 0;
}

static void  PyFT2Font_dealloc(Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    delete self->x;
    for (size_t i = 0; i < self->fallbacks.size(); i++) {
        Py_DECREF(self->fallbacks[i]);
    }

    Py_XDECREF(self->py_file);
    // Py_TYPE(self)->tp_free((PyObject *)self);
}

const char *PyFT2Font_clear__doc__ =
    "clear(self)\n"
    "--\n\n"
    "Clear all the glyphs, reset for a new call to `.set_text`.\n";

static Cystck_Object PyFT2Font_clear(Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    CALL_CPP_CYSTCK(S,"clear", (self->x->clear()));
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None) );
    return 1;
}

const char *PyFT2Font_set_size__doc__ =
    "set_size(self, ptsize, dpi)\n"
    "--\n\n"
    "Set the point size and dpi of the text.\n";

static Cystck_Object PyFT2Font_set_size(Py_State *S, Cystck_Object args)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    double ptsize;
    double dpi;

    if (!CystckArg_parseTuple(S, args, "dd:set_size", &ptsize, &dpi)) {
        return -1;
    }

    CALL_CPP_CYSTCK(S,"set_size", (self->x->set_size(ptsize, dpi)));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None) );
    
    return 1;
}


const char *PyFT2Font_set_charmap__doc__ =
    "set_charmap(self, i)\n"
    "--\n\n"
    "Make the i-th charmap current.\n";

static Cystck_Object PyFT2Font_set_charmap(Py_State *S, Cystck_Object args)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    int i;

    if (!CystckArg_parseTuple(S, args, "i:set_charmap", &i)) {
        return -1;
    }

    CALL_CPP_CYSTCK(S,"set_charmap", (self->x->set_charmap(i)));
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None) );
    
    return 1;
}

const char *PyFT2Font_select_charmap__doc__ =
    "select_charmap(self, i)\n"
    "--\n\n"
    "Select a charmap by its FT_Encoding number.\n";

static Cystck_Object PyFT2Font_select_charmap(Py_State *S, Cystck_Object args)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    unsigned long i;

    if (!CystckArg_parseTuple(S, args, "k:select_charmap", &i)) {
        return -1;
    }

    CALL_CPP_CYSTCK(S,"select_charmap", self->x->select_charmap(i));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None) );
    
    return 1;
}

const char *PyFT2Font_get_kerning__doc__ =
    "get_kerning(self, left, right, mode)\n"
    "--\n\n"
    "Get the kerning between *left* and *right* glyph indices.\n"
    "*mode* is a kerning mode constant:\n\n"
    "    - KERNING_DEFAULT  - Return scaled and grid-fitted kerning distances\n"
    "    - KERNING_UNFITTED - Return scaled but un-grid-fitted kerning distances\n"
    "    - KERNING_UNSCALED - Return the kerning vector in original font units\n";

static Cystck_Object PyFT2Font_get_kerning(Py_State *S, Cystck_Object args)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    FT_UInt left, right, mode;
    int result;
    int fallback = 1;

    if (!CystckArg_parseTuple(S, args, "III:get_kerning", &left, &right, &mode)) {
        return -1;
    }

    CALL_CPP_CYSTCK(S,"get_kerning", (result = self->x->get_kerning(left, right, mode, (bool)fallback)));

    Cystck_pushobject(S, CystckLong_FromLong(S, result));
    return 1;
}

const char *PyFT2Font_get_fontmap__doc__ =
    "_get_fontmap(self, string)\n"
    "--\n\n"
    "Get a mapping between characters and the font that includes them.\n"
    "A dictionary mapping unicode characters to PyFT2Font objects.";
static PyObject *PyFT2Font_get_fontmap(PyFT2Font *self, PyObject *args, PyObject *kwds)
{
    PyObject *textobj;
    const char *names[] = { "string", NULL };

    if (!PyArg_ParseTupleAndKeywords(
            args, kwds, "O:_get_fontmap", (char **)names, &textobj)) {
        return NULL;
    }

    std::set<FT_ULong> codepoints;
    size_t size;

    if (PyUnicode_Check(textobj)) {
        size = PyUnicode_GET_LENGTH(textobj);
        for (size_t i = 0; i < size; ++i) {
            codepoints.insert(PyUnicode_ReadChar(textobj, i));
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "string must be str");
        return NULL;
    }
    PyObject *char_to_font;
    if (!(char_to_font = PyDict_New())) {
        return NULL;
    }
    for (auto it = codepoints.begin(); it != codepoints.end(); ++it) {
        auto x = *it;
        PyObject* target_font;
        int index;
        if (self->x->get_char_fallback_index(x, index)) {
            if (index >= 0) {
                target_font = self->fallbacks[index];
            } else {
                target_font = (PyObject *)self;
            }
        } else {
            // TODO Handle recursion!
            target_font = (PyObject *)self;
        }

        PyObject *key = NULL;
        bool error = (!(key = PyUnicode_FromFormat("%c", x))
                      || (PyDict_SetItem(char_to_font, key, target_font) == -1));
        Py_XDECREF(key);
        if (error) {
            Py_DECREF(char_to_font);
            PyErr_SetString(PyExc_ValueError, "Something went very wrong");
            return NULL;
        }
    }
    return char_to_font;
}


const char *PyFT2Font_set_text__doc__ =
    "set_text(self, string, angle, flags=32)\n"
    "--\n\n"
    "Set the text *string* and *angle*.\n"
    "*flags* can be a bitwise-or of the LOAD_XXX constants;\n"
    "the default value is LOAD_FORCE_AUTOHINT.\n"
    "You must call this before `.draw_glyphs_to_bitmap`.\n"
    "A sequence of x,y positions is returned.\n";

static Cystck_Object PyFT2Font_set_text(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self); 
    Cystck_Object textobj;
    double angle = 0.0;
    FT_Int32 flags = FT_LOAD_FORCE_AUTOHINT;
    std::vector<double> xys;
    const char *names[] = { "string", "angle", "flags", NULL };

    /* This makes a technically incorrect assumption that FT_Int32 is
       int. In theory it can also be long, if the size of int is less
       than 32 bits. This is very unlikely on modern platforms. */
    if (!CystckArg_parseTupleAndKeywords(S, 
             args, kwds, "O|di:set_text", (const char **)names, &textobj, &angle, &flags)) {
        return -1;
    }

    std::vector<uint32_t> codepoints;
    size_t size;

    if (CystckUnicode_Check(S, textobj)) {
        size = Cystck_Length(S, textobj);
        codepoints.resize(size);
        for (size_t i = 0; i < size; ++i) {
            codepoints[i] = CystckUnicode_ReadChar(S,textobj, i);
        }
    } else {
        CystckErr_SetString(S, S->Cystck_TypeError, "set_text requires str-input.");
        return -1;
    }

    uint32_t* codepoints_array = NULL;
    if (size > 0) {
        codepoints_array = &codepoints[0];
    }
    CALL_CPP_CYSTCK(S,"set_text", self->x->set_text(S, size, codepoints_array, angle, flags, xys));

    Cystck_pushobject(S, convert_xys_to_array(S, xys));

    return 1;
}

const char *PyFT2Font_get_num_glyphs__doc__ =
    "get_num_glyphs(self)\n"
    "--\n\n"
    "Return the number of loaded glyphs.\n";

static Cystck_Object PyFT2Font_get_num_glyphs(Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_num_glyphs()));
    return 1;
}

const char *PyFT2Font_load_char__doc__ =
    "load_char(self, charcode, flags=32)\n"
    "--\n\n"
    "Load character with *charcode* in current fontfile and set glyph.\n"
    "*flags* can be a bitwise-or of the LOAD_XXX constants;\n"
    "the default value is LOAD_FORCE_AUTOHINT.\n"
    "Return value is a Glyph object, with attributes\n\n"
    "- width: glyph width\n"
    "- height: glyph height\n"
    "- bbox: the glyph bbox (xmin, ymin, xmax, ymax)\n"
    "- horiBearingX: left side bearing in horizontal layouts\n"
    "- horiBearingY: top side bearing in horizontal layouts\n"
    "- horiAdvance: advance width for horizontal layout\n"
    "- vertBearingX: left side bearing in vertical layouts\n"
    "- vertBearingY: top side bearing in vertical layouts\n"
    "- vertAdvance: advance height for vertical layout\n";

static Cystck_Object PyFT2Font_load_char(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    long charcode;
    int fallback = 1;
    FT_Int32 flags = FT_LOAD_FORCE_AUTOHINT;
    const char *names[] = { "m", "charcode", "flags", NULL };
    /* This makes a technically incorrect assumption that FT_Int32 is
       int. In theory it can also be long, if the size of int is less
       than 32 bits. This is very unlikely on modern platforms. */
    Cystck_Object m;
    if (!CystckArg_parseTupleAndKeywords(S, args, kwds, "Ol|i:load_char", (const char **)names, &m, &charcode,
                                     &flags)) {
        return -1;
    }

    FT2Font *ft_object = NULL;
    CALL_CPP_CYSTCK(S,"load_char", (self->x->load_char(S,charcode, flags, ft_object, (bool)fallback)));
    
    Cystck_pushobject(S, PyGlyph_from_FT2Font(S, m, ft_object));

    return 1;
}

const char *PyFT2Font_load_glyph__doc__ =
    "load_glyph(self, glyphindex, flags=32)\n"
    "--\n\n"
    "Load character with *glyphindex* in current fontfile and set glyph.\n"
    "*flags* can be a bitwise-or of the LOAD_XXX constants;\n"
    "the default value is LOAD_FORCE_AUTOHINT.\n"
    "Return value is a Glyph object, with attributes\n\n"
    "- width: glyph width\n"
    "- height: glyph height\n"
    "- bbox: the glyph bbox (xmin, ymin, xmax, ymax)\n"
    "- horiBearingX: left side bearing in horizontal layouts\n"
    "- horiBearingY: top side bearing in horizontal layouts\n"
    "- horiAdvance: advance width for horizontal layout\n"
    "- vertBearingX: left side bearing in vertical layouts\n"
    "- vertBearingY: top side bearing in vertical layouts\n"
    "- vertAdvance: advance height for vertical layout\n";

static Cystck_Object PyFT2Font_load_glyph(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    FT_UInt glyph_index;
    FT_Int32 flags = FT_LOAD_FORCE_AUTOHINT;
    int fallback = 1;
    const char *names[] = {"m", "glyph_index", "flags", NULL };

    /* This makes a technically incorrect assumption that FT_Int32 is
       int. In theory it can also be long, if the size of int is less
       than 32 bits. This is very unlikely on modern platforms. */
    Cystck_Object m;
    if (!CystckArg_parseTupleAndKeywords(S, args, kwds, "OI|i:load_glyph", (const char **)names, &m, &glyph_index,
                                     &flags)) {
        return -1;
    }

    FT2Font *ft_object = NULL;
    CALL_CPP_CYSTCK(S,"load_glyph", (self->x->load_glyph(glyph_index, flags, ft_object, (bool)fallback)));
    
    Cystck_pushobject(S, PyGlyph_from_FT2Font(S, m, ft_object));

    return 1;
}

const char *PyFT2Font_get_width_height__doc__ =
    "get_width_height(self)\n"
    "--\n\n"
    "Get the width and height in 26.6 subpixels of the current string set by `.set_text`.\n"
    "The rotation of the string is accounted for.  To get width and height\n"
    "in pixels, divide these values by 64.\n";

static Cystck_Object PyFT2Font_get_width_height(Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    long width, height;

    CALL_CPP_CYSTCK(S,"get_width_height", (self->x->get_width_height(&width, &height)));

    Cystck_BuildValue(S, "ll", width, height);
    return 1;
}

const char *PyFT2Font_get_bitmap_offset__doc__ =
    "get_bitmap_offset(self)\n"
    "--\n\n"
    "Get the (x, y) offset in 26.6 subpixels for the bitmap if ink hangs left or below (0, 0).\n"
    "Since Matplotlib only supports left-to-right text, y is always 0.\n";

static Cystck_Object PyFT2Font_get_bitmap_offset(Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    long x, y;

    CALL_CPP_CYSTCK(S,"get_bitmap_offset", (self->x->get_bitmap_offset(&x, &y)));
    
    Cystck_BuildValue(S, "ll", x, y);
    return 1;
}

const char *PyFT2Font_get_descent__doc__ =
    "get_descent(self)\n"
    "--\n\n"
    "Get the descent in 26.6 subpixels of the current string set by `.set_text`.\n"
    "The rotation of the string is accounted for.  To get the descent\n"
    "in pixels, divide this value by 64.\n";

static Cystck_Object PyFT2Font_get_descent(Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    long descent;

    CALL_CPP_CYSTCK(S,"get_descent", (descent = self->x->get_descent()));

    Cystck_pushobject(S, CystckLong_FromLong(S, descent));

    return 1;
}

const char *PyFT2Font_draw_glyphs_to_bitmap__doc__ =
    "draw_glyphs_to_bitmap(self, antialiased=True)\n"
    "--\n\n"
    "Draw the glyphs that were loaded by `.set_text` to the bitmap.\n"
    "The bitmap size will be automatically set to include the glyphs.\n";

static Cystck_Object PyFT2Font_draw_glyphs_to_bitmap(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_Object h_antialiased = 0;
    bool antialiased = true;
    const char *names[] = { "antialiased", NULL };

    if (!CystckArg_parseTupleAndKeywords(S, args, kwds, "|O:draw_glyphs_to_bitmap",
                                     (const char **)names, &h_antialiased)) {
        return -1;
    }

    if (!convert_bool( Cystck_AsPyObject(S, h_antialiased), &antialiased)) {
    if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "draw_glyphs_to_bitmap");
    return -1;
    }

    CALL_CPP_CYSTCK(S,"draw_glyphs_to_bitmap", (self->x->draw_glyphs_to_bitmap(antialiased)));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;
}

const char *PyFT2Font_get_xys__doc__ =
    "get_xys(self, antialiased=True)\n"
    "--\n\n"
    "Get the xy locations of the current glyphs.\n";

static Cystck_Object PyFT2Font_get_xys(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    bool antialiased = true;
    Cystck_Object h_antialiased = 0;
    std::vector<double> xys;
    const char *names[] = { "antialiased", NULL };

    if (!CystckArg_parseTupleAndKeywords(S, args, kwds, "|O&:get_xys",
                                     (const char **)names, &h_antialiased)) {
        return -1;
    }

    if (!convert_bool( Cystck_AsPyObject(S, h_antialiased), &antialiased)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "get_xys"); 
        return -1;
    }

    CALL_CPP_CYSTCK(S,"get_xys", (self->x->get_xys(antialiased, xys)));

    Cystck_pushobject(S, convert_xys_to_array(S, xys));

    return 1;
}

const char *PyFT2Font_draw_glyph_to_bitmap__doc__ =
    "draw_glyph_to_bitmap(self, image, x, y, glyph, antialiased=True)\n"
    "--\n\n"
    "Draw a single glyph to the bitmap at pixel locations x, y\n"
    "Note it is your responsibility to set up the bitmap manually\n"
    "with ``set_bitmap_size(w, h)`` before this call is made.\n"
    "\n"
    "If you want automatic layout, use `.set_text` in combinations with\n"
    "`.draw_glyphs_to_bitmap`.  This function is instead intended for people\n"
    "who want to render individual glyphs (e.g., returned by `.load_char`)\n"
    "at precise locations.\n";

static Cystck_Object PyFT2Font_draw_glyph_to_bitmap(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    PyFT2Image *image;
    double xd, yd;
    PyGlyph *glyph;
    Cystck_Object h_image = 0, h_glyph = 0, h_antialiased = 0;
    Cystck_Object m;
    bool antialiased = true;
    const char *names[] = { "m", "image", "x", "y", "glyph", "antialiased", NULL };

    if (!CystckArg_parseTupleAndKeywords(S, args,
                                     kwds,
                                     "OOddO|O:draw_glyph_to_bitmap",
                                     (const char **)names,
                                     &m,
                                     &h_image,
                                     &xd,
                                     &yd,
                                     &h_glyph,
                                     &h_antialiased)) {
        return -1;
    }

    Cystck_Object h_PyFT2ImageType = Cystck_GetAttr_s(S, m, "FT2Image");
    if (!CystckTypeCheck(S, h_image, h_PyFT2ImageType)) {
        Cystck_DECREF(S, h_PyFT2ImageType);
        CystckErr_SetString(S, S->Cystck_TypeError, "arg must be FT2Image"); 
        return -1;
    } 
    Cystck_DECREF(S, h_PyFT2ImageType);
    Cystck_Object h_PyGlyphType = Cystck_GetAttr_s(S, m, "PyGlyph");
    if (!CystckTypeCheck(S, h_glyph, h_PyGlyphType)) {
        Cystck_DECREF(S, h_PyGlyphType);
        CystckErr_SetString(S, S->Cystck_TypeError, "mismatch type Glyph"); // TODO
        return -1;
    } 
    Cystck_DECREF(S, h_PyGlyphType);
    if(!convert_bool( Cystck_AsPyObject(S, h_antialiased), &antialiased)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "draw_glyph_to_bitmap"); // TODO
        return -1;
    }

    image = PyFT2Image_AsStruct(S, h_image);
    glyph = PyGlyph_AsStruct(S, h_glyph);

    CALL_CPP_CYSTCK(S,"draw_glyph_to_bitmap",
             self->x->draw_glyph_to_bitmap(*(image->x), xd, yd, glyph->glyphInd, antialiased));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1; 
}

const char *PyFT2Font_get_glyph_name__doc__ =
    "get_glyph_name(self, index)\n"
    "--\n\n"
    "Retrieve the ASCII name of a given glyph *index* in a face.\n"
    "\n"
    "Due to Matplotlib's internal design, for fonts that do not contain glyph\n"
    "names (per FT_FACE_FLAG_GLYPH_NAMES), this returns a made-up name which\n"
    "does *not* roundtrip through `.get_name_index`.\n";

static Cystck_Object PyFT2Font_get_glyph_name(Py_State *S, Cystck_Object args)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    unsigned int glyph_number;
    char buffer[128];
    int fallback = 1;

    if (!CystckArg_parseTuple(S, args, "I:get_glyph_name", &glyph_number)) {
        return -1;
    }
    CALL_CPP_CYSTCK(S,"get_glyph_name", (self->x->get_glyph_name(glyph_number, buffer, (bool)fallback)));
    Cystck_pushobject(S,  CystckUnicode_FromString(S,buffer));
    return 1;
}

const char *PyFT2Font_get_charmap__doc__ =
    "get_charmap(self)\n"
    "--\n\n"
    "Return a dict that maps the character codes of the selected charmap\n"
    "(Unicode by default) to their corresponding glyph indices.\n";

static Cystck_Object PyFT2Font_get_charmap(Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_Object charmap = CystckDict_New(S);
    if (Cystck_IsNULL(charmap)) {
        return -1;
    }
    Cystck_pushobject(S, charmap);
    FT_UInt index;
    FT_ULong code = FT_Get_First_Char(self->x->get_face(), &index);
    while (index != 0) {
        Cystck_Object key = 0, val = 0;
        bool error = (Cystck_IsNULL(key = CystckLong_FromLong(S, code))
                      || Cystck_IsNULL(val = CystckLong_FromLong(S, index))
                      || (Cystck_SetItem(S, charmap, key, val) == -1));
        Cystck_DECREF(S, key);
        Cystck_DECREF(S, val);
        if (error) {
            Cystck_pop(S, charmap);
            return -1;
        }
        code = FT_Get_Next_Char(self->x->get_face(), code, &index);
    }
    return 1;
}


const char *PyFT2Font_get_char_index__doc__ =
    "get_char_index(self, codepoint)\n"
    "--\n\n"
    "Return the glyph index corresponding to a character *codepoint*.\n";

static Cystck_Object PyFT2Font_get_char_index(Py_State *S, Cystck_Object args)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    FT_UInt index;
    FT_ULong ccode;
    int fallback = 1;

    if (!CystckArg_parseTuple(S, args, "k:get_char_index", &ccode)) {
        return -1;
    }

    CALL_CPP_CYSTCK(S,"get_char_index", index = self->x->get_char_index(ccode, (bool)fallback));

    Cystck_pushobject(S, CystckLong_FromLong(S, index));
    
    return 1;
}


const char *PyFT2Font_get_sfnt__doc__ =
    "get_sfnt(self)\n"
    "--\n\n"
    "Load the entire SFNT names table, as a dict whose keys are\n"
    "(platform-ID, ISO-encoding-scheme, language-code, and description)\n"
    "tuples.\n";

static Cystck_Object PyFT2Font_get_sfnt(Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_Object names;

    if (!(self->x->get_face()->face_flags & FT_FACE_FLAG_SFNT)) {
        CystckErr_SetString(S, S->Cystck_ValueError, "No SFNT name table");
        return -1;
    }

    size_t count = FT_Get_Sfnt_Name_Count(self->x->get_face());

    names = CystckDict_New(S);
    if (Cystck_IsNULL(names)) {
        return -1;
    }

    for (FT_UInt j = 0; j < count; ++j) {
        FT_SfntName sfnt;
        FT_Error error = FT_Get_Sfnt_Name(self->x->get_face(), j, &sfnt);

        if (error) {
            Cystck_DECREF(S, names);
            CystckErr_SetString(S, S->Cystck_ValueError, "Could not get SFNT name");
            return -1;
        }

        Cystck_Object key = Cystck_BuildValue(S,
            "llll", (unsigned int)sfnt.platform_id, (unsigned int)sfnt.encoding_id, (unsigned int)sfnt.language_id, (unsigned int)sfnt.name_id);
        if (Cystck_IsNULL(key)) {
            Cystck_DECREF(S, names);
            return -1;
        }
        Cystck_Object val = CystckBytes_FromStringAndSize(S, (const char *)sfnt.string, sfnt.string_len);
        if (Cystck_IsNULL(val)) {
            Cystck_pop(S, key);
            Cystck_DECREF(S, names);
            return -1;
        }

        if (Cystck_SetItem(S, names, key, val)) {
            Cystck_pop(S, key);
            Cystck_DECREF(S, val);
            Cystck_DECREF(S, names);
            return -1;
        }

        Cystck_pop(S, key);
        Cystck_DECREF(S, val);
    }
    Cystck_pushobject(S, names);

    return 1;
}

const char *PyFT2Font_get_name_index__doc__ =
    "get_name_index(self, name)\n"
    "--\n\n"
    "Return the glyph index of a given glyph *name*.\n"
    "The glyph index 0 means 'undefined character code'.\n";

static Cystck_Object PyFT2Font_get_name_index(Py_State *S, Cystck_Object args)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    char *glyphname;
    long name_index;
    if (!CystckArg_parseTuple(S, args, "s:get_name_index", &glyphname)) {
        return -1;
    }
    CALL_CPP_CYSTCK(S,"get_name_index", name_index = self->x->get_name_index(glyphname));
    
    Cystck_pushobject(S, CystckLong_FromLong(S, name_index));
    
    return 1;
}

const char *PyFT2Font_get_ps_font_info__doc__ =
    "get_ps_font_info(self)\n"
    "--\n\n"
    "Return the information in the PS Font Info structure.\n";

static Cystck_Object PyFT2Font_get_ps_font_info(Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    PS_FontInfoRec fontinfo;

    FT_Error error = FT_Get_PS_Font_Info(self->x->get_face(), &fontinfo);
    if (error) {
        CystckErr_SetString(S, S->Cystck_ValueError, "Could not get PS font info");
        return -1;
    }

    Cystck_BuildValue(S, "sssssliil",
                         fontinfo.version ? fontinfo.version : "",
                         fontinfo.notice ? fontinfo.notice : "",
                         fontinfo.full_name ? fontinfo.full_name : "",
                         fontinfo.family_name ? fontinfo.family_name : "",
                         fontinfo.weight ? fontinfo.weight : "",
                         fontinfo.italic_angle,
                         fontinfo.is_fixed_pitch,
                         fontinfo.underline_position,
                         (unsigned int)fontinfo.underline_thickness);
    return 1;
}

static Cystck_Object build_str(Py_State *S, const char *v, Cystck_ssize_t len){
    if (v) {
        return CystckBytes_FromStringAndSize(S, v, len);
    }
    return Cystck_Dup(S, S->Cystck_None);
}

const char *PyFT2Font_get_sfnt_table__doc__ =
    "get_sfnt_table(self, name)\n"
    "--\n\n"
    "Return one of the following SFNT tables: head, maxp, OS/2, hhea, "
    "vhea, post, or pclt.\n";

static Cystck_Object PyFT2Font_get_sfnt_table(Py_State *S, Cystck_Object args)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    char *tagname;
    if (!CystckArg_parseTuple(S, args, "s:get_sfnt_table", &tagname)) {
        return -1;
    }

    int tag;
    const char *tags[] = { "head", "maxp", "OS/2", "hhea", "vhea", "post", "pclt", NULL };

    for (tag = 0; tags[tag] != NULL; tag++) {
        if (strncmp(tagname, tags[tag], 5) == 0) {
            break;
        }
    }

    void *table = FT_Get_Sfnt_Table(self->x->get_face(), (FT_Sfnt_Tag)tag);
    if (!table) {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

        return 1;
    }

    switch (tag) {
    case 0: {
        char head_dict[] =
            "{s:(i,i), s:(i,i), s:l, s:l, s:i, s:i,"
            "s:(l,l), s:(l,l), s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i}";
        TT_Header *t = (TT_Header *)table;
        Cystck_BuildValue(S, head_dict,
                             "version",
                             (int)FIXED_MAJOR(t->Table_Version),
                             (int)FIXED_MINOR(t->Table_Version),
                             "fontRevision",
                             (int)FIXED_MAJOR(t->Font_Revision),
                             (int)FIXED_MINOR(t->Font_Revision),
                             "checkSumAdjustment",
                             t->CheckSum_Adjust,
                             "magicNumber",
                             t->Magic_Number,
                             "flags",
                             (int)t->Flags,
                             "unitsPerEm",
                             (int)t->Units_Per_EM,
                             "created",
                             t->Created[0],
                             t->Created[1],
                             "modified",
                             t->Modified[0],
                             t->Modified[1],
                             "xMin",
                             (int)t->xMin,
                             "yMin",
                             (int)t->yMin,
                             "xMax",
                             (int)t->xMax,
                             "yMax",
                             (int)t->yMax,
                             "macStyle",
                             (int)t->Mac_Style,
                             "lowestRecPPEM",
                             (int)t->Lowest_Rec_PPEM,
                             "fontDirectionHint",
                             (int)t->Font_Direction,
                             "indexToLocFormat",
                             (int)t->Index_To_Loc_Format,
                             "glyphDataFormat",
                             (int)t->Glyph_Data_Format);
    return 1;
    }
    
    case 1: {
        char maxp_dict[] =
            "{s:(i,i), s:i, s:i, s:i, s:i, s:i, s:i,"
            "s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i}";
        TT_MaxProfile *t = (TT_MaxProfile *)table;
        Cystck_BuildValue(S, maxp_dict,
                             "version",
                             (int)FIXED_MAJOR(t->version),
                             (int)FIXED_MINOR(t->version),
                             "numGlyphs",
                             (int)t->numGlyphs,
                             "maxPoints",
                             (int)t->maxPoints,
                             "maxContours",
                             (int)t->maxContours,
                             "maxComponentPoints",
                             (int)t->maxCompositePoints,
                             "maxComponentContours",
                             (int)t->maxCompositeContours,
                             "maxZones",
                             (int)t->maxZones,
                             "maxTwilightPoints",
                             (int)t->maxTwilightPoints,
                             "maxStorage",
                             (int)t->maxStorage,
                             "maxFunctionDefs",
                             (int)t->maxFunctionDefs,
                             "maxInstructionDefs",
                             (int)t->maxInstructionDefs,
                             "maxStackElements",
                             (int)t->maxStackElements,
                             "maxSizeOfInstructions",
                             (int)t->maxSizeOfInstructions,
                             "maxComponentElements",
                             (int)t->maxComponentElements,
                             "maxComponentDepth",
                             (int)t->maxComponentDepth);
    return 1;
    }
    
    case 2: {
        char os_2_dict[] =
            "{s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i,"
            "s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:i, s:O, s:(kkkk),"
            "s:O, s:i, s:i, s:i}";
        TT_OS2 *t = (TT_OS2 *)table;
        Cystck_BuildValue(S, os_2_dict,
                             "version",
                             (int)t->version,
                             "xAvgCharWidth",
                             (int)t->xAvgCharWidth,
                             "usWeightClass",
                             (int)t->usWeightClass,
                             "usWidthClass",
                             (int)t->usWidthClass,
                             "fsType",
                             (int)t->fsType,
                             "ySubscriptXSize",
                             (int)t->ySubscriptXSize,
                             "ySubscriptYSize",
                             (int)t->ySubscriptYSize,
                             "ySubscriptXOffset",
                             (int)t->ySubscriptXOffset,
                             "ySubscriptYOffset",
                             (int)t->ySubscriptYOffset,
                             "ySuperscriptXSize",
                             (int)t->ySuperscriptXSize,
                             "ySuperscriptYSize",
                             (int)t->ySuperscriptYSize,
                             "ySuperscriptXOffset",
                             (int)t->ySuperscriptXOffset,
                             "ySuperscriptYOffset",
                             (int)t->ySuperscriptYOffset,
                             "yStrikeoutSize",
                             (int)t->yStrikeoutSize,
                             "yStrikeoutPosition",
                             (int)t->yStrikeoutPosition,
                             "sFamilyClass",
                             (int)t->sFamilyClass,
                             "panose",
                             build_str(S, (const char *)t->panose,
                             Cystck_ssize_t(10)),
                             Cystck_ssize_t(10),
                             "ulCharRange",
                             t->ulUnicodeRange1,
                             t->ulUnicodeRange2,
                             t->ulUnicodeRange3,
                             t->ulUnicodeRange4,
                             "achVendID",
                             build_str(S, (const char *)t->achVendID,
                             Cystck_ssize_t(4)),
                             "fsSelection",
                             (int)t->fsSelection,
                             "fsFirstCharIndex",
                             (int)t->usFirstCharIndex,
                             "fsLastCharIndex",
                             (int)t->usLastCharIndex);
    return 1;
    }
    case 3: {
        char hhea_dict[] =
            "{s:(i,i), s:i, s:i, s:i, s:i, s:i, s:i, s:i,"
            "s:i, s:i, s:i, s:i, s:i}";
        TT_HoriHeader *t = (TT_HoriHeader *)table;
        Cystck_BuildValue(S, hhea_dict,
                             "version",
                             (int)FIXED_MAJOR(t->Version),
                             (int)FIXED_MINOR(t->Version),
                             "ascent",
                             (int)t->Ascender,
                             "descent",
                             (int)t->Descender,
                             "lineGap",
                             (int)t->Line_Gap,
                             "advanceWidthMax",
                             (int)t->advance_Width_Max,
                             "minLeftBearing",
                             (int)t->min_Left_Side_Bearing,
                             "minRightBearing",
                             (int)t->min_Right_Side_Bearing,
                             "xMaxExtent",
                             (int)t->xMax_Extent,
                             "caretSlopeRise",
                             (int)t->caret_Slope_Rise,
                             "caretSlopeRun",
                             (int)t->caret_Slope_Run,
                             "caretOffset",
                             (int)t->caret_Offset,
                             "metricDataFormat",
                             (int)t->metric_Data_Format,
                             "numOfLongHorMetrics",
                             (int)t->number_Of_HMetrics);
    return 1;
    }
    case 4: {
        char vhea_dict[] =
            "{s:(i,i), s:i, s:i, s:i, s:i, s:i, s:i, s:i,"
            "s:i, s:i, s:i, s:i, s:i}";
        TT_VertHeader *t = (TT_VertHeader *)table;
        Cystck_BuildValue(S, vhea_dict,
                             "version",
                             (int)FIXED_MAJOR(t->Version),
                             (int)FIXED_MINOR(t->Version),
                             "vertTypoAscender",
                             (int)t->Ascender,
                             "vertTypoDescender",
                             (int)t->Descender,
                             "vertTypoLineGap",
                             (int)t->Line_Gap,
                             "advanceHeightMax",
                             (int)t->advance_Height_Max,
                             "minTopSideBearing",
                             (int)t->min_Top_Side_Bearing,
                             "minBottomSizeBearing",
                             (int)t->min_Bottom_Side_Bearing,
                             "yMaxExtent",
                             (int)t->yMax_Extent,
                             "caretSlopeRise",
                             (int)t->caret_Slope_Rise,
                             "caretSlopeRun",
                             (int)t->caret_Slope_Run,
                             "caretOffset",
                             (int)t->caret_Offset,
                             "metricDataFormat",
                             (int)t->metric_Data_Format,
                             "numOfLongVerMetrics",
                             (int)t->number_Of_VMetrics);
    return 1;
    }
    case 5: {
        char post_dict[] = "{s:(i,i), s:(i,i), s:i, s:i, s:k, s:k, s:k, s:k, s:k}";
        TT_Postscript *t = (TT_Postscript *)table;
        Cystck_BuildValue(S, post_dict,
                             "format",
                             (int)FIXED_MAJOR(t->FormatType),
                             (int)FIXED_MINOR(t->FormatType),
                             "italicAngle",
                             (int)FIXED_MAJOR(t->italicAngle),
                             (int)FIXED_MINOR(t->italicAngle),
                             "underlinePosition",
                             (int)t->underlinePosition,
                             "underlineThickness",
                             (int)t->underlineThickness,
                             "isFixedPitch",
                             (int)t->isFixedPitch,
                             "minMemType42",
                             (int)t->minMemType42,
                             "maxMemType42",
                             (int)t->maxMemType42,
                             "minMemType1",
                             (int)t->minMemType1,
                             "maxMemType1",
                             (int)t->maxMemType1);
    return 1;
    }
    case 6: {
        char pclt_dict[] =
            "{s:(i,i), s:k, s:i, s:i, s:i, s:i, s:i, s:i, s:O, s:O, s:i, "
            "s:i, s:i}";
        TT_PCLT *t = (TT_PCLT *)table;
        Cystck_BuildValue(S, pclt_dict,
                             "version",
                             (int)FIXED_MAJOR(t->Version),
                             (int)FIXED_MINOR(t->Version),
                             "fontNumber",
                             (int)t->FontNumber,
                             "pitch",
                             (int)t->Pitch,
                             "xHeight",
                             (int)t->xHeight,
                             "style",
                             (int)t->Style,
                             "typeFamily",
                             (int)t->TypeFamily,
                             "capHeight",
                             (int)t->CapHeight,
                             "symbolSet",
                             (int)t->SymbolSet,
                             "typeFace",
                             build_str(S, (const char *)t->TypeFace,
                             Cystck_ssize_t(16)),
                             "characterComplement",
                             build_str(S, (const char *)t->CharacterComplement,
                             Cystck_ssize_t(8)),
                             "strokeWeight",
                             (int)t->StrokeWeight,
                             "widthType",
                             (int)t->WidthType,
                             "serifStyle",
                             (int)t->SerifStyle);
    return 1;
    }
    default:
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
        return 1;
    }
}
const char *PyFT2Font_get_path__doc__ =
    "get_path(self)\n"
    "--\n\n"
    "Get the path data from the currently loaded glyph as a tuple of vertices, "
    "codes.\n";

static Cystck_Object PyFT2Font_get_path( Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);

    CALL_CPP_CYSTCK(S,"get_path", Cystck_pushobject(S, self->x->get_path(S)); return 1);

}

const char *PyFT2Font_get_image__doc__ =
    "get_image(self)\n"
    "--\n\n"
    "Return the underlying image buffer for this font object.\n";

static Cystck_Object PyFT2Font_get_image( Py_State *S)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    FT2Image &im = self->x->get_image();
    npy_intp dims[] = {(npy_intp)im.get_height(), (npy_intp)im.get_width() };
    
    Cystck_pushobject(S, Cystck_FromPyObject(S, PyArray_SimpleNewFromData(2, dims, NPY_UBYTE, im.get_buffer())));
    
    return 1;
}

static  Cystck_Object PyFT2Font_postscript_name(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    const char *ps_name = FT_Get_Postscript_Name(self->x->get_face());
    if (ps_name == NULL) {
        ps_name = "UNAVAILABLE";
    }

    Cystck_pushobject(S, CystckUnicode_FromString(S, ps_name));

    return 1;
}

static Cystck_Object PyFT2Font_num_faces(Py_State *S, void *closure)
{
    
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S,self->x->get_face()->num_faces));
    return 1;
}

static Cystck_Object PyFT2Font_family_name(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    const char *name = self->x->get_face()->family_name;
    if (name == NULL) {
        name = "UNAVAILABLE";
    }

    Cystck_pushobject(S, CystckUnicode_FromString(S, name));
    
    return 1;
}

static Cystck_Object PyFT2Font_style_name(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    const char *name = self->x->get_face()->style_name;
    if (name == NULL) {
        name = "UNAVAILABLE";
    }
    Cystck_pushobject(S, CystckUnicode_FromString(S, name));
    
    return 1;
}

static Cystck_Object PyFT2Font_face_flags(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->face_flags));
    return 1;
}

static Cystck_Object PyFT2Font_style_flags(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->style_flags));
    return 1;

}

static Cystck_Object PyFT2Font_num_glyphs(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->num_glyphs));
    return 1;
}

static  Cystck_Object PyFT2Font_num_fixed_sizes(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->num_fixed_sizes));
    return 1;
}

static Cystck_Object PyFT2Font_num_charmaps(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->num_charmaps));
    
    return 1;
}

static Cystck_Object PyFT2Font_scalable(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    if (FT_IS_SCALABLE(self->x->get_face())) {
        
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
        return 1;
    }
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_False));
    return 1;
}

static Cystck_Object PyFT2Font_units_per_EM(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->units_per_EM));
    return 1;
}

static Cystck_Object PyFT2Font_get_bbox(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    FT_BBox *bbox = &(self->x->get_face()->bbox);

    Cystck_BuildValue(S, "llll",
                         bbox->xMin, bbox->yMin, bbox->xMax, bbox->yMax);
    return 1;
}

static Cystck_Object PyFT2Font_ascender(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->ascender));
    
    return 1;
}

static Cystck_Object PyFT2Font_descender(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->descender));
    return 1;
}

static Cystck_Object PyFT2Font_height(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->height));
    return 1;
}

static Cystck_Object PyFT2Font_max_advance_width(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->max_advance_width));
    return 1;
}

static Cystck_Object PyFT2Font_max_advance_height(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->max_advance_height));
    return 1;
}

static Cystck_Object PyFT2Font_underline_position(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->underline_position));
    return 1;
}

static Cystck_Object PyFT2Font_underline_thickness(Py_State  *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    Cystck_pushobject(S, CystckLong_FromLong(S, self->x->get_face()->underline_thickness));
    return 1;
}

static Cystck_Object PyFT2Font_fname(Py_State *S, void *closure)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, S->self);
    if (self->stream.close) {  // Called passed a filename to the constructor.
        Cystck_pushobject(S, Cystck_GetAttr_s(S, Cystck_FromPyObject(S, self->py_file), "name"));
        return 1;
    } else {
        Cystck_pushobject(S, Cystck_Dup(S, Cystck_FromPyObject(S, self->py_file)));
        return 1;
    }
}

static int PyFT2Font_get_buffer(Py_State *S, Cystck_Object c_self, Cystck_buffer *buf, int flags)
{
    PyFT2Font *self = PyFT2Font_AsStruct(S, c_self);
    FT2Image &im = self->x->get_image();

    buf->obj = Cystck_Dup(S, c_self);
    buf->buf = im.get_buffer();
    buf->len = im.get_width() * im.get_height();
    buf->readonly = 0;
    buf->format = (char *)"B";
    buf->ndim = 2;
    self->shape[0] = im.get_height();
    self->shape[1] = im.get_width();
    buf->shape = self->shape;
    self->strides[0] = im.get_width();
    self->strides[1] = 1;
    buf->strides = self->strides;
    buf->suboffsets = NULL;
    buf->itemsize = 1;
    buf->internal = NULL;

    return 1;
}

CystckDef_GET(PyFT2Font_postscript_name_get, "postscript_name", PyFT2Font_postscript_name, NULL,NULL)
CystckDef_GET(PyFT2Font_num_faces_get, "num_faces", PyFT2Font_num_faces, NULL, NULL)
CystckDef_GET(PyFT2Font_family_name_get, "family_name", PyFT2Font_family_name, NULL, NULL)
CystckDef_GET(PyFT2Font_style_name_get, "style_name", PyFT2Font_style_name, NULL, NULL)
CystckDef_GET(PyFT2Font_face_flags_get, "face_flags", PyFT2Font_face_flags,NULL, NULL)
CystckDef_GET(PyFT2Font_style_flags_get, "style_flags", PyFT2Font_style_flags, NULL, NULL)
CystckDef_GET(PyFT2Font_num_glyphs_get, "num_glyphs", PyFT2Font_num_glyphs, NULL, NULL)
CystckDef_GET(PyFT2Font_num_fixed_sizes_get, "num_fixed_sizes", PyFT2Font_num_fixed_sizes, NULL, NULL)
CystckDef_GET(PyFT2Font_num_charmaps_get, "num_charmaps", PyFT2Font_num_charmaps, NULL, NULL)
CystckDef_GET(PyFT2Font_scalable_get, "scalable", PyFT2Font_scalable, NULL, NULL)
CystckDef_GET(PyFT2Font_units_per_EM_get, "units_per_EM", PyFT2Font_units_per_EM, NULL, NULL)
CystckDef_GET(PyFT2Font_get_bbox_get, "bbox", PyFT2Font_get_bbox, NULL, NULL)
CystckDef_GET(PyFT2Font_ascender_get, "ascender", PyFT2Font_ascender, NULL, NULL)
CystckDef_GET(PyFT2Font_descender_get, "descender", PyFT2Font_descender, NULL, NULL)
CystckDef_GET(PyFT2Font_height_get, "height", PyFT2Font_height, NULL, NULL)
CystckDef_GET(PyFT2Font_max_advance_width_get, "max_advance_width", PyFT2Font_max_advance_width, NULL, NULL)
CystckDef_GET(PyFT2Font_max_advance_height_get, "max_advance_height", PyFT2Font_max_advance_height, NULL, NULL)
CystckDef_GET(PyFT2Font_underline_position_get, "underline_position", PyFT2Font_underline_position, NULL, NULL)
CystckDef_GET(PyFT2Font_underline_thickness_get, "underline_thickness", PyFT2Font_underline_thickness, NULL, NULL)
CystckDef_GET(PyFT2Font_fname_get, "fname", PyFT2Font_fname, NULL, NULL)


CystckDef_METH(PyFT2Font_clear_meth, "clear", PyFT2Font_clear, Cystck_METH_NOARGS, PyFT2Font_clear__doc__)
CystckDef_METH(PyFT2Font_set_size_meth, "set_size", PyFT2Font_set_size, Cystck_METH_VARARGS, PyFT2Font_set_size__doc__)
CystckDef_METH(PyFT2Font_set_charmap_meth, "set_charmap", PyFT2Font_set_charmap, Cystck_METH_VARARGS, PyFT2Font_set_charmap__doc__)
CystckDef_METH(PyFT2Font_select_charmap_meth, "select_charmap", PyFT2Font_select_charmap, Cystck_METH_VARARGS, PyFT2Font_select_charmap__doc__)
CystckDef_METH(PyFT2Font_get_kerning_meth, "get_kerning", PyFT2Font_get_kerning, Cystck_METH_VARARGS, PyFT2Font_get_kerning__doc__)
CystckDef_METH(PyFT2Font_set_text_meth, "set_text", PyFT2Font_set_text, Cystck_METH_KEYWORDS, PyFT2Font_set_text__doc__)
CystckDef_METH(PyFT2Font_get_xys_meth, "get_xys", PyFT2Font_get_xys, Cystck_METH_KEYWORDS, PyFT2Font_get_xys__doc__)
CystckDef_METH(PyFT2Font_get_num_glyphs_meth, "get_num_glyphs", PyFT2Font_get_num_glyphs, Cystck_METH_NOARGS, PyFT2Font_get_num_glyphs__doc__)
CystckDef_METH(PyFT2Font_load_char_meth, "load_char", PyFT2Font_load_char, Cystck_METH_KEYWORDS, PyFT2Font_load_char__doc__)
CystckDef_METH(PyFT2Font_load_glyph_meth, "load_glyph", PyFT2Font_load_glyph, Cystck_METH_KEYWORDS, PyFT2Font_load_glyph__doc__)
CystckDef_METH(PyFT2Font_get_width_height_meth, "get_width_height", PyFT2Font_get_width_height, Cystck_METH_NOARGS, PyFT2Font_get_width_height__doc__)
CystckDef_METH(PyFT2Font_get_bitmap_offset_meth, "get_bitmap_offset", PyFT2Font_get_bitmap_offset, Cystck_METH_NOARGS, PyFT2Font_get_bitmap_offset__doc__)
CystckDef_METH(PyFT2Font_get_descent_meth, "get_descent", PyFT2Font_get_descent, Cystck_METH_NOARGS, PyFT2Font_get_descent__doc__)
CystckDef_METH(PyFT2Font_draw_glyphs_to_bitmap_meth, "draw_glyphs_to_bitmap", PyFT2Font_draw_glyphs_to_bitmap, Cystck_METH_KEYWORDS, PyFT2Font_draw_glyphs_to_bitmap__doc__)
CystckDef_METH(PyFT2Font_draw_glyph_to_bitmap_meth, "draw_glyph_to_bitmap", PyFT2Font_draw_glyph_to_bitmap, Cystck_METH_KEYWORDS, PyFT2Font_draw_glyph_to_bitmap__doc__)
CystckDef_METH(PyFT2Font_get_glyph_name_meth, "get_glyph_name", PyFT2Font_get_glyph_name, Cystck_METH_VARARGS, PyFT2Font_get_glyph_name__doc__)
CystckDef_METH(PyFT2Font_get_charmap_meth, "get_charmap", PyFT2Font_get_charmap, Cystck_METH_NOARGS, PyFT2Font_get_charmap__doc__)
CystckDef_METH(PyFT2Font_get_char_index_meth, "get_char_index", PyFT2Font_get_char_index, Cystck_METH_VARARGS, PyFT2Font_get_char_index__doc__)
CystckDef_METH(PyFT2Font_get_sfnt_meth, "get_sfnt", PyFT2Font_get_sfnt, Cystck_METH_NOARGS, PyFT2Font_get_sfnt__doc__)
CystckDef_METH(PyFT2Font_get_name_index_meth, "get_name_index", PyFT2Font_get_name_index, Cystck_METH_VARARGS, PyFT2Font_get_name_index__doc__)
CystckDef_METH(PyFT2Font_get_ps_font_info_meth, "get_ps_font_info", PyFT2Font_get_ps_font_info, Cystck_METH_NOARGS, PyFT2Font_get_ps_font_info__doc__)
CystckDef_METH(PyFT2Font_get_path_meth, "get_path", PyFT2Font_get_path, Cystck_METH_NOARGS, PyFT2Font_get_path__doc__)
CystckDef_METH(PyFT2Font_get_image_meth, "get_image", PyFT2Font_get_image, Cystck_METH_NOARGS, PyFT2Font_get_image__doc__)
CystckDef_METH(PyFT2Font_get_sfnt_table_meth, "get_sfnt_table", PyFT2Font_get_sfnt_table, Cystck_METH_VARARGS, PyFT2Font_get_sfnt_table__doc__)
static PyMethodDef methods1[] = {
    {"_get_fontmap", (PyCFunction)PyFT2Font_get_fontmap, METH_VARARGS|METH_KEYWORDS, PyFT2Font_get_fontmap__doc__},
    {NULL}
};

CystckDef_SLOT(PyFT2Font_new_def, PyFT2Font_new, Cystck_tp_new)
CystckDef_SLOT(PyFT2Font_get_buffer_def, PyFT2Font_get_buffer, Cystck_bf_getbuffer)
CystckDef_SLOT(PyFT2Font_init_def, PyFT2Font_init, Cystck_tp_init)
CystckDef_SLOT(PyFT2Font_dealloc_def, PyFT2Font_dealloc, Cystck_tp_dealloc)
CystckDef *PyFT2Font_defines[] = {
    // slots
    &PyFT2Font_new_def,
    &PyFT2Font_get_buffer_def,
    &PyFT2Font_init_def,
    &PyFT2Font_dealloc_def,
    // getset
    &PyFT2Font_postscript_name_get,
    &PyFT2Font_num_faces_get,
    &PyFT2Font_family_name_get,
    &PyFT2Font_style_name_get,
    &PyFT2Font_face_flags_get,
    &PyFT2Font_style_flags_get,
    &PyFT2Font_num_glyphs_get,
    &PyFT2Font_num_fixed_sizes_get,
    &PyFT2Font_num_charmaps_get,
    &PyFT2Font_scalable_get,
    &PyFT2Font_units_per_EM_get,
    &PyFT2Font_get_bbox_get,
    &PyFT2Font_ascender_get,
    &PyFT2Font_descender_get,
    &PyFT2Font_height_get,
    &PyFT2Font_max_advance_width_get,
    &PyFT2Font_max_advance_height_get,
    &PyFT2Font_underline_position_get,
    &PyFT2Font_underline_thickness_get,
    &PyFT2Font_fname_get,
    // methods
    &PyFT2Font_clear_meth,
    &PyFT2Font_set_size_meth,
    &PyFT2Font_set_charmap_meth,
    &PyFT2Font_select_charmap_meth,
    &PyFT2Font_get_kerning_meth,
    &PyFT2Font_set_text_meth,
    &PyFT2Font_get_xys_meth,
    &PyFT2Font_get_num_glyphs_meth,
    &PyFT2Font_load_char_meth,
    &PyFT2Font_load_glyph_meth,
    &PyFT2Font_get_width_height_meth,
    &PyFT2Font_get_bitmap_offset_meth,
    &PyFT2Font_get_descent_meth,
    &PyFT2Font_draw_glyphs_to_bitmap_meth,
    &PyFT2Font_draw_glyph_to_bitmap_meth,
    &PyFT2Font_get_glyph_name_meth,
    &PyFT2Font_get_charmap_meth,
    &PyFT2Font_get_char_index_meth,
    &PyFT2Font_get_sfnt_meth,
    &PyFT2Font_get_name_index_meth,
    &PyFT2Font_get_ps_font_info_meth,
    &PyFT2Font_get_path_meth,
    &PyFT2Font_get_image_meth,
    &PyFT2Font_get_sfnt_table_meth,
    NULL
};
static PyType_Slot FT2Font_slots[] = {
    {Py_tp_doc, (void *)PyFT2Font_init__doc__},
    {Py_tp_methods, methods1},
    {0, 0}
};
Cystck_Type_Spec PyFT2Font_type_spec = {
    .name = "matplotlib.ft2font.FT2Font",
    .basicsize = sizeof(PyFT2Font),
    .flags = Cystck_TPFLAGS_DEFAULT | Cystck_TPFLAGS_BASETYPE,
    .legacy_slots = FT2Font_slots,
    .m_methods = PyFT2Font_defines
};


static CyModuleDef moduledef = {
    .m_name = "ft2font",
    .m_doc = NULL,
    .m_size = 0,
};

int add_dict_int(Py_State *S, Cystck_Object dict, const char *key, long val)
{
    Cystck_Object valobj = CystckLong_FromLong(S, val);
    if (Cystck_IsNULL(valobj)) {
        return 1;
    }
    Cystck_pushobject(S, valobj);

    if (Cystck_SetAttr_s(S, dict, key, valobj)) {
        Cystck_pop(S, valobj);
        return 1;
    }

    Cystck_pop(S, valobj);
    return 0;
}

int add_dict_string(Py_State *S, Cystck_Object dict, const char *key, const char *value)
{
    Cystck_Object valobj = CystckUnicode_FromString(S, value);
    if (Cystck_IsNULL(valobj)) {
        return 1;
    }
    Cystck_pushobject(S, valobj);

    if (Cystck_SetAttr_s(S, dict, key, valobj)) {
        Cystck_pop(S, valobj);
        return 1;
    }

    Cystck_pop(S, valobj);
    return 0;
}

// Logic is from NumPy's import_array()
static int npy_import_array(Py_State *S) {
    if (_import_array() < 0) {
        CystckErr_SetString(S, S->Cystck_ImportError, "numpy.core.multiarray failed to import"); 
        return 0; 
    }
    return 1;
}

#ifdef __cplusplus
extern "C" {
#endif
#pragma GCC visibility push(default)
CyMODINIT_FUNC(ft2font) 
CyInit_ft2font(Py_State *S)
{

    if (!npy_import_array(S)) {
        return 0;
    }

    Cystck_Object m = CystckModule_Create(S, &moduledef);
    if (Cystck_IsNULL(m)) {
        return 0;
    }

    if (!CystckHelpers_AddType(S, m, "FT2Image", &PyFT2Image_type_spec)) {
        Cystck_pop(S, m);
        return 0;
    }

    if (!CystckHelpers_AddType(S, m, "PyGlyph", &PyGlyph_type_spec)) {
        Cystck_pop(S, m);
        return 0;
    }

    if (!CystckHelpers_AddType(S, m, "FT2Font", &PyFT2Font_type_spec)) {
        Cystck_pop(S, m);
        return 0;
    }

    if (add_dict_int(S, m, "SCALABLE", FT_FACE_FLAG_SCALABLE) ||
        add_dict_int(S, m, "FIXED_SIZES", FT_FACE_FLAG_FIXED_SIZES) ||
        add_dict_int(S, m, "FIXED_WIDTH", FT_FACE_FLAG_FIXED_WIDTH) ||
        add_dict_int(S, m, "SFNT", FT_FACE_FLAG_SFNT) ||
        add_dict_int(S, m, "HORIZONTAL", FT_FACE_FLAG_HORIZONTAL) ||
        add_dict_int(S, m, "VERTICAL", FT_FACE_FLAG_VERTICAL) ||
        add_dict_int(S, m, "KERNING", FT_FACE_FLAG_KERNING) ||
        add_dict_int(S, m, "FAST_GLYPHS", FT_FACE_FLAG_FAST_GLYPHS) ||
        add_dict_int(S, m, "MULTIPLE_MASTERS", FT_FACE_FLAG_MULTIPLE_MASTERS) ||
        add_dict_int(S, m, "GLYPH_NAMES", FT_FACE_FLAG_GLYPH_NAMES) ||
        add_dict_int(S, m, "EXTERNAL_STREAM", FT_FACE_FLAG_EXTERNAL_STREAM) ||
        add_dict_int(S, m, "ITALIC", FT_STYLE_FLAG_ITALIC) ||
        add_dict_int(S, m, "BOLD", FT_STYLE_FLAG_BOLD) ||
        add_dict_int(S, m, "KERNING_DEFAULT", FT_KERNING_DEFAULT) ||
        add_dict_int(S, m, "KERNING_UNFITTED", FT_KERNING_UNFITTED) ||
        add_dict_int(S, m, "KERNING_UNSCALED", FT_KERNING_UNSCALED) ||
        add_dict_int(S, m, "LOAD_DEFAULT", FT_LOAD_DEFAULT) ||
        add_dict_int(S, m, "LOAD_NO_SCALE", FT_LOAD_NO_SCALE) ||
        add_dict_int(S, m, "LOAD_NO_HINTING", FT_LOAD_NO_HINTING) ||
        add_dict_int(S, m, "LOAD_RENDER", FT_LOAD_RENDER) ||
        add_dict_int(S, m, "LOAD_NO_BITMAP", FT_LOAD_NO_BITMAP) ||
        add_dict_int(S, m, "LOAD_VERTICAL_LAYOUT", FT_LOAD_VERTICAL_LAYOUT) ||
        add_dict_int(S, m, "LOAD_FORCE_AUTOHINT", FT_LOAD_FORCE_AUTOHINT) ||
        add_dict_int(S, m, "LOAD_CROP_BITMAP", FT_LOAD_CROP_BITMAP) ||
        add_dict_int(S, m, "LOAD_PEDANTIC", FT_LOAD_PEDANTIC) ||
        add_dict_int(S, m, "LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH", FT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH) ||
        add_dict_int(S, m, "LOAD_NO_RECURSE", FT_LOAD_NO_RECURSE) ||
        add_dict_int(S, m, "LOAD_IGNORE_TRANSFORM", FT_LOAD_IGNORE_TRANSFORM) ||
        add_dict_int(S, m, "LOAD_MONOCHROME", FT_LOAD_MONOCHROME) ||
        add_dict_int(S, m, "LOAD_LINEAR_DESIGN", FT_LOAD_LINEAR_DESIGN) ||
        add_dict_int(S, m, "LOAD_NO_AUTOHINT", (unsigned long)FT_LOAD_NO_AUTOHINT) ||
        add_dict_int(S, m, "LOAD_TARGET_NORMAL", (unsigned long)FT_LOAD_TARGET_NORMAL) ||
        add_dict_int(S, m, "LOAD_TARGET_LIGHT", (unsigned long)FT_LOAD_TARGET_LIGHT) ||
        add_dict_int(S, m, "LOAD_TARGET_MONO", (unsigned long)FT_LOAD_TARGET_MONO) ||
        add_dict_int(S, m, "LOAD_TARGET_LCD", (unsigned long)FT_LOAD_TARGET_LCD) ||
        add_dict_int(S, m, "LOAD_TARGET_LCD_V", (unsigned long)FT_LOAD_TARGET_LCD_V)) {
        Cystck_pop(S, m);
        return 0;
    }

    // initialize library
    int error = FT_Init_FreeType(&_ft2Library);

    if (error) {
        CystckErr_SetString(S, S->Cystck_RuntimeError, "Could not initialize the freetype2 library");
        Cystck_pop(S, m);
        return 0;
    }

    {
        FT_Int major, minor, patch;
        char version_string[64];

        FT_Library_Version(_ft2Library, &major, &minor, &patch);
        sprintf(version_string, "%d.%d.%d", major, minor, patch);
        if (add_dict_string(S, m, "__freetype_version__", version_string)) {
            return 0;
        }
    }

    if (add_dict_string(S, m, "__freetype_build_type__", STRINGIFY(FREETYPE_BUILD_TYPE))) {
        Cystck_pop(S, m);
        return 0;
    }

    return m;
}

#pragma GCC visibility pop
#ifdef __cplusplus
}
#endif