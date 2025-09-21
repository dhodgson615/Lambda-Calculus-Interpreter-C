#ifndef ENHANCED_OUTPUT_H
#define ENHANCED_OUTPUT_H

#include "types.h"
#include <stddef.h>

/**
 * @brief Enhanced output mode configuration
 */
typedef enum {
    OUTPUT_MODE_NORMAL = 0,     /* Standard parentheses, full format */
    OUTPUT_MODE_COLORED = 1,    /* Colored parentheses */
    OUTPUT_MODE_COMPACT = 2,    /* Compact format */
    OUTPUT_MODE_BOTH = 3        /* Colored + Compact */
} output_mode_t;

/**
 * @brief Convert expression to buffer with enhanced formatting
 * @param e   The expression to convert
 * @param buf The output buffer
 * @param cap The buffer capacity
 * @param mode The output mode to use
 */
void enhanced_expr_to_buffer(const expr *e, char *buf, size_t cap, output_mode_t mode);

/**
 * @brief Convert expression to buffer with enhanced formatting (recursive helper)
 * @param e    The expression to convert
 * @param buf  The output buffer
 * @param pos  Current position in buffer (updated)
 * @param cap  The buffer capacity
 * @param mode The output mode to use
 * @param depth Current nesting depth for color selection
 */
void enhanced_expr_to_buffer_rec(const expr *e, char *buf, size_t *pos, size_t cap, 
                                output_mode_t mode, int depth);

#endif /* ENHANCED_OUTPUT_H */