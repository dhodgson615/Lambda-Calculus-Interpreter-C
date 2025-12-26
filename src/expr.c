#include "../include/expr.h"

#include "../include/types.h"
#include "../include/arena.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

expr *make_variable(cchar *n) {
    expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = VAR_expr;
    e->var_name = strdup(n);

    return e;
}

expr *make_abstraction(cchar *p, cexpr *b) {
    expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = ABS_expr;
    e->abs_param = strdup(p);
    e->abs_body = (expr *)b;

    return e;
}

expr *make_application(expr *f, expr *a) {
    expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = APP_expr;
    e->app_fn = f;
    e->app_arg = a;

    return e;
}

void free_expr(expr *e) {
    if (!e) return;

    switch (e->type) {
        case VAR_expr:
            free(e->var_name);
            break;
        case ABS_expr:
            free(e->abs_param);
            free_expr(e->abs_body);
            break;
        case APP_expr:
            free_expr(e->app_fn);
            free_expr(e->app_arg);
            break;
    }
    free(e);
}

/* Internal helper for arena-based string duplication */
static char *arena_strdup(Arena *arena, cchar *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *copy = arena_alloc(arena, len + 1, 1);
    memcpy(copy, s, len + 1);
    return copy;
}

/* Internal helper for arena-based variable creation */
static expr *arena_make_variable(Arena *arena, cchar *n) {
    expr *e = arena_alloc(arena, sizeof(expr), sizeof(void *));
    e->type = VAR_expr;
    e->var_name = arena_strdup(arena, n);
    return e;
}

/* Internal helper for arena-based abstraction creation */
static expr *arena_make_abstraction(Arena *arena, cchar *p, cexpr *b) {
    expr *e = arena_alloc(arena, sizeof(expr), sizeof(void *));
    e->type = ABS_expr;
    e->abs_param = arena_strdup(arena, p);
    e->abs_body = (expr *)b;
    return e;
}

/* Internal helper for arena-based application creation */
static expr *arena_make_application(Arena *arena, expr *f, expr *a) {
    expr *e = arena_alloc(arena, sizeof(expr), sizeof(void *));
    e->type = APP_expr;
    e->app_fn = f;
    e->app_arg = a;
    return e;
}

/* Internal recursive copy with arena allocation */
static expr *copy_expr_arena_internal(expr *e, Arena *arena) {
    if (!e) return NULL;

    switch (e->type) {
        case VAR_expr:
            return arena_make_variable(arena, e->var_name);
        case ABS_expr:
            return arena_make_abstraction(arena, e->abs_param, 
                                          copy_expr_arena_internal(e->abs_body, arena));
        case APP_expr:
            return arena_make_application(arena, 
                                          copy_expr_arena_internal(e->app_fn, arena),
                                          copy_expr_arena_internal(e->app_arg, arena));
    }

    return NULL; // unreachable
}

/* Helper to convert arena-allocated expr tree to heap-allocated tree */
static expr *expr_arena_to_heap(cexpr *e) {
    if (!e) return NULL;

    switch (e->type) {
        case VAR_expr:
            return make_variable(e->var_name);
        case ABS_expr:
            return make_abstraction(e->abs_param, expr_arena_to_heap(e->abs_body));
        case APP_expr:
            return make_application(expr_arena_to_heap(e->app_fn), 
                                    expr_arena_to_heap(e->app_arg));
    }

    return NULL; // unreachable
}

/* 
 * Public API: Copy expression using arena allocator for efficient memory management.
 * 
 * This function uses an arena allocator internally to reduce malloc overhead
 * during the copy operation. The arena allows for efficient bulk allocation
 * and deallocation, which is particularly beneficial when copying large or
 * deeply nested expression trees.
 * 
 * The implementation:
 * 1. Creates a temporary arena for intermediate allocations
 * 2. Copies the expression tree using arena allocation (fast, cache-friendly)
 * 3. Converts the arena-allocated tree to a regular heap-allocated tree
 * 4. Frees the arena in one operation (much faster than individual frees)
 * 
 * The caller receives a standard heap-allocated expression tree and is
 * responsible for freeing it with free_expr().
 */
expr *copy_expr(expr *e) {
    if (!e) return NULL;

    /* Create arena for efficient temporary allocation */
    Arena arena;
    arena_init(&arena, 8192); /* 8KB initial block size */
    
    /* Perform arena-based copy (fast, no malloc overhead per node) */
    expr *arena_copy = copy_expr_arena_internal(e, &arena);
    
    /* Convert to heap-allocated tree for normal lifecycle management */
    expr *heap_copy = expr_arena_to_heap(arena_copy);
    
    /* Free entire arena at once (fast, single operation) */
    arena_free(&arena);
    
    return heap_copy;
}

expr *church(const int n) {
    expr *body = make_variable("x");
    for (int i = 0; i < n; i++) {
        expr *fv = make_variable("f");
        body = make_application(fv, body);
    }
    cexpr *abs_x = make_abstraction("x", body);

    return make_abstraction("f", abs_x);
}

HOT void expr_to_buffer_rec(cexpr *e, char *buf, size_t *pos, const size_t cap) {
    if (*pos >= cap - 1) return;

    switch (e->type) {
        case VAR_expr: {
            size_t L = strlen(e->var_name);
            if (*pos + L > cap - 1) L = cap - 1 - *pos;
            memcpy(buf + *pos, e->var_name, L);
            *pos += L;
            break;
        }

        case ABS_expr: {
            // UTF-8 'λ' = 0xCE 0xBB
            if (*pos + 2 < cap - 1) {
                buf[(*pos)++] = (char) 0xCE;
                buf[(*pos)++] = (char) 0xBB;
            }
            size_t L = strlen(e->abs_param);
            if (*pos + L > cap - 1) L = cap - 1 - *pos;
            memcpy(buf + *pos, e->abs_param, L);
            *pos += L;
            if (*pos < cap - 1) buf[(*pos)++] = '.';
            if (e->abs_body->type == ABS_expr) {
                if (*pos < cap - 1) buf[(*pos)++] = '(';
                expr_to_buffer_rec(e->abs_body, buf, pos, cap);
                if (*pos < cap - 1) buf[(*pos)++] = ')';
            } else expr_to_buffer_rec(e->abs_body, buf, pos, cap);
            break;
        }

        case APP_expr: {
            if (e->app_fn->type == ABS_expr) {
                if (*pos < cap - 1) buf[(*pos)++] = '(';
                expr_to_buffer_rec(e->app_fn, buf, pos, cap);
                if (*pos < cap - 1) buf[(*pos)++] = ')';
            } else expr_to_buffer_rec(e->app_fn, buf, pos, cap);
            if (*pos < cap - 1) buf[(*pos)++] = ' ';
            if (e->app_arg->type != VAR_expr) {
                if (*pos < cap - 1) buf[(*pos)++] = '(';
                expr_to_buffer_rec(e->app_arg, buf, pos, cap);
                if (*pos < cap - 1) buf[(*pos)++] = ')';
            } else expr_to_buffer_rec(e->app_arg, buf, pos, cap);
            break;
        }
    }
}

void expr_to_buffer(cexpr *e, char *buf, const size_t cap) {
    size_t pos = 0;
    expr_to_buffer_rec(e, buf, &pos, cap);
    buf[pos < cap ? pos : cap - 1] = '\0';
}

PURE bool is_church_numeral(cexpr *e) {
    if (e->type != ABS_expr) return false;
    cexpr *e1 = e->abs_body;
    if (e1->type != ABS_expr) return false;
    cchar *f = e->abs_param;
    cchar *x = e1->abs_param;
    cexpr *current_expr = e1->abs_body;
    
    while ((current_expr->type == APP_expr) &&
           (current_expr->app_fn->type == VAR_expr) &&
           (!strcmp(current_expr->app_fn->var_name, f))) {
        current_expr = current_expr->app_arg;
    }

    return current_expr->type == VAR_expr && !strcmp(current_expr->var_name, x);
}

PURE int count_applications(cexpr *e) {
    cexpr *cur = e->abs_body->abs_body;
    cchar *f = e->abs_param;
    int n = 0;
    while ((cur->type == APP_expr) && (cur->app_fn->type == VAR_expr)
                                   && (!strcmp(cur->app_fn->var_name, f))) {
        n++;
        cur = cur->app_arg;
    }

    return n;
}

expr *abstract_numerals(cexpr *e) {
    if (is_church_numeral(e)) {
        const int n = count_applications(e);
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", n);
        return make_variable(buf);
    }
    if (e->type == ABS_expr) 
        return make_abstraction(e->abs_param, abstract_numerals(e->abs_body));
    if (e->type == APP_expr) 
        return make_application(abstract_numerals(e->app_fn), abstract_numerals(e->app_arg));

    return make_variable(e->var_name);
}
