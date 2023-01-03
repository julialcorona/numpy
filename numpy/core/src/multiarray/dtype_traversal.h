#ifndef NUMPY_CORE_SRC_MULTIARRAY_DTYPE_TRAVERSAL_H_
#define NUMPY_CORE_SRC_MULTIARRAY_DTYPE_TRAVERSAL_H_

#define NPY_NO_DEPRECATED_API NPY_API_VERSION
#define _MULTIARRAYMODULE
#define _UMATHMODULE

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "numpy/ndarraytypes.h"
#include "numpy/arrayobject.h"

#include "alloc.h"
#include "array_method.h"
#include "conversion_utils.h"

/*
 * A simplified loop, similar to a general strided-loop function.
 * Using a `void *reserved`, because I think we probably need to pass in
 * Intepreter state or similar in the future.  But I don't want to pass in
 * a full context (with pointers to dtypes, method, caller which all make
 * no sense for a simple function).
 *
 * We assume for now that this context can be just passed through in the
 * the future (for structured dtypes).
 */
typedef int (simple_loop_function)(
        void *traverse_context, PyArray_Descr *descr, char *data,
        npy_intp size, npy_intp stride, NpyAuxData *auxdata);


/* Simplified get_loop function specific to dtype traversal */
typedef int (get_simple_loop_function)(
        void *traverse_context, PyArray_Descr *descr,
        int aligned, npy_intp fixed_stride,
        simple_loop_function **out_loop, NpyAuxData **out_auxdata,
        NPY_ARRAYMETHOD_FLAGS *flags);


/* NumPy DType clear implementations */

NPY_NO_EXPORT int
npy_get_clear_object_strided_loop(
        void *NPY_UNUSED(traverse_context), int NPY_UNUSED(aligned),
        npy_intp NPY_UNUSED(fixed_stride),
        simple_loop_function **out_loop, NpyAuxData **out_transferdata,
        NPY_ARRAYMETHOD_FLAGS *flags);

NPY_NO_EXPORT int
npy_get_clear_void_and_legacy_user_dtype_loop(
        PyArrayMethod_Context *context, int aligned, npy_intp strides,
        simple_loop_function **out_loop, NpyAuxData **out_transferdata,
        NPY_ARRAYMETHOD_FLAGS *flags);


/* Helper to deal with calling or nesting simple strided loops */

typedef struct {
    simple_loop_function *func;
    NpyAuxData *auxdata;
    PyArray_Descr *descr;
} NPY_traverse_info;


static inline void
NPY_traverse_info_init(NPY_traverse_info *cast_info)
{
    cast_info->func = NULL;  /* mark as uninitialized. */
}


static inline void
NPY_traverse_info_xfree(NPY_traverse_info *traverse_info)
{
    if (traverse_info->func == NULL) {
        return;
    }
    traverse_info->func = NULL;
    NPY_AUXDATA_FREE(traverse_info->auxdata);
    Py_DECREF(traverse_info->descr);
}


static inline int
NPY_traverse_info_copy(
        NPY_traverse_info *traverse_info, NPY_traverse_info *original)
{
    traverse_info->auxdata = NULL;
    if (original->auxdata != NULL) {
        traverse_info->auxdata = NPY_AUXDATA_CLONE(original->auxdata);
        if (traverse_info->auxdata == NULL) {
            return -1;
        }
    }
    Py_INCREF(original->descr);
    traverse_info->descr = original->descr;
    traverse_info->func = original->func;

    return 0;
}


NPY_NO_EXPORT int
PyArray_GetClearFunction(
        int aligned, npy_intp stride, PyArray_Descr *dtype,
        NPY_traverse_info *clear_info, NPY_ARRAYMETHOD_FLAGS *flags);


#endif  /* NUMPY_CORE_SRC_MULTIARRAY_DTYPE_TRAVERSAL_H_ */