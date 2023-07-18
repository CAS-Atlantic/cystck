#include "../../include/Cystck.h"
#include "numpy_cpp.h"

#include "_path.h"

#include "py_converters.h"
#include "py_adaptors.h"

Cystck_Object convert_polygon_vector( Py_State *S,  std::vector<Polygon> &polygons)
{
    CystckListBuilder pyresult = CystckList_New(S, polygons.size());

    for (size_t i = 0; i < polygons.size(); ++i) {
        Polygon poly = polygons[i];
        npy_intp dims[2];
        dims[1] = 2;

        dims[0] = (npy_intp)poly.size();

        numpy::array_view<double, 2> subresult(dims);
        memcpy(subresult.data(), &poly[0], sizeof(double) * poly.size() * 2);

        CystckListBuilder_Set(S, pyresult, i, Cystck_FromPyObject(S, subresult.pyobj()));
    }

    return CystckListBuilder_Build(S, pyresult);
}

const char *Py_point_in_path__doc__ =
    "point_in_path(x, y, radius, path, trans)\n"
    "--\n\n";

static Cystck_Object Py_point_in_path(Py_State *S,  Cystck_Object args)
{
    double x, y, r;
    py::PathIterator path;
    agg::trans_affine trans;
    bool result;
    Cystck_Object h_path = 0, h_trans = 0;

    if (!CystckArg_parseTuple(S, args,
                          "dddOO:point_in_path",
                          &x,
                          &y,
                          &r,
                          &h_path,
                          &h_trans)) {
        return -1;
    }
    
    if(!convert_path(Cystck_AsPyObject(S, h_path), &path) 
                        || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)){
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "point_in_path"); // TODO
            return -1;
        }

    CALL_CPP_CYSTCK(S,"point_in_path", (result = point_in_path(x, y, r, path, trans)));

    if (result) {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
        return 1;
    } else {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_False));
        return 1;
    }
}

const char *Py_points_in_path__doc__ =
    "points_in_path(points, radius, path, trans)\n"
    "--\n\n";

static Cystck_Object Py_points_in_path( Py_State *S, Cystck_Object args)
{
    numpy::array_view<const double, 2> points;
    double r;
    py::PathIterator path;
    agg::trans_affine trans;
    Cystck_Object h_points = 0;
    Cystck_Object h_path = 0;
    Cystck_Object h_trans = 0;
    if (!CystckArg_parseTuple(S, args,
                          "OdOO:points_in_path",
                          &h_points,
                          &r,
                          &h_path,
                          &h_trans)) {
        return -1;
    }
    if (!convert_points(Cystck_AsPyObject(S, h_points), &points)
                || !convert_path(Cystck_AsPyObject(S, h_path), &path)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "points_in_path"); // TODO
        return-1;
    }
    npy_intp dims[] = { (npy_intp)points.size() };
    numpy::array_view<uint8_t, 1> results(dims);

    CALL_CPP_CYSTCK(S,"points_in_path", (points_in_path(points, r, path, trans, results)));

    Cystck_pushobject(S, Cystck_FromPyObject(S, results.pyobj()));
    return 1;
}

const char *Py_point_on_path__doc__ =
    "point_on_path(x, y, radius, path, trans)\n"
    "--\n\n";

static Cystck_Object Py_point_on_path(Py_State *S, Cystck_Object args)
{
    double x, y, r;
    py::PathIterator path;
    agg::trans_affine trans;
    bool result;
    Cystck_Object h_path = 0;
    Cystck_Object h_trans = 0;

    if (!CystckArg_parseTuple(S, args,
                          "dddOO:point_on_path",
                          &x,
                          &y,
                          &r,
                          &h_path,
                          &h_trans)) {
        return -1;
    }

    if (!convert_path(Cystck_AsPyObject(S, h_path), &path)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "point_on_path"); // TODO
        return -1;
    }
    CALL_CPP_CYSTCK(S,"point_on_path", (result = point_on_path(x, y, r, path, trans)));

    if (result) {

        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
        return 1;
    } else {

        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_False));
        return 1;
    }
}

const char *Py_points_on_path__doc__ =
    "points_on_path(points, radius, path, trans)\n"
    "--\n\n";

static Cystck_Object Py_points_on_path(Py_State *S, Cystck_Object args)
{
    numpy::array_view<const double, 2> points;
    double r;
    py::PathIterator path;
    agg::trans_affine trans;
    Cystck_Object h_points = 0;
    Cystck_Object h_path = 0;
    Cystck_Object h_trans = 0;

    if (!CystckArg_parseTuple(S, args,
                          "OdOO:points_on_path",
                          &h_points,
                          &r,
                          &h_path,
                          &h_trans)) {
        return -1;
    }

    if (!convert_points(Cystck_AsPyObject(S, h_points), &points)
                || !convert_path(Cystck_AsPyObject(S, h_path), &path)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "points_on_path"); // TODO
        return -1;
    }

    npy_intp dims[] = { (npy_intp)points.size() };
    numpy::array_view<uint8_t, 1> results(dims);

    CALL_CPP_CYSTCK(S,"points_on_path", (points_on_path(points, r, path, trans, results)));

    Cystck_pushobject(S, Cystck_FromPyObject(S, results.pyobj()));
    return 1;
}

const char *Py_get_path_extents__doc__ =
    "get_path_extents(path, trans)\n"
    "--\n\n";

static Cystck_Object Py_get_path_extents(Py_State *S, Cystck_Object args)
{
    py::PathIterator path;
    agg::trans_affine trans;
    Cystck_Object h_path = 0;
    Cystck_Object h_trans = 0;

    if (!CystckArg_parseTuple(S, 
             args, "OO:get_path_extents", &h_path, &h_trans)) {
        return -1;
    }

    if (!convert_path(Cystck_AsPyObject(S, h_path), &path)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "get_path_extents"); // TODO
        return -1;
    }
    extent_limits e;

    CALL_CPP_CYSTCK(S,"get_path_extents", (reset_limits(e)));
    CALL_CPP_CYSTCK(S,"get_path_extents", (update_path_extents(path, trans, e)));

    npy_intp dims[] = { 2, 2 };
    numpy::array_view<double, 2> extents(dims);
    extents(0, 0) = e.x0;
    extents(0, 1) = e.y0;
    extents(1, 0) = e.x1;
    extents(1, 1) = e.y1;

    Cystck_pushobject(S, Cystck_FromPyObject(S, extents.pyobj()));
    return 1;
}

const char *Py_update_path_extents__doc__ =
    "update_path_extents(path, trans, rect, minpos, ignore)\n"
    "--\n\n";

static Cystck_Object Py_update_path_extents(Py_State *S, Cystck_Object args)
{
    py::PathIterator path;
    agg::trans_affine trans;
    agg::rect_d rect;
    numpy::array_view<double, 1> minpos;
    int ignore;
    int changed;
    Cystck_Object h_path = 0;
    Cystck_Object h_trans = 0;
    Cystck_Object h_rect = 0;
    Cystck_Object h_minpos = 0;

    if (!CystckArg_parseTuple(S, args,
                          "OOOOi:update_path_extents",
                          &h_path,
                          &h_trans,
                          &h_rect,
                          &h_minpos,
                          &ignore)) {
        return -1;
    }

    if (!convert_path(Cystck_AsPyObject(S, h_path), &path)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)
                || !convert_rect(Cystck_AsPyObject(S, h_rect), &rect)
                || !minpos.converter(Cystck_AsPyObject(S, h_minpos), &minpos)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "update_path_extents"); // TODO
        return -1;
    }


    if (minpos.dim(0) != 2) {
        CystckErr_SetString(S, S->Cystck_ValueError,
                     "minpos must be of length 2");
        return -1;
    }

    extent_limits e;

    if (ignore) {
        CALL_CPP_CYSTCK(S,"update_path_extents", reset_limits(e));
    } else {
        if (rect.x1 > rect.x2) {
            e.x0 = std::numeric_limits<double>::infinity();
            e.x1 = -std::numeric_limits<double>::infinity();
        } else {
            e.x0 = rect.x1;
            e.x1 = rect.x2;
        }
        if (rect.y1 > rect.y2) {
            e.y0 = std::numeric_limits<double>::infinity();
            e.y1 = -std::numeric_limits<double>::infinity();
        } else {
            e.y0 = rect.y1;
            e.y1 = rect.y2;
        }
        e.xm = minpos(0);
        e.ym = minpos(1);
    }

    CALL_CPP_CYSTCK(S,"update_path_extents", (update_path_extents(path, trans, e)));

    changed = (e.x0 != rect.x1 || e.y0 != rect.y1 || e.x1 != rect.x2 || e.y1 != rect.y2 ||
               e.xm != minpos(0) || e.ym != minpos(1));

    npy_intp extentsdims[] = { 2, 2 };
    numpy::array_view<double, 2> outextents(extentsdims);
    outextents(0, 0) = e.x0;
    outextents(0, 1) = e.y0;
    outextents(1, 0) = e.x1;
    outextents(1, 1) = e.y1;

    npy_intp minposdims[] = { 2 };
    numpy::array_view<double, 1> outminpos(minposdims);
    outminpos(0) = e.xm;
    outminpos(1) = e.ym;
    Cystck_BuildValue(S, "OOi", 
                                Cystck_FromPyObject(S, outextents.pyobj()), 
                                Cystck_FromPyObject(S, outminpos.pyobj()), changed);
    return 1;
}

const char *Py_get_path_collection_extents__doc__ =
    "get_path_collection_extents("
    "master_transform, paths, transforms, offsets, offset_transform)\n"
    "--\n\n";

static Cystck_Object Py_get_path_collection_extents(Py_State *S, Cystck_Object args)
{
    Cystck_Object h_master_transform = 0;
    Cystck_Object h_paths = 0;
    Cystck_Object h_transforms = 0;
    Cystck_Object h_offsets = 0;
    Cystck_Object h_offset_trans = 0;
    agg::trans_affine master_transform;
    py::PathGenerator paths;
    numpy::array_view<const double, 3> transforms;
    numpy::array_view<const double, 2> offsets;
    agg::trans_affine offset_trans;
    extent_limits e;

    if (!CystckArg_parseTuple(S, args,
                          "OOOOO:get_path_collection_extents",
                          &h_master_transform,
                          &h_paths,
                          &h_transforms,
                          &h_offsets,
                          &h_offset_trans)) {
        return -1;
    }

    if (!convert_trans_affine(Cystck_AsPyObject(S, h_master_transform), &master_transform)
                || !convert_pathgen(Cystck_AsPyObject(S,h_paths), &paths)
                || !convert_transforms(Cystck_AsPyObject(S,h_transforms), &transforms)
                || !convert_points(Cystck_AsPyObject(S, h_offsets), &offsets)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_offset_trans), &offset_trans)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "get_path_collection_extents"); // TODO
        return -1;
    }

    CALL_CPP_CYSTCK(S,"get_path_collection_extents",
             (get_path_collection_extents(
                 master_transform, paths, transforms, offsets, offset_trans, e)));

    npy_intp dims[] = { 2, 2 };
    numpy::array_view<double, 2> extents(dims);
    extents(0, 0) = e.x0;
    extents(0, 1) = e.y0;
    extents(1, 0) = e.x1;
    extents(1, 1) = e.y1;

    npy_intp minposdims[] = { 2 };
    numpy::array_view<double, 1> minpos(minposdims);
    minpos(0) = e.xm;
    minpos(1) = e.ym;

    Cystck_BuildValue(S, "OO", 
                                Cystck_FromPyObject(S, extents.pyobj()), 
                                Cystck_FromPyObject(S, minpos.pyobj()));

    return 1;
}

const char *Py_point_in_path_collection__doc__ =
    "point_in_path_collection("
    "x, y, radius, master_transform, paths, transforms, offsets, "
    "offset_trans, filled)\n"
    "--\n\n";

static Cystck_Object Py_point_in_path_collection( Py_State *S, Cystck_Object args)
{
    Cystck_Object h_master_transform = 0;
    Cystck_Object h_paths = 0;
    Cystck_Object h_transforms = 0;
    Cystck_Object h_offsets = 0;
    Cystck_Object h_offset_trans = 0;
    Cystck_Object h_filled = 0;
    double x, y, radius;
    agg::trans_affine master_transform;
    py::PathGenerator paths;
    numpy::array_view<const double, 3> transforms;
    numpy::array_view<const double, 2> offsets;
    agg::trans_affine offset_trans;
    bool filled;
    std::vector<int> result;

    if (!CystckArg_parseTuple(S, args,
                          "dddOOOOOO:point_in_path_collection",
                          &x,
                          &y,
                          &radius,
                          &h_master_transform,
                          &h_paths,
                          &h_transforms,
                          &h_offsets,
                          &h_offset_trans,
                          &h_filled)) {
        return -1;
    }

    if (!convert_trans_affine(Cystck_AsPyObject(S, h_master_transform), &master_transform)
                || !convert_pathgen(Cystck_AsPyObject(S,h_paths), &paths)
                || !convert_transforms(Cystck_AsPyObject(S, h_transforms), &transforms)
                || !convert_points(Cystck_AsPyObject(S, h_offsets), &offsets)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_offset_trans), &offset_trans)
                || !convert_bool(Cystck_AsPyObject(S, h_filled), &filled)){
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "point_in_path_collection"); // TODO
        return -1;
    }
    
    CALL_CPP_CYSTCK(S,"point_in_path_collection",
             (point_in_path_collection(x,
                                       y,
                                       radius,
                                       master_transform,
                                       paths,
                                       transforms,
                                       offsets,
                                       offset_trans,
                                       filled,
                                       result)));

    npy_intp dims[] = {(npy_intp)result.size() };
    numpy::array_view<int, 1> pyresult(dims);
    if (result.size() > 0) {
        memcpy(pyresult.data(), &result[0], result.size() * sizeof(int));
    }
    Cystck_pushobject(S, Cystck_FromPyObject(S, pyresult.pyobj()));
    return 1;
}

const char *Py_path_in_path__doc__ =
    "path_in_path(path_a, trans_a, path_b, trans_b)\n"
    "--\n\n";

static Cystck_Object Py_path_in_path(Py_State *S, Cystck_Object args)
{
    Cystck_Object h_a = 0;
    Cystck_Object h_atrans = 0;
    Cystck_Object h_b = 0;
    Cystck_Object h_btrans = 0;
    py::PathIterator a;
    agg::trans_affine atrans;
    py::PathIterator b;
    agg::trans_affine btrans;
    bool result;

    if (!CystckArg_parseTuple(S, args,
                          "OOOO:path_in_path",
                          &h_a,
                          &h_atrans,
                          &h_b,
                          &h_btrans)) {
        return -1;
    }
    if (!convert_path(Cystck_AsPyObject(S,h_a), &a)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_atrans), &atrans)
                || !convert_path(Cystck_AsPyObject(S,h_b), &b)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_btrans), &btrans)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "path_in_path"); // TODO
        return -1;
    }
    CALL_CPP_CYSTCK(S,"path_in_path", (result = path_in_path(a, atrans, b, btrans)));

    if (result) {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
        return 1;
    } else {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_False));
        return 1;
    }
}

const char *Py_clip_path_to_rect__doc__ =
    "clip_path_to_rect(path, rect, inside)\n"
    "--\n\n";

static Cystck_Object Py_clip_path_to_rect(Py_State *S, Cystck_Object args)
{
    Cystck_Object h_path = 0;
    Cystck_Object h_rect = 0;
    Cystck_Object h_inside = 0;
    py::PathIterator path;
    agg::rect_d rect;
    bool inside;
    std::vector<Polygon> result;

    if (!CystckArg_parseTuple(S, args,
                          "OOO:clip_path_to_rect",
                          &h_path,
                          &h_rect,
                          &h_inside)) {
        return -1;
    }
    if (!convert_path(Cystck_AsPyObject(S, h_path), &path)
                || !convert_rect(Cystck_AsPyObject(S, h_rect), &rect)
                || !convert_bool(Cystck_AsPyObject(S, h_inside), &inside)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "clip_path_to_rect"); // TODO
        return -1;
    }
    CALL_CPP_CYSTCK(S,"clip_path_to_rect", (clip_path_to_rect(path, rect, inside, result)));
    Cystck_pushobject(S, convert_polygon_vector(S, result));
    return 1;
}

const char *Py_affine_transform__doc__ =
    "affine_transform(points, trans)\n"
    "--\n\n";

static Cystck_Object Py_affine_transform(Py_State *S, Cystck_Object args)
{
    Cystck_Object vertices_obj = 0;
    Cystck_Object h_trans = 0;
    agg::trans_affine trans;

    if (!CystckArg_parseTuple(S, args,
                          "OO:affine_transform",
                          &vertices_obj,
                          &h_trans)) {
        return -1;
    }
    if (!convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "affine_transform"); // TODO
        return -1;
    }
    PyArrayObject* vertices_arr = (PyArrayObject *)PyArray_ContiguousFromAny(Cystck_AsPyObject(S, vertices_obj), NPY_DOUBLE, 1, 2);
    if (vertices_arr == NULL) {
        return -1;
    }

    if (PyArray_NDIM(vertices_arr) == 2) {
        numpy::array_view<double, 2> vertices(vertices_arr);
        Py_DECREF(vertices_arr);

        npy_intp dims[] = { (npy_intp)vertices.size(), 2 };
        numpy::array_view<double, 2> result(dims);
        CALL_CPP_CYSTCK(S,"affine_transform", (affine_transform_2d(vertices, trans, result)));
        Cystck_pushobject(S, Cystck_FromPyObject(S, result.pyobj()));
        return 1;
    } else { // PyArray_NDIM(vertices_arr) == 1
        numpy::array_view<double, 1> vertices(vertices_arr);
        Py_DECREF(vertices_arr);

        npy_intp dims[] = { (npy_intp)vertices.size() };
        numpy::array_view<double, 1> result(dims);
        CALL_CPP_CYSTCK(S,"affine_transform", (affine_transform_1d(vertices, trans, result)));
        Cystck_pushobject(S, Cystck_FromPyObject(S, result.pyobj()));
        return 1;
    }
}

const char *Py_count_bboxes_overlapping_bbox__doc__ =
    "count_bboxes_overlapping_bbox(bbox, bboxes)\n"
    "--\n\n";

static Cystck_Object Py_count_bboxes_overlapping_bbox(Py_State *S, Cystck_Object args)
{
    Cystck_Object h_bbox = 0;
    Cystck_Object h_bboxes = 0;
    agg::rect_d bbox;
    numpy::array_view<const double, 3> bboxes;
    int result;

    if (!CystckArg_parseTuple(S, args,
                          "OO:count_bboxes_overlapping_bbox",
                          &h_bbox,
                          &h_bboxes)) {
        return -1;
    }
    if (!convert_rect(Cystck_AsPyObject(S, h_bbox), &bbox)
                || !convert_bboxes(Cystck_AsPyObject(S, h_bboxes), &bboxes)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "count_bboxes_overlapping_bbox"); // TODO
        return -1;
    }
    CALL_CPP_CYSTCK(S,"count_bboxes_overlapping_bbox",
             (result = count_bboxes_overlapping_bbox(bbox, bboxes)));
    Cystck_pushobject(S, CystckLong_FromLong(S, result));
    return 1;
}

const char *Py_path_intersects_path__doc__ =
    "path_intersects_path(path1, path2, filled=False)\n"
    "--\n\n";

static Cystck_Object Py_path_intersects_path(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    Cystck_Object h_p1 = 0;
    Cystck_Object h_p2 = 0;
    py::PathIterator p1;
    py::PathIterator p2;
    agg::trans_affine t1;
    agg::trans_affine t2;
    int filled = 0;
    const char *names[] = { "p1", "p2", "filled", NULL };
    bool result;

    if (!CystckArg_parseTupleAndKeywords(S, args,
                                     kwds,
                                     "OOi:path_intersects_path",
                                     names,
                                     &h_p1,
                                     &h_p2,
                                     &filled)) {
        return -1;
    }
    if (!convert_path(Cystck_AsPyObject(S, h_p1), &p1)
                || !convert_path(Cystck_AsPyObject(S, h_p2), &p2)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "path_intersects_path"); // TODO
        return -1;
    }
    CALL_CPP_CYSTCK(S,"path_intersects_path", (result = path_intersects_path(p1, p2)));
    if (filled) {
        if (!result) {
            CALL_CPP_CYSTCK(S,"path_intersects_path",
                     (result = path_in_path(p1, t1, p2, t2)));
        }
        if (!result) {
            CALL_CPP_CYSTCK(S,"path_intersects_path",
                     (result = path_in_path(p2, t1, p1, t2)));
        }
    }

    if (result) {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
        return 1;
    } else {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_False));
        return 1;
    }
}

const char *Py_path_intersects_rectangle__doc__ =
    "path_intersects_rectangle("
    "path, rect_x1, rect_y1, rect_x2, rect_y2, filled=False)\n"
    "--\n\n";

static Cystck_Object Py_path_intersects_rectangle(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    Cystck_Object h_path = 0;
    Cystck_Object h_filled = 0;
    py::PathIterator path;
    double rect_x1, rect_y1, rect_x2, rect_y2;
    bool filled = false;
    const char *names[] = { "path", "rect_x1", "rect_y1", "rect_x2", "rect_y2", "filled", NULL };
    bool result;

    if (!CystckArg_parseTupleAndKeywords(S, args,
                                     kwds,
                                     "Odddd|O:path_intersects_rectangle",
                                     names,
                                     &h_path,
                                     &rect_x1,
                                     &rect_y1,
                                     &rect_x2,
                                     &rect_y2,
                                     &h_filled)) {
        return -1;
    }
    if (!convert_path( Cystck_AsPyObject(S, h_path), &path)
                || (!Cystck_IsNULL(h_filled) && !convert_bool(Cystck_AsPyObject(S, h_filled), &filled))) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "path_intersects_rectangle"); // TODO
        return -1;
    }
    CALL_CPP_CYSTCK(S,"path_intersects_rectangle", (result = path_intersects_rectangle(path, rect_x1, rect_y1, rect_x2, rect_y2, filled)));

    if (result) {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
        return 1;
    } else {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_False));
        return 1;
    }
}

const char *Py_convert_path_to_polygons__doc__ =
    "convert_path_to_polygons(path, trans, width=0, height=0)\n"
    "--\n\n";

static Cystck_Object Py_convert_path_to_polygons(Py_State *S, Cystck_Object args, Cystck_Object kwds)
{
    Cystck_Object h_path = 0;
    Cystck_Object h_trans = 0;
    py::PathIterator path;
    agg::trans_affine trans;
    double width = 0.0, height = 0.0;
    int closed_only = 1;
    std::vector<Polygon> result;
    const char *names[] = { "path", "transform", "width", "height", "closed_only", NULL };

    if (!CystckArg_parseTupleAndKeywords(S, args,
                                     kwds,
                                     "OO|ddi:convert_path_to_polygons",
                                     names,
                                     &h_path,
                                     &h_trans,
                                     &width,
                                     &height,
                                     &closed_only)) {
        return -1;
    }

    if (!convert_path(Cystck_AsPyObject(S, h_path), &path)
                || !convert_trans_affine(Cystck_AsPyObject(S,h_trans), &trans)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "convert_path_to_polygons"); // TODO
        return -1;
    }

    CALL_CPP_CYSTCK(S,"convert_path_to_polygons",
             (convert_path_to_polygons(path, trans, width, height, closed_only, result)));
    Cystck_pushobject(S, convert_polygon_vector(S, result));
    return 1;
}

const char *Py_cleanup_path__doc__ =
    "cleanup_path("
    "path, trans, remove_nans, clip_rect, snap_mode, stroke_width, simplify, "
    "return_curves, sketch)\n"
    "--\n\n";

static Cystck_Object Py_cleanup_path(Py_State *S, Cystck_Object args)
{
    Cystck_Object h_path = 0;
    Cystck_Object h_trans = 0;
    Cystck_Object h_remove_nans = 0;
    Cystck_Object h_clip_rect = 0;
    Cystck_Object h_snap_mode = 0;
    Cystck_Object h_return_curves = 0;
    Cystck_Object h_sketch = 0;
    py::PathIterator path;
    agg::trans_affine trans;
    bool remove_nans;
    agg::rect_d clip_rect;
    e_snap_mode snap_mode;
    double stroke_width;
    Cystck_Object simplifyobj;
    bool simplify = false;
    bool return_curves;
    SketchParams sketch;

    if (!CystckArg_parseTuple(S, args,
                          "OOOOOdOOO:cleanup_path",
                          &h_path,
                          &h_trans,
                          &h_remove_nans,
                          &h_clip_rect,
                          &h_snap_mode,
                          &stroke_width,
                          &simplifyobj,
                          &h_return_curves,
                          &sketch)) {
        return -1;
    }
    if (!convert_path( Cystck_AsPyObject(S, h_path), &path)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)
                || !convert_bool( Cystck_AsPyObject(S, h_remove_nans), &remove_nans)
                || !convert_rect(Cystck_AsPyObject(S, h_clip_rect), &clip_rect)
                || !convert_snap(Cystck_AsPyObject(S, h_snap_mode), &snap_mode)
                || !convert_bool( Cystck_AsPyObject(S, h_return_curves), &return_curves)
                || !convert_sketch_params(Cystck_AsPyObject(S, h_sketch), &sketch)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "cleanup_path"); // TODO
        return -1;
    }

    if (Cystck_Is(S, simplifyobj, S->Cystck_None)) {
        simplify = path.should_simplify();
    } else {
        switch (Cystck_IsTrue(S, simplifyobj)) {
            case 0: simplify = false; break;
            case 1: simplify = true; break;
            default: return -1;  // errored.
        }
    }

    bool do_clip = (clip_rect.x1 < clip_rect.x2 && clip_rect.y1 < clip_rect.y2);

    std::vector<double> vertices;
    std::vector<npy_uint8> codes;

    CALL_CPP_CYSTCK(S,"cleanup_path",
             (cleanup_path(path,
                           trans,
                           remove_nans,
                           do_clip,
                           clip_rect,
                           snap_mode,
                           stroke_width,
                           simplify,
                           return_curves,
                           sketch,
                           vertices,
                           codes)));

    size_t length = codes.size();

    npy_intp vertices_dims[] = {(npy_intp)length, 2 };
    numpy::array_view<double, 2> pyvertices(vertices_dims);

    npy_intp codes_dims[] = {(npy_intp)length };
    numpy::array_view<unsigned char, 1> pycodes(codes_dims);

    memcpy(pyvertices.data(), &vertices[0], sizeof(double) * 2 * length);
    memcpy(pycodes.data(), &codes[0], sizeof(unsigned char) * length);

    Cystck_BuildValue(S, "OO", 
                            Cystck_FromPyObject(S, pyvertices.pyobj()), 
                            Cystck_FromPyObject(S, pycodes.pyobj()));

    return 1;
}

const char *Py_convert_to_string__doc__ =
    "convert_to_string("
    "path, trans, clip_rect, simplify, sketch, precision, codes, postfix)\n"
    "--\n\n"
    "Convert *path* to a bytestring.\n"
    "\n"
    "The first five parameters (up to *sketch*) are interpreted as in\n"
    "`.cleanup_path`.  The following ones are detailed below.\n"
    "\n"
    "Parameters\n"
    "----------\n"
    "path : Path\n"
    "trans : Transform or None\n"
    "clip_rect : sequence of 4 floats, or None\n"
    "simplify : bool\n"
    "sketch : tuple of 3 floats, or None\n"
    "precision : int\n"
    "    The precision used to \"%.*f\"-format the values.  Trailing zeros\n"
    "    and decimal points are always removed.  (precision=-1 is a special\n"
    "    case used to implement ttconv-back-compatible conversion.)\n"
    "codes : sequence of 5 bytestrings\n"
    "    The bytes representation of each opcode (MOVETO, LINETO, CURVE3,\n"
    "    CURVE4, CLOSEPOLY), in that order.  If the bytes for CURVE3 is\n"
    "    empty, quad segments are automatically converted to cubic ones\n"
    "    (this is used by backends such as pdf and ps, which do not support\n"
    "    quads).\n"
    "postfix : bool\n"
    "    Whether the opcode comes after the values (True) or before (False).\n"
    ;

static Cystck_Object Py_convert_to_string(Py_State *S, Cystck_Object args)
{
    Cystck_Object h_path = 0;
    Cystck_Object h_trans = 0;
    Cystck_Object h_cliprect = 0;
    Cystck_Object h_sketch = 0;
    Cystck_Object h_postfix = 0;
    Cystck_Object h_codes = 0;
    py::PathIterator path;
    agg::trans_affine trans;
    agg::rect_d cliprect;
    Cystck_Object simplifyobj;
    bool simplify = false;
    SketchParams sketch;
    int precision;
    char *codes[5];
    bool postfix;
    std::string buffer;
    bool status;

    if (!CystckArg_parseTuple(S, args,
                          "OOOOOiO0:convert_to_string",
                          &h_path,
                          &h_trans,
                          &h_cliprect,
                          &simplifyobj,
                          &h_sketch,
                          &precision,
                          &h_codes,
                          &h_postfix)) {
        return -1;
    }

    if (!convert_path(Cystck_AsPyObject(S, h_path), &path)
                || !convert_trans_affine(Cystck_AsPyObject(S, h_trans), &trans)
                || !convert_rect(Cystck_AsPyObject(S, h_cliprect), &cliprect)
                || !convert_sketch_params(Cystck_AsPyObject(S, h_sketch), &sketch)
                || !convert_bool(Cystck_AsPyObject(S, h_postfix), &postfix)) {
        if (!Cystck_Err_Occurred(S)) CystckErr_SetString(S, S->Cystck_SystemError, "convert_to_string"); // TODO
        return -1;
    }

    if (!CystckTuple_Check(S, h_codes) && !CystckList_Check(S, h_codes)) { // (yyyyy) not supported
        CystckErr_SetString(S, S->Cystck_TypeError, "convert_to_string");
        return -1;
    }

    Cystck_ssize_t codes_len = Cystck_Length(S, h_codes);
    if (codes_len != 5) {
        CystckErr_SetString(S, S->Cystck_ValueError, "convert_to_string");
        return -1;
    }

    for (Cystck_ssize_t i=0; i < codes_len; i++) {
        Cystck_Object item = Cystck_GetItem_i(S, h_codes, i);
        if (!CystckBytes_Check(S, item)) {
            CystckErr_SetString(S, S->Cystck_TypeError, "convert_to_string");
            return -1;
        }
        Cystck_pushobject(S, item);
        codes[i] = CystckBytes_AsString(S, item);
        Cystck_pop(S, item);
    }

    if (Cystck_Is(S, simplifyobj, S->Cystck_None)) {
        simplify = path.should_simplify();
    } else {
        switch (Cystck_IsTrue(S, simplifyobj)) {
            case 0: simplify = false; break;
            case 1: simplify = true; break;
            default: return -1;  // errored.
        }
    }

    CALL_CPP_CYSTCK(S,"convert_to_string",
             (status = convert_to_string(
                 path, trans, cliprect, simplify, sketch,
                 precision, codes, postfix, buffer)));

    if (!status) {
        CystckErr_SetString(S, S->Cystck_ValueError, "Malformed path codes");
        return -1;
    }

    Cystck_Object res = CystckBytes_FromStringAndSize(S, buffer.c_str(), buffer.size());
    Cystck_pushobject(S, res);
    return 1;
}


const char *Py_is_sorted__doc__ =
    "is_sorted(array)\n"
    "--\n\n"
    "Return whether the 1D *array* is monotonically increasing, ignoring NaNs.\n";

static Cystck_Object Py_is_sorted(Py_State *S, Cystck_Object obj)
{
    npy_intp size;
    bool result;

    PyArrayObject *array = (PyArrayObject *)PyArray_FromAny(
        Cystck_AsPyObject(S, obj), NULL, 1, 1, 0, NULL);

    if (array == NULL) {
        return -1;
    }

    size = PyArray_DIM(array, 0);

    if (size < 2) {
        Py_DECREF(array);
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
        return 1;
    }

    /* Handle just the most common types here, otherwise coerce to
    double */
    switch (PyArray_TYPE(array)) {
    case NPY_INT:
        result = is_sorted<npy_int>(array);
        break;
    case NPY_LONG:
        result = is_sorted<npy_long>(array);
        break;
    case NPY_LONGLONG:
        result = is_sorted<npy_longlong>(array);
        break;
    case NPY_FLOAT:
        result = is_sorted<npy_float>(array);
        break;
    case NPY_DOUBLE:
        result = is_sorted<npy_double>(array);
        break;
    default:
        Py_DECREF(array);
        array = (PyArrayObject *)PyArray_FromObject(Cystck_AsPyObject(S, obj), NPY_DOUBLE, 1, 1);
        if (array == NULL) {
            return -1;
        }
        result = is_sorted<npy_double>(array);
    }

    Py_DECREF(array);

    if (result) {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
        return 1;
    } else {
        Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_True));
        return 1;
    }
}



CystckDef_METH(Py_point_in_path_def, "point_in_path", Py_point_in_path, Cystck_METH_VARARGS, Py_point_in_path__doc__);
CystckDef_METH(Py_points_in_path_def, "points_in_path", Py_points_in_path, Cystck_METH_VARARGS,  Py_points_in_path__doc__);
CystckDef_METH(Py_point_on_path_def, "point_on_path", Py_point_on_path, Cystck_METH_VARARGS, Py_point_on_path__doc__);
CystckDef_METH(Py_points_on_path_def, "points_on_path", Py_points_on_path, Cystck_METH_VARARGS,  Py_points_on_path__doc__);
CystckDef_METH(Py_get_path_extents_def, "get_path_extents", Py_get_path_extents, Cystck_METH_VARARGS,  Py_get_path_extents__doc__);
CystckDef_METH(Py_update_path_extents_def, "update_path_extents", Py_update_path_extents, Cystck_METH_VARARGS, Py_update_path_extents__doc__);
CystckDef_METH(Py_get_path_collection_extents_def, "get_path_collection_extents", Py_get_path_collection_extents, Cystck_METH_VARARGS,  Py_get_path_collection_extents__doc__);
CystckDef_METH(Py_point_in_path_collection_def, "point_in_path_collection", Py_point_in_path_collection, Cystck_METH_VARARGS, Py_point_in_path_collection__doc__);
CystckDef_METH(Py_path_in_path_def, "path_in_path", Py_path_in_path, Cystck_METH_VARARGS,  Py_path_in_path__doc__);
CystckDef_METH(Py_clip_path_to_rect_def, "clip_path_to_rect", Py_clip_path_to_rect, Cystck_METH_VARARGS,  Py_clip_path_to_rect__doc__);
CystckDef_METH(Py_affine_transform_def, "affine_transform", Py_affine_transform, Cystck_METH_VARARGS, Py_affine_transform__doc__);
CystckDef_METH(Py_count_bboxes_overlapping_bbox_def, "count_bboxes_overlapping_bbox", Py_count_bboxes_overlapping_bbox, Cystck_METH_VARARGS,  Py_count_bboxes_overlapping_bbox__doc__);
CystckDef_METH(Py_path_intersects_path_def, "path_intersects_path", Py_path_intersects_path, Cystck_METH_KEYWORDS,  Py_path_intersects_path__doc__);
CystckDef_METH(Py_path_intersects_rectangle_def, "path_intersects_rectangle", Py_path_intersects_rectangle, Cystck_METH_KEYWORDS,  Py_path_intersects_rectangle__doc__);
CystckDef_METH(Py_convert_path_to_polygons_def, "convert_path_to_polygons", Py_convert_path_to_polygons, Cystck_METH_KEYWORDS,  Py_convert_path_to_polygons__doc__);
CystckDef_METH(Py_cleanup_path_def, "cleanup_path", Py_cleanup_path, Cystck_METH_VARARGS,   Py_cleanup_path__doc__);
CystckDef_METH(Py_convert_to_string_def, "convert_to_string", Py_convert_to_string, Cystck_METH_VARARGS,  Py_convert_to_string__doc__);
CystckDef_METH(Py_is_sorted_def, "is_sorted", Py_is_sorted, Cystck_METH_O,  Py_is_sorted__doc__);


static CystckDef *module_defines[] = {
    &Py_point_in_path_def,
    &Py_points_in_path_def,
    &Py_point_on_path_def,
    &Py_points_on_path_def,
    &Py_get_path_extents_def,
    &Py_update_path_extents_def,
    &Py_get_path_collection_extents_def,
    &Py_point_in_path_collection_def,
    &Py_path_in_path_def,
    &Py_clip_path_to_rect_def,
    &Py_affine_transform_def,
    &Py_count_bboxes_overlapping_bbox_def,
    &Py_path_intersects_path_def,
    &Py_path_intersects_rectangle_def,
    &Py_convert_path_to_polygons_def,
    &Py_cleanup_path_def,
    &Py_convert_to_string_def,
    &Py_is_sorted_def,
    NULL
};


static CyModuleDef moduledef = {
  .m_name = "_path",
  .m_doc = NULL,
  .m_size = 0,
  .m_methods = module_defines,
};

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
CyMODINIT_FUNC(_path)
CyInit__path(Py_State *S)
{
    printf("CYSTCK\n");
    if (!npy_import_array(S)) {
        return 0;
    }

    return CystckModule_Create(S, &moduledef);
}

#pragma GCC visibility pop
#ifdef __cplusplus
}
#endif
