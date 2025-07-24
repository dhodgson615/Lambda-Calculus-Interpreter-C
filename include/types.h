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

/**
 * @brief              RGB color structure for pretty printing.
 */
typedef struct color {
    uint8          r;
    uint8          g;
    uint8          b;
} color;

#endif /* TYPES_H */
