#include "../include/expr.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

expr *make_variable(const char *n) {
    expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = VAR_expr;
    e->var_name = strdup(n);

    return e;
}

expr *make_abstraction(const char *p, const expr *b) {
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

PURE expr *copy_expr(expr *e) {
    if (!e) return NULL;

    switch (e->type) {
        case VAR_expr:
            return make_variable(e->var_name);
        case ABS_expr:
            return make_abstraction(e->abs_param, copy_expr(e->abs_body));
        case APP_expr:
            return make_application(copy_expr(e->app_fn), copy_expr(e->app_arg));
    }

    return NULL; // unreachable
}

expr *church(const int n) {
    expr *body = make_variable("x");
    for (int i = 0; i < n; i++) {
        expr *fv = make_variable("f");
        body = make_application(fv, body);
    }
    expr *abs_x = make_abstraction("x", body);

    return make_abstraction("f", abs_x);
}

HOT void expr_to_buffer_rec(const expr *e, char *buf, size_t *pos, const size_t cap) {
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

void expr_to_buffer(const expr *e, char *buf, const size_t cap) {
    size_t pos = 0;
    expr_to_buffer_rec(e, buf, &pos, cap);
    buf[pos < cap ? pos : cap - 1] = '\0';
}

PURE bool is_church_numeral(const expr *e) {
    if (e->type != ABS_expr) return false;
    const expr *e1 = e->abs_body;
    if (e1->type != ABS_expr) return false;
    const char *f = e->abs_param;
    const char *x = e1->abs_param;
    const expr *current_expr = e1->abs_body;
    
    while ((current_expr->type == APP_expr) &&
           (current_expr->app_fn->type == VAR_expr) &&
           (!strcmp(current_expr->app_fn->var_name, f))) {
        current_expr = current_expr->app_arg;
    }

    return current_expr->type == VAR_expr && !strcmp(current_expr->var_name, x);
}

PURE int count_applications(const expr *e) {
    const expr *cur = e->abs_body->abs_body;
    const char *f = e->abs_param;
    int n = 0;
    while ((cur->type == APP_expr) && (cur->app_fn->type == VAR_expr) &&
           (!strcmp(cur->app_fn->var_name, f))) {
        n++;
        cur = cur->app_arg;
    }

    return n;
}

expr *abstract_numerals(const expr *e) {
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
