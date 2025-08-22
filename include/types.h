#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdlib.h>

/**
 * @brief              Variable set structure.
 */
typedef struct VarSet {
    char         **v;
    int            c;
} VarSet;

typedef enum {
    VAR_expr, ABS_expr, APP_expr
} exprType;

typedef struct expr {
    exprType       type;
    char          *var_name;
    char          *abs_param;
    struct expr   *abs_body;
    struct expr   *app_fn;
    struct expr   *app_arg;
} expr;

typedef unsigned char          uchar;
typedef unsigned char          byte;
typedef unsigned short         word;
typedef unsigned int           dword;
typedef unsigned long long     qword;
typedef uint8_t                uint8;
typedef uint16_t               uint16;
typedef uint32_t               uint32;
typedef uint64_t               uint64;
typedef int8_t                 int8;
typedef int16_t                int16;
typedef int32_t                int32;
typedef int64_t                int64;
typedef const char             cchar;
typedef const expr             cexpr;
typedef const int              cint;

#endif /* TYPES_H */
