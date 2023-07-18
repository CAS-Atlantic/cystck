/* -*- mode: c++; c-basic-offset: 4 -*- */

#ifndef MPL_PY_CONVERTERS_H
#define MPL_PY_CONVERTERS_H

/***************************************************************************
 * This module contains a number of conversion functions from Python types
 * to C++ types.  Most of them meet the Python "converter" signature:
 *
 *    typedef int (*converter)(PyObject *, void *);
 *
 * and thus can be passed as conversion functions to PyArg_ParseTuple
 * and friends.
 */

#include "_backend_agg_basic_types.h"

extern "C" {
#ifdef HPY

#include "../../include/Cystck.h"
#include "cystck_helper.h"

typedef int (*converter_cystck)(Py_State *, Cystck_Object, void *);

int convert_from_attr_cystck(Py_State *S, Cystck_Object obj, const char *name, converter_cystck func, void *p);
int convert_from_method_cystck(Py_State *S, Cystck_Object obj, const char *name, converter_cystck func, void *p);

int convert_double_cystck(Py_State *S, Cystck_Object obj, void *p);
int convert_bool_cystck(Py_State *S, Cystck_Object obj, void *p);
int convert_cap_cystck(Py_State *S, Cystck_Object capobj, void *capp);
int convert_join_cystck(Py_State *S, Cystck_Object joinobj, void *joinp);
int convert_rect_cystck(Py_State *S, Cystck_Object rectobj, void *rectp);
int convert_rgba_cystck(Py_State *S, Cystck_Object rgbaocj, void *rgbap);
int convert_dashes_cystck(Py_State *S, Cystck_Object dashobj, void *gcp);
int convert_dashes_vector_cystck(Py_State *S, Cystck_Object obj, void *dashesp);
int convert_trans_affine_cystck(Py_State *S, Cystck_Object obj, void *transp);
int convert_path_cystck(Py_State *S, Cystck_Object obj, void *pathp);
int convert_clippath_cystck(Py_State *S, Cystck_Object clippath_tuple, void *clippathp);
int convert_snap_cystck(Py_State *S, Cystck_Object obj, void *snapp);
int convert_offset_position_cystck(Py_State *S, Cystck_Object obj, void *offsetp);
int convert_sketch_params_cystck(Py_State *S, Cystck_Object obj, void *sketchp);
int convert_gcagg_cystck(Py_State *S, Cystck_Object pygc, void *gcp);
int convert_points_cystck(Py_State *S, Cystck_Object pygc, void *pointsp);
int convert_transforms_cystck(Py_State *S, Cystck_Object pygc, void *transp);
int convert_bboxes_cystck(Py_State *S, Cystck_Object pygc, void *bboxp);
int convert_colors_cystck(Py_State *S, Cystck_Object pygc, void *colorsp);

int convert_face_cystck(Py_State *S, Cystck_Object color, GCAgg &gc, agg::rgba *rgba);

#else

#include <Python.h>

typedef int (*converter)(PyObject *, void *);

int convert_from_attr(PyObject *obj, const char *name, converter func, void *p);
int convert_from_method(PyObject *obj, const char *name, converter func, void *p);

int convert_double(PyObject *obj, void *p);
int convert_bool(PyObject *obj, void *p);
int convert_cap(PyObject *capobj, void *capp);
int convert_join(PyObject *joinobj, void *joinp);
int convert_rect(PyObject *rectobj, void *rectp);
int convert_rgba(PyObject *rgbaocj, void *rgbap);
int convert_dashes(PyObject *dashobj, void *gcp);
int convert_dashes_vector(PyObject *obj, void *dashesp);
int convert_trans_affine(PyObject *obj, void *transp);
int convert_path(PyObject *obj, void *pathp);
int convert_pathgen(PyObject *obj, void *pathgenp);
int convert_clippath(PyObject *clippath_tuple, void *clippathp);
int convert_snap(PyObject *obj, void *snapp);
int convert_sketch_params(PyObject *obj, void *sketchp);
int convert_gcagg(PyObject *pygc, void *gcp);
int convert_points(PyObject *pygc, void *pointsp);
int convert_transforms(PyObject *pygc, void *transp);
int convert_bboxes(PyObject *pygc, void *bboxp);
int convert_colors(PyObject *pygc, void *colorsp);

int convert_face(PyObject *color, GCAgg &gc, agg::rgba *rgba);

#endif
}

#endif