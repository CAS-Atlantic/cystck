#define NO_IMPORT_ARRAY
#define PY_SSIZE_T_CLEAN
#include "py_converters.h"
#include "numpy_cpp.h"

#include "agg_basics.h"
#include "agg_color_rgba.h"
#include "agg_math_stroke.h"

extern "C" {

#ifdef CYSTCK

static int convert_string_enum_cystck(Py_State *S, Cystck_Object obj, const char *name, const char **names, int *values, int *result)
{
    Cystck_Object bytesobj;
    char *str;

    if (Cystck_IsNULL(obj) || Cystck_Is(S, obj, S->Cystck_None)) {
        return 1;
    }

    if (CystckUnicode_Check(S, obj)) {
        bytesobj = CystckUnicode_AsASCIIString(S, obj);
        if (Cystck_IsNULL(bytesobj)) {
            return 0;
        }
        Cystck_pushobject(S, bytesobj);
    } else if (CystckBytes_Check(S, obj)) {
        bytesobj = Cystck_Dup(S, obj);
        Cystck_pushobject(S, bytesobj);
    } else {
        CystckErr_SetString(S, S->Cystck_TypeError, "must be str or bytes");
        return 0;
    }

    str = CystckBytes_AsString(S, bytesobj);
    if (str == NULL) {
        Cystck_pop(S, bytesobj);
        return 0;
    }

    for ( ; *names != NULL; names++, values++) {
        if (strncmp(str, *names, 64) == 0) {
            *result = *values;
            Cystck_pop(S, bytesobj);
            return 1;
        }
    }

    // PyErr_Format(PyExc_ValueError, "invalid %s value", name);
    CystckErr_SetString(S, S->Cystck_ValueError, "invalid value");
    Cystck_pop(S, bytesobj);
    return 0;
}

int convert_from_method_cystck(Py_State *S, Cystck_Object obj, const char *name, converter_cystck func, void *p)
{
    if (!Cystck_HasAttr_s(S, obj, name)) {
        return 1;
    }
    Cystck_Object callable = Cystck_GetAttr_s(S, obj, name);
    Cystck_pushobject(S, callable);
    Cystck_Object value = Cystck_CallTupleDict(S, callable, 0, 0);
    Cystck_pop(S, callable);
    // value = PyObject_CallMethod(obj, name, NULL);
    if (Cystck_IsNULL(value)) {
        return 0;
    }
    Cystck_pushobject(S, value);
    if (!func(S, value, p)) {
        Cystck_pop(S, value);
        return 0;
    }

    Cystck_pop(S, value);
    return 1;
}

int convert_from_attr_cystck(Py_State *S, Cystck_Object obj, const char *name, converter_cystck func, void *p)
{
    Cystck_Object value = Cystck_GetAttr_s(S, obj, name);
    if (Cystck_IsNULL(value)) {
        if (!Cystck_HasAttr_s(S, obj, name)) {
            CystckErr_Clear(S);
            return 1;
        }
        return 0;
    }
    Cystck_pushobject(S, value);

    if (!func(S, value, p)) {
        Cystck_pop(S, value);
        return 0;
    }

    Cystck_pop(S, value);
    return 1;
}

int convert_double_cystck(Py_State *S, Cystck_Object obj, void *p)
{
    double *val = (double *)p;

    *val = CystckFloat_AsDouble(S, obj);
    if (Cystck_Err_Occurred(S)) {
        return 0;
    }

    return 1;
}

int convert_bool_cystck(Py_State *S, Cystck_Object obj, void *p)
{
    bool *val = (bool *)p;
    switch (Cystck_IsTrue(S, obj)) {
        case 0: *val = false; break;
        case 1: *val = true; break;
        default: return 0;  // errored.
    }
    return 1;
}

int convert_cap_cystck(Py_State *S, Cystck_Object capobj, void *capp)
{
    const char *names[] = {"butt", "round", "projecting", NULL};
    int values[] = {agg::butt_cap, agg::round_cap, agg::square_cap};
    int result = agg::butt_cap;

    if (!convert_string_enum_cystck(S, capobj, "capstyle", names, values, &result)) {
        return 0;
    }

    *(agg::line_cap_e *)capp = (agg::line_cap_e)result;
    return 1;
}

int convert_join_cystck(Py_State *S, Cystck_Object joinobj, void *joinp)
{
    const char *names[] = {"miter", "round", "bevel", NULL};
    int values[] = {agg::miter_join_revert, agg::round_join, agg::bevel_join};
    int result = agg::miter_join_revert;

    if (!convert_string_enum_cystck(S, joinobj, "joinstyle", names, values, &result)) {
        return 0;
    }

    *(agg::line_join_e *)joinp = (agg::line_join_e)result;
    return 1;
}

int convert_rect_cystck(Py_State *S, Cystck_Object rectobj, void *rectp)
{
    agg::rect_d *rect = (agg::rect_d *)rectp;

    if (Cystck_IsNULL(rectobj) || Cystck_Is(S, rectobj, S->Cystck_None)) {
        rect->x1 = 0.0;
        rect->y1 = 0.0;
        rect->x2 = 0.0;
        rect->y2 = 0.0;
    } else {
        PyArrayObject *rect_arr = (PyArrayObject *)PyArray_ContiguousFromAny(
                Cystck_AsPyObject(S, rectobj), NPY_DOUBLE, 1, 2);
        if (rect_arr == NULL) {
            return 0;
        }

        if (PyArray_NDIM(rect_arr) == 2) {
            if (PyArray_DIM(rect_arr, 0) != 2 ||
                PyArray_DIM(rect_arr, 1) != 2) {
                CystckErr_SetString(S, S->Cystck_ValueError, "Invalid bounding box");
                Py_DECREF(rect_arr);
                return 0;
            }

        } else {  // PyArray_NDIM(rect_arr) == 1
            if (PyArray_DIM(rect_arr, 0) != 4) {
                CystckErr_SetString(S, S->Cystck_ValueError, "Invalid bounding box");
                Py_DECREF(rect_arr);
                return 0;
            }
        }

        double *buff = (double *)PyArray_DATA(rect_arr);
        rect->x1 = buff[0];
        rect->y1 = buff[1];
        rect->x2 = buff[2];
        rect->y2 = buff[3];

        Py_DECREF(rect_arr);
    }
    return 1;
}

int convert_rgba_cystck(Py_State *S, Cystck_Object rgbaobj, void *rgbap)
{
    agg::rgba *rgba = (agg::rgba *)rgbap;

    if (Cystck_IsNULL(rgbaobj) || Cystck_Is(S, rgbaobj, S->Cystck_None)) {
        rgba->r = 0.0;
        rgba->g = 0.0;
        rgba->b = 0.0;
        rgba->a = 0.0;
    } else {
        rgba->a = 1.0;
        // Cystck_ssize_t nargs = Cystck_Length(S, rgbaobj);
        // Cystck_Object args[nargs];
        // for (Cystck_ssize_t i = 0; i < nargs; i++) {
        //     args[i] = Cystck_GetItem_i(S, rgbaobj, i);
        // }
        int res = CystckArg_parseTuple(S, rgbaobj,
                 "ddd|d:rgba", &(rgba->r), &(rgba->g), &(rgba->b), &(rgba->a));                
        if (!res) {
            return 0;
        }
    }

    return 1;
}

int convert_dashes_cystck(Py_State *S, Cystck_Object dashobj, void *dashesp)
{
    if (Cystck_IsNULL(dashobj) || Cystck_Is(S, dashobj, S->Cystck_None)) {
        return 1;
    }

    Dashes *dashes = (Dashes *)dashesp;

    double dash_offset = 0.0;
    Cystck_Object dashes_seq = 0;

    int ret;
    Arg_ParseTuple(ret, S, dashobj, "dO:dashes", &dash_offset, &dashes_seq)
    dashes_seq = Cystck_Dup(S, dashes_seq); // copy before closing tuple items
    Cystck_pushobject(S, dashes_seq);
    //Arg_ParseTupleClose(S, dashobj);

    if (!ret) {
        return 0;
    }

    if (Cystck_Is(S, dashes_seq, S->Cystck_None)) {
        Cystck_pop(S, dashes_seq);
        return 1;
    }

    if (!CystckList_Check(S, dashes_seq) && !CystckTuple_Check(S, dashes_seq)) {
        CystckErr_SetString(S, S->Cystck_TypeError, "Invalid dashes sequence");
        Cystck_pop(S, dashes_seq);
        return 0;
    }

    Cystck_ssize_t nentries = Cystck_Length(S, dashes_seq);
    // If the dashpattern has odd length, iterate through it twice (in
    // accordance with the pdf/ps/svg specs).
    Cystck_ssize_t dash_pattern_length = (nentries % 2) ? 2 * nentries : nentries;

    for (Cystck_ssize_t i = 0; i < dash_pattern_length; ++i) {
        double length;
        double skip;

        Cystck_Object item = Cystck_GetItem_i(S, dashes_seq, i % nentries);
        if (Cystck_IsNULL(item)) {
            Cystck_pop(S, dashes_seq);
            return 0;
        }
        Cystck_pushobject(S, item);
        length = CystckFloat_AsDouble(S, item);
        if (Cystck_Err_Occurred(S)) {
            Cystck_pop(S, item);
            Cystck_pop(S, dashes_seq);
            return 0;
        }
        Cystck_pop(S, item);

        ++i;

        item = Cystck_GetItem_i(S, dashes_seq, i % nentries);
        if (Cystck_IsNULL(item)) {
            Cystck_pop(S, dashes_seq);
            return 0;
        }
        Cystck_pushobject(S, item);
        skip = CystckFloat_AsDouble(S, item);
        if (Cystck_Err_Occurred(S)) {
            Cystck_pop(S, item);
            Cystck_pop(S, dashes_seq);
            return 0;
        }
        Cystck_pop(S, item);

        dashes->add_dash_pair(length, skip);
    }
    Cystck_pop(S, dashes_seq);

    dashes->set_dash_offset(dash_offset);

    return 1;
}

int convert_dashes_vector_cystck(Py_State *S, Cystck_Object obj, void *dashesp)
{
    DashesVector *dashes = (DashesVector *)dashesp;

    if (!CystckList_Check(S, obj) && !CystckTuple_Check(S, obj)) {
        return 0;
    }

    Cystck_ssize_t n = Cystck_Length(S, obj);

    for (Cystck_ssize_t i = 0; i < n; ++i) {
        Dashes subdashes;

        Cystck_Object item = Cystck_GetItem_i(S, obj, i);
        if (Cystck_IsNULL(item)) {
            return 0;
        }
        Cystck_pushobject(S, item);
        if (!convert_dashes_cystck(S, item, &subdashes)) {
            Cystck_pop(S, item);
            return 0;
        }
        Cystck_pop(S, item);

        dashes->push_back(subdashes);
    }

    return 1;
}

int convert_trans_affine_cystck(Py_State *S, Cystck_Object obj, void *transp)
{
    agg::trans_affine *trans = (agg::trans_affine *)transp;

    /** If None assume identity transform. */
    if (Cystck_IsNULL(obj) || Cystck_Is(S, obj, S->Cystck_None)) {
        return 1;
    }

    PyArrayObject *array = (PyArrayObject *)PyArray_ContiguousFromAny(Cystck_AsPyObject(S, obj), NPY_DOUBLE, 2, 2);
    if (array == NULL) {
        return 0;
    }

    if (PyArray_DIM(array, 0) == 3 && PyArray_DIM(array, 1) == 3) {
        double *buffer = (double *)PyArray_DATA(array);
        trans->sx = buffer[0];
        trans->shx = buffer[1];
        trans->tx = buffer[2];

        trans->shy = buffer[3];
        trans->sy = buffer[4];
        trans->ty = buffer[5];

        Py_DECREF(array);
        return 1;
    }

    Py_DECREF(array);
    CystckErr_SetString(S, S->Cystck_ValueError, "Invalid affine transformation matrix");
    return 0;
}


int convert_path_cystck(Py_State *S, Cystck_Object obj, void *pathp)
{
    py::PathIterator *path = (py::PathIterator *)pathp;

    Cystck_Object vertices_obj = 0;
    Cystck_Object codes_obj = 0;
    Cystck_Object should_simplify_obj = 0;
    Cystck_Object simplify_threshold_obj = 0;
    bool should_simplify;
    double simplify_threshold;

    int status = 0;

    if (Cystck_IsNULL(obj) || Cystck_Is(S, obj, S->Cystck_None)) {
        return 1;
    }

    vertices_obj = Cystck_GetAttr_s(S, obj, "vertices");
    if (Cystck_IsNULL(vertices_obj)) {
        goto exit;
    }

    codes_obj = Cystck_GetAttr_s(S, obj, "codes");
    if (Cystck_IsNULL(codes_obj)) {
        goto exit;
    }

    should_simplify_obj = Cystck_GetAttr_s(S, obj, "should_simplify");
    if (Cystck_IsNULL(should_simplify_obj)) {
        goto exit;
    }
    switch (Cystck_IsTrue(S, should_simplify_obj)) {
        case 0: should_simplify = 0; break;
        case 1: should_simplify = 1; break;
        default: goto exit;  // errored.
    }

    simplify_threshold_obj = Cystck_GetAttr_s(S, obj, "simplify_threshold");
    if (Cystck_IsNULL(simplify_threshold_obj)) {
        goto exit;
    }
    simplify_threshold = CystckFloat_AsDouble(S, simplify_threshold_obj);
    if (Cystck_Err_Occurred(S)) {
        goto exit;
    }

    if (!path->set(
            Cystck_AsPyObject(S, vertices_obj), // PyArrayObject (NumPy)
            Cystck_AsPyObject(S, codes_obj), // PyArrayObject (NumPy)
            should_simplify, simplify_threshold)) {
        goto exit;
    }

    status = 1;

exit:
    Py_XDECREF(Cystck2py(vertices_obj));
    Py_XDECREF(Cystck2py(codes_obj));
    Py_XDECREF(Cystck2py(should_simplify_obj));
    Py_XDECREF(Cystck2py(simplify_threshold_obj));

    return status;
}

int convert_clippath_cystck(Py_State *S, Cystck_Object clippath_tuple, void *clippathp)
{
    Cystck_Object h_path = 0, h_trans = 0;
    ClipPath *clippath = (ClipPath *)clippathp;
    py::PathIterator path;
    agg::trans_affine trans;

    int res = 1;
    if (!Cystck_IsNULL(clippath_tuple) && !Cystck_Is(S, clippath_tuple, S->Cystck_None)) {
        Arg_ParseTuple(res, S, clippath_tuple, "OO:clippath",
                              &h_path,
                              &h_trans);
        if (res && (!convert_path_cystck(S, h_path, &clippath->path)
                    || !convert_trans_affine_cystck(S, h_trans, &clippath->trans))) {
            if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "clippath"); // TODO
            res = 0;
        }
        Py_XDECREF(Cystck2py(clippath_tuple));
    }

    return res;
}

int convert_snap_cystck(Py_State *S, Cystck_Object obj, void *snapp)
{
    e_snap_mode *snap = (e_snap_mode *)snapp;
    if (Cystck_IsNULL(obj) || Cystck_Is(S, obj, S->Cystck_None)) {
        *snap = SNAP_AUTO;
    } else {
        switch (Cystck_IsTrue(S, obj)) {
            case 0: *snap = SNAP_FALSE; break;
            case 1: *snap = SNAP_TRUE; break;
            default: return 0;  // errored.
        }
    }
    return 1;
}

int convert_sketch_params_cystck(Py_State *S, Cystck_Object obj, void *sketchp)
{
    SketchParams *sketch = (SketchParams *)sketchp;

    if (Cystck_IsNULL(obj) || Cystck_Is(S, obj, S->Cystck_None)) {
        sketch->scale = 0.0;
    } else {
        int res;
        Arg_ParseTuple(res, S, obj, "ddd:sketch_params",
                                 &sketch->scale,
                                 &sketch->length,
                                 &sketch->randomness);
        Py_XDECREF(Cystck2py(obj));
        if (!res) {
            return 0;
        }
    }

    return 1;
}

int convert_gcagg_cystck(Py_State *S, Cystck_Object pygc, void *gcp)
{
    GCAgg *gc = (GCAgg *)gcp;

    if (!(convert_from_attr_cystck(S, pygc, "_linewidth", &convert_double_cystck, &gc->linewidth) &&
          convert_from_attr_cystck(S, pygc, "_alpha", &convert_double_cystck, &gc->alpha) &&
          convert_from_attr_cystck(S, pygc, "_forced_alpha", &convert_bool_cystck, &gc->forced_alpha) &&
          convert_from_attr_cystck(S, pygc, "_rgb", &convert_rgba_cystck, &gc->color) &&
          convert_from_attr_cystck(S, pygc, "_antialiased", &convert_bool_cystck, &gc->isaa) &&
          convert_from_attr_cystck(S, pygc, "_capstyle", &convert_cap_cystck, &gc->cap) &&
          convert_from_attr_cystck(S, pygc, "_joinstyle", &convert_join_cystck, &gc->join) &&
          convert_from_method_cystck(S, pygc, "get_dashes", &convert_dashes_cystck, &gc->dashes) &&
          convert_from_attr_cystck(S, pygc, "_cliprect", &convert_rect_cystck, &gc->cliprect) &&
          convert_from_method_cystck(S, pygc, "get_clip_path", &convert_clippath_cystck, &gc->clippath) &&
          convert_from_method_cystck(S, pygc, "get_snap", &convert_snap_cystck, &gc->snap_mode) &&
          convert_from_method_cystck(S, pygc, "get_hatch_path", &convert_path_cystck, &gc->hatchpath) &&
          convert_from_method_cystck(S, pygc, "get_hatch_color", &convert_rgba_cystck, &gc->hatch_color) &&
          convert_from_method_cystck(S, pygc, "get_hatch_linewidth", &convert_double_cystck, &gc->hatch_linewidth) &&
          convert_from_method_cystck(S, pygc, "get_sketch_params", &convert_sketch_params_cystck, &gc->sketch))) {
        return 0;
    }

    return 1;
}

int convert_offset_position_cystck(Py_State *S, Cystck_Object obj, void *offsetp)
{
    e_offset_position *offset = (e_offset_position *)offsetp;
    const char *names[] = {"data", NULL};
    int values[] = {OFFSET_POSITION_DATA};
    int result = (int)OFFSET_POSITION_FIGURE;

    if (!convert_string_enum_cystck(S, obj, "offset_position", names, values, &result)) {
        CystckErr_Clear(S);
    }

    *offset = (e_offset_position)result;

    return 1;
}

int convert_face_cystck(Py_State *S, Cystck_Object color, GCAgg &gc, agg::rgba *rgba)
{
    if (!convert_rgba_cystck(S, color, rgba)) {
        return 0;
    }

    if (!Cystck_IsNULL(color) && !Cystck_Is(S, color, S->Cystck_None)) {
        if (gc.forced_alpha || Cystck_Length(S, color) == 3) {
            rgba->a = gc.alpha;
        }
    }

    return 1;
}

int convert_points_cystck(Py_State *S, Cystck_Object obj, void *pointsp)
{
    numpy::array_view<double, 2> *points = (numpy::array_view<double, 2> *)pointsp;

    if (Cystck_IsNULL(obj) || Cystck_Is(S, obj, S->Cystck_None)) {
        return 1;
    }

    points->set(Cystck_AsPyObject(S, obj));

    if (points->size() == 0) {
        return 1;
    }

    if (points->dim(1) != 2) {
        // PyErr_Format(PyExc_ValueError,
        //              "Points must be Nx2 array, got %" NPY_INTP_FMT "x%" NPY_INTP_FMT,
        //              points->dim(0), points->dim(1)); 
        CystckErr_SetString(S, S->Cystck_ValueError,
                     "Points must be Nx2 array");
        return 0;
    }

    return 1;
}

int convert_transforms_cystck(Py_State *S, Cystck_Object obj, void *transp)
{
    numpy::array_view<double, 3> *trans = (numpy::array_view<double, 3> *)transp;

    if (Cystck_IsNULL(obj) || Cystck_Is(S, obj, S->Cystck_None)) {
        return 1;
    }

    trans->set(Cystck_AsPyObject(S, obj));

    if (trans->size() == 0) {
        return 1;
    }

    if (trans->dim(1) != 3 || trans->dim(2) != 3) {
        CystckErr_SetString(S, S->Cystck_ValueError,
                     "Transforms must be Nx3x3 array");
        return 0;
    }

    return 1;
}

int convert_bboxes_cystck(Py_State *S, Cystck_Object obj, void *bboxp)
{
    numpy::array_view<double, 3> *bbox = (numpy::array_view<double, 3> *)bboxp;

    if (Cystck_IsNULL(obj) || Cystck_Is(S, obj, S->Cystck_None)) {
        return 1;
    }

    bbox->set(Cystck_AsPyObject(S, obj));

    if (bbox->size() == 0) {
        return 1;
    }

    if (bbox->dim(1) != 2 || bbox->dim(2) != 2) {
        // PyErr_Format(PyExc_ValueError,
        //              "Bbox array must be Nx2x2 array, got %" NPY_INTP_FMT "x%" NPY_INTP_FMT "x%" NPY_INTP_FMT,
        //              bbox->dim(0), bbox->dim(1), bbox->dim(2)); 
        CystckErr_SetString(S, S->Cystck_ValueError,
                     "Bbox array must be Nx2x2 array");
        return 0;
    }

    return 1;
}

int convert_colors_cystck(Py_State *S, Cystck_Object obj, void *colorsp)
{
    numpy::array_view<double, 2> *colors = (numpy::array_view<double, 2> *)colorsp;

    if (Cystck_IsNULL(obj) || Cystck_Is(S, obj, S->Cystck_None)) {
        return 1;
    }

    colors->set(Cystck_AsPyObject(S, obj));

    if (colors->size() == 0) {
        return 1;
    }

    if (colors->dim(1) != 4) {
        // PyErr_Format(PyExc_ValueError,
        //              "Colors array must be Nx4 array, got %" NPY_INTP_FMT "x%" NPY_INTP_FMT,
        //              colors->dim(0), colors->dim(1)); 
        CystckErr_SetString(S, S->Cystck_ValueError,
                     "Colors array must be Nx4 array");
        return 0;
    }

    return 1;
}

#else

static int convert_string_enum(PyObject *obj, const char *name, const char **names, int *values, int *result)
{
    PyObject *bytesobj;
    char *str;

    if (obj == NULL || obj == Py_None) {
        return 1;
    }

    if (PyUnicode_Check(obj)) {
        bytesobj = PyUnicode_AsASCIIString(obj);
        if (bytesobj == NULL) {
            return 0;
        }
    } else if (PyBytes_Check(obj)) {
        Py_INCREF(obj);
        bytesobj = obj;
    } else {
        PyErr_Format(PyExc_TypeError, "%s must be str or bytes", name);
        return 0;
    }

    str = PyBytes_AsString(bytesobj);
    if (str == NULL) {
        Py_DECREF(bytesobj);
        return 0;
    }

    for ( ; *names != NULL; names++, values++) {
        if (strncmp(str, *names, 64) == 0) {
            *result = *values;
            Py_DECREF(bytesobj);
            return 1;
        }
    }

    PyErr_Format(PyExc_ValueError, "invalid %s value", name);
    Py_DECREF(bytesobj);
    return 0;
}

int convert_from_method(PyObject *obj, const char *name, converter func, void *p)
{
    PyObject *value;

    value = PyObject_CallMethod(obj, name, NULL);
    if (value == NULL) {
        if (!PyObject_HasAttrString(obj, name)) {
            PyErr_Clear();
            return 1;
        }
        return 0;
    }

    if (!func(value, p)) {
        Py_DECREF(value);
        return 0;
    }

    Py_DECREF(value);
    return 1;
}

int convert_from_attr(PyObject *obj, const char *name, converter func, void *p)
{
    PyObject *value;

    value = PyObject_GetAttrString(obj, name);
    if (value == NULL) {
        if (!PyObject_HasAttrString(obj, name)) {
            PyErr_Clear();
            return 1;
        }
        return 0;
    }

    if (!func(value, p)) {
        Py_DECREF(value);
        return 0;
    }

    Py_DECREF(value);
    return 1;
}

int convert_double(PyObject *obj, void *p)
{
    double *val = (double *)p;

    *val = PyFloat_AsDouble(obj);
    if (PyErr_Occurred()) {
        return 0;
    }

    return 1;
}

int convert_bool(PyObject *obj, void *p)
{
    bool *val = (bool *)p;
    switch (PyObject_IsTrue(obj)) {
        case 0: *val = false; break;
        case 1: *val = true; break;
        default: return 0;  // errored.
    }
    return 1;
}

int convert_cap(PyObject *capobj, void *capp)
{
    const char *names[] = {"butt", "round", "projecting", NULL};
    int values[] = {agg::butt_cap, agg::round_cap, agg::square_cap};
    int result = agg::butt_cap;

    if (!convert_string_enum(capobj, "capstyle", names, values, &result)) {
        return 0;
    }

    *(agg::line_cap_e *)capp = (agg::line_cap_e)result;
    return 1;
}

int convert_join(PyObject *joinobj, void *joinp)
{
    const char *names[] = {"miter", "round", "bevel", NULL};
    int values[] = {agg::miter_join_revert, agg::round_join, agg::bevel_join};
    int result = agg::miter_join_revert;

    if (!convert_string_enum(joinobj, "joinstyle", names, values, &result)) {
        return 0;
    }

    *(agg::line_join_e *)joinp = (agg::line_join_e)result;
    return 1;
}

int convert_rect(PyObject *rectobj, void *rectp)
{
    agg::rect_d *rect = (agg::rect_d *)rectp;

    if (rectobj == NULL || rectobj == Py_None) {
        rect->x1 = 0.0;
        rect->y1 = 0.0;
        rect->x2 = 0.0;
        rect->y2 = 0.0;
    } else {
        PyArrayObject *rect_arr = (PyArrayObject *)PyArray_ContiguousFromAny(
                rectobj, NPY_DOUBLE, 1, 2);
        if (rect_arr == NULL) {
            return 0;
        }

        if (PyArray_NDIM(rect_arr) == 2) {
            if (PyArray_DIM(rect_arr, 0) != 2 ||
                PyArray_DIM(rect_arr, 1) != 2) {
                PyErr_SetString(PyExc_ValueError, "Invalid bounding box");
                Py_DECREF(rect_arr);
                return 0;
            }

        } else {  // PyArray_NDIM(rect_arr) == 1
            if (PyArray_DIM(rect_arr, 0) != 4) {
                PyErr_SetString(PyExc_ValueError, "Invalid bounding box");
                Py_DECREF(rect_arr);
                return 0;
            }
        }

        double *buff = (double *)PyArray_DATA(rect_arr);
        rect->x1 = buff[0];
        rect->y1 = buff[1];
        rect->x2 = buff[2];
        rect->y2 = buff[3];

        Py_DECREF(rect_arr);
    }
    return 1;
}

int convert_rgba(PyObject *rgbaobj, void *rgbap)
{
    agg::rgba *rgba = (agg::rgba *)rgbap;
    PyObject *rgbatuple = NULL;
    int success = 1;
    if (rgbaobj == NULL || rgbaobj == Py_None) {
        rgba->r = 0.0;
        rgba->g = 0.0;
        rgba->b = 0.0;
        rgba->a = 0.0;
    } else {
        if (!(rgbatuple = PySequence_Tuple(rgbaobj))) {
            success = 0;
            goto exit;
        }
        rgba->a = 1.0;
        if (!PyArg_ParseTuple(
                 rgbatuple, "ddd|d:rgba", &(rgba->r), &(rgba->g), &(rgba->b), &(rgba->a))) {
            success = 0;
            goto exit;
        }
    }
exit:
    Py_XDECREF(rgbatuple);
    return success;
}

int convert_dashes(PyObject *dashobj, void *dashesp)
{
    Dashes *dashes = (Dashes *)dashesp;

    double dash_offset = 0.0;
    PyObject *dashes_seq = NULL;

    if (!PyArg_ParseTuple(dashobj, "dO:dashes", &dash_offset, &dashes_seq)) {
        return 0;
    }

    if (dashes_seq == Py_None) {
        return 1;
    }

    if (!PySequence_Check(dashes_seq)) {
        PyErr_SetString(PyExc_TypeError, "Invalid dashes sequence");
        return 0;
    }

    Py_ssize_t nentries = PySequence_Size(dashes_seq);
    // If the dashpattern has odd length, iterate through it twice (in
    // accordance with the pdf/ps/svg specs).
    Py_ssize_t dash_pattern_length = (nentries % 2) ? 2 * nentries : nentries;

    for (Py_ssize_t i = 0; i < dash_pattern_length; ++i) {
        PyObject *item;
        double length;
        double skip;

        item = PySequence_GetItem(dashes_seq, i % nentries);
        if (item == NULL) {
            return 0;
        }
        length = PyFloat_AsDouble(item);
        if (PyErr_Occurred()) {
            Py_DECREF(item);
            return 0;
        }
        Py_DECREF(item);

        ++i;

        item = PySequence_GetItem(dashes_seq, i % nentries);
        if (item == NULL) {
            return 0;
        }
        skip = PyFloat_AsDouble(item);
        if (PyErr_Occurred()) {
            Py_DECREF(item);
            return 0;
        }
        Py_DECREF(item);

        dashes->add_dash_pair(length, skip);
    }

    dashes->set_dash_offset(dash_offset);

    return 1;
}

int convert_dashes_vector(PyObject *obj, void *dashesp)
{
    DashesVector *dashes = (DashesVector *)dashesp;

    if (!PySequence_Check(obj)) {
        return 0;
    }

    Py_ssize_t n = PySequence_Size(obj);

    for (Py_ssize_t i = 0; i < n; ++i) {
        PyObject *item;
        Dashes subdashes;

        item = PySequence_GetItem(obj, i);
        if (item == NULL) {
            return 0;
        }

        if (!convert_dashes(item, &subdashes)) {
            Py_DECREF(item);
            return 0;
        }
        Py_DECREF(item);

        dashes->push_back(subdashes);
    }

    return 1;
}

int convert_trans_affine(PyObject *obj, void *transp)
{
    agg::trans_affine *trans = (agg::trans_affine *)transp;

    /** If None assume identity transform. */
    if (obj == NULL || obj == Py_None) {
        return 1;
    }

    PyArrayObject *array = (PyArrayObject *)PyArray_ContiguousFromAny(obj, NPY_DOUBLE, 2, 2);
    if (array == NULL) {
        return 0;
    }

    if (PyArray_DIM(array, 0) == 3 && PyArray_DIM(array, 1) == 3) {
        double *buffer = (double *)PyArray_DATA(array);
        trans->sx = buffer[0];
        trans->shx = buffer[1];
        trans->tx = buffer[2];

        trans->shy = buffer[3];
        trans->sy = buffer[4];
        trans->ty = buffer[5];

        Py_DECREF(array);
        return 1;
    }

    Py_DECREF(array);
    PyErr_SetString(PyExc_ValueError, "Invalid affine transformation matrix");
    return 0;
}

int convert_path(PyObject *obj, void *pathp)
{
    py::PathIterator *path = (py::PathIterator *)pathp;

    PyObject *vertices_obj = NULL;
    PyObject *codes_obj = NULL;
    PyObject *should_simplify_obj = NULL;
    PyObject *simplify_threshold_obj = NULL;
    bool should_simplify;
    double simplify_threshold;

    int status = 0;

    if (obj == NULL || obj == Py_None) {
        return 1;
    }

    vertices_obj = PyObject_GetAttrString(obj, "vertices");
    if (vertices_obj == NULL) {
        goto exit;
    }

    codes_obj = PyObject_GetAttrString(obj, "codes");
    if (codes_obj == NULL) {
        goto exit;
    }

    should_simplify_obj = PyObject_GetAttrString(obj, "should_simplify");
    if (should_simplify_obj == NULL) {
        goto exit;
    }
    switch (PyObject_IsTrue(should_simplify_obj)) {
        case 0: should_simplify = 0; break;
        case 1: should_simplify = 1; break;
        default: goto exit;  // errored.
    }

    simplify_threshold_obj = PyObject_GetAttrString(obj, "simplify_threshold");
    if (simplify_threshold_obj == NULL) {
        goto exit;
    }
    simplify_threshold = PyFloat_AsDouble(simplify_threshold_obj);
    if (PyErr_Occurred()) {
        goto exit;
    }

    if (!path->set(vertices_obj, codes_obj, should_simplify, simplify_threshold)) {
        goto exit;
    }

    status = 1;

exit:
    Py_XDECREF(vertices_obj);
    Py_XDECREF(codes_obj);
    Py_XDECREF(should_simplify_obj);
    Py_XDECREF(simplify_threshold_obj);

    return status;
}

int convert_pathgen(PyObject *obj, void *pathgenp)
{
    py::PathGenerator *paths = (py::PathGenerator *)pathgenp;
    if (!paths->set(obj)) {
        PyErr_SetString(PyExc_TypeError, "Not an iterable of paths");
        return 0;
    }
    return 1;
}

int convert_clippath(PyObject *clippath_tuple, void *clippathp)
{
    ClipPath *clippath = (ClipPath *)clippathp;
    py::PathIterator path;
    agg::trans_affine trans;

    if (clippath_tuple != NULL && clippath_tuple != Py_None) {
        if (!PyArg_ParseTuple(clippath_tuple,
                              "O&O&:clippath",
                              &convert_path,
                              &clippath->path,
                              &convert_trans_affine,
                              &clippath->trans)) {
            return 0;
        }
    }

    return 1;
}

int convert_snap(PyObject *obj, void *snapp)
{
    e_snap_mode *snap = (e_snap_mode *)snapp;
    if (obj == NULL || obj == Py_None) {
        *snap = SNAP_AUTO;
    } else {
        switch (PyObject_IsTrue(obj)) {
            case 0: *snap = SNAP_FALSE; break;
            case 1: *snap = SNAP_TRUE; break;
            default: return 0;  // errored.
        }
    }
    return 1;
}

int convert_sketch_params(PyObject *obj, void *sketchp)
{
    SketchParams *sketch = (SketchParams *)sketchp;

    if (obj == NULL || obj == Py_None) {
        sketch->scale = 0.0;
    } else if (!PyArg_ParseTuple(obj,
                                 "ddd:sketch_params",
                                 &sketch->scale,
                                 &sketch->length,
                                 &sketch->randomness)) {
        return 0;
    }

    return 1;
}

int convert_gcagg(PyObject *pygc, void *gcp)
{
    GCAgg *gc = (GCAgg *)gcp;

    if (!(convert_from_attr(pygc, "_linewidth", &convert_double, &gc->linewidth) &&
          convert_from_attr(pygc, "_alpha", &convert_double, &gc->alpha) &&
          convert_from_attr(pygc, "_forced_alpha", &convert_bool, &gc->forced_alpha) &&
          convert_from_attr(pygc, "_rgb", &convert_rgba, &gc->color) &&
          convert_from_attr(pygc, "_antialiased", &convert_bool, &gc->isaa) &&
          convert_from_attr(pygc, "_capstyle", &convert_cap, &gc->cap) &&
          convert_from_attr(pygc, "_joinstyle", &convert_join, &gc->join) &&
          convert_from_method(pygc, "get_dashes", &convert_dashes, &gc->dashes) &&
          convert_from_attr(pygc, "_cliprect", &convert_rect, &gc->cliprect) &&
          convert_from_method(pygc, "get_clip_path", &convert_clippath, &gc->clippath) &&
          convert_from_method(pygc, "get_snap", &convert_snap, &gc->snap_mode) &&
          convert_from_method(pygc, "get_hatch_path", &convert_path, &gc->hatchpath) &&
          convert_from_method(pygc, "get_hatch_color", &convert_rgba, &gc->hatch_color) &&
          convert_from_method(pygc, "get_hatch_linewidth", &convert_double, &gc->hatch_linewidth) &&
          convert_from_method(pygc, "get_sketch_params", &convert_sketch_params, &gc->sketch))) {
        return 0;
    }

    return 1;
}

int convert_face(PyObject *color, GCAgg &gc, agg::rgba *rgba)
{
    if (!convert_rgba(color, rgba)) {
        return 0;
    }

    if (color != NULL && color != Py_None) {
        if (gc.forced_alpha || PySequence_Size(color) == 3) {
            rgba->a = gc.alpha;
        }
    }

    return 1;
}

int convert_points(PyObject *obj, void *pointsp)
{
    numpy::array_view<double, 2> *points = (numpy::array_view<double, 2> *)pointsp;

    if (obj == NULL || obj == Py_None) {
        return 1;
    }

    points->set(obj);

    if (points->size() == 0) {
        return 1;
    }

    if (points->dim(1) != 2) {
        PyErr_Format(PyExc_ValueError,
                     "Points must be Nx2 array, got %" NPY_INTP_FMT "x%" NPY_INTP_FMT,
                     points->dim(0), points->dim(1));
        return 0;
    }

    return 1;
}

int convert_transforms(PyObject *obj, void *transp)
{
    numpy::array_view<double, 3> *trans = (numpy::array_view<double, 3> *)transp;

    if (obj == NULL || obj == Py_None) {
        return 1;
    }

    trans->set(obj);

    if (trans->size() == 0) {
        return 1;
    }

    if (trans->dim(1) != 3 || trans->dim(2) != 3) {
        PyErr_Format(PyExc_ValueError,
                     "Transforms must be Nx3x3 array, got %" NPY_INTP_FMT "x%" NPY_INTP_FMT "x%" NPY_INTP_FMT,
                     trans->dim(0), trans->dim(1), trans->dim(2));
        return 0;
    }

    return 1;
}

int convert_bboxes(PyObject *obj, void *bboxp)
{
    numpy::array_view<double, 3> *bbox = (numpy::array_view<double, 3> *)bboxp;

    if (obj == NULL || obj == Py_None) {
        return 1;
    }

    bbox->set(obj);

    if (bbox->size() == 0) {
        return 1;
    }

    if (bbox->dim(1) != 2 || bbox->dim(2) != 2) {
        PyErr_Format(PyExc_ValueError,
                     "Bbox array must be Nx2x2 array, got %" NPY_INTP_FMT "x%" NPY_INTP_FMT "x%" NPY_INTP_FMT,
                     bbox->dim(0), bbox->dim(1), bbox->dim(2));
        return 0;
    }

    return 1;
}

int convert_colors(PyObject *obj, void *colorsp)
{
    numpy::array_view<double, 2> *colors = (numpy::array_view<double, 2> *)colorsp;

    if (obj == NULL || obj == Py_None) {
        return 1;
    }

    colors->set(obj);

    if (colors->size() == 0) {
        return 1;
    }

    if (colors->dim(1) != 4) {
        PyErr_Format(PyExc_ValueError,
                     "Colors array must be Nx4 array, got %" NPY_INTP_FMT "x%" NPY_INTP_FMT,
                     colors->dim(0), colors->dim(1));
        return 0;
    }

    return 1;
}
#endif
}
