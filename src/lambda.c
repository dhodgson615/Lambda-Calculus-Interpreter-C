#include "lambda.h"

static strbuf sb;

static bool CONFIG_SHOW_STEP_TYPE = true;
static bool CONFIG_DELTA_ABSTRACT = true;

/**
 * @brief Get the current configuration values.
 * @return the current configuration values
 */
bool get_config_show_step_type(void) {
    return CONFIG_SHOW_STEP_TYPE;
}

/**
 * @brief Set the configuration value for showing step type.
 * @param value the new value for showing step type
 */
void set_config_show_step_type(bool value) {
    CONFIG_SHOW_STEP_TYPE = value;
}

/**
 * @brief Get the current configuration value for delta abstraction.
 * @return the current configuration value for delta abstraction
 */
bool get_config_delta_abstract(void) {
    return CONFIG_DELTA_ABSTRACT;
}

/**
 * @brief Set the configuration value for delta abstraction.
 * @param value the new value for delta abstraction
 */
void set_config_delta_abstract(bool value) {
    CONFIG_DELTA_ABSTRACT = value;
}

/**
 * @brief Initialize a string buffer.
 * @param sb the string buffer to initialize
 * @param init_cap the initial capacity of the buffer
 */
void sb_init(strbuf *sb, size_t init_cap) {
    sb->data = malloc(init_cap);
    if (!sb->data) {
        perror("malloc");
        exit(1);
    }
    sb->cap = init_cap;
    sb->len = 0;
    sb->data[0] = '\0';
}

/**
 * @brief Ensure that the string buffer has enough capacity.
 * @param sb the string buffer to check
 * @param need the amount of space needed
 */
void sb_ensure(strbuf *sb, size_t need) {
    if (sb->len + need + 1 > sb->cap) {
        size_t new = sb->cap * 2;
        while (new < sb->len + need + 1)
            new *= 2;

        sb->data = realloc(sb->data, new);
        if (!sb->data) {
            perror("realloc");
            exit(1);
        }
        sb->cap = new;
    }
}

/**
 * @brief Reset the string buffer.
 * @param sb the string buffer to reset
 */
void sb_reset(strbuf *sb) {
    sb->len = 0;
    sb->data[0] = '\0';
}

/**
 * @brief Destroy the string buffer.
 * @param sb the string buffer to destroy
 */
void sb_destroy(strbuf *sb) {
    free(sb->data);
    sb->data = NULL;
    sb->cap = sb->len = 0;
}

/**
 * @brief Make a new variable expression.
 * @param n the name of the variable
 * @return the new expression
 */
Expr *make_variable(const char *n) {
    Expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = VAR_EXPR;
    e->var_name = strdup(n);
    return e;
}

/**
 * @brief Make a new abstraction expression.
 * @param p the name of the parameter
 * @param b the body of the abstraction
 * @return the new expression
 */
Expr *make_abstraction(const char *p, Expr *b) {
    Expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = ABS_EXPR;
    e->abs_param = strdup(p);
    e->abs_body = b;
    return e;
}

/**
 * @brief Make a new application expression.
 * @param f the function
 * @param a the argument
 * @return the new expression
 */
Expr *make_application(Expr *f, Expr *a) {
    Expr *e = malloc(sizeof *e);
    if (!e) {
        perror("malloc");
        exit(1);
    }
    e->type = APP_EXPR;
    e->app_fn = f;
    e->app_arg = a;
    return e;
}

/**
 * @brief Free an expression.
 * @param e the expression to free
 */
void free_expr(Expr *e) {
    if (!e) return;

    switch (e->type) {
        case VAR_EXPR: free(e->var_name);
                       break;
        case ABS_EXPR: free(e->abs_param);
                       free_expr(e->abs_body);
                       break;
        case APP_EXPR: free_expr(e->app_fn);
                       free_expr(e->app_arg);
                       break;
    }
    free(e);
}

/**
 * @brief Copy an expression.
 * @param e the expression to copy
 * @return the copied expression
 */
Expr *copy_expr(Expr *e) {
    if (!e) return NULL;

    switch (e->type) {
        case VAR_EXPR: return make_variable(e->var_name);
        case ABS_EXPR: return make_abstraction(e->abs_param, copy_expr(e->abs_body));
        case APP_EXPR: return make_application(copy_expr(e->app_fn), copy_expr(e->app_arg));
    }
    return NULL; // unreachable
}

/**
 * @brief Create a Church numeral.
 * @param n the number
 * @return the Church numeral expression
 */
Expr *church(int n) {
    Expr *body = make_variable("x");
    for (int i = 0; i < n; i++) {
        Expr *fv = make_variable("f");
        body = make_application(fv, body);
    }
    Expr *abs_x = make_abstraction("x", body);
    return make_abstraction("f", abs_x);
}

/**
 * @brief Convert an expression to a string.
 * @param e the expression to convert
 * @param buf the buffer to write into
 * @param cap the size of the buffer
 */
void expr_to_buffer_rec(Expr *e, char *buf, size_t *pos, size_t cap) {
    if (*pos >= cap - 1) return;

    switch (e->type) {
        case VAR_EXPR: {
            size_t L = strlen(e->var_name);
            if (*pos + L > cap - 1)
                L = cap - 1 - *pos;

            memcpy(buf + *pos, e->var_name, L);
            *pos += L;
            break;
        }
        case ABS_EXPR:
            // UTF-8 'λ' = 0xCE 0xBB
            if (*pos + 2 < cap - 1) {
                buf[(*pos)++] = (char) 0xCE;
                buf[(*pos)++] = (char) 0xBB;
            }

            size_t L = strlen(e->abs_param);
            if (*pos + L > cap - 1)
                L = cap - 1 - *pos;

            memcpy(buf + *pos, e->abs_param, L);
            *pos += L;

            if (*pos < cap - 1)
                buf[(*pos)++] = '.';

            if (e->abs_body->type == ABS_EXPR) {
                if (*pos < cap - 1)
                    buf[(*pos)++] = '(';

                expr_to_buffer_rec(e->abs_body, buf, pos, cap);

                if (*pos < cap - 1)
                    buf[(*pos)++] = ')';

            } else
                expr_to_buffer_rec(e->abs_body, buf, pos, cap);

            break;

        case APP_EXPR:
            if (e->app_fn->type == ABS_EXPR) {
                if (*pos < cap - 1)
                    buf[(*pos)++] = '(';

                expr_to_buffer_rec(e->app_fn, buf, pos, cap);
                if (*pos < cap - 1)
                    buf[(*pos)++] = ')';

            } else
                expr_to_buffer_rec(e->app_fn, buf, pos, cap);

            if (*pos < cap - 1)
                buf[(*pos)++] = ' ';

            if (e->app_arg->type != VAR_EXPR) {
                if (*pos < cap - 1)
                    buf[(*pos)++] = '(';

                expr_to_buffer_rec(e->app_arg, buf, pos, cap);

                if (*pos < cap - 1)
                    buf[(*pos)++] = ')';

            } else
                expr_to_buffer_rec(e->app_arg, buf, pos, cap);

            break;
    }
}

/**
 * @brief Convert an expression to a string.
 * @param e the expression to convert
 * @param buf the buffer to write into
 * @param cap the size of the buffer
 */
void expr_to_buffer(Expr *e, char *buf, size_t cap) {
    size_t pos = 0;
    expr_to_buffer_rec(e, buf, &pos, cap);
    buf[pos < cap ? pos : cap - 1] = '\0';
}

/**
 * @brief Initialize a variable set.
 * @param s the variable set to initialize
 */
void vs_init(VarSet *s) {
    s->v = NULL;
    s->c = 0;
}

/**
 * @brief Check if a variable is in the set.
 * @param s the variable set
 * @param x the variable to check
 * @return true if the variable is in the set, false otherwise
 */
bool vs_has(VarSet *s, const char *x) {
    for (int i = 0; i < s->c; i++)
        if (!strcmp(s->v[i], x))
            return true;

    return false;
}

/**
 * @brief Add a variable to the set.
 * @param s the variable set
 * @param x the variable to add
 */
void vs_add(VarSet *s, const char *x) {
    if (vs_has(s, x)) return;

    s->v = realloc(s->v, sizeof(char *) * (s->c + 1));
    s->v[s->c++] = strdup(x);
}

/**
 * @brief Remove a variable from the set.
 * @param s the variable set
 * @param x the variable to remove
 */
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

/**
 * @brief Free a variable set.
 * @param s the variable set to free
 */
void vs_free(VarSet *s) {
    for (int i = 0; i < s->c; i++)
        free(s->v[i]);

    free(s->v);
}

/**
 * @brief Recursively find free variables in an expression.
 * @param e the expression to check
 * @param s the variable set to fill
 */
void free_vars_rec(Expr *e, VarSet *s) {
    if (e->type == VAR_EXPR) {
        vs_add(s, e->var_name);
    } else if (e->type == ABS_EXPR) {
        free_vars_rec(e->abs_body, s);
        vs_rm(s, e->abs_param);
    } else {
        free_vars_rec(e->app_fn, s);
        free_vars_rec(e->app_arg, s);
    }
}

/**
 * @brief Find free variables in an expression.
 * @param e the expression to check
 * @return a variable set containing the free variables
 */
VarSet free_vars(Expr *e) {
    VarSet s;
    vs_init(&s);
    free_vars_rec(e, &s);
    return s;
}

/**
 * @brief Create a fresh variable name that is not in the set.
 * @param s the variable set
 * @return a new variable name
 */
char *fresh_var(VarSet *s) {
    for (char c = 'a'; c <= 'z'; c++) {
        char buf[2] = {c, '\0'};
        if (!vs_has(s, buf))
            return strdup(buf);

    }
    int idx = 1;
    while (true) {
        for (char c = 'a'; c <= 'z'; c++) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%c%d", c, idx);
            if (!vs_has(s, buf))
                return strdup(buf);

        }
        idx++;
    }
}

/**
 * @brief Substitute a variable in an expression with another expression.
 * @param e the expression to substitute in
 * @param v the variable to substitute
 * @param val the value to substitute with
 * @return the new expression
 */
Expr *substitute(Expr *e, const char *v, Expr *val) {
    if (e->type == VAR_EXPR) {
        if (strcmp(e->var_name, v) == 0)
            return copy_expr(val);
        else
            return copy_expr(e);
    }

    if (e->type == ABS_EXPR) {
        if (strcmp(e->abs_param, v) == 0)
            return copy_expr(e);

        VarSet fv_val = free_vars(val);

        if (vs_has(&fv_val, e->abs_param)) {
            VarSet forbidden_vars = free_vars(e);
            vs_add(&forbidden_vars, e->abs_param);
            for (int i = 0; i < fv_val.c; i++)
                vs_add(&forbidden_vars, fv_val.v[i]);

            char *nv_name = fresh_var(&forbidden_vars);
            Expr *nv_expr = make_variable(nv_name);

            Expr *renamed_body = substitute(e->abs_body, e->abs_param, nv_expr);
            free_expr(nv_expr);

            Expr *substituted_renamed_body = substitute(renamed_body, v, val);

            Expr *result_expr = make_abstraction(nv_name, substituted_renamed_body);

            free(nv_name);
            free_expr(renamed_body);
            vs_free(&forbidden_vars);
            vs_free(&fv_val);
            return result_expr;
        } else {
            Expr *new_body = substitute(e->abs_body, v, val);
            Expr *result_expr = make_abstraction(e->abs_param, new_body);
            vs_free(&fv_val);
            return result_expr;
        }
    }

    Expr *substituted_fn = substitute(e->app_fn, v, val);
    Expr *substituted_arg = substitute(e->app_arg, v, val);
    return make_application(substituted_fn, substituted_arg);
}

static const char *def_names[N_DEFS] = {"⊤", "⊥", "∧", "∨", "↓", "↑",
                                        "+", "*", "is_zero", "-", "≤", "pair"};
static const char *def_src[N_DEFS] = {"λx.λy.x", "λx.λy.y", "λp.λq.p q p", "λp.λq.p p q",
                                      "λn.λf.λx.n (λg.λh.h (g f)) (λu.x) (λu.u)", "λn.λf.λx.f (n f x)", "λm.λn.m ↑ n",
                                      "λm.λn.m (+ n) 0", "λn.n (λx.⊥) ⊤", "λm.λn.n ↓ m", "λm.λn.is_zero (- m n)",
                                      "λx.λy.λf.f x y"};
static Expr *def_vals[N_DEFS];

/**
 * @brief Find the index of a definition by name.
 * @param s the name of the definition
 * @return the index of the definition, or -1 if not found
 */
int find_def(const char *s) {
    for (int i = 0; i < N_DEFS; i++)
        if (!strcmp(def_names[i], s))
            return i;

    return -1;
}

/**
 * @brief Delta reduction: replace a variable with its definition.
 * @param e the expression to reduce
 * @param out the reduced expression
 * @return true if a reduction was made, false otherwise
 */
bool delta_reduce(Expr *e, Expr **out) {
    if (e->type == VAR_EXPR) {
        int i = find_def(e->var_name);
        if (i >= 0) {
            *out = copy_expr(def_vals[i]);
            return true;
        }
    }
    return false;
}

/**
 * @brief Beta reduction: apply a function to an argument.
 * @param e the expression to reduce
 * @param out the reduced expression
 * @return true if a reduction was made, false otherwise
 */
bool beta_reduce(Expr *e, Expr **out) {
    if ((e->type == APP_EXPR) && (e->app_fn->type == ABS_EXPR)) {
        Expr *argcp = copy_expr(e->app_arg);
        *out = substitute(e->app_fn->abs_body, e->app_fn->abs_param, argcp);
        free_expr(argcp);
        return true;
    }
    return false;
}

/**
 * @brief Reduce an expression once, either by delta or beta reduction.
 * @param e the expression to reduce
 * @param ne the reduced expression
 * @param rtype the type of reduction made
 * @return true if a reduction was made, false otherwise
 */
bool reduce_once(Expr *e, Expr **ne, const char **rtype) {
    Expr *tmp;
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
    if (e->type == APP_EXPR) {
        if (reduce_once(e->app_fn, &tmp, rtype)) {
            *ne = make_application(tmp, copy_expr(e->app_arg));
            return true;
        }
        if (reduce_once(e->app_arg, &tmp, rtype)) {
            *ne = make_application(copy_expr(e->app_fn), tmp);
            return true;
        }
    }
    if (e->type == ABS_EXPR && reduce_once(e->abs_body, &tmp, rtype)) {
        *ne = make_abstraction(e->abs_param, tmp);
        return true;
    }

    return false;
}

/**
 * @brief Check if an expression is a Church numeral.
 * @param e the expression to check
 * @return true if it is a Church numeral, false otherwise
 */
bool is_church_numeral(Expr *e) {
    if (e->type != ABS_EXPR) return false;

    Expr *e1 = e->abs_body;

    if (e1->type != ABS_EXPR)
        return false;

    const char *f = e->abs_param;
    const char *x = e1->abs_param;
    Expr *cur = e1->abs_body;
    while ((cur->type == APP_EXPR) && (cur->app_fn->type == VAR_EXPR)
                                   && (!strcmp(cur->app_fn->var_name, f))) {
        cur = cur->app_arg;
    }
    return (cur->type == VAR_EXPR) && (!strcmp(cur->var_name, x));
}

/**
 * @brief Count the number of applications of a Church numeral.
 * @param e the expression to count
 * @return the number of applications
 */
int count_applications(Expr *e) {
    Expr *cur = e->abs_body->abs_body;
    const char *f = e->abs_param;
    int cnt = 0;
    while ((cur->type == APP_EXPR) && (cur->app_fn->type == VAR_EXPR)
                                   && (!strcmp(cur->app_fn->var_name, f))) {
        cnt++;
        cur = cur->app_arg;
    }
    return cnt;
}

/**
 * @brief Abstract Church numerals in an expression.
 * @param e the expression to abstract
 * @return the abstracted expression
 */
Expr *abstract_numerals(Expr *e) {
    if (is_church_numeral(e)) {
        int n = count_applications(e);
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", n);
        return make_variable(buf);
    }
    if (e->type == ABS_EXPR)
        return make_abstraction(e->abs_param, abstract_numerals(e->abs_body));

    if (e->type == APP_EXPR)
        return make_application(abstract_numerals(e->app_fn), abstract_numerals(e->app_arg));

    return make_variable(e->var_name);
}

/**
 * @brief Normalize an expression by reducing it to normal form.
 * @param expr the expression to normalize
 */
void normalize(Expr *expr) {
    sb_reset(&sb);
    expr_to_buffer(expr, sb.data, sb.cap);
    printf("Step 0: %s\n", sb.data);

    int step = 1;
    while (true) {
        Expr *next;
        const char *rtype;
        if (!reduce_once(expr, &next, &rtype)) {
            printf("→ normal form reached.\n");
            break;
        }
        free_expr(expr);
        expr = next;

        sb_reset(&sb);
        expr_to_buffer(expr, sb.data, sb.cap);

        if (CONFIG_SHOW_STEP_TYPE)
            printf("Step %d (%s): %s\n", step++, rtype, sb.data);
        else
            printf("Step %d: %s\n", step++, sb.data);
    }

    if (CONFIG_DELTA_ABSTRACT) {
        Expr *abs = abstract_numerals(expr);
        sb_reset(&sb);
        expr_to_buffer(abs, sb.data, sb.cap);
        printf("\nδ-abstracted: %s\n", sb.data);
        free_expr(abs);
    }
    free_expr(expr);
}

/**
 * @brief Main function: parse input, normalize, and print result.
 * @param argc the number of arguments
 * @param argv the arguments
 * @return 0 on success, 1 on error
 */
int main(int argc, char *argv[]) {
    // load δ-definitions
    for (int i = 0; i < N_DEFS; i++) {
        Parser dp = {def_src[i], 0, strlen(def_src[i])};
        def_vals[i] = parse(&dp);
    }

    sb_init(&sb, MAX_PRINT_LEN);

    char *input;
    char buf[2048];
    if (argc > 1) {
        size_t L = 0;
        for (int i = 1; i < argc; i++)
            L += strlen(argv[i]) + 1;

        input = malloc(L + 1);
        input[0] = '\0';
        for (int i = 1; i < argc; i++) {
            strcat(input, argv[i]);
            if (i < argc - 1)
                strcat(input, " ");
        }
    } else {
        printf("λ-expr> ");
        if (!fgets(buf, sizeof(buf), stdin))
            return 0;

        buf[strcspn(buf, "\n")] = '\0';
        input = strdup(buf);
    }

    Parser p = {input, 0, strlen(input)};
    Expr *e = parse(&p);
    normalize(e);

    // cleanup
    free(input);
    for (int i = 0; i < N_DEFS; i++)
        free_expr(def_vals[i]);

    sb_destroy(&sb);

    return 0;
}

/**
 * @brief Check if the next character is valid.
 * @param p the parser
 */
char peek(Parser *p) {
    if (p->i < p->n) return p->src[p->i];
    else return '\0';

}

/**
 * @brief Consume the next character and return it.
 * @param p the parser
 * @return the consumed character
 */
char consume(Parser *p) {
    if (!peek(p))
        return '\0';

    // UTF-8 λ = 0xCE 0xBB
    if ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE)
                          && ((uchar) p->src[p->i + 1] == 0xBB)) {
        p->i += 2;
        return '\0';
    }
    return p->src[p->i++];
}

/**
 * @brief Skip whitespace characters in the input.
 * @param p the parser
 */
void skip_whitespace(Parser *p) {
    while (isspace((uchar) peek(p)))
        p->i++;
}

/**
 * @brief Parse a lambda expression from the input.
 * @param p the parser
 * @return the parsed expression
 */
Expr *parse(Parser *p) {
    skip_whitespace(p);
    Expr *e = parse_expr(p);
    skip_whitespace(p);
    if (peek(p)) {
        fprintf(stderr, "Unexpected '%c' at %zu\n", peek(p), p->i);
        exit(1);
    }
    return e;
}

/**
 * @brief Parse an expression from the input.
 * @param p the parser
 * @return the parsed expression
 */
Expr *parse_expr(Parser *p) {
    skip_whitespace(p);
    // detect λ
    if ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE)
                          && ((uchar) p->src[p->i + 1] == 0xBB)) {
        return parse_abs(p);
    }
    return parse_app(p);
}

/**
 * @brief Parse an abstraction from the input.
 * @param p the parser
 * @return the parsed expression
 */
Expr *parse_abs(Parser *p) {
    p->i += 2; // consume λ
    char *v = parse_varname(p);
    skip_whitespace(p);
    if (consume(p) != '.') {
        fprintf(stderr, "Expected '.' after λ\n");
        exit(1);
    }
    Expr *body = parse_expr(p);
    Expr *ret = make_abstraction(v, body);
    free(v);
    return ret;
}

/**
 * @brief Parse an application from the input.
 * @param p the parser
 * @return the parsed expression
 */
Expr *parse_app(Parser *p) {
    skip_whitespace(p);
    Expr *e = parse_atom(p);
    skip_whitespace(p);
    while ((peek(p)) && (peek(p) != ')') && (peek(p) != '.')) {
        Expr *a = parse_atom(p);
        e = make_application(e, a);
        skip_whitespace(p);
    }
    return e;
}

/**
 * @brief Parse an atom from the input.
 * @param p the parser
 * @return the parsed expression
 */
Expr *parse_atom(Parser *p) {
    skip_whitespace(p);
    // λ as atom
    if ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE)
                          && ((uchar) p->src[p->i + 1] == 0xBB)) {
        return parse_abs(p);
    }

    char c = peek(p);

    if (c == '(') {
        consume(p);
        Expr *e = parse_expr(p);
        skip_whitespace(p);
        if (consume(p) != ')') {
            fprintf(stderr, "Expected ')'\n");
            exit(1);
        }
        return e;
    }
    if (isdigit((uchar) c)) {
        int v = parse_number(p);
        return church(v);
    }
    char *name = parse_varname(p);
    Expr *v = make_variable(name);
    free(name);
    return v;
}

/**
 * @brief Parse a number from the input.
 * @param p the parser
 * @return the parsed number
 */
int parse_number(Parser *p) {
    int v = 0;
    if (!isdigit((uchar) peek(p))) {
        fprintf(stderr, "Expected digit at %zu\n", p->i);
        exit(1);
    }
    while (isdigit((uchar) peek(p)))
        v = v * 10 + (consume(p) - '0');

    return v;
}

/**
 * @brief Parse a variable name from the input.
 * @param p the parser
 * @return the parsed variable name
 */
char *parse_varname(Parser *p) {
    skip_whitespace(p);
    char c = peek(p);
    if ((!c) || (isspace((uchar) c))
             || (c == '(')
             || (c == ')')
             || (c == '.')
             || ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE) && ((uchar) p->src[p->i + 1] == 0xBB))) {
        fprintf(stderr, "Invalid var start at %zu\n", p->i);
        exit(1);
    }
    size_t start = p->i;
    while (p->i < p->n) {
        c = peek(p);
        if ((!c) || (isspace((uchar) c))
                 || (c == '(')
                 || (c == ')')
                 || (c == '.')
                 || ((p->i + 1 < p->n) && ((uchar) p->src[p->i] == 0xCE) && ((uchar) p->src[p->i + 1] == 0xBB))) {
            break;
        }
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
