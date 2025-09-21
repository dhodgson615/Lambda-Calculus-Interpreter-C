#include "../include/parser.h"

#include "../include/expr.h"
#include "../include/macros.h"
#include "../include/types.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char peek(const Parser *p) {
    if (p->i < p->n) return p->src[p->i];
    return '\0';
}

char consume(Parser *p) {
    if (p->i < p->n) return p->src[p->i++];
    return '\0';
}

void skip_whitespace(Parser *p) {
    while (p->i < p->n && isspace(p->src[p->i])) p->i++;
}

bool is_lambda(const Parser *p) {
    return (p->i + 1 < p->n) && (p->src[p->i] == 0xCE) && (p->src[p->i + 1] == 0xBB);
}

bool is_invalid_char(const Parser *p, char c) {
    return c != '(' && c != ')' && c != '.' && c != ' ' && c != '\t' && c != '\n'
        && c != '\r' && !isalnum(c) && !is_lambda(p);
}

expr *parse_expr(Parser *p);
expr *parse_abs(Parser *p);
expr *parse_app(Parser *p);
expr *parse_atom(Parser *p);
char *parse_varname(Parser *p);

expr *parse_expr(Parser *p) {
    skip_whitespace(p);
    return is_lambda(p) ? parse_abs(p) : parse_app(p);
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
    expr *lhs = parse_atom(p);
    skip_whitespace(p);

    while (p->i < p->n && peek(p) != ')' && !is_lambda(p)) {
        expr *rhs = parse_atom(p);
        lhs = make_application(lhs, rhs);
        skip_whitespace(p);
    }

    return lhs;
}

expr *parse_atom(Parser *p) {
    skip_whitespace(p);

    if (peek(p) == '(') {
        consume(p); // '('
        expr *e = parse_expr(p);
        skip_whitespace(p);
        if (consume(p) != ')') {
            fprintf(stderr, "Expected ')'\n");
            exit(1);
        }
        return e;
    }

    if (is_lambda(p)) return parse_abs(p);

    char *v = parse_varname(p);
    expr *e = make_variable(v);
    free(v);

    return e;
}

char *parse_varname(Parser *p) {
    skip_whitespace(p);
    size_t start = p->i;
    
    while (p->i < p->n && isalnum(p->src[p->i])) p->i++;
    
    if (p->i == start) {
        fprintf(stderr, "Expected variable name\n");
        exit(1);
    }
    
    size_t len = p->i - start;
    char *name = malloc(len + 1);
    memcpy(name, p->src + start, len);
    name[len] = '\0';
    
    return name;
}

expr *parse(Parser *p) {
    skip_whitespace(p);
    if (p->i >= p->n) {
        fprintf(stderr, "Empty input\n");
        return NULL;
    }

    expr *e = parse_expr(p);
    skip_whitespace(p);

    if (p->i < p->n) {
        fprintf(stderr, "Unexpected character at position %zu\n", p->i);
        free_expr(e);
        return NULL;
    }

    return e;
}