#include "../include/enhanced_output.h"
#include "../include/expr.h"
#include "../include/macros.h"
#include "../include/types.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ANSI color codes for parentheses */
static const char* const PAREN_COLORS[] = {
    "\x1b[31m",  /* Red */
    "\x1b[32m",  /* Green */
    "\x1b[33m",  /* Yellow */
    "\x1b[34m",  /* Blue */
    "\x1b[35m",  /* Magenta */
    "\x1b[36m",  /* Cyan */
    "\x1b[91m",  /* Bright Red */
    "\x1b[92m",  /* Bright Green */
    "\x1b[93m",  /* Bright Yellow */
    "\x1b[94m",  /* Bright Blue */
    "\x1b[95m",  /* Bright Magenta */
    "\x1b[96m"   /* Bright Cyan */
};

#define NUM_COLORS (sizeof(PAREN_COLORS) / sizeof(PAREN_COLORS[0]))

/**
 * @brief Add colored parenthesis to buffer
 */
static void add_colored_paren(char *buf, size_t *pos, size_t cap, char paren, int depth) {
    if (*pos >= cap - 1) return;
    
    const char* color = PAREN_COLORS[depth % NUM_COLORS];
    size_t color_len = strlen(color);
    
    /* Add color code */
    if (*pos + color_len < cap - 1) {
        memcpy(buf + *pos, color, color_len);
        *pos += color_len;
    }
    
    /* Add parenthesis */
    if (*pos < cap - 1) {
        buf[(*pos)++] = paren;
    }
    
    /* Add reset code */
    const char* reset = "\x1b[0m";
    size_t reset_len = strlen(reset);
    if (*pos + reset_len < cap - 1) {
        memcpy(buf + *pos, reset, reset_len);
        *pos += reset_len;
    }
}

/**
 * @brief Add regular parenthesis to buffer
 */
static void add_paren(char *buf, size_t *pos, size_t cap, char paren) {
    if (*pos < cap - 1) {
        buf[(*pos)++] = paren;
    }
}

/**
 * @brief Check if compact mode should omit parentheses
 */
static bool should_omit_parens_compact(const expr *e, bool is_function_pos) {
    if (!e) return false;
    
    /* In compact mode, omit parentheses around simple variables */
    if (e->type == VAR_expr) return true;
    
    /* For function position in application, only omit if it's a variable */
    if (is_function_pos && e->type == VAR_expr) return true;
    
    return false;
}

void enhanced_expr_to_buffer_rec(const expr *e, char *buf, size_t *pos, size_t cap, 
                                output_mode_t mode, int depth) {
    if (*pos >= cap - 1 || !e) return;

    const bool use_colors = (mode & OUTPUT_MODE_COLORED) != 0;
    const bool use_compact = (mode & OUTPUT_MODE_COMPACT) != 0;

    switch (e->type) {
        case VAR_expr: {
            size_t L = strlen(e->var_name);
            if (*pos + L > cap - 1) L = cap - 1 - *pos;
            memcpy(buf + *pos, e->var_name, L);
            *pos += L;
            break;
        }

        case ABS_expr: {
            /* Lambda symbol - use Unicode λ in both modes */
            if (*pos + 2 < cap - 1) {
                buf[(*pos)++] = (char) 0xCE;
                buf[(*pos)++] = (char) 0xBB;
            }
            
            /* Parameter name */
            size_t L = strlen(e->abs_param);
            if (*pos + L > cap - 1) L = cap - 1 - *pos;
            memcpy(buf + *pos, e->abs_param, L);
            *pos += L;
            
            /* Dot separator */
            if (*pos < cap - 1) buf[(*pos)++] = '.';
            
            /* Body - add parentheses only if needed */
            bool need_parens = !use_compact || e->abs_body->type == ABS_expr;
            
            if (need_parens) {
                if (use_colors) {
                    add_colored_paren(buf, pos, cap, '(', depth);
                } else {
                    add_paren(buf, pos, cap, '(');
                }
            }
            
            enhanced_expr_to_buffer_rec(e->abs_body, buf, pos, cap, mode, depth + 1);
            
            if (need_parens) {
                if (use_colors) {
                    add_colored_paren(buf, pos, cap, ')', depth);
                } else {
                    add_paren(buf, pos, cap, ')');
                }
            }
            break;
        }

        case APP_expr: {
            /* Function part */
            bool fn_needs_parens = (e->app_fn->type == ABS_expr);
            if (use_compact) {
                fn_needs_parens = fn_needs_parens && !should_omit_parens_compact(e->app_fn, true);
            }
            
            if (fn_needs_parens) {
                if (use_colors) {
                    add_colored_paren(buf, pos, cap, '(', depth);
                } else {
                    add_paren(buf, pos, cap, '(');
                }
            }
            
            enhanced_expr_to_buffer_rec(e->app_fn, buf, pos, cap, mode, depth + 1);
            
            if (fn_needs_parens) {
                if (use_colors) {
                    add_colored_paren(buf, pos, cap, ')', depth);
                } else {
                    add_paren(buf, pos, cap, ')');
                }
            }
            
            /* Space separator (compact mode uses single space) */
            if (*pos < cap - 1) buf[(*pos)++] = ' ';
            
            /* Argument part */
            bool arg_needs_parens = (e->app_arg->type != VAR_expr);
            if (use_compact) {
                arg_needs_parens = arg_needs_parens && !should_omit_parens_compact(e->app_arg, false);
            }
            
            if (arg_needs_parens) {
                if (use_colors) {
                    add_colored_paren(buf, pos, cap, '(', depth);
                } else {
                    add_paren(buf, pos, cap, '(');
                }
            }
            
            enhanced_expr_to_buffer_rec(e->app_arg, buf, pos, cap, mode, depth + 1);
            
            if (arg_needs_parens) {
                if (use_colors) {
                    add_colored_paren(buf, pos, cap, ')', depth);
                } else {
                    add_paren(buf, pos, cap, ')');
                }
            }
            break;
        }
    }
}

void enhanced_expr_to_buffer(const expr *e, char *buf, size_t cap, output_mode_t mode) {
    size_t pos = 0;
    enhanced_expr_to_buffer_rec(e, buf, &pos, cap, mode, 0);
    buf[pos < cap ? pos : cap - 1] = '\0';
}