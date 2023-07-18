#include "mplutils.h"
#include "_image_resample.h"
#include "numpy_cpp.h"
#include "py_converters.h"
#include "../../include/Cystck.h"


/**********************************************************************
 * Free functions
 * */

const char* image_resample__doc__ =
"resample(input_array, output_array, matrix, interpolation=NEAREST, alpha=1.0, norm=False, radius=1)\n"
"--\n\n"

"Resample input_array, blending it in-place into output_array, using an\n"
"affine transformation.\n\n"

"Parameters\n"
"----------\n"
"input_array : 2-d or 3-d Numpy array of float, double or uint8\n"
"    If 2-d, the image is grayscale.  If 3-d, the image must be of size\n"
"    4 in the last dimension and represents RGBA data.\n\n"

"output_array : 2-d or 3-d Numpy array of float, double or uint8\n"
"    The dtype and number of dimensions must match `input_array`.\n\n"

"transform : matplotlib.transforms.Transform instance\n"
"    The transformation from the input array to the output\n"
"    array.\n\n"

"interpolation : int, optional\n"
"    The interpolation method.  Must be one of the following constants\n"
"    defined in this module:\n\n"

"      NEAREST (default), BILINEAR, BICUBIC, SPLINE16, SPLINE36,\n"
"      HANNING, HAMMING, HERMITE, KAISER, QUADRIC, CATROM, GAUSSIAN,\n"
"      BESSEL, MITCHELL, SINC, LANCZOS, BLACKMAN\n\n"

"resample : bool, optional\n"
"    When `True`, use a full resampling method.  When `False`, only\n"
"    resample when the output image is larger than the input image.\n\n"

"alpha : float, optional\n"
"    The level of transparency to apply.  1.0 is completely opaque.\n"
"    0.0 is completely transparent.\n\n"

"norm : bool, optional\n"
"    Whether to norm the interpolation function.  Default is `False`.\n\n"

"radius: float, optional\n"
"    The radius of the kernel, if method is SINC, LANCZOS or BLACKMAN.\n"
"    Default is 1.\n";

static Cystck_Object CystckCall(Py_State *S, Cystck_Object obj, const char *func_name, Cystck_Object argtuple, Cystck_Object kwds) {
    Cystck_Object func = Cystck_GetAttr_s(S, obj, func_name);
    if (Cystck_IsNULL(func)) {
        return 0;
    }
    Cystck_pushobject(S, func);
    if (!Cystck_Callable_Check(S, func)) {
        Cystck_pop(S, func);
        return 0;
    }
    Cystck_Object result = Cystck_CallTupleDict(S, func, argtuple, kwds);
    Cystck_pop(S, func);
    return result;
}


static PyArrayObject *
_get_transform_mesh( Py_State *S, Cystck_Object py_affine, npy_intp *dims)
{
    /* TODO: Could we get away with float, rather than double, arrays here? */

    /* Given a non-affine transform object, create a mesh that maps
    every pixel in the output image to the input image.  This is used
    as a lookup table during the actual resampling. */

    
    npy_intp out_dims[3];

    out_dims[0] = dims[0] * dims[1];
    out_dims[1] = 2;

    Cystck_Object py_inverse = CystckCall(S, py_affine, "inverted", 0, 0);
    if (Cystck_IsNULL(py_inverse)) {
        return NULL;
    }

    numpy::array_view<double, 2> input_mesh(out_dims);
    double *p = (double *)input_mesh.data();

    for (npy_intp y = 0; y < dims[0]; ++y) {
        for (npy_intp x = 0; x < dims[1]; ++x) {
            *p++ = (double)x;
            *p++ = (double)y;
        }
    }
    Cystck_Object h_val = Cystck_FromPyObject(S, input_mesh.pyobj_steal());
    Cystck_pushobject(S, h_val);
    Cystck_Object tuple_transform[] = {h_val};
    Cystck_Object argtuple_transform = CystckTuple_FromArray(S, tuple_transform, 1);
    Cystck_pushobject(S, argtuple_transform);
    Cystck_Object output_mesh = CystckCall(S, py_inverse, "transform", argtuple_transform, 0);
    Cystck_pop(S, h_val);
    Cystck_pop(S, argtuple_transform);
    Cystck_pushobject(S, output_mesh);

    if (Cystck_IsNULL(output_mesh) ) {
        return NULL;
    }

    PyArrayObject *output_mesh_array =
        (PyArrayObject *)PyArray_ContiguousFromAny(
            Cystck_AsPyObject(S, output_mesh), NPY_DOUBLE, 2, 2);

    Cystck_pop(S, output_mesh);

    if (output_mesh_array == NULL) {
        return NULL;
    }

    return output_mesh_array;
}


template<class T>
static void
resample(PyArrayObject* input, PyArrayObject* output, resample_params_t params)
{
    Py_BEGIN_ALLOW_THREADS
    resample(
        (T*)PyArray_DATA(input), PyArray_DIM(input, 1), PyArray_DIM(input, 0),
        (T*)PyArray_DATA(output), PyArray_DIM(output, 1), PyArray_DIM(output, 0),
        params);
    Py_END_ALLOW_THREADS
}


static Cystck_Object
image_resample(Py_State *S, Cystck_Object args, Cystck_Object kwargs)
{
    Cystck_Object py_input_array = 0;
    Cystck_Object py_output_array = 0;
    Cystck_Object py_transform = 0;
    resample_params_t params;

    PyArrayObject *input_array = NULL;
    PyArrayObject *output_array = NULL;
    PyArrayObject *transform_mesh_array = NULL;
    
    Cystck_Object h_resample = 0;
    Cystck_Object h_norm = 0;
    params.interpolation = NEAREST;
    params.transform_mesh = NULL;
    params.resample = false;
    params.norm = false;
    params.radius = 1.0;
    params.alpha = 1.0;

    const char *kwlist[] = {
        "input_array", "output_array", "transform", "interpolation",
        "resample", "alpha", "norm", "radius", NULL };

    if (!CystckArg_parseTupleAndKeywords(S,
            args, kwargs, "OOO|iOdOd:resample", kwlist,
            &py_input_array, 
            &py_output_array, 
            &py_transform,
            &params.interpolation, 
            &h_resample, 
            &params.resample,
            &params.alpha, 
            &h_norm,  
            &params.radius)) {
        return -1;
    }
    if ( (!Cystck_IsNULL(h_resample) && !convert_bool(Cystck_AsPyObject(S, h_resample), &params.resample) ) || 
            (!Cystck_IsNULL(h_norm) && !convert_bool(Cystck_AsPyObject(S, h_norm), &params.norm))){
                goto error;
            }
    if (params.interpolation < 0 || params.interpolation >= _n_interpolation) {
        CystckErr_SetString(S, S->Cystck_ValueError, "invalid interpolation value");
        goto error;
    }

    input_array = (PyArrayObject *)PyArray_FromAny(
        Cystck_AsPyObject(S, py_input_array), NULL, 2, 3, NPY_ARRAY_C_CONTIGUOUS, NULL);
    if (input_array == NULL) {
        goto error;
    }

    output_array = (PyArrayObject *)Cystck_AsPyObject(S, py_output_array);
    if (!PyArray_Check(output_array)) {
        CystckErr_SetString(S, S->Cystck_ValueError, "output array must be a NumPy array");
        goto error;
    }
    
    if (!PyArray_IS_C_CONTIGUOUS(output_array)) {
        CystckErr_SetString(S, S->Cystck_ValueError, "output array must be C-contiguous");
        goto error;
    }
    if (PyArray_NDIM(output_array) < 2 || PyArray_NDIM(output_array) > 3) {
        CystckErr_SetString(S, S->Cystck_ValueError,
                        "output array must be 2- or 3-dimensional");
        goto error;
    }

    if ( Cystck_IsNULL(py_transform)  ||  Cystck_Is(S, py_transform, S->Cystck_None)) {
        params.is_affine = true;
    } else {
        Cystck_Object py_is_affine;
        int py_is_affine2;
        py_is_affine = Cystck_GetAttr_s(S, py_transform, "is_affine");
        if (Cystck_IsNULL(py_is_affine)) {
            goto error;
        }
        Cystck_pushobject(S, py_is_affine);
        py_is_affine2 = Cystck_IsTrue(S, py_is_affine);
        Cystck_pop(S, py_is_affine);

        if (py_is_affine2 == -1) {
            goto error;
        } else if (py_is_affine2) {
            if (!convert_trans_affine(Cystck_AsPyObject(S, py_transform), &params.affine)) {
                goto error;
            }
            params.is_affine = true;
        } else {
            transform_mesh_array = _get_transform_mesh(S, 
                py_transform, PyArray_DIMS(output_array));
            if (transform_mesh_array == NULL) {
                goto error;
            }
            params.transform_mesh = (double *)PyArray_DATA(transform_mesh_array);
            params.is_affine = false;
        }
    }

    if (PyArray_NDIM(input_array) != PyArray_NDIM(output_array)) {
        CystckErr_SetString(S, S->Cystck_ValueError,
            "Mismatched number of dimensions. ");
        goto error;
    }

    if (PyArray_TYPE(input_array) != PyArray_TYPE(output_array)) {
        CystckErr_SetString(S, S->Cystck_ValueError, "Mismatched types");
        goto error;
    }

    if (PyArray_NDIM(input_array) == 3) {
        if (PyArray_DIM(output_array, 2) != 4) {
            CystckErr_SetString(S, S->Cystck_ValueError,
                "Output array must be RGBA");
            goto error;
        }

        if (PyArray_DIM(input_array, 2) == 4) {
            switch (PyArray_TYPE(input_array)) {
            case NPY_UINT8:
            case NPY_INT8:
                resample<agg::rgba8>(input_array, output_array, params);
                break;
            case NPY_UINT16:
            case NPY_INT16:
                resample<agg::rgba16>(input_array, output_array, params);
                break;
            case NPY_FLOAT32:
                resample<agg::rgba32>(input_array, output_array, params);
                break;
            case NPY_FLOAT64:
                resample<agg::rgba64>(input_array, output_array, params);
                break;
            default:
                CystckErr_SetString(S, S->Cystck_ValueError,
                    "3-dimensional arrays must be of dtype unsigned byte, "
                    "unsigned short, float32 or float64");
                goto error;
            }
        } else {
            CystckErr_SetString(S, S->Cystck_ValueError,
                "If 3-dimensional, array must be RGBA");
            goto error;
        }
    } else { // NDIM == 2
        switch (PyArray_TYPE(input_array)) {
        case NPY_DOUBLE:
            resample<double>(input_array, output_array, params);
            break;
        case NPY_FLOAT:
            resample<float>(input_array, output_array, params);
            break;
        case NPY_UINT8:
        case NPY_INT8:
            resample<unsigned char>(input_array, output_array, params);
            break;
        case NPY_UINT16:
        case NPY_INT16:
            resample<unsigned short>(input_array, output_array, params);
            break;
        default:
            CystckErr_SetString(S, S->Cystck_ValueError, "Unsupported dtype");
            goto error;
        }
    }

    Py_DECREF(input_array);
    Py_XDECREF(transform_mesh_array);
    Cystck_pushobject(S, Cystck_Dup(S, S->Cystck_None));
    return 1;

 error:
    Py_XDECREF(input_array);
    Py_XDECREF(transform_mesh_array);
    return -1;
}

CystckDef_METH(image_resample_def, "resample", image_resample, Cystck_METH_KEYWORDS,image_resample__doc__)
static CystckDef *module_defines[] = {
    &image_resample_def,
    NULL
};
static CyModuleDef moduledef = {
    .m_name = "_image",
    .m_doc = NULL,
    .m_size = 0,
    .m_methods = module_defines,
};

int add_dict_int(Py_State *S, Cystck_Object dict, const char *key, long val)
{
    Cystck_Object valobj = CystckLong_FromLong(S, val);
    Cystck_pushobject(S, valobj);
    if (Cystck_IsNULL(valobj)) {
        return 1;
    }

    if (Cystck_SetAttr_s(S, dict, key, valobj)) {
        Cystck_pop(S, valobj);
        return 1;
    }

    Cystck_pop(S, valobj);
    return 0;
}


// Logic is from NumPy's import_array()
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

CyMODINIT_FUNC(_image)
CyInit__image(Py_State *S)
{
    if (!npy_import_array_cystck(S)) {
        return 0;
    }

    Cystck_Object m = CystckModule_Create(S, &moduledef);
    if (Cystck_IsNULL(m)) {
        return 0;
    }

    if (add_dict_int(S, m, "NEAREST", NEAREST) ||
        add_dict_int(S, m, "BILINEAR", BILINEAR) ||
        add_dict_int(S, m, "BICUBIC", BICUBIC) ||
        add_dict_int(S, m, "SPLINE16", SPLINE16) ||
        add_dict_int(S, m, "SPLINE36", SPLINE36) ||
        add_dict_int(S, m, "HANNING", HANNING) ||
        add_dict_int(S, m, "HAMMING", HAMMING) ||
        add_dict_int(S, m, "HERMITE", HERMITE) ||
        add_dict_int(S, m, "KAISER", KAISER) ||
        add_dict_int(S, m, "QUADRIC", QUADRIC) ||
        add_dict_int(S, m, "CATROM", CATROM) ||
        add_dict_int(S, m, "GAUSSIAN", GAUSSIAN) ||
        add_dict_int(S, m, "BESSEL", BESSEL) ||
        add_dict_int(S, m, "MITCHELL", MITCHELL) ||
        add_dict_int(S, m, "SINC", SINC) ||
        add_dict_int(S, m, "LANCZOS", LANCZOS) ||
        add_dict_int(S, m, "BLACKMAN", BLACKMAN) ||
        add_dict_int(S, m, "_n_interpolation", _n_interpolation)) {
        Cystck_pop(S, m);
        return 0;
    }

    return m;
}

#pragma GCC visibility pop

#ifdef __cplusplus
}
#endif