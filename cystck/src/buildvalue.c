#include "../include/Cystck.h"

#define MESSAGE_BUF_SIZE 128

static Cystck_ssize_t count_items(Py_State *S, const char *fmt, char end);
static Cystck_Object build_tuple(Py_State *S, const char **fmt, va_list *values, Cystck_ssize_t size, char expected_end);
static Cystck_Object build_list(Py_State *S, const char **fmt, va_list *values, Cystck_ssize_t size);
static Cystck_Object build_dict(Py_State *S, const char **fmt, va_list *values);
static Cystck_Object build_single(Py_State *S, const char **fmt, va_list *values, int *needs_close);

Cystck_Object Cystck_BuildValue(Py_State *S, const char *fmt, ...)
{
    va_list values;
    Cystck_Object result;
    va_start(values, fmt);
    Cystck_ssize_t size = count_items(S, fmt, '\0');
    if (size < 0) {
        result = 0;
    } else if (size == 0) {
        result = Cystck_Dup(S, S->Cystck_None);
    } else if (size == 1) {
        int needs_close;
        result = build_single(S, &fmt, &values, &needs_close);
        if (!needs_close) {
            result = Cystck_Dup(S, result);
        }
    } else {
        result = build_tuple(S, &fmt, &values, size, '\0');
    }
    va_end(values);
    Cystck_pushobject(S, result);
    return result;
}

static Cystck_ssize_t count_items(Py_State *S, const char *fmt, char end)
{
    Cystck_ssize_t level = 0, result = 0;
    char top_level_par = 'X';
    while (level != 0 || *fmt != end) {
        char c = *fmt++;
        switch (c) {
            case '\0': {
                // Premature end
                // We try to provide slightly better diagnostics than CPython
                char msg[MESSAGE_BUF_SIZE];
                char par_type;
                if (end == ')') {
                    par_type = '(';
                } else if (end == ']') {
                    par_type = '[';
                } else if (end == '}') {
                    par_type = '{';
                } else {
                    if (level == 0 || top_level_par == 'X') {
                        CystckErr_SetString(S, S->Cystck_SystemError, "internal error in Cystck_BuildValue");
                        return -1;
                    }
                    par_type = top_level_par;
                }
                snprintf(msg, MESSAGE_BUF_SIZE, "unmatched '%c' in the format string passed to Cystck_BuildValue", par_type);
                CystckErr_SetString(S, S->Cystck_SystemError, msg);
                return -1;
            }

            case '[':
            case '(':
            case '{':
                if (level == 0) {
                    top_level_par = c;
                    result++;
                }
                level++;
                break;

            case ']':
            case ')':
            case '}':
                level--;
                break;

            case ',':
            case ' ':
                break;

            default:
                if (level == 0) {
                    result++;
                }
        }
    }
    return result;
}

static Cystck_Object build_single(Py_State *S, const char **fmt, va_list *values, int *needs_close)
{
    char format_char = *(*fmt)++;
    *needs_close = 1;
    switch (format_char) {
        case '(': {
            Cystck_ssize_t size = count_items(S, *fmt, ')');
            if (size < 0) {
                return 0;
            }
            return build_tuple(S, fmt, values, size, ')');
        }

        case '[': {
            Cystck_ssize_t size = count_items(S, *fmt, ']');
            if (size < 0) {
                return 0;
            }
            return build_list(S, fmt, values, size);
        }

        case '{': {
            return build_dict(S, fmt, values);
        }

        case 'i':
            return CystckLong_FromLong(S, (long)va_arg(*values, int));

        case 'I':
            return CystckLong_FromUnsignedLong(S, (unsigned long)va_arg(*values, unsigned int));

        case 'k':
            return CystckLong_FromUnsignedLong(S, va_arg(*values, unsigned long));

        case 'l':
            return CystckLong_FromLong(S, va_arg(*values, long));

        case 'L':
            return CystckLong_FromLongLong(S, va_arg(*values, long long));

        case 'K':
            return CystckLong_FromUnsignedLongLong(S, va_arg(*values, unsigned long long));

        case 's':
            return CystckUnicode_FromString(S, va_arg(*values, const char*));

        case 'O':
        case 'S': {
            Cystck_Object handle = va_arg(*values, Cystck_Object);
            if (Cystck_IsNULL(handle)) {
                if (!Cystck_Err_Occurred(S)) {
                    CystckErr_SetString(S, S->Cystck_SystemError, "0 object passed to Cystck_BuildValue");
                }
                return handle;
            }
            *needs_close = 0;
            return handle;
        }

        case 'N': {
            CystckErr_SetString(S, S->Cystck_SystemError,
                             "Cystck_BuildValue does not support the 'N' formatting unit. "
                             "Instead, use the 'O' formatting unit and manually close "
                             "the handle in the caller if necessary. Cystck_Object API functions "
                             "never 'steal' handles and always make a duplicate handle if "
                             "needed, the 'ownership' of the original handle is never "
                             "'transferred'. ");
            return 0;
        }

        case 'f': // Note: floats are promoted to doubles when passed in "..."
        case 'd':
            return CystckFloat_FromDouble(S, va_arg(*values, double));

        default: {
            char message[MESSAGE_BUF_SIZE];
            snprintf(message, MESSAGE_BUF_SIZE, "bad format char '%c' in the format string passed to Cystck_BuildValue", format_char);
            CystckErr_SetString(S, S->Cystck_SystemError, message);
            return 0;
        }
    } // switch
}

static Cystck_Object build_dict(Py_State *S, const char **fmt, va_list *values)
{
    Cystck_Object dict = CystckDict_New(S);
    int expect_comma = 0;
    while (**fmt != '}' && **fmt != '\0') {
        if (**fmt == ' ') {
            (*fmt)++;
            continue;
        }
        if (**fmt == ',') {
            if (!expect_comma) {
                CystckErr_SetString(S, S->Cystck_SystemError,
                    "unexpected ',' in the format string passed to Cystck_BuildValue");
                Cystck_DECREF(S, dict);
                return 0;
            }
            (*fmt)++;
            expect_comma = 0;
            continue;
        } else {
            if (expect_comma) {
                CystckErr_SetString(S, S->Cystck_SystemError,
                    "missing ',' in the format string passed to Cystck_BuildValue");
                Cystck_DECREF(S, dict);
                return 0;
            }
        }
        int needs_key_close, needs_value_close;
        Cystck_Object key = build_single(S, fmt, values, &needs_key_close);
        if (Cystck_IsNULL(key)) {
            Cystck_DECREF(S, dict);
            return 0;
        }
        if (**fmt != ':') {
            CystckErr_SetString(S, S->Cystck_SystemError,
                            "missing ':' in the format string passed to Cystck_BuildValue");
            if (needs_key_close) {
                Cystck_DECREF(S, key);
            }
            Cystck_DECREF(S, dict);
            return 0;
        } else {
            (*fmt)++;
        }
        Cystck_Object value = build_single(S, fmt, values, &needs_value_close);
        if (Cystck_IsNULL(value)) {
            if (needs_key_close) {
                Cystck_DECREF(S, key);
            }
            Cystck_DECREF(S, dict);
            return 0;
        }
        int res = Cystck_SetItem(S, dict, key, value);
        if (needs_key_close) {
            Cystck_DECREF(S, key);
        }
        if (needs_value_close) {
            Cystck_DECREF(S, value);
        }
        if (res < 0) {
            Cystck_DECREF(S, dict);
            return 0;
        }

        expect_comma = 1;
    }
    if (**fmt != '}') {
        // count_items does not check the type of the matching paren, that's what we do here
        Cystck_DECREF(S, dict);
        CystckErr_SetString(S, S->Cystck_SystemError,
                         "unmatched '{' in the format string passed to Cystck_BuildValue");
        return 0;
    }
    ++*fmt;
    return dict;
}

static Cystck_Object build_list(Py_State *S, const char **fmt, va_list *values, Cystck_ssize_t size)
{
    CystckListBuilder builder = CystckListBuilder_New(S, size);
    for (Cystck_ssize_t i = 0; i < size; ++i) {
        int needs_close;
        Cystck_Object item = build_single(S, fmt, values, &needs_close);
        if (Cystck_IsNULL(item)) {
            CystckListBuilder_Cancel(S, builder);
            return 0;
        }
        CystckListBuilder_Set(S, builder, i, item);
        if (needs_close) {
            Cystck_DECREF(S, item);
        }
        if (**fmt == ',') {
            (*fmt)++;
        }
    }
    if (**fmt != ']') {
        // count_items does not check the type of the matching paren, that's what we do here
        CystckListBuilder_Cancel(S, builder);
        CystckErr_SetString(S, S->Cystck_SystemError,
                         "unmatched '[' in the format string passed to Cystck_BuildValue");
        return 0;
    }
    ++*fmt;
    return CystckListBuilder_Build(S, builder);
}

static Cystck_Object build_tuple(Py_State *S, const char **fmt, va_list *values, Cystck_ssize_t size, char expected_end)
{
    CystckTupleBuilder builder = CystckTupleBuilder_New(S, size);
    for (Cystck_ssize_t i = 0; i < size; ++i) {
        int needs_close;
        Cystck_Object item = build_single(S, fmt, values, &needs_close);
        if (Cystck_IsNULL(item)) {
            CystckTupleBuilder_Cancel(S, builder);
            return 0;
        }
        CystckTupleBuilder_Set(S, builder, i, item);
        if (needs_close) {
            Cystck_DECREF(S, item);
        }
        if (**fmt == ',') {
            (*fmt)++;
        }
    }
    if (**fmt != expected_end) {
        // count_items does not check the type of the matching paren, that's what we do here
        // if expected_end == '\0', then there would have to be a bug in count_items
        CystckTupleBuilder_Cancel(S, builder);
        if (expected_end == '\0') {
            CystckErr_SetString(S, S->Cystck_SystemError, "internal error in Cystck_BuildValue");
        } else {
            CystckErr_SetString(S, S->Cystck_SystemError,
                             "unmatched '[' in the format string passed to Cystck_BuildValue");
        }
        return 0;
    }
    if (expected_end != '\0') {
        ++*fmt;
    }
    return CystckTupleBuilder_Build(S, builder);
}