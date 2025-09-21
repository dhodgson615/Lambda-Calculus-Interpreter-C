#ifndef ENHANCED_WRAPPER_H
#define ENHANCED_WRAPPER_H

/**
 * Enhanced Output Module for Lambda Calculus Interpreter
 * 
 * This module provides colored parentheses and compact mode functionality
 * that can be enabled via compiler directives without modifying existing files.
 * 
 * Usage:
 *   - Define ENABLE_ENHANCED_OUTPUT to activate the module
 *   - Define ENHANCED_OUTPUT_MODE to set the mode:
 *     0 = Normal (default)
 *     1 = Colored parentheses only  
 *     2 = Compact mode only
 *     3 = Both colored and compact
 * 
 * Example:
 *   gcc -DENABLE_ENHANCED_OUTPUT -DENHANCED_OUTPUT_MODE=3 ...
 */

#ifdef ENABLE_ENHANCED_OUTPUT

#include "enhanced_output.h"
#include "types.h"

/* Default mode if not specified */
#ifndef ENHANCED_OUTPUT_MODE
#define ENHANCED_OUTPUT_MODE OUTPUT_MODE_COLORED
#endif

/* Override the original expr_to_buffer function when enhanced output is enabled */
#define expr_to_buffer(e, buf, cap) \
    enhanced_expr_to_buffer((e), (buf), (cap), ENHANCED_OUTPUT_MODE)

/* Also provide a macro for the recursive version if needed */
#define expr_to_buffer_rec(e, buf, pos, cap) \
    enhanced_expr_to_buffer_rec((e), (buf), (pos), (cap), ENHANCED_OUTPUT_MODE, 0)

#endif /* ENABLE_ENHANCED_OUTPUT */

#endif /* ENHANCED_WRAPPER_H */