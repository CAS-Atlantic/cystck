#ifndef MPL_CYSTCK_HELPERS_H
#define MPL_CYSTCK_HELPERS_H

/*
 * This header contains helper macros for cystck
 */


#define Arg_ParseTuple(ret, S, tuple, fmt, ...)                                   \
    ret = CystckArg_parseTuple(S, tuple, fmt, ##__VA_ARGS__);


#define Arg_ParseTupleAndClose(ret, S, tuple, fmt, ...)   \
    Arg_ParseTuple(ret, S, tuple, fmt, ##__VA_ARGS__)     \
    Cystck_DECREF(S, tuple);

#endif
