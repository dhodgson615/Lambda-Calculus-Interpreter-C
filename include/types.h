#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdlib.h>

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

/**
 * @brief              Memory arena structure for temporary allocations.
 */
typedef struct arena {
    uint8        *buf;
    size_t        cap;
    size_t        pos;
} arena;

/**
 * @brief              Variable set structure.
 */
typedef struct VarSet {
    char         **v;
    int            c;
} VarSet;

/**
 * @brief              RGB color structure for pretty printing.
 */
typedef struct color {
    uint8          r;
    uint8          g;
    uint8          b;
} color;

#endif /* TYPES_H */
