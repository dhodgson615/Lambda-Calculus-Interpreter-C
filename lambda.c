#include "lambda.h"
#include "expr.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

strbuf sb;

static bool CONFIG_SHOW_STEP_TYPE = true;
static bool CONFIG_DELTA_ABSTRACT = true;

/* UNUSED bool get_config_show_step_type(void) { return CONFIG_SHOW_STEP_TYPE; } */

/* UNUSED void set_config_show_step_type(const bool value) { CONFIG_SHOW_STEP_TYPE = value; } */

/* UNUSED bool get_config_delta_abstract(void) { return CONFIG_DELTA_ABSTRACT; } */

/* UNUSED void set_config_delta_abstract(const bool value) { CONFIG_DELTA_ABSTRACT = value; } */

void sb_init(strbuf *sb, const size_t init_cap) {
    sb->data = malloc(init_cap);
    if (!sb->data) {
        perror("malloc for strbuf");
        exit(1);
    }
    sb->cap = init_cap;
    sb->len = 0;
    sb->data[0] = '\0';
}

void sb_ensure(strbuf *sb, const size_t need) {
    if (sb->len + need + 1 > sb->cap) {
        size_t new = sb->cap * 2;
        while (new < sb->len + need + 1) new *= 2;

        sb->data = realloc(sb->data, new);
        if (!sb->data) {
            perror("realloc for strbuf");
            exit(1);
        }
        sb->cap = new;
    }
}

void sb_reset(strbuf *sb) {
    sb->len = 0;
    sb->data[0] = '\0';
}

void sb_destroy(strbuf *sb) {
    free(sb->data);
    sb->data = NULL;
    sb->cap = sb->len = 0;
}

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
    s->v = realloc(s->v, sizeof(char *) * (s->c + 1));
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

HOT PURE INLINE char peek(const Parser *p) {
    if (p->i < p->n) return p->src[p->i];
    return '\0';
}

HOT INLINE char consume(Parser *p) {
    if (!peek(p)) return '\0';

    // UTF-8 λ = 0xCE 0xBB
    /* TODO: Factor this into separate λ detection macros. */
    if ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE) && ((uchar) p->src[p->i + 1] == 0xBB)) {
        p->i += 2;
        return '\0';
    }

    return p->src[p->i++];
}

HOT INLINE void skip_whitespace(Parser *p) { while (isspace((uchar) peek(p))) p->i++; }

expr *parse(Parser *p) {
    skip_whitespace(p);
    expr *e = parse_expr(p);
    skip_whitespace(p);
    if (peek(p)) {
        fprintf(stderr, "Unexpected '%c' at %zu\n", peek(p), p->i);
        exit(1);
    }

    return e;
}

/* TODO: Factor out the λ detection logic into a */
expr *parse_expr(Parser *p) {
    skip_whitespace(p);
    // detect λ
    if ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE) && ((uchar) p->src[p->i + 1] == 0xBB)) {
        return parse_abs(p);
    }

    return parse_app(p);
}

expr *parse_abs(Parser *p) {
    p->i += 2; // consume λ
    char *v = parse_varname(p);
    skip_whitespace(p);
    if (consume(p) != '.') {
        fprintf(stderr, "Expected '.' after λ\n");
        exit(1);
    }
    expr *body = parse_expr(p);
    expr *ret = make_abstraction(v, body);
    free(v);

    return ret;
}

expr *parse_app(Parser *p) {
    skip_whitespace(p);
    expr *e = parse_atom(p);
    skip_whitespace(p);
    char c = peek(p);
    while (c && c != ')' && c != '.') {
        expr *a = parse_atom(p);
        e = make_application(e, a);
        skip_whitespace(p);
        c = peek(p);
    }

    return e;
}

expr *parse_atom(Parser *p) {
    skip_whitespace(p);
    // λ as atom
    if ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE) && ((uchar) p->src[p->i + 1] == 0xBB)) {
        return parse_abs(p);
    }
    const char c = peek(p);
    if (c == '(') {
        consume(p);
        expr *e = parse_expr(p);
        skip_whitespace(p);
        if (consume(p) != ')') {
            fprintf(stderr, "Expected ')'\n");
            exit(1);
        }
        return e;
    }
    if (isdigit((uchar) c)) {
        const int v = parse_number(p);
        return church(v);
    }
    char *name = parse_varname(p);
    expr *v = make_variable(name);
    free(name);

    return v;
}

int parse_number(Parser *p) {
    int v = 0;
    if (!isdigit((uchar) peek(p))) {
        fprintf(stderr, "Expected digit at %zu\n", p->i);
        exit(1);
    }
    while (isdigit((uchar) peek(p))) v = v * 10 + (consume(p) - '0');

    return v;
}

HOT PURE INLINE bool is_invalid_char(const Parser *p, const char c) {
    return (!c) || (c == '(') || (c == ')') || (c == '.') ||
            (isspace((uchar) c)) || ((p->i + 1 < p->n) &&
             ((uchar) p->src[p->i] == 0xCE) &&
              ((uchar) p->src[p->i + 1] == 0xBB));
}

char *parse_varname(Parser *p) {
    skip_whitespace(p);
    char c = peek(p);
    if (is_invalid_char(p, c)) {
        fprintf(stderr, "Invalid var start at %zu\n", p->i);
        exit(1);
    }
    const size_t start = p->i;
    while (p->i < p->n) {
        c = peek(p);
        if (is_invalid_char(p, c)) break;
        p->i++;
    }
    size_t len = p->i - start;
    char *out = malloc(len + 1);
    if (!out) {
        perror("malloc");
        exit(1);
    }
    memcpy(out, p->src + start, len);
    out[len] = '\0';

    return out;
}
