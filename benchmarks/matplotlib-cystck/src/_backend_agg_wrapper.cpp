// #define CYSTCK
#include "mplutils.h"
#include "numpy_cpp.h"
#include "py_converters.h"
#include "_backend_agg.h"
#include "../../include/Cystck.h"

typedef struct
{
    Cystck_HEAD
    RendererAgg *x;
    Cystck_ssize_t shape[3];
    Cystck_ssize_t strides[3];
    Cystck_ssize_t suboffsets[3];
} PyRendererAgg;
CystckType_HELPERS(PyRendererAgg)

//static PyTypeObject PyRendererAggType;

typedef struct
{
    Cystck_HEAD
    BufferRegion *x;
    Cystck_ssize_t shape[3];
    Cystck_ssize_t strides[3];
    Cystck_ssize_t suboffsets[3];
} PyBufferRegion;
CystckType_HELPERS(PyBufferRegion)
static PyTypeObject PyBufferRegionType;


/**********************************************************************
 * BufferRegion
 * */

static Cystck_Object PyBufferRegion_new( Py_State *S, Cystck_Object type, Cystck_Object args, Cystck_Object kwds)
{
    PyBufferRegion *self;
    Cystck_Object h_self = Cystck_New(S, type, &self);
    self->x = NULL;
    Cystck_pushobject(S, h_self);
    return 1;
}
// static PyObject *PyBufferRegion_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
// {
//     PyBufferRegion *self;
//     self = (PyBufferRegion *)type->tp_alloc(type, 0);
//     self->x = NULL;
//     return (PyObject *)self;
// }

static void PyBufferRegion_dealloc(Py_State *S)
{
    PyBufferRegion *self = (PyBufferRegion*)S->self;
    delete self->x;
    //Py_TYPE(self)->tp_free((PyObject *)self);
}

static Cystck_Object PyBufferRegion_to_string(Py_State *S)
{
    PyBufferRegion* self = PyBufferRegion_AsStruct(S, S->self);

    Cystck_pushobject(S, CystckBytes_FromStringAndSize(S, (const char *)self->x->get_data(),
                                     (Cystck_ssize_t) self->x->get_height() * self->x->get_stride()));
    return 1;
}

/* TODO: This doesn't seem to be used internally.  Remove? */

static Cystck_Object PyBufferRegion_set_x(Py_State *S, Cystck_Object args)
{
    int x;
    if (!CystckArg_parseTuple(S, args, "i:set_x", &x)) {
        return -1;
    }
    PyBufferRegion *self = PyBufferRegion_AsStruct(S, S->self);
    self->x->get_rect().x1 = x;
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;
}

static Cystck_Object PyBufferRegion_set_y(Py_State *S, Cystck_Object args)
{
    int y;
    if (!CystckArg_parseTuple(S, args, "i:set_y", &y)) {
        return -1;
    }
    PyBufferRegion *self = PyBufferRegion_AsStruct(S, S->self);
    self->x->get_rect().y1 = y;
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;
}

static Cystck_Object PyBufferRegion_get_extents(Py_State *S)
{
    PyBufferRegion* self = PyBufferRegion_AsStruct(S, S->self);
    agg::rect_i rect = self->x->get_rect();
    Cystck_Pushobject(S, Py_BuildValue("IIII", rect.x1, rect.y1, rect.x2, rect.y2));
    return 1;
}

static Cystck_Object PyBufferRegion_to_string_argb(Py_State *S)
{
    PyBufferRegion *self = PyBufferRegion_AsStruct(S, S->self);
    char const* msg =
        "BufferRegion.to_string_argb is deprecated since Matplotlib 3.7 and "
        "will be removed two minor releases later; use "
        "np.take(region, [2, 1, 0, 3], axis=2) instead.";
    if (CystckErr_WarnEx(S, S->Cystck_DeprecationWarning, msg, 1)) {
        return -1;
    }
    Cystck_Object bufobj = CystckBytes_FromStringAndSize(S, NULL, self->x->get_height() * self->x->get_stride());
    Cystck_pushobject(S, bufobj);
    uint8_t *buf = (uint8_t *)CystckBytes_AS_STRING(S, bufobj);
    CALL_CPP_CLEANUP_CYSTCK(S, "to_string_argb", (self->x->to_string_argb(buf)), Cystck_pop(S,bufobj));
    // CALL_CPP_CLEANUP("to_string_argb", (self->x->to_string_argb(buf)), Py_XDECREF(Cystck2py(bufobj)));
    return 1;
}

int PyBufferRegion_get_buffer(Py_State *S, Cystck_Object h_self,  Cystck_buffer *buf, int flags)
{
    PyBufferRegion *self = PyBufferRegion_AsStruct(S, h_self);
    buf->obj = Cystck_Dup(S, h_self);
    buf->buf = self->x->get_data();
    buf->len = (Cystck_ssize_t)self->x->get_width() * (Cystck_ssize_t)self->x->get_height() * 4;
    buf->readonly = 0;
    buf->format = (char *)"B";
    buf->ndim = 3;
    self->shape[0] = self->x->get_height();
    self->shape[1] = self->x->get_width();
    self->shape[2] = 4;
    buf->shape = self->shape;
    self->strides[0] = self->x->get_width() * 4;
    self->strides[1] = 4;
    self->strides[2] = 1;
    buf->strides = self->strides;
    buf->suboffsets = NULL;
    buf->itemsize = 1;
    buf->internal = NULL;

    return 1;
}

//CystckDef_SLOT(PyBufferRegion_new_def, PyBufferRegion_new, Cystck_tp_new)
CystckDef_SLOT(PyBufferRegion_get_buffer_def, PyBufferRegion_get_buffer, Cystck_bf_getbuffer)
CystckDef_SLOT(PyBufferRegion_dealloc_def, PyBufferRegion_dealloc, Cystck_tp_dealloc)

CystckDef_METH(PyBufferRegion_to_string_def, "to_string", PyBufferRegion_to_string, Cystck_METH_NOARGS, NULL)
CystckDef_METH(PyBufferRegion_to_string_argb_def, "to_string_argb", PyBufferRegion_to_string_argb, Cystck_METH_NOARGS, NULL)
CystckDef_METH(PyBufferRegion_set_x_def, "set_x", PyBufferRegion_set_x, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyBufferRegion_set_y_def, "set_y", PyBufferRegion_set_y, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyBufferRegion_get_extents_def, "get_extents", PyBufferRegion_get_extents, Cystck_METH_NOARGS, NULL)

static PyType_Slot PyBufferRegion_defineslegacy []={
    {Py_tp_new, (void *)PyBufferRegion_new},
    {0,0}
};
CystckDef *PyBufferRegion_defines[] = {
    // slots
    //&PyBufferRegion_new_def,
    &PyBufferRegion_get_buffer_def,
    &PyBufferRegion_dealloc_def,

    // methods
    &PyBufferRegion_to_string_def,
    &PyBufferRegion_to_string_argb_def,
    &PyBufferRegion_set_x_def,
    &PyBufferRegion_set_y_def,
    &PyBufferRegion_get_extents_def,
    NULL
};

Cystck_Type_Spec PyBufferRegion_type_spec = {
    .name = "matplotlib.backends._backend_agg.BufferRegion",
    .basicsize = sizeof(PyBufferRegion),
    .flags = Cystck_TPFLAGS_DEFAULT | Cystck_TPFLAGS_BASETYPE,
    .legacy_slots = PyBufferRegion_defineslegacy,
    .m_methods = PyBufferRegion_defines,
};

/**********************************************************************
 * RendererAgg
 * */
// #undef CYSTCK
static Cystck_Object  PyRendererAgg_new(Py_State *S, Cystck_Object type, Cystck_Object args, Cystck_Object kwds)
{
    PyRendererAgg *self;
    Cystck_Object h_self = Cystck_New(S, type, &self);
    self->x = NULL;
    Cystck_pushobject(S, h_self);
    return 1;
}
static int PyRendererAgg_init(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    unsigned int width;
    unsigned int height;
    double dpi;
    int debug = 0;

    if (!CystckArg_parseTuple(S, args, "IId|i:RendererAgg", &width, &height, &dpi, &debug)) {
        return -1;
    }

    if (dpi <= 0.0) {
        CystckErr_SetString(S, S->Cystck_ValueError, "dpi must be positive");
        return -1;
    }

    if (width >= 1 << 16 || height >= 1 << 16) {
        CystckErr_SetString(S, S->Cystck_ValueError,
            "Image size is too large. "
            "It must be less than 2^16 in each direction.");
        return -1;
    }
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    CALL_CPP_INIT_CYSTCK(S, "RendererAgg", self->x = new RendererAgg(width, height, dpi));
    // CALL_CPP_INIT("RendererAgg", self->x = new RendererAgg(width, height, dpi));

    return 0;
}

static void PyRendererAgg_dealloc(Py_State *S)
{
    PyRendererAgg *self = (PyRendererAgg*)S->self;
    delete self->x;
    //Py_TYPE(self)->tp_free((PyObject *)self);
}

static Cystck_Object PyRendererAgg_draw_path( Py_State *S, Cystck_Object args)
{
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    Cystck_Object h_gc = 0;
    Cystck_Object h_path = 0;
    Cystck_Object h_trans = 0;
    GCAgg gc;
    py::PathIterator path;
    agg::trans_affine trans;
    Cystck_Object faceobj = 0;
    agg::rgba face;

    if (!CystckArg_parseTuple(S, args,
                          "OOO|O:draw_path",
                          &h_gc,
                          &h_path,
                          &h_trans,
                          &faceobj)) {
        return -1;
    }
    if (!convert_gcagg(Cystck_AsPyObject(S, h_gc), &gc)
                || !convert_path(Cystck_AsPyObject(S, h_path), &path)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)
                || !convert_face(Cystck_AsPyObject(S,faceobj), gc, &face)) {
            if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "draw_path"); 
        return -1;
    }

    CALL_CPP_CYSTCK(S,"draw_path", (self->x->draw_path(gc, path, trans, face)));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

    return 1;
}

static Cystck_Object PyRendererAgg_draw_text_image(Py_State *S, Cystck_Object args)
{
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    numpy::array_view<agg::int8u, 2> image;
    double x;
    double y;
    double angle;
    GCAgg gc;
    Cystck_Object h_image = 0;
    Cystck_Object h_gc = 0;

    if (!CystckArg_parseTuple(S, args,
                          "OdddO:draw_text_image",
                          &h_image,
                          &x,
                          &y,
                          &angle,
                          &h_gc)) {
        return -1;
    }
    if (!image.converter_contiguous(Cystck_AsPyObject(S, h_image), &image)
            || !convert_gcagg(Cystck_AsPyObject(S, h_gc), &gc)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "draw_text_image"); 
        return -1;
    }

    CALL_CPP_CYSTCK(S,"draw_text_image", (self->x->draw_text_image(gc, image, x, y, angle)));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

    return 1;
}

static Cystck_Object PyRendererAgg_draw_markers(Py_State *S, Cystck_Object args)
{
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    Cystck_Object h_gc = 0;
    Cystck_Object h_marker_path = 0;
    Cystck_Object h_marker_path_trans = 0;
    Cystck_Object h_path = 0;
    Cystck_Object h_trans = 0;
    GCAgg gc;
    py::PathIterator marker_path;
    agg::trans_affine marker_path_trans;
    py::PathIterator path;
    agg::trans_affine trans;
    Cystck_Object faceobj = 0;
    agg::rgba face;

    if (!CystckArg_parseTuple(S, args,
                          "OOOOO|O:draw_markers",
                          &h_gc,
                          &h_marker_path,
                          &h_marker_path_trans,
                          &h_path,
                          &h_trans,
                          &faceobj)) {
        return -1;
    }

    if (!convert_gcagg(Cystck_AsPyObject(S, h_gc), &gc)
                || !convert_path(Cystck_AsPyObject(S, h_marker_path), &marker_path)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_marker_path_trans), &marker_path_trans)
                || !convert_path(Cystck_AsPyObject(S, h_path), &path)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)
                || !convert_face(Cystck_AsPyObject(S, faceobj), gc, &face)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "draw_markers");
        return -1;
    }

    CALL_CPP_CYSTCK(S,"draw_markers",
             (self->x->draw_markers(gc, marker_path, marker_path_trans, path, trans, face)));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

    return 1;
}

static Cystck_Object PyRendererAgg_draw_image(Py_State *S, Cystck_Object args)
{
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    Cystck_Object h_gc = 0;
    Cystck_Object h_image = 0;
    GCAgg gc;
    double x;
    double y;
    numpy::array_view<agg::int8u, 3> image;

    if (!CystckArg_parseTuple(S, args,
                          "OddO:draw_image",
                          &h_gc,
                          &x,
                          &y,
                          &h_image)) {
        return -1;
    }

    if (!convert_gcagg(Cystck_AsPyObject(S, h_gc), &gc)
                || !image.converter_contiguous(Cystck_AsPyObject(S, h_image), &image)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "draw_image");
        return -1;
    }

    x = mpl_round(x);
    y = mpl_round(y);

    gc.alpha = 1.0;
    CALL_CPP_CYSTCK(S,"draw_image", (self->x->draw_image(gc, x, y, image)));
    
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

    return 1;
}

static Cystck_Object
PyRendererAgg_draw_path_collection(Py_State *S,  Cystck_Object args)
{
    Cystck_Object h_gc = 0;
    Cystck_Object h_master_transform = 0;
    Cystck_Object h_paths = 0;
    Cystck_Object h_transforms = 0;
    Cystck_Object h_offsets = 0;
    Cystck_Object h_offset_trans = 0;
    Cystck_Object h_facecolors = 0;
    Cystck_Object h_edgecolors = 0;
    Cystck_Object h_linewidths = 0;
    Cystck_Object h_dashes = 0;
    Cystck_Object h_antialiaseds = 0;
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    GCAgg gc;
    agg::trans_affine master_transform;
    py::PathGenerator paths;
    numpy::array_view<const double, 3> transforms;
    numpy::array_view<const double, 2> offsets;
    agg::trans_affine offset_trans;
    numpy::array_view<const double, 2> facecolors;
    numpy::array_view<const double, 2> edgecolors;
    numpy::array_view<const double, 1> linewidths;
    DashesVector dashes;
    numpy::array_view<const uint8_t, 1> antialiaseds;
    Cystck_Object ignored;
    Cystck_Object offset_position; // offset position is no longer used

    if (!CystckArg_parseTuple(S, args,
                          "OOOOOOOOOOOOO:draw_path_collection",
                          &h_gc,
                          &h_master_transform,
                          &h_paths,
                          &h_transforms,
                          &h_offsets,
                          &h_offset_trans,
                          &h_facecolors,
                          &h_edgecolors,
                          &h_linewidths,
                          &h_dashes,
                          &h_antialiaseds,
                          &ignored,
                          &offset_position)) {
        return -1;
    }

    if (!convert_gcagg( Cystck_AsPyObject(S, h_gc), &gc)
                || !convert_trans_affine( Cystck_AsPyObject(S, h_master_transform), &master_transform)
                || !convert_transforms(Cystck_AsPyObject(S, h_transforms), &transforms)
                || !convert_points( Cystck_AsPyObject(S, h_offsets), &offsets)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_offset_trans), &offset_trans)
                || !convert_colors(Cystck_AsPyObject(S, h_facecolors), &facecolors)
                || !convert_colors(Cystck_AsPyObject(S, h_edgecolors), &edgecolors)
                || !linewidths.converter(Cystck_AsPyObject(S, h_linewidths), &linewidths)
                || !convert_dashes_vector(Cystck_AsPyObject(S, h_dashes), &dashes)
                || !antialiaseds.converter(Cystck_AsPyObject(S, h_antialiaseds), &antialiaseds)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "draw_path_collection"); 
        return -1;
    }

    CALL_CPP_CYSTCK(S,"draw_path_collection",
             (self->x->draw_path_collection(gc,
                                            master_transform,
                                            paths,
                                            transforms,
                                            offsets,
                                            offset_trans,
                                            facecolors,
                                            edgecolors,
                                            linewidths,
                                            dashes,
                                            antialiaseds)));
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

    return 1;
}

static Cystck_Object PyRendererAgg_draw_quad_mesh(Py_State *S, Cystck_Object args)
{
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    Cystck_Object h_gc = 0;
    Cystck_Object h_master_transform = 0;
    Cystck_Object h_coordinates = 0;
    Cystck_Object h_offsets = 0;
    Cystck_Object h_offset_trans = 0;
    Cystck_Object h_facecolors = 0;
    Cystck_Object h_antialiased = 0;
    Cystck_Object h_edgecolors = 0;
    GCAgg gc;
    agg::trans_affine master_transform;
    unsigned int mesh_width;
    unsigned int mesh_height;
    numpy::array_view<const double, 3> coordinates;
    numpy::array_view<const double, 2> offsets;
    agg::trans_affine offset_trans;
    numpy::array_view<const double, 2> facecolors;
    bool antialiased;
    numpy::array_view<const double, 2> edgecolors;

    if (!CystckArg_parseTuple(S, args,
                          "OOIIOOOOOO:draw_quad_mesh",
                          &h_gc,
                          &h_master_transform,
                          &mesh_width,
                          &mesh_height,
                          &h_coordinates,
                          &h_offsets,
                          &h_offset_trans,
                          &h_facecolors,
                          &h_antialiased,
                          &h_edgecolors)) {
        return -1;
    }

    if (!convert_gcagg(Cystck_AsPyObject(S, h_gc), &gc)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_master_transform), &master_transform)
                || !coordinates.converter(Cystck_AsPyObject(S, h_coordinates), &coordinates)
                || !convert_points(Cystck_AsPyObject(S, h_offsets), &offsets)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_offset_trans), &offset_trans)
                || !convert_colors(Cystck_AsPyObject(S, h_facecolors), &facecolors)
                || !convert_bool(Cystck_AsPyObject(S, h_antialiased), &antialiased)
                || !convert_colors(Cystck_AsPyObject(S, h_edgecolors), &edgecolors)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "draw_quad_mesh"); // TODO
        return -1;
    }

    CALL_CPP_CYSTCK(S,"draw_quad_mesh",
             (self->x->draw_quad_mesh(gc,
                                      master_transform,
                                      mesh_width,
                                      mesh_height,
                                      coordinates,
                                      offsets,
                                      offset_trans,
                                      facecolors,
                                      antialiased,
                                      edgecolors)));
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

    return 1;
}

static Cystck_Object
PyRendererAgg_draw_gouraud_triangle(Py_State *S, Cystck_Object args)
{
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    Cystck_Object h_gc = 0;
    Cystck_Object h_points = 0;
    Cystck_Object h_colors = 0;
    Cystck_Object h_trans = 0;
    GCAgg gc;
    numpy::array_view<const double, 2> points;
    numpy::array_view<const double, 2> colors;
    agg::trans_affine trans;

    if (!CystckArg_parseTuple(S, args,
                          "OOOO|O:draw_gouraud_triangle",
                          &h_gc,
                          &h_points,
                          &h_colors,
                          &h_trans)) {
        return -1;
    }

    if (!convert_gcagg( Cystck_AsPyObject(S, h_gc), &gc)
                || !points.converter(Cystck_AsPyObject(S, h_points), &points)
                || !colors.converter(Cystck_AsPyObject(S, h_colors), &colors)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "draw_gouraud_triangle"); // TODO
        return -1;
    }

    if (points.dim(0) != 3 || points.dim(1) != 2) {
        CystckErr_SetString(S, S->Cystck_ValueError,
                     "points must be a 3x2 array");
        return -1;
    }

    if (colors.dim(0) != 3 || colors.dim(1) != 4) {
        CystckErr_SetString(S, S->Cystck_ValueError,
                     "points must be a 3x2 array");
        return -1;
    }


    CALL_CPP_CYSTCK(S,"draw_gouraud_triangle", (self->x->draw_gouraud_triangle(gc, points, colors, trans)));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

    return 1;
}

static Cystck_Object
PyRendererAgg_draw_gouraud_triangles(Py_State *S, Cystck_Object args)
{
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    GCAgg gc;
    numpy::array_view<const double, 3> points;
    numpy::array_view<const double, 3> colors;
    agg::trans_affine trans;
    Cystck_Object h_gc = 0;
    Cystck_Object h_points = 0;
    Cystck_Object h_colors = 0;
    Cystck_Object h_trans = 0;

    if (!CystckArg_parseTuple(S, args,
                          "OOOO|O:draw_gouraud_triangles",
                          &h_gc,
                          &h_points,
                          &h_colors,
                          &h_trans)) {
        return -1;
    }

    if (!convert_gcagg(Cystck_AsPyObject(S, h_gc), &gc)
                || !points.converter(Cystck_AsPyObject(S, h_points), &points)
                || !colors.converter(Cystck_AsPyObject(S, h_colors), &colors)
                || !convert_trans_affine( Cystck_AsPyObject(S, h_trans), &trans)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "draw_gouraud_triangles"); // TODO
        return -1;
    }

    if (points.size() != 0 && (points.dim(1) != 3 || points.dim(2) != 2)) {
        CystckErr_SetString(S, S->Cystck_ValueError,
                     "points must be a N3x2 array");
        return -1;
    }

    if (colors.size() != 0 && (colors.dim(1) != 3 || colors.dim(2) != 4)) {
                CystckErr_SetString(S, S->Cystck_ValueError,
                     "points must be a N3x2 array");
        return -1;
    }

    if (points.size() != colors.size()) {
        CystckErr_SetString(S, S->Cystck_ValueError,
                     "points must be a N3x2 array");
        return -1;
    }

    CALL_CPP_CYSTCK(S,"draw_gouraud_triangles", self->x->draw_gouraud_triangles(gc, points, colors, trans));

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));

    return 1;
}

int PyRendererAgg_get_buffer(Py_State *S, Cystck_Object h_self, Cystck_buffer *buf, int flags)
{
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, h_self);
    buf->obj = Cystck_Dup(S, h_self);;
    buf->buf = self->x->pixBuffer;
    buf->len = (Py_ssize_t)self->x->get_width() * (Cystck_ssize_t)self->x->get_height() * 4;
    buf->readonly = 0;
    buf->format = (char *)"B";
    buf->ndim = 3;
    self->shape[0] = self->x->get_height();
    self->shape[1] = self->x->get_width();
    self->shape[2] = 4;
    buf->shape = self->shape;
    self->strides[0] = self->x->get_width() * 4;
    self->strides[1] = 4;
    self->strides[2] = 1;
    buf->strides = self->strides;
    buf->suboffsets = NULL;
    buf->itemsize = 1;
    buf->internal = NULL;

    return 1;
}

static Cystck_Object PyRendererAgg_clear(Py_State *S)
{
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    CALL_CPP_CYSTCK(S,"clear", self->x->clear());
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;
}

static Cystck_Object PyRendererAgg_copy_from_bbox(Py_State *S, Cystck_Object args)
{
    PyRendererAgg *self = PyRendererAgg_AsStruct(S, S->self);
    agg::rect_d bbox;
    BufferRegion *reg;
    int h_regobj;
    Cystck_Object m;
    Cystck_Object h_bbox = 0;

    if (!CystckArg_parseTuple(S, args, "O0:copy_from_bbox",  &h_bbox, &m)) {
        return -1;
    }
    if (!convert_rect(Cystck_AsPyObject(S, h_bbox), &bbox)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "copy_from_bbox"); // TODO
        return -1;
    }
    CALL_CPP_CYSTCK(S,"copy_from_bbox", (reg = self->x->copy_from_bbox(bbox)));

    Cystck_Object h_PyBufferRegionType = Cystck_GetAttr_s(S, m, "BufferRegion");
    Cystck_pushobject(S, h_PyBufferRegionType);
    h_regobj = PyBufferRegion_new(S, h_PyBufferRegionType, 0, 0);
    Cystck_pop(S, h_PyBufferRegionType);
    PyBufferRegion* regobj = PyBufferRegion_AsStruct(S, Cystck_FromPyObject(S, GetResults(S, h_regobj)));
    regobj->x = reg;

    return 1;
}

static Cystck_Object PyRendererAgg_restore_region(Py_State *S, Cystck_Object args)
{
    Cystck_Object h_regobj;
    Cystck_Object m;
    int xx1 = 0, yy1 = 0, xx2 = 0, yy2 = 0, x = 0, y = 0;

    if (!CystckArg_parseTuple(S, args,
                          "OO|iiiiii:restore_region",
                          &h_regobj,
                          &m,
                          &xx1,
                          &yy1,
                          &xx2,
                          &yy2,
                          &x,
                          &y)) {
        return -1;
    }
    Cystck_Object h_PyBufferRegionType = Cystck_GetAttr_s(S, m, "BufferRegion");
    Cystck_pushobject(S, h_PyBufferRegionType);
    if (!CystckTypeCheck(S, h_regobj, h_PyBufferRegionType)) {
        Cystck_pop(S, h_PyBufferRegionType);
        CystckErr_SetString(S, S->Cystck_TypeError, "arg must be BufferRegion"); // TODO
        return -1;
    }
    Cystck_pop(S, h_PyBufferRegionType);
    PyRendererAgg* self = PyRendererAgg_AsStruct(S, S->self);
    PyBufferRegion* regobj = PyBufferRegion_AsStruct(S, h_regobj);
    if (Cystck_Length(S, args) == 1) {
        CALL_CPP_CYSTCK(S,"restore_region", self->x->restore_region(*(regobj->x)));
    } else {
        CALL_CPP_CYSTCK(S,"restore_region", self->x->restore_region(*(regobj->x), xx1, yy1, xx2, yy2, x, y));
    }

    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;
}


CystckDef_SLOT(PyRendererAgg_new_def, PyRendererAgg_new, Cystck_tp_new)
CystckDef_SLOT(PyRendererAgg_init_def, PyRendererAgg_init, Cystck_tp_init)
CystckDef_SLOT(PyRendererAgg_get_buffer_def, PyRendererAgg_get_buffer, Cystck_bf_getbuffer)
CystckDef_SLOT(PyRendererAgg_dealloc_def, PyRendererAgg_dealloc, Cystck_tp_dealloc)

CystckDef_METH(PyRendererAgg_draw_path_def, "draw_path", PyRendererAgg_draw_path, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyRendererAgg_draw_markers_def, "draw_markers", PyRendererAgg_draw_markers, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyRendererAgg_draw_text_image_def, "draw_text_image", PyRendererAgg_draw_text_image, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyRendererAgg_draw_image_def, "draw_image", PyRendererAgg_draw_image, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyRendererAgg_draw_path_collection_def, "draw_path_collection", PyRendererAgg_draw_path_collection, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyRendererAgg_draw_quad_mesh_def, "draw_quad_mesh", PyRendererAgg_draw_quad_mesh, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyRendererAgg_draw_gouraud_triangle_def, "draw_gouraud_triangle", PyRendererAgg_draw_gouraud_triangle, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyRendererAgg_draw_gouraud_triangles_def, "draw_gouraud_triangles", PyRendererAgg_draw_gouraud_triangles, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyRendererAgg_clear_def, "clear", PyRendererAgg_clear, Cystck_METH_NOARGS, NULL)
CystckDef_METH(PyRendererAgg_copy_from_bbox_def, "copy_from_bbox", PyRendererAgg_copy_from_bbox, Cystck_METH_VARARGS, NULL)
CystckDef_METH(PyRendererAgg_restore_region_def, "restore_region", PyRendererAgg_restore_region, Cystck_METH_VARARGS, NULL)



CystckDef *PyRendererAgg_defines[] = {
    // slots
    &PyRendererAgg_new_def,
    &PyRendererAgg_init_def,
    &PyRendererAgg_get_buffer_def,
    &PyRendererAgg_dealloc_def,
    
    // methods
    &PyRendererAgg_draw_path_def,
    &PyRendererAgg_draw_markers_def,
    &PyRendererAgg_draw_text_image_def,
    &PyRendererAgg_draw_image_def,
    &PyRendererAgg_draw_path_collection_def,
    &PyRendererAgg_draw_quad_mesh_def,
    &PyRendererAgg_draw_gouraud_triangle_def,
    &PyRendererAgg_draw_gouraud_triangles_def,
    &PyRendererAgg_clear_def,
    &PyRendererAgg_copy_from_bbox_def,
    &PyRendererAgg_restore_region_def,
    NULL
};

Cystck_Type_Spec PyRendererAgg_type_spec = {
    .name = "matplotlib.backends._backend_agg.RendererAgg",
    .basicsize = sizeof(PyRendererAgg),
    .flags = Cystck_TPFLAGS_DEFAULT | Cystck_TPFLAGS_BASETYPE,
    .m_methods = PyRendererAgg_defines
};
static  CyModuleDef moduledef = 
{ 
    .m_name = "_backend_agg",
    .m_doc = NULL,
    .m_size= 0,
 };
static int npy_import_array_cystck(Py_State *S) {
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

CyMODINIT_FUNC(_backend_agg)
CyInit__backend_agg(Py_State *S)
{
    if(!npy_import_array_cystck(S))
    {
        return 0;
    }
    Cystck_Object m = CystckModule_Create(S, &moduledef);
    if(Cystck_IsNULL(m))
    {
        return 0;
    }
    if (!CystckHelpers_AddType(S, m, "RendererAgg", &PyRendererAgg_type_spec))
    {
        Cystck_pop(S, m);
        return 0;
    }
    if (!CystckHelpers_AddType(S, m, "BufferRegion", &PyBufferRegion_type_spec))
    {
        Cystck_pop(S, m);
        return 0;
    }
    return m;
}

#pragma GCC visibility pop

#ifdef __cplusplus
}
#endif