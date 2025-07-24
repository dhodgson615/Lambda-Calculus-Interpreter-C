/* NOTE: The contents of this file are meant to act as a placeholder
         before all of the files of the actual project are factored out
         in order to modularize the build system and isolate the
         components. This file is not meant to be used as a library
         file for the project, but rather as a temporary solution to
         allow the project to compile and run with the current
         structure of the codebase.                                  */

#include "../include/lambda.h"

#include "../include/expr.h"
#include "../include/strbuf.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

strbuf sb;

static bool CONFIG_SHOW_STEP_TYPE = true;
static bool CONFIG_DELTA_ABSTRACT = true;

INLINE void vs_init(VarSet *s) {
    s->v = NULL;
    s->c = 0;
}

bool vs_has(const VarSet *s, const char *x) {
    for (int i = 0; i < s->c; i++) if (!strcmp(s->v[i], x)) return true;

    return false;
}

void vs_add(VarSet *s, const char *x) {
    if (vs_has(s, x)) return;
    if (s->c % 8 == 0) { // Grow in chunks of 8
        s->v = realloc(s->v, sizeof(char *) * (s->c + 8));
        if (!s->v) { perror("realloc"); exit(1); }
    }
    s->v[s->c++] = strdup(x);
}

void vs_rm(VarSet *s, const char *x) {
    for (int i = 0; i < s->c; i++) {
        if (!strcmp(s->v[i], x)) {
            free(s->v[i]);
            memmove(&s->v[i], &s->v[i + 1], sizeof(char *) * (s->c - i - 1));
            s->c--;
            return;
        }
    }
}

void vs_free(const VarSet *s) {
    for (int i = 0; i < s->c; i++) free(s->v[i]);
    free(s->v);
}

void free_vars_rec(const expr *e, VarSet *s) {
    if (e->type == VAR_expr) vs_add(s, e->var_name);
    else if (e->type == ABS_expr) {
        free_vars_rec(e->abs_body, s);
        vs_rm(s, e->abs_param);
    } else {
        free_vars_rec(e->app_fn, s);
        free_vars_rec(e->app_arg, s);
    }
}

VarSet free_vars(const expr *e) {
    VarSet s;
    vs_init(&s);
    free_vars_rec(e, &s);

    return s;
}

char *fresh_var(const VarSet *s) {
    for (char c = 'a'; c <= 'z'; c++) {
        const char buf[2] = {c, '\0'};
        if (!vs_has(s, buf)) return strdup(buf);
    }
    int idx = 1;
    while (true) {
        for (char c = 'a'; c <= 'z'; c++) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%c%d", c, idx);
            if (!vs_has(s, buf)) return strdup(buf);
        }
        idx++;
    }
}

expr *substitute(expr *e, const char *v, expr *val) {
    if (e->type == VAR_expr) return copy_expr(strcmp(e->var_name, v) == 0 ? val : e);

    if (e->type == ABS_expr) {
        if (strcmp(e->abs_param, v) == 0) return copy_expr(e);
        const VarSet fv_val = free_vars(val);
        if (vs_has(&fv_val, e->abs_param)) {
            VarSet forbidden_vars = free_vars(e);
            vs_add(&forbidden_vars, e->abs_param);
            for (int i = 0; i < fv_val.c; i++) vs_add(&forbidden_vars, fv_val.v[i]);

            char *nv_name = fresh_var(&forbidden_vars);
            expr *nv_expr = make_variable(nv_name);
            expr *renamed_body = substitute(e->abs_body, e->abs_param, nv_expr);
            expr *substituted_renamed_body = substitute(renamed_body, v, val);
            expr *result_expr = make_abstraction(nv_name, substituted_renamed_body);

            free_expr(nv_expr);
            free(nv_name);
            free_expr(renamed_body);
            vs_free(&forbidden_vars);
            vs_free(&fv_val);

            return result_expr;
        }
        expr *new_body = substitute(e->abs_body, v, val);
        expr *result_expr = make_abstraction(e->abs_param, new_body);
        vs_free(&fv_val);

        return result_expr;
    }
    expr *substituted_fn = substitute(e->app_fn, v, val);
    expr *substituted_arg = substitute(e->app_arg, v, val);

    return make_application(substituted_fn, substituted_arg);
}

/*
TODO: This is a hacky way to find definitions. Consider using a
      different structure for better performance.
*/
CONST int find_def(const char *s) {
    for (int i = 0; i < N_DEFS; i++) if (!strcmp(def_names[i], s)) return i;

    return -1;
}

HOT bool delta_reduce(const expr *e, expr **out) {
    if (e->type == VAR_expr) {
        const int i = find_def(e->var_name);
        if (i >= 0) {
            *out = copy_expr(def_vals[i]);
            return true;
        }
    }

    return false;
}

HOT bool beta_reduce(const expr *e, expr **out) {
    if ((e->type == APP_expr) && (e->app_fn->type == ABS_expr)) {
        expr *argcp = copy_expr(e->app_arg);
        *out = substitute(e->app_fn->abs_body, e->app_fn->abs_param, argcp);
        free_expr(argcp);
        return true;
    }

    return false;
}

HOT bool reduce_once(const expr *e, expr **ne, const char **rtype) {
    expr *tmp;
    if (delta_reduce(e, &tmp)) {
        *ne = tmp;
        *rtype = "δ";
        return true;
    }
    if (beta_reduce(e, &tmp)) {
        *ne = tmp;
        *rtype = "β";
        return true;
    }
    if (e->type == APP_expr) {
        if (reduce_once(e->app_fn, &tmp, rtype)) {
            *ne = make_application(tmp, copy_expr(e->app_arg));
            return true;
        }
        if (reduce_once(e->app_arg, &tmp, rtype)) {
            *ne = make_application(copy_expr(e->app_fn), tmp);
            return true;
        }
    }
    if (e->type == ABS_expr && reduce_once(e->abs_body, &tmp, rtype)) {
        *ne = make_abstraction(e->abs_param, tmp);
        return true;
    }

    return false;
}

void normalize(expr *e) {
    sb_reset(&sb);
    expr_to_buffer(e, sb.data, sb.cap);
    printf("Step 0: %s\n", sb.data);
    int step = 1;
    while (true) {
        expr *next;
        const char *rtype;
        if (!reduce_once(e, &next, &rtype)) {
            printf("\n→ normal form reached.\n");
            break;
        }
        free_expr(e);
        e = next;
        sb_reset(&sb);
        expr_to_buffer(e, sb.data, sb.cap);

        if (CONFIG_SHOW_STEP_TYPE) printf("Step %d (%s): %s\n", step++, rtype, sb.data);
        else printf("Step %d: %s\n", step++, sb.data);
    }
    if (CONFIG_DELTA_ABSTRACT) {
        expr *abs = abstract_numerals(e);
        sb_reset(&sb);
        expr_to_buffer(abs, sb.data, sb.cap);
        printf("\nδ-abstracted: %s\n", sb.data);
        free_expr(abs);
    }
    free_expr(e);
}
