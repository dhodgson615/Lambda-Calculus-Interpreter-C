#include "../include/parser.h"

#include "../include/expr.h"
#include "../include/macros.h"
#include "../include/types.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

HOT PURE char peek(const Parser *p) {
    if (p->i < p->n) return p->src[p->i];
    return '\0';
}

HOT char consume(Parser *p) {
    if (!peek(p)) return '\0';

    if (is_lambda(p)) {
        p->i += 2;
        return '\0';
    }

    return p->src[p->i++];
}

/* TODO: This function is called very often, so it should be optimized
         for performance. Calls to this function should also be
         consolidated together to reduce redundant computation. */
HOT void skip_whitespace(Parser *p) {
    cchar *src = p->src;
    size_t i = p->i;
    const size_t n = p->n;

    while (i < n && isspace((uchar)src[i])) i++;

    p->i = i;
}

HOT PURE bool is_lambda(const Parser *p) {
    return (p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE)
                             && ((uchar) p->src[p->i + 1] == 0xBB);
}

HOT PURE bool is_invalid_char(const Parser *p, cchar c) {
    return (!c) || (c == '(')
                || (c == ')')
                || (c == '.')
                || (isspace((uchar) c))
                || is_lambda(p);
}

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

HOT expr *parse_expr(Parser *p) {
    skip_whitespace(p);
    return is_lambda(p) ? parse_abs(p) : parse_app(p);
}

HOT expr *parse_abs(Parser *p) {
    p->i += 2; // consume λ
    char *v = parse_varname(p);
    skip_whitespace(p);
    if (consume(p) != '.') {
        fprintf(stderr, "Expected '.' after λ\n");
        exit(1);
    }
    cexpr *body = parse_expr(p);
    expr *ret = make_abstraction(v, body);
    free(v);

    return ret;
}

HOT expr *parse_app(Parser *p) {
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

HOT expr *parse_atom(Parser *p) {
    skip_whitespace(p);
    // Check for lambda as atom
    if (is_lambda(p)) return parse_abs(p);
    
    cchar c = peek(p);
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

HOT char *parse_varname(Parser *p) {
    skip_whitespace(p);
    const unsigned int start = p->i;
    while (p->i < p->n && !is_invalid_char(p, peek(p))) p->i++;
    unsigned int len = p->i - start;
    if (len == 0) {
        fprintf(stderr, "Invalid var start at %zu\n", p->i);
        exit(1);
    }
    char *out = malloc(len + 1);
    if (!out) {
        perror("malloc");
        exit(1);
    }
    memcpy(out, p->src + start, len);
    out[len] = '\0';

    return out;
}
